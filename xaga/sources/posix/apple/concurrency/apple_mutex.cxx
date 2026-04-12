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

    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx || !*mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);

        int res = 0;
        if (!target) {
            // 阻塞锁
            res = pthread_mutex_lock(&mutex->handle);
        } else if (target->tv_sec == 0 && target->tv_nsec == 0) {
            // 尝试锁
            res = pthread_mutex_trylock(&mutex->handle);
        } else {
            // 超时锁：直接使用 pthread_mutex_timedlock（macOS 支持）
            res = pthread_mutex_timedlock(&mutex->handle, target);
        }

        if (res != 0) {
            if (res == ETIMEDOUT) {
                errno = ETIMEDOUT;
                return thrd_result::timed_out;
            }
            errno = EBUSY;
            return thrd_result::busy;
        }

        // 成功获取锁，更新自定义计数和线程ID（用于 mtx_current_owns）
        if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
            mutex->thread_id = pthread_self();
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

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (flags & mutex_types::recursive_mtx) {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        } else {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        }
        pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);

        // 以下初始化保留仅为兼容，实际不再使用
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
        if (core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count)) == 0) {
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