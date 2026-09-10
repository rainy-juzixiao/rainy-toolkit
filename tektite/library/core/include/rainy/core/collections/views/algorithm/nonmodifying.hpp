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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_NONMODIFYING_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_NONMODIFYING_HPP

#include <rainy/core/algorithm/nonmodifying.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * @brief Applies a function to every element of a range.
     *        对范围的每个元素应用函数。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Fx The function type
     *            函数类型
     * @param range The range to apply the function to
     *              要应用函数的范围
     * @param fx The function to apply
     *           要应用的函数
     * @return The moved function object
     *         移动后的函数对象
     */
    template <typename Range, typename Fx>
    constexpr auto for_each(Range &&range, Fx fx) {
        return core::algorithm::for_each(utility::begin(range), utility::end(range), utility::move(fx));
    }

    /**
     * @brief Applies a function to the first n elements of a range.
     *        对范围的前n个元素应用函数。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Size The count type
     *              数量类型
     * @tparam Fx The function type
     *            函数类型
     * @param range The range to apply the function to
     *              要应用函数的范围
     * @param n The number of elements to visit
     *          要访问的元素数量
     * @param fx The function to apply
     *           要应用的函数
     * @return Iterator one past the last element visited
     *         指向最后一个被访问元素之后的迭代器
     */
    template <typename Range, typename Size, typename Fx>
    constexpr auto for_each_n(Range &&range, Size n, Fx fx) {
        return core::algorithm::for_each_n(utility::begin(range), n, utility::move(fx));
    }

    /**
     * @brief Counts the elements of a range equal to a value.
     *        统计范围中等于某值的元素数量。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Ty The value type to count
     *            要统计的值类型
     * @param range The range to count in
     *              要统计的范围
     * @param value The value to count
     *              要统计的值
     * @return The number of matching elements
     *         匹配元素的数量
     */
    template <typename Range, typename Ty>
    RAINY_NODISCARD constexpr auto count(Range &&range, const Ty &value) {
        return core::algorithm::count(utility::begin(range), utility::end(range), value);
    }

    /**
     * @brief Counts the elements of a range satisfying a predicate.
     *        统计范围中满足谓词的元素数量。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Pred The predicate type
     *              谓词类型
     * @param range The range to count in
     *              要统计的范围
     * @param pred The predicate to apply
     *             要应用的谓词
     * @return The number of matching elements
     *         匹配元素的数量
     */
    template <typename Range, typename Pred>
    RAINY_NODISCARD constexpr auto count_if(Range &&range, Pred pred) {
        return core::algorithm::count_if(utility::begin(range), utility::end(range), utility::move(pred));
    }

    /**
     * @brief Finds the first position where a range and a second sequence differ.
     *        查找范围与第二个序列第一个不同的位置。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam InputIt The second sequence iterator type
     *                 第二个序列迭代器类型
     * @param range The range to compare
     *              要比较的范围
     * @param first2 Iterator to the beginning of the second sequence
     *               指向第二个序列起始的迭代器
     * @return A pair of iterators to the first mismatching elements
     *         指向第一对不同元素的迭代器对
     */
    template <typename Range, typename InputIt>
    RAINY_NODISCARD constexpr auto mismatch(Range &&range, InputIt first2) {
        return core::algorithm::mismatch(utility::begin(range), utility::end(range), first2);
    }

    /**
     * @brief Finds the first adjacent pair of equal elements in a range.
     *        查找范围中第一对相邻相等元素。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to search
     *              要搜索的范围
     * @return Iterator to the first of the adjacent equal elements, or end
     *         指向相邻相等元素中第一个的迭代器，未找到则返回end
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto adjacent_find(Range &&range) {
        return core::algorithm::adjacent_find(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Searches a range for the first occurrence of a subsequence.
     *        在范围中查找子序列的第一次出现。
     *
     * @tparam Range The range type to search
     *               被搜索的范围类型
     * @tparam SubRange The subsequence range type
     *                  子序列范围类型
     * @param range The range to search
     *              要搜索的范围
     * @param sub The subsequence to find
     *            要查找的子序列
     * @return Iterator to the beginning of the first occurrence, or end
     *         指向第一次出现位置起始的迭代器，未找到则返回end
     */
    template <typename Range, typename SubRange>
    RAINY_NODISCARD constexpr auto search(Range &&range, SubRange &&sub) {
        return core::algorithm::search(utility::begin(range), utility::end(range), utility::begin(sub), utility::end(sub));
    }

    /**
     * @brief Searches a range for the last occurrence of a subsequence.
     *        在范围中查找子序列的最后一次出现。
     *
     * @tparam Range The range type to search
     *               被搜索的范围类型
     * @tparam SubRange The subsequence range type
     *                  子序列范围类型
     * @param range The range to search
     *              要搜索的范围
     * @param sub The subsequence to find
     *            要查找的子序列
     * @return Iterator to the beginning of the last occurrence, or end
     *         指向最后一次出现位置起始的迭代器，未找到则返回end
     */
    template <typename Range, typename SubRange>
    RAINY_NODISCARD constexpr auto find_end(Range &&range, SubRange &&sub) {
        return core::algorithm::find_end(utility::begin(range), utility::end(range), utility::begin(sub), utility::end(sub));
    }

    /**
     * @brief Finds the first element of a range that is also in a set.
     *        查找范围中第一个也在集合中的元素。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam SetRange The set range type
     *                  集合范围类型
     * @param range The range to search
     *              要搜索的范围
     * @param set The set of values to look for
     *            要查找的值集合
     * @return Iterator to the first found element, or end
     *         指向第一个找到元素的迭代器，未找到则返回end
     */
    template <typename Range, typename SetRange>
    RAINY_NODISCARD constexpr auto find_first_of(Range &&range, SetRange &&set) {
        return core::algorithm::find_first_of(utility::begin(range), utility::end(range), utility::begin(set),
                                              utility::end(set));
    }

    /**
     * @brief Searches a range for n consecutive copies of a value.
     *        在范围中查找n个连续相等的值。
     *
     * @tparam Range The range type
     *               范围类型
     * @tparam Size The count type
     *              数量类型
     * @tparam Ty The value type
     *            值类型
     * @param range The range to search
     *              要搜索的范围
     * @param n The length of the sequence to find
     *          要查找序列的长度
     * @param value The value to find
     *              要查找的值
     * @return Iterator to the beginning of the found sequence, or end
     *         指向找到序列起始的迭代器，未找到则返回end
     */
    template <typename Range, typename Size, typename Ty>
    RAINY_NODISCARD constexpr auto search_n(Range &&range, Size n, const Ty &value) {
        return core::algorithm::search_n(utility::begin(range), utility::end(range), n, value);
    }
}

namespace rainy::collections::views {
    using core::collections::views::adjacent_find;
    using core::collections::views::count;
    using core::collections::views::count_if;
    using core::collections::views::find_end;
    using core::collections::views::find_first_of;
    using core::collections::views::for_each;
    using core::collections::views::for_each_n;
    using core::collections::views::mismatch;
    using core::collections::views::search;
    using core::collections::views::search_n;
}

#endif
