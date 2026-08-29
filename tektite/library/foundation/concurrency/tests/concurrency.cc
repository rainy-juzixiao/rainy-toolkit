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
#include <catch2/catch_all.hpp>
#include <rainy/foundation/concurrency/barrier.hpp>
#include <rainy/foundation/concurrency/concurrency.hpp>
#include <rainy/foundation/concurrency/shared_mutex.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <sstream>
#include <vector>

using namespace rainy::foundation;
using namespace std::chrono_literals;

namespace {
    constexpr int thread_count = 4;
    constexpr int increments_per_thread = 500;
}

TEST_CASE("mutex basic operations", "[concurrency]") {
    SECTION("lock/unlock") {
        concurrency::mutex mutex;
        int counter = 0;
        std::vector<std::unique_ptr<concurrency::thread>> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.push_back(std::make_unique<concurrency::thread>([&] {
                for (int j = 0; j < increments_per_thread; ++j) {
                    concurrency::unique_lock<concurrency::mutex> guard{mutex};
                    ++counter;
                }
            }));
        }
        for (auto &thread : threads) {
            thread->join();
        }
        REQUIRE(counter == thread_count * increments_per_thread);
    }
    SECTION("try_lock") {
        concurrency::mutex mutex;
        {
            concurrency::unique_lock<concurrency::mutex> guard{mutex};
            REQUIRE_FALSE(mutex.try_lock());
        }
        REQUIRE(mutex.try_lock());
        mutex.unlock();
        REQUIRE(mutex.try_lock());
        mutex.unlock();
    }
}

TEST_CASE("timed_mutex", "[concurrency]") {
    concurrency::timed_mutex mutex;
    REQUIRE(mutex.try_lock());
    mutex.unlock();

    concurrency::unique_lock<concurrency::timed_mutex> guard{mutex};
    REQUIRE_FALSE(mutex.try_lock_for(10ms));
    guard.unlock();
    REQUIRE(mutex.try_lock_for(10ms));
    guard.release()->unlock();
}

TEST_CASE("condition_variable", "[concurrency]") {
    SECTION("wait with predicate") {
        concurrency::mutex mutex;
        concurrency::condition_variable condition;
        bool notified = false;
        bool ready = false;

        concurrency::thread waiter([&] {
            concurrency::unique_lock<concurrency::mutex> guard{mutex};
            ready = true;
            condition.notify_one();
            condition.wait(guard, [&] { return notified; });
        });
        while (true) {
            {
                concurrency::unique_lock<concurrency::mutex> guard{mutex};
                if (ready) {
                    break;
                }
            }
            system::this_thread::yield();
        }
        {
            concurrency::unique_lock<concurrency::mutex> guard{mutex};
            notified = true;
        }
        condition.notify_one();
        waiter.join();
        REQUIRE(notified);
    }
    SECTION("wait_for timeout") {
        concurrency::mutex mutex;
        concurrency::condition_variable condition;
        concurrency::unique_lock<concurrency::mutex> guard{mutex};
        const bool status = condition.wait_for(guard, 10ms, [] { return false; });
        REQUIRE_FALSE(status);
    }
    SECTION("notify_all") {
        concurrency::mutex mutex;
        concurrency::condition_variable condition;
        std::atomic<int> wakeups{0};
        constexpr int waiter_count = 3;

        std::vector<std::unique_ptr<concurrency::thread>> waiters;
        for (int i = 0; i < waiter_count; ++i) {
            waiters.push_back(std::make_unique<concurrency::thread>([&] {
                concurrency::unique_lock<concurrency::mutex> guard{mutex};
                condition.wait(guard);
                ++wakeups;
            }));
        }
        system::this_thread::sleep_for(50ms);
        condition.notify_all();
        for (auto &waiter : waiters) {
            waiter->join();
        }
        REQUIRE(wakeups.load() == waiter_count);
    }
}

TEST_CASE("shared_mutex", "[concurrency]") {
    concurrency::shared_mutex mutex;
    std::atomic<int> readers_inside{0};
    std::atomic<int> max_concurrent_readers{0};

    constexpr int reader_count = 3;
    std::vector<std::unique_ptr<concurrency::thread>> readers;
    for (int i = 0; i < reader_count; ++i) {
        readers.push_back(std::make_unique<concurrency::thread>([&] {
            concurrency::shared_lock<concurrency::shared_mutex> guard{mutex};
            const int inside = ++readers_inside;
            int current = max_concurrent_readers.load();
            while (inside > current && !max_concurrent_readers.compare_exchange_weak(current, inside)) {
            }
            system::this_thread::sleep_for(20ms);
            --readers_inside;
        }));
    }
    system::this_thread::sleep_for(5ms);
    {
        concurrency::unique_lock<concurrency::shared_mutex> writer{mutex};
        REQUIRE(readers_inside.load() == 0);
    }
    for (auto &reader : readers) {
        reader->join();
    }
    REQUIRE(max_concurrent_readers.load() >= 2);
}

TEST_CASE("thread", "[concurrency]") {
    SECTION("join") {
        concurrency::mutex mutex;
        int value = 0;
        concurrency::thread thread([&] {
            concurrency::unique_lock<concurrency::mutex> guard{mutex};
            value = 42;
        });
        REQUIRE(thread.joinable());
        thread.join();
        REQUIRE_FALSE(thread.joinable());
        REQUIRE(value == 42);
    }
    SECTION("auto_join policy") {
        std::atomic<int> value{0};
        {
            concurrency::thread thread(concurrency::thread::policy::auto_join, [&] { value.store(7); });
            REQUIRE(thread.joinable());
        }
        REQUIRE(value.load() == 7);
    }
    SECTION("detach") {
        std::atomic<bool> done{false};
        concurrency::thread thread([&] { done.store(true); });
        thread.detach();
        REQUIRE_FALSE(thread.joinable());
        while (!done.load()) {
            system::this_thread::yield();
        }
        REQUIRE(done.load());
    }
    SECTION("id") {
        concurrency::thread::id self = system::this_thread::get_id();
        REQUIRE(self == system::this_thread::get_id());
        concurrency::thread::id other{};
        REQUIRE(self != other);
        std::hash<concurrency::thread::id> hasher;
        REQUIRE(hasher(self) == hasher(self));
        std::ostringstream stream;
        stream << self;
        REQUIRE_FALSE(stream.str().empty());
    }
    SECTION("make_thread") {
        std::atomic<int> result{0};
        concurrency::thread thread = concurrency::make_thread([&] { result.store(9); });
        thread.join();
        REQUIRE(result.load() == 9);
    }
    SECTION("sleep_for") {
        const auto start = std::chrono::steady_clock::now();
        system::this_thread::sleep_for(20ms);
        system::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start >= 15ms);
    }
}

TEST_CASE("work_stealing_deque", "[concurrency]") {
    SECTION("empty deque") {
        concurrency::work_stealing_deque deque;
        REQUIRE(deque.empty());
        REQUIRE(deque.size() == 0);
        REQUIRE_FALSE(deque.pop().has_value());
        REQUIRE_FALSE(deque.steal().has_value());
    }
    SECTION("pop drains from the front, steal from the back") {
        concurrency::work_stealing_deque deque;
        std::vector<int> order;
        deque.push([&] { order.push_back(1); });
        deque.push([&] { order.push_back(2); });
        deque.push([&] { order.push_back(3); });
        REQUIRE(deque.size() == 3);

        deque.pop().value()();
        deque.steal().value()();
        deque.pop().value()();
        REQUIRE((order == std::vector<int>{1, 3, 2}));
        REQUIRE(deque.empty());
    }
}

TEST_CASE("barrier", "[concurrency]") {
    constexpr int participants = 4;
    concurrency::barrier barrier(participants);
    std::atomic<int> released{0};

    std::vector<std::unique_ptr<concurrency::thread>> threads;
    for (int i = 0; i < participants; ++i) {
        threads.push_back(std::make_unique<concurrency::thread>([&] {
            barrier.arrive_and_wait();
            ++released;
        }));
    }
    for (auto &thread : threads) {
        thread->join();
    }
    REQUIRE(released.load() == participants);
}

TEST_CASE("future and promise", "[concurrency]") {
    SECTION("set_value and get") {
        concurrency::promise<int> promise;
        concurrency::future<int> future = promise.get_future();
        REQUIRE(future.valid());

        concurrency::thread producer(concurrency::thread::policy::auto_join, [&] { promise.set_value(42); });
        producer.join();

        REQUIRE(future.get() == 42);
        REQUIRE_FALSE(future.valid());
    }
    SECTION("wait_for") {
        concurrency::promise<int> promise;
        concurrency::future<int> future = promise.get_future();

        REQUIRE(future.wait_for(10ms) == concurrency::future_status::timeout);
        promise.set_value(1);
        REQUIRE(future.wait_for(10ms) == concurrency::future_status::ready);
        REQUIRE(future.get() == 1);
    }
    SECTION("exception propagation") {
        concurrency::promise<int> promise;
        concurrency::future<int> future = promise.get_future();

        promise.set_exception(std::make_exception_ptr(std::runtime_error("boom")));
        REQUIRE_THROWS_AS(future.get(), std::runtime_error);
    }
}

TEST_CASE("actor pools", "[concurrency]") {
    SECTION("pooled_actor_pool") {
        concurrency::pooled_actor_pool pool(2);
        std::atomic<int> completed{0};
        for (int i = 0; i < 20; ++i) {
            pool.submit([&completed] { ++completed; });
        }
        pool.wait_all();
        REQUIRE(completed.load() == 20);
        pool.stop();
        pool.join();
    }
    SECTION("dedicated_actor_pool") {
        concurrency::dedicated_actor_pool pool(2);
        std::atomic<int> completed{0};
        for (int i = 0; i < 10; ++i) {
            pool.submit([&completed] { ++completed; });
        }
        pool.wait_all();
        REQUIRE(completed.load() == 10);
        pool.stop();
        pool.join();
    }
}

TEST_CASE("executor", "[concurrency]") {
    concurrency::pooled_actor_pool pool(2);
    concurrency::executor executor(pool);

    auto future = executor.submit([] { return 40 + 2; });
    REQUIRE(future.get() == 42);

    auto void_future = executor.submit([] {});
    void_future.get();

    pool.stop();
    pool.join();
}
