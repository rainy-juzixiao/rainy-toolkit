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
#ifndef RAINY_CORE_TMP_TYPE_RELATIONS_HPP
#define RAINY_CORE_TMP_TYPE_RELATIONS_HPP
#include <rainy/core/tmp/implements.hpp>

// 类型关系
namespace rainy::type_traits::type_relations {
    template <typename Base, typename Derived>
    struct is_base_of : helper::bool_constant<implements::is_base_of_v<Base, Derived>> {};

    template <typename Base, typename Derived>
    RAINY_CONSTEXPR_BOOL is_base_of_v = implements::is_base_of_v<Base, Derived>;

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_of_v = (is_same_v<Ty, Types> || ...); // NOLINT

    template <typename Ty, typename... Types>
    struct is_any_of : helper::bool_constant<is_any_of_v<Ty, Types...>> {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_convertible_v = implements::is_convertible_v<From, To>;

    template <typename From ,typename To>
    struct is_convertible : helper::bool_constant<is_convertible_v<From, To>> {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_nothrow_convertible_v = implements::is_nothrow_convertible_v<From, To>;

    template <typename From, typename To>
    struct is_nothrow_convertible : helper::bool_constant<is_nothrow_convertible_v<From, To>> {};


    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_convertible_v = (is_convertible_v<Ty, Types> || ...);

    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL different_from_v =
        !type_traits::type_relations::is_same_v<implements::remove_cvref_t<Ty1>, implements::remove_cvref_t<Ty2>>;
}

namespace rainy::type_traits::other_trans {
    template <typename Ty = void, typename... Types>
    struct forbbiden_instantiant {
        using type = enable_if_t<!type_relations::is_any_of_v<Ty, Types...>, int>;
    };

    template <typename Ty = void, typename... Types>
    using forbbiden_instantiant_t = typename forbbiden_instantiant<Ty, Types...>::type;
}

#if RAINY_HAS_CXX20

namespace rainy::type_traits::concepts {
    template <typename Ty = void, typename... Types>
    concept forbbiden_instantiant = !implements::is_any_of_v<Ty, Types...>;
}

#endif

#endif