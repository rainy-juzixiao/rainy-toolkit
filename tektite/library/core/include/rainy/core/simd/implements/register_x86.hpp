#ifndef RAINY_CORE_SIMD_REGISTER_X86_HPP
#define RAINY_CORE_SIMD_REGISTER_X86_HPP

#include <cstdint>
#include <cstring>
#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/simd/native_abi.hpp>
#include <rainy/core/simd/implements/register_scalar.hpp>

#if RAINY_USING_MSVC
#include <immintrin.h>
#elif defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#endif

#if defined(__AVX512BW__)
#define RAINY_SIMD_AVX512BW 1
#else
#define RAINY_SIMD_AVX512BW 0
#endif

#if defined(__AVX512F__)
#define RAINY_SIMD_AVX512F 1
#else
#define RAINY_SIMD_AVX512F 0
#endif

#if defined(__AVX2__)
#define RAINY_SIMD_AVX2 1
#else
#define RAINY_SIMD_AVX2 0
#endif

#if defined(__AVX__)
#define RAINY_SIMD_AVX 1
#else
#define RAINY_SIMD_AVX 0
#endif

#if defined(__SSE4_1__) || defined(__AVX__)
#define RAINY_SIMD_SSE41 1
#else
#define RAINY_SIMD_SSE41 0
#endif

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define RAINY_SIMD_SSE2 1
#else
#define RAINY_SIMD_SSE2 0
#endif

namespace rainy::core::implements {
#if RAINY_USING_MSVC
    template <typename Ty, std::size_t Bits>
    struct simd_register<Ty, vector_abi<Bits>> {
        using type = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_floating_point_v<Ty>,
            type_traits::other_trans::conditional_t<
                sizeof(Ty) == 4,
                type_traits::other_trans::conditional_t<Bits == 128, __m128,
                                                        type_traits::other_trans::conditional_t<Bits == 256, __m256, __m512>>,
                type_traits::other_trans::conditional_t<Bits == 128, __m128d,
                                                        type_traits::other_trans::conditional_t<Bits == 256, __m256d, __m512d>>>,
            type_traits::other_trans::conditional_t<Bits == 128, __m128i,
                                                    type_traits::other_trans::conditional_t<Bits == 256, __m256i, __m512i>>>;
    };
#else
    template <typename Ty, std::size_t Bits>
    struct simd_register<Ty, vector_abi<Bits>> {
#if RAINY_USING_CLANG
        using type = Ty __attribute__((ext_vector_type(Bits / (8 * sizeof(Ty)))));
#else
        typedef Ty type __attribute__((vector_size(Bits / 8)));
#endif
    };
#endif

#if RAINY_USING_MSVC
    template <typename Ty, std::size_t Bits>
    struct simd_ops<Ty, vector_abi<Bits>> {
        static_assert(RAINY_SIMD_SSE2, "x86 vector backend requires SSE2");

        using register_type = typename simd_register<Ty, vector_abi<Bits>>::type;
        using mask_register_type = register_t<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>;
        static constexpr simd_size_type size = simd_size_v<Ty, vector_abi<Bits>>;

        static RAINY_CONSTEXPR26 register_type broadcast(Ty value) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_set1_ps(value);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_set1_ps(value);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_set1_ps(value);
                    } else {
                        return broadcast_lanes(value);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_set1_pd(value);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_set1_pd(value);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_set1_pd(value);
                    } else {
                        return broadcast_lanes(value);
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    if constexpr (sizeof(Ty) == 1) {
                        return _mm_set1_epi8(static_cast<char>(value));
                    } else if constexpr (sizeof(Ty) == 2) {
                        return _mm_set1_epi16(static_cast<short>(value));
                    } else if constexpr (sizeof(Ty) == 4) {
                        return _mm_set1_epi32(static_cast<int>(value));
                    } else {
                        return _mm_set_epi64x(static_cast<__int64>(value), static_cast<__int64>(value));
                    }
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    if constexpr (sizeof(Ty) == 1) {
                        return _mm256_set1_epi8(static_cast<char>(value));
                    } else if constexpr (sizeof(Ty) == 2) {
                        return _mm256_set1_epi16(static_cast<short>(value));
                    } else if constexpr (sizeof(Ty) == 4) {
                        return _mm256_set1_epi32(static_cast<int>(value));
                    } else {
                        return _mm256_set1_epi64x(static_cast<__int64>(value));
                    }
                } else {
                    return broadcast_lanes(value);
                }
            }
        }

        static RAINY_CONSTEXPR26 Ty get_lane(const register_type &reg, simd_size_type i) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return reg.m128_f32[i];
                    } else if constexpr (Bits == 256) {
                        return reg.m256_f32[i];
                    } else {
                        return reg.m512_f32[i];
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return reg.m128d_f64[i];
                    } else if constexpr (Bits == 256) {
                        return reg.m256d_f64[i];
                    } else {
                        return reg.m512d_f64[i];
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    if constexpr (sizeof(Ty) == 1) {
                        return reg.m128i_i8[i];
                    } else if constexpr (sizeof(Ty) == 2) {
                        return reg.m128i_i16[i];
                    } else if constexpr (sizeof(Ty) == 4) {
                        return reg.m128i_i32[i];
                    } else {
                        return reg.m128i_i64[i];
                    }
                } else if constexpr (Bits == 256) {
                    if constexpr (sizeof(Ty) == 1) {
                        return reg.m256i_i8[i];
                    } else if constexpr (sizeof(Ty) == 2) {
                        return reg.m256i_i16[i];
                    } else if constexpr (sizeof(Ty) == 4) {
                        return reg.m256i_i32[i];
                    } else {
                        return reg.m256i_i64[i];
                    }
                } else {
                    if constexpr (sizeof(Ty) == 1) {
                        return reg.m512i_i8[i];
                    } else if constexpr (sizeof(Ty) == 2) {
                        return reg.m512i_i16[i];
                    } else if constexpr (sizeof(Ty) == 4) {
                        return reg.m512i_i32[i];
                    } else {
                        return reg.m512i_i64[i];
                    }
                }
            }
        }

        static RAINY_CONSTEXPR26 void set_lane(register_type &reg, simd_size_type i, Ty value) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        reg.m128_f32[i] = value;
                    } else if constexpr (Bits == 256) {
                        reg.m256_f32[i] = value;
                    } else {
                        reg.m512_f32[i] = value;
                    }
                } else {
                    if constexpr (Bits == 128) {
                        reg.m128d_f64[i] = value;
                    } else if constexpr (Bits == 256) {
                        reg.m256d_f64[i] = value;
                    } else {
                        reg.m512d_f64[i] = value;
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    if constexpr (sizeof(Ty) == 1) {
                        reg.m128i_i8[i] = value;
                    } else if constexpr (sizeof(Ty) == 2) {
                        reg.m128i_i16[i] = value;
                    } else if constexpr (sizeof(Ty) == 4) {
                        reg.m128i_i32[i] = value;
                    } else {
                        reg.m128i_i64[i] = value;
                    }
                } else if constexpr (Bits == 256) {
                    if constexpr (sizeof(Ty) == 1) {
                        reg.m256i_i8[i] = value;
                    } else if constexpr (sizeof(Ty) == 2) {
                        reg.m256i_i16[i] = value;
                    } else if constexpr (sizeof(Ty) == 4) {
                        reg.m256i_i32[i] = value;
                    } else {
                        reg.m256i_i64[i] = value;
                    }
                } else {
                    if constexpr (sizeof(Ty) == 1) {
                        reg.m512i_i8[i] = value;
                    } else if constexpr (sizeof(Ty) == 2) {
                        reg.m512i_i16[i] = value;
                    } else if constexpr (sizeof(Ty) == 4) {
                        reg.m512i_i32[i] = value;
                    } else {
                        reg.m512i_i64[i] = value;
                    }
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type add(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_add_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_add_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_add_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a + b); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_add_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_add_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_add_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a + b); });
                    }
                }
            } else if constexpr (Bits == 128) {
                if constexpr (sizeof(Ty) == 1) {
                    return _mm_add_epi8(left, right);
                } else if constexpr (sizeof(Ty) == 2) {
                    return _mm_add_epi16(left, right);
                } else if constexpr (sizeof(Ty) == 4) {
                    return _mm_add_epi32(left, right);
                } else if constexpr (RAINY_SIMD_SSE41) {
                    return _mm_add_epi64(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a + b); });
                }
            } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                if constexpr (sizeof(Ty) == 1) {
                    return _mm256_add_epi8(left, right);
                } else if constexpr (sizeof(Ty) == 2) {
                    return _mm256_add_epi16(left, right);
                } else if constexpr (sizeof(Ty) == 4) {
                    return _mm256_add_epi32(left, right);
                } else {
                    return _mm256_add_epi64(left, right);
                }
            } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F && sizeof(Ty) >= 4) {
                if constexpr (sizeof(Ty) == 4) {
                    return _mm512_add_epi32(left, right);
                } else {
                    return _mm512_add_epi64(left, right);
                }
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a + b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type sub(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_sub_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_sub_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_sub_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a - b); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_sub_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_sub_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_sub_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a - b); });
                    }
                }
            } else if constexpr (Bits == 128) {
                if constexpr (sizeof(Ty) == 1) {
                    return _mm_sub_epi8(left, right);
                } else if constexpr (sizeof(Ty) == 2) {
                    return _mm_sub_epi16(left, right);
                } else if constexpr (sizeof(Ty) == 4) {
                    return _mm_sub_epi32(left, right);
                } else if constexpr (RAINY_SIMD_SSE41) {
                    return _mm_sub_epi64(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a - b); });
                }
            } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                if constexpr (sizeof(Ty) == 1) {
                    return _mm256_sub_epi8(left, right);
                } else if constexpr (sizeof(Ty) == 2) {
                    return _mm256_sub_epi16(left, right);
                } else if constexpr (sizeof(Ty) == 4) {
                    return _mm256_sub_epi32(left, right);
                } else {
                    return _mm256_sub_epi64(left, right);
                }
            } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F && sizeof(Ty) >= 4) {
                if constexpr (sizeof(Ty) == 4) {
                    return _mm512_sub_epi32(left, right);
                } else {
                    return _mm512_sub_epi64(left, right);
                }
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a - b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type mul(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_mul_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_mul_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_mul_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a * b); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_mul_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_mul_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_mul_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a * b); });
                    }
                }
            } else if constexpr (sizeof(Ty) == 2) {
                if constexpr (Bits == 128) {
                    return _mm_mullo_epi16(left, right);
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return _mm256_mullo_epi16(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a * b); });
                }
            } else if constexpr (sizeof(Ty) == 4) {
                if constexpr (Bits == 128) {
                    return _mm_mullo_epi32(left, right);
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return _mm256_mullo_epi32(left, right);
                } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                    return _mm512_mullo_epi32(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a * b); });
                }
            } else if constexpr (sizeof(Ty) == 8 && Bits == 256 && RAINY_SIMD_AVX2) {
                return _mm256_mullo_epi64(left, right);
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a * b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type div(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_div_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_div_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_div_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a / b); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_div_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_div_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_div_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a / b); });
                    }
                }
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a / b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type mod(register_type left, register_type right) noexcept {
            return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a % b); });
        }

        static RAINY_CONSTEXPR26 register_type bit_and(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_and_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_and_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_and_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 0); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_and_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_and_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_and_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 0); });
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    return _mm_and_si128(left, right);
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return _mm256_and_si256(left, right);
                } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                    return _mm512_and_si512(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 0); });
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type bit_or(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_or_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_or_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_or_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 1); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_or_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_or_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_or_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 1); });
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    return _mm_or_si128(left, right);
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return _mm256_or_si256(left, right);
                } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                    return _mm512_or_si512(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 1); });
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type bit_xor(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_xor_ps(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_xor_ps(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_xor_ps(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 2); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_xor_pd(left, right);
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_xor_pd(left, right);
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_xor_pd(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 2); });
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    return _mm_xor_si128(left, right);
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return _mm256_xor_si256(left, right);
                } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                    return _mm512_xor_si512(left, right);
                } else {
                    return lane_binary(left, right, [](Ty a, Ty b) { return word_op(a, b, 2); });
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type shl(register_type left, register_type right) noexcept {
            if constexpr (RAINY_SIMD_AVX2 && sizeof(Ty) >= 2 && !type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 8) {
                    if constexpr (Bits == 128) {
                        return _mm_sllv_epi64(left, right);
                    } else if constexpr (Bits == 256) {
                        return _mm256_sllv_epi64(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a << b); });
                    }
                } else if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_sllv_epi32(left, right);
                    } else if constexpr (Bits == 256) {
                        return _mm256_sllv_epi32(left, right);
                    } else if constexpr (RAINY_SIMD_AVX512F) {
                        return _mm512_sllv_epi32(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a << b); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_sllv_epi16(left, right);
                    } else if constexpr (Bits == 256) {
                        return _mm256_sllv_epi16(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a << b); });
                    }
                }
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a << b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type shr(register_type left, register_type right) noexcept {
            if constexpr (RAINY_SIMD_AVX2 && sizeof(Ty) >= 2 && !type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 8) {
                    if constexpr (Bits == 128) {
                        return _mm_srlv_epi64(left, right);
                    } else if constexpr (Bits == 256) {
                        return _mm256_srlv_epi64(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a >> b); });
                    }
                } else if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_srlv_epi32(left, right);
                    } else if constexpr (Bits == 256) {
                        return _mm256_srlv_epi32(left, right);
                    } else if constexpr (RAINY_SIMD_AVX512F) {
                        return _mm512_srlv_epi32(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a >> b); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_srlv_epi16(left, right);
                    } else if constexpr (Bits == 256) {
                        return _mm256_srlv_epi16(left, right);
                    } else {
                        return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a >> b); });
                    }
                }
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a >> b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type shl_scalar(register_type left, simd_size_type count) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                return left;
            } else if constexpr (sizeof(Ty) == 1 || Bits == 512) {
                return lane_unary(left, [count](Ty a) { return static_cast<Ty>(a << count); });
            } else {
                auto amount = _mm_cvtsi32_si128(count);
                if constexpr (sizeof(Ty) == 8) {
                    if constexpr (Bits == 128) {
                        return _mm_sll_epi64(left, amount);
                    } else {
                        return _mm256_sll_epi64(left, amount);
                    }
                } else if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_sll_epi32(left, amount);
                    } else {
                        return _mm256_sll_epi32(left, amount);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_sll_epi16(left, amount);
                    } else {
                        return _mm256_sll_epi16(left, amount);
                    }
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type shr_scalar(register_type left, simd_size_type count) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                return left;
            } else if constexpr (sizeof(Ty) == 1 || Bits == 512) {
                return lane_unary(left, [count](Ty a) { return static_cast<Ty>(a >> count); });
            } else {
                auto amount = _mm_cvtsi32_si128(count);
                if constexpr (sizeof(Ty) == 8) {
                    if constexpr (Bits == 128) {
                        return _mm_srl_epi64(left, amount);
                    } else {
                        return _mm256_srl_epi64(left, amount);
                    }
                } else if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_srl_epi32(left, amount);
                    } else {
                        return _mm256_srl_epi32(left, amount);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_srl_epi16(left, amount);
                    } else {
                        return _mm256_srl_epi16(left, amount);
                    }
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type negate(register_type reg) noexcept {
            return sub(broadcast(static_cast<Ty>(0)), reg);
        }

        static RAINY_CONSTEXPR26 register_type bit_not(register_type reg) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return _mm_xor_ps(reg, _mm_castsi128_ps(_mm_set1_epi32(-1)));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_xor_ps(reg, _mm256_castsi256_ps(_mm256_set1_epi32(-1)));
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_xor_ps(reg, _mm512_castsi512_ps(_mm512_set1_epi32(-1)));
                    } else {
                        return lane_unary(reg, [](Ty a) { return word_not(a); });
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return _mm_xor_pd(reg, _mm_castsi128_pd(_mm_set1_epi32(-1)));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return _mm256_xor_pd(reg, _mm256_castsi256_pd(_mm256_set1_epi32(-1)));
                    } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                        return _mm512_xor_pd(reg, _mm512_castsi512_pd(_mm512_set1_epi32(-1)));
                    } else {
                        return lane_unary(reg, [](Ty a) { return word_not(a); });
                    }
                }
            } else {
                if constexpr (Bits == 128) {
                    return _mm_xor_si128(reg, _mm_set1_epi32(-1));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return _mm256_xor_si256(reg, _mm256_set1_epi32(-1));
                } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                    return _mm512_xor_si512(reg, _mm512_set1_epi32(-1));
                } else {
                    return lane_unary(reg, [](Ty a) { return word_not(a); });
                }
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_eq(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmpeq_ps(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmpeq_ps(left, right));
                    } else {
                        return lane_mask<0>(left, right);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmpeq_pd(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmpeq_pd(left, right));
                    } else {
                        return lane_mask<0>(left, right);
                    }
                }
            } else {
                return integer_eq(left, right);
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ne(register_type left, register_type right) noexcept {
            return bit_not_mask(cmp_eq(left, right));
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_lt(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmplt_ps(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmplt_ps(left, right));
                    } else {
                        return lane_mask<2>(left, right);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmplt_pd(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmplt_pd(left, right));
                    } else {
                        return lane_mask<2>(left, right);
                    }
                }
            } else if constexpr (type_traits::properties::is_signed_v<Ty> && sizeof(Ty) <= 4) {
                return integer_gt(right, left);
            } else {
                return lane_mask<2>(left, right);
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_le(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmple_ps(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmple_ps(left, right));
                    } else {
                        return lane_mask<3>(left, right);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmple_pd(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmple_pd(left, right));
                    } else {
                        return lane_mask<3>(left, right);
                    }
                }
            } else {
                return bit_not_mask(cmp_gt(left, right));
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_gt(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmpgt_ps(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmpgt_ps(left, right));
                    } else {
                        return lane_mask<4>(left, right);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmpgt_pd(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmpgt_pd(left, right));
                    } else {
                        return lane_mask<4>(left, right);
                    }
                }
            } else if constexpr (type_traits::properties::is_signed_v<Ty> && sizeof(Ty) <= 4) {
                return integer_gt(left, right);
            } else {
                return lane_mask<4>(left, right);
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ge(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmpge_ps(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmpge_ps(left, right));
                    } else {
                        return lane_mask<5>(left, right);
                    }
                } else {
                    if constexpr (Bits == 128) {
                        return bitcast<mask_register_type>(_mm_cmpge_pd(left, right));
                    } else if constexpr (Bits == 256 && RAINY_SIMD_AVX) {
                        return bitcast<mask_register_type>(_mm256_cmpge_pd(left, right));
                    } else {
                        return lane_mask<5>(left, right);
                    }
                }
            } else {
                return bit_not_mask(cmp_lt(left, right));
            }
        }

        static RAINY_CONSTEXPR26 register_type blend(const mask_register_type &mask, const register_type &on_true,
                                   const register_type &on_false) noexcept {
            if constexpr (Bits == 128) {
                auto true_bits = bitcast<mask_register_type>(on_true);
                auto false_bits = bitcast<mask_register_type>(on_false);
                return bitcast<register_type>(
                    _mm_or_si128(_mm_and_si128(mask, true_bits), _mm_andnot_si128(mask, false_bits)));
            } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                auto true_bits = bitcast<mask_register_type>(on_true);
                auto false_bits = bitcast<mask_register_type>(on_false);
                return bitcast<register_type>(
                    _mm256_or_si256(_mm256_and_si256(mask, true_bits), _mm256_andnot_si256(mask, false_bits)));
            } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                auto true_bits = bitcast<mask_register_type>(on_true);
                auto false_bits = bitcast<mask_register_type>(on_false);
                return bitcast<register_type>(
                    _mm512_or_si512(_mm512_and_si512(mask, true_bits), _mm512_andnot_si512(mask, false_bits)));
            } else {
                using int_ops = simd_ops<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>;
                register_type result{};
                for (simd_size_type i = 0; i < size; ++i) {
                    set_lane(result, i, int_ops::get_lane(mask, i) != 0 ? get_lane(on_true, i) : get_lane(on_false, i));
                }
                return result;
            }
        }

        template <typename U, typename UAbi>
        static RAINY_CONSTEXPR26 register_type convert(const register_t<U, UAbi> &src) noexcept {
            register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                set_lane(result, i, static_cast<Ty>(simd_ops<U, UAbi>::get_lane(src, i)));
            }
            return result;
        }

        static RAINY_CONSTEXPR26 register_type min_reg(register_type left, register_type right) noexcept {
            return blend(cmp_lt(right, left), right, left);
        }

        static RAINY_CONSTEXPR26 register_type max_reg(register_type left, register_type right) noexcept {
            return blend(cmp_lt(left, right), right, left);
        }

        static RAINY_CONSTEXPR26 register_type load(const Ty *ptr) noexcept {
            register_type result;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                for (simd_size_type i = 0; i < size; ++i) {
                    result.m128_f32[i] = ptr[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&result, ptr, sizeof(result));
            }
            return result;
        }

        static RAINY_CONSTEXPR26 void store(Ty *ptr, const register_type &reg) noexcept {
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                for (simd_size_type i = 0; i < size; ++i) {
                    ptr[i] = reg.m128_f32[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(ptr, &reg, sizeof(reg));
            }
        }

        template <typename To, typename From>
        static RAINY_CONSTEXPR26 To bitcast(const From &src) noexcept {
            To dst;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                const auto *src_bytes = reinterpret_cast<const unsigned char *>(&src);
                auto *dst_bytes = reinterpret_cast<unsigned char *>(&dst);
                for (std::size_t i = 0; i < sizeof(To); ++i) {
                    dst_bytes[i] = src_bytes[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&dst, &src, sizeof(To));
            }
            return dst;
        }

        using word = integer_from_t<sizeof(Ty)>;

        static RAINY_CONSTEXPR26 Ty word_op(Ty a, Ty b, int op) noexcept {
            word wa, wb;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                const auto *a_bytes = reinterpret_cast<const unsigned char *>(&a);
                auto *wa_bytes = reinterpret_cast<unsigned char *>(&wa);
                for (std::size_t i = 0; i < sizeof(wa); ++i) {
                    wa_bytes[i] = a_bytes[i];
                }
                const auto *b_bytes = reinterpret_cast<const unsigned char *>(&b);
                auto *wb_bytes = reinterpret_cast<unsigned char *>(&wb);
                for (std::size_t i = 0; i < sizeof(wb); ++i) {
                    wb_bytes[i] = b_bytes[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&wa, &a, sizeof(wa));
                core::builtin::copy_memory(&wb, &b, sizeof(wb));
            }
            word wr = op == 0 ? static_cast<word>(wa & wb) : (op == 1 ? static_cast<word>(wa | wb) : static_cast<word>(wa ^ wb));
            Ty out;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                const auto *wr_bytes = reinterpret_cast<const unsigned char *>(&wr);
                auto *out_bytes = reinterpret_cast<unsigned char *>(&out);
                for (std::size_t i = 0; i < sizeof(out); ++i) {
                    out_bytes[i] = wr_bytes[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&out, &wr, sizeof(out));
            }
            return out;
        }

        static RAINY_CONSTEXPR26 Ty word_not(Ty a) noexcept {
            word wa;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                const auto *a_bytes = reinterpret_cast<const unsigned char *>(&a);
                auto *wa_bytes = reinterpret_cast<unsigned char *>(&wa);
                for (std::size_t i = 0; i < sizeof(wa); ++i) {
                    wa_bytes[i] = a_bytes[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&wa, &a, sizeof(wa));
            }
            wa = static_cast<word>(~wa);
            Ty out;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                const auto *wa_bytes = reinterpret_cast<const unsigned char *>(&wa);
                auto *out_bytes = reinterpret_cast<unsigned char *>(&out);
                for (std::size_t i = 0; i < sizeof(out); ++i) {
                    out_bytes[i] = wa_bytes[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&out, &wa, sizeof(out));
            }
            return out;
        }

        static RAINY_CONSTEXPR26 mask_register_type bit_not_mask(mask_register_type mask) noexcept {
            if constexpr (Bits == 128) {
                return _mm_xor_si128(mask, _mm_set1_epi32(-1));
            } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                return _mm256_xor_si256(mask, _mm256_set1_epi32(-1));
            } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                return _mm512_xor_si512(mask, _mm512_set1_epi32(-1));
            } else {
                mask_register_type result{};
                for (simd_size_type i = 0; i < size; ++i) {
                    simd_ops<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>::set_lane(
                        result, i, static_cast<integer_from_t<sizeof(Ty)>>(~simd_ops<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>::get_lane(mask, i)));
                }
                return result;
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type integer_gt(const register_type &left, const register_type &right) noexcept {
            if constexpr (sizeof(Ty) == 4) {
                if constexpr (Bits == 128) {
                    return bitcast<mask_register_type>(_mm_cmpgt_epi32(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpgt_epi32(left, right));
                } else {
                    return lane_mask<4>(left, right);
                }
            } else if constexpr (sizeof(Ty) == 2) {
                if constexpr (Bits == 128) {
                    return bitcast<mask_register_type>(_mm_cmpgt_epi16(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpgt_epi16(left, right));
                } else {
                    return lane_mask<4>(left, right);
                }
            } else {
                if constexpr (Bits == 128) {
                    return bitcast<mask_register_type>(_mm_cmpgt_epi8(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpgt_epi8(left, right));
                } else {
                    return lane_mask<4>(left, right);
                }
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type integer_eq(const register_type &left, const register_type &right) noexcept {
            if constexpr (sizeof(Ty) == 8) {
                if constexpr (Bits == 128 && RAINY_SIMD_SSE41) {
                    return bitcast<mask_register_type>(_mm_cmpeq_epi64(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpeq_epi64(left, right));
                } else {
                    return lane_mask<0>(left, right);
                }
            } else if constexpr (sizeof(Ty) == 4) {
                if constexpr (Bits == 128) {
                    return bitcast<mask_register_type>(_mm_cmpeq_epi32(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpeq_epi32(left, right));
                } else if constexpr (Bits == 512 && RAINY_SIMD_AVX512F) {
                    return bitcast<mask_register_type>(_mm512_cmpeq_epi32(left, right));
                } else {
                    return lane_mask<0>(left, right);
                }
            } else if constexpr (sizeof(Ty) == 2) {
                if constexpr (Bits == 128) {
                    return bitcast<mask_register_type>(_mm_cmpeq_epi16(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpeq_epi16(left, right));
                } else {
                    return lane_mask<0>(left, right);
                }
            } else {
                if constexpr (Bits == 128) {
                    return bitcast<mask_register_type>(_mm_cmpeq_epi8(left, right));
                } else if constexpr (Bits == 256 && RAINY_SIMD_AVX2) {
                    return bitcast<mask_register_type>(_mm256_cmpeq_epi8(left, right));
                } else {
                    return lane_mask<0>(left, right);
                }
            }
        }

        template <typename Fn>
        static RAINY_CONSTEXPR26 register_type lane_binary(const register_type &left, const register_type &right, Fn fn) noexcept {
            register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                set_lane(result, i, fn(get_lane(left, i), get_lane(right, i)));
            }
            return result;
        }

        template <typename Fn>
        static RAINY_CONSTEXPR26 register_type lane_unary(const register_type &src, Fn fn) noexcept {
            register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                set_lane(result, i, fn(get_lane(src, i)));
            }
            return result;
        }

        template <int Op>
        static RAINY_CONSTEXPR26 mask_register_type lane_mask(const register_type &left, const register_type &right) noexcept {
            mask_register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                bool r = false;
                if constexpr (Op == 0) {
                    r = get_lane(left, i) == get_lane(right, i);
                } else if constexpr (Op == 1) {
                    r = get_lane(left, i) != get_lane(right, i);
                } else if constexpr (Op == 2) {
                    r = get_lane(left, i) < get_lane(right, i);
                } else if constexpr (Op == 3) {
                    r = get_lane(left, i) <= get_lane(right, i);
                } else if constexpr (Op == 4) {
                    r = get_lane(left, i) > get_lane(right, i);
                } else {
                    r = get_lane(left, i) >= get_lane(right, i);
                }
                simd_ops<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>::set_lane(
                    result, i, static_cast<integer_from_t<sizeof(Ty)>>(r ? -1 : 0));
            }
            return result;
        }

        static RAINY_CONSTEXPR26 register_type broadcast_lanes(Ty value) noexcept {
            register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                set_lane(result, i, value);
            }
            return result;
        }
    };
#else
    template <typename Ty, std::size_t Bits>
    struct simd_ops<Ty, vector_abi<Bits>> {
        using register_type = typename simd_register<Ty, vector_abi<Bits>>::type;
        using mask_register_type = register_t<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>;
        static constexpr simd_size_type size = simd_size_v<Ty, vector_abi<Bits>>;

        static RAINY_CONSTEXPR26 register_type broadcast(Ty value) noexcept {
            return broadcast_impl(value, type_traits::helper::make_index_sequence<static_cast<std::size_t>(size)>{});
        }

        static RAINY_CONSTEXPR26 Ty get_lane(const register_type &reg, simd_size_type i) noexcept {
            return reg[i];
        }

        static RAINY_CONSTEXPR26 void set_lane(register_type &reg, simd_size_type i, Ty value) noexcept {
            reg[i] = value;
        }

        static RAINY_CONSTEXPR26 register_type add(register_type left, register_type right) noexcept {
            return left + right;
        }

        static RAINY_CONSTEXPR26 register_type sub(register_type left, register_type right) noexcept {
            return left - right;
        }

        static RAINY_CONSTEXPR26 register_type mul(register_type left, register_type right) noexcept {
            return left * right;
        }

        static RAINY_CONSTEXPR26 register_type div(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                return left / right;
            } else {
                register_type result{};
                for (simd_size_type i = 0; i < size; ++i) {
                    result[i] = static_cast<Ty>(left[i] / right[i]);
                }
                return result;
            }
        }

        static RAINY_CONSTEXPR26 register_type mod(register_type left, register_type right) noexcept {
            register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                result[i] = static_cast<Ty>(left[i] % right[i]);
            }
            return result;
        }

        static RAINY_CONSTEXPR26 register_type bit_and(register_type left, register_type right) noexcept {
            return left & right;
        }

        static RAINY_CONSTEXPR26 register_type bit_or(register_type left, register_type right) noexcept {
            return left | right;
        }

        static RAINY_CONSTEXPR26 register_type bit_xor(register_type left, register_type right) noexcept {
            return left ^ right;
        }

        static RAINY_CONSTEXPR26 register_type shl(register_type left, register_type right) noexcept {
            return left << right;
        }

        static RAINY_CONSTEXPR26 register_type shr(register_type left, register_type right) noexcept {
            return left >> right;
        }

        static RAINY_CONSTEXPR26 register_type shl_scalar(register_type left, simd_size_type count) noexcept {
            return left << count;
        }

        static RAINY_CONSTEXPR26 register_type shr_scalar(register_type left, simd_size_type count) noexcept {
            return left >> count;
        }

        static RAINY_CONSTEXPR26 register_type negate(register_type reg) noexcept {
            return -reg;
        }

        static RAINY_CONSTEXPR26 register_type bit_not(register_type reg) noexcept {
            return ~reg;
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_eq(register_type left, register_type right) noexcept {
            return bitcast<mask_register_type>(left == right);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ne(register_type left, register_type right) noexcept {
            return bitcast<mask_register_type>(left != right);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_lt(register_type left, register_type right) noexcept {
            return bitcast<mask_register_type>(left < right);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_le(register_type left, register_type right) noexcept {
            return bitcast<mask_register_type>(left <= right);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_gt(register_type left, register_type right) noexcept {
            return bitcast<mask_register_type>(left > right);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ge(register_type left, register_type right) noexcept {
            return bitcast<mask_register_type>(left >= right);
        }

        static RAINY_CONSTEXPR26 register_type blend(const mask_register_type &mask, const register_type &on_true,
                                   const register_type &on_false) noexcept {
            auto true_bits = bitcast<mask_register_type>(on_true);
            auto false_bits = bitcast<mask_register_type>(on_false);
            return bitcast<register_type>((mask & true_bits) | (~mask & false_bits));
        }

        template <typename U, typename UAbi>
        static RAINY_CONSTEXPR26 register_type convert(const register_t<U, UAbi> &src) noexcept {
            register_type result{};
            for (simd_size_type i = 0; i < size; ++i) {
                result[i] = static_cast<Ty>(simd_ops<U, UAbi>::get_lane(src, i));
            }
            return result;
        }

        static RAINY_CONSTEXPR26 register_type min_reg(register_type left, register_type right) noexcept {
            return blend(cmp_lt(right, left), right, left);
        }

        static RAINY_CONSTEXPR26 register_type max_reg(register_type left, register_type right) noexcept {
            return blend(cmp_lt(left, right), right, left);
        }

        static RAINY_CONSTEXPR26 register_type load(const Ty *ptr) noexcept {
            register_type result;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                for (simd_size_type i = 0; i < size; ++i) {
                    result[i] = ptr[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&result, ptr, sizeof(result));
            }
            return result;
        }

        static RAINY_CONSTEXPR26 void store(Ty *ptr, const register_type &reg) noexcept {
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                for (simd_size_type i = 0; i < size; ++i) {
                    ptr[i] = reg[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(ptr, &reg, sizeof(reg));
            }
        }

        template <typename To, typename From>
        static RAINY_CONSTEXPR26 To bitcast(const From &src) noexcept {
            To dst;
#if RAINY_HAS_CXX26
            if (std::is_constant_evaluated()) {
                const auto *src_bytes = reinterpret_cast<const unsigned char *>(&src);
                auto *dst_bytes = reinterpret_cast<unsigned char *>(&dst);
                for (std::size_t i = 0; i < sizeof(To); ++i) {
                    dst_bytes[i] = src_bytes[i];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&dst, &src, sizeof(To));
            }
            return dst;
        }

        template <std::size_t... I>
        static RAINY_CONSTEXPR26 register_type broadcast_impl(Ty value, type_traits::helper::index_sequence<I...>) noexcept {
            return register_type{((void)I, value)...};
        }
    };
#endif
}

#endif