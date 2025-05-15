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
    RAINY_CONSTEXPR20 void iter_swap(Iter1 a, Iter2 b) {
        using std::swap;
        swap(*a, *b);
    }

    template <typename Ty,
              typename type_traits::implements::_enable_if<
                  type_traits::implements::_is_move_constructible_v<Ty> && type_traits::implements::_is_move_assignable_v<Ty>, int>::type = 0>
    constexpr void swap(Ty &left, Ty &right) noexcept(type_traits::implements::_is_nothrow_move_constructible_v<Ty> &&
                                                      type_traits::implements::_is_nothrow_move_assignable_v<Ty>) {
        Ty temp = utility::move(left);
        left = utility::move(right);
        right = utility::move(temp);
    }
}

namespace rainy::core::algorithm {
    template <typename Iter1, typename Iter2>
    RAINY_CONSTEXPR20 Iter1 swap_ranges(Iter1 first1, Iter1 last1, Iter2 first2) {
        for (; first1 != last1; ++first1, ++first2) {
            iter_swap(first1, first2);
        }
        return first2;
    }

    template <typename Iter, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    void fill(Iter first, Iter end, const Ty &value) {
        for (; first != end; ++first) {
            *first = value;
        }
    }

    template <typename Iter, typename Size, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    Iter fill_n(Iter first, Size count, const Ty &value) {
        for (Size i = 0; i < count; i++) {
            *first++ = value;
        }
        return first;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool all_of(Iter first, Iter last, Pred pred) {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return false;
            }
        }
        return true;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool any_of(Iter first, Iter last, Pred pred) {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return true;
            }
        }
        return false;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool none_of(Iter first, Iter last, Pred pred) {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return false;
            }
        }
        return true;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD RAINY_CONSTEXPR20 Iter find_if(Iter first, Iter last, Pred pred) {
        for (; first != last; ++first) {
            if (pred(*first)) {
                return first;
            }
        }
        return last;
    }

    template <typename Iter, typename Pred>
    RAINY_NODISCARD RAINY_CONSTEXPR20 Iter find_if_not(Iter first, Iter last, Pred pred) {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return first;
            }
        }
        return last;
    }
}

#endif