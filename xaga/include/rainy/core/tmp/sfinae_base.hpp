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
#ifndef RAINY_CORE_TYPE_TRAITS_SFINAE_BASE_HPP
#define RAINY_CORE_TYPE_TRAITS_SFINAE_BASE_HPP
#include <rainy/core/platform.hpp>

namespace rainy::type_traits::other_trans {
    /**
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。 当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     * Condition 才存在（并且是 true 的同义词）。
     * @tparam Test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型。
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef（它是 Ty 的同义词）。如果 test 为
     * false，则 enable_if_t<test, Ty> 结果不会拥有名为“type”的嵌套 typedef
     */
    template <bool Test, typename Ty>
    struct enable_if {
        enable_if() = delete;
        enable_if(const enable_if &) = delete;
        enable_if(enable_if &&) = delete;
        enable_if &operator=(const enable_if &) = delete;
        enable_if &operator=(enable_if &&) = delete;
    };

    template <typename Ty>
    struct enable_if<true, Ty> {
        using type = Ty;
    };

    /**
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。 当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     * Condition 才存在（并且是 true 的同义词）。
     * @tparam Test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型。
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef（它是 Ty 的同义词）。如果 test 为
     * false，则 enable_if_t<test, Ty> 结果不会拥有名为“type”的嵌套 typedef
     */
    template <bool Test, typename Ty = void>
    using enable_if_t = typename enable_if<Test, Ty>::type;

    template <bool, typename IfTrue, typename>
    struct conditional {
        using type = IfTrue;
    };

    template <typename IfTrue, typename IfFalse>
    struct conditional<false, IfTrue, IfFalse> {
        using type = IfFalse;
    };

    template <bool Test, typename IfTrue, typename IfFalse>
    using conditional_t = typename conditional<Test, IfTrue, IfFalse>::type;

    template <typename...>
    using void_t = void;

    template <bool>
    struct select {
        template <typename Ty1, typename>
        using apply = Ty1;
    };

    template <>
    struct select<false> {
        template <typename, typename Ty2>
        using apply = Ty2;
    };

    struct dummy_t {};
}


#endif