#ifndef RAINY_CORE_TYPETRAITS_HPP
#define RAINY_CORE_TYPETRAITS_HPP

#ifndef RAINY_CORE_HPP
#include <rainy/core/core.hpp>
#endif

/* 数组修改 */
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
}

namespace rainy::type_traits::internals {
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL _is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_same_v<Ty, Ty> = true;

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL _is_any_of_v = (_is_same_v<Ty, Types> || ...);

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
    RAINY_CONSTEXPR_BOOL _is_function_v = !_is_const_v<const Ty> && !_is_reference_v<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_nothrow_move_constructible = __is_nothrow_constructible(Ty, Ty);

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR_BOOL _is_nothrow_constructible_v = __is_nothrow_constructible(Ty, Args...);

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_enum_v = __is_enum(Ty);

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
    RAINY_CONSTEXPR_BOOL _is_trivially_destructible_v = std::__is_destructible_safe<Ty>::value && __has_trivial_destructor(Ty);
#endif

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_lvalue_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_lvalue_reference_v<Ty &> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_void_v = _is_same_v<Ty, void>;

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

    template <typename...>
    using void_t = void;

    template <typename Ty, class = void>
    struct add_reference_impl {
        using lvalue = Ty;
        using rvalue = Ty;
    };

    template <typename Ty>
    struct add_reference_impl<Ty, void_t<Ty &>> {
        using lvalue = Ty &;
        using rvalue = Ty &&;
    };


    template <typename Ty>
    using remove_reference_t_ = typename remove_reference<Ty>::type;

    template <typename Ty>
    struct add_lvalue_reference {
        using type = typename internals::add_reference_impl<Ty>::lvalue;
    };

    template <typename Ty>
    using add_lvalue_reference_t = typename add_lvalue_reference<Ty>::type;

    template <typename Ty>
    struct add_rvalue_reference {
        using type = typename internals::add_reference_impl<Ty>::rvalue;
    };

    template <typename Ty>
    using add_rvalue_reference_t = typename add_rvalue_reference<Ty>::type;

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
    using remove_cvref_t = remove_cv_t<remove_reference_t_<Ty>>;

    template <typename Ty, typename = void>
    struct add_pointer_ { // add pointer (pointer type cannot be formed)
        using type = Ty;
    };

    template <typename Ty>
    struct add_pointer_<Ty, _void_t<remove_reference_t_<Ty> *>> { // (pointer type can be formed)
        using type = remove_reference_t_<Ty> *;
    };

    template <typename Ty>
    using add_pointer_t_ = typename add_pointer_<Ty>::type;

    enum class invoker_strategy {
        functor,
        pmf_object,
        pmf_refwrap,
        pmf_pointer,
        pmd_object,
        pmd_refwrap,
        pmd_pointer
    };

    struct invoker_functor {
        static constexpr auto strategy = invoker_strategy::functor;

        template <typename Callable, typename... Args>
        static constexpr auto invoke(Callable &&object, Args &&...args) noexcept(
            noexcept(static_cast<Callable &&>(object)(static_cast<Args &&>(args)...)))
            -> decltype(static_cast<Callable &&>(object)(static_cast<Args &&>(args)...)) {
            return static_cast<Callable &&>(object)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_object {
        static constexpr auto strategy = invoker_strategy::pmf_object;

        template <typename decayed, typename Ty, typename... Args>
        static constexpr auto invoke(decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept((static_cast<Ty &&>(args1).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)) {
            return (static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_refwrap {
        static constexpr auto strategy = invoker_strategy::pmf_refwrap;

        template <typename decayed, typename ref_wrap, typename... Args>
        static constexpr auto invoke(decayed pmf, ref_wrap r_ref_wrap,
                                     Args &&...args) noexcept(noexcept((r_ref_wrap.get().*pmf)(static_cast<Args &&>(args)...)))
            -> decltype((r_ref_wrap.get().*pmf)(static_cast<Args &&>(args)...)) {
            return (r_ref_wrap.get().*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_pointer {
        static constexpr auto strategy = invoker_strategy::pmf_pointer;

        template <typename decayed, typename Ty, typename... Args>
        static constexpr auto invoke(decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept(((*static_cast<Ty &&>(args1)).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)) {
            return ((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmd_object {
        static constexpr auto strategy = invoker_strategy::pmd_object;

        template <typename decayed, typename Ty>
        static constexpr auto invoke(decayed pmd, Ty &&args) noexcept -> decltype(static_cast<Ty &&>(args).*pmd) {
            return static_cast<Ty &&>(args).*pmd;
        }
    };

    struct invoker_pmd_refwrap {
        static constexpr auto strategy = invoker_strategy::pmd_refwrap;

        template <typename decayed, typename ref_wrap>
        static constexpr auto invoke(decayed pmd, ref_wrap r_ref_wrap) noexcept -> decltype(r_ref_wrap.get().*pmd) {
            return r_ref_wrap.get().*pmd;
        }
    };

    struct invoker_pmd_pointer {
        static constexpr auto strategy = invoker_strategy::pmd_pointer;

        template <typename decayed, typename Ty>
        static constexpr auto invoke(decayed pmd, Ty &&args) noexcept(noexcept((*static_cast<Ty &&>(args)).*pmd))
            -> decltype((*static_cast<Ty &&>(args)).*pmd) {
            return (*static_cast<Ty &&>(args)).*pmd;
        }
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
    RAINY_CONSTEXPR_BOOL _is_integral_v =
        _is_any_of_v<remove_cv_t<Ty>, bool, char, signed char, unsigned char, wchar_t,
#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
                     char8_t,
#endif
                     char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_floating_point_v = _is_any_of_v<remove_cv_t<Ty>, float, double, long double>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_arithmetic_v = _is_integral_v<Ty> || _is_floating_point_v<Ty>;

    template <typename Ty, bool = _is_integral_v<Ty>>
    struct sign_base {
        using uty = remove_cv_t<Ty>;

        static constexpr bool _signed = static_cast<uty>(-1) < static_cast<uty>(0);
        static constexpr bool _unsigned = !_signed;
    };

    template <typename Ty>
    struct sign_base<Ty, false> {
        static constexpr bool _signed = _is_floating_point_v<Ty>;
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

/* 指针修改 */
namespace rainy::type_traits::pointer_modify {
    template <typename Ty>
    using add_pointer = internals::add_pointer_<Ty>;

    template <typename Ty>
    using add_pointer_t = internals::add_pointer_t_<Ty>;

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

// 引用修改
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

    template <typename Ty>
    struct add_lvalue_reference {
        using type = typename internals::add_reference_impl<Ty>::lvalue;
    };

    template <typename Ty>
    using add_lvalue_reference_t = typename add_lvalue_reference<Ty>::type;

    template <typename Ty>
    struct add_rvalue_reference {
        using type = typename internals::add_reference_impl<Ty>::rvalue;
    };

    template <typename Ty>
    using add_rvalue_reference_t = typename add_rvalue_reference<Ty>::type;
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

    template <typename Ty>
    struct cv_modify_helper {
        using remove_cv = cv_modify::remove_cv_t<Ty>;
        using remove_cvref = cv_modify::remove_cvref_t<Ty>;
        using remove_volatile = cv_modify::remove_volatile_t<Ty>;
        using remove_const = cv_modify::remove_const_t<Ty>;
        using add_const = cv_modify::add_const_t<Ty>;
        using add_volatile = cv_modify::add_volatile_t<Ty>;
        using add_cvlref = cv_modify::add_cvlref_t<Ty>;
        using add_cvrref = cv_modify::add_cvrref_t<Ty>;
        using add_cv = cv_modify::add_cv_t<Ty>;
    };
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
    RAINY_NODISCARD constexpr type_traits::internals::remove_reference_t_<Ty> &&move(Ty &&arg) noexcept {
        return static_cast<typename type_traits::internals::remove_reference<Ty>::type &&>(arg);
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
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::internals::remove_reference_t_<Ty> &arg) noexcept {
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
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::internals::remove_reference_t_<Ty> &&arg) noexcept {
        static_assert(!type_traits::internals::_is_lvalue_reference_v<Ty>, "bad forward call");
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
    constexpr Ty exchange(Ty &val, Other &&new_val) noexcept(type_traits::internals::_is_nothrow_move_constructible<Ty> &&
                                                             type_traits::internals::_is_nothrow_assignable_v<Ty &, Other>) {
        Ty old_val = static_cast<Ty &&>(val);
        val = static_cast<Other &&>(new_val);
        return old_val;
    }
}

namespace rainy::utility {
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty *addressof(Ty &val) noexcept {
        return __builtin_addressof(val);
    }

    template <typename Ty>
    const Ty *addressof(const Ty &&) = delete;

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
        type_traits::internals::_is_nothrow_constructible_v<Ty, Args...>) {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            std::construct_at(utility::addressof(object), utility::forward<Args>(args)...);
            return;
        }
#endif
        ::new (static_cast<void *>(utility::addressof(object))) Ty(utility::forward<Args>(args)...);
    }
}

namespace rainy::type_traits::other_trans {
    /**
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。 当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     * Condition 才存在（并且是 true 的同义词）。
     * @tparam test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型。
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef（它是 Ty 的同义词）。如果 test 为
     * false，则 enable_if_t<test, Ty> 结果不会拥有名为“type”的嵌套 typedef
     */
    template <bool Test, typename Ty>
    struct enable_if;

    template <typename Ty>
    struct enable_if<true, Ty> {
        using type = Ty;
    };

    /**
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。 当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     * Condition 才存在（并且是 true 的同义词）。
     * @tparam test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型。
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef（它是 Ty 的同义词）。如果 test 为
     * false，则 enable_if_t<test, Ty> 结果不会拥有名为“type”的嵌套 typedef
     */
    template <bool Test, typename Ty = void>
    using enable_if_t = typename enable_if<Test, Ty>::type;

    template <bool, typename IfTrue, typename>
    struct conditional {
        using type = IfTrue;
    };

    template <typename IfTrue, typename IfFalse>
    struct conditional<false, IfTrue, IfFalse> {
        using type = IfFalse;
    };

    template <bool Test, typename IfTrue, typename IfFalse>
    using conditional_t = typename conditional<Test, IfTrue, IfFalse>::type;

    template <typename ValueType, bool Test, ValueType IfTrue, ValueType>
    struct conditional_value {
        static inline constexpr ValueType value = IfTrue;
    };

    template <typename ValueType, ValueType IfTrue, ValueType IfFalse>
    struct conditional_value<ValueType, false, IfTrue, IfFalse> {
        static inline constexpr ValueType value = IfFalse;
    };

    template <bool Test, bool IfTrue, bool IfFalse>
    struct conditional_bool {
        static RAINY_CONSTEXPR_BOOL value = IfTrue;
    };

    template <bool IfTrue, bool IfFalse>
    struct conditional_bool<false, IfTrue, IfFalse> {
        static RAINY_CONSTEXPR_BOOL value = IfFalse;
    };

    template <bool Test, bool IfTrue, bool IfFalse>
    static RAINY_CONSTEXPR_BOOL conditional_bool_v = conditional_bool<Test, IfTrue, IfFalse>::value;

    template <typename ValueType, bool Test, ValueType IfTrue, ValueType IfFalse>
    static inline constexpr ValueType conditional_value_v = conditional_value<ValueType, Test, IfTrue, IfFalse>::value;

    template <typename...>
    using void_t = void;

    template <bool>
    struct select {
        template <typename Ty1, typename>
        using apply = Ty1;
    };

    template <>
    struct select<false> {
        template <typename, typename Ty2>
        using apply = Ty2;
    };

    template <typename Ty>
    struct decay {
        using Ty1 = reference_modify::remove_reference_t<Ty>;
        using Ty2 = typename select<internals::_is_function_v<Ty1>>::template apply<pointer_modify::add_pointer<Ty1>,
                                                                                    cv_modify::remove_cv<Ty1>>;
        using type =
            typename select<internals::_is_array_v<Ty1>>::template apply<pointer_modify::add_pointer<array_modify::remove_extent<Ty1>>,
                                                                         Ty2>::type;
    };

    template <typename Ty>
    using decay_t = typename decay<Ty>::type;
}

// type_traits的帮助程序类和 typedef
namespace rainy::type_traits::helper {
    /**
     * @brief 从类型和值生成整型常量。
     * @tparam Ty 类型
     * @tparam data 值
     */
    template <typename Ty, Ty data>
    struct integral_constant {
        using value_type = Ty;
        using type = integral_constant;

        constexpr explicit operator value_type() const noexcept {
            return value;
        }
        constexpr value_type operator()() const noexcept {
            return value;
        }

        static constexpr Ty value = data;
    };

    /**
     * @brief 将 bool 用作 Ty 参数的 integral_constant 的显式部分特化
     */
    template <bool boolean>
    using bool_constant = integral_constant<bool, boolean>;

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    template <typename>
    struct char_space : integral_constant<char, ' '> {};

    template <>
    struct char_space<wchar_t> : integral_constant<wchar_t, ' '> {};

    template <>
    struct char_space<char16_t> : integral_constant<char16_t, u' '> {};

    template <>
    struct char_space<char32_t> : integral_constant<char32_t, u' '> {};

    template <typename char_type>
    RAINY_INLINE_CONSTEXPR char_type char_space_v = char_space<char_type>::value;

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    template <>
    struct char_space<char8_t> : integral_constant<char8_t, ' '> {};
#endif
    template <typename char_type>
    struct char_null : integral_constant<char, '\0'> {};

    template <>
    struct char_null<wchar_t> : integral_constant<wchar_t, '\0'> {};

    template <>
    struct char_null<char16_t> : integral_constant<char16_t, u'\0'> {};

    template <>
    struct char_null<char32_t> : integral_constant<char32_t, u'\0'> {};

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    template <>
    struct char_null<char8_t> : integral_constant<char8_t, '\0'> {};
#endif

    template <typename char_type>
    RAINY_INLINE_CONSTEXPR char_type char_null_v = char_null<char_type>::value;

    template <typename char_type>
    RAINY_CONSTEXPR_BOOL is_wchar_t = internals::_is_same_v<char_type, wchar_t>;

    template <typename Ty>
    struct wrapper {
        inline static cv_modify::remove_cvref_t<Ty> value;
    };

    template <typename Ty>
    inline constexpr cv_modify::remove_cvref_t<Ty> &get_fake_object() noexcept {
        return wrapper<cv_modify::remove_cvref_t<Ty>>::value;
    }

    template <typename Ty>
    struct identity {
        using type = Ty;
    };

    template <typename Ty>
    using identity_t = typename identity<Ty>::type;
}

// 类型关系
namespace rainy::type_traits::type_relations {
    /**
     * @brief 测试两个类型是否相同
     * @tparam Ty1 检索的第一个类型
     * @tparam Ty2 检索的第二个类型
     */
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_same_v<Ty, Ty> = true;

    /**
     * @brief 测试两个类型是否相同。
     * @tparam Ty1 检索的第一个类型
     * @tparam Ty2 检索的第二个类型
     */
    template <typename Ty1, typename Ty2>
    struct is_same : helper::bool_constant<is_same_v<Ty1, Ty2>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = is_same_v<Ty, void>;

    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};

    template <typename base, typename derived>
    struct is_base_of : helper::bool_constant<__is_base_of(base, derived)> {};

    template <typename base, typename derived>
    RAINY_CONSTEXPR_BOOL is_base_of_v = __is_base_of(base, derived);

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_of_v = (is_same_v<Ty, Types> || ...);

#if RAINY_USING_MSVC || RAINY_USING_CLANG
    template <typename from, typename to>
    RAINY_CONSTEXPR_BOOL is_convertible_v = __is_convertible_to(from, to);
#else
    template <typename from, typename to>
    RAINY_CONSTEXPR_BOOL is_convertible_v = __is_convertible(from, to);
#endif

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_convertible_v = (is_convertible_v<Ty, Types> || ...);
}

namespace rainy::type_traits::logical_traits {
    template <typename...>
    struct conjunction : helper::true_type {};

    template <typename First, typename... Rest>
    struct conjunction<First, Rest...> : internals::_conjunction<First::value, First, Rest...>::type {};

    template <typename... Traits>
    RAINY_CONSTEXPR_BOOL conjunction_v = conjunction<Traits...>::value;

    template <typename...>
    struct disjunction : helper::false_type {};

    template <typename first, typename... rest>
    struct disjunction<first, rest...> : internals::_disjunction<first::value, first, rest...>::type {};

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
    type_traits::internals::add_rvalue_reference_t<Ty> declval() noexcept {
        static_assert(type_traits::internals::always_false<Ty>, "Calling declval is ill-formed, see N4950 [declval]/2.");
        static bool never_returned = true;
        return reinterpret_cast<type_traits::internals::add_rvalue_reference_t<Ty>>(never_returned);
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
            static_assert(rainy::type_traits::internals::always_false<Uty>,
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
            static_assert(rainy::type_traits::internals::always_false<Uty>,
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
            static_assert(rainy::type_traits::internals::always_false<Uty>,
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
            static_assert(rainy::type_traits::internals::always_false<Uty>,
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
                                 other_trans::void_t<decltype(internals::_fake_copy_init<To>(internals::_returns_exactly<From>()))>>
        : helper::true_type {};

    template <typename From,typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_convertible_v = is_invoke_convertible<From, To>::value;

    template <typename From, typename To>
    struct is_invoke_nothrow_convertible
        : helper::bool_constant<noexcept(internals::_fake_copy_init<To>(internals::_returns_exactly<From>()))> {};

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_invoke_nothrow_convertible_v = is_invoke_convertible<From, To>::value;
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
    RAINY_CONSTEXPR_BOOL is_enum_v = internals::_is_enum_v<Ty>;

    template <typename Ty>
    struct is_enum : helper::bool_constant<is_enum_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pod_v = std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v = internals::_is_integral_v<Ty>;

    template <typename Ty>
    struct is_integral : helper::bool_constant<is_integral_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = internals::_is_floating_point_v<Ty>;

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
    RAINY_CONSTEXPR_BOOL is_function_v = type_traits::internals::_is_function_v<Ty>;

    template <typename Ty>
    struct is_function : helper::bool_constant<type_traits::internals::_is_function_v<Ty>> {};

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
    RAINY_CONSTEXPR_BOOL is_array_v = type_traits::internals::_is_array_v<Ty>;

    template <typename Ty>
    struct is_array : helper::bool_constant<is_array_v<Ty>> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_pointer_v = type_traits::internals::_is_pointer_v<Ty>;

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
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = internals::is_member_object_pointer_<cv_modify::remove_cv_t<Ty>>::value;
#endif

    template <typename Ty>
    struct is_member_object_pointer : helper::bool_constant<is_member_object_pointer_v<Ty>> {};
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

namespace rainy::type_traits::internals {
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

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_object_pointer_v = is_member_function_pointer_helper<cv_modify::remove_cv_t<Ty>>::value;
}

#undef RAINY_IS_MEMBER_FUNCTION_POINTER_HELPER_SPEC

namespace rainy::type_traits::primary_types
{
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_function_pointer_v = internals::is_member_function_pointer_helper<cv_modify::remove_cv_t<Ty>>::value;

    template <typename Ty>
    struct is_member_function_pointer : helper::bool_constant<is_member_function_pointer_v<Ty>> {};
}

/* 元方法Trait */
namespace rainy::type_traits::extras::meta_method {
    template <typename Ty>
    struct has_iterator {
        template <typename U>
        static auto test(int) -> decltype(utility::begin(utility::declval<U &>()) != utility::end(utility::declval<U &>()),
                                          ++utility::declval<decltype(utility::begin(utility::declval<U &>())) &>(),
                                          --utility::declval<decltype(utility::begin(utility::declval<U &>())) &>(),
                                          utility::declval<decltype(utility::begin(utility::declval<U &>())) &>()++,
                                          utility::declval<decltype(utility::begin(utility::declval<U &>())) &>()--,
                                          *utility::begin(utility::declval<U &>()), helper::true_type{}) {
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
    class finally_impl : Callable {
    public:
        finally_impl(Callable &&callable) noexcept : Callable(utility::forward<Callable>(callable)), invalidate(false) {
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
            return invalidate;
        }

        void to_invalidate() noexcept {
            invalidate = true;
        }

        void to_useable() noexcept {
            invalidate = false;
        }

        void invoke_now() const {
            (*this)();
        }

        template <typename Pred>
        void set_condition(Pred &&pred) {
            invalidate = static_cast<bool>(pred());
        }

    private:
        bool invalidate;
    };

    template <typename Callable>
    auto make_finally(Callable &&callable) -> finally_impl<Callable> {
        return finally_impl<Callable>(utility::forward<Callable>(callable));
    }
}


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

#define RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, SPEC)                                                    \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args...) SPEC> : internals::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> { \
        using return_type = Rx;                                                                                                       \
        using tuple_like_type = std::tuple<Args...>;                                                                                  \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args..., ...) SPEC>                                                                                     \
        : internals::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> {                                      \
        using return_type = Rx;                                                                                                       \
    };

#define RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer, IsLValue, IsRValue, SPEC)  \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args...) SPEC>                                                                               \
        : internals::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                 \
          internals::member_function_traits_base<IsLValue, IsRValue> {                                                                \
        using return_type = Rx;                                                                                                       \
        using tuple_like_type = std::tuple<Args...>;                                                                                  \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args..., ...) SPEC>                                                                          \
        : internals::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                 \
          internals::member_function_traits_base<IsLValue, IsRValue> {                                                                \
        using return_type = Rx;                                                                                                       \
    };


namespace rainy::type_traits::internals {
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
    struct function_traits<Rx(Args...)> : internals::function_traits_base<> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args..., ...)> : internals::function_traits_base<> {
        using return_type = Rx;
    };

    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(false, true, volatile)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, false, noexcept)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, true, volatile noexcept)

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...)> : internals::function_traits_base<false, true> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...)> : internals::function_traits_base<false, true> {
        using return_type = Rx;
    };

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...) noexcept> : internals::function_traits_base<false, true, true> {
        using return_type = Rx;
        using tuple_like_type = std::tuple<Args...>;
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...) noexcept> : internals::function_traits_base<false, true, true> {
        using return_type = Rx;
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
}

namespace rainy::utility::internals {
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
        static_assert(type_traits::internals::_is_object_v<Ty> || type_traits::internals::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        using type = Ty;

        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<
                                                            type_traits::logical_traits::negation<type_traits::type_relations::is_same<
                                                                type_traits::cv_modify::remove_cvref_t<Uty>, reference_wrapper>>,
                                                            internals::refwrap_has_ctor_from<Ty, Uty>>,
                                                        int> = 0>
        constexpr reference_wrapper(Uty &&val) noexcept(noexcept(internals::refwrap_ctor_fun<Ty>(declval<Uty>()))) {
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
            noexcept(internals::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
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
            noexcept(internals::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
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

    template <class _Ty>
    using unwrap_reference_t = typename unwrap_reference<_Ty>::type;

    template <typename Ty>
    using unwrap_ref_decay_t = unwrap_reference_t<other_trans::decay_t<Ty>>;

    template <typename Ty>
    struct unwrap_ref_decay {
        using type = unwrap_ref_decay_t<Ty>;
    };
}

namespace rainy::utility {
    enum class invoker_category {
        functor, // 仿函数或函数类型
        pmf_object, // 类成员函数——对象调用
        pmf_refwrap, // 类成员函数——对象引用包装器调用
        pmf_pointer, // 类成员函数——对象指针调用
        pmd_object, // 类成员变量——对象调用
        pmd_refwrap, // 类成员变量——对象引用包装器调用
        pmd_pointer // 类成员变量——对象指针调用
    };
}

namespace rainy::utility::internals {
    template <invoker_category>
    struct invoker_impl {
    };

    template <>
    struct invoker_impl<invoker_category::functor> {
        static constexpr auto category = invoker_category::functor;

        template <typename Callable, typename... Args>
        static auto invoke(Callable &&callable,
                           Args &&...args) noexcept(noexcept(static_cast<Callable &&>(callable)(static_cast<Args &&>(args)...)))
            -> decltype(rainy::utility::forward<Callable>(callable)(rainy::utility::forward<Args>(args)...)) {
            return static_cast<Callable &&>(callable)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_object> {
        static constexpr auto category = invoker_category::pmf_object;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept((static_cast<Ty &&>(args1).*pmf)(
            static_cast<Args &&>(args)...))) -> decltype((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)) {
            return (static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_refwrap> {
        static constexpr auto category = invoker_category::pmf_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, RefWrap<Ty> ref_wrap,
                                     Args &&...args) noexcept(noexcept((ref_wrap.get().*pmf)(static_cast<Args &&>(args)...))) {
            return (ref_wrap.get().*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_pointer> {
        static constexpr auto category = invoker_category::pmf_pointer;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept(((*static_cast<Ty &&>(args1)).*pmf)(
            static_cast<Args &&>(args)...))) -> decltype(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)) {
            return ((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_object> {
        static constexpr auto category = invoker_category::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept -> decltype(static_cast<Ty &&>(args).*pmd) {
            return static_cast<Ty &&>(args).*pmd;
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_refwrap> {
        static constexpr auto category = invoker_category::pmd_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty>
        static constexpr auto invoke(Decayed pmd, RefWrap<Ty> ref_wrap) noexcept -> decltype(ref_wrap.get().*pmd) {
            return ref_wrap.get().*pmd;
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_pointer> {
        static constexpr auto category = invoker_category::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept(noexcept((*static_cast<Ty &&>(args)).*
                                                                               pmd)) -> decltype((*static_cast<Ty &&>(args)).*pmd) {
            return (*static_cast<Ty &&>(args)).*pmd;
        }
    };

    template <typename Callable, typename Ty1, typename RemovedCvref = type_traits::cv_modify::remove_cvref_t<Callable>,
              bool is_pmf = type_traits::primary_types::is_member_function_pointer_v<RemovedCvref>,
              bool is_pmd = type_traits::primary_types::is_member_object_pointer_v<RemovedCvref>>
    struct select_invoker {};

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, true, false> {
        using __class_type = typename type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type;
        using __ty1_type = type_traits::cv_modify::remove_cvref_t<Ty1>;

        template <bool Test, invoker_category IfTrue, invoker_category IfFalse>
        static constexpr auto cond_v = type_traits::other_trans::conditional_value_v<invoker_category, Test, IfTrue, IfFalse>;

        static constexpr auto value = cond_v < type_traits::type_relations::is_same_v<__class_type, __ty1_type> ||
                                      type_traits::type_relations::is_base_of_v<__class_type, __ty1_type>,
                              invoker_category::pmf_object,
                              cond_v < type_traits::primary_types::is_specialization_v<__ty1_type, std::reference_wrapper> ||
                                  type_traits::primary_types::is_specialization_v<__ty1_type, utility::reference_wrapper>,
                              invoker_category::pmf_refwrap, invoker_category::pmf_pointer >>
            ;
    };

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, false, true> {
        using __class_type = typename type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type;
        using __ty1_type = type_traits::cv_modify::remove_cvref_t<Ty1>;

        template <bool Test, invoker_category IfTrue, invoker_category IfFalse>
        static constexpr auto cond_v = type_traits::other_trans::conditional_value_v<invoker_category, Test, IfTrue, IfFalse>;

        static constexpr auto value = cond_v < type_traits::type_relations::is_same_v<__class_type, __ty1_type> ||
                                      type_traits::type_relations::is_base_of_v<__class_type, __ty1_type>,
                              invoker_category::pmd_object,
                              cond_v < type_traits::primary_types::is_specialization_v<__ty1_type, std::reference_wrapper> ||
                                  type_traits::primary_types::is_specialization_v<__ty1_type, utility::reference_wrapper>,
                              invoker_category::pmd_refwrap, invoker_category::pmd_pointer >>
            ;
    };

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, false, false> {
        static constexpr auto value = invoker_category::functor;
    };
}

namespace rainy::utility {
    template <typename Callable, typename Ty1 = void>
    struct invoker : internals::invoker_impl<internals::select_invoker<Callable, Ty1>::value> {};

    template <typename Callable>
    constexpr auto invoke(Callable &&object) noexcept(noexcept(static_cast<Callable &&>(object)()))
        -> decltype(static_cast<Callable &&>(object)()) {
        return static_cast<Callable &&>(object)();
    }

    template <typename Callable, typename Ty, typename... Args>
    constexpr auto invoke(Callable &&object, Ty &&args1, Args &&...args2) noexcept(noexcept(
        invoker<Callable, Ty>::invoke(static_cast<Callable &&>(object), static_cast<Ty &&>(args1), static_cast<Args &&>(args2)...)))
        -> decltype(invoker<Callable, Ty>::invoke(static_cast<Callable &&>(object), static_cast<Ty &&>(args1),
                                                  static_cast<Args &&>(args2)...)) {
        // 我们可以直接使用invoker提供的invoke模板函数完成调用
        return invoker<Callable, Ty>::invoke(utility::forward<Callable>(object), utility::forward<Ty>(args1),
                                             utility::forward<Args>(args2)...);
    }
}

namespace rainy::type_traits::internals {
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

    template <typename Callable, typename Ty1, typename... Args2>
    using decltype_invoke_nonzero =
        decltype(utility::invoker<Callable, Ty1>::invoke(utility::declval<Callable>(), utility::declval<Ty1>(), utility::declval<Args2>()...));

    template <typename Callable, typename Ty1, typename... Args2>
    struct invoke_traits_nonzero<other_trans::void_t<decltype_invoke_nonzero<Callable, Ty1, Args2...>>, Callable, Ty1, Args2...>
        : invoke_traits_common<decltype_invoke_nonzero<Callable, Ty1, Args2...>,
                               noexcept(utility::invoker<Callable, Ty1>::invoke(utility::declval<Callable>(), utility::declval<Ty1>(),
                                                                       utility::declval<Args2>()...))> {};

    template <typename Callable, typename... Args>
    using select_invoke_traits = other_trans::conditional_t<sizeof...(Args) == 0, invoke_traits_zero<void, Callable>,
                                                            invoke_traits_nonzero<void, Callable, Args...>>;

    template <typename Rx,typename Callable,typename... Args>
    using is_invocable_r_helper = typename select_invoke_traits<Callable,Args...>::template is_invocable_r<Rx>;
}

namespace rainy::type_traits::type_properties {
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_r_v = internals::is_invocable_r_helper<Rx, Callable, Args...>::value;

    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_r : helper::bool_constant<is_invocable_r_v<Rx, Callable, Args...>> {};
}

namespace rainy::utility {
    template <typename Ty1, typename Ty2>
    class compressed_pair;

    namespace internals {
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
            using first_param_type = typename type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::internals::_call_traits<second_type>::const_reference;

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
        class compressed_pair_impl<Ty1, Ty2, 1> : protected type_traits::internals::remove_cv_t<Ty1> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::internals::_call_traits<second_type>::const_reference;

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
        class compressed_pair_impl<Ty1, Ty2, 2> : protected type_traits::internals::remove_cv_t<Ty2>::type {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::internals::_call_traits<second_type>::const_reference;

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
        class compressed_pair_impl<Ty1, Ty2, 3> : protected type_traits::internals::remove_cv_t<Ty1>,
                                                  protected type_traits::internals::remove_cv_t<Ty2> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::internals::_call_traits<second_type>::const_reference;

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
        class compressed_pair_impl<Ty1, Ty2, 4> : protected type_traits::internals::remove_cv_t<Ty1> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::internals::_call_traits<second_type>::const_reference;

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
            using first_param_type = typename type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename type_traits::internals::_call_traits<second_type>::const_reference;

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
        : public internals::compressed_pair_impl<
              Ty1, Ty2,
              internals::compressed_pair_switch<Ty1, Ty2,
                                                type_traits::internals::_is_same_v<type_traits::cv_modify::remove_cv_t<Ty1>,
                                                                                   type_traits::cv_modify::remove_cv_t<Ty2>>,
                                                internals::compressed_pair_empty<Ty1>::value,
                                                internals::compressed_pair_empty<Ty2>::value>::value> {
    public:
        using base = internals::compressed_pair_impl<
            Ty1, Ty2,
            internals::compressed_pair_switch<
                Ty1, Ty2,
                type_traits::internals::_is_same_v<type_traits::cv_modify::remove_cv_t<Ty1>, type_traits::cv_modify::remove_cv_t<Ty2>>,
                internals::compressed_pair_empty<Ty1>::value, internals::compressed_pair_empty<Ty2>::value>::value>;
        using base::base;
    };

    template <typename Ty>
    class compressed_pair<Ty, Ty>
        : public internals::compressed_pair_impl<
              Ty, Ty,
              internals::compressed_pair_switch<
                  Ty, Ty,
                  type_traits::internals::_is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, type_traits::cv_modify::remove_cv_t<Ty>>,
                  internals::compressed_pair_empty<Ty>::value, internals::compressed_pair_empty<Ty>::value>::value> {
    public:
        using base = internals::compressed_pair_impl<
            Ty, Ty,
            internals::compressed_pair_switch<
                Ty, Ty,
                type_traits::internals::_is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, type_traits::cv_modify::remove_cv_t<Ty>>,
                internals::compressed_pair_empty<Ty>::value, internals::compressed_pair_empty<Ty>::value>::value>;
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

#endif