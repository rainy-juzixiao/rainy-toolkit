#ifndef RAINY_TYPE_TRAITS_HPP
#define RAINY_TYPE_TRAITS_HPP

#ifdef _MSC_VER

#if _MSVC_LANG < 201703L
static_assert(false, "由于使用C++14及以下版本，而库仅支持C++17及以上，请修改你的C++标准版本为C++17及以上再尝试编译！");
#endif

#elif defined __GNUC__ || defined __clang__

#if __cplusplus < 201703L
static_assert(false, "由于使用C++14及以下版本，而库仅支持C++17及以上，请修改你的C++标准版本为C++17及以上再尝试编译！");
#endif // __cplusplus < 201703L

#else // elif defined __GNUC__ || defined __clang__

static_assert(false, "使用了未知的编译器");

#endif // else

#include <rainy/core.hpp>

namespace rainy::type_traits {
    // 类型属性
    namespace type_properties {
        /*
        名称 描述
        is_copy_assignable 测试是否可以将类型的常量引用值分配给该类型。
        is_move_assignable 测试是否可以将类型的右值引用分配给该类型。
        is_swappable
        is_swappable_with
        is_destructible 测试该类型是否易损坏。
        is_trivially_constructible 测试在使用指定类型构造类型时，该类型是否未使用非常用操作。
        is_trivially_default_constructible 测试在构造默认时，该类型是否未使用非常用操作。
        is_trivially_copy_constructible 测试在构造复制时，该类型是否未使用非常用操作。
        is_trivially_move_constructible 测试在构造移动时，该类型是否未使用非常用操作。
        is_trivially_assignable 测试类型是否可赋值，以及赋值是否未使用非常用操作。
        is_trivially_copy_assignable 测试类型是否为复制赋值，以及赋值是否未使用非常用操作。
        is_trivially_move_assignable 测试类型是否为移动赋值，以及赋值是否未使用非常用操作。
        is_trivially_destructible 测试类型是否易损坏，以及析构函数是否未使用非常用操作。
        is_nothrow_default_constructible 测试类型是否是默认构造，以及是否确定在构造默认时不引发。
        is_nothrow_copy_constructible 测试类型是否是复制构造，以及复制构造函数是否确定不引发。
        is_nothrow_assignable 测试类型是否可使用指定类型进行赋值，以及赋值是否确定不引发。
        is_nothrow_copy_assignable 测试类型是否是复制赋值，以及赋值是否确定不引发。
        is_nothrow_move_assignable 测试类型是否是移动赋值，以及赋值是否确定不引发。
        is_nothrow_swappable
        is_nothrow_swappable_with
        is_nothrow_destructible 测试类型是否易损坏，以及析构函数是否确定不引发。
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
        RAINY_CONSTEXPR_BOOL is_signed_v = internals::sign_base<Ty>::_signed;

        template <typename Ty>
        struct is_signed : helper::bool_constant<is_signed_v<Ty>> {};

        template <typename Ty>
        RAINY_CONSTEXPR_BOOL is_unsigned_v = internals::sign_base<Ty>::_unsigned;

        template <typename Ty>
        struct is_unsigned : helper::bool_constant<is_unsigned_v<Ty>> {};

        template <typename Ty, typename... Args>
        RAINY_CONSTEXPR_BOOL is_constructible_v = __is_constructible(Ty, Args...);

        template <typename Ty, typename... Args>
        struct is_constructible : helper::bool_constant<is_constructible_v<Ty>> {};

        template <typename Ty>
        RAINY_CONSTEXPR_BOOL is_copy_constructible_v = __is_constructible(Ty, reference_modify::add_lvalue_reference_t<const Ty>);

        template <typename Ty>
        RAINY_CONSTEXPR_BOOL is_default_constructible_v = __is_constructible(Ty);

        template <typename Ty>
        RAINY_CONSTEXPR_BOOL is_move_constructible_v = __is_constructible(Ty, Ty);

        template <typename callable, typename Ty1, typename removed_cvref = cv_modify::remove_cvref_t<callable>,
                  bool is_pmf = primary_types::is_member_function_pointer_v<removed_cvref>,
                  bool is_pmd = primary_types::is_member_object_pointer_v<removed_cvref>>
        struct invoker;

        template <typename callable, typename Ty1, typename removed_cvref>
        struct invoker<callable, Ty1, removed_cvref, true, false>
            : other_transformations::conditional_t<
                  std::is_same_v<typename std::is_member_function_pointer<removed_cvref>::type, cv_modify::remove_cvref_t<Ty1>> ||
                      std::is_base_of_v<typename std::is_member_function_pointer<removed_cvref>::type, cv_modify::remove_cvref_t<Ty1>>,
                  internals::invoker_pmf_object,
                  other_transformations::conditional_t<
                      primary_types::is_specialization_v<cv_modify::remove_cvref_t<Ty1>, std::reference_wrapper>,
                      internals::invoker_pmf_refwrap, internals::invoker_pmf_pointer>> {}; // pointer to member function

        template <typename callable, typename Ty1, typename removed_cvref>
        struct invoker<callable, Ty1, removed_cvref, false, true>
            : std::conditional_t<type_relations::is_same_v<typename std::is_member_object_pointer<removed_cvref>::type,
                                                           cv_modify::remove_cvref_t<Ty1>> ||
                                     type_relations::is_base_of_v<typename std::is_member_object_pointer<removed_cvref>::type,
                                                                  cv_modify::remove_cvref_t<Ty1>>,
                                 internals::invoker_pmd_object,
                                 other_transformations::conditional_t<
                                     primary_types::is_specialization_v<cv_modify::remove_cvref_t<Ty1>, std::reference_wrapper>,
                                     internals::invoker_pmd_refwrap, internals::invoker_pmd_pointer>> {}; // pointer to member data

        template <typename callable, typename Ty1, typename removed_cvref>
        struct invoker<callable, Ty1, removed_cvref, false, false> : internals::invoker_functor {};

        template <typename from, typename to>
        struct is_invoke_nothrow_convertible
            : helper::bool_constant<noexcept(internals::_fake_copy_init<to>(internals::_returns_exactly<from>()))> {};

        template <typename Rx, bool no_throw>
        struct invoke_traits_common {
            using type = Rx;
            using is_invocable = helper::true_type;
            using is_nothrow_invocable = helper::bool_constant<no_throw>;
            template <typename Rx_>
            using is_invocable_r =
                helper::bool_constant<logical_traits::disjunction_v<primary_types::is_void<Rx>, is_invoke_convertible<type, Rx>>>;
            template <typename Rx_>
            using is_nothrow_invocable_r = helper::bool_constant<logical_traits::conjunction_v<
                is_nothrow_invocable,
                logical_traits::disjunction<
                    primary_types::is_void<Rx>,
                    logical_traits::conjunction<is_invoke_convertible<type, Rx>, is_invoke_nothrow_convertible<type, Rx>>>>>;
        };

        template <typename _void, typename callable>
        struct invoke_traits_zero {
            // selected when callable isn't callable with zero _Args
            using is_invocable = helper::false_type;
            using is_nothrow_invocable = helper::false_type;
            template <typename Rx>
            using is_invocable_r = helper::false_type;
            template <typename Rx>
            using is_nothrow_invocable_r = helper::false_type;
            using is_void_ = primary_types::is_void<_void>;
            using callable_type_ = callable;
        };

        template <typename callable>
        using _decltype_invoke_zero = decltype(std::declval<callable>()());

        template <typename callable>
        struct invoke_traits_zero<other_transformations::void_t<_decltype_invoke_zero<callable>>, callable>
            : invoke_traits_common<_decltype_invoke_zero<callable>, noexcept(std::declval<callable>()())> {};

        template <typename _void, typename... Args>
        struct invoke_traits_nonzero {
            using is_invocable = helper::false_type;
            using is_nothrow_invocable = helper::false_type;

            template <typename Rx>
            using is_invocable_r = helper::false_type;

            template <typename Rx>
            using is_nothrow_invocable_r = helper::false_type;

            using is_void_ = primary_types::is_void<_void>;
        };

        template <typename callable, typename Ty1, typename... Args2>
        using _decltype_invoke_nonzero =
            decltype(invoker<callable, Ty1>::invoke(std::declval<callable>(), std::declval<Ty1>(), std::declval<Args2>()...));

        template <typename callable, typename Ty1, typename... Args2>
        struct invoke_traits_nonzero<other_transformations::void_t<_decltype_invoke_nonzero<callable, Ty1, Args2...>>, callable, Ty1,
                                     Args2...>
            : invoke_traits_common<_decltype_invoke_nonzero<callable, Ty1, Args2...>,
                                   noexcept(invoker<callable, Ty1>::invoke(std::declval<callable>(), std::declval<Ty1>(),
                                                                           std::declval<Args2>()...))> {};

        template <typename callable, typename... Args>
        using select_invoke_traits = other_transformations::conditional_t<sizeof...(Args) == 0, invoke_traits_zero<void, callable>,
                                                                          invoke_traits_nonzero<void, callable, Args...>>;

        template <typename callable, typename... Args>
        using _invoke_rx_t = typename select_invoke_traits<callable, Args...>::type;

        template <typename Rx, typename callable, typename... Args>
        using _is_invocable_r_ = typename select_invoke_traits<callable, Args...>::template is_invocable_r<Rx>;

        template <typename Rx, typename callable, typename... Args>
        struct _is_invocable_r : _is_invocable_r_<Rx, callable, Args...> {};

        template <typename callable, typename... Args>
        struct invoke_result : select_invoke_traits<callable, Args...> {};

        template <typename callable, typename... Args>
        using invoke_result_t = typename select_invoke_traits<callable, Args...>::type;

        template <typename callable, typename... Args>
        struct is_nothrow_invocable : select_invoke_traits<callable, Args...>::is_nothrow_invocable {};

        template <typename Rx, typename callable, typename... Args>
        struct is_nothrow_invocable_r : select_invoke_traits<callable, Args...>::template is_nothrow_invocable_r<Rx> {};

        template <typename callable, typename... Args>
        RAINY_CONSTEXPR_BOOL is_nothrow_invocable_v = select_invoke_traits<callable, Args...>::is_nothrow_invocable::value;

        template <typename Rx, typename callable, typename... Args>
        RAINY_CONSTEXPR_BOOL is_nothrow_invocable_r_v =
            select_invoke_traits<callable, Args...>::template is_nothrow_invocable_r<Rx>::value;

        template <typename to, typename from>
        RAINY_CONSTEXPR_BOOL is_assignable_v = __is_assignable(to, from);

        template <typename to, typename from>
        struct is_assignable : helper::bool_constant<is_assignable_v<to, from>> {};

        template <typename Ty>
        RAINY_CONSTEXPR_BOOL is_nothrow_move_constructible_v = __is_nothrow_constructible(Ty, Ty);

        template <typename Ty, typename... Args>
        RAINY_CONSTEXPR_BOOL is_nothrow_constructible_v = __is_nothrow_constructible(Ty, Args...);

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
        RAINY_CONSTEXPR_BOOL is_nothrow_assignable_v = internals::_is_nothrow_assignable_v<to, from>;

        template <typename to, typename from>
        struct is_nothrow_assignable : helper::bool_constant<is_nothrow_assignable_v<to, from>> {};
    }

    // 复合类型
    namespace composite_types {
        template <typename Ty>
        RAINY_CONSTEXPR_BOOL is_arithmetic_v = internals::_is_arithmetic_v<Ty>;

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
        RAINY_CONSTEXPR_BOOL is_reference_v = internals::_is_reference_v<Ty>;

        template <typename Ty>
        struct is_reference : helper::bool_constant<internals::_is_reference_v<Ty>> {};

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
}

/* 元组及可变体Trait */
namespace rainy::type_traits::extras::tuple_like {
    template <typename...>
    struct meta_list;

    template <typename List>
    struct meta_front_impl;

    template <template <typename...> typename List, typename First, typename... Rest>
    struct meta_front_impl<List<First, Rest...>> {
        using type = First;
    };

    template <typename List>
    using meta_front = typename meta_front_impl<List>::type;

    template <typename Ty, typename List>
    struct meta_count;

    template <typename Ty, template <typename...> typename List, typename First, typename... Rest>
    struct meta_count<Ty, List<First, Rest...>> {
        static constexpr std::size_t value = (std::is_same_v<Ty, First> ? 1 : 0) + meta_count<Ty, List<Rest...>>::value;
    };

    template <typename Ty, template <typename...> typename List>
    struct meta_count<Ty, List<>> {
        static constexpr std::size_t value = 0;
    };

    template <typename List>
    struct meta_pop_front_impl;

    template <template <typename...> typename List, typename First, typename... Rest>
    struct meta_pop_front_impl<List<First, Rest...>> {
        using type = List<Rest...>;
    };

    template <typename List>
    using meta_pop_front = typename meta_pop_front_impl<List>::type;

    template <typename Ty, typename List>
    struct meta_find_unique_index;

    template <typename Ty, template <typename...> typename List, typename First, typename... Rest>
    struct meta_find_unique_index<Ty, List<First, Rest...>> {
        static constexpr std::size_t value =
            (meta_count<Ty, List<First, Rest...>>::value != 1)
                ? static_cast<std::size_t>(-1)
                : (type_relations::is_same_v<Ty, First> ? 0 : 1 + meta_find_unique_index<Ty, List<Rest...>>::value);
    };

    template <typename Ty, template <typename...> typename List>
    struct meta_find_unique_index<Ty, List<>> {
        static constexpr std::size_t value = static_cast<std::size_t>(-1);
    };

    template <typename Ty, typename List>
    struct is_type_in_list;

    template <typename Ty, template <typename...> class List, typename First, typename... Rest>
    struct is_type_in_list<Ty, List<First, Rest...>>
        : other_transformations::conditional_t<type_relations::is_same_v<Ty, First>, std::true_type,
                                               is_type_in_list<Ty, List<Rest...>>> {};

    template <typename T, template <typename...> class List>
    struct is_type_in_list<T, List<>> : std::false_type {};

    template <std::size_t Index, typename List>
    struct meta_at_c;

    template <std::size_t Index, template <typename...> class List, typename First, typename... Rest>
    struct meta_at_c<Index, List<First, Rest...>> {
        using type = typename meta_at_c<Index - 1, List<Rest...>>::type;
    };

    template <template <typename...> class List, typename First, typename... Rest>
    struct meta_at_c<0, List<First, Rest...>> {
        using type = First;
    };

    template <std::size_t Index, typename List>
    using meta_at_c_t = typename meta_at_c<Index, List>::type;

    template <typename T, typename List>
    constexpr bool is_type_in_list_v = is_type_in_list<T, List>::value;

    template <class>
    constexpr bool is_in_placeholder_index_specialization = false;

    template <size_t Idx>
    constexpr bool is_in_placeholder_index_specialization<utility::placeholder_index_t<Idx>> = true;
}

#if RAINY_HAS_CXX20
/*
 * 追加概念库，需要启用C++20
 */
namespace rainy::type_traits::concepts {}
#endif // RAINY_HAS_CXX20

#if RAINY_USING_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace rainy::type_traits::extras::winapi {
    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_support_charset_v =
        rainy::type_traits::type_relations::is_any_of_v<CharType, CHAR, LPSTR, LPCSTR, LPCTSTR, WCHAR, LPWSTR, LPCWSTR, TCHAR,
                                                        const TCHAR *>;

    template <typename CharType>
    struct is_support_charset : rainy::type_traits::helper::bool_constant<is_support_charset_v<CharType>> {};

    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_support_char_v = rainy::type_traits::type_relations::is_any_of_v<CharType, char, wchar_t>;
}
#endif

namespace rainy::foundation::reflection::type_traits {
    template <typename Class, typename Base = void>
    RAINY_CONSTEXPR_BOOL support_type = (std::is_enum_v<Class> || std::is_abstract_v<Class> || std::is_class_v<Class> ||
                                         std::is_union_v<Class>) &&!std::is_same_v<Class, Base>;
}

#endif // RAINY_TYPE_TRAITS_HPP
