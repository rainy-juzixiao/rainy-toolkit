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
     * \lang english
     * @brief Type identity template.
     *         Simple identity template whose type member is the template parameter Ty itself.
     *         Commonly used in template metaprogramming to suppress template argument deduction.
     *
     * @tparam Ty Any type
     *
     * \lang simp-chinese
     * @brief 类型标识模板。
     *         简单的标识模板，其 type 成员就是模板参数 Ty 本身。
     *         常用于模板元编程中抑制模板参数推导。
     *
     * @tparam Ty 任意类型
     */
    template <typename Ty>
    struct type_identity {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for type identity, providing simplified access.
     *
     * @tparam Ty Any type
     *
     * \lang simp-chinese
     * @brief 类型标识模板的别名简化，提供便捷访问。
     *
     * @tparam Ty 任意类型
     */
    template <typename Ty>
    using type_identity_t = typename type_identity<Ty>::type;

    /**
     * \lang english
     * @brief Variable template for checking if a type is void.
     *         Checks whether type Ty is void (including cv-qualified void).
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为 void 的变量模板。
     *         检查类型 Ty 是否为 void 类型（包括 cv 限定的 void）。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = type_relations::is_same_v<void, Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is void.
     *         Inherits from bool_constant, providing ::value member constant indicating whether Ty is void.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为 void 的类型模板。
     *         继承自 bool_constant，提供 ::value 成员常量表示 Ty 是否为 void。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a specialization of a template.
     *         Detects whether Type is an instantiation of Template.
     *
     * @tparam Type The type to check
     * @tparam Template The template to check against
     *
     * \lang simp-chinese
     * @brief 检索类型是否具有特化的变量模板。
     *         检测 Type 是否为 Template 的实例化。
     *
     * @tparam Type 要检查的类型
     * @tparam Template 要检查的特化模板
     */
    template <typename Type, template <typename...> typename Template>
    RAINY_CONSTEXPR_BOOL is_specialization_v = false;

    template <template <typename...> typename Template, typename... Types>
    RAINY_CONSTEXPR_BOOL is_specialization_v<Template<Types...>, Template> = true;

    /**
     * \lang english
     * @brief Type template for checking if a type is a specialization of a template.
     *         Inherits from bool_constant based on is_specialization_v.
     *
     * @tparam Type The type to check
     * @tparam Template The template to check against
     *
     * \lang simp-chinese
     * @brief 检索类型是否具有特化的类型模板。
     *         基于 is_specialization_v 继承自 bool_constant。
     *
     * @tparam Type 要检查的类型
     * @tparam Template 要检查的特化模板
     */
    template <typename Type, template <typename...> typename Template>
    struct is_specialization : helper::bool_constant<is_specialization_v<Type, Template>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is an enumeration.
     *         Detects whether Ty is an enum type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为枚举类型的变量模板。
     *         检测 Ty 是否为枚举类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_enum_v = type_traits::implements::_is_enum_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is an enumeration.
     *         Inherits from bool_constant based on is_enum_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为枚举类型的类型模板。
     *         基于 is_enum_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_enum : helper::bool_constant<is_enum_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is POD (Plain Old Data).
     *         POD types are both standard layout and trivial.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为 POD（Plain Old Data）类型的变量模板。
     *         POD 类型满足标准布局且是平凡类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;

    /**
     * \lang english
     * @brief Variable template for checking if a type is integral.
     *         Detects whether Ty is an integral type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为整型的变量模板。
     *         检测 Ty 是否为整型类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v = implements::is_integral_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is integral.
     *         Inherits from bool_constant based on is_integral_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为整型的类型模板。
     *         基于 is_integral_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_integral : helper::bool_constant<is_integral_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is floating point.
     *         Detects whether Ty is a floating point type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为浮点类型的变量模板。
     *         检测 Ty 是否为浮点类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = type_traits::implements::is_floating_point_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is floating point.
     *         Inherits from bool_constant based on is_floating_point_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为浮点类型的类型模板。
     *         基于 is_floating_point_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_floating_point : helper::bool_constant<is_floating_point_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a union.
     *         Detects whether Ty is a union type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为联合体（union）的变量模板。
     *         检测 Ty 是否为联合体类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_union_v = implements::is_union_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is a union.
     *         Inherits from bool_constant based on is_union_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为联合体（union）的类型模板。
     *         基于 is_union_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_union : helper::bool_constant<is_union_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a class.
     *         Detects whether Ty is a class type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为类的变量模板。
     *         检测 Ty 是否为类类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_class_v = implements::is_class_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is a class.
     *         Inherits from bool_constant based on is_class_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为类的类型模板。
     *         基于 is_class_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_class : helper::bool_constant<implements::is_class_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a function.
     *         Detects whether Ty is a function type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为函数的变量模板。
     *         检测 Ty 是否为函数类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_function_v = type_traits::implements::_is_function_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is a function.
     *         Inherits from bool_constant based on is_function_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为函数的类型模板。
     *         基于 is_function_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_function : helper::bool_constant<type_traits::implements::_is_function_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is an lvalue reference (primary template).
     *         Primary template defaults to false.
     *
     * \lang simp-chinese
     * @brief 判断类型是否为左值引用的变量模板（主模板）。
     *         主模板默认为 false。
     */
    template <typename>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v = false;

    /**
     * \lang english
     * @brief Variable template for checking if a type is an lvalue reference (lvalue reference specialization).
     *         Specialization for lvalue reference types.
     *
     * @tparam Ty The referenced type
     *
     * \lang simp-chinese
     * @brief 判断类型是否为左值引用的变量模板（左值引用特化）。
     *         左值引用类型的特化。
     *
     * @tparam Ty 被引用的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v<Ty &> = true;

    /**
     * \lang english
     * @brief Type template for checking if a type is an lvalue reference.
     *         Inherits from bool_constant based on is_lvalue_reference_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为左值引用的类型模板。
     *         基于 is_lvalue_reference_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_lvalue_reference : helper::bool_constant<is_lvalue_reference_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is an rvalue reference (primary template).
     *         Primary template defaults to false.
     *
     * \lang simp-chinese
     * @brief 判断类型是否为右值引用的变量模板（主模板）。
     *         主模板默认为 false。
     */
    template <typename>
    RAINY_CONSTEXPR_BOOL is_rvalue_reference_v = false;

    /**
     * \lang english
     * @brief Variable template for checking if a type is an rvalue reference (rvalue reference specialization).
     *         Specialization for rvalue reference types.
     *
     * @tparam Ty The referenced type
     *
     * \lang simp-chinese
     * @brief 判断类型是否为右值引用的变量模板（右值引用特化）。
     *         右值引用类型的特化。
     *
     * @tparam Ty 被引用的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_rvalue_reference_v<Ty &&> = true;

    /**
     * \lang english
     * @brief Type template for checking if a type is an rvalue reference.
     *         Inherits from bool_constant based on is_rvalue_reference_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为右值引用的类型模板。
     *         基于 is_rvalue_reference_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_rvalue_reference : helper::bool_constant<is_rvalue_reference_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is an array.
     *         Detects whether Ty is an array type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为数组的变量模板。
     *         检测 Ty 是否为数组类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_array_v = type_traits::implements::_is_array_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is an array.
     *         Inherits from bool_constant based on is_array_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为数组的类型模板。
     *         基于 is_array_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_array : helper::bool_constant<is_array_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer.
     *         Detects whether Ty is a pointer type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为指针的变量模板。
     *         检测 Ty 是否为指针类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_v = type_traits::implements::_is_pointer_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is a pointer.
     *         Inherits from bool_constant based on is_pointer_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为指针的类型模板。
     *         基于 is_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_pointer : helper::bool_constant<is_pointer_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer reference (primary template).
     *         Checks whether the type is an lvalue or rvalue reference to a pointer.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为指针引用的变量模板（主模板）。
     *         检查类型是否为指向指针的左值或右值引用。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v = false;

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer reference (lvalue pointer reference specialization).
     *         Specialization for lvalue references to pointers.
     *
     * @tparam Ty The type pointed to
     *
     * \lang simp-chinese
     * @brief 判断类型是否为指针引用的变量模板（左值指针引用特化）。
     *         指向指针的左值引用特化。
     *
     * @tparam Ty 指针指向的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v<Ty *&> = true;

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer reference (rvalue pointer reference specialization).
     *         Specialization for rvalue references to pointers.
     *
     * @tparam Ty The type pointed to
     *
     * \lang simp-chinese
     * @brief 判断类型是否为指针引用的变量模板（右值指针引用特化）。
     *         指向指针的右值引用特化。
     *
     * @tparam Ty 指针指向的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v<Ty *&&> = true;

    /**
     * \lang english
     * @brief Variable template for checking if a type is std::nullptr_t (primary template).
     *         Primary template defaults to false.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为空指针类型 std::nullptr_t 的变量模板（主模板）。
     *         主模板默认为 false。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v = false;

    /**
     * \lang english
     * @brief Variable template for checking if a type is std::nullptr_t (specialization).
     *         Specialization for std::nullptr_t.
     *
     * \lang simp-chinese
     * @brief 判断类型是否为空指针类型 std::nullptr_t 的变量模板（特化）。
     *         std::nullptr_t 的特化。
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v<std::nullptr_t> = true;

    /**
     * \lang english
     * @brief Type template for checking if a type is std::nullptr_t.
     *         Inherits from bool_constant based on is_null_pointer_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为空指针类型的类型模板。
     *         基于 is_null_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_null_pointer : helper::bool_constant<is_null_pointer_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer to member object.
     *         Member object pointers point to non-static data members of a class.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为成员对象指针的变量模板。
     *         成员对象指针指向类的非静态数据成员。
     *
     * @tparam Ty 要检查的类型
     */
#if RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = __is_member_object_pointer(Ty);
#else
    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer to member object.
     *         Member object pointers point to non-static data members of a class.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为成员对象指针的变量模板。
     *         成员对象指针指向类的非静态数据成员。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = implements::is_member_object_pointer_<modifers::remove_cv_t<Ty>>::value;
#endif

    /**
     * \lang english
     * @brief Type template for checking if a type is a pointer to member object.
     *         Inherits from bool_constant based on is_member_object_pointer_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为成员对象指针的类型模板。
     *         基于 is_member_object_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_member_object_pointer : helper::bool_constant<is_member_object_pointer_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer to member function.
     *         Member function pointers point to non-static member functions of a class.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为成员函数指针的变量模板。
     *         成员函数指针指向类的非静态成员函数。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_function_pointer_v =
        implements::is_member_function_pointer_helper<modifers::remove_cv_t<Ty>>::value;

    /**
     * \lang english
     * @brief Type template for checking if a type is a pointer to member function.
     *         Inherits from bool_constant based on is_member_function_pointer_v.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为成员函数指针的类型模板。
     *         基于 is_member_function_pointer_v 继承自 bool_constant。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_member_function_pointer : helper::bool_constant<is_member_function_pointer_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for retrieving array size (primary template).
     *         Returns 0 for non-array types.
     *
     * @tparam Ty Type (array or non-array)
     *
     * \lang simp-chinese
     * @brief 获取数组大小的变量模板（主模板）。
     *         对于非数组类型返回 0。
     *
     * @tparam Ty 类型（数组或非数组）
     */
    template <typename Ty>
    static RAINY_INLINE_CONSTEXPR std::size_t array_size_v = 0;

    /**
     * \lang english
     * @brief Variable template for retrieving array size (array specialization).
     *         Returns the size of the array.
     *
     * @tparam Ty Array element type
     * @tparam N Array size
     *
     * \lang simp-chinese
     * @brief 获取数组大小的变量模板（数组特化）。
     *         返回数组的大小。
     *
     * @tparam Ty 数组元素类型
     * @tparam N 数组大小
     */
    template <typename Ty, std::size_t N>
    static RAINY_INLINE_CONSTEXPR std::size_t array_size_v<Ty[N]> = N;

    /**
     * \lang english
     * @brief Type template for retrieving array size.
     *         Provides ::value member constant with array size (0 for non-arrays).
     *
     * @tparam Ty Type (array or non-array)
     *
     * \lang simp-chinese
     * @brief 获取数组大小的类型模板。
     *         通过 ::value 成员常量获取数组的大小，非数组类型返回 0。
     *
     * @tparam Ty 类型（可以是数组或非数组）
     */
    template <typename Ty>
    struct array_size : helper::integral_constant<std::size_t, array_size_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for detecting unbounded array types.
     *         Returns true for array types with unknown bound (e.g., Ty[]).
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检测无界数组类型的变量模板。
     *         对于未知边界的数组类型（如 Ty[]）返回 true。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unbounded_array_v = false;

    /**
     * \lang english
     * @brief Variable template for detecting unbounded array types (partial specialization).
     *         Specialization for unbounded arrays.
     *
     * @tparam Ty Array element type
     *
     * \lang simp-chinese
     * @brief 检测无界数组类型的变量模板（偏特化）。
     *         无界数组的特化版本。
     *
     * @tparam Ty 数组元素类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unbounded_array_v<Ty[]> = true;

    /**
     * \lang english
     * @brief Type template for detecting unbounded array types.
     *         Provides ::value member constant indicating whether Ty is an unbounded array.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检测无界数组类型的类型模板。
     *         通过 ::value 成员常量指示 Ty 是否为无界数组。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_unbounded_array : helper::bool_constant<is_unbounded_array_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for detecting bounded array types.
     *         Returns true for array types with known bound (e.g., Ty[N]).
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检测有界数组类型的变量模板。
     *         对于已知边界的数组类型（如 Ty[N]）返回 true。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_bounded_array_v = false;

    /**
     * \lang english
     * @brief Variable template for detecting bounded array types (partial specialization).
     *         Specialization for bounded arrays.
     *
     * @tparam Ty Array element type
     * @tparam Ni Array size
     *
     * \lang simp-chinese
     * @brief 检测有界数组类型的变量模板（偏特化）。
     *         有界数组的特化版本。
     *
     * @tparam Ty 数组元素类型
     * @tparam Ni 数组大小
     */
    template <typename Ty, std::size_t Ni>
    RAINY_CONSTEXPR_BOOL is_bounded_array_v<Ty[Ni]> = true;

    /**
     * \lang english
     * @brief Type template for detecting bounded array types.
     *         Provides ::value member constant indicating whether Ty is a bounded array.
     *
     * @tparam Ty Type to check
     * @tparam Ni Array size (used for specialization)
     *
     * \lang simp-chinese
     * @brief 检测有界数组类型的类型模板。
     *         通过 ::value 成员常量指示 Ty 是否为有界数组。
     *
     * @tparam Ty 要检查的类型
     * @tparam Ni 数组大小（用于特化）
     */
    template <typename Ty, std::size_t Ni>
    struct is_bounded_array : helper::bool_constant<is_bounded_array_v<Ty>> {};
}

#endif
