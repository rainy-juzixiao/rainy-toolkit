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
        int             type{0};
        pthread_t       thread_id{};   // 当前持有锁的线程 ID
        int             count{0};      // 递归计数
        pthread_cond_t  timed_cond{};  // 条件变量，用于等待锁释放
        pthread_mutex_t inner_mutex{}; // 保护 locked 和 count 的内部互斥量
        bool            locked{false}; // 锁是否被持有
    };

    static int custom_mutex_timedlock(mutex_handle *mutex, const ::timespec *abs_timeout) noexcept {
        pthread_mutex_lock(&mutex->inner_mutex);
        while (mutex->locked) {
            if (abs_timeout == nullptr) {
                pthread_cond_wait(&mutex->timed_cond, &mutex->inner_mutex);
            } else {
                int res = pthread_cond_timedwait(&mutex->timed_cond, &mutex->inner_mutex, abs_timeout);
                if (res == ETIMEDOUT) {
                    pthread_mutex_unlock(&mutex->inner_mutex);
                    return ETIMEDOUT;
                }
            }
        }
        mutex->locked = true;
        pthread_mutex_unlock(&mutex->inner_mutex);
        return 0;
    }

    // 内部函数：释放锁（仅当计数归零时调用）
    static void custom_mutex_unlock(mutex_handle *mutex) noexcept {
        pthread_mutex_lock(&mutex->inner_mutex);
        mutex->locked = false;
        pthread_cond_signal(&mutex->timed_cond);
        pthread_mutex_unlock(&mutex->inner_mutex);
    }

    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx || !*mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t current_thread_id = pthread_self();
        const bool is_recursive = (mutex->type & mutex_types::recursive_mtx) == mutex_types::recursive_mtx;

        // 递归锁且当前线程已持有：直接增加计数
        if (is_recursive && pthread_equal(mutex->thread_id, current_thread_id)) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }

        // 准备绝对超时时间（仅当 target 非空且非零时）
        ::timespec abs_timeout;
        const ::timespec *timeout_ptr = nullptr;
        bool is_trylock = false;

        if (target != nullptr) {
            if (target->tv_sec == 0 && target->tv_nsec == 0) {
                // try_lock：使用当前时间作为绝对时间，实现零等待
                clock_gettime(CLOCK_REALTIME, &abs_timeout);
                timeout_ptr = &abs_timeout;
                is_trylock = true;
            } else {
                clock_gettime(CLOCK_REALTIME, &abs_timeout);
                abs_timeout.tv_sec += target->tv_sec;
                abs_timeout.tv_nsec += target->tv_nsec;
                if (abs_timeout.tv_nsec >= 1000000000) {
                    abs_timeout.tv_sec += 1;
                    abs_timeout.tv_nsec -= 1000000000;
                }
                timeout_ptr = &abs_timeout;
            }
        }

        int res = custom_mutex_timedlock(mutex, timeout_ptr);
        if (res != 0) {
            if (res == ETIMEDOUT) {
                errno = is_trylock ? EBUSY : ETIMEDOUT;
                return is_trylock ? thrd_result::busy : thrd_result::timed_out;
            }
            errno = EBUSY;
            return thrd_result::busy;
        }

        // 成功获取锁
        if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
            mutex->thread_id = current_thread_id;
        }
        return thrd_result::success;
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        mutex->type     = flags;
        mutex->count    = 0;
        mutex->thread_id = {};
        mutex->locked   = false;

        // 初始化底层 mutex（仅用于 native_handle，不参与同步）
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (flags & mutex_types::recursive_mtx) {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        } else {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        }
        pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);

        // 初始化内部同步原语
        pthread_mutex_init(&mutex->inner_mutex, nullptr);
        pthread_cond_init(&mutex->timed_cond, nullptr);
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
            custom_mutex_unlock(mutex);
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
        pthread_cond_destroy(&mutex->timed_cond);
        pthread_mutex_destroy(&mutex->inner_mutex);
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