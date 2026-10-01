#ifndef RAINY_CORE_SIMD_REGISTER_ARM64_HPP
#define RAINY_CORE_SIMD_REGISTER_ARM64_HPP

#include <cstring>
#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/simd/implements/register_scalar.hpp>
#include <rainy/core/simd/native_abi.hpp>

#if defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#endif

namespace rainy::core::implements {
#if RAINY_USING_MSVC
    template <typename Ty, std::size_t Bits>
    struct simd_register<Ty, vector_abi<Bits>> {
        using type = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_floating_point_v<Ty>,
            type_traits::other_trans::conditional_t<sizeof(Ty) == 4, float32x4_t, float64x2_t>,
            type_traits::other_trans::conditional_t<
                sizeof(Ty) == 1, int8x16_t,
                type_traits::other_trans::conditional_t<
                    sizeof(Ty) == 2, int16x8_t, type_traits::other_trans::conditional_t<sizeof(Ty) == 4, int32x4_t, int64x2_t>>>>;
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
        static_assert(Bits == 128, "NEON backend provides 128-bit registers only");

        using register_type = typename simd_register<Ty, vector_abi<Bits>>::type;
        using mask_register_type = register_t<integer_from_t<sizeof(Ty)>, vector_abi<Bits>>;
        static constexpr simd_size_type size = simd_size_v<Ty, vector_abi<Bits>>;

        static RAINY_CONSTEXPR26 register_type broadcast(Ty value) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return vdupq_n_f32(value);
                } else {
                    return vdupq_n_f64(value);
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return vdupq_n_s8(static_cast<signed char>(value));
                } else if constexpr (sizeof(Ty) == 2) {
                    return vdupq_n_s16(static_cast<short>(value));
                } else if constexpr (sizeof(Ty) == 4) {
                    return vdupq_n_s32(static_cast<int>(value));
                } else {
                    return vdupq_n_s64(static_cast<__int64>(value));
                }
            }
        }

        static RAINY_CONSTEXPR26 Ty get_lane(const register_type &reg, simd_size_type i) noexcept {
            Ty tmp[size];
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                const auto *reg_bytes = reinterpret_cast<const unsigned char *>(&reg);
                auto *tmp_bytes = reinterpret_cast<unsigned char *>(tmp);
                for (std::size_t k = 0; k < sizeof(tmp); ++k) {
                    tmp_bytes[k] = reg_bytes[k];
                }
            } else
#endif
            {
                core::builtin::copy_memory(tmp, &reg, sizeof(tmp));
            }
            return tmp[i];
        }

        static RAINY_CONSTEXPR26 void set_lane(register_type &reg, simd_size_type i, Ty value) noexcept {
            Ty tmp[size];
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                const auto *reg_bytes = reinterpret_cast<const unsigned char *>(&reg);
                auto *tmp_bytes = reinterpret_cast<unsigned char *>(tmp);
                for (std::size_t k = 0; k < sizeof(tmp); ++k) {
                    tmp_bytes[k] = reg_bytes[k];
                }
            } else
#endif
            {
                core::builtin::copy_memory(tmp, &reg, sizeof(tmp));
            }
            tmp[i] = value;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                const auto *tmp_bytes = reinterpret_cast<const unsigned char *>(tmp);
                auto *reg_bytes = reinterpret_cast<unsigned char *>(&reg);
                for (std::size_t k = 0; k < sizeof(tmp); ++k) {
                    reg_bytes[k] = tmp_bytes[k];
                }
            } else
#endif
            {
                core::builtin::copy_memory(&reg, tmp, sizeof(tmp));
            }
        }

        static RAINY_CONSTEXPR26 register_type add(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return vaddq_f32(left, right);
                } else {
                    return vaddq_f64(left, right);
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return vaddq_s8(left, right);
                } else if constexpr (sizeof(Ty) == 2) {
                    return vaddq_s16(left, right);
                } else if constexpr (sizeof(Ty) == 4) {
                    return vaddq_s32(left, right);
                } else {
                    return vaddq_s64(left, right);
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type sub(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return vsubq_f32(left, right);
                } else {
                    return vsubq_f64(left, right);
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return vsubq_s8(left, right);
                } else if constexpr (sizeof(Ty) == 2) {
                    return vsubq_s16(left, right);
                } else if constexpr (sizeof(Ty) == 4) {
                    return vsubq_s32(left, right);
                } else {
                    return vsubq_s64(left, right);
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type mul(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return vmulq_f32(left, right);
                } else {
                    return vmulq_f64(left, right);
                }
            } else if constexpr (sizeof(Ty) == 2) {
                return vmulq_s16(left, right);
            } else if constexpr (sizeof(Ty) == 4) {
                return vmulq_s32(left, right);
            } else {
                return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a * b); });
            }
        }

        static RAINY_CONSTEXPR26 register_type div(register_type left, register_type right) noexcept {
            return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a / b); });
        }

        static RAINY_CONSTEXPR26 register_type mod(register_type left, register_type right) noexcept {
            return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a % b); });
        }

        static RAINY_CONSTEXPR26 register_type bit_and(register_type left, register_type right) noexcept {
            return bitcast<register_type>(vandq_u8(bitcast<uint8x16_t>(left), bitcast<uint8x16_t>(right)));
        }

        static RAINY_CONSTEXPR26 register_type bit_or(register_type left, register_type right) noexcept {
            return bitcast<register_type>(vorrq_u8(bitcast<uint8x16_t>(left), bitcast<uint8x16_t>(right)));
        }

        static RAINY_CONSTEXPR26 register_type bit_xor(register_type left, register_type right) noexcept {
            return bitcast<register_type>(veorq_u8(bitcast<uint8x16_t>(left), bitcast<uint8x16_t>(right)));
        }

        static RAINY_CONSTEXPR26 register_type shl(register_type left, register_type right) noexcept {
            return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a << b); });
        }

        static RAINY_CONSTEXPR26 register_type shr(register_type left, register_type right) noexcept {
            return lane_binary(left, right, [](Ty a, Ty b) { return static_cast<Ty>(a >> b); });
        }

        static RAINY_CONSTEXPR26 register_type shl_scalar(register_type left, simd_size_type count) noexcept {
            if constexpr (type_traits::properties::is_signed_v<Ty>) {
                if constexpr (sizeof(Ty) == 1) {
                    return vshlq_s8(left, vdupq_n_s8(static_cast<signed char>(count)));
                } else if constexpr (sizeof(Ty) == 2) {
                    return vshlq_s16(left, vdupq_n_s16(static_cast<short>(count)));
                } else if constexpr (sizeof(Ty) == 4) {
                    return vshlq_s32(left, vdupq_n_s32(static_cast<int>(count)));
                } else {
                    return vshlq_s64(left, vdupq_n_s64(static_cast<__int64>(count)));
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<register_type>(vshlq_u8(bitcast<uint8x16_t>(left), vdupq_n_s8(static_cast<signed char>(count))));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<register_type>(vshlq_u16(bitcast<uint16x8_t>(left), vdupq_n_s16(static_cast<short>(count))));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<register_type>(vshlq_u32(bitcast<uint32x4_t>(left), vdupq_n_s32(static_cast<int>(count))));
                } else {
                    return bitcast<register_type>(vshlq_u64(bitcast<uint64x2_t>(left), vdupq_n_s64(static_cast<__int64>(count))));
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type shr_scalar(register_type left, simd_size_type count) noexcept {
            auto amount = -count;
            if constexpr (type_traits::properties::is_signed_v<Ty>) {
                if constexpr (sizeof(Ty) == 1) {
                    return vshlq_s8(left, vdupq_n_s8(static_cast<signed char>(amount)));
                } else if constexpr (sizeof(Ty) == 2) {
                    return vshlq_s16(left, vdupq_n_s16(static_cast<short>(amount)));
                } else if constexpr (sizeof(Ty) == 4) {
                    return vshlq_s32(left, vdupq_n_s32(static_cast<int>(amount)));
                } else {
                    return vshlq_s64(left, vdupq_n_s64(static_cast<__int64>(amount)));
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<register_type>(vshlq_u8(bitcast<uint8x16_t>(left), vdupq_n_s8(static_cast<signed char>(amount))));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<register_type>(vshlq_u16(bitcast<uint16x8_t>(left), vdupq_n_s16(static_cast<short>(amount))));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<register_type>(vshlq_u32(bitcast<uint32x4_t>(left), vdupq_n_s32(static_cast<int>(amount))));
                } else {
                    return bitcast<register_type>(vshlq_u64(bitcast<uint64x2_t>(left), vdupq_n_s64(static_cast<__int64>(amount))));
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type negate(register_type reg) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return vnegq_f32(reg);
                } else {
                    return vnegq_f64(reg);
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return vnegq_s8(reg);
                } else if constexpr (sizeof(Ty) == 2) {
                    return vnegq_s16(reg);
                } else if constexpr (sizeof(Ty) == 4) {
                    return vnegq_s32(reg);
                } else {
                    return vnegq_s64(reg);
                }
            }
        }

        static RAINY_CONSTEXPR26 register_type bit_not(register_type reg) noexcept {
            return bitcast<register_type>(veorq_u8(bitcast<uint8x16_t>(reg), vdupq_n_u8(0xFF)));
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_eq(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vceqq_f32(left, right));
                } else {
                    return bitcast<mask_register_type>(vceqq_f64(left, right));
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<mask_register_type>(vceqq_s8(left, right));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<mask_register_type>(vceqq_s16(left, right));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vceqq_s32(left, right));
                } else {
                    return bitcast<mask_register_type>(vceqq_s64(left, right));
                }
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ne(register_type left, register_type right) noexcept {
            return bit_not_mask(cmp_eq(left, right));
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_lt(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcltq_f32(left, right));
                } else {
                    return bitcast<mask_register_type>(vcltq_f64(left, right));
                }
            } else {
                return integer_lt(left, right);
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_le(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcleq_f32(left, right));
                } else {
                    return bitcast<mask_register_type>(vcleq_f64(left, right));
                }
            } else {
                return bit_not_mask(cmp_gt(left, right));
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_gt(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcgtq_f32(left, right));
                } else {
                    return bitcast<mask_register_type>(vcgtq_f64(left, right));
                }
            } else {
                return integer_gt(left, right);
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ge(register_type left, register_type right) noexcept {
            if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
                if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcgeq_f32(left, right));
                } else {
                    return bitcast<mask_register_type>(vcgeq_f64(left, right));
                }
            } else {
                return bit_not_mask(cmp_lt(left, right));
            }
        }

        static RAINY_CONSTEXPR26 register_type blend(const mask_register_type &mask, const register_type &on_true,
                                                     const register_type &on_false) noexcept {
            return bitcast<register_type>(
                vbslq_u8(bitcast<uint8x16_t>(mask), bitcast<uint8x16_t>(on_true), bitcast<uint8x16_t>(on_false)));
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
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                register_type result{};
                for (simd_size_type i = 0; i < size; ++i) {
                    set_lane(result, i, ptr[i]);
                }
                return result;
            } else
#endif
            {
                return bitcast<register_type>(vld1q_u8(reinterpret_cast<const unsigned char *>(ptr)));
            }
        }

        static RAINY_CONSTEXPR26 void store(Ty *ptr, const register_type &reg) noexcept {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (simd_size_type i = 0; i < size; ++i) {
                    ptr[i] = get_lane(reg, i);
                }
            } else
#endif
            {
                vst1q_u8(reinterpret_cast<unsigned char *>(ptr), bitcast<uint8x16_t>(reg));
            }
        }

        template <typename To, typename From>
        static RAINY_CONSTEXPR26 To bitcast(const From &src) noexcept {
            To dst;
#if RAINY_HAS_CXX20
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

        static RAINY_CONSTEXPR26 mask_register_type bit_not_mask(mask_register_type mask) noexcept {
            return bitcast<mask_register_type>(veorq_u8(bitcast<uint8x16_t>(mask), vdupq_n_u8(0xFF)));
        }

        static RAINY_CONSTEXPR26 mask_register_type integer_gt(const register_type &left, const register_type &right) noexcept {
            if constexpr (type_traits::properties::is_signed_v<Ty>) {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<mask_register_type>(vcgtq_s8(left, right));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<mask_register_type>(vcgtq_s16(left, right));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcgtq_s32(left, right));
                } else {
                    return bitcast<mask_register_type>(vcgtq_s64(left, right));
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<mask_register_type>(vcgtq_u8(bitcast<uint8x16_t>(left), bitcast<uint8x16_t>(right)));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<mask_register_type>(vcgtq_u16(bitcast<uint16x8_t>(left), bitcast<uint16x8_t>(right)));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcgtq_u32(bitcast<uint32x4_t>(left), bitcast<uint32x4_t>(right)));
                } else {
                    return bitcast<mask_register_type>(vcgtq_u64(bitcast<uint64x2_t>(left), bitcast<uint64x2_t>(right)));
                }
            }
        }

        static RAINY_CONSTEXPR26 mask_register_type integer_lt(const register_type &left, const register_type &right) noexcept {
            if constexpr (type_traits::properties::is_signed_v<Ty>) {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<mask_register_type>(vcltq_s8(left, right));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<mask_register_type>(vcltq_s16(left, right));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcltq_s32(left, right));
                } else {
                    return bitcast<mask_register_type>(vcltq_s64(left, right));
                }
            } else {
                if constexpr (sizeof(Ty) == 1) {
                    return bitcast<mask_register_type>(vcltq_u8(bitcast<uint8x16_t>(left), bitcast<uint8x16_t>(right)));
                } else if constexpr (sizeof(Ty) == 2) {
                    return bitcast<mask_register_type>(vcltq_u16(bitcast<uint16x8_t>(left), bitcast<uint16x8_t>(right)));
                } else if constexpr (sizeof(Ty) == 4) {
                    return bitcast<mask_register_type>(vcltq_u32(bitcast<uint32x4_t>(left), bitcast<uint32x4_t>(right)));
                } else {
                    return bitcast<mask_register_type>(vcltq_u64(bitcast<uint64x2_t>(left), bitcast<uint64x2_t>(right)));
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
#if RAINY_HAS_CXX20
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
#if RAINY_HAS_CXX20
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
#if RAINY_HAS_CXX20
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
            return register_type{((void) I, value)...};
        }
    };
#endif
}

#endif
