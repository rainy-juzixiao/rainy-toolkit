/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_CORE_CONTAINER_TUPLE_HPP
#define RAINY_CORE_CONTAINER_TUPLE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/decay.hpp>
#include <rainy/core/type_traits/logical.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_list.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <tuple>
#include <utility>

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A fixed-size collection of heterogeneous values.
     *
     * @tparam Types The types of elements contained in the tuple
     *
     * \lang simp-chinese
     * @brief 固定大小的异构值集合。
     *
     * @tparam Types tuple中包含的元素类型
     */
    template <typename... Types>
    class tuple;

    /**
     * \lang english
     * @brief Specialization for empty tuple.
     *
     * \lang simp-chinese
     * @brief 空tuple的特化。
     */
    template <>
    class tuple<> {
    public:
        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        constexpr tuple() noexcept = default;

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         */
        constexpr tuple(const tuple &) = default;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         */
        constexpr tuple(tuple &&) = default;

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         */
        constexpr tuple &operator=(const tuple &) = default;

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         */
        constexpr tuple &operator=(tuple &&) = default;

        /**
         * \lang english
         * @brief Swaps two empty tuples (no-op).
         *
         * @param right The right tuple to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个空tuple（无操作）。
         *
         * @param right 要交换的另一个tuple
         */
        static constexpr void swap(tuple &) noexcept {
        }

        /**
         * \lang english
         * @brief Allocator-extended default constructor.
         *
         * @tparam Alloc Allocator type
         * @param tag allocator_arg_t tag for disambiguation
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 分配器扩展的默认构造函数。
         *
         * @tparam Alloc 分配器类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param alloc 要使用的分配器
         */
        template <typename Alloc>
        constexpr tuple(std::allocator_arg_t, const Alloc &) noexcept {
        }

        /**
         * \lang english
         * @brief Allocator-extended constructor with arguments.
         *
         * @tparam Alloc Allocator type
         * @tparam Args Types of the arguments
         * @param tag allocator_arg_t tag for disambiguation
         * @param alloc The allocator to use
         * @param args Values used to construct the elements
         *
         * \lang simp-chinese
         * @brief 带参数的分配器扩展构造函数。
         *
         * @tparam Alloc 分配器类型
         * @tparam Args 参数的类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param alloc 要使用的分配器
         * @param args 用于构造元素的值
         */
        template <typename Alloc, typename... Args>
        constexpr tuple(std::allocator_arg_t, const Alloc &, Args &&...) noexcept {
        }
    };

    /**
     * \lang english
     * @brief Provides compile-time access to the type of a tuple element.
     *
     * @tparam Indices The index of the element
     * @tparam Tuple The tuple type
     *
     * \lang simp-chinese
     * @brief 提供对tuple元素类型的编译时访问。
     *
     * @tparam Indices 元素的索引
     * @tparam Tuple tuple类型
     */
    template <std::size_t Indices, typename Tuple>
    struct tuple_element {};

    /**
     * \lang english
     * @brief Specialization for tuple types.
     *
     * @tparam Indices The index of the element
     * @tparam Types The types contained in the tuple
     *
     * \lang simp-chinese
     * @brief tuple类型的特化。
     *
     * @tparam Indices 元素的索引
     * @tparam Types tuple中包含的类型
     */
    template <std::size_t Indices, typename... Types>
    struct tuple_element<Indices, tuple<Types...>> {
        using type = typename type_traits::other_trans::type_at<Indices, type_traits::other_trans::type_list<Types...>>::type;
    };

    /**
     * \lang english
     * @brief Alias template for tuple element type.
     *
     * @tparam Indicies The index of the element
     * @tparam Tuple The tuple type
     *
     * \lang simp-chinese
     * @brief tuple元素类型的别名模板。
     *
     * @tparam Indicies 元素的索引
     * @tparam Tuple tuple类型
     */
    template <std::size_t Indicies, typename Tuple>
    using tuple_element_t = typename tuple_element<Indicies, Tuple>::type;

    /**
     * \lang english
     * @brief Gets a reference to the element at index I in a mutable tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The tuple to access
     * @return Reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取可变tuple中索引I处元素的引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的tuple
     * @return 索引I处元素的引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &) noexcept;

    /**
     * \lang english
     * @brief Gets a const reference to the element at index I in a const tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The const tuple to access
     * @return Const reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取常量tuple中索引I处元素的常量引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的常量tuple
     * @return 索引I处元素的常量引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &) noexcept;

    /**
     * \lang english
     * @brief Gets an rvalue reference to the element at index I in a mutable tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The tuple to access (as rvalue)
     * @return Rvalue reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取可变tuple中索引I处元素的右值引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的tuple（作为右值）
     * @return 索引I处元素的右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&) noexcept;

    /**
     * \lang english
     * @brief Gets a const rvalue reference to the element at index I in a const tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The const tuple to access (as rvalue)
     * @return Const rvalue reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取常量tuple中索引I处元素的常量右值引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的常量tuple（作为右值）
     * @return 索引I处元素的常量右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&) noexcept;
}

namespace rainy::core::container::implements {
    template <typename Ty, typename Alloc, typename... Args>
    using _alloc_tag =
        std::integral_constant<int, !std::uses_allocator_v<Ty, Alloc> ? 0
                                    : type_traits::properties::is_constructible_v<Ty, std::allocator_arg_t, const Alloc &, Args...>
                                        ? 1
                                        : 2>;

    template <typename Head, typename Alloc, typename HeadArg>
    constexpr Head _make_head(const Alloc &, HeadArg &&head_arg, std::integral_constant<int, 0>) {
        return Head(utility::forward<HeadArg>(head_arg));
    }

    template <typename Head, typename Alloc, typename HeadArg>
    constexpr Head _make_head(const Alloc &alloc, HeadArg &&head_arg, std::integral_constant<int, 1>) {
        return Head(std::allocator_arg, alloc, utility::forward<HeadArg>(head_arg));
    }

    template <typename Head, typename Alloc, typename HeadArg>
    constexpr Head _make_head(const Alloc &alloc, HeadArg &&head_arg, std::integral_constant<int, 2>) {
        return Head(utility::forward<HeadArg>(head_arg), alloc);
    }

    template <typename Tuple, std::size_t Offset, typename... Types>
    class tuple_impl;

    template <typename Tuple, std::size_t Offset, typename Head>
    class tuple_impl<Tuple, Offset, Head> {
    protected:
        Head value;

        constexpr tuple_impl() : value() {
        }

        template <typename HeadArg>
        constexpr tuple_impl(HeadArg &&head_arg) : value(utility::forward<HeadArg>(head_arg)) { // NOLINT
        }

        template <typename Alloc>
        constexpr tuple_impl(std::allocator_arg_t, const Alloc &) : value() {
        }

        template <typename Alloc, typename HeadArg>
        constexpr tuple_impl(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg) :
            value(_make_head<Head>(alloc, utility::forward<HeadArg>(head_arg), _alloc_tag<Head, Alloc, HeadArg &&>{})) {
        }

        static constexpr Head &myhead(tuple_impl &t) noexcept {
            return t.value;
        }

        static constexpr const Head &myhead(const tuple_impl &t) noexcept {
            return t.value;
        }

        template <std::size_t I, typename Uty, typename Hd, typename... Rest>
        friend constexpr Hd &get_val(implements::tuple_impl<Uty, I, Hd, Rest...> &) noexcept;

        template <std::size_t I, typename Uty, typename Hd, typename... Rest>
        friend constexpr const Hd &get_val(const implements::tuple_impl<Uty, I, Hd, Rest...> &) noexcept;

        template <std::size_t I, typename Uty, typename Hd>
        friend constexpr Hd &get_val(implements::tuple_impl<Uty, I, Hd> &) noexcept;

        template <std::size_t I, typename Uty, typename Hd>
        friend constexpr const Hd &get_val(const implements::tuple_impl<Uty, I, Hd> &) noexcept;

        constexpr void swap_impl(tuple_impl &right) noexcept(type_traits::properties::is_nothrow_swappable_v<Head>) {
            using std::swap;
            swap(value, right.value);
        }
    };

    template <typename Tuple, std::size_t Offset, typename Head, typename... Rest>
    class tuple_impl<Tuple, Offset, Head, Rest...> : public tuple_impl<Tuple, Offset + 1, Rest...> {
    protected:
        using inherited = tuple_impl<Tuple, Offset + 1, Rest...>;
        Head value;

        constexpr tuple_impl() : inherited(), value() {
        }

        template <typename HeadArg, typename... RestArgs>
        constexpr tuple_impl(HeadArg &&head_arg, RestArgs &&...rest_args) :
            inherited(utility::forward<RestArgs>(rest_args)...), value(utility::forward<HeadArg>(head_arg)) {
        }

        template <typename Alloc>
        constexpr tuple_impl(std::allocator_arg_t, const Alloc &) : inherited(), value() {
        }

        template <typename Alloc, typename HeadArg, typename... TailArgs>
        constexpr tuple_impl(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg, TailArgs &&...tail_args) :
            inherited(tag, alloc, utility::forward<TailArgs>(tail_args)...),
            value(_make_head<Head>(alloc, utility::forward<HeadArg>(head_arg), _alloc_tag<Head, Alloc, HeadArg &&>{})) {
        }

        static constexpr Head &myhead(tuple_impl &t) noexcept {
            return t.value;
        }

        static constexpr const Head &myhead(const tuple_impl &t) noexcept {
            return t.value;
        }

        template <std::size_t I, typename Uty, typename Hd, typename... Rs>
        friend constexpr Hd &get_val(implements::tuple_impl<Uty, I, Hd, Rs...> &) noexcept;

        template <std::size_t I, typename Uty, typename Hd, typename... Rs>
        friend constexpr const Hd &get_val(const implements::tuple_impl<Uty, I, Hd, Rs...> &) noexcept;

        template <std::size_t I, typename Uty, typename Hd>
        friend constexpr Hd &get_val(implements::tuple_impl<Uty, I, Hd> &) noexcept;

        template <std::size_t I, typename Uty, typename Hd>
        friend constexpr const Hd &get_val(const implements::tuple_impl<Uty, I, Hd> &) noexcept;

        constexpr void swap_impl(tuple_impl &right) noexcept(type_traits::properties::is_nothrow_swappable_v<Head> &&
                                                             (type_traits::properties::is_nothrow_swappable_v<Rest> && ...)) {
            using std::swap;
            swap(value, right.value);
            this->inherited::swap_impl(static_cast<inherited &>(right));
        }
    };

    template <template <typename... Types> typename Tuple, typename TypeList>
    struct rest_tuple {};

    template <template <typename... Types> typename Tuple, typename... Types>
    struct rest_tuple<Tuple, type_traits::other_trans::type_list<Types...>> {
        using type = Tuple<Types...>;
    };

    template <std::size_t I, typename Uty, typename Head, typename... Rest>
    constexpr Head &get_val(implements::tuple_impl<Uty, I, Head, Rest...> &t) noexcept {
        return implements::tuple_impl<Uty, I, Head, Rest...>::myhead(t);
    }

    template <std::size_t I, typename Uty, typename Head, typename... Rest>
    constexpr const Head &get_val(const implements::tuple_impl<Uty, I, Head, Rest...> &t) noexcept {
        return implements::tuple_impl<Uty, I, Head, Rest...>::myhead(t);
    }
}

// NOLINTBEGIN
namespace std {
    template <std::size_t I, typename... Types>
    struct tuple_element<I, rainy::core::container::tuple<Types...>>
        : rainy::core::container::tuple_element<I, rainy::core::container::tuple<Types...>> {};

    template <typename... Types>
    struct tuple_size<rainy::core::container::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};
}
// NOLINTEND

namespace rainy::core::container {
    template <typename... Types>
    class tuple : public implements::tuple_impl<tuple<Types...>, 0, Types...> {
    public:
        using base = implements::tuple_impl<tuple<Types...>, 0, Types...>;
        using head_type = type_traits::other_trans::type_at<0, type_traits::other_trans::type_list<Types...>>::type;
        using rest_tuple = typename implements::rest_tuple<
            tuple, typename type_traits::other_trans::type_list_pop_front<type_traits::other_trans::type_list<Types...>>::type>::type;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        constexpr tuple() : base() {
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         */
        constexpr tuple(const tuple &) = default;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         */
        constexpr tuple(tuple &&) = default;

        /**
         * \lang english
         * @brief Constructs a tuple from the given arguments.
         *
         * @tparam Args Types of the arguments
         * @param args Values used to construct the tuple elements
         *
         * \lang simp-chinese
         * @brief 从给定参数构造tuple。
         *
         * @tparam Args 参数的类型
         * @param args 用于构造tuple元素的值
         */
        template <
            typename... Args,
            type_traits::other_trans::enable_if_t<
                sizeof...(Args) == sizeof...(Types) && (type_traits::properties::is_constructible_v<Types, Args> && ...), int> = 0>
        constexpr tuple(Args &&...args) : base(utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief Constructs a tuple by copying each element from another tuple.
         *
         * @tparam Args Types of the elements of the source tuple
         * @param right The source tuple to copy from
         *
         * \lang simp-chinese
         * @brief 通过从另一个tuple拷贝每个元素来构造tuple。
         *
         * @tparam Args 源tuple的元素类型
         * @param right 要拷贝的源tuple
         */
        template <
            typename... Args,
            type_traits::other_trans::enable_if_t<
                sizeof...(Args) == sizeof...(Types) && (type_traits::properties::is_constructible_v<Types, Args> && ...), int> = 0>
        constexpr tuple(const tuple<Args...> &right) :
            tuple(rest_in_place, right, type_traits::helper::index_sequence_for<Args...>{}) {
        }

        /**
         * \lang english
         * @brief Allocator-extended default constructor.
         *
         * @tparam Alloc Allocator type
         * @param tag allocator_arg_t tag for disambiguation
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 分配器扩展的默认构造函数。
         *
         * @tparam Alloc 分配器类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param alloc 要使用的分配器
         */
        template <typename Alloc>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc) : base(tag, alloc) {
        }

        /**
         * \lang english
         * @brief Allocator-extended constructor with arguments.
         *
         * @tparam Alloc Allocator type
         * @tparam HeadArg Type of the head argument
         * @tparam TailArgs Types of the remaining arguments
         * @param tag allocator_arg_t tag for disambiguation
         * @param alloc The allocator to use
         * @param head_arg Value for the first element
         * @param tail_args Values for the remaining elements
         *
         * \lang simp-chinese
         * @brief 带参数的分配器扩展构造函数。
         *
         * @tparam Alloc 分配器类型
         * @tparam HeadArg 头参数的类型
         * @tparam TailArgs 剩余参数的类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param alloc 要使用的分配器
         * @param head_arg 第一个元素的值
         * @param tail_args 剩余元素的值
         */
        template <typename Alloc, typename HeadArg, typename... TailArgs>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg, TailArgs &&...tail_args) :
            base(tag, alloc, utility::forward<HeadArg>(head_arg), utility::forward<TailArgs>(tail_args)...) {
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @param right The right tuple to copy from
         * @return Reference to this tuple
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @param right 要拷贝的另一个tuple
         * @return 此tuple的引用
         */
        constexpr tuple &operator=(const tuple &right) {
            copy_assign_impl(right, type_traits::helper::index_sequence_for<Types...>{});
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right The right tuple to move from
         * @return Reference to this tuple
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 要移动的另一个tuple
         * @return 此tuple的引用
         */
        constexpr tuple &operator=(tuple &&right) noexcept((type_traits::properties::is_nothrow_move_assignable_v<Types> && ...)) {
            move_assign_impl(right, type_traits::helper::index_sequence_for<Types...>{});
            return *this;
        }

        /**
         * \lang english
         * @brief Converting assignment from another tuple.
         *
         * @tparam RightHead Type of the first element of the right tuple
         * @tparam RightRest Types of the remaining elements of the right tuple
         * @param right The right tuple to copy from
         * @return Reference to this tuple
         *
         * \lang simp-chinese
         * @brief 从另一个tuple的转换赋值。
         *
         * @tparam RightHead 另一个tuple的第一个元素类型
         * @tparam RightRest 另一个tuple的剩余元素类型
         * @param right 要拷贝的另一个tuple
         * @return 此tuple的引用
         */
        template <typename... UTypes,
                  type_traits::other_trans::enable_if_t<sizeof...(UTypes) == sizeof...(Types) &&
                                                            (type_traits::properties::is_assignable_v<Types &, const UTypes &> && ...),
                                                        int> = 0>
        constexpr tuple &operator=(const tuple<UTypes...> &right) {
            copy_assign_impl(right, type_traits::helper::index_sequence_for<Types...>{});
            return *this;
        }

        /**
         * \lang english
         * @brief Swaps the contents with another tuple.
         *
         * @param right The right tuple to swap with
         *
         * \lang simp-chinese
         * @brief 与另一个tuple交换内容。
         *
         * @param right 要交换的另一个tuple
         */
        constexpr void swap(tuple &right) noexcept((type_traits::properties::is_nothrow_swappable_v<Types> && ...)) {
            base::swap_impl(right);
        }

        /**
         * \lang english
         * @brief Gets a reference to the element at index I.
         *
         * @tparam I The index of the element to access
         * @return Reference to the element at index I
         *
         * \lang simp-chinese
         * @brief 获取索引I处元素的引用。
         *
         * @tparam I 要访问的元素的索引
         * @return 索引I处元素的引用
         */
        template <std::size_t I>
        constexpr tuple_element_t<I, tuple> &get() noexcept {
            static_assert(I < sizeof...(Types), "Index out of bounds");
            return implements::get_val<I>(*this);
        }

        /**
         * \lang english
         * @brief Gets a const reference to the element at index I.
         *
         * @tparam I The index of the element to access
         * @return Const reference to the element at index I
         *
         * \lang simp-chinese
         * @brief 获取索引I处元素的常量引用。
         *
         * @tparam I 要访问的元素的索引
         * @return 索引I处元素的常量引用
         */
        template <std::size_t I>
        constexpr const tuple_element_t<I, tuple> &get() const noexcept {
            static_assert(I < sizeof...(Types), "Index out of bounds");
            return implements::get_val<I>(*this);
        }

        /**
         * \lang english
         * @brief Concatenates this tuple with another tuple.
         *
         * @tparam RightHead Type of the first element of the right tuple
         * @tparam RightRest Types of the remaining elements of the right tuple
         * @param right The right tuple to concatenate
         * @return A new tuple containing all elements of this tuple followed by all elements of right
         *
         * \lang simp-chinese
         * @brief 将当前tuple与另一个tuple连接。
         *
         * @tparam RightHead 另一个tuple的第一个元素类型
         * @tparam RightRest 另一个tuple的剩余元素类型
         * @param right 要连接的另一个tuple
         * @return 包含当前tuple所有元素后跟other所有元素的新tuple
         */
        template <typename RightHead, typename... RightRest>
        constexpr auto concat(const tuple<RightHead, RightRest...> &right) const {
            return concat_impl(right, type_traits::helper::make_index_sequence<sizeof...(Types)>{},
                               type_traits::helper::make_index_sequence<sizeof...(RightRest)>{});
        }

        /**
         * \lang english
         * @brief Concatenates this tuple with another tuple (rvalue version).
         *
         * @tparam RightHead Type of the first element of the right tuple
         * @tparam RightRest Types of the remaining elements of the right tuple
         * @param right The right tuple to concatenate
         * @return A new tuple containing all elements of this tuple followed by all elements of right
         *
         * \lang simp-chinese
         * @brief 将当前tuple与另一个tuple连接（右值版本）。
         *
         * @tparam RightHead 另一个tuple的第一个元素类型
         * @tparam RightRest 另一个tuple的剩余元素类型
         * @param right 要连接的另一个tuple
         * @return 包含当前tuple所有元素后跟other所有元素的新tuple
         */
        template <typename... UTypes>
        constexpr auto concat(tuple<Types...> &right) && {
            return concat_impl(utility::move(right), type_traits::helper::make_index_sequence<sizeof...(Types)>{},
                               type_traits::helper::make_index_sequence<sizeof...(UTypes)>{});
        }

        /**
         * \lang english
         * @brief Returns a new tuple with the first N elements removed.
         *
         * @tparam N Number of elements to drop
         * @return A tuple containing elements from index N to end
         *
         * \lang simp-chinese
         * @brief 返回移除了前N个元素的新tuple。
         *
         * @tparam N 要丢弃的元素数量
         * @return 包含从索引N到末尾的元素的新tuple
         */
        template <std::size_t N>
        constexpr auto drop() const & {
            static_assert(N <= sizeof...(Types), "Cannot drop more elements than tuple size");
            if constexpr (N == 0) {
                return tuple(*this);
            } else if constexpr (N == sizeof...(Types)) {
                return tuple<>{};
            } else {
                return drop_impl<N>(type_traits::helper::make_index_sequence<sizeof...(Types) - N>{});
            }
        }

        /**
         * \lang english
         * @brief Returns a new tuple with the first N elements removed.
         *
         * @tparam N Number of elements to drop
         * @return A tuple containing elements from index N to end
         *
         * \lang simp-chinese
         * @brief 返回移除了前N个元素的新tuple（右值版本）。
         *
         * @tparam N 要丢弃的元素数量
         * @return 包含从索引N到末尾的元素的新tuple
         */
        template <std::size_t N>
        constexpr auto drop() && {
            static_assert(N <= sizeof...(Types), "Cannot drop more elements than tuple size");
            if constexpr (N == 0) {
                return utility::move(*this);
            } else if constexpr (N == sizeof...(Types)) {
                return tuple<>{};
            } else {
                return const_cast<const tuple &>(*this).template drop<N>();
            }
        }

        /**
         * \lang english
         * @brief Returns a new tuple with the first N elements.
         *
         * @tparam N Number of elements to take
         * @return A tuple containing the first N elements
         *
         * \lang simp-chinese
         * @brief 返回包含前N个元素的新tuple。
         *
         * @tparam N 要获取的元素数量
         * @return 包含前N个元素的新tuple
         */
        template <std::size_t N>
        constexpr auto take() const & {
            static_assert(N <= sizeof...(Types), "Cannot take more elements than tuple size");
            if constexpr (N == 0) {
                return tuple<>{};
            } else {
                return take_helper<N>(type_traits::helper::make_index_sequence<N>{});
            }
        }

        /**
         * \lang english
         * @brief Returns a new tuple with the first N elements.
         *
         * @tparam N Number of elements to take
         * @return A tuple containing the first N elements
         *
         * \lang simp-chinese
         * @brief 返回包含前N个元素的新tuple（右值版本）。
         *
         * @tparam N 要获取的元素数量
         * @return 包含前N个元素的新tuple
         */

        template <std::size_t N>
        constexpr auto take() && {
            static_assert(N <= sizeof...(Types), "Cannot take more elements than tuple size");
            if constexpr (N == 0) {
                return tuple<>{};
            } else {
                return take_helper<N>(type_traits::helper::make_index_sequence<N>{});
            }
        }

        /**
         * \lang english
         * @brief Returns a subtuple containing elements from index Start to End (exclusive).
         *
         * @tparam Start Starting index (inclusive)
         * @tparam End Ending index (exclusive)
         * @return A tuple containing elements in the range [Start, End)
         *
         * \lang simp-chinese
         * @brief 返回包含从索引Start到End（不包含）的元素的新tuple。
         *
         * @tparam Start 起始索引（包含）
         * @tparam End 结束索引（不包含）
         * @return 包含范围[Start, End)内元素的新tuple
         */
        template <std::size_t Start, std::size_t End>
        constexpr auto subtuple() const & {
            static_assert(Start <= End, "Start must be less than or equal to End");
            static_assert(End <= sizeof...(Types), "End must not exceed tuple size");
            if constexpr (Start == End) {
                return tuple<>{};
            } else {
                return drop<Start>().template take<End - Start>();
            }
        }

        /**
         * \lang english
         * @brief Returns a subtuple containing elements from index Start to End (exclusive).
         *
         * @tparam Start Starting index (inclusive)
         * @tparam End Ending index (exclusive)
         * @return A tuple containing elements in the range [Start, End)
         *
         * \lang simp-chinese
         * @brief 返回包含从索引Start到End（不包含）的元素的新tuple（右值版本）。
         *
         * @tparam Start 起始索引（包含）
         * @tparam End 结束索引（不包含）
         * @return 包含范围[Start, End)内元素的新tuple
         */
        template <std::size_t Start, std::size_t End>
        constexpr auto subtuple() && {
            static_assert(Start <= End, "Start must be less than or equal to End");
            static_assert(End <= sizeof...(Types), "End must not exceed tuple size");
            if constexpr (Start == End) {
                return tuple<>{};
            } else {
                return utility::move(*this).template drop<Start>().template take<End - Start>();
            }
        }

        /**
         * \lang english
         * @brief Applies a function to the elements of the tuple.
         *
         * @tparam Func Callable type
         * @param func The function to apply
         * @return Result of the function call
         *
         * \lang simp-chinese
         * @brief 将函数应用于tuple的元素。
         *
         * @tparam Func 可调用类型
         * @param func 要应用的函数
         * @return 函数调用的结果
         */
        template <typename Func>
        constexpr auto apply(Func &&func) const & {
            return apply_impl(utility::forward<Func>(func), type_traits::helper::make_index_sequence<sizeof...(Types)>{});
        }

        /**
         * \lang english
         * @brief Applies a function to the elements of the tuple.
         *
         * @tparam Func Callable type
         * @param func The function to apply
         * @return Result of the function call
         *
         * \lang simp-chinese
         * @brief 将函数应用于tuple的元素（右值版本）。
         *
         * @tparam Func 可调用类型
         * @param func 要应用的函数
         * @return 函数调用的结果
         */
        template <typename Func>
        constexpr auto apply(Func &&func) && {
            return apply_impl(utility::forward<Func>(func), type_traits::helper::make_index_sequence<sizeof...(Types)>{});
        }

        /**
         * \lang english
         * @brief Equality comparison between two tuples.
         *
         * @tparam TTypes Types of the left tuple
         * @tparam UTypes Types of the right tuple
         * @param left The left tuple
         * @param right The right tuple
         * @return true if the tuples are element-wise equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 两个tuple之间的相等比较。
         *
         * @tparam TTypes 左tuple的类型
         * @tparam UTypes 右tuple的类型
         * @param left 左tuple
         * @param right 右tuple
         * @return 如果tuple逐元素相等则为true，否则为false
         */
        template <typename... TTypes, typename... UTypes>
        friend inline constexpr bool operator==(const tuple<TTypes...> &left, const tuple<UTypes...> &right);

    private:
        struct rest_in_place_t {};
        static constexpr rest_in_place_t rest_in_place{};

        struct take_in_place_t {};
        static constexpr take_in_place_t take_in_place{};

        template <typename RightHead, typename... RightRest, std::size_t... Is>
        constexpr tuple(rest_in_place_t, const tuple<RightHead, RightRest...> &right, type_traits::helper::index_sequence<Is...>) :
            tuple(right.template get<Is>()...) {
        }

        template <typename HeadArg, typename... RestArgs>
        constexpr rain_fn take_in_tuple(take_in_place_t, HeadArg &&head_arg, RestArgs &&...args) const & {
            return tuple<type_traits::other_trans::decay_t<HeadArg>, type_traits::other_trans::decay_t<RestArgs>...>(
                utility::forward<HeadArg>(head_arg), utility::forward<RestArgs>(args)...);
        }

        template <typename HeadArg, typename... RestArgs>
        constexpr rain_fn take_in_tuple(take_in_place_t, HeadArg &&head_arg, RestArgs &&...args) && {
            return tuple<type_traits::other_trans::decay_t<HeadArg>, type_traits::other_trans::decay_t<RestArgs>...>(
                utility::move(head_arg), utility::move(args)...);
        }

        template <std::size_t N, std::size_t... Is>
        constexpr auto take_helper(type_traits::helper::index_sequence<Is...>) const {
            return take_in_tuple(take_in_place, this->template get<Is>()...);
        }

        template <std::size_t N, std::size_t... Is>
        constexpr auto take_helper_rvalue(type_traits::helper::index_sequence<Is...>) && {
            return take_in_tuple(take_in_place, utility::move(this->template get<Is>())...);
        }

        template <typename OtherTuple, std::size_t... Is, std::size_t... Js>
        constexpr auto concat_impl(const OtherTuple &right, type_traits::helper::index_sequence<Is...>,
                                   type_traits::helper::index_sequence<Js...>) const {
            return tuple<decltype(get<Is>())..., decltype(right.template get<Js>())...>(get<Is>()..., right.template get<Js>()...);
        }

        template <typename OtherTuple, std::size_t... Is, std::size_t... Js>
        constexpr auto concat_impl(OtherTuple &&right, type_traits::helper::index_sequence<Is...>,
                                   type_traits::helper::index_sequence<Js...>) && {
            return tuple<decltype(utility::move(get<Is>()))..., decltype(utility::move(right.template get<Js>()))...>(
                utility::move(get<Is>())..., utility::move(right.template get<Js>())...);
        }

        template <typename Func, std::size_t... Is>
        constexpr auto apply_impl(Func &&func, type_traits::helper::index_sequence<Is...>) const & {
            return utility::forward<Func>(func)(get<Is>()...);
        }

        template <typename Func, std::size_t... Is>
        constexpr auto apply_impl(Func &&func, type_traits::helper::index_sequence<Is...>) && {
            return utility::forward<Func>(func)(utility::move(get<Is>())...);
        }

        template <typename OtherTuple, std::size_t... Is>
        constexpr void copy_assign_impl(OtherTuple &&right, type_traits::helper::index_sequence<Is...>) {
            ((get<Is>() = right.template get<Is>(), 0), ...);
        }

        template <typename OtherTuple, std::size_t... Is>
        constexpr void move_assign_impl(OtherTuple &&right, type_traits::helper::index_sequence<Is...>) {
            ((get<Is>() = utility::move(right.template get<Is>()), 0), ...);
        }

        template <std::size_t N, std::size_t... Is>
        constexpr auto drop_impl(type_traits::helper::index_sequence<Is...>) const {
            return tuple<type_traits::other_trans::decay_t<decltype(this->template get<N + Is>())>...>(
                this->template get<N + Is>()...);
        }
    };

    /**
     * \lang english
     * @brief Gets a reference to the element at index I in a mutable tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The tuple to access
     * @return Reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取可变tuple中索引I处元素的引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的tuple
     * @return 索引I处元素的引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    /**
     * \lang english
     * @brief Gets a const reference to the element at index I in a const tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The const tuple to access
     * @return Const reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取常量tuple中索引I处元素的常量引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的常量tuple
     * @return 索引I处元素的常量引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    /**
     * \lang english
     * @brief Gets an rvalue reference to the element at index I in a mutable tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The tuple to access (as rvalue)
     * @return Rvalue reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取可变tuple中索引I处元素的右值引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的tuple（作为右值）
     * @return 索引I处元素的右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&t) noexcept { // NOLINT
        return utility::move(t.template get<I>());
    }

    /**
     * \lang english
     * @brief Gets a const rvalue reference to the element at index I in a const tuple.
     *
     * @tparam I The index of the element to access
     * @tparam Types The types contained in the tuple
     * @param t The const tuple to access (as rvalue)
     * @return Const rvalue reference to the element at index I
     *
     * \lang simp-chinese
     * @brief 获取常量tuple中索引I处元素的常量右值引用。
     *
     * @tparam I 要访问的元素的索引
     * @tparam Types tuple中包含的类型
     * @param t 要访问的常量tuple（作为右值）
     * @return 索引I处元素的常量右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&t) noexcept {
        return utility::move(t.template get<I>());
    }

    /**
     * \lang english
     * @brief Drops the first N elements from a tuple.
     *
     * @tparam N Number of elements to drop
     * @tparam Types Types in the tuple
     * @param t The tuple to drop from
     * @return A new tuple without the first N elements
     *
     * \lang simp-chinese
     * @brief 从tuple中丢弃前N个元素。
     *
     * @tparam N 要丢弃的元素数量
     * @tparam Types tuple中的类型
     * @param t 要丢弃元素的tuple
     * @return 不包含前N个元素的新tuple
     */
    template <std::size_t N, typename... Types>
    constexpr auto drop(const tuple<Types...> &t) {
        return t.template drop<N>();
    }

    /**
     * \lang english
     * @brief Drops the first N elements from a tuple (rvalue version).
     *
     * @tparam N Number of elements to drop
     * @tparam Types Types in the tuple
     * @param t The tuple to drop from
     * @return A new tuple without the first N elements
     *
     * \lang simp-chinese
     * @brief 从tuple中丢弃前N个元素（右值版本）。
     *
     * @tparam N 要丢弃的元素数量
     * @tparam Types tuple中的类型
     * @param t 要丢弃元素的tuple
     * @return 不包含前N个元素的新tuple
     */
    template <std::size_t N, typename... Types>
    constexpr auto drop(tuple<Types...> &&t) {
        return utility::move(t).template drop<N>();
    }

    /**
     * \lang english
     * @brief Takes the first N elements from a tuple.
     *
     * @tparam N Number of elements to take
     * @tparam Types Types in the tuple
     * @param t The tuple to take from
     * @return A new tuple with the first N elements
     *
     * \lang simp-chinese
     * @brief 从tuple中获取前N个元素。
     *
     * @tparam N 要获取的元素数量
     * @tparam Types tuple中的类型
     * @param t 要获取元素的tuple
     * @return 包含前N个元素的新tuple
     */
    template <std::size_t N, typename... Types>
    constexpr auto take(const tuple<Types...> &t) {
        return t.template take<N>();
    }

    /**
     * \lang english
     * @brief Takes the first N elements from a tuple (rvalue version).
     *
     * @tparam N Number of elements to take
     * @tparam Types Types in the tuple
     * @param t The tuple to take from
     * @return A new tuple with the first N elements
     *
     * \lang simp-chinese
     * @brief 从tuple中获取前N个元素（右值版本）。
     *
     * @tparam N 要获取的元素数量
     * @tparam Types tuple中的类型
     * @param t 要获取元素的tuple
     * @return 包含前N个元素的新tuple
     */
    template <std::size_t N, typename... Types>
    constexpr auto take(tuple<Types...> &&t) {
        return utility::move(t).template take<N>();
    }

    /**
     * \lang english
     * @brief Extracts a subtuple from a tuple.
     *
     * @tparam Start Starting index (inclusive)
     * @tparam End Ending index (exclusive)
     * @tparam Types Types in the tuple
     * @param t The tuple to extract from
     * @return A tuple containing elements in the range [Start, End)
     *
     * \lang simp-chinese
     * @brief 从tuple中提取子tuple。
     *
     * @tparam Start 起始索引（包含）
     * @tparam End 结束索引（不包含）
     * @tparam Types tuple中的类型
     * @param t 要提取的tuple
     * @return 包含范围[Start, End)内元素的新tuple
     */
    template <std::size_t Start, std::size_t End, typename... Types>
    constexpr auto subtuple(const tuple<Types...> &t) {
        return t.template subtuple<Start, End>();
    }

    /**
     * \lang english
     * @brief Extracts a subtuple from a tuple (rvalue version).
     *
     * @tparam Start Starting index (inclusive)
     * @tparam End Ending index (exclusive)
     * @tparam Types Types in the tuple
     * @param t The tuple to extract from
     * @return A tuple containing elements in the range [Start, End)
     *
     * \lang simp-chinese
     * @brief 从tuple中提取子tuple（右值版本）。
     *
     * @tparam Start 起始索引（包含）
     * @tparam End 结束索引（不包含）
     * @tparam Types tuple中的类型
     * @param t 要提取的tuple
     * @return 包含范围[Start, End)内元素的新tuple
     */
    template <std::size_t Start, std::size_t End, typename... Types>
    constexpr auto subtuple(tuple<Types...> &&t) {
        return utility::move(t).template subtuple<Start, End>();
    }

    /**
     * \lang english
     * @brief Applies a function to the elements of a tuple.
     *
     * @tparam Func Callable type
     * @tparam Types Types in the tuple
     * @param func The function to apply
     * @param t The tuple to apply to
     * @return Result of the function call
     *
     * \lang simp-chinese
     * @brief 将函数应用于tuple的元素。
     *
     * @tparam Func 可调用类型
     * @tparam Types tuple中的类型
     * @param func 要应用的函数
     * @param t 要应用函数的tuple
     * @return 函数调用的结果
     */
    template <typename Func, typename... Types>
    constexpr auto apply(Func &&func, const tuple<Types...> &t) {
        return t.apply(utility::forward<Func>(func));
    }

    /**
     * \lang english
     * @brief Applies a function to the elements of a tuple (rvalue version).
     *
     * @tparam Func Callable type
     * @tparam Types Types in the tuple
     * @param func The function to apply
     * @param t The tuple to apply to
     * @return Result of the function call
     *
     * \lang simp-chinese
     * @brief 将函数应用于tuple的元素（右值版本）。
     *
     * @tparam Func 可调用类型
     * @tparam Types tuple中的类型
     * @param func 要应用的函数
     * @param t 要应用函数的tuple
     * @return 函数调用的结果
     */
    template <typename Func, typename... Types>
    constexpr auto apply(Func &&func, tuple<Types...> &&t) {
        return utility::move(t).apply(utility::forward<Func>(func));
    }

    /**
     * \lang english
     * @brief Creates a tuple by deducing the types of the arguments and decaying them.
     *
     * @tparam Types The types of the arguments
     * @param args The values to store in the tuple
     * @return A tuple containing the decayed copies of the arguments
     *
     * \lang simp-chinese
     * @brief 通过推导参数类型并退化它们来创建tuple。
     *
     * @tparam Types 参数的类型
     * @param args 要存储在tuple中的值
     * @return 包含参数退化副本的tuple
     */
    template <typename... Types>
    constexpr tuple<type_traits::other_trans::decay_t<Types>...> make_tuple(Types &&...args) {
        return tuple<type_traits::other_trans::decay_t<Types>...>(utility::forward<Types>(args)...);
    }

    /**
     * \lang english
     * @brief Creates a tuple of references to the arguments.
     *
     * @tparam Types The types of the arguments
     * @param args The values to create references to
     * @return A tuple containing references to the arguments
     *
     * \lang simp-chinese
     * @brief 创建参数引用的tuple。
     *
     * @tparam Types 参数的类型
     * @param args 要创建引用的值
     * @return 包含参数引用的tuple
     */
    template <typename... Types>
    constexpr tuple<Types &&...> forward_as_tuple(Types &&...args) noexcept {
        return tuple<Types &&...>(utility::forward<Types>(args)...);
    }

    /**
     * \lang english
     * @brief Swaps two tuples.
     *
     * @tparam Types The types contained in the tuples
     * @param left The first tuple
     * @param right The second tuple
     *
     * \lang simp-chinese
     * @brief 交换两个tuple。
     *
     * @tparam Types tuple中包含的类型
     * @param left 第一个tuple
     * @param right 第二个tuple
     */
    template <typename... Types>
    constexpr void swap(tuple<Types...> &left, tuple<Types...> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Provides the number of elements in a tuple.
     *
     * @tparam Tuple The tuple type
     *
     * \lang simp-chinese
     * @brief 提供tuple中的元素数量。
     *
     * @tparam Tuple tuple类型
     */
    template <typename Tuple>
    struct tuple_size {
        static RAINY_INLINE_CONSTEXPR std::size_t value = 0;
    };

    /**
     * \lang english
     * @brief Specialization for tuple types.
     *
     * @tparam Args The types contained in the tuple
     *
     * \lang simp-chinese
     * @brief tuple类型的特化。
     *
     * @tparam Args tuple中包含的类型
     */
    template <typename... Args>
    struct tuple_size<tuple<Args...>> {
        static RAINY_INLINE_CONSTEXPR std::size_t value = sizeof...(Args);
    };

    /**
     * \lang english
     * @brief Variable template for tuple size.
     *
     * @tparam Tuple The tuple type
     *
     * \lang simp-chinese
     * @brief tuple大小的变量模板。
     *
     * @tparam Tuple tuple类型
     */
    template <typename Tuple>
    static inline constexpr std::size_t tuple_size_v = tuple_size<Tuple>::value;

    /**
     * \lang english
     * @brief Concatenates multiple tuples into a single tuple.
     *
     * @tparam First Type of the first tuple
     * @tparam Rest Types of the remaining tuples
     * @param first The first tuple to concatenate
     * @param rest The remaining tuples to concatenate
     * @return A tuple containing all elements from all input tuples in order
     *
     * \lang simp-chinese
     * @brief 将多个tuple连接成一个单独的tuple。
     *
     * @tparam First 第一个tuple的类型
     * @tparam Rest 剩余tuple的类型
     * @param first 要连接的第一个tuple
     * @param rest 要连接的剩余tuple
     * @return 包含所有输入tuple所有元素按顺序排列的新tuple
     */
    template <typename First, typename... Rest>
    constexpr auto tuple_cat(First &&first, Rest &&...rest) {
        if constexpr (sizeof...(Rest) == 0) {
            return utility::forward<First>(first);
        } else {
            return utility::forward<First>(first).concat(tuple_cat(utility::forward<Rest>(rest)...));
        }
    }

    /**
     * \lang english
     * @brief Concatenates zero tuples, producing an empty tuple.
     *
     * @return An empty tuple
     *
     * \lang simp-chinese
     * @brief 连接零个tuple，产生一个空tuple。
     *
     * @return 空tuple
     */
    constexpr auto tuple_cat() {
        return tuple<>{};
    }
}

namespace std {
    using rainy::core::container::get;
}

namespace rainy::utility::container {
    using rainy::core::container::forward_as_tuple;
    using rainy::core::container::get;
    using rainy::core::container::make_tuple;

    using rainy::core::container::tuple_element;
    using rainy::core::container::tuple_element_t;
    using rainy::core::container::tuple_size;
    using rainy::core::container::tuple_size_v;
    using rainy::core::container::tuple;
}

namespace rainy::utility::container {
    using rainy::core::container::apply;
    using rainy::core::container::drop;
    using rainy::core::container::subtuple;
    using rainy::core::container::take;
    using rainy::core::container::tuple_cat;
}

namespace rainy::core::container::implements {
    template <typename... TTypes, typename... UTypes, std::size_t... Is>
    constexpr bool compare_tuple_elements(const tuple<TTypes...> &left, const tuple<UTypes...> &right,
                                          type_traits::helper::index_sequence<Is...>) {
        return ((get<Is>(left) == get<Is>(right)) && ...);
    }
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Equality comparison between two tuples.
     *
     * @tparam TTypes Types of the left tuple
     * @tparam UTypes Types of the right tuple
     * @param left The left tuple
     * @param right The right tuple
     * @return true if the tuples are element-wise equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个tuple之间的相等比较。
     *
     * @tparam TTypes 左tuple的类型
     * @tparam UTypes 右tuple的类型
     * @param left 左tuple
     * @param right 右tuple
     * @return 如果tuple逐元素相等则为true，否则为false
     */
    template <typename... TTypes, typename... UTypes>
    constexpr bool operator==(const tuple<TTypes...> &left, const tuple<UTypes...> &right) {
        if constexpr (sizeof...(TTypes) != sizeof...(UTypes)) {
            return false;
        } else if constexpr (sizeof...(TTypes) == 0) {
            return true;
        } else {
            return implements::compare_tuple_elements(left, right, type_traits::helper::make_index_sequence<sizeof...(TTypes)>{});
        }
    }
}


namespace rainy::container {
    using rainy::core::container::tuple;
    using rainy::core::container::tuple_element;
    using rainy::core::container::tuple_element_t;
    using rainy::core::container::tuple_size;
    using rainy::core::container::tuple_size_v;
    using rainy::core::container::get;
    using rainy::core::container::drop;
    using rainy::core::container::take;
    using rainy::core::container::subtuple;
    using rainy::core::container::apply;
    using rainy::core::container::make_tuple;
    using rainy::core::container::forward_as_tuple;
    using rainy::core::container::swap;
    using rainy::core::container::tuple_cat;
    using rainy::core::container::operator==;
}


#endif
