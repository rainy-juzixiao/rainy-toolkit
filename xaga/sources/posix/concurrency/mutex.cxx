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

#if RAINY_USING_MACOS
#include <dispatch/dispatch.h>
#endif

namespace rainy::foundation::concurrency::implements {
    struct mutex_handle {
        pthread_mutex_t handle{};
        int type{0};
        pthread_t thread_id{};
        int count{0};
#if RAINY_USING_MACOS
        dispatch_semaphore_t sem{};
        pthread_mutex_t meta{};
        bool locked{false};
#endif
    };


#if RAINY_USING_MACOS
    /**
     * macOS（无论 Intel 还是 Apple Silicon）均不提供 pthread_mutex_timedlock，
     * 因此借助 pthread_cond_timedwait 在循环中等待直到超时或成功获取锁。
     *
     * 在 Apple Silicon (RAINY_USING_MACOS_AND_IS_APPLE_SILICON) 上，
     * pthread_cond_timedwait 同样使用 CLOCK_REALTIME 语义的绝对时间，
     * 行为与 Intel macOS 一致，无需额外区分。
     *
     * @param mutex   已分配并初始化完毕的 mutex_handle 指针
     * @param target  超时绝对时间（CLOCK_REALTIME），调用方保证非空且非 {0,0}
     * @return        0 表示成功，ETIMEDOUT 表示超时，其他值表示系统错误
     */
    static int apple_mutex_timedlock(mutex_handle *mutex, const ::timespec *target) noexcept {
        constexpr long long min_sleep_ns = 100'000LL;
        constexpr long long max_sleep_ns = 5'000'000LL;
        long long sleep_ns = min_sleep_ns;
        while (true) {
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
            if (remaining_ns <= 0) {
                return ETIMEDOUT;
            }
            long long actual_sleep = (core::min) (sleep_ns, remaining_ns);
            ::timespec sleep_ts{0, static_cast<long>(actual_sleep)};
            ::nanosleep(&sleep_ts, nullptr);
            sleep_ns = (core::min) (sleep_ns * 2, max_sleep_ns);
        }
    }
#endif

    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t current_thread_id = pthread_self();
        const bool is_recursive = (mutex->type & mutex_types::recursive_mtx) == mutex_types::recursive_mtx;
#if RAINY_USING_MACOS
        pthread_mutex_lock(&mutex->meta);
        if (is_recursive && mutex->count > 0 && pthread_equal(mutex->thread_id, current_thread_id)) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            pthread_mutex_unlock(&mutex->meta);
            return thrd_result::success;
        }
        pthread_mutex_unlock(&mutex->meta);
        long wait_result;
        if (!target) {
            wait_result = dispatch_semaphore_wait(mutex->sem, DISPATCH_TIME_FOREVER);
        } else if (target->tv_sec == 0 && target->tv_nsec == 0) {
            wait_result = dispatch_semaphore_wait(mutex->sem, DISPATCH_TIME_NOW);
            if (wait_result != 0) {
                errno = EBUSY;
                return thrd_result::busy;
            }
        } else {
            dispatch_time_t due = dispatch_walltime(target, 0);
            wait_result = dispatch_semaphore_wait(mutex->sem, due);
            if (wait_result != 0) {
                errno = ETIMEDOUT;
                return thrd_result::timed_out;
            }
        }
        pthread_mutex_lock(&mutex->meta);
        mutex->locked = true;
        mutex->thread_id = current_thread_id;
        core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
        pthread_mutex_unlock(&mutex->meta);
        return thrd_result::success;

#else
        const bool is_same_thread = pthread_equal(mutex->thread_id, current_thread_id);
        if (is_same_thread && is_recursive && mutex->count > 0) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }
        int res = -1;
        if (!target) {
            res = pthread_mutex_lock(&mutex->handle);
        } else if (target->tv_sec == 0 && target->tv_nsec == 0) {
            res = pthread_mutex_trylock(&mutex->handle);
            if (res == EBUSY) {
                errno = EBUSY;
                return thrd_result::busy;
            }
        } else {
            res = pthread_mutex_timedlock(&mutex->handle, target);
        }
        if (res == 0) {
            if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                mutex->thread_id = current_thread_id;
            }
            if (!is_recursive && mutex->count > 1) {
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
        if (res == EBUSY) {
            errno = EBUSY;
            return thrd_result::busy;
        }
        errno = res;
        return thrd_result::error;
#endif
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx)
            return thrd_result::nomem;
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        mutex->type = flags;
        mutex->count = 0;
        mutex->thread_id = {};

#if RAINY_USING_MACOS
        mutex->locked = false;
        mutex->sem = dispatch_semaphore_create(1); // 初始值 1，相当于未锁
        if (!mutex->sem)
            return thrd_result::error;
        pthread_mutex_init(&mutex->meta, nullptr);
        // handle 仍初始化供 native_handle() 使用
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, flags & mutex_types::recursive_mtx ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK);
        int res = pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);
        return res == 0 ? thrd_result::success : thrd_result::error;
#else
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, flags & mutex_types::recursive_mtx ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
        int res = pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);
        return res == 0 ? thrd_result::success : thrd_result::error;
#endif
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
        ::timespec ts{0, 0}; // NOLINT
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
#if RAINY_USING_MACOS
        pthread_mutex_lock(&mutex->meta);
        if (mutex->count == 0) {
            pthread_mutex_unlock(&mutex->meta);
            errno = EPERM;
            return thrd_result::error;
        }
        const long new_count = core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));
        if (new_count == 0) {
            mutex->locked = false;
            mutex->thread_id = {};
            pthread_mutex_unlock(&mutex->meta);
            dispatch_semaphore_signal(mutex->sem);
            return thrd_result::success;
        }
        pthread_mutex_unlock(&mutex->meta);
        return thrd_result::success;
#else
        if (mutex->count == 0) {
            errno = EPERM;
            return thrd_result::error;
        }
        const long new_count = core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));
        if (new_count == 0) {
            mutex->thread_id = {};
            return pthread_mutex_unlock(&mutex->handle) == 0 ? thrd_result::success : thrd_result::error;
        }
        return thrd_result::success;
#endif
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
#if RAINY_USING_MACOS
        dispatch_release(mutex->sem);
        pthread_mutex_destroy(&mutex->meta);
#endif
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
