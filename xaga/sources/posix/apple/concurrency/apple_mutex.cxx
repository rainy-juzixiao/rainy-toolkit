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
#include <errno.h>
#include <pthread.h>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/pal.hpp>
#include <time.h>

namespace rainy::foundation::concurrency::implements {
    struct mutex_handle {
        pthread_mutex_t handle{};
        pthread_t owner{};
        int count{0};
        int type{0};
    };

    static int apple_mutex_timedlock(mutex_handle *mutex, const ::timespec *target) noexcept {
        if (pthread_equal(mutex->owner, pthread_self()) && mutex->count > 0) {
            return EDEADLK;
        }
        constexpr long long min_sleep_ns = 100'000LL;
        constexpr long long max_sleep_ns = 5'000'000LL;
        long long sleep_ns = min_sleep_ns;
        ::timespec now{};
        ::clock_gettime(CLOCK_REALTIME, &now);
        long long deadline_ns =
            (static_cast<long long>(target->tv_sec)  * 1'000'000'000LL + target->tv_nsec) -
            (static_cast<long long>(now.tv_sec)       * 1'000'000'000LL + now.tv_nsec);

        if (deadline_ns <= 0) {
            return ETIMEDOUT;
        }
        ::timespec start{};
        ::clock_gettime(CLOCK_MONOTONIC, &start);
        const long long start_ns = static_cast<long long>(start.tv_sec) * 1'000'000'000LL + start.tv_nsec;
        while (true) {
            int res = pthread_mutex_trylock(&mutex->handle);
            if (res == 0) return 0;
            if (res != EBUSY) return res;

            ::timespec mono_now{};
            ::clock_gettime(CLOCK_MONOTONIC, &mono_now);
            long long elapsed_ns =
                (static_cast<long long>(mono_now.tv_sec)  * 1'000'000'000LL + mono_now.tv_nsec) - start_ns;

            if (elapsed_ns >= deadline_ns) {
                return ETIMEDOUT;
            }

            long long remaining_ns = deadline_ns - elapsed_ns;
            long long actual_sleep = std::min(sleep_ns, remaining_ns);
            ::timespec sleep_ts{0, static_cast<long>(actual_sleep)};
            ::nanosleep(&sleep_ts, nullptr);
            sleep_ns = std::min(sleep_ns * 2, max_sleep_ns);
        }
    }

    thrd_result mtx_do_lock(mtx_t *const mtx, const struct timespec *target) noexcept {
        if (!mtx || !*mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t self = pthread_self();
        const bool is_recursive = (mutex->type & mutex_types::recursive_mtx) != 0;
        if (is_recursive && pthread_equal(mutex->owner, self)) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }
        // try_lock（零超时）
        if (target != nullptr && target->tv_sec == 0 && target->tv_nsec == 0) {
            if (!is_recursive && mutex->count > 0 && pthread_equal(mutex->owner, self)) {
                errno = EBUSY;
                return thrd_result::busy;
            }
            if (const int res = pthread_mutex_trylock(&mutex->handle); res == 0) {
                if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                    mutex->owner = self;
                }
                return thrd_result::success;
            }
            errno = EBUSY;
            return thrd_result::busy;
        }

        // timed_lock（指数退避轮询）
        if (target != nullptr) {
            int res = apple_mutex_timedlock(mutex, target);
            if (res == ETIMEDOUT) {
                errno = ETIMEDOUT;
                return thrd_result::timed_out;
            }
            if (res != 0) {
                errno = EBUSY;
                return thrd_result::busy;
            }
            if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                mutex->owner = self;
            }
            return thrd_result::success;
        }
        // 普通阻塞锁
        int res = pthread_mutex_lock(&mutex->handle);
        if (res != 0) {
            errno = EBUSY;
            return thrd_result::busy;
        }
        if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
            mutex->owner = self;
        }
        return thrd_result::success;
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        mutex->type = flags;
        mutex->owner = {};
        mutex->count = 0;
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
        struct timespec ts{0, 0};
        return mtx_do_lock(mtx, &ts);
    }

    thrd_result mtx_timedlock(mtx_t *mtx, const struct timespec *ts) noexcept {
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
        if (const int new_count = core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count)); new_count == 0) {
            mutex->owner = {};
            pthread_mutex_unlock(&mutex->handle);
        }
        return thrd_result::success;
    }

    bool mtx_current_owns(mtx_t *const mtx) noexcept {
        if (!mtx || !*mtx) {
            return false;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        return mutex->count != 0 && pthread_equal(mutex->owner, pthread_self());
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
