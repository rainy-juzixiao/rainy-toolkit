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
     * @brief Removes the outermost array extent from a type.
     *        从类型中移除最外层的数组维度。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_extent {
        using type = Ty;
    };

    /**
     * @brief Specialization for bounded arrays.
     *        有界数组的特化。
     *
     * @tparam Ty The element type
     *            元素类型
     * @tparam Idx The array size
     *             数组大小
     */
    template <typename Ty, std::size_t Idx>
    struct remove_extent<Ty[Idx]> {
        using type = Ty;
    };

    /**
     * @brief Specialization for unbounded arrays.
     *        无界数组的特化。
     *
     * @tparam Ty The element type
     *            元素类型
     */
    template <typename Ty>
    struct remove_extent<Ty[]> {
        using type = Ty;
    };

    /**
     * @brief Alias template for remove_extent.
     *        remove_extent 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_extent_t = typename remove_extent<Ty>::type;

    /**
     * @brief Removes all array extents from a type.
     *        从类型中移除所有数组维度。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_all_extents {
        using type = Ty;
    };

    /**
     * @brief Recursive specialization for bounded arrays.
     *        有界数组的递归特化。
     *
     * @tparam Ty The element type
     *            元素类型
     * @tparam Idx The array size
     *             数组大小
     */
    template <typename Ty, std::size_t Idx>
    struct remove_all_extents<Ty[Idx]> {
        using type = typename remove_all_extents<Ty>::type;
    };

    /**
     * @brief Recursive specialization for unbounded arrays.
     *        无界数组的递归特化。
     *
     * @tparam Ty The element type
     *            元素类型
     */
    template <typename Ty>
    struct remove_all_extents<Ty[]> {
        using type = typename remove_all_extents<Ty>::type;
    };

    /**
     * @brief Alias template for remove_all_extents.
     *        remove_all_extents 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_all_extents_t = typename remove_all_extents<Ty>::type;

    /**
     * @brief Returns the number of array dimensions (rank) of a type.
     *        返回类型的数组维度数量（秩）。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    struct rank : std::integral_constant<std::size_t, 0> {};

    /**
     * @brief Specialization for bounded arrays that increments the rank.
     *        有界数组的特化，增加秩计数。
     *
     * @tparam Ty The element type
     *            元素类型
     * @tparam N The array size
     *           数组大小
     */
    template <typename Ty, std::size_t N>
    struct rank<Ty[N]> : std::integral_constant<std::size_t, rank<Ty>::value + 1> {};

    /**
     * @brief Specialization for unbounded arrays that increments the rank.
     *        无界数组的特化，增加秩计数。
     *
     * @tparam Ty The element type
     *            元素类型
     */
    template <typename Ty>
    struct rank<Ty[]> : std::integral_constant<std::size_t, rank<Ty>::value + 1> {};

    /**
     * @brief Variable template for rank.
     *        rank 的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr std::size_t rank_v = rank<Ty>::value;

    /**
     * @brief Returns the size of the Nth array dimension.
     *        返回第N个数组维度的大小。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     * @tparam N The dimension index (default 0)
     *           维度索引（默认0）
     */
    template <typename Ty, std::size_t N = 0>
    struct extent : std::integral_constant<std::size_t, 0> {};

    /**
     * @brief Specialization for the first dimension of a bounded array.
     *        有界数组第一维度的特化。
     *
     * @tparam Ty The element type
     *            元素类型
     * @tparam Size The array size
     *              数组大小
     */
    template <typename Ty, std::size_t Size>
    struct extent<Ty[Size], 0> : std::integral_constant<std::size_t, Size> {};

    /**
     * @brief Recursive specialization for higher dimensions of a bounded array.
     *        有界数组更高维度的递归特化。
     *
     * @tparam Ty The element type
     *            元素类型
     * @tparam Size The array size
     *              数组大小
     * @tparam N The dimension index
     *           维度索引
     */
    template <typename Ty, std::size_t Size, std::size_t N>
    struct extent<Ty[Size], N> : extent<Ty, N - 1> {};

    /**
     * @brief Specialization for the first dimension of an unbounded array (returns 0).
     *        无界数组第一维度的特化（返回0）。
     *
     * @tparam Ty The element type
     *            元素类型
     * @tparam N The dimension index
     *           维度索引
     */
    template <typename Ty, std::size_t N>
    struct extent<Ty[], N> : extent<Ty, N - 1> {};

    /**
     * @brief Variable template for extent.
     *        extent 的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     * @tparam N The dimension index
     *           维度索引
     */
    template <typename Ty, std::size_t N = 0>
    inline constexpr std::size_t extent_v = extent<Ty, N>::value;
}

namespace rainy::type_traits::reference_modify {
    /**
     * @brief Removes reference qualifiers from a type.
     *        从类型中移除引用限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_reference {
        using type = Ty;
    };

    /**
     * @brief Specialization for lvalue references.
     *        左值引用的特化。
     *
     * @tparam Ty The referenced type
     *            被引用的类型
     */
    template <typename Ty>
    struct remove_reference<Ty &> {
        using type = Ty;
    };

    /**
     * @brief Specialization for rvalue references.
     *        右值引用的特化。
     *
     * @tparam Ty The referenced type
     *            被引用的类型
     */
    template <typename Ty>
    struct remove_reference<Ty &&> {
        using type = Ty;
    };

    /**
     * @brief Alias template for remove_reference.
     *        remove_reference 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_reference_t = typename remove_reference<Ty>::type;

    /**
     * @brief Adds lvalue and rvalue reference types.
     *        添加左值和右值引用类型。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty, typename = void>
    struct add_reference {
        using lvalue = Ty;
        using rvalue = Ty;
    };

    /**
     * @brief Specialization that adds references when possible.
     *        在可能时添加引用的特化。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_reference<Ty, other_trans::void_t<Ty &>> {
        using lvalue = Ty &;
        using rvalue = Ty &&;
    };

    /**
     * @brief Adds an lvalue reference to a type.
     *        向类型添加左值引用。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_lvalue_reference {
        using type = typename add_reference<Ty>::lvalue;
    };

    /**
     * @brief Alias template for add_lvalue_reference.
     *        add_lvalue_reference 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_lvalue_reference_t = typename add_lvalue_reference<Ty>::type;

    /**
     * @brief Adds an rvalue reference to a type.
     *        向类型添加右值引用。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_rvalue_reference {
        using type = typename add_reference<Ty>::rvalue;
    };

    /**
     * @brief Alias template for add_rvalue_reference.
     *        add_rvalue_reference 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_rvalue_reference_t = typename add_rvalue_reference<Ty>::type;

    /**
     * @brief Adds a const lvalue reference to a type.
     *        向类型添加const左值引用。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_const_lvalue_ref {
        using type = typename add_lvalue_reference<typename implements::_add_const<typename remove_reference<Ty>::type>::type>::type;
    };

    /**
     * @brief Alias template for add_const_lvalue_ref.
     *        add_const_lvalue_ref 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_const_lvalue_ref_t = typename add_const_lvalue_ref<Ty>::type;

    /**
     * @brief Adds a const rvalue reference to a type.
     *        向类型添加const右值引用。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_const_rvalue_ref {
        using type = typename add_rvalue_reference<typename implements::_add_const<typename remove_reference<Ty>::type>::type>::type;
    };

    /**
     * @brief Alias template for add_const_rvalue_ref.
     *        add_const_rvalue_ref 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_const_rvalue_ref_t = typename add_const_rvalue_ref<Ty>::type;
}

namespace rainy::type_traits::pointer_modify {
    /**
     * @brief Adds a pointer to a type.
     *        向类型添加指针。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty, typename = void>
    struct add_pointer {
        using type = Ty;
    };

    /**
     * @brief Specialization that adds a pointer when possible.
     *        在可能时添加指针的特化。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_pointer<Ty, other_trans::void_t<reference_modify::remove_reference_t<Ty> *>> {
        using type = reference_modify::remove_reference_t<Ty> *;
    };

    /**
     * @brief Alias template for add_pointer.
     *        add_pointer 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_pointer_t = typename add_pointer<Ty>::type;

    /**
     * @brief Removes a pointer from a type.
     *        从类型中移除指针。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_pointer {
        using type = Ty;
    };

    /**
     * @brief Specialization for plain pointers.
     *        普通指针的特化。
     *
     * @tparam Ty The pointed-to type
     *            指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *> {
        using type = Ty;
    };

    /**
     * @brief Specialization for const pointers.
     *        const指针的特化。
     *
     * @tparam Ty The pointed-to type
     *            指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *const> {
        using type = Ty;
    };

    /**
     * @brief Specialization for volatile pointers.
     *        volatile指针的特化。
     *
     * @tparam Ty The pointed-to type
     *            指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *volatile> {
        using type = Ty;
    };

    /**
     * @brief Specialization for const volatile pointers.
     *        const volatile指针的特化。
     *
     * @tparam Ty The pointed-to type
     *            指向的类型
     */
    template <typename Ty>
    struct remove_pointer<Ty *const volatile> {
        using type = Ty;
    };

    /**
     * @brief Alias template for remove_pointer.
     *        remove_pointer 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_pointer_t = typename remove_pointer<Ty>::type;
}

namespace rainy::type_traits::cv_modify {
    /**
     * @brief Adds const qualifier to a type.
     *        向类型添加const限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_const {
        using type = const Ty;
    };

    /**
     * @brief Alias template for add_const.
     *        add_const 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_const_t = typename add_const<Ty>::type;

    /**
     * @brief Adds volatile qualifier to a type.
     *        向类型添加volatile限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_volatile {
        using type = volatile Ty;
    };

    /**
     * @brief Alias template for add_volatile.
     *        add_volatile 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_volatile_t = typename add_volatile<Ty>::type;

    /**
     * @brief Adds both const and volatile qualifiers to a type.
     *        向类型添加const和volatile限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_cv {
        using type = const volatile Ty;
    };

    /**
     * @brief Alias template for add_cv.
     *        add_cv 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_cv_t = typename add_cv<Ty>::type;

    /**
     * @brief Adds const volatile qualifiers and lvalue reference.
     *        添加const volatile限定符和左值引用。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_cvlref {
        using type = reference_modify::add_lvalue_reference_t<add_cv_t<Ty>>;
    };

    /**
     * @brief Adds const volatile qualifiers and rvalue reference.
     *        添加const volatile限定符和右值引用。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct add_cvrref {
        using type = reference_modify::add_rvalue_reference_t<add_cv_t<Ty>>;
    };

    /**
     * @brief Alias template for add_cvlref.
     *        add_cvlref 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_cvlref_t = typename add_cvlref<Ty>::type;

    /**
     * @brief Alias template for add_cvrref.
     *        add_cvrref 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using add_cvrref_t = typename add_cvrref<Ty>::type;

    /**
     * @brief Removes const qualifier from a type.
     *        从类型中移除const限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_const {
        using type = Ty;
    };

    /**
     * @brief Specialization for const-qualified types.
     *        const限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    struct remove_const<const Ty> {
        using type = Ty;
    };

    /**
     * @brief Specialization for const volatile-qualified types.
     *        const volatile限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    struct remove_const<const volatile Ty> {
        using type = Ty;
    };

    /**
     * @brief Alias template for remove_const.
     *        remove_const 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_const_t = typename remove_const<Ty>::type;

    /**
     * @brief Removes volatile qualifier from a type.
     *        从类型中移除volatile限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_volatile {
        using type = Ty;
    };

    /**
     * @brief Specialization for volatile-qualified types.
     *        volatile限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    struct remove_volatile<volatile Ty> {
        using type = Ty;
    };

    /**
     * @brief Alias template for remove_volatile.
     *        remove_volatile 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_volatile_t = typename remove_volatile<Ty>::type;

    /**
     * @brief Removes both const and volatile qualifiers from a type.
     *        从类型中移除const和volatile限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_cv {
        using type = Ty;
    };

    /**
     * @brief Specialization for const-qualified types.
     *        const限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    struct remove_cv<const Ty> {
        using type = Ty;
    };

    /**
     * @brief Specialization for volatile-qualified types.
     *        volatile限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    struct remove_cv<volatile Ty> {
        using type = Ty;
    };

    /**
     * @brief Specialization for const volatile-qualified types.
     *        const volatile限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    struct remove_cv<const volatile Ty> {
        using type = Ty;
    };

    /**
     * @brief Alias template for remove_cv.
     *        remove_cv 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_cv_t = typename remove_cv<Ty>::type;

    /**
     * @brief Removes const, volatile, and reference qualifiers from a type.
     *        从类型中移除const、volatile和引用限定符。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    struct remove_cvref {
        using type = remove_cv_t<reference_modify::remove_reference_t<Ty>>;
    };

    /**
     * @brief Alias template for remove_cvref.
     *        remove_cvref 的别名模板。
     *
     * @tparam Ty The type to modify
     *            要修改的类型
     */
    template <typename Ty>
    using remove_cvref_t = remove_cv_t<reference_modify::remove_reference_t<Ty>>;

    /**
     * @brief Applies const qualifier from one type to another.
     *        从一个类型向另一个类型应用const限定符。
     *
     * @tparam To The target type
     *            目标类型
     * @tparam From The source type providing constness
     *              提供const限定的源类型
     */
    template <typename To, typename From>
    struct constness_as {
        using type = remove_const_t<To>;
    };

    /**
     * @brief Specialization that adds const when source is const.
     *        当源类型为const时添加const的特化。
     *
     * @tparam To The target type
     *            目标类型
     * @tparam From The source const type
     *              源const类型
     */
    template <typename To, typename From>
    struct constness_as<To, const From> {
        using type = const To;
    };

    /**
     * @brief Alias template for constness_as.
     *        constness_as 的别名模板。
     *
     * @tparam To The target type
     *            目标类型
     * @tparam From The source type
     *              源类型
     */
    template <typename To, typename From>
    using constness_as_t = typename constness_as<To, From>::type;
}

#endif
