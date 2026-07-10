#ifndef RAINY_META_TYPE_TRAITS_VALUE_LIST_HPP
#define RAINY_META_TYPE_TRAITS_VALUE_LIST_HPP
#include <cstddef> // For std::size_t
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/helper.hpp>

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
    /**
     * @brief A compile-time list of non-type template parameters (values).
     *        非类型模板参数（值）的编译时列表。
     *
     * @tparam Values The values contained in the list
     *                列表中包含的值
     */
    template <auto... Values>
    struct value_list {
        using type = value_list;
        static constexpr std::size_t size = sizeof...(Values);
    };

    /**
     * @brief Retrieves the value at a given index in a value list.
     *        获取值列表中指定索引处的值。
     *
     * @tparam N The index to retrieve
     *           要获取的索引
     * @tparam ValueList The value list to query
     *                   要查询的值列表
     */
    template <std::size_t N, typename ValueList>
    struct value_at;

    /**
     * @brief Specialization that forwards to the implementation.
     *        转发给实现的特化。
     *
     * @tparam N The index to retrieve
     *           要获取的索引
     * @tparam Values The values in the list
     *                列表中的值
     */
    template <std::size_t N, auto... Values>
    struct value_at<N, value_list<Values...>> : implements::value_at_impl<N, Values...> {};

    /**
     * @brief Adds a value to the front of a value list.
     *        在值列表的前面添加一个值。
     *
     * @tparam Value The value to add
     *               要添加的值
     * @tparam ValueList The value list to modify
     *                   要修改的值列表
     */
    template <auto Value, typename ValueList>
    struct value_list_push_front {};

    /**
     * @brief Specialization that performs the front push.
     *        执行前推操作的特化。
     *
     * @tparam Value The value to add
     *               要添加的值
     * @tparam Values The existing values
     *                现有的值
     */
    template <auto Value, auto... Values>
    struct value_list_push_front<Value, value_list<Values...>> {
        using type = value_list<Value, Values...>;
    };

    /**
     * @brief Adds a value to the back of a value list.
     *        在值列表的后面添加一个值。
     *
     * @tparam Value The value to add
     *               要添加的值
     * @tparam ValueList The value list to modify
     *                   要修改的值列表
     */
    template <auto Value, typename ValueList>
    struct value_list_push_back {};

    /**
     * @brief Specialization that performs the back push.
     *        执行后推操作的特化。
     *
     * @tparam Value The value to add
     *               要添加的值
     * @tparam Values The existing values
     *                现有的值
     */
    template <auto Value, auto... Values>
    struct value_list_push_back<Value, value_list<Values...>> {
        using type = value_list<Values..., Value>;
    };

    /**
     * @brief Adds multiple values to the back of a value list.
     *        在值列表的后面添加多个值。
     *
     * @tparam ValueList The value list to modify
     *                   要修改的值列表
     * @tparam PushValues The values to add to the back
     *                    要添加到后面的值
     */
    template <typename ValueList, auto... PushValues>
    struct value_list_push_back_all;

    /**
     * @brief Specialization that performs the multiple back push.
     *        执行多个后推操作的特化。
     *
     * @tparam Values The existing values
     *                现有的值
     * @tparam PushValues The values to add to the back
     *                    要添加到后面的值
     */
    template <auto... Values, auto... PushValues>
    struct value_list_push_back_all<value_list<Values...>, PushValues...> {
        using type = value_list<Values..., PushValues...>;
    };

    /**
     * @brief Adds multiple values to the front of a value list.
     *        在值列表的前面添加多个值。
     *
     * @tparam ValueList The value list to modify
     *                   要修改的值列表
     * @tparam PushValues The values to add to the front
     *                    要添加到前面的值
     */
    template <typename ValueList, auto... PushValues>
    struct value_list_push_front_all;

    /**
     * @brief Specialization that performs the multiple front push.
     *        执行多个前推操作的特化。
     *
     * @tparam Values The existing values
     *                现有的值
     * @tparam PushValues The values to add to the front
     *                    要添加到前面的值
     */
    template <auto... Values, auto... PushValues>
    struct value_list_push_front_all<value_list<Values...>, PushValues...> {
        using type = value_list<PushValues..., Values...>;
    };

    /**
     * @brief Variable template for getting the size of a value list.
     *        获取值列表大小的变量模板。
     *
     * @tparam ValueList The value list
     *                   值列表
     */
    template <typename ValueList>
    RAINY_INLINE_CONSTEXPR std::size_t value_list_size_v = ValueList::size;

    /**
     * @brief Type template for getting the size of a value list.
     *        获取值列表大小的类型模板。
     *
     * @tparam ValueList The value list
     *                   值列表
     */
    template <typename ValueList>
    struct value_list_size : helper::integer_sequence<std::size_t, value_list_size_v<ValueList>> {};

    /**
     * @brief Variable template for counting occurrences of a value in a value list.
     *        计算值在值列表中出现次数的变量模板。
     *
     * @tparam Target The value to count
     *                要计数的值
     * @tparam ValueList The value list to search
     *                   要搜索的值列表
     */
    template <auto Target, typename ValueList>
    RAINY_INLINE_CONSTEXPR std::size_t count_value_v = 0;

    /**
     * @brief Specialization that performs the count.
     *        执行计数的特化。
     *
     * @tparam Target The value to count
     *                要计数的值
     * @tparam Values The values in the list
     *                列表中的值
     */
    template <auto Target, auto... Values>
    RAINY_INLINE_CONSTEXPR std::size_t count_value_v<Target, value_list<Values...>> =
        (0 + ... + (static_cast<std::size_t>(Target == Values ? 1 : 0)));

    /**
     * @brief Type template for counting occurrences of a value in a value list.
     *        计算值在值列表中出现次数的类型模板。
     *
     * @tparam Target The value to count
     *                要计数的值
     * @tparam ValueList The value list to search
     *                   要搜索的值列表
     */
    template <auto Target, typename ValueList>
    struct count_value : helper::integral_constant<std::size_t, count_value_v<Target, ValueList>> {};

    /**
     * @brief Extracts a sub-list starting from a given index.
     *        从给定索引开始提取子列表。
     *
     * @tparam StartIndex The starting index
     *                    起始索引
     * @tparam ValueList The value list to slice
     *                   要切片的值列表
     */
    template <std::size_t StartIndex, typename ValueList>
    struct sub_value_list;

    /**
     * @brief Recursive specialization to skip to the start index.
     *        跳过到起始索引的递归特化。
     *
     * @tparam StartIndex The remaining index
     *                    剩余索引
     * @tparam First The first value (to be skipped)
     *               第一个值（将被跳过）
     * @tparam Rest The remaining values
     *              剩余值
     */
    template <std::size_t StartIndex, auto First, auto... Rest>
    struct sub_value_list<StartIndex, value_list<First, Rest...>> : sub_value_list<StartIndex - 1, value_list<Rest...>> {};

    /**
     * @brief Specialization that returns the sub-list starting from index 0.
     *        返回从索引0开始的子列表的特化。
     *
     * @tparam First The first value
     *               第一个值
     * @tparam Rest The remaining values
     *              剩余值
     */
    template <auto First, auto... Rest>
    struct sub_value_list<0, value_list<First, Rest...>> {
        using type = value_list<First, Rest...>;
    };

    /**
     * @brief Specialization for empty list at index 0.
     *        索引0的空列表的特化。
     */
    template <>
    struct sub_value_list<0, value_list<>> {
        using type = value_list<>;
    };

    /**
     * @brief Specialization when index is beyond list size.
     *        当索引超出列表大小时的特化。
     *
     * @tparam StartIndex The out-of-range index
     *                    超出范围的索引
     */
    template <std::size_t StartIndex>
    struct sub_value_list<StartIndex, value_list<>> {
        using type = value_list<>;
    };
}

#endif
