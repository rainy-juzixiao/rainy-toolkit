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
#ifndef RAINY_CORE_ALGORITHM_PARTITIONING_HPP
#define RAINY_CORE_ALGORITHM_PARTITIONING_HPP

#include <rainy/core/algorithm/modifying.hpp>
#include <rainy/core/collections/vector.hpp>

namespace rainy::core::algorithm {
    /**
     * \lang english
     * @brief Checks whether the range is partitioned by a predicate.
     *
     * @tparam InputIt Input iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param pred The predicate to apply
     * @return true if all elements satisfying pred precede all elements not satisfying it
     *
     * \lang simp-chinese
     * @brief 检查范围是否已按谓词分区。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param pred 要应用的谓词
     * @return 如果所有满足pred的元素都在不满足的元素之前则为true
     */
    template <typename InputIt, typename Pred>
    RAINY_NODISCARD constexpr rain_fn is_partitioned(InputIt first, InputIt last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                break;
            }
        }
        for (; first != last; ++first) {
            if (pred(*first)) {
                return false;
            }
        }
        return true;
    }

    /**
     * \lang english
     * @brief Reorders the range so that elements satisfying pred come first.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param pred The predicate to apply
     * @return Iterator to the first element of the second group
     *
     * \lang simp-chinese
     * @brief 重排范围使满足pred的元素排在前面。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向第二组第一个元素的迭代器
     */
    template <typename ForwardIt, typename Pred>
    constexpr rain_fn partition(ForwardIt first, ForwardIt last, Pred pred) -> ForwardIt {
        first = core::algorithm::find_if_not(first, last, pred);
        if (first == last) {
            return first;
        }
        for (ForwardIt it = first; ++it != last;) {
            if (pred(*it)) {
                core::algorithm::iter_swap(it, first);
                ++first;
            }
        }
        return first;
    }

    /**
     * \lang english
     * @brief Partitions a range, copying the two groups into separate destinations.
     *
     * @tparam InputIt Input iterator type
     * @tparam OutIt1 First output iterator type
     * @tparam OutIt2 Second output iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param dest_true Iterator to the destination of elements satisfying pred
     * @param dest_false Iterator to the destination of elements not satisfying pred
     * @return Pair of iterators to the ends of both destination ranges
     *
     * \lang simp-chinese
     * @brief 分区范围，将两组元素分别拷贝到不同目标。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam OutIt1 第一个输出迭代器类型
     * @tparam OutIt2 第二个输出迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param dest_true 满足pred元素的输出迭代器
     * @param dest_false 不满足pred元素的输出迭代器
     * @return 两个目标范围末尾的迭代器对
     */
    template <typename InputIt, typename OutIt1, typename OutIt2, typename Pred>
    constexpr rain_fn partition_copy(InputIt first, InputIt last, OutIt1 dest_true, OutIt2 dest_false, Pred pred)
        -> container::pair<OutIt1, OutIt2> {
        for (; first != last; ++first) {
            if (pred(*first)) {
                *dest_true = *first;
                ++dest_true;
            } else {
                *dest_false = *first;
                ++dest_false;
            }
        }
        return {dest_true, dest_false};
    }

    /**
     * \lang english
     * @brief Partitions the range preserving the relative order within each group.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param pred The predicate to apply
     * @return Iterator to the first element of the second group
     *
     * \lang simp-chinese
     * @brief 分区范围，同时保持各组内元素的相对顺序。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向第二组第一个元素的迭代器
     */
    template <typename ForwardIt, typename Pred>
    constexpr rain_fn stable_partition(ForwardIt first, ForwardIt last, Pred pred) -> ForwardIt {
        ForwardIt dest = first;
        collections::vector<typename utility::iterator_traits<ForwardIt>::value_type> false_group;
        for (; first != last; ++first) {
            if (pred(*first)) {
                *dest = utility::move(*first);
                ++dest;
            } else {
                false_group.push_back(utility::move(*first));
            }
        }
        for (auto &value : false_group) {
            *dest = utility::move(value);
            ++dest;
        }
        return last - static_cast<typename utility::iterator_traits<ForwardIt>::difference_type>(false_group.size());
    }

    /**
     * \lang english
     * @brief Finds the partition point of a partitioned range.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param pred The predicate to apply
     * @return Iterator to the first element not satisfying pred
     *
     * \lang simp-chinese
     * @brief 查找已分区范围的分区点。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向第一个不满足pred元素的迭代器
     */
    template <typename ForwardIt, typename Pred>
    RAINY_NODISCARD constexpr rain_fn partition_point(ForwardIt first, ForwardIt last, Pred pred) -> ForwardIt {
        auto count = utility::distance(first, last);
        while (count > 0) {
            auto step = count / 2;
            ForwardIt middle = first;
            utility::advance(middle, step);
            if (pred(*middle)) {
                first = ++middle;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }
}

namespace rainy::algorithm {
    using core::algorithm::is_partitioned;
    using core::algorithm::partition;
    using core::algorithm::partition_copy;
    using core::algorithm::partition_point;
    using core::algorithm::stable_partition;
}

#endif
