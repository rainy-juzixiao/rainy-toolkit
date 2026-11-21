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
#ifndef RAINY_FOUNDATION_IO_IPC_SEMAPHORE_HPP
#define RAINY_FOUNDATION_IO_IPC_SEMAPHORE_HPP
#include <rainy/foundation/io/ipc/implements/semaphore_impl.hpp>
#include <chrono>

namespace rainy::foundation::io::ipc::semaphore {
    class RAINY_TOOLKIT_API semaphore {
    public:
        using executor_type = io_context::executor_type;
        using native_handle_type = stream::native_handle_type;

        semaphore(const semaphore &) = delete;
        semaphore &operator=(const semaphore &) = delete;
        semaphore(semaphore &&) noexcept;
        semaphore &operator=(semaphore &&) noexcept;
        ~semaphore();

        static semaphore create(executor_type ex, core::text::string_view name, const attributes &attr);
        static semaphore create(executor_type ex, core::text::string_view name, const attributes &attr, std::error_code &ec);

        static semaphore open(executor_type ex, core::text::string_view name);
        static semaphore open(executor_type ex, core::text::string_view name, std::error_code &ec);

        static semaphore open_or_create(executor_type ex, core::text::string_view name, const attributes &attr);
        static semaphore open_or_create(executor_type ex, core::text::string_view name, const attributes &attr, std::error_code &ec);

        static std::error_code unlink(core::text::string_view name);

        std::error_code wait() noexcept;
        std::error_code try_wait() noexcept;
        std::error_code timed_wait(std::chrono::milliseconds timeout) noexcept;
        std::error_code post() noexcept;

        RAINY_NODISCARD bool is_open() const noexcept;
        std::error_code close() noexcept;
        RAINY_NODISCARD native_handle_type native_handle() const noexcept;
        RAINY_NODISCARD executor_type get_executor() const noexcept;

        void swap(semaphore &other) noexcept;

    private:
        explicit semaphore(executor_type ex, core::memory::nebula_ptr<implements::semaphore_impl_base> impl) noexcept;

        executor_type executor_;
        core::memory::nebula_ptr<implements::semaphore_impl_base> impl_;
    };

    RAINY_TOOLKIT_API void swap(semaphore &a, semaphore &b) noexcept;
}

#endif