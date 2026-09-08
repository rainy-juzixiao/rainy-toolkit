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
#include <rainy/foundation/io/network/fwd.hpp>
#include <rainy/foundation/io/network/internet/address.hpp>

namespace rainy::foundation::io::net::ip {
    class network_v4 {
    public:
        constexpr network_v4() noexcept : addr_{}, prefix_len_{0} {
        }

        constexpr network_v4(const address_v4 &addr, int prefix_len) : addr_{addr}, prefix_len_{prefix_len} {
            if (prefix_len_ < 0 || prefix_len_ > 32) {
                exceptions::io::throw_bad_address_cast(core::diagnostics::source_location::current());
            }
        }

        constexpr network_v4(const address_v4 &addr, const address_v4 &mask) : addr_{addr}, prefix_len_{mask_to_prefix(mask)} {
        }

        constexpr address_v4 address() const noexcept {
            return addr_;
        }

        constexpr int prefix_length() const noexcept {
            return prefix_len_;
        }

        constexpr address_v4 netmask() const noexcept {
            const address_v4::uint_type mask =
                prefix_len_ == 0 ? address_v4::uint_type{0} : ~address_v4::uint_type{0} << (32 - prefix_len_);
            return address_v4{mask};
        }

        constexpr address_v4 network() const noexcept {
            return address_v4{addr_.to_uint() & netmask().to_uint()};
        }

        constexpr address_v4 broadcast() const noexcept {
            const auto net = network().to_uint();
            const auto wild = ~netmask().to_uint();
            return address_v4{net | wild};
        }

        address_v4_range hosts() const noexcept;

        constexpr network_v4 canonical() const noexcept {
            return network_v4{network(), prefix_len_};
        }

        constexpr bool is_host() const noexcept {
            return prefix_len_ == 32;
        }

        constexpr bool is_subnet_of(const network_v4 &other) const noexcept {
            if (prefix_len_ <= other.prefix_len_)
                return false;
            return (network().to_uint() & other.netmask().to_uint()) == other.network().to_uint();
        }

        template <typename Allocator = memory::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> to_string(const Allocator &alloc = Allocator()) const {
            auto result = network().to_string(alloc);
            result += '/';
            char len_buf[4]{};
            auto [ptr, ec] = text::to_chars(len_buf, len_buf + sizeof(len_buf), prefix_len_);
            result.append(len_buf, ptr);
            return result;
        }

        friend constexpr bool operator==(const network_v4 &a, const network_v4 &b) noexcept {
            return a.prefix_len_ == b.prefix_len_ && a.network() == b.network();
        }

        friend constexpr bool operator!=(const network_v4 &a, const network_v4 &b) noexcept {
            return !(a == b);
        }

    private:
        static constexpr int mask_to_prefix(const address_v4 &mask) {
            const auto m = mask.to_uint();
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
                exceptions::io::throw_bad_address_cast(core::diagnostics::source_location::current());
            }
            return len;
        }

        address_v4 addr_;
        int prefix_len_;
    };

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

        constexpr network_v6(const address_v6 &addr, int prefix_len) : addr_{addr}, prefix_len_{prefix_len} {
            if (prefix_len_ < 0 || prefix_len_ > 128) {
                exceptions::io::throw_bad_address_cast(core::diagnostics::source_location::current());
            }
        }

        constexpr address_v6 address() const noexcept {
            return addr_;
        }

        constexpr int prefix_length() const noexcept {
            return prefix_len_;
        }

        constexpr address_v6 network() const noexcept {
            auto bytes = addr_.to_bytes();
            apply_prefix_mask(bytes, prefix_len_);
            return address_v6{bytes, addr_.scope_id()};
        }

        address_v6_range hosts() const noexcept;

        constexpr network_v6 canonical() const noexcept {
            return network_v6{network(), prefix_len_};
        }

        constexpr bool is_host() const noexcept {
            return prefix_len_ == 128;
        }

        constexpr bool is_subnet_of(const network_v6 &other) const noexcept {
            if (prefix_len_ <= other.prefix_len_)
                return false;
            auto self_net = network().to_bytes();
            apply_prefix_mask(self_net, other.prefix_len_);
            return address_v6{self_net} == other.network();
        }

        template <typename Allocator = memory::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> to_string(const Allocator &alloc = Allocator()) const {
            auto result = network().to_string(alloc);
            result += '/';
            char len_buf[4]{};
            auto [ptr, ec] = text::to_chars(len_buf, len_buf + sizeof(len_buf), prefix_len_);
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
        static constexpr void apply_prefix_mask(address_v6::bytes_type &bytes, int prefix_len) noexcept {
            const int full_bytes = prefix_len / 8;
            const int remainder = prefix_len % 8;
            if (full_bytes < 16) {
                const unsigned char partial_mask =
                    remainder == 0 ? static_cast<unsigned char>(0x00) : static_cast<unsigned char>(0xFF << (8 - remainder));
                bytes[full_bytes] &= partial_mask;
            }
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
