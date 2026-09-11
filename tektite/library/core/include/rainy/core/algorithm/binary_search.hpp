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
#ifndef RAINY_CORE_ALGORITHM_BINARY_SEARCH_HPP
#define RAINY_CORE_ALGORITHM_BINARY_SEARCH_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>

namespace rainy::core::algorithm {
    /**
     * \lang english
     * @brief Finds the first element greater than a value in a partitioned range (operator<).
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Ty The value type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param value The value to compare against
     * @return Iterator to the first element greater than value
     *
     * \lang simp-chinese
     * @brief 在分区范围中查找第一个大于某值的元素（operator<）。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Ty 值类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param value 要比较的值
     * @return 指向第一个大于value元素的迭代器
     */
    template <typename ForwardIt, typename Ty>
    RAINY_NODISCARD constexpr rain_fn upper_bound(ForwardIt first, ForwardIt last, const Ty &value) -> ForwardIt {
        return core::algorithm::upper_bound(first, last, value,
                                            [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * \lang english
     * @brief Finds the subrange of elements equal to a value in a partitioned range.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Ty The value type
     * @tparam Comp The comparison type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param value The value to compare against
     * @param comp The comparison to apply
     * @return Pair of iterators delimiting the equal range
     *
     * \lang simp-chinese
     * @brief 在分区范围中查找等于某值的子范围。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Ty 值类型
     * @tparam Comp 比较器类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param value 要比较的值
     * @param comp 要应用的比较器
     * @return 界定相等范围的迭代器对
     */
    template <typename ForwardIt, typename Ty, typename Comp>
    RAINY_NODISCARD constexpr rain_fn equal_range(ForwardIt first, ForwardIt last, const Ty &value, Comp comp)
        -> container::pair<ForwardIt, ForwardIt> {
        ForwardIt lower = core::algorithm::lower_bound(first, last, value, comp);
        ForwardIt upper = core::algorithm::upper_bound(lower, last, value, comp);
        return {lower, upper};
    }

    /**
     * \lang english
     * @brief Finds the subrange of elements equal to a value (operator<).
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Ty The value type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param value The value to compare against
     *
     * \lang simp-chinese
     * @brief 查找等于某值的子范围（operator<）。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Ty 值类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param value 要比较的值
     */
    template <typename ForwardIt, typename Ty>
    RAINY_NODISCARD constexpr rain_fn equal_range(ForwardIt first, ForwardIt last, const Ty &value)
        -> container::pair<ForwardIt, ForwardIt> {
        return core::algorithm::equal_range(first, last, value, [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::algorithm {
    using core::algorithm::equal_range;
    using core::algorithm::upper_bound;
    using core::algorithm::upper_bound;
}

#endif
