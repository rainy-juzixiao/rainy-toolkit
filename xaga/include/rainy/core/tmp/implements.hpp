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

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

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
        using class_type = Ty2;
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

namespace rainy::type_traits::implements {
    template <typename To, typename From>
    RAINY_CONSTEXPR_BOOL _is_assignable_v = __is_assignable(To, From);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_move_assignable_v = __is_assignable(implements::_add_lvalue_reference_t<Ty>, Ty);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_move_constructible_v = __is_constructible(Ty, Ty);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_nothrow_move_constructible_v = __is_nothrow_constructible(Ty, Ty);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_nothrow_move_assignable_v = __is_nothrow_assignable(_add_lvalue_reference_t<Ty>, Ty);

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL _is_nothrow_constructible_v = __is_nothrow_constructible(Ty, Args...);
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
        return static_cast<type_traits::implements::remove_reference_t<Ty> && > (arg);
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
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::implements::remove_reference_t<Ty> &arg) noexcept {
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
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::implements::remove_reference_t<Ty> &&arg) noexcept { // NOLINT
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
    constexpr Ty exchange(Ty &val, Other &&new_val) noexcept(type_traits::implements::_is_nothrow_move_constructible_v<Ty> &&
                                                             type_traits::implements::_is_nothrow_assignable_v<Ty &, Other>) {
        Ty old_val = static_cast<Ty &&>(val);
        val = static_cast<Other &&>(new_val);
        return old_val;
    }
}

#endif