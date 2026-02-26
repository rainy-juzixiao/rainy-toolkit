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

    thrd_result cnd_timedwait(cnd_t *const cnd, mtx_t *const mtx, const ::timespec *abstime) noexcept {
        if (!cnd || !*cnd || !mtx || !*mtx || !abstime) {
            return thrd_result::nomem;
        }
        if (abstime->tv_sec < 0 || abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000) {
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

        ::timespec now;
        if (::timespec_get(&now, TIME_UTC) == 0) {
            mutex->count = saved_count;
            mutex->thread_id = saved_tid;
            return thrd_result::error;
        }
        constexpr std::int64_t NS_PER_SEC = 1000000000LL;
        constexpr std::int64_t NS_PER_MS = 1000000LL;
        std::int64_t abs_ns = static_cast<std::int64_t>(abstime->tv_sec * NS_PER_SEC + (std::int64_t) abstime->tv_nsec);
        std::int64_t now_ns = static_cast<std::int64_t>(now.tv_sec * NS_PER_SEC + (std::int64_t) now.tv_nsec);
        DWORD timeout_ms;
        if (abs_ns > now_ns) {
            std::int64_t delta_ns = abs_ns - now_ns;
            if (delta_ns >= static_cast<std::int64_t>(INFINITE * NS_PER_MS)) {
                timeout_ms = INFINITE - 1;
            } else {
                timeout_ms = static_cast<DWORD>((delta_ns + NS_PER_MS - 1) / NS_PER_MS);
            }
            if (timeout_ms == 0) {
                timeout_ms = 1;
            }
        } else {
            timeout_ms = 0;
        }
        BOOL ret = SleepConditionVariableSRW(&obj->cond, psrwlock, timeout_ms, 0);
        mutex->count = saved_count;
        mutex->thread_id = saved_tid;
        if (ret) {
            return thrd_result::success;
        }
        DWORD error = GetLastError();
        if (error == ERROR_TIMEOUT) {
            return thrd_result::timed_out;
        }
        if (error == ERROR_INVALID_PARAMETER) {
            return thrd_result::nomem;
        }
        return thrd_result::error;
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
