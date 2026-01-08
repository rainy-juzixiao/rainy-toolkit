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
#include <rainy/collections/dense_map.hpp>
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::foundation::ctti::implements {
    class dynamic_converter_registry::impl {
    public:
        void register_converter(const ctti::typeinfo& to_type,const converter_fn fn) {
            table_.emplace(to_type, fn); // NOLINT
        }

        RAINY_NODISCARD converter_fn find(const ctti::typeinfo& to_type) const {
            const auto it = table_.find(to_type);
            return it == table_.end() ? nullptr : it->second;
        }

    private:
        collections::dense_map<ctti::typeinfo, converter_fn> table_;
    };

    dynamic_converter_registry &dynamic_converter_registry::instance() {
        static dynamic_converter_registry instance;
        static impl impl_instance;
        instance.global_ptr = &impl_instance;
        return instance;
    }

    void dynamic_converter_registry::register_converter(const ctti::typeinfo& to_type,const converter_fn fn) { // NOLINT
        global_ptr->register_converter(to_type, fn);
    }

    converter_fn dynamic_converter_registry::find(const ctti::typeinfo& to_type) const {
        return global_ptr->find(to_type);
    }
}
