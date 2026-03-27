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
#ifndef RAINY_FOUNDATION_IO_NET_SOCKET_HPP
#define RAINY_FOUNDATION_IO_NET_SOCKET_HPP

#include <rainy/foundation/io/net/buffer.hpp>
#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>
#include <rainy/foundation/io/net/io_context.hpp>

namespace rainy::foundation::io::net::implements {
    // ABI
    static constexpr int sol_socket = 1;
    // SO_REUSEADDR: Windows=4, Linux=2
#if RAINY_USING_WINDOWS
    static constexpr int so_reuseaddr = 4;
#else
    static constexpr int so_reuseaddr = 2;
#endif
}

namespace rainy::foundation::io::net {
    enum class socket_errc : int {
        already_open = 1,
        not_found = 2
    };

    class socket_error_category : public std::error_category {
    public:
        RAINY_NODISCARD const char *name() const noexcept override {
            return "socket";
        }

        RAINY_NODISCARD std::string message(int ev) const override {
            switch (static_cast<socket_errc>(ev)) {
                case socket_errc::already_open:
                    return "socket already open";
                case socket_errc::not_found:
                    return "socket not found";
                default:
                    return "unknown socket error";
            }
        }
    };

    inline const std::error_category &socket_category() noexcept {
        static socket_error_category inst;
        return inst;
    }

    inline std::error_code make_error_code(socket_errc e) noexcept {
        return {static_cast<int>(e), socket_category()};
    }

    inline std::error_condition make_error_condition(socket_errc e) noexcept {
        return {static_cast<int>(e), socket_category()};
    }

    class socket_base {
    public:
        template <int Level, int Name, typename Ty>
        class socket_option_base {
        public:
            explicit socket_option_base(Ty val = Ty{}) : value_(val) {
            }

            RAINY_NODISCARD implements::socket_option to_raw() const noexcept {
                return {Level, Name, &value_, sizeof(Ty)};
            }

            implements::socket_option to_raw_mutable() noexcept {
                return {Level, Name, &value_, sizeof(Ty)};
            }

            Ty value() const noexcept {
                return value_;
            }

        protected:
            Ty value_;
        };

        using shutdown_type = int;
        using wait_type = int;
        using message_flags = int;

#if RAINY_USING_WINDOWS
        static constexpr shutdown_type shutdown_receive = 0;
        static constexpr shutdown_type shutdown_send = 1;
        static constexpr shutdown_type shutdown_both = 2;
        static constexpr message_flags message_peek = 0x2;
        static constexpr message_flags message_out_of_band = 0x1;
        static constexpr message_flags message_do_not_route = 0x4;
#else
        static constexpr shutdown_type shutdown_receive = 0;
        static constexpr shutdown_type shutdown_send = 1;
        static constexpr shutdown_type shutdown_both = 2;
        static constexpr message_flags message_peek = 0x2;
        static constexpr message_flags message_out_of_band = 0x1;
        static constexpr message_flags message_do_not_route = 0x4;
#endif


        static constexpr wait_type wait_read = 0;
        static constexpr wait_type wait_write = 1;
        static constexpr wait_type wait_error = 2;

        RAINY_TOOLKIT_API static const int max_listen_connections;

    protected:
        socket_base() = default;
        ~socket_base() = default;
    };

    template <typename Protocol>
    class basic_socket : public socket_base {
    public:
        using executor_type = io_context::executor_type;
        using native_handle_type = implements::native_socket_t;
        using protocol_type = Protocol;
        using endpoint_type = typename protocol_type::endpoint; // NOLINT

        basic_socket(const basic_socket &) = delete;
        basic_socket &operator=(const basic_socket &) = delete;

        executor_type get_executor() noexcept {
            return executor_;
        }

        native_handle_type native_handle() {
            if (!impl_) {
                return implements::invalid_socket_value;
            }
            return impl_->native_handle();
        }

        void open(const protocol_type &protocol = protocol_type()) {
            std::error_code ec;
            open(protocol, ec);
            if (ec) {
                throw std::system_error(ec, "open");
            }
        }

        void open(const protocol_type &protocol, std::error_code &ec) {
            ec = impl_->open(protocol.family(), protocol.type(), protocol.protocol());
            if (!ec) {
                protocol_ = protocol;
            }
        }

        void assign(const protocol_type &protocol, const native_handle_type &native_sock) {
            std::error_code ec;
            assign(protocol, native_sock, ec);
            if (ec) {
                throw std::system_error(ec, "assign");
            }
        }

        void assign(const protocol_type &protocol, const native_handle_type &native_sock, std::error_code &ec) {
            ec = impl_->assign(protocol.family(), protocol.type(), protocol.protocol(), native_sock);
            if (!ec) {
                protocol_ = protocol;
            }
        }

        native_handle_type release() {
            std::error_code ec;
            return release(ec);
        }

        native_handle_type release(std::error_code &ec) {
            ec.clear();
            return impl_->release();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            return impl_ && impl_->is_open();
        }

        void close() {
            std::error_code ec;
            close(ec);
            if (ec) {
                throw std::system_error(ec, "close");
            }
        }

        void close(std::error_code &ec) {
            ec = impl_->close();
        }

        void cancel() {
            std::error_code ec;
            cancel(ec);
            if (ec) {
                throw std::system_error(ec, "cancel");
            }
        }

        void cancel(std::error_code &ec) {
            ec = impl_->cancel();
        }

        template <typename SettableSocketOption>
        void set_option(const SettableSocketOption &option) {
            std::error_code ec;
            set_option(option, ec);
            if (ec) {
                throw std::system_error(ec, "set_option");
            }
        }

        template <typename SettableSocketOption>
        void set_option(const SettableSocketOption &option, std::error_code &ec) {
            ec = impl_->set_option(option.to_raw());
        }

        template <typename GettableSocketOption>
        void get_option(GettableSocketOption &option) const {
            std::error_code ec;
            get_option(option, ec);
            if (ec) {
                throw std::system_error(ec, "get_option");
            }
        }

        template <typename GettableSocketOption>
        void get_option(GettableSocketOption &option, std::error_code &ec) const {
            auto raw = option.to_raw_mutable();
            ec = impl_->get_option(raw);
        }

        template <typename IoControlCommand>
        void io_control(IoControlCommand &command) {
            std::error_code ec;
            io_control(command, ec);
            if (ec) {
                throw std::system_error(ec, "io_control");
            }
        }

        template <typename IoControlCommand>
        void io_control(IoControlCommand &command, std::error_code &ec) {
            ec = impl_->io_control(command.name(), command.data());
        }

        void non_blocking(const bool mode) {
            std::error_code ec;
            non_blocking(mode, ec);
            if (ec) {
                throw std::system_error(ec, "non_blocking");
            }
        }

        void non_blocking(const bool mode, std::error_code &ec) {
            ec = impl_->set_non_blocking(mode);
        }

        RAINY_NODISCARD bool non_blocking() const {
            return impl_->non_blocking();
        }

        void native_non_blocking(const bool mode) {
            std::error_code ec;
            native_non_blocking(mode, ec);
            if (ec) {
                throw std::system_error(ec, "native_non_blocking");
            }
        }

        void native_non_blocking(const bool mode, std::error_code &ec) {
            ec = impl_->set_native_non_blocking(mode);
        }

        RAINY_NODISCARD bool native_non_blocking() const {
            return impl_->native_non_blocking();
        }

        RAINY_NODISCARD bool at_mark() const {
            std::error_code ec;
            const bool r = at_mark(ec);
            if (ec) {
                throw std::system_error(ec, "at_mark");
            }
            return r;
        }

        bool at_mark(std::error_code &ec) const {
            return impl_->at_mark(ec);
        }

        RAINY_NODISCARD std::size_t available() const {
            std::error_code ec;
            const std::size_t n = available(ec);
            if (ec) {
                throw std::system_error(ec, "available");
            }
            return n;
        }

        std::size_t available(std::error_code &ec) const {
            return impl_->available(ec);
        }

        void bind(const endpoint_type &ep) {
            std::error_code ec;
            bind(ep, ec);
            if (ec) {
                throw std::system_error(ec, "bind");
            }
        }

        void bind(const endpoint_type &ep, std::error_code &ec) {
            ec = impl_->bind(ep.to_raw());
        }

        void shutdown(const shutdown_type what) {
            std::error_code ec;
            shutdown(what, ec);
            if (ec) {
                throw std::system_error(ec, "shutdown");
            }
        }

        void shutdown(shutdown_type what, std::error_code &ec) {
            ec = impl_->shutdown(static_cast<implements::shutdown_how>(what));
        }

        endpoint_type local_endpoint() const {
            std::error_code ec;
            auto ep = local_endpoint(ec);
            if (ec) {
                throw std::system_error(ec, "local_endpoint");
            }
            return ep;
        }

        endpoint_type local_endpoint(std::error_code &ec) const {
            implements::raw_endpoint raw;
            ec = impl_->local_endpoint(raw);
            return ec ? endpoint_type{} : endpoint_type::from_raw(raw);
        }

        endpoint_type remote_endpoint() const {
            std::error_code ec;
            auto ep = remote_endpoint(ec);
            if (ec) {
                throw std::system_error(ec, "remote_endpoint");
            }
            return ep;
        }

        endpoint_type remote_endpoint(std::error_code &ec) const {
            implements::raw_endpoint raw;
            ec = impl_->remote_endpoint(raw);
            return ec ? endpoint_type{} : endpoint_type::from_raw(raw);
        }

        void connect(const endpoint_type &ep) {
            std::error_code ec;
            connect(ep, ec);
            if (ec) {
                throw std::system_error(ec, "connect");
            }
        }

        void connect(const endpoint_type &ep, std::error_code &ec) {
            ec = impl_->connect(ep.to_raw());
        }

        template <typename CompletionToken>
        auto async_connect(const endpoint_type &ep, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto raw_ep = ep.to_raw();
            auto *op = implements::make_io_completion_op([handler](const implements::op_result &r, const bool cancelled) mutable {
                if (cancelled) {
                    return;
                }
                std::error_code ec;
                if (r.error_code) {
                    ec = std::error_code{r.error_code, std::system_category()};
                }
                handler(ec);
            });
            executor_.context().impl_->post_immediate_completion(op, false);
            return init.result.get();
        }

        void wait(const wait_type w) {
            std::error_code ec;
            wait(w, ec);
            if (ec) {
                throw std::system_error(ec, "wait");
            }
        }

        void wait(wait_type w, std::error_code &ec) { // NOLINT
            const auto wt = static_cast<implements::wait_type>(w);
            (void) wt;
            ec.clear();
        }

        template <typename CompletionToken>
        auto async_wait(wait_type w, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto *op = implements::make_io_completion_op([handler](const implements::op_result &r, const bool cancelled) mutable {
                if (cancelled) {
                    return;
                }
                std::error_code ec;
                if (r.error_code) {
                    ec = std::error_code{r.error_code, std::system_category()};
                }
                handler(ec);
            });
            auto &ctx_impl = *executor_.context().impl_;
            impl_->async_wait(static_cast<implements::wait_type>(w), ctx_impl, op);
            return init.result.get();
        }

    protected:
        explicit basic_socket(io_context &ctx) :
            executor_(ctx.get_executor()), protocol_(protocol_type::v4()), impl_(implements::create_socket_impl()) {
        }

        basic_socket(io_context &ctx, const protocol_type &protocol) :
            executor_(ctx.get_executor()), protocol_(protocol), impl_(implements::create_socket_impl()) {
            open(protocol);
        }

        basic_socket(io_context &ctx, const endpoint_type &ep) :
            executor_(ctx.get_executor()), protocol_(ep.protocol()), impl_(implements::create_socket_impl()) {
            open(ep.protocol());
            bind(ep);
        }

        basic_socket(io_context &ctx, const protocol_type &protocol, const native_handle_type &native_sock) :
            executor_(ctx.get_executor()), impl_(implements::create_socket_impl()) {
            assign(protocol, native_sock);
        }

        basic_socket(basic_socket &&right) noexcept :
            executor_(utility::move(right.executor_)), protocol_(right.protocol_), impl_(utility::move(right.impl_)) {
        }

        basic_socket &operator=(basic_socket &&right) noexcept {
            if (this != &right) {
                if (impl_ && impl_->is_open()) {
                    utility::ignore = impl_->close();
                }
                executor_ = utility::move(right.executor_);
                protocol_ = right.protocol_;
                impl_ = utility::move(right.impl_);
            }
            return *this;
        }

        template <typename OtherProtocol>
        basic_socket(basic_socket<OtherProtocol> &&right) noexcept : // NOLINT
            executor_(utility::move(right.executor_)), impl_(utility::move(right.impl_)) {
        }

        template <typename OtherProtocol>
        basic_socket &operator=(basic_socket<OtherProtocol> &&right) noexcept {
            close();
            executor_ = utility::move(right.executor_);
            impl_ = utility::move(right.impl_);
            return *this;
        }

        ~basic_socket() {
            if (impl_ && impl_->is_open()) {
                utility::ignore = impl_->close();
            }
        }

        executor_type executor_;
        protocol_type protocol_;
        memory::nebula_ptr<implements::socket_impl_base> impl_;
    };

    template <typename Protocol>
    class basic_datagram_socket : public basic_socket<Protocol> {
    public:
        using base = basic_socket<Protocol>;

        using native_handle_type = typename base::native_handle_type; // NOLINT
        using protocol_type = Protocol;
        using endpoint_type = typename protocol_type::endpoint; // NOLINT

        explicit basic_datagram_socket(io_context &ctx) : base(ctx) {
        }

        basic_datagram_socket(io_context &ctx, const protocol_type &p) : base(ctx, p) {
        }

        basic_datagram_socket(io_context &ctx, const endpoint_type &ep) : base(ctx, ep) {
        }

        basic_datagram_socket(io_context &ctx, const protocol_type &p, const native_handle_type &n) : base(ctx, p, n) {
        }

        basic_datagram_socket(const basic_datagram_socket &) = delete;
        basic_datagram_socket &operator=(const basic_datagram_socket &) = delete;
        basic_datagram_socket(basic_datagram_socket &&) = default;
        basic_datagram_socket &operator=(basic_datagram_socket &&) = default;
        ~basic_datagram_socket() = default;

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers) {
            std::error_code ec;
            const std::size_t n = receive(buffers, ec);
            if (ec) {
                throw std::system_error(ec, "receive");
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers, std::error_code &ec) {
            return receive(buffers, 0, ec);
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers, socket_base::message_flags flags) {
            std::error_code ec;
            const std::size_t n = receive(buffers, flags, ec);
            if (ec) {
                throw std::system_error(ec, "receive");
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers, socket_base::message_flags flags, std::error_code &ec) {
            auto mb = io::net::buffer(buffers);
            auto r = this->impl_->receive(mb.data(), mb.size(), flags, ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_receive(const MutableBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_receive(buffers, 0, std::forward<CompletionToken>(token));
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_receive(const MutableBufferSequence &buffers, socket_base::message_flags flags, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::net::buffer(buffers);
            auto *op = implements::make_io_completion_op([handler](const implements::op_result &r, const bool cancelled) mutable {
                if (cancelled) {
                    return;
                }
                std::error_code ec;
                if (r.error_code) {
                    ec = std::error_code{r.error_code, std::system_category()};
                }
                handler(ec, r.bytes_transferred);
            });
            this->impl_->async_receive(mb.data(), mb.size(), flags, *this->executor_.context().impl_, op);
            return init.result.get();
        }

        template <typename MutableBufferSequence>
        std::size_t receive_from(const MutableBufferSequence &buffers, endpoint_type &sender) {
            std::error_code ec;
            const std::size_t n = receive_from(buffers, sender, ec);
            if (ec) {
                throw std::system_error(ec, "receive_from");
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t receive_from(const MutableBufferSequence &buffers, endpoint_type &sender, std::error_code &ec) {
            return receive_from(buffers, sender, 0, ec);
        }

        template <typename MutableBufferSequence>
        std::size_t receive_from(const MutableBufferSequence &buffers, endpoint_type &sender, socket_base::message_flags flags) {
            std::error_code ec;
            const std::size_t n = receive_from(buffers, sender, flags, ec);
            if (ec) {
                throw std::system_error(ec, "receive_from");
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t receive_from(const MutableBufferSequence &buffers, endpoint_type &sender, socket_base::message_flags flags,
                                 std::error_code &ec) {
            auto mb = net::buffer(buffers);
            implements::raw_endpoint raw_sender;
            auto r = this->impl_->receive_from(mb.data(), mb.size(), flags, raw_sender, ec);
            if (!ec) {
                sender = endpoint_type::from_raw(raw_sender);
            }
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_receive_from(const MutableBufferSequence &buffers, endpoint_type &sender, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_receive_from(buffers, sender, 0, std::forward<CompletionToken>(token));
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_receive_from(const MutableBufferSequence &buffers, endpoint_type &sender, socket_base::message_flags flags,
                                CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::net::buffer(buffers);

            auto raw_sender = std::make_shared<implements::raw_endpoint>();
            auto *op = implements::make_function_op([this, mb, flags, &sender, raw_sender, handler]() mutable {
                std::error_code ec;
                auto n = this->impl_->receive_from(mb.data(), mb.size(), flags, *raw_sender, ec);
                if (!ec) {
                    sender = endpoint_type::from_raw(*raw_sender);
                }
                handler(ec, n < 0 ? 0 : static_cast<std::size_t>(n));
            });
            this->impl_->async_receive_from(mb.data(), mb.size(), flags, *raw_sender, *this->executor_.context().impl_, op);
            return init.result.get();
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers) {
            std::error_code ec;
            const std::size_t n = send(buffers, ec);
            if (ec) {
                throw std::system_error(ec, "send");
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers, std::error_code &ec) {
            return send(buffers, 0, ec);
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers, socket_base::message_flags flags) {
            std::error_code ec;
            const std::size_t n = send(buffers, flags, ec);
            if (ec) {
                throw std::system_error(ec, "send");
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers, socket_base::message_flags flags, std::error_code &ec) {
            auto cb = net::buffer(buffers);
            auto r = this->impl_->send(cb.data(), cb.size(), flags, ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_send(const ConstBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_send(buffers, 0, std::forward<CompletionToken>(token));
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_send(const ConstBufferSequence &buffers, socket_base::message_flags flags, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto cb = io::net::buffer(buffers);
            auto *op = implements::make_io_completion_op([handler](const implements::op_result &r, const bool cancelled) mutable {
                if (cancelled) {
                    return;
                }
                std::error_code ec;
                if (r.error_code) {
                    ec = std::error_code{r.error_code, std::system_category()};
                }
                handler(ec, r.bytes_transferred);
            });
            this->impl_->async_send(cb.data(), cb.size(), flags, *this->executor_.context().impl_, op);
            return init.result.get();
        }

        template <typename ConstBufferSequence>
        std::size_t send_to(const ConstBufferSequence &buffers, const endpoint_type &recipient) {
            std::error_code ec;
            const std::size_t n = send_to(buffers, recipient, ec);
            if (ec) {
                throw std::system_error(ec, "send_to");
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t send_to(const ConstBufferSequence &buffers, const endpoint_type &recipient, std::error_code &ec) {
            return send_to(buffers, recipient, 0, ec);
        }

        template <typename ConstBufferSequence>
        std::size_t send_to(const ConstBufferSequence &buffers, const endpoint_type &recipient, socket_base::message_flags flags) {
            std::error_code ec;
            const std::size_t n = send_to(buffers, recipient, flags, ec);
            if (ec) {
                throw std::system_error(ec, "send_to");
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t send_to(const ConstBufferSequence &buffers, const endpoint_type &recipient, socket_base::message_flags flags,
                            std::error_code &ec) {
            auto cb = io::net::buffer(buffers);
            auto raw = recipient.to_raw();
            auto r = this->impl_->send_to(cb.data(), cb.size(), flags, raw, ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_send_to(const ConstBufferSequence &buffers, const endpoint_type &recipient, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_send_to(buffers, recipient, 0, std::forward<CompletionToken>(token));
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_send_to(const ConstBufferSequence &buffers, const endpoint_type &recipient, socket_base::message_flags flags,
                           CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto cb = io::net::buffer(buffers);
            auto raw_dest = recipient.to_raw();

            auto *op = implements::make_function_op([this, cb, flags, raw_dest, handler]() mutable {
                std::error_code ec;
                auto n = this->impl_->send_to(cb.data(), cb.size(), flags, raw_dest, ec);
                handler(ec, n < 0 ? 0 : static_cast<std::size_t>(n));
            });
            this->impl_->async_send_to(cb.data(), cb.size(), flags, raw_dest, *this->executor_.context().impl_, op);
            return init.result.get();
        }
    };

    template <typename Protocol>
    class basic_stream_socket : public basic_socket<Protocol> {
    public:
        using base = basic_socket<Protocol>;

        using native_handle_type = typename base::native_handle_type; // NOLINT
        using protocol_type = Protocol;
        using endpoint_type = typename protocol_type::endpoint; // NOLINT

        explicit basic_stream_socket(io_context &ctx) : base(ctx) {
        }
        basic_stream_socket(io_context &ctx, const protocol_type &p) : base(ctx, p) {
        }
        basic_stream_socket(io_context &ctx, const endpoint_type &ep) : base(ctx, ep) {
        }
        basic_stream_socket(io_context &ctx, const protocol_type &p, const native_handle_type &n) : base(ctx, p, n) {
        }
        basic_stream_socket(const basic_stream_socket &) = delete;
        basic_stream_socket &operator=(const basic_stream_socket &) = delete;
        basic_stream_socket(basic_stream_socket &&) = default;
        basic_stream_socket &operator=(basic_stream_socket &&) = default;
        ~basic_stream_socket() = default;

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers) {
            std::error_code ec;
            const std::size_t n = receive(buffers, ec);
            if (ec) {
                throw std::system_error(ec, "receive");
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers, std::error_code &ec) {
            return receive(buffers, 0, ec);
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers, socket_base::message_flags flags) {
            std::error_code ec;
            const std::size_t n = receive(buffers, flags, ec);
            if (ec) {
                throw std::system_error(ec, "receive");
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence &buffers, socket_base::message_flags flags, std::error_code &ec) {
            auto mb = net::buffer(buffers);
            auto r = this->impl_->receive(mb.data(), mb.size(), flags, ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers) {
            return receive(buffers);
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers, std::error_code &ec) {
            return receive(buffers, ec);
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers) {
            std::error_code ec;
            const std::size_t n = send(buffers, ec);
            if (ec) {
                throw std::system_error(ec, "send");
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers, std::error_code &ec) {
            return send(buffers, 0, ec);
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers, socket_base::message_flags flags) {
            std::error_code ec;
            const std::size_t n = send(buffers, flags, ec);
            if (ec) {
                throw std::system_error(ec, "send");
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence &buffers, socket_base::message_flags flags, std::error_code &ec) {
            auto cb = io::net::buffer(buffers);
            auto r = this->impl_->send(cb.data(), cb.size(), flags, ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers) {
            return send(buffers);
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers, std::error_code &ec) {
            return send(buffers, ec);
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_receive(const MutableBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_receive(buffers, 0, std::forward<CompletionToken>(token));
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_receive(const MutableBufferSequence &buffers, socket_base::message_flags flags, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::net::buffer(buffers);
            auto *op = implements::make_io_completion_op([handler](const implements::op_result &r, const bool cancelled) mutable {
                if (cancelled) {
                    return;
                }
                std::error_code ec;
                if (r.error_code) {
                    ec = std::error_code{r.error_code, std::system_category()};
                }
                handler(ec, r.bytes_transferred);
            });
            this->impl_->async_receive(mb.data(), mb.size(), flags, *this->executor_.context().impl_, op);
            return init.result.get();
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_read_some(const MutableBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_receive(buffers, std::forward<CompletionToken>(token));
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_send(const ConstBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_send(buffers, 0, std::forward<CompletionToken>(token));
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_send(const ConstBufferSequence &buffers, socket_base::message_flags flags, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto cb = net::buffer(buffers);
            auto *op = implements::make_function_op([handler](const implements::op_result &r) mutable {
                std::error_code ec;
                if (r.error_code) {
                    ec = std::error_code{r.error_code, std::system_category()};
                }
                handler(ec, r.bytes_transferred);
            });
            this->impl_->async_send(cb.data(), cb.size(), flags, *this->executor_.context().impl_, op);
            return init.result.get();
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_write_some(const ConstBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            return async_send(buffers, std::forward<CompletionToken>(token));
        }
    };

    template <typename AcceptableProtocol>
    class basic_socket_acceptor : public socket_base {
    public:
        using executor_type = io_context::executor_type;
        using native_handle_type = implements::native_socket_t;
        using protocol_type = AcceptableProtocol;
        using endpoint_type = typename protocol_type::endpoint; // NOLINT
        using socket_type = typename protocol_type::socket; // NOLINT

        explicit basic_socket_acceptor(io_context &ctx) :
            executor_(ctx.get_executor()), protocol_(protocol_type::v4()), impl_(implements::create_socket_impl()) {
        }

        basic_socket_acceptor(io_context &ctx, const protocol_type &protocol) :
            executor_(ctx.get_executor()), impl_(implements::create_socket_impl()) {
            open(protocol);
        }

        basic_socket_acceptor(io_context &ctx, const endpoint_type &ep,const bool reuse_addr = true) :
            executor_(ctx.get_executor()), protocol_(protocol_type::v4()), impl_(implements::create_socket_impl()) {
            open(ep.protocol());
            if (reuse_addr) {
                int opt = 1;
                const implements::socket_option raw{implements::sol_socket, implements::so_reuseaddr, &opt, sizeof(opt)}; // NOLINT
                utility::ignore = impl_->set_option(raw);
            }
            utility::ignore = impl_->bind(ep.to_raw());
            utility::ignore = impl_->listen(max_listen_connections);
        }

        basic_socket_acceptor(const basic_socket_acceptor &) = delete;
        basic_socket_acceptor &operator=(const basic_socket_acceptor &) = delete;

        basic_socket_acceptor(basic_socket_acceptor &&right) noexcept :
            executor_(utility::move(right.executor_)), protocol_(right.protocol_), impl_(utility::move(right.impl_)) {
        }

        basic_socket_acceptor &operator=(basic_socket_acceptor &&right) noexcept {
            if (this != &right) {
                if (impl_ && impl_->is_open()) {
                    utility::ignore = impl_->close();
                }
                executor_ = utility::move(right.executor_);
                protocol_ = right.protocol_;
                impl_ = utility::move(right.impl_);
            }
            return *this;
        }

        ~basic_socket_acceptor() {
            if (impl_ && impl_->is_open()) {
                utility::ignore = impl_->close();
            }
        }

        executor_type get_executor() noexcept {
            return executor_;
        }

        native_handle_type native_handle() {
            return impl_->native_handle();
        }

        void open(const protocol_type &p = protocol_type()) {
            std::error_code ec;
            open(p, ec);
            if (ec) {
                throw std::system_error(ec, "acceptor::open");
            }
        }

        void open(const protocol_type &p, std::error_code &ec) {
            ec = impl_->open(p.family(), p.type(), p.protocol());
            if (!ec) {
                protocol_ = p;
            }
        }

        void assign(const protocol_type &p, const native_handle_type &n) {
            std::error_code ec;
            assign(p, n, ec);
            if (ec) {
                throw std::system_error(ec, "acceptor::assign");
            }
        }

        void assign(const protocol_type &p, const native_handle_type &n, std::error_code &ec) {
            ec = impl_->assign(p.family(), p.type(), p.protocol(), n);
            if (!ec) {
                protocol_ = p;
            }
        }

        native_handle_type release() {
            std::error_code ec;
            return release(ec);
        }
        native_handle_type release(std::error_code &ec) {
            ec.clear();
            return impl_->release();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            return impl_ && impl_->is_open();
        }

        void close() {
            std::error_code ec;
            close(ec);
            if (ec) {
                throw std::system_error(ec, "acceptor::close");
            }
        }
        void close(std::error_code &ec) {
            ec = impl_->close();
        }

        void cancel() {
            std::error_code ec;
            cancel(ec);
            if (ec) {
                throw std::system_error(ec, "acceptor::cancel");
            }
        }

        void cancel(std::error_code &ec) {
            if (!impl_) {
                ec.clear();
                return;
            }
            ec = impl_->cancel();
        }

        template <typename SettableSocketOption>
        void set_option(const SettableSocketOption &opt) {
            std::error_code ec;
            set_option(opt, ec);
            if (ec) {
                throw std::system_error(ec, "set_option");
            }
        }
        template <typename SettableSocketOption>
        void set_option(const SettableSocketOption &opt, std::error_code &ec) {
            ec = impl_->set_option(opt.to_raw());
        }

        template <typename GettableSocketOption>
        void get_option(GettableSocketOption &opt) const {
            std::error_code ec;
            get_option(opt, ec);
            if (ec) {
                throw std::system_error(ec, "get_option");
            }
        }
        template <typename GettableSocketOption>
        void get_option(GettableSocketOption &opt, std::error_code &ec) const {
            auto raw = opt.to_raw_mutable();
            ec = impl_->get_option(raw);
        }

        void non_blocking(const bool m) {
            std::error_code ec;
            non_blocking(m, ec);
            if (ec) {
                throw std::system_error(ec, "non_blocking");
            }
        }
        void non_blocking(const bool m, std::error_code &ec) {
            ec = impl_->set_non_blocking(m);
        }

        RAINY_NODISCARD bool non_blocking() const {
            return impl_->non_blocking();
        }

        void native_non_blocking(const bool m) {
            std::error_code ec;
            native_non_blocking(m, ec);
            if (ec) {
                throw std::system_error(ec, "native_non_blocking");
            }
        }
        void native_non_blocking(const bool m, std::error_code &ec) {
            ec = impl_->set_native_non_blocking(m);
        }

        RAINY_NODISCARD bool native_non_blocking() const {
            return impl_->native_non_blocking();
        }

        void bind(const endpoint_type &ep) {
            std::error_code ec;
            bind(ep, ec);
            if (ec) {
                throw std::system_error(ec, "acceptor::bind");
            }
        }
        void bind(const endpoint_type &ep, std::error_code &ec) {
            ec = impl_->bind(ep.to_raw());
        }

        void listen(const int backlog = max_listen_connections) {
            std::error_code ec;
            listen(backlog, ec);
            if (ec) {
                throw std::system_error(ec, "listen");
            }
        }

        void listen(const int backlog, std::error_code &ec) {
            ec = impl_->listen(backlog);
        }

        endpoint_type local_endpoint() const {
            std::error_code ec;
            auto ep = local_endpoint(ec);
            if (ec) {
                throw std::system_error(ec, "local_endpoint");
            }
            return ep;
        }

        endpoint_type local_endpoint(std::error_code &ec) const {
            implements::raw_endpoint raw;
            ec = impl_->local_endpoint(raw);
            return ec ? endpoint_type{} : endpoint_type::from_raw(raw);
        }

        void enable_connection_aborted(const bool mode) {
            enable_conn_aborted_ = mode;
        }

        RAINY_NODISCARD bool enable_connection_aborted() const {
            return enable_conn_aborted_;
        }

        socket_type accept() {
            std::error_code ec;
            socket_type s = accept(ec);
            if (ec) {
                throw std::system_error(ec, "accept");
            }
            return s;
        }

        socket_type accept(std::error_code &ec) {
            implements::raw_endpoint peer;
            auto fd = impl_->accept(&peer, ec);
            if (ec) {
                return socket_type{executor_.context()};
            }
            socket_type s{executor_.context()};
            s.assign(protocol_, fd, ec);
            return s;
        }

        socket_type accept(io_context &ctx) {
            std::error_code ec;
            socket_type s = accept(ctx, ec);
            if (ec) {
                throw std::system_error(ec, "accept");
            }
            return s;
        }

        socket_type accept(io_context &ctx, std::error_code &ec) {
            implements::raw_endpoint peer;
            auto fd = impl_->accept(&peer, ec);
            if (ec) {
                return socket_type{ctx};
            }
            socket_type s{ctx};
            s.assign(protocol_, fd, ec);
            return s;
        }

        socket_type accept(endpoint_type &ep) {
            std::error_code ec;
            socket_type s = accept(ep, ec);
            if (ec) {
                throw std::system_error(ec, "accept");
            }
            return s;
        }

        socket_type accept(endpoint_type &ep, std::error_code &ec) {
            implements::raw_endpoint peer;
            auto fd = impl_->accept(&peer, ec);
            if (!ec) {
                ep = endpoint_type::from_raw(peer);
            }
            if (ec) {
                return socket_type{executor_.context()};
            }
            socket_type s{executor_.context()};
            s.assign(protocol_, fd, ec);
            return s;
        }

        socket_type accept(io_context &ctx, endpoint_type &ep) {
            std::error_code ec;
            socket_type s = accept(ctx, ep, ec);
            if (ec) {
                throw std::system_error(ec, "accept");
            }
            return s;
        }

        socket_type accept(io_context &ctx, endpoint_type &ep, std::error_code &ec) {
            implements::raw_endpoint peer;
            auto fd = impl_->accept(&peer, ec);
            if (!ec) {
                ep = endpoint_type::from_raw(peer);
            }
            if (ec) {
                return socket_type{ctx};
            }
            socket_type s{ctx};
            s.assign(protocol_, fd, ec);
            return s;
        }

        template <typename CompletionToken>
        auto async_accept(CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, socket_type)>::return_type { // NOLINTs
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, socket_type)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto raw_peer = std::make_shared<implements::raw_endpoint>();
            auto executor = executor_;
            auto protocol = protocol_;
            executor.on_work_started();
            auto *op = implements::make_io_completion_op(
                [this, raw_peer, handler, executor, protocol](const implements::op_result &r, const bool cancelled) mutable {
                    executor.on_work_finished();
                    if (cancelled) {
                        return;
                    }
                    std::error_code ec;
                    if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    socket_type s{executor.context()};
                    if (!ec) {
                        auto fd = static_cast<implements::native_socket_t>(r.bytes_transferred);
                        s.assign(protocol, fd, ec);
                    }
                    handler(ec, utility::move(s));
                    utility::ignore = raw_peer;
                });
            impl_->async_accept(raw_peer.get(), *executor_.context().impl_, op);
            return init.result.get();
        }

        template <typename CompletionToken>
        auto async_accept(io_context &ctx, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, socket_type)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, socket_type)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto raw_peer = std::make_shared<implements::raw_endpoint>();
            auto *op = implements::make_function_op([this, &ctx, raw_peer, handler]() mutable {
                std::error_code ec;
                auto fd = this->impl_->accept(raw_peer.get(), ec);
                socket_type s{ctx};
                if (!ec) {
                    s.assign(this->protocol_, fd, ec);
                }
                handler(ec, utility::move(s));
            });
            impl_->async_accept(raw_peer, *executor_.context().impl_, op);
            return init.result.get();
        }

        void wait(wait_type w) { // NOLINT
            std::error_code ec;
            wait(w, ec);
            if (ec) {
                throw std::system_error(ec, "wait");
            }
        }

        void wait(wait_type w, std::error_code &ec) { // NOLINT
            ec.clear();
            (void) w;
        }

        template <typename CompletionToken>
        auto async_wait(wait_type w, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto *op = implements::make_function_op([handler]() mutable { handler(std::error_code{}); });
            impl_->async_wait(static_cast<implements::wait_type>(w), *executor_.context().impl_, op);
            return init.result.get();
        }

    private:
        executor_type executor_;
        protocol_type protocol_;
        memory::nebula_ptr<implements::socket_impl_base> impl_;
        bool enable_conn_aborted_{false};
    };
}

namespace std { // NOLINT
    template <>
    struct is_error_code_enum<rainy::foundation::io::net::socket_errc> : true_type {};
}

#endif
