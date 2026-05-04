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
#include <synchapi.h>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26110)
#endif

namespace rainy::foundation::concurrency::implements {
    struct shared_mutex_handle {
        SRWLOCK  srwlock;
        LONG volatile state;
    };

    DWORD timespec_to_ms_remaining(const ::timespec *timeout) noexcept {
        using namespace std::chrono;
        auto ns = seconds(timeout->tv_sec) + nanoseconds(timeout->tv_nsec);
        auto deadline = system_clock::time_point(
            duration_cast<system_clock::duration>(ns));
        auto now = system_clock::now();
        if (now >= deadline) {
            return 0;
        }
        auto ms = duration_cast<milliseconds>(deadline - now).count();
        return static_cast<DWORD>(ms < 1 ? 1 : ms);
    }

    constexpr LONG STATE_FREE      = 0;
    constexpr LONG STATE_SHARED    = 1;
    constexpr LONG STATE_EXCLUSIVE = 2;

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
        InterlockedExchange(&handle->state, STATE_FREE);
        return thrd_result::success;
    }

    thrd_result smtx_create(smtx_t *const smtx) noexcept {
        if (smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        *smtx = core::pal::allocate(
            sizeof(shared_mutex_handle), alignof(shared_mutex_handle));
        if (*smtx == nullptr) {
            return thrd_result::nomem;
        }
        return smtx_init(smtx);
    }

    thrd_result smtx_destroy(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        core::pal::deallocate(
            handle, sizeof(shared_mutex_handle), alignof(shared_mutex_handle));
        *smtx = nullptr;
        return thrd_result::success;
    }

    thrd_result smtx_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        InterlockedExchange(&handle->state, STATE_EXCLUSIVE);
        AcquireSRWLockExclusive(&handle->srwlock);
        return thrd_result::success;
    }

    thrd_result smtx_timed_lock(smtx_t *const smtx, const ::timespec *timeout) noexcept {
        if (smtx == nullptr || *smtx == nullptr || timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        while (true) {
            if (TryAcquireSRWLockExclusive(&handle->srwlock)) {
                InterlockedExchange(&handle->state, STATE_EXCLUSIVE);
                return thrd_result::success;
            }
            DWORD ms = timespec_to_ms_remaining(timeout);
            if (ms == 0) {
                return thrd_result::timed_out;
            }
            LONG observed = handle->state;
            if (observed == STATE_FREE) {
                continue;
            }
            WaitOnAddress(
                const_cast<LONG *>(&handle->state),
                &observed,
                sizeof(LONG),
                ms);
        }
    }

    thrd_result smtx_try_lock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        if (TryAcquireSRWLockExclusive(&handle->srwlock)) {
            InterlockedExchange(&handle->state, STATE_EXCLUSIVE);
            return thrd_result::success;
        }
        return thrd_result::busy;
    }

    thrd_result smtx_unlock(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        InterlockedExchange(&handle->state, STATE_FREE);
        ReleaseSRWLockExclusive(&handle->srwlock);
        // 唤醒所有在 state 上等待的线程
        WakeByAddressAll(const_cast<LONG *>(&handle->state));
        return thrd_result::success;
    }

    thrd_result smtx_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        AcquireSRWLockShared(&handle->srwlock);
        InterlockedCompareExchange(&handle->state, STATE_SHARED, STATE_FREE);
        return thrd_result::success;
    }

    thrd_result smtx_timed_lock_shared(smtx_t *const smtx, const ::timespec *timeout) noexcept {
        if (smtx == nullptr || *smtx == nullptr || timeout == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);

        while (true) {
            if (TryAcquireSRWLockShared(&handle->srwlock)) {
                InterlockedCompareExchange(&handle->state, STATE_SHARED, STATE_FREE);
                return thrd_result::success;
            }

            DWORD ms = timespec_to_ms_remaining(timeout);
            if (ms == 0) {
                return thrd_result::timed_out;
            }

            LONG observed = handle->state;
            if (observed == STATE_FREE) {
                continue;
            }
            WaitOnAddress(
                const_cast<LONG *>(&handle->state),
                &observed,
                sizeof(LONG),
                ms);
        }
    }

    thrd_result smtx_try_lock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        if (TryAcquireSRWLockShared(&handle->srwlock)) {
            InterlockedCompareExchange(&handle->state, STATE_SHARED, STATE_FREE);
            return thrd_result::success;
        }
        return thrd_result::busy;
    }

    thrd_result smtx_unlock_shared(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            errno = EINVAL;
            return thrd_result::nomem;
        }
        auto *handle = static_cast<shared_mutex_handle *>(*smtx);
        ReleaseSRWLockShared(&handle->srwlock);
        InterlockedCompareExchange(&handle->state, STATE_FREE, STATE_SHARED);
        WakeByAddressAll(const_cast<LONG *>(&handle->state));
        return thrd_result::success;
    }

    void *native_smtx_handle(smtx_t *const smtx) noexcept {
        if (smtx == nullptr || *smtx == nullptr) {
            return nullptr;
        }
        return &static_cast<shared_mutex_handle *>(*smtx)->srwlock;
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif