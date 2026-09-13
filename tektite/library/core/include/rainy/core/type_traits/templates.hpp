#ifndef RAINY_CORE_TYPE_TRAITS_TEMPLATES_HPP
#define RAINY_CORE_TYPE_TRAITS_TEMPLATES_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/helper.hpp>
#include <rainy/core/type_traits/type_list.hpp>

namespace rainy::type_traits::extras::templates {
    /**
     * \lang english
     * @brief Primary template for getting the first template parameter of a template instantiation.
     *
     * @tparam Ty The template instantiation type
     *
     * \lang simp-chinese
     * @brief 获取模板实例化的第一个模板参数的主模板。
     *
     * @tparam Ty 模板实例化类型
     */
    template <typename Ty>
    struct get_first_parameter;

    /**
     * \lang english
     * @brief Specialization that extracts the first template parameter.
     *
     * @tparam Ty The template template parameter
     * @tparam First The first template parameter type
     * @tparam Rest The remaining template parameter types
     *
     * \lang simp-chinese
     * @brief 提取第一个模板参数的特化。
     *
     * @tparam Ty 模板模板参数
     * @tparam First 第一个模板参数类型
     * @tparam Rest 剩余的模板参数类型
     */
    template <template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct get_first_parameter<Ty<First, Rest...>> {
        using type = First;
    };

    /**
     * \lang english
     * @brief Primary template for replacing the first template parameter of a template instantiation.
     *
     * @tparam newfirst The new type for the first parameter
     * @tparam Ty The template instantiation type
     *
     * \lang simp-chinese
     * @brief 替换模板实例化的第一个模板参数的主模板。
     *
     * @tparam newfirst 第一个参数的新类型
     * @tparam Ty 模板实例化类型
     */
    template <typename newfirst, typename Ty>
    struct replace_first_parameter;

    /**
     * \lang english
     * @brief Specialization that performs the replacement.
     *
     * @tparam NewFirst The new type for the first parameter
     * @tparam Ty The template template parameter
     * @tparam First The original first parameter type (to be replaced)
     * @tparam Rest The remaining template parameter types (preserved)
     *
     * \lang simp-chinese
     * @brief 执行替换操作的特化。
     *
     * @tparam NewFirst 第一个参数的新类型
     * @tparam Ty 模板模板参数
     * @tparam First 原始的第一个参数类型（将被替换）
     * @tparam Rest 剩余的模板参数类型（保持不变）
     */
    template <typename NewFirst, template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct replace_first_parameter<NewFirst, Ty<First, Rest...>> {
        using type = Ty<NewFirst, Rest...>;
    };

    /**
     * \lang english
     * @brief Primary template for getting the pointer difference type.
     *         Defaults to ptrdiff_t.
     *
     * @tparam Ty The type to query for difference_type
     *
     * \lang simp-chinese
     * @brief 获取指针差类型的主模板。
     *         默认为 ptrdiff_t。
     *
     * @tparam Ty 要查询 difference_type 的类型
     */
    template <typename, typename = void>
    struct get_ptr_difference_type {
        using type = ptrdiff_t;
    };

    /**
     * \lang english
     * @brief Specialization for types that provide a difference_type member.
     *
     * @tparam Ty The type that provides difference_type
     *
     * \lang simp-chinese
     * @brief 为提供 difference_type 成员的类型提供的特化。
     *
     * @tparam Ty 提供 difference_type 的类型
     */
    template <typename Ty>
    struct get_ptr_difference_type<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    /**
     * \lang english
     * @brief Primary template for getting the rebound alias of an allocator or similar template.
     *         Uses replace_first_parameter as fallback.
     *
     * @tparam Ty The template type to rebind
     * @tparam Other The new type to bind to
     *
     * \lang simp-chinese
     * @brief 获取分配器或类似模板的重绑定别名的主模板。
     *         使用 replace_first_parameter 作为回退。
     *
     * @tparam Ty 要重绑定的模板类型
     * @tparam Other 要绑定到的新类型
     */
    template <typename Ty, typename Other, typename = void>
    struct get_rebind_alias {
        using type = typename replace_first_parameter<Other, Ty>::type;
    };

    /**
     * \lang english
     * @brief Specialization for types that provide a rebind member template.
     *
     * @tparam Ty The type that provides rebind
     * @tparam Other The new type to bind to
     *
     * \lang simp-chinese
     * @brief 为提供 rebind 成员模板的类型提供的特化。
     *
     * @tparam Ty 提供 rebind 的类型
     * @tparam Other 要绑定到的新类型
     */
    template <typename Ty, typename Other>
    struct get_rebind_alias<Ty, Other, type_traits::other_trans::void_t<typename Ty::template rebind<Other>>> {
        using type = typename Ty::template rebind<Other>;
    };

    /**
     * \lang english
     * @brief Replaces the last template parameter of a template instantiation.
     *
     * @tparam Ty The template instantiation type
     * @tparam NewLast The new type for the last parameter
     *
     * \lang simp-chinese
     * @brief 替换模板实例化的最后一个模板参数。
     *
     * @tparam Ty 模板实例化类型
     * @tparam NewLast 最后一个参数的新类型
     */
    template <typename Ty, typename NewLast>
    struct replace_last_parameter;

    /**
     * \lang english
     * @brief Specialization that performs the replacement.
     *
     * @tparam Template The template template parameter
     * @tparam Args The original template arguments
     * @tparam NewLast The new type for the last parameter
     *
     * \lang simp-chinese
     * @brief 执行替换操作的特化。
     *
     * @tparam Template 模板模板参数
     * @tparam Args 原始的模板参数
     * @tparam NewLast 最后一个参数的新类型
     */
    template <template <typename...> typename Template, typename... Args, typename NewLast>
    struct replace_last_parameter<Template<Args...>, NewLast> {
    private:
        static constexpr std::size_t N = sizeof...(Args);
        using type_list = type_traits::other_trans::type_list<Args...>;

        template <std::size_t... I>
        static auto helper(type_traits::helper::index_sequence<I...>)
            -> Template<typename type_traits::other_trans::type_at<I, type_list>::type..., NewLast>;

    public:
        using type = decltype(helper(type_traits::helper::make_index_sequence<N - 1>{}));
    };
}

#endif