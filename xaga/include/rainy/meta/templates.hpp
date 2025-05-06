#ifndef RAINY_META_TEMPLATES_HPP
#define RAINY_META_TEMPLATES_HPP
#include <rainy/core/core.hpp>

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
        : other_trans::conditional_t<type_relations::is_same_v<Ty, First>, std::true_type,
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

#endif // RAINY_TYPE_TRAITS_HPP
