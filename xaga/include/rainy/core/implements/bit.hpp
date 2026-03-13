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
/**
 * @mergeto rainy/core/core.hpp
 */
#ifndef RAINY_CORE_IMPLEMENTS_BIT_HPP
#define RAINY_CORE_IMPLEMENTS_BIT_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>

namespace rainy::core::builtin {
    /**
     * @brief Counts the number of 1 bits in an unsigned integer.
     *        计算无符号整数中1的位数。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The value to count bits for
     *              要计算位数的值
     * @return Number of 1 bits
     *         1的位数
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn popcount(const Type value) noexcept -> int {
        return value ? (int(value & 1) + popcount(static_cast<Type>(value >> 1))) : 0;
    }

    /**
     * @brief Checks if a value has exactly one 1 bit (is a power of two).
     *        检查一个值是否恰好有一个1位（是否为2的幂）。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The value to check
     *              要检查的值
     * @return true if the value is a power of two, false otherwise
     *         如果值是2的幂则为true，否则为false
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn has_single_bit(const Type value) noexcept -> bool {
        return value && ((value & (value - 1)) == 0);
    }

    /**
     * @brief Returns the next power of two greater than or equal to the input.
     *        返回大于或等于输入值的下一个2的幂。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The input value
     *              输入值
     * @return The smallest power of two that is >= value
     *         大于等于value的最小2的幂
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn next_power_of_two(const Type value) noexcept -> Type {
        assert(value < (Type{1u} << (utility::numeric_limits<Type>::digits - 1)) && "Numeric limits has exceeded");
        Type curr = value - (value != 0u);
        for (int next = 1; next < utility::numeric_limits<Type>::digits; next = next * 2) {
            curr |= (curr >> next);
        }
        return ++curr;
    }

    /**
     * @brief Returns the number of bits needed to represent the value.
     *        返回表示该值所需的位数。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The input value
     *              输入值
     * @return The bit width (position of the highest 1 bit + 1, or 0 for 0)
     *         位宽（最高1位的位置+1，0返回0）
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn bit_width(const Type value) noexcept -> int {
        int width = 0;
        Type v = value;
        while (v != 0) {
            ++width;
            v >>= 1;
        }
        return width;
    }

    /**
     * @brief Returns the largest power of two not greater than the value.
     *        返回不大于该值的最大2的幂。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The input value
     *              输入值
     * @return The largest power of two <= value, or 0 if value is 0
     *         不大于value的最大2的幂，如果value为0则返回0
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn bit_floor(const Type value) noexcept -> Type {
        if (value == 0)
            return 0;
        return Type{1} << (bit_width(value) - 1);
    }

    /**
     * @brief Counts the number of consecutive 0 bits starting from the most significant bit.
     *        从最高有效位开始计算连续0位的数量。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The input value
     *              输入值
     * @return Number of leading zero bits
     *         前导零位的数量
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn countl_zero(Type value) noexcept -> int {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        int count = 0;
        for (int i = bits - 1; i >= 0; --i) {
            if ((value >> i) & 1u) {
                break;
            }
            ++count;
        }
        return count;
    }

    /**
     * @brief Counts the number of consecutive 0 bits starting from the least significant bit.
     *        从最低有效位开始计算连续0位的数量。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The input value
     *              输入值
     * @return Number of trailing zero bits
     *         尾随零位的数量
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn countr_zero(Type value) noexcept -> int {
        if (value == 0) {
            return utility::numeric_limits<Type>::digits;
        }
        int count = 0;
        while ((value & 1u) == 0) {
            value >>= 1;
            ++count;
        }
        return count;
    }

    /**
     * @brief Computes value % mod when mod is a power of two.
     *        当mod是2的幂时计算value % mod。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The value to compute modulo for
     *              要计算模的值
     * @param mod The modulus (must be a power of two)
     *            模数（必须是2的幂）
     * @return value % mod
     *         value % mod的结果
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn mod(const Type value, const std::size_t mod) noexcept -> Type {
        assert(has_single_bit(mod) && "Value must be a power of two");
        return static_cast<Type>(value & (mod - 1u));
    }

    /**
     * @brief Rotates bits to the left.
     *        向左旋转位。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The value to rotate
     *              要旋转的值
     * @param shift Number of positions to rotate
     *              旋转的位置数
     * @return Rotated value
     *         旋转后的值
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn rotate_left(Type value, int shift) noexcept -> Type {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        shift %= bits;
        return (value << shift) | (value >> (bits - shift));
    }

    /**
     * @brief Rotates bits to the right.
     *        向右旋转位。
     *
     * @tparam Type Unsigned integer type
     *              无符号整数类型
     * @param value The value to rotate
     *              要旋转的值
     * @param shift Number of positions to rotate
     *              旋转的位置数
     * @return Rotated value
     *         旋转后的值
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn rotate_right(Type value, int shift) noexcept -> Type {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        shift %= bits;
        return (value >> shift) | (value << (bits - shift));
    }
}

#endif