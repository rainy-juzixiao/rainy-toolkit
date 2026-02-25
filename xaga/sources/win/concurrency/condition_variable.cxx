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
#include <rainy/foundation/concurrency/pal.hpp>
#include <windows.h>

namespace rainy::foundation::concurrency::implements {
    struct mutex_handle {
        void *handle{nullptr};
        int type{0};
        unsigned long thread_id{0};
        int count{0};
    };

    struct cnd_internal {
        CONDITION_VARIABLE cond;
    };

    thrd_result cnd_create(cnd_t *const cnd) noexcept {
        if (!cnd) {
            return thrd_result::nomem;
        }
        auto *obj = static_cast<cnd_internal *>(core::pal::allocate(sizeof(cnd_internal), alignof(cnd_internal)));
        if (!obj) {
            return thrd_result::nomem;
        }
        InitializeConditionVariable(&obj->cond);
        *cnd = static_cast<cnd_t>(obj);
        return thrd_result::success;
    }

    thrd_result cnd_init(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        InitializeConditionVariable(&obj->cond);
        return thrd_result::success;
    }

    thrd_result cnd_wait(cnd_t *const cnd, mtx_t *const mtx) noexcept {
        if (!cnd || !*cnd || !mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto *obj = static_cast<cnd_internal *>(*cnd);
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        auto *psrwlock = static_cast<PSRWLOCK>(native_mtx_handle(mtx));
        if (!psrwlock) {
            return thrd_result::nomem;
        }
        const int saved_count = mutex->count;
        const DWORD saved_tid = mutex->thread_id;
        mutex->count = 0;
        mutex->thread_id = 0;

        BOOL ret = SleepConditionVariableSRW(&obj->cond, psrwlock, INFINITE, 0);

        mutex->count = saved_count;
        mutex->thread_id = GetCurrentThreadId();

        if (ret) {
            return thrd_result::success;
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_PARAMETER) {
                return thrd_result::nomem;
            }
            return thrd_result::error;
        }
    }

    thrd_result cnd_timedwait(cnd_t *const cnd, mtx_t *const mtx, const ::timespec *timeout) noexcept {
        if (!cnd || !*cnd || !mtx || !*mtx || !timeout) {
            return thrd_result::nomem;
        }
        // 验证 timeout 参数
        if (timeout->tv_sec < 0 || timeout->tv_nsec < 0 || timeout->tv_nsec >= 1000000000) {
            return thrd_result::nomem;
        }
        auto *obj = static_cast<cnd_internal *>(*cnd);
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        auto *psrwlock = static_cast<PSRWLOCK>(native_mtx_handle(mtx));
        if (!psrwlock) {
            return thrd_result::nomem;
        }
        const int saved_count = mutex->count;
        const DWORD saved_tid = mutex->thread_id;
        mutex->count = 0;
        mutex->thread_id = 0;
        // 获取当前时间
        ::timespec now;
        if (::timespec_get(&now, TIME_UTC) == 0) {
            mutex->count = saved_count;
            mutex->thread_id = GetCurrentThreadId();
            return thrd_result::error;
        }
        // 计算相对超时时间（timeout - now）
        DWORD dwMilliseconds;
        long long diff_sec = static_cast<long long>(timeout->tv_sec) - static_cast<long long>(now.tv_sec);
        long long diff_nsec = static_cast<long long>(timeout->tv_nsec) - static_cast<long long>(now.tv_nsec);
        // 如果已经超时（deadline 在过去）
        if (diff_sec < 0 || (diff_sec == 0 && diff_nsec <= 0)) {
            dwMilliseconds = 0;
        } else {
            long long total_ns = diff_sec * 1000000000LL + diff_nsec;
            // 向上取整到毫秒，加 1ms 补偿
            long long total_ms = (total_ns + 999999LL) / 1000000LL;
            if (total_ms > 0) {
                total_ms += 1; // 补偿 Windows 定时器精度
            }

            if (total_ms > (long long) (INFINITE - 1)) {
                dwMilliseconds = INFINITE - 1;
            } else {
                dwMilliseconds = (DWORD) total_ms;
            }
        }
        BOOL ret = SleepConditionVariableSRW(&obj->cond, psrwlock, dwMilliseconds, 0);
        mutex->count = saved_count;
        mutex->thread_id = GetCurrentThreadId();
        if (ret) {
            return thrd_result::success;
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_TIMEOUT) {
                return thrd_result::timed_out;
            } else if (error == ERROR_INVALID_PARAMETER) {
                return thrd_result::nomem;
            }
            return thrd_result::error;
        }
    }

    thrd_result cnd_signal(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        WakeConditionVariable(&obj->cond);
        return thrd_result::success;
    }

    thrd_result cnd_broadcast(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        WakeAllConditionVariable(&obj->cond);
        return thrd_result::success;
    }

    thrd_result cnd_destroy(cnd_t *cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        // Windows CONDITION_VARIABLE 不需要显式销毁
        core::pal::deallocate(obj, sizeof(cnd_internal), alignof(cnd_internal));
        *cnd = nullptr;
        return thrd_result::success;
    }

    void *native_cnd_handle(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return nullptr;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        return &obj->cond;
    }
}
