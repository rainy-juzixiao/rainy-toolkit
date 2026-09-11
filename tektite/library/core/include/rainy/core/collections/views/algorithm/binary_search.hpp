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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_BINARY_SEARCH_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_BINARY_SEARCH_HPP

#include <rainy/core/algorithm/binary_search.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief Finds the first element of a range greater than a value.
     *
     * @tparam Range The range type
     * @tparam Ty The value type
     * @param range The range to search
     * @param value The value to compare against
     * @return Iterator to the first element greater than value
     *
     * \lang simp-chinese
     * @brief 查找范围中第一个大于某值的元素。
     *
     * @tparam Range 范围类型
     * @tparam Ty 值类型
     * @param range 要搜索的范围
     * @param value 要比较的值
     * @return 指向第一个大于value元素的迭代器
     */
    template <typename Range, typename Ty>
    RAINY_NODISCARD constexpr auto upper_bound(Range &&range, const Ty &value) {
        return core::algorithm::upper_bound(utility::begin(range), utility::end(range), value);
    }

    /**
     * \lang english
     * @brief Finds the subrange of a range equal to a value.
     *
     * @tparam Range The range type
     * @tparam Ty The value type
     * @param range The range to search
     * @param value The value to compare against
     * @return Pair of iterators delimiting the equal range
     *
     * \lang simp-chinese
     * @brief 查找范围中等于某值的子范围。
     *
     * @tparam Range 范围类型
     * @tparam Ty 值类型
     * @param range 要搜索的范围
     * @param value 要比较的值
     * @return 界定相等范围的迭代器对
     */
    template <typename Range, typename Ty>
    RAINY_NODISCARD constexpr auto equal_range(Range &&range, const Ty &value) {
        return core::algorithm::equal_range(utility::begin(range), utility::end(range), value);
    }

    /**
     * \lang english
     * @brief Finds the first element of a range not less than a value (delegates to core lower_bound).
     *
     * @tparam Range The range type
     * @tparam Ty The value type
     * @param range The range to search
     * @param value The value to compare against
     * @return Iterator to the first element not less than value
     *
     * \lang simp-chinese
     * @brief 查找范围中第一个不小于某值的元素（委托core的lower_bound）。
     *
     * @tparam Range 范围类型
     * @tparam Ty 值类型
     * @param range 要搜索的范围
     * @param value 要比较的值
     * @return 指向第一个不小于value元素的迭代器
     */
    template <typename Range, typename Ty>
    RAINY_NODISCARD constexpr auto lower_bound(Range &&range, const Ty &value) {
        return core::algorithm::lower_bound(utility::begin(range), utility::end(range), value,
                                            [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * \lang english
     * @brief Checks whether a range contains a value (delegates to core binary_search).
     *
     * @tparam Range The range type
     * @tparam Ty The value type
     * @param range The range to search
     * @param value The value to find
     * @return true if the range contains value
     *
     * \lang simp-chinese
     * @brief 检查范围是否包含某值（委托core的binary_search）。
     *
     * @tparam Range 范围类型
     * @tparam Ty 值类型
     * @param range 要搜索的范围
     * @param value 要查找的值
     * @return 如果范围包含value则为true
     */
    template <typename Range, typename Ty>
    RAINY_NODISCARD constexpr auto binary_search(Range &&range, const Ty &value) -> bool {
        return core::algorithm::binary_search(utility::begin(range), utility::end(range), value,
                                              [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::collections::views {
    using core::collections::views::binary_search;
    using core::collections::views::equal_range;
    using core::collections::views::lower_bound;
    using core::collections::views::upper_bound;
}

#endif
