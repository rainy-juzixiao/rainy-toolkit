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
#if RAINY_USING_MACOS
        // 由于 macOS 不支持 pthread_mutex_timedlock，因此，使用一个条件变量来模拟超时语义
        pthread_cond_t timed_cond{};
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
        int res = pthread_mutex_lock(&mutex->handle);
        if (res != 0) {
            return res;
        }
        while (mutex->locked) {
            res = pthread_cond_timedwait(&mutex->timed_cond, &mutex->handle, target);
            if (res == ETIMEDOUT) {
                pthread_mutex_unlock(&mutex->handle);
                return ETIMEDOUT;
            }
            if (res != 0) {
                pthread_mutex_unlock(&mutex->handle);
                return res;
            }
        }
        mutex->locked = true;
        return 0;
    }

    /**
     * 配套的 unlock，需要广播条件变量唤醒等待者。
     * 由 mtx_unlock 在 count 归零时调用，替代直接的 pthread_mutex_unlock。
     */
    static int apple_mutex_unlock(mutex_handle *mutex) noexcept {
        // handle 在 apple_mutex_timedlock 成功后仍处于锁定状态，这里先标记再广播
        mutex->locked = false;
        pthread_cond_broadcast(&mutex->timed_cond);
        return pthread_mutex_unlock(&mutex->handle);
    }
#endif

    thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept {
        if (!mtx) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        const pthread_t current_thread_id = pthread_self();
        // --- 非递归普通 mutex (plain_mtx) ---
        if ((mutex->type & ~mutex_types::recursive_mtx) == mutex_types::plain_mtx) {
            // 同线程重入直接增加计数
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
                pthread_mutex_lock(&mutex->handle);
                mutex->thread_id = current_thread_id;
            }
            core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count));
            return thrd_result::success;
        }
        // --- 递归 mutex / 需要超时支持 ---
        int res = -1;
        if (!target) {
            // 阻塞锁
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
                res = pthread_mutex_lock(&mutex->handle);
            } else {
                // 同线程递归 lock
                res = 0;
            }
        } else if (target->tv_sec == 0 && target->tv_nsec == 0) {
            // 尝试锁
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
                res = pthread_mutex_trylock(&mutex->handle);
            } else {
                res = 0;
            }
        } else {
            // 带超时锁
            if (!pthread_equal(mutex->thread_id, current_thread_id)) {
#if RAINY_USING_MACOS
                res = apple_mutex_timedlock(mutex, target);
#else
                res = pthread_mutex_timedlock(&mutex->handle, target);
#endif

            } else {
                res = 0;
            }
        }
        if (res == 0) {
            // 如果是第一次 lock，则设置 thread_id
            if (core::pal::interlocked_increment(reinterpret_cast<volatile long *>(&mutex->count)) == 1) {
                mutex->thread_id = current_thread_id;
            }

            // 非递归 mutex 被同线程重复 lock → 模拟 Windows 风格 BUSY
            if ((mutex->count > 1) && ((mutex->type & mutex_types::recursive_mtx) != mutex_types::recursive_mtx)) {
                core::pal::interlocked_decrement(reinterpret_cast<volatile long *>(&mutex->count));
#if RAINY_USING_MACOS
                apple_mutex_unlock(mutex);
#else
                pthread_mutex_unlock(&mutex->handle);
#endif
                errno = EBUSY;
                return thrd_result::busy;
            }

            return thrd_result::success;
        }
        // --- 处理失败返回 ---
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

#if RAINY_USING_MACOS
        mutex->locked = false;
        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        pthread_cond_init(&mutex->timed_cond, &cond_attr);
        pthread_condattr_destroy(&cond_attr);
#endif

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
        if (--mutex->count == 0) {
            mutex->thread_id = {};
#if RAINY_USING_MACOS
            apple_mutex_unlock(mutex);
#else
            pthread_mutex_unlock(&mutex->handle);
#endif
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
        // 移除对count的检查，不再保证 (若仍有线程持有该锁，行为未定义)
        pthread_mutex_destroy(&mutex->handle);
#if RAINY_USING_MACOS
        pthread_cond_destroy(&mutex->timed_cond);
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
