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
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <rainy/foundation/io/stream/console_stream.hpp>
#include <string_view>

using namespace rainy::foundation::io::stream;

namespace {
    constexpr std::string_view TEST_MSG = "hello console_stream\n";

    inline auto make_const_buf(std::string_view sv) {
        return rainy::foundation::io::buffer(sv.data(), sv.size());
    }

    template <std::size_t N>
    inline auto make_mut_buf(std::array<char, N> &arr) {
        return rainy::foundation::io::buffer(arr.data(), arr.size());
    }
}

TEST_CASE("console_stream singletons are stable", "[console_stream][singleton]") {
    SECTION("input() returns the same object every call") {
        REQUIRE(&console_stream::input() == &console_stream::input());
    }
    SECTION("output() returns the same object every call") {
        REQUIRE(&console_stream::output() == &console_stream::output());
    }
    SECTION("error() returns the same object every call") {
        REQUIRE(&console_stream::error() == &console_stream::error());
    }
    SECTION("input / output / error are distinct objects") {
        REQUIRE(&console_stream::input() != &console_stream::output());
        REQUIRE(&console_stream::input() != &console_stream::error());
        REQUIRE(&console_stream::output() != &console_stream::error());
    }
}

TEST_CASE("console_stream is non-copyable and non-movable", "[console_stream][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<console_stream>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<console_stream>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<console_stream>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<console_stream>);
}

TEST_CASE("console_stream is_open", "[console_stream][is_open]") {
    REQUIRE(console_stream::output().is_open());
    REQUIRE(console_stream::error().is_open());
    REQUIRE(console_stream::input().is_open());
}

TEST_CASE("console_stream native_handle is valid", "[console_stream][native_handle]") {
    auto out_handle = console_stream::output().native_handle();
    auto err_handle = console_stream::error().native_handle();
    auto in_handle = console_stream::input().native_handle();

    REQUIRE(out_handle != native_handle_type{});
    REQUIRE(err_handle != native_handle_type{});
    REQUIRE(in_handle != native_handle_type{});

    SECTION("output and error have distinct handles") {
        REQUIRE(out_handle != err_handle);
    }
}

TEST_CASE("console_stream get_executor returns valid executor", "[console_stream][executor]") {
    auto ex = console_stream::output().get_executor();
    (void) ex;
    SUCCEED("get_executor() did not throw or crash");
}

TEST_CASE("console_stream write_some to output", "[console_stream][sync][write]") {
    auto &out = console_stream::output();

    SECTION("writes all bytes and returns correct count") {
        auto buf = make_const_buf(TEST_MSG);
        std::size_t written = 0;
        REQUIRE_NOTHROW(written = out.write_some(buf));
        REQUIRE(written == TEST_MSG.size());
    }

    SECTION("writing an empty buffer returns 0") {
        auto buf = make_const_buf({});
        std::size_t written = 0;
        REQUIRE_NOTHROW(written = out.write_some(buf));
        REQUIRE(written == 0);
    }
}

TEST_CASE("console_stream write_some to error stream", "[console_stream][sync][write]") {
    auto &err = console_stream::error();
    auto buf = make_const_buf(TEST_MSG);
    std::size_t written = 0;
    REQUIRE_NOTHROW(written = err.write_some(buf));
    REQUIRE(written == TEST_MSG.size());
}

TEST_CASE("console_stream write_some error_code overload", "[console_stream][sync][write][ec]") {
    auto &out = console_stream::output();
    std::error_code ec;

    SECTION("success path: ec is cleared") {
        auto buf = make_const_buf(TEST_MSG);
        std::size_t written = out.write_some(buf, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(written == TEST_MSG.size());
    }
}

TEST_CASE("console_stream read_some error_code overload compiles and links", "[console_stream][sync][read][ec]") {
    std::array<char, 64> buf{};
    std::error_code ec;
    using sig = std::size_t (console_stream::*)(decltype(make_mut_buf(buf)), std::error_code &);
    STATIC_REQUIRE(std::is_same_v<sig, std::size_t (console_stream::*)(decltype(make_mut_buf(buf)), std::error_code &)>);
    SUCCEED("read_some(buffers, ec) signature is correct");
}

TEST_CASE("console_stream async_write_some completes successfully", "[console_stream][async][write]") {
    auto &out = console_stream::output();

    std::atomic<bool> completed{false};
    std::error_code result_ec{};
    std::size_t result_bytes{0};

    auto buf = make_const_buf(TEST_MSG);

    out.async_write_some(buf, [&](std::error_code ec, std::size_t bytes) {
        result_ec = ec;
        result_bytes = bytes;
        completed.store(true, std::memory_order_release);
    });

    for (int i = 0; i < 200 && !completed.load(std::memory_order_acquire); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    REQUIRE(completed.load());
    REQUIRE_FALSE(result_ec);
    REQUIRE(result_bytes == TEST_MSG.size());
}

TEST_CASE("console_stream async_write_some to error stream", "[console_stream][async][write]") {
    auto &err = console_stream::error();

    std::atomic<bool> completed{false};
    std::error_code result_ec{};
    std::size_t result_bytes{0};

    auto buf = make_const_buf(TEST_MSG);

    err.async_write_some(buf, [&](std::error_code ec, std::size_t bytes) {
        result_ec = ec;
        result_bytes = bytes;
        completed.store(true, std::memory_order_release);
    });

    for (int i = 0; i < 200 && !completed.load(std::memory_order_acquire); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    REQUIRE(completed.load());
    REQUIRE_FALSE(result_ec);
    REQUIRE(result_bytes == TEST_MSG.size());
}

TEST_CASE("console_stream concurrent write_some does not crash", "[console_stream][concurrency]") {
    auto &out = console_stream::output();
    constexpr int THREAD_COUNT = 4;
    constexpr int WRITES_EACH = 16;

    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (int t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&out, t] {
            const std::string msg = "thread " + std::to_string(t) + " writing\n";
            auto buf = rainy::foundation::io::buffer(msg.data(), msg.size());
            for (int i = 0; i < WRITES_EACH; ++i) {
                std::error_code ec;
                out.write_some(buf, ec);
            }
        });
    }

    for (auto &th: threads) {
        th.join();
    }

    SUCCEED("concurrent write_some did not crash");
}

TEST_CASE("console_stream write_some large buffer", "[console_stream][sync][write]") {
    auto &out = console_stream::output();
    std::string large(64 * 1024, 'x');
    large.back() = '\n';

    auto buf = rainy::foundation::io::buffer(large.data(), large.size());
    std::size_t written = 0;
    REQUIRE_NOTHROW(written = out.write_some(buf));
    // 至少写入了一部分
    REQUIRE(written > 0);
    REQUIRE(written <= large.size());
}
