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
#ifndef RAINY_META_REFLECTION_SHARED_OBJECT_HPP
#define RAINY_META_REFLECTION_SHARED_OBJECT_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/any.hpp>
#include <rainy/meta/reflection/type.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace rainy::meta::reflection {
    class shared_object {
    public:
        shared_object() noexcept = default;

        shared_object(utility::any &&object) : internal_storage{std::make_shared<impl>(utility::move(object))} {
        }

        const foundation::ctti::typeinfo& type() const noexcept {
            return internal_storage->object.type();
        }

        const void *target_as_void_ptr() noexcept {
            return internal_storage->object.target_as_void_ptr();
        }

        template <typename... Args>
        utility::any invoke(std::string_view name, Args &&...args) const {
            if (!internal_storage) {
                return {};
            }
            return internal_storage->type_of_so.invoke_method(name, as_object_view(internal_storage->object),
                                                              utility::forward<Args>(args)...);
        }

        operator object_view() noexcept {
            return as_object_view(internal_storage->object);
        }

    private:
        struct impl {
            impl(utility::any&& object) : type_of_so{}, object(utility::move(object)) {
                type_of_so = type::get_by_typeinfo(this->object.type());
            }

            reflection::type type_of_so;
            utility::any object;
        };

        std::shared_ptr<impl> internal_storage;
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif