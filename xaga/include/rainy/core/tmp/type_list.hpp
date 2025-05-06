#ifndef RAINY_META_TMP_TYPE_LIST_HPP
#define RAINY_META_TMP_TYPE_LIST_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/helper.hpp>

namespace rainy::type_traits::other_trans {
    template <typename... Args>
    struct type_list;

    template <typename First, typename... Rest>
    struct type_list<First, Rest...> {
        using type = First;
    };

    template <>
    struct type_list<> {};

    template <std::size_t N, typename TypeList>
    struct type_at;

    template <std::size_t N, typename First, typename... Rest>
    struct type_at<N, type_list<First, Rest...>> : type_at<N - 1, type_list<Rest...>> {};

    template <typename First, typename... Rest>
    struct type_at<0, type_list<First, Rest...>> {
        using type = First;
    };

    template <typename TupleLike>
    struct tuple_like_to_type_list {
        template <typename UTupleLike, typename List = type_list<>>
        struct impl;

        template <template <typename...> typename UTupleLike, typename First, typename... Rest, typename... Args>
        struct impl<UTupleLike<First, Rest...>, type_list<Args...>> {
            using type = typename impl<UTupleLike<Rest...>, type_list<Args..., First>>::type;
        };

        template <template <typename...> typename UTupleLike, typename... Args>
        struct impl<UTupleLike<>, type_list<Args...>> {
            using type = type_list<Args...>;
        };

        using type = typename impl<TupleLike>::type;
    };

    template <typename TypeList, template <typename...> class TupleLike>
    struct type_list_to_tuple_like;

    template <typename... Types, template <typename...> class TupleLike>
    struct type_list_to_tuple_like<type_list<Types...>, TupleLike> {
        using type = TupleLike<Types...>;
    };

    template <typename TypeList>
    inline constexpr std::size_t type_list_size_v = 0;

    template <>
    inline constexpr std::size_t type_list_size_v<type_list<>> = 0;

    template <typename... Types>
    inline constexpr std::size_t type_list_size_v<type_list<Types...>> = sizeof...(Types);

    template <typename TypeList>
    struct type_list_size : helper::integer_sequence<std::size_t, type_list_size_v<TypeList>> {};
}

#endif