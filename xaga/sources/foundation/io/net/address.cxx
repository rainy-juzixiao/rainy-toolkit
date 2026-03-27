#include <rainy/foundation/io/net/internet/address.hpp>
#include <system_error>

namespace pal = rainy::foundation::io::net::ip::implements;

namespace rainy::foundation::io::net::ip {
    template <std::size_t N>
    bool sv_to_cstr(const text::string_view sv, char (&buf)[N]) noexcept {
        if (sv.size() >= N) {
            return false;
        }
        std::memcpy(buf, sv.data(), sv.size());
        buf[sv.size()] = '\0';
        return true;
    }

    address_v4 make_address_v4(const text::string_view sv, std::error_code &ec) noexcept {
        char buf[16]{};
        if (!sv_to_cstr(sv, buf)) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return {};
        }
        pal::ipv4_bytes raw{};
        if (!pal::ipv4_from_string(buf, raw, ec)) {
            return {};
        }
        const address_v4::bytes_type b{raw.data[0], raw.data[1], raw.data[2], raw.data[3]};
        return address_v4{b};
    }

    address_v4 make_address_v4(const text::string_view sv) {
        std::error_code ec;
        auto r = make_address_v4(sv, ec);
        if (ec) {
            throw std::system_error(ec);
        }
        return r;
    }

    address_v6 make_address_v6(const text::string_view sv, std::error_code &ec) noexcept { // NOLINT
        scope_id_type scope = 0;
        text::string_view addr_part = sv;
        if (const auto percent = sv.find('%'); percent != text::string_view::npos) {
            const auto scope_sv = sv.substr(percent + 1);
            addr_part = sv.substr(0, percent);
            scope_id_type val = 0;
            if (const auto [ptr, e] = text::from_chars(scope_sv.data(), scope_sv.data() + scope_sv.size(), val);
                e != std::errc{} || ptr != scope_sv.data() + scope_sv.size()) { // NOLINT
                ec = std::make_error_code(std::errc::invalid_argument);
                return {};
            }
            scope = val;
        }
        char buf[46]{};
        if (!sv_to_cstr(addr_part, buf)) {
            ec = std::make_error_code(std::errc::invalid_argument);
            return {};
        }
        pal::ipv6_bytes raw{};
        if (!pal::ipv6_from_string(buf, raw, ec)) {
            return {};
        }
        address_v6::bytes_type bytes{};
        std::memcpy(bytes.data(), raw.data, 16);
        return address_v6{bytes, scope};
    }

    address_v6 make_address_v6(const text::string_view sv) {
        std::error_code ec;
        auto r = make_address_v6(sv, ec);
        if (ec) {
            throw std::system_error(ec);
        }
        return r;
    }

    address make_address(const text::string_view addr, std::error_code &ec) noexcept {
        if (const auto v4 = make_address_v4(addr, ec); !ec) {
            return address{v4};
        }
        if (const auto v6 = make_address_v6(addr, ec); !ec) {
            return address{v6};
        }
        return {};
    }

    address make_address(const text::string_view addr) {
        std::error_code ec;
        auto r = make_address(addr, ec);
        if (ec) {
            throw std::system_error(ec);
        }
        return r;
    }
}
