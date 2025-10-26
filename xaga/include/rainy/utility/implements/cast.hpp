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
#ifndef RAINY_UTILITY_IMPLEMENTS_CAST_HPP
#define RAINY_UTILITY_IMPLEMENTS_CAST_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/utility/implements/any_exceptions.hpp>

namespace rainy::utility::implements {
    template <typename Type>
    RAINY_INLINE bool is_as_runnable(utility::in<foundation::ctti::typeinfo> type) {
        using namespace type_traits;
        using namespace foundation::ctti;
        using namespace foundation::exceptions::cast;
        using match_t = cv_modify::remove_cvref_t<Type>;
        using real_convert_type =
            other_trans::conditional_t<composite_types::is_reference_v<Type>, Type, reference_modify::add_lvalue_reference_t<Type>>;
        switch (type) {
            case rainy_typehash(match_t):
                return true;
            case rainy_typehash(match_t &):
                return type_relations::is_convertible_v<match_t &, real_convert_type>;
            case rainy_typehash(match_t &&):
                return type_relations::is_convertible_v<match_t &&, real_convert_type>;
            case rainy_typehash(const match_t):
                return true;
            case rainy_typehash(const match_t &):
                return type_relations::is_convertible_v<const match_t &, real_convert_type>;
            case rainy_typehash(const match_t &&):
                return type_relations::is_convertible_v<const match_t &&, real_convert_type>;
            case rainy_typehash(const volatile match_t):
                return true;
            case rainy_typehash(const volatile match_t &):
                return type_relations::is_convertible_v<const volatile match_t &, real_convert_type>;
            case rainy_typehash(const volatile match_t &&):
                return type_relations::is_convertible_v<const volatile match_t &&, real_convert_type>;
            default:
                return false;
        }
    }

    template <typename Type>
    RAINY_NODISCARD RAINY_INLINE auto as_impl(utility::in<const void *> target_pointer, utility::in<foundation::ctti::typeinfo> type)
        -> decltype(auto) {
        utility::throw_exception_if<utility::exception_semantic::ignored_in_release>(
            utility::with_this_exception<foundation::exceptions::cast::bad_any_cast>, utility::implements::is_as_runnable<Type>(type));
        using namespace foundation::ctti;
        void *ptr = const_cast<void *>(target_pointer);
        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Type>) {
            if constexpr (type_traits::type_properties::is_const_v<type_traits::reference_modify::remove_reference_t<Type>>) {
                // 返回 const lvalue 引用
                return *static_cast<const type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
            } else {
                // 返回非 const lvalue 引用
                return *static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
            }
        } else if constexpr (type_traits::primary_types::is_rvalue_reference_v<Type>) {
            if constexpr (type_traits::type_properties::is_const_v<std::remove_reference_t<Type>>) {
                // 返回 const rvalue 引用
                return utility::move(*static_cast<const type_traits::reference_modify::remove_reference_t<Type> *>(ptr));
            } else {
                // 返回非 const rvalue 引用
                return utility::move(*static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr));
            }
        } else {
            return *static_cast<Type *>(ptr);
        }
    }
}

#endif