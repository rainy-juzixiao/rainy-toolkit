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
        pthread_mutex_t cond_mutex{};
        pthread_cond_t cond{};
        pthread_t owner{};
        int count{0};
        int type{0};
        bool locked{false};
        pthread_mutex_t handle{};
    };

    static int wait_for_lock(mutex_handle *mutex, const struct timespec *abs_timeout) noexcept {
        pthread_mutex_lock(&mutex->cond_mutex);
        while (mutex->locked) {
            if (abs_timeout == nullptr) { // NOLINT
                pthread_cond_wait(&mutex->cond, &mutex->cond_mutex);
            } else {
                int res = pthread_cond_timedwait(&mutex->cond, &mutex->cond_mutex, abs_timeout);
                if (res == ETIMEDOUT) {
                    pthread_mutex_unlock(&mutex->cond_mutex);
                    return ETIMEDOUT;
                }
            }
        }
        mutex->locked = true;
        pthread_mutex_unlock(&mutex->cond_mutex);
        return 0;
    }

    static void signal_unlock(mutex_handle *mutex) noexcept {
        pthread_mutex_lock(&mutex->cond_mutex);
        mutex->locked = false;
        pthread_cond_signal(&mutex->cond);
        pthread_mutex_unlock(&mutex->cond_mutex);
    }

    thrd_result mtx_do_lock(mtx_t *const mtx, const struct timespec *target) noexcept {
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        pthread_t self = pthread_self();
        const bool is_recursive = (mutex->type & mutex_types::recursive_mtx) != 0;

        pthread_mutex_lock(&mutex->cond_mutex);

        // 递归锁
        if (is_recursive && mutex->locked && pthread_equal(mutex->owner, self)) {
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            pthread_mutex_unlock(&mutex->cond_mutex);
            return thrd_result::success;
        }

        // try_lock（零超时）
        if (target != nullptr && target->tv_sec == 0 && target->tv_nsec == 0) {
            if (mutex->locked) {
                pthread_mutex_unlock(&mutex->cond_mutex);
                errno = EBUSY;
                return thrd_result::busy;
            }
            mutex->locked = true;
            mutex->owner = self;
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            pthread_mutex_unlock(&mutex->cond_mutex);
            return thrd_result::success;
        }

        // timed_lock（target 是绝对时间）
        if (target != nullptr) {
            while (mutex->locked) {
                int res = pthread_cond_timedwait(&mutex->cond, &mutex->cond_mutex, target);
                if (res == ETIMEDOUT) {
                    // 醒来后再检查一次，防止虚假超时
                    if (mutex->locked) {
                        pthread_mutex_unlock(&mutex->cond_mutex);
                        errno = ETIMEDOUT;
                        return thrd_result::timed_out;
                    }
                    break;
                }
            }
            mutex->locked = true;
            mutex->owner = self;
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            pthread_mutex_unlock(&mutex->cond_mutex);
            return thrd_result::success;
        }

        // 普通阻塞锁
        while (mutex->locked) {
            pthread_cond_wait(&mutex->cond, &mutex->cond_mutex);
        }
        mutex->locked = true;
        mutex->owner = self;
        core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
        pthread_mutex_unlock(&mutex->cond_mutex);
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
        mutex->locked = false;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (flags & mutex_types::recursive_mtx) {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        } else {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        }
        pthread_mutex_init(&mutex->handle, &attr);
        pthread_mutexattr_destroy(&attr);

        pthread_mutex_init(&mutex->cond_mutex, nullptr);
        pthread_cond_init(&mutex->cond, nullptr);
        return thrd_result::success;
    }

    thrd_result mtx_create(mtx_t *mtx, int flags) noexcept {
        if (!mtx)
            return thrd_result::nomem;
        *mtx = core::pal::allocate(sizeof(mutex_handle), alignof(mutex_handle));
        if (!*mtx)
            return thrd_result::nomem;
        return mtx_init(mtx, flags);
    }

    thrd_result mtx_lock(mtx_t *mtx) noexcept {
        return mtx_do_lock(mtx, nullptr);
    }

    thrd_result mtx_trylock(mtx_t *mtx) noexcept {
        struct timespec ts = {0, 0};
        return mtx_do_lock(mtx, &ts);
    }

    thrd_result mtx_timedlock(mtx_t *mtx, const struct timespec *ts) noexcept {
        return mtx_do_lock(mtx, ts);
    }

    thrd_result mtx_unlock(mtx_t* const mtx) noexcept {
        if (!mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto* mutex = static_cast<mutex_handle*>(*mtx);
        pthread_mutex_lock(&mutex->cond_mutex);
        if (mutex->count == 0) {
            pthread_mutex_unlock(&mutex->cond_mutex);
            errno = EPERM;
            return thrd_result::error;
        }
        if (const int new_count = core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count)); new_count == 0) {
            mutex->locked = false;
            mutex->owner  = {};
            pthread_cond_signal(&mutex->cond);
        }
        pthread_mutex_unlock(&mutex->cond_mutex);
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
        pthread_cond_destroy(&mutex->cond);
        pthread_mutex_destroy(&mutex->cond_mutex);
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
