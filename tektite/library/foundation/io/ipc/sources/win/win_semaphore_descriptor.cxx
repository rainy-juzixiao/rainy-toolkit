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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <rainy/core/text/string.hpp>
#include <rainy/foundation/io/ipc/implements/semaphore_impl.hpp>
#include <windows.h>

namespace rainy::foundation::io::ipc::semaphore::implements {
    using io::implements::op_result;

    static std::error_code last_error() noexcept {
        return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
    }

    static auto to_wide(const char *name) {
        int n = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        rainy::core::text::wstring wname;
        wname.resize(static_cast<rainy::core::text::wstring::size_type>(n - 1));
        ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wname.data(), n);
        return wname;
    }

    class win_semaphore_impl final : public semaphore_impl_base {
    public:
        win_semaphore_impl(HANDLE sem, unsigned int max_value) noexcept : sem_(sem), max_value_(max_value) {
        }

        ~win_semaphore_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return sem_ != nullptr && sem_ != INVALID_HANDLE_VALUE;
        }

        std::error_code close() noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            ::CloseHandle(sem_);
            sem_ = INVALID_HANDLE_VALUE;
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(sem_);
        }

        std::error_code wait() noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            DWORD result = ::WaitForSingleObject(sem_, INFINITE);
            if (result == WAIT_OBJECT_0) {
                return {};
            }
            return last_error();
        }

        std::error_code try_wait() noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            DWORD result = ::WaitForSingleObject(sem_, 0);
            if (result == WAIT_OBJECT_0) {
                return {};
            }
            if (result == WAIT_TIMEOUT) {
                return std::make_error_code(std::errc::operation_would_block);
            }
            return last_error();
        }

        std::error_code timed_wait(std::chrono::milliseconds timeout) noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            DWORD ms = static_cast<DWORD>(timeout.count());
            DWORD result = ::WaitForSingleObject(sem_, ms);
            if (result == WAIT_OBJECT_0) {
                return {};
            }
            if (result == WAIT_TIMEOUT) {
                return std::make_error_code(std::errc::timed_out);
            }
            return last_error();
        }

        std::error_code post() noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            if (!::ReleaseSemaphore(sem_, 1, nullptr)) {
                return last_error();
            }
            return {};
        }

    private:
        HANDLE sem_{INVALID_HANDLE_VALUE};
        unsigned int max_value_{0};
    };

    class null_semaphore_impl final : public semaphore_impl_base {
    public:
        explicit null_semaphore_impl(executor_type) noexcept {
        }

        bool is_open() const noexcept override {
            return false;
        }

        std::error_code close() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(INVALID_HANDLE_VALUE);
        }

        std::error_code wait() noexcept override {
            return {};
        }

        std::error_code try_wait() noexcept override {
            return {};
        }

        std::error_code timed_wait(std::chrono::milliseconds) noexcept override {
            return {};
        }

        std::error_code post() noexcept override {
            return {};
        }
    };

    core::memory::nebula_ptr<semaphore_impl_base> create_semaphore_impl(executor_type executor, const char *name,
                                                                        const attributes &attr, std::error_code &ec) {
        auto wname = to_wide(name);
        HANDLE sem = ::CreateSemaphoreW(nullptr, attr.initial_value, attr.max_value, wname.c_str());
        if (!sem) {
            ec = last_error();
            return create_null_semaphore_impl(executor);
        }
        if (::GetLastError() == ERROR_ALREADY_EXISTS) {
            ::CloseHandle(sem);
            ec = std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            return create_null_semaphore_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<win_semaphore_impl>(sem, attr.max_value);
    }

    core::memory::nebula_ptr<semaphore_impl_base> open_semaphore_impl(executor_type executor, const char *name, std::error_code &ec) {
        auto wname = to_wide(name);
        HANDLE sem = ::OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, FALSE, wname.c_str());
        if (!sem) {
            ec = last_error();
            return create_null_semaphore_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<win_semaphore_impl>(sem, 0);
    }

    core::memory::nebula_ptr<semaphore_impl_base> open_or_create_semaphore_impl(executor_type executor, const char *name,
                                                                                const attributes &attr, std::error_code &ec) {
        auto wname = to_wide(name);
        HANDLE sem = ::CreateSemaphoreW(nullptr, attr.initial_value, attr.max_value, wname.c_str());
        if (!sem) {
            ec = last_error();
            return create_null_semaphore_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<win_semaphore_impl>(sem, attr.max_value);
    }

    core::memory::nebula_ptr<semaphore_impl_base> create_null_semaphore_impl(executor_type executor) {
        return core::memory::make_nebula<null_semaphore_impl>(executor);
    }

    std::error_code unlink_semaphore(const char *name) noexcept {
        return {};
    }
}
