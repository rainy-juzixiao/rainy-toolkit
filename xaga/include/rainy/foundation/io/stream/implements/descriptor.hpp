/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0

 * *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS
 * IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language
 * governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_FOUNDATION_IO_STREAM_IMPLEMENTS_DESCRIPTOR_HPP
#define RAINY_FOUNDATION_IO_STREAM_IMPLEMENTS_DESCRIPTOR_HPP
#include <rainy/foundation/io/io_context.hpp>
#include <rainy/foundation/io/stream/fwd.hpp>
#include <system_error>

namespace rainy::foundation::io::stream::implements {
    using executor_type = io_context::executor_type;

    class descriptor_impl_base {
    public:
        using native_handle_type = stream::native_handle_type; // NOLINT

        using completion_op = io::implements::completion_op;

        virtual ~descriptor_impl_base() = default;

        RAINY_NODISCARD virtual bool is_open() const noexcept = 0;
        virtual std::error_code close() noexcept = 0;

        virtual std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept = 0;
        virtual std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept = 0;

        virtual void async_read_some(void *buf, std::size_t len, io_context::executor_type executor, completion_op *op) noexcept = 0;
        virtual void async_write_some(const void *buf, std::size_t len, io_context::executor_type executor,
                                      completion_op *op) noexcept = 0;

        virtual std::error_code cancel() noexcept = 0;
        RAINY_NODISCARD virtual native_handle_type native_handle() const noexcept = 0;

        RAINY_NODISCARD virtual bool wants_read() const noexcept = 0;
        RAINY_NODISCARD virtual bool wants_write() const noexcept = 0;
        virtual void reset_operation() noexcept = 0;

        virtual std::error_code attach(native_handle_type handle) noexcept = 0;
        virtual native_handle_type release() noexcept = 0;
        virtual std::error_code attach_from(descriptor_impl_base *other) noexcept = 0;
    };

    RAINY_TOOLKIT_API memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl(executor_type executor);

    RAINY_TOOLKIT_API memory::nebula_ptr<descriptor_impl_base> create_console_impl(executor_type executor, console_stream_kind kind);

    RAINY_TOOLKIT_API utility::pair<memory::nebula_ptr<descriptor_impl_base>, memory::nebula_ptr<descriptor_impl_base>>
    create_pipe_impl(executor_type executor, std::error_code &ec);
    RAINY_TOOLKIT_API memory::nebula_ptr<descriptor_impl_base> create_named_pipe_server_impl(executor_type executor, const char *name,
                                                                                             pipe_direction dir, std::error_code &ec);

    RAINY_TOOLKIT_API memory::nebula_ptr<descriptor_impl_base> create_named_pipe_client_impl(executor_type executor, const char *name,
                                                                                             pipe_direction dir, std::error_code &ec);

    RAINY_TOOLKIT_API memory::nebula_ptr<descriptor_impl_base> create_null_impl(executor_type executor);
    RAINY_TOOLKIT_API memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl_from_native(executor_type executor,
                                                                                                  native_handle_type handle);
}

#endif
