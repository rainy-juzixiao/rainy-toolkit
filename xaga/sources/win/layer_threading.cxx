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
#include <chrono>
#include <windows.h>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/pal/implements/tgc_layer_threading.hpp>

namespace rainy::foundation::pal::threading::implements {
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

    RAINY_NODISCARD schd_thread_t create_thread(
        void *security, unsigned int stack_size,
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

namespace rainy::foundation::pal::threading::implements {
    struct mutex_handle {
        void *handle{nullptr};
        int type{0};
        unsigned long thread_id{0};
        int count{0};
    };

    static RAINY_INLINE PSRWLOCK get_srw_lock(mtx_t *mtx) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return nullptr;
        }
        return reinterpret_cast<PSRWLOCK>(&reinterpret_cast<mutex_handle *>(*mtx)->handle);
    }

    thrd_result mtx_do_lock(mtx_t *mtx, const ::timespec *target) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        rainy_let mutex = reinterpret_cast<mutex_handle *>(*mtx);
        const auto current_thread_id = static_cast<unsigned long>(GetCurrentThreadId());
        if ((mutex->type & ~mutex_types::recursive_mtx) == mutex_types::plain_mtx) {
            if (mutex->thread_id != current_thread_id) {
                AcquireSRWLockExclusive(get_srw_lock(mtx));
                mutex->thread_id = current_thread_id;
            }
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }
        int res = WAIT_TIMEOUT;
        if (!target) {
            if (mutex->thread_id != current_thread_id) {
                AcquireSRWLockExclusive(get_srw_lock(mtx));
            }
            res = WAIT_OBJECT_0;
        } else if (target->tv_sec < 0 || target->tv_sec == 0 && target->tv_nsec <= 0) {
            if (mutex->thread_id != current_thread_id) {
                if (TryAcquireSRWLockExclusive(get_srw_lock(mtx)) != 0) {
                    res = WAIT_OBJECT_0;
                }
            } else {
                res = WAIT_OBJECT_0;
            }
        } else {
            auto now = std::chrono::system_clock::now();
            while (true) {
                auto now_duration = now.time_since_epoch();
                auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now_duration);
                auto now_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now_duration) % std::chrono::seconds(1);
                if (now_seconds.count() > target->tv_sec ||
                    (now_seconds.count() == target->tv_sec && now_nanos.count() >= target->tv_nsec)) {
                    break;
                }
                if (mutex->thread_id == current_thread_id || TryAcquireSRWLockExclusive(get_srw_lock(mtx)) != 0) {
                    res = WAIT_OBJECT_0;
                    break;
                }
                now = std::chrono::system_clock::now();
            }
        }
        if (res == WAIT_OBJECT_0) {
            if (1 < core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count))) {
                if ((mutex->type & static_cast<int>(mutex_types::recursive_mtx)) != static_cast<int>(mutex_types::recursive_mtx)) {
                    core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));
                    res = WAIT_TIMEOUT;
                }
            } else {
                mutex->thread_id = current_thread_id;
            }
        }
        if (res == WAIT_OBJECT_0) {
            return thrd_result::success;
        }
        if (!target || (target->tv_sec == 0 && target->tv_nsec == 0)) {
            errno = EBUSY;
            return thrd_result::busy;
        }
        errno = ETIMEDOUT;
        return thrd_result::timed_out;
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        mutex->type = flags;
        mutex->count = 0;
        mutex->handle = nullptr;
        mutex->thread_id = 0;
        return thrd_result::success;
    }

    thrd_result mtx_create(mtx_t *mtx, int flags) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        *mtx = core::pal::allocate(sizeof(implements::mutex_handle), alignof(implements::mutex_handle));
        mtx_init(mtx, flags);
        return thrd_result::success;
    }

    thrd_result mtx_lock(mtx_t *mtx) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        return implements::mtx_do_lock(mtx, nullptr);
    }

    thrd_result mtx_trylock(mtx_t *mtx) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        utility::expects((mutex->type & (mutex_types::try_mtx | mutex_types::timed_mtx)) != 0, "trylock not supported by mutex");
        ::timespec xt{};
        xt.tv_sec = 0;
        xt.tv_nsec = 0;
        return implements::mtx_do_lock(mtx, &xt);
    }

    thrd_result mtx_unlock(mtx_t *mtx) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        if (--mutex->count == 0) {
            mutex->thread_id = 0;
            auto *srw_lock = implements::get_srw_lock(mtx);
            _Analysis_assume_lock_held_(*srw_lock);
            ReleaseSRWLockExclusive(srw_lock);
        }
        return thrd_result::success;
    }

    thrd_result mtx_timedlock(mtx_t *mtx, const ::timespec *xt) noexcept {
        if (!mtx || !xt) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        thrd_result res{};
        res = implements::mtx_do_lock(mtx, xt);
        return res == thrd_result::busy ? thrd_result::timed_out : res;
    }

    bool mtx_current_owns(mtx_t *mtx) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return false;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        return mutex->count != 0 && mutex->thread_id == GetCurrentThreadId();
    }

    thrd_result mtx_destroy(mtx_t *mtx) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        utility::expects(mutex->count == 0, "mutex destroyed while busy");
        core::pal::deallocate(mutex, sizeof(implements::mutex_handle), alignof(implements::mutex_handle));
        *mtx = nullptr;
        return thrd_result::success;
    }

    void *native_mtx_handle(mtx_t *mtx) noexcept {
        if (!mtx) {
            return nullptr;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        return mutex->handle;
    }
}

namespace rainy::foundation::pal::threading::implements {
    core::handle tss_create() {
        const core::handle out_of_indexes = TLS_OUT_OF_INDEXES;
        core::handle tss_key = TlsAlloc();
        if (tss_key == out_of_indexes) {
            errno = EFAULT;
            return core::invalid_handle;
        }
        return tss_key;
    }

    void *tss_get(core::handle tss_key) {
        if (tss_key == core::invalid_handle) {
            return nullptr;
        }
        return TlsGetValue(tss_key);
    }

    bool tss_set(core::handle tss_key, void *value) {
        if (tss_key == core::invalid_handle) {
            return false;
        }
        return TlsSetValue(tss_key, const_cast<void *>(value));
    }

    bool tss_delete(core::handle tss_key) {
        if (tss_key == core::invalid_handle) {
            return false;
        }
        return TlsFree(tss_key);
    }
}
