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
#ifndef RAINY_CORE_TMP_MODIFERS_HPP
#define RAINY_CORE_TMP_MODIFERS_HPP
#include <rainy/core/tmp/sfinae_base.hpp>

namespace rainy::type_traits::array_modify {
    template <typename Ty>
    struct remove_extent { // remove array extent
        using type = Ty;
    };

    template <typename Ty, std::size_t Idx>
    struct remove_extent<Ty[Idx]> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_extent<Ty[]> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_extent_t = typename remove_extent<Ty>::type;

    template <typename Ty>
    struct remove_all_extents { // remove all array extents
        using type = Ty;
    };

    template <typename Ty, std::size_t Idx>
    struct remove_all_extents<Ty[Idx]> {
        using type = typename remove_all_extents<Ty>::type;
    };

    template <typename Ty>
    struct remove_all_extents<Ty[]> {
        using type = typename remove_all_extents<Ty>::type;
    };

    template <typename Ty>
    using remove_all_extents_t = typename remove_all_extents<Ty>::type;

    template <typename Ty>
    struct rank : std::integral_constant<std::size_t, 0> {};

    template <typename Ty, std::size_t N>
    struct rank<Ty[N]> : std::integral_constant<std::size_t, rank<Ty>::value + 1> {};

    template <typename Ty>
    struct rank<Ty[]> : std::integral_constant<std::size_t, rank<Ty>::value + 1> {};

    template <typename Ty>
    inline constexpr std::size_t rank_v = rank<Ty>::value;
    
    template <typename Ty, std::size_t N = 0>
    struct extent : std::integral_constant<std::size_t, 0> {};

    template <typename Ty, std::size_t Size>
    struct extent<Ty[Size], 0> : std::integral_constant<std::size_t, Size> {};

    template <typename Ty, std::size_t Size, std::size_t N>
    struct extent<Ty[Size], N> : extent<Ty, N - 1> {};

    template <typename Ty, std::size_t N>
    struct extent<Ty[], N> : extent<Ty, N - 1> {};

    template <typename Ty, std::size_t N = 0>
    inline constexpr std::size_t extent_v = extent<Ty, N>::value;
}

namespace rainy::type_traits::reference_modify {
    template <typename Ty>
    struct remove_reference {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_reference<Ty &> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_reference<Ty &&> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_reference_t = typename remove_reference<Ty>::type;

    template <typename Ty, typename = void>
    struct add_reference {
        using lvalue = Ty;
        using rvalue = Ty;
    };

    template <typename Ty>
    struct add_reference<Ty, other_trans::void_t<Ty &>> {
        using lvalue = Ty &;
        using rvalue = Ty &&;
    };

    template <typename Ty>
    struct add_lvalue_reference {
        using type = typename add_reference<Ty>::lvalue;
    };

    template <typename Ty>
    using add_lvalue_reference_t = typename add_lvalue_reference<Ty>::type;

    template <typename Ty>
    struct add_rvalue_reference {
        using type = typename add_reference<Ty>::rvalue;
    };

    template <typename Ty>
    using add_rvalue_reference_t = typename add_rvalue_reference<Ty>::type;

    template <typename Ty>
    struct add_const_lvalue_ref {
        using type = typename add_lvalue_reference<typename implements::_add_const<typename remove_reference<Ty>::type>::type>::type;
    };

    template <typename Ty>
    using add_const_lvalue_ref_t = typename add_const_lvalue_ref<Ty>::type;

    template <typename Ty>
    struct add_const_rvalue_ref {
        using type = typename add_rvalue_reference<typename implements::_add_const<typename remove_reference<Ty>::type>::type>::type;
    };

    template <typename Ty>
    using add_const_rvalue_ref_t = typename add_const_lvalue_ref<Ty>::type;
}

namespace rainy::type_traits::pointer_modify {
    template <typename Ty, typename = void>
    struct add_pointer { // add pointer (pointer type cannot be formed)
        using type = Ty;
    };

    template <typename Ty>
    struct add_pointer<Ty, other_trans::void_t<reference_modify::remove_reference_t<Ty> *>> { // (pointer type can be formed)
        using type = reference_modify::remove_reference_t<Ty> *;
    };

    template <typename Ty>
    using add_pointer_t = typename add_pointer<Ty>::type;

    template <typename Ty>
    struct remove_pointer {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_pointer<Ty *> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_pointer<Ty *const> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_pointer<Ty *volatile> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_pointer<Ty *const volatile> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_pointer_t = typename remove_pointer<Ty>::type;
}

// const-volatile 修改
namespace rainy::type_traits::cv_modify {
    /**
     * @brief 从类型设置常量类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_const {
        using type = const Ty;
    };

    /**
     * @brief 从类型设置常量类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_const_t = typename add_const<Ty>::type;

    /**
     * @brief 从类型设置volatile类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_volatile {
        using type = volatile Ty;
    };

    /**
     * @brief 从类型设置volatile类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_volatile_t = typename add_volatile<Ty>::type;

    /**
     * @brief 从类型设置const volatile类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct add_cv {
        using type = const volatile Ty;
    };

    /**
     * @brief 从类型设置const volatile类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using add_cv_t = typename add_cv<Ty>::type;

    template <typename Ty>
    struct add_cvlref {
        using type = add_cv_t<reference_modify::add_lvalue_reference_t<Ty>>;
    };

    template <typename Ty>
    struct add_cvrref {
        using type = add_cv_t<reference_modify::add_rvalue_reference_t<Ty>>;
    };

    template <typename Ty>
    using add_cvlref_t = typename add_cvlref<Ty>::type;

    template <typename Ty>
    using add_cvrref_t = typename add_cvrref<Ty>::type;

    /**
     * @brief 从类型创建非 const 类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_const {
        using type = Ty;
    };

    /**
     * @brief 从类型创建非 const 类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_const<const Ty> {
        using type = Ty;
    };

    /**
     * @brief 从类型创建非 const 类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_const<const volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_const_t = typename remove_const<Ty>::type;

    /**
     * @brief 从类型创建非 volatile 类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_volatile {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_volatile<volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_volatile_t = typename remove_volatile<Ty>::type;

    /**
     * @brief 从类型创建非 const volatile 类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    struct remove_cv {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<const Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<const volatile Ty> {
        using type = Ty;
    };

    /**
     * @brief 从类型创建非 const volatile 类型
     * @tparam Ty 要修改的类型
     */
    template <typename Ty>
    using remove_cv_t = typename remove_cv<Ty>::type;

    template <typename Ty>
    struct remove_cvref {
        using type = remove_cv_t<reference_modify::remove_reference_t<Ty>>;
    };

    template <typename Ty>
    using remove_cvref_t = remove_cv_t<reference_modify::remove_reference_t<Ty>>;
}

#endif