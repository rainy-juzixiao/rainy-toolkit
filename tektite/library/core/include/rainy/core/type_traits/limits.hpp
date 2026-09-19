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
#ifndef RAINY_CORE_LIMITS_HPP
#define RAINY_CORE_LIMITS_HPP
#include <cfloat>
#include <climits>
#include <rainy/core/platform.hpp>

namespace rainy::utility {
    /**
     * \lang english
     * @brief Enumeration representing floating-point denormalization support.
     *
     * \lang simp-chinese
     * @brief 表示浮点数反规范化支持的枚举。
     */
    enum float_denorm_style {
        /**
         * \lang english
         * @brief Denormalization status indeterminate
         *
         *  Cannot determine if denormalization is supported.
         *
         * \lang simp-chinese
         * @brief 反规范化状态无法确定
         *
         *  无法确定是否支持反规范化。
         */
        denorm_indeterminate = -1,

        /**
         * \lang english
         * @brief Denormalization not supported
         *
         *  Denormalization is not supported by the implementation.
         *
         * \lang simp-chinese
         * @brief 不支持反规范化
         *
         *  实现不支持反规范化。
         */
        denorm_absent = 0,

        /**
         * \lang english
         * @brief Denormalization supported
         *
         *  Denormalization is supported by the implementation.
         *
         * \lang simp-chinese
         * @brief 支持反规范化
         *
         *  实现支持反规范化。
         */
        denorm_present = 1
    };

    /**
     * \lang english
     * @brief Enumeration representing floating-point rounding styles.
     *
     * \lang simp-chinese
     * @brief 表示浮点数舍入方式的枚举。
     */
    enum float_round_style {
        /**
         * \lang english
         * @brief Rounding style indeterminate
         *
         *  Rounding style cannot be determined.
         *
         * \lang simp-chinese
         * @brief 舍入方式无法确定
         *
         *  无法确定舍入方式。
         */
        round_indeterminate = -1,

        /**
         * \lang english
         * @brief Round toward zero
         *
         *  Round toward zero, also known as truncation.
         *
         * \lang simp-chinese
         * @brief 向零舍入
         *
         *  向零舍入，也称为截断舍入。
         */
        round_toward_zero = 0,

        /**
         * \lang english
         * @brief Round to nearest
         *
         *  Round to nearest representable value.
         *
         * \lang simp-chinese
         * @brief 向最接近值舍入
         *
         *  向最接近的可表示值舍入。
         */
        round_to_nearest = 1,

        /**
         * \lang english
         * @brief Round toward positive infinity
         *
         *  Round toward positive infinity, also known as ceil.
         *
         * \lang simp-chinese
         * @brief 向正无穷舍入
         *
         *  向正无穷舍入，也称为向上取整。
         */
        round_toward_infinity = 2,

        /**
         * \lang english
         * @brief Round toward negative infinity
         *
         *  Round toward negative infinity, also known as floor.
         *
         * \lang simp-chinese
         * @brief 向负无穷舍入
         *
         *  向负无穷舍入，也称为向下取整。
         */
        round_toward_neg_infinity = 3
    };
}

namespace rainy::utility::implements {
    struct number_base {
        /**
         * \lang english
         * @brief Indicates whether the type supports denormalized values.
         *
         * \lang simp-chinese
         * @brief 指示类型是否支持反规范化值。
         */
        static constexpr float_denorm_style has_denorm = denorm_absent;

        /**
         * \lang english
         * @brief Indicates whether loss of precision is detected as denormalization loss.
         *
         * \lang simp-chinese
         * @brief 指示是否将精度损失检测为反规范化损失。
         */
        static constexpr bool has_denorm_loss = false;

        /**
         * \lang english
         * @brief Indicates whether the type has a representation for positive infinity.
         *
         * \lang simp-chinese
         * @brief 指示类型是否具有正无穷的表示形式。
         */
        static constexpr bool has_infinity = false;

        /**
         * \lang english
         * @brief Indicates whether the type has a representation for quiet NaN.
         *
         * \lang simp-chinese
         * @brief 指示类型是否具有安静NaN的表示形式。
         */
        static constexpr bool has_quiet_NaN = false;

        /**
         * \lang english
         * @brief Indicates whether the type has a representation for signaling NaN.
         *
         * \lang simp-chinese
         * @brief 指示类型是否具有发信号NaN的表示形式。
         */
        static constexpr bool has_signaling_NaN = false;

        /**
         * \lang english
         * @brief Indicates whether the set of values representable by the type is finite.
         *
         * \lang simp-chinese
         * @brief 指示类型可表示的值的集合是否有限。
         */
        static constexpr bool is_bounded = false;

        /**
         * \lang english
         * @brief Indicates whether the type uses exact representation.
         *
         * \lang simp-chinese
         * @brief 指示类型是否使用精确表示。
         */
        static constexpr bool is_exact = false;

        /**
         * \lang english
         * @brief Indicates whether the type conforms to IEC 559 (IEEE 754) standard.
         *
         * \lang simp-chinese
         * @brief 指示类型是否符合IEC 559（IEEE 754）标准。
         */
        static constexpr bool is_iec559 = false;

        /**
         * \lang english
         * @brief Indicates whether the type is integer.
         *
         * \lang simp-chinese
         * @brief 指示类型是否为整型。
         */
        static constexpr bool is_integer = false;

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = false;

        /**
         * \lang english
         * @brief Indicates whether the type is signed.
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号。
         */
        static constexpr bool is_signed = false;

        /**
         * \lang english
         * @brief Indicates whether the numeric_limits specialization is provided for this type.
         *
         * \lang simp-chinese
         * @brief 指示是否为该类型提供了numeric_limits特化。
         */
        static constexpr bool is_specialized = false;

        /**
         * \lang english
         * @brief Indicates whether tinyness is detected before rounding.
         *
         * \lang simp-chinese
         * @brief 指示是否在舍入前检测微小的值。
         */
        static constexpr bool tinyness_before = false;

        /**
         * \lang english
         * @brief Indicates whether trapping is implemented for this type.
         *
         * \lang simp-chinese
         * @brief 指示是否为该类型实现了陷阱。
         */
        static constexpr bool traps = false;

        /**
         * \lang english
         * @brief The rounding style used for the type.
         *
         * \lang simp-chinese
         * @brief 该类型使用的舍入方式。
         */
        static constexpr float_round_style round_style = round_toward_zero;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 0;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 0;

        /**
         * \lang english
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *
         * \lang simp-chinese
         * @brief 区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 0;

        /**
         * \lang english
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = 0;

        /**
         * \lang english
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = 0;

        /**
         * \lang english
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = 0;

        /**
         * \lang english
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = 0;

        /**
         * \lang english
         * @brief The base of the number system used for the type's representation.
         *
         * \lang simp-chinese
         * @brief 用于类型表示的数制基数。
         */
        static constexpr int radix = 0;
    };

    /**
     * \lang english
     * @brief Specialized base class providing numeric traits for integer types.
     *
     *  This class overrides number_base with values appropriate for integral types.
     *
     * \lang simp-chinese
     * @brief 为整数类型提供数值特性的特化基类。
     *
     *  此类使用适用于整型的值覆盖number_base。
     */
    struct number_int_base : number_base {
        /**
         * \lang english
         * @brief Indicates whether the set of values representable by the type is finite.
         *
         * \lang simp-chinese
         * @brief 指示类型可表示的值的集合是否有限。
         */
        static constexpr bool is_bounded = true;

        /**
         * \lang english
         * @brief Indicates whether the type uses exact representation.
         *
         * \lang simp-chinese
         * @brief 指示类型是否使用精确表示。
         */
        static constexpr bool is_exact = true;

        /**
         * \lang english
         * @brief Indicates whether the type is integer.
         *
         * \lang simp-chinese
         * @brief 指示类型是否为整型。
         */
        static constexpr bool is_integer = true;

        /**
         * \lang english
         * @brief Indicates whether the numeric_limits specialization is provided for this type.
         *
         * \lang simp-chinese
         * @brief 指示是否为该类型提供了numeric_limits特化。
         */
        static constexpr bool is_specialized = true;

        /**
         * \lang english
         * @brief The base of the number system used for the type's representation.
         *
         * \lang simp-chinese
         * @brief 用于类型表示的数制基数。
         */
        static constexpr int radix = 2;
    };

    struct number_float_base : number_base {
        /**
         * \lang english
         * @brief Indicates whether the type supports denormalized values.
         *
         * \lang simp-chinese
         * @brief 指示类型是否支持反规范化值。
         */
        static constexpr float_denorm_style has_denorm = denorm_present;

        /**
         * \lang english
         * @brief Indicates whether the type has a representation for positive infinity.
         *
         * \lang simp-chinese
         * @brief 指示类型是否具有正无穷的表示形式。
         */
        static constexpr bool has_infinity = true;

        /**
         * \lang english
         * @brief Indicates whether the type has a representation for quiet NaN.
         *
         * \lang simp-chinese
         * @brief 指示类型是否具有安静NaN的表示形式。
         */
        static constexpr bool has_quiet_NaN = true;

        /**
         * \lang english
         * @brief Indicates whether the type has a representation for signaling NaN.
         *
         * \lang simp-chinese
         * @brief 指示类型是否具有发信号NaN的表示形式。
         */
        static constexpr bool has_signaling_NaN = true;

        /**
         * \lang english
         * @brief Indicates whether the set of values representable by the type is finite.
         *
         * \lang simp-chinese
         * @brief 指示类型可表示的值的集合是否有限。
         */
        static constexpr bool is_bounded = true;

        /**
         * \lang english
         * @brief Indicates whether the type conforms to IEC 559 (IEEE 754) standard.
         *
         * \lang simp-chinese
         * @brief 指示类型是否符合IEC 559（IEEE 754）标准。
         */
        static constexpr bool is_iec559 = true;

        /**
         * \lang english
         * @brief Indicates whether the type is signed.
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号。
         */
        static constexpr bool is_signed = true;

        /**
         * \lang english
         * @brief Indicates whether the numeric_limits specialization is provided for this type.
         *
         * \lang simp-chinese
         * @brief 指示是否为该类型提供了numeric_limits特化。
         */
        static constexpr bool is_specialized = true;

        /**
         * \lang english
         * @brief The rounding style used for the type.
         *
         * \lang simp-chinese
         * @brief 该类型使用的舍入方式。
         */
        static constexpr float_round_style round_style = round_to_nearest;

        /**
         * \lang english
         * @brief The base of the number system used for the type's representation.
         *
         * \lang simp-chinese
         * @brief 用于类型表示的数制基数。
         */
        static constexpr int radix = FLT_RADIX;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Primary template for numeric_limits, providing information about scalar types.
     *
     *  This template provides a generic implementation that returns default values.
     *  Specializations should be provided for specific types to return accurate values.
     *
     * @tparam Ty The type to query
     *
     * \lang simp-chinese
     * @brief numeric_limits 的主模板，提供关于标量类型的信息。
     *
     *  此模板提供返回默认值的通用实现。
     *  应为特定类型提供特化以返回准确的值。
     *
     * @tparam Ty 要查询的类型
     */
    template <typename Ty>
    class numeric_limits : public implements::number_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of the type.
         *
         * @return The minimum finite value
         *
         * \lang simp-chinese
         * @brief 返回类型的最小有限值。
         *
         * @return 最小有限值
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of the type.
         *
         * @return The maximum finite value
         *
         * \lang simp-chinese
         * @brief 返回类型的最大有限值。
         *
         * @return 最大有限值
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value of the type.
         *
         * @return The most negative finite value
         *
         * \lang simp-chinese
         * @brief 返回类型的最负有限值。
         *
         * @return 最负有限值
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the difference between 1 and the next representable value.
         *
         * @return The machine epsilon
         *
         * \lang simp-chinese
         * @brief 返回1和下一个可表示值之间的差。
         *
         * @return 机器精度
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error.
         *
         * @return The maximum rounding error
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差。
         *
         * @return 最大舍入误差
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value.
         *
         * @return The minimum positive denormalized value
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值。
         *
         * @return 最小正反规范化值
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the representation of positive infinity, if available.
         *
         * @return The representation of positive infinity
         *
         * \lang simp-chinese
         * @brief 返回正无穷的表示形式（如果可用）。
         *
         * @return 正无穷的表示形式
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the representation of a quiet NaN, if available.
         *
         * @return The representation of a quiet NaN
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（如果可用）。
         *
         * @return 安静NaN的表示形式
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> Ty {
            return Ty();
        }

        /**
         * \lang english
         * @brief Returns the representation of a signaling NaN, if available.
         *
         * @return The representation of a signaling NaN
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（如果可用）。
         *
         * @return 发信号NaN的表示形式
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> Ty {
            return Ty();
        }
    };

    /**
     * \lang english
     * @brief Partial specialization for const-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief 对于const限定类型的偏特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    class numeric_limits<const Ty> : public numeric_limits<Ty> {};

    /**
     * \lang english
     * @brief Partial specialization for volatile-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief 对于volatile限定类型的偏特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    class numeric_limits<volatile Ty> : public numeric_limits<Ty> {};

    /**
     * \lang english
     * @brief Partial specialization for const volatile-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief 对于const volatile限定类型的偏特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    class numeric_limits<const volatile Ty> : public numeric_limits<Ty> {};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for bool.
     *
     * \lang simp-chinese
     * @brief bool 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<bool> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value (false).
         *
         * @return false
         *
         * \lang simp-chinese
         * @brief 返回最小有限值（false）。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> bool {
            return false;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value (true).
         *
         * @return true
         *
         * \lang simp-chinese
         * @brief 返回最大有限值（true）。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> bool {
            return true;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return false
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> bool {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> bool {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> bool {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> bool {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> bool {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> bool {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> bool {
            return 0;
        }

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 1;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for signed char.
     *
     * \lang simp-chinese
     * @brief signed char 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<signed char> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of signed char.
         *
         * @return SCHAR_MIN
         *
         * \lang simp-chinese
         * @brief 返回 signed char 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> signed char {
            return SCHAR_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of signed char.
         *
         * @return SCHAR_MAX
         *
         * \lang simp-chinese
         * @brief 返回 signed char 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> signed char {
            return SCHAR_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return SCHAR_MIN
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> signed char {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> signed char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> signed char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> signed char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> signed char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> signed char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> signed char {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is signed (always true for signed char).
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号（对于signed char始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 7;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 2;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for unsigned char.
     *
     * \lang simp-chinese
     * @brief unsigned char 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned char> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of unsigned char.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 unsigned char 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of unsigned char.
         *
         * @return UCHAR_MAX
         *
         * \lang simp-chinese
         * @brief 返回 unsigned char 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned char {
            return UCHAR_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned char {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned char {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 8;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 2;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for char16_t.
     *
     * \lang simp-chinese
     * @brief char16_t 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<char16_t> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of char16_t.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 char16_t 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of char16_t.
         *
         * @return USHRT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 char16_t 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> char16_t {
            return USHRT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> char16_t {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> char16_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 16;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for char32_t.
     *
     * \lang simp-chinese
     * @brief char32_t 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<char32_t> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of char32_t.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 char32_t 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of char32_t.
         *
         * @return UINT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 char32_t 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> char32_t {
            return UINT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> char32_t {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> char32_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 32;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for wchar_t.
     *
     * \lang simp-chinese
     * @brief wchar_t 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<wchar_t> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of wchar_t.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 wchar_t 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of wchar_t.
         *
         * @return 0xffff
         *
         * \lang simp-chinese
         * @brief 返回 wchar_t 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> wchar_t {
            return 0xffff;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> wchar_t {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> wchar_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 16;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for short.
     *
     * \lang simp-chinese
     * @brief short 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<short> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of short.
         *
         * @return SHRT_MIN
         *
         * \lang simp-chinese
         * @brief 返回 short 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> short {
            return SHRT_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of short.
         *
         * @return SHRT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 short 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> short {
            return SHRT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return SHRT_MIN
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> short {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> short {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is signed (always true for short).
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号（对于short始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 15;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for int.
     *
     * \lang simp-chinese
     * @brief int 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<int> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of int.
         *
         * @return INT_MIN
         *
         * \lang simp-chinese
         * @brief 返回 int 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> int {
            return INT_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of int.
         *
         * @return INT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 int 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> int {
            return INT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return INT_MIN
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> int {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> int {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is signed (always true for int).
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号（对于int始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 31;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for long.
     *
     * \lang simp-chinese
     * @brief long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<long> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of long.
         *
         * @return LONG_MIN
         *
         * \lang simp-chinese
         * @brief 返回 long 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> long {
            return LONG_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of long.
         *
         * @return LONG_MAX
         *
         * \lang simp-chinese
         * @brief 返回 long 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> long {
            return LONG_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return LONG_MIN
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> long {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> long {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is signed (always true for long).
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号（对于long始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 31;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for long long.
     *
     * \lang simp-chinese
     * @brief long long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<long long> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of long long.
         *
         * @return LLONG_MIN
         *
         * \lang simp-chinese
         * @brief 返回 long long 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> long long {
            return LLONG_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of long long.
         *
         * @return LLONG_MAX
         *
         * \lang simp-chinese
         * @brief 返回 long long 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> long long {
            return LLONG_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return LLONG_MIN
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> long long {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is signed (always true for long long).
         *
         * \lang simp-chinese
         * @brief 指示类型是否有符号（对于long long始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 63;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 18;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for unsigned short.
     *
     * \lang simp-chinese
     * @brief unsigned short 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned short> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of unsigned short.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 unsigned short 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of unsigned short.
         *
         * @return USHRT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 unsigned short 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned short {
            return USHRT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned short {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned short {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 16;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for unsigned int.
     *
     * \lang simp-chinese
     * @brief unsigned int 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned int> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of unsigned int.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 unsigned int 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of unsigned int.
         *
         * @return UINT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 unsigned int 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned int {
            return UINT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned int {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned int {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 32;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for unsigned long.
     *
     * \lang simp-chinese
     * @brief unsigned long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned long> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of unsigned long.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 unsigned long 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of unsigned long.
         *
         * @return ULONG_MAX
         *
         * \lang simp-chinese
         * @brief 返回 unsigned long 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned long {
            return ULONG_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned long {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned long {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 32;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for unsigned long long.
     *
     * \lang simp-chinese
     * @brief unsigned long long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned long long> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of unsigned long long.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 unsigned long long 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of unsigned long long.
         *
         * @return ULLONG_MAX
         *
         * \lang simp-chinese
         * @brief 返回 unsigned long long 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned long long {
            return ULLONG_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned long long {
            return (min) ();
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 64;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 19;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for float.
     *
     * \lang simp-chinese
     * @brief float 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<float> : public implements::number_float_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of float.
         *
         * @return FLT_MIN
         *
         * \lang simp-chinese
         * @brief 返回 float 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> float {
            return FLT_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of float.
         *
         * @return FLT_MAX
         *
         * \lang simp-chinese
         * @brief 返回 float 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> float {
            return FLT_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (negative of max).
         *
         * @return -FLT_MAX
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（max的相反数）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> float {
            return -(max) ();
        }

        /**
         * \lang english
         * @brief Returns the machine epsilon (difference between 1 and the next representable value).
         *
         * @return FLT_EPSILON
         *
         * \lang simp-chinese
         * @brief 返回机器精度（1和下一个可表示值之间的差）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> float {
            return FLT_EPSILON;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error.
         *
         * @return 0.5F
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> float {
            return 0.5F;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value.
         *
         * @return FLT_TRUE_MIN
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> float {
            return FLT_TRUE_MIN;
        }

        /**
         * \lang english
         * @brief Returns the representation of positive infinity.
         *
         * @return Positive infinity value for float
         *
         * \lang simp-chinese
         * @brief 返回正无穷的表示形式。
         *
         * @return float的正无穷值
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> float {
            return core::builtin::huge_valf();
        }

        /**
         * \lang english
         * @brief Returns the representation of a quiet NaN.
         *
         * @return Quiet NaN value for float
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式。
         *
         * @return float的安静NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> float {
            return __builtin_nanf("1");
        }

        /**
         * \lang english
         * @brief Returns the representation of a signaling NaN.
         *
         * @return Signaling NaN value for float
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式。
         *
         * @return float的发信号NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> float {
            return __builtin_nansf("1");
        }

        /**
         * \lang english
         * @brief Number of radix digits in the mantissa.
         *
         * \lang simp-chinese
         * @brief 尾数中的基数位数。
         */
        static constexpr int digits = FLT_MANT_DIG;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = FLT_DIG;

        /**
         * \lang english
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *
         * \lang simp-chinese
         * @brief 区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 9;

        /**
         * \lang english
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = FLT_MAX_EXP;

        /**
         * \lang english
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = FLT_MAX_10_EXP;

        /**
         * \lang english
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = FLT_MIN_EXP;

        /**
         * \lang english
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = FLT_MIN_10_EXP;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for double.
     *
     * \lang simp-chinese
     * @brief double 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<double> : public implements::number_float_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of double.
         *
         * @return DBL_MIN
         *
         * \lang simp-chinese
         * @brief 返回 double 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> double {
            return DBL_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of double.
         *
         * @return DBL_MAX
         *
         * \lang simp-chinese
         * @brief 返回 double 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> double {
            return DBL_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (negative of max).
         *
         * @return -DBL_MAX
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（max的相反数）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> double {
            return -(max) ();
        }

        /**
         * \lang english
         * @brief Returns the machine epsilon (difference between 1 and the next representable value).
         *
         * @return DBL_EPSILON
         *
         * \lang simp-chinese
         * @brief 返回机器精度（1和下一个可表示值之间的差）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> double {
            return DBL_EPSILON;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error.
         *
         * @return 0.5
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> double {
            return 0.5;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value.
         *
         * @return DBL_TRUE_MIN
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> double {
            return DBL_TRUE_MIN;
        }

        /**
         * \lang english
         * @brief Returns the representation of positive infinity.
         *
         * @return Positive infinity value for double
         *
         * \lang simp-chinese
         * @brief 返回正无穷的表示形式。
         *
         * @return double的正无穷值
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> double {
            return core::builtin::huge_val();
        }

        /**
         * \lang english
         * @brief Returns the representation of a quiet NaN.
         *
         * @return Quiet NaN value for double
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式。
         *
         * @return double的安静NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> double {
            return __builtin_nan("0");
        }

        /**
         * \lang english
         * @brief Returns the representation of a signaling NaN.
         *
         * @return Signaling NaN value for double
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式。
         *
         * @return double的发信号NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> double {
            return __builtin_nans("1");
        }

        /**
         * \lang english
         * @brief Number of radix digits in the mantissa.
         *
         * \lang simp-chinese
         * @brief 尾数中的基数位数。
         */
        static constexpr int digits = DBL_MANT_DIG;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = DBL_DIG;

        /**
         * \lang english
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *
         * \lang simp-chinese
         * @brief 区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 17;

        /**
         * \lang english
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = DBL_MAX_EXP;

        /**
         * \lang english
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = DBL_MAX_10_EXP;

        /**
         * \lang english
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = DBL_MIN_EXP;

        /**
         * \lang english
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = DBL_MIN_10_EXP;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Specialization of numeric_limits for long double.
     *
     * \lang simp-chinese
     * @brief long double 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<long double> : public implements::number_float_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of long double.
         *
         * @return LDBL_MIN
         *
         * \lang simp-chinese
         * @brief 返回 long double 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> long double {
            return LDBL_MIN;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of long double.
         *
         * @return LDBL_MAX
         *
         * \lang simp-chinese
         * @brief 返回 long double 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> long double {
            return LDBL_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (negative of max).
         *
         * @return -LDBL_MAX
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（max的相反数）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> long double {
            return -(max) ();
        }

        /**
         * \lang english
         * @brief Returns the machine epsilon (difference between 1 and the next representable value).
         *
         * @return LDBL_EPSILON
         *
         * \lang simp-chinese
         * @brief 返回机器精度（1和下一个可表示值之间的差）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> long double {
            return LDBL_EPSILON;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error.
         *
         * @return 0.5L
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> long double {
            return 0.5L;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value.
         *
         * @return LDBL_TRUE_MIN
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> long double {
            return LDBL_TRUE_MIN;
        }

        /**
         * \lang english
         * @brief Returns the representation of positive infinity.
         *
         * @return Positive infinity value for long double
         *
         * \lang simp-chinese
         * @brief 返回正无穷的表示形式。
         *
         * @return long double的正无穷值
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> long double {
            return (core::builtin::huge_val)();
        }

        /**
         * \lang english
         * @brief Returns the representation of a quiet NaN.
         *
         * @return Quiet NaN value for long double
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式。
         *
         * @return long double的安静NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> long double {
            return __builtin_nan("0");
        }

        /**
         * \lang english
         * @brief Returns the representation of a signaling NaN.
         *
         * @return Signaling NaN value for long double
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式。
         *
         * @return long double的发信号NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> long double {
            return __builtin_nans("1");
        }

        /**
         * \lang english
         * @brief Number of radix digits in the mantissa.
         *
         * \lang simp-chinese
         * @brief 尾数中的基数位数。
         */
        static constexpr int digits = LDBL_MANT_DIG;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = LDBL_DIG;

        /**
         * \lang english
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *
         * \lang simp-chinese
         * @brief 区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 2 + LDBL_MANT_DIG * 301L / 1000;

        /**
         * \lang english
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = LDBL_MAX_EXP;

        /**
         * \lang english
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *
         * \lang simp-chinese
         * @brief 最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = LDBL_MAX_10_EXP;

        /**
         * \lang english
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = LDBL_MIN_EXP;

        /**
         * \lang english
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *
         * \lang simp-chinese
         * @brief 最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = LDBL_MIN_10_EXP;
    };
}

namespace rainy::utility {
#ifdef __cpp_char8_t
    /**
     * \lang english
     * @brief Specialization of numeric_limits for char8_t (C++20).
     *
     * \lang simp-chinese
     * @brief char8_t 类型的 numeric_limits 特化（C++20）。
     */
    template <>
    class numeric_limits<char8_t> : public implements::number_int_base {
    public:
        /**
         * \lang english
         * @brief Returns the minimum finite value of char8_t.
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回 char8_t 的最小有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum finite value of char8_t.
         *
         * @return UCHAR_MAX
         *
         * \lang simp-chinese
         * @brief 返回 char8_t 的最大有限值。
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> char8_t {
            return UCHAR_MAX;
        }

        /**
         * \lang english
         * @brief Returns the most negative finite value (same as min).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最负有限值（与min相同）。
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns epsilon (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回机器精度（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the maximum rounding error (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最大舍入误差（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回最小正反规范化值（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns infinity representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回无穷的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns quiet NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回安静NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Returns signaling NaN representation (0 for integral types).
         *
         * @return 0
         *
         * \lang simp-chinese
         * @brief 返回发信号NaN的表示形式（对于整型为0）。
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> char8_t {
            return 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *
         * \lang simp-chinese
         * @brief 指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * \lang english
         * @brief Number of radix digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的基数位数。
         */
        static constexpr int digits = 8;

        /**
         * \lang english
         * @brief Number of decimal digits that can be represented without change.
         *
         * \lang simp-chinese
         * @brief 可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 2;
    };
#endif
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Checks if a value is within the valid range for its type.
     *
     *  This function verifies that the given value is not less than the type's
     *  lowest representable value and not greater than the type's maximum
     *  representable value.
     *
     * @tparam Ty The type of the value to check
     * @param value The value to check
     * @return true if the value is within the valid range for type Ty,
     *          false otherwise
     *
     * \lang simp-chinese
     * @brief 检查一个值是否在其类型的有效范围内。
     *
     *  此函数验证给定值不小于该类型的最小可表示值，
     *  且不大于该类型的最大可表示值。
     *
     * @tparam Ty 要检查的值的类型
     * @param value 要检查的值
     * @return 如果值在类型Ty的有效范围内则为true，否则为false
     */
    template <typename Ty>
    constexpr rain_fn is_in_valid_range(Ty value) -> bool {
        return value >= numeric_limits<Ty>::lowest() && value <= (numeric_limits<Ty>::max)();
    }
}

#endif
