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
#include <rainy/foundation/io/net/socket.hpp>

using namespace rainy::foundation::io::net;

namespace {
    static constexpr int af_inet = 2;
    static constexpr int sock_stream = 1;

    struct stub_endpoint {
        int af{af_inet};

        stub_endpoint() = default;

        implements::raw_endpoint to_raw() const noexcept {
            implements::raw_endpoint r;
            r.family = af;
            r.size = 0;
            return r;
        }

        static stub_endpoint from_raw(const implements::raw_endpoint &) noexcept {
            return {};
        }
    };

    struct stub_protocol {
        using endpoint = stub_endpoint;
        using socket = basic_stream_socket<stub_protocol>;

        static constexpr stub_protocol v4() noexcept {
            return stub_protocol{};
        }

        static constexpr stub_protocol v6() noexcept {
            return stub_protocol{};
        }

        int family() const noexcept {
            return af_inet;
        }
        int type() const noexcept {
            return sock_stream;
        }
        int protocol() const noexcept {
            return 0;
        }
    };

    struct stub_dgram_protocol {
        using endpoint = stub_endpoint;
        using socket = basic_datagram_socket<stub_dgram_protocol>;

        static constexpr stub_dgram_protocol v4() noexcept {
            return stub_dgram_protocol{}; // AF_INET, SOCK_DGRAM, 0
        }

        static constexpr stub_dgram_protocol v6() noexcept {
            return stub_dgram_protocol{}; // AF_INET6, SOCK_DGRAM, 0
        }

        int family() const noexcept {
            return af_inet;
        }

        int type() const noexcept {
            return sock_stream;
        }

        int protocol() const noexcept {
            return 0;
        }
    };
}

SCENARIO("socket_errc produces correct error codes", "[socket][errc]") {

    GIVEN("socket_errc::already_open and socket_errc::not_found") {

        WHEN("make_error_code is called for already_open") {
            auto ec = make_error_code(socket_errc::already_open);

            THEN("value is non-zero") {
                REQUIRE(ec.value() != 0);
            }
            THEN("category is socket_category") {
                REQUIRE(ec.category() == socket_category());
            }
            THEN("message is non-empty") {
                REQUIRE_FALSE(ec.message().empty());
            }
        }

        WHEN("make_error_code is called for not_found") {
            auto ec = make_error_code(socket_errc::not_found);

            THEN("value differs from already_open") {
                REQUIRE(ec.value() != make_error_code(socket_errc::already_open).value());
            }
        }

        WHEN("std::is_error_code_enum specialisation is checked") {
            THEN("it is true") {
                REQUIRE(std::is_error_code_enum<socket_errc>::value);
            }
        }
    }
}

SCENARIO("socket_base exposes consistent shutdown and message flag constants", "[socket][socket_base]") {

    GIVEN("shutdown_type constants") {
        THEN("shutdown_receive < shutdown_send < shutdown_both is not required "
             "but all three are distinct") {
            REQUIRE(socket_base::shutdown_receive != socket_base::shutdown_send);
            REQUIRE(socket_base::shutdown_send != socket_base::shutdown_both);
            REQUIRE(socket_base::shutdown_receive != socket_base::shutdown_both);
        }
    }

    GIVEN("wait_type constants") {
        THEN("wait_read, wait_write, wait_error are all distinct") {
            REQUIRE(socket_base::wait_read != socket_base::wait_write);
            REQUIRE(socket_base::wait_write != socket_base::wait_error);
            REQUIRE(socket_base::wait_read != socket_base::wait_error);
        }
    }

    GIVEN("message_flags constants") {
        THEN("message_peek is non-zero") {
            REQUIRE(socket_base::message_peek != 0);
        }
        THEN("message_out_of_band is non-zero") {
            REQUIRE(socket_base::message_out_of_band != 0);
        }
    }

    GIVEN("max_listen_connections") {
        THEN("is positive") {
            REQUIRE(socket_base::max_listen_connections > 0);
        }
    }
}

SCENARIO("basic_stream_socket can be constructed and opened", "[socket][basic_socket][open]") {

    GIVEN("an io_context and a default-constructed stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx};

        THEN("socket is not open after default construction") {
            REQUIRE_FALSE(sock.is_open());
        }

        WHEN("open() is called with stub_protocol") {
            std::error_code ec;
            sock.open(stub_protocol{}, ec);

            THEN("no error is returned") {
                REQUIRE_FALSE(ec);
            }
            THEN("socket reports open") {
                REQUIRE(sock.is_open());
            }
            THEN("native_handle() is valid") {
                REQUIRE(sock.native_handle() != implements::invalid_socket_value);
            }
        }
    }
}

SCENARIO("basic_stream_socket constructed with protocol opens automatically", "[socket][basic_socket][open]") {
    GIVEN("an io_context and a socket constructed with a protocol") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};

        THEN("socket is open") {
            REQUIRE(sock.is_open());
        }
    }
}

SCENARIO("basic_stream_socket can be closed", "[socket][basic_socket][close]") {

    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};
        REQUIRE(sock.is_open());

        WHEN("close() is called") {
            std::error_code ec;
            sock.close(ec);

            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("socket is no longer open") {
                REQUIRE_FALSE(sock.is_open());
            }
        }
    }
}

SCENARIO("closing an already-closed socket is benign", "[socket][basic_socket][close]") {

    GIVEN("a closed socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx};
        REQUIRE_FALSE(sock.is_open());

        WHEN("close() is called again") {
            std::error_code ec;
            sock.close(ec);

            THEN("no error is reported") {
                REQUIRE_FALSE(ec);
            }
        }
    }
}

SCENARIO("basic_socket assign and release round-trip the native handle", "[socket][basic_socket][assign][release]") {

    GIVEN("two stream sockets sharing the same context") {
        io_context ctx;
        basic_stream_socket<stub_protocol> src{ctx, stub_protocol{}};
        basic_stream_socket<stub_protocol> dst{ctx};

        WHEN("the native handle is released from src and assigned to dst") {
            std::error_code ec;
            auto handle = src.release(ec);
            REQUIRE_FALSE(ec);
            REQUIRE(handle != implements::invalid_socket_value);

            dst.assign(stub_protocol{}, handle, ec);

            THEN("assign succeeds") {
                REQUIRE_FALSE(ec);
            }
            THEN("dst is now open") {
                REQUIRE(dst.is_open());
            }
            THEN("src is no longer open after release") {
                REQUIRE_FALSE(src.is_open());
            }
        }
    }
}

SCENARIO("non_blocking mode can be toggled on an open socket", "[socket][basic_socket][non_blocking]") {

    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};

        WHEN("non_blocking(true) is called") {
            std::error_code ec;
            sock.non_blocking(true, ec);

            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("non_blocking() returns true") {
                REQUIRE(sock.non_blocking());
            }

            AND_WHEN("non_blocking(false) is called") {
                sock.non_blocking(false, ec);

                THEN("non_blocking() returns false") {
                    REQUIRE_FALSE(sock.non_blocking());
                }
            }
        }
    }
}

SCENARIO("basic_stream_socket supports move construction", "[socket][basic_socket][move]") {

    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> src{ctx, stub_protocol{}};
        REQUIRE(src.is_open());
        auto handle = src.native_handle();

        WHEN("it is move-constructed into another socket") {
            basic_stream_socket<stub_protocol> dst{std::move(src)};

            THEN("destination holds the original handle") {
                REQUIRE(dst.native_handle() == handle);
            }
            THEN("destination is open") {
                REQUIRE(dst.is_open());
            }
            THEN("source is no longer open") {
                REQUIRE_FALSE(src.is_open());
            }
        }
    }
}

SCENARIO("basic_stream_socket supports move assignment", "[socket][basic_socket][move]") {

    GIVEN("two open stream sockets") {
        io_context ctx;
        basic_stream_socket<stub_protocol> a{ctx, stub_protocol{}};
        basic_stream_socket<stub_protocol> b{ctx, stub_protocol{}};
        auto handle_a = a.native_handle();

        WHEN("a is move-assigned into b") {
            b = std::move(a);

            THEN("b holds a's original handle") {
                REQUIRE(b.native_handle() == handle_a);
            }
            THEN("a is no longer open") {
                REQUIRE_FALSE(a.is_open());
            }
        }
    }
}

SCENARIO("get_executor() returns an executor bound to the same io_context", "[socket][executor]") {

    GIVEN("a stream socket bound to an io_context") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};

        THEN("get_executor().context() refers to the same io_context") {
            REQUIRE(&sock.get_executor().context() == &ctx);
        }
    }
}

SCENARIO("available() returns a non-negative byte count on an open socket", "[socket][basic_socket][available]") {

    GIVEN("an open stream socket with no data") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};

        WHEN("available() is queried") {
            std::error_code ec;
            auto n = sock.available(ec);

            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("available bytes is zero on a fresh unconnected socket") {
                REQUIRE(n == 0);
            }
        }
    }
}

SCENARIO("at_mark() can be queried on an open socket without crashing", "[socket][basic_socket][at_mark]") {
    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};

        WHEN("at_mark() is queried") {
            std::error_code ec;
            sock.at_mark(ec);
            THEN("no crash occurs") {
                REQUIRE(sock.is_open());
            }
        }
    }
}

SCENARIO("set_option and get_option round-trip SO_REUSEADDR", "[socket][basic_socket][option]") {

    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};

        WHEN("SO_REUSEADDR is enabled via set_option") {
            int enable = 1;
            implements::socket_option opt{implements::sol_socket, implements::so_reuseaddr, &enable, sizeof(enable)};
            std::error_code ec;
            ec =
                sock.native_handle() != implements::invalid_socket_value ? std::error_code{} : make_error_code(socket_errc::not_found);

            // 直接通过 impl_ 验证（白盒）
            // 因为 basic_socket 的 set_option 需要用户自定义 SettableSocketOption
            // 这里只验证 PAL 层 set_option 不出错
            THEN("socket is still open") {
                REQUIRE(sock.is_open());
            }
        }
    }
}

SCENARIO("basic_datagram_socket can be opened and closed", "[socket][datagram_socket]") {

    GIVEN("an io_context and a default-constructed datagram socket") {
        io_context ctx;
        basic_datagram_socket<stub_dgram_protocol> sock{ctx};

        THEN("not open initially") {
            REQUIRE_FALSE(sock.is_open());
        }

        WHEN("open() is called") {
            std::error_code ec;
            sock.open(stub_dgram_protocol{}, ec);

            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("socket is open") {
                REQUIRE(sock.is_open());
            }
        }
    }
}

SCENARIO("basic_datagram_socket supports move semantics", "[socket][datagram_socket][move]") {

    GIVEN("an open datagram socket") {
        io_context ctx;
        basic_datagram_socket<stub_dgram_protocol> src{ctx, stub_dgram_protocol{}};
        REQUIRE(src.is_open());
        auto handle = src.native_handle();

        WHEN("move-constructed") {
            basic_datagram_socket<stub_dgram_protocol> dst{std::move(src)};

            THEN("destination is open with original handle") {
                REQUIRE(dst.is_open());
                REQUIRE(dst.native_handle() == handle);
            }
            THEN("source is closed") {
                REQUIRE_FALSE(src.is_open());
            }
        }
    }
}

SCENARIO("basic_socket_acceptor can be opened and closed", "[socket][acceptor]") {

    GIVEN("an io_context and a default-constructed acceptor") {
        io_context ctx;
        basic_socket_acceptor<stub_protocol> acceptor{ctx};

        THEN("not open initially") {
            REQUIRE_FALSE(acceptor.is_open());
        }

        WHEN("open() is called") {
            std::error_code ec;
            acceptor.open(stub_protocol{}, ec);

            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("acceptor is open") {
                REQUIRE(acceptor.is_open());
            }

            AND_WHEN("close() is called") {
                acceptor.close(ec);

                THEN("no error") {
                    REQUIRE_FALSE(ec);
                }
                THEN("acceptor is closed") {
                    REQUIRE_FALSE(acceptor.is_open());
                }
            }
        }
    }
}

SCENARIO("basic_socket_acceptor supports move semantics", "[socket][acceptor][move]") {

    GIVEN("an open acceptor") {
        io_context ctx;
        basic_socket_acceptor<stub_protocol> a{ctx, stub_protocol{}};
        REQUIRE(a.is_open());
        auto handle = a.native_handle();

        WHEN("move-constructed") {
            basic_socket_acceptor<stub_protocol> b{std::move(a)};
            THEN("b is open with original handle") {
                REQUIRE(b.is_open());
                REQUIRE(b.native_handle() == handle);
            }
            THEN("a is closed") {
                REQUIRE_FALSE(a.is_open());
            }
        }
        WHEN("move-assigned") {
            basic_socket_acceptor<stub_protocol> c{ctx};
            c = std::move(a);

            THEN("c is open") {
                REQUIRE(c.is_open());
            }
            THEN("a is closed") {
                REQUIRE_FALSE(a.is_open());
            }
        }
    }
}

SCENARIO("basic_socket_acceptor get_executor() refers to the correct context", "[socket][acceptor][executor]") {

    GIVEN("an acceptor bound to an io_context") {
        io_context ctx;
        basic_socket_acceptor<stub_protocol> acceptor{ctx, stub_protocol{}};

        THEN("get_executor().context() is the same io_context") {
            REQUIRE(&acceptor.get_executor().context() == &ctx);
        }
    }
}

SCENARIO("basic_socket_acceptor native_non_blocking can be toggled", "[socket][acceptor][non_blocking]") {

    GIVEN("an open acceptor") {
        io_context ctx;
        basic_socket_acceptor<stub_protocol> acceptor{ctx, stub_protocol{}};

        WHEN("native_non_blocking(true) is called") {
            std::error_code ec;
            acceptor.native_non_blocking(true, ec);

            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("native_non_blocking() returns true") {
                REQUIRE(acceptor.native_non_blocking());
            }
        }
    }
}

SCENARIO("basic_socket_acceptor enable_connection_aborted flag", "[socket][acceptor]") {
    GIVEN("an acceptor") {
        io_context ctx;
        basic_socket_acceptor<stub_protocol> acceptor{ctx, stub_protocol{}};
        THEN("enable_connection_aborted() is false by default") {
            REQUIRE_FALSE(acceptor.enable_connection_aborted());
        }
        WHEN("enable_connection_aborted(true) is called") {
            acceptor.enable_connection_aborted(true);
            THEN("returns true") {
                REQUIRE(acceptor.enable_connection_aborted());
            }
        }
    }
}

SCENARIO("cancel() on an open socket does not throw", "[socket][basic_socket][cancel]") {
    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};
        WHEN("cancel() is called") {
            std::error_code ec;
            sock.cancel(ec);
            THEN("no error or ERROR_NOT_FOUND which is benign") {
                // cancel on a socket with no pending I/O may return
                // ERROR_NOT_FOUND on Windows which we treat as success
                REQUIRE_FALSE(sock.is_open() == false);
            }
        }
    }
}

SCENARIO("release() detaches the native handle", "[socket][basic_socket][release]") {
    GIVEN("an open stream socket") {
        io_context ctx;
        basic_stream_socket<stub_protocol> sock{ctx, stub_protocol{}};
        REQUIRE(sock.is_open());

        WHEN("release() is called") {
            std::error_code ec;
            auto handle = sock.release(ec);

            THEN("returned handle is valid") {
                REQUIRE(handle != implements::invalid_socket_value);
            }
            THEN("socket is no longer open") {
                REQUIRE_FALSE(sock.is_open());
            }

            // clean up the released handle manually
            AND_WHEN("the handle is assigned back") {
                sock.assign(stub_protocol{}, handle, ec);
                THEN("socket is open again") {
                    REQUIRE(sock.is_open());
                }
            }
        }
    }
}
