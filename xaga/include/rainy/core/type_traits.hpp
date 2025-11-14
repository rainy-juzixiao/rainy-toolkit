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
#include <utility>
#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/limits.hpp>
#include <rainy/core/tmp/implements.hpp>
#include <rainy/core/tmp/helper.hpp>
#include <rainy/core/tmp/type_relations.hpp>
#include <rainy/core/tmp/modifers.hpp>
#include <rainy/core/tmp/type_list.hpp>
#include <rainy/core/tmp/value_list.hpp>
#include <rainy/core/tmp/ranges_traits.hpp>
#include <rainy/core/tmp/iter_traits.hpp>
#include <rainy/core/tmp/meta_methods.hpp>
#include <rainy/core/implements/basic_algorithm.hpp>
#if RAINY_USING_GCC
#include <rainy/core/gnu/typetraits.hpp>
#endif

namespace rainy::type_traits::other_trans {
    template <typename Ty>
    struct decay {
        using Ty1 = reference_modify::remove_reference_t<Ty>;

        using Ty2 = typename select<implements::_is_function_v<Ty1>>::template apply<
            pointer_modify::add_pointer<Ty1>, cv_modify::remove_cv<std::conditional_t<!implements::_is_function_v<Ty1>, Ty1, void>>>;

        using type = typename select<implements::_is_array_v<Ty1>>::template apply<
            pointer_modify::add_pointer<modifers::remove_extent_t<Ty1>>, Ty2>::type;
    };

    template <typename Ty>
    using decay_t = typename decay<Ty>::type;

    template <typename Ty>
    struct underlying_type {
        using type = __underlying_type(Ty);
    };

    template <typename Ty>
    using underlying_type_t = typename underlying_type<Ty>::type;
}

#define RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(EnumType)                                                                           \
    inline constexpr EnumType operator|(EnumType lhs, EnumType rhs) {                                                                 \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(lhs) | static_cast<type>(rhs));                                                \
    }                                                                                                                                 \
    inline constexpr EnumType operator&(EnumType lhs, EnumType rhs) {                                                                 \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(lhs) & static_cast<type>(rhs));                                                \
    }                                                                                                                                 \
    inline constexpr EnumType operator^(EnumType lhs, EnumType rhs) {                                                                 \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(lhs) ^ static_cast<type>(rhs));                                                \
    }                                                                                                                                 \
    inline constexpr EnumType operator~(EnumType val) {                                                                               \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(~static_cast<type>(val));                                                                        \
    }                                                                                                                                 \
    inline constexpr EnumType &operator|=(EnumType &lhs, EnumType rhs) {                                                              \
        lhs = lhs | rhs;                                                                                                              \
        return lhs;                                                                                                                   \
    }                                                                                                                                 \
    inline constexpr EnumType &operator&=(EnumType &lhs, EnumType rhs) {                                                              \
        lhs = lhs & rhs;                                                                                                              \
        return lhs;                                                                                                                   \
    }                                                                                                                                 \
    inline constexpr EnumType &operator^=(EnumType &lhs, EnumType rhs) {                                                              \
        lhs = lhs ^ rhs;                                                                                                              \
        return lhs;                                                                                                                   \
    }

namespace rainy::type_traits::primary_types {
    template <typename Ty>
    struct type_identity {
        using type = Ty;
    };

    template <typename Ty>
    using type_identity_t = typename type_identity<Ty>::type;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = type_relations::is_same_v<void, Ty>;

    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};

    /**
     * @brief 检索类型是否具有特化
     * @tparam Type 要检查的特化模板
     * @tparam Template 特化的类型
     * @remark
     */
    template <typename Type, template <typename...> typename Template>
    RAINY_CONSTEXPR_BOOL is_specialization_v = false;

    template <template <typename...> typename Template, typename... Types>
    RAINY_CONSTEXPR_BOOL is_specialization_v<Template<Types...>, Template> = true;

    /**
     * @brief 检索类型是否具有特化
     * @tparam Type 要检查的特化模板
     * @tparam Template 特化的类型
     */
    template <typename Type, template <typename...> typename Template>
    struct is_specialization : helper::bool_constant<is_specialization_v<Type, Template>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_enum_v = type_traits::implements::_is_enum_v<Ty>;

    template <typename Ty>
    struct is_enum : helper::bool_constant<is_enum_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v = implements::is_integral_v<Ty>;

    template <typename Ty>
    struct is_integral : helper::bool_constant<is_integral_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = type_traits::implements::is_floating_point_v<Ty>;

    template <typename Ty>
    struct is_floating_point : helper::bool_constant<is_floating_point_v<Ty>> {};

    template <typename Ty>
    struct is_union : helper::bool_constant<__is_union(Ty)> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_union_v = __is_union(Ty);

    template <typename Ty>
    struct is_class : helper::bool_constant<__is_class(Ty)> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_class_v = __is_class(Ty);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_function_v = type_traits::implements::_is_function_v<Ty>;

    template <typename Ty>
    struct is_function : helper::bool_constant<type_traits::implements::_is_function_v<Ty>> {};

    template <typename>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v<Ty &> = true;

    template <typename Ty>
    struct is_lvalue_reference : helper::bool_constant<is_lvalue_reference_v<Ty>> {};

    template <typename>
    RAINY_CONSTEXPR_BOOL is_rvalue_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_rvalue_reference_v<Ty &&> = true;

    template <typename Ty>
    struct is_rvalue_reference : helper::bool_constant<is_rvalue_reference_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_array_v = type_traits::implements::_is_array_v<Ty>;

    template <typename Ty>
    struct is_array : helper::bool_constant<is_array_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_v = type_traits::implements::_is_pointer_v<Ty>;

    template <typename Ty>
    struct is_pointer : helper::bool_constant<is_pointer_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v<Ty*&> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_reference_v<Ty*&&> = true;

    template <typename>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v = false;

    template <>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v<std::nullptr_t> = true;

    template <typename Ty>
    struct is_null_pointer : helper::bool_constant<is_null_pointer_v<Ty>> {};

#if RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = __is_member_object_pointer(Ty);
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = implements::is_member_object_pointer_<cv_modify::remove_cv_t<Ty>>::value;
#endif

    template <typename Ty>
    struct is_member_object_pointer : helper::bool_constant<is_member_object_pointer_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_function_pointer_v =
        implements::is_member_function_pointer_helper<cv_modify::remove_cv_t<Ty>>::value;

    template <typename Ty>
    struct is_member_function_pointer : helper::bool_constant<is_member_function_pointer_v<Ty>> {};

    template <typename Ty>
    static RAINY_INLINE_CONSTEXPR std::size_t array_size_v = 0;

    template <typename Ty,std::size_t N>
    static RAINY_INLINE_CONSTEXPR std::size_t array_size_v<Ty[N]> = N;

    template <typename Ty>
    struct array_size : helper::integral_constant<std::size_t, array_size_v<Ty>> {};
}

namespace rainy::utility {
    using core::builtin::addressof;

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 Ty *construct_at(Ty *location, Args &&...args) noexcept(noexcept(::new(static_cast<void *>(location))
                                                                                           Ty(utility::forward<Args>(args)...))) {
        if (!location) {
            return nullptr;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return std::construct_at(location, utility::forward<Args>(args)...);
        }
#endif
        return ::new (static_cast<void *>(location)) Ty(utility::forward<Args>(args)...);
    }

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 void construct_in_place(Ty &object, Args &&...args) noexcept(
        type_traits::implements::is_nothrow_constructible_v<Ty, Args...>) {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            std::construct_at(utility::addressof(object), utility::forward<Args>(args)...);
            return;
        }
#endif
        ::new (static_cast<void *>(utility::addressof(object))) Ty(utility::forward<Args>(args)...);
    }

    //template <typename Ty>
    //RAINY_CONSTEXPR20 void destory_range(Ty const* ptr) {
    //
    //}

    //template <typename Ty>
    //RAINY_CONSTEXPR20 void destroy_at(Ty const *ptr) {
    //    ptr->~Ty();
    //}

    //// 定义destroy函数
    //template <typename T>
    //void destroy(T *first, T *last) {
    //    for (T *ptr = first; ptr != last; ++ptr) {
    //        destroy_at(ptr); // 销毁每个元素
    //    }
    //}
}

namespace rainy::type_traits::logical_traits {
    template <typename...>
    struct conjunction : helper::true_type {};

    template <typename First, typename... Rest>
    struct conjunction<First, Rest...> : implements::_conjunction<First::value, First, Rest...>::type {};

    template <typename... Traits>
    RAINY_CONSTEXPR_BOOL conjunction_v = conjunction<Traits...>::value;

    template <typename...>
    struct disjunction : helper::false_type {};

    template <typename first, typename... rest>
    struct disjunction<first, rest...> : implements::_disjunction<first::value, first, rest...>::type {};

    template <typename... traits>
    RAINY_CONSTEXPR_BOOL disjunction_v = disjunction<traits...>::value;

    template <typename trait>
    struct negation : helper::bool_constant<!static_cast<bool>(trait::value)> {};

    template <typename trait>
    RAINY_CONSTEXPR_BOOL negation_v = negation<trait>::value;
}

namespace rainy::utility {
    struct placeholder_t final {
        explicit placeholder_t() = default;
    };

    constexpr placeholder_t placeholder{};

    template <typename = void>
    struct placeholder_type_t final {
        explicit placeholder_type_t() = default;
    };

    template <typename Ty>
    constexpr placeholder_type_t<Ty> placeholder_type{};

    template <std::size_t>
    struct in_place_index_t final {
        explicit in_place_index_t() = default;
    };

    template <std::size_t Idx>
    constexpr in_place_index_t<Idx> in_place_index{};

    template <class>
    constexpr bool is_in_place_index_specialization = false;

    template <std::size_t Idx>
    constexpr bool is_in_place_index_specialization<utility::in_place_index_t<Idx>> = true;

    using allocator_arg_t = std::allocator_arg_t;

    inline constexpr allocator_arg_t allocator_arg{};
}

namespace rainy::type_traits::type_properties {
    template <typename From, typename To, typename = void>
    struct is_invoke_convertible : helper::false_type {};

    template <typename From, typename To>
    struct is_invoke_convertible<From, To,
                                 other_trans::void_t<decltype(implements::fake_copy_init<To>(implements::returns_exactly<From>()))>>
        : helper::true_type {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_convertible_v = is_invoke_convertible<From, To>::value;

    template <typename From, typename To>
    struct is_invoke_nothrow_convertible
        : helper::bool_constant<noexcept(implements::fake_copy_init<To>(implements::returns_exactly<From>()))> {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_nothrow_convertible_v = is_invoke_convertible<From, To>::value;

    /*
    名称 描述
    is_swappable_with
    is_trivially_assignable 测试类型是否可赋值，以及赋值是否未使用非常用操作。
    is_nothrow_default_constructible 测试类型是否是默认构造，以及是否确定在构造默认时不引发。
    is_nothrow_swappable_with
    has_virtual_destructor 测试类型是否包含虚拟的析构函数。
    has_unique_object_representations
    is_nothrow_invocable 测试是否可以使用指定的参数类型调用可调用类型及其是否已知不会引发异常。
    已在 C++17 中添加。
    is_nothrow_invocable_r
    测试是否可以使用指定的参数类型调用可调用类型及其是否已知不会引发异常，以及结果是否可转换为指定类型。 已在 C++17
    中添加。
    */

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_move_assignable_v = __is_trivially_assignable(reference_modify::add_lvalue_reference_t<Ty>, Ty);

    template <typename Ty>
    struct is_trivially_move_assignable : helper::bool_constant<is_trivially_move_assignable_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_copy_assignable_v =
        __is_trivially_assignable(reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<const Ty>);

    template <typename Ty>
    struct is_trivially_copy_assignable : helper::bool_constant<is_trivially_copy_assignable_v<Ty>> {};

    template <typename Ty>
    constexpr bool is_move_assignable_v = __is_assignable(reference_modify::add_lvalue_reference_t<Ty>, Ty);

    template <typename Ty>
    struct is_move_assignable : helper::bool_constant<is_move_assignable_v<Ty>> {};

    template <typename Ty>
    constexpr bool is_copy_assignable_v =
        __is_assignable(reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<const Ty>);

    template <typename Ty>
    struct is_copy_assignable : helper::bool_constant<is_copy_assignable_v<Ty>> {};

    /**
     * @brief 测试类型是否为普通复制类型
     * @tparam Ty 要查询的类型
     * @remark 如果类型 Ty 是普通复制类型，则类型谓词的实例为 true；否则为 false。
     * 普通复制类型不具有任何重要的复制操作、移动操作或析构函数。 一般而言，如果复制操作可作为按位复制实现，则可将其视为普通复制。
     * 内置类型和普通复制类型数组都可进行普通复制。
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_copyable_v = __is_trivially_copyable(Ty);

    /**
     * @brief 测试类型是否为普通复制类型
     * @tparam Ty 要查询的类型
     * @remark 如果类型 Ty 是普通复制类型，则类型谓词的实例为 true；否则为 false。
     * 普通复制类型不具有任何重要的复制操作、移动操作或析构函数。 一般而言，如果复制操作可作为按位复制实现，则可将其视为普通复制。
     * 内置类型和普通复制类型数组都可进行普通复制。
     */
    template <typename Ty>
    struct is_trivially_copyable : helper::bool_constant<is_trivially_copyable_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_standard_layout_v = __is_standard_layout(Ty);

    template <typename Ty>
    struct is_standard_layout : helper::bool_constant<is_standard_layout_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;

    template <typename Ty>
    struct is_pod : helper::bool_constant<is_pod_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_empty_v = __is_empty(Ty);

    template <typename Ty>
    struct is_empty : helper::bool_constant<is_empty_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_polymorphic_v = __is_polymorphic(Ty);

    template <typename Ty>
    struct is_polymorphic : helper::bool_constant<is_polymorphic_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_abstract_v = __is_abstract(Ty);

    template <typename Ty>
    struct is_abstract : helper::bool_constant<is_abstract_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_final_v = __is_final(Ty);

    template <typename Ty>
    struct is_final : helper::bool_constant<is_final_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_aggregate_v = __is_aggregate(Ty);

    template <typename Ty>
    struct is_aggregate : helper::bool_constant<is_aggregate_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_signed_v = implements::sign_base<Ty>::is_signed;

    template <typename Ty>
    struct is_signed : helper::bool_constant<is_signed_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unsigned_v = implements::sign_base<Ty>::is_unsigned;

    template <typename Ty>
    struct is_unsigned : helper::bool_constant<is_unsigned_v<Ty>> {};

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_constructible_v = __is_constructible(Ty, Args...);

    template <typename Ty, typename... Args>
    struct is_constructible : helper::bool_constant<is_constructible_v<Ty, Args...>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copy_constructible_v = __is_constructible(Ty, reference_modify::add_lvalue_reference_t<const Ty>);

    template <typename Ty, typename... Args>
    struct is_copy_constructible : helper::bool_constant<is_copy_constructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_default_constructible_v = __is_constructible(Ty);

    template <typename Ty, typename... Args>
    struct is_default_constructible : helper::bool_constant<is_default_constructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_move_constructible_v = __is_constructible(Ty, Ty);

    template <typename Ty, typename... Args>
    struct is_move_constructible : helper::bool_constant<is_move_constructible_v<Ty>> {};

    template <typename to, typename from>
    RAINY_CONSTEXPR_BOOL is_assignable_v = __is_assignable(to, from);

    template <typename to, typename from>
    struct is_assignable : helper::bool_constant<is_assignable_v<to, from>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_move_constructible_v = __is_nothrow_constructible(Ty, Ty);

    template <typename Ty>
    struct is_nothrow_move_constructible : helper::bool_constant<is_nothrow_move_constructible_v<Ty>> {};

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_constructible_v = __is_nothrow_constructible(Ty, Args...);

    template <typename Ty>
    struct is_nothrow_constructible : helper::bool_constant<is_nothrow_constructible_v<Ty>> {};

#if RAINY_USING_MSVC || RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_destructible_v = __is_trivially_destructible(Ty);
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_destructible_v = implements::gcc_detail_impl::_is_destructible_safe<Ty>::value;
#endif

    template <typename Ty>
    struct is_trivially_destructible : helper::bool_constant<is_trivially_destructible_v<Ty>> {};

    template <typename to, typename from>
    RAINY_CONSTEXPR_BOOL is_nothrow_assignable_v = implements::_is_nothrow_assignable_v<to, from>;

    template <typename to, typename from>
    struct is_nothrow_assignable : helper::bool_constant<is_nothrow_assignable_v<to, from>> {};

    template <typename Ty>
    constexpr bool is_nothrow_copy_constructible_v =
        __is_nothrow_constructible(Ty, reference_modify::add_lvalue_reference_t<const Ty>);

    template <typename Ty>
    struct is_nothrow_copy_constructible : helper::bool_constant<is_nothrow_copy_constructible_v<Ty>> {};

    template <typename Ty>
    constexpr bool is_nothrow_move_assignable_v = __is_nothrow_assignable(reference_modify::add_lvalue_reference_t<Ty>, Ty);

    template <typename Ty>
    struct is_nothrow_move_assignable : helper::bool_constant<is_nothrow_move_assignable_v<Ty>> {};

    template <typename Ty>
    constexpr bool is_nothrow_copy_assignable_v =
        __is_nothrow_assignable(reference_modify::add_lvalue_reference_t<Ty>, reference_modify::add_lvalue_reference_t<const Ty>);

    template <typename Ty>
    struct is_nothrow_copy_assignable : helper::bool_constant<is_nothrow_copy_assignable_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_swappable_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        is_swappable_v<Ty, other_trans::void_t<decltype(std::swap(utility::declval<Ty &>(), utility::declval<Ty &>()))>> = true;

    template <typename Ty>
    struct is_swappable : helper::bool_constant<is_swappable_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_nothrow_swappable_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        is_nothrow_swappable_v<Ty, other_trans::enable_if_t<noexcept(std::swap(utility::declval<Ty &>(), utility::declval<Ty &>()))>> =
            true;

    template <typename Ty>
    struct is_nothrow_swappable : helper::bool_constant<is_nothrow_swappable_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_adl_swap_v = implements::has_adl_swap<Ty>;

    template <typename Ty>
    struct has_adl_swap : helper::bool_constant<has_adl_swap_v<Ty>> {};

#if RAINY_USING_MSVC || RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_destructible_v = __is_destructible(Ty);

    template <typename Ty>
    struct is_destructible : helper::bool_constant<is_destructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_destructible_v = __is_nothrow_destructible(Ty);

    template <typename Ty>
    struct is_nothrow_destructible : helper::bool_constant<is_nothrow_destructible_v<Ty>> {};
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_destructible_v = implements::gcc_detail_impl::_is_destructible_safe<Ty>::value;

    template <typename Ty>
    struct is_destructible : helper::bool_constant<is_destructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_destructible_v = std::is_nothrow_destructible<Ty>::value;

    template <typename Ty>
    struct is_nothrow_destructible : helper::bool_constant<is_nothrow_destructible_v<Ty>> {};
#endif

    template <typename Ty1, typename Ty2, typename = void>
    RAINY_CONSTEXPR_BOOL is_equal_comparable_v = false;

    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_equal_comparable_v<
        Ty1, Ty2, other_trans::void_t<decltype(utility::declval<const Ty1 &>() == utility::declval<const Ty2 &>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_transparent_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_transparent_v<Ty, other_trans::void_t<typename Ty::is_transparent>> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_default_constructible_v = __is_nothrow_constructible(Ty);

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_trivially_constructible_v = __is_trivially_constructible(Ty, Args...);

    template <typename Ty, typename... Args>
    struct is_trivially_constructible : helper::bool_constant<is_trivially_constructible_v<Ty, Args...>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_default_constructible_v = is_trivially_constructible_v<Ty>;

    template <typename Ty>
    struct is_trivially_default_constructible : helper::bool_constant<is_trivially_default_constructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_copy_constructible_v =
        is_trivially_constructible_v<Ty, reference_modify::add_lvalue_reference_t<const Ty>>;

    template <typename Ty>
    struct is_trivially_copy_constructible : helper::bool_constant<is_trivially_copy_constructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_move_constructible_v = is_trivially_constructible_v<Ty, Ty>;

    template <typename Ty>
    struct is_trivially_move_constructible : helper::bool_constant<is_trivially_move_constructible_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivially_swappable_v = is_trivially_destructible_v<Ty> && is_trivially_move_constructible_v<Ty> &&
                                                    is_trivially_move_assignable_v<Ty> && !has_adl_swap_v<Ty>;

    template <typename Ty>
    struct is_trivially_swappable : helper::bool_constant<is_trivially_swappable_v<Ty>> {};

    template <typename Ty, bool = primary_types::is_enum_v<Ty>>
    RAINY_CONSTEXPR_BOOL is_scoped_enum_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_scoped_enum_v<Ty, true> = !type_relations::is_convertible_v<Ty, other_trans::underlying_type_t<Ty>>;

    template <typename Ty, bool = primary_types::is_enum_v<Ty>>
    RAINY_CONSTEXPR_BOOL is_unscoped_enum_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unscoped_enum_v<Ty, true> = !type_relations::is_convertible_v<Ty, other_trans::underlying_type_t<Ty>>;

    template <typename Ty, bool = type_traits::primary_types::is_class_v<Ty> || type_traits::primary_types::is_union_v<Ty> ||
                                  type_traits::primary_types::is_array_v<Ty> || type_traits::primary_types::is_function_v<Ty>>
    RAINY_CONSTEXPR_BOOL prefer_pass_by_value_v = sizeof(Ty) <= 2 * sizeof(void *) && std::is_trivially_copy_constructible_v<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL prefer_pass_by_value_v<Ty, true> = false;
}

namespace rainy::utility {
    template <typename... Types>
    class tuple;

    // 空tuple特化
    template <>
    class tuple<> {
    public:
        constexpr tuple() noexcept = default;
        constexpr tuple(const tuple &) = default;
        constexpr tuple(tuple &&) = default;
        constexpr tuple &operator=(const tuple &) = default;
        constexpr tuple &operator=(tuple &&) = default;

        static constexpr void swap(tuple &) noexcept {
        }
    };

    template <std::size_t Indices, typename Tuple>
    struct tuple_element {};

    template <std::size_t Indices, typename... Types>
    struct tuple_element<Indices, tuple<Types...>> {
        using type = typename type_traits::other_trans::type_at<Indices, type_traits::other_trans::type_list<Types...>>::type;
    };

    template <std::size_t Indicies, typename Tuple>
    using tuple_element_t = typename tuple_element<Indicies, Tuple>::type;

    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &) noexcept;

    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &) noexcept;

    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&) noexcept;

    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&) noexcept;
}

namespace rainy::utility::implements {
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
    struct tuple_element<I, rainy::utility::tuple<Types...>> : ::rainy::utility::tuple_element<I, ::rainy::utility::tuple<Types...>> {
    };

    template <typename... Types>
    struct tuple_size<::rainy::utility::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};
}
// NOLINTEND

namespace rainy::utility {
    template <typename Head, typename... Rest>
    class tuple<Head, Rest...> : private implements::tuple_leaf_index<0, Head, sizeof...(Rest)>, private tuple<Rest...> {
    public:
        using head_base = implements::tuple_leaf_index<0, Head, sizeof...(Rest)>;
        using rest_base = tuple<Rest...>;

        constexpr tuple() : head_base{}, rest_base{} {
        }

        constexpr tuple(const tuple &) = default;

        constexpr tuple(tuple &&) = default;

        template <typename HeadArg, typename... RestArgs,
            type_traits::other_trans::enable_if_t<sizeof...(RestArgs) == sizeof...(Rest) && type_traits::type_properties::is_constructible_v<Head, HeadArg> &&
                                                      (type_traits::type_properties::is_constructible_v<Rest, RestArgs> && ...),
                                                  int> = 0 > constexpr tuple(HeadArg &&head_arg, RestArgs &&...rest_args) :
            head_base(utility::forward<HeadArg>(head_arg)),
                                                  rest_base(utility::forward<RestArgs>(rest_args)...) {
        }

        template <typename OtherHead, typename... OtherRest,
                  type_traits::other_trans::enable_if_t<sizeof...(OtherRest) == sizeof...(Rest) &&
                                                            type_traits::type_properties::is_constructible_v<Head, const OtherHead &> &&
                                                            (type_traits::type_properties::is_constructible_v<Rest, const OtherRest &> && ...),
                                                        int> = 0>
        constexpr tuple(const tuple<OtherHead, OtherRest...> &other) :
            head_base(get<0>(other)), rest_base(static_cast<const tuple<OtherRest...> &>(other)) {
        }

        template <typename Alloc>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc) : head_base(tag, alloc), rest_base(tag, alloc) {
        }

        template <typename Alloc, typename HeadArg, typename... TailArgs>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg, TailArgs &&...tail_args) :
            head_base(tag, alloc, utility::forward<HeadArg>(head_arg)),
            rest_base(tag, alloc, utility::forward<TailArgs>(tail_args)...) {
        }

        constexpr tuple &operator=(const tuple &other) {
            head_base::value = static_cast<const head_base &>(other).value;
            static_cast<rest_base &>(*this) = static_cast<const rest_base &>(other);
            return *this;
        }

        constexpr tuple &operator=(tuple &&other) noexcept(std::is_nothrow_move_assignable_v<Head> &&
                                                           std::is_nothrow_move_assignable_v<tuple<Rest...>>) {
            head_base::value = utility::move(static_cast<head_base &&>(other).value);
            static_cast<rest_base &>(*this) = static_cast<rest_base &&>(other);
            return *this;
        }

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

        constexpr void swap(tuple &other) noexcept(std::is_nothrow_swappable_v<Head> && std::is_nothrow_swappable_v<tuple<Rest...>>) {
            head_base::swap(static_cast<head_base &>(other));
            static_cast<rest_base &>(*this).swap(static_cast<rest_base &>(other));
        }

        template <std::size_t I>
        constexpr tuple_element_t<I, tuple> &get() noexcept {
            if constexpr (I == 0) {
                return head_base::value;
            } else {
                return static_cast<rest_base &>(*this).template get<I - 1>();
            }
        }

        template <std::size_t I>
        constexpr const tuple_element_t<I, tuple> &get() const noexcept {
            if constexpr (I == 0) {
                return head_base::value;
            } else {
                return static_cast<const rest_base &>(*this).template get<I - 1>();
            }
        }
    };

    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&t) noexcept { // NOLINT
        return utility::move(t.template get<I>());
    }

    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&t) noexcept {
        return utility::move(t.template get<I>());
    }

    template <typename... Types>
    constexpr tuple<std::decay_t<Types>...> make_tuple(Types &&...args) {
        return tuple<std::decay_t<Types>...>(utility::forward<Types>(args)...);
    }

    template <typename... Types>
    constexpr tuple<Types &&...> forward_as_tuple(Types &&...args) noexcept {
        return tuple<Types &&...>(utility::forward<Types>(args)...);
    }

    template <typename... Types>
    constexpr void swap(tuple<Types...> &lhs, tuple<Types...> &rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    template <typename... TTypes, typename... UTypes>
    constexpr bool operator==(const tuple<TTypes...> &lhs, const tuple<UTypes...> &rhs) {
        if constexpr (sizeof...(TTypes) != sizeof...(UTypes)) {
            return false;
        } else if constexpr (sizeof...(TTypes) == 0) {
            return true;
        } else {
            return utility::get<0>(lhs) == utility::get<0>(rhs) &&
                   static_cast<const tuple<TTypes...> &>(lhs) == static_cast<const tuple<UTypes...> &>(rhs);
        }
    }
}

namespace rainy::utility {
    template <typename Tuple>
    struct tuple_size {
        static RAINY_INLINE_CONSTEXPR std::size_t value = 0;
    };

    template <typename... Args>
    struct tuple_size<tuple<Args...>> {
        static RAINY_INLINE_CONSTEXPR std::size_t value = sizeof...(Args);
    };
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
    template <typename>
    struct member_pointer_traits {
        static RAINY_CONSTEXPR_BOOL valid = false;
        using class_type = void;
    };

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

    template <typename Type, typename Class>
    struct member_pointer_traits<Type Class::*> {
        using type = Type;
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
    struct function_traits<Rx(Args...) SPEC> : implements::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> { \
        using return_type = Rx;                                                                                                       \
        using tuple_like_type = std::tuple<Args...>;                                                                                  \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args..., ...) SPEC>                                                                                     \
        : implements::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> {                                      \
        using return_type = Rx;                                                                                                       \
        using tuple_like_type = std::tuple<Args...>;                                                                                  \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };

#define RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer, IsLValue, IsRValue, SPEC)  \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args...) SPEC>                                                                               \
        : implements::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                 \
          implements::member_function_traits_base<IsLValue, IsRValue> {                                                                \
        using return_type = Rx;                                                                                                       \
        using tuple_like_type = std::tuple<Args...>;                                                                                  \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args..., ...) SPEC>                                                                          \
        : implements::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                 \
          implements::member_function_traits_base<IsLValue, IsRValue> {                                                                \
        using return_type = Rx;                                                                                                       \
        using tuple_like_type = std::tuple<Args...>;                                                                                  \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };
// NOLINTEND(bugprone-macro-parentheses)

namespace rainy::type_traits::implements {
    template <bool IsMemberFunctionPointer = false, bool IsFunctionPointer = false, bool IsNoexcept = false, bool IsVolatile = false,
              bool IsConstMemberFunction = false>
    struct function_traits_base {
        static RAINY_CONSTEXPR_BOOL is_member_function_pointer = IsMemberFunctionPointer;
        static RAINY_CONSTEXPR_BOOL is_function_pointer = IsFunctionPointer;
        static RAINY_CONSTEXPR_BOOL is_noexcept = IsNoexcept;
        static RAINY_CONSTEXPR_BOOL is_volatile = IsVolatile;
        static RAINY_CONSTEXPR_BOOL is_const_member_function = IsConstMemberFunction;
        static RAINY_CONSTEXPR_BOOL valid = true;
        static RAINY_CONSTEXPR_BOOL is_function_object = false;
    };

    template <bool IsLvalue, bool IsRvalue>
    struct member_function_traits_base {
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = IsLvalue;
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

    template <typename Traits,bool Enable = Traits::valid>
    struct fn_obj_traits {
        using tuple_like_type = typename Traits::tuple_like_type;
        using return_type = typename Traits::return_type;
        static inline constexpr std::size_t arity = Traits::arity;
        static RAINY_CONSTEXPR_BOOL is_noexcept = Traits::is_noexcept;
        static RAINY_CONSTEXPR_BOOL is_volatile = Traits::is_volatile;
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = false;
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = false;
        static RAINY_CONSTEXPR_BOOL is_function_object = true;
        static RAINY_CONSTEXPR_BOOL valid = true;
        static RAINY_CONSTEXPR_BOOL is_const_member_function = Traits::is_const_member_function;
    };

    template <typename Traits>
    struct fn_obj_traits<Traits, false> : empty_function_traits {};
}

namespace rainy::type_traits::primary_types {
    template <typename Ty>
    struct function_traits : implements::fn_obj_traits<function_traits<typename implements::try_to_get_operator<type_traits::cv_modify::remove_cvref_t<Ty>>::type>> {
    };

    template <>
    struct function_traits<void> : implements::empty_function_traits {};

    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args...)> : implements::function_traits_base<> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args..., ...)> : implements::function_traits_base<> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(false, true, volatile)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, false, noexcept)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, true, volatile noexcept)

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...)> : implements::function_traits_base<false, true> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...)> : implements::function_traits_base<false, true> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...) noexcept> : implements::function_traits_base<false, true, true> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...) noexcept> : implements::function_traits_base<false, true, true> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /*------------------
    [normal]
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, false, false,)
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

    template <typename Fx>
    using function_return_type = typename function_traits<Fx>::return_type;

    template <typename Fx>
    static inline constexpr std::size_t arity = function_traits<Fx>::arity;

    template <typename Fx>
    using param_list_in_tuple = typename function_traits<Fx>::tuple_like_type;

    template <typename NewRx, typename Fx, typename TypeListFront = other_trans::type_list<>,
              typename TypeListEnd = other_trans::type_list<>, typename Tuple = param_list_in_tuple<Fx>>
    struct make_normalfx_type_with_pl {};

    template <typename NewRx, typename Fx, typename... TypeListFrontArgs, typename... TypeListEndArgs,typename... OriginalArgs>
    struct make_normalfx_type_with_pl<NewRx, Fx, other_trans::type_list<TypeListFrontArgs...>,
                                      other_trans::type_list<TypeListEndArgs...>, std::tuple<OriginalArgs...>> {
        template <typename UFx, bool IsMemPtr = function_traits<UFx>::is_member_function_pointer>
        struct helper {
            using fn_traits = function_traits<UFx>;

            using prototype = NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...);

            using type =
                other_trans::conditional_t<fn_traits::is_noexcept,
                                           NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept, prototype>;
        };

        template <typename UFx>
        struct helper<UFx, true> {
            using fn_traits = function_traits<UFx>;

            using type =
                other_trans::conditional_t<fn_traits::is_noexcept,
                                           NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept,
                                           NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...)>;
        };

        using type = typename helper<Fx>::type;
    };

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
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(type_traits::implements::_is_object_v<Ty> || type_traits::implements::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        using type = Ty;

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

        constexpr operator Ty &() const noexcept {
            return *reference_data;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get() const noexcept {
            return *reference_data;
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<std::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) try_to_invoke_as_function(Args &&...args) const
            noexcept(implements::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            using f_traits = type_traits::primary_types::function_traits<Ty>;
            if constexpr (f_traits::valid) {
                using return_type = typename f_traits::return_type;
                if constexpr (std::is_invocable_r_v<return_type, Ty, Args...>) {
                    // 经过实践证明的是，reference_wrapper不能引用成员函数，因此像不同函数一样调用也就够了
                    if constexpr (type_traits::primary_types::is_void_v<return_type>) {
                        get(utility::forward<Args>(args)...);
                    } else {
                        return get()(utility::forward<Args>(args)...);
                    }
                }
            }
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<std::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) operator()(Args &&...args) const
            noexcept(implements::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            return try_to_invoke_as_function(utility::forward<Args>(args)...);
        }

    private:
        Ty *reference_data{nullptr};
    };

    template <class Ty>
    reference_wrapper(Ty &) -> reference_wrapper<Ty>;

    template <typename Uty>
    reference_wrapper(Uty &) -> reference_wrapper<type_traits::cv_modify::remove_cvref_t<Uty>>; // 使用Deduction Guide模板参数推导

    template <typename Ty>
    void ref(const Ty &&) = delete;

    template <typename Ty>
    void cref(const Ty &&) = delete;

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(Ty &val) noexcept {
        return reference_wrapper<Ty>(val);
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(reference_wrapper<Ty> val) noexcept {
        return val;
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(const Ty &val) noexcept {
        return reference_wrapper<const Ty>(val);
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(reference_wrapper<Ty> val) noexcept {
        return val;
    }
}

namespace rainy::type_traits::cv_modify {
    template <typename Ty>
    struct unwrap_reference {
        using type = Ty;
    };

    template <typename Ty>
    struct unwrap_reference<utility::reference_wrapper<Ty>> {
        using type = Ty;
    };

    template <typename Ty>
    struct unwrap_reference<std::reference_wrapper<Ty>> {
        using type = Ty;
    };

    template <typename Ty>
    using unwrap_reference_t = typename unwrap_reference<Ty>::type;

    template <typename Ty>
    using unwrap_ref_decay_t = unwrap_reference_t<other_trans::decay_t<Ty>>;

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
    template <typename Callable, typename Ty1 = void>
    struct invoker : implements::invoker_impl<Callable, Ty1> {};
}

namespace rainy::utility {
    template <typename Callable>
    constexpr RAINY_INLINE auto invoke(Callable &&obj) noexcept(noexcept(static_cast<Callable &&>(obj)()))
        -> decltype(static_cast<Callable &&>(obj)()) {
        return static_cast<Callable &&>(obj)();
    }

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
    template <typename Callable, typename Tuple>
    constexpr decltype(auto) apply(Callable &&obj, Tuple &&tuple) noexcept(noexcept(
        implements::apply_impl(utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
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
    using decltype_invoke_nonzero =
        decltype(utility::invoker<Callable, Ty1>::invoke(utility::declval<Callable>(), utility::declval<Ty1>(), utility::declval<Args>()...));

    template <typename Callable, typename Ty1, typename... Args>
    struct invoke_traits_nonzero<other_trans::void_t<decltype_invoke_nonzero<Callable, Ty1, Args...>>, Callable, Ty1, Args...>
        : invoke_traits_common<decltype_invoke_nonzero<Callable, Ty1, Args...>,
                               noexcept(utility::invoker<Callable, Ty1>::invoke(utility::declval<Callable>(), utility::declval<Ty1>(),
                                                                       utility::declval<Args>()...))> {};

    template <typename Callable, typename... Args>
    using select_invoke_traits = other_trans::conditional_t<sizeof...(Args) == 0, invoke_traits_zero<void, Callable>,
                                                            invoke_traits_nonzero<void, Callable, Args...>>;

    template <typename Rx,typename Callable,typename... Args>
    using is_invocable_r_helper = typename select_invoke_traits<Callable,Args...>::template is_invocable_r<Rx>;

    template <typename Callable,typename... Args>
    using is_invocable_helper = typename select_invoke_traits<Callable,Args...>::is_invocable;
}

namespace rainy::type_traits::type_properties {
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_r_v = implements::is_invocable_r_helper<Rx, Callable, Args...>::value;

    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_r : helper::bool_constant<is_invocable_r_v<Rx, Callable, Args...>> {};

    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_v = implements::is_invocable_helper<Callable, Args...>::value;

    template <typename Callable, typename... Args>
    struct is_invocable : helper::bool_constant<is_invocable_v<Callable, Args...>> {};

    template <typename Callable, typename... Args>
    struct invoke_result {
        using type = typename implements::select_invoke_traits<Callable, Args...>::type;
    };

    template <typename Callable, typename... Args>
    using invoke_result_t = typename invoke_result<Callable, Args...>::type;
}

namespace rainy::utility {
    template <typename Ty1, typename Ty2>
    class compressed_pair;

    namespace implements {
        template <typename Ty, bool = std::is_final_v<Ty>>
        struct compressed_pair_empty : std::false_type {};

        template <typename Ty>
        struct compressed_pair_empty<Ty, false> : std::is_empty<Ty> {};

        template <typename Ty1, typename Ty2, bool is_same, bool first_empty, bool second_empty>
        struct compressed_pair_switch;

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, false, false> {
            RAINY_CONSTEXPR static int value = 0;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, true, true> {
            RAINY_CONSTEXPR static int value = 3;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, true, false> {
            RAINY_CONSTEXPR static int value = 1;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, false, true> {
            RAINY_CONSTEXPR static int value = 2;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, true, true, true> {
            RAINY_CONSTEXPR static int value = 4;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, true, false, false> {
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

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first(x), second(y) {
            }

            explicit constexpr compressed_pair_impl(first_param_type x) : first(x) {
            }

            explicit constexpr compressed_pair_impl(second_param_type y) : second(y) {
            }

            constexpr first_reference get_first() {
                return first;
            }

            constexpr first_const_reference get_first() const {
                return first;
            }

            constexpr second_reference get_second() {
                return second;
            }

            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair_impl &pair) {
                using std::swap;
                swap(first, pair.second);
                swap(second, pair.second);
            }

            first_type first;
            second_type second;
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

            constexpr compressed_pair_impl() = default;

            explicit constexpr compressed_pair_impl(first_param_type first, second_param_type second) :
                first_type(first), second(second) {
            }

            explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
            }

            explicit constexpr compressed_pair_impl(second_param_type second) : second(second) {
            }

            constexpr first_reference get_first() {
                return *this;
            }

            constexpr first_const_reference get_first() const {
                return *this;
            }

            constexpr second_reference get_second() {
                return second;
            }

            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair_impl &pair) {
                std::swap(second, pair.second);
            }

            second_type second;
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

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type x, second_param_type y) : second_type(y), first(x) {
            }

            constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
            }

            constexpr explicit compressed_pair_impl(second_param_type y) : second_type(y) {
            }

            constexpr first_reference get_first() {
                return first;
            }

            constexpr first_const_reference get_first() const {
                return first;
            }

            constexpr second_reference get_second() {
                return *this;
            }

            constexpr second_const_reference get_second() const {
                return *this;
            }

            constexpr void swap(compressed_pair_impl &pair) {
                std::swap(first, pair.first);
            }

            first_type first;
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

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second_type(second) {
            }

            explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
            }

            explicit constexpr compressed_pair_impl(second_param_type second) : second_type(second) {
            }

            constexpr first_reference get_first() {
                return *this;
            }

            constexpr first_const_reference get_first() const {
                return *this;
            }

            constexpr second_reference get_second() {
                return *this;
            }

            constexpr second_const_reference get_second() const {
                return *this;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &) {
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

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first_type(x), second(y) {
            }

            explicit compressed_pair_impl(first_param_type x) : first_type(x), second(x) {
            }

            constexpr first_reference get_first() {
                return *this;
            }

            constexpr first_const_reference get_first() const {
                return *this;
            }

            constexpr second_reference get_second() {
                return second;
            }

            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
                std::swap(second, pair.second);
            }

            Ty2 second;
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

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first(first), second(second) {
            }

            explicit constexpr compressed_pair_impl(first_param_type first) : first(first), second(first) {
            }

            constexpr first_reference get_first() {
                return first;
            }

            constexpr first_const_reference get_first() const {
                return first;
            }

            constexpr second_reference get_second() {
                return second;
            }

            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &y) {
                std::swap(first, y.first);
                std::swap(second, y.second);
            }

            Ty1 first;
            Ty2 second;
        };
    }

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
        using base::base;

        constexpr compressed_pair(const compressed_pair &) = default;
        constexpr compressed_pair(compressed_pair &&) = default;

        constexpr compressed_pair &operator=(const compressed_pair &other) {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        constexpr compressed_pair &operator=(compressed_pair &&other) noexcept(std::is_nothrow_move_assignable_v<Ty1> &&
                                                                     std::is_nothrow_move_assignable_v<Ty2>) {
            utility::construct_in_place(this->get_first(), utility::move(other.get_first()));
            utility::construct_in_place(this->get_second(), utility::move(other.get_second()));
            return *this;
        }
    };

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
        using base::base;

        constexpr compressed_pair(const compressed_pair &) = default;
        constexpr compressed_pair(compressed_pair &&) = default;

        constexpr compressed_pair &operator=(const compressed_pair &other) {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        constexpr compressed_pair &operator=(compressed_pair &&other) noexcept(std::is_nothrow_move_assignable_v<Ty>) {
            this->get_first() = utility::move(other.get_first());
            this->get_second() = utility::move(other.get_second());
            return *this;
        }
    };

    template <typename Ty1, typename Ty2>
    constexpr void swap(compressed_pair<Ty1, Ty2> &left, compressed_pair<Ty1, Ty2> &right) {
        left.swap(right);
    }
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief 测试类型是否为常量
     * @tparam Ty 要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_v = false;


    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_v<const Ty> = true;

    /**
     * @brief 测试类型是否为常量
     * @tparam Ty 要查询的类型
     */
    template <typename Ty>
    struct is_const : helper::bool_constant<is_const_v<Ty>> {};

    /**
     * @brief 测试类型是否为volatile
     * @tparam Ty 要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_volatile_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_volatile_v<volatile Ty> = true;

    /**
     * @brief 测试类型是否为volatile
     * @tparam Ty 要查询的类型
     */
    template <typename Ty>
    struct is_volatile : helper::bool_constant<is_volatile_v<Ty>> {};

    /**
     * @brief 测试类型是否为平凡类型
     * @tparam Ty 要查询的类型
     */
#if RAINY_USING_CLANG || RAINY_USING_MSVC
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivial_v = __is_trivially_constructible(Ty) && __is_trivially_copyable(Ty);
#else
    /**
     * @brief 测试类型是否为平凡类型
     * @tparam Ty 要查询的类型
     * @remark 如果类型 Ty 是常用类型，则类型谓词的实例为 true；否则为 false。
     * 常用类型是标量类型、完全可复制类类型、这些类型的数组以及这些类型的 cv 限定版本。
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivial_v = __is_trivial(Ty);
#endif // RAINY_USING_CLANG || RAINY_USING_MSVC

    /**
     * @brief 测试类型是否为平凡类型
     * @tparam Ty 要查询的类型
     * @remark 如果类型 Ty 是常用类型，则类型谓词的实例为 true；否则为 false。
     * 常用类型是标量类型、完全可复制类类型、这些类型的数组以及这些类型的 cv 限定版本。
     */
    template <typename Ty>
    struct is_trivial : helper::bool_constant<is_trivial_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_unique_object_representations_v = __has_unique_object_representations(Ty);

    template <typename Ty>
    struct has_unique_object_representations : helper::bool_constant<has_unique_object_representations_v<Ty>> {};
}

namespace rainy::type_traits::extras::templates {
    template <typename Ty>
    struct get_first_parameter;

    template <template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct get_first_parameter<Ty<First, Rest...>> {
        using type = First;
    };

    template <typename newfirst, typename Ty>
    struct replace_first_parameter;

    template <typename NewFirst, template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct replace_first_parameter<NewFirst, Ty<First, Rest...>> {
        using type = Ty<NewFirst, Rest...>;
    };

    template <typename, typename = void>
    struct get_ptr_difference_type {
        using type = ptrdiff_t;
    };

    template <typename Ty>
    struct get_ptr_difference_type<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    template <typename Ty, typename Other, typename = void>
    struct get_rebind_alias {
        using type = typename replace_first_parameter<Other, Ty>::type;
    };

    template <typename Ty, typename Other>
    struct get_rebind_alias<Ty, Other, type_traits::other_trans::void_t<typename Ty::template rebind<Other>>> {
        using type = typename Ty::template rebind<Other>;
    };
}

namespace rainy::foundation::system::memory::implements {
    template <typename Ty, typename Elem>
    struct pointer_traits_base {
        using pointer = Ty;
        using element_type = Elem;
        using difference_type = typename type_traits::extras::templates::get_ptr_difference_type<Ty>::type;

        template <typename other>
        using rebind = typename type_traits::extras::templates::get_rebind_alias<Ty, other>::type;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Elem>, char, Elem> &;

        RAINY_NODISCARD static RAINY_CONSTEXPR20 pointer pointer_to(ref_type val) noexcept(noexcept(Ty::pointer_to(val))) {
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

namespace rainy::foundation::system::memory {
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

        RAINY_NODISCARD static constexpr pointer pointer_to(ref_type val) noexcept {
            return utility::addressof(val);
        }
    };
}

namespace rainy::utility {
    using foundation::system::memory::pointer_traits;
}

namespace rainy::utility::implements {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_to_address = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_to_address<Ty, type_traits::other_trans::void_t<decltype(foundation::system::memory::pointer_traits<Ty>::to_address(
                               utility::declval<const Ty &>()))>> = true;
}

namespace rainy::utility {
    template <typename Ty>
    constexpr Ty *to_address(Ty *const val) noexcept {
        static_assert(!type_traits::primary_types::is_function_v<Ty>, "Ty cannot be a function type.");
        return val;
    }

    template <typename Pointer>
    RAINY_NODISCARD constexpr auto to_address(const Pointer &val) noexcept {
        if constexpr (implements::has_to_address<Pointer>) {
            return pointer_traits<Pointer>::to_address(val);
        } else {
            return utility::to_address(val.operator->());
        }
    }
}

namespace rainy::type_traits::composite_types {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = implements::is_arithmetic_v<Ty>;

    template <typename Ty>
    struct is_arithmetic : helper::bool_constant<is_arithmetic_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_fundamental_v =
        is_arithmetic_v<Ty> || primary_types::is_void_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    template <typename Ty>
    struct is_fundamental : helper::bool_constant<is_fundamental_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_object_v = type_properties::is_const_v<const Ty> && !primary_types::is_void_v<Ty>;

    template <typename Ty>
    struct is_object : helper::bool_constant<is_object_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_reference_v = implements::_is_reference_v<Ty>;

    template <typename Ty>
    struct is_reference : helper::bool_constant<implements::_is_reference_v<Ty>> {};

#if RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_pointer_v = __is_member_pointer(Ty);
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_pointer_v =
        primary_types::is_member_object_pointer_v<Ty> || primary_types::is_member_function_pointer_v<Ty>;
#endif // RAINY_USING_CLANG

    template <typename Ty>
    struct is_member_pointer : helper::bool_constant<is_member_pointer_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_scalar_v = is_arithmetic_v<Ty> || primary_types::is_enum_v<Ty> || primary_types::is_pointer_v<Ty> ||
                                       is_member_pointer_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    template <typename Ty>
    struct is_scalar : helper::bool_constant<is_scalar_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_compound_v = !is_fundamental_v<Ty>;

    template <typename Ty>
    struct is_compound : helper::bool_constant<is_compound_v<Ty>> {};
}

namespace rainy::utility {
#if RAINY_HAS_CXX20
    template <typename Class, typename Fx>
        requires(type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                 !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>)
    constexpr auto get_overloaded_func(Fx Class::*memfn) {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }
#else
    template <typename Class, typename Fx,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                                                        !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>,
                                                    int> = 0>
    constexpr auto get_overloaded_func(Fx Class::*memfn) {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }
#endif
}

#if RAINY_HAS_CXX20

namespace rainy::type_traits::concepts {
    template <typename Ty1, typename Ty2>
    concept same_as = implements::is_same_v<Ty1, Ty2>;

    template <typename base, typename derived>
    concept derived_from =
        __is_base_of(base, derived) && type_relations::is_convertible_v<const volatile derived *, const volatile base *>;

    template <typename Ty, typename... Types>
    concept in_types = (type_traits::type_relations::is_any_of_v<Ty, Types...>);
}

#if __has_include("format")
#include <format>
namespace rainy::type_traits::concepts {
    template <typename Ty, typename Context = std::format_context,
              typename Formatter = typename Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with = std::semiregular<Formatter> &&
                               requires(Formatter &formatter, const Formatter &const_formatter, Ty &&type, Context format_context,
                                        std::basic_format_parse_context<typename Context::char_type> parse_context) {
                                   { formatter.parse(parse_context) } -> same_as<typename decltype(parse_context)::iterator>;
                                   { const_formatter.format(type, format_context) } -> same_as<typename Context::iterator>;
                               };

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
    template <typename Template>
    struct template_traits : helper::false_type {};

    template <template <typename...> typename Template, typename... Types>
    struct template_traits<Template<Types...>> : helper::true_type {
        using types = other_trans::type_list<Types...>;
    };

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_template_v = template_traits<Ty>::value;

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

    template <typename Fx, typename... Args>
    constexpr method_flags deduction_invoker_type() noexcept {
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
    template <typename Concept>
    class poly_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;
        using vtable_type = decltype(implements::make_vtable<Concept>::make_with_vl(typename Concept::template impl<inspector>{}));
        static constexpr bool is_mono = std::tuple_size_v<vtable_type> == 1u;

        using type = std::conditional_t<is_mono, std::tuple_element_t<0u, vtable_type>, const vtable_type *>;

        template <typename Type>
        RAINY_NODISCARD static type instance() noexcept {
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

    template <typename Poly>
    struct poly_base {
        template <std::size_t Member, typename... Args>
        decltype(auto) invoke(const poly_base &self, Args &&...args) const {
            const auto &poly = static_cast<const Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return std::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }

        template <std::size_t Member, typename... Args>
        decltype(auto) invoke(poly_base &self, Args &&...args) {
            auto &poly = static_cast<Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                static_assert(Member == 0, "Unknown member");
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return std::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }
    };

    template <std::size_t Member, typename Poly, typename... Args>
    decltype(auto) poly_call(Poly &&self, Args &&...args) {
        return utility::forward<Poly>(self).template invoke<Member>(self, utility::forward<Args>(args)...);
    }

    template <typename AbstractBody>
    class basic_poly : private AbstractBody::template type<poly_base<basic_poly<AbstractBody>>> {
    public:
        friend struct poly_base<basic_poly>;

        using abstract_type = typename AbstractBody::template type<poly_base<basic_poly>>;
        using vtable_info = poly_vtable<AbstractBody>;
        using vtable_type = typename vtable_info::type;

        basic_poly() noexcept : _ptr(nullptr), vtable{} {
        }

        template <typename Type>
        basic_poly(Type *ptr) noexcept :
            _ptr(static_cast<void *>(ptr)), vtable{vtable_info::template instance<std::remove_cv_t<std::remove_pointer_t<Type>>>()} {
        }

        basic_poly(std::nullptr_t) {
        }

        basic_poly(basic_poly &&other) noexcept : _ptr(other._ptr), vtable(other.vtable) {
            other._ptr = nullptr;
            other.vtable = {};
        }

        basic_poly &operator=(basic_poly &&other) noexcept {
            if (this != &other) {
                _ptr = other._ptr;
                vtable = other.vtable;
                other._ptr = nullptr;
                other.vtable = {};
            }
            return *this;
        }

        basic_poly &operator=(std::nullptr_t) noexcept {
            _ptr = nullptr;
            return *this;
        }

        basic_poly(const basic_poly &) = default;
        basic_poly &operator=(const basic_poly &) = default;

        void reset() noexcept {
            _ptr = nullptr;
            vtable = {};
        }

        template <typename Type>
        void reset(Type *ptr) noexcept {
            _ptr = static_cast<void *>(ptr);
            vtable = vtable_info::template instance<std::remove_cv_t<std::remove_pointer_t<Type>>>();
        }

        RAINY_NODISCARD explicit operator bool() const noexcept {
            return _ptr != nullptr;
        }

        RAINY_NODISCARD abstract_type *operator->() noexcept {
            return this;
        }

        RAINY_NODISCARD const abstract_type *operator->() const noexcept {
            return this;
        }

        void *target_as_void_ptr() const noexcept {
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
    template <typename Ty, typename... Args>
    struct ctor : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>,
                                                        implements::ctor_impl<Ty, Args...>> {};

    template <typename Ty>
    struct dtor
        : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, implements::dtor_impl<Ty>> {};
}

namespace rainy::utility {
    template <typename Ty, typename... Args,
              typename type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
    constexpr auto get_ctor_fn() {
        return &ctor<Ty, Args...>::invoke;
    }

    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, int> = 0>
    constexpr auto get_dtor_fn() {
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
    template <typename Ty, typename Assign>
    struct assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                              !type_traits::type_properties::is_const_v<Ty> &&
                                                              !type_traits::composite_types::is_reference_v<Ty>,
                                                          implements::assign_impl<Ty, Assign>> {};

    template <typename Ty>
    struct copy_assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                               implements::copy_assign_impl<Ty>> {};

    template <typename Ty>
    struct move_assign : type_traits::other_trans::conditional_t<
                             type_traits::type_properties::is_move_assignable_v<Ty>, implements::move_assign_impl<Ty>,
                             type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                                   implements::copy_assign_impl<Ty>>> {};
}

namespace rainy::utility {
    template <typename Ty, typename Assign,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                        !type_traits::type_properties::is_const_v<Ty> &&
                                                        !type_traits::composite_types::is_reference_v<Ty>,
                                                    int> = 0>
    constexpr auto get_assign() {
        return &assign<Ty, Assign>::invoke;
    }

    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty> ||
                                                                     type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>,
                                                                 int> = 0>
    constexpr auto get_move_assign() {
        return &move_assign<Ty>::invoke;
    }

    template <typename Ty,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>, int> = 0>
    constexpr auto get_copy_assign() {
        return &copy_assign<Ty>::invoke;
    }
}

namespace rainy::utility::cpp_methods {
    static constexpr std::string_view method_operator_add = "operator+";
    static constexpr std::string_view method_operator_sub = "operator-";
    static constexpr std::string_view method_operator_mul = "operator*";
    static constexpr std::string_view method_operator_div = "operator/";
    static constexpr std::string_view method_operator_mod = "operator%";
    static constexpr std::string_view method_operator_eq = "operator==";
    static constexpr std::string_view method_operator_neq = "operator!=";
    static constexpr std::string_view method_operator_lt = "operator<";
    static constexpr std::string_view method_operator_gt = "operator>";
    static constexpr std::string_view method_operator_le = "operator<=";
    static constexpr std::string_view method_operator_ge = "operator>=";
    static constexpr std::string_view method_operator_assign = "operator=";
    static constexpr std::string_view method_operator_index = "operator[]";
    static constexpr std::string_view method_operator_call = "operator()";
    static constexpr std::string_view method_operator_arrow = "operator->";
    static constexpr std::string_view method_operator_deref = "operator*";
    static constexpr std::string_view method_operator_addr = "operator&";
    static constexpr std::string_view method_operator_preinc = "operator++";
    static constexpr std::string_view method_operator_postinc = "operator++(int)";
    static constexpr std::string_view method_operator_predec = "operator--";
    static constexpr std::string_view method_operator_postdec = "operator--(int)";
    static constexpr std::string_view method_operator_or = "operator||";
    static constexpr std::string_view method_operator_and = "operator&&";
    static constexpr std::string_view method_operator_not = "operator!";
    static constexpr std::string_view method_operator_bit_or = "operator|";
    static constexpr std::string_view method_operator_bit_and = "operator&";
    static constexpr std::string_view method_operator_bit_xor = "operator^";
    static constexpr std::string_view method_operator_bit_not = "operator~";
    static constexpr std::string_view method_operator_shift_l = "operator<<";
    static constexpr std::string_view method_operator_shift_r = "operator>>";
    static constexpr std::string_view method_begin = "begin";
    static constexpr std::string_view method_end = "end";
    static constexpr std::string_view method_cbegin = "cbegin";
    static constexpr std::string_view method_cend = "cend";
    static constexpr std::string_view method_rbegin = "rbegin";
    static constexpr std::string_view method_rend = "rend";
    static constexpr std::string_view method_size = "size";
    static constexpr std::string_view method_empty = "empty";
    static constexpr std::string_view method_clear = "clear";
    static constexpr std::string_view method_push_back = "push_back";
    static constexpr std::string_view method_pop_back = "pop_back";
    static constexpr std::string_view method_length = "length";
    static constexpr std::string_view method_insert = "insert";
    static constexpr std::string_view method_erase = "erase";
    static constexpr std::string_view method_find = "find";
    static constexpr std::string_view method_resize = "resize";
    static constexpr std::string_view method_swap = "swap";
    static constexpr std::string_view method_at = "at";
    static constexpr std::string_view method_front = "front";
    static constexpr std::string_view method_back = "back";
    static constexpr std::string_view method_append = "append";
}

namespace rainy::core {
    template <typename InputIt, typename Ty>
    RAINY_INLINE constexpr Ty accumulate(InputIt first, InputIt last, Ty init) {
        for (; first != last; ++first) {
#if RAINY_HAS_CXX20
            init = utility::move(init) + *first;
#else
            init += *first;
#endif
        }
        return init;
    }

    template <typename InputIt, typename Ty, typename BinaryOperation>
    RAINY_INLINE constexpr Ty accumulate(InputIt first, InputIt last, Ty init, BinaryOperation op) {
        for (; first != last; ++first) {
            init = utility::invoke(op, init, *first);
        }
        return init;
    }
}

namespace rainy::type_traits::extras::iterators {
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_input_iterator_v = meta_method::has_operator_deref_v<It> && meta_method::has_operator_preinc_v<It>;

    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_output_iterator_v = false;

    template <typename It>
    RAINY_CONSTEXPR_BOOL
        is_output_iterator_v<It, other_trans::enable_if_t<meta_method::has_operator_deref_v<It> &&
                                                          primary_types::is_lvalue_reference_v<decltype(*utility::declval<It &>())>>> =
            false;

    template <typename It>
    RAINY_CONSTEXPR_BOOL is_forward_iterator_v =
        is_input_iterator_v<It> && type_properties::is_copy_constructible_v<It> && type_properties::is_copy_assignable_v<It> &&
        type_properties::is_default_constructible_v<It>;

    template <typename It>
    RAINY_CONSTEXPR_BOOL is_bidirectional_iterator_v = is_forward_iterator_v<It> && meta_method::has_operator_predec_v<It>;

    template <typename It>
    RAINY_CONSTEXPR_BOOL is_random_access_iterator_v = is_bidirectional_iterator_v<It> && meta_method::has_operator_addition_v<It> &&
                                                       meta_method::has_operator_index_v<It> && meta_method::has_operator_lt_v<It>;

    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v = false;

    template <typename It>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v<
        It,
        type_traits::other_trans::enable_if_t<is_random_access_iterator_v<It> &&
                                              primary_types::is_pointer_v<decltype(utility::to_address(utility::declval<It>()))>>> =
        true;
}

namespace rainy::utility::implements {
    template <typename Ty>
    constexpr void verify_range(const Ty *const first, const Ty *const last) noexcept {
        // special case range verification for pointers
        expects(first <= last, "transposed pointer range");
    }

    template <typename Iter, typename = void>
    constexpr bool allow_inheriting_unwrap_v = true;

    template <typename Iter>
    constexpr bool allow_inheriting_unwrap_v<Iter, type_traits::other_trans::void_t<typename Iter::prevent_inheriting_unwrap>> =
        type_traits::implements::is_same_v<Iter, typename Iter::prevent_inheriting_unwrap>;

    template <typename Iter, typename Sentinel = Iter, typename = void>
    constexpr bool range_verifiable_v = false;

    template <typename Iter, typename Sentinel>
    constexpr bool range_verifiable_v<
        Iter, Sentinel,
        type_traits::implements::void_t<decltype(verify_range(declval<const Iter &>(), declval<const Sentinel &>()))>> =
        allow_inheriting_unwrap_v<Iter>;

    template <typename iter, typename sentinel>
    constexpr void adl_verify_range(const iter &first, const sentinel &last) {
        // check that [first, last) forms an iterator range
        if constexpr (type_traits::implements::_is_pointer_v<iter> && type_traits::implements::_is_pointer_v<sentinel>) {
            expects(first <= last, "transposed pointer range");
        } else if constexpr (range_verifiable_v<iter, sentinel>) {
            verify_range(first, last);
        }
    }
}

namespace rainy::utility {
    template <typename Iter>
    RAINY_NODISCARD constexpr std::ptrdiff_t distance(Iter first, Iter last) {
        if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
            return last - first; // assume the iterator will do debug checking
        } else {
            implements::adl_verify_range(first, last);
            std::ptrdiff_t off = 0;
            for (; first != last; ++first) {
                ++off;
            }
            return off;
        }
    }
}

namespace rainy::type_traits::type_properties {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_movable_v = composite_types::is_object_v<Ty> && type_properties::is_move_constructible_v<Ty> &&
                                        type_properties::is_assignable_v<Ty &, Ty> && type_properties::is_swappable_v<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copyable_v =
        type_properties::is_copy_constructible_v<Ty> && is_movable_v<Ty> &&
        type_traits::type_properties::is_assignable_v<Ty &, Ty &> && type_traits::type_properties::is_assignable_v<Ty &, const Ty &> &&
        type_traits::type_properties::is_assignable_v<Ty &, const Ty>;
}

namespace rainy::utility {
    template <typename Diff>
    constexpr Diff max_possible_v{static_cast<type_traits::helper::make_unsigned_t<Diff>>(-1) >> 1};

    template <typename Diff>
    constexpr Diff min_possible_v{-max_possible_v<Diff> - 1};
}

#endif