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
#ifndef RAINY_FOUNDATION_IO_NET_IO_INTERNET_NETWORK_HPP
#define RAINY_FOUNDATION_IO_NET_IO_INTERNET_NETWORK_HPP
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/internet/address.hpp>

namespace rainy::foundation::io::net::ip {
    class network_v4 {
    public:
        constexpr network_v4() noexcept : addr_{}, prefix_len_{0} {
        }

        /*
         * prefix_len 必须在 [0, 32]，否则抛 std::out_of_range。
         * constexpr 上下文中违反此约束会导致编译失败（常量求值错误）。
         */
        constexpr network_v4(const address_v4 &addr, int prefix_len) : addr_{addr}, prefix_len_{prefix_len} {
            if (prefix_len_ < 0 || prefix_len_ > 32) {
                exceptions::net::throw_bad_address_cast(diagnostics::source_location::current());
            }
        }

        /*
         * 从地址 + 子网掩码构造。
         * 掩码必须是连续的高位 1（合法子网掩码），否则抛异常。
         */
        constexpr network_v4(const address_v4 &addr, const address_v4 &mask) : addr_{addr}, prefix_len_{mask_to_prefix(mask)} {
        }

        constexpr address_v4 address() const noexcept {
            return addr_;
        }

        constexpr int prefix_length() const noexcept {
            return prefix_len_;
        }

        /*
         * 生成子网掩码：prefix_len 个高位 1，其余 0。
         * 例如 prefix_len=24 → 0xFFFFFF00 → 255.255.255.0
         */
        constexpr address_v4 netmask() const noexcept {
            // prefix_len == 0 时移位 32 位是 UB，特殊处理
            const address_v4::uint_type mask =
                prefix_len_ == 0 ? address_v4::uint_type{0} : ~address_v4::uint_type{0} << (32 - prefix_len_);
            return address_v4{mask};
        }

        /*
         * 网络地址：addr & netmask
         */
        constexpr address_v4 network() const noexcept {
            return address_v4{addr_.to_uint() & netmask().to_uint()};
        }

        /*
         * 广播地址：network | ~netmask
         */
        constexpr address_v4 broadcast() const noexcept {
            const auto net = network().to_uint();
            const auto wild = ~netmask().to_uint();
            return address_v4{net | wild};
        }

        /*
         * 可用主机范围：(network+1) .. (broadcast-1)
         * /31 和 /32 按 RFC 3021 / RFC 4632 处理：
         *   /32 → 空范围
         *   /31 → [network, broadcast)（两端均可用）
         * 定义在 internet_network.cpp（依赖 address_v4_range 构造）
         */
        address_v4_range hosts() const noexcept;

        /*
         * 规范形式：地址部分只保留网络位，主机位清零。
         */
        constexpr network_v4 canonical() const noexcept {
            return network_v4{network(), prefix_len_};
        }

        /*
         * /32 表示单一主机路由。
         */
        constexpr bool is_host() const noexcept {
            return prefix_len_ == 32;
        }

        /*
         * 判断 *this 是否是 other 的子网（strict subset）。
         * 条件：
         *   1. *this 的前缀长度 > other 的前缀长度
         *   2. *this 的网络地址落在 other 的网络范围内
         */
        constexpr bool is_subnet_of(const network_v4 &other) const noexcept {
            if (prefix_len_ <= other.prefix_len_)
                return false;
            // 用 other 的掩码掩蔽 *this 的网络地址，结果应等于 other 的网络地址
            return (network().to_uint() & other.netmask().to_uint()) == other.network().to_uint();
        }

        // ── to_string（CIDR 记法，例如 "192.168.1.0/24"）──────────────────

        template <typename Allocator = std::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> to_string(const Allocator &alloc = Allocator()) const {
            auto result = network().to_string(alloc);
            result += '/';
            char len_buf[4]{};
            auto [ptr, ec] = std::to_chars(len_buf, len_buf + sizeof(len_buf), prefix_len_);
            result.append(len_buf, ptr);
            return result;
        }

        // ── 比较（canonical 形式下按网络地址+前缀长度排序）──────────────────

        friend constexpr bool operator==(const network_v4 &a, const network_v4 &b) noexcept {
            return a.prefix_len_ == b.prefix_len_ && a.network() == b.network();
        }

        friend constexpr bool operator!=(const network_v4 &a, const network_v4 &b) noexcept {
            return !(a == b);
        }

    private:
        /*
         * 将合法子网掩码转换为前缀长度。
         * 掩码必须是形如 1…10…0 的连续高位 1，否则抛 bad_address_cast。
         */
        static constexpr int mask_to_prefix(const address_v4 &mask) {
            const auto m = mask.to_uint();
            // 找到第一个 0 位的位置（从高位起）
            int len = 0;
            for (int bit = 31; bit >= 0; --bit) {
                if (m & (address_v4::uint_type{1} << bit)) {
                    ++len;
                } else {
                    break;
                }
            }
            const address_v4::uint_type tail_mask = len == 32 ? address_v4::uint_type{0} : ~(~address_v4::uint_type{0} << (32 - len));
            if (m & tail_mask) {
                // 非连续掩码
                exceptions::net::throw_bad_address_cast(diagnostics::source_location::current());
            }
            return len;
        }

        address_v4 addr_;
        int prefix_len_;
    };

    /*
     * 从 CIDR 字符串（如 "192.168.1.0/24"）构造 network_v4。
     * 字符串解析依赖 PAL，定义在 internet_network.cpp。
     */
    network_v4 make_network_v4(text::string_view cidr);
    network_v4 make_network_v4(text::string_view cidr, std::error_code &ec) noexcept;

    constexpr network_v4 make_network_v4(const address_v4 &addr, int prefix_len) {
        return network_v4{addr, prefix_len};
    }

    constexpr network_v4 make_network_v4(const address_v4 &addr, const address_v4 &mask) {
        return network_v4{addr, mask};
    }

    class network_v6 {
    public:
        constexpr network_v6() noexcept : addr_{}, prefix_len_{0} {
        }

        /*
         * prefix_len 必须在 [0, 128]。
         */
        constexpr network_v6(const address_v6 &addr, int prefix_len) : addr_{addr}, prefix_len_{prefix_len} {
            if (prefix_len_ < 0 || prefix_len_ > 128) {
                exceptions::net::throw_bad_address_cast(diagnostics::source_location::current());
            }
        }

        constexpr address_v6 address() const noexcept {
            return addr_;
        }

        constexpr int prefix_length() const noexcept {
            return prefix_len_;
        }

        /*
         * 网络地址：addr 按前缀长度掩码，主机位清零。
         */
        constexpr address_v6 network() const noexcept {
            auto bytes = addr_.to_bytes();
            apply_prefix_mask(bytes, prefix_len_);
            return address_v6{bytes, addr_.scope_id()};
        }

        /*
         * 可用主机范围。
         * /128 → 空；/127 → [network, network+1)；其余 → (network, last)
         * 定义在 internet_network.cpp。
         */
        address_v6_range hosts() const noexcept;

        /*
         * 规范形式：主机位清零。
         */
        constexpr network_v6 canonical() const noexcept {
            return network_v6{network(), prefix_len_};
        }

        /*
         * /128 表示单一主机路由。
         */
        constexpr bool is_host() const noexcept {
            return prefix_len_ == 128;
        }

        /*
         * 判断 *this 是否是 other 的子网（strict subset）。
         */
        constexpr bool is_subnet_of(const network_v6 &other) const noexcept {
            if (prefix_len_ <= other.prefix_len_)
                return false;
            // 将 *this 的网络地址用 other 的前缀长度掩码后，应等于 other 的网络地址
            auto self_net = network().to_bytes();
            apply_prefix_mask(self_net, other.prefix_len_);
            return address_v6{self_net} == other.network();
        }

        template <typename Allocator = std::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> to_string(const Allocator &alloc = Allocator()) const {
            auto result = network().to_string(alloc);
            result += '/';
            char len_buf[4]{};
            auto [ptr, ec] = std::to_chars(len_buf, len_buf + sizeof(len_buf), prefix_len_);
            result.append(len_buf, ptr);
            return result;
        }

        friend constexpr bool operator==(const network_v6 &a, const network_v6 &b) noexcept {
            return a.prefix_len_ == b.prefix_len_ && a.network() == b.network();
        }

        friend constexpr bool operator!=(const network_v6 &a, const network_v6 &b) noexcept {
            return !(a == b);
        }

    private:
        /*
         * 将 bytes（16 字节大端）的低 (128 - prefix_len) 位清零。
         * 纯字节操作，无平台依赖，constexpr 安全。
         */
        static constexpr void apply_prefix_mask(address_v6::bytes_type &bytes, int prefix_len) noexcept {
            // 完整掩码字节数
            const int full_bytes = prefix_len / 8;
            // 部分掩码字节的有效位数
            const int remainder = prefix_len % 8;
            // 部分字节：保留高 remainder 位
            if (full_bytes < 16) {
                const unsigned char partial_mask =
                    remainder == 0 ? static_cast<unsigned char>(0x00) : static_cast<unsigned char>(0xFF << (8 - remainder));
                bytes[full_bytes] &= partial_mask;
            }
            // 剩余字节全部清零
            for (int i = full_bytes + (remainder != 0 ? 1 : 0); i < 16; ++i) {
                bytes[i] = 0;
            }
        }

        address_v6 addr_;
        int prefix_len_;
    };

    network_v6 make_network_v6(text::string_view cidr);
    network_v6 make_network_v6(text::string_view cidr, std::error_code &ec) noexcept;

    constexpr network_v6 make_network_v6(const address_v6 &addr, int prefix_len) {
        return network_v6{addr, prefix_len};
    }

}

#endif
