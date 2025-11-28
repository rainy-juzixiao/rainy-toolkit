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
#ifndef RAINY_META_REFLECTION_REFL_IMPL_HPP
#define RAINY_META_REFLECTION_REFL_IMPL_HPP
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::meta::reflection {
    class type;
}

namespace rainy::meta::reflection::implements {
    struct type_accessor;
}

namespace rainy::meta::reflection::implements {
    class RAINY_TOOLKIT_API register_table {
    public:
        static void register_type(std::string_view name, type_accessor *type);

        static type_accessor *get_accessor(annotations::lifetime::in<foundation::ctti::typeinfo> type);

        static void unregister(std::string_view name, foundation::ctti::typeinfo ctti);

        static type_accessor *get_accessor_by_name(std::string_view name);

        static bool has_register(annotations::lifetime::in<foundation::ctti::typeinfo> type) noexcept;

    private:
        class myimpl;

        static register_table &instance();

        myimpl *global_ptr;
    };
}

namespace rainy::meta::reflection::implements {
    class RAINY_TOOLKIT_API injector {
    public:
        static void register_type(std::string_view name, type_accessor *type);

        static void unregister_all();

    private:
        struct registration_entry {
            std::string_view name;
            foundation::ctti::typeinfo ctti;
        };

        class myimpl;

        static injector &instance();

        myimpl *global_ptr;
    };
}

#endif