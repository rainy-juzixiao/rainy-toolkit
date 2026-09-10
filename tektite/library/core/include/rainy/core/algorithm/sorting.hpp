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
#ifndef RAINY_CORE_ALGORITHM_SORTING_HPP
#define RAINY_CORE_ALGORITHM_SORTING_HPP

#include <rainy/core/algorithm/heap.hpp>
#include <rainy/core/collections/vector.hpp>

namespace rainy::core::algorithm::implements {
    inline constexpr int introsort_threshold = 16;

    template <typename RandomIt, typename Comp>
    constexpr void insertion_sort(RandomIt first, RandomIt last, Comp comp) {
        for (RandomIt it = first + 1; it != last; ++it) {
            auto value = utility::move(*it);
            RandomIt hole = it;
            for (; hole != first && comp(value, *(hole - 1)); --hole) {
                *hole = utility::move(*(hole - 1));
            }
            *hole = utility::move(value);
        }
    }

    template <typename RandomIt, typename Comp>
    constexpr RandomIt partition_at_pivot(RandomIt first, RandomIt last, Comp comp) {
        RandomIt mid = first + (last - first) / 2;
        if (comp(*mid, *first)) {
            core::algorithm::iter_swap(mid, first);
        }
        if (comp(*(last - 1), *first)) {
            core::algorithm::iter_swap(last - 1, first);
        }
        if (comp(*(last - 1), *mid)) {
            core::algorithm::iter_swap(last - 1, mid);
        }
        auto pivot = utility::move(*(last - 1));
        RandomIt cut = first;
        for (RandomIt it = first; it != last - 1; ++it) {
            if (comp(*it, pivot)) {
                core::algorithm::iter_swap(cut, it);
                ++cut;
            }
        }
        *(last - 1) = utility::move(*cut);
        *cut = utility::move(pivot);
        return cut;
    }

    template <typename RandomIt, typename Comp>
    constexpr void introsort(RandomIt first, RandomIt last, int depth, Comp comp) {
        auto size = last - first;
        while (size > introsort_threshold) {
            if (depth <= 0) {
                core::algorithm::make_heap(first, last, comp);
                core::algorithm::sort_heap(first, last, comp);
                return;
            }
            --depth;
            RandomIt cut = partition_at_pivot(first, last, comp);
            if (cut - first < last - (cut + 1)) {
                introsort(first, cut, depth, comp);
                first = cut + 1;
            } else {
                introsort(cut + 1, last, depth, comp);
                last = cut;
            }
            size = last - first;
        }
        if (size > 1) {
            insertion_sort(first, last, comp);
        }
    }

    template <typename RandomIt, typename Comp>
    constexpr void introselect(RandomIt first, RandomIt nth, RandomIt last, int depth, Comp comp) {
        auto size = last - first;
        while (size > introsort_threshold) {
            if (depth <= 0) {
                core::algorithm::make_heap(first, last, comp);
                core::algorithm::sort_heap(first, last, comp);
                return;
            }
            --depth;
            RandomIt cut = partition_at_pivot(first, last, comp);
            if (nth == cut) {
                return;
            }
            if (nth < cut) {
                last = cut;
            } else {
                first = cut + 1;
            }
            size = last - first;
        }
        insertion_sort(first, last, comp);
    }

    template <typename RandomIt, typename Comp>
    constexpr void merge_sort(RandomIt first, RandomIt last, Comp comp,
                              collections::vector<typename utility::iterator_traits<RandomIt>::value_type> &buffer) {
        auto size = last - first;
        if (size <= introsort_threshold) {
            insertion_sort(first, last, comp);
            return;
        }
        RandomIt mid = first + size / 2;
        merge_sort(first, mid, comp, buffer);
        merge_sort(mid, last, comp, buffer);
        buffer.clear();
        RandomIt left = first;
        RandomIt right = mid;
        while (left != mid && right != last) {
            if (comp(*right, *left)) {
                buffer.push_back(utility::move(*right));
                ++right;
            } else {
                buffer.push_back(utility::move(*left));
                ++left;
            }
        }
        while (left != mid) {
            buffer.push_back(utility::move(*left));
            ++left;
        }
        while (right != last) {
            buffer.push_back(utility::move(*right));
            ++right;
        }
        RandomIt out = first;
        for (auto &value : buffer) {
            *out = utility::move(value);
            ++out;
        }
    }
}

namespace rainy::core::algorithm {
    /**
     * @brief Finds the first position where the range is no longer sorted.
     *        查找范围不再有序的第一个位置。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return Iterator to the end of the longest sorted prefix
     *         指向最长有序前缀末尾的迭代器
     */
    template <typename ForwardIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn is_sorted_until(ForwardIt first, ForwardIt last, Comp comp) -> ForwardIt {
        if (first == last) {
            return last;
        }
        ForwardIt previous = first;
        for (++first; first != last; ++first, ++previous) {
            if (comp(*first, *previous)) {
                return first;
            }
        }
        return last;
    }

    /**
     * @brief Finds the first position where the range is no longer sorted (operator<).
     *        查找范围不再有序的第一个位置（operator<）。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn is_sorted_until(ForwardIt first, ForwardIt last) -> ForwardIt {
        return core::algorithm::is_sorted_until(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Checks whether the range is sorted with respect to comp.
     *        检查范围是否按comp有序。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return true if the range is sorted
     *         如果范围有序则为true
     */
    template <typename ForwardIt, typename Comp>
    RAINY_NODISCARD constexpr rain_fn is_sorted(ForwardIt first, ForwardIt last, Comp comp) -> bool {
        return core::algorithm::is_sorted_until(first, last, comp) == last;
    }

    /**
     * @brief Checks whether the range is sorted (operator<).
     *        检查范围是否有序（operator<）。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn is_sorted(ForwardIt first, ForwardIt last) -> bool {
        return core::algorithm::is_sorted(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Sorts the range in ascending order with respect to comp (introsort).
     *        按comp将范围升序排序（introsort，非稳定）。
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
    constexpr rain_fn sort(RandomIt first, RandomIt last, Comp comp) -> void {
        implements::introsort(first, last, utility::distance(first, last) * 2, comp);
    }

    /**
     * @brief Sorts the range in ascending order (operator<, introsort).
     *        将范围升序排序（operator<，introsort，非稳定）。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn sort(RandomIt first, RandomIt last) -> void {
        core::algorithm::sort(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Sorts the range stably in ascending order with respect to comp (merge sort).
     *        按comp稳定升序排序（归并排序）。
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
    constexpr rain_fn stable_sort(RandomIt first, RandomIt last, Comp comp) -> void {
        auto size = utility::distance(first, last);
        if (size <= implements::introsort_threshold) {
            implements::insertion_sort(first, last, comp);
            return;
        }
        collections::vector<typename utility::iterator_traits<RandomIt>::value_type> buffer;
        buffer.reserve(static_cast<std::size_t>(size));
        implements::merge_sort(first, last, comp, buffer);
    }

    /**
     * @brief Sorts the range stably in ascending order (operator<, merge sort).
     *        稳定升序排序（operator<，归并排序）。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn stable_sort(RandomIt first, RandomIt last) -> void {
        core::algorithm::stable_sort(first, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Partially sorts the range: the smallest [first, middle) elements are sorted at the front.
     *        部分排序：最小的[first, middle)个元素有序地放在前面。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param middle Iterator delimiting the sorted prefix
     *               界定有序前缀的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename RandomIt, typename Comp>
    constexpr rain_fn partial_sort(RandomIt first, RandomIt middle, RandomIt last, Comp comp) -> void {
        if (first == middle) {
            return;
        }
        core::algorithm::make_heap(first, middle, comp);
        for (RandomIt it = middle; it != last; ++it) {
            if (comp(*it, *first)) {
                core::algorithm::iter_swap(it, first);
                implements::sift_down(first, first, middle, comp);
            }
        }
        core::algorithm::sort_heap(first, middle, comp);
    }

    /**
     * @brief Partially sorts the range with the default comparison.
     *        使用默认比较的部分排序。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param middle Iterator delimiting the sorted prefix
     *               界定有序前缀的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn partial_sort(RandomIt first, RandomIt middle, RandomIt last) -> void {
        core::algorithm::partial_sort(first, middle, last, [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Copies the smallest elements of the source range, sorted, into the destination.
     *        将源范围中最小的若干元素排序后拷贝到目标范围。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam RandomIt Random access iterator type of the destination
     *                  目标的随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *             指向源范围末尾的迭代器
     * @param dest_first Iterator to the beginning of the destination range
     *                   指向目标范围起始的迭代器
     * @param dest_last Iterator to the end of the destination range
     *                  指向目标范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     * @return Iterator to the end of the written destination range
     *         指向已写入目标范围末尾的迭代器
     */
    template <typename InputIt, typename RandomIt, typename Comp>
    constexpr rain_fn partial_sort_copy(InputIt first, InputIt last, RandomIt dest_first, RandomIt dest_last, Comp comp)
        -> RandomIt {
        const auto dest_size = dest_last - dest_first;
        if (dest_size <= 0) {
            return dest_first;
        }
        RandomIt result = dest_first;
        for (; first != last && result != dest_last; ++first, ++result) {
            *result = *first;
        }
        core::algorithm::make_heap(dest_first, result, comp);
        for (; first != last; ++first) {
            if (comp(*first, *dest_first)) {
                *dest_first = *first;
                implements::sift_down(dest_first, dest_first, result, comp);
            }
        }
        core::algorithm::sort_heap(dest_first, result, comp);
        return result;
    }

    /**
     * @brief Copies the smallest elements of the source range, sorted, into the destination (operator<).
     *        将源范围中最小的若干元素排序后拷贝到目标范围（operator<）。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam RandomIt Random access iterator type of the destination
     *                  目标的随机访问迭代器类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *             指向源范围末尾的迭代器
     * @param dest_first Iterator to the beginning of the destination range
     *                   指向目标范围起始的迭代器
     * @param dest_last Iterator to the end of the destination range
     *                  指向目标范围末尾的迭代器
     * @return Iterator to the end of the written destination range
     *         指向已写入目标范围末尾的迭代器
     */
    template <typename InputIt, typename RandomIt>
    constexpr rain_fn partial_sort_copy(InputIt first, InputIt last, RandomIt dest_first, RandomIt dest_last) -> RandomIt {
        return core::algorithm::partial_sort_copy(first, last, dest_first, dest_last,
                                                  [](const auto &left, const auto &right) { return left < right; });
    }

    /**
     * @brief Places the element that would be at position nth after sorting there, partitioning around it.
     *        将排序后应位于nth位置的元素放到该处，并以其为界分区。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @tparam Comp The comparison type
     *              比较器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param nth Iterator delimiting the position to settle
     *            界定待确定位置的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param comp The comparison to apply
     *             要应用的比较器
     */
    template <typename RandomIt, typename Comp>
    constexpr rain_fn nth_element(RandomIt first, RandomIt nth, RandomIt last, Comp comp) -> void {
        if (nth == last) {
            return;
        }
        implements::introselect(first, nth, last, utility::distance(first, last) * 2, comp);
    }

    /**
     * @brief Places the element that would be at position nth after sorting there (operator<).
     *        将排序后应位于nth位置的元素放到该处（operator<）。
     *
     * @tparam RandomIt Random access iterator type
     *                  随机访问迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param nth Iterator delimiting the position to settle
     *            界定待确定位置的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     */
    template <typename RandomIt>
    constexpr rain_fn nth_element(RandomIt first, RandomIt nth, RandomIt last) -> void {
        core::algorithm::nth_element(first, nth, last, [](const auto &left, const auto &right) { return left < right; });
    }
}

namespace rainy::algorithm {
    using core::algorithm::is_sorted;
    using core::algorithm::is_sorted_until;
    using core::algorithm::nth_element;
    using core::algorithm::partial_sort;
    using core::algorithm::partial_sort_copy;
    using core::algorithm::sort;
    using core::algorithm::stable_sort;
}

#endif
