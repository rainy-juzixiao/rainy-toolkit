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
#ifndef RAINY_CORE_IMPLEMENTS_TUPLE_HPP
#define RAINY_CORE_IMPLEMENTS_TUPLE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/type_list.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/core/type_traits/decay.hpp>
#include <rainy/core/type_traits/logical.hpp>
#include <rainy/core/type_traits/properties.hpp>

namespace rainy::foundation::container {
    /**
     * @brief A fixed-size collection of heterogeneous values.
     *        固定大小的异构值集合。
     *
     * @tparam Types The types of elements contained in the tuple
     *               tuple中包含的元素类型
     */
    template <typename... Types>
    class tuple;

    /**
     * @brief Specialization for empty tuple.
     *        空tuple的特化。
     */
    template <>
    class tuple<> {
    public:
        /**
         * @brief Default constructor.
         *        默认构造函数。
         */
        constexpr tuple() noexcept = default;

        /**
         * @brief Copy constructor.
         *        拷贝构造函数。
         */
        constexpr tuple(const tuple &) = default;

        /**
         * @brief Move constructor.
         *        移动构造函数。
         */
        constexpr tuple(tuple &&) = default;

        /**
         * @brief Copy assignment operator.
         *        拷贝赋值运算符。
         */
        constexpr tuple &operator=(const tuple &) = default;

        /**
         * @brief Move assignment operator.
         *        移动赋值运算符。
         */
        constexpr tuple &operator=(tuple &&) = default;

        /**
         * @brief Swaps two empty tuples (no-op).
         *        交换两个空tuple（无操作）。
         *
         * @param other The other tuple to swap with
         *              要交换的另一个tuple
         */
        static constexpr void swap(tuple &) noexcept {
        }
    };

    /**
     * @brief Provides compile-time access to the type of a tuple element.
     *        提供对tuple元素类型的编译时访问。
     *
     * @tparam Indices The index of the element
     *                 元素的索引
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <std::size_t Indices, typename Tuple>
    struct tuple_element {};

    /**
     * @brief Specialization for tuple types.
     *        tuple类型的特化。
     *
     * @tparam Indices The index of the element
     *                 元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     */
    template <std::size_t Indices, typename... Types>
    struct tuple_element<Indices, tuple<Types...>> {
        using type = typename type_traits::other_trans::type_at<Indices, type_traits::other_trans::type_list<Types...>>::type;
    };

    /**
     * @brief Alias template for tuple element type.
     *        tuple元素类型的别名模板。
     *
     * @tparam Indicies The index of the element
     *                  元素的索引
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <std::size_t Indicies, typename Tuple>
    using tuple_element_t = typename tuple_element<Indicies, Tuple>::type;

    /**
     * @brief Gets a reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access
     *          要访问的tuple
     * @return Reference to the element at index I
     *         索引I处元素的引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &) noexcept;

    /**
     * @brief Gets a const reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access
     *          要访问的常量tuple
     * @return Const reference to the element at index I
     *         索引I处元素的常量引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &) noexcept;

    /**
     * @brief Gets an rvalue reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access (as rvalue)
     *          要访问的tuple（作为右值）
     * @return Rvalue reference to the element at index I
     *         索引I处元素的右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&) noexcept;

    /**
     * @brief Gets a const rvalue reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access (as rvalue)
     *          要访问的常量tuple（作为右值）
     * @return Const rvalue reference to the element at index I
     *         索引I处元素的常量右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&) noexcept;
}

namespace rainy::foundation::container::implements {
    template <typename Ty>
    struct tuple_val {
        constexpr tuple_val() : value() {
        }

        constexpr tuple_val(const tuple_val &other) : value(other.value) {
        }

        constexpr tuple_val(tuple_val &&other) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Ty>) :
            value(utility::move(other.value)) {
        }

        constexpr tuple_val &operator=(const tuple_val &other) {
            if (this != &other) {
                value = other.value;
            }
            return *this;
        }

        constexpr tuple_val &operator=(tuple_val &&other) noexcept(type_traits::type_properties::is_nothrow_move_assignable_v<Ty>) {
            if (this != &other) {
                value = utility::move(other.value);
            }
            return *this;
        }

        template <typename Other,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, Other &&> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Other>, tuple_val> &&
                          !type_traits::type_relations::is_base_of_v<tuple_val, type_traits::other_trans::decay_t<Other>>,
                      int> = 0>
        constexpr explicit tuple_val(Other &&arg) : value(utility::forward<Other>(arg)) { // NOLINT
        }

        template <typename Alloc, typename... Args, type_traits::other_trans::enable_if_t<!std::uses_allocator_v<Ty, Alloc>, int> = 0>
        constexpr tuple_val(std::allocator_arg_t, const Alloc &, Args &&...args) : value(utility::forward<Args>(args)...) {
        }

        template <typename Alloc, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          std::uses_allocator<Ty, Alloc>, std::is_constructible<Ty, std::allocator_arg_t, const Alloc &, Args...>>,
                      int> = 0>
        constexpr tuple_val(std::allocator_arg_t, const Alloc &allocator, Args &&...args) :
            value(std::allocator_arg, allocator, utility::forward<Args>(args)...) {
        }

        template <
            typename Alloc, typename... Args,
            type_traits::other_trans::enable_if_t<
                type_traits::logical_traits::conjunction_v<
                    std::uses_allocator<Ty, Alloc>,
                    type_traits::logical_traits::negation<std::is_constructible<Ty, std::allocator_arg_t, const Alloc &, Args...>>>,
                int> = 0>
        constexpr tuple_val(std::allocator_arg_t, const Alloc &allocator, Args &&...args) :
            value(utility::forward<Args>(args)..., allocator) {
        }

        Ty value;
    };

    template <std::size_t I, typename Ty, std::size_t Index>
    struct tuple_leaf_index : tuple_val<Ty> {
        constexpr tuple_leaf_index() : tuple_val<Ty>() {
        }

        template <typename Uty>
        constexpr tuple_leaf_index(Uty &&arg) : tuple_val<Ty>(utility::forward<Uty>(arg)) { // NOLINT
        }

        template <typename Alloc, typename... Args>
        constexpr tuple_leaf_index(const Alloc &alloc, std::allocator_arg_t tag, Args &&...args) :
            tuple_val<Ty>(tag, alloc, utility::forward<Args>(args)...) {
        }
    };
}

// NOLINTBEGIN
namespace std {
    template <std::size_t I, typename... Types>
    struct tuple_element<I, rainy::foundation::container::tuple<Types...>>
        : rainy::foundation::container::tuple_element<I, rainy::foundation::container::tuple<Types...>> {};

    template <typename... Types>
    struct tuple_size<rainy::foundation::container::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};
}
// NOLINTEND

namespace rainy::foundation::container {
    /**
     * @brief Primary template for tuple with at least one element.
     *        至少包含一个元素的tuple主模板。
     *
     * @tparam Head Type of the first element
     *              第一个元素的类型
     * @tparam Rest Types of the remaining elements
     *              剩余元素的类型
     */
    template <typename Head, typename... Rest>
    class tuple<Head, Rest...> : private implements::tuple_leaf_index<0, Head, sizeof...(Rest)>, private tuple<Rest...> {
    public:
        using head_base = implements::tuple_leaf_index<0, Head, sizeof...(Rest)>;
        using rest_base = tuple<Rest...>;

        /**
         * @brief Default constructor.
         *        默认构造函数。
         */
        constexpr tuple() : head_base{}, rest_base{} {
        }

        /**
         * @brief Copy constructor.
         *        拷贝构造函数。
         */
        constexpr tuple(const tuple &) = default;

        /**
         * @brief Move constructor.
         *        移动构造函数。
         */
        constexpr tuple(tuple &&) = default;

        /**
         * @brief Constructs tuple from explicit arguments.
         *        从显式参数构造tuple。
         *
         * @tparam HeadArg Type of the head argument
         *                 头参数的类型
         * @tparam RestArgs Types of the rest arguments
         *                  剩余参数的类型
         * @param head_arg Value for the first element
         *                 第一个元素的值
         * @param rest_args Values for the remaining elements
         *                  剩余元素的值
         */
        template <typename HeadArg, typename... RestArgs,
                  type_traits::other_trans::enable_if_t<sizeof...(RestArgs) == sizeof...(Rest) &&
                                                            type_traits::type_properties::is_constructible_v<Head, HeadArg> &&
                                                            (type_traits::type_properties::is_constructible_v<Rest, RestArgs> && ...),
                                                        int> = 0>
        constexpr tuple(HeadArg &&head_arg, RestArgs &&...rest_args) :
            head_base(utility::forward<HeadArg>(head_arg)), rest_base(utility::forward<RestArgs>(rest_args)...) {
        }

        /**
         * @brief Converting constructor from another tuple.
         *        从另一个tuple的转换构造函数。
         *
         * @tparam OtherHead Type of the first element of the other tuple
         *                   另一个tuple的第一个元素类型
         * @tparam OtherRest Types of the remaining elements of the other tuple
         *                   另一个tuple的剩余元素类型
         * @param other The other tuple to copy from
         *              要拷贝的另一个tuple
         */
        template <
            typename OtherHead, typename... OtherRest,
            type_traits::other_trans::enable_if_t<
                sizeof...(OtherRest) == sizeof...(Rest) && type_traits::type_properties::is_constructible_v<Head, const OtherHead &> &&
                    (type_traits::type_properties::is_constructible_v<Rest, const OtherRest &> && ...),
                int> = 0>
        constexpr tuple(const tuple<OtherHead, OtherRest...> &other) :
            head_base(get<0>(other)), rest_base(static_cast<const tuple<OtherRest...> &>(other)) {
        }

        /**
         * @brief Allocator-extended default constructor.
         *        分配器扩展的默认构造函数。
         *
         * @tparam Alloc Allocator type
         *               分配器类型
         * @param tag allocator_arg_t tag for disambiguation
         *            用于消歧的 allocator_arg_t 标签
         * @param alloc The allocator to use
         *              要使用的分配器
         */
        template <typename Alloc>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc) : head_base(tag, alloc), rest_base(tag, alloc) {
        }

        /**
         * @brief Allocator-extended constructor with arguments.
         *        带参数的分配器扩展构造函数。
         *
         * @tparam Alloc Allocator type
         *               分配器类型
         * @tparam HeadArg Type of the head argument
         *                 头参数的类型
         * @tparam TailArgs Types of the remaining arguments
         *                  剩余参数的类型
         * @param tag allocator_arg_t tag for disambiguation
         *            用于消歧的 allocator_arg_t 标签
         * @param alloc The allocator to use
         *              要使用的分配器
         * @param head_arg Value for the first element
         *                 第一个元素的值
         * @param tail_args Values for the remaining elements
         *                  剩余元素的值
         */
        template <typename Alloc, typename HeadArg, typename... TailArgs>
        constexpr tuple(std::allocator_arg_t tag, const Alloc &alloc, HeadArg &&head_arg, TailArgs &&...tail_args) :
            head_base(tag, alloc, utility::forward<HeadArg>(head_arg)),
            rest_base(tag, alloc, utility::forward<TailArgs>(tail_args)...) {
        }

        /**
         * @brief Copy assignment operator.
         *        拷贝赋值运算符。
         *
         * @param other The other tuple to copy from
         *              要拷贝的另一个tuple
         * @return Reference to this tuple
         *         此tuple的引用
         */
        constexpr tuple &operator=(const tuple &other) {
            head_base::value = static_cast<const head_base &>(other).value;
            static_cast<rest_base &>(*this) = static_cast<const rest_base &>(other);
            return *this;
        }

        /**
         * @brief Move assignment operator.
         *        移动赋值运算符。
         *
         * @param other The other tuple to move from
         *              要移动的另一个tuple
         * @return Reference to this tuple
         *         此tuple的引用
         */
        constexpr tuple &operator=(tuple &&other) noexcept(std::is_nothrow_move_assignable_v<Head> &&
                                                           std::is_nothrow_move_assignable_v<tuple<Rest...>>) {
            head_base::value = utility::move(static_cast<head_base &&>(other).value);
            static_cast<rest_base &>(*this) = static_cast<rest_base &&>(other);
            return *this;
        }

        /**
         * @brief Converting assignment from another tuple.
         *        从另一个tuple的转换赋值。
         *
         * @tparam OtherHead Type of the first element of the other tuple
         *                   另一个tuple的第一个元素类型
         * @tparam OtherRest Types of the remaining elements of the other tuple
         *                   另一个tuple的剩余元素类型
         * @param other The other tuple to copy from
         *              要拷贝的另一个tuple
         * @return Reference to this tuple
         *         此tuple的引用
         */
        template <typename OtherHead, typename... OtherRest,
                  type_traits::other_trans::enable_if_t<sizeof...(OtherRest) == sizeof...(Rest) &&
                                                            std::is_assignable_v<Head &, const OtherHead &> &&
                                                            (std::is_assignable_v<Rest &, const OtherRest &> && ...),
                                                        int> = 0>
        constexpr tuple &operator=(const tuple<OtherHead, OtherRest...> &other) {
            head_base::value = get<0>(other);
            static_cast<rest_base &>(*this) = static_cast<const tuple<OtherRest...> &>(other);
            return *this;
        }

        /**
         * @brief Swaps the contents with another tuple.
         *        与另一个tuple交换内容。
         *
         * @param other The other tuple to swap with
         *              要交换的另一个tuple
         */
        constexpr void swap(tuple &other) noexcept(std::is_nothrow_swappable_v<Head> && std::is_nothrow_swappable_v<tuple<Rest...>>) {
            head_base::swap(static_cast<head_base &>(other));
            static_cast<rest_base &>(*this).swap(static_cast<rest_base &>(other));
        }

        /**
         * @brief Gets a reference to the element at index I.
         *        获取索引I处元素的引用。
         *
         * @tparam I The index of the element to access
         *           要访问的元素的索引
         * @return Reference to the element at index I
         *         索引I处元素的引用
         */
        template <std::size_t I>
        constexpr tuple_element_t<I, tuple> &get() noexcept {
            if constexpr (I == 0) {
                return head_base::value;
            } else {
                return static_cast<rest_base &>(*this).template get<I - 1>();
            }
        }

        /**
         * @brief Gets a const reference to the element at index I.
         *        获取索引I处元素的常量引用。
         *
         * @tparam I The index of the element to access
         *           要访问的元素的索引
         * @return Const reference to the element at index I
         *         索引I处元素的常量引用
         */
        template <std::size_t I>
        constexpr const tuple_element_t<I, tuple> &get() const noexcept {
            if constexpr (I == 0) {
                return head_base::value;
            } else {
                return static_cast<const rest_base &>(*this).template get<I - 1>();
            }
        }
    };

    /**
     * @brief Gets a reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access
     *          要访问的tuple
     * @return Reference to the element at index I
     *         索引I处元素的引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &get(tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    /**
     * @brief Gets a const reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access
     *          要访问的常量tuple
     * @return Const reference to the element at index I
     *         索引I处元素的常量引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &get(const tuple<Types...> &t) noexcept {
        return t.template get<I>();
    }

    /**
     * @brief Gets an rvalue reference to the element at index I in a mutable tuple.
     *        获取可变tuple中索引I处元素的右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The tuple to access (as rvalue)
     *          要访问的tuple（作为右值）
     * @return Rvalue reference to the element at index I
     *         索引I处元素的右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr tuple_element_t<I, tuple<Types...>> &&get(tuple<Types...> &&t) noexcept { // NOLINT
        return utility::move(t.template get<I>());
    }

    /**
     * @brief Gets a const rvalue reference to the element at index I in a const tuple.
     *        获取常量tuple中索引I处元素的常量右值引用。
     *
     * @tparam I The index of the element to access
     *           要访问的元素的索引
     * @tparam Types The types contained in the tuple
     *               tuple中包含的类型
     * @param t The const tuple to access (as rvalue)
     *          要访问的常量tuple（作为右值）
     * @return Const rvalue reference to the element at index I
     *         索引I处元素的常量右值引用
     */
    template <std::size_t I, typename... Types>
    constexpr const tuple_element_t<I, tuple<Types...>> &&get(const tuple<Types...> &&t) noexcept {
        return utility::move(t.template get<I>());
    }

    /**
     * @brief Creates a tuple by deducing the types of the arguments and decaying them.
     *        通过推导参数类型并退化它们来创建tuple。
     *
     * @tparam Types The types of the arguments
     *               参数的类型
     * @param args The values to store in the tuple
     *             要存储在tuple中的值
     * @return A tuple containing the decayed copies of the arguments
     *         包含参数退化副本的tuple
     */
    template <typename... Types>
    constexpr tuple<std::decay_t<Types>...> make_tuple(Types &&...args) {
        return tuple<std::decay_t<Types>...>(utility::forward<Types>(args)...);
    }

    /**
     * @brief Creates a tuple of references to the arguments.
     *        创建参数引用的tuple。
     *
     * @tparam Types The types of the arguments
     *               参数的类型
     * @param args The values to create references to
     *             要创建引用的值
     * @return A tuple containing references to the arguments
     *         包含参数引用的tuple
     */
    template <typename... Types>
    constexpr tuple<Types &&...> forward_as_tuple(Types &&...args) noexcept {
        return tuple<Types &&...>(utility::forward<Types>(args)...);
    }

    /**
     * @brief Swaps two tuples.
     *        交换两个tuple。
     *
     * @tparam Types The types contained in the tuples
     *               tuple中包含的类型
     * @param left The first tuple
     *             第一个tuple
     * @param right The second tuple
     *              第二个tuple
     */
    template <typename... Types>
    constexpr void swap(tuple<Types...> &left, tuple<Types...> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }

    /**
     * @brief Equality comparison between two tuples.
     *        两个tuple之间的相等比较。
     *
     * @tparam TTypes Types of the left tuple
     *                左tuple的类型
     * @tparam UTypes Types of the right tuple
     *                右tuple的类型
     * @param left The left tuple
     *             左tuple
     * @param right The right tuple
     *              右tuple
     * @return true if the tuples are element-wise equal, false otherwise
     *         如果tuple逐元素相等则为true，否则为false
     */
    template <typename... TTypes, typename... UTypes>
    constexpr bool operator==(const tuple<TTypes...> &left, const tuple<UTypes...> &right) {
        if constexpr (sizeof...(TTypes) != sizeof...(UTypes)) {
            return false;
        } else if constexpr (sizeof...(TTypes) == 0) {
            return true;
        } else {
            return get<0>(left) == get<0>(right) &&
                   static_cast<const tuple<TTypes...> &>(left) == static_cast<const tuple<UTypes...> &>(right);
        }
    }
}

namespace rainy::foundation::container {
    /**
     * @brief Provides the number of elements in a tuple.
     *        提供tuple中的元素数量。
     *
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <typename Tuple>
    struct tuple_size {
        static RAINY_INLINE_CONSTEXPR std::size_t value = 0;
    };

    /**
     * @brief Specialization for tuple types.
     *        tuple类型的特化。
     *
     * @tparam Args The types contained in the tuple
     *              tuple中包含的类型
     */
    template <typename... Args>
    struct tuple_size<tuple<Args...>> {
        static RAINY_INLINE_CONSTEXPR std::size_t value = sizeof...(Args);
    };

    /**
     * @brief Variable template for tuple size.
     *        tuple大小的变量模板。
     *
     * @tparam Tuple The tuple type
     *               tuple类型
     */
    template <typename Tuple>
    static inline constexpr std::size_t tuple_size_v = tuple_size<Tuple>::value;
}

namespace std {
    using rainy::foundation::container::get;
}

namespace rainy::utility {
    using rainy::foundation::container::forward_as_tuple;
    using rainy::foundation::container::get;
    using rainy::foundation::container::make_tuple;
    using rainy::foundation::container::tuple;
    using rainy::foundation::container::tuple_element;
    using rainy::foundation::container::tuple_element_t;
    using rainy::foundation::container::tuple_size;
    using rainy::foundation::container::tuple_size_v;
}

#endif