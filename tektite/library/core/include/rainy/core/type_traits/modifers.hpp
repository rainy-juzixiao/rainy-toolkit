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
#ifndef RAINY_CORE_TYPE_TRAITS_MODIFERS_HPP
#define RAINY_CORE_TYPE_TRAITS_MODIFERS_HPP
#include <rainy/core/platform.hpp>

namespace rainy::type_traits::modifers {
    /**
     * \lang english
     * @brief Removes the outermost array extent from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除最外层的数组维度。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_extent {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for bounded arrays.
     *
     * @tparam Ty The element type
     * @tparam Idx The array size
     *
     * \lang simp-chinese
     * @brief 有界数组的特化。
     *
     * @tparam Ty 元素类型
     * @tparam Idx 数组大小
     */
    template <typename Ty, std::size_t Idx>
    struct remove_extent<Ty[Idx]> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for unbounded arrays.
     *
     * @tparam Ty The element type
     *
     * \lang simp-chinese
     * @brief 无界数组的特化。
     *
     * @tparam Ty 元素类型
     */
    template <typename Ty>
    struct remove_extent<Ty[]> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for remove_extent.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_extent 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_extent_t = typename remove_extent<Ty>::type;

    /**
     * \lang english
     * @brief Removes all array extents from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除所有数组维度。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_all_extents {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Recursive specialization for bounded arrays.
     *
     * @tparam Ty The element type
     * @tparam Idx The array size
     *
     * \lang simp-chinese
     * @brief 有界数组的递归特化。
     *
     * @tparam Ty 元素类型
     * @tparam Idx 数组大小
     */
    template <typename Ty, std::size_t Idx>
    struct remove_all_extents<Ty[Idx]> {
        using type = typename remove_all_extents<Ty>::type;
    };

    /**
     * \lang english
     * @brief Recursive specialization for unbounded arrays.
     *
     * @tparam Ty The element type
     *
     * \lang simp-chinese
     * @brief 无界数组的递归特化。
     *
     * @tparam Ty 元素类型
     */
    template <typename Ty>
    struct remove_all_extents<Ty[]> {
        using type = typename remove_all_extents<Ty>::type;
    };

    /**
     * \lang english
     * @brief Alias template for remove_all_extents.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_all_extents 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_all_extents_t = typename remove_all_extents<Ty>::type;

    /**
     * \lang english
     * @brief Returns the number of array dimensions (rank) of a type.
     *
     * @tparam Ty The type to examine
     *
     * \lang simp-chinese
     * @brief 返回类型的数组维度数量（秩）。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct rank : std::integral_constant<std::size_t, 0> {};

    /**
     * \lang english
     * @brief Specialization for bounded arrays that increments the rank.
     *
     * @tparam Ty The element type
     * @tparam N The array size
     *
     * \lang simp-chinese
     * @brief 有界数组的特化，增加秩计数。
     *
     * @tparam Ty 元素类型
     * @tparam N 数组大小
     */
    template <typename Ty, std::size_t N>
    struct rank<Ty[N]> : std::integral_constant<std::size_t, rank<Ty>::value + 1> {};

    /**
     * \lang english
     * @brief Specialization for unbounded arrays that increments the rank.
     *
     * @tparam Ty The element type
     *
     * \lang simp-chinese
     * @brief 无界数组的特化，增加秩计数。
     *
     * @tparam Ty 元素类型
     */
    template <typename Ty>
    struct rank<Ty[]> : std::integral_constant<std::size_t, rank<Ty>::value + 1> {};

    /**
     * \lang english
     * @brief Variable template for rank.
     *
     * @tparam Ty The type to examine
     *
     * \lang simp-chinese
     * @brief rank 的变量模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    inline constexpr std::size_t rank_v = rank<Ty>::value;

    /**
     * \lang english
     * @brief Returns the size of the Nth array dimension.
     *
     * @tparam Ty The type to examine
     * @tparam N The dimension index (default 0)
     *
     * \lang simp-chinese
     * @brief 返回第N个数组维度的大小。
     *
     * @tparam Ty 要检查的类型
     * @tparam N 维度索引（默认0）
     */
    template <typename Ty, std::size_t N = 0>
    struct extent : std::integral_constant<std::size_t, 0> {};

    /**
     * \lang english
     * @brief Specialization for the first dimension of a bounded array.
     *
     * @tparam Ty The element type
     * @tparam Size The array size
     *
     * \lang simp-chinese
     * @brief 有界数组第一维度的特化。
     *
     * @tparam Ty 元素类型
     * @tparam Size 数组大小
     */
    template <typename Ty, std::size_t Size>
    struct extent<Ty[Size], 0> : std::integral_constant<std::size_t, Size> {};

    /**
     * \lang english
     * @brief Recursive specialization for higher dimensions of a bounded array.
     *
     * @tparam Ty The element type
     * @tparam Size The array size
     * @tparam N The dimension index
     *
     * \lang simp-chinese
     * @brief 有界数组更高维度的递归特化。
     *
     * @tparam Ty 元素类型
     * @tparam Size 数组大小
     * @tparam N 维度索引
     */
    template <typename Ty, std::size_t Size, std::size_t N>
    struct extent<Ty[Size], N> : extent<Ty, N - 1> {};

    /**
     * \lang english
     * @brief Specialization for the first dimension of an unbounded array (returns 0).
     *
     * @tparam Ty The element type
     * @tparam N The dimension index
     *
     * \lang simp-chinese
     * @brief 无界数组第一维度的特化（返回0）。
     *
     * @tparam Ty 元素类型
     * @tparam N 维度索引
     */
    template <typename Ty, std::size_t N>
    struct extent<Ty[], N> : extent<Ty, N - 1> {};

    /**
     * \lang english
     * @brief Variable template for extent.
     *
     * @tparam Ty The type to examine
     * @tparam N The dimension index
     *
     * \lang simp-chinese
     * @brief extent 的变量模板。
     *
     * @tparam Ty 要检查的类型
     * @tparam N 维度索引
     */
    template <typename Ty, std::size_t N = 0>
    inline constexpr std::size_t extent_v = extent<Ty, N>::value;
}

namespace rainy::type_traits::modifers {
    /**
     * \lang english
     * @brief Alias template for remove_reference.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_reference 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_reference_t = typename remove_reference<Ty>::type;

    /**
     * \lang english
     * @brief Adds lvalue and rvalue reference types.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 添加左值和右值引用类型。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty, typename = void>
    struct add_reference {
        using lvalue = Ty;
        using rvalue = Ty;
    };

    /**
     * \lang english
     * @brief Specialization that adds references when possible.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 在可能时添加引用的特化。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_reference<Ty, other_trans::void_t<Ty &>> {
        using lvalue = Ty &;
        using rvalue = Ty &&;
    };

    /**
     * \lang english
     * @brief Adds an lvalue reference to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加左值引用。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_lvalue_reference {
        using type = typename add_reference<Ty>::lvalue;
    };

    /**
     * \lang english
     * @brief Alias template for add_lvalue_reference.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_lvalue_reference 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_lvalue_reference_t = typename add_lvalue_reference<Ty>::type;

    /**
     * \lang english
     * @brief Adds an rvalue reference to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加右值引用。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_rvalue_reference {
        using type = typename add_reference<Ty>::rvalue;
    };

    /**
     * \lang english
     * @brief Alias template for add_rvalue_reference.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_rvalue_reference 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_rvalue_reference_t = typename add_rvalue_reference<Ty>::type;

    /**
     * \lang english
     * @brief Adds a const lvalue reference to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加const左值引用。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_const_lvalue_ref {
        using type = typename add_lvalue_reference<const remove_reference_t<Ty>>::type;
    };

    /**
     * \lang english
     * @brief Alias template for add_const_lvalue_ref.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_const_lvalue_ref 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_const_lvalue_ref_t = typename add_const_lvalue_ref<Ty>::type;

    /**
     * \lang english
     * @brief Adds a const rvalue reference to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加const右值引用。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_const_rvalue_ref {
        using type = typename add_rvalue_reference<const remove_reference_t<Ty>>::type;
    };

    /**
     * \lang english
     * @brief Alias template for add_const_rvalue_ref.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_const_rvalue_ref 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_const_rvalue_ref_t = typename add_const_rvalue_ref<Ty>::type;
}

namespace rainy::type_traits::modifers {
    /**
     * \lang english
     * @brief Adds a pointer to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加指针。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty, typename = void>
    struct add_pointer {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization that adds a pointer when possible.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 在可能时添加指针的特化。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_pointer<Ty, other_trans::void_t<remove_reference_t<Ty> *>> {
        using type = remove_reference_t<Ty> *;
    };

    /**
     * \lang english
     * @brief Alias template for add_pointer.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_pointer 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_pointer_t = typename add_pointer<Ty>::type;

    /**
     * \lang english
     * @brief Removes a pointer from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除指针。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_pointer {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for plain pointers.
     *
     * @tparam Ty The pointed-to type
     *
     * \lang simp-chinese
     * @brief 普通指针的特化。
     *
     * @tparam Ty 指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for const pointers.
     *
     * @tparam Ty The pointed-to type
     *
     * \lang simp-chinese
     * @brief const指针的特化。
     *
     * @tparam Ty 指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *const> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for volatile pointers.
     *
     * @tparam Ty The pointed-to type
     *
     * \lang simp-chinese
     * @brief volatile指针的特化。
     *
     * @tparam Ty 指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *volatile> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for const volatile pointers.
     *
     * @tparam Ty The pointed-to type
     *
     * \lang simp-chinese
     * @brief const volatile指针的特化。
     *
     * @tparam Ty 指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *const volatile> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for remove_pointer.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_pointer 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_pointer_t = typename remove_pointer<Ty>::type;
}

namespace rainy::type_traits::modifers {
    /**
     * \lang english
     * @brief Adds const qualifier to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加const限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_const {
        using type = const Ty;
    };

    /**
     * \lang english
     * @brief Alias template for add_const.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_const 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_const_t = typename add_const<Ty>::type;

    /**
     * \lang english
     * @brief Adds volatile qualifier to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加volatile限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_volatile {
        using type = volatile Ty;
    };

    /**
     * \lang english
     * @brief Alias template for add_volatile.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_volatile 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_volatile_t = typename add_volatile<Ty>::type;

    /**
     * \lang english
     * @brief Adds both const and volatile qualifiers to a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 向类型添加const和volatile限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_cv {
        using type = const volatile Ty;
    };

    /**
     * \lang english
     * @brief Alias template for add_cv.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_cv 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_cv_t = typename add_cv<Ty>::type;

    /**
     * \lang english
     * @brief Adds const volatile qualifiers and lvalue reference.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 添加const volatile限定符和左值引用。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_cvlref {
        using type = add_lvalue_reference_t<add_cv_t<Ty>>;
    };

    /**
     * \lang english
     * @brief Adds const volatile qualifiers and rvalue reference.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 添加const volatile限定符和右值引用。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_cvrref {
        using type = add_rvalue_reference_t<add_cv_t<Ty>>;
    };

    /**
     * \lang english
     * @brief Alias template for add_cvlref.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_cvlref 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_cvlref_t = typename add_cvlref<Ty>::type;

    /**
     * \lang english
     * @brief Alias template for add_cvrref.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief add_cvrref 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_cvrref_t = typename add_cvrref<Ty>::type;

    /**
     * \lang english
     * @brief Removes const qualifier from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除const限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_const {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for const-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief const限定类型的特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    struct remove_const<const Ty> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for const volatile-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief const volatile限定类型的特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    struct remove_const<const volatile Ty> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for remove_const.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_const 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_const_t = typename remove_const<Ty>::type;

    /**
     * \lang english
     * @brief Removes volatile qualifier from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除volatile限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_volatile {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for volatile-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief volatile限定类型的特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    struct remove_volatile<volatile Ty> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for remove_volatile.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_volatile 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_volatile_t = typename remove_volatile<Ty>::type;

    /**
     * \lang english
     * @brief Removes both const and volatile qualifiers from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除const和volatile限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_cv {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for const-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief const限定类型的特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    struct remove_cv<const Ty> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for volatile-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief volatile限定类型的特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    struct remove_cv<volatile Ty> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for const volatile-qualified types.
     *
     * @tparam Ty The underlying type
     *
     * \lang simp-chinese
     * @brief const volatile限定类型的特化。
     *
     * @tparam Ty 底层类型
     */
    template <typename Ty>
    struct remove_cv<const volatile Ty> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for remove_cv.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_cv 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_cv_t = typename remove_cv<Ty>::type;

    /**
     * \lang english
     * @brief Removes const, volatile, and reference qualifiers from a type.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief 从类型中移除const、volatile和引用限定符。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_cvref {
        using type = remove_cv_t<remove_reference_t<Ty>>;
    };

    /**
     * \lang english
     * @brief Alias template for remove_cvref.
     *
     * @tparam Ty The type to modify
     *
     * \lang simp-chinese
     * @brief remove_cvref 的别名模板。
     *
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_cvref_t = remove_cv_t<remove_reference_t<Ty>>;

    /**
     * \lang english
     * @brief Applies const qualifier from one type to another.
     *
     * @tparam To The target type
     * @tparam From The source type providing constness
     *
     * \lang simp-chinese
     * @brief 从一个类型向另一个类型应用const限定符。
     *
     * @tparam To 目标类型
     * @tparam From 提供const限定的源类型
     */
    template <typename To, typename From>
    struct constness_as {
        using type = remove_const_t<To>;
    };

    /**
     * \lang english
     * @brief Specialization that adds const when source is const.
     *
     * @tparam To The target type
     * @tparam From The source const type
     *
     * \lang simp-chinese
     * @brief 当源类型为const时添加const的特化。
     *
     * @tparam To 目标类型
     * @tparam From 源const类型
     */
    template <typename To, typename From>
    struct constness_as<To, const From> {
        using type = const To;
    };

    /**
     * \lang english
     * @brief Alias template for constness_as.
     *
     * @tparam To The target type
     * @tparam From The source type
     *
     * \lang simp-chinese
     * @brief constness_as 的别名模板。
     *
     * @tparam To 目标类型
     * @tparam From 源类型
     */
    template <typename To, typename From>
    using constness_as_t = typename constness_as<To, From>::type;
}

#endif
