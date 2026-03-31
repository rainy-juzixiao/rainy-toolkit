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
#ifndef RAINY_FOUNDATION_IO_NET_IO_INTERNET_ENDPOINT_HPP
#define RAINY_FOUNDATION_IO_NET_IO_INTERNET_ENDPOINT_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/net/internet/address.hpp>
#include <rainy/foundation/io/net/socket.hpp>

namespace rainy::foundation::io::net::ip {
    template <typename InternetProtocol>
    class basic_endpoint {
    public:
        using protocol_type = InternetProtocol;

        constexpr basic_endpoint() noexcept : protocol_{protocol_type::v4()} {
        }

        constexpr basic_endpoint(const protocol_type &proto, port_type port_num) noexcept :
            protocol_{proto}, port_{port_num}, addr_{proto.family() == 6 // AF_INET6
                                                         ? ip::address{address_v6::any()}
                                                         : ip::address{address_v4::any()}} {
        }

        constexpr basic_endpoint(const ip::address &addr, port_type port_num) noexcept :
            protocol_{addr.is_v4() ? protocol_type::v4() : protocol_type::v6()}, port_{port_num}, addr_{addr} {
        }

        constexpr protocol_type protocol() const noexcept {
            return protocol_;
        }

        constexpr ip::address address() const noexcept {
            return addr_;
        }

        void address(const ip::address &addr) noexcept {
            addr_ = addr;
            protocol_ = addr.is_v4() ? protocol_type::v4() : protocol_type::v6();
        }

        constexpr port_type port() const noexcept {
            return port_;
        }

        void port(port_type port_num) noexcept {
            port_ = port_num;
        }

        net::implements::raw_endpoint to_raw() const noexcept {
            net::implements::raw_endpoint r;
            if (addr_.is_v4()) {
                auto &sa = *reinterpret_cast<sockaddr_in_layout *>(r.data);
                sa.family = 2; // AF_INET
                sa.port = host_to_net16(port_);
                auto b = addr_.to_v4().to_bytes();
                std::memcpy(sa.addr, b.data(), 4);
                r.size = 16;
                r.family = 2;
            } else {
                auto &sa = *reinterpret_cast<sockaddr_in6_layout *>(r.data);
                sa.family = 10; // AF_INET6
                sa.port = host_to_net16(port_);
                sa.flowinfo = 0;
                auto b = addr_.to_v6().to_bytes();
                std::memcpy(sa.addr, b.data(), 16);
                sa.scope_id = addr_.to_v6().scope_id();
                r.size = 28;
                r.family = 10;
            }
            return r;
        }

        static basic_endpoint from_raw(const net::implements::raw_endpoint &r) noexcept {
            basic_endpoint ep;
            if (r.family == 2) { // AF_INET
                const auto &sa = *reinterpret_cast<const sockaddr_in_layout *>(r.data);
                ep.port_ = net_to_host16(sa.port);
                address_v4::bytes_type b{};
                std::memcpy(b.data(), sa.addr, 4);
                ep.addr_ = ip::address{address_v4{b}};
                ep.protocol_ = protocol_type::v4();
            } else { // AF_INET6
                const auto &sa = *reinterpret_cast<const sockaddr_in6_layout *>(r.data);
                ep.port_ = net_to_host16(sa.port);
                address_v6::bytes_type b{};
                std::memcpy(b.data(), sa.addr, 16);
                ep.addr_ = ip::address{address_v6{b, sa.scope_id}};
                ep.protocol_ = protocol_type::v6();
            }
            return ep;
        }

        void *data() noexcept {
            return raw_.data;
        }

        const void *data() const noexcept {
            return raw_.data;
        }

        constexpr std::size_t size() const noexcept {
            return addr_.is_v4() ? 16 : 28;
        }

        void resize(std::size_t) {
            // 固定大小，忽略
        }

        constexpr std::size_t capacity() const noexcept {
            return sizeof(raw_.data);
        }

    private:
        struct sockaddr_in_layout {
            std::uint16_t family;
            std::uint16_t port;
            std::uint8_t addr[4];
            std::uint8_t pad[8];
        };

        struct sockaddr_in6_layout {
            std::uint16_t family;
            std::uint16_t port;
            std::uint32_t flowinfo;
            std::uint8_t addr[16];
            std::uint32_t scope_id;
        };

        static std::uint16_t host_to_net16(std::uint16_t v) noexcept {
            // 大端序转换
            return static_cast<std::uint16_t>(((v & 0xFFu) << 8) | ((v >> 8) & 0xFFu));
        }

        static std::uint16_t net_to_host16(std::uint16_t v) noexcept {
            return host_to_net16(v); // 对称操作
        }

        protocol_type protocol_{};
        port_type port_{0};
        ip::address addr_{};
        net::implements::raw_endpoint raw_{};
    };

    template <typename InternetProtocol>
    constexpr bool operator==(const basic_endpoint<InternetProtocol> &l, const basic_endpoint<InternetProtocol> &r) noexcept {
        return l.address() == r.address() && l.port() == r.port();
    }

    template <typename InternetProtocol>
    constexpr bool operator!=(const basic_endpoint<InternetProtocol> &l, const basic_endpoint<InternetProtocol> &r) noexcept {
        return !(l == r);
    }

    template <typename InternetProtocol>
    constexpr bool operator<(const basic_endpoint<InternetProtocol> &l, const basic_endpoint<InternetProtocol> &r) noexcept {
        if (l.address() != r.address())
            return l.address() < r.address();
        return l.port() < r.port();
    }

    template <typename InternetProtocol>
    constexpr bool operator>(const basic_endpoint<InternetProtocol> &l, const basic_endpoint<InternetProtocol> &r) noexcept {
        return r < l;
    }

    template <typename InternetProtocol>
    constexpr bool operator<=(const basic_endpoint<InternetProtocol> &l, const basic_endpoint<InternetProtocol> &r) noexcept {
        return !(r < l);
    }

    template <typename InternetProtocol>
    constexpr bool operator>=(const basic_endpoint<InternetProtocol> &l, const basic_endpoint<InternetProtocol> &r) noexcept {
        return !(l < r);
    }

    template <typename CharT, typename Traits, typename InternetProtocol>
    std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, const basic_endpoint<InternetProtocol> &ep) {
        if (ep.address().is_v6()) {
            os << '[' << ep.address().to_string().c_str() << ']';
        } else {
            os << ep.address().to_string().c_str();
        }
        os << ':' << ep.port();
        return os;
    }

    // =========================================================================
    // basic_resolver_entry
    // =========================================================================
    template <typename InternetProtocol>
    class basic_resolver_entry {
    public:
        using protocol_type = InternetProtocol;
        using endpoint_type = typename InternetProtocol::endpoint;

        basic_resolver_entry() = default;

        basic_resolver_entry(const endpoint_type &ep, text::string_view h, text::string_view s) :
            endpoint_{ep}, host_{h.data(), h.size()}, service_{s.data(), s.size()} {
        }

        endpoint_type endpoint() const {
            return endpoint_;
        }

        operator endpoint_type() const {
            return endpoint_;
        }

        template <typename Allocator = std::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> host_name(const Allocator &alloc = Allocator()) const {
            return {host_.data(), host_.size(), alloc};
        }

        template <typename Allocator = std::allocator<char>>
        text::basic_string<char, text::char_traits<char>, Allocator> service_name(const Allocator &alloc = Allocator()) const {
            return {service_.data(), service_.size(), alloc};
        }

    private:
        endpoint_type endpoint_{};
        text::string host_{};
        text::string service_{};
    };

    // =========================================================================
    // basic_resolver_results
    // =========================================================================
    template <typename InternetProtocol>
    class basic_resolver_results {
    public:
        using protocol_type = InternetProtocol;
        using endpoint_type = typename protocol_type::endpoint;
        using value_type = basic_resolver_entry<protocol_type>;
        using const_reference = const value_type &;
        using reference = value_type &;
        using const_iterator = typename collections::vector<value_type>::const_iterator;
        using iterator = const_iterator;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        basic_resolver_results() = default;
        basic_resolver_results(const basic_resolver_results &) = default;
        basic_resolver_results(basic_resolver_results &&) noexcept = default;
        basic_resolver_results &operator=(const basic_resolver_results &) = default;
        basic_resolver_results &operator=(basic_resolver_results &&) noexcept = default;
        ~basic_resolver_results() = default;

        size_type size() const noexcept {
            return entries_.size();
        }

        size_type max_size() const noexcept {
            return entries_.max_size();
        }

        bool empty() const noexcept {
            return entries_.empty();
        }

        const_iterator begin() const {
            return entries_.begin();
        }
        const_iterator end() const {
            return entries_.end();
        }
        const_iterator cbegin() const {
            return entries_.cbegin();
        }
        const_iterator cend() const {
            return entries_.cend();
        }

        void swap(basic_resolver_results &other) noexcept {
            entries_.swap(other.entries_);
        }

        // 内部构造接口，供 basic_resolver 填充
        void push_back(const value_type &entry) {
            entries_.push_back(entry);
        }

    private:
        collections::vector<value_type> entries_;
    };

    template <typename InternetProtocol>
    bool operator==(const basic_resolver_results<InternetProtocol> &l, const basic_resolver_results<InternetProtocol> &r) {
        if (l.size() != r.size())
            return false;
        auto li = l.begin(), ri = r.begin();
        for (; li != l.end(); ++li, ++ri) {
            if (li->endpoint() != ri->endpoint())
                return false;
        }
        return true;
    }

    template <typename InternetProtocol>
    bool operator!=(const basic_resolver_results<InternetProtocol> &l, const basic_resolver_results<InternetProtocol> &r) {
        return !(l == r);
    }

    template <typename InternetProtocol>
    class basic_resolver {
    public:
        using executor_type = io_context::executor_type;
        using protocol_type = InternetProtocol;
        using endpoint_type = typename InternetProtocol::endpoint;
        using results_type = basic_resolver_results<InternetProtocol>;
        using flags = int; // resolver flags，与 Networking TS 一致

        explicit basic_resolver(io_context &ctx) : executor_{ctx.get_executor()} {
        }

        basic_resolver(const basic_resolver &) = delete;
        basic_resolver &operator=(const basic_resolver &) = delete;

        basic_resolver(basic_resolver &&other) noexcept :
            executor_{utility::move(other.executor_)}, cancelled_{other.cancelled_.load()} {
        }

        basic_resolver &operator=(basic_resolver &&other) noexcept {
            if (this != &other) {
                executor_ = utility::move(other.executor_);
                cancelled_ = other.cancelled_.load();
            }
            return *this;
        }

        ~basic_resolver() = default;

        executor_type get_executor() noexcept {
            return executor_;
        }

        void cancel() {
            cancelled_.store(true, std::memory_order_release);
        }

        // ------------------------------------------------------------------
        // 同步解析
        // ------------------------------------------------------------------
        results_type resolve(text::string_view host_name, text::string_view service_name) {
            std::error_code ec;
            auto r = resolve(host_name, service_name, ec);
            if (ec)
                throw std::system_error(ec, "resolve");
            return r;
        }

        results_type resolve(text::string_view host_name, text::string_view service_name, std::error_code &ec) {
            return resolve(host_name, service_name, 0, ec);
        }

        results_type resolve(text::string_view host_name, text::string_view service_name, flags f) {
            std::error_code ec;
            auto r = resolve(host_name, service_name, f, ec);
            if (ec)
                throw std::system_error(ec, "resolve");
            return r;
        }

        results_type resolve(text::string_view host_name, text::string_view service_name, flags f, std::error_code &ec) {
            return do_resolve(protocol_type{}, host_name, service_name, f, ec);
        }

        results_type resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name) {
            std::error_code ec;
            auto r = resolve(protocol, host_name, service_name, ec);
            if (ec)
                throw std::system_error(ec, "resolve");
            return r;
        }

        results_type resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name,
                             std::error_code &ec) {
            return do_resolve(protocol, host_name, service_name, 0, ec);
        }

        results_type resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name, flags f) {
            std::error_code ec;
            auto r = resolve(protocol, host_name, service_name, f, ec);
            if (ec)
                throw std::system_error(ec, "resolve");
            return r;
        }

        results_type resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name, flags f,
                             std::error_code &ec) {
            return do_resolve(protocol, host_name, service_name, f, ec);
        }

        results_type resolve(const endpoint_type &e) {
            std::error_code ec;
            auto r = resolve(e, ec);
            if (ec)
                throw std::system_error(ec, "resolve");
            return r;
        }

        results_type resolve(const endpoint_type &e, std::error_code &ec) {
            return do_reverse_resolve(e, ec);
        }

        // ------------------------------------------------------------------
        // 异步解析
        // ------------------------------------------------------------------
        template <typename CompletionToken>
        auto async_resolve(text::string_view host_name, text::string_view service_name, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, results_type)>::return_type {
            return async_resolve(protocol_type{}, host_name, service_name, 0, std::forward<CompletionToken>(token));
        }

        template <typename CompletionToken>
        auto async_resolve(text::string_view host_name, text::string_view service_name, flags f, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, results_type)>::return_type {
            return async_resolve(protocol_type{}, host_name, service_name, f, std::forward<CompletionToken>(token));
        }

        template <typename CompletionToken>
        auto async_resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name,
                           CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, results_type)>::return_type {
            return async_resolve(protocol, host_name, service_name, 0, std::forward<CompletionToken>(token));
        }

        template <typename CompletionToken>
        auto async_resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name, flags f,
                           CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, results_type)>::return_type {
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, results_type)> init(token);
            auto handler = utility::move(init.completion_handler);
            text::string host{host_name.data(), host_name.size()};
            text::string svc{service_name.data(), service_name.size()};

            executor_.post(
                [this, protocol, host = utility::move(host), svc = utility::move(svc), f, handler]() mutable {
                    std::error_code ec;
                    auto r = do_resolve(protocol, text::string_view{host}, text::string_view{svc}, f, ec);
                    handler(ec, utility::move(r));
                },
                std::allocator<void>{});

            return init.result.get();
        }

        template <typename CompletionToken>
        auto async_resolve(const endpoint_type &e, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, results_type)>::return_type {
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, results_type)> init(token);
            auto handler = utility::move(init.completion_handler);

            executor_.post(
                [this, e, handler]() mutable {
                    std::error_code ec;
                    auto r = do_reverse_resolve(e, ec);
                    handler(ec, utility::move(r));
                },
                std::allocator<void>{});

            return init.result.get();
        }

    private:
        results_type do_resolve(const protocol_type &protocol, text::string_view host_name, text::string_view service_name, flags f,
                                std::error_code &ec) {
            if (cancelled_.load(std::memory_order_acquire)) {
                ec = std::make_error_code(std::errc::operation_canceled);
                return {};
            }
            // getaddrinfo 通过 implements::resolve 封装，不直接调用平台 API
            text::string host{host_name.data(), host_name.size()};
            text::string svc{service_name.data(), service_name.size()};
            collections::vector<implements::resolved_entry> raw_results;
            ec = implements::resolve(host.c_str(), svc.c_str(), protocol.family(), protocol.type(), protocol.protocol(), f,
                                     raw_results);
            if (ec) {
                return {};
            }
            results_type results;
            for (auto &entry: raw_results) {
                net::implements::raw_endpoint raw;
                raw.family = entry.family;
                raw.size = entry.addr_size;
                std::memcpy(raw.data, entry.addr_data, (std::min) (entry.addr_size, sizeof(raw.data)));

                endpoint_type ep = endpoint_type::from_raw(raw);
                results.push_back(value_type{ep, text::string_view{entry.canonical_name}, svc});
            }
            return results;
        }

        results_type do_reverse_resolve(const endpoint_type &e, std::error_code &ec) {
            if (cancelled_.load(std::memory_order_acquire)) {
                ec = std::make_error_code(std::errc::operation_canceled);
                return {};
            }
            auto raw = e.to_raw();
            char host_buf[256]{};
            char svc_buf[64]{};
            ec = implements::reverse_resolve(raw.data, static_cast<int>(raw.size), host_buf, sizeof(host_buf), svc_buf,
                                             sizeof(svc_buf));
            if (ec) {
                return {};
            }
            results_type results;
            results.push_back(value_type{e, text::string_view{host_buf}, text::string_view{svc_buf}});
            return results;
        }

        using value_type = basic_resolver_entry<InternetProtocol>;

        executor_type executor_;
        std::atomic<bool> cancelled_{false};
    };
}

#endif
