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
 * \lang english
 * @mergeto rainy/core/core.hpp
 */
#ifndef RAINY_CORE_UTILITY_BIT_HPP
#define RAINY_CORE_UTILITY_BIT_HPP
#include <bit>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/properties.hpp>

namespace rainy::utility {
    /**
     * \lang english
     * @brief Counts the number of 1 bit in an unsigned integer.
     *
     * @tparam Type Unsigned integer type
     * @param value The value to count bits for
     * @return Number of 1 bit
     *
     * \lang simp-chinese
     * @brief 计算无符号整数中1的位数。
     *
     * @tparam Type 无符号整数类型
     * @param value 要计算位数的值
     * @return 1的位数
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn popcount(const Type value) noexcept -> int {
        return value ? (static_cast<int>(value & 1) + popcount(static_cast<Type>(value >> 1))) : 0;
    }

    /**
     * \lang english
     * @brief Checks if a value has exactly one 1 bit (is a power of two).
     *
     * @tparam Type Unsigned integer type
     * @param value The value to check
     * @return true if the value is a power of two, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查一个值是否恰好有一个1位（是否为2的幂）。
     *
     * @tparam Type 无符号整数类型
     * @param value 要检查的值
     * @return 如果值是2的幂则为true，否则为false
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn has_single_bit(const Type value) noexcept -> bool { // NOLINT
        return value && ((value & (value - 1)) == 0);
    }

    /**
     * \lang english
     * @brief Returns the next power of two greater than or equal to the input.
     *
     * @tparam Type Unsigned integer type
     * @param value The input value
     * @return The smallest power of two that is >= value
     *
     * \lang simp-chinese
     * @brief 返回大于或等于输入值的下一个2的幂。
     *
     * @tparam Type 无符号整数类型
     * @param value 输入值
     * @return 大于等于value的最小2的幂
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn next_power_of_two(const Type value) noexcept -> Type {
        assert(value < (Type{1u} << (utility::numeric_limits<Type>::digits - 1)) && "Numeric limits has exceeded");
        Type curr = value - (value != 0u);
        for (int next = 1; next < utility::numeric_limits<Type>::digits; next = next * 2) {
            curr |= (curr >> next);
        }
        return ++curr;
    }

    /**
     * \lang english
     * @brief Returns the number of bits needed to represent the value.
     *
     * @tparam Type Unsigned integer type
     * @param value The input value
     * @return The bit width (position of the highest 1 bit + 1, or 0 for 0)
     *
     * \lang simp-chinese
     * @brief 返回表示该值所需的位数。
     *
     * @tparam Type 无符号整数类型
     * @param value 输入值
     * @return 位宽（最高1位的位置+1，0返回0）
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
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
     * \lang english
     * @brief Returns the largest power of two not greater than the value.
     *
     * @tparam Type Unsigned integer type
     * @param value The input value
     * @return The largest power of two <= value, or 0 if value is 0
     *
     * \lang simp-chinese
     * @brief 返回不大于该值的最大2的幂。
     *
     * @tparam Type 无符号整数类型
     * @param value 输入值
     * @return 不大于value的最大2的幂，如果value为0则返回0
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn bit_floor(const Type value) noexcept -> Type {
        if (value == 0)
            return 0;
        return Type{1} << (bit_width(value) - 1);
    }

    /**
     * \lang english
     * @brief Counts the number of consecutive 0 bits starting from the most significant bit.
     *
     * @tparam Type Unsigned integer type
     * @param value The input value
     * @return Number of leading zero bits
     *
     * \lang simp-chinese
     * @brief 从最高有效位开始计算连续0位的数量。
     *
     * @tparam Type 无符号整数类型
     * @param value 输入值
     * @return 前导零位的数量
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
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
     * \lang english
     * @brief Counts the number of consecutive 0 bits starting from the least significant bit.
     *
     * @tparam Type Unsigned integer type
     * @param value The input value
     * @return Number of trailing zero bits
     *
     * \lang simp-chinese
     * @brief 从最低有效位开始计算连续0位的数量。
     *
     * @tparam Type 无符号整数类型
     * @param value 输入值
     * @return 尾随零位的数量
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
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
     * \lang english
     * @brief Computes value % mod when mod is a power of two.
     *
     * @tparam Type Unsigned integer type
     * @param value The value to compute modulo for
     * @param mod The modulus (must be a power of two)
     * @return value % mod
     *
     * \lang simp-chinese
     * @brief 当mod是2的幂时计算value % mod。
     *
     * @tparam Type 无符号整数类型
     * @param value 要计算模的值
     * @param mod 模数（必须是2的幂）
     * @return value % mod的结果
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn mod(const Type value, const std::size_t mod) noexcept -> Type {
        assert(has_single_bit(mod) && "Value must be a power of two");
        return static_cast<Type>(value & (mod - 1u));
    }

    /**
     * \lang english
     * @brief Rotates bits to the left.
     *
     * @tparam Type Unsigned integer type
     * @param value The value to rotate
     * @param shift Number of positions to rotate
     * @return Rotated value
     *
     * \lang simp-chinese
     * @brief 向左旋转位。
     *
     * @tparam Type 无符号整数类型
     * @param value 要旋转的值
     * @param shift 旋转的位置数
     * @return 旋转后的值
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn rotate_left(Type value, int shift) noexcept -> Type {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        shift %= bits;
        return (value << shift) | (value >> (bits - shift));
    }

    /**
     * \lang english
     * @brief Rotates bits to the right.
     *
     * @tparam Type Unsigned integer type
     * @param value The value to rotate
     * @param shift Number of positions to rotate
     * @return Rotated value
     *
     * \lang simp-chinese
     * @brief 向右旋转位。
     *
     * @tparam Type 无符号整数类型
     * @param value 要旋转的值
     * @param shift 旋转的位置数
     * @return 旋转后的值
     */
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn rotate_right(Type value, int shift) noexcept -> Type {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        shift %= bits;
        return (value >> shift) | (value << (bits - shift));
    }

    /**
     * \lang english
     * @brief Reinterpret the object representation of one type as another type.
     *
     * @details This function forwards to std::bit_cast when C++20 is available.
     *           It provides a safe way to type-pun between types of the same size
     *           without violating strict aliasing rules.
     *
     * @tparam To The target type to cast to
     * @tparam From The source type to cast from
     * @param from The source value to cast
     * @return A value of type To with the same object representation as from
     *
     * @note The types To and From must be trivially copyable and have the same size.
     *
     * \lang simp-chinese
     * @brief 将一种类型的对象表示重新解释为另一种类型。
     *
     * @details 当C++20可用时，此函数转发给std::bit_cast。
     *           它提供了一种在不违反严格别名规则的情况下，在相同大小的类型之间进行类型双关的安全方式。
     *
     * @tparam To 要转换到的目标类型
     * @tparam From 要转换的源类型
     * @param from 要转换的源值
     * @return 一个类型为To的值，其对象表示与from相同
     *
     * @note To和From类型必须是可平凡复制的且具有相同的大小。
     */
    template <typename To, typename From>
    RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn bit_cast(const From &from) noexcept -> To {
#if RAINY_HAS_CXX20
        return std::bit_cast<To>(from);
#else
        static_assert(sizeof(To) == sizeof(From), "To and From must have the same size");
        static_assert(type_traits::properties::is_trivially_copyable_v<To>, "To must be trivially copyable");
        static_assert(type_traits::properties::is_trivially_copyable_v<From>, "From must be trivially copyable");
        To to;
        std::memcpy(&to, &from, sizeof(To));
        return to;
#endif
    }
}

#endif
