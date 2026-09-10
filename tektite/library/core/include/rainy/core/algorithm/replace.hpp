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
#ifndef RAINY_CORE_ALGORITHM_REPLACE_HPP
#define RAINY_CORE_ALGORITHM_REPLACE_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>

namespace rainy::core::algorithm {
    /**
     * @brief Replaces every element equal to old_value in-place with new_value.
     *        就地将每个等于old_value的元素替换为new_value。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Ty The element value type
     *            元素值类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param old_value The value to be replaced
     *                  要被替换的值
     * @param new_value The replacement value
     *                  替换值
     */
    template <typename ForwardIt, typename Ty>
    constexpr rain_fn replace(ForwardIt first, ForwardIt last, const Ty &old_value, const Ty &new_value)
        noexcept(noexcept(*first == old_value) && noexcept(*first = new_value)) -> void {
        for (; first != last; ++first) {
            if (*first == old_value) {
                *first = new_value;
            }
        }
    }

    /**
     * @brief Replaces every element satisfying pred in-place with new_value.
     *        就地将每个满足pred的元素替换为new_value。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Pred The predicate type
     *              谓词类型
     * @tparam Ty The element value type
     *            元素值类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param pred The predicate to apply
     *             要应用的谓词
     * @param new_value The replacement value
     *                  替换值
     */
    template <typename ForwardIt, typename Pred, typename Ty>
    constexpr rain_fn replace_if(ForwardIt first, ForwardIt last, Pred pred, const Ty &new_value)
        noexcept(noexcept(pred(*first)) && noexcept(*first = new_value)) -> void {
        for (; first != last; ++first) {
            if (pred(*first)) {
                *first = new_value;
            }
        }
    }

    /**
     * @brief Copies the range, replacing every element equal to old_value with new_value.
     *        拷贝范围，将每个等于old_value的元素替换为new_value。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Ty The element value type
     *            元素值类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *             指向源范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @param old_value The value to be replaced
     *                  要被替换的值
     * @param new_value The replacement value
     *                  替换值
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIt, typename OutIt, typename Ty>
    constexpr rain_fn replace_copy(InputIt first, InputIt last, OutIt dest, const Ty &old_value, const Ty &new_value)
        noexcept(noexcept(*first == old_value) && noexcept(*dest = new_value)) -> OutIt {
        for (; first != last; ++first, ++dest) {
            *dest = *first == old_value ? new_value : *first;
        }
        return dest;
    }

    /**
     * @brief Copies the range, replacing every element satisfying pred with new_value.
     *        拷贝范围，将每个满足pred的元素替换为new_value。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam OutIt Output iterator type
     *               输出迭代器类型
     * @tparam Pred The predicate type
     *              谓词类型
     * @tparam Ty The element value type
     *            元素值类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *             指向源范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @param pred The predicate to apply
     *             要应用的谓词
     * @param new_value The replacement value
     *                  替换值
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIt, typename OutIt, typename Pred, typename Ty>
    constexpr rain_fn replace_copy_if(InputIt first, InputIt last, OutIt dest, Pred pred, const Ty &new_value)
        noexcept(noexcept(pred(*first)) && noexcept(*dest = new_value)) -> OutIt {
        for (; first != last; ++first, ++dest) {
            *dest = pred(*first) ? new_value : *first;
        }
        return dest;
    }
}

#endif
