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
#ifndef RAINY_CORE_ALGORITHM_MINMAX_HPP
#define RAINY_CORE_ALGORITHM_MINMAX_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>

namespace rainy::core::algorithm {
    /**
     * @brief Returns the smaller of two values.
     *        返回两个值中较小的一个。
     *
     * @tparam Ty The value type
     *            值类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param left The first value
     *             第一个值
     * @param right The second value
     *              第二个值
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return The smaller value
     *         较小的值
     */
    template <typename Ty, typename Comp>
    RAINY_NODISCARD constexpr rain_fn min(const Ty &left, const Ty &right, Comp comp) -> const Ty & {
        return comp(right, left) ? right : left;
    }

    /**
     * @brief Returns the smaller of two values (operator<).
     *        返回两个值中较小的一个（operator<）。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn min(const Ty &left, const Ty &right) -> const Ty & {
        return core::algorithm::min(left, right, [](const Ty &l, const Ty &r) { return l < r; });
    }

    /**
     * @brief Returns the greater of two values.
     *        返回两个值中较大的一个。
     */
    template <typename Ty, typename Comp>
    RAINY_NODISCARD constexpr rain_fn max(const Ty &left, const Ty &right, Comp comp) -> const Ty & {
        return comp(left, right) ? right : left;
    }

    /**
     * @brief Returns the greater of two values (operator<).
     *        返回两个值中较大的一个（operator<）。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn max(const Ty &left, const Ty &right) -> const Ty & {
        return core::algorithm::max(left, right, [](const Ty &l, const Ty &r) { return l < r; });
    }

    /**
     * @brief Returns both the smaller and greater of two values.
     *        同时返回两个值中较小和较大的一个。
     */
    template <typename Ty, typename Comp>
    RAINY_NODISCARD constexpr rain_fn minmax(const Ty &left, const Ty &right, Comp comp) -> container::pair<const Ty &, const Ty &> {
        if (comp(right, left)) {
            return {right, left};
        }
        return {left, right};
    }

    /**
     * @brief Returns both the smaller and greater of two values (operator<).
     *        同时返回两个值中较小和较大的一个（operator<）。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn minmax(const Ty &left, const Ty &right) -> container::pair<const Ty &, const Ty &> {
        return core::algorithm::minmax(left, right, [](const Ty &l, const Ty &r) { return l < r; });
    }

    /**
     * @brief Clamps a value into the [low, high] range with respect to comp.
     *        按comp将值限制在[low, high]范围内。
     *
     * @tparam Ty The value type
     *            值类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param value The value to clamp
     *              要限制的值
     * @param low The lower bound
     *             下界
     * @param high The upper bound
     *             上界
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return The clamped value
     *         限制后的值
     */
    template <typename Ty, typename Comp>
    RAINY_NODISCARD constexpr rain_fn clamp(const Ty &value, const Ty &low, const Ty &high, Comp comp) -> const Ty & {
        return comp(value, low) ? low : comp(high, value) ? high : value;
    }

    /**
     * @brief Clamps a value into the [low, high] range (operator<).
     *        将值限制在[low, high]范围内（operator<）。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn clamp(const Ty &value, const Ty &low, const Ty &high) -> const Ty & {
        return core::algorithm::clamp(value, low, high, [](const Ty &l, const Ty &r) { return l < r; });
    }

    /**
     * @brief Finds the smallest element of a range.
     *        查找范围中最小的元素。
     */
    template <typename ForwardIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn min_element(ForwardIt first, ForwardIt last, Comp comp) -> ForwardIt {
        if (first == last) {
            return last;
        }
        ForwardIt smallest = first;
        for (++first; first != last; ++first) {
            if (comp(*first, *smallest)) {
                smallest = first;
            }
        }
        return smallest;
    }

    /**
     * @brief Finds the smallest element of a range (operator<).
     *        查找范围中最小的元素（operator<）。
     */
    template <typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn min_element(ForwardIt first, ForwardIt last) -> ForwardIt {
        return core::algorithm::min_element(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Finds the greatest element of a range.
     *        查找范围中最大的元素。
     */
    template <typename ForwardIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn max_element(ForwardIt first, ForwardIt last, Comp comp) -> ForwardIt {
        if (first == last) {
            return last;
        }
        ForwardIt greatest = first;
        for (++first; first != last; ++first) {
            if (comp(*greatest, *first)) {
                greatest = first;
            }
        }
        return greatest;
    }

    /**
     * @brief Finds the greatest element of a range (operator<).
     *        查找范围中最大的元素（operator<）。
     */
    template <typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn max_element(ForwardIt first, ForwardIt last) -> ForwardIt {
        return core::algorithm::max_element(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Finds both the smallest and greatest elements of a range.
     *        同时查找范围中最小和最大的元素。
     */
    template <typename ForwardIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn minmax_element(ForwardIt first, ForwardIt last, Comp comp)
        -> container::pair<ForwardIt, ForwardIt> {
        if (first == last) {
            return {last, last};
        }
        ForwardIt smallest = first;
        ForwardIt greatest = first;
        for (++first; first != last; ++first) {
            if (comp(*first, *smallest)) {
                smallest = first;
            }
            if (!comp(*first, *greatest)) {
                greatest = first;
            }
        }
        return {smallest, greatest};
    }

    /**
     * @brief Finds both the smallest and greatest elements of a range (operator<).
     *        同时查找范围中最小和最大的元素（operator<）。
     */
    template <typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn minmax_element(ForwardIt first, ForwardIt last) -> container::pair<ForwardIt, ForwardIt> {
        return core::algorithm::minmax_element(first, last, [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::algorithm {
    using core::algorithm::clamp;
    using core::algorithm::max;
    using core::algorithm::max_element;
    using core::algorithm::min;
    using core::algorithm::min_element;
    using core::algorithm::minmax;
    using core::algorithm::minmax_element;
}

#endif
