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
    /**
     * @brief 测试两个类型是否相同
     * @tparam Ty1 检索的第一个类型
     * @tparam Ty2 检索的第二个类型
     */
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_same_v<Ty, Ty> = true;

    /**
     * @brief 测试两个类型是否相同。
     * @tparam Ty1 检索的第一个类型
     * @tparam Ty2 检索的第二个类型
     */
    template <typename Ty1, typename Ty2>
    struct is_same : helper::bool_constant<is_same_v<Ty1, Ty2>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = is_same_v<Ty, void>;

    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};

    template <typename base, typename Derived>
    struct is_base_of : helper::bool_constant<__is_base_of(base, Derived)> {};

    template <typename Base, typename Derived>
    RAINY_CONSTEXPR_BOOL is_base_of_v = __is_base_of(Base, Derived);

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_of_v = (is_same_v<Ty, Types> || ...); // NOLINT

#if RAINY_USING_MSVC || RAINY_USING_CLANG
    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_convertible_v = __is_convertible_to(From, To);
#else
    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_convertible_v = __is_convertible(From, To);
#endif

    template <typename From ,typename To>
    struct is_convertible : helper::bool_constant<is_convertible_v<From, To>> {};

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