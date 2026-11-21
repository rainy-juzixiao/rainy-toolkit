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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_PARTITIONING_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_PARTITIONING_HPP

#include <rainy/core/algorithm/partitioning.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief Checks whether a range is partitioned by a predicate.
     *
     * @tparam Range The range type
     * @tparam Pred The predicate type
     * @param range The range to check
     * @param pred The predicate to apply
     * @return true if the range is partitioned
     *
     * \lang simp-chinese
     * @brief 检查范围是否已按谓词分区。
     *
     * @tparam Range 范围类型
     * @tparam Pred 谓词类型
     * @param range 要检查的范围
     * @param pred 要应用的谓词
     * @return 如果范围已分区则为true
     */
    template <typename Range, typename Pred>
    RAINY_NODISCARD constexpr auto is_partitioned(Range &&range, Pred pred) -> bool {
        return core::algorithm::is_partitioned(utility::begin(range), utility::end(range), utility::move(pred));
    }

    /**
     * \lang english
     * @brief Reorders a range so that elements satisfying pred come first.
     *
     * @tparam Range The range type
     * @tparam Pred The predicate type
     * @param range The range to partition
     * @param pred The predicate to apply
     * @return Iterator to the first element of the second group
     *
     * \lang simp-chinese
     * @brief 重排范围使满足pred的元素排在前面。
     *
     * @tparam Range 范围类型
     * @tparam Pred 谓词类型
     * @param range 要分区的范围
     * @param pred 要应用的谓词
     * @return 指向第二组第一个元素的迭代器
     */
    template <typename Range, typename Pred>
    constexpr auto partition(Range &&range, Pred pred) {
        return core::algorithm::partition(utility::begin(range), utility::end(range), utility::move(pred));
    }

    /**
     * \lang english
     * @brief Partitions a range, copying the two groups into separate destinations.
     *
     * @tparam Range The range type
     * @tparam OutIt1 First output iterator type
     * @tparam OutIt2 Second output iterator type
     * @tparam Pred The predicate type
     * @param range The range to partition
     * @param dest_true Iterator to the destination of elements satisfying pred
     * @param dest_false Iterator to the destination of elements not satisfying pred
     * @return Pair of iterators to the ends of both destination ranges
     *
     * \lang simp-chinese
     * @brief 分区范围，将两组元素分别拷贝到不同目标。
     *
     * @tparam Range 范围类型
     * @tparam OutIt1 第一个输出迭代器类型
     * @tparam OutIt2 第二个输出迭代器类型
     * @tparam Pred 谓词类型
     * @param range 要分区的范围
     * @param dest_true 满足pred元素的输出迭代器
     * @param dest_false 不满足pred元素的输出迭代器
     * @return 两个目标范围末尾的迭代器对
     */
    template <typename Range, typename OutIt1, typename OutIt2, typename Pred>
    constexpr auto partition_copy(Range &&range, OutIt1 dest_true, OutIt2 dest_false, Pred pred) {
        return core::algorithm::partition_copy(utility::begin(range), utility::end(range), dest_true, dest_false,
                                               utility::move(pred));
    }

    /**
     * \lang english
     * @brief Partitions a range preserving the relative order within each group.
     *
     * @tparam Range The range type
     * @tparam Pred The predicate type
     * @param range The range to partition
     * @param pred The predicate to apply
     * @return Iterator to the first element of the second group
     *
     * \lang simp-chinese
     * @brief 分区范围，同时保持各组内元素的相对顺序。
     *
     * @tparam Range 范围类型
     * @tparam Pred 谓词类型
     * @param range 要分区的范围
     * @param pred 要应用的谓词
     * @return 指向第二组第一个元素的迭代器
     */
    template <typename Range, typename Pred>
    constexpr auto stable_partition(Range &&range, Pred pred) {
        return core::algorithm::stable_partition(utility::begin(range), utility::end(range), utility::move(pred));
    }

    /**
     * \lang english
     * @brief Finds the partition point of a partitioned range.
     *
     * @tparam Range The range type
     * @tparam Pred The predicate type
     * @param range The range to inspect
     * @param pred The predicate to apply
     * @return Iterator to the first element not satisfying pred
     *
     * \lang simp-chinese
     * @brief 查找已分区范围的分区点。
     *
     * @tparam Range 范围类型
     * @tparam Pred 谓词类型
     * @param range 要检查的范围
     * @param pred 要应用的谓词
     * @return 指向第一个不满足pred元素的迭代器
     */
    template <typename Range, typename Pred>
    RAINY_NODISCARD constexpr auto partition_point(Range &&range, Pred pred) {
        return core::algorithm::partition_point(utility::begin(range), utility::end(range), utility::move(pred));
    }
}

namespace rainy::collections::views {
    using core::collections::views::is_partitioned;
    using core::collections::views::partition;
    using core::collections::views::partition_copy;
    using core::collections::views::partition_point;
    using core::collections::views::stable_partition;
}

#endif
