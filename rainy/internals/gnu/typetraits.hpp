#ifndef RAINY_CORE_GCC_TYPETRAITS_HPP
#define RAINY_CORE_GCC_TYPETRAITS_HPP

#ifndef RAINY_HAS_INCLUDE_PTR_DECLARE
#include <rainy/internals/core_typetraits.hpp>
#endif

#if RAINY_USING_GCC
/*
此命名空间是预先声明给gnu编译器的，请勿使用
原因是此区域是处于混乱的
*/
namespace rainy::foundation::type_traits::internals::gcc_detail_impl {
    template <bool, typename Ty = void>
    struct enable_if {};

    template <typename Ty>
    struct enable_if<true, Ty> {
        typedef Ty type;
    };

    template <bool Cond, typename Ty = void>
    using _enable_if_t = typename enable_if<Cond, Ty>::type;

    template <typename Ty, typename...>
    using _first_t = Ty;

    template <typename... Bn>
    auto _or_fn(int) -> _first_t<helper::false_type, _enable_if_t<!static_cast<bool>(Bn::value)>...> {
        return 0;
    }

    template <typename... Bn>
    auto _or_fn(...) -> helper::true_type {
        return helper::true_type{};
    }

    template <typename... Bn>
    auto _and_fn(int) -> _first_t<helper::true_type, _enable_if_t<static_cast<bool>(Bn::value)>...> {
        return 0;
    }

    template <typename... Bn>
    auto _and_fn(...) -> helper::false_type {
        return helper::false_type{};
    }

    template <typename... Bn>
    struct _or : decltype(_or_fn<Bn...>(0)) {};

    template <typename... Bn>
    struct _and : decltype(_and_fn<Bn...>(0)) {};

    template <typename Pp>
    struct _not : helper::bool_constant<!static_cast<bool>(Pp::value)> {};

    template <typename Ty>
    struct _is_array_known_bounds;

    template <typename Ty>
    struct _is_array_known_bounds : helper::false_type {};

    template <typename Ty, size_t Size>
    struct _is_array_known_bounds<Ty[Size]> : helper::true_type {};

    template <typename Ty>
    struct _is_array_unknown_bounds : helper::false_type {};

    template <typename Ty>
    struct _is_array_unknown_bounds<Ty[]> : helper::true_type {};

    template <typename Ty>
    struct is_reference : helper::bool_constant<_is_reference_v<Ty>> {};

    template <typename Ty>
    struct is_function : helper::bool_constant<_is_function_v<Ty>> {};

    struct _do_is_destructible_impl {
        template <typename Ty, typename = decltype(utility::declval<Ty &>().~Ty())>
        static helper::true_type _test(int) {
            return helper::true_type{};
        }

        template <typename>
        static helper::false_type _test(...) {
            return helper::false_type{};
        }
    };

    template <typename Ty>
    struct is_void : helper::bool_constant<_is_same_v<void, Ty>> {};

    template <typename Ty>
    struct _is_destructible_impl : _do_is_destructible_impl {
        typedef decltype(_test<Ty>(0)) type;
    };

    template <typename>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v = false;

    template <>
    RAINY_CONSTEXPR_BOOL is_null_pointer_v<std::nullptr_t> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = internals::_is_integral_v<Ty> || _is_floating_point_v<Ty>;

    template <typename Ty>
    struct is_scalar : helper::bool_constant<is_arithmetic_v<Ty> || __is_enum(Ty) || _is_pointer_v<Ty> || _is_member_pointer_v<Ty> ||
                                             is_null_pointer_v<Ty>> {};

    template <typename Ty, bool = _or<is_void<Ty>, _is_array_unknown_bounds<Ty>, is_function<Ty>>::value,
              bool = _or<is_reference<Ty>, is_scalar<Ty>>::value>
    struct _is_destructible_safe;

    template <typename Ty>
    struct _is_destructible_safe<Ty, false, false> : _is_destructible_impl<typename array_modify::remove_all_extents<Ty>::type>::type {};

    template <typename Ty>
    struct _is_destructible_safe<Ty, true, false> : helper:: false_type {};

    template <typename Ty>
    struct _is_destructible_safe<Ty, false, true> : helper:: true_type {};
}
#endif


#endif // RAINY_CORE_GCC_TYPETRAITS_HPP
