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
#include <pthread.h>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/pal.hpp>
#include <time.h>
#include <errno.h>

namespace rainy::foundation::concurrency::implements {
    struct mutex_handle {
        pthread_mutex_t handle{};
        int             type{0};
        pthread_t       thread_id{};
        int             count{0};
    };

    // 将相对时间转换为绝对时间
    static void to_absolute_timespec(const ::timespec *rel, ::timespec *abs) {
        clock_gettime(CLOCK_REALTIME, abs);
        abs->tv_sec += rel->tv_sec;
        abs->tv_nsec += rel->tv_nsec;
        if (abs->tv_nsec >= 1000000000) {
            abs->tv_sec += 1;
            abs->tv_nsec -= 1000000000;
        }
    }

    // 检查绝对时间是否已过期
    static bool is_absolute_timeout(const ::timespec *abs) {
        ::timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        if (now.tv_sec > abs->tv_sec) return true;
        if (now.tv_sec < abs->tv_sec) return false;
        return now.tv_nsec >= abs->tv_nsec;
    }

    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx || !*mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t current_thread_id = pthread_self();
        const bool is_recursive = (mutex->type & mutex_types::recursive_mtx) == mutex_types::recursive_mtx;

        // 递归锁且当前线程已持有：增加计数
        if (is_recursive && pthread_equal(mutex->thread_id, current_thread_id)) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }

        // 处理 try_lock（零超时）
        if (target != nullptr && target->tv_sec == 0 && target->tv_nsec == 0) {
            int res = pthread_mutex_trylock(&mutex->handle);
            if (res == 0) {
                if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                    mutex->thread_id = current_thread_id;
                }
                return thrd_result::success;
            }
            errno = EBUSY;
            return thrd_result::busy;
        }

        // 处理 timed_lock（使用轮询 + 小睡）
        if (target != nullptr) {
            ::timespec abs_timeout;
            to_absolute_timespec(target, &abs_timeout);
            while (!is_absolute_timeout(&abs_timeout)) {
                int res = pthread_mutex_trylock(&mutex->handle);
                if (res == 0) {
                    if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                        mutex->thread_id = current_thread_id;
                    }
                    return thrd_result::success;
                }
                // 休眠 1ms 避免忙等
                timespec sleep_time = {0, 1000000};
                nanosleep(&sleep_time, nullptr);
            }
            errno = ETIMEDOUT;
            return thrd_result::timed_out;
        }

        // 普通 lock（阻塞）
        int res = pthread_mutex_lock(&mutex->handle);
        if (res == 0) {
            if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                mutex->thread_id = current_thread_id;
            }
            return thrd_result::success;
        }
        errno = EBUSY;
        return thrd_result::busy;
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        mutex->type     = flags;
        mutex->count    = 0;
        mutex->thread_id = {};

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (flags & mutex_types::recursive_mtx) {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        } else {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        }
        pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);
        return thrd_result::success;
    }

    thrd_result mtx_create(mtx_t *mtx, const int flags) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        *mtx = core::pal::allocate(sizeof(mutex_handle), alignof(mutex_handle));
        if (!*mtx) {
            return thrd_result::nomem;
        }
        return mtx_init(mtx, flags);
    }

    thrd_result mtx_lock(mtx_t *mtx) noexcept {
        return mtx_do_lock(mtx, nullptr);
    }

    thrd_result mtx_trylock(mtx_t *mtx) noexcept {
        ::timespec ts{0, 0};
        return mtx_do_lock(mtx, &ts);
    }

    thrd_result mtx_timedlock(mtx_t *mtx, const ::timespec *ts) noexcept {
        return mtx_do_lock(mtx, ts);
    }

    thrd_result mtx_unlock(mtx_t *const mtx) noexcept {
        if (!mtx || !*mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        if (mutex->count == 0) {
            errno = EPERM;
            return thrd_result::error;
        }
        const int new_count = core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));
        if (new_count == 0) {
            mutex->thread_id = {};
            pthread_mutex_unlock(&mutex->handle);
        }
        return thrd_result::success;
    }

    bool mtx_current_owns(mtx_t *const mtx) noexcept {
        if (!mtx || !*mtx) {
            errno = EINVAL;
            return false;
        }
        const auto *mutex = static_cast<mutex_handle *>(*mtx);
        return mutex->count != 0 && pthread_equal(mutex->thread_id, pthread_self());
    }

    thrd_result mtx_destroy(mtx_t *const mtx) noexcept {
        if (!mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        pthread_mutex_destroy(&mutex->handle);
        core::pal::deallocate(mutex, sizeof(mutex_handle), alignof(mutex_handle));
        *mtx = nullptr;
        return thrd_result::success;
    }

    void *native_mtx_handle(mtx_t *const mtx) noexcept {
        if (!mtx || !*mtx) {
            return nullptr;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        return &mutex->handle;
    }
}