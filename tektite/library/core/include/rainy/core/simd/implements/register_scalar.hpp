#ifndef RAINY_CORE_SIMD_IMPLEMENTS_REGISTER_SCALAR_HPP
#define RAINY_CORE_SIMD_IMPLEMENTS_REGISTER_SCALAR_HPP

#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/simd/native_abi.hpp>

namespace rainy::core::implements {
    template <typename Ty, typename Abi>
    struct simd_register;

    template <typename Ty>
    struct simd_register<Ty, scalar_abi> {
        using type = Ty;
    };

    template <typename Ty, typename Abi>
    using register_t = typename simd_register<Ty, Abi>::type;

    template <typename Ty, typename Abi>
    struct simd_ops {
        using register_type = typename simd_register<Ty, Abi>::type;
        using mask_register_type = register_t<integer_from_t<sizeof(Ty)>, Abi>;
        static constexpr simd_size_type size = simd_size_v<Ty, Abi>;

        static RAINY_CONSTEXPR26 register_type broadcast(Ty value) noexcept {
            return value;
        }

        static RAINY_CONSTEXPR26 Ty get_lane(const register_type &reg, simd_size_type) noexcept {
            return reg;
        }

        static RAINY_CONSTEXPR26 void set_lane(register_type &reg, simd_size_type, Ty value) noexcept {
            reg = value;
        }

        static RAINY_CONSTEXPR26 register_type add(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left + right);
        }

        static RAINY_CONSTEXPR26 register_type sub(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left - right);
        }

        static RAINY_CONSTEXPR26 register_type mul(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left * right);
        }

        static RAINY_CONSTEXPR26 register_type div(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left / right);
        }

        static RAINY_CONSTEXPR26 register_type mod(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left % right);
        }

        static RAINY_CONSTEXPR26 register_type bit_and(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left & right);
        }

        static RAINY_CONSTEXPR26 register_type bit_or(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left | right);
        }

        static RAINY_CONSTEXPR26 register_type bit_xor(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left ^ right);
        }

        static RAINY_CONSTEXPR26 register_type shl(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left << right);
        }

        static RAINY_CONSTEXPR26 register_type shr(register_type left, register_type right) noexcept {
            return static_cast<register_type>(left >> right);
        }

        static RAINY_CONSTEXPR26 register_type shl_scalar(register_type left, simd_size_type count) noexcept {
            return static_cast<register_type>(left << count);
        }

        static RAINY_CONSTEXPR26 register_type shr_scalar(register_type left, simd_size_type count) noexcept {
            return static_cast<register_type>(left >> count);
        }

        static RAINY_CONSTEXPR26 register_type negate(register_type reg) noexcept {
            return static_cast<register_type>(-reg);
        }

        static RAINY_CONSTEXPR26 register_type bit_not(register_type reg) noexcept {
            return static_cast<register_type>(~reg);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_eq(register_type left, register_type right) noexcept {
            return left == right ? static_cast<mask_register_type>(-1) : static_cast<mask_register_type>(0);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ne(register_type left, register_type right) noexcept {
            return left != right ? static_cast<mask_register_type>(-1) : static_cast<mask_register_type>(0);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_lt(register_type left, register_type right) noexcept {
            return left < right ? static_cast<mask_register_type>(-1) : static_cast<mask_register_type>(0);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_le(register_type left, register_type right) noexcept {
            return left <= right ? static_cast<mask_register_type>(-1) : static_cast<mask_register_type>(0);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_gt(register_type left, register_type right) noexcept {
            return left > right ? static_cast<mask_register_type>(-1) : static_cast<mask_register_type>(0);
        }

        static RAINY_CONSTEXPR26 mask_register_type cmp_ge(register_type left, register_type right) noexcept {
            return left >= right ? static_cast<mask_register_type>(-1) : static_cast<mask_register_type>(0);
        }

        static RAINY_CONSTEXPR26 register_type blend(const mask_register_type &mask, const register_type &on_true,
                                   const register_type &on_false) noexcept {
            return mask != 0 ? on_true : on_false;
        }

        template <typename U, typename UAbi>
        static RAINY_CONSTEXPR26 register_type convert(const register_t<U, UAbi> &src) noexcept {
            return static_cast<register_type>(simd_ops<U, UAbi>::get_lane(src, 0));
        }

        static RAINY_CONSTEXPR26 register_type min_reg(register_type left, register_type right) noexcept {
            return right < left ? right : left;
        }

        static RAINY_CONSTEXPR26 register_type max_reg(register_type left, register_type right) noexcept {
            return left < right ? right : left;
        }

        static RAINY_CONSTEXPR26 register_type load(const Ty *ptr) noexcept {
            return *ptr;
        }

        static RAINY_CONSTEXPR26 void store(Ty *ptr, const register_type &reg) noexcept {
            *ptr = reg;
        }
    };
}

#endif
