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
#ifndef RAINY_CORE_TYPE_TRAITS_LOGICAL_HPP
#define RAINY_CORE_TYPE_TRAITS_LOGICAL_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>

namespace rainy::type_traits::logical_traits {
    /**
     * @brief Logical conjunction trait (AND) for type traits.
     *        Primary template for empty parameter pack yields true_type.
     *
     *        类型特征的逻辑与（AND） traits。
     *        空参数包的主模板生成 true_type。
     *
     * @tparam ... Parameter pack of type traits with ::value members
     *             具有 ::value 成员的类型特征参数包
     */
    template <typename...>
    struct conjunction : helper::true_type {};

    /**
     * @brief Logical conjunction trait (AND) for type traits.
     *        Recursive template that evaluates the conjunction of all traits.
     *
     *        类型特征的逻辑与（AND） traits。
     *        递归模板，计算所有特征的逻辑与。
     *
     * @tparam First First trait to evaluate
     *               要评估的第一个特征
     * @tparam Rest Remaining traits to evaluate
     *              要评估的剩余特征
     */
    template <typename First, typename... Rest>
    struct conjunction<First, Rest...> : implements::_conjunction<First::value, First, Rest...>::type {};

    /**
     * @brief Variable template for logical conjunction.
     *        Provides the value of conjunction<Traits...>::value.
     *
     *        逻辑与的变量模板。
     *        提供 conjunction<Traits...>::value 的值。
     *
     * @tparam Traits Type traits to evaluate
     *                要评估的类型特征
     */
    template <typename... Traits>
    RAINY_CONSTEXPR_BOOL conjunction_v = conjunction<Traits...>::value;

    /**
     * @brief Logical disjunction trait (OR) for type traits.
     *        Primary template for empty parameter pack yields false_type.
     *
     *        类型特征的逻辑或（OR） traits。
     *        空参数包的主模板生成 false_type。
     *
     * @tparam ... Parameter pack of type traits with ::value members
     *             具有 ::value 成员的类型特征参数包
     */
    template <typename...>
    struct disjunction : helper::false_type {};

    /**
     * @brief Logical disjunction trait (OR) for type traits.
     *        Recursive template that evaluates the disjunction of all traits.
     *
     *        类型特征的逻辑或（OR） traits。
     *        递归模板，计算所有特征的逻辑或。
     *
     * @tparam first First trait to evaluate
     *               要评估的第一个特征
     * @tparam rest Remaining traits to evaluate
     *              要评估的剩余特征
     */
    template <typename first, typename... rest>
    struct disjunction<first, rest...> : implements::_disjunction<first::value, first, rest...>::type {};

    /**
     * @brief Variable template for logical disjunction.
     *        Provides the value of disjunction<traits...>::value.
     *
     *        逻辑或的变量模板。
     *        提供 disjunction<traits...>::value 的值。
     *
     * @tparam traits Type traits to evaluate
     *                要评估的类型特征
     */
    template <typename... traits>
    RAINY_CONSTEXPR_BOOL disjunction_v = disjunction<traits...>::value;

    /**
     * @brief Logical negation trait (NOT) for a single type trait.
     *        Returns the opposite boolean value of the input trait.
     *
     *        单个类型特征的逻辑非（NOT） traits。
     *        返回输入特征的反向布尔值。
     *
     * @tparam trait Type trait with ::value member to negate
     *               要取反的具有 ::value 成员的类型特征
     */
    template <typename trait>
    struct negation : helper::bool_constant<!static_cast<bool>(trait::value)> {};

    /**
     * @brief Variable template for logical negation.
     *        Provides the value of negation<trait>::value.
     *
     *        逻辑非的变量模板。
     *        提供 negation<trait>::value 的值。
     *
     * @tparam trait Type trait to negate
     *               要取反的类型特征
     */
    template <typename trait>
    RAINY_CONSTEXPR_BOOL negation_v = negation<trait>::value;
}

#endif