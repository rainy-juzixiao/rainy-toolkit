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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_SORTING_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_SORTING_HPP

#include <rainy/core/algorithm/sorting.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * @brief Sorts a range in ascending order (introsort).
     *        将范围升序排序（introsort，非稳定）。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to sort
     *              要排序的范围
     */
    template <typename Range>
    constexpr auto sort(Range &&range) {
        return core::algorithm::sort(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Sorts a range in ascending order with respect to comp.
     *        按comp将范围升序排序。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param range The range to sort
     *              要排序的范围
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename Range, typename Comp>
    constexpr auto sort(Range &&range, Comp comp) {
        return core::algorithm::sort(utility::begin(range), utility::end(range), utility::move(comp));
    }

    /**
     * @brief Stably sorts a range in ascending order (merge sort).
     *        稳定升序排序范围（归并排序）。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to sort
     *              要排序的范围
     */
    template <typename Range>
    constexpr auto stable_sort(Range &&range) {
        return core::algorithm::stable_sort(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Partially sorts a range: the smallest prefix becomes sorted at the front.
     *        部分排序范围：最小前缀有序地排在前面。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Iter The iterator type pointing into the range
     *              指向范围的迭代器类型
     * @param range The range to sort
     *              要排序的范围
     * @param middle Iterator delimiting the sorted prefix
     *               界定有序前缀的迭代器
     * @return Iterator to the end of the sorted prefix
     *         指向有序前缀末尾的迭代器
     */
    template <typename Range, typename Iter>
    constexpr auto partial_sort(Range &&range, Iter middle) {
        return core::algorithm::partial_sort(utility::begin(range), middle, utility::end(range));
    }

    /**
     * @brief Copies the smallest elements of a range, sorted, into a destination pair of iterators.
     *        将范围中最小的若干元素排序后拷贝到目标迭代器对。
     *
     * @tparam Range The source range type
     *               源范围类型
     * @tparam RandomIt Random access iterator type of the destination
     *                  目标的随机访问迭代器类型
     * @param range The source range
     *              源范围
     * @param dest_first Iterator to the beginning of the destination
     *                   指向目标起始的迭代器
     * @param dest_last Iterator to the end of the destination
     *                  指向目标末尾的迭代器
     * @return Iterator to the end of the written destination range
     *         指向已写入目标范围末尾的迭代器
     */
    template <typename Range, typename RandomIt>
    constexpr auto partial_sort_copy(Range &&range, RandomIt dest_first, RandomIt dest_last) {
        return core::algorithm::partial_sort_copy(utility::begin(range), utility::end(range), dest_first, dest_last);
    }

    /**
     * @brief Places the element that would be at position nth after sorting there.
     *        将排序后应位于nth位置的元素放到该处。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Iter The iterator type pointing into the range
     *              指向范围的迭代器类型
     * @param range The range to rearrange
     *              要重排的范围
     * @param nth Iterator delimiting the position to settle
     *            界定待确定位置的迭代器
     */
    template <typename Range, typename Iter>
    constexpr auto nth_element(Range &&range, Iter nth) {
        return core::algorithm::nth_element(utility::begin(range), nth, utility::end(range));
    }

    /**
     * @brief Checks whether a range is sorted.
     *        检查范围是否有序。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to inspect
     *              要检查的范围
     * @return true if the range is sorted
     *         如果范围有序则为true
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto is_sorted(Range &&range) -> bool {
        return core::algorithm::is_sorted(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Finds the first position where a range is no longer sorted.
     *        查找范围不再有序的第一个位置。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to inspect
     *              要检查的范围
     * @return Iterator to the end of the longest sorted prefix
     *         指向最长有序前缀末尾的迭代器
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto is_sorted_until(Range &&range) {
        return core::algorithm::is_sorted_until(utility::begin(range), utility::end(range));
    }
}

namespace rainy::collections::views {
    using core::collections::views::is_sorted;
    using core::collections::views::is_sorted_until;
    using core::collections::views::nth_element;
    using core::collections::views::partial_sort;
    using core::collections::views::partial_sort_copy;
    using core::collections::views::sort;
    using core::collections::views::stable_sort;
}

#endif
