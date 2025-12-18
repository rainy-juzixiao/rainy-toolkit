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
#ifndef RAINY_CORE_TMP_IMPLEMENTS_HPP
#define RAINY_CORE_TMP_IMPLEMENTS_HPP
#include <cstdlib>
#include <rainy/core/platform.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4180)
#endif

namespace rainy::type_traits::implements {
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_same_v<Ty, Ty> = true;

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_of_v = (is_same_v<Ty, Types> || ...);

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_reference_v<Ty &> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_reference_v<Ty &&> = true;

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_const_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_const_v<const Ty> = true;

    template <typename Ty>
    constexpr bool _is_function_v = !_is_const_v<const Ty> && !_is_reference_v<Ty>;

    template <typename Ty>
    struct remove_volatile {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_volatile<volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<const Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<const volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_cv_t = typename remove_cv<Ty>::type;

    template <typename Ty>
    using remove_cvref_t = remove_cv_t<remove_reference_t<Ty>>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v =
        is_any_of_v<remove_cv_t<Ty>, bool, char, signed char, unsigned char, wchar_t,
#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
                    char8_t,
#endif
                    char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = is_any_of_v<remove_cv_t<Ty>, float, double, long double>;

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_pointer_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *const> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *volatile> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *const volatile> = true;
}

#define RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(...)                                                                             \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct is_member_function_pointer_helper<Rx (Class::*)(Args...) __VA_ARGS__> {                                                    \
        static constexpr bool value = true;                                                                                           \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct is_member_function_pointer_helper<Rx (Class::*)(Args..., ...) __VA_ARGS__> {                                               \
        static constexpr bool value = true;                                                                                           \
    }

namespace rainy::type_traits::implements {
#if RAINY_USING_CLANG
    template <typename Fx>
    struct is_member_function_pointer_helper {
        static constexpr bool value = __is_member_function_pointer(Fx);
    };
#else
    template <typename Fx>
    struct is_member_function_pointer_helper {
        static constexpr bool value = false;
    };

    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC();
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(&);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(&&);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const &);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const &&);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(& noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(&& noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const & noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const && noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile &);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile &&);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile & noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(volatile && noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile &);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile &&);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile & noexcept);
    RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC(const volatile && noexcept);
#endif
}

#undef RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC

namespace rainy::type_traits::implements {
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

    template <typename Ty, typename = void>
    struct _add_reference {
        using lvalue = Ty;
        using rvalue = Ty;
    };

    template <typename Ty>
    struct _add_reference<Ty, _void_t<Ty &>> {
        using lvalue = Ty &;
        using rvalue = Ty &&;
    };

    template <typename Ty>
    struct _add_lvalue_reference {
        using type = typename _add_reference<Ty>::lvalue;
    };

    template <typename Ty>
    using _add_lvalue_reference_t = typename _add_lvalue_reference<Ty>::type;

    template <typename Ty>
    struct _add_rvalue_reference {
        using type = typename _add_reference<Ty>::rvalue;
    };

    template <typename Ty>
    using _add_rvalue_reference_t = typename _add_rvalue_reference<Ty>::type;

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
    struct add_volatile {
        using type = volatile Ty;
    };

    template <typename Ty>
    struct add_cv {
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
    RAINY_CONSTEXPR20 Ty fake_copy_init(Ty) noexcept {
        return Ty{};
    }

    template <typename Ty>
    Ty returns_exactly() noexcept {
        return Ty{};
    }

    template <typename Ty>
    struct is_member_object_pointer_ {
        static constexpr bool value = false;
    };

    template <typename Ty1, typename Ty2>
    struct is_member_object_pointer_<Ty1 Ty2::*> {
        static constexpr bool value = !_is_function_v<Ty1>;
        using class_type = Ty2;
    };

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = is_integral_v<Ty> || is_floating_point_v<Ty>;

    template <typename Ty, bool = is_integral_v<Ty>>
    struct sign_base {
        using uty = remove_cv_t<Ty>;

        static constexpr bool is_signed = static_cast<uty>(-1) < static_cast<uty>(0);
        static constexpr bool is_unsigned = !is_signed;
    };

    template <typename Ty>
    struct sign_base<Ty, false> {
        static constexpr bool is_signed = is_floating_point_v<Ty>;
        static constexpr bool is_unsigned = false;
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
    struct call_traits {
    public:
        using value_type = Ty;
        using reference = Ty &;
        using const_reference = const Ty &;
        using param_type = typename ct_imp<Ty, _is_pointer_v<Ty>, is_arithmetic_v<Ty>, _is_enum_v<Ty>>::param_type;
    };

    template <typename Ty>
    struct call_traits<Ty &> {
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

namespace rainy::type_traits::implements {
    template <std::size_t>
    struct make_unsigned_by_size;

    template <>
    struct make_unsigned_by_size<1> {
        template <typename>
        using apply = unsigned char;
    };

    template <>
    struct make_unsigned_by_size<2> {
        template <typename>
        using apply = unsigned short;
    };

    template <>
    struct make_unsigned_by_size<4> {
        template <typename Ty>
        using apply =
            typename _select<is_same_v<Ty, long> || is_same_v<Ty, unsigned long>>::template type<unsigned long, unsigned int>;
    };

    template <>
    struct make_unsigned_by_size<8> {
        template <typename>
        using apply = unsigned long long;
    };

    template <typename Ty>
    using make_unsigned_raw = typename make_unsigned_by_size<sizeof(Ty)>::template apply<Ty>;

    template <std::size_t>
    struct make_signed_by_size;

    template <>
    struct make_signed_by_size<1> {
        template <typename>
        using apply = signed char;
    };

    template <>
    struct make_signed_by_size<2> {
        template <typename>
        using apply = short;
    };

    template <>
    struct make_signed_by_size<4> {
        template <typename Ty>
        using apply = typename _select<is_same_v<Ty, unsigned long> || is_same_v<Ty, long>>::template type<long, int>;
    };

    template <>
    struct make_signed_by_size<8> {
        template <typename>
        using apply = long long;
    };

    template <typename Ty>
    using make_signed_raw = typename make_signed_by_size<sizeof(Ty)>::template apply<Ty>;
}

namespace rainy::utility {
    template <typename Ty>
    rain_fn declval() noexcept -> type_traits::implements::_add_rvalue_reference_t<Ty> {
        static_assert(type_traits::implements::always_false<Ty>, "Calling declval is ill-formed.");
        std::abort();
    }
}

namespace rainy::type_traits::implements {
    struct is_constructible_impl {
        template <typename Ty, typename... Args, typename = decltype(Ty(utility::declval<Args>()...))>
        static helper::true_type test(int);

        template <typename, typename...>
        static helper::false_type test(...);

        template <typename Ty, typename... Args, typename = decltype(noexcept(Ty(utility::declval<Args>()...)))>
        static helper::true_type test_nothrow(int);

        template <typename, typename...>
        static helper::false_type test_nothrow(...);

        template <typename Ty, typename Arg, typename = decltype(noexcept(::new(std::nothrow) Ty(utility::declval<Arg>())))>
        static helper::true_type test_nothrow_for_one_arg(int);

        template <typename, typename...>
        static helper::false_type test_nothrow_for_one_arg(...);

        template <typename Ty, typename Arg, typename = decltype(::new Ty(utility::declval<Arg>()))>
        static helper::true_type test_for_one_arg(int);

        template <typename, typename>
        static helper::false_type test_for_one_arg(...);

        template <typename T>
        static helper::true_type ref_test(T);
        template <typename T>
        static helper::false_type ref_test(...);
    };

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_constructible_v = decltype(is_constructible_impl::test<Ty, Args...>(0))::value;

    template <typename Ty, typename Arg1>
    RAINY_CONSTEXPR_BOOL is_constructible_v<Ty, Arg1> = decltype(is_constructible_impl::test<Ty, Arg1>(0))::value;

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_constructible_v = decltype(is_constructible_impl::test_nothrow<Ty, Args...>(0))::value;

    template <typename Ty, typename Arg1>
    RAINY_CONSTEXPR_BOOL is_nothrow_constructible_v<Ty, Arg1> =
        decltype(is_constructible_impl::test_nothrow_for_one_arg<Ty, Arg1>(0))::value;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copy_constructible_v = is_constructible_v<Ty, _add_lvalue_reference_t<const Ty>>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_move_constructible_v = is_constructible_v<Ty, Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_move_constructible_v = is_nothrow_constructible_v<Ty, Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_nothrow_copy_constructible_v = is_nothrow_constructible_v<Ty, _add_lvalue_reference_t<const Ty>>;
}

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
    RAINY_NODISCARD constexpr type_traits::implements::remove_reference_t<Ty> &&move(Ty &&arg) noexcept {
        return static_cast<type_traits::implements::remove_reference_t<Ty> &&>(arg);
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
    constexpr Ty exchange(Ty &val, Other &&new_val) noexcept(type_traits::implements::is_nothrow_move_constructible_v<Ty> &&
                                                             type_traits::implements::_is_nothrow_assignable_v<Ty &, Other>) {
        Ty old_val = static_cast<Ty &&>(val);
        val = static_cast<Other &&>(new_val);
        return old_val;
    }
}

namespace rainy::type_traits::implements {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_union_v = __is_union(Ty);

    template <typename Ty,typename = void>
    RAINY_CONSTEXPR_BOOL is_class_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_class_v<Ty, type_traits::other_trans::void_t<int Ty::*>> = !is_union_v<Ty>;
}

namespace rainy::type_traits::implements {
    template <typename B>
    helper::true_type ibo_test_ptr_conv(const volatile B *) {
        return helper::true_type{};
    }

    template <typename>
    helper::false_type ibo_test_ptr_conv(const volatile void *) {
        return helper::false_type{};
    }

    template <typename B, typename D>
    auto test_is_base_of(int) -> decltype(ibo_test_ptr_conv<B>(static_cast<D *>(nullptr))) {
        return ibo_test_ptr_conv<B>(static_cast<D *>(nullptr))();
    }

    template <typename, typename>
    auto test_is_base_of(...) -> helper::true_type {
        return helper::true_type{};
    }

    template <typename Base, typename Derived>
    RAINY_CONSTEXPR_BOOL is_base_of_v =
        is_class_v<Base> && is_class_v<Derived> && (decltype(test_is_base_of<Base, Derived>(0))::value == true);
}

namespace rainy::type_traits::implements {
    template <typename Ty>
    auto ic_test_returnable(int) -> decltype(void(static_cast<Ty (*)()>(nullptr)), helper::true_type{}) {
        return helper::true_type{};
    }

    template <typename>
    auto ic_test_returnable(...) -> helper::false_type {
        return helper::false_type{};
    }

    template <typename From, typename To>
    auto test_implicitly_convertible(int)
        -> decltype(void(utility::declval<void (&)(To)>()(utility::declval<From>())), helper::true_type{}) {
        return helper::true_type{};
    }

    template <typename, typename>
    auto test_implicitly_convertible(...) -> helper::false_type {
        return helper::false_type{};
    }

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_convertible_v =
        (decltype(ic_test_returnable<To>(0))::value && decltype(test_implicitly_convertible<From, To>(0))::value) ||
        (_is_void_v<From> && _is_void_v<To>);

    template <typename From, typename To, typename = void>
    RAINY_CONSTEXPR_BOOL is_nothrow_convertible_v = false;

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL
        is_nothrow_convertible_v<From, To,
                                 other_trans::void_t<decltype(static_cast<To (*)()>(nullptr)),
                                                     decltype(utility::declval<void (&)(To) noexcept>()(utility::declval<From>()))>> =
            true;
}

namespace rainy::type_traits::implements {
    template <typename To, typename From, typename = void>
    RAINY_CONSTEXPR_BOOL is_assignable_v = false;

    template <typename To, typename From>
    RAINY_CONSTEXPR_BOOL
        is_assignable_v<To, From, type_traits::other_trans::void_t<decltype(utility::declval<To>() = utility::declval<From>())>> =
            true;

    template <typename To, typename From, typename = void>
    RAINY_CONSTEXPR_BOOL is_nothrow_assignable_v = false;

    template <typename To, typename From>
    RAINY_CONSTEXPR_BOOL is_nothrow_assignable_v<
        To, From, type_traits::other_trans::void_t<decltype(noexcept(utility::declval<To>() = utility::declval<From>()))>> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_move_assignable_v = is_assignable_v<implements::_add_lvalue_reference_t<Ty>, Ty>;
}

namespace rainy::utility {
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn move_if_noexcept(Ty &arg) noexcept
        -> type_traits::other_trans::conditional_t<!type_traits::implements::is_nothrow_move_constructible_v<Ty> &&
                                                       type_traits::implements::is_copy_constructible_v<Ty>,
                                                   const Ty &, Ty &&> {
        return utility::move(arg);
    }
}

namespace rainy::type_traits::implements {
    void swap() = delete;

    template <typename, typename = void>
    RAINY_CONSTEXPR_BOOL has_adl_swap = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_adl_swap<Ty, void_t<decltype(swap(utility::declval<Ty &>(), utility::declval<Ty &>()))>> = true;
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
