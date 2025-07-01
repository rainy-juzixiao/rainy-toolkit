#ifndef RAINY_META_TMP_TYPE_LIST_HPP
#define RAINY_META_TMP_TYPE_LIST_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/helper.hpp>

namespace rainy::type_traits::other_trans {
    template <typename... Types>
    struct type_list;

    template <typename First, typename... Rest>
    struct type_list<First, Rest...> {
        using type = First;
        using next = type_list<Rest...>;
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

    template <typename Ty,typename TypeList>
    struct type_list_push_front {};

    template <typename Ty, typename... Types>
    struct type_list_push_front<Ty, type_list<Types...>> {
        using type = type_list<Ty, Types...>;
    };

    template <typename Ty, typename TypeList>
    struct type_list_push_back {};

    template <typename Ty, typename... Types>
    struct type_list_push_back<Ty, type_list<Types...>> {
        using type = type_list<Types..., Ty>;
    };

    template <typename TypeList, typename... PushTypes>
    struct type_list_push_back_all;

    template <typename... Types, typename... PushTypes>
    struct type_list_push_back_all<type_list<Types...>, PushTypes...> {
        using type = type_list<Types..., PushTypes...>;
    };

    template <typename TypeList, typename... PushTypes>
    struct type_list_push_front_all;

    template <typename... Types, typename... PushTypes>
    struct type_list_push_front_all<type_list<Types...>, PushTypes...> {
        using type = type_list<PushTypes..., Types...>;
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
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v = 0;

    template <>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v<type_list<>> = 0;

    template <typename... Types>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v<type_list<Types...>> = sizeof...(Types);

    template <typename TypeList>
    struct type_list_size : helper::integer_sequence<std::size_t, type_list_size_v<TypeList>> {};

    template <typename Target, typename TypeList>
    RAINY_INLINE_CONSTEXPR std::size_t count_type_v = static_cast<std::size_t>(-1);

    template <typename Target, typename... Args>
    RAINY_INLINE_CONSTEXPR std::size_t count_type_v<Target, type_list<Args...>> =
        (0 + ... + (type_traits::type_relations::is_same_v<Target, Args> ? 1 : 0));

    template <typename Target, typename TypeList>
    struct count_type : helper::integral_constant<std::size_t, count_type_v<Target, TypeList>> {};

    template <std::size_t StartIndex, typename TypeList>
    struct sub_type_list;

    template <std::size_t StartIndex, typename First, typename... Rest>
    struct sub_type_list<StartIndex, type_list<First, Rest...>> : sub_type_list<StartIndex - 1, type_list<Rest...>> {};

    template <typename First, typename... Rest>
    struct sub_type_list<0, type_list<First, Rest...>> {
        using type = type_list<First, Rest...>;
    };

    template <>
    struct sub_type_list<0, type_list<>> {
        using type = type_list<>;
    };

    template <std::size_t StartIndex>
    struct sub_type_list<StartIndex, type_list<>> {
        using type = type_list<>;
    };
}

#endif