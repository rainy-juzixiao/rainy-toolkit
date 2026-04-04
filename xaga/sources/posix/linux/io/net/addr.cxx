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
// NOLINTBEGIN

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <rainy/foundation/io/net/implements/addr.hpp>
#include <sys/socket.h>
#include <system_error>

// NOLINTEND

namespace rainy::foundation::io::net::ip::implements {
    const resolver_errc_values &get_resolver_errc_values() noexcept {
        // NOLINTBEGIN
        static constexpr resolver_errc_values values{
            EAI_NONAME, // host_not_found
            EAI_AGAIN, // try_again
            EAI_SERVICE // service_not_found
        };
        // NOLINTEND
        return values;
    }

    const char *resolver_errc_message(const int ev) noexcept {
        return ::gai_strerror(ev);
    }

    bool ipv4_from_string(core::czstring str, ipv4_bytes &out, std::error_code &ec) noexcept { // NOLINT
        in_addr native{};
        if (::inet_pton(AF_INET, str, &native) != 1) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }
        std::memcpy(out.data, &native.s_addr, 4);
        ec.clear();
        return true;
    }

    bool ipv4_to_string(const ipv4_bytes &in, core::cstring buf, const std::size_t buf_size) noexcept { // NOLINT
        in_addr native{};
        std::memcpy(&native.s_addr, in.data, 4);
        return ::inet_ntop(AF_INET, &native, buf, static_cast<socklen_t>(buf_size)) != nullptr;
    }

    bool ipv6_from_string(core::czstring str, ipv6_bytes &out, std::error_code &ec) noexcept { // NOLINT
        in6_addr native{};
        if (::inet_pton(AF_INET6, str, &native) != 1) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }
        std::memcpy(out.data, native.s6_addr, 16);
        ec.clear();
        return true;
    }

    bool ipv6_to_string(const ipv6_bytes &in, core::cstring buf, const std::size_t buf_size) noexcept { // NOLINT
        in6_addr native{};
        std::memcpy(native.s6_addr, in.data, 16);
        return ::inet_ntop(AF_INET6, &native, buf, static_cast<socklen_t>(buf_size)) != nullptr;
    }
    
    std::error_code resolve(const char *host, const char *service, int family, int socktype, int protocol, int flags,
                            collections::vector<resolved_entry> &out) noexcept {
        addrinfo hints{};
        hints.ai_family = family;
        hints.ai_socktype = socktype;
        hints.ai_protocol = protocol;
        hints.ai_flags = flags;

        addrinfo *result = nullptr;
        const int ret = ::getaddrinfo(host, service, &hints, &result);

        if (ret != 0) {
            return std::error_code(ret, std::generic_category());
        }

        out.clear();
        for (addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
            resolved_entry entry{};
            entry.family = rp->ai_family;
            entry.socktype = rp->ai_socktype;
            entry.protocol = rp->ai_protocol;
            entry.addr_size = rp->ai_addrlen;

            if (entry.addr_size <= sizeof(entry.addr_data)) {
                std::memcpy(entry.addr_data, rp->ai_addr, entry.addr_size);
            }

            if (rp->ai_canonname != nullptr) {
                std::strncpy(entry.canonical_name, rp->ai_canonname, sizeof(entry.canonical_name) - 1);
                entry.canonical_name[sizeof(entry.canonical_name) - 1] = '\0';
            }

            out.push_back(entry);
        }

        ::freeaddrinfo(result);
        return std::error_code();
    }

    std::error_code reverse_resolve(const void *addr_data, int addr_len, char *host_buf, std::size_t host_buf_size, char *svc_buf,
                                    std::size_t svc_buf_size) noexcept {
        if (addr_data == nullptr || host_buf == nullptr || host_buf_size == 0) {
            return std::make_error_code(std::errc::invalid_argument);
        }

        sockaddr_storage ss{};
        std::memcpy(&ss, addr_data, addr_len);

        const int ret = ::getnameinfo(reinterpret_cast<sockaddr *>(&ss), addr_len, host_buf, static_cast<socklen_t>(host_buf_size),
                                      svc_buf, static_cast<socklen_t>(svc_buf_size), NI_NUMERICHOST | NI_NUMERICSERV);

        if (ret != 0) {
            return std::error_code(ret, std::generic_category());
        }

        return std::error_code();
    }
}
