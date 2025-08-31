#ifndef RAINY_META_TMP_VALUE_LIST_HPP
#define RAINY_META_TMP_VALUE_LIST_HPP
#include <cstddef> // For std::size_t
#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/helper.hpp>

namespace rainy::type_traits::implements {
    template <std::size_t N, auto... Values>
    struct value_at_impl {
        template <std::size_t Index, auto First, auto... Rest>
        struct impl : impl<Index - 1, Rest...> {};

        template <auto First, auto... Rest>
        struct impl<0, First, Rest...> {
            static constexpr auto value = First;
        };

        static constexpr auto value = impl<N, Values...>::value;
    };
}

namespace rainy::type_traits::other_trans {
    template <auto... Values>
    struct value_list {
        using type = value_list;
        static constexpr std::size_t size = sizeof...(Values);
    };

    template <std::size_t N, typename ValueList>
    struct value_at;

    template <std::size_t N, auto... Values>
    struct value_at<N, value_list<Values...>> : implements::value_at_impl<N, Values...> {};

    template <auto Value, typename ValueList>
    struct value_list_push_front {};

    template <auto Value, auto... Values>
    struct value_list_push_front<Value, value_list<Values...>> {
        using type = value_list<Value, Values...>;
    };

    template <auto Value, typename ValueList>
    struct value_list_push_back {};

    template <auto Value, auto... Values>
    struct value_list_push_back<Value, value_list<Values...>> {
        using type = value_list<Values..., Value>;
    };

    template <typename ValueList, auto... PushValues>
    struct value_list_push_back_all;

    template <auto... Values, auto... PushValues>
    struct value_list_push_back_all<value_list<Values...>, PushValues...> {
        using type = value_list<Values..., PushValues...>;
    };

    template <typename ValueList, auto... PushValues>
    struct value_list_push_front_all;

    template <auto... Values, auto... PushValues>
    struct value_list_push_front_all<value_list<Values...>, PushValues...> {
        using type = value_list<PushValues..., Values...>;
    };

    template <typename ValueList>
    RAINY_INLINE_CONSTEXPR std::size_t value_list_size_v = ValueList::size;

    template <typename ValueList>
    struct value_list_size : helper::integer_sequence<std::size_t, value_list_size_v<ValueList>> {};

    template <auto Target, typename ValueList>
    RAINY_INLINE_CONSTEXPR std::size_t count_value_v = 0;

    template <auto Target, auto... Values>
    RAINY_INLINE_CONSTEXPR std::size_t count_value_v<Target, value_list<Values...>> =
        (0 + ... + (static_cast<std::size_t>(Target == Values ? 1 : 0)));

    template <auto Target, typename ValueList>
    struct count_value : helper::integral_constant<std::size_t, count_value_v<Target, ValueList>> {};

    template <std::size_t StartIndex, typename ValueList>
    struct sub_value_list;

    template <std::size_t StartIndex, auto First, auto... Rest>
    struct sub_value_list<StartIndex, value_list<First, Rest...>> : sub_value_list<StartIndex - 1, value_list<Rest...>> {};

    template <auto First, auto... Rest>
    struct sub_value_list<0, value_list<First, Rest...>> {
        using type = value_list<First, Rest...>;
    };

    template <>
    struct sub_value_list<0, value_list<>> {
        using type = value_list<>;
    };

    template <std::size_t StartIndex>
    struct sub_value_list<StartIndex, value_list<>> {
        using type = value_list<>;
    };

}

#endif
