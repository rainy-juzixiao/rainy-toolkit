/*
* Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain left copy of the License at
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
#include <rainy/foundation/io/executor/strand.hpp>
#include <rainy/foundation/io/io_context.hpp>
#include <atomic>
#include <thread>
#include <vector>

using namespace rainy::foundation::io;

TEST_CASE("strand default construction", "[strand]") {
    io_context ctx;
    strand s(ctx.get_executor());
    REQUIRE_FALSE(s.running_in_this_thread());
}

TEST_CASE("strand copy construction shares impl", "[strand]") {
    io_context ctx;
    strand a(ctx.get_executor());
    strand b(a);
    REQUIRE(a == b);
}

TEST_CASE("strand move construction", "[strand]") {
    io_context ctx;
    strand a(ctx.get_executor());
    strand b(std::move(a));
    REQUIRE_FALSE(b.running_in_this_thread());
}

TEST_CASE("strand copy assignment shares impl", "[strand]") {
    io_context ctx;
    strand a(ctx.get_executor());
    strand b(ctx.get_executor());
    REQUIRE(a != b);
    b = a;
    REQUIRE(a == b);
}

TEST_CASE("strand equality for independent strands", "[strand]") {
    io_context ctx;
    strand a(ctx.get_executor());
    strand b(ctx.get_executor());
    REQUIRE(a != b);
}

TEST_CASE("strand get_inner_executor returns original executor", "[strand]") {
    io_context ctx;
    auto ex = ctx.get_executor();
    strand s(ex);
    REQUIRE(s.get_inner_executor() == ex);
}

TEST_CASE("strand context returns same execution_context", "[strand]") {
    io_context ctx;
    strand s(ctx.get_executor());
    REQUIRE(&s.context() == &ctx);
}

// ─────────────────────── strand serialization ───────────────────────

TEST_CASE("strand serializes handler execution", "[strand]") {
    io_context ctx{4};
    strand s(ctx.get_executor());

    std::atomic<int> counter{0};
    std::atomic<bool> overlap{false};
    std::atomic<bool> running{false};
    constexpr int N = 200;

    for (int i = 0; i < N; ++i) {
        post(s, [&] {
            if (running.exchange(true)) {
                overlap = true;
            }
            std::this_thread::yield();
            running = false;
            ++counter;
        });
    }

    std::vector<std::thread> threads;
    threads.reserve(4);
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]{ ctx.run(); });
    }
    for (auto &t : threads) t.join();

    REQUIRE(counter == N);
    REQUIRE_FALSE(overlap);
}

TEST_CASE("strand running_in_this_thread is true inside dispatch", "[strand]") {
    io_context ctx;
    strand s(ctx.get_executor());

    bool observed = false;
    dispatch(s, [&] {
        observed = s.running_in_this_thread();
    });
    ctx.run();

    REQUIRE(observed);
}

TEST_CASE("strand dispatch runs inline when already in strand", "[strand]") {
    io_context ctx;
    strand s(ctx.get_executor());

    std::vector<int> order;
    post(s, [&] {
        order.push_back(1);
        dispatch(s, [&] {
            order.push_back(2);
        });
        order.push_back(3);
    });
    ctx.run();

    REQUIRE(order == std::vector<int>{1, 2, 3});
}

TEST_CASE("strand post does not run inline when in strand", "[strand]") {
    io_context ctx;
    strand s(ctx.get_executor());

    std::vector<int> order;
    post(s, [&] {
        order.push_back(1);
        post(s, [&] {
            order.push_back(3);
        });
        order.push_back(2);
    });
    ctx.run();

    REQUIRE(order == std::vector<int>{1, 2, 3});
}

TEST_CASE("dispatch(executor, token) executes handler", "[dispatch]") {
    io_context ctx;
    bool called = false;
    dispatch(ctx.get_executor(), [&]{ called = true; });
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("dispatch(context, token) executes handler", "[dispatch]") {
    io_context ctx;
    bool called = false;
    dispatch(ctx, [&]{ called = true; });
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("post(executor, token) executes handler", "[post]") {
    io_context ctx;
    bool called = false;
    post(ctx.get_executor(), [&]{ called = true; });
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("post(context, token) executes handler", "[post]") {
    io_context ctx;
    bool called = false;
    post(ctx, [&]{ called = true; });
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("defer(executor, token) executes handler", "[defer]") {
    io_context ctx;
    bool called = false;
    defer(ctx.get_executor(), [&]{ called = true; });
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("defer(context, token) executes handler", "[defer]") {
    io_context ctx;
    bool called = false;
    defer(ctx, [&]{ called = true; });
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("post does not execute handler before run", "[post]") {
    io_context ctx;
    bool called = false;
    post(ctx, [&]{ called = true; });
    REQUIRE_FALSE(called);
    ctx.run();
    REQUIRE(called);
}

TEST_CASE("multiple post handlers execute in submission order on single thread", "[post]") {
    io_context ctx;
    std::vector<int> order;
    post(ctx, [&]{ order.push_back(1); });
    post(ctx, [&]{ order.push_back(2); });
    post(ctx, [&]{ order.push_back(3); });
    ctx.run();
    REQUIRE(order == std::vector<int>{1, 2, 3});
}

TEST_CASE("post to strand from multiple threads, all handlers complete", "[strand][post]") {
    io_context ctx{4};
    strand s(ctx.get_executor());

    constexpr int N = 100;
    std::atomic<int> count{0};

    std::vector<std::thread> producers;
    producers.reserve(4);
    for (int i = 0; i < 4; ++i) {
        producers.emplace_back([&]{
            for (int j = 0; j < N / 4; ++j) {
                post(s, [&]{ ++count; });
            }
        });
    }
    for (auto &t : producers) t.join();

    std::vector<std::thread> workers;
    workers.reserve(4);
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&]{ ctx.run(); });
    }
    for (auto &t : workers) t.join();

    REQUIRE(count == N);
}

TEST_CASE("defer to strand completes after post within same context", "[strand][defer]") {
    io_context ctx;
    strand s(ctx.get_executor());
    std::vector<int> order;

    post(s, [&]{ order.push_back(1); });
    defer(s, [&]{ order.push_back(2); });
    ctx.run();

    REQUIRE(order.size() == 2);
    REQUIRE(order[0] == 1);
}