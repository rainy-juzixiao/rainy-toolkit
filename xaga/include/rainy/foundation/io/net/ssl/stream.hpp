#ifndef RAINY_FOUNDATION_IO_NET_SSL_STREAM_HPP
#define RAINY_FOUNDATION_IO_NET_SSL_STREAM_HPP

#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/io/net/ssl/context.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>
#include <rainy/foundation/io/net/socket.hpp>

namespace rainy::foundation::io::net::ssl {
    template <typename NextLayer>
    class ssl_stream {
    public:
        using next_layer_type = type_traits::other_trans::decay_t<NextLayer>;
        using executor_type = typename next_layer_type::executor_type;
        using native_handle_type = void *;

        explicit ssl_stream(NextLayer &&next, context &ctx)
            : next_layer_(utility::forward<NextLayer>(next)), context_(ctx) {
            impl_ = implements::create_ssl_stream_impl();
            attach_socket();
        }

        explicit ssl_stream(NextLayer &next, context &ctx)
            : next_layer_(next), context_(ctx) {
            impl_ = implements::create_ssl_stream_impl();
            attach_socket();
        }

        ssl_stream(ssl_stream &&other) noexcept
            : next_layer_(utility::move(other.next_layer_)),
              context_(other.context_),
              impl_(utility::move(other.impl_)),
              server_name_(utility::move(other.server_name_)) {}

        ssl_stream &operator=(ssl_stream &&other) noexcept {
            if (this != &other) {
                next_layer_ = utility::move(other.next_layer_);
                context_ = utility::move(other.context_);
                impl_ = utility::move(other.impl_);
                server_name_ = utility::move(other.server_name_);
            }
            return *this;
        }

        next_layer_type &next_layer() noexcept {
            return next_layer_;
        }

        const next_layer_type &next_layer() const noexcept {
            return next_layer_;
        }

        executor_type get_executor() noexcept {
            return next_layer_.get_executor();
        }

        void set_server_name(const std::string &hostname) {
            server_name_ = hostname;
            if (impl_) {
                impl_->set_server_name(hostname.c_str());
            }
        }

        std::string server_name() const {
            return server_name_;
        }

        void handshake() {
            std::error_code ec;
            handshake(ec);
            if (ec) {
                throw std::system_error(ec, "SSL handshake failed");
            }
        }

        void handshake(std::error_code &ec) {
            if (!impl_) {
                ec = std::make_error_code(std::errc::invalid_argument);
                return;
            }

            if (!is_open()) {
                ec = std::make_error_code(std::errc::not_connected);
                return;
            }

            if (!context_.native_handle()) {
                ec = std::make_error_code(std::errc::invalid_argument);
                return;
            }

            if (auto err = impl_->apply_context(context_.get_params()); err) {
                ec = err;
                return;
            }

            if (!server_name_.empty()) {
                impl_->set_server_name(server_name_.c_str());
            }

            ec = impl_->handshake();
        }

        template <typename CompletionToken>
        auto async_handshake(CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code)>::return_type {

            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code)> init(token);
            auto handler = utility::move(init.completion_handler);

            // 先应用 context 和 SNI（这些是同步操作）
            if (!impl_ || !is_open() || !context_.native_handle()) {
                std::error_code ec = std::make_error_code(std::errc::not_connected);
                handler(ec);
                return init.result.get();
            }

            if (auto err = impl_->apply_context(context_.get_params()); err) {
                handler(err);
                return init.result.get();
            }

            if (!server_name_.empty()) {
                impl_->set_server_name(server_name_.c_str());
            }

            // 创建完成回调 op
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

            // 调用底层异步握手
            impl_->async_handshake(*next_layer_.get_executor().context().impl_, op);

            return init.result.get();
        }

        void shutdown() {
            std::error_code ec;
            shutdown(ec);
            if (ec) {
                throw std::system_error(ec, "SSL shutdown failed");
            }
        }

        void shutdown(std::error_code &ec) {
            if (!impl_ || !is_handshaked()) {
                ec = std::make_error_code(std::errc::not_connected);
                return;
            }
            ec = impl_->shutdown();
        }

        template <typename CompletionToken>
        auto async_shutdown(CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code)>::return_type {

            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code)> init(token);
            auto handler = utility::move(init.completion_handler);

            if (!impl_ || !is_handshaked()) {
                handler(std::make_error_code(std::errc::not_connected));
                return init.result.get();
            }

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

            impl_->async_shutdown(*next_layer_.get_executor().context().impl_, op);

            return init.result.get();
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers) {
            std::error_code ec;
            auto result = write_some(buffers, ec);
            if (ec) {
                throw std::system_error(ec, "SSL write failed");
            }
            return result;
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers, std::error_code &ec) {
            if (!impl_ || !is_handshaked()) {
                ec = std::make_error_code(std::errc::not_connected);
                return 0;
            }

            // 获取第一个 buffer（简化实现，实际应该处理 buffer 序列）
            auto buf = io::net::buffer(buffers);
            auto result = impl_->write_some(buf.data(), buf.size(), ec);

            if (ec) {
                return 0;
            }

            return static_cast<std::size_t>(result);
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers) {
            std::error_code ec;
            auto result = read_some(buffers, ec);
            if (ec) {
                throw std::system_error(ec, "SSL read failed");
            }
            return result;
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers, std::error_code &ec) {
            if (!impl_ || !is_handshaked()) {
                ec = std::make_error_code(std::errc::not_connected);
                return 0;
            }

            auto buf = io::net::buffer(buffers);
            auto result = impl_->read_some(buf.data(), buf.size(), ec);

            if (ec) {
                return 0;
            }

            return static_cast<std::size_t>(result);
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        auto async_write_some(const ConstBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {

            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);

            if (!impl_ || !is_handshaked()) {
                handler(std::make_error_code(std::errc::not_connected), 0);
                return init.result.get();
            }

            auto buf = io::net::buffer(buffers);

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

            impl_->async_write_some(buf.data(), buf.size(), *next_layer_.get_executor().context().impl_, op);

            return init.result.get();
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        auto async_read_some(const MutableBufferSequence &buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {

            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);

            if (!impl_ || !is_handshaked()) {
                handler(std::make_error_code(std::errc::not_connected), 0);
                return init.result.get();
            }

            auto buf = io::net::buffer(buffers);

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

            impl_->async_read_some(buf.data(), buf.size(), *next_layer_.get_executor().context().impl_, op);

            return init.result.get();
        }

        bool is_open() const noexcept {
            return impl_ && impl_->is_open();
        }

        bool is_handshaked() const noexcept {
            return impl_ && impl_->is_handshaked();
        }

        native_handle_type native_handle() noexcept {
            return impl_ ? impl_->native_handle() : nullptr;
        }

    private:
        void attach_socket() {
            if (impl_ && next_layer_.is_open()) {
                implements::native_socket_t sock = next_layer_.native_handle();
                impl_->attach(sock, false); // false = client mode
            }
        }

        next_layer_type next_layer_;
        context &context_;
        memory::nebula_ptr<implements::ssl_stream_impl_base> impl_;
        std::string server_name_;
    };
}

#endif