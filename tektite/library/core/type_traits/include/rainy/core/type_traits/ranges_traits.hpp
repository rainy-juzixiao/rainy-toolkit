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
#ifndef RAINY_CORE_TYPE_TRAITS_RANGES_TRAITS_HPP
#define RAINY_CORE_TYPE_TRAITS_RANGES_TRAITS_HPP
#include <rainy/core/platform.hpp>

namespace rainy::type_traits::extras::ranges::implements {
    template <typename Range, typename = void>
    struct add_range_traits {};

    template <typename Range>
    struct add_range_traits<
        Range, other_trans::void_t<typename Range::difference_type, typename Range::value_type, typename Range::reference,
                                   typename Range::const_reference, typename Range::iterator, typename Range::const_iterator>> {
        using difference_type = typename Range::difference_type;
        using value_type = typename Range::value_type;
        using reference = typename Range::reference;
        using const_reference = typename Range::const_reference;
        using iterator = typename Range::iterator;
        using const_iterator = typename Range::const_iterator;
    };
}

namespace rainy::type_traits::extras::ranges {
    /**
     * @brief Primary template for range traits, providing uniform interface for range properties.
     *        range_traits 的主模板，为范围属性提供统一接口。
     *
     * @tparam Range The range type to examine
     *               要检查的范围类型
     */
    template <typename Range>
    struct range_traits : implements::add_range_traits<Range> {};

    /**
     * @brief Template for obtaining the reference type of a range.
     *        获取范围的引用类型的模板。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    struct range_reference {
        using type = typename range_traits<Range>::reference;
    };

    /**
     * @brief Alias template for range_reference, providing direct access to the range's reference type.
     *        range_reference 的别名模板，提供对范围引用类型的直接访问。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    using range_reference_t = typename range_reference<Range>::type;

    /**
     * @brief Template for obtaining the const reference type of a range.
     *        获取范围的常量引用类型的模板。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    struct range_const_reference {
        using type = typename range_traits<Range>::const_reference;
    };

    /**
     * @brief Alias template for range_const_reference, providing direct access to the range's const reference type.
     *        range_const_reference 的别名模板，提供对范围常量引用类型的直接访问。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    using range_const_reference_t = typename range_const_reference<Range>::type;

    /**
     * @brief Template for obtaining the difference type of a range.
     *        获取范围的差类型的模板。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    struct range_difference {
        using type = typename range_traits<Range>::difference_type;
    };

    /**
     * @brief Alias template for range_difference, providing direct access to the range's difference type.
     *        range_difference 的别名模板，提供对范围差类型的直接访问。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    using range_difference_t = typename range_difference<Range>::type;

    /**
     * @brief Template for obtaining the value type of a range.
     *        获取范围的值类型的模板。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    struct range_value_type {
        using type = typename range_traits<Range>::value_type;
    };

    /**
     * @brief Alias template for range_value_type, providing direct access to the range's value type.
     *        range_value_type 的别名模板，提供对范围值类型的直接访问。
     *
     * @tparam Range The range type
     *               范围类型
     */
    template <typename Range>
    using range_value_type_t = typename range_value_type<Range>::type;
}

#endif