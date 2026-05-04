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
#ifndef RAINY_FOUNDATION_IO_NET_IO_INTERNET_ADDRESS_HPP
#define RAINY_FOUNDATION_IO_NET_IO_INTERNET_ADDRESS_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/implements/addr.hpp>

namespace rainy::foundation::exceptions::net {
    class bad_address_cast : public runtime::runtime_error {
    public:
        bad_address_cast(const source &location) : runtime_error("bad address cast", location) { // NOLINT
        }
    };

    RAINY_INLINE rain_fn throw_bad_address_cast(const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(bad_address_cast{location});
    }
}

namespace rainy::foundation::io::net::ip {
    enum class resolver_errc : int {
        host_not_found = 1,
        try_again = 2,
        service_not_found = 3
    };

    class resolver_error_category final : public std::error_category {
    public:
        const char *name() const noexcept override {
            return "rainy.resolver";
        }
        std::string message(int ev) const override {
            const char *msg = implements::resolver_errc_message(ev);
            return msg ? msg : "unknown resolver error";
        }
    };

    RAINY_INLINE const std::error_category &resolver_category() noexcept {
        static resolver_error_category instance;
        return instance;
    }

    RAINY_INLINE std::error_code make_error_code(resolver_errc e) noexcept {
        return {static_cast<int>(e), resolver_category()};
    }

    RAINY_INLINE std::error_condition make_error_condition(resolver_errc e) noexcept {
        return {static_cast<int>(e), resolver_category()};
    }
}

namespace rainy::foundation::io::net::ip {
    using port_type = uint_least16_t;
    using scope_id_type = uint_least32_t;
    struct v4_mapped_t {};
    constexpr v4_mapped_t v4_mapped;
}

namespace rainy::foundation::io::net::ip {
    class address_v4 {
    public:
        using uint_type = std::uint_least32_t;

        struct bytes_type : collections::array<unsigned char, 4> {
            template <typename... Ty>
            explicit constexpr bytes_type(Ty... t) : array{std::in_place, static_cast<unsigned char>(t)...} {
            }
        };

        constexpr address_v4() noexcept : bytes_{0, 0, 0, 0} {
        }

        constexpr address_v4(const address_v4 &) noexcept = default;
        address_v4 &operator=(const address_v4 &) noexcept = default;

        explicit constexpr address_v4(const bytes_type &bytes) : bytes_{bytes} {
        }

        explicit constexpr address_v4(uint_type val) :
            bytes_{static_cast<unsigned char>((val >> 24) & 0xFFu), static_cast<unsigned char>((val >> 16) & 0xFFu),
                   static_cast<unsigned char>((val >> 8) & 0xFFu), static_cast<unsigned char>(val & 0xFFu)} {
        }

        constexpr bool is_unspecified() const noexcept {
            // NOLINTBEGIN
            return bytes_[0] == 0 && bytes_[1] == 0 && bytes_[2] == 0 && bytes_[3] == 0;
            // NOLINTEND
        }

        constexpr bool is_loopback() const noexcept {
            return bytes_[0] == 127; // 127.0.0.0/8 NOLINT
        }

        constexpr bool is_multicast() const noexcept {
            return (bytes_[0] & 0xF0u) == 0xE0u; // 224.0.0.0/4 NOLINT
        }

        constexpr bytes_type to_bytes() const noexcept {
            return bytes_;
        }

        RAINY_NODISCARD constexpr uint_type to_uint() const noexcept {
            return (static_cast<uint_type>(bytes_[0]) << 24) | (static_cast<uint_type>(bytes_[1]) << 16) |
                   (static_cast<uint_type>(bytes_[2]) << 8) | static_cast<uint_type>(bytes_[3]);
        }

        template <typename Allocator = std::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> to_string(const Allocator &alloc = Allocator()) const {
            char buf[16]{}; // INET_ADDRSTRLEN
            implements::ipv4_bytes raw{};
            std::memcpy(raw.data, bytes_.data(), 4);
            utility::ignore = implements::ipv4_to_string(raw, buf, sizeof(buf));
            return {buf, alloc};
        }

        static constexpr address_v4 any() noexcept {
            return address_v4{uint_type{0}}; // 0.0.0.0
        }

        static constexpr address_v4 loopback() noexcept {
            return address_v4{uint_type{0x7F000001u}}; // 127.0.0.1
        }

        static constexpr address_v4 broadcast() noexcept {
            return address_v4{uint_type{0xFFFFFFFFu}}; // 255.255.255.255
        }

    private:
        bytes_type bytes_;
    };

    constexpr bool operator==(const address_v4 &left, const address_v4 &right) noexcept { // NOLINT
        return left.to_uint() == right.to_uint();
    }

    constexpr bool operator!=(const address_v4 &left, const address_v4 &right) noexcept {
        return !(left == right);
    }

    constexpr bool operator<(const address_v4 &left, const address_v4 &right) noexcept { // NOLINT
        return left.to_uint() < right.to_uint();
    }

    constexpr bool operator>(const address_v4 &left, const address_v4 &right) noexcept {
        return right < left;
    }

    constexpr bool operator<=(const address_v4 &left, const address_v4 &right) noexcept {
        return !(right < left);
    }

    constexpr bool operator>=(const address_v4 &left, const address_v4 &right) noexcept {
        return !(left < right);
    }
}

namespace rainy::foundation::io::net::ip {
    class address_v6 {
    public:
        struct bytes_type : collections::array<unsigned char, 16> {
            template <typename... Ty>
            explicit constexpr bytes_type(Ty... t) : array{std::in_place, static_cast<unsigned char>(t)...} {
            }
        };

        constexpr address_v6() noexcept : bytes_{}, scope_id_{0} {
        }

        constexpr address_v6(const address_v6 &) noexcept = default;
        address_v6 &operator=(const address_v6 &) noexcept = default;

        constexpr address_v6(const bytes_type &bytes, const scope_id_type scope = 0) : bytes_{bytes}, scope_id_{scope} {
        }

        void scope_id(scope_id_type id) noexcept {
            scope_id_ = id;
        }

        constexpr scope_id_type scope_id() const noexcept {
            return scope_id_;
        }

        RAINY_NODISCARD constexpr bool is_unspecified() const noexcept {
            for (auto right: bytes_) { // NOLINT
                if (right != 0) {
                    return false;
                }
            }
            return true;
        }

        RAINY_NODISCARD constexpr bool is_loopback() const noexcept {
            for (int i = 0; i < 15; ++i) {
                if (bytes_[i] != 0) {
                    return false;
                }
            }
            return bytes_[15] == 1;
        }

        RAINY_NODISCARD constexpr bool is_multicast() const noexcept {
            return bytes_[0] == 0xFFu;
        }

        RAINY_NODISCARD constexpr bool is_link_local() const noexcept {
            // fe80::/10
            return bytes_[0] == 0xFEu && (bytes_[1] & 0xC0u) == 0x80u;
        }

        RAINY_NODISCARD constexpr bool is_site_local() const noexcept {
            // fec0::/10（废弃，但要求实现）
            return bytes_[0] == 0xFEu && (bytes_[1] & 0xC0u) == 0xC0u;
        }

        RAINY_NODISCARD constexpr bool is_v4_mapped() const noexcept {
            // ::ffff:0:0/96
            for (int i = 0; i < 10; ++i) {
                if (bytes_[i] != 0) {
                    return false;
                }
            }
            return bytes_[10] == 0xFFu && bytes_[11] == 0xFFu; // NOLINT
        }

        RAINY_NODISCARD constexpr bool is_multicast_node_local() const noexcept {
            return is_multicast() && (bytes_[1] & 0x0Fu) == 0x01u;
        }

        RAINY_NODISCARD constexpr bool is_multicast_link_local() const noexcept {
            return is_multicast() && (bytes_[1] & 0x0Fu) == 0x02u;
        }

        RAINY_NODISCARD constexpr bool is_multicast_site_local() const noexcept {
            return is_multicast() && (bytes_[1] & 0x0Fu) == 0x05u;
        }

        RAINY_NODISCARD constexpr bool is_multicast_org_local() const noexcept {
            return is_multicast() && (bytes_[1] & 0x0Fu) == 0x08u;
        }

        RAINY_NODISCARD constexpr bool is_multicast_global() const noexcept {
            return is_multicast() && (bytes_[1] & 0x0Fu) == 0x0Eu;
        }

        RAINY_NODISCARD constexpr bytes_type to_bytes() const noexcept {
            return bytes_;
        }

        template <typename Allocator = std::allocator<char>>
        RAINY_NODISCARD text::basic_string<char, text::char_traits<char>, Allocator> to_string(const Allocator &alloc = Allocator()) const {
            char buf[46]{}; // INET6_ADDRSTRLEN
            implements::ipv6_bytes raw{};
            std::memcpy(raw.data, bytes_.data(), 16);
            utility::ignore = implements::ipv6_to_string(raw, buf, sizeof(buf));

            text::basic_string<char, text::char_traits<char>, Allocator> result(buf, alloc);
            if (scope_id_ != 0) {
                char scope_buf[12]{};
                auto [ptr, ec] = std::to_chars(scope_buf, scope_buf + sizeof(scope_buf), scope_id_);
                result += '%';
                result.append(scope_buf, ptr);
            }
            return result;
        }

        static constexpr address_v6 any() noexcept {
            return address_v6{}; // ::
        }

        static constexpr address_v6 loopback() noexcept {
            bytes_type right{};
            right[15] = 1; // NOLINT
            return address_v6{right}; // ::1
        }

    private:
        bytes_type bytes_;
        scope_id_type scope_id_;
    };

    constexpr bool operator==(const address_v6 &left, const address_v6 &right) noexcept { // NOLINT
        if (left.scope_id() != right.scope_id()) {
            return false;
        }
        const auto ab = left.to_bytes(), bb = right.to_bytes();
        for (int i = 0; i < 16; ++i) {
            if (ab[i] != bb[i]) { // NOLINT
                return false;
            }
        }
        return true;
    }

    constexpr bool operator!=(const address_v6 &left, const address_v6 &right) noexcept {
        return !(left == right);
    }

    constexpr bool operator<(const address_v6 &left, const address_v6 &right) noexcept { // NOLINT
        if (left.scope_id() != right.scope_id()) {
            return left.scope_id() < right.scope_id();
        }
        const auto ab = left.to_bytes(), bb = right.to_bytes();
        for (int i = 0; i < 16; ++i) {
            // NOLINTBEGIN
            if (ab[i] != bb[i]) {
                return ab[i] < bb[i];
            }
            // NOLINTEND
        }
        return false;
    }
    constexpr bool operator>(const address_v6 &left, const address_v6 &right) noexcept {
        return right < left;
    }
    constexpr bool operator<=(const address_v6 &left, const address_v6 &right) noexcept {
        return !(right < left);
    }
    constexpr bool operator>=(const address_v6 &left, const address_v6 &right) noexcept {
        return !(left < right);
    }
}

namespace rainy::foundation::io::net::ip {

    class address {
    public:
        constexpr address() noexcept : v4_{}, v6_{}, is_v4_{true} {
        }

        constexpr address(const address &) noexcept = default;
        address &operator=(const address &) noexcept = default;

        constexpr address(const address_v4 &right) noexcept // NOLINT(google-explicit-constructor)
            : v4_{right}, is_v4_{true} {
        }

        constexpr address(const address_v6 &right) noexcept // NOLINT(google-explicit-constructor)
            : v6_{right}, is_v4_{false} {
        }

        address &operator=(const address_v4 &right) noexcept {
            v4_ = right;
            is_v4_ = true;
            return *this;
        }

        address &operator=(const address_v6 &right) noexcept {
            v6_ = right;
            is_v4_ = false;
            return *this;
        }

        RAINY_NODISCARD constexpr bool is_v4() const noexcept {
            return is_v4_;
        }

        RAINY_NODISCARD constexpr bool is_v6() const noexcept {
            return !is_v4_;
        }

        RAINY_NODISCARD constexpr address_v4 to_v4() const {
            if (!is_v4_) {
                exceptions::net::throw_bad_address_cast(diagnostics::source_location::current());
            }
            return v4_;
        }

        RAINY_NODISCARD constexpr address_v6 to_v6() const {
            if (is_v4_) {
                exceptions::net::throw_bad_address_cast(diagnostics::source_location::current());
            }
            return v6_;
        }

        RAINY_NODISCARD constexpr bool is_unspecified() const noexcept {
            return is_v4_ ? v4_.is_unspecified() : v6_.is_unspecified();
        }

        RAINY_NODISCARD constexpr bool is_loopback() const noexcept {
            return is_v4_ ? v4_.is_loopback() : v6_.is_loopback();
        }

        RAINY_NODISCARD constexpr bool is_multicast() const noexcept {
            return is_v4_ ? v4_.is_multicast() : v6_.is_multicast();
        }

        template <typename Allocator = std::allocator<char>>
        RAINY_NODISCARD rain_fn to_string(const Allocator &alloc = Allocator()) const
            -> text::basic_string<char, text::char_traits<char>, Allocator> {
            return is_v4_ ? v4_.to_string(alloc) : v6_.to_string(alloc);
        }

    private:
        address_v4 v4_;
        address_v6 v6_;
        bool is_v4_;
    };

    constexpr bool operator==(const address &right, const address &b) noexcept { // NOLINT
        if (right.is_v4() != b.is_v4()) {
            return false;
        }
        return right.is_v4() ? (right.to_v4() == b.to_v4()) : (right.to_v6() == b.to_v6());
    }

    constexpr bool operator!=(const address &right, const address &b) noexcept {
        return !(right == b);
    }

    constexpr bool operator<(const address &right, const address &b) noexcept { // NOLINT
        if (right.is_v4() && b.is_v6()) {
            return true;
        }
        if (right.is_v6() && b.is_v4()) {
            return false;
        }
        return right.is_v4() ? (right.to_v4() < b.to_v4()) : (right.to_v6() < b.to_v6());
    }

    constexpr bool operator>(const address &right, const address &b) noexcept {
        return b < right;
    }
    constexpr bool operator<=(const address &right, const address &b) noexcept {
        return !(b < right);
    }
    constexpr bool operator>=(const address &right, const address &b) noexcept {
        return !(right < b);
    }
}

namespace rainy::foundation::io::net::ip {
    constexpr address_v4 make_address_v4(const address_v4::bytes_type &b) {
        return address_v4{b};
    }

    constexpr address_v4 make_address_v4(address_v4::uint_type v) {
        return address_v4{v};
    }

    constexpr address_v4 make_address_v4(v4_mapped_t, const address_v6 &v6) {
        if (!v6.is_v4_mapped()) {
            exceptions::net::throw_bad_address_cast(diagnostics::source_location::current());
        }
        const auto b6 = v6.to_bytes();
        const address_v4::bytes_type b4{b6[12], b6[13], b6[14], b6[15]}; // NOLINT
        return address_v4{b4};
    }

    constexpr address_v6 make_address_v6(const address_v6::bytes_type &b, scope_id_type scope = 0) {
        return address_v6{b, scope};
    }

    constexpr address_v6 make_address_v6(v4_mapped_t, const address_v4 &v4) noexcept {
        const auto b4 = v4.to_bytes();
        address_v6::bytes_type b6{};
        // NOLINTBEGIN
        b6[10] = 0xFFu;
        b6[11] = 0xFFu;
        b6[12] = b4[0];
        b6[13] = b4[1];
        b6[14] = b4[2];
        b6[15] = b4[3];
        // NOLINTEND
        return address_v6{b6};
    }

    address make_address(text::string_view addr);
    address make_address(text::string_view addr, std::error_code &) noexcept;

    address_v4 make_address_v4(text::string_view sv);
    address_v4 make_address_v4(text::string_view sv, std::error_code &) noexcept;

    address_v6 make_address_v6(text::string_view sv);
    address_v6 make_address_v6(text::string_view sv, std::error_code &) noexcept;

    template <class CharT, class Traits>
    std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, const address &right) {
        return os << right.to_string().c_str();
    }

    template <class CharT, class Traits>
    std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, const address_v4 &right) {
        return os << right.to_string().c_str();
    }

    template <class CharT, class Traits>
    std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, const address_v6 &right) {
        return os << right.to_string().c_str();
    }
}

namespace rainy::foundation::io::net::ip {
    template <typename Address>
    class basic_address_iterator {
    public:
        using value_type = Address;
        using difference_type = std::ptrdiff_t;
        using pointer = const Address *;
        using reference = const Address &;
        using iterator_category = std::input_iterator_tag;

        explicit basic_address_iterator(const Address &a) noexcept : address_{a} {
        }

        reference operator*() const noexcept {
            return address_;
        }
        pointer operator->() const noexcept {
            return &address_;
        }

        basic_address_iterator &operator++() noexcept {
            increment(address_);
            return *this;
        }

        basic_address_iterator operator++(int) noexcept {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        basic_address_iterator &operator--() noexcept {
            decrement(address_);
            return *this;
        }

        basic_address_iterator operator--(int) noexcept {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        friend bool operator==(const basic_address_iterator &x, const basic_address_iterator &y) noexcept {
            return *x == *y;
        }
        friend bool operator!=(const basic_address_iterator &x, const basic_address_iterator &y) noexcept {
            return !(x == y);
        }

    private:
        Address address_;

        // address_v4 特化：主机字节序 uint32 直接加减
        static void increment(address_v4 &a) noexcept {
            a = address_v4{a.to_uint() + 1u};
        }
        static void decrement(address_v4 &a) noexcept {
            a = address_v4{a.to_uint() - 1u};
        }

        // address_v6 特化：128 位大端字节序加减
        static void increment(address_v6 &a) noexcept {
            auto bytes = a.to_bytes();
            for (int i = 15; i >= 0; --i) {
                if (++bytes[i] != 0) {
                    break;
                }
            }
            a = address_v6{bytes, a.scope_id()};
        }
        static void decrement(address_v6 &a) noexcept {
            auto bytes = a.to_bytes();
            for (int i = 15; i >= 0; --i) {
                if (bytes[i]-- != 0) {
                    break;
                }
            }
            a = address_v6{bytes, a.scope_id()};
        }
    };
}

namespace rainy::foundation::io::net::ip {
    template <typename Address>
    class basic_address_range {
    public:
        using iterator = basic_address_iterator<Address>;

        basic_address_range() noexcept : begin_{Address{}}, end_{Address{}} {
        }

        basic_address_range(const Address &first, const Address &last) noexcept : begin_{first}, end_{last} {
        }

        iterator begin() const noexcept {
            return begin_;
        }
        iterator end() const noexcept {
            return end_;
        }

        bool empty() const noexcept {
            return *begin_ == *end_;
        }

        std::size_t size() const noexcept {
            return range_size(begin_, end_);
        }

        iterator find(const Address &addr) const noexcept {
            for (auto it = begin_; it != end_; ++it) {
                if (*it == addr) {
                    return it;
                }
            }
            return end_;
        }

    private:
        static std::size_t range_size(const iterator &b, const iterator &e) noexcept {
            if constexpr (type_traits::type_relations::is_same_v<Address, address_v6>) {
                // NOLINTBEGIN
                const auto bv = (*b).to_bytes();
                const auto ev = (*e).to_bytes();
                // e < b → 空
                for (int i = 0; i < 16; ++i) {
                    if (ev[i] < bv[i]) {
                        return 0u;
                    }
                    if (ev[i] > bv[i]) {
                        break;
                    }
                }
                // 128 位大端减法
                unsigned char borrow = 0;
                unsigned char diff[16]{};
                for (int i = 15; i >= 0; --i) {
                    int d = static_cast<int>(ev[i]) - static_cast<int>(bv[i]) - borrow;
                    if (d < 0) {
                        d += 256;
                        borrow = 1;
                    } else {
                        borrow = 0;
                    }
                    diff[i] = static_cast<unsigned char>(d);
                }
                // 高 (16 - sizeof(size_t)) 字节非零 → 饱和
                constexpr int sz = static_cast<int>(sizeof(std::size_t));
                for (int i = 0; i < 16 - sz; ++i) {
                    if (diff[i] != 0) {
                        return utility::numeric_limits<std::size_t>::max();
                    }
                }
                // 低 sz 字节组合为 size_t（大端）
                std::size_t result = 0;
                for (int i = 16 - sz; i < 16; ++i) {
                    result = (result << 8) | diff[i];
                }
                return result;
                // NOLINTEND
            } else { // address_v4
                const auto bv = (*b).to_uint();
                const auto ev = (*e).to_uint();
                return ev >= bv ? static_cast<std::size_t>(ev - bv) : 0u;
            }
        }

        iterator begin_;
        iterator end_;
    };
}

#endif
