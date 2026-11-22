#ifndef RAINY_META_TYPE_TRAITS_TYPE_LIST_HPP
#define RAINY_META_TYPE_TRAITS_TYPE_LIST_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/helper.hpp>

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief A compile-time list of types.
     *
     * @tparam Types The types contained in the list
     *
     * \lang simp-chinese
     * @brief 编译时类型列表。
     *
     * @tparam Types 列表中包含的类型
     */
    template <typename... Types>
    struct type_list;

    /**
     * \lang english
     * @brief Specialization for non-empty type lists.
     *
     * @tparam First The first type in the list
     * @tparam Rest The remaining types in the list
     *
     * \lang simp-chinese
     * @brief 非空类型列表的特化。
     *
     * @tparam First 列表中的第一个类型
     * @tparam Rest 列表中剩余的类型
     */
    template <typename First, typename... Rest>
    struct type_list<First, Rest...> {
        /**
         * \lang english
         * @brief The first type in the list.
         *
         * \lang simp-chinese
         * @brief 列表中的第一个类型。
         */
        using type = First;

        /**
         * \lang english
         * @brief The remaining types in the list as a nested type_list.
         *
         * \lang simp-chinese
         * @brief 列表中剩余的类型作为嵌套的 type_list。
         */
        using next = type_list<Rest...>;
    };

    /**
     * \lang english
     * @brief Specialization for empty type lists.
     *
     * \lang simp-chinese
     * @brief 空类型列表的特化。
     */
    template <>
    struct type_list<> {};
}

namespace rainy::type_traits::implements {
    template <typename...>
    struct type_list_concat_impl;
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Concatenates multiple type lists into a single type list.
     *
     *  This alias template provides a way to combine multiple type_list
     *  instantiations into one unified type list containing all types
     *  from the input lists in order.
     *
     * @tparam Types The type lists to concatenate
     * @return A single type_list containing all types from the input lists
     *
     * \lang simp-chinese
     * @brief 将多个类型列表连接成一个类型列表。
     *
     *  此别名模板提供了一种将多个 type_list 实例合并为一个统一类型列表的方法，
     *  该列表按顺序包含所有输入列表中的类型。
     *
     * @tparam Types 要连接的类型列表
     * @return 包含所有输入列表中类型的单个 type_list
     */
    template <typename... Types>
    using type_list_concat = typename implements::type_list_concat_impl<Types...>::type;
}

namespace rainy::type_traits::implements {
    template <template <typename...> typename type_list>
    struct type_list_concat_impl<type_list<>> {
        using type = type_list<>;
    };

    template <typename... Items1>
    struct type_list_concat_impl<other_trans::type_list<Items1...>> {
        using type = other_trans::type_list<Items1...>;
    };

    template <typename... Items1, typename... Items2>
    struct type_list_concat_impl<other_trans::type_list<Items1...>, other_trans::type_list<Items2...>> {
        using type = other_trans::type_list<Items1..., Items2...>;
    };

    template <typename... Items1, typename... Items2, typename... Items3>
    struct type_list_concat_impl<other_trans::type_list<Items1...>, other_trans::type_list<Items2...>,
                                 other_trans::type_list<Items3...>> {
        using type = other_trans::type_list<Items1..., Items2..., Items3...>;
    };

    template <typename... Items1, typename... Items2, typename... Items3, typename... Rest>
    struct type_list_concat_impl<other_trans::type_list<Items1...>, other_trans::type_list<Items2...>,
                                 other_trans::type_list<Items3...>, Rest...> {
        using type = other_trans::type_list_concat<other_trans::type_list<Items1..., Items2..., Items3...>, Rest...>;
    };
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Retrieves the type at a given index in a type list.
     *
     * @tparam N The index to retrieve
     * @tparam TypeList The type list to query
     *
     * \lang simp-chinese
     * @brief 获取类型列表中指定索引处的类型。
     *
     * @tparam N 要获取的索引
     * @tparam TypeList 要查询的类型列表
     */
    template <std::size_t N, typename TypeList>
    struct type_at;

    /**
     * \lang english
     * @brief Recursive specialization for non-zero indices.
     *
     * @tparam N The remaining index
     * @tparam First The first type in the list (ignored)
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 非零索引的递归特化。
     *
     * @tparam N 剩余索引
     * @tparam First 列表中的第一个类型（忽略）
     * @tparam Rest 剩余类型
     */
    template <std::size_t N, typename First, typename... Rest>
    struct type_at<N, type_list<First, Rest...>> : type_at<N - 1, type_list<Rest...>> {};

    /**
     * \lang english
     * @brief Specialization for index 0, returns the first type.
     *
     * @tparam First The first type
     * @tparam Rest The remaining types (ignored)
     *
     * \lang simp-chinese
     * @brief 索引0的特化，返回第一个类型。
     *
     * @tparam First 第一个类型
     * @tparam Rest 剩余类型（忽略）
     */
    template <typename First, typename... Rest>
    struct type_at<0, type_list<First, Rest...>> {
        using type = First;
    };

    /**
     * \lang english
     * @brief Alias template for type_at.
     *
     * @tparam N The index to retrieve
     * @tparam TypeList The type list to query
     *
     * \lang simp-chinese
     * @brief type_at 的别名模板。
     *
     * @tparam N 要获取的索引
     * @tparam TypeList 要查询的类型列表
     */
    template <std::size_t N, typename TypeList>
    using type_at_t = typename type_at<N, TypeList>::type;

    /**
     * \lang english
     * @brief Adds a type to the front of a type list.
     *
     * @tparam Ty The type to add
     * @tparam TypeList The type list to modify
     *
     * \lang simp-chinese
     * @brief 在类型列表的前面添加一个类型。
     *
     * @tparam Ty 要添加的类型
     * @tparam TypeList 要修改的类型列表
     */
    template <typename Ty, typename TypeList>
    struct type_list_push_front {};

    /**
     * \lang english
     * @brief Specialization that performs the front push.
     *
     * @tparam Ty The type to add
     * @tparam Types The existing types
     *
     * \lang simp-chinese
     * @brief 执行前推操作的特化。
     *
     * @tparam Ty 要添加的类型
     * @tparam Types 现有的类型
     */
    template <typename Ty, typename... Types>
    struct type_list_push_front<Ty, type_list<Types...>> {
        using type = type_list<Ty, Types...>;
    };

    /**
     * \lang english
     * @brief Alias template for type_list_push_front.
     *
     * @tparam Ty The type to add
     * @tparam TypeList The type list to modify
     *
     * \lang simp-chinese
     * @brief type_list_push_front 的别名模板。
     *
     * @tparam Ty 要添加的类型
     * @tparam TypeList 要修改的类型列表
     */
    template <typename Ty, typename TypeList>
    using type_list_push_front_t = typename type_list_push_front<Ty, TypeList>::type;

    /**
     * \lang english
     * @brief Adds a type to the back of a type list.
     *
     * @tparam Ty The type to add
     * @tparam TypeList The type list to modify
     *
     * \lang simp-chinese
     * @brief 在类型列表的后面添加一个类型。
     *
     * @tparam Ty 要添加的类型
     * @tparam TypeList 要修改的类型列表
     */
    template <typename Ty, typename TypeList>
    struct type_list_push_back {};

    /**
     * \lang english
     * @brief Specialization for empty list.
     *
     * @tparam Ty The type to add
     *
     * \lang simp-chinese
     * @brief 空列表的特化。
     *
     * @tparam Ty 要添加的类型
     */
    template <typename Ty>
    struct type_list_push_back<Ty, type_list<>> {
        using type = type_list<Ty>;
    };

    /**
     * \lang english
     * @brief Specialization that performs the back push.
     *
     * @tparam Ty The type to add
     * @tparam Types The existing types
     *
     * \lang simp-chinese
     * @brief 执行后推操作的特化。
     *
     * @tparam Ty 要添加的类型
     * @tparam Types 现有的类型
     */
    template <typename Ty, typename... Types>
    struct type_list_push_back<Ty, type_list<Types...>> {
        using type = type_list<Types..., Ty>;
    };

    /**
     * \lang english
     * @brief Adds multiple types to the front of a type list.
     *
     * @tparam TypeList The type list to modify
     * @tparam PushTypes The types to add to the front
     *
     * \lang simp-chinese
     * @brief 在类型列表的前面添加多个类型。
     *
     * @tparam TypeList 要修改的类型列表
     * @tparam PushTypes 要添加到前面的类型
     */
    template <typename TypeList, typename... PushTypes>
    struct type_list_push_front_all;

    /**
     * \lang english
     * @brief Specialization that performs the multiple front push.
     *
     * @tparam Types The existing types
     * @tparam PushTypes The types to add to the front
     *
     * \lang simp-chinese
     * @brief 执行多个前推操作的特化。
     *
     * @tparam Types 现有的类型
     * @tparam PushTypes 要添加到前面的类型
     */
    template <typename... Types, typename... PushTypes>
    struct type_list_push_front_all<type_list<Types...>, PushTypes...> {
        using type = type_list<PushTypes..., Types...>;
    };

    /**
     * \lang english
     * @brief Adds multiple types to the back of a type list.
     *
     * @tparam TypeList The type list to modify
     * @tparam PushTypes The types to add to the back
     *
     * \lang simp-chinese
     * @brief 在类型列表的后面添加多个类型。
     *
     * @tparam TypeList 要修改的类型列表
     * @tparam PushTypes 要添加到后面的类型
     */
    template <typename TypeList, typename... PushTypes>
    struct type_list_push_back_all;

    /**
     * \lang english
     * @brief Specialization that performs the multiple back push.
     *
     * @tparam Types The existing types
     * @tparam PushTypes The types to add to the back
     *
     * \lang simp-chinese
     * @brief 执行多个后推操作的特化。
     *
     * @tparam Types 现有的类型
     * @tparam PushTypes 要添加到后面的类型
     */
    template <typename... Types, typename... PushTypes>
    struct type_list_push_back_all<type_list<Types...>, PushTypes...> {
        using type = type_list<Types..., PushTypes...>;
    };

    /**
     * \lang english
     * @brief Removes the first type from a type list.
     *
     * @tparam TypeList The type list to modify
     *
     * \lang simp-chinese
     * @brief 从类型列表中移除第一个类型。
     *
     * @tparam TypeList 要修改的类型列表
     */
    template <typename TypeList>
    struct type_list_pop_front;

    /**
     * \lang english
     * @brief Specialization for empty list.
     *
     * \lang simp-chinese
     * @brief 空列表的特化。
     */
    template <>
    struct type_list_pop_front<type_list<>> {
        using type = type_list<>;
    };

    /**
     * \lang english
     * @brief Recursive specialization that removes the first type.
     *
     * @tparam Front The first type (to be removed)
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 移除第一个类型的递归特化。
     *
     * @tparam Front 第一个类型（将被移除）
     * @tparam Rest 剩余类型
     */
    template <typename Front, typename... Rest>
    struct type_list_pop_front<type_list<Front, Rest...>> {
        using type = type_list<Rest...>;
    };

    /**
     * \lang english
     * @brief Removes the last type from a type list.
     *
     * @tparam List The type list to modify
     *
     * \lang simp-chinese
     * @brief 从类型列表中移除最后一个类型。
     *
     * @tparam List 要修改的类型列表
     */
    template <typename List>
    struct type_list_pop_back;

    /**
     * \lang english
     * @brief Specialization for empty list.
     *
     * \lang simp-chinese
     * @brief 空列表的特化。
     */
    template <>
    struct type_list_pop_back<type_list<>> {
        using type = type_list<>;
    };

    /**
     * \lang english
     * @brief Specialization for list with one element.
     *
     * @tparam Ty The single type (to be removed)
     *
     * \lang simp-chinese
     * @brief 只有一个元素的列表的特化。
     *
     * @tparam Ty 单一类型（将被移除）
     */
    template <typename Ty>
    struct type_list_pop_back<type_list<Ty>> {
        using type = type_list<>;
    };

    /**
     * \lang english
     * @brief Recursive specialization that removes the last type.
     *
     * @tparam Head The first type
     * @tparam Tail The remaining types
     *
     * \lang simp-chinese
     * @brief 移除最后一个类型的递归特化。
     *
     * @tparam Head 第一个类型
     * @tparam Tail 剩余类型
     */
    template <typename Head, typename... Tail>
    struct type_list_pop_back<type_list<Head, Tail...>> {
        using type = typename type_list_push_front<Head, typename type_list_pop_back<type_list<Tail...>>::type>::type;
    };

    /**
     * \lang english
     * @brief Converts a tuple-like type to a type_list.
     *
     * @tparam TupleLike The tuple-like type to convert
     *
     * \lang simp-chinese
     * @brief 将类似tuple的类型转换为type_list。
     *
     * @tparam TupleLike 要转换的类似tuple的类型
     */
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

    /**
     * \lang english
     * @brief Converts a type_list to a tuple-like type.
     *
     * @tparam TypeList The type list to convert
     * @tparam TupleLike The tuple-like template to instantiate
     *
     * \lang simp-chinese
     * @brief 将type_list转换为类似tuple的类型。
     *
     * @tparam TypeList 要转换的类型列表
     * @tparam TupleLike 要实例化的类似tuple的模板
     */
    template <typename TypeList, template <typename...> class TupleLike>
    struct type_list_to_tuple_like;

    /**
     * \lang english
     * @brief Specialization that performs the conversion.
     *
     * @tparam Types The types in the list
     * @tparam TupleLike The tuple-like template
     *
     * \lang simp-chinese
     * @brief 执行转换操作的特化。
     *
     * @tparam Types 列表中的类型
     * @tparam TupleLike 类似tuple的模板
     */
    template <typename... Types, template <typename...> class TupleLike>
    struct type_list_to_tuple_like<type_list<Types...>, TupleLike> {
        using type = TupleLike<Types...>;
    };

    /**
     * \lang english
     * @brief Variable template for getting the size of a type list.
     *
     * @tparam TypeList The type list
     *
     * \lang simp-chinese
     * @brief 获取类型列表大小的变量模板。
     *
     * @tparam TypeList 类型列表
     */
    template <typename TypeList>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v = 0;

    /**
     * \lang english
     * @brief Specialization for empty list.
     *
     * \lang simp-chinese
     * @brief 空列表的特化。
     */
    template <>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v<type_list<>> = 0;

    /**
     * \lang english
     * @brief Specialization for non-empty lists.
     *
     * @tparam Types The types in the list
     *
     * \lang simp-chinese
     * @brief 非空列表的特化。
     *
     * @tparam Types 列表中的类型
     */
    template <typename... Types>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v<type_list<Types...>> = sizeof...(Types);

    /**
     * \lang english
     * @brief Type template for getting the size of a type list.
     *
     * @tparam TypeList The type list
     *
     * \lang simp-chinese
     * @brief 获取类型列表大小的类型模板。
     *
     * @tparam TypeList 类型列表
     */
    template <typename TypeList>
    struct type_list_size : helper::integer_sequence<std::size_t, type_list_size_v<TypeList>> {};

    /**
     * \lang english
     * @brief Variable template for counting occurrences of a type in a type list.
     *
     * @tparam Target The type to count
     * @tparam TypeList The type list to search
     *
     * \lang simp-chinese
     * @brief 计算类型在类型列表中出现的次数的变量模板。
     *
     * @tparam Target 要计数的类型
     * @tparam TypeList 要搜索的类型列表
     */
    template <typename Target, typename TypeList>
    RAINY_INLINE_CONSTEXPR std::size_t count_type_v = static_cast<std::size_t>(-1);

    /**
     * \lang english
     * @brief Specialization that performs the count.
     *
     * @tparam Target The type to count
     * @tparam Args The types in the list
     *
     * \lang simp-chinese
     * @brief 执行计数的特化。
     *
     * @tparam Target 要计数的类型
     * @tparam Args 列表中的类型
     */
    template <typename Target, typename... Args>
    RAINY_INLINE_CONSTEXPR std::size_t count_type_v<Target, type_list<Args...>> =
        (0 + ... + (type_traits::type_relations::is_same_v<Target, Args> ? 1 : 0));

    /**
     * \lang english
     * @brief Type template for counting occurrences of a type in a type list.
     *
     * @tparam Target The type to count
     * @tparam TypeList The type list to search
     *
     * \lang simp-chinese
     * @brief 计算类型在类型列表中出现的次数的类型模板。
     *
     * @tparam Target 要计数的类型
     * @tparam TypeList 要搜索的类型列表
     */
    template <typename Target, typename TypeList>
    struct count_type : helper::integral_constant<std::size_t, count_type_v<Target, TypeList>> {};

    /**
     * \lang english
     * @brief Extracts a sub-list starting from a given index.
     *
     * @tparam StartIndex The starting index
     * @tparam TypeList The type list to slice
     *
     * \lang simp-chinese
     * @brief 从给定索引开始提取子列表。
     *
     * @tparam StartIndex 起始索引
     * @tparam TypeList 要切片的类型列表
     */
    template <std::size_t StartIndex, typename TypeList>
    struct sub_type_list;

    /**
     * \lang english
     * @brief Recursive specialization to skip to the start index.
     *
     * @tparam StartIndex The remaining index
     * @tparam First The first type (to be skipped)
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 跳过到起始索引的递归特化。
     *
     * @tparam StartIndex 剩余索引
     * @tparam First 第一个类型（将被跳过）
     * @tparam Rest 剩余类型
     */
    template <std::size_t StartIndex, typename First, typename... Rest>
    struct sub_type_list<StartIndex, type_list<First, Rest...>> : sub_type_list<StartIndex - 1, type_list<Rest...>> {};

    /**
     * \lang english
     * @brief Specialization that returns the sub-list starting from index 0.
     *
     * @tparam First The first type
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 返回从索引0开始的子列表的特化。
     *
     * @tparam First 第一个类型
     * @tparam Rest 剩余类型
     */
    template <typename First, typename... Rest>
    struct sub_type_list<0, type_list<First, Rest...>> {
        using type = type_list<First, Rest...>;
    };

    /**
     * \lang english
     * @brief Specialization for empty list at index 0.
     *
     * \lang simp-chinese
     * @brief 索引0的空列表的特化。
     */
    template <>
    struct sub_type_list<0, type_list<>> {
        using type = type_list<>;
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
    struct sub_type_list<StartIndex, type_list<>> {
        using type = type_list<>;
    };

    /**
     * \lang english
     * @brief Gets the first type in a type list.
     *
     * @tparam TypeList The type list to query
     *
     * \lang simp-chinese
     * @brief 获取类型列表中的第一个类型。
     *
     * @tparam TypeList 要查询的类型列表
     */
    template <typename TypeList>
    struct type_list_front {};

    /**
     * \lang english
     * @brief Specialization that returns the first type.
     *
     * @tparam Type The first type
     * @tparam Rest The remaining types (ignored)
     *
     * \lang simp-chinese
     * @brief 返回第一个类型的特化。
     *
     * @tparam Type 第一个类型
     * @tparam Rest 剩余类型（忽略）
     */
    template <typename Type, typename... Rest>
    struct type_list_front<type_list<Type, Rest...>> {
        using type = Type;
    };

    /**
     * \lang english
     * @brief Finds the index of a unique type in a type list.
     *
     * @tparam Find The type to find
     * @tparam TypeList The type list to search
     *
     * \lang simp-chinese
     * @brief 在类型列表中查找唯一类型的索引。
     *
     * @tparam Find 要查找的类型
     * @tparam TypeList 要搜索的类型列表
     */
    template <typename Find, typename TypeList>
    struct type_find_unique {};

    /**
     * \lang english
     * @brief Recursive specialization that performs the search.
     *
     * @tparam Find The type to find
     * @tparam First The first type in the current sublist
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 执行搜索的递归特化。
     *
     * @tparam Find 要查找的类型
     * @tparam First 当前子列表中的第一个类型
     * @tparam Rest 剩余类型
     */
    template <typename Find, typename First, typename... Rest>
    struct type_find_unique<Find, type_list<First, Rest...>> {
        static constexpr std::size_t value =
            (count_type_v<Find, type_list<First, Rest...>> != 1)
                ? static_cast<std::size_t>(-1)
                : (type_relations::is_same_v<Find, First> ? 0 : 1 + type_find_unique<Find, type_list<Rest...>>::value);
    };

    /**
     * \lang english
     * @brief Specialization for empty list (type not found).
     *
     * @tparam Ty The type that was not found
     *
     * \lang simp-chinese
     * @brief 空列表的特化（类型未找到）。
     *
     * @tparam Ty 未找到的类型
     */
    template <typename Ty>
    struct type_find_unique<Ty, type_list<>> {
        static constexpr std::size_t value = static_cast<std::size_t>(-1);
    };

    /**
     * \lang english
     * @brief Checks if a type is present in a type list.
     *
     * @tparam Ty The type to check for
     * @tparam List The type list to search
     *
     * \lang simp-chinese
     * @brief 检查类型是否存在于类型列表中。
     *
     * @tparam Ty 要检查的类型
     * @tparam List 要搜索的类型列表
     */
    template <typename Ty, typename List>
    struct is_type_in_list;

    /**
     * \lang english
     * @brief Recursive specialization that performs the check.
     *
     * @tparam Ty The type to check for
     * @tparam List The template template parameter
     * @tparam First The first type in the list
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 执行检查的递归特化。
     *
     * @tparam Ty 要检查的类型
     * @tparam List 模板模板参数
     * @tparam First 列表中的第一个类型
     * @tparam Rest 剩余类型
     */
    template <typename Ty, template <typename...> class List, typename First, typename... Rest>
    struct is_type_in_list<Ty, List<First, Rest...>>
        : other_trans::conditional_t<implements::is_same_v<Ty, First>, type_traits::helper::true_type, is_type_in_list<Ty, List<Rest...>>> {};

    /**
     * \lang english
     * @brief Specialization for empty list (type not found).
     *
     * @tparam Ty The type that was not found
     * @tparam List The template template parameter
     *
     * \lang simp-chinese
     * @brief 空列表的特化（类型未找到）。
     *
     * @tparam Ty 未找到的类型
     * @tparam List 模板模板参数
     */
    template <typename Ty, template <typename...> class List>
    struct is_type_in_list<Ty, List<>> : type_traits::helper::false_type {};

    /**
     * \lang english
     * @brief Constant representing "not found" for type list indices.
     *
     * \lang simp-chinese
     * @brief 表示类型列表索引中“未找到”的常量。
     */
    RAINY_INLINE_CONSTEXPR std::size_t type_list_npos = static_cast<std::size_t>(-1);
}

namespace rainy::type_traits::implements {
    template <typename Void, template <typename...> typename Fx, typename... Args>
    struct meta_quote_helper;

    template <template <typename...> typename Fx, typename... Args>
    struct meta_quote_helper<other_trans::void_t<Fx<Args...>>, Fx, Args...> {
        using type = Fx<Args...>;
    };
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Quotes a template template parameter for use in higher-order metafunctions.
     *
     *  This template allows passing a template template parameter as a type,
     *  enabling it to be used in metafunctions that operate on templates.
     *
     * @tparam Fx The template template parameter to quote
     *
     * \lang simp-chinese
     * @brief 引用一个模板模板参数，用于高阶元函数中。
     *
     *  此模板允许将模板模板参数作为类型传递，使其能够在操作模板的元函数中使用。
     *
     * @tparam Fx 要引用的模板模板参数
     */
    template <template <typename...> typename Fx>
    struct type_list_quote {
        /**
         * \lang english
         * @brief Invokes the quoted template with the given type arguments.
         *
         * @tparam Types The type arguments to pass to the template
         *
         * \lang simp-chinese
         * @brief 使用给定的类型参数调用引用的模板。
         *
         * @tparam Types 要传递给模板的类型参数
         */
        template <typename... Types>
        using invoke = typename implements::meta_quote_helper<void, Fx, Types...>::type;
    };

    /**
     * \lang english
     * @brief Invokes a quoted template metafunction with the given arguments.
     *
     * @tparam Fn The quoted template metafunction (from type_list_quote)
     * @tparam Args The type arguments to pass to the metafunction
     *
     * \lang simp-chinese
     * @brief 使用给定的参数调用引用的模板元函数。
     *
     * @tparam Fn 引用的模板元函数（来自 type_list_quote）
     * @tparam Args 要传递给元函数的类型参数
     */
    template <typename Fn, typename... Args>
    using type_list_invoke = typename Fn::template invoke<Args...>;

    /**
     * \lang english
     * @brief Binds additional arguments to the back of a quoted template metafunction.
     *
     *  This creates a new metafunction that, when invoked, will pass the original
     *  arguments followed by the bound arguments.
     *
     * @tparam Fn The quoted template metafunction to bind arguments to
     * @tparam Args The arguments to bind to the back
     *
     * \lang simp-chinese
     * @brief 将额外的参数绑定到引用的模板元函数的后面。
     *
     *  这创建了一个新的元函数，当调用时，它将传递原始参数，后跟绑定的参数。
     *
     * @tparam Fn 要绑定参数的引用的模板元函数
     * @tparam Args 要绑定到后面的参数
     */
    template <typename Fn, typename... Args>
    struct type_list_bind_back {
        /**
         * \lang english
         * @brief Invokes the bound metafunction with the given arguments,
         *         followed by the previously bound arguments.
         *
         * @tparam Types The arguments to pass first to the metafunction
         *
         * \lang simp-chinese
         * @brief 使用给定的参数调用绑定的元函数，后跟之前绑定的参数。
         *
         * @tparam Types 要首先传递给元函数的参数
         */
        template <typename... Types>
        using invoke = type_list_invoke<Fn, Types..., Args...>;
    };
}

namespace rainy::type_traits::implements {
    template <typename Fn, typename List>
    struct type_list_apply_impl;

    template <typename Fn, typename... Types>
    struct type_list_apply_impl<Fn, other_trans::type_list<Types...>> {
        using type = other_trans::type_list_invoke<Fn, Types...>;
    };

    template <typename Fn, typename Ty, Ty... Idxs>
    struct type_list_apply_impl<Fn, helper::integer_sequence<Ty, Idxs...>> {
        using type = other_trans::type_list_invoke<Fn, helper::integral_constant<Ty, Idxs>...>;
    };

    template <typename Fn, typename List>
    struct type_list_transform_impl {};

    template <typename Fn, typename... Types>
    struct type_list_transform_impl<Fn, other_trans::type_list<Types...>> {
        using type = other_trans::type_list<other_trans::type_list_invoke<Fn, Types>...>;
    };

    template <typename List>
    struct meta_as_integer_sequence_impl;

    template <typename Ty, Ty... Idxs>
    struct meta_as_integer_sequence_impl<other_trans::type_list<helper::integral_constant<Ty, Idxs>...>> {
        using type = helper::integer_sequence<Ty, Idxs...>;
    };

    template <typename>
    struct type_list_cartesian_product_impl {};
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Computes the Cartesian product of a list of type lists.
     *
     *  Given a list of type lists, this metafunction generates a new type list
     *  containing all possible combinations where one type is taken from each
     *  input list. Each combination is represented as a type_list.
     *
     * @tparam ListOfLists A type list where each element is itself a type list
     * @return A type list of type lists, representing all possible combinations
     *
     * \lang simp-chinese
     * @brief 计算类型列表列表的笛卡尔积。
     *
     *  给定一个类型列表的列表，此元函数生成一个新的类型列表，
     *  包含从每个输入列表中取一个类型的所有可能组合。
     *  每个组合表示为一个 type_list。
     *
     * @tparam ListOfLists 一个类型列表，其中每个元素本身是一个类型列表
     * @return 类型列表的列表，表示所有可能的组合
     */
    template <typename ListOfLists>
    using type_list_cartesian_product = typename implements::type_list_cartesian_product_impl<ListOfLists>::type;
}

namespace rainy::type_traits::implements {
    template <typename ListLike>
    struct as_list_impl {};

    template <template <typename...> typename List, typename... Types>
    struct as_list_impl<List<Types...>> {
        using type = other_trans::type_list<Types...>;
    };

    template <typename Ty, Ty... Idxs>
    struct as_list_impl<helper::integer_sequence<Ty, Idxs...>> {
        using type = other_trans::type_list<helper::integral_constant<Ty, Idxs>...>;
    };
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Applies a quoted metafunction to each element of a type list and collects the results.
     *
     * @tparam Fn The quoted metafunction to apply
     * @tparam List The type list to transform
     *
     * \lang simp-chinese
     * @brief 将引用的元函数应用于类型列表的每个元素并收集结果。
     *
     * @tparam Fn 要应用的引用的元函数
     * @tparam List 要转换的类型列表
     */
    template <typename Fn, typename List>
    using type_list_apply = typename implements::type_list_apply_impl<Fn, List>::type;

    /**
     * \lang english
     * @brief Joins a list of type lists into a single type list.
     *
     *  This metafunction flattens a list of type lists by concatenating all
     *  the inner lists in order.
     *
     * @tparam TypeList A type list where each element is itself a type list
     *
     * \lang simp-chinese
     * @brief 将类型列表的列表连接成一个单一的类型列表。
     *
     *  此元函数通过按顺序连接所有内部列表来展平类型列表的列表。
     *
     * @tparam TypeList 一个类型列表，其中每个元素本身是一个类型列表
     */
    template <typename TypeList>
    using type_list_join = type_list_apply<type_list_quote<type_list_concat>, TypeList>;

    /**
     * \lang english
     * @brief Transforms each element of a type list using a quoted metafunction.
     *
     * @tparam Fn The quoted metafunction to apply to each element
     * @tparam List The type list to transform
     *
     * \lang simp-chinese
     * @brief 使用引用的元函数转换类型列表的每个元素。
     *
     * @tparam Fn 要应用于每个元素的引用的元函数
     * @tparam List 要转换的类型列表
     */
    template <typename Fn, typename List>
    using type_list_transform = typename implements::type_list_transform_impl<Fn, List>::type;
}

namespace rainy::type_traits::implements {
    template <>
    struct type_list_cartesian_product_impl<other_trans::type_list<>> {
        using type = other_trans::type_list<>;
    };

    template <typename... Items>
    struct type_list_cartesian_product_impl<other_trans::type_list<other_trans::type_list<Items...>>> {
        using type = other_trans::type_list<other_trans::type_list<Items>...>;
    };

    template <typename... Items, typename... Lists>
    struct type_list_cartesian_product_impl<other_trans::type_list<other_trans::type_list<Items...>, Lists...>> {
        using type = other_trans::type_list_join<other_trans::type_list<other_trans::type_list_transform<
            other_trans::type_list_bind_back<other_trans::type_list_quote<other_trans::type_list_push_front>, Items>,
            other_trans::type_list_cartesian_product<other_trans::type_list<Lists...>>>...>>;
    };
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Converts a type list of integral constants to an integer_sequence.
     *
     * @tparam List A type list of integral constants
     *
     * \lang simp-chinese
     * @brief 将整数常量的类型列表转换为 integer_sequence。
     *
     * @tparam List 整数常量的类型列表
     */
    template <typename List>
    struct meta_as_integer_sequence {
        using invoke = typename implements::meta_as_integer_sequence_impl<List>::type;
    };

    /**
     * \lang english
     * @brief Converts any type to a type_list.
     *
     *  For non-tuple-like types, this typically results in a type_list
     *  containing the type itself. For tuple-like types, it expands to
     *  a type_list of the contained types.
     *
     * @tparam Ty The type to convert
     *
     * \lang simp-chinese
     * @brief 将任何类型转换为 type_list。
     *
     *  对于非类似tuple的类型，这通常会导致一个包含类型本身的 type_list。
     *  对于类似tuple的类型，它会展开为包含类型的 type_list。
     *
     * @tparam Ty 要转换的类型
     */
    template <typename Ty>
    using as_list = typename implements::as_list_impl<Ty>::type;

    /**
     * \lang english
     * @brief Checks if a type is contained in a type list.
     *
     * @tparam Ty The type to search for
     * @tparam List The type list to search
     *
     * \lang simp-chinese
     * @brief 检查类型是否包含在类型列表中。
     *
     * @tparam Ty 要搜索的类型
     * @tparam List 要搜索的类型列表
     */
    template <typename Ty, typename List>
    struct type_list_contains;

    /**
     * \lang english
     * @brief Specialization for empty list (type not found).
     *
     * @tparam Ty The type that was not found
     *
     * \lang simp-chinese
     * @brief 空列表的特化（类型未找到）。
     *
     * @tparam Ty 未找到的类型
     */
    template <typename Ty>
    struct type_list_contains<Ty, type_list<>> : type_traits::helper::false_type {};

    /**
     * \lang english
     * @brief Recursive specialization that performs the search.
     *
     * @tparam Ty The type to search for
     * @tparam First The first type in the current sublist
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 执行搜索的递归特化。
     *
     * @tparam Ty 要搜索的类型
     * @tparam First 当前子列表中的第一个类型
     * @tparam Rest 剩余类型
     */
    template <typename Ty, typename First, typename... Rest>
    struct type_list_contains<Ty, type_list<First, Rest...>>
        : type_traits::other_trans::conditional_t<type_traits::type_relations::is_same_v<Ty, First>, type_traits::helper::true_type, type_list_contains<Ty, type_list<Rest...>>> {};

    /**
     * \lang english
     * @brief Removes duplicate types from a type list, preserving order of first occurrence.
     *
     * @tparam List The type list to process
     *
     * \lang simp-chinese
     * @brief 从类型列表中移除重复的类型，保留首次出现的顺序。
     *
     * @tparam List 要处理的类型列表
     */
    template <typename List>
    struct unique_type_list;

    /**
     * \lang english
     * @brief Specialization for empty list.
     *
     * \lang simp-chinese
     * @brief 空列表的特化。
     */
    template <>
    struct unique_type_list<type_list<>> {
        using type = type_list<>;
    };

    /**
     * \lang english
     * @brief Specialization for single-element list.
     *
     * @tparam Ty The single type
     *
     * \lang simp-chinese
     * @brief 单元素列表的特化。
     *
     * @tparam Ty 单一类型
     */
    template <typename Ty>
    struct unique_type_list<type_list<Ty>> {
        using type = type_list<Ty>;
    };

    /**
     * \lang english
     * @brief Recursive specialization that builds a unique list.
     *
     * @tparam First The first type in the list
     * @tparam Second The second type in the list
     * @tparam Rest The remaining types
     *
     * \lang simp-chinese
     * @brief 构建唯一列表的递归特化。
     *
     * @tparam First 列表中的第一个类型
     * @tparam Second 列表中的第二个类型
     * @tparam Rest 剩余类型
     */
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

    /**
     * \lang english
     * @brief Alias template for unique_type_list.
     *
     * @tparam List The type list to process
     *
     * \lang simp-chinese
     * @brief unique_type_list 的别名模板。
     *
     * @tparam List 要处理的类型列表
     */
    template <typename List>
    using unique_type_list_t = typename unique_type_list<List>::type;
}

namespace rainy::type_traits::other_trans {
    /**
     * \lang english
     * @brief Selects the first type in TypeList whose size is at least Size.
     *         Returns void if no type satisfies the requirement.
     *
     * @tparam Size The minimum required size of the selected type
     * @tparam TypeList The type list to search
     *
     * \lang simp-chinese
     * @brief 选择 TypeList 中第一个大小不小于 Size 的类型。
     *         若没有类型满足要求则返回 void。
     *
     * @tparam Size 所选类型所需的最小大小
     * @tparam TypeList 要搜索的类型列表
     */
    template <size_t Size, typename TypeList>
    struct select_type;

    template <size_t Size, typename First, typename... Rest>
    struct select_type<Size, type_list<First, Rest...>> {
        using type = type_traits::other_trans::conditional_t<(Size <= sizeof(First)), First, typename select_type<Size, type_list<Rest...>>::type>;
    };

    template <size_t Size>
    struct select_type<Size, type_list<>> {
        using type = void;
    };
}

#endif
