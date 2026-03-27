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
#ifndef RAINY_CORE_TYPE_TRAITS_PRIMARY_TYPES_HPP
#define RAINY_CORE_TYPE_TRAITS_PRIMARY_TYPES_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/modifers.hpp>

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

#endif