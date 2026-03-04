#ifndef RAINY_CORE_LIMITS_HPP
#define RAINY_CORE_LIMITS_HPP
#include <cfloat>
#include <climits>
#include <rainy/core/platform.hpp>

namespace rainy::utility {
    /**
     * @brief Enumeration representing floating-point denormalization support.
     *        表示浮点数反规范化支持的枚举。
     */
    enum float_denorm_style {
        /**
         * @brief Denormalization status indeterminate
         *        反规范化状态无法确定
         *
         * Cannot determine if denormalization is supported.
         * 无法确定是否支持反规范化。
         */
        denorm_indeterminate = -1,

        /**
         * @brief Denormalization not supported
         *        不支持反规范化
         *
         * Denormalization is not supported by the implementation.
         * 实现不支持反规范化。
         */
        denorm_absent = 0,

        /**
         * @brief Denormalization supported
         *        支持反规范化
         *
         * Denormalization is supported by the implementation.
         * 实现支持反规范化。
         */
        denorm_present = 1
    };

    /**
     * @brief Enumeration representing floating-point rounding styles.
     *        表示浮点数舍入方式的枚举。
     */
    enum float_round_style {
        /**
         * @brief Rounding style indeterminate
         *        舍入方式无法确定
         *
         * Rounding style cannot be determined.
         * 无法确定舍入方式。
         */
        round_indeterminate = -1,

        /**
         * @brief Round toward zero
         *        向零舍入
         *
         * Round toward zero, also known as truncation.
         * 向零舍入，也称为截断舍入。
         */
        round_toward_zero = 0,

        /**
         * @brief Round to nearest
         *        向最接近值舍入
         *
         * Round to nearest representable value.
         * 向最接近的可表示值舍入。
         */
        round_to_nearest = 1,

        /**
         * @brief Round toward positive infinity
         *        向正无穷舍入
         *
         * Round toward positive infinity, also known as ceil.
         * 向正无穷舍入，也称为向上取整。
         */
        round_toward_infinity = 2,

        /**
         * @brief Round toward negative infinity
         *        向负无穷舍入
         *
         * Round toward negative infinity, also known as floor.
         * 向负无穷舍入，也称为向下取整。
         */
        round_toward_neg_infinity = 3
    };
}

namespace rainy::utility::implements {
    struct number_base {
        /**
         * @brief Indicates whether the type supports denormalized values.
         *        指示类型是否支持反规范化值。
         */
        static constexpr float_denorm_style has_denorm = denorm_absent;

        /**
         * @brief Indicates whether loss of precision is detected as denormalization loss.
         *        指示是否将精度损失检测为反规范化损失。
         */
        static constexpr bool has_denorm_loss = false;

        /**
         * @brief Indicates whether the type has a representation for positive infinity.
         *        指示类型是否具有正无穷的表示形式。
         */
        static constexpr bool has_infinity = false;

        /**
         * @brief Indicates whether the type has a representation for quiet NaN.
         *        指示类型是否具有安静NaN的表示形式。
         */
        static constexpr bool has_quiet_NaN = false;

        /**
         * @brief Indicates whether the type has a representation for signaling NaN.
         *        指示类型是否具有发信号NaN的表示形式。
         */
        static constexpr bool has_signaling_NaN = false;

        /**
         * @brief Indicates whether the set of values representable by the type is finite.
         *        指示类型可表示的值的集合是否有限。
         */
        static constexpr bool is_bounded = false;

        /**
         * @brief Indicates whether the type uses exact representation.
         *        指示类型是否使用精确表示。
         */
        static constexpr bool is_exact = false;

        /**
         * @brief Indicates whether the type conforms to IEC 559 (IEEE 754) standard.
         *        指示类型是否符合IEC 559（IEEE 754）标准。
         */
        static constexpr bool is_iec559 = false;

        /**
         * @brief Indicates whether the type is integer.
         *        指示类型是否为整型。
         */
        static constexpr bool is_integer = false;

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = false;

        /**
         * @brief Indicates whether the type is signed.
         *        指示类型是否有符号。
         */
        static constexpr bool is_signed = false;

        /**
         * @brief Indicates whether the numeric_limits specialization is provided for this type.
         *        指示是否为该类型提供了numeric_limits特化。
         */
        static constexpr bool is_specialized = false;

        /**
         * @brief Indicates whether tinyness is detected before rounding.
         *        指示是否在舍入前检测微小的值。
         */
        static constexpr bool tinyness_before = false;

        /**
         * @brief Indicates whether trapping is implemented for this type.
         *        指示是否为该类型实现了陷阱。
         */
        static constexpr bool traps = false;

        /**
         * @brief The rounding style used for the type.
         *        该类型使用的舍入方式。
         */
        static constexpr float_round_style round_style = round_toward_zero;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 0;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 0;

        /**
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *        区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 0;

        /**
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *        最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = 0;

        /**
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *        最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = 0;

        /**
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *        最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = 0;

        /**
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *        最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = 0;

        /**
         * @brief The base of the number system used for the type's representation.
         *        用于类型表示的数制基数。
         */
        static constexpr int radix = 0;
    };

    /**
     * @brief Specialized base class providing numeric traits for integer types.
     *        为整数类型提供数值特性的特化基类。
     *
     * This class overrides number_base with values appropriate for integral types.
     *
     * 此类使用适用于整型的值覆盖number_base。
     */
    struct number_int_base : number_base {
        /**
         * @brief Indicates whether the set of values representable by the type is finite.
         *        指示类型可表示的值的集合是否有限。
         */
        static constexpr bool is_bounded = true;

        /**
         * @brief Indicates whether the type uses exact representation.
         *        指示类型是否使用精确表示。
         */
        static constexpr bool is_exact = true;

        /**
         * @brief Indicates whether the type is integer.
         *        指示类型是否为整型。
         */
        static constexpr bool is_integer = true;

        /**
         * @brief Indicates whether the numeric_limits specialization is provided for this type.
         *        指示是否为该类型提供了numeric_limits特化。
         */
        static constexpr bool is_specialized = true;

        /**
         * @brief The base of the number system used for the type's representation.
         *        用于类型表示的数制基数。
         */
        static constexpr int radix = 2;
    };

    struct number_float_base : number_base {
        /**
         * @brief Indicates whether the type supports denormalized values.
         *        指示类型是否支持反规范化值。
         */
        static constexpr float_denorm_style has_denorm = denorm_present;

        /**
         * @brief Indicates whether the type has a representation for positive infinity.
         *        指示类型是否具有正无穷的表示形式。
         */
        static constexpr bool has_infinity = true;

        /**
         * @brief Indicates whether the type has a representation for quiet NaN.
         *        指示类型是否具有安静NaN的表示形式。
         */
        static constexpr bool has_quiet_NaN = true;

        /**
         * @brief Indicates whether the type has a representation for signaling NaN.
         *        指示类型是否具有发信号NaN的表示形式。
         */
        static constexpr bool has_signaling_NaN = true;

        /**
         * @brief Indicates whether the set of values representable by the type is finite.
         *        指示类型可表示的值的集合是否有限。
         */
        static constexpr bool is_bounded = true;

        /**
         * @brief Indicates whether the type conforms to IEC 559 (IEEE 754) standard.
         *        指示类型是否符合IEC 559（IEEE 754）标准。
         */
        static constexpr bool is_iec559 = true;

        /**
         * @brief Indicates whether the type is signed.
         *        指示类型是否有符号。
         */
        static constexpr bool is_signed = true;

        /**
         * @brief Indicates whether the numeric_limits specialization is provided for this type.
         *        指示是否为该类型提供了numeric_limits特化。
         */
        static constexpr bool is_specialized = true;

        /**
         * @brief The rounding style used for the type.
         *        该类型使用的舍入方式。
         */
        static constexpr float_round_style round_style = round_to_nearest;

        /**
         * @brief The base of the number system used for the type's representation.
         *        用于类型表示的数制基数。
         */
        static constexpr int radix = FLT_RADIX;
    };
}

namespace rainy::utility {
    /**
     * @brief Primary template for numeric_limits, providing information about scalar types.
     *        numeric_limits 的主模板，提供关于标量类型的信息。
     *
     * This template provides a generic implementation that returns default values.
     * Specializations should be provided for specific types to return accurate values.
     *
     * 此模板提供返回默认值的通用实现。
     * 应为特定类型提供特化以返回准确的值。
     *
     * @tparam Ty The type to query
     *            要查询的类型
     */
    template <typename Ty>
    class numeric_limits : public implements::number_base {
    public:
        /**
         * @brief Returns the minimum finite value of the type.
         *        返回类型的最小有限值。
         *
         * @return The minimum finite value
         *         最小有限值
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the maximum finite value of the type.
         *        返回类型的最大有限值。
         *
         * @return The maximum finite value
         *         最大有限值
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the most negative finite value of the type.
         *        返回类型的最负有限值。
         *
         * @return The most negative finite value
         *         最负有限值
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the difference between 1 and the next representable value.
         *        返回1和下一个可表示值之间的差。
         *
         * @return The machine epsilon
         *         机器精度
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the maximum rounding error.
         *        返回最大舍入误差。
         *
         * @return The maximum rounding error
         *         最大舍入误差
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the minimum positive denormalized value.
         *        返回最小正反规范化值。
         *
         * @return The minimum positive denormalized value
         *         最小正反规范化值
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the representation of positive infinity, if available.
         *        返回正无穷的表示形式（如果可用）。
         *
         * @return The representation of positive infinity
         *         正无穷的表示形式
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the representation of a quiet NaN, if available.
         *        返回安静NaN的表示形式（如果可用）。
         *
         * @return The representation of a quiet NaN
         *         安静NaN的表示形式
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> Ty {
            return Ty();
        }

        /**
         * @brief Returns the representation of a signaling NaN, if available.
         *        返回发信号NaN的表示形式（如果可用）。
         *
         * @return The representation of a signaling NaN
         *         发信号NaN的表示形式
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> Ty {
            return Ty();
        }
    };

    /**
     * @brief Partial specialization for const-qualified types.
     *        对于const限定类型的偏特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    class numeric_limits<const Ty> : public numeric_limits<Ty> {};

    /**
     * @brief Partial specialization for volatile-qualified types.
     *        对于volatile限定类型的偏特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    class numeric_limits<volatile Ty> : public numeric_limits<Ty> {};

    /**
     * @brief Partial specialization for const volatile-qualified types.
     *        对于const volatile限定类型的偏特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    class numeric_limits<const volatile Ty> : public numeric_limits<Ty> {};
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for bool.
     *        bool 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<bool> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value (false).
         *        返回最小有限值（false）。
         *
         * @return false
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> bool {
            return false;
        }

        /**
         * @brief Returns the maximum finite value (true).
         *        返回最大有限值（true）。
         *
         * @return true
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> bool {
            return true;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return false
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> bool {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> bool {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> bool {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> bool {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> bool {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> bool {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> bool {
            return 0;
        }

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 1;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for signed char.
     *        signed char 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<signed char> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of signed char.
         *        返回 signed char 的最小有限值。
         *
         * @return SCHAR_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> signed char {
            return SCHAR_MIN;
        }

        /**
         * @brief Returns the maximum finite value of signed char.
         *        返回 signed char 的最大有限值。
         *
         * @return SCHAR_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> signed char {
            return SCHAR_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return SCHAR_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> signed char {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> signed char {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> signed char {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> signed char {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> signed char {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> signed char {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> signed char {
            return 0;
        }

        /**
         * @brief Indicates whether the type is signed (always true for signed char).
         *        指示类型是否有符号（对于signed char始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 7;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 2;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for unsigned char.
     *        unsigned char 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned char> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of unsigned char.
         *        返回 unsigned char 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of unsigned char.
         *        返回 unsigned char 的最大有限值。
         *
         * @return UCHAR_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned char {
            return UCHAR_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned char {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned char {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 8;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 2;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for char16_t.
     *        char16_t 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<char16_t> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of char16_t.
         *        返回 char16_t 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of char16_t.
         *        返回 char16_t 的最大有限值。
         *
         * @return USHRT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> char16_t {
            return USHRT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> char16_t {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> char16_t {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 16;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for char32_t.
     *        char32_t 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<char32_t> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of char32_t.
         *        返回 char32_t 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of char32_t.
         *        返回 char32_t 的最大有限值。
         *
         * @return UINT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> char32_t {
            return UINT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> char32_t {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> char32_t {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 32;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for wchar_t.
     *        wchar_t 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<wchar_t> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of wchar_t.
         *        返回 wchar_t 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of wchar_t.
         *        返回 wchar_t 的最大有限值。
         *
         * @return 0xffff
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> wchar_t {
            return 0xffff;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> wchar_t {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> wchar_t {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 16;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for short.
     *        short 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<short> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of short.
         *        返回 short 的最小有限值。
         *
         * @return SHRT_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> short {
            return SHRT_MIN;
        }

        /**
         * @brief Returns the maximum finite value of short.
         *        返回 short 的最大有限值。
         *
         * @return SHRT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> short {
            return SHRT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return SHRT_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> short {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> short {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> short {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> short {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> short {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> short {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> short {
            return 0;
        }

        /**
         * @brief Indicates whether the type is signed (always true for short).
         *        指示类型是否有符号（对于short始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 15;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for int.
     *        int 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<int> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of int.
         *        返回 int 的最小有限值。
         *
         * @return INT_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> int {
            return INT_MIN;
        }

        /**
         * @brief Returns the maximum finite value of int.
         *        返回 int 的最大有限值。
         *
         * @return INT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> int {
            return INT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return INT_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> int {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> int {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> int {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> int {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> int {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> int {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> int {
            return 0;
        }

        /**
         * @brief Indicates whether the type is signed (always true for int).
         *        指示类型是否有符号（对于int始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 31;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for long.
     *        long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<long> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of long.
         *        返回 long 的最小有限值。
         *
         * @return LONG_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> long {
            return LONG_MIN;
        }

        /**
         * @brief Returns the maximum finite value of long.
         *        返回 long 的最大有限值。
         *
         * @return LONG_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> long {
            return LONG_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return LONG_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> long {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> long {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> long {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> long {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> long {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> long {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> long {
            return 0;
        }

        /**
         * @brief Indicates whether the type is signed (always true for long).
         *        指示类型是否有符号（对于long始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 31;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for long long.
     *        long long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<long long> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of long long.
         *        返回 long long 的最小有限值。
         *
         * @return LLONG_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> long long {
            return LLONG_MIN;
        }

        /**
         * @brief Returns the maximum finite value of long long.
         *        返回 long long 的最大有限值。
         *
         * @return LLONG_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> long long {
            return LLONG_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return LLONG_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> long long {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> long long {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> long long {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> long long {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> long long {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> long long {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> long long {
            return 0;
        }

        /**
         * @brief Indicates whether the type is signed (always true for long long).
         *        指示类型是否有符号（对于long long始终为true）。
         */
        static constexpr bool is_signed = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 63;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 18;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for unsigned short.
     *        unsigned short 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned short> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of unsigned short.
         *        返回 unsigned short 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of unsigned short.
         *        返回 unsigned short 的最大有限值。
         *
         * @return USHRT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned short {
            return USHRT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned short {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned short {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 16;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 4;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for unsigned int.
     *        unsigned int 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned int> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of unsigned int.
         *        返回 unsigned int 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of unsigned int.
         *        返回 unsigned int 的最大有限值。
         *
         * @return UINT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned int {
            return UINT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned int {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned int {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 32;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for unsigned long.
     *        unsigned long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned long> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of unsigned long.
         *        返回 unsigned long 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of unsigned long.
         *        返回 unsigned long 的最大有限值。
         *
         * @return ULONG_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned long {
            return ULONG_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned long {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned long {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 32;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 9;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for unsigned long long.
     *        unsigned long long 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<unsigned long long> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of unsigned long long.
         *        返回 unsigned long long 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of unsigned long long.
         *        返回 unsigned long long 的最大有限值。
         *
         * @return ULLONG_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> unsigned long long {
            return ULLONG_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> unsigned long long {
            return (min) ();
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> unsigned long long {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 64;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 19;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for float.
     *        float 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<float> : public implements::number_float_base {
    public:
        /**
         * @brief Returns the minimum finite value of float.
         *        返回 float 的最小有限值。
         *
         * @return FLT_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> float {
            return FLT_MIN;
        }

        /**
         * @brief Returns the maximum finite value of float.
         *        返回 float 的最大有限值。
         *
         * @return FLT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> float {
            return FLT_MAX;
        }

        /**
         * @brief Returns the most negative finite value (negative of max).
         *        返回最负有限值（max的相反数）。
         *
         * @return -FLT_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> float {
            return -(max) ();
        }

        /**
         * @brief Returns the machine epsilon (difference between 1 and the next representable value).
         *        返回机器精度（1和下一个可表示值之间的差）。
         *
         * @return FLT_EPSILON
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> float {
            return FLT_EPSILON;
        }

        /**
         * @brief Returns the maximum rounding error.
         *        返回最大舍入误差。
         *
         * @return 0.5F
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> float {
            return 0.5F;
        }

        /**
         * @brief Returns the minimum positive denormalized value.
         *        返回最小正反规范化值。
         *
         * @return FLT_TRUE_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> float {
            return FLT_TRUE_MIN;
        }

        /**
         * @brief Returns the representation of positive infinity.
         *        返回正无穷的表示形式。
         *
         * @return Positive infinity value for float
         *         float的正无穷值
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> float {
            return core::builtin::huge_valf();
        }

        /**
         * @brief Returns the representation of a quiet NaN.
         *        返回安静NaN的表示形式。
         *
         * @return Quiet NaN value for float
         *         float的安静NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> float {
            return __builtin_nanf("1");
        }

        /**
         * @brief Returns the representation of a signaling NaN.
         *        返回发信号NaN的表示形式。
         *
         * @return Signaling NaN value for float
         *         float的发信号NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> float {
            return __builtin_nansf("1");
        }

        /**
         * @brief Number of radix digits in the mantissa.
         *        尾数中的基数位数。
         */
        static constexpr int digits = FLT_MANT_DIG;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = FLT_DIG;

        /**
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *        区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 9;

        /**
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *        最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = FLT_MAX_EXP;

        /**
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *        最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = FLT_MAX_10_EXP;

        /**
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *        最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = FLT_MIN_EXP;

        /**
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *        最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = FLT_MIN_10_EXP;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for double.
     *        double 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<double> : public implements::number_float_base {
    public:
        /**
         * @brief Returns the minimum finite value of double.
         *        返回 double 的最小有限值。
         *
         * @return DBL_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> double {
            return DBL_MIN;
        }

        /**
         * @brief Returns the maximum finite value of double.
         *        返回 double 的最大有限值。
         *
         * @return DBL_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> double {
            return DBL_MAX;
        }

        /**
         * @brief Returns the most negative finite value (negative of max).
         *        返回最负有限值（max的相反数）。
         *
         * @return -DBL_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> double {
            return -(max) ();
        }

        /**
         * @brief Returns the machine epsilon (difference between 1 and the next representable value).
         *        返回机器精度（1和下一个可表示值之间的差）。
         *
         * @return DBL_EPSILON
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> double {
            return DBL_EPSILON;
        }

        /**
         * @brief Returns the maximum rounding error.
         *        返回最大舍入误差。
         *
         * @return 0.5
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> double {
            return 0.5;
        }

        /**
         * @brief Returns the minimum positive denormalized value.
         *        返回最小正反规范化值。
         *
         * @return DBL_TRUE_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> double {
            return DBL_TRUE_MIN;
        }

        /**
         * @brief Returns the representation of positive infinity.
         *        返回正无穷的表示形式。
         *
         * @return Positive infinity value for double
         *         double的正无穷值
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> double {
            return core::builtin::huge_val();
        }

        /**
         * @brief Returns the representation of a quiet NaN.
         *        返回安静NaN的表示形式。
         *
         * @return Quiet NaN value for double
         *         double的安静NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> double {
            return __builtin_nan("0");
        }

        /**
         * @brief Returns the representation of a signaling NaN.
         *        返回发信号NaN的表示形式。
         *
         * @return Signaling NaN value for double
         *         double的发信号NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> double {
            return __builtin_nans("1");
        }

        /**
         * @brief Number of radix digits in the mantissa.
         *        尾数中的基数位数。
         */
        static constexpr int digits = DBL_MANT_DIG;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = DBL_DIG;

        /**
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *        区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 17;

        /**
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *        最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = DBL_MAX_EXP;

        /**
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *        最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = DBL_MAX_10_EXP;

        /**
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *        最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = DBL_MIN_EXP;

        /**
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *        最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = DBL_MIN_10_EXP;
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization of numeric_limits for long double.
     *        long double 类型的 numeric_limits 特化。
     */
    template <>
    class numeric_limits<long double> : public implements::number_float_base {
    public:
        /**
         * @brief Returns the minimum finite value of long double.
         *        返回 long double 的最小有限值。
         *
         * @return LDBL_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> long double {
            return LDBL_MIN;
        }

        /**
         * @brief Returns the maximum finite value of long double.
         *        返回 long double 的最大有限值。
         *
         * @return LDBL_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> long double {
            return LDBL_MAX;
        }

        /**
         * @brief Returns the most negative finite value (negative of max).
         *        返回最负有限值（max的相反数）。
         *
         * @return -LDBL_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> long double {
            return -(max) ();
        }

        /**
         * @brief Returns the machine epsilon (difference between 1 and the next representable value).
         *        返回机器精度（1和下一个可表示值之间的差）。
         *
         * @return LDBL_EPSILON
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> long double {
            return LDBL_EPSILON;
        }

        /**
         * @brief Returns the maximum rounding error.
         *        返回最大舍入误差。
         *
         * @return 0.5L
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> long double {
            return 0.5L;
        }

        /**
         * @brief Returns the minimum positive denormalized value.
         *        返回最小正反规范化值。
         *
         * @return LDBL_TRUE_MIN
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> long double {
            return LDBL_TRUE_MIN;
        }

        /**
         * @brief Returns the representation of positive infinity.
         *        返回正无穷的表示形式。
         *
         * @return Positive infinity value for long double
         *         long double的正无穷值
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> long double {
            return (core::builtin::huge_val)();
        }

        /**
         * @brief Returns the representation of a quiet NaN.
         *        返回安静NaN的表示形式。
         *
         * @return Quiet NaN value for long double
         *         long double的安静NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> long double {
            return __builtin_nan("0");
        }

        /**
         * @brief Returns the representation of a signaling NaN.
         *        返回发信号NaN的表示形式。
         *
         * @return Signaling NaN value for long double
         *         long double的发信号NaN值
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> long double {
            return __builtin_nans("1");
        }

        /**
         * @brief Number of radix digits in the mantissa.
         *        尾数中的基数位数。
         */
        static constexpr int digits = LDBL_MANT_DIG;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = LDBL_DIG;

        /**
         * @brief Number of decimal digits necessary to differentiate all values of this type.
         *        区分该类型所有值所需的十进制位数。
         */
        static constexpr int max_digits10 = 2 + LDBL_MANT_DIG * 301L / 1000;

        /**
         * @brief Maximum positive integer such that radix raised to that power is a valid finite value.
         *        最大的正整数，使得基数的该次幂是有效的有限值。
         */
        static constexpr int max_exponent = LDBL_MAX_EXP;

        /**
         * @brief Maximum positive integer such that 10 raised to that power is a valid finite value.
         *        最大的正整数，使得10的该次幂是有效的有限值。
         */
        static constexpr int max_exponent10 = LDBL_MAX_10_EXP;

        /**
         * @brief Minimum positive integer such that radix raised to that power is a valid normalized value.
         *        最小的正整数，使得基数的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent = LDBL_MIN_EXP;

        /**
         * @brief Minimum positive integer such that 10 raised to that power is a valid normalized value.
         *        最小的正整数，使得10的该次幂是有效的规范化值。
         */
        static constexpr int min_exponent10 = LDBL_MIN_10_EXP;
    };
}

namespace rainy::utility {
#ifdef __cpp_char8_t
    /**
     * @brief Specialization of numeric_limits for char8_t (C++20).
     *        char8_t 类型的 numeric_limits 特化（C++20）。
     */
    template <>
    class numeric_limits<char8_t> : public implements::number_int_base {
    public:
        /**
         * @brief Returns the minimum finite value of char8_t.
         *        返回 char8_t 的最小有限值。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn(min)() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns the maximum finite value of char8_t.
         *        返回 char8_t 的最大有限值。
         *
         * @return UCHAR_MAX
         */
        RAINY_NODISCARD static constexpr rain_fn(max)() noexcept -> char8_t {
            return UCHAR_MAX;
        }

        /**
         * @brief Returns the most negative finite value (same as min).
         *        返回最负有限值（与min相同）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn lowest() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns epsilon (0 for integral types).
         *        返回机器精度（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn epsilon() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns the maximum rounding error (0 for integral types).
         *        返回最大舍入误差（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn round_error() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns the minimum positive denormalized value (0 for integral types).
         *        返回最小正反规范化值（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn denorm_min() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns infinity representation (0 for integral types).
         *        返回无穷的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn infinity() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns quiet NaN representation (0 for integral types).
         *        返回安静NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn quiet_NaN() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Returns signaling NaN representation (0 for integral types).
         *        返回发信号NaN的表示形式（对于整型为0）。
         *
         * @return 0
         */
        RAINY_NODISCARD static constexpr rain_fn signaling_NaN() noexcept -> char8_t {
            return 0;
        }

        /**
         * @brief Indicates whether the type is modulo (handles overflow by wrapping around).
         *        指示类型是否为模数类型（通过回绕处理溢出）。
         */
        static constexpr bool is_modulo = true;

        /**
         * @brief Number of radix digits that can be represented without change.
         *        可以无变化表示的基数位数。
         */
        static constexpr int digits = 8;

        /**
         * @brief Number of decimal digits that can be represented without change.
         *        可以无变化表示的十进制位数。
         */
        static constexpr int digits10 = 2;
    };
#endif
}

namespace rainy::utility {
    /**
     * @brief Checks if a value is within the valid range for its type.
     *        检查一个值是否在其类型的有效范围内。
     *
     * This function verifies that the given value is not less than the type's
     * lowest representable value and not greater than the type's maximum
     * representable value.
     *
     * 此函数验证给定值不小于该类型的最小可表示值，
     * 且不大于该类型的最大可表示值。
     *
     * @tparam Ty The type of the value to check
     *            要检查的值的类型
     * @param value The value to check
     *              要检查的值
     * @return true if the value is within the valid range for type Ty,
     *         false otherwise
     *         如果值在类型Ty的有效范围内则为true，否则为false
     */
    template <typename Ty>
    constexpr rain_fn is_in_valid_range(Ty value) -> bool {
        return value >= numeric_limits<Ty>::lowest() && value <= (numeric_limits<Ty>::max)();
    }
}

#endif
