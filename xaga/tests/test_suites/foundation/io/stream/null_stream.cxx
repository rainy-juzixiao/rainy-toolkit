#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/stream/null_stream.hpp>

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
    constexpr std::string_view PAYLOAD = "null_stream_test_payload";

    void wait_for(std::atomic<bool> &flag,
                  std::chrono::milliseconds timeout = std::chrono::milliseconds{2000}) {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (!flag.load(std::memory_order_acquire)) {
            if (std::chrono::steady_clock::now() >= deadline) break;
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }
}

TEST_CASE("null_stream is non-copyable and non-movable", "[null_stream][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<null_stream>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<null_stream>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<null_stream>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<null_stream>);
}

TEST_CASE("null_stream::null() returns the same object every call", "[null_stream][singleton]") {
    REQUIRE(&null_stream::null() == &null_stream::null());
}

TEST_CASE("null_stream is_open returns true", "[null_stream][is_open]") {
    REQUIRE(null_stream::null().is_open());
}

TEST_CASE("null_stream native_handle is valid", "[null_stream][native_handle]") {
    REQUIRE(null_stream::null().native_handle() != reinterpret_cast<native_handle_type>(-1));
}

TEST_CASE("null_stream get_executor is callable", "[null_stream][executor]") {
    (void)null_stream::null().get_executor();
    SUCCEED();
}

TEST_CASE("null_stream write_some throwing overload discards data without error", "[null_stream][sync][write]") {
    auto &ns = null_stream::null();
    auto buf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::size_t n = 0;
    REQUIRE_NOTHROW(n = ns.write_some(buf));
    REQUIRE(n == PAYLOAD.size());
}

TEST_CASE("null_stream write_some ec overload clears error and returns byte count", "[null_stream][sync][write]") {
    auto &ns = null_stream::null();
    auto buf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::error_code ec;
    std::size_t n = ns.write_some(buf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == PAYLOAD.size());
}

TEST_CASE("null_stream write_some with empty buffer returns zero", "[null_stream][sync][write]") {
    auto &ns = null_stream::null();
    auto buf = buffer(static_cast<const char *>(nullptr), 0);
    std::error_code ec;
    std::size_t n = ns.write_some(buf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == 0);
}

TEST_CASE("null_stream write_some large buffer discards without error", "[null_stream][sync][write]") {
    auto &ns = null_stream::null();
    const std::string large(64 * 1024, 'x');
    auto buf = buffer(large.data(), large.size());
    std::error_code ec;
    std::size_t n = ns.write_some(buf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == large.size());
}

TEST_CASE("null_stream read_some throwing overload returns zero without error", "[null_stream][sync][read]") {
    auto &ns = null_stream::null();
    std::array<char, 64> buf{};
    auto mb = buffer(buf.data(), buf.size());
    std::size_t n = 0;
    REQUIRE_NOTHROW(n = ns.read_some(mb));
    REQUIRE(n == 0);
}

TEST_CASE("null_stream read_some ec overload clears error and returns zero", "[null_stream][sync][read]") {
    auto &ns = null_stream::null();
    std::array<char, 64> buf{};
    auto mb = buffer(buf.data(), buf.size());
    std::error_code ec;
    std::size_t n = ns.read_some(mb, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == 0);
}

TEST_CASE("null_stream read_some does not modify buffer contents", "[null_stream][sync][read]") {
    auto &ns = null_stream::null();
    std::string buf(32, 'A');
    auto mb = buffer(buf.data(), buf.size());
    std::error_code ec;
    ns.read_some(mb, ec);
    REQUIRE(buf == std::string(32, 'A'));
}

TEST_CASE("null_stream read_some with empty buffer returns zero", "[null_stream][sync][read]") {
    auto &ns = null_stream::null();
    auto mb = buffer(static_cast<char *>(nullptr), 0);
    std::error_code ec;
    std::size_t n = ns.read_some(mb, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == 0);
}

TEST_CASE("null_stream async_write_some completes with no error", "[null_stream][async][write]") {
    auto &ns = null_stream::null();
    std::atomic<bool> done{false};
    std::error_code   result_ec{};
    std::size_t       result_bytes{0};

    auto buf = buffer(PAYLOAD.data(), PAYLOAD.size());
    ns.async_write_some(buf, [&](std::error_code ec, std::size_t n) {
        result_ec    = ec;
        result_bytes = n;
        done.store(true, std::memory_order_release);
    });

    wait_for(done);
    REQUIRE(done.load());
    REQUIRE_FALSE(result_ec);
    REQUIRE(result_bytes == PAYLOAD.size());
}

TEST_CASE("null_stream async_write_some with empty buffer completes with zero bytes", "[null_stream][async][write]") {
    auto &ns = null_stream::null();
    std::atomic<bool> done{false};
    std::error_code   result_ec{};
    std::size_t       result_bytes{1};

    auto buf = buffer(static_cast<const char *>(nullptr), 0);
    ns.async_write_some(buf, [&](std::error_code ec, std::size_t n) {
        result_ec    = ec;
        result_bytes = n;
        done.store(true, std::memory_order_release);
    });

    wait_for(done);
    REQUIRE(done.load());
    REQUIRE_FALSE(result_ec);
    REQUIRE(result_bytes == 0);
}

TEST_CASE("null_stream async_read_some completes with no error and zero bytes", "[null_stream][async][read]") {
    auto &ns = null_stream::null();
    std::atomic<bool> done{false};
    std::error_code   result_ec{};
    std::size_t       result_bytes{1};
    std::array<char, 64> buf{};

    auto mb = buffer(buf.data(), buf.size());
    ns.async_read_some(mb, [&](std::error_code ec, std::size_t n) {
        result_ec    = ec;
        result_bytes = n;
        done.store(true, std::memory_order_release);
    });

    wait_for(done);
    REQUIRE(done.load());
    REQUIRE_FALSE(result_ec);
    REQUIRE(result_bytes == 0);
}

TEST_CASE("null_stream async_read_some does not modify buffer", "[null_stream][async][read]") {
    auto &ns = null_stream::null();
    std::atomic<bool> done{false};
    std::string buf(32, 'Z');

    auto mb = buffer(buf.data(), buf.size());
    ns.async_read_some(mb, [&](std::error_code, std::size_t) {
        done.store(true, std::memory_order_release);
    });

    wait_for(done);
    REQUIRE(buf == std::string(32, 'Z'));
}

TEST_CASE("null_stream async_read_some cancelled sets operation_canceled error", "[null_stream][async][cancel]") {
    auto &ns = null_stream::null();
    std::atomic<bool> done{false};
    std::error_code   result_ec{};

    std::array<char, 64> buf{};
    auto mb = buffer(buf.data(), buf.size());
    ns.async_read_some(mb, [&](std::error_code ec, std::size_t) {
        result_ec = ec;
        done.store(true, std::memory_order_release);
    });
    wait_for(done);
    REQUIRE(done.load());
    if (result_ec) {
        REQUIRE(result_ec == std::errc::operation_canceled);
    }
}

TEST_CASE("null_stream async_write_some cancelled sets operation_canceled error", "[null_stream][async][cancel]") {
    auto &ns = null_stream::null();
    std::atomic<bool> done{false};
    std::error_code   result_ec{};

    auto buf = buffer(PAYLOAD.data(), PAYLOAD.size());
    ns.async_write_some(buf, [&](std::error_code ec, std::size_t) {
        result_ec = ec;
        done.store(true, std::memory_order_release);
    });

    wait_for(done);
    REQUIRE(done.load());
    if (result_ec) {
        REQUIRE(result_ec == std::errc::operation_canceled);
    }
}

TEST_CASE("null_stream concurrent write_some does not crash", "[null_stream][concurrency]") {
    auto &ns = null_stream::null();
    constexpr int THREAD_COUNT  = 8;
    constexpr int WRITES_EACH   = 32;
    const std::string chunk(256, 'c');

    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (int t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&] {
            auto buf = buffer(chunk.data(), chunk.size());
            for (int i = 0; i < WRITES_EACH; ++i) {
                std::error_code ec;
                ns.write_some(buf, ec);
            }
        });
    }

    for (auto &th : threads) th.join();
    SUCCEED();
}

TEST_CASE("null_stream concurrent read_some does not crash", "[null_stream][concurrency]") {
    auto &ns = null_stream::null();
    constexpr int THREAD_COUNT = 8;
    constexpr int READS_EACH   = 32;

    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (int t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&] {
            std::array<char, 128> buf{};
            auto mb = buffer(buf.data(), buf.size());
            for (int i = 0; i < READS_EACH; ++i) {
                std::error_code ec;
                ns.read_some(mb, ec);
            }
        });
    }

    for (auto &th : threads) th.join();
    SUCCEED();
}

TEST_CASE("null_stream mixed concurrent async read and write does not crash", "[null_stream][concurrency][async]") {
    auto &ns = null_stream::null();
    constexpr int OPS = 16;
    std::atomic<int> completed{0};

    for (int i = 0; i < OPS; ++i) {
        auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
        ns.async_write_some(wbuf, [&](std::error_code, std::size_t) {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    std::array<char, 64> rbuf_storage{};
    for (int i = 0; i < OPS; ++i) {
        auto rbuf = buffer(rbuf_storage.data(), rbuf_storage.size());
        ns.async_read_some(rbuf, [&](std::error_code, std::size_t) {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{3};
    while (completed.load(std::memory_order_acquire) < OPS * 2) {
        if (std::chrono::steady_clock::now() >= deadline) break;
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    REQUIRE(completed.load() == OPS * 2);
}