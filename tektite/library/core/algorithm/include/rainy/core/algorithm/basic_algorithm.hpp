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
#ifndef RAINY_CORE_ALGORITHM_BASIC_HPP
#define RAINY_CORE_ALGORITHM_BASIC_HPP
#include <cstring>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::core::algorithm {
    template <typename Iter, typename Size, typename Ty = typename utility::iterator_traits<Iter>::value_type>
    constexpr rain_fn fill_n(Iter first, Size count, const Ty &value) -> Iter {
        for (Size i = 0; i < count; ++i) {
            *first++ = value;
        }
        return first;
    }

    template <typename InputIter, typename OutIter>
    constexpr rain_fn copy_n(InputIter begin, const std::size_t count, OutIter dest) noexcept(
        type_traits::properties::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::modifers::remove_pointer_t<InputIter>,
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
            return dest;
        }
#endif
        if constexpr (type_traits::properties::is_trivially_copyable_v<value_type> &&
                      type_traits::primary_types::is_pointer_v<InputIter> &&
                      type_traits::primary_types::is_pointer_v<OutIter>) {
            const auto input_begin = utility::addressof(*begin);
            auto out_dest = utility::addressof(*dest);
            std::memcpy(out_dest, input_begin, sizeof(value_type) * count);
            return dest + count;
        } else {
            for (std::size_t i = 0; i < count; ++i) {
                *dest = *begin;
                ++dest;
                ++begin;
            }
            return dest;
        }
    }
}

#endif
