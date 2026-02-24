/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_CORE_TYPETRAITS_HPP
#define RAINY_CORE_TYPETRAITS_HPP
#include <iterator>
#include <rainy/core/implements/basic_algorithm.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/helper.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/type_traits/meta_types.hpp>
#include <rainy/core/type_traits/modifers.hpp>
#include <rainy/core/type_traits/ranges_traits.hpp>
#include <rainy/core/type_traits/type_list.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/core/type_traits/value_list.hpp>
#include <utility>
#if RAINY_USING_GCC
#include <rainy/core/gnu/typetraits.hpp>
#endif

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

namespace rainy::type_traits::primary_types {
    /**
     * @brief Type identity template.
     *        Simple identity template whose type member is the template parameter Ty itself.
     *        Commonly used in template metaprogramming to suppress template argument deduction.
     *
     *        类型标识模板。
     *        简单的标识模板，其 type 成员就是模板参数 Ty 本身。
     *        常用于模板元编程中抑制模板参数推导。
     *
     * @tparam Ty Any type
     *            任意类型
     */
    template <typename Ty>
    struct type_identity {
        using type = Ty;
    };

    /**
     * @brief Alias template for type identity, providing simplified access.
     *        类型标识模板的别名简化，提供便捷访问。
     *
     * @tparam Ty Any type
     *            任意类型
     */
    template <typename Ty>
    using type_identity_t = typename type_identity<Ty>::type;

    /**
     * @brief Variable template for checking if a type is void.
     *        Checks whether type Ty is void (including cv-qualified void).
     *
     *        判断类型是否为 void 的变量模板。
     *        检查类型 Ty 是否为 void 类型（包括 cv 限定的 void）。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = type_relations::is_same_v<void, Ty>;

    /**
     * @brief Type template for checking if a type is void.
     *        Inherits from bool_constant, providing ::value member constant indicating whether Ty is void.
     *
     *        判断类型是否为 void 的类型模板。
     *        继承自 bool_constant，提供 ::value 成员常量表示 Ty 是否为 void。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a specialization of a template.
     *        Detects whether Type is an instantiation of Template.
     *
     *        检索类型是否具有特化的变量模板。
     *        检测 Type 是否为 Template 的实例化。
     *
     * @tparam Type The type to check
     *              要检查的类型
     * @tparam Template The template to check against
     *                  要检查的特化模板
     */
    template <typename Type, template <typename...> typename Template>
    RAINY_CONSTEXPR_BOOL is_specialization_v = false;

    template <template <typename...> typename Template, typename... Types>
    RAINY_CONSTEXPR_BOOL is_specialization_v<Template<Types...>, Template> = true;

    /**
     * @brief Type template for checking if a type is a specialization of a template.
     *        Inherits from bool_constant based on is_specialization_v.
     *
     *        检索类型是否具有特化的类型模板。
     *        基于 is_specialization_v 继承自 bool_constant。
     *
     * @tparam Type The type to check
     *              要检查的类型
     * @tparam Template The template to check against
     *                  要检查的特化模板
     */
    template <typename Type, template <typename...> typename Template>
    struct is_specialization : helper::bool_constant<is_specialization_v<Type, Template>> {};

    /**
     * @brief Variable template for checking if a type is an enumeration.
     *        Detects whether Ty is an enum type.
     *
     *        判断类型是否为枚举类型的变量模板。
     *        检测 Ty 是否为枚举类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_enum_v = type_traits::implements::_is_enum_v<Ty>;

    /**
     * @brief Type template for checking if a type is an enumeration.
     *        Inherits from bool_constant based on is_enum_v.
     *
     *        判断类型是否为枚举类型的类型模板。
     *        基于 is_enum_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_enum : helper::bool_constant<is_enum_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is POD (Plain Old Data).
     *        POD types are both standard layout and trivial.
     *
     *        判断类型是否为 POD（Plain Old Data）类型的变量模板。
     *        POD 类型满足标准布局且是平凡类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;

    /**
     * @brief Variable template for checking if a type is integral.
     *        Detects whether Ty is an integral type.
     *
     *        判断类型是否为整型的变量模板。
     *        检测 Ty 是否为整型类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v = implements::is_integral_v<Ty>;

    /**
     * @brief Type template for checking if a type is integral.
     *        Inherits from bool_constant based on is_integral_v.
     *
     *        判断类型是否为整型的类型模板。
     *        基于 is_integral_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_integral : helper::bool_constant<is_integral_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is floating point.
     *        Detects whether Ty is a floating point type.
     *
     *        判断类型是否为浮点类型的变量模板。
     *        检测 Ty 是否为浮点类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = type_traits::implements::is_floating_point_v<Ty>;

    /**
     * @brief Type template for checking if a type is floating point.
     *        Inherits from bool_constant based on is_floating_point_v.
     *
     *        判断类型是否为浮点类型的类型模板。
     *        基于 is_floating_point_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_floating_point : helper::bool_constant<is_floating_point_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a union.
     *        Detects whether Ty is a union type.
     *
     *        判断类型是否为联合体（union）的变量模板。
     *        检测 Ty 是否为联合体类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_union_v = implements::is_union_v<Ty>;

    /**
     * @brief Type template for checking if a type is a union.
     *        Inherits from bool_constant based on is_union_v.
     *
     *        判断类型是否为联合体（union）的类型模板。
     *        基于 is_union_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_union : helper::bool_constant<is_union_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a class.
     *        Detects whether Ty is a class type.
     *
     *        判断类型是否为类的变量模板。
     *        检测 Ty 是否为类类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_class_v = implements::is_class_v<Ty>;

    /**
     * @brief Type template for checking if a type is a class.
     *        Inherits from bool_constant based on is_class_v.
     *
     *        判断类型是否为类的类型模板。
     *        基于 is_class_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_class : helper::bool_constant<implements::is_class_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a function.
     *        Detects whether Ty is a function type.
     *
     *        判断类型是否为函数的变量模板。
     *        检测 Ty 是否为函数类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_function_v = type_traits::implements::_is_function_v<Ty>;

    /**
     * @brief Type template for checking if a type is a function.
     *        Inherits from bool_constant based on is_function_v.
     *
     *        判断类型是否为函数的类型模板。
     *        基于 is_function_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_function : helper::bool_constant<type_traits::implements::_is_function_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is an lvalue reference (primary template).
     *        Primary template defaults to false.
     *
     *        判断类型是否为左值引用的变量模板（主模板）。
     *        主模板默认为 false。
     */
    template <typename>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v = false;

    /**
     * @brief Variable template for checking if a type is an lvalue reference (lvalue reference specialization).
     *        Specialization for lvalue reference types.
     *
     *        判断类型是否为左值引用的变量模板（左值引用特化）。
     *        左值引用类型的特化。
     *
     * @tparam Ty The referenced type
     *            被引用的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v<Ty &> = true;

    /**
     * @brief Type template for checking if a type is an lvalue reference.
     *        Inherits from bool_constant based on is_lvalue_reference_v.
     *
     *        判断类型是否为左值引用的类型模板。
     *        基于 is_lvalue_reference_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_lvalue_reference : helper::bool_constant<is_lvalue_reference_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is an rvalue reference (primary template).
     *        Primary template defaults to false.
     *
     *        判断类型是否为右值引用的变量模板（主模板）。
     *        主模板默认为 false。
     */
    template <typename>
    RAINY_CONSTEXPR_BOOL is_rvalue_reference_v = false;

    /**
     * @brief Variable template for checking if a type is an rvalue reference (rvalue reference specialization).
     *        Specialization for rvalue reference types.
     *
     *        判断类型是否为右值引用的变量模板（右值引用特化）。
     *        右值引用类型的特化。
     *
     * @tparam Ty The referenced type
     *            被引用的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_rvalue_reference_v<Ty &&> = true;

    /**
     * @brief Type template for checking if a type is an rvalue reference.
     *        Inherits from bool_constant based on is_rvalue_reference_v.
     *
     *        判断类型是否为右值引用的类型模板。
     *        基于 is_rvalue_reference_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_rvalue_reference : helper::bool_constant<is_rvalue_reference_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is an array.
     *        Detects whether Ty is an array type.
     *
     *        判断类型是否为数组的变量模板。
     *        检测 Ty 是否为数组类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_array_v = type_traits::implements::_is_array_v<Ty>;

    /**
     * @brief Type template for checking if a type is an array.
     *        Inherits from bool_constant based on is_array_v.
     *
     *        判断类型是否为数组的类型模板。
     *        基于 is_array_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_array : helper::bool_constant<is_array_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a pointer.
     *        Detects whether Ty is a pointer type.
     *
     *        判断类型是否为指针的变量模板。
     *        检测 Ty 是否为指针类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_v = type_traits::implements::_is_pointer_v<Ty>;

    /**
     * @brief Type template for checking if a type is a pointer.
     *        Inherits from bool_constant based on is_pointer_v.
     *
     *        判断类型是否为指针的类型模板。
     *        基于 is_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_pointer : helper::bool_constant<is_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a pointer reference (primary template).
     *        Checks whether the type is an lvalue or rvalue reference to a pointer.
     *
     *        判断类型是否为指针引用的变量模板（主模板）。
     *        检查类型是否为指向指针的左值或右值引用。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v = false;

    /**
     * @brief Variable template for checking if a type is a pointer reference (lvalue pointer reference specialization).
     *        Specialization for lvalue references to pointers.
     *
     *        判断类型是否为指针引用的变量模板（左值指针引用特化）。
     *        指向指针的左值引用特化。
     *
     * @tparam Ty The type pointed to
     *            指针指向的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v<Ty *&> = true;

    /**
     * @brief Variable template for checking if a type is a pointer reference (rvalue pointer reference specialization).
     *        Specialization for rvalue references to pointers.
     *
     *        判断类型是否为指针引用的变量模板（右值指针引用特化）。
     *        指向指针的右值引用特化。
     *
     * @tparam Ty The type pointed to
     *            指针指向的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v<Ty *&&> = true;

    /**
     * @brief Variable template for checking if a type is std::nullptr_t (primary template).
     *        Primary template defaults to false.
     *
     *        判断类型是否为空指针类型 std::nullptr_t 的变量模板（主模板）。
     *        主模板默认为 false。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v = false;

    /**
     * @brief Variable template for checking if a type is std::nullptr_t (specialization).
     *        Specialization for std::nullptr_t.
     *
     *        判断类型是否为空指针类型 std::nullptr_t 的变量模板（特化）。
     *        std::nullptr_t 的特化。
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v<std::nullptr_t> = true;

    /**
     * @brief Type template for checking if a type is std::nullptr_t.
     *        Inherits from bool_constant based on is_null_pointer_v.
     *
     *        判断类型是否为空指针类型的类型模板。
     *        基于 is_null_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_null_pointer : helper::bool_constant<is_null_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a pointer to member object.
     *        Member object pointers point to non-static data members of a class.
     *
     *        判断类型是否为成员对象指针的变量模板。
     *        成员对象指针指向类的非静态数据成员。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
#if RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = __is_member_object_pointer(Ty);
#else
    /**
     * @brief Variable template for checking if a type is a pointer to member object.
     *        Member object pointers point to non-static data members of a class.
     *
     *        判断类型是否为成员对象指针的变量模板。
     *        成员对象指针指向类的非静态数据成员。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = implements::is_member_object_pointer_<cv_modify::remove_cv_t<Ty>>::value;
#endif

    /**
     * @brief Type template for checking if a type is a pointer to member object.
     *        Inherits from bool_constant based on is_member_object_pointer_v.
     *
     *        判断类型是否为成员对象指针的类型模板。
     *        基于 is_member_object_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_member_object_pointer : helper::bool_constant<is_member_object_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a pointer to member function.
     *        Member function pointers point to non-static member functions of a class.
     *
     *        判断类型是否为成员函数指针的变量模板。
     *        成员函数指针指向类的非静态成员函数。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_function_pointer_v =
        implements::is_member_function_pointer_helper<cv_modify::remove_cv_t<Ty>>::value;

    /**
     * @brief Type template for checking if a type is a pointer to member function.
     *        Inherits from bool_constant based on is_member_function_pointer_v.
     *
     *        判断类型是否为成员函数指针的类型模板。
     *        基于 is_member_function_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_member_function_pointer : helper::bool_constant<is_member_function_pointer_v<Ty>> {};

    /**
     * @brief Variable template for retrieving array size (primary template).
     *        Returns 0 for non-array types.
     *
     *        获取数组大小的变量模板（主模板）。
     *        对于非数组类型返回 0。
     *
     * @tparam Ty Type (array or non-array)
     *            类型（数组或非数组）
     */
    template <typename Ty>
    static RAINY_INLINE_CONSTEXPR std::size_t array_size_v = 0;

    /**
     * @brief Variable template for retrieving array size (array specialization).
     *        Returns the size of the array.
     *
     *        获取数组大小的变量模板（数组特化）。
     *        返回数组的大小。
     *
     * @tparam Ty Array element type
     *            数组元素类型
     * @tparam N Array size
     *            数组大小
     */
    template <typename Ty, std::size_t N>
    static RAINY_INLINE_CONSTEXPR std::size_t array_size_v<Ty[N]> = N;

    /**
     * @brief Type template for retrieving array size.
     *        Provides ::value member constant with array size (0 for non-arrays).
     *
     *        获取数组大小的类型模板。
     *        通过 ::value 成员常量获取数组的大小，非数组类型返回 0。
     *
     * @tparam Ty Type (array or non-array)
     *            类型（可以是数组或非数组）
     */
    template <typename Ty>
    struct array_size : helper::integral_constant<std::size_t, array_size_v<Ty>> {};
}

namespace rainy::utility {
    using core::builtin::addressof;
    using core::builtin::construct_at;
    using core::builtin::forward;

    /**
     * @brief Constructs an object in-place at the given memory location.
     *        在给定的内存位置就地构造对象。
     *
     * @tparam Ty Type of object to construct
     *            要构造的对象类型
     * @tparam Args Constructor argument types
     *               构造函数参数类型
     * @param object Reference to memory location where object will be constructed
     *               指向将要构造对象的内存位置的引用
     * @param args Constructor arguments to forward
     *              要转发的构造函数参数
     * @throws noexcept if Ty's constructor is noexcept
     *                  如果 Ty 的构造函数是 noexcept 则不抛出异常
     */
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 rain_fn
    construct_in_place(Ty &object, Args &&...args) noexcept(type_traits::implements::is_nothrow_constructible_v<Ty, Args...>) -> void {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            std::construct_at(utility::addressof(object), utility::forward<Args>(args)...);
            return;
        }
#endif
        ::new (static_cast<void *>(utility::addressof(object))) Ty(utility::forward<Args>(args)...);
    }

    /**
     * @brief Destroys an object at the given pointer location.
     *        销毁给定指针位置的对象。
     *
     * @tparam Ty Type of object to destroy
     *            要销毁的对象类型
     * @param ptr Pointer to the object to destroy
     *            指向要销毁对象的指针
     * @throws noexcept Always noexcept
     *                  始终不抛出异常
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn destroy_at(Ty *ptr) noexcept -> void {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            std::destroy_at(ptr);
            return;
        }
#endif
        if constexpr (!type_traits::implements::_is_trivially_destructible_v<Ty>) {
            ptr->~Ty();
        }
    }

    /**
     * @brief Destroys a range of objects from first to last.
     *        销毁从 first 到 last 范围内的对象。
     *
     * @tparam Ty Type of objects to destroy
     *            要销毁的对象类型
     * @param first Pointer to the first object in the range
     *              指向范围内第一个对象的指针
     * @param last Pointer to one past the last object in the range
     *             指向范围内最后一个对象之后位置的指针
     * @throws noexcept Always noexcept
     *                  始终不抛出异常
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn destroy_range(Ty *first, Ty *last) noexcept -> void {
        if constexpr (!type_traits::implements::_is_trivially_destructible_v<Ty>) {
            for (Ty *ptr = first; ptr != last; ++ptr) {
                destroy_at(ptr);
            }
        }
    }

    /**
     * @brief Destroys a range of objects (alias for destroy_range).
     *        销毁范围内的对象（destroy_range 的别名）。
     *
     * @tparam Ty Type of objects to destroy
     *            要销毁的对象类型
     * @param first Pointer to the first object in the range
     *              指向范围内第一个对象的指针
     * @param last Pointer to one past the last object in the range
     *             指向范围内最后一个对象之后位置的指针
     * @throws noexcept Always noexcept
     *                  始终不抛出异常
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn destroy(Ty *first, Ty *last) noexcept -> void {
        destroy_range(first, last);
    }

}

namespace rainy::type_traits::logical_traits {
    /**
     * @brief Logical conjunction trait (AND) for type traits.
     *        Primary template for empty parameter pack yields true_type.
     *
     *        类型特征的逻辑与（AND） traits。
     *        空参数包的主模板生成 true_type。
     *
     * @tparam ... Parameter pack of type traits with ::value members
     *             具有 ::value 成员的类型特征参数包
     */
    template <typename...>
    struct conjunction : helper::true_type {};

    /**
     * @brief Logical conjunction trait (AND) for type traits.
     *        Recursive template that evaluates the conjunction of all traits.
     *
     *        类型特征的逻辑与（AND） traits。
     *        递归模板，计算所有特征的逻辑与。
     *
     * @tparam First First trait to evaluate
     *               要评估的第一个特征
     * @tparam Rest Remaining traits to evaluate
     *              要评估的剩余特征
     */
    template <typename First, typename... Rest>
    struct conjunction<First, Rest...> : implements::_conjunction<First::value, First, Rest...>::type {};

    /**
     * @brief Variable template for logical conjunction.
     *        Provides the value of conjunction<Traits...>::value.
     *
     *        逻辑与的变量模板。
     *        提供 conjunction<Traits...>::value 的值。
     *
     * @tparam Traits Type traits to evaluate
     *                要评估的类型特征
     */
    template <typename... Traits>
    RAINY_CONSTEXPR_BOOL conjunction_v = conjunction<Traits...>::value;

    /**
     * @brief Logical disjunction trait (OR) for type traits.
     *        Primary template for empty parameter pack yields false_type.
     *
     *        类型特征的逻辑或（OR） traits。
     *        空参数包的主模板生成 false_type。
     *
     * @tparam ... Parameter pack of type traits with ::value members
     *             具有 ::value 成员的类型特征参数包
     */
    template <typename...>
    struct disjunction : helper::false_type {};

    /**
     * @brief Logical disjunction trait (OR) for type traits.
     *        Recursive template that evaluates the disjunction of all traits.
     *
     *        类型特征的逻辑或（OR） traits。
     *        递归模板，计算所有特征的逻辑或。
     *
     * @tparam first First trait to evaluate
     *               要评估的第一个特征
     * @tparam rest Remaining traits to evaluate
     *              要评估的剩余特征
     */
    template <typename first, typename... rest>
    struct disjunction<first, rest...> : implements::_disjunction<first::value, first, rest...>::type {};

    /**
     * @brief Variable template for logical disjunction.
     *        Provides the value of disjunction<traits...>::value.
     *
     *        逻辑或的变量模板。
     *        提供 disjunction<traits...>::value 的值。
     *
     * @tparam traits Type traits to evaluate
     *                要评估的类型特征
     */
    template <typename... traits>
    RAINY_CONSTEXPR_BOOL disjunction_v = disjunction<traits...>::value;

    /**
     * @brief Logical negation trait (NOT) for a single type trait.
     *        Returns the opposite boolean value of the input trait.
     *
     *        单个类型特征的逻辑非（NOT） traits。
     *        返回输入特征的反向布尔值。
     *
     * @tparam trait Type trait with ::value member to negate
     *               要取反的具有 ::value 成员的类型特征
     */
    template <typename trait>
    struct negation : helper::bool_constant<!static_cast<bool>(trait::value)> {};

    /**
     * @brief Variable template for logical negation.
     *        Provides the value of negation<trait>::value.
     *
     *        逻辑非的变量模板。
     *        提供 negation<trait>::value 的值。
     *
     * @tparam trait Type trait to negate
     *               要取反的类型特征
     */
    template <typename trait>
    RAINY_CONSTEXPR_BOOL negation_v = negation<trait>::value;
}

namespace rainy::utility {
    /**
     * @brief Placeholder type for template metaprogramming and function argument tagging.
     *        Used to indicate a position to be filled or as a default argument.
     *
     *        用于模板元编程和函数参数标记的占位符类型。
     *        用于指示需要填充的位置或作为默认参数。
     */
    struct placeholder_t final {
        explicit placeholder_t() = default;
    };

    /**
     * @brief Global constexpr instance of placeholder_t.
     *        placeholder_t 的全局 constexpr 实例。
     */
    constexpr placeholder_t placeholder{};

    /**
     * @brief Type-parameterized placeholder template.
     *        Allows creating placeholders that carry type information.
     *
     *        类型参数化的占位符模板。
     *        允许创建携带类型信息的占位符。
     *
     * @tparam Ty Type to associate with this placeholder (defaults to void)
     *            与此占位符关联的类型（默认为 void）
     */
    template <typename = void>
    struct placeholder_type_t final {
        explicit placeholder_type_t() = default;
    };

    /**
     * @brief Global constexpr instance of placeholder_type_t<Ty>.
     *        placeholder_type_t<Ty> 的全局 constexpr 实例。
     *
     * @tparam Ty Type associated with the placeholder
     *            与占位符关联的类型
     */
    template <typename Ty>
    constexpr placeholder_type_t<Ty> placeholder_type{};

    /**
     * @brief In-place construction tag with index.
     *        Used to disambiguate constructors that take an index parameter.
     *
     *        带索引的就地构造标签。
     *        用于区分接受索引参数的构造函数。
     *
     * @tparam Idx Index value for tag specialization
     *             标签特化的索引值
     */
    template <std::size_t>
    struct in_place_index_t final {
        explicit in_place_index_t() = default;
    };

    /**
     * @brief Global constexpr instance of in_place_index_t<Idx>.
     *        in_place_index_t<Idx> 的全局 constexpr 实例。
     *
     * @tparam Idx Index value
     *             索引值
     */
    template <std::size_t Idx>
    constexpr in_place_index_t<Idx> in_place_index{};

    /**
     * @brief Variable template to check if a type is a specialization of in_place_index_t.
     *        Primary template defaults to false.
     *
     *        检查类型是否为 in_place_index_t 特化的变量模板。
     *        主模板默认为 false。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <class>
    constexpr bool is_in_place_index_specialization = false;

    /**
     * @brief Variable template to check if a type is a specialization of in_place_index_t.
     *        Specialization for in_place_index_t types.
     *
     *        检查类型是否为 in_place_index_t 特化的变量模板。
     *        in_place_index_t 类型的特化。
     *
     * @tparam Idx Index value
     *             索引值
     */
    template <std::size_t Idx>
    constexpr bool is_in_place_index_specialization<utility::in_place_index_t<Idx>> = true;

    /**
     * @brief Alias for std::allocator_arg_t.
     *        Tag type for allocator construction disambiguation.
     *
     *        std::allocator_arg_t 的别名。
     *        用于分配器构造歧义消除的标签类型。
     */
    using allocator_arg_t = std::allocator_arg_t;

    /**
     * @brief Global constexpr instance of allocator_arg_t.
     *        allocator_arg_t 的全局 constexpr 实例。
     */
    inline constexpr allocator_arg_t allocator_arg{};
}

namespace rainy::type_traits::implements::swap_adl {
    using std::swap; // ADL

    template <typename Ty, typename UTy, typename = void>
    struct is_swappable_with_impl : helper::false_type {};

    template <typename Ty, typename UTy>
    struct is_swappable_with_impl<Ty, UTy,
                                  other_trans::void_t<decltype(swap(utility::declval<Ty>(), utility::declval<UTy>())),
                                                      decltype(swap(utility::declval<UTy>(), utility::declval<Ty>()))>>
        : helper::true_type {};

    template <typename Ty, typename UTy>
    struct is_nothrow_swappable_with_impl {
        static constexpr bool value = noexcept(swap(utility::declval<Ty>(), utility::declval<UTy>())) &&
                                      noexcept(swap(utility::declval<UTy>(), utility::declval<Ty>()));
    };
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Primary template for checking if From is convertible to To for invoke operations.
     *        Defaults to false_type.
     *
     *        检查 From 类型是否可以转换为 To 类型用于调用操作的主模板。
     *        默认为 false_type。
     *
     * @tparam From Source type
     *              源类型
     * @tparam To Target type
     *            目标类型
     */
    template <typename From, typename To, typename = void>
    struct is_invoke_convertible : helper::false_type {};

    /**
     * @brief Specialization for when conversion is possible.
     *        Detected via fake_copy_init with returns_exactly.
     *
     *        当转换可能时的特化。
     *        通过 fake_copy_init 和 returns_exactly 检测。
     *
     * @tparam From Source type
     *              源类型
     * @tparam To Target type
     *            目标类型
     */
    template <typename From, typename To>
    struct is_invoke_convertible<From, To,
                                 other_trans::void_t<decltype(implements::fake_copy_init<To>(implements::returns_exactly<From>()))>>
        : helper::true_type {};

    /**
     * @brief Variable template for invoke convertibility check.
     *        Provides the value of is_invoke_convertible<From, To>::value.
     *
     *        调用转换检查的变量模板。
     *        提供 is_invoke_convertible<From, To>::value 的值。
     *
     * @tparam From Source type
     *              源类型
     * @tparam To Target type
     *            目标类型
     */
    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_convertible_v = is_invoke_convertible<From, To>::value;

    /**
     * @brief Checks if conversion from From to To is noexcept for invoke operations.
     *
     *        检查 From 到 To 的转换对于调用操作是否为 noexcept。
     *
     * @tparam From Source type
     *              源类型
     * @tparam To Target type
     *            目标类型
     */
    template <typename From, typename To>
    struct is_invoke_nothrow_convertible
        : helper::bool_constant<noexcept(implements::fake_copy_init<To>(implements::returns_exactly<From>()))> {};

    /**
     * @brief Variable template for noexcept invoke convertibility check.
     *        Provides the value of is_invoke_nothrow_convertible<From, To>::value.
     *
     *        noexcept 调用转换检查的变量模板。
     *        提供 is_invoke_nothrow_convertible<From, To>::value 的值。
     *
     * @tparam From Source type
     *              源类型
     * @tparam To Target type
     *            目标类型
     */
    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_nothrow_convertible_v = is_invoke_convertible<From, To>::value;

    /**
     * @brief Variable template for checking if a type is complete.
     *        Primary template defaults to false.
     *
     *        检查类型是否完整的变量模板。
     *        主模板默认为 false。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_complete_v = false;

    /**
     * @brief Variable template for checking if a type is complete.
     *        Specialization that detects completeness via sizeof.
     *
     *        检查类型是否完整的变量模板。
     *        通过 sizeof 检测完整性的特化。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_complete_v<Ty, type_traits::other_trans::void_t<decltype(sizeof(Ty))>> = true;

    /**
     * @brief Type template for checking if a type is complete.
     *        Inherits from bool_constant based on is_complete_v.
     *
     *        检查类型是否完整的类型模板。
     *        基于 is_complete_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_complete : helper::bool_constant<is_complete_v<Ty>> {};

    /**
     * @brief Checks if types Ty and Uy are swappable with each other.
     *
     *        检查类型 Ty 和 Uy 是否可以相互交换。
     *
     * @tparam Ty First type
     *            第一个类型
     * @tparam Uy Second type
     *            第二个类型
     */
    template <typename Ty, typename Uy>
    struct is_swappable_with : implements::swap_adl::is_swappable_with_impl<Ty, Uy> {};

    /**
     * @brief Variable template for swappable_with check.
     *        Provides the value of is_swappable_with<Ty, Uy>::value.
     *
     *        swappable_with 检查的变量模板。
     *        提供 is_swappable_with<Ty, Uy>::value 的值。
     *
     * @tparam Ty First type
     *            第一个类型
     * @tparam Uy Second type
     *            第二个类型
     */
    template <typename Ty, typename Uy>
    inline constexpr bool is_swappable_with_v = is_swappable_with<Ty, Uy>::value;

    /**
     * @brief Checks if types Ty and Uy are swappable with each other and the swap is noexcept.
     *
     *        检查类型 Ty 和 Uy 是否可以相互交换且交换操作是 noexcept。
     *
     * @tparam Ty First type
     *            第一个类型
     * @tparam Uy Second type
     *            第二个类型
     */
    template <typename Ty, typename Uy>
    struct is_nothrow_swappable_with
        : helper::bool_constant<is_swappable_with_v<Ty, Uy> && implements::swap_adl::is_nothrow_swappable_with_impl<Ty, Uy>::value> {};

    /**
     * @brief Variable template for nothrow_swappable_with check.
     *        Provides the value of is_nothrow_swappable_with<Ty, Uy>::value.
     *
     *        nothrow_swappable_with 检查的变量模板。
     *        提供 is_nothrow_swappable_with<Ty, Uy>::value 的值。
     *
     * @tparam Ty First type
     *            第一个类型
     * @tparam Uy Second type
     *            第二个类型
     */
    template <typename Ty, typename Uy>
    inline constexpr bool is_nothrow_swappable_with_v = is_nothrow_swappable_with<Ty, Uy>::value;

    /**
     * @brief Checks if type Ty is swappable with itself.
     *        Uses lvalue references to Ty for the check.
     *
     *        检查类型 Ty 是否可以与自身交换。
     *        使用 Ty 的左值引用进行检查。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_swappable
        : is_swappable_with<reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<Ty>> {};

    /**
     * @brief Variable template for swappable check.
     *        Provides the value of is_swappable<Ty>::value.
     *
     *        swappable 检查的变量模板。
     *        提供 is_swappable<Ty>::value 的值。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool is_swappable_v = is_swappable<Ty>::value;

    /**
     * @brief Checks if type Ty is swappable with itself and the swap is noexcept.
     *        Uses lvalue references to Ty for the check.
     *
     *        检查类型 Ty 是否可以与自身交换且交换操作是 noexcept。
     *        使用 Ty 的左值引用进行检查。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_swappable
        : is_nothrow_swappable_with<reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<Ty>> {};

    /**
     * @brief Variable template for nothrow_swappable check.
     *        Provides the value of is_nothrow_swappable<Ty>::value.
     *
     *        nothrow_swappable 检查的变量模板。
     *        提供 is_nothrow_swappable<Ty>::value 的值。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool is_nothrow_swappable_v = is_nothrow_swappable<Ty>::value;

    /**
     * @brief Variable template for checking if type Ty has an ADL (Argument-Dependent Lookup) swap.
     *
     *        检查类型 Ty 是否有 ADL（参数依赖查找）swap 的变量模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_adl_swap_v = implements::has_adl_swap<Ty>;

    /**
     * @brief Type template for checking if type Ty has an ADL swap.
     *        Inherits from bool_constant based on has_adl_swap_v.
     *
     *        检查类型 Ty 是否有 ADL swap 的类型模板。
     *        基于 has_adl_swap_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_adl_swap : helper::bool_constant<has_adl_swap_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a virtual destructor.
     *        Uses compiler built-in __has_virtual_destructor.
     *
     *        检查类型是否具有虚析构函数的变量模板。
     *        使用编译器内建 __has_virtual_destructor。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_virtual_destructor_v = __has_virtual_destructor(Ty);

    /**
     * @brief Type template for checking if a type has a virtual destructor.
     *        Inherits from bool_constant based on has_virtual_destructor_v.
     *
     *        检查类型是否具有虚析构函数的类型模板。
     *        基于 has_virtual_destructor_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_virtual_destructor : helper::bool_constant<has_virtual_destructor_v<Ty>> {};

    /**
     * @brief Variable template for checking if assignment from From to To is trivially assignable.
     *        Uses compiler built-in __is_trivially_assignable.
     *
     *        检查从 From 到 To 的赋值是否可平凡赋值的变量模板。
     *        使用编译器内建 __is_trivially_assignable。
     *
     * @tparam To Target type
     *            目标类型
     * @tparam From Source type
     *              源类型
     */
    template <typename To, typename From>
    RAINY_CONSTEXPR_BOOL is_trivially_assignable = __is_trivially_assignable(reference_modify::add_lvalue_reference_t<To>, From);

    /**
     * @brief Variable template for checking if a type is trivially move assignable.
     *        Uses compiler built-in __is_trivially_assignable.
     *
     *        检查类型是否可平凡移动赋值的变量模板。
     *        使用编译器内建 __is_trivially_assignable。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_move_assignable_v = __is_trivially_assignable(reference_modify::add_lvalue_reference_t<Ty>, Ty);

    /**
     * @brief Type template for checking if a type is trivially move assignable.
     *        Inherits from bool_constant based on is_trivially_move_assignable_v.
     *
     *        检查类型是否可平凡移动赋值的类型模板。
     *        基于 is_trivially_move_assignable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_move_assignable : helper::bool_constant<is_trivially_move_assignable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivially copy assignable.
     *        Uses compiler built-in __is_trivially_assignable with const lvalue reference.
     *
     *        检查类型是否可平凡复制赋值的变量模板。
     *        使用带有 const 左值引用的编译器内建 __is_trivially_assignable。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_copy_assignable_v =
        __is_trivially_assignable(reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<const Ty>);

    /**
     * @brief Type template for checking if a type is trivially copy assignable.
     *        Inherits from bool_constant based on is_trivially_copy_assignable_v.
     *
     *        检查类型是否可平凡复制赋值的类型模板。
     *        基于 is_trivially_copy_assignable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_copy_assignable : helper::bool_constant<is_trivially_copy_assignable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is move assignable.
     *        Uses compiler built-in __is_assignable.
     *
     *        检查类型是否可移动赋值的变量模板。
     *        使用编译器内建 __is_assignable。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    constexpr bool is_move_assignable_v = __is_assignable(reference_modify::add_lvalue_reference_t<Ty>, Ty);

    /**
     * @brief Type template for checking if a type is move assignable.
     *        Inherits from bool_constant based on is_move_assignable_v.
     *
     *        检查类型是否可移动赋值的类型模板。
     *        基于 is_move_assignable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_move_assignable : helper::bool_constant<is_move_assignable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is copy assignable.
     *        Uses compiler built-in __is_assignable with const lvalue reference.
     *
     *        检查类型是否可复制赋值的变量模板。
     *        使用带有 const 左值引用的编译器内建 __is_assignable。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    constexpr bool is_copy_assignable_v =
        __is_assignable(reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<const Ty>);

    /**
     * @brief Type template for checking if a type is copy assignable.
     *        Inherits from bool_constant based on is_copy_assignable_v.
     *
     *        检查类型是否可复制赋值的类型模板。
     *        基于 is_copy_assignable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_copy_assignable : helper::bool_constant<is_copy_assignable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivially copyable.
     *        A trivially copyable type has no significant copy operations, move operations,
     *        or destructors. Generally, if copy operations can be implemented as bitwise copies,
     *        the type is trivially copyable.
     *
     *        检查类型是否为普通复制类型的变量模板。
     *        普通复制类型不具有任何重要的复制操作、移动操作或析构函数。
     *        一般而言，如果复制操作可作为按位复制实现，则可将其视为普通复制。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     * @remark If type Ty is trivially copyable, the instance is true; otherwise false.
     *         Built-in types and arrays of trivially copyable types are trivially copyable.
     *         如果类型 Ty 是普通复制类型，则类型谓词的实例为 true；否则为 false。
     *         内置类型和普通复制类型数组都可进行普通复制。
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_copyable_v = __is_trivially_copyable(Ty);

    /**
     * @brief Type template for checking if a type is trivially copyable.
     *        Inherits from bool_constant based on is_trivially_copyable_v.
     *
     *        检查类型是否为普通复制类型的类型模板。
     *        基于 is_trivially_copyable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     * @remark If type Ty is trivially copyable, the instance is true; otherwise false.
     *         Built-in types and arrays of trivially copyable types are trivially copyable.
     *         如果类型 Ty 是普通复制类型，则类型谓词的实例为 true；否则为 false。
     *         内置类型和普通复制类型数组都可进行普通复制。
     */
    template <typename Ty>
    struct is_trivially_copyable : helper::bool_constant<is_trivially_copyable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has standard layout.
     *        Uses compiler built-in __is_standard_layout.
     *
     *        检查类型是否为标准布局的变量模板。
     *        使用编译器内建 __is_standard_layout。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_standard_layout_v = __is_standard_layout(Ty);

    /**
     * @brief Type template for checking if a type has standard layout.
     *        Inherits from bool_constant based on is_standard_layout_v.
     *
     *        检查类型是否为标准布局的类型模板。
     *        基于 is_standard_layout_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_standard_layout : helper::bool_constant<is_standard_layout_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is POD (Plain Old Data).
     *        POD types are both standard layout and trivial.
     *
     *        检查类型是否为 POD（Plain Old Data）类型的变量模板。
     *        POD 类型满足标准布局且是平凡类型。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;
    /**
     * @brief Type template for checking if a type is POD (Plain Old Data).
     *        Inherits from bool_constant based on is_pod_v.
     *
     *        检查类型是否为 POD（Plain Old Data）类型的类型模板。
     *        基于 is_pod_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_pod : helper::bool_constant<is_pod_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is empty.
     *        An empty type has no non-static data members except bit-fields of size 0,
     *        no virtual functions, no virtual base classes, and no non-empty base classes.
     *        Uses compiler built-in __is_empty.
     *
     *        检查类型是否为空类型的变量模板。
     *        空类型没有非静态数据成员（除了大小为0的位域），没有虚函数，没有虚基类，
     *        也没有非空基类。使用编译器内建 __is_empty。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_empty_v = __is_empty(Ty);

    /**
     * @brief Type template for checking if a type is empty.
     *        Inherits from bool_constant based on is_empty_v.
     *
     *        检查类型是否为空类型的类型模板。
     *        基于 is_empty_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_empty : helper::bool_constant<is_empty_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is polymorphic.
     *        A polymorphic type has at least one virtual function.
     *        Uses compiler built-in __is_polymorphic.
     *
     *        检查类型是否为多态类型的变量模板。
     *        多态类型至少有一个虚函数。使用编译器内建 __is_polymorphic。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_polymorphic_v = __is_polymorphic(Ty);

    /**
     * @brief Type template for checking if a type is polymorphic.
     *        Inherits from bool_constant based on is_polymorphic_v.
     *
     *        检查类型是否为多态类型的类型模板。
     *        基于 is_polymorphic_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_polymorphic : helper::bool_constant<is_polymorphic_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is abstract.
     *        An abstract type has at least one pure virtual function.
     *        Uses compiler built-in __is_abstract.
     *
     *        检查类型是否为抽象类型的变量模板。
     *        抽象类型至少有一个纯虚函数。使用编译器内建 __is_abstract。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_abstract_v = __is_abstract(Ty);

    /**
     * @brief Type template for checking if a type is abstract.
     *        Inherits from bool_constant based on is_abstract_v.
     *
     *        检查类型是否为抽象类型的类型模板。
     *        基于 is_abstract_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_abstract : helper::bool_constant<is_abstract_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is final.
     *        A final type cannot be derived from.
     *        Uses compiler built-in __is_final.
     *
     *        检查类型是否为 final 类型的变量模板。
     *        final 类型不能被继承。使用编译器内建 __is_final。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_final_v = __is_final(Ty);

    /**
     * @brief Type template for checking if a type is final.
     *        Inherits from bool_constant based on is_final_v.
     *
     *        检查类型是否为 final 类型的类型模板。
     *        基于 is_final_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_final : helper::bool_constant<is_final_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is an aggregate.
     *        An aggregate is an array or a class with no user-declared constructors,
     *        no private or protected non-static data members, no virtual functions,
     *        and no virtual, private, or protected base classes.
     *        Uses compiler built-in __is_aggregate.
     *
     *        检查类型是否为聚合类型的变量模板。
     *        聚合类型是数组或没有用户声明的构造函数、没有私有或受保护的非静态数据成员、
     *        没有虚函数、没有虚基类、私有基类或受保护基类的类。
     *        使用编译器内建 __is_aggregate。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_aggregate_v = __is_aggregate(Ty);

    /**
     * @brief Type template for checking if a type is an aggregate.
     *        Inherits from bool_constant based on is_aggregate_v.
     *
     *        检查类型是否为聚合类型的类型模板。
     *        基于 is_aggregate_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_aggregate : helper::bool_constant<is_aggregate_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is signed.
     *        Uses implements::sign_base to determine signedness.
     *
     *        检查类型是否为有符号类型的变量模板。
     *        使用 implements::sign_base 判断是否有符号。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_signed_v = implements::sign_base<Ty>::is_signed;

    /**
     * @brief Type template for checking if a type is signed.
     *        Inherits from bool_constant based on is_signed_v.
     *
     *        检查类型是否为有符号类型的类型模板。
     *        基于 is_signed_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_signed : helper::bool_constant<is_signed_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is unsigned.
     *        Uses implements::sign_base to determine signedness.
     *
     *        检查类型是否为无符号类型的变量模板。
     *        使用 implements::sign_base 判断是否无符号。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unsigned_v = implements::sign_base<Ty>::is_unsigned;

    /**
     * @brief Type template for checking if a type is unsigned.
     *        Inherits from bool_constant based on is_unsigned_v.
     *
     *        检查类型是否为无符号类型的类型模板。
     *        基于 is_unsigned_v 继承自 bool_constant。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_unsigned : helper::bool_constant<is_unsigned_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is constructible with given arguments.
     *        Uses compiler built-in __is_constructible.
     *
     *        检查类型是否可以使用给定参数构造的变量模板。
     *        使用编译器内建 __is_constructible。
     *
     * @tparam Ty Type to construct
     *            要构造的类型
     * @tparam Args Argument types for construction
     *              构造参数类型
     */
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_constructible_v = __is_constructible(Ty, Args...);

    /**
     * @brief Type template for checking if a type is constructible with given arguments.
     *        Inherits from bool_constant based on is_constructible_v.
     *
     *        检查类型是否可以使用给定参数构造的类型模板。
     *        基于 is_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to construct
     *            要构造的类型
     * @tparam Args Argument types for construction
     *              构造参数类型
     */
    template <typename Ty, typename... Args>
    struct is_constructible : helper::bool_constant<is_constructible_v<Ty, Args...>> {};

    /**
     * @brief Variable template for checking if a type is copy constructible.
     *        Checks construction from const lvalue reference.
     *
     *        检查类型是否可复制构造的变量模板。
     *        检查从 const 左值引用构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copy_constructible_v = __is_constructible(Ty, reference_modify::add_lvalue_reference_t<const Ty>);

    /**
     * @brief Type template for checking if a type is copy constructible.
     *        Inherits from bool_constant based on is_copy_constructible_v.
     *
     *        检查类型是否可复制构造的类型模板。
     *        基于 is_copy_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_copy_constructible : helper::bool_constant<is_copy_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is default constructible.
     *        Checks construction with no arguments.
     *
     *        检查类型是否可默认构造的变量模板。
     *        检查无参数构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_default_constructible_v = __is_constructible(Ty);

    /**
     * @brief Type template for checking if a type is default constructible.
     *        Inherits from bool_constant based on is_default_constructible_v.
     *
     *        检查类型是否可默认构造的类型模板。
     *        基于 is_default_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_default_constructible : helper::bool_constant<is_default_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is move constructible.
     *        Checks construction from rvalue reference.
     *
     *        检查类型是否可移动构造的变量模板。
     *        检查从右值引用构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_move_constructible_v = __is_constructible(Ty, Ty);

    /**
     * @brief Type template for checking if a type is move constructible.
     *        Inherits from bool_constant based on is_move_constructible_v.
     *
     *        检查类型是否可移动构造的类型模板。
     *        基于 is_move_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_move_constructible : helper::bool_constant<is_move_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if assignment from 'from' to 'to' is possible.
     *        Uses compiler built-in __is_assignable.
     *
     *        检查从 'from' 到 'to' 的赋值是否可能的变量模板。
     *        使用编译器内建 __is_assignable。
     *
     * @tparam to Target type (as lvalue reference)
     *            目标类型（作为左值引用）
     * @tparam from Source type
     *              源类型
     */
    template <typename to, typename from>
    RAINY_CONSTEXPR_BOOL is_assignable_v = __is_assignable(to, from);

    /**
     * @brief Type template for checking if assignment from 'from' to 'to' is possible.
     *        Inherits from bool_constant based on is_assignable_v.
     *
     *        检查从 'from' 到 'to' 的赋值是否可能的类型模板。
     *        基于 is_assignable_v 继承自 bool_constant。
     *
     * @tparam to Target type (as lvalue reference)
     *            目标类型（作为左值引用）
     * @tparam from Source type
     *              源类型
     */
    template <typename to, typename from>
    struct is_assignable : helper::bool_constant<is_assignable_v<to, from>> {};

    /**
     * @brief Variable template for checking if a type is nothrow move constructible.
     *        Checks move construction that is guaranteed not to throw.
     *
     *        检查类型是否可无异常移动构造的变量模板。
     *        检查保证不抛异常的移动构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_move_constructible_v = __is_nothrow_constructible(Ty, Ty);

    /**
     * @brief Type template for checking if a type is nothrow move constructible.
     *        Inherits from bool_constant based on is_nothrow_move_constructible_v.
     *
     *        检查类型是否可无异常移动构造的类型模板。
     *        基于 is_nothrow_move_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_move_constructible : helper::bool_constant<is_nothrow_move_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is nothrow constructible with given arguments.
     *        Uses compiler built-in __is_nothrow_constructible.
     *
     *        检查类型是否可以使用给定参数无异常构造的变量模板。
     *        使用编译器内建 __is_nothrow_constructible。
     *
     * @tparam Ty Type to construct
     *            要构造的类型
     * @tparam Args Argument types for construction
     *              构造参数类型
     */
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_constructible_v = __is_nothrow_constructible(Ty, Args...);

    /**
     * @brief Type template for checking if a type is nothrow constructible with given arguments.
     *        Inherits from bool_constant based on is_nothrow_constructible_v.
     *
     *        检查类型是否可以使用给定参数无异常构造的类型模板。
     *        基于 is_nothrow_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to construct
     *            要构造的类型
     * @tparam Args Argument types for construction
     *              构造参数类型
     */
    template <typename Ty, typename... Args>
    struct is_nothrow_constructible : helper::bool_constant<is_nothrow_constructible_v<Ty, Args...>> {};
#if RAINY_USING_MSVC || RAINY_USING_CLANG
    /**
     * @brief Variable template for checking if a type is trivially destructible.
     *        Uses compiler built-in __is_trivially_destructible for MSVC and Clang.
     *
     *        检查类型是否可平凡析构的变量模板。
     *        对于 MSVC 和 Clang 使用编译器内建 __is_trivially_destructible。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_destructible_v = __is_trivially_destructible(Ty);
#else
    /**
     * @brief Variable template for checking if a type is trivially destructible.
     *        Uses GCC-specific implementation for other compilers.
     *
     *        检查类型是否可平凡析构的变量模板。
     *        对于其他编译器使用 GCC 特定的实现。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_destructible_v = implements::gcc_detail_impl::_is_destructible_safe<Ty>::value;
#endif

    /**
     * @brief Type template for checking if a type is trivially destructible.
     *        Inherits from bool_constant based on is_trivially_destructible_v.
     *
     *        检查类型是否可平凡析构的类型模板。
     *        基于 is_trivially_destructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_destructible : helper::bool_constant<is_trivially_destructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if assignment from 'from' to 'to' is nothrow.
     *        Uses implements::_is_nothrow_assignable_v.
     *
     *        检查从 'from' 到 'to' 的赋值是否无异常的变量模板。
     *        使用 implements::_is_nothrow_assignable_v。
     *
     * @tparam to Target type (as lvalue reference)
     *            目标类型（作为左值引用）
     * @tparam from Source type
     *              源类型
     */
    template <typename to, typename from>
    RAINY_CONSTEXPR_BOOL is_nothrow_assignable_v = implements::_is_nothrow_assignable_v<to, from>;

    /**
     * @brief Type template for checking if assignment from 'from' to 'to' is nothrow.
     *        Inherits from bool_constant based on is_nothrow_assignable_v.
     *
     *        检查从 'from' 到 'to' 的赋值是否无异常的类型模板。
     *        基于 is_nothrow_assignable_v 继承自 bool_constant。
     *
     * @tparam to Target type (as lvalue reference)
     *            目标类型（作为左值引用）
     * @tparam from Source type
     *              源类型
     */
    template <typename to, typename from>
    struct is_nothrow_assignable : helper::bool_constant<is_nothrow_assignable_v<to, from>> {};

    /**
     * @brief Variable template for checking if a type is nothrow copy constructible.
     *        Checks copy construction from const lvalue reference that is noexcept.
     *
     *        检查类型是否可无异常复制构造的变量模板。
     *        检查从 const 左值引用进行的不抛异常的复制构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    constexpr bool is_nothrow_copy_constructible_v =
        __is_nothrow_constructible(Ty, reference_modify::add_lvalue_reference_t<const Ty>);

    /**
     * @brief Type template for checking if a type is nothrow copy constructible.
     *        Inherits from bool_constant based on is_nothrow_copy_constructible_v.
     *
     *        检查类型是否可无异常复制构造的类型模板。
     *        基于 is_nothrow_copy_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_copy_constructible : helper::bool_constant<is_nothrow_copy_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is nothrow move assignable.
     *        Uses compiler built-in __is_nothrow_assignable.
     *
     *        检查类型是否可无异常移动赋值的变量模板。
     *        使用编译器内建 __is_nothrow_assignable。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    constexpr bool is_nothrow_move_assignable_v = __is_nothrow_assignable(reference_modify::add_lvalue_reference_t<Ty>, Ty);

    /**
     * @brief Type template for checking if a type is nothrow move assignable.
     *        Inherits from bool_constant based on is_nothrow_move_assignable_v.
     *
     *        检查类型是否可无异常移动赋值的类型模板。
     *        基于 is_nothrow_move_assignable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_move_assignable : helper::bool_constant<is_nothrow_move_assignable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is nothrow copy assignable.
     *        Uses compiler built-in __is_nothrow_assignable with const lvalue reference.
     *
     *        检查类型是否可无异常复制赋值的变量模板。
     *        使用带有 const 左值引用的编译器内建 __is_nothrow_assignable。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    constexpr bool is_nothrow_copy_assignable_v =
        __is_nothrow_assignable(reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<const Ty>);

    /**
     * @brief Type template for checking if a type is nothrow copy assignable.
     *        Inherits from bool_constant based on is_nothrow_copy_assignable_v.
     *
     *        检查类型是否可无异常复制赋值的类型模板。
     *        基于 is_nothrow_copy_assignable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_copy_assignable : helper::bool_constant<is_nothrow_copy_assignable_v<Ty>> {};

#if RAINY_USING_MSVC || RAINY_USING_CLANG

    /**
     * @brief Variable template for checking if a type is destructible.
     *        Uses compiler built-in __is_destructible for MSVC and Clang.
     *
     *        检查类型是否可析构的变量模板。
     *        对于 MSVC 和 Clang 使用编译器内建 __is_destructible。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_destructible_v = __is_destructible(Ty);

    /**
     * @brief Type template for checking if a type is destructible.
     *        Inherits from bool_constant based on is_destructible_v.
     *
     *        检查类型是否可析构的类型模板。
     *        基于 is_destructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_destructible : helper::bool_constant<is_destructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is nothrow destructible.
     *        Uses compiler built-in __is_nothrow_destructible for MSVC and Clang.
     *
     *        检查类型是否可无异常析构的变量模板。
     *        对于 MSVC 和 Clang 使用编译器内建 __is_nothrow_destructible。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_destructible_v = __is_nothrow_destructible(Ty);

    /**
     * @brief Type template for checking if a type is nothrow destructible.
     *        Inherits from bool_constant based on is_nothrow_destructible_v.
     *
     *        检查类型是否可无异常析构的类型模板。
     *        基于 is_nothrow_destructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_destructible : helper::bool_constant<is_nothrow_destructible_v<Ty>> {};

#else
    /**
     * @brief Variable template for checking if a type is destructible.
     *        Uses GCC-specific implementation for other compilers.
     *
     *        检查类型是否可析构的变量模板。
     *        对于其他编译器使用 GCC 特定的实现。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_destructible_v = implements::gcc_detail_impl::_is_destructible_safe<Ty>::value;

    /**
     * @brief Type template for checking if a type is destructible.
     *        Inherits from bool_constant based on is_destructible_v.
     *
     *        检查类型是否可析构的类型模板。
     *        基于 is_destructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_destructible : helper::bool_constant<is_destructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is nothrow destructible.
     *        Uses std::is_nothrow_destructible for other compilers.
     *
     *        检查类型是否可无异常析构的变量模板。
     *        对于其他编译器使用 std::is_nothrow_destructible。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_destructible_v = std::is_nothrow_destructible<Ty>::value;

    /**
     * @brief Type template for checking if a type is nothrow destructible.
     *        Inherits from bool_constant based on is_nothrow_destructible_v.
     *
     *        检查类型是否可无异常析构的类型模板。
     *        基于 is_nothrow_destructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_nothrow_destructible : helper::bool_constant<is_nothrow_destructible_v<Ty>> {};
#endif

    /**
     * @brief Variable template for checking if two types are equality comparable.
     *        Primary template defaults to false.
     *
     *        检查两个类型是否可相等比较的变量模板。
     *        主模板默认为 false。
     *
     * @tparam Ty1 First type
     *             第一个类型
     * @tparam Ty2 Second type
     *             第二个类型
     */
    template <typename Ty1, typename Ty2, typename = void>
    RAINY_CONSTEXPR_BOOL is_equal_comparable_v = false;

    /**
     * @brief Variable template for checking if two types are equality comparable.
     *        Specialization that detects the presence of operator==.
     *
     *        检查两个类型是否可相等比较的变量模板。
     *        检测 operator== 是否存在的特化。
     *
     * @tparam Ty1 First type
     *             第一个类型
     * @tparam Ty2 Second type
     *             第二个类型
     */
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_equal_comparable_v<
        Ty1, Ty2, other_trans::void_t<decltype(utility::declval<const Ty1 &>() == utility::declval<const Ty2 &>())>> = true;

    /**
     * @brief Variable template for checking if a type has a transparent functor.
     *        Detects the presence of the is_transparent member type.
     *
     *        检查类型是否具有透明仿函数的变量模板。
     *        检测 is_transparent 成员类型是否存在。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_transparent_v = false;

    /**
     * @brief Variable template for checking if a type has a transparent functor.
     *        Specialization that detects the is_transparent member type.
     *
     *        检查类型是否具有透明仿函数的变量模板。
     *        检测 is_transparent 成员类型的特化。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_transparent_v<Ty, other_trans::void_t<typename Ty::is_transparent>> = true;

    /**
     * @brief Variable template for checking if a type is nothrow default constructible.
     *        Uses compiler built-in __is_nothrow_constructible with no arguments.
     *
     *        检查类型是否可无异常默认构造的变量模板。
     *        使用无参数的编译器内建 __is_nothrow_constructible。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_default_constructible_v = __is_nothrow_constructible(Ty);

    /**
     * @brief Variable template for checking if a type is trivially constructible with given arguments.
     *        Uses compiler built-in __is_trivially_constructible.
     *
     *        检查类型是否可以使用给定参数平凡构造的变量模板。
     *        使用编译器内建 __is_trivially_constructible。
     *
     * @tparam Ty Type to construct
     *            要构造的类型
     * @tparam Args Argument types for construction
     *              构造参数类型
     */
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_trivially_constructible_v = __is_trivially_constructible(Ty, Args...);

    /**
     * @brief Type template for checking if a type is trivially constructible with given arguments.
     *        Inherits from bool_constant based on is_trivially_constructible_v.
     *
     *        检查类型是否可以使用给定参数平凡构造的类型模板。
     *        基于 is_trivially_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to construct
     *            要构造的类型
     * @tparam Args Argument types for construction
     *              构造参数类型
     */
    template <typename Ty, typename... Args>
    struct is_trivially_constructible : helper::bool_constant<is_trivially_constructible_v<Ty, Args...>> {};
    /**
     * @brief Variable template for checking if a type is trivially default constructible.
     *        Aliases is_trivially_constructible_v<Ty>.
     *
     *        检查类型是否可平凡默认构造的变量模板。
     *        是 is_trivially_constructible_v<Ty> 的别名。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_default_constructible_v = is_trivially_constructible_v<Ty>;

    /**
     * @brief Type template for checking if a type is trivially default constructible.
     *        Inherits from bool_constant based on is_trivially_default_constructible_v.
     *
     *        检查类型是否可平凡默认构造的类型模板。
     *        基于 is_trivially_default_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_default_constructible : helper::bool_constant<is_trivially_default_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivially copy constructible.
     *        Checks trivial construction from const lvalue reference.
     *
     *        检查类型是否可平凡复制构造的变量模板。
     *        检查从 const 左值引用的平凡构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_copy_constructible_v =
        is_trivially_constructible_v<Ty, reference_modify::add_lvalue_reference_t<const Ty>>;

    /**
     * @brief Type template for checking if a type is trivially copy constructible.
     *        Inherits from bool_constant based on is_trivially_copy_constructible_v.
     *
     *        检查类型是否可平凡复制构造的类型模板。
     *        基于 is_trivially_copy_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_copy_constructible : helper::bool_constant<is_trivially_copy_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivially move constructible.
     *        Checks trivial construction from rvalue reference.
     *
     *        检查类型是否可平凡移动构造的变量模板。
     *        检查从右值引用的平凡构造。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_move_constructible_v = is_trivially_constructible_v<Ty, Ty>;

    /**
     * @brief Type template for checking if a type is trivially move constructible.
     *        Inherits from bool_constant based on is_trivially_move_constructible_v.
     *
     *        检查类型是否可平凡移动构造的类型模板。
     *        基于 is_trivially_move_constructible_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_move_constructible : helper::bool_constant<is_trivially_move_constructible_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivially swappable.
     *        A type is trivially swappable if it is trivially destructible,
     *        trivially move constructible, trivially move assignable, and has no ADL swap.
     *
     *        检查类型是否可平凡交换的变量模板。
     *        如果类型可平凡析构、可平凡移动构造、可平凡移动赋值且没有 ADL swap，
     *        则该类型可平凡交换。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_swappable_v = is_trivially_destructible_v<Ty> && is_trivially_move_constructible_v<Ty> &&
                                                    is_trivially_move_assignable_v<Ty> && !has_adl_swap_v<Ty>;

    /**
     * @brief Type template for checking if a type is trivially swappable.
     *        Inherits from bool_constant based on is_trivially_swappable_v.
     *
     *        检查类型是否可平凡交换的类型模板。
     *        基于 is_trivially_swappable_v 继承自 bool_constant。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_trivially_swappable : helper::bool_constant<is_trivially_swappable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a scoped enum.
     *        Primary template defaults to false.
     *
     *        检查类型是否为有作用域枚举的变量模板。
     *        主模板默认为 false。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     * @tparam is_enum Boolean indicating if Ty is an enum
     *                 指示 Ty 是否为枚举的布尔值
     */
    template <typename Ty, bool = primary_types::is_enum_v<Ty>>
    RAINY_CONSTEXPR_BOOL is_scoped_enum_v = false;

    /**
     * @brief Variable template for checking if a type is a scoped enum.
     *        Specialization for enum types. Scoped enums are not implicitly convertible
     *        to their underlying type.
     *
     *        检查类型是否为有作用域枚举的变量模板。
     *        枚举类型的特化。有作用域枚举不能隐式转换为其底层类型。
     *
     * @tparam Ty Enum type to check
     *            要检查的枚举类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_scoped_enum_v<Ty, true> = !type_relations::is_convertible_v<Ty, other_trans::underlying_type_t<Ty>>;

    /**
     * @brief Variable template for checking if a type is an unscoped enum.
     *        Primary template defaults to false.
     *
     *        检查类型是否为无作用域枚举的变量模板。
     *        主模板默认为 false。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     * @tparam is_enum Boolean indicating if Ty is an enum
     *                 指示 Ty 是否为枚举的布尔值
     */
    template <typename Ty, bool = primary_types::is_enum_v<Ty>>
    RAINY_CONSTEXPR_BOOL is_unscoped_enum_v = false;

    /**
     * @brief Variable template for checking if a type is an unscoped enum.
     *        Specialization for enum types. Unscoped enums are implicitly convertible
     *        to their underlying type.
     *
     *        检查类型是否为无作用域枚举的变量模板。
     *        枚举类型的特化。无作用域枚举可以隐式转换为其底层类型。
     *
     * @tparam Ty Enum type to check
     *            要检查的枚举类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unscoped_enum_v<Ty, true> = type_relations::is_convertible_v<Ty, other_trans::underlying_type_t<Ty>>;

    /**
     * @brief Variable template for determining if a type should be passed by value.
     *        Primary template for non-class, non-union, non-array, non-function types.
     *        Types smaller than or equal to 2 pointers and trivially copyable are preferred
     *        to be passed by value.
     *
     *        确定类型是否应通过值传递的变量模板。
     *        非类、非联合、非数组、非函数类型的主模板。
     *        大小小于等于2个指针且可平凡复制的类型优先通过值传递。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     * @tparam is_special Boolean indicating if Ty is a class/union/array/function type
     *                    指示 Ty 是否为类/联合/数组/函数类型的布尔值
     */
    template <typename Ty, bool = type_traits::primary_types::is_class_v<Ty> || type_traits::primary_types::is_union_v<Ty> ||
                                  type_traits::primary_types::is_array_v<Ty> || type_traits::primary_types::is_function_v<Ty>>
    RAINY_CONSTEXPR_BOOL prefer_pass_by_value_v = sizeof(Ty) <= 2 * sizeof(void *) && std::is_trivially_copy_constructible_v<Ty>;

    /**
     * @brief Variable template for determining if a type should be passed by value.
     *        Specialization for class, union, array, and function types.
     *        These types are never preferred to be passed by value.
     *
     *        确定类型是否应通过值传递的变量模板。
     *        类、联合、数组和函数类型的特化。
     *        这些类型永远不优先通过值传递。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL prefer_pass_by_value_v<Ty, true> = false;
}

namespace rainy::foundation::container {
    /**
     * @brief A fixed-size collection of heterogeneous values.
     *        固定大小的异构值集合。
     *
     * @tparam Types The types of elements contained in the tuple
     *               tuple中包含的元素类型
     */
    template <typename... Types>
    class tuple;

    /**
     * @brief Specialization for empty tuple.
     *        空tuple的特化。
     */
    template <>
    class tuple<> {
    public:
        /**
         * @brief Default constructor.
         *        默认构造函数。
         */
        constexpr tuple() noexcept = default;

        /**
         * @brief Copy constructor.
         *        拷贝构造函数。
         */
        constexpr tuple(const tuple &) = default;

        /**
         * @brief Move constructor.
         *        移动构造函数。
         */
        constexpr tuple(tuple &&) = default;

        /**
         * @brief Copy assignment operator.
         *        拷贝赋值运算符。
         */
        constexpr tuple &operator=(const tuple &) = default;

        /**
         * @brief Move assignment operator.
         *        移动赋值运算符。
         */
        constexpr tuple &operator=(tuple &&) = default;

        /**
         * @brief Swaps two empty tuples (no-op).
         *        交换两个空tuple（无操作）。
         *
         * @param other The other tuple to swap with
         *              要交换的另一个tuple
         */
        static constexpr void swap(tuple &) noexcept {
        }
    };

    /**
     * @brief Provides compile-time access to the type of a tuple element.
     *        提供对tuple元素类型的编译时访问。
     *
     * @tparam Indices The index of the element
     *                 元素的索引
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <std::size_t Indices, typename Tuple>
    struct tuple_element {};

    /**
     * @brief Specialization for tuple types.
     *        tuple类型的特化。
     *
     * @tparam Indices The index of the element
     *                 元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     */
    template <std::size_t Indices, typename... Types>
    struct tuple_element<Indices, tuple<Types...>> {
        using type = typename type_traits::other_trans::type_at<Indices, type_traits::other_trans::type_list<Types...>>::type;
    };

    /**
     * @brief Alias template for tuple element type.
     *        tuple元素类型的别名模板。
     *
     * @tparam Indicies The index of the element
     *                  元素的索引
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <std::size_t Indicies, typename Tuple>
    using tuple_element_t = typename tuple_element<Indicies, Tuple>::type;

    /**
     * @brief Gets a reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access
     *          要访问的tuple
     * @return Reference to the element at index I
     *         索引I处元素的引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &) noexcept;

    /**
     * @brief Gets a const reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access
     *          要访问的常量tuple
     * @return Const reference to the element at index I
     *         索引I处元素的常量引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &) noexcept;

    /**
     * @brief Gets an rvalue reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access (as rvalue)
     *          要访问的tuple（作为右值）
     * @return Rvalue reference to the element at index I
     *         索引I处元素的右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&) noexcept;

    /**
     * @brief Gets a const rvalue reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access (as rvalue)
     *          要访问的常量tuple（作为右值）
     * @return Const rvalue reference to the element at index I
     *         索引I处元素的常量右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&) noexcept;
}

namespace rainy::foundation::container::implements {
    template <typename Ty>
    struct tuple_val {
        constexpr tuple_val() : value() {
        }

        constexpr tuple_val(const tuple_val &other) : value(other.value) {
        }

        constexpr tuple_val(tuple_val &&other) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Ty>) :
            value(utility::move(other.value)) {
        }

        constexpr tuple_val &operator=(const tuple_val &other) {
            if (this != &other) {
                value = other.value;
            }
            return *this;
        }

        constexpr tuple_val &operator=(tuple_val &&other) noexcept(type_traits::type_properties::is_nothrow_move_assignable_v<Ty>) {
            if (this != &other) {
                value = utility::move(other.value);
            }
            return *this;
        }

        template <typename Other,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, Other &&> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Other>, tuple_val> &&
                          !type_traits::type_relations::is_base_of_v<tuple_val, type_traits::other_trans::decay_t<Other>>,
                      int> = 0>
        constexpr explicit tuple_val(Other &&arg) : value(utility::forward<Other>(arg)) { // NOLINT
        }

        template <typename Alloc, typename... Args, type_traits::other_trans::enable_if_t<!std::uses_allocator_v<Ty, Alloc>, int> = 0>
        constexpr tuple_val(std::allocator_arg_t, const Alloc &, Args &&...args) : value(utility::forward<Args>(args)...) {
        }

        template <typename Alloc, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          std::uses_allocator<Ty, Alloc>, std::is_constructible<Ty, std::allocator_arg_t, const Alloc &, Args...>>,
                      int> = 0>
        constexpr tuple_val(std::allocator_arg_t, const Alloc &allocator, Args &&...args) :
            value(std::allocator_arg, allocator, utility::forward<Args>(args)...) {
        }

        template <
            typename Alloc, typename... Args,
            type_traits::other_trans::enable_if_t<
                type_traits::logical_traits::conjunction_v<
                    std::uses_allocator<Ty, Alloc>,
                    type_traits::logical_traits::negation<std::is_constructible<Ty, std::allocator_arg_t, const Alloc &, Args...>>>,
                int> = 0>
        constexpr tuple_val(std::allocator_arg_t, const Alloc &allocator, Args &&...args) :
            value(utility::forward<Args>(args)..., allocator) {
        }

        Ty value;
    };

    template <std::size_t I, typename Ty, std::size_t Index>
    struct tuple_leaf_index : tuple_val<Ty> {
        constexpr tuple_leaf_index() : tuple_val<Ty>() {
        }

        template <typename Uty>
        constexpr tuple_leaf_index(Uty &&arg) : tuple_val<Ty>(utility::forward<Uty>(arg)) { // NOLINT
        }

        template <typename Alloc, typename... Args>
        constexpr tuple_leaf_index(const Alloc &alloc, std::allocator_arg_t tag, Args &&...args) :
            tuple_val<Ty>(tag, alloc, utility::forward<Args>(args)...) {
        }
    };
}

// NOLINTBEGIN
namespace std {
    template <std::size_t I, typename... Types>
    struct tuple_element<I, rainy::foundation::container::tuple<Types...>>
        : rainy::foundation::container::tuple_element<I, rainy::foundation::container::tuple<Types...>> {};

    template <typename... Types>
    struct tuple_size<rainy::foundation::container::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};
}
// NOLINTEND

namespace rainy::foundation::container {
    /**
     * @brief Primary template for tuple with at least one element.
     *        至少包含一个元素的tuple主模板。
     *
     * @tparam Head Type of the first element
     *              第一个元素的类型
     * @tparam Rest Types of the remaining elements
     *              剩余元素的类型
     */
    template <typename Head, typename... Rest>
    class tuple<Head, Rest...> : private implements::tuple_leaf_index<0, Head, sizeof...(Rest)>, private tuple<Rest...> {
    public:
        using head_base = implements::tuple_leaf_index<0, Head, sizeof...(Rest)>;
        using rest_base = tuple<Rest...>;

        /**
         * @brief Default constructor.
         *        默认构造函数。
         */
        constexpr tuple() : head_base{}, rest_base{} {
        }

        /**
         * @brief Copy constructor.
         *        拷贝构造函数。
         */
        constexpr tuple(const tuple &) = default;

        /**
         * @brief Move constructor.
         *        移动构造函数。
         */
        constexpr tuple(tuple &&) = default;

        /**
         * @brief Constructs tuple from explicit arguments.
         *        从显式参数构造tuple。
         *
         * @tparam HeadArg Type of the head argument
         *                 头参数的类型
         * @tparam RestArgs Types of the rest arguments
         *                  剩余参数的类型
         * @param head_arg Value for the first element
         *                 第一个元素的值
         * @param rest_args Values for the remaining elements
         *                  剩余元素的值
         */
        template <typename HeadArg, typename... RestArgs,
                  type_traits::other_trans::enable_if_t<sizeof...(RestArgs) == sizeof...(Rest) &&
                                                            type_traits::type_properties::is_constructible_v<Head, HeadArg> &&
                                                            (type_traits::type_properties::is_constructible_v<Rest, RestArgs> && ...),
                                                        int> = 0>
        constexpr tuple(HeadArg &&head_arg, RestArgs &&...rest_args) :
            head_base(utility::forward<HeadArg>(head_arg)), rest_base(utility::forward<RestArgs>(rest_args)...) {
        }

        /**
         * @brief Converting constructor from another tuple.
         *        从另一个tuple的转换构造函数。
         *
         * @tparam OtherHead Type of the first element of the other tuple
         *                   另一个tuple的第一个元素类型
         * @tparam OtherRest Types of the remaining elements of the other tuple
         *                   另一个tuple的剩余元素类型
         * @param other The other tuple to copy from
         *              要拷贝的另一个tuple
         */
        template <
            typename OtherHead, typename... OtherRest,
            type_traits::other_trans::enable_if_t<
                sizeof...(OtherRest) == sizeof...(Rest) && type_traits::type_properties::is_constructible_v<Head, const OtherHead &> &&
                    (type_traits::type_properties::is_constructible_v<Rest, const OtherRest &> && ...),
                int> = 0>
        constexpr tuple(const tuple<OtherHead, OtherRest...> &other) :
            head_base(get<0>(other)), rest_base(static_cast<const tuple<OtherRest...> &>(other)) {
        }

        /**
         * @brief Allocator-extended default constructor.
         *        分配器扩展的默认构造函数。
         *
         * @tparam Alloc Allocator type
         *               分配器类型
         * @param tag allocator_arg_t tag for disambiguation
         *            用于消歧的 allocator_arg_t 标签
         * @param alloc The allocator to use
         *              要使用的分配器
         */
        template <typename Alloc>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc) : head_base(tag, alloc), rest_base(tag, alloc) {
        }

        /**
         * @brief Allocator-extended constructor with arguments.
         *        带参数的分配器扩展构造函数。
         *
         * @tparam Alloc Allocator type
         *               分配器类型
         * @tparam HeadArg Type of the head argument
         *                 头参数的类型
         * @tparam TailArgs Types of the remaining arguments
         *                  剩余参数的类型
         * @param tag allocator_arg_t tag for disambiguation
         *            用于消歧的 allocator_arg_t 标签
         * @param alloc The allocator to use
         *              要使用的分配器
         * @param head_arg Value for the first element
         *                 第一个元素的值
         * @param tail_args Values for the remaining elements
         *                  剩余元素的值
         */
        template <typename Alloc, typename HeadArg, typename... TailArgs>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg, TailArgs &&...tail_args) :
            head_base(tag, alloc, utility::forward<HeadArg>(head_arg)),
            rest_base(tag, alloc, utility::forward<TailArgs>(tail_args)...) {
        }

        /**
         * @brief Copy assignment operator.
         *        拷贝赋值运算符。
         *
         * @param other The other tuple to copy from
         *              要拷贝的另一个tuple
         * @return Reference to this tuple
         *         此tuple的引用
         */
        constexpr tuple &operator=(const tuple &other) {
            head_base::value = static_cast<const head_base &>(other).value;
            static_cast<rest_base &>(*this) = static_cast<const rest_base &>(other);
            return *this;
        }

        /**
         * @brief Move assignment operator.
         *        移动赋值运算符。
         *
         * @param other The other tuple to move from
         *              要移动的另一个tuple
         * @return Reference to this tuple
         *         此tuple的引用
         */
        constexpr tuple &operator=(tuple &&other) noexcept(std::is_nothrow_move_assignable_v<Head> &&
                                                           std::is_nothrow_move_assignable_v<tuple<Rest...>>) {
            head_base::value = utility::move(static_cast<head_base &&>(other).value);
            static_cast<rest_base &>(*this) = static_cast<rest_base &&>(other);
            return *this;
        }

        /**
         * @brief Converting assignment from another tuple.
         *        从另一个tuple的转换赋值。
         *
         * @tparam OtherHead Type of the first element of the other tuple
         *                   另一个tuple的第一个元素类型
         * @tparam OtherRest Types of the remaining elements of the other tuple
         *                   另一个tuple的剩余元素类型
         * @param other The other tuple to copy from
         *              要拷贝的另一个tuple
         * @return Reference to this tuple
         *         此tuple的引用
         */
        template <typename OtherHead, typename... OtherRest,
                  type_traits::other_trans::enable_if_t<sizeof...(OtherRest) == sizeof...(Rest) &&
                                                            std::is_assignable_v<Head &, const OtherHead &> &&
                                                            (std::is_assignable_v<Rest &, const OtherRest &> && ...),
                                                        int> = 0>
        constexpr tuple &operator=(const tuple<OtherHead, OtherRest...> &other) {
            head_base::value = get<0>(other);
            static_cast<rest_base &>(*this) = static_cast<const tuple<OtherRest...> &>(other);
            return *this;
        }

        /**
         * @brief Swaps the contents with another tuple.
         *        与另一个tuple交换内容。
         *
         * @param other The other tuple to swap with
         *              要交换的另一个tuple
         */
        constexpr void swap(tuple &other) noexcept(std::is_nothrow_swappable_v<Head> && std::is_nothrow_swappable_v<tuple<Rest...>>) {
            head_base::swap(static_cast<head_base &>(other));
            static_cast<rest_base &>(*this).swap(static_cast<rest_base &>(other));
        }

        /**
         * @brief Gets a reference to the element at index I.
         *        获取索引I处元素的引用。
         *
         * @tparam I The index of the element to access
         *           要访问的元素的索引
         * @return Reference to the element at index I
         *         索引I处元素的引用
         */
        template <std::size_t I>
        constexpr tuple_element_t<I, tuple> &get() noexcept {
            if constexpr (I == 0) {
                return head_base::value;
            } else {
                return static_cast<rest_base &>(*this).template get<I - 1>();
            }
        }

        /**
         * @brief Gets a const reference to the element at index I.
         *        获取索引I处元素的常量引用。
         *
         * @tparam I The index of the element to access
         *           要访问的元素的索引
         * @return Const reference to the element at index I
         *         索引I处元素的常量引用
         */
        template <std::size_t I>
        constexpr const tuple_element_t<I, tuple> &get() const noexcept {
            if constexpr (I == 0) {
                return head_base::value;
            } else {
                return static_cast<const rest_base &>(*this).template get<I - 1>();
            }
        }
    };

    /**
     * @brief Gets a reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access
     *          要访问的tuple
     * @return Reference to the element at index I
     *         索引I处元素的引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    /**
     * @brief Gets a const reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access
     *          要访问的常量tuple
     * @return Const reference to the element at index I
     *         索引I处元素的常量引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    /**
     * @brief Gets an rvalue reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access (as rvalue)
     *          要访问的tuple（作为右值）
     * @return Rvalue reference to the element at index I
     *         索引I处元素的右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&t) noexcept { // NOLINT
        return utility::move(t.template get<I>());
    }

    /**
     * @brief Gets a const rvalue reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access (as rvalue)
     *          要访问的常量tuple（作为右值）
     * @return Const rvalue reference to the element at index I
     *         索引I处元素的常量右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&t) noexcept {
        return utility::move(t.template get<I>());
    }

    /**
     * @brief Creates a tuple by deducing the types of the arguments and decaying them.
     *        通过推导参数类型并退化它们来创建tuple。
     *
     * @tparam Types The types of the arguments
     *               参数的类型
     * @param args The values to store in the tuple
     *             要存储在tuple中的值
     * @return A tuple containing the decayed copies of the arguments
     *         包含参数退化副本的tuple
     */
    template <typename... Types>
    constexpr tuple<std::decay_t<Types>...> make_tuple(Types &&...args) {
        return tuple<std::decay_t<Types>...>(utility::forward<Types>(args)...);
    }

    /**
     * @brief Creates a tuple of references to the arguments.
     *        创建参数引用的tuple。
     *
     * @tparam Types The types of the arguments
     *               参数的类型
     * @param args The values to create references to
     *             要创建引用的值
     * @return A tuple containing references to the arguments
     *         包含参数引用的tuple
     */
    template <typename... Types>
    constexpr tuple<Types &&...> forward_as_tuple(Types &&...args) noexcept {
        return tuple<Types &&...>(utility::forward<Types>(args)...);
    }

    /**
     * @brief Swaps two tuples.
     *        交换两个tuple。
     *
     * @tparam Types The types contained in the tuples
     *               tuple中包含的类型
     * @param left The first tuple
     *             第一个tuple
     * @param right The second tuple
     *              第二个tuple
     */
    template <typename... Types>
    constexpr void swap(tuple<Types...> &left, tuple<Types...> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }

    /**
     * @brief Equality comparison between two tuples.
     *        两个tuple之间的相等比较。
     *
     * @tparam TTypes Types of the left tuple
     *                左tuple的类型
     * @tparam UTypes Types of the right tuple
     *                右tuple的类型
     * @param left The left tuple
     *             左tuple
     * @param right The right tuple
     *              右tuple
     * @return true if the tuples are element-wise equal, false otherwise
     *         如果tuple逐元素相等则为true，否则为false
     */
    template <typename... TTypes, typename... UTypes>
    constexpr bool operator==(const tuple<TTypes...> &left, const tuple<UTypes...> &right) {
        if constexpr (sizeof...(TTypes) != sizeof...(UTypes)) {
            return false;
        } else if constexpr (sizeof...(TTypes) == 0) {
            return true;
        } else {
            return get<0>(left) == get<0>(right) &&
                   static_cast<const tuple<TTypes...> &>(left) == static_cast<const tuple<UTypes...> &>(right);
        }
    }
}

namespace rainy::foundation::container {
    /**
     * @brief Provides the number of elements in a tuple.
     *        提供tuple中的元素数量。
     *
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <typename Tuple>
    struct tuple_size {
        static RAINY_INLINE_CONSTEXPR std::size_t value = 0;
    };

    /**
     * @brief Specialization for tuple types.
     *        tuple类型的特化。
     *
     * @tparam Args The types contained in the tuple
     *              tuple中包含的类型
     */
    template <typename... Args>
    struct tuple_size<tuple<Args...>> {
        static RAINY_INLINE_CONSTEXPR std::size_t value = sizeof...(Args);
    };

    /**
     * @brief Variable template for tuple size.
     *        tuple大小的变量模板。
     *
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <typename Tuple>
    static inline constexpr std::size_t tuple_size_v = tuple_size<Tuple>::value;
}

namespace std {
    using rainy::foundation::container::get;
}

namespace rainy::utility {
    using rainy::foundation::container::forward_as_tuple;
    using rainy::foundation::container::get;
    using rainy::foundation::container::make_tuple;
    using rainy::foundation::container::tuple;
    using rainy::foundation::container::tuple_element;
    using rainy::foundation::container::tuple_element_t;
    using rainy::foundation::container::tuple_size;
    using rainy::foundation::container::tuple_size_v;
}

namespace rainy::utility {
    template <typename Callable>
    class finally_impl : Callable { // NOLINT
    public:
        finally_impl(Callable &&callable) noexcept : Callable(utility::forward<Callable>(callable)), invalidate_(false) { // NOLINT
        }

        ~finally_impl() {
            if (!is_invalidate()) {
                invoke_now();
            }
        }

        finally_impl(const finally_impl &) = delete;

        finally_impl &operator=(const finally_impl &) = delete;
        finally_impl &operator=(finally_impl &&) = delete;

        RAINY_NODISCARD bool is_invalidate() const noexcept {
            return invalidate_;
        }

        void to_invalidate() noexcept {
            invalidate_ = true;
        }

        void to_useable() noexcept {
            invalidate_ = false;
        }

        void invoke_now() const {
            (*this)();
        }

        template <typename Pred>
        void set_condition(Pred &&pred) {
            invalidate_ = static_cast<bool>(pred());
        }

    private:
        bool invalidate_;
    };

    template <typename Callable>
    auto make_finally(Callable &&callable) -> finally_impl<Callable> {
        return finally_impl<Callable>(utility::forward<Callable>(callable));
    }
}

// NOLINTBEGIN (bugprone-marco-parentheses)
#define RAINY_MEMBER_POINTER_TRAITS_SPEC(SPEC)                                                                                        \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct member_pointer_traits<Rx (Class::*)(Args...) SPEC> {                                                                       \
        static constexpr bool valid = true;                                                                                           \
        using class_type = Class;                                                                                                     \
        using return_type = Rx;                                                                                                       \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct member_pointer_traits<Rx (Class::*)(Args..., ...) SPEC> {                                                                  \
        using class_type = Class;                                                                                                     \
        using return_type = Rx;                                                                                                       \
        static constexpr bool valid = true;                                                                                           \
    };
// NOLINTEND (bugprone-marco-parenthese)

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4003)
#endif

namespace rainy::type_traits::primary_types {
    /**
     * @brief Primary template for member pointer traits.
     *        Provides information about member pointer types.
     *
     *        成员指针特性的主模板。
     *        提供关于成员指针类型的信息。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename>
    struct member_pointer_traits {
        /**
         * @brief Indicates whether the type is a valid member pointer.
         *        指示类型是否为有效的成员指针。
         */
        static RAINY_CONSTEXPR_BOOL valid = false;

        /**
         * @brief The class type that the member pointer belongs to.
         *        成员指针所属的类类型。
         */
        using class_type = void;
    };

    // Specializations for various member pointer cv-qualifier and ref-qualifier combinations
    // 各种成员指针 cv-限定符和引用限定符组合的特化
    RAINY_MEMBER_POINTER_TRAITS_SPEC()
    RAINY_MEMBER_POINTER_TRAITS_SPEC(&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(&&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const &)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const &&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(& noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(&& noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const & noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const && noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile &)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile &&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile & noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile && noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile &)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile &&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile & noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile && noexcept)

    /**
     * @brief Specialization of member_pointer_traits for actual member pointer types.
     *        Provides the member type and class type for a valid member pointer.
     *
     *        member_pointer_traits 对于实际成员指针类型的特化。
     *        为有效的成员指针提供成员类型和类类型。
     *
     * @tparam Type The type of the member
     *              成员的类型
     * @tparam Class The class type that the member belongs to
     *               成员所属的类类型
     */
    template <typename Type, typename Class>
    struct member_pointer_traits<Type Class::*> {
        /**
         * @brief Indicates that this is a valid member pointer.
         *        指示这是一个有效的成员指针。
         */
        using type = Type;

        /**
         * @brief The class type that the member pointer belongs to.
         *        成员指针所属的类类型。
         */
        using class_type = Class;
    };
}

#undef RAINY_MEMBER_POINTER_TRAITS_SPEC

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

// NOLINTBEGIN(bugprone-macro-parentheses)
#define RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, SPEC)                                                    \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args...) SPEC>                                                                                          \
        : implements::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> {                                     \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args..., ...) SPEC>                                                                                     \
        : implements::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> {                                     \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };

#define RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer, IsLValue, IsRValue, SPEC)  \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args...) SPEC>                                                                               \
        : implements::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                \
          implements::member_function_traits_base<IsLValue, IsRValue> {                                                               \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args..., ...) SPEC>                                                                          \
        : implements::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                \
          implements::member_function_traits_base<IsLValue, IsRValue> {                                                               \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };
// NOLINTEND(bugprone-macro-parentheses)

namespace rainy::type_traits::implements {
    template <bool IsMemberFunctionPointer = false, bool IsFunctionPointer = false, bool IsNoexcept = false, bool IsVolatile = false,
              bool IsConstMemberFunction = false>
    struct function_traits_base {
        /**
         * @brief Indicates whether this is a member function pointer
         *        指示是否为成员函数指针
         */
        static RAINY_CONSTEXPR_BOOL is_member_function_pointer = IsMemberFunctionPointer;

        /**
         * @brief Indicates whether this is a function pointer
         *        指示是否为函数指针
         */
        static RAINY_CONSTEXPR_BOOL is_function_pointer = IsFunctionPointer;

        /**
         * @brief Indicates whether the function is noexcept
         *        指示函数是否为 noexcept
         */
        static RAINY_CONSTEXPR_BOOL is_noexcept = IsNoexcept;

        /**
         * @brief Indicates whether the member function is volatile-qualified
         *        指示成员函数是否为 volatile 限定
         */
        static RAINY_CONSTEXPR_BOOL is_volatile = IsVolatile;

        /**
         * @brief Indicates whether the member function is const-qualified
         *        指示成员函数是否为 const 限定
         */
        static RAINY_CONSTEXPR_BOOL is_const_member_function = IsConstMemberFunction;

        /**
         * @brief Indicates whether this is a valid function type
         *        指示是否为有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = true;

        /**
         * @brief Indicates whether this is a function object
         *        指示是否为函数对象
         */
        static RAINY_CONSTEXPR_BOOL is_function_object = false;
    };

    template <bool IsLvalue, bool IsRvalue>
    struct member_function_traits_base {
        /**
         * @brief Indicates whether the function can be invoked on lvalue objects
         *        指示函数是否可以在左值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = IsLvalue;

        /**
         * @brief Indicates whether the function can be invoked on rvalue objects
         *        指示函数是否可以在右值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = IsRvalue;
    };
}

namespace rainy::type_traits::implements {
    template <typename Fx, typename = void>
    struct fx_traits_has_invoke_operator : helper::false_type {};

    template <typename Fx>
    struct fx_traits_has_invoke_operator<
        Fx, type_traits::other_trans::void_t<decltype(&type_traits::cv_modify::remove_cv_t<Fx>::operator())>> : helper::true_type {};

    struct empty_function_traits {
        /**
         * @brief Indicates that this is not a valid function type
         *        指示这不是一个有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = false;
    };

    template <typename Fx, bool Enable = fx_traits_has_invoke_operator<Fx>::value>
    struct try_to_get_operator {
        using type = void;
    };

    template <typename Fx>
    struct try_to_get_operator<Fx, true> {
        using type = decltype(&type_traits::cv_modify::remove_cv_t<Fx>::operator());
    };

    template <typename Traits, bool Enable = Traits::valid>
    struct fn_obj_traits {
        /**
         * @brief List of argument types for the function object
         *        函数对象的参数类型列表
         */
        using argument_list = typename Traits::argument_list;

        /**
         * @brief Return type of the function object
         *        函数对象的返回类型
         */
        using return_type = typename Traits::return_type;

        /**
         * @brief Number of arguments the function object takes
         *        函数对象接受的参数数量
         */
        static inline constexpr std::size_t arity = Traits::arity;

        /**
         * @brief Indicates whether the function object is noexcept
         *        指示函数对象是否为 noexcept
         */
        static RAINY_CONSTEXPR_BOOL is_noexcept = Traits::is_noexcept;

        /**
         * @brief Indicates whether the function object is volatile-qualified
         *        指示函数对象是否为 volatile 限定
         */
        static RAINY_CONSTEXPR_BOOL is_volatile = Traits::is_volatile;

        /**
         * @brief Indicates whether the function can be invoked on lvalue objects
         *        指示函数是否可以在左值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = false;

        /**
         * @brief Indicates whether the function can be invoked on rvalue objects
         *        指示函数是否可以在右值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = false;

        /**
         * @brief Indicates that this is a function object
         *        指示这是一个函数对象
         */
        static RAINY_CONSTEXPR_BOOL is_function_object = true;

        /**
         * @brief Indicates that this is a valid function type
         *        指示这是一个有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = true;

        /**
         * @brief Indicates whether the member function is const-qualified
         *        指示成员函数是否为 const 限定
         */
        static RAINY_CONSTEXPR_BOOL is_const_member_function = Traits::is_const_member_function;
    };

    template <typename Traits>
    struct fn_obj_traits<Traits, false> : empty_function_traits {};
}

namespace rainy::type_traits::primary_types {
    /**
     * @brief Primary template for function traits.
     *        Provides comprehensive information about function types, function pointers, and function objects.
     *
     *        函数特性的主模板。
     *        提供关于函数类型、函数指针和函数对象的全面信息。
     *
     * @tparam Ty The type to examine (function type, function pointer, or function object)
     *            要检查的类型（函数类型、函数指针或函数对象）
     */
    template <typename Ty>
    struct function_traits
        : implements::fn_obj_traits<
              function_traits<typename implements::try_to_get_operator<type_traits::cv_modify::remove_cvref_t<Ty>>::type>> {};

    /**
     * @brief Specialization for void type.
     *        Provides empty traits for void.
     *
     *        void类型的特化。
     *        为void提供空特性。
     */
    template <>
    struct function_traits<void> : implements::empty_function_traits {};

    /**
     * @brief Specialization for normal function types.
     *        Provides traits for regular function types.
     *
     *        普通函数类型的特化。
     *        为常规函数类型提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Parameter types of the function
     *              函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args...)> : implements::function_traits_base<> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of parameter types
         *        参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of parameters
         *        参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for variadic function types.
     *        Provides traits for C-style variadic functions.
     *
     *        可变参数函数类型的特化。
     *        为C风格可变参数函数提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Fixed parameter types of the function
     *              函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args..., ...)> : implements::function_traits_base<> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of fixed parameter types (excluding variadic part)
         *        固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of fixed parameters
         *        固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    // Macro expansions for various cv-qualifier and noexcept combinations
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(false, true, volatile)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, false, noexcept)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, true, volatile noexcept)

    /**
     * @brief Specialization for function pointer types.
     *        Provides traits for regular function pointers.
     *
     *        函数指针类型的特化。
     *        为常规函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Parameter types of the function
     *              函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...)> : implements::function_traits_base<false, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of parameter types
         *        参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of parameters
         *        参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for variadic function pointer types.
     *        Provides traits for C-style variadic function pointers.
     *
     *        可变参数函数指针类型的特化。
     *        为C风格可变参数函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Fixed parameter types of the function
     *              函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...)> : implements::function_traits_base<false, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of fixed parameter types (excluding variadic part)
         *        固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = std::tuple<Args...>;

        /**
         * @brief Number of fixed parameters
         *        固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for noexcept function pointer types.
     *        Provides traits for noexcept function pointers.
     *
     *        noexcept函数指针类型的特化。
     *        为noexcept函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Parameter types of the function
     *              函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...) noexcept> : implements::function_traits_base<false, true, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of parameter types
         *        参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of parameters
         *        参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for variadic noexcept function pointer types.
     *        Provides traits for C-style variadic noexcept function pointers.
     *
     *        可变参数noexcept函数指针类型的特化。
     *        为C风格可变参数noexcept函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Fixed parameter types of the function
     *              函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...) noexcept> : implements::function_traits_base<false, true, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of fixed parameter types (excluding variadic part)
         *        固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of fixed parameters
         *        固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /*------------------
    [normal]
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, false, false, )
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, true, false, &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, false, true, &&)
    /*------------------
    (const)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, true, false, false, const)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, true, true, false, const &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, true, false, true, const &&)
    /*------------------
    (const noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, true, false, false, const noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, true, true, false, const & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, true, false, true, const && noexcept)
    /*------------------
    (const volatile)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, true, false, false, const volatile)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, true, true, false, const volatile &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, true, false, true, const volatile &&)
    /*------------------
    (const volatile noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, true, false, false, const volatile noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, true, true, false, const volatile & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, true, false, true, const volatile && noexcept)
    /*------------------
    (noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, false, false, false, noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, false, true, false, & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, false, false, true, && noexcept)
    /*------------------
    (volatile noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, false, false, false, volatile noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, false, true, false, volatile & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, false, false, true, volatile && noexcept)
    /*------------------
    (volatile)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, false, false, false, volatile)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, false, true, false, volatile &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, false, false, true, volatile &&)

    /**
     * @brief Alias for function return type
     *        函数返回类型的别名
     *
     * @tparam Fx Function type to query
     *            要查询的函数类型
     */
    template <typename Fx>
    using function_return_type = typename function_traits<Fx>::return_type;

    /**
     * @brief Number of arguments for a function type
     *        函数类型的参数数量
     *
     * @tparam Fx Function type to query
     *            要查询的函数类型
     */
    template <typename Fx>
    static inline constexpr std::size_t arity = function_traits<Fx>::arity;

    /**
     * @brief List of argument types for a function type
     *        函数类型的参数类型列表
     *
     * @tparam Fx Function type to query
     *            要查询的函数类型
     */
    template <typename Fx>
    using function_argument_list = typename function_traits<Fx>::argument_list;

    /**
     * @brief Variable template for checking if a function type is variadic
     *        检查函数类型是否为可变参数的变量模板
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v = false;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx (*)(Args..., ...)> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...)> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...) volatile> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...) noexcept> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...) volatile noexcept> = true;

    /**
     * @brief Primary template for constructing a new function type with modified parameter lists.
     *        Allows inserting type lists at the front and end of the original argument list.
     *
     *        用于构造具有修改后参数列表的新函数类型的主模板。
     *        允许在原始参数列表的前面和后面插入类型列表。
     *
     * @tparam NewRx The new return type for the function
     *               函数的新返回类型
     * @tparam Fx The original function type to base the new type on
     *            作为新类型基础的原始函数类型
     * @tparam TypeListFront Type list to insert at the beginning of the parameter list
     *                       要插入到参数列表开头的类型列表
     * @tparam TypeListEnd Type list to insert at the end of the parameter list
     *                     要插入到参数列表末尾的类型列表
     * @tparam ArgList The original argument list (deduced from Fx)
     *                 原始参数列表（从Fx推导）
     */
    template <typename NewRx, typename Fx, typename TypeListFront = other_trans::type_list<>,
              typename TypeListEnd = other_trans::type_list<>, typename ArgList = function_argument_list<Fx>>
    struct make_normalfx_type_with_pl {};

    /**
     * @brief Specialization that performs the actual type construction.
     *        Combines front list, original arguments, and end list into a new function type.
     *
     *        执行实际类型构造的特化。
     *        将前置列表、原始参数和后置列表组合成新的函数类型。
     *
     * @tparam NewRx The new return type
     *               新的返回类型
     * @tparam Fx The original function type
     *            原始函数类型
     * @tparam TypeListFrontArgs Types to insert at the front
     *                           要插入到前面的类型
     * @tparam TypeListEndArgs Types to insert at the end
     *                         要插入到后面的类型
     * @tparam OriginalArgs The original parameter types
     *                      原始参数类型
     */
    template <typename NewRx, typename Fx, typename... TypeListFrontArgs, typename... TypeListEndArgs, typename... OriginalArgs>
    struct make_normalfx_type_with_pl<NewRx, Fx, other_trans::type_list<TypeListFrontArgs...>,
                                      other_trans::type_list<TypeListEndArgs...>,
                                      type_traits::other_trans::type_list<OriginalArgs...>> {
        /**
         * @brief Helper template for constructing the new function type.
         *        Handles both regular functions and member functions.
         *
         *        用于构造新函数类型的辅助模板。
         *        处理普通函数和成员函数。
         *
         * @tparam UFx The function type (may be same as Fx)
         *             函数类型（可能与Fx相同）
         * @tparam IsMemPtr Indicates whether this is a member function pointer
         *                  指示是否为成员函数指针
         */
        template <typename UFx, bool IsMemPtr = function_traits<UFx>::is_member_function_pointer>
        struct helper {
            using fn_traits = function_traits<UFx>;

            using prototype = NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...);

            /**
             * @brief The resulting function type, with appropriate noexcept specification
             *        最终的函数类型，带有适当的noexcept说明符
             */
            using type =
                other_trans::conditional_t<fn_traits::is_noexcept,
                                           NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept, prototype>;
        };

        /**
         * @brief Specialization for member function pointers.
         *        Member function pointers have different syntax requirements.
         *
         *        成员函数指针的特化。
         *        成员函数指针有不同的语法要求。
         *
         * @tparam UFx The member function pointer type
         *             成员函数指针类型
         */
        template <typename UFx>
        struct helper<UFx, true> {
            using fn_traits = function_traits<UFx>;

            /**
             * @brief The resulting member function pointer type
             *        最终的成员函数指针类型
             */
            using type = other_trans::conditional_t<fn_traits::is_noexcept,
                                                    NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept,
                                                    NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...)>;
        };

        /**
         * @brief The constructed function type
         *        构造完成的函数类型
         */
        using type = typename helper<Fx>::type;
    };
}

namespace rainy::utility::implements {
    template <typename Ty>
    void refwrap_ctor_fun(type_traits::helper::identity_t<Ty &>) noexcept {
    }

    template <typename Ty>
    void refwrap_ctor_fun(type_traits::helper::identity_t<Ty &&>) = delete;

    template <typename Ty, typename Uty, typename = void>
    struct refwrap_has_ctor_from : type_traits::helper::false_type {};

    template <typename Ty, typename Uty>
    struct refwrap_has_ctor_from<Ty, Uty, type_traits::other_trans::void_t<decltype(refwrap_ctor_fun<Ty>(declval<Uty>()))>>
        : type_traits::helper::true_type {};

    template <typename Fx, typename... Args>
    struct test_refwrap_nothrow_invoke {
        static auto test() {
            if constexpr (std::is_invocable_v<Fx, Args...>) {
                if constexpr (type_traits::primary_types::function_traits<Fx>::is_nothrow_invocable) {
                    return type_traits::helper::true_type{};
                } else {
                    return type_traits::helper::false_type{};
                }
            } else {
                return type_traits::helper::false_type{};
            }
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test())::value;
    };
}

namespace rainy::utility {
    /**
     * @brief A wrapper that stores a reference to an object or function.
     *        Provides reference semantics in contexts where copies are normally made.
     *
     *        存储对象或函数引用的包装器。
     *        在通常进行拷贝的上下文中提供引用语义。
     *
     * @tparam Ty The type of the referenced object or function
     *            被引用对象或函数的类型
     */
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(type_traits::implements::_is_object_v<Ty> || type_traits::implements::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        /**
         * @brief The type of the referenced object or function
         *        被引用对象或函数的类型
         */
        using type = Ty;

        /**
         * @brief Constructs a reference_wrapper from a compatible type.
         *        从兼容类型构造 reference_wrapper。
         *
         * @tparam Uty The type of the value to wrap
         *             要包装的值的类型
         * @param val The value to wrap a reference to
         *            要包装引用的值
         */
        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<
                                                            type_traits::logical_traits::negation<type_traits::type_relations::is_same<
                                                                type_traits::cv_modify::remove_cvref_t<Uty>, reference_wrapper>>,
                                                            implements::refwrap_has_ctor_from<Ty, Uty>>,
                                                        int> = 0>
        constexpr reference_wrapper(Uty &&val) noexcept(noexcept(implements::refwrap_ctor_fun<Ty>(declval<Uty>()))) {
            Ty &ref = static_cast<Uty &&>(val);
            this->reference_data = utility::addressof(ref);
        }

        reference_wrapper(const reference_wrapper &) = delete;
        reference_wrapper(reference_wrapper &&) = delete;

        /**
         * @brief Conversion operator to the referenced type.
         *        到被引用类型的转换运算符。
         *
         * @return Reference to the wrapped object
         *         被包装对象的引用
         */
        constexpr operator Ty &() const noexcept {
            return *reference_data;
        }

        /**
         * @brief Gets the wrapped reference.
         *        获取被包装的引用。
         *
         * @return Reference to the wrapped object
         *         被包装对象的引用
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get() const noexcept {
            return *reference_data;
        }

        /**
         * @brief Invokes the wrapped function with the given arguments.
         *        使用给定参数调用被包装的函数。
         *
         * @tparam Args Argument types
         *              参数类型
         * @param args Arguments to forward to the function
         *             要转发给函数的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <typename... Args, type_traits::other_trans::enable_if_t<std::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) try_to_invoke_as_function(Args &&...args) const
            noexcept(implements::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            using f_traits = type_traits::primary_types::function_traits<Ty>;
            if constexpr (f_traits::valid) {
                using return_type = typename f_traits::return_type;
                if constexpr (std::is_invocable_r_v<return_type, Ty, Args...>) {
                    if constexpr (type_traits::primary_types::is_void_v<return_type>) {
                        get(utility::forward<Args>(args)...);
                    } else {
                        return get()(utility::forward<Args>(args)...);
                    }
                }
            }
        }

        /**
         * @brief Function call operator to invoke the wrapped function.
         *        调用被包装函数的函数调用运算符。
         *
         * @tparam Args Argument types
         *              参数类型
         * @param args Arguments to forward to the function
         *             要转发给函数的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <typename... Args, type_traits::other_trans::enable_if_t<std::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) operator()(Args &&...args) const
            noexcept(implements::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            return try_to_invoke_as_function(utility::forward<Args>(args)...);
        }

    private:
        Ty *reference_data{nullptr};
    };

    /**
     * @brief Deduction guide for reference_wrapper.
     *        reference_wrapper 的推导指引。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     */
    template <class Ty>
    reference_wrapper(Ty &) -> reference_wrapper<Ty>;

    /**
     * @brief Deduction guide for reference_wrapper with cv-qualifier removal.
     *        带有 cv-限定符移除的 reference_wrapper 推导指引。
     *
     * @tparam Uty The type of the referenced object
     *             被引用对象的类型
     */
    template <typename Uty>
    reference_wrapper(Uty &) -> reference_wrapper<type_traits::cv_modify::remove_cvref_t<Uty>>;

    /**
     * @brief Deleted overload for rvalue references.
     *        右值引用的已删除重载。
     *
     * @tparam Ty The type of the rvalue
     *            右值的类型
     */
    template <typename Ty>
    void ref(const Ty &&) = delete;

    /**
     * @brief Deleted overload for rvalue references.
     *        右值引用的已删除重载。
     *
     * @tparam Ty The type of the rvalue
     *            右值的类型
     */
    template <typename Ty>
    void cref(const Ty &&) = delete;

    /**
     * @brief Creates a reference_wrapper to the given object.
     *        为给定对象创建 reference_wrapper。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The object to wrap a reference to
     *            要包装引用的对象
     * @return A reference_wrapper to the object
     *         对象的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(Ty &val) noexcept {
        return reference_wrapper<Ty>(val);
    }

    /**
     * @brief Returns the given reference_wrapper unchanged.
     *        原样返回给定的 reference_wrapper。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The reference_wrapper to return
     *            要返回的 reference_wrapper
     * @return The same reference_wrapper
     *         相同的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(reference_wrapper<Ty> val) noexcept {
        return val;
    }

    /**
     * @brief Creates a const-qualified reference_wrapper to the given object.
     *        为给定对象创建 const 限定的 reference_wrapper。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The object to wrap a const reference to
     *            要包装 const 引用的对象
     * @return A const-qualified reference_wrapper to the object
     *         对象的 const 限定 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(const Ty &val) noexcept {
        return reference_wrapper<const Ty>(val);
    }

    /**
     * @brief Returns the given reference_wrapper as a const-qualified wrapper.
     *        将给定的 reference_wrapper 作为 const 限定包装器返回。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The reference_wrapper to convert
     *            要转换的 reference_wrapper
     * @return A const-qualified reference_wrapper
     *         const 限定的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(reference_wrapper<Ty> val) noexcept {
        return val;
    }
}

namespace rainy::type_traits::cv_modify {
    /**
     * @brief Primary template for unwrapping reference wrappers.
     *        If Ty is not a reference wrapper, returns Ty unchanged.
     *
     *        解包引用包装器的主模板。
     *        如果 Ty 不是引用包装器，则原样返回 Ty。
     *
     * @tparam Ty The type to potentially unwrap
     *            可能需要解包的类型
     */
    template <typename Ty>
    struct unwrap_reference {
        using type = Ty;
    };

    /**
     * @brief Specialization for rainy::utility::reference_wrapper.
     *        Extracts the underlying type from a reference_wrapper.
     *
     *        rainy::utility::reference_wrapper 的特化。
     *        从 reference_wrapper 中提取底层类型。
     *
     * @tparam Ty The type wrapped by reference_wrapper
     *            reference_wrapper 包装的类型
     */
    template <typename Ty>
    struct unwrap_reference<utility::reference_wrapper<Ty>> {
        using type = Ty;
    };

    /**
     * @brief Specialization for std::reference_wrapper.
     *        Extracts the underlying type from a std::reference_wrapper.
     *
     *        std::reference_wrapper 的特化。
     *        从 std::reference_wrapper 中提取底层类型。
     *
     * @tparam Ty The type wrapped by std::reference_wrapper
     *            std::reference_wrapper 包装的类型
     */
    template <typename Ty>
    struct unwrap_reference<std::reference_wrapper<Ty>> {
        using type = Ty;
    };

    /**
     * @brief Alias template for unwrapping reference wrappers.
     *        解包引用包装器的别名模板。
     *
     * @tparam Ty The type to potentially unwrap
     *            可能需要解包的类型
     */
    template <typename Ty>
    using unwrap_reference_t = typename unwrap_reference<Ty>::type;

    /**
     * @brief Alias template that decays and then unwraps a type.
     *        先退化然后解包类型的别名模板。
     *
     * @tparam Ty The type to decay and unwrap
     *            要退化并解包的类型
     */
    template <typename Ty>
    using unwrap_ref_decay_t = unwrap_reference_t<other_trans::decay_t<Ty>>;

    /**
     * @brief Type template for decaying and unwrapping a type.
     *        退化并解包类型的类型模板。
     *
     * @tparam Ty The type to decay and unwrap
     *            要退化并解包的类型
     */
    template <typename Ty>
    struct unwrap_ref_decay {
        using type = unwrap_ref_decay_t<Ty>;
    };
}

namespace rainy::utility {
    enum class invoker_strategy : std::uint8_t {
        functor, // 仿函数或函数类型
        pmf_object, // 类成员函数——对象调用
        pmf_refwrap, // 类成员函数——对象引用包装器调用
        pmf_pointer, // 类成员函数——对象指针调用
        pmd_object, // 类成员变量——对象调用
        pmd_refwrap, // 类成员变量——对象引用包装器调用
        pmd_pointer // 类成员变量——对象指针调用
    };
}

namespace rainy::utility::implements {
    struct invoker_functor {
        static constexpr auto strategy = invoker_strategy::functor;

        template <typename Callable, typename... Args>
        static constexpr auto invoke(Callable &&object, Args &&...args) noexcept(
            noexcept(static_cast<Callable &&>(object)(static_cast<Args &&>(args)...))) //
            -> decltype(static_cast<Callable &&>(object)(static_cast<Args &&>(args)...)) {
            return static_cast<Callable &&>(object)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_object {
        static constexpr auto strategy = invoker_strategy::pmf_object;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept((static_cast<Ty &&>(args1).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)) {
            return (static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_refwrap {
        static constexpr auto strategy = invoker_strategy::pmf_refwrap;

        template <typename Decayed, typename Refwrap, typename... Args>
        static constexpr auto invoke(Decayed _Pmf, Refwrap _Rw,
                                     Args &&...args) noexcept(noexcept((_Rw.get().*_Pmf)(static_cast<Args &&>(args)...))) //
            -> decltype((_Rw.get().*_Pmf)(static_cast<Args &&>(args)...)) {
            return (_Rw.get().*_Pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_pointer {
        static constexpr auto strategy = invoker_strategy::pmf_pointer;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept(((*static_cast<Ty &&>(args1)).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)) {
            return ((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmd_object {
        static constexpr auto strategy = invoker_strategy::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept -> decltype(static_cast<Ty &&>(args).*pmd) {
            return static_cast<Ty &&>(args).*pmd;
        }
    };

    struct invoker_pmd_refwrap {
        static constexpr auto strategy = invoker_strategy::pmd_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty>
        static constexpr auto invoke(Decayed pmd, RefWrap<Ty> ref_wrap) noexcept -> decltype(ref_wrap.get().*pmd) {
            return ref_wrap.get().*pmd;
        }
    };

    struct invoker_pmd_pointer {
        static constexpr auto strategy = invoker_strategy::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept(noexcept((*static_cast<Ty &&>(args)).*pmd))
            -> decltype((*static_cast<Ty &&>(args)).*pmd) {
            return (*static_cast<Ty &&>(args)).*pmd;
        }
    };


    template <typename Callable, typename Ty1, typename RemoveCvref = type_traits::cv_modify::remove_cvref_t<Callable>,
              bool _Is_pmf = type_traits::primary_types::is_member_function_pointer_v<RemoveCvref>,
              bool _Is_pmd = type_traits::primary_types::is_member_object_pointer_v<RemoveCvref>>
    struct invoker_impl;

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, true, false>
        : std::conditional_t<
              std::is_same_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                             type_traits::cv_modify::remove_cvref_t<Ty1>> ||
                  std::is_base_of_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                                    type_traits::cv_modify::remove_cvref_t<Ty1>>,
              invoker_pmf_object,
              std::conditional_t<rainy::type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                                        std::reference_wrapper>,
                                 invoker_pmf_refwrap, invoker_pmf_pointer>> {};

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, false, true>
        : std::conditional_t<
              std::is_same_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                             type_traits::cv_modify::remove_cvref_t<Ty1>> ||
                  std::is_base_of_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                                    type_traits::cv_modify::remove_cvref_t<Ty1>>,
              invoker_pmd_object,
              std::conditional_t<rainy::type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                                        std::reference_wrapper>,
                                 invoker_pmd_refwrap, invoker_pmd_pointer>> {};

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, false, false> : invoker_functor {};

    template <typename Callable, typename Ty1>
    struct select_invoker {
        static constexpr auto value = invoker_impl<Callable, Ty1>::strategy;
    };
}

namespace rainy::utility {
    /**
     * @brief Primary template for function object invoker.
     *        Provides a unified interface for invoking various callable types.
     *
     *        函数对象调用器的主模板。
     *        为调用各种可调用类型提供统一接口。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Ty1 The type of the first argument (or void if not applicable)
     *             第一个参数的类型（如果不适用则为void）
     */
    template <typename Callable, typename Ty1 = void>
    struct invoker : implements::invoker_impl<Callable, Ty1> {};
}

namespace rainy::utility {
    /**
     * @brief Invokes a callable object with no arguments.
     *        调用无参数的可调用对象。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @param obj The callable object to invoke
     *            要调用的可调用对象
     * @return The result of invoking the callable object
     *         调用可调用对象的结果
     */
    template <typename Callable>
    constexpr RAINY_INLINE auto invoke(Callable &&obj) noexcept(noexcept(static_cast<Callable &&>(obj)()))
        -> decltype(static_cast<Callable &&>(obj)()) {
        return static_cast<Callable &&>(obj)();
    }

    /**
     * @brief Invokes a callable object with one or more arguments.
     *        调用带有一个或多个参数的可调用对象。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Ty1 The type of the first argument
     *             第一个参数的类型
     * @tparam Args The types of the remaining arguments
     *              剩余参数的类型
     * @param obj The callable object to invoke
     *            要调用的可调用对象
     * @param arg1 The first argument
     *             第一个参数
     * @param args The remaining arguments
     *             剩余参数
     * @return The result of invoking the callable object with the given arguments
     *         使用给定参数调用可调用对象的结果
     */
    template <typename Callable, typename Ty1, typename... Args>
    constexpr RAINY_INLINE auto invoke(Callable &&obj, Ty1 &&arg1, Args &&...args) noexcept(noexcept(
        invoker<Callable, Ty1>::invoke(static_cast<Callable &&>(obj), static_cast<Ty1 &&>(arg1), static_cast<Args &&>(args)...)))
        -> decltype(invoker<Callable, Ty1>::invoke(static_cast<Callable &&>(obj), static_cast<Ty1 &&>(arg1),
                                                   static_cast<Args &&>(args)...)) {
        return invoker<Callable, Ty1>::invoke(utility::forward<Callable>(obj), utility::forward<Ty1>(arg1),
                                              utility::forward<Args>(args)...);
    }
}

namespace rainy::utility::implements {
    template <typename Callable, typename Tuple, std::size_t... Indices>
    constexpr decltype(auto) apply_impl(Callable &&obj, Tuple &&tuple, type_traits::helper::index_sequence<Indices...>) noexcept(
        noexcept(utility::invoke(utility::forward<Callable>(obj), std::get<Indices>(utility::forward<Tuple>(tuple))...))) {
        return utility::invoke(utility::forward<Callable>(obj), std::get<Indices>(utility::forward<Tuple>(tuple))...);
    }
}

namespace rainy::utility {
    /**
     * @brief Invokes a callable object with the arguments from a tuple.
     *        使用元组中的参数调用可调用对象。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Tuple The type of the tuple containing the arguments
     *               包含参数的元组类型
     * @param obj The callable object to invoke
     *            要调用的可调用对象
     * @param tuple The tuple containing the arguments to pass to the callable
     *              包含要传递给可调用对象的参数的元组
     * @return The result of invoking the callable object with the unpacked tuple arguments
     *         使用解包后的元组参数调用可调用对象的结果
     */
    template <typename Callable, typename Tuple>
    constexpr decltype(auto) apply(Callable &&obj, Tuple &&tuple) noexcept(noexcept(implements::apply_impl(
        utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
        type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::reference_modify::remove_reference_t<Tuple>>>{}))) {
        return implements::apply_impl(
            utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
            type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::reference_modify::remove_reference_t<Tuple>>>{});
    }
}

namespace rainy::type_traits::implements {
    template <typename Rx, bool NoThrow>
    struct invoke_traits_common {
        using type = Rx;
        using is_invocable = helper::true_type;
        using is_nothrow_invocable = helper::bool_constant<NoThrow>;

        template <typename Rx_>
        using is_invocable_r = helper::bool_constant<
            logical_traits::disjunction_v<primary_types::is_void<Rx>, type_properties::is_invoke_convertible<type, Rx_>>>;

        template <typename Rx_>
        using is_nothrow_invocable_r = helper::bool_constant<logical_traits::conjunction_v<
            is_nothrow_invocable,
            logical_traits::disjunction<primary_types::is_void<Rx>,
                                        logical_traits::conjunction<type_properties::is_invoke_convertible<type, Rx_>,
                                                                    type_properties::is_invoke_nothrow_convertible<type, Rx_>>>>>;
    };

    template <typename Void, typename Callable>
    struct invoke_traits_zero {
        using is_invocable = helper::false_type;
        using is_nothrow_invocable = helper::false_type;
        template <typename Rx>
        using is_invocable_r = helper::false_type;
        template <typename Rx>
        using is_nothrow_invocable_r = helper::false_type;
        using is_void_ = primary_types::is_void<Void>;
        using Callable_type_ = Callable;
    };

    template <typename Callable>
    using decltype_invoke_zero = decltype(utility::declval<Callable>()());

    template <typename Callable>
    struct invoke_traits_zero<other_trans::void_t<decltype_invoke_zero<Callable>>, Callable>
        : invoke_traits_common<decltype_invoke_zero<Callable>, noexcept(utility::declval<Callable>()())> {};

    template <typename Void, typename... Args>
    struct invoke_traits_nonzero {
        using is_invocable = helper::false_type;
        using is_nothrow_invocable = helper::false_type;

        template <typename Rx>
        using is_invocable_r = helper::false_type;

        template <typename Rx>
        using is_nothrow_invocable_r = helper::false_type;

        using is_void_ = primary_types::is_void<Void>;
    };

    template <typename Callable, typename Ty1, typename... Args>
    using decltype_invoke_nonzero = decltype(utility::invoker<Callable, Ty1>::invoke(
        utility::declval<Callable>(), utility::declval<Ty1>(), utility::declval<Args>()...));

    template <typename Callable, typename Ty1, typename... Args>
    struct invoke_traits_nonzero<other_trans::void_t<decltype_invoke_nonzero<Callable, Ty1, Args...>>, Callable, Ty1, Args...>
        : invoke_traits_common<decltype_invoke_nonzero<Callable, Ty1, Args...>,
                               noexcept(utility::invoker<Callable, Ty1>::invoke(utility::declval<Callable>(), utility::declval<Ty1>(),
                                                                                utility::declval<Args>()...))> {};

    template <typename Callable, typename... Args>
    using select_invoke_traits = other_trans::conditional_t<sizeof...(Args) == 0, invoke_traits_zero<void, Callable>,
                                                            invoke_traits_nonzero<void, Callable, Args...>>;

    template <typename Rx, typename Callable, typename... Args>
    using is_invocable_r_helper = typename select_invoke_traits<Callable, Args...>::template is_invocable_r<Rx>;

    template <typename Callable, typename... Args>
    using is_invocable_helper = typename select_invoke_traits<Callable, Args...>::is_invocable;

    template <typename Callable, typename... Args>
    using is_nothrow_invocable_helper = typename select_invoke_traits<Callable, Args...>::is_nothrow_invocable;

    template <typename Rx, typename Callable, typename... Args>
    using is_nothrow_invocable_r_helper = typename select_invoke_traits<Callable, Args...>::template is_nothrow_invocable_r<Rx>;
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *        and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用，并且结果可以转换为指定的返回类型的变量模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_r_v = implements::is_invocable_r_helper<Rx, Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *        and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用，并且结果可以转换为指定的返回类型的类型模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_r : helper::bool_constant<is_invocable_r_v<Rx, Callable, Args...>> {};

    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments.
     *
     *        检查可调用对象是否可以使用给定参数调用的变量模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_v = implements::is_invocable_helper<Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments.
     *
     *        检查可调用对象是否可以使用给定参数调用的类型模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    struct is_invocable : helper::bool_constant<is_invocable_v<Callable, Args...>> {};

    /**
     * @brief Type template that yields the result type of invoking a callable object with the given arguments.
     *
     *        产生使用给定参数调用可调用对象的结果类型的类型模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    struct invoke_result {
        using type = typename implements::select_invoke_traits<Callable, Args...>::type;
    };

    /**
     * @brief Alias template for invoke_result, providing direct access to the result type.
     *
     *        invoke_result 的别名模板，提供对结果类型的直接访问。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    using invoke_result_t = typename invoke_result<Callable, Args...>::type;

    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常的变量模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_v = implements::is_nothrow_invocable_helper<Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常的类型模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    struct is_nothrow_invocable : helper::bool_constant<is_nothrow_invocable_v<Callable, Args...>> {};

    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions, and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常，并且结果可以转换为指定的返回类型的变量模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_r_v = implements::is_nothrow_invocable_r_helper<Rx, Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions, and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常，并且结果可以转换为指定的返回类型的类型模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    struct is_nothrow_invocable_r : helper::bool_constant<is_nothrow_invocable_r_v<Rx, Callable, Args...>> {};
}

namespace rainy::utility {
    /**
     * @brief A compressed pair that optimizes storage when one or both types are empty.
     *        Uses empty base optimization to reduce memory footprint.
     *
     *        压缩对，当一个或两个类型为空时优化存储。
     *        使用空基类优化来减少内存占用。
     *
     * @tparam Ty1 The type of the first element
     *             第一个元素的类型
     * @tparam Ty2 The type of the second element
     *             第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    class compressed_pair;
}

namespace rainy::utility::implements {
    // Internal implementation details - members are documented as they are exposed through public API
    template <typename Ty, bool = std::is_final_v<Ty>>
    struct compressed_pair_empty : std::false_type {};

    template <typename Ty>
    struct compressed_pair_empty<Ty, false> : std::is_empty<Ty> {};

    template <typename Ty1, typename Ty2, bool is_same, bool first_empty, bool second_empty>
    struct compressed_pair_switch;

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, false, false> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 0;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, true, true> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 3;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, true, false> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 1;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, false, true> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 2;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, true, true, true> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 4;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, true, false, false> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 5;
    };

    template <typename Ty1, typename Ty2, int Version>
    class compressed_pair_impl;

    template <class Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 0> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values
         *        从给定值构造两个元素
         *
         * @param x Value for the first element
         *          第一个元素的值
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first(x), second(y) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param x Value for the first element
         *          第一个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type x) : first(x) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param y Value for the second element
         *          第二个元素的值
         */
        explicit constexpr compressed_pair_impl(second_param_type y) : second(y) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element
         *        获取第一个元素的引用
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * @brief Gets a const reference to the first element
         *        获取第一个元素的常量引用
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair_impl
         *        与另一个 compressed_pair_impl 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            using std::swap;
            swap(first, pair.first);
            swap(second, pair.second);
        }

        first_type first;
        second_type second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            first(rainy::utility::get<I1>(utility::move(first_args))...), second(std::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 1> : protected type_traits::implements::remove_cv_t<Ty1> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (first via base class)
         *        从给定值构造两个元素（第一个通过基类）
         *
         * @param first Value for the first element
         *              第一个元素的值
         * @param second Value for the second element
         *               第二个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second(second) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param first Value for the first element
         *              第一个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param second Value for the second element
         *               第二个元素的值
         */
        explicit constexpr compressed_pair_impl(second_param_type second) : second(second) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element (via base class)
         *        获取第一个元素的引用（通过基类）
         */
        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the first element (via base class)
         *        获取第一个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair_impl
         *        与另一个 compressed_pair_impl 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            using std::swap;
            swap(second, pair.second);
        }

        second_type second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            type_traits::cv_modify::remove_cv_t<Ty1>(std::get<I1>(utility::move(first_args))...),
            second(std::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 2> : protected type_traits::implements::remove_cv_t<Ty2> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (second via base class)
         *        从给定值构造两个元素（第二个通过基类）
         *
         * @param x Value for the first element
         *          第一个元素的值
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : second_type(y), first(x) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param x Value for the first element
         *          第一个元素的值
         */
        constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr explicit compressed_pair_impl(second_param_type y) : second_type(y) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element
         *        获取第一个元素的引用
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * @brief Gets a const reference to the first element
         *        获取第一个元素的常量引用
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * @brief Gets a reference to the second element (via base class)
         *        获取第二个元素的引用（通过基类）
         */
        constexpr rain_fn get_second() -> second_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the second element (via base class)
         *        获取第二个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return *this;
        }

        /**
         * @brief Swaps the contents with another compressed_pair_impl
         *        与另一个 compressed_pair_impl 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            std::swap(first, pair.first);
        }

        first_type first;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            first(std::get<I1>(utility::move(first_args))...),
            type_traits::cv_modify::remove_cv_t<Ty2>(std::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 3> : protected type_traits::implements::remove_cv_t<Ty1>,
                                              protected type_traits::implements::remove_cv_t<Ty2> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (both via base classes)
         *        从给定值构造两个元素（都通过基类）
         *
         * @param first Value for the first element
         *              第一个元素的值
         * @param second Value for the second element
         *               第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second_type(second) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param first Value for the first element
         *              第一个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param second Value for the second element
         *               第二个元素的值
         */
        explicit constexpr compressed_pair_impl(second_param_type second) : second_type(second) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element (via base class)
         *        获取第一个元素的引用（通过基类）
         */
        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the first element (via base class)
         *        获取第一个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        /**
         * @brief Gets a reference to the second element (via base class)
         *        获取第二个元素的引用（通过基类）
         */
        constexpr rain_fn get_second() -> second_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the second element (via base class)
         *        获取第二个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return *this;
        }

        /**
         * @brief Swaps the contents with another compressed_pair (no-op as both are empty)
         *        与另一个 compressed_pair 交换内容（空操作，因为两者都为空）
         *
         * @param  The other pair to swap with
         *         要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &) -> void {
        }

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            type_traits::cv_modify::remove_cv_t<Ty1>(std::get<I1>(utility::move(first_args))...),
            type_traits::cv_modify::remove_cv_t<Ty2>(std::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 4> : protected type_traits::implements::remove_cv_t<Ty1> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (first via base class)
         *        从给定值构造两个元素（第一个通过基类）
         *
         * @param x Value for the first element
         *          第一个元素的值
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first_type(x), second(y) {
        }

        /**
         * @brief Constructs only the first element, second uses the same value
         *        只构造第一个元素，第二个使用相同的值
         *
         * @param x Value for both elements
         *          两个元素的值
         */
        explicit compressed_pair_impl(first_param_type x) : first_type(x), second(x) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element (via base class)
         *        获取第一个元素的引用（通过基类）
         */
        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the first element (via base class)
         *        获取第一个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair
         *        与另一个 compressed_pair 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &pair) -> void {
            std::swap(second, pair.second);
        }

        Ty2 second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            type_traits::cv_modify::remove_cv_t<Ty1>(std::get<I1>(utility::move(first_args))...),
            second(std::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 5> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values
         *        从给定值构造两个元素
         *
         * @param first Value for the first element
         *              第一个元素的值
         * @param second Value for the second element
         *               第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first(first), second(second) {
        }

        /**
         * @brief Constructs only the first element, second uses the same value
         *        只构造第一个元素，第二个使用相同的值
         *
         * @param first Value for both elements
         *              两个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first) : first(first), second(first) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element
         *        获取第一个元素的引用
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * @brief Gets a const reference to the first element
         *        获取第一个元素的常量引用
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair
         *        与另一个 compressed_pair 交换内容
         *
         * @param y The other pair to swap with
         *          要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &y) -> void {
            std::swap(first, y.first);
            std::swap(second, y.second);
        }

        Ty1 first;
        Ty2 second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            first(rainy::utility::get<I1>(utility::move(first_args))...), second(std::get<I2>(utility::move(second_args))...) {
        }
    };
}

namespace rainy::utility {
    /**
     * @brief A compressed pair that optimizes storage when one or both types are empty.
     *        Uses empty base optimization to reduce memory footprint.
     *
     *        压缩对，当一个或两个类型为空时优化存储。
     *        使用空基类优化来减少内存占用。
     *
     * @tparam Ty1 The type of the first element
     *             第一个元素的类型
     * @tparam Ty2 The type of the second element
     *             第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    class compressed_pair
        : public implements::compressed_pair_impl<
              Ty1, Ty2,
              implements::compressed_pair_switch<Ty1, Ty2,
                                                 type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty1>,
                                                                                    type_traits::cv_modify::remove_cv_t<Ty2>>,
                                                 implements::compressed_pair_empty<Ty1>::value,
                                                 implements::compressed_pair_empty<Ty2>::value>::value> {
    public:
        using base = implements::compressed_pair_impl<
            Ty1, Ty2,
            implements::compressed_pair_switch<
                Ty1, Ty2,
                type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty1>, type_traits::cv_modify::remove_cv_t<Ty2>>,
                implements::compressed_pair_empty<Ty1>::value, implements::compressed_pair_empty<Ty2>::value>::value>;

        /**
         * @brief Inherit all constructors from the base implementation
         *        继承基类实现的所有构造函数
         */
        using base::base;

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        constexpr compressed_pair(const compressed_pair &) = default;

        /**
         * @brief Move constructor
         *        移动构造函数
         */
        constexpr compressed_pair(compressed_pair &&) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         *
         * @param other The other pair to copy from
         *              要拷贝的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(const compressed_pair &other)->compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other pair to move from
         *              要移动的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(compressed_pair &&other) noexcept(std::is_nothrow_move_assignable_v<Ty1> &&
                                                                      std::is_nothrow_move_assignable_v<Ty2>)
            ->compressed_pair & {
            utility::construct_in_place(this->get_first(), utility::move(other.get_first()));
            utility::construct_in_place(this->get_second(), utility::move(other.get_second()));
            return *this;
        }
    };

    /**
     * @brief Specialization for when both types are the same.
     *        Handles the case where Ty1 and Ty2 are identical.
     *
     *        当两个类型相同时的特化。
     *        处理 Ty1 和 Ty2 相同的情况。
     *
     * @tparam Ty The type of both elements
     *            两个元素的类型
     */
    template <typename Ty>
    class compressed_pair<Ty, Ty>
        : public implements::compressed_pair_impl<
              Ty, Ty,
              implements::compressed_pair_switch<
                  Ty, Ty,
                  type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, type_traits::cv_modify::remove_cv_t<Ty>>,
                  implements::compressed_pair_empty<Ty>::value, implements::compressed_pair_empty<Ty>::value>::value> {
    public:
        using base = implements::compressed_pair_impl<
            Ty, Ty,
            implements::compressed_pair_switch<
                Ty, Ty,
                type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, type_traits::cv_modify::remove_cv_t<Ty>>,
                implements::compressed_pair_empty<Ty>::value, implements::compressed_pair_empty<Ty>::value>::value>;

        /**
         * @brief Inherit all constructors from the base implementation
         *        继承基类实现的所有构造函数
         */
        using base::base;

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        constexpr compressed_pair(const compressed_pair &) = default;

        /**
         * @brief Move constructor
         *        移动构造函数
         */
        constexpr compressed_pair(compressed_pair &&) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         *
         * @param other The other pair to copy from
         *              要拷贝的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(const compressed_pair &other)->compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other pair to move from
         *              要移动的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(compressed_pair &&other) noexcept(std::is_nothrow_move_assignable_v<Ty>)->compressed_pair & {
            this->get_first() = utility::move(other.get_first());
            this->get_second() = utility::move(other.get_second());
            return *this;
        }
    };

    /**
     * @brief Swaps two compressed pairs
     *        交换两个压缩对
     *
     * @tparam Ty1 The type of the first element
     *             第一个元素的类型
     * @tparam Ty2 The type of the second element
     *             第二个元素的类型
     * @param left The first pair
     *             第一个对
     * @param right The second pair
     *              第二个对
     */
    template <typename Ty1, typename Ty2>
    constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &left, compressed_pair<Ty1, Ty2> &right) -> void {
        left.swap(right);
    }
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Variable template for checking if a type is const-qualified.
     *        检查类型是否为 const 限定的变量模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_v = false;

    /**
     * @brief Specialization for const-qualified types.
     *        const 限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_v<const Ty> = true;

    /**
     * @brief Type template for checking if a type is const-qualified.
     *        检查类型是否为 const 限定的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_const : helper::bool_constant<is_const_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is volatile-qualified.
     *        检查类型是否为 volatile 限定的变量模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_volatile_v = false;

    /**
     * @brief Specialization for volatile-qualified types.
     *        volatile 限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_volatile_v<volatile Ty> = true;

    /**
     * @brief Type template for checking if a type is volatile-qualified.
     *        检查类型是否为 volatile 限定的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_volatile : helper::bool_constant<is_volatile_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivial.
     *        A trivial type has a trivial default constructor, copy/move constructors,
     *        copy/move assignment operators, and destructor.
     *
     *        检查类型是否为平凡类型的变量模板。
     *        平凡类型具有平凡的默认构造函数、复制/移动构造函数、
     *        复制/移动赋值运算符和析构函数。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
#if RAINY_USING_CLANG || RAINY_USING_MSVC
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivial_v = __is_trivially_constructible(Ty) && __is_trivially_copyable(Ty);
#else
    /**
     * @brief Variable template for checking if a type is trivial (GCC implementation).
     *        If type Ty is a trivial type, the instance is true; otherwise false.
     *        Trivial types are scalar types, trivially copyable class types,
     *        arrays of these types, and cv-qualified versions of these types.
     *
     *        检查类型是否为平凡类型的变量模板（GCC实现）。
     *        如果类型 Ty 是平凡类型，则实例为 true；否则为 false。
     *        平凡类型是标量类型、完全可复制类类型、这些类型的数组
     *        以及这些类型的 cv 限定版本。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivial_v = __is_trivial(Ty);
#endif

    /**
     * @brief Type template for checking if a type is trivial.
     *        检查类型是否为平凡类型的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     * @remark If type Ty is a trivial type, the instance is true; otherwise false.
     *         Trivial types are scalar types, trivially copyable class types,
     *         arrays of these types, and cv-qualified versions of these types.
     *         如果类型 Ty 是平凡类型，则实例为 true；否则为 false。
     *         平凡类型是标量类型、完全可复制类类型、这些类型的数组
     *         以及这些类型的 cv 限定版本。
     */
    template <typename Ty>
    struct is_trivial : helper::bool_constant<is_trivial_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has unique object representations.
     *        Indicates whether every object of type Ty has a unique representation
     *        (i.e., no padding bits).
     *
     *        检查类型是否具有唯一对象表示的变量模板。
     *        指示类型 Ty 的每个对象是否具有唯一的表示形式（即没有填充位）。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_unique_object_representations_v = __has_unique_object_representations(Ty);

    /**
     * @brief Type template for checking if a type has unique object representations.
     *        检查类型是否具有唯一对象表示的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct has_unique_object_representations : helper::bool_constant<has_unique_object_representations_v<Ty>> {};
}

namespace rainy::type_traits::extras::templates {
    /**
     * @brief Primary template for getting the first template parameter of a template instantiation.
     *        获取模板实例化的第一个模板参数的主模板。
     *
     * @tparam Ty The template instantiation type
     *            模板实例化类型
     */
    template <typename Ty>
    struct get_first_parameter;

    /**
     * @brief Specialization that extracts the first template parameter.
     *        提取第一个模板参数的特化。
     *
     * @tparam Ty The template template parameter
     *            模板模板参数
     * @tparam First The first template parameter type
     *               第一个模板参数类型
     * @tparam Rest The remaining template parameter types
     *              剩余的模板参数类型
     */
    template <template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct get_first_parameter<Ty<First, Rest...>> {
        using type = First;
    };

    /**
     * @brief Primary template for replacing the first template parameter of a template instantiation.
     *        替换模板实例化的第一个模板参数的主模板。
     *
     * @tparam newfirst The new type for the first parameter
     *                  第一个参数的新类型
     * @tparam Ty The template instantiation type
     *            模板实例化类型
     */
    template <typename newfirst, typename Ty>
    struct replace_first_parameter;

    /**
     * @brief Specialization that performs the replacement.
     *        执行替换操作的特化。
     *
     * @tparam NewFirst The new type for the first parameter
     *                  第一个参数的新类型
     * @tparam Ty The template template parameter
     *            模板模板参数
     * @tparam First The original first parameter type (to be replaced)
     *               原始的第一个参数类型（将被替换）
     * @tparam Rest The remaining template parameter types (preserved)
     *              剩余的模板参数类型（保持不变）
     */
    template <typename NewFirst, template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct replace_first_parameter<NewFirst, Ty<First, Rest...>> {
        using type = Ty<NewFirst, Rest...>;
    };

    /**
     * @brief Primary template for getting the pointer difference type.
     *        Defaults to ptrdiff_t.
     *
     *        获取指针差类型的主模板。
     *        默认为 ptrdiff_t。
     *
     * @tparam Ty The type to query for difference_type
     *            要查询 difference_type 的类型
     */
    template <typename, typename = void>
    struct get_ptr_difference_type {
        using type = ptrdiff_t;
    };

    /**
     * @brief Specialization for types that provide a difference_type member.
     *        为提供 difference_type 成员的类型提供的特化。
     *
     * @tparam Ty The type that provides difference_type
     *            提供 difference_type 的类型
     */
    template <typename Ty>
    struct get_ptr_difference_type<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    /**
     * @brief Primary template for getting the rebound alias of an allocator or similar template.
     *        Uses replace_first_parameter as fallback.
     *
     *        获取分配器或类似模板的重绑定别名的主模板。
     *        使用 replace_first_parameter 作为回退。
     *
     * @tparam Ty The template type to rebind
     *            要重绑定的模板类型
     * @tparam Other The new type to bind to
     *               要绑定到的新类型
     */
    template <typename Ty, typename Other, typename = void>
    struct get_rebind_alias {
        using type = typename replace_first_parameter<Other, Ty>::type;
    };

    /**
     * @brief Specialization for types that provide a rebind member template.
     *        为提供 rebind 成员模板的类型提供的特化。
     *
     * @tparam Ty The type that provides rebind
     *            提供 rebind 的类型
     * @tparam Other The new type to bind to
     *               要绑定到的新类型
     */
    template <typename Ty, typename Other>
    struct get_rebind_alias<Ty, Other, type_traits::other_trans::void_t<typename Ty::template rebind<Other>>> {
        using type = typename Ty::template rebind<Other>;
    };
}

namespace rainy::foundation::memory::implements {
    template <typename Ty, typename Elem>
    struct pointer_traits_base {
        using pointer = Ty;
        using element_type = Elem;
        using difference_type = typename type_traits::extras::templates::get_ptr_difference_type<Ty>::type;

        template <typename other>
        using rebind = typename type_traits::extras::templates::get_rebind_alias<Ty, other>::type;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Elem>, char, Elem> &;

        /**
         * @brief Creates a pointer to the given reference using the type's pointer_to function.
         *        使用类型的 pointer_to 函数创建指向给定引用的指针。
         *
         * @param val The reference to create a pointer to
         *            要创建指针的引用
         * @return A pointer created by Ty::pointer_to(val)
         *         由 Ty::pointer_to(val) 创建的指针
         */
        RAINY_NODISCARD static RAINY_CONSTEXPR20 rain_fn pointer_to(ref_type val) noexcept(noexcept(Ty::pointer_to(val))) -> pointer {
            return Ty::pointer_to(val);
        }
    };

    template <typename, typename = void, typename = void>
    struct ptr_traits_sfinae_layer {};

    template <typename Ty, typename Uty>
    struct ptr_traits_sfinae_layer<
        Ty, Uty, type_traits::other_trans::void_t<typename type_traits::extras::templates::get_first_parameter<Ty>::type>>
        : implements::pointer_traits_base<Ty, typename type_traits::extras::templates::get_first_parameter<Ty>::type> {};

    template <typename Ty>
    struct ptr_traits_sfinae_layer<Ty, type_traits::other_trans::void_t<typename Ty::element_type>, void>
        : implements::pointer_traits_base<Ty, typename Ty::element_type> {};
}

namespace rainy::foundation::memory {
    template <typename Ty>
    struct pointer_traits : implements::ptr_traits_sfinae_layer<Ty> {};

    template <typename Ty>
    struct pointer_traits<Ty *> {
        using pointer = Ty *;
        using elemen_type = Ty;
        using difference_type = ptrdiff_t;

        template <typename other>
        using rebind = other *;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Ty>, char, Ty> &;

        /**
         * @brief Creates a pointer to the given reference using addressof.
         *        使用 addressof 创建指向给定引用的指针。
         *
         * @param val The reference to create a pointer to
         *            要创建指针的引用
         * @return A pointer to the referenced object
         *         指向被引用对象的指针
         */
        RAINY_NODISCARD static constexpr rain_fn pointer_to(ref_type val) noexcept -> pointer {
            return utility::addressof(val);
        }
    };
}

namespace rainy::utility {
    using foundation::memory::pointer_traits;
}

namespace rainy::utility::implements {
    /**
     * @brief Variable template for detecting if pointer_traits<Pointer> has a to_address member function.
     *        检测 pointer_traits<Pointer> 是否具有 to_address 成员函数的变量模板。
     *
     * @tparam Ty The pointer type to check
     *            要检查的指针类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_to_address = false;

    /**
     * @brief Specialization that detects the presence of pointer_traits<Pointer>::to_address.
     *        检测 pointer_traits<Pointer>::to_address 是否存在的特化。
     *
     * @tparam Ty The pointer type that provides to_address
     *            提供 to_address 的指针类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_to_address<Ty, type_traits::other_trans::void_t<decltype(foundation::memory::pointer_traits<Ty>::to_address(
                               utility::declval<const Ty &>()))>> = true;
}

namespace rainy::utility {
    /**
     * @brief Converts a raw pointer to an address (identity function).
     *        将原始指针转换为地址（恒等函数）。
     *
     * @tparam Ty The type pointed to
     *            指向的类型
     * @param val The raw pointer
     *            原始指针
     * @return The same pointer value
     *         相同的指针值
     */
    template <typename Ty>
    constexpr rain_fn to_address(Ty *const val) noexcept -> Ty * {
        static_assert(!type_traits::primary_types::is_function_v<Ty>, "Ty cannot be a function type.");
        return val;
    }

    /**
     * @brief Converts any fancy pointer to a raw address.
     *        将任何花哨指针转换为原始地址。
     *
     * @tparam Pointer The fancy pointer type
     *                 花哨指针类型
     * @param val The fancy pointer to convert
     *            要转换的花哨指针
     * @return The raw address obtained either from pointer_traits or operator->
     *         从 pointer_traits 或 operator-> 获取的原始地址
     */
    template <typename Pointer>
    RAINY_NODISCARD constexpr rain_fn to_address(const Pointer &val) noexcept -> auto {
        if constexpr (implements::has_to_address<Pointer>) {
            return pointer_traits<Pointer>::to_address(val);
        } else {
            return utility::to_address(val.operator->());
        }
    }
}

namespace rainy::type_traits::composite_types {
    /**
     * @brief Variable template for checking if a type is arithmetic.
     *        Arithmetic types include integral and floating-point types.
     *
     *        检查类型是否为算术类型的变量模板。
     *        算术类型包括整型和浮点型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = implements::is_arithmetic_v<Ty>;

    /**
     * @brief Type template for checking if a type is arithmetic.
     *        检查类型是否为算术类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_arithmetic : helper::bool_constant<is_arithmetic_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is fundamental.
     *        Fundamental types include arithmetic types, void, and nullptr_t.
     *
     *        检查类型是否为基本类型的变量模板。
     *        基本类型包括算术类型、void 和 nullptr_t。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_fundamental_v =
        is_arithmetic_v<Ty> || primary_types::is_void_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    /**
     * @brief Type template for checking if a type is fundamental.
     *        检查类型是否为基本类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_fundamental : helper::bool_constant<is_fundamental_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is an object type.
     *        Object types are cv-qualifiable and not void.
     *
     *        检查类型是否为对象类型的变量模板。
     *        对象类型可以具有 cv 限定符，且不是 void。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_object_v = type_properties::is_const_v<const Ty> && !primary_types::is_void_v<Ty>;

    /**
     * @brief Type template for checking if a type is an object type.
     *        检查类型是否为对象类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_object : helper::bool_constant<is_object_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a reference.
     *        References include both lvalue and rvalue references.
     *
     *        检查类型是否为引用类型的变量模板。
     *        引用类型包括左值引用和右值引用。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_reference_v = implements::_is_reference_v<Ty>;

    /**
     * @brief Type template for checking if a type is a reference.
     *        检查类型是否为引用类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_reference : helper::bool_constant<implements::_is_reference_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a pointer to member.
     *        Member pointers can point to either data members or member functions.
     *
     *        检查类型是否为成员指针的变量模板。
     *        成员指针可以指向数据成员或成员函数。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
#if RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_pointer_v = __is_member_pointer(Ty);
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_pointer_v =
        primary_types::is_member_object_pointer_v<Ty> || primary_types::is_member_function_pointer_v<Ty>;
#endif

    /**
     * @brief Type template for checking if a type is a pointer to member.
     *        检查类型是否为成员指针的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_member_pointer : helper::bool_constant<is_member_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is scalar.
     *        Scalar types include arithmetic, enum, pointer, member pointer, and nullptr_t.
     *
     *        检查类型是否为标量类型的变量模板。
     *        标量类型包括算术、枚举、指针、成员指针和 nullptr_t。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_scalar_v = is_arithmetic_v<Ty> || primary_types::is_enum_v<Ty> || primary_types::is_pointer_v<Ty> ||
                                       is_member_pointer_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    /**
     * @brief Type template for checking if a type is scalar.
     *        检查类型是否为标量类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_scalar : helper::bool_constant<is_scalar_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is compound.
     *        Compound types are all types that are not fundamental.
     *
     *        检查类型是否为复合类型的变量模板。
     *        复合类型是不是基本类型的所有类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_compound_v = !is_fundamental_v<Ty>;

    /**
     * @brief Type template for checking if a type is compound.
     *        检查类型是否为复合类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_compound : helper::bool_constant<is_compound_v<Ty>> {};
}

namespace rainy::utility {
    /**
     * @brief Helper function to get an overloaded member function pointer with proper type deduction.
     *        Ensures the function pointer is not null and helps with overload resolution.
     *
     *        获取重载成员函数指针的辅助函数，带有正确的类型推导。
     *        确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Class The class type containing the member function
     *               包含成员函数的类类型
     * @tparam Fx The type of the member function
     *            成员函数的类型
     * @param memfn The member function pointer
     *              成员函数指针
     * @return The same member function pointer (validated non-null)
     *         相同的成员函数指针（已验证非空）
     */
#if RAINY_HAS_CXX20
    template <typename Class, typename Fx>
        requires(type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                 !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>)
    constexpr rain_fn get_overloaded_func(Fx Class::*memfn) -> auto {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }

    /**
     * @brief Helper function to get an overloaded function pointer with proper type deduction.
     *        Ensures the function pointer is not null and helps with overload resolution.
     *
     *        获取重载函数指针的辅助函数，带有正确的类型推导。
     *        确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Fx The type of the function
     *            函数的类型
     * @param fn The function pointer
     *           函数指针
     * @return The same function pointer (validated non-null)
     *         相同的函数指针（已验证非空）
     */
    template <typename Fx>
        requires(!type_traits::primary_types::is_member_function_pointer_v<Fx *> &&
                 !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>)
    constexpr rain_fn get_overloaded_func(Fx fn) -> auto {
        assert(fn != nullptr);
        return fn;
    }
#else
    /**
     * @brief Helper function to get an overloaded member function pointer with proper type deduction (C++17 version).
     *        Ensures the function pointer is not null and helps with overload resolution.
     *
     *        获取重载成员函数指针的辅助函数，带有正确的类型推导（C++17版本）。
     *        确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Class The class type containing the member function
     *               包含成员函数的类类型
     * @tparam Fx The type of the member function
     *            成员函数的类型
     * @param memfn The member function pointer
     *              成员函数指针
     * @return The same member function pointer (validated non-null)
     *         相同的成员函数指针（已验证非空）
     */
    template <typename Class, typename Fx,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                                                        !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>,
                                                    int> = 0>
    constexpr rain_fn get_overloaded_func(Fx Class::*memfn) -> auto {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }
#endif
}

#if RAINY_HAS_CXX20

namespace rainy::type_traits::concepts {
    /**
     * @brief Concept that checks if two types are exactly the same.
     *        检查两个类型是否完全相同的概念。
     *
     * @tparam Ty1 The first type to compare
     *             要比较的第一个类型
     * @tparam Ty2 The second type to compare
     *             要比较的第二个类型
     */
    template <typename Ty1, typename Ty2>
    concept same_as = implements::is_same_v<Ty1, Ty2>;

    /**
     * @brief Concept that checks if a type is derived from another type.
     *        Requires both inheritance relationship and convertibility.
     *
     *        检查一个类型是否从另一个类型派生的概念。
     *        需要同时满足继承关系和可转换性。
     *
     * @tparam base The base class type
     *              基类类型
     * @tparam derived The derived class type to check
     *                 要检查的派生类类型
     */
    template <typename base, typename derived>
    concept derived_from =
        __is_base_of(base, derived) && type_relations::is_convertible_v<const volatile derived *, const volatile base *>;

    /**
     * @brief Concept that checks if a type is one of the given types.
     *        检查一个类型是否属于给定类型之一的概念。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     * @tparam Types The pack of possible types
     *               可能的类型包
     */
    template <typename Ty, typename... Types>
    concept in_types = (type_traits::type_relations::is_any_of_v<Ty, Types...>);
}

#if __has_include("format")
#include <format>

namespace rainy::type_traits::concepts {
    /**
     * @brief Concept that checks if a type is formattable with the given context and formatter.
     *        Requires both const and non-const formatter operations to be valid.
     *
     *        检查类型是否可以使用给定的上下文和格式化器进行格式化的概念。
     *        要求常量和非常量格式化器操作都有效。
     *
     * @tparam Ty The type to check for formattability
     *            要检查可格式化性的类型
     * @tparam Context The format context type (defaults to std::format_context)
     *                 格式化上下文类型（默认为 std::format_context）
     * @tparam Formatter The formatter type for Ty (deduced from Context)
     *                   Ty 的格式化器类型（从 Context 推导）
     */
    template <typename Ty, typename Context = std::format_context,
              typename Formatter = typename Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with = std::semiregular<Formatter> &&
                               requires(Formatter &formatter, const Formatter &const_formatter, Ty &&type, Context format_context,
                                        std::basic_format_parse_context<typename Context::char_type> parse_context) {
                                   { formatter.parse(parse_context) } -> same_as<typename decltype(parse_context)::iterator>;
                                   { const_formatter.format(type, format_context) } -> same_as<typename Context::iterator>;
                               };

    /**
     * @brief Concept that checks if a type is formattable with non-const formatter only.
     *        Similar to formattable_with but only requires non-const formatter operations.
     *
     *        检查类型是否仅使用非常量格式化器即可进行格式化的概念。
     *        类似于 formattable_with，但仅要求非常量格式化器操作有效。
     *
     * @tparam Ty The type to check for formattability
     *            要检查可格式化性的类型
     * @tparam Context The format context type (defaults to std::format_context)
     *                 格式化上下文类型（默认为 std::format_context）
     * @tparam Formatter The formatter type for Ty (deduced from Context)
     *                   Ty 的格式化器类型（从 Context 推导）
     */
    template <typename Ty, typename Context = std::format_context,
              typename Formatter = typename Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with_non_const =
        std::semiregular<Formatter> && requires(Formatter &formatter, Ty &&type, Context format_context,
                                                std::basic_format_parse_context<typename Context::char_type> parse_context) {
            { formatter.parse(format_context) } -> std::same_as<typename decltype(parse_context)::iterator>;
            { formatter.format(type, format_context) } -> std::same_as<typename Context::iterator>;
        };
}

#endif

#endif

namespace rainy::type_traits::extras::templates {
    /**
     * @brief Primary template for template traits.
     *        Provides information about whether a type is a template instantiation.
     *
     *        模板特性的主模板。
     *        提供关于类型是否为模板实例化的信息。
     *
     * @tparam Template The type to examine
     *                  要检查的类型
     */
    template <typename Template>
    struct template_traits : helper::false_type {};

    /**
     * @brief Specialization for actual template instantiations.
     *        Extracts the template template parameter and the type arguments.
     *
     *        实际模板实例化的特化。
     *        提取模板模板参数和类型参数。
     *
     * @tparam Template The template template parameter
     *                  模板模板参数
     * @tparam Types The template arguments
     *               模板参数
     */
    template <template <typename...> typename Template, typename... Types>
    struct template_traits<Template<Types...>> : helper::true_type {
        /**
         * @brief Type list containing all template arguments.
         *        包含所有模板参数的类型列表。
         */
        using types = other_trans::type_list<Types...>;
    };

    /**
     * @brief Variable template for checking if a type is a template instantiation.
     *        检查类型是否为模板实例化的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_template_v = template_traits<Ty>::value;

    /**
     * @brief Type template for checking if a type is a template instantiation.
     *        检查类型是否为模板实例化的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_template : helper::bool_constant<is_template_v<Ty>> {};
}

namespace rainy::type_traits::primary_types {
    using extras::templates::is_template;
    using extras::templates::template_traits;

    using extras::templates::is_template_v;
}

namespace rainy::core {
    enum class method_flags : std::uint8_t {
        none = 0,
        static_specified = 1, // static method
        memfn_specified = 2, // member method
        noexcept_specified = 4, // noexcept
        lvalue_qualified = 8, // left qualifier (e.g. &)
        rvalue_qualified = 16, // right qualifier (e.g. &&)
        const_qualified = 32, // const
        volatile_qualified = 64, // volatile
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(method_flags);

    /**
     * @brief Deduces the method flags for a given function type and arguments at compile time.
     *        Determines whether the function is static, member function, const/volatile qualified,
     *        ref-qualified, and whether the invocation is noexcept.
     *
     *        在编译时推导给定函数类型和参数的方法标志。
     *        确定函数是静态函数、成员函数、const/volatile限定、引用限定，
     *        以及调用是否为noexcept。
     *
     * @tparam Fx The function type to analyze
     *            要分析的函数类型
     * @tparam Args The argument types that would be used to invoke the function
     *              将用于调用函数的参数类型
     * @return method_flags A bitmask of method flags indicating the function's properties
     *                      method_flags 位掩码，指示函数的属性
     */
    template <typename Fx, typename... Args>
    constexpr rain_fn deduction_invoker_type() noexcept -> method_flags {
        auto flag{method_flags::none};
        using traits = type_traits::primary_types::function_traits<Fx>;
        if constexpr (!traits::valid) {
            return flag;
        }
        if constexpr (!type_traits::primary_types::is_member_function_pointer_v<Fx>) {
            constexpr bool noexcept_invoke = noexcept(utility::invoke(utility::declval<Fx>(), utility::declval<Args>()...));
            flag |=
                (noexcept_invoke ? method_flags::static_specified | method_flags::noexcept_specified : method_flags::static_specified);
        } else {
            flag |= method_flags::memfn_specified;
            using method_traits = type_traits::primary_types::member_pointer_traits<Fx>;
            using raw_class_type = typename method_traits::class_type;
            bool noexcept_invoke{false};
            if constexpr (traits::is_invoke_for_lvalue || traits::is_invoke_for_rvalue) {
                if constexpr (traits::is_invoke_for_lvalue) {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type &>(), utility::declval<Args>()...));
                } else {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type &&>(), utility::declval<Args>()...));
                }
            } else {
                noexcept_invoke = noexcept(
                    utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type *>(), utility::declval<Args>()...));
            }
            if constexpr (traits::is_const_member_function) {
                flag |= method_flags::const_qualified;
            }
            if constexpr (traits::is_volatile) {
                flag |= method_flags::volatile_qualified;
            }
            if constexpr (traits::is_invoke_for_lvalue) {
                flag |= method_flags::lvalue_qualified;
            } else if constexpr (traits::is_invoke_for_rvalue) {
                flag |= method_flags::rvalue_qualified;
            }
            if (noexcept_invoke) {
                flag |= method_flags::noexcept_specified;
            }
        }
        return flag;
    }
}

namespace rainy::core::implements {
    struct poly_inspector {
        template <typename Type>
        operator Type &&() const;

        template <std::size_t Member, typename... Args>
        RAINY_NODISCARD poly_inspector invoke(Args &&...args) const;

        template <std::size_t Member, typename... Args>
        RAINY_NODISCARD poly_inspector invoke(Args &&...args);
    };

    template <typename Concept>
    struct make_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;

        template <auto... Candidate>
        static auto make(type_traits::other_trans::value_list<Candidate...>) noexcept
            -> decltype(std::make_tuple(vtable_entry(Candidate)...));

        template <typename... Func>
        RAINY_NODISCARD static constexpr auto make(type_traits::other_trans::type_list<Func...>) noexcept {
            if constexpr (sizeof...(Func) == 0u) {
                return decltype(make_with_vl(typename Concept::template impl<inspector>{}))();
            } else if constexpr ((std::is_function_v<Func> && ...)) {
                return decltype(std::make_tuple(vtable_entry(std::declval<Func>())...))();
            }
        }

        template <auto... V>
        RAINY_NODISCARD static constexpr auto make_with_vl(type_traits::other_trans::value_list<V...>) noexcept {
            return decltype(std::make_tuple(vtable_entry(V)...))();
        }

        template <typename Func>
        static auto vtable_entry(Func) noexcept {
            using namespace type_traits;
            using namespace type_traits::primary_types;
            using Traits = function_traits<Func>;
            using ret = typename Traits::return_type;
            if constexpr (Traits::is_const_member_function) {
                return static_cast<pointer_modify::add_pointer_t<
                    typename make_normalfx_type_with_pl<ret, Func, other_trans::type_list<const void *>>::type>>(nullptr);
            } else if constexpr (Traits::is_member_function_pointer) {
                return static_cast<pointer_modify::add_pointer_t<
                    typename make_normalfx_type_with_pl<ret, Func, other_trans::type_list<void *>>::type>>(nullptr);
            } else {
                return static_cast<Func>(nullptr);
            }
        }

        template <typename Type, auto Candidate, typename Ret, typename PtrType, typename... Args>
        static void fill_vtable_entry(Ret (*&entry)(PtrType, Args...)) noexcept {
            if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
                entry = +[](PtrType, Args... args) -> Ret { return utility::invoke(Candidate, utility::forward<Args>(args)...); };
            } else {
                entry = +[](PtrType instance_ptr, Args... args) -> Ret {
                    return static_cast<Ret>(utility::invoke(
                        Candidate,
                        *static_cast<type_traits::cv_modify::constness_as_t<Type, std::remove_pointer_t<PtrType>> *>(instance_ptr),
                        utility::forward<Args>(args)...));
                };
            }
        }

        template <typename VtableType, typename Type, auto... Index>
        RAINY_NODISCARD static auto fill_vtable(std::index_sequence<Index...>) noexcept {
            VtableType impl{};
            (fill_vtable_entry<Type, type_traits::other_trans::value_at<Index, typename Concept::template impl<Type>>::value>(
                 std::get<Index>(impl)),
             ...);
            return impl;
        }
    };
}

namespace rainy::core {
    /**
     * @brief Virtual table provider for polymorphic concepts.
     *        Generates and stores the vtable for a given concept.
     *
     *        多态概念的虚表提供者。
     *        为给定概念生成并存储虚表。
     *
     * @tparam Concept The concept that defines the polymorphic interface
     *                 定义多态接口的概念
     */
    template <typename Concept>
    class poly_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;
        using vtable_type = decltype(implements::make_vtable<Concept>::make_with_vl(typename Concept::template impl<inspector>{}));
        static constexpr bool is_mono = std::tuple_size_v<vtable_type> == 1u;

        using type = std::conditional_t<is_mono, std::tuple_element_t<0u, vtable_type>, const vtable_type *>;

        /**
         * @brief Gets the vtable instance for a specific type.
         *        获取特定类型的虚表实例。
         *
         * @tparam Type The concrete type to get the vtable for
         *              要获取虚表的具体类型
         * @return The vtable (direct value if mono, pointer otherwise)
         *         虚表（如果是单函数则为直接值，否则为指针）
         */
        template <typename Type>
        RAINY_NODISCARD static rain_fn instance() noexcept -> type {
            static_assert(std::is_same_v<Type, std::decay_t<Type>>, "Type differs from its decayed form");
            static const vtable_type vtable = implements::make_vtable<Concept>::template fill_vtable<vtable_type, Type>(
                std::make_index_sequence<type_traits::other_trans::value_list_size_v<typename Concept::template impl<Type>>>{});
            if constexpr (is_mono) {
                return std::get<0>(vtable);
            } else {
                return &vtable;
            }
        }
    };

    /**
     * @brief Base class for polymorphic objects providing virtual call dispatching.
     *        为多态对象提供虚调用分派的基类。
     *
     * @tparam Poly The derived polymorphic type (CRTP)
     *              派生的多态类型（CRTP）
     */
    template <typename Poly>
    struct poly_base {
        /**
         * @brief Invokes a member function on a const polymorphic object.
         *        在常量多态对象上调用成员函数。
         *
         * @tparam Member The index of the member function to invoke
         *                要调用的成员函数索引
         * @tparam Args The argument types
         *              参数类型
         * @param self The const polymorphic object
         *             常量多态对象
         * @param args The arguments to forward
         *             要转发的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <std::size_t Member, typename... Args>
        rain_fn invoke(const poly_base &self, Args &&...args) const -> decltype(auto) {
            const auto &poly = static_cast<const Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return std::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }

        /**
         * @brief Invokes a member function on a mutable polymorphic object.
         *        在可变多态对象上调用成员函数。
         *
         * @tparam Member The index of the member function to invoke
         *                要调用的成员函数索引
         * @tparam Args The argument types
         *              参数类型
         * @param self The mutable polymorphic object
         *             可变多态对象
         * @param args The arguments to forward
         *             要转发的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <std::size_t Member, typename... Args>
        rain_fn invoke(poly_base &self, Args &&...args) -> decltype(auto) {
            auto &poly = static_cast<Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                static_assert(Member == 0, "Unknown member");
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return std::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }
    };

    /**
     * @brief Helper function to make polymorphic calls.
     *        进行多态调用的辅助函数。
     *
     * @tparam Member The index of the member function to invoke
     *                要调用的成员函数索引
     * @tparam Poly The polymorphic type
     *              多态类型
     * @tparam Args The argument types
     *              参数类型
     * @param self The polymorphic object
     *             多态对象
     * @param args The arguments to forward
     *             要转发的参数
     * @return The result of the function call
     *         函数调用的结果
     */
    template <std::size_t Member, typename Poly, typename... Args>
    rain_fn poly_call(Poly &&self, Args &&...args) -> decltype(auto) {
        return utility::forward<Poly>(self).template invoke<Member>(self, utility::forward<Args>(args)...);
    }

    /**
     * @brief Basic polymorphic wrapper that stores a void pointer and vtable.
     *        存储void指针和虚表的基本多态包装器。
     *
     * @tparam AbstractBody The abstract concept body that defines the interface
     *                      定义接口的抽象概念主体
     */
    template <typename AbstractBody>
    class basic_poly : AbstractBody::template type<poly_base<basic_poly<AbstractBody>>> {
    public:
        friend struct poly_base<basic_poly>;

        using abstract_type = typename AbstractBody::template type<poly_base<basic_poly>>;
        using vtable_info = poly_vtable<AbstractBody>;
        using vtable_type = typename vtable_info::type;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        basic_poly() noexcept = default;

        /**
         * @brief Constructs from a pointer to a concrete type.
         *        从指向具体类型的指针构造。
         *
         * @tparam Type The concrete type
         *              具体类型
         * @param ptr Pointer to the object to wrap
         *            要包装的对象的指针
         */
        template <typename Type>
        basic_poly(Type *ptr) noexcept : // NOLINT
            _ptr(static_cast<void *>(ptr)), vtable{vtable_info::template instance<std::remove_cv_t<std::remove_pointer_t<Type>>>()} {
        }

        /**
         * @brief Constructs a null polymorphic object.
         *        构造空多态对象。
         *
         * @param  nullptr_t
         */
        basic_poly(std::nullptr_t) { // NOLINT
        }

        /**
         * @brief Move constructor
         *        移动构造函数
         *
         * @param other The other polymorphic object to move from
         *              要移动的另一个多态对象
         */
        basic_poly(basic_poly &&other) noexcept : _ptr(other._ptr), vtable(other.vtable) {
            other._ptr = nullptr;
            other.vtable = {};
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other polymorphic object to move from
         *              要移动的另一个多态对象
         * @return Reference to this object
         *         此对象的引用
         */
        basic_poly &operator=(basic_poly &&other) noexcept {
            if (this != &other) {
                _ptr = other._ptr;
                vtable = other.vtable;
                other._ptr = nullptr;
                other.vtable = {};
            }
            return *this;
        }

        /**
         * @brief Assigns a null pointer.
         *        赋值为空指针。
         *
         * @param  nullptr_t
         * @return Reference to this object
         *         此对象的引用
         */
        basic_poly &operator=(std::nullptr_t) noexcept {
            _ptr = nullptr;
            return *this;
        }

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        basic_poly(const basic_poly &) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         */
        basic_poly &operator=(const basic_poly &) = default;

        /**
         * @brief Resets to null state.
         *        重置为空状态。
         */
        rain_fn reset() noexcept -> void {
            _ptr = nullptr;
            vtable = {};
        }

        /**
         * @brief Resets to point to a new object.
         *        重置为指向新对象。
         *
         * @tparam Type The concrete type
         *              具体类型
         * @param ptr Pointer to the new object
         *            新对象的指针
         */
        template <typename Type>
        rain_fn reset(Type *ptr) noexcept -> void {
            _ptr = static_cast<void *>(ptr);
            vtable = vtable_info::template instance<std::remove_cv_t<std::remove_pointer_t<Type>>>();
        }

        /**
         * @brief Checks if the object is empty.
         *        检查对象是否为空。
         *
         * @return true if empty, false otherwise
         *         如果为空则为true，否则为false
         */
        rain_fn empty() const noexcept -> bool {
            return vtable == nullptr;
        }

        /**
         * @brief Checks if the object holds a non-null pointer.
         *        检查对象是否持有非空指针。
         *
         * @return true if pointer is non-null, false otherwise
         *         如果指针非空则为true，否则为false
         */
        RAINY_NODISCARD explicit operator bool() const noexcept {
            return _ptr != nullptr;
        }

        /**
         * @brief Arrow operator to access the abstract interface.
         *        箭头运算符，用于访问抽象接口。
         *
         * @return Pointer to the abstract interface
         *         指向抽象接口的指针
         */
        RAINY_NODISCARD rain_fn operator->() noexcept -> abstract_type * {
            return this;
        }

        /**
         * @brief Const arrow operator to access the abstract interface.
         *        常量箭头运算符，用于访问抽象接口。
         *
         * @return Const pointer to the abstract interface
         *         指向抽象接口的常量指针
         */
        RAINY_NODISCARD rain_fn operator->() const noexcept -> const abstract_type * {
            return this;
        }

        /**
         * @brief Gets the stored pointer as void*.
         *        获取存储的指针作为void*。
         *
         * @return The stored void pointer
         *         存储的void指针
         */
        rain_fn target_as_void_ptr() const noexcept -> void * {
            return _ptr;
        }

    private:
        void *_ptr{};
        vtable_type vtable{};
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename... Args>
    struct ctor_impl {
        static constexpr Ty invoke(Args... args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) {
            return Ty(utility::forward<Args>(args)...);
        }
    };

    template <typename Ty>
    struct dtor_impl {
        static RAINY_CONSTEXPR20 void invoke(const Ty *object) noexcept(std::is_nothrow_destructible_v<Ty>) {
            if (object) {
                object->~Ty();
            }
        }
    };
}

namespace rainy::utility {
    /**
     * @brief Constructor invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking constructors.
     *
     *        带有 SFINAE 约束的构造函数调用器包装器。
     *        为调用构造函数提供统一接口。
     *
     * @tparam Ty The type to construct
     *            要构造的类型
     * @tparam Args The constructor argument types
     *              构造函数参数类型
     */
    template <typename Ty, typename... Args>
    struct ctor : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>,
                                                        implements::ctor_impl<Ty, Args...>> {};

    /**
     * @brief Destructor invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking destructors.
     *
     *        带有 SFINAE 约束的析构函数调用器包装器。
     *        为调用析构函数提供统一接口。
     *
     * @tparam Ty The type to destroy
     *            要销毁的类型
     */
    template <typename Ty>
    struct dtor
        : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, implements::dtor_impl<Ty>> {};
}

namespace rainy::utility {
    /**
     * @brief Gets a function pointer to the constructor invoker for type Ty.
     *        获取指向类型 Ty 的构造函数调用器的函数指针。
     *
     * @tparam Ty The type to construct
     *            要构造的类型
     * @tparam Args The constructor argument types
     *              构造函数参数类型
     * @return Function pointer to ctor<Ty, Args...>::invoke
     *         指向 ctor<Ty, Args...>::invoke 的函数指针
     */
    template <typename Ty, typename... Args,
              typename type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
    constexpr rain_fn get_ctor_fn() -> auto {
        return &ctor<Ty, Args...>::invoke;
    }

    /**
     * @brief Gets a function pointer to the destructor invoker for type Ty.
     *        获取指向类型 Ty 的析构函数调用器的函数指针。
     *
     * @tparam Ty The type to destroy
     *            要销毁的类型
     * @return Function pointer to dtor<Ty>::invoke
     *         指向 dtor<Ty>::invoke 的函数指针
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, int> = 0>
    constexpr rain_fn get_dtor_fn() -> auto {
        return &dtor<Ty>::invoke;
    }
}

namespace rainy::utility::implements {
    template <typename Ty, typename Assign>
    struct assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            Assign assign) noexcept(type_traits::type_properties::is_nothrow_assignable_v<Ty, Assign>) {
            return (this_ = assign);
        }
    };

    template <typename Ty>
    struct copy_assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            type_traits::reference_modify::add_rvalue_reference_t<Ty>
                &&rvalue) noexcept(type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>) {
            return (this_ = rvalue);
        }
    };

    template <typename Ty>
    struct move_assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            type_traits::reference_modify::add_rvalue_reference_t<Ty>
                rvalue) noexcept(type_traits::type_properties::is_nothrow_move_assignable_v<Ty>) {
            return (this_ = utility::move(rvalue));
        }
    };
}

namespace rainy::utility {
    /**
     * @brief Assignment operator invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking assignment operators.
     *
     *        带有 SFINAE 约束的赋值运算符调用器包装器。
     *        为调用赋值运算符提供统一接口。
     *
     * @tparam Ty The type being assigned to
     *            被赋值的类型
     * @tparam Assign The type being assigned from
     *                来源类型
     */
    template <typename Ty, typename Assign>
    struct assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                              !type_traits::type_properties::is_const_v<Ty> &&
                                                              !type_traits::composite_types::is_reference_v<Ty>,
                                                          implements::assign_impl<Ty, Assign>> {};

    /**
     * @brief Copy assignment operator invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking copy assignment operators.
     *
     *        带有 SFINAE 约束的拷贝赋值运算符调用器包装器。
     *        为调用拷贝赋值运算符提供统一接口。
     *
     * @tparam Ty The type being copy assigned
     *            被拷贝赋值的类型
     */
    template <typename Ty>
    struct copy_assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                               implements::copy_assign_impl<Ty>> {};

    /**
     * @brief Move assignment operator invoker wrapper with fallback to copy.
     *        Provides move assignment if available, otherwise falls back to copy assignment.
     *
     *        带有拷贝回退的移动赋值运算符调用器包装器。
     *        如果可用则提供移动赋值，否则回退到拷贝赋值。
     *
     * @tparam Ty The type being move assigned
     *            被移动赋值的类型
     */
    template <typename Ty>
    struct move_assign : type_traits::other_trans::conditional_t<
                             type_traits::type_properties::is_move_assignable_v<Ty>, implements::move_assign_impl<Ty>,
                             type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                                   implements::copy_assign_impl<Ty>>> {};
}

namespace rainy::utility {
    /**
     * @brief Gets a function pointer to the assignment operator invoker.
     *        获取指向赋值运算符调用器的函数指针。
     *
     * @tparam Ty The type being assigned to
     *            被赋值的类型
     * @tparam Assign The type being assigned from
     *                来源类型
     * @return Function pointer to assign<Ty, Assign>::invoke
     *         指向 assign<Ty, Assign>::invoke 的函数指针
     */
    template <typename Ty, typename Assign,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                        !type_traits::type_properties::is_const_v<Ty> &&
                                                        !type_traits::composite_types::is_reference_v<Ty>,
                                                    int> = 0>
    constexpr rain_fn get_assign() -> auto {
        return &assign<Ty, Assign>::invoke;
    }

    /**
     * @brief Gets a function pointer to the move assignment operator invoker.
     *        获取指向移动赋值运算符调用器的函数指针。
     *
     * @tparam Ty The type being move assigned
     *            被移动赋值的类型
     * @return Function pointer to move_assign<Ty>::invoke
     *         指向 move_assign<Ty>::invoke 的函数指针
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty> ||
                                                                     type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>,
                                                                 int> = 0>
    constexpr rain_fn get_move_assign() -> auto {
        return &move_assign<Ty>::invoke;
    }

    /**
     * @brief Gets a function pointer to the copy assignment operator invoker.
     *        获取指向拷贝赋值运算符调用器的函数指针。
     *
     * @tparam Ty The type being copy assigned
     *            被拷贝赋值的类型
     * @return Function pointer to copy_assign<Ty>::invoke
     *         指向 copy_assign<Ty>::invoke 的函数指针
     */
    template <typename Ty,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>, int> = 0>
    constexpr rain_fn get_copy_assign() -> auto {
        return &copy_assign<Ty>::invoke;
    }
}

namespace rainy::utility::cpp_methods {
    /**
     * @brief String representation of operator+
     *        operator+ 的字符串表示
     */
    static constexpr std::string_view method_operator_add = "operator+";

    /**
     * @brief String representation of operator-
     *        operator- 的字符串表示
     */
    static constexpr std::string_view method_operator_sub = "operator-";

    /**
     * @brief String representation of operator*
     *        operator* 的字符串表示
     */
    static constexpr std::string_view method_operator_mul = "operator*";

    /**
     * @brief String representation of operator/
     *        operator/ 的字符串表示
     */
    static constexpr std::string_view method_operator_div = "operator/";

    /**
     * @brief String representation of operator%
     *        operator% 的字符串表示
     */
    static constexpr std::string_view method_operator_mod = "operator%";

    /**
     * @brief String representation of operator==
     *        operator== 的字符串表示
     */
    static constexpr std::string_view method_operator_eq = "operator==";

    /**
     * @brief String representation of operator!=
     *        operator!= 的字符串表示
     */
    static constexpr std::string_view method_operator_neq = "operator!=";

    /**
     * @brief String representation of operator<
     *        operator< 的字符串表示
     */
    static constexpr std::string_view method_operator_lt = "operator<";

    /**
     * @brief String representation of operator>
     *        operator> 的字符串表示
     */
    static constexpr std::string_view method_operator_gt = "operator>";

    /**
     * @brief String representation of operator<=
     *        operator<= 的字符串表示
     */
    static constexpr std::string_view method_operator_le = "operator<=";

    /**
     * @brief String representation of operator>=
     *        operator>= 的字符串表示
     */
    static constexpr std::string_view method_operator_ge = "operator>=";

    /**
     * @brief String representation of operator=
     *        operator= 的字符串表示
     */
    static constexpr std::string_view method_operator_assign = "operator=";

    /**
     * @brief String representation of operator[]
     *        operator[] 的字符串表示
     */
    static constexpr std::string_view method_operator_index = "operator[]";

    /**
     * @brief String representation of operator()
     *        operator() 的字符串表示
     */
    static constexpr std::string_view method_operator_call = "operator()";

    /**
     * @brief String representation of operator->
     *        operator-> 的字符串表示
     */
    static constexpr std::string_view method_operator_arrow = "operator->";

    /**
     * @brief String representation of operator* (dereference)
     *        operator*（解引用）的字符串表示
     */
    static constexpr std::string_view method_operator_deref = "operator*";

    /**
     * @brief String representation of operator& (address-of)
     *        operator&（取地址）的字符串表示
     */
    static constexpr std::string_view method_operator_addr = "operator&";

    /**
     * @brief String representation of operator++ (prefix)
     *        operator++（前缀）的字符串表示
     */
    static constexpr std::string_view method_operator_preinc = "operator++";

    /**
     * @brief String representation of operator++ (postfix)
     *        operator++（后缀）的字符串表示
     */
    static constexpr std::string_view method_operator_postinc = "operator++(int)";

    /**
     * @brief String representation of operator-- (prefix)
     *        operator--（前缀）的字符串表示
     */
    static constexpr std::string_view method_operator_predec = "operator--";

    /**
     * @brief String representation of operator-- (postfix)
     *        operator--（后缀）的字符串表示
     */
    static constexpr std::string_view method_operator_postdec = "operator--(int)";

    /**
     * @brief String representation of operator||
     *        operator|| 的字符串表示
     */
    static constexpr std::string_view method_operator_or = "operator||";

    /**
     * @brief String representation of operator&&
     *        operator&& 的字符串表示
     */
    static constexpr std::string_view method_operator_and = "operator&&";

    /**
     * @brief String representation of operator!
     *        operator! 的字符串表示
     */
    static constexpr std::string_view method_operator_not = "operator!";

    /**
     * @brief String representation of operator|
     *        operator| 的字符串表示
     */
    static constexpr std::string_view method_operator_bit_or = "operator|";

    /**
     * @brief String representation of operator& (bitwise AND)
     *        operator&（按位与）的字符串表示
     */
    static constexpr std::string_view method_operator_bit_and = "operator&";

    /**
     * @brief String representation of operator^
     *        operator^ 的字符串表示
     */
    static constexpr std::string_view method_operator_bit_xor = "operator^";

    /**
     * @brief String representation of operator~
     *        operator~ 的字符串表示
     */
    static constexpr std::string_view method_operator_bit_not = "operator~";

    /**
     * @brief String representation of operator<<
     *        operator<< 的字符串表示
     */
    static constexpr std::string_view method_operator_shift_l = "operator<<";

    /**
     * @brief String representation of operator>>
     *        operator>> 的字符串表示
     */
    static constexpr std::string_view method_operator_shift_r = "operator>>";

    /**
     * @brief String representation of begin()
     *        begin() 的字符串表示
     */
    static constexpr std::string_view method_begin = "begin";

    /**
     * @brief String representation of end()
     *        end() 的字符串表示
     */
    static constexpr std::string_view method_end = "end";

    /**
     * @brief String representation of cbegin()
     *        cbegin() 的字符串表示
     */
    static constexpr std::string_view method_cbegin = "cbegin";

    /**
     * @brief String representation of cend()
     *        cend() 的字符串表示
     */
    static constexpr std::string_view method_cend = "cend";

    /**
     * @brief String representation of rbegin()
     *        rbegin() 的字符串表示
     */
    static constexpr std::string_view method_rbegin = "rbegin";

    /**
     * @brief String representation of rend()
     *        rend() 的字符串表示
     */
    static constexpr std::string_view method_rend = "rend";

    /**
     * @brief String representation of size()
     *        size() 的字符串表示
     */
    static constexpr std::string_view method_size = "size";

    /**
     * @brief String representation of empty()
     *        empty() 的字符串表示
     */
    static constexpr std::string_view method_empty = "empty";

    /**
     * @brief String representation of clear()
     *        clear() 的字符串表示
     */
    static constexpr std::string_view method_clear = "clear";

    /**
     * @brief String representation of push_back()
     *        push_back() 的字符串表示
     */
    static constexpr std::string_view method_push_back = "push_back";

    /**
     * @brief String representation of pop_back()
     *        pop_back() 的字符串表示
     */
    static constexpr std::string_view method_pop_back = "pop_back";

    /**
     * @brief String representation of length()
     *        length() 的字符串表示
     */
    static constexpr std::string_view method_length = "length";

    /**
     * @brief String representation of insert()
     *        insert() 的字符串表示
     */
    static constexpr std::string_view method_insert = "insert";

    /**
     * @brief String representation of erase()
     *        erase() 的字符串表示
     */
    static constexpr std::string_view method_erase = "erase";

    /**
     * @brief String representation of find()
     *        find() 的字符串表示
     */
    static constexpr std::string_view method_find = "find";

    /**
     * @brief String representation of resize()
     *        resize() 的字符串表示
     */
    static constexpr std::string_view method_resize = "resize";

    /**
     * @brief String representation of swap()
     *        swap() 的字符串表示
     */
    static constexpr std::string_view method_swap = "swap";

    /**
     * @brief String representation of at()
     *        at() 的字符串表示
     */
    static constexpr std::string_view method_at = "at";

    /**
     * @brief String representation of front()
     *        front() 的字符串表示
     */
    static constexpr std::string_view method_front = "front";

    /**
     * @brief String representation of back()
     *        back() 的字符串表示
     */
    static constexpr std::string_view method_back = "back";

    /**
     * @brief String representation of append()
     *        append() 的字符串表示
     */
    static constexpr std::string_view method_append = "append";
}

namespace rainy::core {
    /**
     * @brief Accumulates values in a range using operator+.
     *        使用 operator+ 累加范围内的值。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam Ty Initial value and result type
     *            初始值和结果类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param init Initial value for accumulation
     *             累加的初始值
     * @return The result of accumulating all elements with init
     *         使用 init 累加所有元素的结果
     */
    template <typename InputIt, typename Ty>
    RAINY_INLINE constexpr rain_fn accumulate(InputIt first, InputIt last, Ty init) -> Ty {
        for (; first != last; ++first) {
#if RAINY_HAS_CXX20
            init = utility::move(init) + *first;
#else
            init += *first;
#endif
        }
        return init;
    }

    /**
     * @brief Accumulates values in a range using a custom binary operation.
     *        使用自定义二元操作累加范围内的值。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam Ty Initial value and result type
     *            初始值和结果类型
     * @tparam BinaryOperation Binary operation type
     *                         二元操作类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param init Initial value for accumulation
     *             累加的初始值
     * @param op Binary operation to apply
     *           要应用的二元操作
     * @return The result of applying op to all elements with init
     *         使用 init 对所有元素应用 op 的结果
     */
    template <typename InputIt, typename Ty, typename BinaryOperation>
    RAINY_INLINE constexpr rain_fn accumulate(InputIt first, InputIt last, Ty init, BinaryOperation op) -> Ty {
        for (; first != last; ++first) {
            init = utility::invoke(op, init, *first);
        }
        return init;
    }
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Variable template for checking if a type is an input iterator.
     *        Input iterators support dereference and pre-increment.
     *
     *        检查类型是否为输入迭代器的变量模板。
     *        输入迭代器支持解引用和前自增。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_input_iterator_v = meta_method::has_operator_deref_v<It> && meta_method::has_operator_preinc_v<It>;

    /**
     * @brief Variable template for checking if a type is an output iterator (primary template).
     *        Output iterators support dereference as lvalue.
     *
     *        检查类型是否为输出迭代器的变量模板（主模板）。
     *        输出迭代器支持解引用作为左值。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_output_iterator_v = false;

    /**
     * @brief Specialization that checks for lvalue reference from dereference.
     *        检查解引用是否产生左值引用的特化。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL
        is_output_iterator_v<It, other_trans::enable_if_t<meta_method::has_operator_deref_v<It> &&
                                                          primary_types::is_lvalue_reference_v<decltype(*utility::declval<It &>())>>> =
            false;

    /**
     * @brief Variable template for checking if a type is a forward iterator.
     *        Forward iterators are input iterators that are copyable, default constructible,
     *        and support multiple passes.
     *
     *        检查类型是否为前向迭代器的变量模板。
     *        前向迭代器是可拷贝、可默认构造的输入迭代器，支持多次遍历。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_forward_iterator_v =
        is_input_iterator_v<It> && type_properties::is_copy_constructible_v<It> && type_properties::is_copy_assignable_v<It> &&
        type_properties::is_default_constructible_v<It>;

    /**
     * @brief Variable template for checking if a type is a bidirectional iterator.
     *        Bidirectional iterators support decrement in addition to forward iterator operations.
     *
     *        检查类型是否为双向迭代器的变量模板。
     *        双向迭代器在前向迭代器操作的基础上支持自减。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_bidirectional_iterator_v = is_forward_iterator_v<It> && meta_method::has_operator_predec_v<It>;

    /**
     * @brief Variable template for checking if a type is a random access iterator.
     *        Random access iterators support addition, indexing, and comparison operations.
     *
     *        检查类型是否为随机访问迭代器的变量模板。
     *        随机访问迭代器支持加法、索引和比较操作。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_random_access_iterator_v = is_bidirectional_iterator_v<It> && meta_method::has_operator_addition_v<It> &&
                                                       meta_method::has_operator_index_v<It> && meta_method::has_operator_lt_v<It>;

    /**
     * @brief Variable template for checking if a type is a contiguous iterator (primary template).
     *        Contiguous iterators store elements in contiguous memory.
     *
     *        检查类型是否为连续迭代器的变量模板（主模板）。
     *        连续迭代器将元素存储在连续内存中。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v = false;

    /**
     * @brief Specialization that checks if the iterator can be converted to a pointer.
     *        检查迭代器是否可以转换为指针的特化。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v<
        It,
        type_traits::other_trans::enable_if_t<is_random_access_iterator_v<It> &&
                                              primary_types::is_pointer_v<decltype(utility::to_address(utility::declval<It>()))>>> =
        true;
}

namespace rainy::utility {
    /**
     * @brief Verifies that a pointer range is valid (first <= last).
     *        验证指针范围是否有效（first <= last）。
     *
     * @tparam Ty The element type
     *            元素类型
     * @param first Pointer to the beginning of the range
     *              指向范围起始的指针
     * @param last Pointer to the end of the range
     *             指向范围末尾的指针
     */
    template <typename Ty>
    constexpr rain_fn verify_range(const Ty *const first, const Ty *const last) noexcept -> void {
        expects(first <= last, "transposed pointer range");
    }

    /**
     * @brief Variable template for determining if unwrapping should be allowed.
     *        Defaults to true unless prevent_inheriting_unwrap is defined.
     *
     *        确定是否允许解包的变量模板。
     *        默认为true，除非定义了 prevent_inheriting_unwrap。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter, typename = void>
    constexpr bool allow_inheriting_unwrap_v = true;

    /**
     * @brief Specialization that checks prevent_inheriting_unwrap.
     *        检查 prevent_inheriting_unwrap 的特化。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    constexpr bool allow_inheriting_unwrap_v<Iter, type_traits::other_trans::void_t<typename Iter::prevent_inheriting_unwrap>> =
        type_traits::implements::is_same_v<Iter, typename Iter::prevent_inheriting_unwrap>;

    /**
     * @brief Variable template for checking if a range can be verified (primary template).
     *        检查范围是否可验证的变量模板（主模板）。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     * @tparam Sentinel The sentinel type
     *                  哨兵类型
     */
    template <typename Iter, typename Sentinel = Iter, typename = void>
    constexpr bool range_verifiable_v = false;

    /**
     * @brief Specialization that detects if verify_range is callable.
     *        检测 verify_range 是否可调用的特化。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     * @tparam Sentinel The sentinel type
     *                  哨兵类型
     */
    template <typename Iter, typename Sentinel>
    constexpr bool range_verifiable_v<
        Iter, Sentinel,
        type_traits::implements::void_t<decltype(verify_range(declval<const Iter &>(), declval<const Sentinel &>()))>> =
        allow_inheriting_unwrap_v<Iter>;

    /**
     * @brief ADL-enabled range verification function.
     *        Verifies that [first, last) forms a valid iterator range.
     *
     *        启用ADL的范围验证函数。
     *        验证 [first, last) 是否构成有效的迭代器范围。
     *
     * @tparam iter The iterator type
     *              迭代器类型
     * @tparam sentinel The sentinel type
     *                  哨兵类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Sentinel to the end of the range
     *             指向范围末尾的哨兵
     */
    template <typename iter, typename sentinel>
    constexpr rain_fn adl_verify_range(const iter &first, const sentinel &last) -> void {
        if constexpr (type_traits::implements::_is_pointer_v<iter> && type_traits::implements::_is_pointer_v<sentinel>) {
            expects(first <= last, "transposed pointer range");
        } else if constexpr (range_verifiable_v<iter, sentinel>) {
            verify_range(first, last);
        }
    }
}

namespace rainy::utility {
    /**
     * @brief Computes the distance between two iterators.
     *        计算两个迭代器之间的距离。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @return The number of elements between first and last
     *         first 和 last 之间的元素数量
     */
    template <typename Iter>
    RAINY_NODISCARD constexpr rain_fn distance(Iter first, Iter last) -> std::ptrdiff_t {
        if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
            return last - first; // assume the iterator will do debug checking
        } else {
            adl_verify_range(first, last);
            std::ptrdiff_t off = 0;
            for (; first != last; ++first) {
                ++off;
            }
            return off;
        }
    }
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Variable template for checking if a type is movable.
     *        A type is movable if it is an object type, move constructible,
     *        assignable from rvalue, and swappable.
     *
     *        检查类型是否可移动的变量模板。
     *        类型如果是对象类型、可移动构造、可从右值赋值且可交换，则是可移动的。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_movable_v = composite_types::is_object_v<Ty> && type_properties::is_move_constructible_v<Ty> &&
                                        type_properties::is_assignable_v<Ty &, Ty> && type_properties::is_swappable_v<Ty>;

    /**
     * @brief Type template for checking if a type is movable.
     *        检查类型是否可移动的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_movable : helper::bool_constant<is_movable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is copyable.
     *        A type is copyable if it is copy constructible, movable,
     *        and assignable from lvalue, const lvalue, and const rvalue.
     *
     *        检查类型是否可拷贝的变量模板。
     *        类型如果是可拷贝构造、可移动、可从左值、常量左值和常量右值赋值，则是可拷贝的。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copyable_v =
        type_properties::is_copy_constructible_v<Ty> && is_movable_v<Ty> &&
        type_traits::type_properties::is_assignable_v<Ty &, Ty &> && type_traits::type_properties::is_assignable_v<Ty &, const Ty &> &&
        type_traits::type_properties::is_assignable_v<Ty &, const Ty>;

    /**
     * @brief Type template for checking if a type is copyable.
     *        检查类型是否可拷贝的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_copyable : helper::bool_constant<is_copyable_v<Ty>> {};
}

namespace rainy::utility {
    /**
     * @brief Maximum possible value for a signed difference type.
     *        有符号差类型的最大可能值。
     *
     * @tparam Diff The signed difference type
     *              有符号差类型
     */
    template <typename Diff>
    constexpr Diff max_possible_v{static_cast<type_traits::helper::make_unsigned_t<Diff>>(-1) >> 1};

    /**
     * @brief Minimum possible value for a signed difference type.
     *        有符号差类型的最小可能值。
     *
     * @tparam Diff The signed difference type
     *              有符号差类型
     */
    template <typename Diff>
    constexpr Diff min_possible_v{-max_possible_v<Diff> - 1};
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Type template for checking if a type is a sequential container.
     *        Sequential containers support push_back or are arrays.
     *
     *        检查类型是否为顺序容器的类型模板。
     *        顺序容器支持 push_back 或是数组。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    struct is_sequential_container
        : helper::bool_constant<type_traits::extras::meta_method::has_push_back_v<T> || primary_types::is_array_v<T>> {};

    /**
     * @brief Variable template for checking if a type is a sequential container.
     *        检查类型是否为顺序容器的变量模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    RAINY_CONSTEXPR_BOOL is_sequential_container_v = is_sequential_container<T>::value;

    /**
     * @brief Type template for checking if a type is an associative container.
     *        Associative containers support insert with key or key-value pairs,
     *        and do not support push_back.
     *
     *        检查类型是否为关联容器的类型模板。
     *        关联容器支持使用键或键值对的插入，不支持 push_back。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    struct is_associative_container : helper::bool_constant<type_traits::extras::meta_method::has_insert_for_key_v<T> &&
                                                            type_traits::extras::meta_method::has_insert_for_key_and_value_v<T> &&
                                                            !type_traits::extras::meta_method::has_push_back_v<T>> {};

    /**
     * @brief Variable template for checking if a type is an associative container.
     *        检查类型是否为关联容器的变量模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    RAINY_CONSTEXPR_BOOL is_associative_container_v = is_associative_container<T>::value;

    /**
     * @brief Type template for checking if a type is map-like (has key_type and mapped_type).
     *        检查类型是否为类似映射的类型（具有 key_type 和 mapped_type）的类型模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename, typename = void>
    struct is_map_like : helper::false_type {};

    /**
     * @brief Specialization that detects key_type and mapped_type members.
     *        检测 key_type 和 mapped_type 成员的特化。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    struct is_map_like<T, other_trans::void_t<typename T::key_type, typename T::mapped_type>> : helper::true_type {};

    /**
     * @brief Variable template for checking if a type is map-like.
     *        检查类型是否为类似映射的类型的变量模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    inline constexpr bool is_map_like_v = is_map_like<T>::value;
}

#endif
