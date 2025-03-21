/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/winapi/system/multithread/thread.h>
#include <rainy/foundation/diagnostics/contract.hpp>

namespace rainy::winapi::system::multithread {
    void endthread() {
        ExitThread(0);
    }

    void endthreadex(unsigned int return_code) {
        ExitThread(return_code);
    }

    RAINY_NODISCARD std::uintptr_t create_thread(
        foundation::functional::function_pointer<unsigned int(*)(void *)> invoke_function_addr, unsigned int stack_size, void *arg_list) {
        if (!invoke_function_addr) {
            // 如果函数地址为空
            errno = EINVAL;
            return -1;
        }
        HANDLE handle = CreateThread(nullptr, stack_size, reinterpret_cast<LPTHREAD_START_ROUTINE>(invoke_function_addr.get()),
                                     arg_list, 0, nullptr);
        if (!handle) {
            DWORD error = GetLastError();
            if (error == ERROR_NOT_ENOUGH_MEMORY) {
                errno = EAGAIN;
            } else if (error == ERROR_INVALID_PARAMETER) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
#if RAINY_ENABLE_DEBUG
            throw std::system_error(errno, std::system_category());
#else
            return -1;
#endif
        }
        return reinterpret_cast<std::uintptr_t>(handle);
    }

    RAINY_NODISCARD std::uintptr_t create_thread(
        void *security, unsigned int stack_size, foundation::functional::function_pointer<unsigned int(*)(void *)> invoke_function_addr,
        void *arg_list, unsigned int init_flag, std::uint64_t *thrd_addr) {
        DWORD thread_id = 0;
        if (!invoke_function_addr) {
            errno = EINVAL;
            return -1;
        }
        HANDLE handle =
            CreateThread(static_cast<LPSECURITY_ATTRIBUTES>(security), stack_size,
                         reinterpret_cast<LPTHREAD_START_ROUTINE>(invoke_function_addr.get()), arg_list, init_flag, &thread_id);
        if (thrd_addr) {
            *thrd_addr = thread_id;
        }
        if (!handle) {
            DWORD error = GetLastError();
            if (error == ERROR_NOT_ENOUGH_MEMORY) {
                errno = EAGAIN;
            } else if (error == ERROR_INVALID_PARAMETER) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
#if RAINY_ENABLE_DEBUG
            throw std::system_error(errno, std::system_category());
#else
            return -1;
#endif
        }
        return reinterpret_cast<std::uintptr_t>(handle);
    }

    win32_thread_result thread_join(std::uintptr_t thread_handle, std::int64_t *result_receiver) noexcept {
        if (!thread_handle) {
            errno = EINVAL;
            return win32_thread_result::error;
        }
        rainy_let handle = reinterpret_cast<HANDLE>(thread_handle);
        DWORD wait_result = WaitForSingleObject(handle, INFINITE);
        if (wait_result == WAIT_OBJECT_0) {
            static DWORD placeholder;
            DWORD *param_result_receiver = nullptr;
            if (!result_receiver) {
                param_result_receiver = &placeholder;
            } else {
                param_result_receiver = reinterpret_cast<LPDWORD>(result_receiver);
            }
            if (GetExitCodeThread(handle, param_result_receiver) == 0) {
                return win32_thread_result::error;
            }
            if (CloseHandle(handle)) {
                return win32_thread_result::success;
            }
            return win32_thread_result::error;
        }
        DWORD error = GetLastError();
        if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
            errno = EINVAL;
            return win32_thread_result::error;
        }
        if (wait_result == WAIT_ABANDONED) {
            return win32_thread_result::busy;
        }
        if (wait_result == WAIT_TIMEOUT) {
            return win32_thread_result::timed_out;
        }
        return win32_thread_result::error;
    }

    win32_thread_result thread_detach(std::uintptr_t thread_handle) noexcept {
        if (!thread_handle) {
            errno = EINVAL;
            return win32_thread_result::error;
        }
        if (CloseHandle(reinterpret_cast<HANDLE>(thread_handle))) {
            return win32_thread_result::success;
        }
        return win32_thread_result::error;
    }

    void thread_yield() noexcept {
        SwitchToThread();
    }

    unsigned int thread_hardware_concurrency() noexcept {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return static_cast<unsigned int>(sysinfo.dwNumberOfProcessors);
    }

    std::uint64_t get_thread_id() noexcept {
        return GetCurrentThreadId();
    }

    void thread_sleep_for(const unsigned long ms) noexcept {
        Sleep(ms);
    }

    void suspend_thread(std::uintptr_t thread_handle) noexcept {
        if (!thread_handle) {
            return;
        }
        if (SuspendThread(reinterpret_cast<HANDLE>(thread_handle)) == static_cast<DWORD>(-1)) {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
        }
    }

    void resume_thread(std::uintptr_t thread_handle) noexcept {
        if (!thread_handle) {
            return;
        }
        if (ResumeThread(reinterpret_cast<HANDLE>(thread_handle)) == static_cast<DWORD>(-1)) {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
        }
    }
}
