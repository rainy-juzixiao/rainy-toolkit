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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_SET_OPERATIONS_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_SET_OPERATIONS_HPP

#include <rainy/core/algorithm/set_operations.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief Checks whether one sorted range contains another.
     *
     * @tparam Range The containing range type
     * @tparam SubRange The contained range type
     * @param range The containing range
     * @param sub The contained range
     * @return true if sub is a subsequence of range
     *
     * \lang simp-chinese
     * @brief 检查一个有序范围是否包含另一个。
     *
     * @tparam Range 包含范围类型
     * @tparam SubRange 被包含范围类型
     * @param range 包含范围
     * @param sub 被包含范围
     * @return 如果sub是range的子序列则为true
     */
    template <typename Range, typename SubRange>
    RAINY_NODISCARD constexpr auto includes(Range &&range, SubRange &&sub) -> bool {
        return core::algorithm::includes(utility::begin(range), utility::end(range), utility::begin(sub), utility::end(sub));
    }

    /**
     * \lang english
     * @brief Merges two sorted ranges into a destination.
     *
     * @tparam Range1 The first range type
     * @tparam Range2 The second range type
     * @tparam OutIt Output iterator type
     * @param range1 The first range
     * @param range2 The second range
     * @param dest Iterator to the beginning of the destination
     * @return Iterator to the end of the destination range
     *
     * \lang simp-chinese
     * @brief 将两个有序范围合并到目标位置。
     *
     * @tparam Range1 第一个范围类型
     * @tparam Range2 第二个范围类型
     * @tparam OutIt 输出迭代器类型
     * @param range1 第一个范围
     * @param range2 第二个范围
     * @param dest 指向目标起始的迭代器
     * @return 指向目标范围末尾的迭代器
     */
    template <typename Range1, typename Range2, typename OutIt>
    constexpr auto merge(Range1 &&range1, Range2 &&range2, OutIt dest) {
        return core::algorithm::merge(utility::begin(range1), utility::end(range1), utility::begin(range2),
                                      utility::end(range2), dest);
    }

    /**
     * \lang english
     * @brief Computes the union of two sorted ranges into a destination.
     *
     * \lang simp-chinese
     * @brief 计算两个有序范围的并集到目标位置。
     */
    template <typename Range1, typename Range2, typename OutIt>
    constexpr auto set_union(Range1 &&range1, Range2 &&range2, OutIt dest) {
        return core::algorithm::set_union(utility::begin(range1), utility::end(range1), utility::begin(range2),
                                          utility::end(range2), dest);
    }

    /**
     * \lang english
     * @brief Computes the intersection of two sorted ranges into a destination.
     *
     * \lang simp-chinese
     * @brief 计算两个有序范围的交集到目标位置。
     */
    template <typename Range1, typename Range2, typename OutIt>
    constexpr auto set_intersection(Range1 &&range1, Range2 &&range2, OutIt dest) {
        return core::algorithm::set_intersection(utility::begin(range1), utility::end(range1), utility::begin(range2),
                                                 utility::end(range2), dest);
    }

    /**
     * \lang english
     * @brief Computes the difference of two sorted ranges into a destination.
     *
     * \lang simp-chinese
     * @brief 计算两个有序范围的差集到目标位置。
     */
    template <typename Range1, typename Range2, typename OutIt>
    constexpr auto set_difference(Range1 &&range1, Range2 &&range2, OutIt dest) {
        return core::algorithm::set_difference(utility::begin(range1), utility::end(range1), utility::begin(range2),
                                               utility::end(range2), dest);
    }

    /**
     * \lang english
     * @brief Computes the symmetric difference of two sorted ranges into a destination.
     *
     * \lang simp-chinese
     * @brief 计算两个有序范围的对称差到目标位置。
     */
    template <typename Range1, typename Range2, typename OutIt>
    constexpr auto set_symmetric_difference(Range1 &&range1, Range2 &&range2, OutIt dest) {
        return core::algorithm::set_symmetric_difference(utility::begin(range1), utility::end(range1),
                                                         utility::begin(range2), utility::end(range2), dest);
    }
}

namespace rainy::collections::views {
    using core::collections::views::includes;
    using core::collections::views::merge;
    using core::collections::views::set_difference;
    using core::collections::views::set_intersection;
    using core::collections::views::set_symmetric_difference;
    using core::collections::views::set_union;
}

#endif
