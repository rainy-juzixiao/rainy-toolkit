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
#ifndef RAINY_FOUNDATION_IO_IPC_IMPLEMENTS_SHARED_MEMORY_IMPL_HPP
#define RAINY_FOUNDATION_IO_IPC_IMPLEMENTS_SHARED_MEMORY_IMPL_HPP
#include <rainy/foundation/io/io_context.hpp>
#include <rainy/foundation/io/ipc/fwd.hpp>
#include <cstddef>
#include <system_error>

namespace rainy::foundation::io::ipc::shared_memory::implements {
    using executor_type = io_context::executor_type;
    using native_handle_type = stream::native_handle_type;

    class shared_memory_impl_base {
    public:
        virtual ~shared_memory_impl_base() = default;

        RAINY_NODISCARD virtual bool is_open() const noexcept = 0;
        virtual std::error_code close() noexcept = 0;
        RAINY_NODISCARD virtual native_handle_type native_handle() const noexcept = 0;

        RAINY_NODISCARD virtual void *data() const noexcept = 0;
        RAINY_NODISCARD virtual std::size_t size() const noexcept = 0;
        virtual std::error_code flush() noexcept = 0;
        virtual std::error_code flush(std::size_t offset, std::size_t length) noexcept = 0;
    };

    RAINY_TOOLKIT_API core::memory::nebula_ptr<shared_memory_impl_base>
    create_shared_memory_impl(executor_type executor, const char *name, const attributes &attr, std::error_code &ec);

    RAINY_TOOLKIT_API core::memory::nebula_ptr<shared_memory_impl_base>
    open_shared_memory_impl(executor_type executor, const char *name, access_mode mode, std::error_code &ec);

    RAINY_TOOLKIT_API core::memory::nebula_ptr<shared_memory_impl_base>
    open_or_create_shared_memory_impl(executor_type executor, const char *name, const attributes &attr, std::error_code &ec);

    RAINY_TOOLKIT_API core::memory::nebula_ptr<shared_memory_impl_base> create_null_shared_memory_impl(executor_type executor);

    RAINY_TOOLKIT_API std::error_code unlink_shared_memory(const char *name) noexcept;
}

#endif