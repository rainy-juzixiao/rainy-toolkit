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
#if RAINY_HAS_OPENSSL

#include <rainy/core/core.hpp>

#if RAINY_USING_MSVC || RAINY_USING_MSVC_CLANG
#define _CRT_SECURE_NO_WARNINGS // NOLINT
#endif

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/io_context.hpp>
#include <rainy/foundation/io/net/internet.hpp>
#include <rainy/foundation/io/net/ssl/context.hpp>
#include <rainy/foundation/io/net/ssl/stream.hpp>

using namespace rainy;

namespace rio = foundation::io;
namespace rip = foundation::io::net::ip;
namespace rssl = foundation::io::net::ssl;

using tcp_t = rip::tcp;
using socket_t = tcp_t::socket;
using acceptor_t = tcp_t::acceptor;
using endpoint_t = tcp_t::endpoint;

static bool generate_test_files(const char *cert_path, const char *key_path) { // NOLINT
    // 生成 RSA 2048 私钥
    EVP_PKEY *pkey = EVP_RSA_gen(2048);
    if (!pkey) {
        return false;
    }

    // 创建 X509 证书
    X509 *x509 = X509_new();
    if (!x509) {
        EVP_PKEY_free(pkey);
        return false;
    }

    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_getm_notBefore(x509), 0);
    X509_gmtime_adj(X509_getm_notAfter(x509), 60L * 60 * 24 * 3650);
    X509_set_pubkey(x509, pkey);

    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char *>("localhost"), -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());

    // 写证书 PEM - 使用标准 fopen
    FILE *fcert = fopen(cert_path, "w");
    if (!fcert) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return false;
    }
    PEM_write_X509(fcert, x509);
    utility::ignore = fclose(fcert);

    // 写私钥 PEM - 使用标准 fopen
    FILE *fkey = fopen(key_path, "w");
    if (!fkey) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return false;
    }
    PEM_write_PrivateKey(fkey, pkey, nullptr, nullptr, 0, nullptr, nullptr);
    utility::ignore = fclose(fkey);

    X509_free(x509);
    EVP_PKEY_free(pkey);
    return true;
}

static auto TEST_HOST = "127.0.0.1";
static auto CERT_FILE = "test_cert.pem";
static auto KEY_FILE = "test_key.pem";

static const bool s_certs_ready = generate_test_files(CERT_FILE, KEY_FILE); // NOLINT

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

static std::error_code setup_server_context(rssl::context &ctx) {
    utility::ignore = s_certs_ready;
    rssl::certificate_options cert_opts;
    cert_opts.add_certificate_file(CERT_FILE).add_private_key_file(KEY_FILE);

    if (const auto ec = ctx.load_certificate(cert_opts)) {
        return ec;
    }
    if (const auto ec = ctx.use_private_key(cert_opts)) {
        return ec;
    }

    ctx.set_cipher_list("ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:AES128-GCM-SHA256:AES256-GCM-SHA384");
    ctx.set_verify_mode(rssl::verify_mode::none);
    return {};
}

static void setup_client_context(rssl::context &ctx) {
    utility::ignore = s_certs_ready;
    // 使用与服务器完全相同的 cipher 列表
    ctx.set_cipher_list("ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:AES128-GCM-SHA256:AES256-GCM-SHA384");
    ctx.set_verify_mode(rssl::verify_mode::none);
}

static std::pair<std::future<std::error_code>, std::thread> start_echo_server_thread(std::promise<void> &ready,
                                                                                     std::promise<int> &port_promise) {
    auto promise = std::make_shared<std::promise<std::error_code>>();
    auto fut = promise->get_future();
    std::thread t([&ready, &port_promise, promise = utility::move(promise)]() mutable {
        std::error_code ec;
        rio::io_context ioc;
        rssl::context srv_ctx(rssl::method::tlsv12_server);

        if (auto e = setup_server_context(srv_ctx); e) {
            ready.set_value();
            promise->set_value(e);
            return;
        }

        using endpoint_t = typename acceptor_t::endpoint_type;
        constexpr endpoint_t ep(tcp_t::v4(), 0);
        acceptor_t acceptor(ioc, ep, true);
        port_promise.set_value(acceptor.local_endpoint().port());
        ready.set_value();
        socket_t tcp_sock = acceptor.accept(ec);
        if (ec) {
            promise->set_value(ec);
            return;
        }

        rssl::stream<socket_t> stream(utility::move(tcp_sock), srv_ctx);
        stream.handshake(ec);
        if (ec) {
            promise->set_value(ec);
            return;
        }

        rainy::collections::array<char, 256> buf{};
        std::size_t n = stream.read_some(rio::buffer(buf), ec);
        if (ec) {
            promise->set_value(ec);
            return;
        }

        stream.write_some(rio::buffer(buf.data(), n), ec);
        if (ec) {
            promise->set_value(ec);
            return;
        }

        stream.shutdown(ec);
        promise->set_value(ec);
    });
    return {utility::move(fut), utility::move(t)};
}

static std::pair<std::future<std::error_code>, std::thread> start_echo_server_buffered(
    std::promise<void> &ready, std::promise<int> &port_promise, std::size_t expect_bytes) {

    auto promise = std::make_shared<std::promise<std::error_code>>();
    auto fut = promise->get_future();

    std::thread t([&ready, &port_promise, expect_bytes, promise = utility::move(promise)]() mutable {
        std::error_code ec;
        rio::io_context ioc;
        rssl::context srv_ctx(rssl::method::tlsv12_server);

        if (auto e = setup_server_context(srv_ctx); e) {
            port_promise.set_value(0);
            ready.set_value();
            promise->set_value(e);
            return;
        }

        using endpoint_t = typename acceptor_t::endpoint_type;
        constexpr endpoint_t ep(tcp_t::v4(), 0);
        acceptor_t acceptor(ioc, ep, true);

        port_promise.set_value(acceptor.local_endpoint().port());
        ready.set_value();

        socket_t tcp_sock = acceptor.accept(ec);
        if (ec) { promise->set_value(ec); return; }

        rssl::stream<socket_t> stream(utility::move(tcp_sock), srv_ctx);
        stream.handshake(ec);
        if (ec) { promise->set_value(ec); return; }

        rainy::collections::array<char, 256> buf{};
        std::size_t total = 0;
        while (total < expect_bytes) {
            std::size_t n = stream.read_some(rio::buffer(buf.data() + total, buf.size() - total), ec);
            if (ec) { promise->set_value(ec); return; }
            total += n;
        }

        stream.write_some(rio::buffer(buf.data(), total), ec);
        if (ec) { promise->set_value(ec); return; }

        collections::array<char, 1> drain{};
        stream.read_some(rio::buffer(drain), ec);
        ec.clear();

        stream.shutdown(ec);
        ec.clear();
        promise->set_value({});
    });

    return {utility::move(fut), utility::move(t)};
}

TEST_CASE("context: tlsv12_server marks context as server", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_server);
    CHECK(ctx.is_server());
    CHECK(ctx.native_handle() != nullptr);
}

TEST_CASE("context: tlsv12_client marks context as client", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_client);
    CHECK_FALSE(ctx.is_server());
    CHECK(ctx.native_handle() != nullptr);
}

TEST_CASE("context: set_verify_mode persists", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_client);
    ctx.set_verify_mode(rssl::verify_mode::peer);
    CHECK(ctx.verify_mode() == rssl::verify_mode::peer);
}

TEST_CASE("context: set_verify_mode combined flags", "[ssl_context]") {
    using vm = rssl::verify_mode;
    rssl::context ctx(rssl::method::tlsv12_server);
    ctx.set_verify_mode(vm::peer | vm::fail_if_no_peer_cert);
    CHECK(ctx.verify_mode() == (vm::peer | vm::fail_if_no_peer_cert));
}

TEST_CASE("context: load_certificate succeeds with valid files", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_server);
    rssl::certificate_options opts;
    opts.add_certificate_file(CERT_FILE).add_private_key_file(KEY_FILE);

    std::error_code ec = ctx.load_certificate(opts);
    REQUIRE_FALSE(ec);
    ec = ctx.use_private_key(opts);
    REQUIRE_FALSE(ec);
}

TEST_CASE("context: load_certificate fails with missing file", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_server);
    rssl::certificate_options opts;
    opts.add_certificate_file("nonexistent_cert.pem").add_private_key_file("nonexistent_key.pem");

    std::error_code ec = ctx.load_certificate(opts);
    CHECK(ec);
}

TEST_CASE("context: set_cipher_list does not throw for valid string", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_client);
    REQUIRE_NOTHROW(ctx.set_cipher_list("HIGH:!aNULL:!eNULL"));
}

TEST_CASE("context: set_alpn_protos accepts protocol list", "[ssl_context]") {
    rssl::context ctx(rssl::method::tlsv12_client);
    REQUIRE_NOTHROW(ctx.set_alpn_protos({"http/1.1", "h2"}));
}

TEST_CASE("ssl_stream: is_open is false on unconnected socket", "[ssl_stream_basic]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    CHECK_FALSE(stream.is_open());
}

TEST_CASE("ssl_stream: is_handshaked is false before handshake", "[ssl_stream_basic]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    CHECK_FALSE(stream.is_handshaked());
}

TEST_CASE("ssl_stream: set_server_name / server_name round-trip", "[ssl_stream_basic]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    stream.set_server_name("example.com");
    CHECK(stream.server_name() == "example.com");
}

TEST_CASE("ssl_stream: get_executor returns without throwing", "[ssl_stream_basic]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    auto ex = stream.get_executor();
    (void) ex;
    SUCCEED("get_executor() returned without throwing");
}

TEST_CASE("ssl_stream: move-construction preserves server_name", "[ssl_stream_basic]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> s1(sock, ctx);
    s1.set_server_name("moved.host");

    rssl::stream<socket_t> s2(utility::move(s1));
    CHECK(s2.server_name() == "moved.host");
}

TEST_CASE("ssl_stream: handshake on unconnected socket gives not_connected", "[ssl_error_paths]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    std::error_code ec;
    stream.handshake(ec);
    CHECK(ec == std::make_error_code(std::errc::not_connected));
}

TEST_CASE("ssl_stream: write_some before handshake gives not_connected", "[ssl_error_paths]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    const rainy::foundation::text::string data = "hello";
    std::error_code ec;
    std::size_t n = stream.write_some(rio::buffer(data), ec);
    CHECK(ec == std::make_error_code(std::errc::not_connected));
    CHECK(n == 0);
}

TEST_CASE("ssl_stream: read_some before handshake gives not_connected", "[ssl_error_paths]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    rainy::collections::array<char, 64> buf{};
    std::error_code ec;
    std::size_t n = stream.read_some(rio::buffer(buf), ec);
    CHECK(ec == std::make_error_code(std::errc::not_connected));
    CHECK(n == 0);
}

TEST_CASE("ssl_stream: shutdown before handshake gives not_connected", "[ssl_error_paths]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    std::error_code ec;
    stream.shutdown(ec);
    CHECK(ec == std::make_error_code(std::errc::not_connected));
}

TEST_CASE("ssl_stream: throwing handshake wraps std::system_error", "[ssl_error_paths]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    CHECK_THROWS_AS(stream.handshake(), std::system_error);
}

TEST_CASE("ssl_stream: synchronous TLS 1.2 handshake client <-> server", "[ssl_handshake]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);
    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    ;
    REQUIRE_FALSE(ec);
    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.set_server_name(TEST_HOST);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);
    CHECK(stream.is_handshaked());
    const rainy::foundation::text::string msg = "ping";
    stream.write_some(rio::buffer(msg), ec);
    REQUIRE_FALSE(ec);

    rainy::collections::array<char, 64> buf{};
    std::size_t n = stream.read_some(rio::buffer(buf), ec);
    REQUIRE_FALSE(ec);
    CHECK(std::string(buf.data(), n) == msg);

    stream.shutdown(ec);

    auto srv_ec = server_fut.get();
    CHECK_FALSE(srv_ec);
    server_thread.join();
}

TEST_CASE("ssl_stream: repeated handshake does not crash", "[ssl_handshake]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);

    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    REQUIRE_FALSE(ec);

    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);
    std::error_code ec2;
    stream.handshake(ec2);
    SUCCEED("second handshake did not crash");

    // Drain server so its thread exits cleanly.
    const rainy::foundation::text::string drain = "x";
    stream.write_some(rio::buffer(drain), ec);
    rainy::collections::array<char, 8> tmp{};
    stream.read_some(rio::buffer(tmp), ec);
    stream.shutdown(ec);

    server_fut.get();
    server_thread.join();
}

TEST_CASE("ssl_stream: async_handshake completes without error", "[ssl_async_handshake]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);
    cli_ctx.set_verify_mode(rssl::verify_mode::none);
    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    REQUIRE_FALSE(ec);
    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.set_server_name(TEST_HOST);
    std::error_code async_ec;
    bool called = false;

    stream.async_handshake([&](std::error_code e) {
        async_ec = e;
        called = true;
    });

    ioc.run();

    CHECK(called);
    CHECK_FALSE(async_ec);
    CHECK(stream.is_handshaked());

    if (!async_ec) {
        const rainy::foundation::text::string msg = "async_ping";
        stream.write_some(rio::buffer(msg), ec);
        rainy::collections::array<char, 64> buf{};
        stream.read_some(rio::buffer(buf), ec);
        stream.shutdown(ec);
    }

    auto srv_ec = server_fut.get();
    server_thread.join();
    CHECK_FALSE(srv_ec);
}

TEST_CASE("ssl_stream: write_some + read_some echo round-trip", "[ssl_io]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);

    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    ;
    REQUIRE_FALSE(ec);

    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);

    const rainy::foundation::text::string payload = "Hello, TLS 1.2!";
    std::size_t written = stream.write_some(rio::buffer(payload), ec);
    REQUIRE_FALSE(ec);
    CHECK(written == payload.size());

    rainy::collections::array<char, 128> buf{};
    std::size_t read = stream.read_some(rio::buffer(buf), ec);
    REQUIRE_FALSE(ec);
    CHECK(std::string(buf.data(), read) == payload);

    stream.shutdown(ec);
    server_fut.get();
    server_thread.join();
}

TEST_CASE("ssl_stream: two consecutive write_some calls are received intact", "[ssl_io]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_buffered(ready, port_promise, 6);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);

    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    REQUIRE_FALSE(ec);

    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);

    stream.write_some(rio::buffer(foundation::text::string("abc")), ec);
    REQUIRE_FALSE(ec);
    stream.write_some(rio::buffer(foundation::text::string("def")), ec);
    REQUIRE_FALSE(ec);

    collections::array<char, 64> buf{};
    std::size_t n = stream.read_some(rio::buffer(buf), ec);
    REQUIRE_FALSE(ec);
    CHECK(std::string(buf.data(), n) == "abcdef");

    stream.shutdown(ec);
    ec.clear();

    auto srv_ec = server_fut.get();
    server_thread.join();
    CHECK_FALSE(srv_ec);
}

TEST_CASE("ssl_stream: async_write_some + async_read_some echo round-trip", "[ssl_async_io]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);

    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    REQUIRE_FALSE(ec);

    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);

    const rainy::foundation::text::string payload = "async_io_test";
    rainy::collections::array<char, 128> recv_buf{};

    std::error_code write_ec, read_ec;
    std::size_t bytes_written = 0, bytes_read = 0;
    int pending = 2;

    stream.async_write_some(rio::buffer(payload), [&](std::error_code e, std::size_t n) {
        write_ec = e;
        bytes_written = n;
        --pending;
    });

    stream.async_read_some(rio::buffer(recv_buf), [&](std::error_code e, std::size_t n) {
        read_ec = e;
        bytes_read = n;
        --pending;
    });

    while (pending > 0) {
        ioc.run_one();
    }

    CHECK_FALSE(write_ec);
    CHECK(bytes_written == payload.size());
    CHECK_FALSE(read_ec);
    CHECK(std::string(recv_buf.data(), bytes_read) == payload);

    stream.shutdown(ec);
    server_fut.get();
    server_thread.join();
}

TEST_CASE("ssl_stream: async_write_some before handshake fires handler with not_connected", "[ssl_async_io]") {
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(sock, cli_ctx);

    std::error_code captured;
    bool called = false;

    stream.async_write_some(rio::buffer(rainy::foundation::text::string("will_fail")), [&](std::error_code e, std::size_t) {
        captured = e;
        called = true;
    });

    ioc.run();

    CHECK(called);
    CHECK(captured == std::make_error_code(std::errc::not_connected));
}

TEST_CASE("ssl_stream: async_read_some before handshake fires handler with not_connected", "[ssl_async_io]") {
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(sock, cli_ctx);

    rainy::collections::array<char, 64> buf{};
    std::error_code captured;
    bool called = false;

    stream.async_read_some(rio::buffer(buf), [&](std::error_code e, std::size_t) {
        captured = e;
        called = true;
    });

    ioc.run();

    CHECK(called);
    CHECK(captured == std::make_error_code(std::errc::not_connected));
}

TEST_CASE("ssl_stream: synchronous shutdown after handshake succeeds", "[ssl_shutdown]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);

    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    REQUIRE_FALSE(ec);

    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);

    // Feed the server so it exits its read loop before we shut down.
    const rainy::foundation::text::string msg = "bye";
    stream.write_some(rio::buffer(msg), ec);
    rainy::collections::array<char, 8> tmp{};
    stream.read_some(rio::buffer(tmp), ec);

    stream.shutdown(ec);
    CHECK_FALSE(ec);

    auto srv_ec = server_fut.get();
    CHECK_FALSE(srv_ec);
    server_thread.join();
}

TEST_CASE("ssl_stream: async_shutdown invokes handler without error", "[ssl_shutdown]") {
    std::promise<void> ready;
    std::promise<int> port_promise;
    auto ready_fut = ready.get_future();
    auto port_fut = port_promise.get_future();

    auto [server_fut, server_thread] = start_echo_server_thread(ready, port_promise);
    ready_fut.wait();
    int test_port = port_fut.get();
    std::error_code ec;
    rio::io_context ioc;
    rssl::context cli_ctx(rssl::method::tlsv12_client);
    setup_client_context(cli_ctx);

    socket_t tcp_sock(ioc);
    tcp_sock.connect(endpoint_t(rip::address_v4::loopback(), test_port), ec);
    ;
    REQUIRE_FALSE(ec);

    rssl::stream<socket_t> stream(utility::move(tcp_sock), cli_ctx);
    stream.handshake(ec);
    REQUIRE_FALSE(ec);

    // Feed the server first.
    const rainy::foundation::text::string msg = "async_bye";
    stream.write_some(rio::buffer(msg), ec);
    rainy::collections::array<char, 16> tmp{};
    stream.read_some(rio::buffer(tmp), ec);

    std::error_code shutdown_ec;
    bool called = false;

    stream.async_shutdown([&](std::error_code e) {
        shutdown_ec = e;
        called = true;
    });

    ioc.run();

    CHECK(called);
    CHECK_FALSE(shutdown_ec);

    auto srv_ec = server_fut.get();
    CHECK_FALSE(srv_ec);
    server_thread.join();
}

TEST_CASE("ssl_stream: throwing shutdown before handshake wraps std::system_error", "[ssl_shutdown]") {
    rio::io_context ioc;
    rssl::context ctx(rssl::method::tlsv12_client);
    socket_t sock(ioc);

    rssl::stream<socket_t> stream(utility::move(sock), ctx);
    CHECK_THROWS_AS(stream.shutdown(), std::system_error);
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)

#endif
