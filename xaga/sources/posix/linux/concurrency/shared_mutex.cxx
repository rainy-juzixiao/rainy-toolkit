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
        int result = pthread_rwlock_init(&handle->rwlock, nullptr);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
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
        int result = pthread_rwlock_wrlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
    }

    thrd_result smtx_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        int result = pthread_rwlock_rdlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
    }

    thrd_result smtx_try_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        int result = pthread_rwlock_trywrlock(&handle->rwlock);
        if (result == EBUSY) {
            return thrd_result::busy;
        }
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
    }

    thrd_result smtx_try_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        int result = pthread_rwlock_tryrdlock(&handle->rwlock);
        if (result == EBUSY) {
            return thrd_result::busy;
        }
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
    }

    thrd_result smtx_unlock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        int result = pthread_rwlock_unlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
    }

    thrd_result smtx_unlock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        // pthread_rwlock_unlock can unlock both read and write locks
        int result = pthread_rwlock_unlock(&handle->rwlock);
        if (result != 0) {
            errno = result;
            return thrd_result::error;
        }
        return thrd_result::success;
    }

    thrd_result smtx_destroy(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        int result = pthread_rwlock_destroy(&handle->rwlock);
        if (result != 0) {
            errno = result;
            // 继续处理
        }
        core::pal::deallocate(handle, sizeof(implements::shared_mutex_handle), alignof(implements::shared_mutex_handle));
        *smtx = nullptr;
        return thrd_result::success;
    }

    void *native_smtx_handle(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            return nullptr;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        return &handle->rwlock;
    }
}
