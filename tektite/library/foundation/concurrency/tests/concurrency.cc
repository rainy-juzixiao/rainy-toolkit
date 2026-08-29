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
#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/concurrency/concurrency.hpp>
#include <rainy/foundation/concurrency/shared_mutex.hpp>

#include <vector>

namespace fc = rainy::foundation::concurrency;
namespace this_thread = rainy::foundation::system::this_thread;

TEST_CASE("mutex serializes concurrent increments", "[foundation][concurrency]") {
    fc::mutex mutex;
    int counter = 0;

    constexpr int thread_count = 4;
    constexpr int increments_per_thread = 500;
    std::vector<fc::thread> threads;
    threads.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < increments_per_thread; ++j) {
                fc::unique_lock<fc::mutex> guard{mutex};
                ++counter;
            }
        });
    }
    for (auto &thread : threads) {
        thread.join();
    }
    REQUIRE(counter == thread_count * increments_per_thread);
}

TEST_CASE("mutex try_lock detects ownership", "[foundation][concurrency]") {
    fc::mutex mutex;
    {
        fc::unique_lock<fc::mutex> guard{mutex};
        REQUIRE_FALSE(mutex.try_lock());
    }
    REQUIRE(mutex.try_lock());
    mutex.unlock();
    REQUIRE(mutex.try_lock());
    mutex.unlock();
}

TEST_CASE("condition_variable wakes waiters", "[foundation][concurrency]") {
    fc::mutex mutex;
    fc::condition_variable condition;
    bool ready = false;
    bool notified = false;

    fc::thread waiter([&] {
        fc::unique_lock<fc::mutex> guard{mutex};
        ready = true;
        condition.notify_one();
        condition.wait(guard, [&] { return notified; });
    });

    while (true) {
        {
            fc::unique_lock<fc::mutex> guard{mutex};
            if (ready) {
                break;
            }
        }
        this_thread::yield();
    }
    {
        fc::unique_lock<fc::mutex> guard{mutex};
        notified = true;
    }
    condition.notify_one();
    waiter.join();
    REQUIRE(notified);
}

TEST_CASE("shared_mutex distinguishes exclusive and shared access", "[foundation][concurrency]") {
    fc::shared_mutex mutex;
    int shared_value = 0;

    {
        fc::unique_lock<fc::shared_mutex> exclusive{mutex};
        shared_value += 1;
        REQUIRE(shared_value == 1);
    }
    {
        fc::shared_lock<fc::shared_mutex> reader{mutex};
        REQUIRE(shared_value == 1);
    }
    REQUIRE(shared_value == 1);
}

TEST_CASE("thread runs and joins work", "[foundation][concurrency]") {
    fc::mutex mutex;
    bool executed = false;
    fc::thread thread([&] {
        fc::unique_lock<fc::mutex> guard{mutex};
        executed = true;
    });
    REQUIRE(thread.joinable());
    thread.join();
    REQUIRE_FALSE(thread.joinable());
    REQUIRE(executed);
}
