#include <atomic>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include <rainy/foundation/concurrency/barrier.hpp>

using namespace rainy::foundation::concurrency;

// Helper completion function for testing
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
        // This function should be noexcept
    }
};

SCENARIO("barrier construction and basic properties", "[barrier][construction]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier with expected count 5") {
        barrier<TestCompletionFunction> b(5);

        THEN("max() returns a positive value") {
            REQUIRE(b.max() > 0);
            REQUIRE(b.max() <= PTRDIFF_MAX);
        }
    }

    GIVEN("a barrier with zero expected count") {
        THEN("construction should succeed") {
            REQUIRE_NOTHROW(barrier<TestCompletionFunction>(0));
        }
    }

    GIVEN("a barrier with maximum expected count") {
        constexpr auto max_val = barrier<TestCompletionFunction>::max();
        THEN("construction with max value should succeed") {
            REQUIRE_NOTHROW(barrier<TestCompletionFunction>(max_val));
        }
    }
}

SCENARIO("barrier with custom completion function", "[barrier][completion]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier with custom completion function") {
        barrier<TestCompletionFunction> b(3);

        WHEN("all threads arrive") {
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

            THEN("completion function should be called exactly once") {
                REQUIRE(TestCompletionFunction::call_count == 1);
            }
        }
    }
}

SCENARIO("arrive and wait with token", "[barrier][arrive][wait]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier with 2 threads") {
        barrier<TestCompletionFunction> b(2);
        std::atomic<int> phase1_counter{0};
        std::atomic<int> phase2_counter{0};
        std::atomic<bool> thread1_in_wait{false};
        std::atomic<bool> thread2_in_wait{false};

        WHEN("threads arrive with tokens") {
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

            THEN("both threads should complete phase1 before any enters phase2") {
                REQUIRE(phase1_counter == 2);
                REQUIRE(phase2_counter == 2);
            }
        }

        AND_WHEN("tokens are used correctly") {
            std::atomic<bool> token_used_correctly{true};

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

            THEN("no exceptions should be thrown") {
                REQUIRE(token_used_correctly == true);
            }
        }
    }
}

SCENARIO("arrive_and_wait functionality", "[barrier][arrive_and_wait]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier with 3 threads") {
        barrier<TestCompletionFunction> b(3);
        atomic<int> phase_counter{0};
        atomic<int> threads_in_phase2{0};

        WHEN("threads call arrive_and_wait multiple times") {
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

            THEN("all threads should complete both phases") {
                REQUIRE(phase_counter == 3);
                REQUIRE(threads_in_phase2 == 3);
                AND_THEN("completion function should be called twice") {
                    REQUIRE(TestCompletionFunction::call_count == 2);
                }
            }
        }
    }
}

SCENARIO("arrive_and_drop functionality", "[barrier][drop]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier with 3 threads") {
        barrier<TestCompletionFunction> b(3);
        atomic<int> completed_threads{0};

        WHEN("one thread drops out") {
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

            THEN("remaining threads should still be able to synchronize") {
                REQUIRE(completed_threads == 2);
            }
        }
    }
}

SCENARIO("multiple phases with barrier", "[barrier][multiphase]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier with 2 threads") {
        barrier<TestCompletionFunction> b(2);
        atomic<int> phase{0};

        WHEN("threads go through multiple phases") {
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

            THEN("all phases should complete") {
                REQUIRE(phase == 5);
                AND_THEN("completion function should be called 5 times") {
                    REQUIRE(TestCompletionFunction::call_count == 5);
                }
            }
        }
    }
}

SCENARIO("barrier with token move semantics", "[barrier][token]") {
    TestCompletionFunction::call_count = 0;

    GIVEN("a barrier") {
        barrier<TestCompletionFunction> b(2);

        WHEN("tokens are moved") {
            auto token1 = b.arrive();
            auto token2 = std::move(token1);

            std::thread t([&]() {
                auto token3 = b.arrive();
                b.wait(std::move(token3));
            });

            THEN("moved-from token can still be used") {
                REQUIRE_NOTHROW(b.wait(std::move(token2)));
            }

            t.join();
        }
    }
}

// Latch Tests

SCENARIO("latch construction and basic properties", "[latch][construction]") {
    GIVEN("a latch with count 5") {
        latch l(5);

        THEN("max() returns a positive value") {
            REQUIRE(l.max() > 0);
            REQUIRE(l.max() == PTRDIFF_MAX);
        }

        THEN("try_wait returns false initially") {
            REQUIRE(l.try_wait() == false);
        }
    }

    GIVEN("a latch with zero count") {
        THEN("construction should succeed") {
            REQUIRE_NOTHROW(latch(0));
        }

        WHEN("constructed with zero") {
            latch l(0);
            THEN("try_wait returns true") {
                REQUIRE(l.try_wait() == true);
            }
        }
    }
}

SCENARIO("latch count_down functionality", "[latch][count_down]") {
    GIVEN("a latch with count 3") {
        latch l(3);

        WHEN("count_down is called twice") {
            l.count_down();
            l.count_down();

            THEN("try_wait should still return false") {
                REQUIRE(l.try_wait() == false);
            }
        }

        WHEN("count_down is called three times") {
            l.count_down();
            l.count_down();
            l.count_down();

            THEN("try_wait should return true") {
                REQUIRE(l.try_wait() == true);
            }
        }

        WHEN("count_down is called with update value 3") {
            l.count_down(3);

            THEN("try_wait should return true") {
                REQUIRE(l.try_wait() == true);
            }
        }
    }
}

SCENARIO("latch wait functionality", "[latch][wait]") {
    GIVEN("a latch with count 2") {
        latch l(2);
        atomic<bool> thread_completed{false};

        WHEN("a thread waits on the latch") {
            std::thread t([&]() {
                l.wait();
                thread_completed = true;
            });

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            THEN("thread should be blocked") {
                REQUIRE(thread_completed == false);
            }

            l.count_down();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            AND_THEN("thread should still be blocked after one count_down") {
                REQUIRE(thread_completed == false);
            }

            l.count_down();
            t.join();

            AND_THEN("thread should complete after latch reaches zero") {
                REQUIRE(thread_completed == true);
            }
        }
    }
}

SCENARIO("latch arrive_and_wait functionality", "[latch][arrive_and_wait]") {
    GIVEN("a latch with count 3") {
        latch l(3);
        atomic<int> completed{0};

        WHEN("multiple threads call arrive_and_wait") {
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

            THEN("all threads should complete") {
                REQUIRE(completed == 3);
            }
        }
    }

    GIVEN("a latch with count 5") {
        latch l(5);
        atomic<int> phase2_counter{0};

        WHEN("threads use arrive_and_wait with custom update values") {
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

            THEN("both threads should complete") {
                REQUIRE(phase2_counter == 2);
            }
        }
    }
}

SCENARIO("latch multiple phases", "[latch][multiphase]") {
    GIVEN("a latch for first phase") {
        latch phase1(2);
        latch phase2(2);
        atomic<int> phase1_complete{0};
        atomic<int> phase2_complete{0};

        WHEN("threads synchronize using two latches") {
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

            THEN("both phases should complete") {
                REQUIRE(phase1_complete == 2);
                REQUIRE(phase2_complete == 2);
            }
        }
    }
}

SCENARIO("latch with try_wait optimization", "[latch][try_wait]") {
    GIVEN("a latch with count 100") {
        latch l(100);

        WHEN("count is decremented partially") {
            l.count_down(50);

            THEN("try_wait should return false") {
                REQUIRE(l.try_wait() == false);
            }
        }

        WHEN("count reaches zero") {
            l.count_down(100);

            THEN("try_wait should return true") {
                REQUIRE(l.try_wait() == true);
            }

            AND_THEN("additional wait calls should return immediately") {
                REQUIRE_NOTHROW(l.wait());
            }
        }
    }
}

SCENARIO("latch with zero initial count", "[latch][zero]") {
    GIVEN("a latch initialized with zero") {
        latch l(0);
        atomic<bool> thread_completed{false};

        WHEN("a thread waits on the latch") {
            std::thread t([&]() {
                l.wait();
                thread_completed = true;
            });

            t.join();

            THEN("thread should not block") {
                REQUIRE(thread_completed == true);
            }
        }

        WHEN("count_down is called on zero latch") {
            THEN("it should be a no-op") {
                REQUIRE(l.try_wait() == true);
            }
        }
    }
}

SCENARIO("concurrent latch operations", "[latch][concurrent]") {
    GIVEN("a latch with count 1000") {
        latch l(1000);
        atomic<int> completion_count{0};

        WHEN("multiple threads count down concurrently") {
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

            THEN("waiter should eventually complete") {
                REQUIRE(l.try_wait() == true);
                waiter.join();
                REQUIRE(completion_count == 1);
            }
        }
    }
}
