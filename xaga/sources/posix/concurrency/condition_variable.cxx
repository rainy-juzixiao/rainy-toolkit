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
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency::implements {
    struct mutex_handle {
        pthread_mutex_t handle{};
        int type{0};
        pthread_t thread_id{};
        int count{0};
    };

    struct cnd_internal {
        pthread_cond_t cond;
    };

    thrd_result cnd_create(cnd_t *cnd) noexcept {
        if (!cnd) {
            return thrd_result::nomem;
        }
        auto *obj = static_cast<cnd_internal *>(core::pal::allocate(sizeof(cnd_internal), alignof(cnd_internal)));
        if (!obj) {
            return thrd_result::nomem;
        }
        if (const int ret = pthread_cond_init(&obj->cond, nullptr); ret != 0) {
            core::pal::deallocate(obj, sizeof(cnd_internal), alignof(cnd_internal));
            return thrd_result::error;
        }
        *cnd = static_cast<cnd_t>(obj);
        return thrd_result::success;
    }

    thrd_result cnd_init(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        const int ret = pthread_cond_init(&obj->cond, nullptr);
        return ret == 0 ? thrd_result::success : thrd_result::error;
    }

    thrd_result cnd_wait(cnd_t *const cnd, mtx_t* const mtx) noexcept {
        if (!cnd || !*cnd || !mtx || !*mtx) {
            return thrd_result::nomem;
        }
        auto *obj = static_cast<cnd_internal *>(*cnd);
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        auto *pmutex = static_cast<pthread_mutex_t *>(native_mtx_handle(mtx));
        if (!pmutex) {
            return thrd_result::nomem;
        }
        const int saved_count = mutex->count;
        const pthread_t saved_tid = mutex->thread_id;
        mutex->count = 0;
        mutex->thread_id = {};
        const int ret = pthread_cond_wait(&obj->cond, pmutex);
        mutex->count = saved_count;
        mutex->thread_id = pthread_self();
        switch (ret) {
            case 0:
                return thrd_result::success;
            case EINVAL:
                return thrd_result::nomem;
            default:
                return thrd_result::error;
        }
    }

    thrd_result cnd_timedwait(cnd_t *const cnd, mtx_t* const mtx, const ::timespec *timeout) noexcept {
        if (!cnd || !*cnd || !mtx || !*mtx || !timeout) {
            return thrd_result::nomem;
        }
        auto *obj = static_cast<cnd_internal *>(*cnd);
        auto *mutex = static_cast<mutex_handle *>(*mtx);
        auto *pmutex = static_cast<pthread_mutex_t *>(native_mtx_handle(mtx));
        if (!pmutex) {
            return thrd_result::nomem;
        }

        const int saved_count = mutex->count;
        const pthread_t saved_tid = mutex->thread_id;
        mutex->count = 0;
        mutex->thread_id = {};

        const int ret = pthread_cond_timedwait(&obj->cond, pmutex, timeout);

        // 无论是正常唤醒还是超时，pthread 都会重新持有锁
        mutex->count = saved_count;
        mutex->thread_id = pthread_self();

        switch (ret) {
            case 0:
                return thrd_result::success;
            case ETIMEDOUT:
                return thrd_result::timed_out;
            case EINVAL:
                return thrd_result::nomem;
            default:
                return thrd_result::error;
        }
    }

    thrd_result cnd_signal(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        const int ret = pthread_cond_signal(&obj->cond);
        return ret == 0 ? thrd_result::success : thrd_result::error;
    }

    thrd_result cnd_broadcast(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        const int ret = pthread_cond_broadcast(&obj->cond);
        return ret == 0 ? thrd_result::success : thrd_result::error;
    }

    thrd_result cnd_destroy(cnd_t *cnd) noexcept {
        if (!cnd || !*cnd) {
            return thrd_result::nomem;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        const int ret = pthread_cond_destroy(&obj->cond);
        core::pal::deallocate(obj, sizeof(cnd_internal), alignof(cnd_internal));
        *cnd = nullptr;
        return ret == 0 ? thrd_result::success : thrd_result::error;
    }

    void *native_cnd_handle(cnd_t *const cnd) noexcept {
        if (!cnd || !*cnd) {
            return nullptr;
        }
        rainy_const obj = static_cast<cnd_internal *>(*cnd);
        return &obj->cond;
    }
}
