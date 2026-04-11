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
#include <rainy/foundation/concurrency/pal.hpp>
#include <windows.h>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26110)
#endif

namespace rainy::foundation::concurrency::implements {
    struct shared_mutex_handle {
        SRWLOCK srwlock;
        mtx_t wait_mutex;
        cnd_t wait_cnd;
        bool write_waiting;
        int read_waiting;
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
        InitializeSRWLock(&handle->srwlock);
        thrd_result result = mtx_init(&handle->wait_mutex, mutex_types::plain_mtx | mutex_types::try_mtx | mutex_types::timed_mtx);
        if (result != thrd_result::success) {
            return result;
        }

        result = cnd_init(&handle->wait_cnd);
        if (result != thrd_result::success) {
            mtx_destroy(&handle->wait_mutex);
            return result;
        }

        handle->write_waiting = false;
        handle->read_waiting = 0;

        return thrd_result::success;
    }

    thrd_result smtx_create(smtx_t *const smtx) noexcept {
        if (smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        *smtx = core::pal::allocate(sizeof(implements::shared_mutex_handle), alignof(implements::shared_mutex_handle));
        if (*smtx == nullptr) {
            return thrd_result::nomem;
        }
        return smtx_init(smtx);
    }

    thrd_result smtx_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        mtx_lock(&handle->wait_mutex);
        handle->write_waiting = true;
        while (handle->read_waiting > 0) {
            cnd_wait(&handle->wait_cnd, &handle->wait_mutex);
        }
        mtx_unlock(&handle->wait_mutex);
        AcquireSRWLockExclusive(&handle->srwlock);
        return thrd_result::success;
    }

    thrd_result smtx_timed_lock(smtx_t *const smtx, const ::timespec *timeout) noexcept {
        if (smtx == nullptr || *smtx == nullptr || timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        mtx_lock(&handle->wait_mutex);
        handle->write_waiting = true;
        while (handle->read_waiting > 0) {
            thrd_result result = cnd_timedwait(&handle->wait_cnd, &handle->wait_mutex, timeout);
            if (result != thrd_result::success) {
                handle->write_waiting = false;
                mtx_unlock(&handle->wait_mutex);
                return result == thrd_result::timed_out ? thrd_result::timed_out : thrd_result::nomem;
            }
        }
        handle->write_waiting = false;
        mtx_unlock(&handle->wait_mutex);
        AcquireSRWLockExclusive(&handle->srwlock);
        return thrd_result::success;
    }

    thrd_result smtx_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        mtx_lock(&handle->wait_mutex);
        while (handle->write_waiting) {
            cnd_wait(&handle->wait_cnd, &handle->wait_mutex);
        }
        handle->read_waiting++;
        mtx_unlock(&handle->wait_mutex);
        AcquireSRWLockShared(&handle->srwlock);
        return thrd_result::success;
    }

    thrd_result smtx_timed_lock_shared(smtx_t *const smtx, const ::timespec *timeout) noexcept {
        if (smtx == nullptr || *smtx == nullptr || timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        mtx_lock(&handle->wait_mutex);
        while (handle->write_waiting) {
            thrd_result result = cnd_timedwait(&handle->wait_cnd, &handle->wait_mutex, timeout);
            if (result != thrd_result::success) {
                mtx_unlock(&handle->wait_mutex);
                return result == thrd_result::timed_out ? thrd_result::timed_out : thrd_result::nomem;
            }
        }
        handle->read_waiting++;
        mtx_unlock(&handle->wait_mutex);
        AcquireSRWLockShared(&handle->srwlock);
        return thrd_result::success;
    }

    thrd_result smtx_try_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        if (mtx_trylock(&handle->wait_mutex) != thrd_result::success) {
            return thrd_result::busy;
        }
        if (handle->read_waiting > 0) {
            mtx_unlock(&handle->wait_mutex);
            return thrd_result::busy;
        }
        BOOL acquired = TryAcquireSRWLockExclusive(&handle->srwlock);
        if (acquired) {
            handle->write_waiting = false;
            mtx_unlock(&handle->wait_mutex);
            return thrd_result::success;
        }
        mtx_unlock(&handle->wait_mutex);
        return thrd_result::busy;
    }

    thrd_result smtx_try_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        if (mtx_trylock(&handle->wait_mutex) != thrd_result::success) {
            return thrd_result::busy;
        }
        if (handle->write_waiting) {
            mtx_unlock(&handle->wait_mutex);
            return thrd_result::busy;
        }
        BOOL acquired = TryAcquireSRWLockShared(&handle->srwlock);
        if (acquired) {
            handle->read_waiting++;
            mtx_unlock(&handle->wait_mutex);
            return thrd_result::success;
        }

        mtx_unlock(&handle->wait_mutex);
        return thrd_result::busy;
    }

    thrd_result smtx_unlock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        ReleaseSRWLockExclusive(&handle->srwlock);
        mtx_lock(&handle->wait_mutex);
        cnd_broadcast(&handle->wait_cnd);
        mtx_unlock(&handle->wait_mutex);
        return thrd_result::success;
    }

    thrd_result smtx_unlock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        ReleaseSRWLockShared(&handle->srwlock);
        mtx_lock(&handle->wait_mutex);
        handle->read_waiting--;
        if (handle->read_waiting == 0 && handle->write_waiting) {
            cnd_signal(&handle->wait_cnd);
        }
        mtx_unlock(&handle->wait_mutex);
        return thrd_result::success;
    }

    thrd_result smtx_destroy(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        cnd_destroy(&handle->wait_cnd);
        mtx_destroy(&handle->wait_mutex);
        core::pal::deallocate(handle, sizeof(implements::shared_mutex_handle), alignof(implements::shared_mutex_handle));
        *smtx = nullptr;
        return thrd_result::success;
    }

    void *native_smtx_handle(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            return nullptr;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        return &handle->srwlock;
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
