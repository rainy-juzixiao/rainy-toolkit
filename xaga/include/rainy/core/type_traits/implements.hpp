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
#ifndef RAINY_CORE_TYPE_TRAITS_IMPLEMENTS_HPP
#define RAINY_CORE_TYPE_TRAITS_IMPLEMENTS_HPP
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
    /**
     * @brief Returns an rvalue reference to a type without constructing an object.
     *        返回类型的右值引用而不构造对象。
     *
     * This function is intended for use in unevaluated contexts only, such as decltype,
     * sizeof, or noexcept expressions. Calling this function results in a compile-time error.
     *
     * 此函数仅用于未求值上下文中，如decltype、sizeof或noexcept表达式。
     * 调用此函数会导致编译时错误。
     *
     * @tparam Ty The type to get a reference to
     *            要获取引用的类型
     * @return An rvalue reference to type Ty
     *         类型Ty的右值引用
     */
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
     * @brief Converts an object to an rvalue reference for move semantics.
     *        将对象转换为右值引用以实现移动语义。
     *
     * @tparam Ty The type of the object to convert.
     *            要转换的对象的类型。
     * @param arg The object to convert to an rvalue reference.
     *            要转换为右值引用的对象。
     * @return An rvalue reference of type `remove_reference_t_<Ty>&&`.
     *         返回类型为 `remove_reference_t_<Ty>&&` 的右值引用对象。
     *
     * @remark
     * This function is used to implement move semantics by converting an object
     * to an rvalue reference, allowing resource transfer instead of copying.
     *
     * 该函数用于实现移动语义（move semantics），通过将对象转换为右值引用以允许资源移动而非拷贝。
     *
     * @attention
     * Move semantics are typically used to optimize copying, especially when the
     * original value is no longer needed. Note that the moved-from object is in
     * a valid but unspecified state and should not be used without reinitialization.
     *
     * 移动语义通常用于优化对象的复制，尤其是在对象不再需要其原始值时。
     * 请注意对象在移动后处于有效但未指定的状态，未重新初始化前不可再安全使用。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn move(Ty &&arg) noexcept -> type_traits::implements::remove_reference_t<Ty> && {
        return static_cast<type_traits::implements::remove_reference_t<Ty> &&>(arg);
    }

    /**
     * @brief Exchanges the value of an object and returns the old value.
     *        交换对象的值并返回旧值。
     *
     * @tparam Ty The type of the object to exchange.
     *            被交换的对象的类型。
     * @tparam Other The type of the new value, defaults to `Ty`.
     *               新值的类型，默认为 `Ty`。
     * @param val The object to be exchanged.
     *            要被交换的对象。
     * @param new_val The new value to replace the object's current value.
     *                用于替换的对象的新值。
     * @return The old value of the exchanged object.
     *         返回被交换的对象的旧值。
     *
     * @remark
     * This function saves the current value of `val` as `old_val`, then sets `val`
     * to `new_val`, and finally returns `old_val`.
     *
     * 该函数将 `val` 的当前值保存为 `old_val`，然后将 `val` 设置为 `new_val`，最后返回 `old_val`。
     *
     * @attention
     * When using this function, ensure that `val` and `new_val` are move-constructible
     * and assignable to avoid potential exceptions. This function requires that
     * `Ty` is move-constructible and that `Ty&` is assignable from type `Other`.
     *
     * 使用此函数时，需要确保 `val` 和 `new_val` 的类型是可移动构造和可赋值的，以避免潜在的异常。
     * 因为此函数要求实例化类型的Ty是可移动构造，且Ty的左值引用是能赋值给Other类型的。
     */
    template <typename Ty, typename Other = Ty>
    constexpr rain_fn exchange(Ty &val, Other &&new_val) noexcept(type_traits::implements::is_nothrow_move_constructible_v<Ty> &&
                                                                  type_traits::implements::_is_nothrow_assignable_v<Ty &, Other>)
        -> Ty {
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
    /**
     * @brief Conditionally casts an object to an rvalue reference, falling back to const lvalue reference if move might throw.
     *        有条件地将对象转换为右值引用，如果移动操作可能抛出异常则回退到常量左值引用。
     *
     * @tparam Ty The type of the object to convert.
     *            要转换的对象的类型。
     * @param arg The object to convert.
     *            要转换的对象。
     * @return Either an rvalue reference (`Ty&&`) if `Ty` is nothrow move constructible,
     *         or a const lvalue reference (`const Ty&`) if `Ty` is copy constructible but move might throw.
     *         如果 `Ty` 是不抛异常的移动可构造，则返回右值引用（`Ty&&`）；
     *         如果 `Ty` 是可拷贝构造但移动可能抛出异常，则返回常量左值引用（`const Ty&`）。
     *
     * @remark
     * This function is useful in generic code where strong exception safety is required.
     * It ensures that if a move operation might throw, we instead use a copy operation
     * to maintain the strong exception guarantee.
     *
     * 此函数在需要强异常安全保证的泛型代码中非常有用。
     * 它确保如果移动操作可能抛出异常，我们改用拷贝操作来维持强异常保证。
     *
     * @attention
     * - If `Ty` is nothrow move constructible, returns `Ty&&` (move semantics)
     * - If `Ty` is not nothrow move constructible but is copy constructible, returns `const Ty&` (copy semantics)
     * - If neither condition is satisfied, this function participates in overload resolution
     *   but using it would be ill-formed
     *
     * - 如果 `Ty` 是不抛异常的移动可构造，返回 `Ty&&`（移动语义）
     * - 如果 `Ty` 不是不抛异常的移动可构造但是可拷贝构造，返回 `const Ty&`（拷贝语义）
     * - 如果两个条件都不满足，此函数参与重载决议但使用它会导致格式错误
     */
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

#endif
