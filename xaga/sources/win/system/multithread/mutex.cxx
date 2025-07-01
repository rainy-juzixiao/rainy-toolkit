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
#include <rainy/winapi/system/multithread/mutex.h>
#include <rainy/foundation/diagnostics/contract.hpp>

namespace rainy::winapi::system::multithread::implements {
    RAINY_INLINE PSRWLOCK get_srw_lock(mtx_t *mtx) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return nullptr;
        }
        return reinterpret_cast<PSRWLOCK>(&reinterpret_cast<mutex_handle *>(*mtx)->handle);
    }

    RAINY_INLINE win32_thread_result mtx_do_lock(mtx_t *mtx, const ::timespec *target) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return win32_thread_result::nomem;
        }
        rainy_let mutex = reinterpret_cast<mutex_handle *>(*mtx);
        const auto current_thread_id = static_cast<unsigned long>(GetCurrentThreadId());
        if ((mutex->type & ~mutex_types::recursive_mtx) == mutex_types::plain_mtx) {
            if (mutex->thread_id != current_thread_id) {
                AcquireSRWLockExclusive(get_srw_lock(mtx));
                mutex->thread_id = current_thread_id;
            }
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return win32_thread_result::success;
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
            return win32_thread_result::success;
        }
        if (!target || (target->tv_sec == 0 && target->tv_nsec == 0)) {
            errno = EBUSY;
            return win32_thread_result::busy;
        }
        errno = ETIMEDOUT;
        return win32_thread_result::timed_out;
    }
}

namespace rainy::winapi::system::multithread {
    RAINY_INLINE win32_thread_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx) {
            return win32_thread_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        mutex->type = flags;
        mutex->count = 0;
        mutex->handle = nullptr;
        mutex->thread_id = 0;
        return win32_thread_result::success;
    }

    RAINY_INLINE win32_thread_result mtx_create(mtx_t *mtx, int flags) noexcept {
        if (!mtx) {
            return win32_thread_result::nomem;
        }
        *mtx = core::pal::allocate(sizeof(implements::mutex_handle), alignof(implements::mutex_handle));
        mtx_init(mtx, flags);
        return win32_thread_result::success;
    }

    RAINY_INLINE win32_thread_result mtx_lock(mtx_t *mtx) noexcept {
        if (!mtx) {
            return win32_thread_result::nomem;
        }
        return implements::mtx_do_lock(mtx, nullptr);
    }

    RAINY_INLINE win32_thread_result mtx_trylock(mtx_t *mtx) noexcept {
        if (!mtx) {
            return win32_thread_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        utility::expects((mutex->type & (mutex_types::try_mtx | mutex_types::timed_mtx)) != 0, "trylock not supported by mutex");
        ::timespec xt{};
        xt.tv_sec = 0;
        xt.tv_nsec = 0;
        return implements::mtx_do_lock(mtx, &xt);
    }

    RAINY_INLINE win32_thread_result mtx_unlock(mtx_t *mtx) noexcept {
        if (!mtx) {
            return win32_thread_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        if (--mutex->count == 0) {
            mutex->thread_id = 0;
            auto* srw_lock = implements::get_srw_lock(mtx);
            _Analysis_assume_lock_held_(*srw_lock);
            ReleaseSRWLockExclusive(srw_lock);
        }
        return win32_thread_result::success;
    }

    RAINY_INLINE win32_thread_result mtx_timedlock(mtx_t *mtx, const ::timespec *xt) noexcept {
        if (!mtx || !xt) {
            errno = EINVAL;
            return win32_thread_result::nomem;
        }
        win32_thread_result res{};
        res = implements::mtx_do_lock(mtx, xt);
        return res == win32_thread_result::busy ? win32_thread_result::timed_out : res;
    }

    RAINY_INLINE bool mtx_current_owns(mtx_t *mtx) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return false;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        return mutex->count != 0 && mutex->thread_id == GetCurrentThreadId();
    }

    RAINY_INLINE win32_thread_result mtx_destroy(mtx_t *mtx, bool release) noexcept {
        if (!mtx) {
            return win32_thread_result::nomem;
        }
        rainy_let mutex = static_cast<implements::mutex_handle *>(*mtx);
        utility::expects(mutex->count == 0, "mutex destroyed while busy");
        if (release) {
            core::pal::deallocate(mutex, sizeof(implements::mutex_handle), alignof(implements::mutex_handle));
            *mtx = nullptr;
        }
        return win32_thread_result::success;
    }
}