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
#ifndef RAINY_CORE_SIMD_FWD_HPP
#define RAINY_CORE_SIMD_FWD_HPP

#include <rainy/core/container/pair.hpp>
#include <rainy/core/functional/functor.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/helper.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/primary_types.hpp>
#include <rainy/core/type_traits/properties.hpp>

namespace rainy::core {
    /*
    p1928r11 type simd-size-type is an exposition-only alias for a signed integer type.
    I.e. the implementation is free to choose any signed integer type.
    */
    using simd_size_type = int;

    // [simd.traits], simd type traits. Declared up front because
    // implements::aligned_flag::adjust_pointer already refers to alignment_v.
    template <typename Ty, typename UTy = typename Ty::value_type>
    struct alignment;

    template <typename Ty, typename UTy = typename Ty::value_type>
    constexpr std::size_t alignment_v = alignment<Ty, UTy>::value;
}

namespace rainy::core::implements {
    template <typename Ty>
    constexpr int popcount(Ty __x) noexcept {
        using utype = type_traits::helper::make_unsigned_t<Ty>;
        utype u = static_cast<utype>(__x);

        constexpr int s = utility::numeric_limits<utype>::digits;

        u = u - ((u >> 1) & static_cast<utype>(~utype{0} / 3));
        u = (u & static_cast<utype>(~utype{0} / 5)) + ((u >> 2) & static_cast<utype>(~utype{0} / 5));
        u = (u + (u >> 4)) & static_cast<utype>(~utype{0} / 17);
        u = u + (u >> 8);
        u = u + (u >> 16);

        if constexpr (s > 32) {
            u = u + (u >> 32);
        }
        if constexpr (s > 64) {
            u = u + (u >> 64);
            u = u + (u >> 128);
        }

        return static_cast<int>(u & static_cast<utype>(0x7F));
    }

    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Ty>, int> = 0>
    constexpr bool has_single_bit(Ty val) noexcept {
        return implements::popcount(val) == 1;
    }

    struct load_store_tag {};

    template <std::size_t N, typename UTy>
    constexpr UTy *assume_aligned(UTy *ptr) noexcept {
#if RAINY_USING_CLANG || RAINY_USING_GCC

#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return ptr;
        } else
#endif
        {
            return static_cast<UTy *>(__builtin_assume_aligned(ptr, N));
        }
#else
        return ptr;
#endif
    }

    // exposition-only (see [simd.flags]): convert-flag and aligned-flag
    struct convert_flag : load_store_tag {};

    struct aligned_flag : load_store_tag {
        // The assumed alignment comes from alignment_v<V, element>
        // ([simd.copy] paragraph 13): V is the data-parallel type being loaded or
        // stored and element is the cv-unqualified type of the addressed storage
        // (iter_value_t<It>; bool for mask copy operations). Requires
        // alignment to be defined per [simd.traits].
        template <typename V, typename UTy>
        static constexpr UTy *adjust_pointer(UTy *ptr) noexcept {
            return assume_aligned<alignment_v<V, type_traits::implements::remove_cv_t<UTy>>>(ptr);
        }
    };

    // exposition-only (see [simd.flags]): overaligned-flag<N>
    template <std::size_t N>
    struct overaligned_flag : load_store_tag {
        static_assert(has_single_bit(N));

        template <typename UTy>
        static constexpr UTy *adjust_pointer(UTy *ptr) noexcept {
            return assume_aligned<N>(ptr);
        }
    };

    template <typename It>
    using iter_difference_t = typename type_traits::extras::iterators::iterator_traits<It>::difference_type;

    template <typename Ty>
    struct native_abi_impl; // native-abi<Ty>

    template <typename Ty, simd_size_type N>
    struct deduce_abi_impl; // deduce-t<Ty, N>

    template <typename Ty, typename Abi>
    struct simd_size_impl; // simd-size-val<Ty, Abi>, does not instantiate basic_vec

    // integer-from<Bytes>: a signed integer type of size Bytes ([simd.syn] paragraph 4)
    template <std::size_t Bytes>
    struct integer_from_impl;

    template <>
    struct integer_from_impl<1> {
        using type = std::int8_t;
    };

    template <>
    struct integer_from_impl<2> {
        using type = std::int16_t;
    };

    template <>
    struct integer_from_impl<4> {
        using type = std::int32_t;
    };

    template <>
    struct integer_from_impl<8> {
        using type = std::int64_t;
    };

    template <std::size_t Bytes>
    using integer_from_t = typename integer_from_impl<Bytes>::type;

    // deduced-vec-t<V> (exposition): the data-parallel type deduced from V for
    // the math library of [simd.math]. Definition in simd/deduced_vec.hpp.
    template <typename V>
    struct deduced_vec;

    // math-common-simd-t<Vs...> (exposition): the common data-parallel type of
    // mixed math arguments. Definition in simd/math_common.hpp.
    template <typename... Vs>
    struct math_common_vec;

    template <typename... Vs>
    using math_common_vec_t = typename math_common_vec<Vs...>::type;

    // mask-element-size<Ty> (exposition); the basic_mask partial
    // specialization is added below, after the class template is declared
    template <typename Ty>
    struct mask_element_size_impl;

    template <typename Op, typename Ty, typename = void>
    struct is_reduction_op;

    // [simd.cond] result type of the exposition-only simd-select-impl.
    // Definition in simd/basic_mask.hpp.
    template <typename Mask, typename Ty, typename U>
    struct simd_select_result;

    // [simd.math] constraint: V is a basic_vec of floating-point elements
    template <typename Ty, typename = void>
    struct is_math_floating_point : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_math_floating_point<Ty, type_traits::implements::void_t<typename Ty::value_type>>
        : type_traits::helper::bool_constant<type_traits::primary_types::is_floating_point_v<typename Ty::value_type>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_math_floating_point_v = is_math_floating_point<Ty>::value;

    // C++17 SFINAE helper replacing the exposition-only math constraint
    template <typename V>
    using enable_if_math_t = type_traits::other_trans::enable_if_t<is_math_floating_point_v<V>, int>;

    template <typename G, typename = void>
    struct is_basic_vec_generator : type_traits::helper::false_type {};

    template <typename G>
    struct is_basic_vec_generator<G, type_traits::implements::void_t<decltype(utility::declval<G &>()(
                                          type_traits::helper::integral_constant<simd_size_type, 0>()))>>
        : type_traits::helper::true_type {};
}

namespace rainy::core {
    template <typename Ty>
    using native_abi_t = typename implements::native_abi_impl<Ty>::type;

    template <typename Ty, simd_size_type N>
    using deduce_abi_t = typename implements::deduce_abi_impl<Ty, N>::type;

    // simd-size-val<Ty, Abi> (exposition): width of basic_vec<Ty, Abi>; does not
    // require instantiation of basic_vec ([simd.syn] paragraph 2)
    template <typename Ty, typename Abi>
    constexpr simd_size_type simd_size_v = implements::simd_size_impl<Ty, Abi>::value;

    // [simd.traits], simd type traits (alignment is declared at the top of this header)
    template <typename Ty, typename V>
    struct rebind; // member typedef `type` defined per [simd.traits] paragraph 4/5

    template <typename Ty, typename V>
    using rebind_t = typename rebind<Ty, V>::type;

    template <simd_size_type N, typename V>
    struct resize; // member typedef `type` defined per [simd.traits] paragraph 6-8

    template <simd_size_type N, typename V>
    using resize_t = typename resize<N, V>::type;

    template <typename... Flags>
    struct flags {
        template <typename... Other>
        friend constexpr auto operator|(flags, flags<Other...>) {
            return flags<Flags..., Other...>{};
        }
    };

    inline constexpr flags<> flag_default{};
    inline constexpr flags<implements::convert_flag> flag_convert{};
    inline constexpr flags<implements::aligned_flag> flag_aligned{};
    template <std::size_t N, type_traits::other_trans::enable_if_t<implements::has_single_bit(N), int> = 0>
    constexpr flags<implements::overaligned_flag<N>> flag_overaligned{};

    // [simd.class], Class template basic_vec
    template <typename Ty, typename Abi = native_abi_t<Ty>>
    class basic_vec;

    template <typename Ty, simd_size_type N = simd_size_v<Ty, native_abi_t<Ty>>>
    using vec = basic_vec<Ty, deduce_abi_t<Ty, N>>;

    // [simd.mask.class], Class template basic_mask
    template <std::size_t Bytes, typename Abi = native_abi_t<implements::integer_from_t<Bytes>>>
    class basic_mask;

    template <typename Ty, simd_size_type N = simd_size_v<Ty, native_abi_t<Ty>>>
    using mask = basic_mask<sizeof(Ty), deduce_abi_t<Ty, N>>;

    // deduced-vec-t<V> (exposition), see [simd.math]
    template <typename V>
    using deduced_vec_t = typename implements::deduced_vec<V>::type;
}

namespace rainy::core::implements {
    template <std::size_t Bytes, typename Abi>
    struct mask_element_size_impl<basic_mask<Bytes, Abi>> {
        static constexpr std::size_t value = Bytes;
    };
}

namespace rainy::core {
    // mask-element-size<Ty> (exposition): Bytes of basic_mask<Bytes, Abi> ([simd.syn] paragraph 3)
    template <typename Ty>
    constexpr std::size_t mask_element_size = implements::mask_element_size_impl<Ty>::value;

    // [simd.creation], basic_vec and basic_mask creation
    template <typename Ty, typename Abi>
    constexpr auto chunk(const basic_vec<typename Ty::value_type, Abi> &val) noexcept;
    template <typename Ty, typename Abi>
    constexpr auto chunk(const basic_mask<mask_element_size<Ty>, Abi> &val) noexcept;
    template <simd_size_type N, typename Ty, typename Abi>
    constexpr auto chunk(const basic_vec<Ty, Abi> &val) noexcept;
    template <simd_size_type N, std::size_t Bytes, typename Abi>
    constexpr auto chunk(const basic_mask<Bytes, Abi> &val) noexcept;

    template <typename Ty, typename Abi0, typename... Abis>
    constexpr resize_t<(basic_vec<Ty, Abi0>::size() + ... + basic_vec<Ty, Abis>::size()), basic_vec<Ty, Abi0>> cat(
        const basic_vec<Ty, Abi0> &, const basic_vec<Ty, Abis> &...) noexcept;
    template <std::size_t Bytes, typename Abi0, typename... Abis>
    constexpr resize_t<(basic_mask<Bytes, Abi0>::size() + ... + basic_mask<Bytes, Abis>::size()), basic_mask<Bytes, Abi0>> cat(
        const basic_mask<Bytes, Abi0> &, const basic_mask<Bytes, Abis> &...) noexcept;

    // [simd.mask.reductions], basic_mask reductions
    template <std::size_t Bytes, typename Abi>
    constexpr bool all_of(const basic_mask<Bytes, Abi> &) noexcept;
    template <std::size_t Bytes, typename Abi>
    constexpr bool any_of(const basic_mask<Bytes, Abi> &) noexcept;
    template <std::size_t Bytes, typename Abi>
    constexpr bool none_of(const basic_mask<Bytes, Abi> &) noexcept;
    template <std::size_t Bytes, typename Abi>
    constexpr simd_size_type reduce_count(const basic_mask<Bytes, Abi> &) noexcept;
    template <std::size_t Bytes, typename Abi>
    constexpr simd_size_type reduce_min_index(const basic_mask<Bytes, Abi> &);
    template <std::size_t Bytes, typename Abi>
    constexpr simd_size_type reduce_max_index(const basic_mask<Bytes, Abi> &);

    constexpr bool all_of(bool) noexcept;
    constexpr bool any_of(bool) noexcept;
    constexpr bool none_of(bool) noexcept;
    constexpr simd_size_type reduce_count(bool) noexcept;
    constexpr simd_size_type reduce_min_index(bool);
    constexpr simd_size_type reduce_max_index(bool);

    // [simd.reductions], basic_vec reductions
    template <typename Ty, typename Abi, typename BinaryOperation = functional::plus<>,
              type_traits::other_trans::enable_if_t<implements::is_reduction_op<BinaryOperation, Ty>::value, int> = 0>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &, BinaryOperation = {});
    template <typename Ty, typename Abi, typename BinaryOperation>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                        type_traits::primary_types::type_identity_t<Ty> identity_element, BinaryOperation binary_op);
    template <typename Ty, typename Abi>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                        functional::plus<> binary_op = {}) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                        functional::multiplies<> binary_op) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                        functional::bit_and<> binary_op) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                        functional::bit_or<> binary_op) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                        functional::bit_xor<> binary_op) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce_min(const basic_vec<Ty, Abi> &) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce_min(const basic_vec<Ty, Abi> &, const typename basic_vec<Ty, Abi>::mask_type &) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce_max(const basic_vec<Ty, Abi> &) noexcept;
    template <typename Ty, typename Abi>
    constexpr Ty reduce_max(const basic_vec<Ty, Abi> &, const typename basic_vec<Ty, Abi>::mask_type &) noexcept;

    // [simd.alg], Algorithms
    template <typename Ty, typename Abi>
    constexpr basic_vec<Ty, Abi> min(const basic_vec<Ty, Abi> &a, const basic_vec<Ty, Abi> &b) noexcept;
    template <typename Ty, typename Abi>
    constexpr basic_vec<Ty, Abi> max(const basic_vec<Ty, Abi> &a, const basic_vec<Ty, Abi> &b) noexcept;
    template <typename Ty, typename Abi>
    constexpr container::pair<basic_vec<Ty, Abi>, basic_vec<Ty, Abi>> minmax(const basic_vec<Ty, Abi> &a,
                                                                               const basic_vec<Ty, Abi> &b) noexcept;
    template <typename Ty, typename Abi>
    constexpr basic_vec<Ty, Abi> clamp(const basic_vec<Ty, Abi> &val, const basic_vec<Ty, Abi> &lo, const basic_vec<Ty, Abi> &hi);

    // [simd.cond]
    template <typename Ty, typename UTy>
    constexpr auto select(bool c, const Ty &a, const UTy &b) -> type_traits::implements::remove_cvref_t<decltype(c ? a : b)>;
    template <std::size_t Bytes, typename Abi, typename Ty, typename U>
    constexpr auto select(const basic_mask<Bytes, Abi> &c, const Ty &a, const U &b) noexcept
        -> decltype(simd_select_impl(c, a, b));

    // basic_vec load and store functions
    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr V unchecked_load(It first, implements::iter_difference_t<It> n, flags<Flags...> f = {});
    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr V unchecked_load(It first, implements::iter_difference_t<It> n, const typename V::mask_type &k,
                                    flags<Flags...> f = {});

    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr V partial_load(It first, implements::iter_difference_t<It> n, flags<Flags...> f = {});
    template <typename V, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr V partial_load(It first, implements::iter_difference_t<It> n, const typename V::mask_type &k,
                                  flags<Flags...> f = {});

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr void unchecked_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                        flags<Flags...> f = {});
    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr void unchecked_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                        const typename basic_vec<Ty, Abi>::mask_type &k, flags<Flags...> f = {});

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr void partial_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                      flags<Flags...> f = {});

    template <typename Ty, typename Abi, typename It, typename... Flags,
              type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_contiguous_iterator_v<It>, int> = 0>
    constexpr void partial_store(const basic_vec<Ty, Abi> &val, It first, implements::iter_difference_t<It> n,
                                      const typename basic_vec<Ty, Abi>::mask_type &k, flags<Flags...> f = {});

    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> acos(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> asin(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> atan(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> atan2(const V &y, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> atan2(const deduced_vec_t<V> &y, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> atan2(const V &y, const deduced_vec_t<V> &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> cos(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> sin(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> tan(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> acosh(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> asinh(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> atanh(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> cosh(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> sinh(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> tanh(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> exp(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> exp2(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> expm1(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> frexp(const V &value, rebind_t<int, deduced_vec_t<V>> *exp);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr rebind_t<int, deduced_vec_t<V>> ilogb(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> ldexp(const V &val, const rebind_t<int, deduced_vec_t<V>> &exp);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> log(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> log10(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> log1p(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> log2(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> logb(const V &val);
    template <typename Ty, typename Abi>
    constexpr basic_vec<Ty, Abi> modf(const type_traits::primary_types::type_identity_t<basic_vec<Ty, Abi>> &value,
                                       basic_vec<Ty, Abi> *iptr);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> scalbn(const V &val, const rebind_t<int, deduced_vec_t<V>> &n);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> scalbln(const V &val, const rebind_t<long int, deduced_vec_t<V>> &n);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> cbrt(const V &val);
    template <typename Ty, typename Abi,
              type_traits::other_trans::enable_if_t<
                  type_traits::primary_types::is_integral_v<Ty> && type_traits::properties::is_signed_v<Ty>, int> = 0>
    constexpr basic_vec<Ty, Abi> abs(const basic_vec<Ty, Abi> &j);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> abs(const V &j);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fabs(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const V &val, const V &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const V &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const V &val, const deduced_vec_t<V> &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const V &val, const V &y, const deduced_vec_t<V> &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const deduced_vec_t<V> &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const V &y, const deduced_vec_t<V> &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> hypot(const V &val, const deduced_vec_t<V> &y, const deduced_vec_t<V> &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> pow(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> pow(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> pow(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> sqrt(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> erf(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> erfc(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lgamma(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> tgamma(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> ceil(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> floor(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> nearbyint(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> rint(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    rebind_t<long int, deduced_vec_t<V>> lrint(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    rebind_t<long long int, deduced_vec_t<V>> llrint(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> round(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr rebind_t<long int, deduced_vec_t<V>> lround(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr rebind_t<long long int, deduced_vec_t<V>> llround(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> trunc(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmod(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmod(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmod(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> remainder(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> remainder(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> remainder(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> remquo(const V &val, const V &y, rebind_t<int, deduced_vec_t<V>> *quo);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> remquo(const deduced_vec_t<V> &val, const V &y, rebind_t<int, deduced_vec_t<V>> *quo);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> remquo(const V &val, const deduced_vec_t<V> &y, rebind_t<int, deduced_vec_t<V>> *quo);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> copysign(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> copysign(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> copysign(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> nextafter(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> nextafter(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> nextafter(const V &val, const deduced_vec_t<V> &y);

    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fdim(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fdim(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fdim(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmax(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmax(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmax(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmin(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmin(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fmin(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const V &val, const V &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const deduced_vec_t<V> &val, const V &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const V &val, const deduced_vec_t<V> &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const V &val, const V &y, const deduced_vec_t<V> &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const deduced_vec_t<V> &val, const deduced_vec_t<V> &y, const V &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const deduced_vec_t<V> &val, const V &y, const deduced_vec_t<V> &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> fma(const V &val, const deduced_vec_t<V> &y, const deduced_vec_t<V> &z);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const V &a, const V &b, const V &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const deduced_vec_t<V> &a, const V &b, const V &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const V &a, const deduced_vec_t<V> &b, const V &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const V &a, const V &b, const deduced_vec_t<V> &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const deduced_vec_t<V> &a, const deduced_vec_t<V> &b, const V &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const deduced_vec_t<V> &a, const V &b, const deduced_vec_t<V> &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr deduced_vec_t<V> lerp(const V &a, const deduced_vec_t<V> &b, const deduced_vec_t<V> &t) noexcept;
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr rebind_t<int, deduced_vec_t<V>> fpclassify(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isfinite(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isinf(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isnan(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isnormal(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type signbit(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isgreater(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isgreater(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isgreater(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isgreaterequal(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isgreaterequal(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isgreaterequal(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isless(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isless(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isless(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type islessequal(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type islessequal(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type islessequal(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type islessgreater(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type islessgreater(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type islessgreater(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isunordered(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isunordered(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    constexpr typename deduced_vec_t<V>::mask_type isunordered(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> assoc_laguerre(const rebind_t<unsigned, deduced_vec_t<V>> &n,
                                     const rebind_t<unsigned, deduced_vec_t<V>> &m, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> assoc_legendre(const rebind_t<unsigned, deduced_vec_t<V>> &l,
                                     const rebind_t<unsigned, deduced_vec_t<V>> &m, const V &val);

    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> beta(const V &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> beta(const deduced_vec_t<V> &val, const V &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> beta(const V &val, const deduced_vec_t<V> &y);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> comp_ellint_1(const V &k);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> comp_ellint_2(const V &k);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> comp_ellint_3(const V &k, const V &nu);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> comp_ellint_3(const deduced_vec_t<V> &k, const V &nu);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> comp_ellint_3(const V &k, const deduced_vec_t<V> &nu);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_i(const V &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_i(const deduced_vec_t<V> &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_i(const V &nu, const deduced_vec_t<V> &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_j(const V &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_j(const deduced_vec_t<V> &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_j(const V &nu, const deduced_vec_t<V> &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_k(const V &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_k(const deduced_vec_t<V> &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_bessel_k(const V &nu, const deduced_vec_t<V> &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_neumann(const V &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_neumann(const deduced_vec_t<V> &nu, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> cyl_neumann(const V &nu, const deduced_vec_t<V> &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_1(const V &k, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_1(const deduced_vec_t<V> &k, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_1(const V &k, const deduced_vec_t<V> &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_2(const V &k, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_2(const deduced_vec_t<V> &k, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_2(const V &k, const deduced_vec_t<V> &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const V &k, const V &nu, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const deduced_vec_t<V> &k, const V &nu, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const V &k, const deduced_vec_t<V> &nu, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const V &k, const V &nu, const deduced_vec_t<V> &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const deduced_vec_t<V> &k, const deduced_vec_t<V> &nu, const V &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const deduced_vec_t<V> &k, const V &nu, const deduced_vec_t<V> &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> ellint_3(const V &k, const deduced_vec_t<V> &nu, const deduced_vec_t<V> &phi);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> expint(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> hermite(const rebind_t<unsigned, deduced_vec_t<V>> &n, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> laguerre(const rebind_t<unsigned, deduced_vec_t<V>> &n, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> legendre(const rebind_t<unsigned, deduced_vec_t<V>> &l, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> riemann_zeta(const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> sph_bessel(const rebind_t<unsigned, deduced_vec_t<V>> &n, const V &val);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> sph_legendre(const rebind_t<unsigned, deduced_vec_t<V>> &l,
                                   const rebind_t<unsigned, deduced_vec_t<V>> &m, const V &theta);
    template <typename V, implements::enable_if_math_t<V> = 0>
    deduced_vec_t<V> sph_neumann(const rebind_t<unsigned, deduced_vec_t<V>> &n, const V &val);
}

#endif
