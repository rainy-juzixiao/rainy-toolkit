#include <atomic>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include <rainy/foundation/concurrency/barrier.hpp>

using namespace rainy::foundation::concurrency;
using rainy::core::concurrency::atomic;

class TestCompletionFunction {
public:
    TestCompletionFunction() = default;

    void operator()() noexcept {
        call_count++;
    }

    static atomic<int> call_count;
};

atomic<int> TestCompletionFunction::call_count{0};

class ThrowingCompletionFunction {
public:
    void operator()() noexcept {
    }
};

TEST_CASE("barrier construction and basic properties", "[barrier][construction]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier with expected count 5") {
        barrier<TestCompletionFunction> b(5);

        SECTION("max() returns a positive value") {
            REQUIRE(b.max() > 0);
            REQUIRE(b.max() <= PTRDIFF_MAX);
        }
    }

    SECTION("a barrier with zero expected count") {
        SECTION("construction should succeed") {
            REQUIRE_NOTHROW(barrier<TestCompletionFunction>(0));
        }
    }

    SECTION("a barrier with maximum expected count") {
        constexpr auto max_val = barrier<TestCompletionFunction>::max();
        SECTION("construction with max value should succeed") {
            REQUIRE_NOTHROW(barrier<TestCompletionFunction>(max_val));
        }
    }
}

TEST_CASE("barrier with custom completion function", "[barrier][completion]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier with custom completion function") {
        barrier<TestCompletionFunction> b(3);

        SECTION("all threads arrive") {
            std::vector<std::thread> threads;
            for (int i = 0; i < 3; ++i) {
                threads.emplace_back([&]() {
                    auto token = b.arrive();
                    b.wait(std::move(token));
                });
            }

            for (auto &t: threads) {
                t.join();
            }

            SECTION("completion function should be called exactly once") {
                REQUIRE(TestCompletionFunction::call_count == 1);
            }
        }
    }
}

TEST_CASE("arrive and wait with token", "[barrier][arrive][wait]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier with 2 threads") {
        barrier<TestCompletionFunction> b(2);
        atomic<int> phase1_counter{0};
        atomic<int> phase2_counter{0};
        atomic<bool> thread1_in_wait{false};
        atomic<bool> thread2_in_wait{false};

        SECTION("threads arrive with tokens") {
            std::thread t1([&]() {
                auto token = b.arrive();
                phase1_counter++;
                b.wait(std::move(token));
                phase2_counter++;
            });

            std::thread t2([&]() {
                auto token = b.arrive();
                phase1_counter++;
                b.wait(std::move(token));
                phase2_counter++;
            });

            t1.join();
            t2.join();

            SECTION("both threads should complete phase1 before any enters phase2") {
                REQUIRE(phase1_counter == 2);
                REQUIRE(phase2_counter == 2);
            }
        }

        SECTION("tokens are used correctly") {
            atomic<bool> token_used_correctly{true};

            std::thread t1([&]() {
                auto token = b.arrive();
                try {
                    b.wait(std::move(token));
                } catch (...) {
                    token_used_correctly = false;
                }
            });

            std::thread t2([&]() {
                auto token = b.arrive();
                try {
                    b.wait(std::move(token));
                } catch (...) {
                    token_used_correctly = false;
                }
            });

            t1.join();
            t2.join();

            SECTION("no exceptions should be thrown") {
                REQUIRE(token_used_correctly == true);
            }
        }
    }
}

TEST_CASE("arrive_and_wait functionality", "[barrier][arrive_and_wait]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier with 3 threads") {
        barrier<TestCompletionFunction> b(3);
        atomic<int> phase_counter{0};
        atomic<int> threads_in_phase2{0};

        SECTION("threads call arrive_and_wait multiple times") {
            std::vector<std::thread> threads;
            for (int i = 0; i < 3; ++i) {
                threads.emplace_back([&]() {
                    // Phase 1
                    b.arrive_and_wait();
                    phase_counter++;

                    // Phase 2
                    b.arrive_and_wait();
                    threads_in_phase2++;
                });
            }

            for (auto &t: threads) {
                t.join();
            }

            SECTION("all threads should complete both phases") {
                REQUIRE(phase_counter == 3);
                REQUIRE(threads_in_phase2 == 3);
                SECTION("completion function should be called twice") {
                    REQUIRE(TestCompletionFunction::call_count == 2);
                }
            }
        }
    }
}

TEST_CASE("arrive_and_drop functionality", "[barrier][drop]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier with 3 threads") {
        barrier<TestCompletionFunction> b(3);
        atomic<int> completed_threads{0};

        SECTION("one thread drops out") {
            std::thread t1([&]() { b.arrive_and_drop(); });

            std::thread t2([&]() {
                auto token = b.arrive();
                b.wait(std::move(token));
                completed_threads++;
            });

            std::thread t3([&]() {
                auto token = b.arrive();
                b.wait(std::move(token));
                completed_threads++;
            });

            t1.join();
            t2.join();
            t3.join();

            SECTION("remaining threads should still be able to synchronize") {
                REQUIRE(completed_threads == 2);
            }
        }
    }
}

TEST_CASE("multiple phases with barrier", "[barrier][multiphase]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier with 2 threads") {
        barrier<TestCompletionFunction> b(2);
        atomic<int> phase{0};

        SECTION("threads go through multiple phases") {
            std::thread t1([&]() {
                for (int i = 0; i < 5; ++i) {
                    b.arrive_and_wait();
                }
            });

            std::thread t2([&]() {
                for (int i = 0; i < 5; ++i) {
                    b.arrive_and_wait();
                    phase++;
                }
            });

            t1.join();
            t2.join();

            SECTION("all phases should complete") {
                REQUIRE(phase == 5);
                SECTION("completion function should be called 5 times") {
                    REQUIRE(TestCompletionFunction::call_count == 5);
                }
            }
        }
    }
}

TEST_CASE("barrier with token move semantics", "[barrier][token]") {
    TestCompletionFunction::call_count = 0;

    SECTION("a barrier") {
        barrier<TestCompletionFunction> b(2);

        SECTION("tokens are moved") {
            auto token1 = b.arrive();
            auto token2 = std::move(token1);

            std::thread t([&]() {
                auto token3 = b.arrive();
                b.wait(std::move(token3));
            });

            SECTION("moved-from token can still be used") {
                REQUIRE_NOTHROW(b.wait(std::move(token2)));
            }

            t.join();
        }
    }
}

// Latch Tests

TEST_CASE("latch construction and basic properties", "[latch][construction]") {
    SECTION("a latch with count 5") {
        latch l(5);

        SECTION("max() returns a positive value") {
            REQUIRE(l.max() > 0);
            REQUIRE(l.max() == PTRDIFF_MAX);
        }

        SECTION("try_wait returns false initially") {
            REQUIRE(l.try_wait() == false);
        }
    }

    SECTION("a latch with zero count") {
        SECTION("construction should succeed") {
            REQUIRE_NOTHROW(latch(0));
        }

        SECTION("constructed with zero") {
            latch l(0);
            SECTION("try_wait returns true") {
                REQUIRE(l.try_wait() == true);
            }
        }
    }
}

TEST_CASE("latch count_down functionality", "[latch][count_down]") {
    SECTION("a latch with count 3") {
        latch l(3);

        SECTION("count_down is called twice") {
            l.count_down();
            l.count_down();

            SECTION("try_wait should still return false") {
                REQUIRE(l.try_wait() == false);
            }
        }

        SECTION("count_down is called three times") {
            l.count_down();
            l.count_down();
            l.count_down();

            SECTION("try_wait should return true") {
                REQUIRE(l.try_wait() == true);
            }
        }

        SECTION("count_down is called with update value 3") {
            l.count_down(3);

            SECTION("try_wait should return true") {
                REQUIRE(l.try_wait() == true);
            }
        }
    }
}

TEST_CASE("latch wait functionality", "[latch][wait]") {
    SECTION("a latch with count 2") {
        latch l(2);
        atomic<bool> thread_completed{false};

        SECTION("a thread waits on the latch") {
            std::thread t([&]() {
                l.wait();
                thread_completed = true;
            });

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            SECTION("thread should be blocked") {
                REQUIRE(thread_completed == false);
            }

            l.count_down();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            SECTION("thread should still be blocked after one count_down") {
                REQUIRE(thread_completed == false);
            }

            l.count_down();
            t.join();

            SECTION("thread should complete after latch reaches zero") {
                REQUIRE(thread_completed == true);
            }
        }
    }
}

TEST_CASE("latch arrive_and_wait functionality", "[latch][arrive_and_wait]") {
    SECTION("a latch with count 3") {
        latch l(3);
        atomic<int> completed{0};

        SECTION("multiple threads call arrive_and_wait") {
            std::vector<std::thread> threads;
            for (int i = 0; i < 3; ++i) {
                threads.emplace_back([&]() {
                    l.arrive_and_wait(1);
                    completed++;
                });
            }

            for (auto &t: threads) {
                t.join();
            }

            SECTION("all threads should complete") {
                REQUIRE(completed == 3);
            }
        }
    }

    SECTION("a latch with count 5") {
        latch l(5);
        atomic<int> phase2_counter{0};

        SECTION("threads use arrive_and_wait with custom update values") {
            std::thread t1([&]() {
                l.arrive_and_wait(2);
                phase2_counter++;
            });

            std::thread t2([&]() {
                l.arrive_and_wait(3);
                phase2_counter++;
            });

            t1.join();
            t2.join();

            SECTION("both threads should complete") {
                REQUIRE(phase2_counter == 2);
            }
        }
    }
}

TEST_CASE("latch multiple phases", "[latch][multiphase]") {
    SECTION("a latch for first phase") {
        latch phase1(2);
        latch phase2(2);
        atomic<int> phase1_complete{0};
        atomic<int> phase2_complete{0};

        SECTION("threads synchronize using two latches") {
            std::thread t1([&]() {
                // Phase 1 work
                phase1.arrive_and_wait();
                phase1_complete++;

                // Phase 2 work
                phase2.arrive_and_wait();
                phase2_complete++;
            });

            std::thread t2([&]() {
                // Phase 1 work
                phase1.arrive_and_wait();
                phase1_complete++;

                // Phase 2 work
                phase2.arrive_and_wait();
                phase2_complete++;
            });

            t1.join();
            t2.join();

            SECTION("both phases should complete") {
                REQUIRE(phase1_complete == 2);
                REQUIRE(phase2_complete == 2);
            }
        }
    }
}

TEST_CASE("latch with try_wait", "[latch][try_wait]") {
    SECTION("a latch with count 100") {
        latch l(100);

        SECTION("count is decremented partially") {
            l.count_down(50);

            SECTION("try_wait should return false") {
                REQUIRE(l.try_wait() == false);
            }
        }

        SECTION("count reaches zero") {
            l.count_down(100);

            SECTION("try_wait should return true") {
                REQUIRE(l.try_wait() == true);
            }

            SECTION("additional wait calls should return immediately") {
                REQUIRE_NOTHROW(l.wait());
            }
        }
    }
}

TEST_CASE("latch with zero initial count", "[latch][zero]") {
    SECTION("a latch initialized with zero") {
        latch l(0);
        atomic<bool> thread_completed{false};

        SECTION("a thread waits on the latch") {
            std::thread t([&]() {
                l.wait();
                thread_completed = true;
            });

            t.join();

            SECTION("thread should not block") {
                REQUIRE(thread_completed == true);
            }
        }

        SECTION("count_down is called on zero latch") {
            SECTION("it should be a no-op") {
                REQUIRE(l.try_wait() == true);
            }
        }
    }
}

TEST_CASE("concurrent latch operations", "[latch][concurrent]") {
    SECTION("a latch with count 1000") {
        latch l(1000);
        atomic<int> completion_count{0};

        SECTION("multiple threads count down concurrently") {
            std::vector<std::thread> counters;
            for (int i = 0; i < 100; ++i) {
                counters.emplace_back([&]() {
                    for (int j = 0; j < 10; ++j) {
                        l.count_down();
                    }
                });
            }

            std::thread waiter([&]() {
                l.wait();
                completion_count++;
            });

            for (auto &t: counters) {
                t.join();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            SECTION("waiter should eventually complete") {
                REQUIRE(l.try_wait() == true);
                waiter.join();
                REQUIRE(completion_count == 1);
            }
        }
    }
}
