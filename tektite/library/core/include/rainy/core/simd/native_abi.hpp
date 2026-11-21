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
#ifndef RAINY_CORE_SIMD_NATIVE_ABI_HPP
#define RAINY_CORE_SIMD_NATIVE_ABI_HPP

#include <rainy/core/simd/fwd.hpp>

namespace rainy::core::implements {
    struct scalar_abi {};

    template <std::size_t Bits>
    struct vector_abi {
        static constexpr std::size_t bit_width = Bits;
    };

#if defined(__AVX512F__)
    constexpr std::size_t native_vector_bits = 512;
#elif RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
    constexpr std::size_t native_vector_bits = 256;
#elif RAINY_IS_X86_PLATFORM && (defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2))
    constexpr std::size_t native_vector_bits = 128;
#elif RAINY_IS_ARM64 && (defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM) || defined(_M_ARM64))
    constexpr std::size_t native_vector_bits = 128;
#else
    constexpr std::size_t native_vector_bits = 0;
#endif

    template <std::size_t Bits>
    struct native_abi_from_bits {
        using type = vector_abi<Bits>;
    };

    template <>
    struct native_abi_from_bits<0> {
        using type = scalar_abi;
    };

    template <typename Ty>
    struct native_abi_impl {
        using type = typename native_abi_from_bits<native_vector_bits>::type;
    };

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_vectorizable_v =
        (type_traits::primary_types::is_integral_v<Ty> || type_traits::primary_types::is_floating_point_v<Ty>) &&
        !type_traits::type_relations::is_same_v<type_traits::implements::remove_cv_t<Ty>, bool> &&
        !type_traits::type_relations::is_same_v<type_traits::implements::remove_cv_t<Ty>, long double>;

    template <typename Ty, std::size_t Bits>
    struct simd_size_impl<Ty, vector_abi<Bits>> {
        static constexpr simd_size_type value = is_vectorizable_v<Ty> && Bits % (sizeof(Ty) * 8) == 0
                                                    ? static_cast<simd_size_type>(Bits / (sizeof(Ty) * 8))
                                                    : 0;
    };

    template <typename Ty>
    struct simd_size_impl<Ty, scalar_abi> {
        static constexpr simd_size_type value = is_vectorizable_v<Ty> ? 1 : 0;
    };

    template <typename Ty, simd_size_type N, bool Enable>
    struct deduce_abi_selector {};

    template <typename Ty, simd_size_type N>
    struct deduce_abi_selector<Ty, N, true> {
        using type = type_traits::other_trans::conditional_t<
            N == simd_size_v<Ty, native_abi_t<Ty>>, native_abi_t<Ty>,
            type_traits::other_trans::conditional_t<
                N == 1, scalar_abi, vector_abi<static_cast<std::size_t>(N) * sizeof(Ty) * 8>>>;
    };

    template <typename Ty, simd_size_type N>
    struct deduce_abi_impl
        : deduce_abi_selector<Ty, N,
                              (is_vectorizable_v<Ty> && N > 0 &&
                               (N == simd_size_v<Ty, native_abi_t<Ty>> || N == 1 ||
                                (has_single_bit(static_cast<type_traits::helper::make_unsigned_t<simd_size_type>>(N)) &&
                                 N * static_cast<simd_size_type>(sizeof(Ty) * 8) <= 512)))> {};
}

#endif
