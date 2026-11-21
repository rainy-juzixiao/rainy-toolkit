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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_HEAP_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_HEAP_HPP

#include <rainy/core/algorithm/heap.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief Turns a range into a max-heap with respect to comp.
     *
     * @tparam Range The range type
     * @param range The range to heapify
     *
     * \lang simp-chinese
     * @brief 将范围转换为关于comp的最大堆。
     *
     * @tparam Range 范围类型
     * @param range 要建堆的范围
     */
    template <typename Range>
    constexpr auto make_heap(Range &&range) {
        return core::algorithm::make_heap(utility::begin(range), utility::end(range));
    }

    /**
     * \lang english
     * @brief Inserts the last element of a range into the preceding heap.
     *
     * @tparam Range The range type
     * @param range The range whose last element is inserted
     *
     * \lang simp-chinese
     * @brief 将范围的最后一个元素插入前缀堆。
     *
     * @tparam Range 范围类型
     * @param range 插入其末元素的范围
     */
    template <typename Range>
    constexpr auto push_heap(Range &&range) {
        return core::algorithm::push_heap(utility::begin(range), utility::end(range));
    }

    /**
     * \lang english
     * @brief Swaps the heap head with the last element of a range and restores the heap invariant.
     *
     * @tparam Range The range type
     * @param range The heap range
     *
     * \lang simp-chinese
     * @brief 将堆顶与范围末元素交换并恢复堆性质。
     *
     * @tparam Range 范围类型
     * @param range 堆范围
     */
    template <typename Range>
    constexpr auto pop_heap(Range &&range) {
        return core::algorithm::pop_heap(utility::begin(range), utility::end(range));
    }

    /**
     * \lang english
     * @brief Sorts a heap range into ascending order.
     *
     * @tparam Range The range type
     * @param range The heap range
     *
     * \lang simp-chinese
     * @brief 将堆范围排序为升序。
     *
     * @tparam Range 范围类型
     * @param range 堆范围
     */
    template <typename Range>
    constexpr auto sort_heap(Range &&range) {
        return core::algorithm::sort_heap(utility::begin(range), utility::end(range));
    }

    /**
     * \lang english
     * @brief Finds the first position where a range is no longer a heap.
     *
     * @tparam Range The range type
     * @param range The range to inspect
     * @return Iterator to the end of the longest heap prefix
     *
     * \lang simp-chinese
     * @brief 查找范围不再满足堆性质的第一个位置。
     *
     * @tparam Range 范围类型
     * @param range 要检查的范围
     * @return 指向最长堆前缀末尾的迭代器
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto is_heap_until(Range &&range) {
        return core::algorithm::is_heap_until(utility::begin(range), utility::end(range));
    }

    /**
     * \lang english
     * @brief Checks whether a range is a max-heap.
     *
     * @tparam Range The range type
     * @param range The range to inspect
     * @return true if the range is a max-heap
     *
     * \lang simp-chinese
     * @brief 检查范围是否为最大堆。
     *
     * @tparam Range 范围类型
     * @param range 要检查的范围
     * @return 如果范围是最大堆则为true
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto is_heap(Range &&range) -> bool {
        return core::algorithm::is_heap(utility::begin(range), utility::end(range));
    }
}

namespace rainy::collections::views {
    using core::collections::views::is_heap;
    using core::collections::views::is_heap_until;
    using core::collections::views::make_heap;
    using core::collections::views::pop_heap;
    using core::collections::views::push_heap;
    using core::collections::views::sort_heap;
}

#endif
