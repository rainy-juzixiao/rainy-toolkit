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
#ifndef RAINY_CORE_ALGORITHM_HEAP_HPP
#define RAINY_CORE_ALGORITHM_HEAP_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>

namespace rainy::core::algorithm::implements {
    template <typename RandomIt, typename Comp>
    constexpr void sift_up(RandomIt first, RandomIt last, Comp comp) {
        if (first == last) {
            return;
        }
        RandomIt target = last;
        RandomIt parent = first + (target - first - 1) / 2;
        while (target > first && comp(*parent, *target)) {
            core::algorithm::iter_swap(parent, target);
            target = parent;
            if (target == first) {
                break;
            }
            parent = first + (target - first - 1) / 2;
        }
    }

    template <typename RandomIt, typename Comp>
    constexpr void sift_down(RandomIt first, RandomIt node, RandomIt last, Comp comp) {
        auto size = last - first;
        while (true) {
            auto node_index = node - first;
            auto child_index = node_index * 2 + 1;
            if (child_index >= size) {
                break;
            }
            RandomIt child = first + child_index;
            if (child_index + 1 < size && comp(*child, *(child + 1))) {
                ++child;
            }
            if (!comp(*node, *child)) {
                break;
            }
            core::algorithm::iter_swap(node, child);
            node = child;
        }
    }
}

namespace rainy::core::algorithm {
    /**
     * @brief Turns a range into a max-heap with respect to comp.
     *        将范围转换为关于comp的最大堆。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename RandomIt, typename Comp>
    constexpr rain_fn make_heap(RandomIt first, RandomIt last, Comp comp) -> void {
        auto size = last - first;
        if (size < 2) {
            return;
        }
        for (auto offset = size / 2 - 1; offset >= 0; --offset) {
            implements::sift_down(first, first + offset, last, comp);
        }
    }

    /**
     * @brief Turns a range into a max-heap ordered by operator<.
     *        将范围转换为按operator<排序的最大堆。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn make_heap(RandomIt first, RandomIt last) -> void {
        core::algorithm::make_heap(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Inserts the element before last into the heap [first, last - 1).
     *        将last之前的元素插入堆[first, last - 1)。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the heap
     *              指向堆起始的迭代器
     * @param last Iterator one past the element to insert
     *             指向要插入元素之后的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename RandomIt, typename Comp>
    constexpr rain_fn push_heap(RandomIt first, RandomIt last, Comp comp) -> void {
        implements::sift_up(first, last - 1, comp);
    }

    /**
     * @brief Inserts the element before last into the heap ordered by operator<.
     *        将last之前的元素插入按operator<排序的堆。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the heap
     *              指向堆起始的迭代器
     * @param last Iterator one past the element to insert
     *             指向要插入元素之后的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn push_heap(RandomIt first, RandomIt last) -> void {
        core::algorithm::push_heap(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Swaps the heap head with the last element and restores the heap invariant.
     *        将堆顶与末尾元素交换，并恢复堆性质。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the heap
     *              指向堆起始的迭代器
     * @param last Iterator to the end of the heap
     *             指向堆末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename RandomIt, typename Comp>
    constexpr rain_fn pop_heap(RandomIt first, RandomIt last, Comp comp) -> void {
        if (last - first < 2) {
            return;
        }
        --last;
        core::algorithm::iter_swap(first, last);
        implements::sift_down(first, first, last, comp);
    }

    /**
     * @brief Swaps the heap head with the last element and restores the heap invariant (operator<).
     *        将堆顶与末尾元素交换并恢复堆性质（operator<）。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the heap
     *              指向堆起始的迭代器
     * @param last Iterator to the end of the heap
     *             指向堆末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn pop_heap(RandomIt first, RandomIt last) -> void {
        core::algorithm::pop_heap(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Sorts a heap into ascending order with respect to comp.
     *        将堆按comp排序为升序。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the heap
     *              指向堆起始的迭代器
     * @param last Iterator to the end of the heap
     *             指向堆末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename RandomIt, typename Comp>
    constexpr rain_fn sort_heap(RandomIt first, RandomIt last, Comp comp) -> void {
        while (last - first > 1) {
            core::algorithm::pop_heap(first, last, comp);
            --last;
        }
    }

    /**
     * @brief Sorts a heap into ascending order (operator<).
     *        将堆排序为升序（operator<）。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the heap
     *              指向堆起始的迭代器
     * @param last Iterator to the end of the heap
     *             指向堆末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn sort_heap(RandomIt first, RandomIt last) -> void {
        core::algorithm::sort_heap(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Finds the first position where the range is no longer a heap.
     *        查找范围不再满足堆性质的第一个位置。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return Iterator to the end of the longest heap prefix
     *         指向最长堆前缀末尾的迭代器
     */
    template <typename RandomIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn is_heap_until(RandomIt first, RandomIt last, Comp comp) -> RandomIt {
        auto size = last - first;
        for (decltype(size) offset = 1; offset < size; ++offset) {
            if (comp(*(first + (offset - 1) / 2), *(first + offset))) {
                return first + offset;
            }
        }
        return last;
    }

    /**
     * @brief Finds the first position where the range is no longer a heap (operator<).
     *        查找范围不再满足堆性质的第一个位置（operator<）。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    RAINY_NODISCARD constexpr rain_fn is_heap_until(RandomIt first, RandomIt last) -> RandomIt {
        return core::algorithm::is_heap_until(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Checks whether the range is a max-heap with respect to comp.
     *        检查范围是否为关于comp的最大堆。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return true if the range is a max-heap
     *         如果范围是最大堆则为true
     */
    template <typename RandomIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn is_heap(RandomIt first, RandomIt last, Comp comp) -> bool {
        return core::algorithm::is_heap_until(first, last, comp) == last;
    }

    /**
     * @brief Checks whether the range is a max-heap ordered by operator<.
     *        检查范围是否为按operator<排序的最大堆。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    RAINY_NODISCARD constexpr rain_fn is_heap(RandomIt first, RandomIt last) -> bool {
        return core::algorithm::is_heap(first, last, [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::algorithm {
    using core::algorithm::is_heap;
    using core::algorithm::is_heap_until;
    using core::algorithm::make_heap;
    using core::algorithm::pop_heap;
    using core::algorithm::push_heap;
    using core::algorithm::sort_heap;
}

#endif
