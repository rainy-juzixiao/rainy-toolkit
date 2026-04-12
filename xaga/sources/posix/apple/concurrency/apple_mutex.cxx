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
        pthread_t       thread_id{};
        int             count{0};
        pthread_cond_t  timed_cond{};
        pthread_mutex_t inner_mutex{};
        bool            locked{false};
    };

    static int macos_mutex_timedlock(mutex_handle *mutex, const ::timespec *abs_timeout) noexcept {
        pthread_mutex_lock(&mutex->inner_mutex);
        while (mutex->locked) {
            const int res = pthread_cond_timedwait(&mutex->timed_cond, &mutex->inner_mutex, abs_timeout);
            if (res == ETIMEDOUT) {
                pthread_mutex_unlock(&mutex->inner_mutex);
                return ETIMEDOUT;
            }
        }
        mutex->locked = true;
        pthread_mutex_unlock(&mutex->inner_mutex);
        return 0;
    }

    static void macos_mutex_signal(mutex_handle *mutex) noexcept {
        pthread_mutex_lock(&mutex->inner_mutex);
        if (mutex->locked) {
            mutex->locked = false;
            pthread_cond_signal(&mutex->timed_cond);
        }
        pthread_mutex_unlock(&mutex->inner_mutex);
    }

    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t current_thread_id = pthread_self();

        // 检查是否是递归锁且当前线程已持有锁
        const bool is_recursive = (mutex->type & mutex_types::recursive_mtx) == mutex_types::recursive_mtx;
        const bool same_thread = pthread_equal(mutex->thread_id, current_thread_id);

        // 如果是同一线程且是递归锁，直接增加计数
        if (same_thread && is_recursive) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }

        // 非递归锁或不同线程，需要实际获取锁
        int res = -1;

        if (!target) {
            // 阻塞锁
            res = pthread_mutex_lock(&mutex->handle);
        } else if (target->tv_sec == 0 && target->tv_nsec == 0) {
            // 尝试锁
            res = pthread_mutex_trylock(&mutex->handle);
        } else {
            // 带超时锁（macOS 模拟实现）
            res = macos_mutex_timedlock(mutex, target);
        }

        // 处理锁获取失败
        if (res != 0) {
            if (res == ETIMEDOUT) {
                errno = ETIMEDOUT;
                return thrd_result::timed_out;
            }
            errno = EBUSY;
            return thrd_result::busy;
        }

        // 成功获取锁，更新状态
        if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
            mutex->thread_id = current_thread_id;
        }

        return thrd_result::success;
    }

    thrd_result mtx_init(mtx_t *mtx, int flags) noexcept {
        if (!mtx) {
            return thrd_result::nomem;
        }
        auto *mutex     = static_cast<mutex_handle *>(*mtx);
        mutex->type     = flags;
        mutex->count    = 0;
        mutex->thread_id = {};
        mutex->locked   = false;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (flags & mutex_types::recursive_mtx) {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        } else {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        }
        pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);

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
        ::timespec ts{0, 0}; // NOLINT
        return mtx_do_lock(mtx, &ts);
    }

    thrd_result mtx_timedlock(mtx_t *mtx, const ::timespec *ts) noexcept {
        return mtx_do_lock(mtx, ts);
    }

    thrd_result mtx_unlock(mtx_t *const mtx) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);

        // 检查是否持有锁
        if (mutex->count == 0) {
            errno = EPERM;
            return thrd_result::error;
        }

        // 减少计数
        const int new_count = core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));

        // 只有当计数变为 0 时才真正释放底层锁
        if (new_count == 0) {
            mutex->thread_id = {};
            pthread_mutex_unlock(&mutex->handle);
            macos_mutex_signal(mutex);
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
        if (!mutex) {
            return thrd_result::nomem;
        }
        pthread_cond_destroy(&mutex->timed_cond);
        pthread_mutex_destroy(&mutex->inner_mutex);
        pthread_mutex_destroy(&mutex->handle);
        core::pal::deallocate(mutex, sizeof(mutex_handle), alignof(mutex_handle));
        *mtx = nullptr;
        return thrd_result::success;
    }

    void *native_mtx_handle(mtx_t *const mtx) noexcept {
        if (!mtx) {
            return nullptr;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        if (!mutex) {
            return nullptr;
        }
        return &mutex->handle;
    }
}