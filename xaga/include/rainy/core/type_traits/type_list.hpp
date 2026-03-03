#ifndef RAINY_META_TYPE_TRAITS_TYPE_LIST_HPP
#define RAINY_META_TYPE_TRAITS_TYPE_LIST_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/helper.hpp>

namespace rainy::type_traits::other_trans {
    /**
     * @brief A compile-time list of types.
     *        编译时类型列表。
     *
     * @tparam Types The types contained in the list
     *               列表中包含的类型
     */
    template <typename... Types>
    struct type_list;

    /**
     * @brief Specialization for non-empty type lists.
     *        非空类型列表的特化。
     *
     * @tparam First The first type in the list
     *               列表中的第一个类型
     * @tparam Rest The remaining types in the list
     *              列表中剩余的类型
     */
    template <typename First, typename... Rest>
    struct type_list<First, Rest...> {
        /**
         * @brief The first type in the list.
         *        列表中的第一个类型。
         */
        using type = First;

        /**
         * @brief The remaining types in the list as a nested type_list.
         *        列表中剩余的类型作为嵌套的 type_list。
         */
        using next = type_list<Rest...>;
    };

    /**
     * @brief Specialization for empty type lists.
     *        空类型列表的特化。
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
     * @brief Concatenates multiple type lists into a single type list.
     *        将多个类型列表连接成一个类型列表。
     *
     * This alias template provides a way to combine multiple type_list
     * instantiations into one unified type list containing all types
     * from the input lists in order.
     *
     * 此别名模板提供了一种将多个 type_list 实例合并为一个统一类型列表的方法，
     * 该列表按顺序包含所有输入列表中的类型。
     *
     * @tparam Types The type lists to concatenate
     *               要连接的类型列表
     * @return A single type_list containing all types from the input lists
     *         包含所有输入列表中类型的单个 type_list
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
     * @brief Retrieves the type at a given index in a type list.
     *        获取类型列表中指定索引处的类型。
     *
     * @tparam N The index to retrieve
     *           要获取的索引
     * @tparam TypeList The type list to query
     *                  要查询的类型列表
     */
    template <std::size_t N, typename TypeList>
    struct type_at;

    /**
     * @brief Recursive specialization for non-zero indices.
     *        非零索引的递归特化。
     *
     * @tparam N The remaining index
     *           剩余索引
     * @tparam First The first type in the list (ignored)
     *               列表中的第一个类型（忽略）
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <std::size_t N, typename First, typename... Rest>
    struct type_at<N, type_list<First, Rest...>> : type_at<N - 1, type_list<Rest...>> {};

    /**
     * @brief Specialization for index 0, returns the first type.
     *        索引0的特化，返回第一个类型。
     *
     * @tparam First The first type
     *               第一个类型
     * @tparam Rest The remaining types (ignored)
     *              剩余类型（忽略）
     */
    template <typename First, typename... Rest>
    struct type_at<0, type_list<First, Rest...>> {
        using type = First;
    };

    /**
     * @brief Alias template for type_at.
     *        type_at 的别名模板。
     *
     * @tparam N The index to retrieve
     *           要获取的索引
     * @tparam TypeList The type list to query
     *                  要查询的类型列表
     */
    template <std::size_t N, typename TypeList>
    using type_at_t = typename type_at<N, TypeList>::type;

    /**
     * @brief Adds a type to the front of a type list.
     *        在类型列表的前面添加一个类型。
     *
     * @tparam Ty The type to add
     *            要添加的类型
     * @tparam TypeList The type list to modify
     *                  要修改的类型列表
     */
    template <typename Ty, typename TypeList>
    struct type_list_push_front {};

    /**
     * @brief Specialization that performs the front push.
     *        执行前推操作的特化。
     *
     * @tparam Ty The type to add
     *            要添加的类型
     * @tparam Types The existing types
     *               现有的类型
     */
    template <typename Ty, typename... Types>
    struct type_list_push_front<Ty, type_list<Types...>> {
        using type = type_list<Ty, Types...>;
    };

    /**
     * @brief Alias template for type_list_push_front.
     *        type_list_push_front 的别名模板。
     *
     * @tparam Ty The type to add
     *            要添加的类型
     * @tparam TypeList The type list to modify
     *                  要修改的类型列表
     */
    template <typename Ty, typename TypeList>
    using type_list_push_front_t = typename type_list_push_front<Ty, TypeList>::type;

    /**
     * @brief Adds a type to the back of a type list.
     *        在类型列表的后面添加一个类型。
     *
     * @tparam Ty The type to add
     *            要添加的类型
     * @tparam TypeList The type list to modify
     *                  要修改的类型列表
     */
    template <typename Ty, typename TypeList>
    struct type_list_push_back {};

    /**
     * @brief Specialization for empty list.
     *        空列表的特化。
     *
     * @tparam Ty The type to add
     *            要添加的类型
     */
    template <typename Ty>
    struct type_list_push_back<Ty, type_list<>> {
        using type = type_list<Ty>;
    };

    /**
     * @brief Specialization that performs the back push.
     *        执行后推操作的特化。
     *
     * @tparam Ty The type to add
     *            要添加的类型
     * @tparam Types The existing types
     *               现有的类型
     */
    template <typename Ty, typename... Types>
    struct type_list_push_back<Ty, type_list<Types...>> {
        using type = type_list<Types..., Ty>;
    };

    /**
     * @brief Adds multiple types to the front of a type list.
     *        在类型列表的前面添加多个类型。
     *
     * @tparam TypeList The type list to modify
     *                  要修改的类型列表
     * @tparam PushTypes The types to add to the front
     *                   要添加到前面的类型
     */
    template <typename TypeList, typename... PushTypes>
    struct type_list_push_front_all;

    /**
     * @brief Specialization that performs the multiple front push.
     *        执行多个前推操作的特化。
     *
     * @tparam Types The existing types
     *               现有的类型
     * @tparam PushTypes The types to add to the front
     *                   要添加到前面的类型
     */
    template <typename... Types, typename... PushTypes>
    struct type_list_push_front_all<type_list<Types...>, PushTypes...> {
        using type = type_list<PushTypes..., Types...>;
    };

    /**
     * @brief Adds multiple types to the back of a type list.
     *        在类型列表的后面添加多个类型。
     *
     * @tparam TypeList The type list to modify
     *                  要修改的类型列表
     * @tparam PushTypes The types to add to the back
     *                   要添加到后面的类型
     */
    template <typename TypeList, typename... PushTypes>
    struct type_list_push_back_all;

    /**
     * @brief Specialization that performs the multiple back push.
     *        执行多个后推操作的特化。
     *
     * @tparam Types The existing types
     *               现有的类型
     * @tparam PushTypes The types to add to the back
     *                   要添加到后面的类型
     */
    template <typename... Types, typename... PushTypes>
    struct type_list_push_back_all<type_list<Types...>, PushTypes...> {
        using type = type_list<Types..., PushTypes...>;
    };

    /**
     * @brief Removes the first type from a type list.
     *        从类型列表中移除第一个类型。
     *
     * @tparam TypeList The type list to modify
     *                  要修改的类型列表
     */
    template <typename TypeList>
    struct type_list_pop_front;

    /**
     * @brief Specialization for empty list.
     *        空列表的特化。
     */
    template <>
    struct type_list_pop_front<type_list<>> {
        using type = type_list<>;
    };

    /**
     * @brief Recursive specialization that removes the first type.
     *        移除第一个类型的递归特化。
     *
     * @tparam Front The first type (to be removed)
     *               第一个类型（将被移除）
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <typename Front, typename... Rest>
    struct type_list_pop_front<type_list<Front, Rest...>> {
    public:
        using tail_type = typename type_list_pop_front<type_list<Rest...>>::type;
        using type = typename type_list_push_front<tail_type, Front>::type;
    };

    /**
     * @brief Removes the last type from a type list.
     *        从类型列表中移除最后一个类型。
     *
     * @tparam List The type list to modify
     *              要修改的类型列表
     */
    template <typename List>
    struct type_list_pop_back;

    /**
     * @brief Specialization for empty list.
     *        空列表的特化。
     */
    template <>
    struct type_list_pop_back<type_list<>> {
        using type = type_list<>;
    };

    /**
     * @brief Specialization for list with one element.
     *        只有一个元素的列表的特化。
     *
     * @tparam Ty The single type (to be removed)
     *            单一类型（将被移除）
     */
    template <typename Ty>
    struct type_list_pop_back<type_list<Ty>> {
        using type = type_list<>;
    };

    /**
     * @brief Recursive specialization that removes the last type.
     *        移除最后一个类型的递归特化。
     *
     * @tparam Head The first type
     *              第一个类型
     * @tparam Tail The remaining types
     *              剩余类型
     */
    template <typename Head, typename... Tail>
    struct type_list_pop_back<type_list<Head, Tail...>> {
        using type = typename type_list_push_front<typename type_list_pop_back<type_list<Tail...>>::type, Head>::type;
    };

    /**
     * @brief Converts a tuple-like type to a type_list.
     *        将类似tuple的类型转换为type_list。
     *
     * @tparam TupleLike The tuple-like type to convert
     *                   要转换的类似tuple的类型
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
     * @brief Converts a type_list to a tuple-like type.
     *        将type_list转换为类似tuple的类型。
     *
     * @tparam TypeList The type list to convert
     *                  要转换的类型列表
     * @tparam TupleLike The tuple-like template to instantiate
     *                   要实例化的类似tuple的模板
     */
    template <typename TypeList, template <typename...> class TupleLike>
    struct type_list_to_tuple_like;

    /**
     * @brief Specialization that performs the conversion.
     *        执行转换操作的特化。
     *
     * @tparam Types The types in the list
     *               列表中的类型
     * @tparam TupleLike The tuple-like template
     *                   类似tuple的模板
     */
    template <typename... Types, template <typename...> class TupleLike>
    struct type_list_to_tuple_like<type_list<Types...>, TupleLike> {
        using type = TupleLike<Types...>;
    };

    /**
     * @brief Variable template for getting the size of a type list.
     *        获取类型列表大小的变量模板。
     *
     * @tparam TypeList The type list
     *                  类型列表
     */
    template <typename TypeList>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v = 0;

    /**
     * @brief Specialization for empty list.
     *        空列表的特化。
     */
    template <>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v<type_list<>> = 0;

    /**
     * @brief Specialization for non-empty lists.
     *        非空列表的特化。
     *
     * @tparam Types The types in the list
     *               列表中的类型
     */
    template <typename... Types>
    RAINY_INLINE_CONSTEXPR std::size_t type_list_size_v<type_list<Types...>> = sizeof...(Types);

    /**
     * @brief Type template for getting the size of a type list.
     *        获取类型列表大小的类型模板。
     *
     * @tparam TypeList The type list
     *                  类型列表
     */
    template <typename TypeList>
    struct type_list_size : helper::integer_sequence<std::size_t, type_list_size_v<TypeList>> {};

    /**
     * @brief Variable template for counting occurrences of a type in a type list.
     *        计算类型在类型列表中出现的次数的变量模板。
     *
     * @tparam Target The type to count
     *                要计数的类型
     * @tparam TypeList The type list to search
     *                  要搜索的类型列表
     */
    template <typename Target, typename TypeList>
    RAINY_INLINE_CONSTEXPR std::size_t count_type_v = static_cast<std::size_t>(-1);

    /**
     * @brief Specialization that performs the count.
     *        执行计数的特化。
     *
     * @tparam Target The type to count
     *                要计数的类型
     * @tparam Args The types in the list
     *              列表中的类型
     */
    template <typename Target, typename... Args>
    RAINY_INLINE_CONSTEXPR std::size_t count_type_v<Target, type_list<Args...>> =
        (0 + ... + (type_traits::type_relations::is_same_v<Target, Args> ? 1 : 0));

    /**
     * @brief Type template for counting occurrences of a type in a type list.
     *        计算类型在类型列表中出现的次数的类型模板。
     *
     * @tparam Target The type to count
     *                要计数的类型
     * @tparam TypeList The type list to search
     *                  要搜索的类型列表
     */
    template <typename Target, typename TypeList>
    struct count_type : helper::integral_constant<std::size_t, count_type_v<Target, TypeList>> {};

    /**
     * @brief Extracts a sub-list starting from a given index.
     *        从给定索引开始提取子列表。
     *
     * @tparam StartIndex The starting index
     *                    起始索引
     * @tparam TypeList The type list to slice
     *                  要切片的类型列表
     */
    template <std::size_t StartIndex, typename TypeList>
    struct sub_type_list;

    /**
     * @brief Recursive specialization to skip to the start index.
     *        跳过到起始索引的递归特化。
     *
     * @tparam StartIndex The remaining index
     *                    剩余索引
     * @tparam First The first type (to be skipped)
     *               第一个类型（将被跳过）
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <std::size_t StartIndex, typename First, typename... Rest>
    struct sub_type_list<StartIndex, type_list<First, Rest...>> : sub_type_list<StartIndex - 1, type_list<Rest...>> {};

    /**
     * @brief Specialization that returns the sub-list starting from index 0.
     *        返回从索引0开始的子列表的特化。
     *
     * @tparam First The first type
     *               第一个类型
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <typename First, typename... Rest>
    struct sub_type_list<0, type_list<First, Rest...>> {
        using type = type_list<First, Rest...>;
    };

    /**
     * @brief Specialization for empty list at index 0.
     *        索引0的空列表的特化。
     */
    template <>
    struct sub_type_list<0, type_list<>> {
        using type = type_list<>;
    };

    /**
     * @brief Specialization when index is beyond list size.
     *        当索引超出列表大小时的特化。
     *
     * @tparam StartIndex The out-of-range index
     *                    超出范围的索引
     */
    template <std::size_t StartIndex>
    struct sub_type_list<StartIndex, type_list<>> {
        using type = type_list<>;
    };

    /**
     * @brief Gets the first type in a type list.
     *        获取类型列表中的第一个类型。
     *
     * @tparam TypeList The type list to query
     *                  要查询的类型列表
     */
    template <typename TypeList>
    struct type_list_front {};

    /**
     * @brief Specialization that returns the first type.
     *        返回第一个类型的特化。
     *
     * @tparam Type The first type
     *              第一个类型
     * @tparam Rest The remaining types (ignored)
     *              剩余类型（忽略）
     */
    template <typename Type, typename... Rest>
    struct type_list_front<type_list<Type, Rest...>> {
        using type = Type;
    };

    /**
     * @brief Finds the index of a unique type in a type list.
     *        在类型列表中查找唯一类型的索引。
     *
     * @tparam Find The type to find
     *              要查找的类型
     * @tparam TypeList The type list to search
     *                  要搜索的类型列表
     */
    template <typename Find, typename TypeList>
    struct type_find_unique {};

    /**
     * @brief Recursive specialization that performs the search.
     *        执行搜索的递归特化。
     *
     * @tparam Find The type to find
     *              要查找的类型
     * @tparam First The first type in the current sublist
     *               当前子列表中的第一个类型
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <typename Find, typename First, typename... Rest>
    struct type_find_unique<Find, type_list<First, Rest...>> {
        static constexpr std::size_t value =
            (count_type_v<Find, type_list<First, Rest...>> != 1)
                ? static_cast<std::size_t>(-1)
                : (type_relations::is_same_v<Find, First> ? 0 : 1 + type_find_unique<Find, type_list<Rest...>>::value);
    };

    /**
     * @brief Specialization for empty list (type not found).
     *        空列表的特化（类型未找到）。
     *
     * @tparam Ty The type that was not found
     *            未找到的类型
     */
    template <typename Ty>
    struct type_find_unique<Ty, type_list<>> {
        static constexpr std::size_t value = static_cast<std::size_t>(-1);
    };

    /**
     * @brief Checks if a type is present in a type list.
     *        检查类型是否存在于类型列表中。
     *
     * @tparam Ty The type to check for
     *            要检查的类型
     * @tparam List The type list to search
     *              要搜索的类型列表
     */
    template <typename Ty, typename List>
    struct is_type_in_list;

    /**
     * @brief Recursive specialization that performs the check.
     *        执行检查的递归特化。
     *
     * @tparam Ty The type to check for
     *            要检查的类型
     * @tparam List The template template parameter
     *              模板模板参数
     * @tparam First The first type in the list
     *               列表中的第一个类型
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <typename Ty, template <typename...> class List, typename First, typename... Rest>
    struct is_type_in_list<Ty, List<First, Rest...>>
        : other_trans::conditional_t<implements::is_same_v<Ty, First>, std::true_type, is_type_in_list<Ty, List<Rest...>>> {};

    /**
     * @brief Specialization for empty list (type not found).
     *        空列表的特化（类型未找到）。
     *
     * @tparam T The type that was not found
     *           未找到的类型
     * @tparam List The template template parameter
     *              模板模板参数
     */
    template <typename T, template <typename...> class List>
    struct is_type_in_list<T, List<>> : std::false_type {};

    /**
     * @brief Constant representing "not found" for type list indices.
     *        表示类型列表索引中“未找到”的常量。
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
     * @brief Quotes a template template parameter for use in higher-order metafunctions.
     *        引用一个模板模板参数，用于高阶元函数中。
     *
     * This template allows passing a template template parameter as a type,
     * enabling it to be used in metafunctions that operate on templates.
     *
     * 此模板允许将模板模板参数作为类型传递，使其能够在操作模板的元函数中使用。
     *
     * @tparam Fx The template template parameter to quote
     *            要引用的模板模板参数
     */
    template <template <typename...> typename Fx>
    struct type_list_quote {
        /**
         * @brief Invokes the quoted template with the given type arguments.
         *        使用给定的类型参数调用引用的模板。
         *
         * @tparam Types The type arguments to pass to the template
         *                要传递给模板的类型参数
         */
        template <typename... Types>
        using invoke = typename implements::meta_quote_helper<void, Fx, Types...>::type;
    };

    /**
     * @brief Invokes a quoted template metafunction with the given arguments.
     *        使用给定的参数调用引用的模板元函数。
     *
     * @tparam Fn The quoted template metafunction (from type_list_quote)
     *            引用的模板元函数（来自 type_list_quote）
     * @tparam Args The type arguments to pass to the metafunction
     *              要传递给元函数的类型参数
     */
    template <typename Fn, typename... Args>
    using type_list_invoke = typename Fn::template invoke<Args...>;

    /**
     * @brief Binds additional arguments to the back of a quoted template metafunction.
     *        将额外的参数绑定到引用的模板元函数的后面。
     *
     * This creates a new metafunction that, when invoked, will pass the original
     * arguments followed by the bound arguments.
     *
     * 这创建了一个新的元函数，当调用时，它将传递原始参数，后跟绑定的参数。
     *
     * @tparam Fn The quoted template metafunction to bind arguments to
     *            要绑定参数的引用的模板元函数
     * @tparam Args The arguments to bind to the back
     *              要绑定到后面的参数
     */
    template <typename Fn, typename... Args>
    struct type_list_bind_back {
        /**
         * @brief Invokes the bound metafunction with the given arguments,
         *        followed by the previously bound arguments.
         *        使用给定的参数调用绑定的元函数，后跟之前绑定的参数。
         *
         * @tparam Types The arguments to pass first to the metafunction
         *               要首先传递给元函数的参数
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
     * @brief Computes the Cartesian product of a list of type lists.
     *        计算类型列表列表的笛卡尔积。
     *
     * Given a list of type lists, this metafunction generates a new type list
     * containing all possible combinations where one type is taken from each
     * input list. Each combination is represented as a type_list.
     *
     * 给定一个类型列表的列表，此元函数生成一个新的类型列表，
     * 包含从每个输入列表中取一个类型的所有可能组合。
     * 每个组合表示为一个 type_list。
     *
     * @tparam ListOfLists A type list where each element is itself a type list
     *                     一个类型列表，其中每个元素本身是一个类型列表
     * @return A type list of type lists, representing all possible combinations
     *         类型列表的列表，表示所有可能的组合
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
     * @brief Applies a quoted metafunction to each element of a type list and collects the results.
     *        将引用的元函数应用于类型列表的每个元素并收集结果。
     *
     * @tparam Fn The quoted metafunction to apply
     *            要应用的引用的元函数
     * @tparam List The type list to transform
     *              要转换的类型列表
     */
    template <typename Fn, typename List>
    using type_list_apply = typename implements::type_list_apply_impl<Fn, List>::type;

    /**
     * @brief Joins a list of type lists into a single type list.
     *        将类型列表的列表连接成一个单一的类型列表。
     *
     * This metafunction flattens a list of type lists by concatenating all
     * the inner lists in order.
     *
     * 此元函数通过按顺序连接所有内部列表来展平类型列表的列表。
     *
     * @tparam TypeList A type list where each element is itself a type list
     *                  一个类型列表，其中每个元素本身是一个类型列表
     */
    template <typename TypeList>
    using type_list_join = type_list_apply<type_list_quote<type_list_concat>, TypeList>;

    /**
     * @brief Transforms each element of a type list using a quoted metafunction.
     *        使用引用的元函数转换类型列表的每个元素。
     *
     * @tparam Fn The quoted metafunction to apply to each element
     *            要应用于每个元素的引用的元函数
     * @tparam List The type list to transform
     *              要转换的类型列表
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
     * @brief Converts a type list of integral constants to an integer_sequence.
     *        将整数常量的类型列表转换为 integer_sequence。
     *
     * @tparam List A type list of integral constants
     *              整数常量的类型列表
     */
    template <typename List>
    struct meta_as_integer_sequence {
        using invoke = typename implements::meta_as_integer_sequence_impl<List>::type;
    };

    /**
     * @brief Converts any type to a type_list.
     *        将任何类型转换为 type_list。
     *
     * For non-tuple-like types, this typically results in a type_list
     * containing the type itself. For tuple-like types, it expands to
     * a type_list of the contained types.
     *
     * 对于非类似tuple的类型，这通常会导致一个包含类型本身的 type_list。
     * 对于类似tuple的类型，它会展开为包含类型的 type_list。
     *
     * @tparam Ty The type to convert
     *            要转换的类型
     */
    template <class Ty>
    using as_list = typename implements::as_list_impl<Ty>::type;

    /**
     * @brief Checks if a type is contained in a type list.
     *        检查类型是否包含在类型列表中。
     *
     * @tparam T The type to search for
     *           要搜索的类型
     * @tparam List The type list to search
     *              要搜索的类型列表
     */
    template <typename T, typename List>
    struct type_list_contains;

    /**
     * @brief Specialization for empty list (type not found).
     *        空列表的特化（类型未找到）。
     *
     * @tparam T The type that was not found
     *           未找到的类型
     */
    template <typename T>
    struct type_list_contains<T, type_list<>> : std::false_type {};

    /**
     * @brief Recursive specialization that performs the search.
     *        执行搜索的递归特化。
     *
     * @tparam T The type to search for
     *           要搜索的类型
     * @tparam First The first type in the current sublist
     *               当前子列表中的第一个类型
     * @tparam Rest The remaining types
     *              剩余类型
     */
    template <typename T, typename First, typename... Rest>
    struct type_list_contains<T, type_list<First, Rest...>>
        : std::conditional_t<std::is_same_v<T, First>, std::true_type, type_list_contains<T, type_list<Rest...>>> {};

    /**
     * @brief Removes duplicate types from a type list, preserving order of first occurrence.
     *        从类型列表中移除重复的类型，保留首次出现的顺序。
     *
     * @tparam List The type list to process
     *              要处理的类型列表
     */
    template <typename List>
    struct unique_type_list;

    /**
     * @brief Specialization for empty list.
     *        空列表的特化。
     */
    template <>
    struct unique_type_list<type_list<>> {
        using type = type_list<>;
    };

    /**
     * @brief Specialization for single-element list.
     *        单元素列表的特化。
     *
     * @tparam T The single type
     *           单一类型
     */
    template <typename T>
    struct unique_type_list<type_list<T>> {
        using type = type_list<T>;
    };

    /**
     * @brief Recursive specialization that builds a unique list.
     *        构建唯一列表的递归特化。
     *
     * @tparam First The first type in the list
     *               列表中的第一个类型
     * @tparam Second The second type in the list
     *                列表中的第二个类型
     * @tparam Rest The remaining types
     *              剩余类型
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
     * @brief Alias template for unique_type_list.
     *        unique_type_list 的别名模板。
     *
     * @tparam List The type list to process
     *              要处理的类型列表
     */
    template <typename List>
    using unique_type_list_t = typename unique_type_list<List>::type;
}

namespace rainy::type_traits::other_trans {
    template <size_t Size, typename TypeList>
    struct select_type;

    template <size_t Size, typename First, typename... Rest>
    struct select_type<Size, type_list<First, Rest...>> {
        using type = std::conditional_t<(Size <= sizeof(First)), First, typename select_type<Size, type_list<Rest...>>::type>;
    };

    template <size_t Size>
    struct select_type<Size, type_list<>> {
        using type = void;
    };
}

#endif
