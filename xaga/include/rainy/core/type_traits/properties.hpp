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
#ifndef RAINY_CORE_TYPE_TRAITS_PROPERTIES_HPP
#define RAINY_CORE_TYPE_TRAITS_PROPERTIES_HPP

// NOLINTBEGIN

#include <rainy/core/platform.hpp>
#include <rainy/core/gnu/typetraits.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/primary_types.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/core/type_traits/decay.hpp>

// NOLINTEND

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
    RAINY_CONSTEXPR_BOOL is_trivially_assignable_v = __is_trivially_assignable(reference_modify::add_lvalue_reference_t<To>, From);

    /**
     * @brief Type template for checking if assignment from From to To is trivially assignable.
     *        Uses compiler built-in __is_trivially_assignable.
     *
     *        检查从 From 到 To 的赋值是否可平凡赋值的类型模板。
     *        使用编译器内建 __is_trivially_assignable。
     *
     * @tparam To Target type
     *            目标类型
     * @tparam From Source type
     *              源类型
     */
    template <typename To, typename From>
    struct is_trivially_assignable : helper::bool_constant<is_trivially_assignable_v<To, From>> {};

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
#if RAINY_USING_MSVC || RAINY_USING_CLANG || RAINY_USING_MACOS
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
    RAINY_CONSTEXPR_BOOL is_trivially_destructible_v = __has_trivial_destructor(Ty);
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

#endif