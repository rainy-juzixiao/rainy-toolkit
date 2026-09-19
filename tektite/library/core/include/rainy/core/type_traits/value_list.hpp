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
     * \lang english
     * @brief A compile-time list of non-type template parameters (values).
     *
     * @tparam Values The values contained in the list
     *
     * \lang simp-chinese
     * @brief 非类型模板参数（值）的编译时列表。
     *
     * @tparam Values 列表中包含的值
     */
    template <auto... Values>
    struct value_list {
        using type = value_list;
        static constexpr std::size_t size = sizeof...(Values);
    };

    /**
     * \lang english
     * @brief Retrieves the value at a given index in a value list.
     *
     * @tparam N The index to retrieve
     * @tparam ValueList The value list to query
     *
     * \lang simp-chinese
     * @brief 获取值列表中指定索引处的值。
     *
     * @tparam N 要获取的索引
     * @tparam ValueList 要查询的值列表
     */
    template <std::size_t N, typename ValueList>
    struct value_at;

    /**
     * \lang english
     * @brief Specialization that forwards to the implementation.
     *
     * @tparam N The index to retrieve
     * @tparam Values The values in the list
     *
     * \lang simp-chinese
     * @brief 转发给实现的特化。
     *
     * @tparam N 要获取的索引
     * @tparam Values 列表中的值
     */
    template <std::size_t N, auto... Values>
    struct value_at<N, value_list<Values...>> : implements::value_at_impl<N, Values...> {};

    /**
     * \lang english
     * @brief Adds a value to the front of a value list.
     *
     * @tparam Value The value to add
     * @tparam ValueList The value list to modify
     *
     * \lang simp-chinese
     * @brief 在值列表的前面添加一个值。
     *
     * @tparam Value 要添加的值
     * @tparam ValueList 要修改的值列表
     */
    template <auto Value, typename ValueList>
    struct value_list_push_front {};

    /**
     * \lang english
     * @brief Specialization that performs the front push.
     *
     * @tparam Value The value to add
     * @tparam Values The existing values
     *
     * \lang simp-chinese
     * @brief 执行前推操作的特化。
     *
     * @tparam Value 要添加的值
     * @tparam Values 现有的值
     */
    template <auto Value, auto... Values>
    struct value_list_push_front<Value, value_list<Values...>> {
        using type = value_list<Value, Values...>;
    };

    /**
     * \lang english
     * @brief Adds a value to the back of a value list.
     *
     * @tparam Value The value to add
     * @tparam ValueList The value list to modify
     *
     * \lang simp-chinese
     * @brief 在值列表的后面添加一个值。
     *
     * @tparam Value 要添加的值
     * @tparam ValueList 要修改的值列表
     */
    template <auto Value, typename ValueList>
    struct value_list_push_back {};

    /**
     * \lang english
     * @brief Specialization that performs the back push.
     *
     * @tparam Value The value to add
     * @tparam Values The existing values
     *
     * \lang simp-chinese
     * @brief 执行后推操作的特化。
     *
     * @tparam Value 要添加的值
     * @tparam Values 现有的值
     */
    template <auto Value, auto... Values>
    struct value_list_push_back<Value, value_list<Values...>> {
        using type = value_list<Values..., Value>;
    };

    /**
     * \lang english
     * @brief Adds multiple values to the back of a value list.
     *
     * @tparam ValueList The value list to modify
     * @tparam PushValues The values to add to the back
     *
     * \lang simp-chinese
     * @brief 在值列表的后面添加多个值。
     *
     * @tparam ValueList 要修改的值列表
     * @tparam PushValues 要添加到后面的值
     */
    template <typename ValueList, auto... PushValues>
    struct value_list_push_back_all;

    /**
     * \lang english
     * @brief Specialization that performs the multiple back push.
     *
     * @tparam Values The existing values
     * @tparam PushValues The values to add to the back
     *
     * \lang simp-chinese
     * @brief 执行多个后推操作的特化。
     *
     * @tparam Values 现有的值
     * @tparam PushValues 要添加到后面的值
     */
    template <auto... Values, auto... PushValues>
    struct value_list_push_back_all<value_list<Values...>, PushValues...> {
        using type = value_list<Values..., PushValues...>;
    };

    /**
     * \lang english
     * @brief Adds multiple values to the front of a value list.
     *
     * @tparam ValueList The value list to modify
     * @tparam PushValues The values to add to the front
     *
     * \lang simp-chinese
     * @brief 在值列表的前面添加多个值。
     *
     * @tparam ValueList 要修改的值列表
     * @tparam PushValues 要添加到前面的值
     */
    template <typename ValueList, auto... PushValues>
    struct value_list_push_front_all;

    /**
     * \lang english
     * @brief Specialization that performs the multiple front push.
     *
     * @tparam Values The existing values
     * @tparam PushValues The values to add to the front
     *
     * \lang simp-chinese
     * @brief 执行多个前推操作的特化。
     *
     * @tparam Values 现有的值
     * @tparam PushValues 要添加到前面的值
     */
    template <auto... Values, auto... PushValues>
    struct value_list_push_front_all<value_list<Values...>, PushValues...> {
        using type = value_list<PushValues..., Values...>;
    };

    /**
     * \lang english
     * @brief Variable template for getting the size of a value list.
     *
     * @tparam ValueList The value list
     *
     * \lang simp-chinese
     * @brief 获取值列表大小的变量模板。
     *
     * @tparam ValueList 值列表
     */
    template <typename ValueList>
    RAINY_INLINE_CONSTEXPR std::size_t value_list_size_v = ValueList::size;

    /**
     * \lang english
     * @brief Type template for getting the size of a value list.
     *
     * @tparam ValueList The value list
     *
     * \lang simp-chinese
     * @brief 获取值列表大小的类型模板。
     *
     * @tparam ValueList 值列表
     */
    template <typename ValueList>
    struct value_list_size : helper::integer_sequence<std::size_t, value_list_size_v<ValueList>> {};

    /**
     * \lang english
     * @brief Variable template for counting occurrences of a value in a value list.
     *
     * @tparam Target The value to count
     * @tparam ValueList The value list to search
     *
     * \lang simp-chinese
     * @brief 计算值在值列表中出现次数的变量模板。
     *
     * @tparam Target 要计数的值
     * @tparam ValueList 要搜索的值列表
     */
    template <auto Target, typename ValueList>
    RAINY_INLINE_CONSTEXPR std::size_t count_value_v = 0;

    /**
     * \lang english
     * @brief Specialization that performs the count.
     *
     * @tparam Target The value to count
     * @tparam Values The values in the list
     *
     * \lang simp-chinese
     * @brief 执行计数的特化。
     *
     * @tparam Target 要计数的值
     * @tparam Values 列表中的值
     */
    template <auto Target, auto... Values>
    RAINY_INLINE_CONSTEXPR std::size_t count_value_v<Target, value_list<Values...>> =
        (0 + ... + (static_cast<std::size_t>(Target == Values ? 1 : 0)));

    /**
     * \lang english
     * @brief Type template for counting occurrences of a value in a value list.
     *
     * @tparam Target The value to count
     * @tparam ValueList The value list to search
     *
     * \lang simp-chinese
     * @brief 计算值在值列表中出现次数的类型模板。
     *
     * @tparam Target 要计数的值
     * @tparam ValueList 要搜索的值列表
     */
    template <auto Target, typename ValueList>
    struct count_value : helper::integral_constant<std::size_t, count_value_v<Target, ValueList>> {};

    /**
     * \lang english
     * @brief Extracts a sub-list starting from a given index.
     *
     * @tparam StartIndex The starting index
     * @tparam ValueList The value list to slice
     *
     * \lang simp-chinese
     * @brief 从给定索引开始提取子列表。
     *
     * @tparam StartIndex 起始索引
     * @tparam ValueList 要切片的值列表
     */
    template <std::size_t StartIndex, typename ValueList>
    struct sub_value_list;

    /**
     * \lang english
     * @brief Recursive specialization to skip to the start index.
     *
     * @tparam StartIndex The remaining index
     * @tparam First The first value (to be skipped)
     * @tparam Rest The remaining values
     *
     * \lang simp-chinese
     * @brief 跳过到起始索引的递归特化。
     *
     * @tparam StartIndex 剩余索引
     * @tparam First 第一个值（将被跳过）
     * @tparam Rest 剩余值
     */
    template <std::size_t StartIndex, auto First, auto... Rest>
    struct sub_value_list<StartIndex, value_list<First, Rest...>> : sub_value_list<StartIndex - 1, value_list<Rest...>> {};

    /**
     * \lang english
     * @brief Specialization that returns the sub-list starting from index 0.
     *
     * @tparam First The first value
     * @tparam Rest The remaining values
     *
     * \lang simp-chinese
     * @brief 返回从索引0开始的子列表的特化。
     *
     * @tparam First 第一个值
     * @tparam Rest 剩余值
     */
    template <auto First, auto... Rest>
    struct sub_value_list<0, value_list<First, Rest...>> {
        using type = value_list<First, Rest...>;
    };

    /**
     * \lang english
     * @brief Specialization for empty list at index 0.
     *
     * \lang simp-chinese
     * @brief 索引0的空列表的特化。
     */
    template <>
    struct sub_value_list<0, value_list<>> {
        using type = value_list<>;
    };

    /**
     * \lang english
     * @brief Specialization when index is beyond list size.
     *
     * @tparam StartIndex The out-of-range index
     *
     * \lang simp-chinese
     * @brief 当索引超出列表大小时的特化。
     *
     * @tparam StartIndex 超出范围的索引
     */
    template <std::size_t StartIndex>
    struct sub_value_list<StartIndex, value_list<>> {
        using type = value_list<>;
    };
}

#endif
