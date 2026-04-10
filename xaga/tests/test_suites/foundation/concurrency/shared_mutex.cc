/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <atomic>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <rainy/foundation/concurrency/shared_mutex.hpp>
#include <thread>
#include <vector>

using namespace rainy::foundation::concurrency;
using namespace std::chrono_literals;

SCENARIO("shared_mutex supports exclusive and shared locking", "[shared_mutex]") {
    GIVEN("a shared_mutex") {
        shared_mutex mtx;

        WHEN("acquiring an exclusive lock") {
            mtx.lock();

            THEN("the lock is held exclusively") {
                REQUIRE_NOTHROW(mtx.unlock());
            }
        }

        WHEN("trying to acquire an exclusive lock") {
            bool acquired = mtx.try_lock();

            THEN("the lock can be acquired immediately") {
                REQUIRE(acquired);
                REQUIRE_NOTHROW(mtx.unlock());
            }
        }

        WHEN("acquiring a shared lock") {
            mtx.lock_shared();

            THEN("the lock is held in shared mode") {
                REQUIRE_NOTHROW(mtx.unlock_shared());
            }
        }

        WHEN("trying to acquire a shared lock") {
            bool acquired = mtx.try_lock_shared();

            THEN("the lock can be acquired immediately") {
                REQUIRE(acquired);
                REQUIRE_NOTHROW(mtx.unlock_shared());
            }
        }
    }
}

SCENARIO("shared_mutex exclusive lock blocks all other locks", "[shared_mutex][concurrency]") {
    GIVEN("a shared_mutex held by an exclusive lock") {
        shared_mutex mtx;
        std::atomic<bool> exclusive_locked{false};
        std::atomic<bool> other_lock_blocked{true};

        WHEN("another thread tries to acquire an exclusive lock") {
            mtx.lock();
            exclusive_locked = true;

            std::thread t([&]() {
                mtx.lock();
                other_lock_blocked = false;
                mtx.unlock();
            });

            std::this_thread::sleep_for(100ms);

            THEN("the other thread is blocked") {
                REQUIRE(exclusive_locked);
                REQUIRE(other_lock_blocked);

                mtx.unlock();
                t.join();
                REQUIRE_FALSE(other_lock_blocked);
            }
        }

        WHEN("another thread tries to acquire a shared lock") {
            mtx.lock();
            exclusive_locked = true;

            std::thread t([&]() {
                mtx.lock_shared();
                other_lock_blocked = false;
                mtx.unlock_shared();
            });

            std::this_thread::sleep_for(100ms);

            THEN("the other thread is blocked") {
                REQUIRE(exclusive_locked);
                REQUIRE(other_lock_blocked);

                mtx.unlock();
                t.join();
                REQUIRE_FALSE(other_lock_blocked);
            }
        }
    }
}

SCENARIO("shared_mutex allows multiple shared locks", "[shared_mutex][concurrency]") {
    GIVEN("a shared_mutex") {
        shared_mutex mtx;
        std::atomic<int> concurrent_readers{0};
        std::atomic<int> max_concurrent_readers{0};

        WHEN("multiple threads acquire shared locks") {
            constexpr int num_readers = 5;
            std::vector<std::thread> threads;

            for (int i = 0; i < num_readers; ++i) {
                threads.emplace_back([&]() {
                    mtx.lock_shared();

                    int current = ++concurrent_readers;
                    int expected = max_concurrent_readers.load();
                    while (current > expected && !max_concurrent_readers.compare_exchange_weak(expected, current)) {
                        expected = max_concurrent_readers.load();
                    }

                    std::this_thread::sleep_for(50ms);
                    --concurrent_readers;

                    mtx.unlock_shared();
                });
            }

            for (auto &t: threads) {
                t.join();
            }

            THEN("multiple readers can hold the lock simultaneously") {
                REQUIRE(max_concurrent_readers > 1);
            }
        }
    }
}

SCENARIO("shared_mutex shared lock blocks exclusive lock", "[shared_mutex][concurrency]") {
    GIVEN("a shared_mutex held by shared locks") {
        shared_mutex mtx;
        std::atomic<bool> shared_locked{false};
        std::atomic<bool> exclusive_blocked{true};

        WHEN("a thread holds a shared lock and another tries exclusive") {
            mtx.lock_shared();
            shared_locked = true;

            std::thread t([&]() {
                mtx.lock();
                exclusive_blocked = false;
                mtx.unlock();
            });

            std::this_thread::sleep_for(100ms);

            THEN("the exclusive lock is blocked") {
                REQUIRE(shared_locked);
                REQUIRE(exclusive_blocked);

                mtx.unlock_shared();
                t.join();
                REQUIRE_FALSE(exclusive_blocked);
            }
        }
    }
}

SCENARIO("shared_mutex try_lock operations don't block", "[shared_mutex]") {
    GIVEN("a shared_mutex held exclusively") {
        shared_mutex mtx;

        WHEN("holding an exclusive lock and trying another exclusive lock") {
            mtx.lock();
            bool acquired = mtx.try_lock();

            THEN("try_lock returns false immediately") {
                REQUIRE_FALSE(acquired);
                mtx.unlock();
            }
        }

        WHEN("holding an exclusive lock and trying a shared lock") {
            mtx.lock();
            bool acquired = mtx.try_lock_shared();

            THEN("try_lock_shared returns false immediately") {
                REQUIRE_FALSE(acquired);
                mtx.unlock();
            }
        }
    }

    GIVEN("a shared_mutex held in shared mode") {
        shared_mutex mtx;

        WHEN("holding a shared lock and trying another shared lock") {
            mtx.lock_shared();
            bool acquired = mtx.try_lock_shared();

            THEN("try_lock_shared succeeds") {
                REQUIRE(acquired);
                mtx.unlock_shared();
                mtx.unlock_shared();
            }
        }

        WHEN("holding a shared lock and trying an exclusive lock") {
            mtx.lock_shared();
            bool acquired = mtx.try_lock();

            THEN("try_lock returns false immediately") {
                REQUIRE_FALSE(acquired);
                mtx.unlock_shared();
            }
        }
    }
}

SCENARIO("shared_lock provides RAII semantics", "[shared_lock]") {
    GIVEN("a shared_mutex") {
        shared_mutex mtx;

        WHEN("creating a shared_lock with default locking") {
            {
                shared_lock<shared_mutex> lock(mtx);

                THEN("the lock is acquired") {
                    REQUIRE(lock.owns_lock());
                    REQUIRE(lock.mutex() == &mtx);
                }
            }

            THEN("the lock is released when destroyed") {
                bool acquired = mtx.try_lock_shared();
                REQUIRE(acquired);
                mtx.unlock_shared();
            }
        }

        WHEN("creating a shared_lock with defer_lock") {
            shared_lock<shared_mutex> lock(mtx, defer_lock);

            THEN("the lock is not acquired initially") {
                REQUIRE_FALSE(lock.owns_lock());
            }

            AND_WHEN("explicitly locking") {
                lock.lock();

                THEN("the lock is acquired") {
                    REQUIRE(lock.owns_lock());
                }
            }
        }

        WHEN("creating a shared_lock with try_to_lock") {
            shared_lock<shared_mutex> lock(mtx, try_to_lock);

            THEN("the lock is acquired if available") {
                REQUIRE(lock.owns_lock());
            }
        }

        WHEN("creating a shared_lock with adopt_lock") {
            mtx.lock_shared();
            shared_lock<shared_mutex> lock(mtx, adopt_lock);

            THEN("the lock assumes ownership") {
                REQUIRE(lock.owns_lock());
            }
        }
    }
}

SCENARIO("shared_lock supports move semantics", "[shared_lock]") {
    GIVEN("a locked shared_lock") {
        shared_mutex mtx;
        shared_lock<shared_mutex> lock1(mtx);

        WHEN("moving to another shared_lock") {
            shared_lock<shared_mutex> lock2(std::move(lock1));

            THEN("ownership is transferred") {
                REQUIRE_FALSE(lock1.owns_lock());
                REQUIRE(lock1.mutex() == nullptr);
                REQUIRE(lock2.owns_lock());
                REQUIRE(lock2.mutex() == &mtx);
            }
        }

        WHEN("move-assigning to another shared_lock") {
            shared_lock<shared_mutex> lock2;
            lock2 = std::move(lock1);

            THEN("ownership is transferred") {
                REQUIRE_FALSE(lock1.owns_lock());
                REQUIRE(lock1.mutex() == nullptr);
                REQUIRE(lock2.owns_lock());
                REQUIRE(lock2.mutex() == &mtx);
            }
        }
    }
}

SCENARIO("shared_lock manual locking and unlocking", "[shared_lock]") {
    GIVEN("a shared_lock with deferred locking") {
        shared_mutex mtx;
        shared_lock<shared_mutex> lock(mtx, defer_lock);

        WHEN("manually locking") {
            lock.lock();

            THEN("the lock is acquired") {
                REQUIRE(lock.owns_lock());
            }

            AND_WHEN("trying to lock again") {
                THEN("an exception is thrown") {
                    REQUIRE_THROWS_AS(lock.lock(), std::system_error);
                }
            }
        }

        WHEN("trying to lock") {
            bool acquired = lock.try_lock();

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                REQUIRE(lock.owns_lock());
            }
        }

        WHEN("locking and then unlocking") {
            lock.lock();
            lock.unlock();

            THEN("the lock is released") {
                REQUIRE_FALSE(lock.owns_lock());
            }

            AND_WHEN("trying to unlock again") {
                THEN("an exception is thrown") {
                    REQUIRE_THROWS_AS(lock.unlock(), std::system_error);
                }
            }
        }
    }
}

SCENARIO("shared_lock release functionality", "[shared_lock]") {
    GIVEN("a locked shared_lock") {
        shared_mutex mtx;
        shared_lock<shared_mutex> lock(mtx);

        WHEN("releasing the mutex") {
            shared_mutex *released = lock.release();

            THEN("ownership is given up without unlocking") {
                REQUIRE(released == &mtx);
                REQUIRE_FALSE(lock.owns_lock());
                REQUIRE(lock.mutex() == nullptr);

                // Must manually unlock the mutex
                mtx.unlock_shared();
            }
        }
    }
}

SCENARIO("shared_lock swap functionality", "[shared_lock]") {
    GIVEN("two shared_locks") {
        shared_mutex mtx1, mtx2;
        shared_lock<shared_mutex> lock1(mtx1);
        shared_lock<shared_mutex> lock2(mtx2);

        WHEN("swapping the locks") {
            lock1.swap(lock2);

            THEN("the mutexes are swapped") {
                REQUIRE(lock1.mutex() == &mtx2);
                REQUIRE(lock2.mutex() == &mtx1);
                REQUIRE(lock1.owns_lock());
                REQUIRE(lock2.owns_lock());
            }
        }
    }
}

SCENARIO("shared_timed_mutex supports timed locking operations", "[shared_timed_mutex]") {
    GIVEN("a shared_timed_mutex") {
        shared_timed_mutex mtx;

        WHEN("trying to lock with a timeout while available") {
            bool acquired = mtx.try_lock_for(100ms);

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                mtx.unlock();
            }
        }

        WHEN("trying to lock_shared with a timeout while available") {
            bool acquired = mtx.try_lock_shared_for(100ms);

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                mtx.unlock_shared();
            }
        }
    }
}

SCENARIO("shared_timed_mutex timeout behavior", "[shared_timed_mutex][concurrency]") {
    GIVEN("a shared_timed_mutex held exclusively") {
        shared_timed_mutex mtx;
        std::atomic<bool> timeout_occurred{false};

        WHEN("another thread tries to acquire with timeout") {
            mtx.lock();

            std::thread t([&]() {
                auto start = std::chrono::steady_clock::now();
                bool acquired = mtx.try_lock_for(100ms);
                auto elapsed = std::chrono::steady_clock::now() - start;

                if (!acquired && elapsed >= 100ms) {
                    timeout_occurred = true;
                }

                if (acquired) {
                    mtx.unlock();
                }
            });

            std::this_thread::sleep_for(200ms);
            mtx.unlock();
            t.join();

            THEN("the timeout occurs") {
                REQUIRE(timeout_occurred);
            }
        }

        WHEN("another thread tries to acquire shared lock with timeout") {
            mtx.lock();

            std::thread t([&]() {
                auto start = std::chrono::steady_clock::now();
                bool acquired = mtx.try_lock_shared_for(100ms);
                auto elapsed = std::chrono::steady_clock::now() - start;

                if (!acquired && elapsed >= 80ms) {
                    timeout_occurred = true;
                }

                if (acquired) {
                    mtx.unlock_shared();
                }
            });

            std::this_thread::sleep_for(200ms);
            mtx.unlock();
            t.join();

            THEN("the timeout occurs") {
                REQUIRE(timeout_occurred);
            }
        }
    }
}

SCENARIO("shared_timed_mutex successful acquisition before timeout", "[shared_timed_mutex][concurrency]") {
    GIVEN("a shared_timed_mutex that will be released") {
        shared_timed_mutex mtx;
        std::atomic<bool> acquired_before_timeout{false};

        WHEN("lock is released before timeout") {
            mtx.lock();

            std::thread t([&]() {
                bool acquired = mtx.try_lock_for(500ms);
                if (acquired) {
                    acquired_before_timeout = true;
                    mtx.unlock();
                }
            });

            std::this_thread::sleep_for(100ms);
            mtx.unlock();
            t.join();

            THEN("the lock is acquired before timeout") {
                REQUIRE(acquired_before_timeout);
            }
        }
    }
}

SCENARIO("shared_timed_mutex try_lock_until operations", "[shared_timed_mutex]") {
    GIVEN("a shared_timed_mutex") {
        shared_timed_mutex mtx;

        WHEN("trying to lock until a future time point") {
            auto deadline = std::chrono::steady_clock::now() + 100ms;
            bool acquired = mtx.try_lock_until(deadline);

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                mtx.unlock();
            }
        }

        WHEN("trying to lock_shared until a future time point") {
            auto deadline = std::chrono::steady_clock::now() + 100ms;
            bool acquired = mtx.try_lock_shared_until(deadline);

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                mtx.unlock_shared();
            }
        }
    }
}

SCENARIO("shared_lock works with shared_timed_mutex", "[shared_lock][shared_timed_mutex]") {
    GIVEN("a shared_timed_mutex") {
        shared_timed_mutex mtx;

        WHEN("creating a shared_lock with timeout constructor") {
            shared_lock<shared_timed_mutex> lock(mtx, 100ms);

            THEN("the lock is acquired") {
                REQUIRE(lock.owns_lock());
            }
        }

        WHEN("using try_lock_for with shared_lock") {
            shared_lock<shared_timed_mutex> lock(mtx, defer_lock);
            bool acquired = lock.try_lock_for(100ms);

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                REQUIRE(lock.owns_lock());
            }
        }

        WHEN("using try_lock_until with shared_lock") {
            shared_lock<shared_timed_mutex> lock(mtx, defer_lock);
            auto deadline = std::chrono::steady_clock::now() + 100ms;
            bool acquired = lock.try_lock_until(deadline);

            THEN("the lock is acquired") {
                REQUIRE(acquired);
                REQUIRE(lock.owns_lock());
            }
        }
    }
}

SCENARIO("shared_timed_mutex writer priority prevents reader starvation", "[shared_timed_mutex][concurrency]") {
    GIVEN("a shared_timed_mutex with readers and a waiting writer") {
        shared_timed_mutex mtx;
        std::atomic<bool> writer_acquired{false};
        std::atomic<int> reader_count{0};

        WHEN("readers hold the lock and a writer is waiting") {
            // Start with readers
            mtx.lock_shared();
            ++reader_count;

            std::thread writer([&]() {
                std::this_thread::sleep_for(50ms);
                mtx.lock();
                writer_acquired = true;
                mtx.unlock();
            });

            // Try to acquire more reader locks
            std::this_thread::sleep_for(100ms);

            std::thread late_reader([&]() {
                if (mtx.try_lock_shared()) {
                    ++reader_count;
                    mtx.unlock_shared();
                }
            });

            std::this_thread::sleep_for(50ms);
            mtx.unlock_shared();

            writer.join();
            late_reader.join();

            THEN("writer is prioritized") {
                REQUIRE(writer_acquired);
            }
        }
    }
}

SCENARIO("stress test: multiple readers and writers", "[shared_timed_mutex][stress]") {
    GIVEN("a shared_timed_mutex and shared data") {
        shared_timed_mutex mtx;
        int shared_data = 0;
        std::atomic<int> read_operations{0};
        std::atomic<int> write_operations{0};

        WHEN("multiple readers and writers access the data concurrently") {
            constexpr int num_readers = 10;
            constexpr int num_writers = 3;
            constexpr int operations_per_thread = 50;

            std::vector<std::thread> threads;

            // Readers
            for (int i = 0; i < num_readers; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < operations_per_thread; ++j) {
                        shared_lock<shared_timed_mutex> lock(mtx);
                        int value = shared_data;
                        ++read_operations;
                        (void) value; // Use value to prevent optimization
                    }
                });
            }

            // Writers
            for (int i = 0; i < num_writers; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < operations_per_thread; ++j) {
                        unique_lock<shared_timed_mutex> lock(mtx);
                        ++shared_data;
                        ++write_operations;
                    }
                });
            }

            for (auto &t: threads) {
                t.join();
            }

            THEN("all operations complete successfully") {
                REQUIRE(read_operations == num_readers * operations_per_thread);
                REQUIRE(write_operations == num_writers * operations_per_thread);
                REQUIRE(shared_data == num_writers * operations_per_thread);
            }
        }
    }
}
