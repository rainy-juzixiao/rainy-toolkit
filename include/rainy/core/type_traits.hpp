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
#include <typeinfo>
#include <utility>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/modifers.h>
#include <rainy/core/type_traits/helper.h>
#include <rainy/core/type_traits/type_relations.h>

#if RAINY_USING_GCC
#include <rainy/core/gnu/typetraits.hpp>
#endif

namespace rainy::type_traits::implements {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_nothrow_move_constructible = __is_nothrow_constructible(Ty, Ty);

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL _is_nothrow_constructible_v = __is_nothrow_constructible(Ty, Args...);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_enum_v = __is_enum(Ty);

    template <typename>
    RAINY_CONSTEXPR_BOOL always_false = false;

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_array_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_array_v<Ty[]> = true;

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR_BOOL _is_array_v<Ty[N]> = true;

    template <typename to, typename from>
    RAINY_CONSTEXPR_BOOL _is_nothrow_assignable_v = __is_nothrow_assignable(to, from);

#if RAINY_USING_MSVC || RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_trivially_destructible_v = __is_trivially_destructible(Ty);
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_trivially_destructible_v = std::is_trivially_destructible_v<Ty>;
#endif
    
    template <typename>
    RAINY_CONSTEXPR_BOOL _is_lvalue_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_lvalue_reference_v<Ty &> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_void_v = is_same_v<Ty, void>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_object_v = _is_const_v<const Ty> && !_is_void_v<Ty>;

    template <bool, typename if_true, typename>
    struct _conditional {
        using type = if_true;
    };

    template <typename if_true, typename if_false>
    struct _conditional<false, if_true, if_false> {
        using type = if_false;
    };

    template <bool test, typename Ty>
    struct _enable_if;

    template <typename Ty>
    struct _enable_if<true, Ty> {
        using type = Ty;
    };

    template <bool test, typename if_true, typename if_false>
    using _conditional_t = typename _conditional<test, if_true, if_false>::type;

    template <bool>
    struct _select {
        template <typename Ty1, typename>
        using apply = Ty1;
    };

    template <>
    struct _select<false> {
        template <typename, typename Ty2>
        using apply = Ty2;
    };

    template <typename...>
    using _void_t = void;

    template <bool, typename first, typename...>
    struct _disjunction {
        using type = first;
    };

    template <typename _true, typename next, typename... rest>
    struct _disjunction<false, _true, next, rest...> {
        using type = typename _disjunction<next::value, next, rest...>::type;
    };


    template <bool, typename first, typename...>
    struct _conjunction {
        using type = first;
    };

    template <typename _true, typename next, typename... rest>
    struct _conjunction<true, _true, next, rest...> {
        using type = typename _conjunction<next::value, next, rest...>::type;
    };

    template <typename Ty>
    struct _add_const {
        using type = const Ty;
    };

    template <typename Ty>
    struct _add_volatile {
        using type = volatile Ty;
    };


    template <typename Ty>
    struct _add_cv {
        using type = const volatile Ty;
    };

    template <typename...>
    using void_t = void;

    
    template <typename Ty>
    struct remove_const {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_const<const Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_const<const volatile Ty> {
        using type = Ty;
    };


    template <typename Ty>
    RAINY_CONSTEXPR20 Ty _fake_copy_init(Ty) noexcept {
        return Ty{};
    }

    template <typename Ty>
    Ty _returns_exactly() noexcept {
        return Ty{};
    }

    template <typename Ty>
    struct is_member_object_pointer_ {
        static constexpr bool value = false;
    };

    template <typename Ty1, typename Ty2>
    struct is_member_object_pointer_<Ty1 Ty2::*> {
        static constexpr bool value = !_is_function_v<Ty1>;
        using Class_type = Ty2;
    };

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_arithmetic_v = is_integral_v<Ty> || is_floating_point_v<Ty>;

    template <typename Ty, bool = is_integral_v<Ty>>
    struct sign_base {
        using uty = remove_cv_t<Ty>;

        static constexpr bool _signed = static_cast<uty>(-1) < static_cast<uty>(0);
        static constexpr bool _unsigned = !_signed;
    };

    template <typename Ty>
    struct sign_base<Ty, false> {
        static constexpr bool _signed = is_floating_point_v<Ty>;
        static constexpr bool _unsigned = false;
    };

    template <typename Ty, bool small_>
    struct ct_imp2 {
        using param_type = const Ty &;
    };

    template <typename Ty>
    struct ct_imp2<Ty, true> {
        using param_type = const Ty;
    };

    template <typename Ty, bool isp, bool b1, bool b2>
    struct ct_imp {
        using param_type = const Ty &;
    };

    template <typename Ty, bool isp, bool b2>
    struct ct_imp<Ty, isp, true, b2> {
        using param_type = typename ct_imp2<Ty, sizeof(Ty) <= sizeof(void *)>::param_type;
    };

    template <typename Ty, bool isp, bool b1>
    struct ct_imp<Ty, isp, b1, true> {
        using param_type = typename ct_imp2<Ty, sizeof(Ty) <= sizeof(void *)>::param_type;
    };

    template <typename Ty, bool b1, bool b2>
    struct ct_imp<Ty, true, b1, b2> {
        using param_type = const Ty;
    };

    template <typename Ty>
    struct _call_traits {
    public:
        using value_type = Ty;
        using reference = Ty &;
        using const_reference = const Ty &;
        using param_type = typename ct_imp<Ty, _is_pointer_v<Ty>, _is_arithmetic_v<Ty>, _is_enum_v<Ty>>::param_type;
    };

    template <typename Ty>
    struct _call_traits<Ty &> {
        using value_type = Ty &;
        using reference = Ty &;
        using const_reference = const Ty &;
        using param_type = Ty &;
    };

    template <typename trait>
    struct _negation {
        static RAINY_CONSTEXPR_BOOL value = !static_cast<bool>(trait::value);
    };
}

namespace rainy::type_traits::other_trans {
    template <typename Ty>
    struct decay {
        using Ty1 = reference_modify::remove_reference_t<Ty>;

        using Ty2 = typename select<implements::_is_function_v<Ty1>>::template apply<
            pointer_modify::add_pointer<Ty1>, cv_modify::remove_cv<std::conditional_t<!implements::_is_function_v<Ty1>, Ty1, void>>>;

        using type =
            typename select<implements::_is_array_v<Ty1>>::template apply<pointer_modify::add_pointer<array_modify::remove_extent<Ty1>>,
                                                                         Ty2>::type;
    };

    template <typename Ty>
    using decay_t = typename decay<Ty>::type;
}

/*
forward和move以及exchange和实现
*/
namespace rainy::utility {
    /**
     * @brief 将对象转换为右值引用以实现移动语义。
     *
     * @tparam Ty 要转换的对象的类型。
     * @param arg 要转换为右值引用的对象。
     * @return 返回类型为 `remove_reference_t_<Ty>&&` 的右值引用对象。
     *
     * @remark
     * 该函数用于实现移动语义（move semantics），通过将对象转换为右值引用以允许资源移动而非拷贝。
     *
     * @attention
     * 移动语义通常用于优化对象的复制，尤其是在对象不再需要其原始值时。请注意对象在移动后不可再安全使用。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr type_traits::reference_modify::remove_reference_t<Ty> &&move(Ty &&arg) noexcept {
        return static_cast<type_traits::reference_modify::remove_reference_t<Ty> &&>(arg);
    }

    /**
     * @brief 使用完美转发（perfect forwarding）实现类型安全的引用转发。
     *
     * @tparam Ty 转发对象的类型。
     * @param arg 要转发的左值引用对象。
     * @return 返回类型为 `Ty&&` 的转发对象。
     *
     * @remark
     * 这个函数用于将左值引用安全地转发为相应类型的引用（可能是左值引用或右值引用），
     * 以保留传入参数的左值或右值性质。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::reference_modify::remove_reference_t<Ty> &arg) noexcept {
        return static_cast<Ty &&>(arg);
    }

    /**
     * @brief 使用完美转发（perfect forwarding）实现类型安全的引用转发。
     *
     * @tparam Ty 转发对象的类型。
     * @param arg 要转发的右值引用对象。
     * @return 返回类型为 `Ty&&` 的转发对象。
     *
     * @remark
     * 这个函数用于将右值引用安全地转发为相应类型的引用（可能是左值引用或右值引用），
     * 以保留传入参数的左值或右值性质。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::reference_modify::remove_reference_t<Ty> &&arg) noexcept { // NOLINT
        static_assert(!type_traits::implements::_is_lvalue_reference_v<Ty>, "bad forward call");
        return static_cast<Ty &&>(arg);
    }

    /**
     * @brief 交换对象的值并返回旧值。
     *
     * @tparam Ty 交换的对象的类型。
     * @tparam Other 新值的类型，默认为 `Ty`。
     * @param val 要被交换的对象。
     * @param new_val 用于替换的对象的新值。
     * @return 返回被交换的对象的旧值。
     *
     * @remark
     * 该函数将 `val` 的当前值保存为 `old_val`，然后将 `val` 设置为 `new_val`，最后返回 `old_val`。
     *
     * @attention
     * 使用此函数时，需要确保 `val` 和 `new_val` 的类型是可移动构造和可赋值的，以避免潜在的异常。
     * 因为此函数要求实例化类型的Ty是可移动构造，且Ty的左值引用是能赋值给Other类型的
     */
    template <typename Ty, typename Other = Ty>
    constexpr Ty exchange(Ty &val, Other &&new_val) noexcept(type_traits::implements::_is_nothrow_move_constructible<Ty> &&
                                                             type_traits::implements::_is_nothrow_assignable_v<Ty &, Other>) {
        Ty old_val = static_cast<Ty &&>(val);
        val = static_cast<Other &&>(new_val);
        return old_val;
    }
}

namespace rainy::type_traits::extras::templates {
    template <typename Ty, typename = void>
    struct has_difference_type : helper::false_type {};

    template <typename Ty>
    struct has_difference_type<Ty, other_trans::void_t<typename Ty::difference_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_difference_type_v = has_difference_type<Ty>::value;

    template <typename Ty, typename = void>
    struct has_value_type : helper::false_type {};

    template <typename Ty>
    struct has_value_type<Ty, other_trans::void_t<typename Ty::value_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_value_type_v = has_value_type<Ty>::value;

    template <typename Ty, typename = void>
    struct has_pointer : helper::false_type {};

    template <typename Ty>
    struct has_pointer<Ty, other_trans::void_t<typename Ty::pointer>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_pointer_v = has_pointer<Ty>::value;

    template <typename Ty, typename = void>
    struct has_reference : helper::false_type {};

    template <typename Ty>
    struct has_reference<Ty, other_trans::void_t<typename Ty::reference>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_reference_v = has_reference<Ty>::value;

    template <typename Ty, typename = void>
    struct has_iterator_category : helper::false_type {};

    template <typename Ty>
    struct has_iterator_category<Ty, other_trans::void_t<typename Ty::iterator_category>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_iterator_category_v = has_iterator_category<Ty>::value;

    template <typename Ty, typename = void>
    struct has_element_type : helper::false_type {};

    template <typename Ty>
    struct has_element_type<Ty, other_trans::void_t<typename Ty::element_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_element_type_v = has_element_type<Ty>::value;
}

namespace rainy::utility::implements {
    template <typename Ty, bool Enable = type_traits::extras::templates::has_value_type_v<Ty>>
    struct try_to_add_value_type {
        using value_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_value_type<Ty, true> {
        using value_type = typename Ty::value_type;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_difference_type_v<Ty>>
    struct try_to_add_difference_type {
        using difference_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_difference_type<Ty, true> {
        using difference_type = typename Ty::difference_type;
    };

    template <typename Ty, bool IsPointer = type_traits::implements::_is_pointer_v<Ty>,
              bool Enable = type_traits::extras::templates::has_iterator_category_v<Ty>>
    struct try_to_add_iterator_category {
        using iterator_category = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, true, false> {
        using iterator_category = std::random_access_iterator_tag; // 为了兼容标准库设计
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, false, true> {
        using iterator_category = typename Ty::iterator_category;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_reference_v<Ty>>
    struct try_to_add_reference {
        using reference = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_reference<Ty, true> {
        using reference = typename Ty::reference;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_pointer_v<Ty>>
    struct try_to_add_pointer {
        using pointer = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_pointer<Ty, true> {
        using pointer = typename Ty::pointer;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_element_type_v<Ty>>
    struct try_to_add_element_type {
        using element_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_element_type<Ty, true> {
        using element_type = typename Ty::element_type;
    };

    template <typename Ty>
    struct iterator_traits_base : try_to_add_difference_type<Ty>,
                                  try_to_add_iterator_category<Ty>,
                                  try_to_add_pointer<Ty>,
                                  try_to_add_reference<Ty>,
                                  try_to_add_value_type<Ty> {};
}

namespace rainy::utility {
    template <typename Ty>
    struct iterator_traits : implements::iterator_traits_base<Ty> {};

    template <typename Ty>
    struct iterator_traits<Ty *> {
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        using pointer = Ty *;
        using reference = Ty &;
        using value_type = Ty;
    };

    template <typename DifferenceType, typename Category, typename Pointer, typename Reference, typename ValueType>
    struct make_iterator_traits {
        using difference_type = DifferenceType;
        using iterator_category = Category;
        using pointer = Pointer;
        using reference = Reference;
        using value_type = ValueType;
    };
}

namespace rainy::type_traits::extras::templates {
    template <typename Iter>
    struct iter_value_type {
        using type = typename utility::iterator_traits<Iter>::value_type;
    };

    template <typename Iter, typename = void>
    RAINY_CONSTEXPR_BOOL is_iterator_v = false;

    template <typename Iter>
    RAINY_CONSTEXPR_BOOL is_iterator_v<Iter, other_trans::void_t<typename utility::iterator_traits<Iter>::iterator_category>> = true;
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
        type_traits::implements::_is_nothrow_constructible_v<Ty, Args...>) {
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
    struct placeholder final {
        explicit placeholder() = default;
    };

    template <typename = void>
    struct placeholder_type_t final {
        explicit placeholder_type_t() = default;
    };

    template <typename Ty>
    constexpr placeholder_type_t<Ty> placeholder_type{};

    template <std::size_t>
    struct placeholder_index_t final {
        explicit placeholder_index_t() = default;
    };

    template <std::size_t Idx>
    constexpr placeholder_index_t<Idx> placeholder_index{};

    using allocator_arg_t = std::allocator_arg_t;

    inline constexpr allocator_arg_t allocator_arg{};

    template <typename Ty>
    type_traits::reference_modify::add_rvalue_reference_t<Ty> declval() noexcept {
        static_assert(type_traits::implements::always_false<Ty>, "Calling declval is ill-formed, see N4950 [declval]/2.");
        return type_traits::helper::get_fake_object<Ty>();
    }

    template <typename Ty>
    const std::type_info *get_typeid_ptr() noexcept {
        return &typeid(Ty);
    }

    template <typename Ty>
    const std::type_info &get_typeid() noexcept {
        return typeid(Ty);
    }
}

namespace rainy::type_traits::extras::meta_method {
    template <typename Ty, typename = void>
    struct try_to_invoke_begin {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>,
                          "Can not find begin method! "
                          "rainy::utility::begin"
                          "only support begin() in Container Type"
                          "please add begin() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_begin<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().begin())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(Ty &container) noexcept(noexcept(container.begin())) -> decltype(container.begin()) {
            return container.begin();
        }
    };

    template <typename, typename = void>
    struct try_to_invoke_end {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>,
                          "Can not find end method! "
                          "rainy::utility::end"
                          "only support end() in Container Type"
                          "please add end() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_end<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().end())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(Ty &container) noexcept(noexcept(container.end())) -> decltype(container.end()) {
            return container.end();
        }
    };

    template <typename Ty, typename = void>
    struct try_to_invoke_cbegin {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>,
                          "Can not find begin method! "
                          "rainy::utility::cbegin"
                          "only support cbegin() in Container Type"
                          "please add cbegin() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_cbegin<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().cbegin())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(const Ty &container) noexcept(noexcept(container.cbegin())) -> decltype(container.cbegin()) {
            return container.cbegin();
        }
    };

    template <typename, typename = void>
    struct try_to_invoke_cend {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>,
                          "Can not find end method! "
                          "rainy::utility::cend"
                          "only support cend() in Container Type"
                          "please add cend() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_cend<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().cend())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(const Ty &container) noexcept(noexcept(container.cend())) -> decltype(container.cend()) {
            return container.cend();
        }
    };
}

namespace rainy::utility {
    template <typename Container>
    RAINY_NODISCARD constexpr auto begin(Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto begin(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont);
    }

    template <typename Container, std::size_t N>
    static auto begin(Container (&container)[N]) noexcept {
        return container;
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto end(Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto end(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto cbegin(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto cend(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont);
    }
}

namespace rainy::type_traits::type_properties {
    template <typename From, typename To, typename = void>
    struct is_invoke_convertible : helper::false_type {};

    template <typename From, typename To>
    struct is_invoke_convertible<From, To,
                                 other_trans::void_t<decltype(implements::_fake_copy_init<To>(implements::_returns_exactly<From>()))>>
        : helper::true_type {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_convertible_v = is_invoke_convertible<From, To>::value;

    template <typename From, typename To>
    struct is_invoke_nothrow_convertible
        : helper::bool_constant<noexcept(implements::_fake_copy_init<To>(implements::_returns_exactly<From>()))> {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_nothrow_convertible_v = is_invoke_convertible<From, To>::value;

    /*
    名称 描述
    is_copy_assignable 测试是否可以将类型的常量引用值分配给该类型。
    is_move_assignable 测试是否可以将类型的右值引用分配给该类型。
    is_swappable_with
    is_trivially_constructible 测试在使用指定类型构造类型时，该类型是否未使用非常用操作。
    is_trivially_default_constructible 测试在构造默认时，该类型是否未使用非常用操作。
    is_trivially_copy_constructible 测试在构造复制时，该类型是否未使用非常用操作。
    is_trivially_move_constructible 测试在构造移动时，该类型是否未使用非常用操作。
    is_trivially_assignable 测试类型是否可赋值，以及赋值是否未使用非常用操作。
    is_trivially_copy_assignable 测试类型是否为复制赋值，以及赋值是否未使用非常用操作。
    is_trivially_move_assignable 测试类型是否为移动赋值，以及赋值是否未使用非常用操作。
    is_trivially_destructible 测试类型是否易损坏，以及析构函数是否未使用非常用操作。
    is_nothrow_default_constructible 测试类型是否是默认构造，以及是否确定在构造默认时不引发。
    is_nothrow_swappable_with
    has_virtual_destructor 测试类型是否包含虚拟的析构函数。
    has_unique_object_representations
    is_invocable 测试是否可以使用指定的参数类型调用可调用类型。
    已在 C++17 中添加。
    is_invocable_r 测试是否可以使用指定的参数类型调用可调用类型，以及结果是否可转换为指定类型。
    已在 C++17 中添加。
    is_nothrow_invocable 测试是否可以使用指定的参数类型调用可调用类型及其是否已知不会引发异常。
    已在 C++17 中添加。
    is_nothrow_invocable_r
    测试是否可以使用指定的参数类型调用可调用类型及其是否已知不会引发异常，以及结果是否可转换为指定类型。 已在 C++17
    中添加。
    */

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
    RAINY_CONSTEXPR_BOOL is_signed_v = implements::sign_base<Ty>::_signed;

    template <typename Ty>
    struct is_signed : helper::bool_constant<is_signed_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_unsigned_v = implements::sign_base<Ty>::_unsigned;

    template <typename Ty>
    struct is_unsigned : helper::bool_constant<is_unsigned_v<Ty>> {};

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_constructible_v = __is_constructible(Ty, Args...);

    template <typename Ty, typename... Args>
    struct is_constructible : helper::bool_constant<is_constructible_v<Ty>> {};

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
    RAINY_CONSTEXPR_BOOL is_trivially_destructible_v = std::__is_destructible_safe<Ty>::value;
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
}

namespace rainy::type_traits::primary_types {
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
    RAINY_CONSTEXPR_BOOL is_enum_v = implements::_is_enum_v<Ty>;

    template <typename Ty>
    struct is_enum : helper::bool_constant<is_enum_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v = implements::is_integral_v<Ty>;

    template <typename Ty>
    struct is_integral : helper::bool_constant<is_integral_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = implements::is_floating_point_v<Ty>;

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
    RAINY_CONSTEXPR_BOOL is_template_v = false;

    template <template <typename...> typename Template, typename... Args>
    RAINY_CONSTEXPR_BOOL is_template_v<Template<Args...>> = true;

    template <typename Ty>
    struct is_template : helper::bool_constant<is_template_v<Ty>> {};
}

#define RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(...)                                                                                         \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct is_member_function_pointer_helper<Rx (Class::*)(Args...) __VA_ARGS__> {                                                          \
        static constexpr bool value = true;                                                                                           \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct is_member_function_pointer_helper<Rx (Class::*)(Args..., ...) __VA_ARGS__> {                                                     \
        static constexpr bool value = true;                                                                                           \
    };

namespace rainy::type_traits::implements {
    template <typename>
    struct is_member_function_pointer_helper {
        static constexpr bool value = false;
    };

    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC()
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(&)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(&&)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const &)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const &&)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(& noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(&& noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const & noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const && noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile &)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile &&)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile & noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile && noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile &)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile &&)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile & noexcept)
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile && noexcept)
}

#undef RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC

namespace rainy::type_traits::primary_types {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_function_pointer_v = implements::is_member_function_pointer_helper<cv_modify::remove_cv_t<Ty>>::value;

    template <typename Ty>
    struct is_member_function_pointer : helper::bool_constant<is_member_function_pointer_v<Ty>> {};
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

    template <std::size_t Indices, typename T>
    struct tuple_element;

    template <std::size_t Indices, typename Head, typename... Tail>
    struct tuple_element<Indices, tuple<Head, Tail...>> : tuple_element<Indices - 1, tuple<Tail...>> {};

    template <typename Head, typename... Tail>
    struct tuple_element<0, tuple<Head, Tail...>> {
        using type = Head;
    };

    // 前向声明
    template <std::size_t I, typename... Types>
    constexpr typename std::tuple_element<I, tuple<Types...>>::type &get(tuple<Types...> &) noexcept;

    template <std::size_t I, typename... Types>
    constexpr const typename std::tuple_element<I, tuple<Types...>>::type &get(const tuple<Types...> &) noexcept;

    template <std::size_t I, typename... Types>
    constexpr typename std::tuple_element<I, tuple<Types...>>::type &&get(tuple<Types...> &&) noexcept;

    template <std::size_t I, typename... Types>
    constexpr const typename std::tuple_element<I, tuple<Types...>>::type &&get(const tuple<Types...> &&) noexcept;
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
            value(std::allocator_arg, allocator, std::forward<Args>(args)...) {
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

    template <std::size_t I, typename Ty, size_t Index>
    struct tuple_leaf_index : tuple_val<Ty> {
        constexpr tuple_leaf_index() : tuple_val<Ty>() {
        }

        template <typename Uty>
        constexpr tuple_leaf_index(Uty &&arg) : tuple_val<Ty>(utility::forward<Uty>(arg)) { // NOLINT
        }

        template <typename Alloc, typename... Args>
        constexpr tuple_leaf_index(const Alloc &alloc, allocator_arg_t tag, Args &&...args) :
            tuple_val<Ty>(alloc, tag, utility::forward<Args>(args)...) {
        }
    };
}

// NOLINTBEGIN
namespace std {
    template <size_t I, typename... Types>
    struct tuple_element<I, rainy::utility::tuple<Types...>>
        : ::rainy::utility::tuple_element<I, rainy::utility::tuple<Types...>> {};

    template <typename... Types>
    struct tuple_size<::rainy::utility::tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};
}
// NOLINTEND

namespace rainy::utility {
    template <typename Head, typename... Rest>
    class tuple<Head, Rest...> : private implements::tuple_leaf_index<0, Head, sizeof...(Rest)>, private tuple<Rest...> {        
    public:
        using head_base = implements::tuple_leaf_index<0, Head, sizeof...(Rest)>;
        using rest_base = tuple<Rest...>;

        constexpr tuple() : head_base(), rest_base() {
        }

        constexpr tuple(const tuple &) = default;

        constexpr tuple(tuple &&) = default;

        template <typename HeadArg, typename... RestArgs,
                  type_traits::other_trans::enable_if_t<sizeof...(RestArgs) == sizeof...(Rest) &&
                      std::is_constructible_v<Head, HeadArg> && (std::is_constructible_v<Rest, RestArgs> && ...),
                  int> = 0>
        constexpr tuple(HeadArg &&head_arg, RestArgs &&...rest_args) :
            head_base(utility::forward<HeadArg>(head_arg)), rest_base(utility::forward<RestArgs>(rest_args)...) {
        }

        template <typename OtherHead, typename... OtherRest,
                  type_traits::other_trans::enable_if_t<sizeof...(OtherRest) == sizeof...(Rest) &&
                      std::is_constructible_v<Head, const OtherHead &> && (std::is_constructible_v<Rest, const OtherRest &> && ...),
                  int> = 0>
        constexpr tuple(const tuple<OtherHead, OtherRest...> &other) :
            head_base(get<0>(other)), rest_base(static_cast<const tuple<OtherRest...> &>(other)) {
        }

        // 带分配器构造
        template <typename Alloc>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc) : head_base(tag, alloc), rest_base(tag, alloc) {
        }

        template <typename Alloc, typename HeadArg, typename... TailArgs>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg, TailArgs &&...tail_args) :
            head_base(tag, alloc, utility::forward<HeadArg>(head_arg)),
            rest_base(tag, alloc, utility::forward<TailArgs>(tail_args)...) {
        }

        // 赋值操作符
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

        // 从不同类型tuple赋值
        template <typename OtherHead, typename... OtherRest,
                  type_traits::other_trans::enable_if_t<sizeof...(OtherRest) == sizeof...(Rest) &&
                      std::is_assignable_v<Head &, const OtherHead &> && (std::is_assignable_v<Rest &, const OtherRest &> && ...),
                  int> = 0>
        constexpr tuple &operator=(const tuple<OtherHead, OtherRest...> &other) {
            head_base::value = get<0>(other);
            static_cast<rest_base &>(*this) = static_cast<const tuple<OtherRest...> &>(other);
            return *this;
        }

        // 添加swap成员函数
        constexpr void swap(tuple &other) noexcept(std::is_nothrow_swappable_v<Head> && std::is_nothrow_swappable_v<tuple<Rest...>>) {
            head_base::swap(static_cast<head_base &>(other));
            static_cast<rest_base &>(*this).swap(static_cast<rest_base &>(other));
        }
    };

    template <size_t I, typename... Types>
    constexpr std::tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &t) noexcept {
        using element_type = std::tuple_element_t<I, tuple<Types...>>;
        constexpr size_t remain_size = sizeof...(Types) - I - 1;
        using leaf_type = implements::tuple_leaf_index<I, element_type, remain_size>;
        return static_cast<leaf_type &>(t).value;
    }

    template <size_t I, typename... Types>
    constexpr const std::tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &t) noexcept {
        using element_type = std::tuple_element_t<I, tuple<Types...>>;
        constexpr size_t remain_size = sizeof...(Types) - I - 1;
        using leaf_type = implements::tuple_leaf_index<I, element_type, remain_size>;
        return static_cast<const leaf_type &>(t).value;
    }

    template <size_t I, typename... Types>
    constexpr std::tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&t) noexcept { // NOLINT
        using element_type = std::tuple_element_t<I, tuple<Types...>>;
        constexpr size_t remain_size = sizeof...(Types) - I - 1;
        using leaf_type = implements::tuple_leaf_index<I, element_type, remain_size>;
        return utility::move(static_cast<leaf_type &>(t).value);
    }

    template <size_t I, typename... Types>
    constexpr const std::tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&t) noexcept {
        using element_type = std::tuple_element_t<I, tuple<Types...>>;
        constexpr size_t remain_size = sizeof...(Types) - I - 1;
        using leaf_type = implements::tuple_leaf_index<I, element_type, remain_size>;
        return utility::move(static_cast<const leaf_type &>(t).value);
    }

    // 辅助函数
    template <typename... Types>
    constexpr tuple<std::decay_t<Types>...> make_tuple(Types &&...args) {
        return tuple<std::decay_t<Types>...>(utility::forward<Types>(args)...);
    }

    // 添加forward_as_tuple
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

/* 元方法Trait */
namespace rainy::type_traits::extras::meta_method {
    template <typename Ty>
    struct has_iterator {
        template <typename U>
        static auto test(int) -> decltype(utility::begin(utility::declval<U &>()) != utility::end(utility::declval<U &>()),
                                          ++utility::declval<decltype(utility::begin(utility::declval<U &>())) &>(),
                                          --utility::declval<decltype(utility::begin(utility::declval<U &>())) &>(),
                                          (void)utility::declval<decltype(utility::begin(utility::declval<U &>())) &>()++,
                                          (void)utility::declval<decltype(utility::begin(utility::declval<U &>())) &>()--,
                                          *utility::begin(utility::declval<U &>()), helper::true_type{}) {
            return utility::declval<helper::true_type>();
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    struct has_empty_method {
        template <typename U>
        static auto test(int) -> decltype(utility::declval<U &>().empty(), helper::true_type{}) {
            return utility::declval<helper::true_type>();
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    struct has_size_method {
        template <typename U>
        static auto test(int) -> decltype(utility::declval<U &>().size(), helper::true_type{}) {
            return utility::declval<helper::true_type>();
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    struct has_data_method {
        template <typename U>
        static auto test(int) -> decltype(utility::declval<U &>().data(), helper::true_type{}) {
            return utility::declval<helper::true_type>();
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    struct has_assignment_operator {
        template <typename U>
        static auto test(int) -> decltype(utility::declval<U &>() = utility::declval<const U &>(), helper::true_type{}) {
            return utility::declval<helper::true_type>();
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
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
    };

    template <bool IsLvalue, bool IsRvalue>
    struct member_function_traits_base {
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = IsLvalue;
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = IsRvalue;
    };
}

namespace rainy::type_traits::primary_types {
    template <typename Ty>
    struct function_traits {
        static RAINY_CONSTEXPR_BOOL valid = false;
    };

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

    template <template <typename> typename FunctorContainer, typename Fx>
    struct function_traits<FunctorContainer<Fx>> : function_traits<Fx> {};

    template <typename Fx>
    using function_return_type = typename function_traits<Fx>::return_type;

    template <typename Fx>
    static inline constexpr std::size_t arity = function_traits<Fx>::arity;

    template <typename Fx>
    using param_list_in_tuple = typename function_traits<Fx>::tuple_like_type;

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
    constexpr auto invoke(Callable &&obj) noexcept(noexcept(static_cast<Callable &&>(obj)()))
        -> decltype(static_cast<Callable &&>(obj)()) {
        return static_cast<Callable &&>(obj)();
    }

    template <typename Callable, typename Ty1, typename... Args>
    constexpr auto invoke(Callable &&obj, Ty1 &&arg1, Args &&...args) noexcept(noexcept(
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
            logical_traits::disjunction_v<primary_types::is_void<Rx>, type_properties::is_invoke_convertible<type, Rx>>>;
        template <typename Rx_>
        using is_nothrow_invocable_r = helper::bool_constant<logical_traits::conjunction_v<
            is_nothrow_invocable,
            logical_traits::disjunction<primary_types::is_void<Rx>,
                                        logical_traits::conjunction<type_properties::is_invoke_convertible<type, Rx>,
                                                                    type_properties::is_invoke_nothrow_convertible<type, Rx>>>>>;
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
}

namespace rainy::type_traits::type_properties {
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_r_v = implements::is_invocable_r_helper<Rx, Callable, Args...>::value;

    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_r : helper::bool_constant<is_invocable_r_v<Rx, Callable, Args...>> {};


}

namespace rainy::utility {
    template <typename Ty1, typename Ty2>
    class compressed_pair;

    namespace implements {
        template <typename Ty, bool = std::is_final_v<Ty>>
        struct compressed_pair_empty : std::false_type {};

        template <typename Ty>
        struct compressed_pair_empty<Ty, true> : std::is_empty<Ty> {};

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
            using first_param_type = typename type_traits::implements::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::implements::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::implements::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::implements::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::implements::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::implements::_call_traits<second_type>::const_reference;

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first(x), second(y) {
            }

            constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
            }

            constexpr explicit compressed_pair_impl(second_param_type y) : second(y) {
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

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
                std::swap(first, pair.second);
                std::swap(second, pair.second);
            }

            first_type first;
            second_type second;
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 1> : protected type_traits::implements::remove_cv_t<Ty1> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::implements::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::implements::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::implements::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::implements::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::implements::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::implements::_call_traits<second_type>::const_reference;

            constexpr compressed_pair_impl() = default;

            constexpr explicit compressed_pair_impl(first_param_type first, second_param_type second) :
                first_type(first), second(second) {
            }

            constexpr explicit compressed_pair_impl(first_param_type first) : first_type(first) {
            }

            explicit compressed_pair_impl(second_param_type second) : second(second) {
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

            second_type second;
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 2> : protected type_traits::implements::remove_cv_t<Ty2>::type {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::implements::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::implements::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::implements::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::implements::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::implements::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::implements::_call_traits<second_type>::const_reference;

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

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
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
            using first_param_type = typename type_traits::implements::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::implements::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::implements::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::implements::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::implements::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::implements::_call_traits<second_type>::const_reference;

            compressed_pair_impl() = default;

            compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second_type(second) {
            }

            explicit compressed_pair_impl(first_param_type first) : first_type(first) {
            }

            explicit compressed_pair_impl(second_param_type second) : second_type(second) {
            }

            first_reference get_first() {
                return *this;
            }

            first_const_reference get_first() const {
                return *this;
            }

            second_reference get_second() {
                return *this;
            }
            second_const_reference get_second() const {
                return *this;
            }

            void swap(compressed_pair<Ty1, Ty2> &) {
            }
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 4> : protected type_traits::implements::remove_cv_t<Ty1> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::implements::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::implements::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::implements::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::implements::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::implements::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::implements::_call_traits<second_type>::const_reference;

            compressed_pair_impl() = default;

            compressed_pair_impl(first_param_type x, second_param_type y) : first_type(x), second(y) {
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
            using first_param_type = typename type_traits::implements::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::implements::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::implements::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::implements::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::implements::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::implements::_call_traits<second_type>::const_reference;

            compressed_pair_impl() = default;

            compressed_pair_impl(first_param_type first, second_param_type second) : first(first), second(second) {
            }

            explicit compressed_pair_impl(first_param_type first) : first(first), second(first) {
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

            void swap(compressed_pair<Ty1, Ty2> &y) {
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
    };
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
            return to_address(val.operator->());
        }
    }
}

namespace rainy::type_traits::composite_types {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = implements::_is_arithmetic_v<Ty>;

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
    template <typename Err>
    class unexpected {
    public:
        static_assert(type_traits::composite_types::is_object_v<Err>, "Err must be an object type");
        static_assert(!type_traits::primary_types::is_array_v<Err>, "Err must not be an array type");
        static_assert(!type_traits::type_properties::is_const_v<Err>, "Err must not be const");
        static_assert(!type_traits::type_properties::is_volatile_v<Err>, "Err must not be volatile");
        static_assert(!type_traits::primary_types::is_specialization_v<Err, unexpected>,
                      "Err must not be a specialization of unexpected.");
        template <typename Ty, typename Err2>
        friend class expected;

        unexpected() = delete;

        template <typename UError, typename type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UError>, unexpected> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UError>, std::in_place_t> &&
                                                               type_traits::type_properties::is_constructible_v<Err, UError>,
                                                           int> = 0>
        explicit constexpr unexpected(UError &&unex) noexcept(std::is_nothrow_constructible<Err, UError>::value) :
            unexpected_value(utility::forward<UError>(unex)) {
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Err, Args...>, int> = 0>
        explicit constexpr unexpected(std::in_place_t, Args &&...vals) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Err, Args...>) :
            unexpected_value(utility::forward<Args>(vals)...) {
        }

        template <typename Uty, typename... Args,
                  std::enable_if_t<type_traits::type_properties::is_constructible_v<Err, std::initializer_list<Uty> &, Args...>, int> = 0>
        explicit constexpr unexpected(std::in_place_t, std::initializer_list<Uty> ilist, Args &&...vals) noexcept(
            std::is_nothrow_constructible<Err, std::initializer_list<Uty> &, Args...>::value) :
            unexpected_value(ilist, utility::forward<Args>(vals)...) {
        }

        constexpr const Err &error() const & noexcept {
            return unexpected_value;
        }
        constexpr Err &error() & noexcept {
            return unexpected_value;
        }
        constexpr const Err &&error() const && noexcept {
            return utility::move(unexpected_value);
        }
        constexpr Err &&error() && noexcept {
            return utility::move(unexpected_value);
        }

        constexpr void swap(unexpected &other) noexcept(type_traits::type_properties::is_nothrow_swappable_v<Err>) {
            static_assert(type_traits::type_properties::is_swappable_v<Err>, "Err must be swappable");
            std::swap(unexpected_value, other.unexpected_value);
        }

        friend constexpr void swap(unexpected &left, unexpected &right) noexcept(type_traits::type_properties::is_nothrow_swappable_v<Err>) {
            left.swap(right);
        }

        template <typename UErr>
        friend constexpr bool operator==(const unexpected &left,
                                         const unexpected<UErr> &right) noexcept(noexcept(left.unexpected_value == right.error())) {
            return left.unexpected_value == right.error();
        }

    private:
        Err unexpected_value;
    };

    template <typename Err>
    unexpected(Err) -> unexpected<Err>;
}

namespace rainy::utility {
#if RAINY_HAS_CXX20
    template <typename Fx>
        requires(type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                 !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>)
    consteval auto get_overloaded_memfn(Fx memfn) {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }
#else
    template <typename Class, typename Fx,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                                                        !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>,
                                                    int> = 0>
    constexpr auto get_overloaded_memfn(Fx Class::*func) {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return func;
    }
#endif
}

namespace rainy::utility {
    template <typename Ty>
    struct type_converter {
        template <typename Uty>
        RAINY_NORETURN Ty convert(const Uty &wait_for_convert) {
            return Ty{};
        }
    };
}

#endif