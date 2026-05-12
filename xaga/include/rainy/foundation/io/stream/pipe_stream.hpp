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
#ifndef RAINY_IO_STREAM_PIPE_STREAM_HPP
#define RAINY_IO_STREAM_PIPE_STREAM_HPP

#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>

namespace rainy::foundation::io::stream {
    class RAINY_TOOLKIT_API pipe_end_base {
    public:
        friend class named_pipe_stream;
        friend class unnamed_pipe_stream;

        using executor_type = io_context::executor_type;

        pipe_end_base(const pipe_end_base &) = delete;
        pipe_end_base &operator=(const pipe_end_base &) = delete;

        pipe_end_base(pipe_end_base &&other) noexcept : executor_(other.executor_), impl_(utility::move(other.impl_)) {
        }

        pipe_end_base &operator=(pipe_end_base &&other) noexcept {
            if (this != &other) {
                close_if_open();
                executor_ = other.executor_;
                impl_ = utility::move(other.impl_);
            }
            return *this;
        }

        ~pipe_end_base() {
            close_if_open();
        }

        bool is_open() const noexcept {
            return impl_ && impl_->is_open();
        }

        std::error_code close() noexcept {
            if (!impl_) {
                return {};
            }
            return impl_->close();
        }

        std::error_code cancel() noexcept {
            if (!impl_) {
                return {};
            }
            return impl_->cancel();
        }

        executor_type get_executor() const noexcept {
            return executor_;
        }

        native_handle_type native_handle() const noexcept {
#if RAINY_USING_WINDOWS
            return impl_ ? impl_->native_handle() : reinterpret_cast<native_handle_type>(-1);
#else
            return impl_ ? impl_->native_handle() : static_cast<native_handle_type>(-1);
#endif
        }

    protected:
        explicit pipe_end_base(executor_type ex, memory::nebula_ptr<implements::descriptor_impl_base> impl) noexcept :
            executor_(ex), impl_(utility::move(impl)) {
        }

        void close_if_open() noexcept {
            if (impl_ && impl_->is_open()) {
                utility::ignore = impl_->close();
            }
        }

        executor_type executor_;
        memory::nebula_ptr<implements::descriptor_impl_base> impl_;
    };

    class RAINY_TOOLKIT_API unnamed_pipe_stream {
    public:
        using executor_type = io_context::executor_type;

        class RAINY_TOOLKIT_API read_end final : public pipe_end_base {
        public:
            using pipe_end_base::pipe_end_base;

            template <typename MutableBufferSequence>
            std::size_t read_some(const MutableBufferSequence &buffers) {
                std::error_code ec;
                std::size_t n = read_some(buffers, ec);
                if (ec) {
                    throw exceptions::runtime::system_error(ec);
                }
                return n;
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(const MutableBufferSequence &buffers, std::error_code &ec) {
                auto mb = io::buffer(buffers);
                auto r = impl_->read_some(mb.data(), mb.size(), ec);
                return r < 0 ? 0 : static_cast<std::size_t>(r);
            }

            template <typename MutableBufferSequence, typename CompletionToken>
            rain_fn async_read_some(MutableBufferSequence buffers, CompletionToken &&token) ->
                typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
                using token_t = std::decay_t<CompletionToken>;
                async_completion<token_t, void(std::error_code, std::size_t)> init(token);
                auto handler = utility::move(init.completion_handler);
                auto mb = io::buffer(buffers);
                auto *op = io::implements::make_executor_completion_op(
                    [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                        if (cancelled) {
                            return;
                        }
                        std::error_code ec;
                        if (r.error_code) {
                            ec = std::error_code{r.error_code, std::system_category()};
                        }
                        handler(ec, r.bytes_transferred);
                    },
                    executor_);
                impl_->async_read_some(mb.data(), mb.size(), executor_, op);
                return init.result.get();
            }
        };

        class RAINY_TOOLKIT_API write_end final : public pipe_end_base {
        public:
            using pipe_end_base::pipe_end_base;

            template <typename ConstBufferSequence>
            std::size_t write_some(const ConstBufferSequence &buffers) {
                std::error_code ec;
                std::size_t n = write_some(buffers, ec);
                if (ec) {
                    throw exceptions::runtime::system_error(ec);
                }
                return n;
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(const ConstBufferSequence &buffers, std::error_code &ec) {
                auto cb = io::buffer(buffers);
                auto r = impl_->write_some(cb.data(), cb.size(), ec);
                return r < 0 ? 0 : static_cast<std::size_t>(r);
            }

            template <typename ConstBufferSequence, typename CompletionToken>
            rain_fn async_write_some(ConstBufferSequence buffers, CompletionToken &&token) ->
                typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
                using token_t = std::decay_t<CompletionToken>;
                async_completion<token_t, void(std::error_code, std::size_t)> init(token);
                auto handler = utility::move(init.completion_handler);
                auto cb = io::buffer(buffers);
                auto *op = io::implements::make_executor_completion_op(
                    [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                        if (cancelled) {
                            return;
                        }
                        std::error_code ec;
                        if (r.error_code) {
                            ec = std::error_code{r.error_code, std::system_category()};
                        }
                        handler(ec, r.bytes_transferred);
                    },
                    executor_);
                impl_->async_write_some(cb.data(), cb.size(), executor_, op);
                return init.result.get();
            }
        };

        unnamed_pipe_stream(const unnamed_pipe_stream &) = delete;
        unnamed_pipe_stream &operator=(const unnamed_pipe_stream &) = delete;

        unnamed_pipe_stream(unnamed_pipe_stream &&other) noexcept :
            read_(utility::move(other.read_)), write_(utility::move(other.write_)) {
        }

        unnamed_pipe_stream &operator=(unnamed_pipe_stream &&other) noexcept {
            if (this != &other) {
                read_ = utility::move(other.read_);
                write_ = utility::move(other.write_);
            }
            return *this;
        }

        ~unnamed_pipe_stream() {
        }

        static unnamed_pipe_stream create(executor_type executor);

        static unnamed_pipe_stream create(executor_type executor, std::error_code &ec);

        read_end &get_read_end() noexcept {
            return read_;
        }

        const read_end &get_read_end() const noexcept {
            return read_;
        }

        write_end &get_write_end() noexcept {
            return write_;
        }

        const write_end &get_write_end() const noexcept {
            return write_;
        }

        read_end take_read_end() noexcept {
            return utility::move(read_);
        }

        write_end take_write_end() noexcept {
            return utility::move(write_);
        }

        bool is_read_open() const noexcept {
            return read_.is_open();
        }

        bool is_write_open() const noexcept {
            return write_.is_open();
        }

        bool is_open() const noexcept {
            return read_.is_open() && write_.is_open();
        }

        std::error_code close_read() noexcept {
            return read_.close();
        }

        std::error_code close_write() noexcept {
            return write_.close();
        }

        std::error_code close() noexcept {
            std::error_code ec;
            ec = read_.close();
            if (auto ec2 = write_.close(); !ec) {
                ec = ec2;
            }
            return ec;
        }

    private:
        explicit unnamed_pipe_stream(read_end r, write_end w) noexcept : read_(utility::move(r)), write_(utility::move(w)) {
        }

        read_end read_;
        write_end write_;
    };

    class RAINY_TOOLKIT_API named_pipe_stream {
    public:
        using executor_type = io_context::executor_type;

        named_pipe_stream(const named_pipe_stream &) = delete;
        named_pipe_stream &operator=(const named_pipe_stream &) = delete;

        named_pipe_stream(named_pipe_stream &&other) noexcept : executor_(other.executor_), impl_(utility::move(other.impl_)) {
        }

        named_pipe_stream &operator=(named_pipe_stream &&other) noexcept {
            if (this != &other) {
                close_if_open();
                executor_ = other.executor_;
                impl_ = utility::move(other.impl_);
            }
            return *this;
        }

        ~named_pipe_stream() {
            close_if_open();
        }

        static named_pipe_stream open_server(executor_type executor, foundation::text::string_view name, pipe_direction dir = pipe_direction::inout);

        static named_pipe_stream open_server(executor_type executor, foundation::text::string_view name, pipe_direction dir,
                                             std::error_code &ec);

        static named_pipe_stream open_client(executor_type executor, foundation::text::string_view name, pipe_direction dir = pipe_direction::inout);

        static named_pipe_stream open_client(executor_type executor, foundation::text::string_view name, pipe_direction dir, std::error_code &ec);

        bool is_open() const noexcept {
            return impl_ && impl_->is_open();
        }

        executor_type get_executor() const noexcept {
            return executor_;
        }

        native_handle_type native_handle() const noexcept {
#if RAINY_USING_WINDOWS
            return impl_ ? impl_->native_handle() : reinterpret_cast<native_handle_type>(-1);
#else
            return impl_ ? impl_->native_handle() : static_cast<native_handle_type>(-1);
#endif
        }

        std::error_code close() noexcept {
            if (!impl_) {
                return {};
            }
            return impl_->close();
        }

        std::error_code cancel() noexcept {
            if (!impl_) {
                return {};
            }
            return impl_->cancel();
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers) {
            std::error_code ec;
            std::size_t n = read_some(buffers, ec);
            if (ec) {
                throw exceptions::runtime::system_error(ec);
            }
            return n;
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers, std::error_code &ec) {
            auto mb = io::buffer(buffers);
            auto r = impl_->read_some(mb.data(), mb.size(), ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers) {
            std::error_code ec;
            std::size_t n = write_some(buffers, ec);
            if (ec) {
                throw exceptions::runtime::system_error(ec);
            }
            return n;
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers, std::error_code &ec) {
            auto cb = io::buffer(buffers);
            auto r = impl_->write_some(cb.data(), cb.size(), ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename MutableBufferSequence, typename CompletionToken>
        rain_fn async_read_some(MutableBufferSequence buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::buffer(buffers);
            auto *op = io::implements::make_executor_completion_op(
                [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                    if (cancelled) {
                        return;
                    }
                    std::error_code ec;
                    if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                },
                executor_);
            impl_->async_read_some(mb.data(), mb.size(), executor_, op);
            return init.result.get();
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        rain_fn async_write_some(ConstBufferSequence buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto cb = io::buffer(buffers);
            auto *op = io::implements::make_executor_completion_op(
                [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                    if (cancelled) {
                        return;
                    }
                    std::error_code ec;
                    if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                },
                executor_);
            impl_->async_write_some(cb.data(), cb.size(), executor_, op);
            return init.result.get();
        }

    private:
        explicit named_pipe_stream(executor_type ex, memory::nebula_ptr<implements::descriptor_impl_base> impl) noexcept :
            executor_(ex), impl_(utility::move(impl)) {
        }

        void close_if_open() noexcept {
            if (impl_ && impl_->is_open()) {
                utility::ignore = impl_->close();
            }
        }

        executor_type executor_;
        memory::nebula_ptr<implements::descriptor_impl_base> impl_;
    };
}

#endif
