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
#include <rainy/foundation/io/net/internet/network.hpp>

namespace rainy::foundation::io::net::ip {
    bool split_cidr(text::string_view cidr, text::string_view &addr_part, int &prefix, std::error_code &ec) noexcept {
        const auto slash = cidr.find('/');
        if (slash == text::string_view::npos) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }

        addr_part = cidr.substr(0, slash);
        const auto len_sv = cidr.substr(slash + 1);

        // 使用 from_chars 解析前缀长度，避免 atoi 的未定义行为
        unsigned int val = 0;
        const auto [ptr, e] = std::from_chars(len_sv.data(), len_sv.data() + len_sv.size(), val);

        if (e != std::errc{} || ptr != len_sv.data() + len_sv.size()) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }

        prefix = static_cast<int>(val);
        ec.clear();
        return true;
    }

    address_v4_range network_v4::hosts() const noexcept {
        /*
         * RFC 语义：
         *   /32 → 仅主机本身，无可迭代范围 → 空
         *   /31 → 两端均可用（RFC 3021），返回 [network, broadcast]（含两端）
         *   其余 → [network+1, broadcast)（去掉网络地址和广播地址）
         */
        const auto net = network();
        const auto bcast = broadcast();
        if (prefix_len_ == 32) {
            // 空范围
            return address_v4_range{net, net}; // NOLINT
        }
        if (prefix_len_ == 31) {
            // [network, broadcast]：end 迭代器需要指向 broadcast 的下一个地址
            return address_v4_range{net, address_v4{bcast.to_uint() + 1u}}; // NOLINT
        }
        // 一般情况：跳过网络地址（+1）和广播地址（end = broadcast，不含）
        return address_v4_range{address_v4{net.to_uint() + 1u}, bcast}; // NOLINT
    }

    address_v6_range network_v6::hosts() const noexcept {
        /*
         *   /128 → 空
         *   /127 → [network, network+1]（含两端，RFC 6164）
         *   其余 → [network+1, last)
         *          last = 所有主机位置 1 的地址（类似广播）
         */
        const auto net_addr = network();

        if (prefix_len_ == 128) {
            return address_v6_range{net_addr, net_addr};
        }
        // 计算"广播"等价地址（主机位全 1）
        auto last_bytes = net_addr.to_bytes();
        const int full_bytes = prefix_len_ / 8;
        const int remainder = prefix_len_ % 8;
        if (full_bytes < 16) {
            // 部分字节：低 (8 - remainder) 位置 1
            if (remainder != 0) {
                last_bytes[full_bytes] |= static_cast<unsigned char>(0xFF >> remainder);
            }
        }
        for (int i = full_bytes + (remainder != 0 ? 1 : 0); i < 16; ++i) {
            last_bytes[i] = 0xFF;
        }
        const address_v6 last{last_bytes, addr_.scope_id()};
        if (prefix_len_ == 127) {
            // [network, last]：end = last 的下一个地址
            auto end_bytes = last_bytes;
            for (int i = 15; i >= 0; --i) {
                if (++end_bytes[i] != 0)
                    break;
            }
            return address_v6_range{net_addr, address_v6{end_bytes, addr_.scope_id()}}; // NOLINT
        }
        // 一般情况：跳过网络地址
        auto first_bytes = net_addr.to_bytes();
        for (int i = 15; i >= 0; --i) {
            if (++first_bytes[i] != 0)
                break;
        }
        return address_v6_range{address_v6{first_bytes, addr_.scope_id()}, last}; // NOLINT
    }

    network_v4 make_network_v4(text::string_view cidr, std::error_code &ec) noexcept {
        text::string_view addr_part;
        int prefix = 0;
        if (!split_cidr(cidr, addr_part, prefix, ec))
            return {};
        if (prefix < 0 || prefix > 32) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return {};
        }
        const auto addr = make_address_v4(addr_part, ec);
        if (ec) {
            return {};
        }
        // 构造时若 prefix 越界会抛，但上面已经检查，这里安全
        return network_v4{addr, prefix};
    }

    network_v4 make_network_v4(text::string_view cidr) {
        std::error_code ec;
        auto r = make_network_v4(cidr, ec);
        if (ec)
            throw std::system_error(ec);
        return r;
    }

    network_v6 make_network_v6(text::string_view cidr, std::error_code &ec) noexcept {
        text::string_view addr_part;
        int prefix = 0;
        if (!split_cidr(cidr, addr_part, prefix, ec))
            return {};
        if (prefix < 0 || prefix > 128) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return {};
        }
        const auto addr = make_address_v6(addr_part, ec);
        if (ec)
            return {};
        return network_v6{addr, prefix};
    }

    network_v6 make_network_v6(text::string_view cidr) {
        std::error_code ec;
        auto r = make_network_v6(cidr, ec);
        if (ec)
            throw std::system_error(ec);
        return r;
    }
}