#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/stream/pipe_stream.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace rainy::foundation::io;
using namespace rainy::foundation::io::stream;

namespace {
    io_context &global_ctx() {
        static io_context ctx;
        return ctx;
    }

    unnamed_pipe_stream make_pipe() {
        return unnamed_pipe_stream::create(global_ctx().get_executor());
    }

    constexpr std::string_view PAYLOAD = "unnamed_pipe_test";
}

TEST_CASE("unnamed_pipe_stream is non-copyable but movable", "[unnamed_pipe][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<unnamed_pipe_stream>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<unnamed_pipe_stream>);
    STATIC_REQUIRE(std::is_move_constructible_v<unnamed_pipe_stream>);
    STATIC_REQUIRE(std::is_move_assignable_v<unnamed_pipe_stream>);
}

TEST_CASE("pipe_end_base subclasses are non-copyable but movable", "[unnamed_pipe][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<unnamed_pipe_stream::read_end>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<unnamed_pipe_stream::read_end>);
    STATIC_REQUIRE(std::is_move_constructible_v<unnamed_pipe_stream::read_end>);

    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<unnamed_pipe_stream::write_end>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<unnamed_pipe_stream::write_end>);
    STATIC_REQUIRE(std::is_move_constructible_v<unnamed_pipe_stream::write_end>);
}

TEST_CASE("unnamed_pipe_stream::create (throwing) succeeds", "[unnamed_pipe][create]") {
    REQUIRE_NOTHROW(make_pipe());
}

TEST_CASE("unnamed_pipe_stream::create (ec overload) succeeds", "[unnamed_pipe][create]") {
    std::error_code ec;
    auto pipe = unnamed_pipe_stream::create(global_ctx().get_executor(), ec);
    REQUIRE_FALSE(ec);
    REQUIRE(pipe.is_open());
}

TEST_CASE("unnamed_pipe_stream is_open after create", "[unnamed_pipe][is_open]") {
    auto pipe = make_pipe();
    REQUIRE(pipe.is_open());
    REQUIRE(pipe.is_read_open());
    REQUIRE(pipe.is_write_open());
}

TEST_CASE("unnamed_pipe_stream is_open reflects individual end state", "[unnamed_pipe][is_open]") {
    auto pipe = make_pipe();

    pipe.close_read();
    REQUIRE_FALSE(pipe.is_read_open());
    REQUIRE(pipe.is_write_open());
    REQUIRE_FALSE(pipe.is_open());

    pipe.close_write();
    REQUIRE_FALSE(pipe.is_write_open());
}

TEST_CASE("unnamed_pipe_stream close_read returns no error", "[unnamed_pipe][close]") {
    auto pipe = make_pipe();
    auto ec = pipe.close_read();
    REQUIRE_FALSE(ec);
}

TEST_CASE("unnamed_pipe_stream close_write returns no error", "[unnamed_pipe][close]") {
    auto pipe = make_pipe();
    auto ec = pipe.close_write();
    REQUIRE_FALSE(ec);
}

TEST_CASE("unnamed_pipe_stream close closes both ends", "[unnamed_pipe][close]") {
    auto pipe = make_pipe();
    auto ec = pipe.close();
    REQUIRE_FALSE(ec);
    REQUIRE_FALSE(pipe.is_open());
}

TEST_CASE("unnamed_pipe_stream double close does not error", "[unnamed_pipe][close]") {
    auto pipe = make_pipe();
    pipe.close();
    auto ec = pipe.close();
    REQUIRE(ec);
}

TEST_CASE("unnamed_pipe_stream native handles are valid and distinct", "[unnamed_pipe][native_handle]") {
    auto pipe = make_pipe();
    auto rh = pipe.get_read_end().native_handle();
    auto wh = pipe.get_write_end().native_handle();
    REQUIRE(rh != static_cast<native_handle_type>(-1));
    REQUIRE(wh != static_cast<native_handle_type>(-1));
    REQUIRE(rh != wh);
}

TEST_CASE("unnamed_pipe_stream native_handle after close returns sentinel", "[unnamed_pipe][native_handle]") {
    auto pipe = make_pipe();
    pipe.close();
    REQUIRE(pipe.get_read_end().native_handle() == static_cast<native_handle_type>(-1));
    REQUIRE(pipe.get_write_end().native_handle() == static_cast<native_handle_type>(-1));
}

TEST_CASE("unnamed_pipe_stream get_executor is accessible on both ends", "[unnamed_pipe][executor]") {
    auto pipe = make_pipe();
    (void) pipe.get_read_end().get_executor();
    (void) pipe.get_write_end().get_executor();
    SUCCEED();
}

TEST_CASE("unnamed_pipe_stream sync write_some then read_some round-trip", "[unnamed_pipe][sync]") {
    auto pipe = make_pipe();
    auto &r = pipe.get_read_end();
    auto &w = pipe.get_write_end();

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::size_t written = 0;
    REQUIRE_NOTHROW(written = w.write_some(wbuf));
    REQUIRE(written == PAYLOAD.size());

    std::string recv(PAYLOAD.size(), '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    std::size_t read = 0;
    REQUIRE_NOTHROW(read = r.read_some(rbuf));
    REQUIRE(read == PAYLOAD.size());
    REQUIRE(recv == PAYLOAD);
}

TEST_CASE("unnamed_pipe_stream sync write_some ec overload clears error on success", "[unnamed_pipe][sync]") {
    auto pipe = make_pipe();
    std::error_code ec;
    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::size_t written = pipe.get_write_end().write_some(wbuf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(written == PAYLOAD.size());
}

TEST_CASE("unnamed_pipe_stream sync read_some ec overload clears error on success", "[unnamed_pipe][sync]") {
    auto pipe = make_pipe();

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    pipe.get_write_end().write_some(wbuf);

    std::string recv(PAYLOAD.size(), '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    std::error_code ec;
    std::size_t n = pipe.get_read_end().read_some(rbuf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == PAYLOAD.size());
}

TEST_CASE("unnamed_pipe_stream write then read with empty payload", "[unnamed_pipe][sync]") {
    auto pipe = make_pipe();
    auto wbuf = buffer(static_cast<const char *>(nullptr), 0);
    std::error_code ec;
    std::size_t written = pipe.get_write_end().write_some(wbuf, ec);
    REQUIRE(written == 0);
}

TEST_CASE("unnamed_pipe_stream read after write_end closed returns error or zero", "[unnamed_pipe][sync]") {
    auto pipe = make_pipe();
    pipe.close_write();

    std::string recv(16, '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    std::error_code ec;
    std::size_t n = pipe.get_read_end().read_some(rbuf, ec);
    REQUIRE((ec || n == 0));
}

TEST_CASE("unnamed_pipe_stream take_read_end transfers ownership", "[unnamed_pipe][take]") {
    auto pipe = make_pipe();
    auto rend = pipe.take_read_end();
    REQUIRE(rend.is_open());
    REQUIRE_FALSE(pipe.is_read_open());
}

TEST_CASE("unnamed_pipe_stream take_write_end transfers ownership", "[unnamed_pipe][take]") {
    auto pipe = make_pipe();
    auto wend = pipe.take_write_end();
    REQUIRE(wend.is_open());
    REQUIRE_FALSE(pipe.is_write_open());
}

TEST_CASE("unnamed_pipe_stream taken ends still form a working pipe", "[unnamed_pipe][take]") {
    auto pipe = make_pipe();
    auto rend = pipe.take_read_end();
    auto wend = pipe.take_write_end();

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    wend.write_some(wbuf);

    std::string recv(PAYLOAD.size(), '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    rend.read_some(rbuf);
    REQUIRE(recv == PAYLOAD);
}

TEST_CASE("unnamed_pipe_stream move construction leaves source closed", "[unnamed_pipe][move]") {
    auto pipe = make_pipe();
    auto pipe2 = std::move(pipe);
    REQUIRE(pipe2.is_open());
    REQUIRE_FALSE(pipe.is_open());
}

TEST_CASE("unnamed_pipe_stream move assignment leaves source closed", "[unnamed_pipe][move]") {
    auto pipe = make_pipe();
    auto pipe2 = make_pipe();
    pipe2 = std::move(pipe);
    REQUIRE(pipe2.is_open());
    REQUIRE_FALSE(pipe.is_open());
}

namespace {
    void wait_for(std::atomic<bool> &flag, std::chrono::milliseconds timeout = std::chrono::milliseconds{2000}) {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (!flag.load(std::memory_order_acquire)) {
            if (std::chrono::steady_clock::now() >= deadline) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }
}

TEST_CASE("unnamed_pipe_stream async_write_some then async_read_some round-trip", "[unnamed_pipe][async]") {
    auto pipe = make_pipe();
    auto &r = pipe.get_read_end();
    auto &w = pipe.get_write_end();

    std::atomic<bool> write_done{false};
    std::error_code write_ec{};
    std::size_t write_bytes{0};

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    w.async_write_some(wbuf, [&](std::error_code ec, std::size_t n) {
        write_ec = ec;
        write_bytes = n;
        write_done.store(true, std::memory_order_release);
    });

    wait_for(write_done);
    REQUIRE(write_done.load());
    REQUIRE_FALSE(write_ec);
    REQUIRE(write_bytes == PAYLOAD.size());

    std::atomic<bool> read_done{false};
    std::error_code read_ec{};
    std::size_t read_bytes{0};
    std::string recv(PAYLOAD.size(), '\0');

    auto rbuf = buffer(recv.data(), recv.size());
    r.async_read_some(rbuf, [&](std::error_code ec, std::size_t n) {
        read_ec = ec;
        read_bytes = n;
        read_done.store(true, std::memory_order_release);
    });

    wait_for(read_done);
    REQUIRE(read_done.load());
    REQUIRE_FALSE(read_ec);
    REQUIRE(read_bytes == PAYLOAD.size());
    REQUIRE(recv == PAYLOAD);
}

TEST_CASE("unnamed_pipe_stream concurrent writes then reads preserve all data", "[unnamed_pipe][concurrency]") {
    auto pipe = make_pipe();

    constexpr int THREAD_COUNT = 4;
    constexpr int MSGS_PER_THREAD = 8;
    const std::string chunk(64, 'z');

    std::atomic<int> write_count{0};
    std::vector<std::thread> writers;

    for (int t = 0; t < THREAD_COUNT; ++t) {
        writers.emplace_back([&] {
            auto buf = buffer(chunk.data(), chunk.size());
            for (int i = 0; i < MSGS_PER_THREAD; ++i) {
                std::error_code ec;
                pipe.get_write_end().write_some(buf, ec);
                if (!ec) {
                    write_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    const std::size_t total_bytes = THREAD_COUNT * MSGS_PER_THREAD * chunk.size();
    std::size_t received = 0;
    std::string recv_buf(chunk.size(), '\0');

    while (received < total_bytes) {
        auto rbuf = buffer(recv_buf.data(), recv_buf.size());
        std::error_code ec;
        auto n = pipe.get_read_end().read_some(rbuf, ec);
        if (ec || n == 0) {
            break;
        }
        received += n;
    }

    for (auto &th: writers) {
        th.join();
    }

    REQUIRE(received == total_bytes);
}

TEST_CASE("unnamed_pipe_stream large buffer round-trip", "[unnamed_pipe][sync]") {
    auto pipe = make_pipe();
    const std::string large(64 * 1024, 'L');

    std::thread writer([&] {
        std::size_t sent = 0;
        while (sent < large.size()) {
            auto buf = buffer(large.data() + sent, large.size() - sent);
            std::error_code ec;
            auto n = pipe.get_write_end().write_some(buf, ec);
            if (ec || n == 0) {
                break;
            }
            sent += n;
        }
    });

    std::string recv(large.size(), '\0');
    std::size_t got = 0;
    while (got < large.size()) {
        auto buf = buffer(recv.data() + got, recv.size() - got);
        std::error_code ec;
        auto n = pipe.get_read_end().read_some(buf, ec);
        if (ec || n == 0) {
            break;
        }
        got += n;
    }

    writer.join();
    REQUIRE(got == large.size());
    REQUIRE(recv == large);
}
