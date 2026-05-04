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
    core::handle tss_create() {
        pthread_key_t tss_key = 0;
        if (const int ret = pthread_key_create(&tss_key, nullptr); ret != 0) {
            errno = ret; // POSIX 错误码
            return core::invalid_handle;
        }
        return tss_key;
    }

    void *tss_get(const core::handle tss_key) {
        if (tss_key == core::invalid_handle) {
            return nullptr;
        }
        return pthread_getspecific(tss_key);
    }

    bool tss_set(const core::handle tss_key, const void *value) {
        if (tss_key == core::invalid_handle) {
            return false;
        }
        return pthread_setspecific(tss_key, value) == 0;
    }

    bool tss_delete(const core::handle tss_key) {
        if (tss_key == core::invalid_handle) {
            return false;
        }
        return pthread_key_delete(tss_key) == 0;
    }
}