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
#ifndef RAINY_FOUNDATION_IO_IPC_SHARED_MEMORY_HPP
#define RAINY_FOUNDATION_IO_IPC_SHARED_MEMORY_HPP
#include <rainy/foundation/io/ipc/implements/shared_memory_impl.hpp>

namespace rainy::foundation::io::ipc::shared_memory {
    class RAINY_TOOLKIT_API shared_memory {
    public:
        using executor_type = io_context::executor_type;
        using native_handle_type = stream::native_handle_type;
        using value_type = unsigned char;
        using size_type = std::size_t;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        shared_memory(const shared_memory &) = delete;
        shared_memory &operator=(const shared_memory &) = delete;
        shared_memory(shared_memory &&) noexcept;
        shared_memory &operator=(shared_memory &&) noexcept;
        ~shared_memory();

        static shared_memory create(executor_type ex, core::text::string_view name, const attributes &attr);
        static shared_memory create(executor_type ex, core::text::string_view name, const attributes &attr, std::error_code &ec);

        static shared_memory open(executor_type ex, core::text::string_view name, access_mode mode = access_mode::read_write);
        static shared_memory open(executor_type ex, core::text::string_view name, access_mode mode, std::error_code &ec);

        static shared_memory open_or_create(executor_type ex, core::text::string_view name, const attributes &attr);
        static shared_memory open_or_create(executor_type ex, core::text::string_view name, const attributes &attr, std::error_code &ec);

        static std::error_code unlink(core::text::string_view name);

        RAINY_NODISCARD pointer data() const noexcept;
        RAINY_NODISCARD size_type size() const noexcept;
        RAINY_NODISCARD bool empty() const noexcept;

        std::error_code flush() noexcept;
        std::error_code flush(size_type offset, size_type length) noexcept;

        RAINY_NODISCARD bool is_open() const noexcept;
        std::error_code close() noexcept;
        RAINY_NODISCARD native_handle_type native_handle() const noexcept;
        RAINY_NODISCARD executor_type get_executor() const noexcept;

        void swap(shared_memory &other) noexcept;

    private:
        explicit shared_memory(executor_type ex, core::memory::nebula_ptr<implements::shared_memory_impl_base> impl) noexcept;

        executor_type executor_;
        core::memory::nebula_ptr<implements::shared_memory_impl_base> impl_;
    };

    RAINY_TOOLKIT_API void swap(shared_memory &a, shared_memory &b) noexcept;
}

#endif