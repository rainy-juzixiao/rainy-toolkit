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
#ifndef RAINY_FOUNDATION_IO_IPC_MESSAGE_QUEUE_HPP
#define RAINY_FOUNDATION_IO_IPC_MESSAGE_QUEUE_HPP
#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/io_context.hpp>
#include <rainy/foundation/io/ipc/implements/message_queue_impl.hpp>

namespace rainy::foundation::io::ipc::message_queue {
    class RAINY_TOOLKIT_API message_queue {
    public:
        using executor_type = io_context::executor_type;
        using native_handle_type = stream::native_handle_type;
        using size_type = std::size_t;
        message_queue(const message_queue &) = delete;
        message_queue &operator=(const message_queue &) = delete;
        message_queue(message_queue &&) noexcept;
        message_queue &operator=(message_queue &&) noexcept;
        ~message_queue();

        static message_queue create(executor_type ex, core::text::string_view name, const attributes &attr,
                                   direction dir = direction::inout);
        static message_queue create(executor_type ex, core::text::string_view name, const attributes &attr, direction dir,
                                   std::error_code &ec);

        static message_queue open(executor_type ex, core::text::string_view name, direction dir = direction::inout);
        static message_queue open(executor_type ex, core::text::string_view name, direction dir, std::error_code &ec);

        static message_queue open_or_create(executor_type ex, core::text::string_view name, const attributes &attr,
                                            direction dir = direction::inout);
        static message_queue open_or_create(executor_type ex, core::text::string_view name, const attributes &attr,
                                            direction dir, std::error_code &ec);

        static std::error_code unlink(core::text::string_view name);

        RAINY_NODISCARD bool empty() const noexcept;
        RAINY_NODISCARD size_type size() const noexcept;
        RAINY_NODISCARD size_type capacity() const noexcept;
        RAINY_NODISCARD size_type max_message_size() const noexcept;

        void push(const const_buffer &buffer);
        void push(const const_buffer &buffer, priority priority);
        void push(const const_buffer &buffer, std::error_code &ec);
        void push(const const_buffer &buffer, priority priority, std::error_code &ec);

        bool try_push(const const_buffer &buffer, priority priority, std::error_code &ec);

        template <typename Rep, typename Period>
        bool timed_push(const const_buffer &buffer, priority priority,
                        const std::chrono::duration<Rep, Period> &timeout, std::error_code &ec) {
            return timed_push_ms(buffer, priority, std::chrono::duration_cast<std::chrono::milliseconds>(timeout), ec);
        }

        size_type pop(const mutable_buffer &buffer);
        size_type pop(const mutable_buffer &buffer, priority &priority);
        size_type pop(const mutable_buffer &buffer, priority &priority, std::error_code &ec);

        bool try_pop(const mutable_buffer &buffer, priority &priority, size_type &received, std::error_code &ec);

        template <typename Rep, typename Period>
        bool timed_pop(const mutable_buffer &buffer, priority &priority, size_type &received,
                       const std::chrono::duration<Rep, Period> &timeout, std::error_code &ec) {
            return timed_pop_ms(buffer, priority, received,
                                std::chrono::duration_cast<std::chrono::milliseconds>(timeout), ec);
        }

        template <typename CompletionToken>
        rain_fn async_push(const const_buffer &buffer, priority priority, CompletionToken &&token) ->
            typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
            using token_t = type_traits::other_trans::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto *op = io::implements::make_executor_completion_op(
                [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                },
                executor_);
            impl_->async_send(buffer.data(), buffer.size(), priority, executor_, op);
            return init.result.get();
        }

        template <typename CompletionToken>
        rain_fn async_pop(const mutable_buffer &buffer, CompletionToken &&token) ->
            typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
            using token_t = type_traits::other_trans::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto *op = io::implements::make_executor_completion_op(
                [handler, is_open = impl_->is_open()](const io::implements::op_result &r, const bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    } else if (!is_open) {
                        ec = std::make_error_code(std::errc::bad_file_descriptor);
                    }
                    handler(ec, r.bytes_transferred);
                },
                executor_);
            impl_->async_receive(buffer.data(), buffer.size(), executor_, op);
            return init.result.get();
        }

        RAINY_NODISCARD bool is_open() const noexcept;
        std::error_code close() noexcept;
        std::error_code cancel() noexcept;
        RAINY_NODISCARD native_handle_type native_handle() const noexcept;
        RAINY_NODISCARD executor_type get_executor() const noexcept;

        void swap(message_queue &other) noexcept;

    private:
        bool timed_push_ms(const const_buffer &buffer, priority priority, std::chrono::milliseconds timeout, std::error_code &ec);
        bool timed_pop_ms(const mutable_buffer &buffer, priority &priority, size_type &received,
                          std::chrono::milliseconds timeout, std::error_code &ec);

        explicit message_queue(executor_type ex, core::memory::nebula_ptr<implements::message_queue_impl_base> impl) noexcept;

        executor_type executor_;
        core::memory::nebula_ptr<implements::message_queue_impl_base> impl_;
    };

    RAINY_TOOLKIT_API void swap(message_queue &a, message_queue &b) noexcept;
}

#endif