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
#ifndef RAINY_FOUNDATION_IO_NET_IO_INTERNET_HPP
#define RAINY_FOUNDATION_IO_NET_IO_INTERNET_HPP

#include <rainy/foundation/io/net/buffer.hpp>
#include <rainy/foundation/io/net/internet/endpoint.hpp>
#include <rainy/foundation/io/net/io_context.hpp>
#include <rainy/foundation/io/net/socket.hpp>

namespace rainy::foundation::io::net::ip {
    class tcp {
    public:
        using endpoint = basic_endpoint<tcp>;
        using resolver = basic_resolver<tcp>;
        using socket = basic_stream_socket<tcp>;
        using acceptor = basic_socket_acceptor<tcp>;

        class no_delay : public socket_base::socket_option_base<6, 1, int> {
        public:
            // level=6 (IPPROTO_TCP), name=1 (TCP_NODELAY)

            explicit no_delay(const bool enable = false) : socket_option_base(enable ? 1 : 0) {
            }

            RAINY_NODISCARD bool value() const noexcept { // NOLINT
                return socket_option_base::value() != 0;
            }
        };

        static constexpr tcp v4() noexcept {
            return tcp{2, 1, 6}; // AF_INET, SOCK_STREAM, IPPROTO_TCP
        }

        static constexpr tcp v6() noexcept {
            return tcp{10, 1, 6}; // AF_INET6, SOCK_STREAM, IPPROTO_TCP
        }

        RAINY_NODISCARD constexpr int family() const noexcept {
            return family_;
        }

        RAINY_NODISCARD constexpr int type() const noexcept {
            return type_;
        }

        RAINY_NODISCARD constexpr int protocol() const noexcept {
            return protocol_;
        }

        tcp() = delete;

    private:
        constexpr tcp(const int family, const int type, const int protocol) noexcept :
            family_{family}, type_{type}, protocol_{protocol} {
        }

        int family_;
        int type_;
        int protocol_;
    };

    constexpr bool operator==(const tcp &left, const tcp &right) noexcept { // NOLINT
        return left.family() == right.family() && left.type() == right.type() && left.protocol() == right.protocol();
    }

    constexpr bool operator!=(const tcp &left, const tcp &right) noexcept {
        return !(left == right);
    }

    class udp {
    public:
        using endpoint = basic_endpoint<udp>;
        using resolver = basic_resolver<udp>;
        using socket = basic_datagram_socket<udp>;

        static constexpr udp v4() noexcept {
            return udp{2, 2, 17}; // AF_INET, SOCK_DGRAM, IPPROTO_UDP
        }

        static constexpr udp v6() noexcept {
            return udp{10, 2, 17}; // AF_INET6, SOCK_DGRAM, IPPROTO_UDP
        }

        RAINY_NODISCARD constexpr int family() const noexcept {
            return family_;
        }

        RAINY_NODISCARD constexpr int type() const noexcept {
            return type_;
        }

        RAINY_NODISCARD constexpr int protocol() const noexcept {
            return protocol_;
        }

        udp() = delete;

    private:
        constexpr udp(const int family, const int type, const int protocol) noexcept :
            family_{family}, type_{type}, protocol_{protocol} {
        }

        int family_;
        int type_;
        int protocol_;
    };

    constexpr bool operator==(const udp &left, const udp &right) noexcept { // NOLINT
        return left.family() == right.family() && left.type() == right.type() && left.protocol() == right.protocol();
    }

    constexpr bool operator!=(const udp &left, const udp &right) noexcept {
        return !(left == right);
    }

}

#endif
