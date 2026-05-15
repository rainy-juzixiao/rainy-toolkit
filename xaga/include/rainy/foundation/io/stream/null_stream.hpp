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
#ifndef RAINY_IO_STREAM_NULL_STREAM_HPP
#define RAINY_IO_STREAM_NULL_STREAM_HPP
#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>

namespace rainy::foundation::io::stream {
    class RAINY_TOOLKIT_API null_stream {
    public:
        using executor_type = io_context::executor_type;

        static null_stream &null();

        null_stream(const null_stream &) = delete;
        null_stream &operator=(const null_stream &) = delete;
        null_stream(null_stream &&) = delete;
        null_stream &operator=(null_stream &&) = delete;

        executor_type get_executor() const noexcept;

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers) {
            std::error_code ec;
            auto result = read_some(buffers, ec);
            if (ec) {
                throw exceptions::runtime::system_error(ec);
            }
            return result;
        }

        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers, std::error_code &ec) {
            auto mb = io::buffer(buffers);
            auto r = this->impl_->read_some(mb.data(), mb.size(), ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers) {
            std::error_code ec;
            auto result = write_some(buffers, ec);
            if (ec) {
                throw exceptions::runtime::system_error(ec);
            }
            return result;
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers, std::error_code &ec) {
            auto cb = io::buffer(buffers);
            auto r = this->impl_->write_some(cb.data(), cb.size(), ec);
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
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                });
            this->impl_->async_read_some(mb.data(), mb.size(), this->executor_, op);
            return init.result.get();
        }

        template <typename ConstBufferSequence, typename CompletionToken>
        rain_fn async_write_some(ConstBufferSequence buffers, CompletionToken &&token) {
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::buffer(buffers);
            auto *op = io::implements::make_executor_completion_op(
                [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                });
            this->impl_->async_write_some(mb.data(), mb.size(), this->executor_, op);
            return init.result.get();
        }

        bool is_open() const noexcept;
        native_handle_type native_handle() const noexcept;

    private:
        explicit null_stream(executor_type ex);

        static io_context &default_context();

        executor_type executor_;
        memory::nebula_ptr<implements::descriptor_impl_base> impl_;
    };
}

#endif
