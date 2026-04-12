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

namespace rainy::foundation::concurrency::implements {
    struct mutex_handle {
        pthread_mutex_t handle{};
        int type{0};
        pthread_t thread_id{};
        int count{0};
    };

    static int apple_mutex_timedlock(mutex_handle *mutex, const ::timespec *target) noexcept {
        constexpr long long min_sleep_ns = 100'000LL;
        constexpr long long max_sleep_ns = 5'000'000LL;
        long long sleep_ns = min_sleep_ns;
        rain_loop {
            int res = pthread_mutex_trylock(&mutex->handle);
            if (res == 0) {
                return 0;
            }
            if (res != EBUSY) {
                return res;
            }
            ::timespec now{};
            ::clock_gettime(CLOCK_REALTIME, &now);
            long long remaining_ns = (static_cast<long long>(target->tv_sec - now.tv_sec) * 1'000'000'000LL) +
                                     (static_cast<long long>(target->tv_nsec - now.tv_nsec));
            if (remaining_ns <= 0)
                return ETIMEDOUT;
            long long actual_sleep = std::min(sleep_ns, remaining_ns);
            ::timespec sleep_ts{0, static_cast<long>(actual_sleep)};
            ::nanosleep(&sleep_ts, nullptr);
            sleep_ns = std::min(sleep_ns * 2, max_sleep_ns);
        }
    }

    static int apple_mutex_recursive_timedlock(mutex_handle *mutex, const ::timespec *target) noexcept {
        const pthread_t self = pthread_self();
        constexpr long long min_sleep_ns = 100'000LL;
        constexpr long long max_sleep_ns = 5'000'000LL;
        long long sleep_ns = min_sleep_ns;
        while (true) {
            if (pthread_equal(mutex->thread_id, self)) {
                return 0;
            }
            ::timespec now{};
            ::clock_gettime(CLOCK_REALTIME, &now);
            long long remaining_ns = (static_cast<long long>(target->tv_sec - now.tv_sec) * 1'000'000'000LL) +
                                     (static_cast<long long>(target->tv_nsec - now.tv_nsec));
            if (remaining_ns <= 0) {
                return ETIMEDOUT;
            }
            const int res = pthread_mutex_trylock(&mutex->handle);
            if (res == 0) {
                mutex->thread_id = self;
                return 0;
            }
            if (res != EBUSY) {
                return res;
            }
            long long actual_sleep = std::min(std::max(sleep_ns, 1'000'000LL), remaining_ns);
            ::timespec sleep_ts{actual_sleep / 1'000'000'000LL, (actual_sleep % 1'000'000'000LL)};
            ::nanosleep(&sleep_ts, nullptr);
            sleep_ns = std::min(sleep_ns * 2, max_sleep_ns);
        }
    }


    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t current_thread_id = pthread_self();

        // 非递归普通 mutex
        if ((mutex->type & ~mutex_types::recursive_mtx) == mutex_types::plain_mtx) {
            if (!target) {
                return pthread_mutex_lock(&mutex->handle) == 0 ? thrd_result::success : thrd_result::error;
            }
            if (target->tv_sec == 0 && target->tv_nsec == 0) {
                return pthread_mutex_trylock(&mutex->handle) == 0 ? thrd_result::success : thrd_result::busy;
            }
            return apple_mutex_timedlock(mutex, target) == 0 ? thrd_result::success : thrd_result::timed_out;
        }

        // 递归 mutex / 需要超时支持
        int res = -1;
        if (!target) {
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
                res = pthread_mutex_lock(&mutex->handle);
            } else {
                res = 0;
            }
        } else if (target->tv_sec == 0 && target->tv_nsec == 0) {
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
                res = pthread_mutex_trylock(&mutex->handle);
            } else {
                res = 0;
            }
        } else {
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
                res = apple_mutex_recursive_timedlock(mutex, target);
            } else {
                res = 0;
            }
        }

        if (res == 0) {
            if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                mutex->thread_id = current_thread_id;
            }
            if ((mutex->count > 1) && ((mutex->type & mutex_types::recursive_mtx) != mutex_types::recursive_mtx)) {
                core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));
                pthread_mutex_unlock(&mutex->handle);
                errno = EBUSY;
                return thrd_result::busy;
            }
            return thrd_result::success;
        }

        if (res == ETIMEDOUT) {
            errno = ETIMEDOUT;
            return thrd_result::timed_out;
        }
        errno = EBUSY;
        return thrd_result::busy;
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        mutex->type = flags;
        mutex->count = 0;
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
        if (!mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        if (--mutex->count == 0) {
            mutex->thread_id = {};
            pthread_mutex_unlock(&mutex->handle);
        }
        return thrd_result::success;
    }

    bool mtx_current_owns(mtx_t *const mtx) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return false;
        }
        const auto *mutex = static_cast<mutex_handle *>(*mtx);
        return mutex->count != 0 && pthread_equal(mutex->thread_id, pthread_self());
    }

    thrd_result mtx_destroy(mtx_t *const mtx) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        pthread_mutex_destroy(&mutex->handle);
        core::pal::deallocate(mutex, sizeof(mutex_handle), alignof(mutex_handle));
        return thrd_result::success;
    }

    void *native_mtx_handle(mtx_t *const mtx) noexcept {
        if (!mtx) {
            return nullptr;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        return &mutex->handle;
    }
}
