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
#ifndef RAINY_FOUNDATION_IO_NET_IP_PAL_INTERNET_ADDR_HPP
#define RAINY_FOUNDATION_IO_NET_IP_PAL_INTERNET_ADDR_HPP

// NOLINTBEGIN

#include <cstdint>
#include <rainy/collections/vector.hpp>
#include <rainy/core/core.hpp>
#include <system_error>

// NOLINTEND

namespace rainy::foundation::io::net::ip::implements {
    using port_type = std::uint_least16_t;
    using scope_id_type = std::uint_least32_t;

    struct ipv4_bytes {
        unsigned char data[4];
    };
    struct ipv6_bytes {
        unsigned char data[16];
    };

    // clang-format off
    struct resolver_errc_values {
        int host_not_found;     // EAI_NONAME  / WSAHOST_NOT_FOUND
        int try_again;          // EAI_AGAIN   / WSATRY_AGAIN
        int service_not_found;  // EAI_SERVICE / WSATYPE_NOT_FOUND
    };
    // clang-format on

    const resolver_errc_values &get_resolver_errc_values() noexcept;

    const char *resolver_errc_message(int ev) noexcept;

    /*
     * 将点分十进制字符串（null-terminated）解析为 ipv4_bytes（网络字节序）。
     * 成功返回 true，失败返回 false 并设置 ec。
     */
    RAINY_TOOLKIT_API RAINY_NODISCARD bool ipv4_from_string(core::czstring str, ipv4_bytes &out, std::error_code &ec) noexcept;

    /*
     * 将 ipv4_bytes（网络字节序）格式化为点分十进制写入 buf。
     * buf 至少需要 INET_ADDRSTRLEN（16）字节。
     * 成功返回 true。
     */
    RAINY_TOOLKIT_API RAINY_NODISCARD bool ipv4_to_string(const ipv4_bytes &in, core::cstring buf, std::size_t buf_size) noexcept;

    /*
     * 将 IPv6 字符串（不含 %scope，null-terminated）解析为 ipv6_bytes。
     * 成功返回 true，失败返回 false 并设置 ec。
     */
    RAINY_TOOLKIT_API RAINY_NODISCARD bool ipv6_from_string(core::czstring str, ipv6_bytes &out, std::error_code &ec) noexcept;

    /*
     * 将 ipv6_bytes 格式化为标准 IPv6 字符串写入 buf。
     * buf 至少需要 INET6_ADDRSTRLEN（46）字节。
     * 成功返回 true。
     */
    RAINY_TOOLKIT_API RAINY_NODISCARD bool ipv6_to_string(const ipv6_bytes &in, core::cstring buf, std::size_t buf_size) noexcept;

    struct resolved_entry {
        int family{0};
        int socktype{0};
        int protocol{0};
        std::size_t addr_size{0};
        std::uint8_t addr_data[128]{};
        char canonical_name[256]{};
    };

    // 正向解析：host/service → 地址列表
    // 实现在 addr.win.cxx / addr.linux.cxx 里调用 getaddrinfo
    std::error_code resolve(const char *host, const char *service, int family, int socktype, int protocol, int flags,
                            collections::vector<resolved_entry> &out) noexcept;

    // 反向解析：地址 → host/service
    // 实现在 addr.win.cxx / addr.linux.cxx 里调用 getnameinfo
    std::error_code reverse_resolve(const void *addr_data, int addr_len, char *host_buf, std::size_t host_buf_size, char *svc_buf,
                                    std::size_t svc_buf_size) noexcept;
}

#endif
