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
#ifndef RAINY_FOUNDATION_IO_NET_SSL_PROTOCOL_HPP
#define RAINY_FOUNDATION_IO_NET_SSL_PROTOCOL_HPP

#include <rainy/foundation/io/net/internet.hpp>
#include <rainy/foundation/io/net/ssl/stream.hpp>

namespace rainy::foundation::io::net::ssl {

    class tls {
    public:
        using endpoint = ip::tcp::endpoint;
        using socket = stream<ip::tcp::socket>;
        using acceptor = ip::tcp::acceptor;

        static constexpr tls v4() noexcept {
            return tls{2, 1, 6}; // AF_INET, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr tls v6() noexcept {
            return tls{10, 1, 6}; // AF_INET6, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr int family() noexcept {
            return tls::v4().family_;
        }

        static constexpr int type() noexcept {
            return tls::v4().type_;
        }

        static constexpr int protocol() noexcept {
            return tls::v4().protocol_;
        }

        RAINY_NODISCARD constexpr int get_family() const noexcept {
            return family_;
        }

        RAINY_NODISCARD constexpr int get_type() const noexcept {
            return type_;
        }

        RAINY_NODISCARD constexpr int get_protocol() const noexcept {
            return protocol_;
        }

        static socket create_socket(io_context &ctx, context &ssl_ctx) {
            return socket{ip::tcp::socket{ctx}, ssl_ctx};
        }

        tls() = delete;

    protected:
        constexpr tls(const int family, const int type, const int protocol) noexcept :
            family_{family}, type_{type}, protocol_{protocol} {
        }

        int family_;
        int type_;
        int protocol_;
    };

    constexpr bool operator==(const tls &left, const tls &right) noexcept { // NOLINT
        return left.get_family() == right.get_family() && left.get_type() == right.get_type() &&
               left.get_protocol() == right.get_protocol();
    }

    constexpr bool operator!=(const tls &left, const tls &right) noexcept {
        return !(left == right);
    }

    class tlsv12 : public tls {
    public:
        static constexpr tlsv12 v4() noexcept {
            return tlsv12{2, 1, 6}; // AF_INET, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr tlsv12 v6() noexcept {
            return tlsv12{10, 1, 6}; // AF_INET6, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr int family() noexcept {
            return v4().get_family();
        }

        static constexpr int type() noexcept {
            return v4().get_type();
        }

        static constexpr int protocol() noexcept {
            return v4().get_protocol();
        }

        tlsv12() = delete;

    private:
        constexpr tlsv12(const int family, const int type, const int protocol) noexcept : tls(family, type, protocol) {
        }
    };

    class tlsv13 : public tls {
    public:
        static constexpr tlsv13 v4() noexcept {
            return tlsv13{2, 1, 6}; // AF_INET, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr tlsv13 v6() noexcept {
            return tlsv13{10, 1, 6}; // AF_INET6, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr int family() noexcept {
            return tlsv13::v4().get_family();
        }

        static constexpr int type() noexcept {
            return tlsv13::v4().get_type();
        }

        static constexpr int protocol() noexcept {
            return tlsv13::v4().get_protocol();
        }

        tlsv13() = delete;

    private:
        constexpr tlsv13(const int family, const int type, const int protocol) noexcept : tls(family, type, protocol) {
        }
    };

    constexpr bool operator==(const tlsv12 &left, const tlsv12 &right) noexcept { // NOLINT
        return left.get_family() == right.get_family() && left.get_type() == right.get_type() &&
               left.get_protocol() == right.get_protocol();
    }

    constexpr bool operator!=(const tlsv12 &left, const tlsv12 &right) noexcept {
        return !(left == right);
    }

    constexpr bool operator==(const tlsv13 &left, const tlsv13 &right) noexcept {
        return left.get_family() == right.get_family() && left.get_type() == right.get_type() &&
               left.get_protocol() == right.get_protocol();
    }

    constexpr bool operator!=(const tlsv13 &left, const tlsv13 &right) noexcept {
        return !(left == right);
    }

}

#endif
