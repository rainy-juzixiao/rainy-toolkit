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
#ifndef RAINY_CORE_ALGORITHM_PERMUTATION_HPP
#define RAINY_CORE_ALGORITHM_PERMUTATION_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <rainy/core/algorithm/modifying.hpp>

namespace rainy::core::algorithm {
    /**
     * \lang english
     * @brief Checks whether two ranges are permutations of each other.
     *
     * @tparam InputIt1 First input iterator type
     * @tparam InputIt2 Second input iterator type
     * @param first1 Iterator to the beginning of the first range
     * @param last1 Iterator to the end of the first range
     * @param first2 Iterator to the beginning of the second range
     * @return true if the ranges are permutations of each other
     *
     * \lang simp-chinese
     * @brief 检查两个范围是否互为排列。
     *
     * @tparam InputIt1 第一个输入迭代器类型
     * @tparam InputIt2 第二个输入迭代器类型
     * @param first1 指向第一个范围起始的迭代器
     * @param last1 指向第一个范围末尾的迭代器
     * @param first2 指向第二个范围起始的迭代器
     * @return 如果两个范围互为排列则为true
     */
    template <typename InputIt1, typename InputIt2>
    RAINY_NODISCARD constexpr rain_fn is_permutation(InputIt1 first1, InputIt1 last1, InputIt2 first2) -> bool {
        for (; first1 != last1; ++first1, ++first2) {
            if (*first1 != *first2) {
                break;
            }
        }
        if (first1 == last1) {
            return true;
        }
        InputIt2 last2 = first2;
        utility::advance(last2, utility::distance(first1, last1));
        for (InputIt1 it = first1; it != last1; ++it) {
            if (core::algorithm::find(first1, it, *it) != it) {
                continue;
            }
            auto matches = core::algorithm::count(first2, last2, *it);
            if (matches == 0 || core::algorithm::count(first1, last1, *it) != matches) {
                return false;
            }
        }
        return true;
    }

    /**
     * \lang english
     * @brief Transforms the range into the next lexicographic permutation; returns true if one exists.
     *
     * @tparam BidirIt Bidirectional iterator type
     * @tparam Comp The comparison type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param comp The comparison to apply
     * @return true if the next permutation exists
     *
     * \lang simp-chinese
     * @brief 将范围变换为下一个字典序排列；存在则返回true。
     *
     * @tparam BidirIt 双向迭代器类型
     * @tparam Comp 比较器类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param comp 要应用的比较器
     * @return 如果存在下一个排列则为true
     */
    template <typename BidirIt, typename Comp>
    constexpr rain_fn next_permutation(BidirIt first, BidirIt last, Comp comp) -> bool {
        if (first == last || first + 1 == last) {
            return false;
        }
        BidirIt i = last - 1;
        for (;;) {
            BidirIt previous = i;
            --i;
            if (comp(*i, *previous)) {
                BidirIt successor = last;
                while (comp(*--successor, *i)) {
                }
                core::algorithm::iter_swap(i, successor);
                core::algorithm::reverse(previous, last);
                return true;
            }
            if (i == first) {
                core::algorithm::reverse(first, last);
                return false;
            }
        }
    }

    /**
     * \lang english
     * @brief Transforms the range into the next lexicographic permutation (operator<).
     *
     * \lang simp-chinese
     * @brief 将范围变换为下一个字典序排列（operator<）。
     */
    template <typename BidirIt>
    constexpr rain_fn next_permutation(BidirIt first, BidirIt last) -> bool {
        return core::algorithm::next_permutation(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * \lang english
     * @brief Transforms the range into the previous lexicographic permutation; returns true if one exists.
     *
     * @tparam BidirIt Bidirectional iterator type
     * @tparam Comp The comparison type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param comp The comparison to apply
     * @return true if the previous permutation exists
     *
     * \lang simp-chinese
     * @brief 将范围变换为上一个字典序排列；存在则返回true。
     *
     * @tparam BidirIt 双向迭代器类型
     * @tparam Comp 比较器类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param comp 要应用的比较器
     * @return 如果存在上一个排列则为true
     */
    template <typename BidirIt, typename Comp>
    constexpr rain_fn prev_permutation(BidirIt first, BidirIt last, Comp comp) -> bool {
        if (first == last || first + 1 == last) {
            return false;
        }
        BidirIt i = last - 1;
        for (;;) {
            BidirIt previous = i;
            --i;
            if (comp(*previous, *i)) {
                BidirIt successor = last;
                while (!comp(*--successor, *i)) {
                }
                core::algorithm::iter_swap(i, successor);
                core::algorithm::reverse(previous, last);
                return true;
            }
            if (i == first) {
                core::algorithm::reverse(first, last);
                return false;
            }
        }
    }

    /**
     * \lang english
     * @brief Transforms the range into the previous lexicographic permutation (operator<).
     *
     * \lang simp-chinese
     * @brief 将范围变换为上一个字典序排列（operator<）。
     */
    template <typename BidirIt>
    constexpr rain_fn prev_permutation(BidirIt first, BidirIt last) -> bool {
        return core::algorithm::prev_permutation(first, last, [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::algorithm {
    using core::algorithm::is_permutation;
    using core::algorithm::next_permutation;
    using core::algorithm::prev_permutation;
}

#endif
