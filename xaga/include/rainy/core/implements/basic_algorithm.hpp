/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_CORE_IMPLEMENTS_BASIC_ALGORITHM_HPP
#define RAINY_CORE_IMPLEMENTS_BASIC_ALGORITHM_HPP
#include <utility>
#include <rainy/core/tmp/iter_traits.hpp>
#include <rainy/core/tmp/implements.hpp>

namespace rainy::utility {
    template <typename Iter1, typename Iter2>
    RAINY_CONSTEXPR20 rain_fn iter_swap(Iter1 a, Iter2 b) -> void {
        using std::swap;
        swap(*a, *b);
    }

    using std::swap;
}

namespace rainy::core::algorithm {
    template <typename Iter1, typename Iter2>
    RAINY_CONSTEXPR20 rain_fn swap_ranges(Iter1 first1, Iter1 last1, Iter2 first2) -> Iter1 {
        for (; first1 != last1; ++first1, ++first2) {
            iter_swap(first1, first2);
        }
        return first2;
    }

    template <typename Iter, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn fill(Iter first, Iter end, const Ty &value) -> void {
        for (; first != end; ++first) {
            *first = value;
        }
    }

    template <typename Iter, typename Size, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn fill_n(Iter first, Size count, const Ty &value) -> Iter {
        for (Size i = 0; i < count; i++) {
            *first++ = value;
        }
        return first;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn all_of(Iter first, Iter last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return false;
            }
        }
        return true;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn any_of(Iter first, Iter last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return true;
            }
        }
        return false;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn none_of(Iter first, Iter last, Pred pred) -> bool {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return false;
            }
        }
        return true;
    }

    template <typename Iter, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn find(Iter first, Iter last, const Ty &value) -> Iter {
        for (; first != last; ++first) {
            if (*first == value) {
                return first;
            }
        }
        return last;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn find_if(Iter first, Iter last, Pred pred) -> Iter {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return first;
            }
        }
        return last;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD constexpr rain_fn find_if_not(Iter first, Iter last, Pred pred) -> Iter {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return first;
            }
        }
        return last;
    }

    template <typename Iter1, typename Iter2>
    RAINY_NODISCARD inline constexpr rain_fn equal(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) -> bool {
        for (; first1 != last1 && first2 != last2 && *first1 == *first2; ++first2) {
            ++first1; // 避免错误的优化
        }
        return first1 == last1 && first2 == last2;
    }

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
}

#endif