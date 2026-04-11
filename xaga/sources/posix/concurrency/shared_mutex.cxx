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
#include <cerrno>
#include <chrono>
#include <pthread.h>
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency::implements {
    struct shared_mutex_handle {
        pthread_rwlock_t rwlock;
#if RAINY_USING_MACOS
        pthread_mutex_t internal_mutex;
        pthread_cond_t read_cond;
        pthread_cond_t write_cond;
        int active_readers;
        int waiting_writers;
        bool has_writer;
        pthread_t writer_thread_id;
        int write_recursion_count;
#endif
    };

    thrd_result smtx_init(smtx_t *const smtx) noexcept {
        if (smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        if (handle == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }

#if RAINY_USING_MACOS
        // macOS: 使用自定义实现
        pthread_mutex_init(&handle->internal_mutex, nullptr);
        pthread_cond_init(&handle->read_cond, nullptr);
        pthread_cond_init(&handle->write_cond, nullptr);
        handle->active_readers = 0;
        handle->waiting_writers = 0;
        handle->has_writer = false;
        handle->writer_thread_id = {};
        handle->write_recursion_count = 0;
        return thrd_result::success;
#else
        int result = pthread_rwlock_init(&handle->rwlock, nullptr);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_create(smtx_t *const smtx) noexcept {
        if (smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        *smtx = core::pal::allocate(sizeof(implements::shared_mutex_handle), alignof(implements::shared_mutex_handle));
        return smtx_init(smtx);
    }

    thrd_result smtx_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        pthread_t current_thread = pthread_self();
        pthread_mutex_lock(&handle->internal_mutex);

        // 检查是否当前线程已经持有写锁（递归写锁）
        if (handle->has_writer && pthread_equal(handle->writer_thread_id, current_thread)) {
            handle->write_recursion_count++;
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::success;
        }

        // 等待直到没有读者且没有写者
        while (handle->has_writer || handle->active_readers > 0) {
            handle->waiting_writers++;
            pthread_cond_wait(&handle->write_cond, &handle->internal_mutex);
            handle->waiting_writers--;
        }

        // 获取写锁
        handle->has_writer = true;
        handle->writer_thread_id = current_thread;
        handle->write_recursion_count = 1;
        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::success;
#else
        int result = pthread_rwlock_wrlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_timed_lock(smtx_t *const smtx, const ::timespec *timeout) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        if (timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::error;
        }
        pthread_mutex_lock(&handle->internal_mutex);
        pthread_t current_thread = pthread_self();
        if (handle->has_writer && pthread_equal(handle->writer_thread_id, current_thread)) {
            handle->write_recursion_count++;
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::success;
        }
        bool timed_out = false;
        while (handle->has_writer || handle->active_readers > 0) {
            handle->waiting_writers++;
            int ret = pthread_cond_timedwait(&handle->write_cond, &handle->internal_mutex, timeout);
            handle->waiting_writers--;
            if (ret == ETIMEDOUT) {
                timed_out = true;
                break;
            }
            if (ret != 0) {
                pthread_mutex_unlock(&handle->internal_mutex);
                errno = ret;
                return thrd_result::error;
            }
        }

        if (timed_out) {
            pthread_mutex_unlock(&handle->internal_mutex);
            errno = ETIMEDOUT;
            return thrd_result::timed_out;
        }

        handle->has_writer = true;
        handle->writer_thread_id = current_thread;
        handle->write_recursion_count = 1;
        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::success;
#else
        if (timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::error;
        }
        int result = pthread_rwlock_timedwrlock(&handle->rwlock, timeout);
        if (result == ETIMEDOUT) {
            errno = ETIMEDOUT;
            return thrd_result::timed_out;
        }
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_timed_lock_shared(smtx_t *const smtx, const ::timespec *timeout) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
#if RAINY_USING_MACOS
        if (timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::error;
        }
        pthread_mutex_lock(&handle->internal_mutex);
        pthread_t current_thread = pthread_self();
        if (handle->has_writer && pthread_equal(handle->writer_thread_id, current_thread)) {
            handle->write_recursion_count++;
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::success;
        }
        handle->waiting_writers++;
        bool timed_out = false;
        while (handle->has_writer || handle->active_readers > 0) {
            int ret = pthread_cond_timedwait(&handle->write_cond, &handle->internal_mutex, timeout);
            if (ret == ETIMEDOUT) {
                timed_out = true;
                break;
            }
            ::timespec now{};
            ::clock_gettime(CLOCK_REALTIME, &now);
            if (now.tv_sec > timeout->tv_sec ||
                (now.tv_sec == timeout->tv_sec && now.tv_nsec >= timeout->tv_nsec)) {
                timed_out = true;
                break;
                }
        }
        handle->waiting_writers--;
        if (timed_out) {
            pthread_mutex_unlock(&handle->internal_mutex);
            errno = ETIMEDOUT;
            return thrd_result::timed_out;
        }
        handle->has_writer = true;
        handle->writer_thread_id = current_thread;
        handle->write_recursion_count = 1;
        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::success;
#else
        if (timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::error;
        }
        const int result = pthread_rwlock_timedrdlock(&handle->rwlock, timeout);
        if (result == ETIMEDOUT) {
            errno = ETIMEDOUT;
            return thrd_result::timed_out;
        }
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
#if RAINY_USING_MACOS
        pthread_mutex_lock(&handle->internal_mutex);
        pthread_t current_thread = pthread_self();
        if (handle->has_writer && pthread_equal(handle->writer_thread_id, current_thread)) {
            pthread_mutex_unlock(&handle->internal_mutex);
            errno = EDEADLK;
            return thrd_result::error;
        }

        while (handle->has_writer || handle->waiting_writers > 0) {
            pthread_cond_wait(&handle->read_cond, &handle->internal_mutex);
        }

        handle->active_readers++;
        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::success;
#else
        const int result = pthread_rwlock_rdlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_try_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        pthread_mutex_lock(&handle->internal_mutex);
        pthread_t current_thread = pthread_self();
        if (handle->has_writer && pthread_equal(handle->writer_thread_id, current_thread)) {
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::busy; // 返回 busy，不允许递归写锁
        }
        if (!handle->has_writer && handle->active_readers == 0) {
            handle->has_writer = true;
            handle->writer_thread_id = current_thread;
            handle->write_recursion_count = 1;
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::success;
        }

        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::busy;
#else
        const int result = pthread_rwlock_trywrlock(&handle->rwlock);
        if (result == EBUSY) {
            return thrd_result::busy;
        }
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_try_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        pthread_mutex_lock(&handle->internal_mutex);
        pthread_t current_thread = pthread_self();
        if (handle->has_writer && pthread_equal(handle->writer_thread_id, current_thread)) {
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::busy; // 返回 busy 而不是 success
        }
        if (!handle->has_writer && handle->waiting_writers == 0) {
            handle->active_readers++;
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::success;
        }
        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::busy;
#else
        const int result = pthread_rwlock_tryrdlock(&handle->rwlock);
        if (result == EBUSY) {
            return thrd_result::busy;
        }
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_unlock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        pthread_mutex_lock(&handle->internal_mutex);

        // 递减递归计数
        if (handle->write_recursion_count > 1) {
            handle->write_recursion_count--;
            pthread_mutex_unlock(&handle->internal_mutex);
            return thrd_result::success;
        }

        // 释放写锁
        handle->has_writer = false;
        handle->writer_thread_id = {};
        handle->write_recursion_count = 0;

        // 优先唤醒等待的写者，然后是读者
        if (handle->waiting_writers > 0) {
            pthread_cond_signal(&handle->write_cond);
        } else {
            pthread_cond_broadcast(&handle->read_cond);
        }

        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::success;
#else
        const int result = pthread_rwlock_unlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_unlock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        pthread_mutex_lock(&handle->internal_mutex);

        handle->active_readers--;

        // 当最后一个读者释放锁时，唤醒等待的写者
        if (handle->active_readers == 0 && handle->waiting_writers > 0) {
            pthread_cond_signal(&handle->write_cond);
        }

        pthread_mutex_unlock(&handle->internal_mutex);
        return thrd_result::success;
#else
        const int result = pthread_rwlock_unlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
#endif
    }

    thrd_result smtx_destroy(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

#if RAINY_USING_MACOS
        pthread_mutex_destroy(&handle->internal_mutex);
        pthread_cond_destroy(&handle->read_cond);
        pthread_cond_destroy(&handle->write_cond);
#else
        pthread_rwlock_destroy(&handle->rwlock);
#endif

        core::pal::deallocate(handle, sizeof(implements::shared_mutex_handle), alignof(implements::shared_mutex_handle));
        *smtx = nullptr;
        return thrd_result::success;
    }

    void *native_smtx_handle(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            return nullptr;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
#if RAINY_USING_MACOS
        return &handle->internal_mutex;
#else
        return &handle->rwlock;
#endif
    }
}
