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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cstring>
#include <rainy/foundation/io/net/implements/addr.hpp>
#include <system_error>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace rainy::foundation::io::net::ip::implements {
    struct winsock_initializer {
        WSADATA data{};
        int result{};

        winsock_initializer() noexcept {
            result = ::WSAStartup(MAKEWORD(2, 2), &data);
        }
        ~winsock_initializer() noexcept {
            if (result == 0) {
                ::WSACleanup();
            }
        }
    };

    const winsock_initializer &ensure_winsock() noexcept {
        static winsock_initializer instance;
        return instance;
    }

    const resolver_errc_values &get_resolver_errc_values() noexcept {
        static constexpr resolver_errc_values values{
            WSAHOST_NOT_FOUND, // host_not_found
            WSATRY_AGAIN, // try_again
            WSATYPE_NOT_FOUND // service_not_found
        };
        return values;
    }

    const char *resolver_errc_message(int ev) noexcept {
        return ::gai_strerrorA(ev);
    }

    bool ipv4_from_string(core::czstring str, ipv4_bytes &out, std::error_code &ec) noexcept {
        ensure_winsock();
        IN_ADDR native{};
        if (::inet_pton(AF_INET, str, &native) != 1) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }
        std::memcpy(out.data, &native.S_un.S_un_b, 4);
        ec.clear();
        return true;
    }

    bool ipv4_to_string(const ipv4_bytes &in, core::cstring buf, std::size_t buf_size) noexcept {
        ensure_winsock();
        IN_ADDR native{};
        std::memcpy(&native.S_un.S_un_b, in.data, 4);
        return ::inet_ntop(AF_INET, &native, buf, static_cast<socklen_t>(buf_size)) != nullptr;
    }

    bool ipv6_from_string(core::czstring str, ipv6_bytes &out, std::error_code &ec) noexcept {
        ensure_winsock();
        IN6_ADDR native{};
        if (::inet_pton(AF_INET6, str, &native) != 1) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }
        std::memcpy(out.data, native.u.Byte, 16);
        ec.clear();
        return true;
    }

    bool ipv6_to_string(const ipv6_bytes &in, core::cstring buf, std::size_t buf_size) noexcept {
        ensure_winsock();
        IN6_ADDR native{};
        std::memcpy(native.u.Byte, in.data, 16);
        return ::inet_ntop(AF_INET6, &native, buf, static_cast<socklen_t>(buf_size)) != nullptr;
    }

    std::error_code resolve(const char *host, const char *service, int family, int socktype, int protocol, int flags,
                                              collections::vector<resolved_entry> &out) noexcept {
        ensure_winsock();

        ADDRINFOA hints{};
        hints.ai_family = family;
        hints.ai_socktype = socktype;
        hints.ai_protocol = protocol;
        hints.ai_flags = flags;

        PADDRINFOA result = nullptr;
        int ret = ::getaddrinfo(host, service, &hints, &result);

        if (ret != 0) {
            return std::error_code(ret, std::system_category());
        }

        out.clear();
        for (PADDRINFOA ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            resolved_entry entry{};
            entry.family = ptr->ai_family;
            entry.socktype = ptr->ai_socktype;
            entry.protocol = ptr->ai_protocol;
            entry.addr_size = ptr->ai_addrlen;

            if (ptr->ai_addrlen > 0 && ptr->ai_addrlen <= sizeof(entry.addr_data)) {
                std::memcpy(entry.addr_data, ptr->ai_addr, ptr->ai_addrlen);
            }

            if (ptr->ai_canonname != nullptr) {
                std::strncpy(entry.canonical_name, ptr->ai_canonname, sizeof(entry.canonical_name) - 1);
                entry.canonical_name[sizeof(entry.canonical_name) - 1] = '\0';
            }

            out.push_back(std::move(entry));
        }

        ::freeaddrinfo(result);
        return std::error_code();
    }

    std::error_code reverse_resolve(const void *addr_data, int addr_len, char *host_buf, std::size_t host_buf_size,
                                                      char *svc_buf, std::size_t svc_buf_size) noexcept {
        ensure_winsock();

        if (addr_data == nullptr || addr_len <= 0 || addr_len > static_cast<int>(sizeof(sockaddr_storage))) {
            return std::error_code(static_cast<int>(std::errc::invalid_argument), std::system_category());
        }
        const auto *addr = static_cast<const sockaddr *>(addr_data);
        int ret = ::getnameinfo(addr, static_cast<socklen_t>(addr_len), host_buf, static_cast<socklen_t>(host_buf_size), svc_buf,
                                static_cast<socklen_t>(svc_buf_size), NI_NAMEREQD);

        if (ret != 0) {
            return std::error_code(ret, std::system_category());
        }

        return std::error_code();
    }
}
