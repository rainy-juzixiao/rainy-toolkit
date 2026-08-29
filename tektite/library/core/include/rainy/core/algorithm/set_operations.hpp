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
#ifndef RAINY_CORE_ALGORITHM_SET_OPERATIONS_HPP
#define RAINY_CORE_ALGORITHM_SET_OPERATIONS_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <rainy/core/collections/vector.hpp>

namespace rainy::core::algorithm {
    /**
     * @brief Checks whether one sorted range contains another sorted range.
     *        检查一个有序范围是否包含另一个有序范围。
     *
     * @tparam InputIt1 First input iterator type
     *                  第一个输入迭代器类型
     * @tparam InputIt2 Second input iterator type
     *                  第二个输入迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first1 Iterator to the beginning of the containing range
     *               指向包含范围起始的迭代器
     * @param last1 Iterator to the end of the containing range
     *              指向包含范围末尾的迭代器
     * @param first2 Iterator to the beginning of the contained range
     *               指向被包含范围起始的迭代器
     * @param last2 Iterator to the end of the contained range
     *              指向被包含范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return true if the second range is a subsequence of the first
     *         如果第二个范围是第一个范围的子序列则为true
     */
    template <typename InputIt1, typename InputIt2, typename Comp>
    RAINY_NODISCARD constexpr rain_fn includes(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, Comp comp)
        -> bool {
        for (; first2 != last2; ++first1) {
            if (first1 == last1 || comp(*first2, *first1)) {
                return false;
            }
            if (!comp(*first1, *first2)) {
                ++first2;
            }
        }
        return true;
    }

    /**
     * @brief Checks whether one sorted range contains another (operator<).
     *        检查一个有序范围是否包含另一个（operator<）。
     */
    template <typename InputIt1, typename InputIt2>
    RAINY_NODISCARD constexpr rain_fn includes(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) -> bool {
        return core::algorithm::includes(first1, last1, first2, last2,
                                         [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Merges two sorted ranges into a destination.
     *        将两个有序范围合并到目标位置。
     *
     * @tparam InputIt1 First input iterator type
     *                  第一个输入迭代器类型
     * @tparam InputIt2 Second input iterator type
     *                  第二个输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first1 Iterator to the beginning of the first range
     *               指向第一个范围起始的迭代器
     * @param last1 Iterator to the end of the first range
     *              指向第一个范围末尾的迭代器
     * @param first2 Iterator to the beginning of the second range
     *               指向第二个范围起始的迭代器
     * @param last2 Iterator to the end of the second range
     *              指向第二个范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination
     *             指向目标起始的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIt1, typename InputIt2, typename OutIt, typename Comp>
    constexpr rain_fn merge(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest, Comp comp) -> OutIt {
        for (; first1 != last1 && first2 != last2; ++dest) {
            if (comp(*first2, *first1)) {
                *dest = *first2;
                ++first2;
            } else {
                *dest = *first1;
                ++first1;
            }
        }
        for (; first1 != last1; ++first1, ++dest) {
            *dest = *first1;
        }
        for (; first2 != last2; ++first2, ++dest) {
            *dest = *first2;
        }
        return dest;
    }

    /**
     * @brief Merges two sorted ranges into a destination (operator<).
     *        将两个有序范围合并到目标位置（operator<）。
     */
    template <typename InputIt1, typename InputIt2, typename OutIt>
    constexpr rain_fn merge(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest) -> OutIt {
        return core::algorithm::merge(first1, last1, first2, last2, dest,
                                      [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Merges two adjacent sorted halves of a range in-place.
     *        就地合并范围内两个相邻的有序半区。
     *
     * @tparam BidirIt Bidirectional iterator type
     *                 双向迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param middle Iterator to the beginning of the second sorted half
     *               指向第二个有序半区起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename BidirIt, typename Comp>
    constexpr rain_fn inplace_merge(BidirIt first, BidirIt middle, BidirIt last, Comp comp) -> void {
        using value_type = typename utility::iterator_traits<BidirIt>::value_type;
        if (first == middle || middle == last) {
            return;
        }
        collections::vector<value_type> buffer;
        buffer.reserve(static_cast<std::size_t>(last - first));
        BidirIt left = first;
        BidirIt right = middle;
        while (left != middle && right != last) {
            if (comp(*right, *left)) {
                buffer.push_back(utility::move(*right));
                ++right;
            } else {
                buffer.push_back(utility::move(*left));
                ++left;
            }
        }
        while (left != middle) {
            buffer.push_back(utility::move(*left));
            ++left;
        }
        while (right != last) {
            buffer.push_back(utility::move(*right));
            ++right;
        }
        BidirIt out = first;
        for (auto &value : buffer) {
            *out = utility::move(value);
            ++out;
        }
    }

    /**
     * @brief Merges two adjacent sorted halves of a range in-place (operator<).
     *        就地合并范围内两个相邻的有序半区（operator<）。
     */
    template <typename BidirIt>
    constexpr rain_fn inplace_merge(BidirIt first, BidirIt middle, BidirIt last) -> void {
        core::algorithm::inplace_merge(first, middle, last, [](const auto &left, const auto &right) { return left < right; });
    }

    namespace implements {
        template <typename InputIt1, typename InputIt2, typename OutIt, typename Comp, typename Picker1, typename Picker2,
                  typename PickerBoth>
        constexpr OutIt set_operation(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest, Comp comp,
                                      Picker1 take_left, Picker2 take_right, PickerBoth take_both) {
            for (; first1 != last1 && first2 != last2;) {
                if (comp(*first1, *first2)) {
                    if (take_left) {
                        *dest = *first1;
                        ++dest;
                    }
                    ++first1;
                } else if (comp(*first2, *first1)) {
                    if (take_right) {
                        *dest = *first2;
                        ++dest;
                    }
                    ++first2;
                } else {
                    if (take_both) {
                        *dest = *first1;
                        ++dest;
                    }
                    ++first1;
                    ++first2;
                }
            }
            if (take_left) {
                for (; first1 != last1; ++first1, ++dest) {
                    *dest = *first1;
                }
            }
            if (take_right) {
                for (; first2 != last2; ++first2, ++dest) {
                    *dest = *first2;
                }
            }
            return dest;
        }
    }

    /**
     * @brief Computes the union of two sorted ranges into a destination.
     *        计算两个有序范围的并集到目标位置。
     *
     * @tparam InputIt1 First input iterator type
     *                  第一个输入迭代器类型
     * @tparam InputIt2 Second input iterator type
     *                  第二个输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     */
    template <typename InputIt1, typename InputIt2, typename OutIt, typename Comp>
    constexpr rain_fn set_union(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest, Comp comp)
        -> OutIt {
        return implements::set_operation(first1, last1, first2, last2, dest, comp, std::true_type{}, std::true_type{},
                                         std::true_type{});
    }

    /**
     * @brief Computes the union of two sorted ranges (operator<).
     *        计算两个有序范围的并集（operator<）。
     */
    template <typename InputIt1, typename InputIt2, typename OutIt>
    constexpr rain_fn set_union(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest) -> OutIt {
        return core::algorithm::set_union(first1, last1, first2, last2, dest,
                                          [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Computes the intersection of two sorted ranges into a destination.
     *        计算两个有序范围的交集到目标位置。
     *
     * @tparam InputIt1 First input iterator type
     *                  第一个输入迭代器类型
     * @tparam InputIt2 Second input iterator type
     *                  第二个输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     */
    template <typename InputIt1, typename InputIt2, typename OutIt, typename Comp>
    constexpr rain_fn set_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest, Comp comp)
        -> OutIt {
        return implements::set_operation(first1, last1, first2, last2, dest, comp, std::false_type{}, std::false_type{},
                                         std::true_type{});
    }

    /**
     * @brief Computes the intersection of two sorted ranges (operator<).
     *        计算两个有序范围的交集（operator<）。
     */
    template <typename InputIt1, typename InputIt2, typename OutIt>
    constexpr rain_fn set_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest) -> OutIt {
        return core::algorithm::set_intersection(first1, last1, first2, last2, dest,
                                                 [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Computes the difference of two sorted ranges into a destination.
     *        计算两个有序范围的差集到目标位置。
     *
     * @tparam InputIt1 First input iterator type
     *                  第一个输入迭代器类型
     * @tparam InputIt2 Second input iterator type
     *                  第二个输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     */
    template <typename InputIt1, typename InputIt2, typename OutIt, typename Comp>
    constexpr rain_fn set_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest, Comp comp)
        -> OutIt {
        return implements::set_operation(first1, last1, first2, last2, dest, comp, std::true_type{}, std::false_type{},
                                         std::false_type{});
    }

    /**
     * @brief Computes the difference of two sorted ranges (operator<).
     *        计算两个有序范围的差集（operator<）。
     */
    template <typename InputIt1, typename InputIt2, typename OutIt>
    constexpr rain_fn set_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest) -> OutIt {
        return core::algorithm::set_difference(first1, last1, first2, last2, dest,
                                               [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Computes the symmetric difference of two sorted ranges into a destination.
     *        计算两个有序范围的对称差到目标位置。
     *
     * @tparam InputIt1 First input iterator type
     *                  第一个输入迭代器类型
     * @tparam InputIt2 Second input iterator type
     *                  第二个输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     */
    template <typename InputIt1, typename InputIt2, typename OutIt, typename Comp>
    constexpr rain_fn set_symmetric_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest,
                                               Comp comp) -> OutIt {
        return implements::set_operation(first1, last1, first2, last2, dest, comp, std::true_type{}, std::true_type{},
                                         std::false_type{});
    }

    /**
     * @brief Computes the symmetric difference of two sorted ranges (operator<).
     *        计算两个有序范围的对称差（operator<）。
     */
    template <typename InputIt1, typename InputIt2, typename OutIt>
    constexpr rain_fn set_symmetric_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutIt dest)
        -> OutIt {
        return core::algorithm::set_symmetric_difference(first1, last1, first2, last2, dest,
                                                         [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::algorithm {
    using core::algorithm::includes;
    using core::algorithm::inplace_merge;
    using core::algorithm::merge;
    using core::algorithm::set_difference;
    using core::algorithm::set_intersection;
    using core::algorithm::set_symmetric_difference;
    using core::algorithm::set_union;
}

#endif
