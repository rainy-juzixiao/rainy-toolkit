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
#ifndef RAINY_CORE_TYPE_TRAITS_TYPE_RELATIONS_HPP
#define RAINY_CORE_TYPE_TRAITS_TYPE_RELATIONS_HPP
#include <rainy/core/type_traits/implements.hpp>

// 类型关系
namespace rainy::type_traits::type_relations {
    /**
     * \lang english
     * @brief Checks if Derived is derived from Base.
     *
     * @tparam Base The base class type
     * @tparam Derived The derived class type to check
     *
     * \lang simp-chinese
     * @brief 检查 Derived 是否由 Base 派生。
     *
     * @tparam Base 基类类型
     * @tparam Derived 要检查的派生类类型
     */
    template <typename Base, typename Derived>
    struct is_base_of : helper::bool_constant<implements::is_base_of_v<Base, Derived>> {};

    /**
     * \lang english
     * @brief Variable template for checking if Derived is derived from Base.
     *
     * @tparam Base The base class type
     * @tparam Derived The derived class type to check
     *
     * \lang simp-chinese
     * @brief 检查 Derived 是否由 Base 派生的变量模板。
     *
     * @tparam Base 基类类型
     * @tparam Derived 要检查的派生类类型
     */
    template <typename Base, typename Derived>
    RAINY_CONSTEXPR_BOOL is_base_of_v = implements::is_base_of_v<Base, Derived>;

    /**
     * \lang english
     * @brief Variable template for checking if Ty is the same as any of Types.
     *
     * @tparam Ty The type to compare
     * @tparam Types The candidate types
     *
     * \lang simp-chinese
     * @brief 检查 Ty 是否与 Types 中的任一类型相同的变量模板。
     *
     * @tparam Ty 要比较的类型
     * @tparam Types 候选类型
     */
    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_of_v = (is_same_v<Ty, Types> || ...); // NOLINT

    /**
     * \lang english
     * @brief Checks if Ty is the same as any of Types.
     *
     * @tparam Ty The type to compare
     * @tparam Types The candidate types
     *
     * \lang simp-chinese
     * @brief 检查 Ty 是否与 Types 中的任一类型相同。
     *
     * @tparam Ty 要比较的类型
     * @tparam Types 候选类型
     */
    template <typename Ty, typename... Types>
    struct is_any_of : helper::bool_constant<is_any_of_v<Ty, Types...>> {};

    /**
     * \lang english
     * @brief Variable template for checking if From is convertible to To.
     *
     * @tparam From The source type
     * @tparam To The target type
     *
     * \lang simp-chinese
     * @brief 检查 From 是否可以转换为 To 的变量模板。
     *
     * @tparam From 源类型
     * @tparam To 目标类型
     */
    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_convertible_v = implements::is_convertible_v<From, To>;

    /**
     * \lang english
     * @brief Checks if From is convertible to To.
     *
     * @tparam From The source type
     * @tparam To The target type
     *
     * \lang simp-chinese
     * @brief 检查 From 是否可以转换为 To。
     *
     * @tparam From 源类型
     * @tparam To 目标类型
     */
    template <typename From ,typename To>
    struct is_convertible : helper::bool_constant<is_convertible_v<From, To>> {};

    /**
     * \lang english
     * @brief Variable template for checking if From is nothrow-convertible to To.
     *
     * @tparam From The source type
     * @tparam To The target type
     *
     * \lang simp-chinese
     * @brief 检查 From 是否可以无异常地转换为 To 的变量模板。
     *
     * @tparam From 源类型
     * @tparam To 目标类型
     */
    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_nothrow_convertible_v = implements::is_nothrow_convertible_v<From, To>;

    /**
     * \lang english
     * @brief Checks if From is nothrow-convertible to To.
     *
     * @tparam From The source type
     * @tparam To The target type
     *
     * \lang simp-chinese
     * @brief 检查 From 是否可以无异常地转换为 To。
     *
     * @tparam From 源类型
     * @tparam To 目标类型
     */
    template <typename From, typename To>
    struct is_nothrow_convertible : helper::bool_constant<is_nothrow_convertible_v<From, To>> {};

    /**
     * \lang english
     * @brief Variable template for checking if Ty is convertible to any of Types.
     *
     * @tparam Ty The source type
     * @tparam Types The target types
     *
     * \lang simp-chinese
     * @brief 检查 Ty 是否可以转换为 Types 中的任一类型的变量模板。
     *
     * @tparam Ty 源类型
     * @tparam Types 目标类型
     */
    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_convertible_v = (is_convertible_v<Ty, Types> || ...);

    /**
     * \lang english
     * @brief Variable template for checking if two types differ after removing cv-qualifiers and references.
     *
     * @tparam Ty1 The first type
     * @tparam Ty2 The second type
     *
     * \lang simp-chinese
     * @brief 检查两个类型在移除 cv 限定符和引用后是否不同的变量模板。
     *
     * @tparam Ty1 第一个类型
     * @tparam Ty2 第二个类型
     */
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL different_from_v =
        !type_traits::type_relations::is_same_v<implements::remove_cvref_t<Ty1>, implements::remove_cvref_t<Ty2>>;
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Helper trait that forbids instantiation when Ty matches any of Types.
     *         Used to disable specific template instantiations via SFINAE.
     *
     * @tparam Ty The type to test
     * @tparam Types The types that must not match Ty
     *
     * \lang simp-chinese
     * @brief 当 Ty 与 Types 中的任一类型匹配时禁止实例化的辅助特性。
     *         用于通过 SFINAE 禁用特定的模板实例化。
     *
     * @tparam Ty 要测试的类型
     * @tparam Types 不允许与 Ty 匹配的类型
     */
    template <typename Ty = void, typename... Types>
    struct forbbiden_instantiant {
        using type = enable_if_t<!type_relations::is_any_of_v<Ty, Types...>, int>;
    };

    /**
     * \lang english
     * @brief Alias template for forbbiden_instantiant, providing direct access to the resulting type.
     *
     * @tparam Ty The type to test
     * @tparam Types The types that must not match Ty
     *
     * \lang simp-chinese
     * @brief forbbiden_instantiant 的别名模板，提供对结果类型的直接访问。
     *
     * @tparam Ty 要测试的类型
     * @tparam Types 不允许与 Ty 匹配的类型
     */
    template <typename Ty = void, typename... Types>
    using forbbiden_instantiant_t = typename forbbiden_instantiant<Ty, Types...>::type;
}

#if RAINY_HAS_CXX20

namespace rainy::type_traits::concepts {
    /**
     * \lang english
     * @brief Concept that is satisfied when Ty is not the same as any of Types.
     *
     * @tparam Ty The type to test
     * @tparam Types The types that must not match Ty
     *
     * \lang simp-chinese
     * @brief 当 Ty 与 Types 中的任一类型都不相同时满足的概念。
     *
     * @tparam Ty 要测试的类型
     * @tparam Types 不允许与 Ty 匹配的类型
     */
    template <typename Ty = void, typename... Types>
    concept forbbiden_instantiant = !implements::is_any_of_v<Ty, Types...>;
}

#endif

#endif
