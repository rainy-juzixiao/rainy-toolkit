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
#ifndef RAINY_CORE_TYPE_TRAITS_DECAY_HPP
#define RAINY_CORE_TYPE_TRAITS_DECAY_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/modifers.hpp>

namespace rainy::type_traits::other_trans {
    /**
     * @brief Type decay template, mimicking the behavior of std::decay.
     *        类型退化模板，模拟 std::decay 的行为。
     *
     * Applies lvalue-to-rvalue, array-to-pointer, and function-to-pointer
     * conversions to type Ty, and removes cv-qualifiers.
     *
     * 对类型 Ty 应用左值到右值、数组到指针、函数到指针的转换，
     * 并移除 cv 限定符。
     *
     * @tparam Ty The type to decay
     *            要退化的类型
     */
    template <typename Ty>
    struct decay {
        using Ty1 = reference_modify::remove_reference_t<Ty>;

        using Ty2 = typename select<implements::_is_function_v<Ty1>>::template apply<
            pointer_modify::add_pointer<Ty1>, cv_modify::remove_cv<std::conditional_t<!implements::_is_function_v<Ty1>, Ty1, void>>>;

        using type =
            typename select<implements::_is_array_v<Ty1>>::template apply<pointer_modify::add_pointer<modifers::remove_extent_t<Ty1>>,
                                                                          Ty2>::type;
    };

    /**
     * @brief Alias template for type decay, providing simplified access.
     *        类型退化模板的别名简化，提供便捷访问。
     *
     * @tparam Ty The type to decay
     *            要退化的类型
     */
    template <typename Ty>
    using decay_t = typename decay<Ty>::type;

    /**
     * @brief Retrieves the underlying integral type of an enum type.
     *        获取枚举类型的底层整数类型。
     *
     * Returns the underlying integer type of enum type Ty.
     *
     * @tparam Ty The enum type
     *            枚举类型
     */
    template <typename Ty>
    struct underlying_type {
        using type = __underlying_type(Ty);
    };

    /**
     * @brief Alias template for underlying type, providing simplified access.
     *        底层类型模板的别名简化，提供便捷访问。
     *
     * @tparam Ty The enum type
     *            枚举类型
     */
    template <typename Ty>
    using underlying_type_t = typename underlying_type<Ty>::type;
}

#endif