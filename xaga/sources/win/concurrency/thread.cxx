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
#include <rainy/foundation/concurrency/pal.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <windows.h>

namespace rainy::foundation::concurrency::implements {
    void endthread() {
        ExitThread(0);
    }

    void endthreadex(unsigned int return_code) {
        ExitThread(return_code);
    }

    RAINY_NODISCARD schd_thread_t
    create_thread(foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, unsigned int stack_size,
                  void *arg_list) {
        if (!invoke_function_addr) {
            // 如果函数地址为空
            errno = EINVAL;
            return {};
        }
        DWORD thread_id{0};
        HANDLE handle = CreateThread(nullptr, stack_size, reinterpret_cast<LPTHREAD_START_ROUTINE>(invoke_function_addr.get()),
                                     arg_list, 0, &thread_id);
        if (!handle) {
            DWORD error = GetLastError();
            switch (error) {
                case ERROR_NOT_ENOUGH_MEMORY:
                    errno = EAGAIN;
                    break;
                case ERROR_INVALID_PARAMETER:
                    errno = EINVAL;
                    break;
                case ERROR_ACCESS_DENIED:
                    errno = EACCES;
                    break;
                default:
                    break;
            }
            return {};
        }
        return {reinterpret_cast<std::uintptr_t>(handle), thread_id};
    }

    RAINY_NODISCARD schd_thread_t
    create_thread(void *security, unsigned int stack_size,
                  foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, void *arg_list,
                  unsigned int init_flag, std::uint64_t *thrd_addr) {
        DWORD thread_id = 0;
        if (!invoke_function_addr) {
            errno = EINVAL;
            return {};
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
            return {};
        }
        return {reinterpret_cast<std::uintptr_t>(handle), thread_id};
    }

    thrd_result thread_join(schd_thread_t thread_handle, std::int64_t *result_receiver) noexcept {
        if (!thread_handle.handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        rainy_let handle = reinterpret_cast<HANDLE>(thread_handle.handle);
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
                return thrd_result::error;
            }
            if (CloseHandle(handle)) {
                return thrd_result::success;
            }
            return thrd_result::error;
        }
        DWORD error = GetLastError();
        switch (error) {
            case ERROR_INVALID_PARAMETER:
            case ERROR_INVALID_HANDLE:
                errno = EINVAL;
                return thrd_result::error;
            case WAIT_ABANDONED:
                errno = EBUSY;
                return thrd_result::busy;
            case WAIT_TIMEOUT:
                errno = ETIMEDOUT;
                return thrd_result::timed_out;
            default:
                errno = EIO;
                break;
        }
        return thrd_result::error;
    }

    thrd_result thread_detach(schd_thread_t thread_handle) noexcept {
        if (!thread_handle.handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        if (CloseHandle(reinterpret_cast<HANDLE>(thread_handle.handle))) {
            return thrd_result::success;
        }
        return thrd_result::error;
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

    void suspend_thread(schd_thread_t thread_handle) noexcept {
        if (!thread_handle.handle) {
            return;
        }
        if (SuspendThread(reinterpret_cast<HANDLE>(thread_handle.handle)) == static_cast<DWORD>(-1)) {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
        }
    }

    void resume_thread(schd_thread_t thread_handle) noexcept {
        if (!thread_handle.handle) {
            return;
        }
        if (ResumeThread(reinterpret_cast<HANDLE>(thread_handle.handle)) == static_cast<DWORD>(-1)) {
            DWORD error = GetLastError();
            switch (error) {
                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                    errno = EINVAL;
                    break;
                case ERROR_ACCESS_DENIED:
                    errno = EACCES;
                    break;
            }
        }
    }
}
