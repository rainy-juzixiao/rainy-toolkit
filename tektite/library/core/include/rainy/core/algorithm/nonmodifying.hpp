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
#ifndef RAINY_CORE_ALGORITHM_NONMODIFYING_HPP
#define RAINY_CORE_ALGORITHM_NONMODIFYING_HPP

#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <rainy/core/container/pair.hpp>

namespace rainy::core::algorithm {
    /**
     * \lang english
     * @brief Applies a function to every element of the range.
     *
     * @tparam InputIt Input iterator type
     * @tparam Fx The function type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param fx The function to apply
     * @return The moved function object
     *
     * \lang simp-chinese
     * @brief 对范围内的每个元素应用函数。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Fx 函数类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param fx 要应用的函数
     * @return 移动后的函数对象
     */
    template <typename InputIt, typename Fx>
    constexpr rain_fn for_each(InputIt first, InputIt last, Fx fx) -> Fx {
        for (; first != last; ++first) {
            fx(*first);
        }
        return fx;
    }

    /**
     * \lang english
     * @brief Applies a function to the first n elements of the range.
     *
     * @tparam InputIt Input iterator type
     * @tparam Size The count type
     * @tparam Fx The function type
     * @param first Iterator to the beginning of the range
     * @param n The number of elements to apply the function to
     * @param fx The function to apply
     * @return Iterator one past the last element visited
     *
     * \lang simp-chinese
     * @brief 对范围的前n个元素应用函数。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Size 数量类型
     * @tparam Fx 函数类型
     * @param first 指向范围起始的迭代器
     * @param n 要应用函数的元素数量
     * @param fx 要应用的函数
     * @return 指向最后一个被访问元素之后的迭代器
     */
    template <typename InputIt, typename Size, typename Fx>
    constexpr rain_fn for_each_n(InputIt first, Size n, Fx fx) -> InputIt {
        for (Size i = 0; i < n; ++i, ++first) {
            fx(*first);
        }
        return first;
    }

    /**
     * \lang english
     * @brief Counts the elements equal to a value.
     *
     * @tparam InputIt Input iterator type
     * @tparam Ty The value type to count
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param value The value to count
     * @return The number of matching elements
     *
     * \lang simp-chinese
     * @brief 统计等于某值的元素数量。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Ty 要统计的值类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param value 要统计的值
     * @return 匹配元素的数量
     */
    template <typename InputIt, typename Ty>
    RAINY_NODISCARD constexpr rain_fn count(InputIt first, InputIt last, const Ty &value)
        noexcept(noexcept(*first == value)) -> typename utility::iterator_traits<InputIt>::difference_type {
        typename utility::iterator_traits<InputIt>::difference_type result = 0;
        for (; first != last; ++first) {
            if (*first == value) {
                ++result;
            }
        }
        return result;
    }

    /**
     * \lang english
     * @brief Counts the elements satisfying a predicate.
     *
     * @tparam InputIt Input iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param pred The predicate to apply
     * @return The number of matching elements
     *
     * \lang simp-chinese
     * @brief 统计满足谓词的元素数量。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param pred 要应用的谓词
     * @return 匹配元素的数量
     */
    template <typename InputIt, typename Pred>
    RAINY_NODISCARD constexpr rain_fn count_if(InputIt first, InputIt last, Pred pred)
        noexcept(noexcept(pred(*first))) -> typename utility::iterator_traits<InputIt>::difference_type {
        typename utility::iterator_traits<InputIt>::difference_type result = 0;
        for (; first != last; ++first) {
            if (pred(*first)) {
                ++result;
            }
        }
        return result;
    }

    /**
     * \lang english
     * @brief Finds the first position where two ranges differ.
     *
     * @tparam InputIt1 First input iterator type
     * @tparam InputIt2 Second input iterator type
     * @param first1 Iterator to the beginning of the first range
     * @param last1 Iterator to the end of the first range
     * @param first2 Iterator to the beginning of the second range
     * @return A pair of iterators to the first mismatching elements
     *
     * \lang simp-chinese
     * @brief 查找两个范围第一个不同的位置。
     *
     * @tparam InputIt1 第一个输入迭代器类型
     * @tparam InputIt2 第二个输入迭代器类型
     * @param first1 指向第一个范围起始的迭代器
     * @param last1 指向第一个范围末尾的迭代器
     * @param first2 指向第二个范围起始的迭代器
     * @return 指向第一对不同元素的迭代器对
     */
    template <typename InputIt1, typename InputIt2>
    RAINY_NODISCARD constexpr rain_fn mismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2)
        noexcept(noexcept(*first1 == *first2)) -> container::pair<InputIt1, InputIt2> {
        while (first1 != last1 && *first1 == *first2) {
            ++first1, ++first2;
        }
        return {first1, first2};
    }

    /**
     * \lang english
     * @brief Finds the first position where two ranges differ under a predicate.
     *
     * @tparam InputIt1 First input iterator type
     * @tparam InputIt2 Second input iterator type
     * @tparam Pred The predicate type
     * @param first1 Iterator to the beginning of the first range
     * @param last1 Iterator to the end of the first range
     * @param first2 Iterator to the beginning of the second range
     * @param pred The predicate to apply
     * @return A pair of iterators to the first mismatching elements
     *
     * \lang simp-chinese
     * @brief 在谓词下查找两个范围第一个不同的位置。
     *
     * @tparam InputIt1 第一个输入迭代器类型
     * @tparam InputIt2 第二个输入迭代器类型
     * @tparam Pred 谓词类型
     * @param first1 指向第一个范围起始的迭代器
     * @param last1 指向第一个范围末尾的迭代器
     * @param first2 指向第二个范围起始的迭代器
     * @param pred 要应用的谓词
     * @return 指向第一对不同元素的迭代器对
     */
    template <typename InputIt1, typename InputIt2, typename Pred>
    RAINY_NODISCARD constexpr rain_fn mismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2, Pred pred)
        noexcept(noexcept(pred(*first1, *first2))) -> container::pair<InputIt1, InputIt2> {
        while (first1 != last1 && pred(*first1, *first2)) {
            ++first1, ++first2;
        }
        return {first1, first2};
    }

    /**
     * \lang english
     * @brief Finds the first element from a set within a range.
     *
     * @tparam InputIt Input iterator type
     * @tparam ForwardIt Forward iterator type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param first2 Iterator to the beginning of the set
     * @param last2 Iterator to the end of the set
     * @return Iterator to the first found element, or last
     *
     * \lang simp-chinese
     * @brief 在范围内查找来自集合的第一个元素。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam ForwardIt 前向迭代器类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param first2 指向集合起始的迭代器
     * @param last2 指向集合末尾的迭代器
     * @return 指向第一个找到元素的迭代器，未找到则返回last
     */
    template <typename InputIt, typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn find_first_of(InputIt first, InputIt last, ForwardIt first2, ForwardIt last2)
        noexcept(noexcept(*first == *first2)) -> InputIt {
        for (; first != last; ++first) {
            for (ForwardIt it = first2; it != last2; ++it) {
                if (*first == *it) {
                    return first;
                }
            }
        }
        return last;
    }

    /**
     * \lang english
     * @brief Finds the first element from a set within a range under a predicate.
     *
     * @tparam InputIt Input iterator type
     * @tparam ForwardIt Forward iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param first2 Iterator to the beginning of the set
     * @param last2 Iterator to the end of the set
     * @param pred The predicate to apply
     * @return Iterator to the first found element, or last
     *
     * \lang simp-chinese
     * @brief 在谓词下查找来自集合的第一个元素。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param first2 指向集合起始的迭代器
     * @param last2 指向集合末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向第一个找到元素的迭代器，未找到则返回last
     */
    template <typename InputIt, typename ForwardIt, typename Pred>
    RAINY_NODISCARD constexpr rain_fn find_first_of(InputIt first, InputIt last, ForwardIt first2, ForwardIt last2, Pred pred)
        -> InputIt {
        for (; first != last; ++first) {
            for (ForwardIt it = first2; it != last2; ++it) {
                if (pred(*first, *it)) {
                    return first;
                }
            }
        }
        return last;
    }

    /**
     * \lang english
     * @brief Finds the first two adjacent equal elements.
     *
     * @tparam ForwardIt Forward iterator type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @return Iterator to the first of the adjacent equal elements, or last
     *
     * \lang simp-chinese
     * @brief 查找第一对相邻的相等元素。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @return 指向相邻相等元素中第一个的迭代器，未找到则返回last
     */
    template <typename ForwardIt>
    RAINY_NODISCARD constexpr rain_fn adjacent_find(ForwardIt first, ForwardIt last)
        noexcept(noexcept(*first == *first)) -> ForwardIt {
        if (first == last) {
            return last;
        }
        ForwardIt next = first;
        for (++next; next != last; ++next, ++first) {
            if (*first == *next) {
                return first;
            }
        }
        return last;
    }

    /**
     * \lang english
     * @brief Finds the first two adjacent elements satisfying a binary predicate.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param pred The predicate to apply
     * @return Iterator to the first of the matching adjacent elements, or last
     *
     * \lang simp-chinese
     * @brief 查找第一对满足二元谓词的相邻元素。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向匹配相邻元素中第一个的迭代器，未找到则返回last
     */
    template <typename ForwardIt, typename Pred>
    RAINY_NODISCARD constexpr rain_fn adjacent_find(ForwardIt first, ForwardIt last, Pred pred) -> ForwardIt {
        if (first == last) {
            return last;
        }
        ForwardIt next = first;
        for (++next; next != last; ++next, ++first) {
            if (pred(*first, *next)) {
                return first;
            }
        }
        return last;
    }

    /**
     * \lang english
     * @brief Searches for the first occurrence of a subsequence in a range.
     *
     * @tparam ForwardIt1 First forward iterator type
     * @tparam ForwardIt2 Second forward iterator type
     * @param first1 Iterator to the beginning of the range to search
     * @param last1 Iterator to the end of the range to search
     * @param first2 Iterator to the beginning of the subsequence
     * @param last2 Iterator to the end of the subsequence
     * @return Iterator to the beginning of the first occurrence, or last1
     *
     * \lang simp-chinese
     * @brief 在范围内查找子序列的第一次出现。
     *
     * @tparam ForwardIt1 第一个前向迭代器类型
     * @tparam ForwardIt2 第二个前向迭代器类型
     * @param first1 指向被搜索范围起始的迭代器
     * @param last1 指向被搜索范围末尾的迭代器
     * @param first2 指向子序列起始的迭代器
     * @param last2 指向子序列末尾的迭代器
     * @return 指向第一次出现位置起始的迭代器，未找到则返回last1
     */
    template <typename ForwardIt1, typename ForwardIt2>
    RAINY_NODISCARD constexpr rain_fn search(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2, ForwardIt2 last2)
        noexcept(noexcept(*first1 == *first2)) -> ForwardIt1 {
        if (first2 == last2) {
            return first1;
        }
        for (; first1 != last1; ++first1) {
            ForwardIt1 it1 = first1;
            ForwardIt2 it2 = first2;
            for (; it1 != last1 && it2 != last2; ++it1, ++it2) {
                if (!(*it1 == *it2)) {
                    break;
                }
            }
            if (it2 == last2) {
                return first1;
            }
        }
        return last1;
    }

    /**
     * \lang english
     * @brief Searches for the first occurrence of a subsequence under a predicate.
     *
     * @tparam ForwardIt1 First forward iterator type
     * @tparam ForwardIt2 Second forward iterator type
     * @tparam Pred The predicate type
     * @param first1 Iterator to the beginning of the range to search
     * @param last1 Iterator to the end of the range to search
     * @param first2 Iterator to the beginning of the subsequence
     * @param last2 Iterator to the end of the subsequence
     * @param pred The predicate to apply
     * @return Iterator to the beginning of the first occurrence, or last1
     *
     * \lang simp-chinese
     * @brief 在谓词下查找子序列的第一次出现。
     *
     * @tparam ForwardIt1 第一个前向迭代器类型
     * @tparam ForwardIt2 第二个前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first1 指向被搜索范围起始的迭代器
     * @param last1 指向被搜索范围末尾的迭代器
     * @param first2 指向子序列起始的迭代器
     * @param last2 指向子序列末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向第一次出现位置起始的迭代器，未找到则返回last1
     */
    template <typename ForwardIt1, typename ForwardIt2, typename Pred>
    RAINY_NODISCARD constexpr rain_fn search(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2, ForwardIt2 last2,
                                             Pred pred) -> ForwardIt1 {
        if (first2 == last2) {
            return first1;
        }
        for (; first1 != last1; ++first1) {
            ForwardIt1 it1 = first1;
            ForwardIt2 it2 = first2;
            for (; it1 != last1 && it2 != last2; ++it1, ++it2) {
                if (!pred(*it1, *it2)) {
                    break;
                }
            }
            if (it2 == last2) {
                return first1;
            }
        }
        return last1;
    }

    /**
     * \lang english
     * @brief Searches for the last occurrence of a subsequence in a range.
     *
     * @tparam ForwardIt1 First forward iterator type
     * @tparam ForwardIt2 Second forward iterator type
     * @param first1 Iterator to the beginning of the range to search
     * @param last1 Iterator to the end of the range to search
     * @param first2 Iterator to the beginning of the subsequence
     * @param last2 Iterator to the end of the subsequence
     * @return Iterator to the beginning of the last occurrence, or last1
     *
     * \lang simp-chinese
     * @brief 在范围内查找子序列的最后一次出现。
     *
     * @tparam ForwardIt1 第一个前向迭代器类型
     * @tparam ForwardIt2 第二个前向迭代器类型
     * @param first1 指向被搜索范围起始的迭代器
     * @param last1 指向被搜索范围末尾的迭代器
     * @param first2 指向子序列起始的迭代器
     * @param last2 指向子序列末尾的迭代器
     * @return 指向最后一次出现位置起始的迭代器，未找到则返回last1
     */
    template <typename ForwardIt1, typename ForwardIt2>
    RAINY_NODISCARD constexpr rain_fn find_end(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2, ForwardIt2 last2)
        noexcept(noexcept(*first1 == *first2)) -> ForwardIt1 {
        if (first2 == last2) {
            return last1;
        }
        ForwardIt1 result = last1;
        for (ForwardIt1 candidate = first1;; ++candidate) {
            ForwardIt1 search_pos =
                core::algorithm::search(candidate, last1, first2, last2);
            if (search_pos == last1) {
                break;
            }
            result = search_pos;
            candidate = search_pos;
        }
        return result;
    }

    /**
     * \lang english
     * @brief Searches for the last occurrence of a subsequence under a predicate.
     *
     * @tparam ForwardIt1 First forward iterator type
     * @tparam ForwardIt2 Second forward iterator type
     * @tparam Pred The predicate type
     * @param first1 Iterator to the beginning of the range to search
     * @param last1 Iterator to the end of the range to search
     * @param first2 Iterator to the beginning of the subsequence
     * @param last2 Iterator to the end of the subsequence
     * @param pred The predicate to apply
     * @return Iterator to the beginning of the last occurrence, or last1
     *
     * \lang simp-chinese
     * @brief 在谓词下查找子序列的最后一次出现。
     *
     * @tparam ForwardIt1 第一个前向迭代器类型
     * @tparam ForwardIt2 第二个前向迭代器类型
     * @tparam Pred 谓词类型
     * @param first1 指向被搜索范围起始的迭代器
     * @param last1 指向被搜索范围末尾的迭代器
     * @param first2 指向子序列起始的迭代器
     * @param last2 指向子序列末尾的迭代器
     * @param pred 要应用的谓词
     * @return 指向最后一次出现位置起始的迭代器，未找到则返回last1
     */
    template <typename ForwardIt1, typename ForwardIt2, typename Pred>
    RAINY_NODISCARD constexpr rain_fn find_end(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2, ForwardIt2 last2,
                                               Pred pred) -> ForwardIt1 {
        if (first2 == last2) {
            return last1;
        }
        ForwardIt1 result = last1;
        for (ForwardIt1 candidate = first1;; ++candidate) {
            ForwardIt1 search_pos = core::algorithm::search(candidate, last1, first2, last2, pred);
            if (search_pos == last1) {
                break;
            }
            result = search_pos;
            candidate = search_pos;
        }
        return result;
    }

    /**
     * \lang english
     * @brief Searches for the first occurrence of n consecutive copies of a value.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Size The count type
     * @tparam Ty The value type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param count The length of the sequence to find
     * @param value The value to find
     * @return Iterator to the beginning of the found sequence, or last
     *
     * \lang simp-chinese
     * @brief 查找n个连续相等值的第一次出现。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Size 数量类型
     * @tparam Ty 值类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param count 要查找序列的长度
     * @param value 要查找的值
     * @return 指向找到序列起始的迭代器，未找到则返回last
     */
    template <typename ForwardIt, typename Size, typename Ty>
    RAINY_NODISCARD constexpr rain_fn search_n(ForwardIt first, ForwardIt last, Size count, const Ty &value)
        noexcept(noexcept(*first == value)) -> ForwardIt {
        if (count <= 0) {
            return first;
        }
        for (; first != last; ++first) {
            if (!(*first == value)) {
                continue;
            }
            ForwardIt candidate = first;
            Size matched = 1;
            for (++candidate; matched < count && candidate != last; ++candidate, ++matched) {
                if (!(*candidate == value)) {
                    break;
                }
            }
            if (matched == count) {
                return first;
            }
        }
        return last;
    }

    /**
     * \lang english
     * @brief Searches for the first occurrence of n consecutive elements satisfying pred.
     *
     * @tparam ForwardIt Forward iterator type
     * @tparam Size The count type
     * @tparam Ty The value type
     * @tparam Pred The predicate type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param count The length of the sequence to find
     * @param value The value forwarded to the predicate
     * @param pred The predicate to apply
     * @return Iterator to the beginning of the found sequence, or last
     *
     * \lang simp-chinese
     * @brief 查找n个连续满足pred元素的第一次出现。
     *
     * @tparam ForwardIt 前向迭代器类型
     * @tparam Size 数量类型
     * @tparam Ty 值类型
     * @tparam Pred 谓词类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param count 要查找序列的长度
     * @param value 传递给谓词的值
     * @param pred 要应用的谓词
     * @return 指向找到序列起始的迭代器，未找到则返回last
     */
    template <typename ForwardIt, typename Size, typename Ty, typename Pred>
    RAINY_NODISCARD constexpr rain_fn search_n(ForwardIt first, ForwardIt last, Size count, const Ty &value, Pred pred)
        -> ForwardIt {
        if (count <= 0) {
            return first;
        }
        for (; first != last; ++first) {
            if (!pred(*first, value)) {
                continue;
            }
            ForwardIt candidate = first;
            Size matched = 1;
            for (++candidate; matched < count && candidate != last; ++candidate, ++matched) {
                if (!pred(*candidate, value)) {
                    break;
                }
            }
            if (matched == count) {
                return first;
            }
        }
        return last;
    }
}

namespace rainy::algorithm {
    using core::algorithm::adjacent_find;
    using core::algorithm::count;
    using core::algorithm::count_if;
    using core::algorithm::find_end;
    using core::algorithm::find_first_of;
    using core::algorithm::for_each;
    using core::algorithm::for_each_n;
    using core::algorithm::mismatch;
    using core::algorithm::search;
    using core::algorithm::search_n;
}

#endif
