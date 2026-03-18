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
/**
 * @mergeto rainy/core/core.hpp
 */
#ifndef RAINY_CORE_IMPLEMENTS_BASIC_ALGORITHM_HPP
#define RAINY_CORE_IMPLEMENTS_BASIC_ALGORITHM_HPP
#include <algorithm>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/properties.hpp>

namespace rainy::core::algorithm {
    /**
     * @brief Swaps elements between two ranges.
     *        交换两个范围之间的元素。
     *
     * @tparam ForwardIt1 Forward iterator type for the first range
     *                    第一个范围的前向迭代器类型
     * @tparam ForwardIt2 Forward iterator type for the second range
     *                    第二个范围的前向迭代器类型
     * @param first1 Iterator to the beginning of the first range
     *               指向第一个范围起始的迭代器
     * @param last1 Iterator to the end of the first range
     *              指向第一个范围末尾的迭代器
     * @param first2 Iterator to the beginning of the second range
     *               指向第二个范围起始的迭代器
     * @return Iterator to the element past the last swapped element in the second range
     *         指向第二个范围中最后一个被交换元素之后位置的迭代器
     */
    template <class ForwardIt1, class ForwardIt2>
    RAINY_CONSTEXPR20 rain_fn swap_ranges(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2) -> ForwardIt2 {
        for (; first1 != last1; ++first1, ++first2) {
            std::iter_swap(first1, first2);
        }
        return first2;
    }

    /**
     * @brief Assigns the given value to all elements in a range.
     *        将给定值赋值给范围内的所有元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Ty Value type
     *            值类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param end Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param value The value to assign
     *              要赋值的值
     */
    template <typename Iter, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn fill(Iter first, Iter end, const Ty &value) -> void {
        for (; first != end; ++first) {
            *first = value;
        }
    }

    /**
     * @brief Assigns the given value to the first count elements in a range.
     *        将给定值赋值给范围内的前count个元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Size Size type (integral)
     *              大小类型（整型）
     * @tparam Ty Value type
     *            值类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param count Number of elements to fill
     *              要填充的元素数量
     * @param value The value to assign
     *              要赋值的值
     * @return Iterator one past the last element filled
     *         指向最后一个被填充元素之后位置的迭代器
     */
    template <typename Iter, typename Size, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn fill_n(Iter first, Size count, const Ty &value) -> Iter {
        for (Size i = 0; i < count; ++i) {
            *first++ = value;
        }
        return first;
    }

    /**
     * @brief Checks if a predicate is true for all elements in a range.
     *        检查谓词是否对范围内的所有元素都为真。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Predicate type
     *              谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Predicate to apply
     *             要应用的谓词
     * @return true if pred is true for all elements, false otherwise
     *         如果所有元素都满足谓词则为true，否则为false
     */
    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn all_of(Iter first, Iter last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Checks if a predicate is true for any element in a range.
     *        检查谓词是否对范围内的任意元素为真。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Predicate type
     *              谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Predicate to apply
     *             要应用的谓词
     * @return true if pred is true for any element, false otherwise
     *         如果任意元素满足谓词则为true，否则为false
     */
    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn any_of(Iter first, Iter last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Checks if a predicate is true for no elements in a range.
     *        检查谓词是否对范围内没有元素为真。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Predicate type
     *              谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Predicate to apply
     *             要应用的谓词
     * @return true if pred is false for all elements, false otherwise
     *         如果所有元素都不满足谓词则为true，否则为false
     */
    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn none_of(Iter first, Iter last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Finds the first occurrence of a value in a range.
     *        查找范围内第一次出现的值。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Ty Value type
     *            值类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param value The value to find
     *              要查找的值
     * @return Iterator to the first element equal to value, or last if not found
     *         指向第一个等于value的元素的迭代器，如果未找到则返回last
     */
    template <typename Iter, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn find(Iter first, Iter last, const Ty &value) -> Iter {
        for (; first != last; ++first) {
            if (*first == value) {
                return first;
            }
        }
        return last;
    }

    /**
     * @brief Finds the first element satisfying a predicate.
     *        查找第一个满足谓词的元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Predicate type
     *              谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Predicate to apply
     *             要应用的谓词
     * @return Iterator to the first element satisfying pred, or last if not found
     *         指向第一个满足pred的元素的迭代器，如果未找到则返回last
     */
    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn find_if(Iter first, Iter last, Pred pred) -> Iter {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return first;
            }
        }
        return last;
    }

    /**
     * @brief Finds the first element not satisfying a predicate.
     *        查找第一个不满足谓词的元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Predicate type
     *              谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Predicate to apply
     *             要应用的谓词
     * @return Iterator to the first element not satisfying pred, or last if not found
     *         指向第一个不满足pred的元素的迭代器，如果未找到则返回last
     */
    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn find_if_not(Iter first, Iter last, Pred pred) -> Iter {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return first;
            }
        }
        return last;
    }

    /**
     * @brief Checks if two ranges are equal.
     *        检查两个范围是否相等。
     *
     * @tparam Iter1 First range iterator type
     *               第一个范围的迭代器类型
     * @tparam Iter2 Second range iterator type
     *               第二个范围的迭代器类型
     * @param first1 Iterator to the beginning of the first range
     *               指向第一个范围起始的迭代器
     * @param last1 Iterator to the end of the first range
     *              指向第一个范围末尾的迭代器
     * @param first2 Iterator to the beginning of the second range
     *               指向第二个范围起始的迭代器
     * @param last2 Iterator to the end of the second range
     *              指向第二个范围末尾的迭代器
     * @return true if the ranges are equal, false otherwise
     *         如果范围相等则为true，否则为false
     */
    template <typename Iter1, typename Iter2>
    RAINY_NODISCARD inline constexpr rain_fn equal(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) -> bool {
        for (; first1 != last1 && first2 != last2 && *first1 == *first2; ++first2) {
            ++first1;
        }
        return first1 == last1 && first2 == last2;
    }

    /**
     * @brief Lexicographically compares two ranges.
     *        字典序比较两个范围。
     *
     * @tparam Iter1 First range iterator type
     *               第一个范围的迭代器类型
     * @tparam Iter2 Second range iterator type
     *               第二个范围的迭代器类型
     * @param first1 Iterator to the beginning of the first range
     *               指向第一个范围起始的迭代器
     * @param last1 Iterator to the end of the first range
     *              指向第一个范围末尾的迭代器
     * @param first2 Iterator to the beginning of the second range
     *               指向第二个范围起始的迭代器
     * @param last2 Iterator to the end of the second range
     *              指向第二个范围末尾的迭代器
     * @return true if the first range is lexicographically less than the second, false otherwise
     *         如果第一个范围字典序小于第二个范围则为true，否则为false
     */
    template <typename Iter1, typename Iter2>
    RAINY_NODISCARD inline constexpr rain_fn lexicographical_compare(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) -> bool {
        for (; (first1 != last1) && (first2 != last2); ++first2) {
            if (*first1 < *first2) {
                return true;
            }
            if (*first2 < *first1) {
                return false;
            }
            ++first1;
        }
        return (first1 == last1) && (first2 != last2);
    }

    /**
     * @brief Returns an iterator to the first element not less than the given value.
     *        返回指向第一个不小于给定值的元素的迭代器。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Ty Value type
     *            值类型
     * @tparam Pred Comparison predicate type
     *              比较谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param value The value to compare against
     *              要比较的值
     * @param pred Comparison predicate (returns true if first argument is less than second)
     *             比较谓词（如果第一个参数小于第二个参数则返回true）
     * @return Iterator to the first element not less than value, or last if not found
     *         指向第一个不小于value的元素的迭代器，如果未找到则返回last
     */
    template <typename ForwardIt, typename Ty, typename Pred>
    RAINY_NODISCARD inline constexpr rain_fn lower_bound(ForwardIt first, ForwardIt last, Ty const &value, Pred pred) -> ForwardIt {
        ForwardIt it;
        using diff_t = typename utility::iterator_traits<ForwardIt>::difference_type;
        diff_t count, step;
        count = last - first;
        while (count > 0) {
            it = first;
            step = count / 2;
            it += step;
            if (pred(*it, value)) {
                first = ++it;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    /**
     * @brief Returns an iterator to the first element greater than the given value.
     *        返回指向第一个大于给定值的元素的迭代器。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Ty Value type
     *            值类型
     * @tparam Pred Comparison predicate type
     *              比较谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param value The value to compare against
     *              要比较的值
     * @param pred Comparison predicate (returns true if first argument is less than second)
     *             比较谓词（如果第一个参数小于第二个参数则返回true）
     * @return Iterator to the first element greater than value, or last if not found
     *         指向第一个大于value的元素的迭代器，如果未找到则返回last
     */
    template <typename ForwardIt, typename Ty, typename Pred>
    RAINY_NODISCARD inline constexpr auto upper_bound(ForwardIt first, ForwardIt last, Ty const &value, Pred pred) -> ForwardIt {
        ForwardIt it;
        using diff_t = typename utility::iterator_traits<ForwardIt>::difference_type;
        diff_t count, step;
        count = last - first;
        while (count > 0) {
            it = first;
            step = count / 2;
            it += step;
            if (!pred(value, *it)) {
                first = ++it;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    /**
     * @brief Moves elements from one range to another.
     *        将元素从一个范围移动到另一个范围。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam OutputIt Output iterator type
     *                  输出迭代器类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param d_first Iterator to the beginning of the destination range
     *                指向目标范围起始的迭代器
     * @return Iterator to the element past the last moved element in the destination range
     *         指向目标范围中最后一个被移动元素之后位置的迭代器
     */
    template <typename InputIt, typename OutputIt>
    RAINY_CONSTEXPR20 rain_fn move(InputIt first, InputIt last, OutputIt d_first) -> OutputIt {
        for (; first != last; ++d_first, ++first) {
            *d_first = utility::move(*first);
        }
        return d_first;
    }

    /**
     * @brief Moves elements from one range to another, starting from the end.
     *        从末尾开始将元素从一个范围移动到另一个范围。
     *
     * @tparam BidirIt1 Bidirectional iterator type for source
     *                  源范围的双向迭代器类型
     * @tparam BidirIt2 Bidirectional iterator type for destination
     *                  目标范围的双向迭代器类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param d_last Iterator to the end of the destination range
     *               指向目标范围末尾的迭代器
     * @return Iterator to the first moved element in the destination range
     *         指向目标范围中第一个被移动元素的迭代器
     */
    template <typename BidirIt1, typename BidirIt2>
    RAINY_CONSTEXPR20 inline rain_fn move_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) -> BidirIt2 {
        while (first != last) {
            *(--d_last) = utility::move(*(--last));
        }
        return d_last;
    }

    /**
     * @brief Copies elements from one range to another, starting from the end.
     *        从末尾开始将元素从一个范围复制到另一个范围。
     *
     * @tparam BidirIt1 Bidirectional iterator type for source
     *                  源范围的双向迭代器类型
     * @tparam BidirIt2 Bidirectional iterator type for destination
     *                  目标范围的双向迭代器类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param d_last Iterator to the end of the destination range
     *               指向目标范围末尾的迭代器
     * @return Iterator to the first copied element in the destination range
     *         指向目标范围中第一个被复制元素的迭代器
     */
    template <typename BidirIt1, typename BidirIt2>
    RAINY_CONSTEXPR20 rain_fn copy_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) -> BidirIt2 {
        while (first != last) {
            *(--d_last) = *(--last);
        }
        return d_last;
    }

    /**
     * @brief Moves elements from one range to an uninitialized range, starting from the end.
     *        从末尾开始将元素从一个范围移动到未初始化的范围。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam OutputIt Output iterator type for uninitialized memory
     *                  未初始化内存的输出迭代器类型
     * @param first Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param last Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param d_last Iterator to the end of the destination range
     *               指向目标范围末尾的迭代器
     * @return Iterator to the first moved element in the destination range
     *         指向目标范围中第一个被移动元素的迭代器
     */
    template <typename InputIt, typename OutputIt>
    constexpr rain_fn uninitialized_move_backward(InputIt first, InputIt last, OutputIt d_last) -> OutputIt {
        while (first != last) {
            --d_last;
            --last;
            builtin::construct_at(std::addressof(*d_last), utility::move(*last));
        }
        return d_last;
    }

    /**
     * @brief Checks if a value exists in a sorted range using binary search.
     *        使用二分查找检查值是否存在于已排序的范围中。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Ty Value type
     *            值类型
     * @tparam Compare Comparison function type
     *                 比较函数类型
     * @param first Iterator to the beginning of the sorted range
     *              指向已排序范围起始的迭代器
     * @param last Iterator to the end of the sorted range
     *            指向已排序范围末尾的迭代器
     * @param value The value to search for
     *              要搜索的值
     * @param comp Comparison function (returns true if first argument is less than second)
     *             比较函数（如果第一个参数小于第二个参数则返回true）
     * @return true if the value is found, false otherwise
     *         如果找到该值则为true，否则为false
     */
    template <typename ForwardIt, typename Ty = typename utility::iterator_traits<ForwardIt>::value_type, typename Compare>
    constexpr rain_fn binary_search(ForwardIt first, ForwardIt last, const Ty &value, Compare comp) -> bool {
        first = algorithm::lower_bound(first, last, value, comp);
        return (!(first == last) and !(comp(value, *first)));
    }

    /**
     * @brief Checks if a value exists in a sorted range using binary search (default comparison).
     *        使用二分查找检查值是否存在于已排序的范围中（默认比较）。
     *
     * @tparam ForwardIt Forward iterator type
     *                   前向迭代器类型
     * @tparam Ty Value type
     *            值类型
     * @param first Iterator to the beginning of the sorted range
     *              指向已排序范围起始的迭代器
     * @param last Iterator to the end of the sorted range
     *            指向已排序范围末尾的迭代器
     * @param value The value to search for
     *              要搜索的值
     * @return true if the value is found, false otherwise
     *         如果找到该值则为true，否则为false
     */
    template <typename ForwardIt, typename Ty = typename utility::iterator_traits<ForwardIt>::value_type>
    constexpr rain_fn binary_search(ForwardIt first, ForwardIt last, const Ty &value) -> bool {
        return algorithm::binary_search(first, last, value, [](auto &&left, auto &&right) { return left < right; });
    }
}

namespace rainy::core::algorithm {
    /**
     * @brief Copies elements from a range to another range.
     *        将元素从一个范围复制到另一个范围。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @param begin Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param end Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 rain_fn copy(InputIter begin, InputIter end, OutIter dest) noexcept(
        type_traits::type_properties::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;

        if (begin == end) {
            return dest;
        }

#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            auto input_end = utility::addressof(*(end - 1)) + 1;
            auto out_dest = utility::addressof(*dest);
            for (auto i = input_begin; i != input_end; ++i, ++out_dest) {
                *out_dest = *i;
            }
            return dest;
        }
#endif
        if constexpr (type_traits::type_properties::is_trivially_copyable_v<value_type> &&
                      type_traits::primary_types::is_pointer_v<InputIter> && type_traits::primary_types::is_pointer_v<OutIter>) {
            const auto input_begin = utility::addressof(*begin);
            const auto input_end = utility::addressof(*(end - 1)) + 1;
            auto out_dest = utility::addressof(*dest);
            const auto count = utility::distance(input_begin, input_end);
            std::memcpy(out_dest, input_begin, sizeof(value_type) * count);
            for (auto i = 0; i < count; ++i, ++dest) {
            }
            return dest;
        } else {
            for (InputIter i = begin; i != end; ++i, ++dest) {
                *dest = *i;
            }
        }
        return dest;
    }

    /**
     * @brief Copies exactly n elements from a range to another range.
     *        从一个范围精确复制n个元素到另一个范围。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @param begin Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param count Number of elements to copy
     *              要复制的元素数量
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter>
    constexpr rain_fn copy_n(InputIter begin, const std::size_t count, OutIter dest) noexcept(
        type_traits::type_properties::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;
        if (count == 0) {
            return dest;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            for (std::size_t i = 0; i < count; ++i, ++input_begin, ++dest) {
                *dest = *input_begin;
            }
        } else
#endif
        {
            if constexpr (type_traits::type_properties::is_trivially_copyable_v<value_type> &&
                          type_traits::primary_types::is_pointer_v<InputIter> && type_traits::primary_types::is_pointer_v<OutIter>) {
                const auto input_begin = utility::addressof(*begin);
                auto out_dest = utility::addressof(*dest);
                std::memcpy(out_dest, input_begin, sizeof(value_type) * count);
            } else {
                for (std::size_t i = 0; i < count; ++i, ++begin, ++dest) {
                    *dest = *begin;
                }
            }
        }
        return dest;
    }

    /**
     * @brief Applies a function to each element in a range and stores the results.
     *        对范围内的每个元素应用函数并存储结果。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @tparam Fx Unary function type
     *            一元函数类型
     * @param begin Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param end Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @param func Function to apply to each element
     *             应用于每个元素的函数
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter, typename Fx>
    constexpr rain_fn transform(InputIter begin, InputIter end, OutIter dest, Fx func) noexcept(
        type_traits::type_properties::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        for (InputIter iter = begin; iter != end; ++iter, ++dest) {
            *dest = func(*iter);
        }
        return dest;
    }

    /**
     * @brief Applies a binary function to elements from two ranges and stores the results.
     *        对两个范围的元素应用二元函数并存储结果。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @tparam Fx Binary function type
     *            二元函数类型
     * @param begin1 Iterator to the beginning of the first source range
     *               指向第一个源范围起始的迭代器
     * @param end1 Iterator to the end of the first source range
     *             指向第一个源范围末尾的迭代器
     * @param begin2 Iterator to the beginning of the second source range
     *               指向第二个源范围起始的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @param func Binary function to apply to each pair of elements
     *             应用于每对元素的二元函数
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter, typename Fx>
    constexpr rain_fn transform(InputIter begin1, InputIter end1, InputIter begin2, OutIter dest, Fx func) noexcept(
        type_traits::type_properties::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        if (begin1 == end1 || (end1 - 1) == begin1) {
            return dest;
        }
        for (InputIter iter = begin1; iter != end1; ++iter, ++dest, ++begin2) {
            *dest = func(*iter, *begin2);
        }
        return dest;
    }
}

#endif
