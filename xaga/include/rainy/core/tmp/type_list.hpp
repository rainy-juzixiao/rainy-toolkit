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

    // push front/back

    template <typename Ty, typename TypeList>
    struct type_list_push_front {};

    template <typename Ty, typename... Types>
    struct type_list_push_front<Ty, type_list<Types...>> {
        using type = type_list<Ty, Types...>;
    };

    template <typename Ty, typename TypeList>
    using type_list_push_front_t = typename type_list_push_front<Ty,TypeList>::type;

    template <typename Ty, typename TypeList>
    struct type_list_push_back {};

    template <typename Ty>
    struct type_list_push_back<Ty, type_list<>> {
        using type = type_list<Ty>;
    };

    template <typename Ty, typename... Types>
    struct type_list_push_back<Ty, type_list<Types...>> {
        using type = type_list<Types..., Ty>;
    };

    // push front/back all

    template <typename TypeList, typename... PushTypes>
    struct type_list_push_front_all;

    template <typename... Types, typename... PushTypes>
    struct type_list_push_front_all<type_list<Types...>, PushTypes...> {
        using type = type_list<PushTypes..., Types...>;
    };

    template <typename TypeList, typename... PushTypes>
    struct type_list_push_back_all;

    template <typename... Types, typename... PushTypes>
    struct type_list_push_back_all<type_list<Types...>, PushTypes...> {
        using type = type_list<Types..., PushTypes...>;
    };

    // pop front/back

    template <typename TypeList>
    struct type_list_pop_front;

    template <>
    struct type_list_pop_front<type_list<>> {
        using type = type_list<>;
    };

    template <typename Front, typename... Rest>
    struct type_list_pop_front<type_list<Front, Rest...>> {
    public:
        using tail_type = typename type_list_pop_front<type_list<Rest...>>::type;

        using type = typename type_list_push_front<tail_type, Front>::type;
    };

    template <typename List>
    struct type_list_pop_back;

    template <>
    struct type_list_pop_back<type_list<>> {
        using type = type_list<>;
    };

    template <typename Ty>
    struct type_list_pop_back<type_list<Ty>> {
        using type = type_list<>;
    };

    template <typename Head, typename... Tail>
    struct type_list_pop_back<type_list<Head, Tail...>> {
        using type = typename type_list_push_front<typename type_list_pop_back<type_list<Tail...>>::type, Head>::type;
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

    template <class...>
    struct type_list_concat_impl;

    template <class... _Types>
    using type_list_concat = typename type_list_concat_impl<_Types...>::type;

    template <template <typename...> typename type_list>
    struct type_list_concat_impl<type_list<>> {
        using type = type_list<>;
    };

    template <typename... Items1>
    struct type_list_concat_impl<type_list<Items1...>> {
        using type = type_list<Items1...>;
    };

    template <typename... Items1, typename... Items2>
    struct type_list_concat_impl<type_list<Items1...>, type_list<Items2...>> {
        using type = type_list<Items1..., Items2...>;
    };

    template <typename... Items1, typename... Items2, typename... Items3>
    struct type_list_concat_impl<type_list<Items1...>, type_list<Items2...>, type_list<Items3...>> {
        using type = type_list<Items1..., Items2..., Items3...>;
    };

    template <typename... Items1, typename... Items2, typename... Items3, typename... _Rest>
    struct type_list_concat_impl<type_list<Items1...>, type_list<Items2...>, type_list<Items3...>, _Rest...> {
        using type = type_list_concat<type_list<Items1..., Items2..., Items3...>, _Rest...>;
    };

    template <typename TypeList>
    struct type_list_front {};

    template <typename Type, typename... Rest>
    struct type_list_front<type_list<Type, Rest...>> {
        using type = Type;
    };

    template <typename Find, typename TypeList>
    struct type_find_unique {};

    template <typename Find, typename First, typename... Rest>
    struct type_find_unique<Find, type_list<First, Rest...>> {
        static constexpr std::size_t value =
            (count_type_v<Find, type_list<First, Rest...>> != 1)
                ? static_cast<std::size_t>(-1)
                : (type_relations::is_same_v<Find, First> ? 0 : 1 + type_find_unique<Find, type_list<Rest...>>::value);
    };

    template <typename Ty>
    struct type_find_unique<Ty, type_list<>> {
        static constexpr std::size_t value = static_cast<std::size_t>(-1);
    };

    template <typename Ty, typename List>
    struct is_type_in_list;

    template <typename Ty, template <typename...> class List, typename First, typename... Rest>
    struct is_type_in_list<Ty, List<First, Rest...>>
        : other_trans::conditional_t<implements::is_same_v<Ty, First>, std::true_type, is_type_in_list<Ty, List<Rest...>>> {};

    template <typename T, template <typename...> class List>
    struct is_type_in_list<T, List<>> : std::false_type {};

    RAINY_INLINE_CONSTEXPR std::size_t type_list_npos = static_cast<std::size_t>(-1);

    template <class _Void, template <class...> class _Fn, class... _Args>
    struct _Meta_quote_helper_;
    template <template <class...> class _Fn, class... _Args>
    struct _Meta_quote_helper_<void_t<_Fn<_Args...>>, _Fn, _Args...> {
        using type = _Fn<_Args...>;
    };
    template <template <class...> class _Fn>
    struct type_list_quote { // encapsulate a template into a meta-callable type
        template <class... _Types>
        using invoke = typename _Meta_quote_helper_<void, _Fn, _Types...>::type;
    };

    template <typename Fn, typename... Args>
    using type_list_invoke = typename Fn::template invoke<Args...>;

    template <typename Fn, typename... Args>
    struct type_list_bind_back {
        template <typename... Types>
        using invoke = type_list_invoke<Fn, Types..., Args...>;
    };

    template <typename Fn, typename List>
    struct type_list_apply_impl;

    template <typename Fn, typename... Types>
    struct type_list_apply_impl<Fn, type_list<Types...>> {
        using type = type_list_invoke<Fn, Types...>;
    };

    template <typename Fn, typename Ty, Ty... Idxs>
    struct type_list_apply_impl<Fn, helper::integer_sequence<Ty, Idxs...>> {
        using type = type_list_invoke<Fn, helper::integral_constant<Ty, Idxs>...>;
    };

    template <typename Fn, typename List>
    using type_list_apply = typename type_list_apply_impl<Fn, List>::type;

    template <typename TypeList>
    using type_list_join = type_list_apply<type_list_quote<type_list_concat>, TypeList>;

    template <typename Fn, typename List>
    struct type_list_transform_impl {};

    template <typename Fn, typename List>
    using type_list_transform = typename type_list_transform_impl<Fn, List>::type;

    template <typename Fn, typename... Types>
    struct type_list_transform_impl<Fn, type_list<Types...>> {
        using type = type_list<type_list_invoke<Fn, Types>...>;
    };

    template <class _List>
    struct meta_as_integer_sequence_impl;

    template <typename _Ty, _Ty... _Idxs>
    struct meta_as_integer_sequence_impl<type_list<helper::integral_constant<_Ty, _Idxs>...>> {
        using type = helper::integer_sequence<_Ty, _Idxs...>;
    };

    template <class _List>
    struct meta_as_integer_sequence {
        using invoke = typename meta_as_integer_sequence_impl<_List>::type;
    };

    template <typename>
    struct type_list_cartesian_product_impl {};

    template <typename ListOfLists>
    using type_list_cartesian_product = typename type_list_cartesian_product_impl<ListOfLists>::type;

    template <>
    struct type_list_cartesian_product_impl<type_list<>> {
        using type = type_list<>;
    };

    template <typename... Items>
    struct type_list_cartesian_product_impl<type_list<type_list<Items...>>> {
        using type = type_list<type_list<Items>...>;
    };

    template <typename... Items, typename... Lists>
    struct type_list_cartesian_product_impl<type_list<type_list<Items...>, Lists...>> {
        using type = type_list_join<
            type_list<
                type_list_transform<
                    type_list_bind_back<type_list_quote<type_list_push_front>, Items>,
                    type_list_cartesian_product<type_list<Lists...>>
                >...>
        >;
    };

    template <typename ListLike>
    struct as_list_impl {};

    template <template <typename...> typename List, typename... Types>
    struct as_list_impl<List<Types...>> {
        using type = type_list<Types...>;
    };

    template <typename Ty, Ty... Idxs>
    struct as_list_impl<helper::integer_sequence<Ty, Idxs...>> {
        using type = type_list<helper::integral_constant<Ty, Idxs>...>;
    };

    template <class _Ty>
    using as_list = typename as_list_impl<_Ty>::type;

    template <typename T, typename List>
    struct type_list_contains;

    template <typename T>
    struct type_list_contains<T, type_list<>> : std::false_type {};

    template <typename T, typename First, typename... Rest>
    struct type_list_contains<T, type_list<First, Rest...>>
        : std::conditional_t<std::is_same_v<T, First>, std::true_type, type_list_contains<T, type_list<Rest...>>> {};

    template <typename List>
    struct unique_type_list;

    template <>
    struct unique_type_list<type_list<>> {
        using type = type_list<>;
    };

    template <typename T>
    struct unique_type_list<type_list<T>> {
        using type = type_list<T>;
    };

    template <typename First, typename Second, typename... Rest>
    struct unique_type_list<type_list<First, Second, Rest...>> {
    private:
        using rest_unique = typename unique_type_list<type_list<Second, Rest...>>::type;

        template <typename U, typename V>
        struct prepend_if_unique;

        template <typename... Us>
        struct prepend_if_unique<First, type_list<Us...>> {
            using type = conditional_t<(type_relations::is_same_v<First, Us> || ...), type_list<Us...>, type_list<First, Us...>>;
        };

    public:
        using type = typename prepend_if_unique<First, rest_unique>::type;
    };

    template <typename List>
    using unique_type_list_t = typename unique_type_list<List>::type;
}

#endif
