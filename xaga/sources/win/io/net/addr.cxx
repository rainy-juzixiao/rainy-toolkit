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
    namespace {
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

        // 确保 Winsock 在第一次用到 PAL 函数前初始化
        const winsock_initializer &ensure_winsock() noexcept {
            static winsock_initializer instance;
            return instance;
        }
    } // anonymous namespace

    const resolver_errc_values &get_resolver_errc_values() noexcept {
        static constexpr resolver_errc_values values{
            WSAHOST_NOT_FOUND, // host_not_found
            WSATRY_AGAIN, // try_again
            WSATYPE_NOT_FOUND // service_not_found
        };
        return values;
    }

    const char *resolver_errc_message(int ev) noexcept {
        // gai_strerrorA 在 ws2tcpip.h 中声明，返回静态字符串
        return ::gai_strerrorA(ev);
    }

    bool ipv4_from_string(core::czstring str, ipv4_bytes &out, std::error_code &ec) noexcept {
        ensure_winsock();
        IN_ADDR native{};
        if (::inet_pton(AF_INET, str, &native) != 1) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }
        // inet_pton 写入网络字节序，与平台无关地按字节拷贝
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
}
