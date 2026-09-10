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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_PERMUTATION_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_PERMUTATION_HPP

#include <rainy/core/algorithm/permutation.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * @brief Checks whether a range is a permutation of a second sequence.
     *        检查范围是否为第二个序列的排列。
     *
     * @tparam Range The first range type
     *               第一个范围类型
     * @tparam InputIt The second sequence iterator type
     *                 第二个序列迭代器类型
     * @param range The first range
     *              第一个范围
     * @param first2 Iterator to the beginning of the second sequence
     *               指向第二个序列起始的迭代器
     * @return true if the sequences are permutations of each other
     *         如果两个序列互为排列则为true
     */
    template <typename Range, typename InputIt>
    RAINY_NODISCARD constexpr auto is_permutation(Range &&range, InputIt first2) -> bool {
        return core::algorithm::is_permutation(utility::begin(range), utility::end(range), first2);
    }

    /**
     * @brief Transforms a range into the next lexicographic permutation.
     *        将范围变换为下一个字典序排列。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to transform
     *              要变换的范围
     * @return true if the next permutation exists
     *         如果存在下一个排列则为true
     */
    template <typename Range>
    constexpr auto next_permutation(Range &&range) -> bool {
        return core::algorithm::next_permutation(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Transforms a range into the previous lexicographic permutation.
     *        将范围变换为上一个字典序排列。
     *
     * @tparam Range The range type
     *               范围类型
     * @param range The range to transform
     *              要变换的范围
     * @return true if the previous permutation exists
     *         如果存在上一个排列则为true
     */
    template <typename Range>
    constexpr auto prev_permutation(Range &&range) -> bool {
        return core::algorithm::prev_permutation(utility::begin(range), utility::end(range));
    }
}

namespace rainy::collections::views {
    using core::collections::views::is_permutation;
    using core::collections::views::next_permutation;
    using core::collections::views::prev_permutation;
}

#endif
