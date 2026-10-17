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
#ifndef RAINY_CORE_SIMD_LOAD_STORE_HPP
#define RAINY_CORE_SIMD_LOAD_STORE_HPP

#include <rainy/core/simd/basic_vec.hpp>
#include <rainy/core/simd/fwd.hpp>

namespace rainy::core {
    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 V partial_load(It first, implements::iter_difference_t<It> n,
                                  const typename V::mask_type &mask, flags<Flags...> f) {
        (void)f;
        using ops = implements::simd_ops<typename V::value_type, typename V::abi_type>;
        if constexpr (std::is_pointer_v<It>) {
            if (n >= V::size()) {
                auto loaded = ops::load(first);
                return V{ops::blend(static_cast<typename V::mask_type::register_type>(mask), loaded,
                                    ops::broadcast(typename V::value_type()))};
            }
        }
        typename V::register_type reg{};
        const typename V::value_type empty{};
        for (simd_size_type i = 0; i < V::size(); ++i) {
            ops::set_lane(reg, i, (mask[i] && i < n) ? static_cast<typename V::value_type>(*(first + i)) : empty);
        }
        return V{reg};
    }

    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 V partial_load(It first, implements::iter_difference_t<It> n, flags<Flags...> f) {
        (void)f;
        using ops = implements::simd_ops<typename V::value_type, typename V::abi_type>;
        if constexpr (std::is_pointer_v<It>) {
            if (n >= V::size()) {
                return V{ops::load(first)};
            }
        }
        typename V::register_type reg{};
        const typename V::value_type empty{};
        for (simd_size_type i = 0; i < V::size(); ++i) {
            ops::set_lane(reg, i, i < n ? static_cast<typename V::value_type>(*(first + i)) : empty);
        }
        return V{reg};
    }

    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 V unchecked_load(It first, implements::iter_difference_t<It> n,
                                    const typename V::mask_type &mask, flags<Flags...> f) {
        return partial_load<V>(first, n, mask, f);
    }

    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 V unchecked_load(It first, implements::iter_difference_t<It> n, flags<Flags...> f) {
        return partial_load<V>(first, n, typename V::mask_type(true), f);
    }

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 void partial_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                      const typename basic_vec<Ty, Abi>::mask_type &mask, flags<Flags...> f) {
        (void)f;
        using ops = implements::simd_ops<Ty, Abi>;
        if constexpr (std::is_pointer_v<It>) {
            if (n >= basic_vec<Ty, Abi>::size()) {
                auto stored = ops::load(first);
                auto blended = ops::blend(static_cast<typename basic_vec<Ty, Abi>::mask_type::register_type>(mask),
                                          static_cast<typename basic_vec<Ty, Abi>::register_type>(val), stored);
                ops::store(first, blended);
                return;
            }
        }
        for (simd_size_type i = 0; i < basic_vec<Ty, Abi>::size(); ++i) {
            if (mask[i] && i < n) {
                *(first + i) = val[i];
            }
        }
    }

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 void partial_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                      flags<Flags...> f) {
        using ops = implements::simd_ops<Ty, Abi>;
        if constexpr (std::is_pointer_v<It>) {
            if (n >= basic_vec<Ty, Abi>::size()) {
                ops::store(first, static_cast<typename basic_vec<Ty, Abi>::register_type>(val));
                return;
            }
        }
        (void)f;
        for (simd_size_type i = 0; i < basic_vec<Ty, Abi>::size(); ++i) {
            if (i < n) {
                *(first + i) = val[i];
            }
        }
    }

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 void unchecked_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                        const typename basic_vec<Ty, Abi>::mask_type &mask, flags<Flags...> f) {
        partial_store(val, first, n, mask, f);
    }

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int>>
    RAINY_CONSTEXPR26 void unchecked_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                        flags<Flags...> f) {
        partial_store(val, first, n, f);
    }
}

#endif
