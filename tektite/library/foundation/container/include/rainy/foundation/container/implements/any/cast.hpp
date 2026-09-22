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
#ifndef RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_CAST_HPP
#define RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_CAST_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/typeinfo.hpp>

namespace rainy::foundation::container::implements {
    template <typename Type>
    RAINY_INLINE bool is_as_runnable(const core::typeinfo& type) { // NOLINT
        return rainy_typeid(Type).is_compatible(type);
    }

    template <typename Type>
    RAINY_NODISCARD RAINY_INLINE auto as_impl(const void* const target_pointer, const core::typeinfo& type) // NOLINT
        -> decltype(auto) {
#if RAINY_ENABLE_DEBUG
       if (!implements::is_as_runnable<Type>(type)) {
            throw std::bad_cast{};
       }
#else
        utility::ignore = type;
#endif
        rainy_let ptr = const_cast<void *>(target_pointer);
        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Type>) {
            if constexpr (type_traits::properties::is_const_v<type_traits::modifers::remove_reference_t<Type>>) {
                // 返回 const lvalue 引用
                return *static_cast<const type_traits::modifers::remove_reference_t<Type> *>(ptr);
            } else {
                // 返回非 const lvalue 引用
                return *static_cast<type_traits::modifers::remove_reference_t<Type> *>(ptr);
            }
        } else if constexpr (type_traits::primary_types::is_rvalue_reference_v<Type>) {
            if constexpr (type_traits::properties::is_const_v<type_traits::modifers::remove_reference_t<Type>>) {
                // 返回 const rvalue 引用
                return utility::move(*static_cast<const type_traits::modifers::remove_reference_t<Type> *>(ptr));
            } else {
                // 返回非 const rvalue 引用
                return utility::move(*static_cast<type_traits::modifers::remove_reference_t<Type> *>(ptr));
            }
        } else {
            return *static_cast<Type *>(ptr);
        }
    }
}

#endif
