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
#ifndef RAINY_FOUNDATION_IO_IPC_IMPLEMENTS_MESSAGE_QUEUE_IMPL_HPP
#define RAINY_FOUNDATION_IO_IPC_IMPLEMENTS_MESSAGE_QUEUE_IMPL_HPP
#include <rainy/foundation/io/io_context.hpp>
#include <rainy/foundation/io/ipc/fwd.hpp>
#include <chrono>
#include <cstddef>
#include <system_error>

namespace rainy::foundation::io::ipc::message_queue::implements {
    using executor_type = io_context::executor_type;
    using completion_op = io::implements::completion_op;
    using native_handle_type = stream::native_handle_type;

    class message_queue_impl_base {
    public:
        virtual ~message_queue_impl_base() = default;

        RAINY_NODISCARD virtual bool is_open() const noexcept = 0;
        virtual std::error_code close() noexcept = 0;
        virtual std::error_code cancel() noexcept = 0;
        RAINY_NODISCARD virtual native_handle_type native_handle() const noexcept = 0;

        virtual std::ptrdiff_t send(const void *buf, std::size_t len, priority priority, std::error_code &ec) noexcept = 0;
        virtual std::ptrdiff_t receive(void *buf, std::size_t len, priority &priority, std::error_code &ec) noexcept = 0;

        virtual std::ptrdiff_t try_send(const void *buf, std::size_t len, priority priority, std::error_code &ec) noexcept = 0;
        virtual std::ptrdiff_t try_receive(void *buf, std::size_t len, priority &priority, std::error_code &ec) noexcept = 0;

        virtual std::ptrdiff_t timed_send(const void *buf, std::size_t len, priority priority,
                                          std::chrono::milliseconds timeout, std::error_code &ec) noexcept = 0;
        virtual std::ptrdiff_t timed_receive(void *buf, std::size_t len, priority &priority,
                                             std::chrono::milliseconds timeout, std::error_code &ec) noexcept = 0;

        virtual void async_send(const void *buf, std::size_t len, priority priority,
                                executor_type executor, completion_op *op) noexcept = 0;
        virtual void async_receive(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept = 0;

        RAINY_NODISCARD virtual std::size_t max_messages() const noexcept = 0;
        RAINY_NODISCARD virtual std::size_t max_message_size() const noexcept = 0;
        RAINY_NODISCARD virtual std::size_t current_messages() const noexcept = 0;
    };

    RAINY_TOOLKIT_API core::memory::nebula_ptr<message_queue_impl_base>
    create_message_queue_impl(executor_type executor, const char *name, const attributes &attr, direction dir, std::error_code &ec);

    RAINY_TOOLKIT_API core::memory::nebula_ptr<message_queue_impl_base>
    open_message_queue_impl(executor_type executor, const char *name, direction dir, std::error_code &ec);

    RAINY_TOOLKIT_API core::memory::nebula_ptr<message_queue_impl_base>
    open_or_create_message_queue_impl(executor_type executor, const char *name, const attributes &attr, direction dir,
                                      std::error_code &ec);

    RAINY_TOOLKIT_API core::memory::nebula_ptr<message_queue_impl_base> create_null_message_queue_impl(executor_type executor);

    RAINY_TOOLKIT_API std::error_code unlink_message_queue(const char *name) noexcept;
}

#endif