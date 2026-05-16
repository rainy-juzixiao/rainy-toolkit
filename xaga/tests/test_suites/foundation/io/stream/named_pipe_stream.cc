#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/stream/pipe_stream.hpp>

#include <atomic>
#include <chrono>
#include <string>
#include <string_view>
#include <thread>

using namespace rainy;
using namespace rainy::foundation::io;
using namespace rainy::foundation::io::stream;

namespace {
    io_context &global_ctx() {
        static io_context ctx;
        return ctx;
    }

    foundation::text::string_view test_pipe_name() {
#if RAINY_USING_WINDOWS
        return R"(\\.\pipe\rainy_test_pipe)";
#else
        return "/tmp/rainy_test_pipe";
#endif
    }

    constexpr std::string_view PAYLOAD = "named_pipe_test_payload";

    void wait_for(std::atomic<bool> &flag, std::chrono::milliseconds timeout = std::chrono::milliseconds{3000}) {
        global_ctx().run();
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (!flag.load(std::memory_order_acquire)) {
            if (std::chrono::steady_clock::now() >= deadline) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
        global_ctx().restart();
    }
}

TEST_CASE("named_pipe_stream is non-copyable but movable", "[named_pipe][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<named_pipe_stream>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<named_pipe_stream>);
    STATIC_REQUIRE(std::is_move_constructible_v<named_pipe_stream>);
    STATIC_REQUIRE(std::is_move_assignable_v<named_pipe_stream>);
}

TEST_CASE("named_pipe_stream open_server succeeds", "[named_pipe][open]") {
    REQUIRE_NOTHROW(named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name()));
}

TEST_CASE("named_pipe_stream open_server with explicit direction succeeds", "[named_pipe][open]") {
    REQUIRE_NOTHROW(named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout));
}

TEST_CASE("named_pipe_stream open_server ec overload clears error on success", "[named_pipe][open]") {
    std::error_code ec;
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(server.is_open());
}

TEST_CASE("named_pipe_stream open_client connects to existing server", "[named_pipe][open]") {
    std::error_code ec_s;
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout, ec_s);
    REQUIRE_FALSE(ec_s);

    std::error_code ec_c;
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout, ec_c);
    REQUIRE_FALSE(ec_c);
    REQUIRE(client.is_open());
}

TEST_CASE("named_pipe_stream is_open after open_server", "[named_pipe][is_open]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    REQUIRE(server.is_open());
}

TEST_CASE("named_pipe_stream is_open returns false after close", "[named_pipe][is_open]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    server.close();
    REQUIRE_FALSE(server.is_open());
}

TEST_CASE("named_pipe_stream close returns no error", "[named_pipe][close]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    auto ec = server.close();
    REQUIRE_FALSE(ec);
}

TEST_CASE("named_pipe_stream double close does not error", "[named_pipe][close]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    server.close();
    auto ec = server.close();
    REQUIRE(ec);
}

TEST_CASE("named_pipe_stream cancel on open stream does not error", "[named_pipe][cancel]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    auto ec = server.cancel();
    REQUIRE_FALSE(ec);
}

TEST_CASE("named_pipe_stream cancel on closed stream does not error", "[named_pipe][cancel]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    server.close();
    auto ec = server.cancel();
    REQUIRE(ec);
}

TEST_CASE("named_pipe_stream native_handle is valid after open", "[named_pipe][native_handle]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    REQUIRE(server.native_handle() != reinterpret_cast<native_handle_type>(-1));
}

TEST_CASE("named_pipe_stream native_handle returns sentinel after close", "[named_pipe][native_handle]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    server.close();
    REQUIRE(server.native_handle() == reinterpret_cast<native_handle_type>(-1));
}

TEST_CASE("named_pipe_stream get_executor is callable", "[named_pipe][executor]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    (void) server.get_executor();
    SUCCEED();
}

TEST_CASE("named_pipe_stream move construction transfers open state", "[named_pipe][move]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    auto server2 = std::move(server);
    REQUIRE(server2.is_open());
    REQUIRE_FALSE(server.is_open());
}

TEST_CASE("named_pipe_stream move assignment transfers open state", "[named_pipe][move]") {
    auto s1 = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    auto s2 = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name());
    s2 = std::move(s1);
    REQUIRE(s2.is_open());
    REQUIRE_FALSE(s1.is_open());
}

TEST_CASE("named_pipe_stream sync write_some then read_some round-trip", "[named_pipe][sync]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::size_t written = 0;
    REQUIRE_NOTHROW(written = client.write_some(wbuf));
    REQUIRE(written == PAYLOAD.size());

    std::string recv(PAYLOAD.size(), '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    std::size_t n = 0;
    REQUIRE_NOTHROW(n = server.read_some(rbuf));
    REQUIRE(n == PAYLOAD.size());
    REQUIRE(recv == PAYLOAD);
}

TEST_CASE("named_pipe_stream sync write_some ec overload clears error on success", "[named_pipe][sync]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::error_code ec;
    std::size_t n = client.write_some(wbuf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == PAYLOAD.size());
}

TEST_CASE("named_pipe_stream sync read_some ec overload clears error on success", "[named_pipe][sync]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    client.write_some(wbuf);

    std::string recv(PAYLOAD.size(), '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    std::error_code ec;
    std::size_t n = server.read_some(rbuf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == PAYLOAD.size());
    REQUIRE(recv == PAYLOAD);
}

TEST_CASE("named_pipe_stream bidirectional sync exchange", "[named_pipe][sync]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    const std::string_view req = "ping";
    const std::string_view resp = "pong";

    auto req_buf = buffer(req.data(), req.size());
    client.write_some(req_buf);

    std::string srv_recv(req.size(), '\0');
    server.read_some(buffer(srv_recv.data(), srv_recv.size()));
    REQUIRE(srv_recv == req);

    auto resp_buf = buffer(resp.data(), resp.size());
    server.write_some(resp_buf);

    std::string cli_recv(resp.size(), '\0');
    client.read_some(buffer(cli_recv.data(), cli_recv.size()));
    REQUIRE(cli_recv == resp);
}

TEST_CASE("named_pipe_stream large buffer round-trip", "[named_pipe][sync]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    const std::string large(64 * 1024, 'N');

    std::thread writer([&] {
        std::size_t sent = 0;
        while (sent < large.size()) {
            auto buf = buffer(large.data() + sent, large.size() - sent);
            std::error_code ec;
            auto n = client.write_some(buf, ec);
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
        auto n = server.read_some(buf, ec);
        if (ec || n == 0) {
            break;
        }
        got += n;
    }

    writer.join();
    REQUIRE(got == large.size());
    REQUIRE(recv == large);
}

TEST_CASE("named_pipe_stream async_write_some then async_read_some round-trip", "[named_pipe][async]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    std::atomic<bool> write_done{false};
    std::error_code write_ec{};
    std::size_t write_bytes{0};

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    client.async_write_some(wbuf, [&](std::error_code ec, std::size_t n) {
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
    server.async_read_some(rbuf, [&](std::error_code ec, std::size_t n) {
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

TEST_CASE("named_pipe_stream async_write_some error_code set on closed pipe", "[named_pipe][async]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::inout);

    server.close();

    std::atomic<bool> done{false};
    std::error_code result_ec{};

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    server.async_write_some(wbuf, [&](std::error_code ec, std::size_t) {
        result_ec = ec;
        done.store(true, std::memory_order_release);
    });
    wait_for(done);
    REQUIRE(done.load());
    REQUIRE(result_ec);
}

TEST_CASE("named_pipe_stream inbound server can only read from client", "[named_pipe][direction]") {
    auto server = named_pipe_stream::open_server(global_ctx().get_executor(), test_pipe_name(), pipe_direction::in);
    auto client = named_pipe_stream::open_client(global_ctx().get_executor(), test_pipe_name(), pipe_direction::out);

    auto wbuf = buffer(PAYLOAD.data(), PAYLOAD.size());
    std::error_code ec;
    client.write_some(wbuf, ec);
    REQUIRE_FALSE(ec);

    std::string recv(PAYLOAD.size(), '\0');
    auto rbuf = buffer(recv.data(), recv.size());
    std::size_t n = server.read_some(rbuf, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == PAYLOAD.size());
    REQUIRE(recv == PAYLOAD);
}
