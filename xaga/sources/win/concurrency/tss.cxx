/*
 * Copyright 2025 rainy-juzixiao
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
#include <chrono>
#include <windows.h>
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency::implements {
    core::handle tss_create() {
        const core::handle out_of_indexes = TLS_OUT_OF_INDEXES;
        core::handle tss_key = TlsAlloc();
        if (tss_key == out_of_indexes) {
            errno = EFAULT;
            return core::invalid_handle;
        }
        return tss_key;
    }

    void *tss_get(core::handle tss_key) {
        if (tss_key == core::invalid_handle) {
            return nullptr;
        }
        return TlsGetValue(static_cast<DWORD>(tss_key));
    }

    bool tss_set(core::handle tss_key, void *value) {
        if (tss_key == core::invalid_handle) {
            return false;
        }
        return TlsSetValue(tss_key, const_cast<void *>(value));
    }

    bool tss_delete(core::handle tss_key) {
        if (tss_key == core::invalid_handle) {
            return false;
        }
        return TlsFree(tss_key);
    }
}
