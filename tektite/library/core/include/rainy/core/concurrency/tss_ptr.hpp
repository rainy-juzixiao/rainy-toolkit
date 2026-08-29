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
#ifndef RAINY_CORE_CONCURRENCY_TSS_PTR_HPP
#define RAINY_CORE_CONCURRENCY_TSS_PTR_HPP
#include <rainy/core/layer.hpp>

namespace rainy::core::concurrency {
    template <typename Ty>
    class tss_ptr {
    public:
        tss_ptr() : tss_key{layer::tss_create()} {
        }

        ~tss_ptr() {
            layer::tss_delete(tss_key);
        }

        operator Ty *() { // NOLINT
            return static_cast<Ty*>(layer::tss_get(tss_key));
        }

        void operator=(Ty *value) {
            layer::tss_set(tss_key, value);
        }

    private:
        core::handle tss_key;
    };
}

#endif