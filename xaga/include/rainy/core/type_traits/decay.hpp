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
#ifndef RAINY_CORE_TYPE_TRAITS_DECAY_HPP
#define RAINY_CORE_TYPE_TRAITS_DECAY_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/modifers.hpp>

namespace rainy::type_traits::other_trans {
    /**
     * @brief Type decay template, mimicking the behavior of std::decay.
     *        类型退化模板，模拟 std::decay 的行为。
     *
     * Applies lvalue-to-rvalue, array-to-pointer, and function-to-pointer
     * conversions to type Ty, and removes cv-qualifiers.
     *
     * 对类型 Ty 应用左值到右值、数组到指针、函数到指针的转换，
     * 并移除 cv 限定符。
     *
     * @tparam Ty The type to decay
     *            要退化的类型
     */
    template <typename Ty>
    struct decay {
        using Ty1 = reference_modify::remove_reference_t<Ty>;

        using Ty2 = typename select<implements::_is_function_v<Ty1>>::template apply<
            pointer_modify::add_pointer<Ty1>, cv_modify::remove_cv<std::conditional_t<!implements::_is_function_v<Ty1>, Ty1, void>>>;

        using type =
            typename select<implements::_is_array_v<Ty1>>::template apply<pointer_modify::add_pointer<modifers::remove_extent_t<Ty1>>,
                                                                          Ty2>::type;
    };

    /**
     * @brief Alias template for type decay, providing simplified access.
     *        类型退化模板的别名简化，提供便捷访问。
     *
     * @tparam Ty The type to decay
     *            要退化的类型
     */
    template <typename Ty>
    using decay_t = typename decay<Ty>::type;

    /**
     * @brief Retrieves the underlying integral type of an enum type.
     *        获取枚举类型的底层整数类型。
     *
     * Returns the underlying integer type of enum type Ty.
     *
     * @tparam Ty The enum type
     *            枚举类型
     */
    template <typename Ty>
    struct underlying_type {
        using type = __underlying_type(Ty);
    };

    /**
     * @brief Alias template for underlying type, providing simplified access.
     *        底层类型模板的别名简化，提供便捷访问。
     *
     * @tparam Ty The enum type
     *            枚举类型
     */
    template <typename Ty>
    using underlying_type_t = typename underlying_type<Ty>::type;
}

/**
 * @def RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS
 * @brief Enables bitmask operators for enum classes.
 *        为枚举类启用位掩码操作符。
 *
 * @param EnumType The enum class type name to enable bitmask operations for
 *                 要启用位掩码操作的枚举类类型名
 *
 * @brief
 * This macro generates overloads for bitwise AND, OR, XOR, NOT, and their
 * corresponding assignment operators for the specified enum class.
 * It allows enum classes to be used as bitmasks, similar to traditional
 * C++ enum flags.
 *
 * 该宏为枚举类生成按位与、或、异或、取反以及相应的赋值操作符重载。
 * 使得枚举类可以作为位掩码使用，就像 C++ 中传统的枚举标志位一样。
 *
 * Generated operators include:
 * 生成的操作符包括：
 * - |  : Bitwise OR  / 按位或
 * - &  : Bitwise AND / 按位与
 * - ^  : Bitwise XOR / 按位异或
 * - ~  : Bitwise NOT / 按位取反
 * - |= : Bitwise OR assignment  / 按位或赋值
 * - &= : Bitwise AND assignment / 按位与赋值
 * - ^= : Bitwise XOR assignment / 按位异或赋值
 *
 * Usage example:
 * 使用示例：
 * @code
 * enum class MyFlags { A = 1 << 0, B = 1 << 1, C = 1 << 2 };
 * RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(MyFlags)
 *
 * MyFlags flags = MyFlags::A | MyFlags::B;  // Bitwise combination allowed
 *                                            // 允许按位组合
 * flags &= ~MyFlags::A;                      // Bit operations allowed
 *                                            // 允许位操作
 * @endcode
 */
#define RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(EnumType)                                                                           \
    inline constexpr EnumType operator|(EnumType left, EnumType right) {                                                              \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(left) | static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator&(EnumType left, EnumType right) {                                                              \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(left) & static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator^(EnumType left, EnumType right) {                                                              \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(left) ^ static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator~(EnumType val) {                                                                               \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(~static_cast<type>(val));                                                                        \
    }                                                                                                                                 \
    inline constexpr EnumType &operator|=(EnumType &left, EnumType right) {                                                           \
        left = left | right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline constexpr EnumType &operator&=(EnumType &left, EnumType right) {                                                           \
        left = left & right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline constexpr EnumType &operator^=(EnumType &left, EnumType right) {                                                           \
        left = left ^ right;                                                                                                          \
        return left;                                                                                                                  \
    }

#endif