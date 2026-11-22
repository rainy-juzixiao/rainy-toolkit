/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_CORE_CONTAINER_PAIR_HPP
#define RAINY_CORE_CONTAINER_PAIR_HPP

#include <rainy/core/type_traits.hpp>
#include <utility>

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A pair of heterogeneous values of types Ty1 and Ty2.
     *
     * @tparam Ty1 The type of the first element
     * @tparam Ty2 The type of the second element
     *
     * \lang simp-chinese
     * @brief 由 Ty1 和 Ty2 两种类型组成的异构值对。
     *
     * @tparam Ty1 第一个元素的类型
     * @tparam Ty2 第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    struct pair {
        /**
         * \lang english
         * @brief The type of the first element.
         *
         * \lang simp-chinese
         * @brief 第一个元素的类型。
         */
        using first_type = Ty1;

        /**
         * \lang english
         * @brief The type of the second element.
         *
         * \lang simp-chinese
         * @brief 第二个元素的类型。
         */
        using second_type = Ty2;

        /**
         * \lang english
         * @brief Default constructor. Value-initializes both elements if they are default constructible.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。若两个元素可默认构造，则值初始化它们。
         */
        template <
            typename uty1 = Ty1, typename uty2 = Ty2,
            type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<type_traits::properties::is_default_constructible<uty1>, type_traits::properties::is_default_constructible<uty2>>, int> = 0>
        constexpr pair() noexcept(type_traits::properties::is_nothrow_default_constructible_v<Ty1> && type_traits::properties::is_nothrow_default_constructible_v<Ty1>) :
            first(), second() {
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         */
        constexpr pair(const pair &) = default;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         */
        constexpr pair(pair &&) = default;

        /**
         * \lang english
         * @brief Constructs the pair from copies of the given values.
         *
         * @param val1 Value for the first element
         * @param val2 Value for the second element
         *
         * \lang simp-chinese
         * @brief 从给定值的拷贝构造 pair。
         *
         * @param val1 第一个元素的值
         * @param val2 第二个元素的值
         */
        template <typename uty1 = Ty1, typename uty2 = Ty2,
                  typename = type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<type_traits::properties::is_copy_constructible<uty1>, type_traits::properties::is_copy_constructible<uty2>>>>
        constexpr pair(const Ty1 &val1, const Ty2 &val2) noexcept(type_traits::properties::is_nothrow_copy_constructible_v<uty1> &&
                                                                  type_traits::properties::is_nothrow_copy_constructible_v<uty2>) :
            first(val1), second(val2) {
        }

        /**
         * \lang english
         * @brief Converting copy constructor from another pair.
         *
         * @tparam other1 The type of the first element of the other pair
         * @tparam other2 The type of the second element of the other pair
         * @param right The other pair to copy from
         *
         * \lang simp-chinese
         * @brief 从另一个 pair 的转换拷贝构造函数。
         *
         * @tparam other1 另一个 pair 第一个元素的类型
         * @tparam other2 另一个 pair 第二个元素的类型
         * @param right 要拷贝的另一个 pair
         */
        template <typename other1, typename other2,
                  typename = type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_constructible<Ty1, const other1 &>, type_traits::properties::is_constructible<Ty2, const other2 &>>>>
        constexpr pair(const pair<other1, other2> &right) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty1, other1> && // NOLINT
                                                                   type_traits::properties::is_nothrow_constructible_v<Ty2, other2>) :
            first(right.first), second(right.second) {
        }

        /**
         * \lang english
         * @brief Converting move constructor from another pair.
         *
         * @tparam other1 The type of the first element of the other pair
         * @tparam other2 The type of the second element of the other pair
         * @param right The other pair to move from
         *
         * \lang simp-chinese
         * @brief 从另一个 pair 的转换移动构造函数。
         *
         * @tparam other1 另一个 pair 第一个元素的类型
         * @tparam other2 另一个 pair 第二个元素的类型
         * @param right 要移动的另一个 pair
         */
        template <
            typename other1, typename other2,
            typename = type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<type_traits::properties::is_constructible<Ty1, other1>, type_traits::properties::is_constructible<Ty2, other2>>>>
        constexpr pair(const pair<other1, other2> &&right) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty1, other1> && // NOLINT
                                                                    type_traits::properties::is_nothrow_constructible_v<Ty2, other2>) :
            first(utility::forward<const other1>(right.first)), second(utility::forward<const other2>(right.second)) {
        }

        /**
         * \lang english
         * @brief Constructs the pair by forwarding the given values.
         *
         * @tparam other1 The type of the first value
         * @tparam other2 The type of the second value
         * @param val1 Value for the first element
         * @param val2 Value for the second element
         *
         * \lang simp-chinese
         * @brief 通过转发给定的值构造 pair。
         *
         * @tparam other1 第一个值的类型
         * @tparam other2 第二个值的类型
         * @param val1 第一个元素的值
         * @param val2 第二个元素的值
         */
        template <
            typename other1, typename other2,
            typename = type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<type_traits::properties::is_constructible<Ty1, other1>, type_traits::properties::is_constructible<Ty2, other2>>>>
        constexpr pair(other1 &&val1, other2 &&val2) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty1, other1> &&
                                                              type_traits::properties::is_nothrow_constructible_v<Ty2, other2>) :
            first(utility::forward<other1>(val1)), second(utility::forward<other2>(val2)) {
        }

        /**
         * \lang english
         * @brief Constructs the pair from the elements of the given argument tuples.
         *
         * @tparam Tuple1 Types of the first argument tuple
         * @tparam Tuple2 Types of the second argument tuple
         * @tparam Indices1 Indices of the first argument tuple
         * @tparam Indices2 Indices of the second argument tuple
         * @param first_args Tuple containing the arguments for the first element
         * @param second_args Tuple containing the arguments for the second element
         *
         * \lang simp-chinese
         * @brief 从给定参数元组的元素构造 pair。
         *
         * @tparam Tuple1 第一个参数元组的类型
         * @tparam Tuple2 第二个参数元组的类型
         * @tparam Indices1 第一个参数元组的索引
         * @tparam Indices2 第二个参数元组的索引
         * @param first_args 包含第一个元素参数的元组
         * @param second_args 包含第二个元素参数的元组
         */
        template <typename... Tuple1, typename... Tuple2, std::size_t... Indices1, std::size_t... Indices2>
        constexpr pair(std::tuple<Tuple1...> &&first_args, std::tuple<Tuple2...> &&second_args, std::index_sequence<Indices1...>,
                       std::index_sequence<Indices2...>) :
            first(std::get<Indices1>(utility::move(first_args))...), second(std::get<Indices2>(utility::move(second_args))...) {
        }

        /**
         * \lang english
         * @brief Piecewise construct constructor.
         *
         * @tparam Args1 Types of the arguments for the first element
         * @tparam Args2 Types of the arguments for the second element
         * @param first_args Tuple containing the arguments for the first element
         * @param second_args Tuple containing the arguments for the second element
         *
         * \lang simp-chinese
         * @brief 分段构造构造函数。
         *
         * @tparam Args1 第一个元素的参数类型
         * @tparam Args2 第二个元素的参数类型
         * @param first_args 包含第一个元素参数的元组
         * @param second_args 包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr pair(std::piecewise_construct_t, std::tuple<Args1...> &&first_args, std::tuple<Args2...> &&second_args) :
            pair(utility::move(first_args), utility::move(second_args), std::index_sequence_for<Args1...>{},
                 std::index_sequence_for<Args2...>{}) {
        }

        /**
         * \lang english
         * @brief Deleted copy assignment operator for volatile pairs.
         *
         * \lang simp-chinese
         * @brief 针对 volatile pair 的已删除拷贝赋值运算符。
         */
        constexpr pair &operator=(const volatile pair &) = delete;

        /**
         * \lang english
         * @brief Converting copy assignment from another pair.
         *
         * @tparam Other1 The type of the first element of the other pair
         * @tparam Other2 The type of the second element of the other pair
         * @param right The other pair to copy from
         * @return Reference to this pair
         *
         * \lang simp-chinese
         * @brief 从另一个 pair 的转换拷贝赋值。
         *
         * @tparam Other1 另一个 pair 第一个元素的类型
         * @tparam Other2 另一个 pair 第二个元素的类型
         * @param right 要拷贝的另一个 pair
         * @return 此 pair 的引用
         */
        template <typename Other1, typename Other2,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::logical_traits::negation<type_traits::type_relations::is_same<pair, pair<Other1, Other2>>>,
                          type_traits::properties::is_assignable<Ty1 &, const Other1 &>,
                          type_traits::properties::is_assignable<Ty2 &, const Other2 &>>,
                      int> = 0>
        constexpr pair &operator=(const pair<Other1, Other2> &right) noexcept(
            type_traits::properties::is_nothrow_assignable_v<Ty1 &, const Other1 &> &&
            type_traits::properties::is_nothrow_assignable_v<Ty2 &, const Other2 &>) /* strengthened */ {
            first = right.first;
            second = right.second;
            return *this;
        }

        /**
         * \lang english
         * @brief Copy assignment from another pair.
         *
         * @tparam U1 The type of the first element of the other pair
         * @tparam U2 The type of the second element of the other pair
         * @param p The other pair to copy from
         * @return Reference to this pair
         *
         * \lang simp-chinese
         * @brief 从另一个 pair 的拷贝赋值。
         *
         * @tparam U1 另一个 pair 第一个元素的类型
         * @tparam U2 另一个 pair 第二个元素的类型
         * @param p 要拷贝的另一个 pair
         * @return 此 pair 的引用
         */
        template <typename U1, typename U2,
                  typename = type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_assignable<Ty1 &, const U1 &>, type_traits::properties::is_assignable<Ty2 &, const U2 &>>>>
        constexpr pair &operator=(const pair<U1, U2> &p) {
            first = p.first;
            second = p.second;
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param p The other pair to move from
         * @return Reference to this pair
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param p 要移动的另一个 pair
         * @return 此 pair 的引用
         */
        constexpr pair &operator=(pair &&p) noexcept {
            first = p.first;
            second = p.second;
            return *this;
        }

        /**
         * \lang english
         * @brief Converting move assignment from another pair.
         *
         * @tparam U1 The type of the first element of the other pair
         * @tparam U2 The type of the second element of the other pair
         * @param p The other pair to move from
         * @return Reference to this pair
         *
         * \lang simp-chinese
         * @brief 从另一个 pair 的转换移动赋值。
         *
         * @tparam U1 另一个 pair 第一个元素的类型
         * @tparam U2 另一个 pair 第二个元素的类型
         * @param p 要移动的另一个 pair
         * @return 此 pair 的引用
         */
        template <typename U1, typename U2>
        constexpr pair &operator=(pair<U1, U2> &&p) {
            first = utility::exchange(p.first, {});
            second = utility::exchange(p.second, {});
            return *this;
        }

        /**
         * \lang english
         * @brief Swaps the contents with another pair.
         *
         * @param p The other pair to swap with
         *
         * \lang simp-chinese
         * @brief 与另一个 pair 交换内容。
         *
         * @param p 要交换的另一个 pair
         */
        constexpr void swap(pair &p) noexcept(type_traits::properties::is_nothrow_swappable_v<first_type> && type_traits::properties::is_nothrow_swappable_v<second_type>) {
            if (this != utility::addressof(p)) {
                using std::swap;
                swap(first, p.first);
                swap(second, p.second);
            }
        }

        /**
         * \lang english
         * @brief The first element of the pair.
         *
         * \lang simp-chinese
         * @brief pair 的第一个元素。
         */
        Ty1 first;

        /**
         * \lang english
         * @brief The second element of the pair.
         *
         * \lang simp-chinese
         * @brief pair 的第二个元素。
         */
        Ty2 second;
    };

    /**
     * \lang english
     * @brief Creates a pair from the given values.
     *
     * @tparam Ty1 The type of the first value
     * @tparam Ty2 The type of the second value
     * @param val1 Value for the first element
     * @param val2 Value for the second element
     * @return A pair constructed from the given values
     *
     * \lang simp-chinese
     * @brief 从给定的值创建 pair。
     *
     * @tparam Ty1 第一个值的类型
     * @tparam Ty2 第二个值的类型
     * @param val1 第一个元素的值
     * @param val2 第二个元素的值
     * @return 由给定值构造的 pair
     */
    template <typename Ty1, typename Ty2>
    constexpr auto make_pair(const Ty1 &val1, const Ty2 &val2) noexcept(
        type_traits::properties::is_nothrow_constructible_v<pair<Ty1, Ty2>, const Ty1 &, const Ty2 &>) {
        return pair<Ty1, Ty2>(val1, val2);
    }
}

namespace rainy::container {
    using rainy::core::container::pair;
    using rainy::core::container::make_pair;
}


#endif
