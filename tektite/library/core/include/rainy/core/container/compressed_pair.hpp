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
#ifndef RAINY_CORE_CONTAINER_COMPRESSED_PAIR_HPP
#define RAINY_CORE_CONTAINER_COMPRESSED_PAIR_HPP
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>

namespace rainy::core::container {
    using rainy::container::piecewise_construct;
    using rainy::container::piecewise_construct_t;

    /**
     * \lang english
     * @brief A compressed pair that optimizes storage when one or both types are empty.
     *         Uses empty base optimization to reduce memory footprint.
     *
     * @tparam Ty1 The type of the first element
     * @tparam Ty2 The type of the second element
     *
     * \lang simp-chinese
     * @brief 压缩对，当一个或两个类型为空时优化存储。
     *         使用空基类优化来减少内存占用。
     *
     * @tparam Ty1 第一个元素的类型
     * @tparam Ty2 第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    class compressed_pair;
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::container::implements {
    template <typename Ty, bool = type_traits::properties::is_final_v<Ty>>
    struct compressed_pair_empty : type_traits::helper::false_type {};

    template <typename Ty>
    struct compressed_pair_empty<Ty, false> : type_traits::properties::is_empty<Ty> {};

    template <typename Ty1, typename Ty2, bool is_same, bool first_empty, bool second_empty>
    struct compressed_pair_switch;

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, false, false> {
        RAINY_CONSTEXPR static int value = 0;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, true, true> {
        RAINY_CONSTEXPR static int value = 3;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, true, false> {
        RAINY_CONSTEXPR static int value = 1;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, false, true> {
        RAINY_CONSTEXPR static int value = 2;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, true, true, true> {
        RAINY_CONSTEXPR static int value = 4;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, true, false, false> {
        RAINY_CONSTEXPR static int value = 5;
    };

    template <typename Ty1, typename Ty2, int Version>
    class compressed_pair_impl;

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 0> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        constexpr compressed_pair_impl() = default;

        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first(x), second(y) {
        }

        explicit constexpr compressed_pair_impl(first_param_type x) : first(x) {
        }

        explicit constexpr compressed_pair_impl(second_param_type y) : second(y) {
        }

        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair_impl(piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            using std::swap;
            swap(first, pair.first);
            swap(second, pair.second);
        }

        first_type first;
        second_type second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args,
                                       type_traits::helper::index_sequence<I1...>, type_traits::helper::index_sequence<I2...>) :
            first(container::get<I1>(move(first_args))...), second(container::get<I2>(move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 1> : protected type_traits::implements::remove_cv_t<Ty1> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        constexpr compressed_pair_impl() = default;

        explicit constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second(second) {
        }

        explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
        }

        explicit constexpr compressed_pair_impl(second_param_type second) : second(second) {
        }

        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair_impl(piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            using std::swap;
            swap(second, pair.second);
        }

        second_type second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args,
                                       type_traits::helper::index_sequence<I1...>, type_traits::helper::index_sequence<I2...>) :
            type_traits::modifers::remove_cv_t<Ty1>(container::get<I1>(utility::move(first_args))...),
            second(container::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 2> : protected type_traits::implements::remove_cv_t<Ty2> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        constexpr compressed_pair_impl() = default;

        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : second_type(y), first(x) {
        }

        constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
        }

        constexpr explicit compressed_pair_impl(second_param_type y) : second_type(y) {
        }

        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair_impl(piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        constexpr rain_fn get_second() -> second_reference {
            return *this;
        }

        constexpr rain_fn get_second() const -> second_const_reference {
            return *this;
        }

        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            std::swap(first, pair.first);
        }

        first_type first;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args,
                                       type_traits::helper::index_sequence<I1...>, type_traits::helper::index_sequence<I2...>) :
            first(container::get<I1>(utility::move(first_args))...),
            type_traits::modifers::remove_cv_t<Ty2>(container::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 3> : protected type_traits::implements::remove_cv_t<Ty1>,
                                              protected type_traits::implements::remove_cv_t<Ty2> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        constexpr compressed_pair_impl() = default;

        constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second_type(second) {
        }

        explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
        }

        explicit constexpr compressed_pair_impl(second_param_type second) : second_type(second) {
        }

        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair_impl(piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        constexpr rain_fn get_second() -> second_reference {
            return *this;
        }

        constexpr rain_fn get_second() const -> second_const_reference {
            return *this;
        }

        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &) -> void {
        }

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args,
                                       type_traits::helper::index_sequence<I1...>, type_traits::helper::index_sequence<I2...>) :
            type_traits::modifers::remove_cv_t<Ty1>(container::get<I1>(utility::move(first_args))...),
            type_traits::modifers::remove_cv_t<Ty2>(container::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 4> : protected type_traits::implements::remove_cv_t<Ty1> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        constexpr compressed_pair_impl() = default;

        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first_type(x), second(y) {
        }

        explicit compressed_pair_impl(first_param_type x) : first_type(x), second(x) {
        }

        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair_impl(piecewise_construct, move(first_args), move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &pair) -> void {
            std::swap(second, pair.second);
        }

        Ty2 second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args,
                                       type_traits::helper::index_sequence<I1...>, type_traits::helper::index_sequence<I2...>) :
            type_traits::modifers::remove_cv_t<Ty1>(container::get<I1>(utility::move(first_args))...),
            second(container::get<I2>(utility::move(second_args))...) {
        }
    };

    template <typename Ty1, typename Ty2>
    class compressed_pair_impl<Ty1, Ty2, 5> {
    public:
        using first_type = Ty1;
        using second_type = Ty2;
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        constexpr compressed_pair_impl() = default;

        constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first(first), second(second) {
        }

        explicit constexpr compressed_pair_impl(first_param_type first) : first(first), second(first) {
        }

        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair_impl(piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &y) -> void {
            std::swap(first, y.first);
            std::swap(second, y.second);
        }

        Ty1 first;
        Ty2 second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args,
                                       type_traits::helper::index_sequence<I1...>, type_traits::helper::index_sequence<I2...>) :
            first(container::get<I1>(utility::move(first_args))...), second(container::get<I2>(move(second_args))...) {
        }
    };
}

namespace rainy::core::container {
    template <typename Ty1, typename Ty2>
    class compressed_pair
        : public implements::compressed_pair_impl<
              Ty1, Ty2,
              implements::compressed_pair_switch<
                  Ty1, Ty2,
                  type_traits::implements::is_same_v<type_traits::modifers::remove_cv_t<Ty1>, type_traits::modifers::remove_cv_t<Ty2>>,
                  implements::compressed_pair_empty<Ty1>::value, implements::compressed_pair_empty<Ty2>::value>::value> {
    public:
        using base = implements::compressed_pair_impl<
            Ty1, Ty2,
            implements::compressed_pair_switch<
                Ty1, Ty2,
                type_traits::implements::is_same_v<type_traits::modifers::remove_cv_t<Ty1>, type_traits::modifers::remove_cv_t<Ty2>>,
                implements::compressed_pair_empty<Ty1>::value, implements::compressed_pair_empty<Ty2>::value>::value>;

        using base::base;

        constexpr compressed_pair(const compressed_pair &) = default;

        constexpr compressed_pair(compressed_pair &&) = default;

        constexpr rain_fn operator=(const compressed_pair &other)->compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        constexpr rain_fn operator=(compressed_pair &&other) noexcept(type_traits::properties::is_nothrow_move_assignable_v<Ty1> &&
                                                                      type_traits::properties::is_nothrow_move_assignable_v<Ty2>)
            ->compressed_pair & {
            utility::construct_in_place(this->get_first(), utility::move(other.get_first()));
            utility::construct_in_place(this->get_second(), utility::move(other.get_second()));
            return *this;
        }
    };

    template <typename Ty>
    class compressed_pair<Ty, Ty>
        : public implements::compressed_pair_impl<
              Ty, Ty,
              implements::compressed_pair_switch<
                  Ty, Ty,
                  type_traits::implements::is_same_v<type_traits::modifers::remove_cv_t<Ty>, type_traits::modifers::remove_cv_t<Ty>>,
                  implements::compressed_pair_empty<Ty>::value, implements::compressed_pair_empty<Ty>::value>::value> {
    public:
        using base = implements::compressed_pair_impl<
            Ty, Ty,
            implements::compressed_pair_switch<
                Ty, Ty,
                type_traits::implements::is_same_v<type_traits::modifers::remove_cv_t<Ty>, type_traits::modifers::remove_cv_t<Ty>>,
                implements::compressed_pair_empty<Ty>::value, implements::compressed_pair_empty<Ty>::value>::value>;

        using base::base;

        constexpr compressed_pair(const compressed_pair &) = default;

        constexpr compressed_pair(compressed_pair &&) = default;

        constexpr rain_fn operator=(const compressed_pair &other)->compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        constexpr rain_fn operator=(compressed_pair &&other) noexcept(type_traits::properties::is_nothrow_move_assignable_v<Ty>)->compressed_pair & {
            this->get_first() = utility::move(other.get_first());
            this->get_second() = utility::move(other.get_second());
            return *this;
        }
    };

    template <typename Ty1, typename Ty2>
    constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &left, compressed_pair<Ty1, Ty2> &right) -> void {
        left.swap(right);
    }
}

#else

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A compressed pair that optimizes storage when one or both types are empty.
     *         Uses empty base optimization to reduce memory footprint.
     *
     *  The presented interface corresponds to the full implementation (version 5, both
     *  members stored as ordinary data members). If a member type is empty and non-final,
     *  that member is instead stored as an empty base class and the corresponding data
     *  member does not exist; the member is annotated per-field with an @note tag.
     *
     * @tparam Ty1 The type of the first element
     * @tparam Ty2 The type of the second element
     *
     * \lang simp-chinese
     * @brief 压缩对，当一个或两个类型为空时优化存储。
     *         使用空基类优化来减少内存占用。
     *
     *  所展示的接口对应完整实现（版本 5，两个成员均作为普通数据成员存储）。
     *  若某成员类型为空且非 final，则该成员改为以空基类形式存储，对应的数据成员
     *  不存在；该成员会通过每个字段的 @note 标注说明。
     *
     * @tparam Ty1 第一个元素的类型
     * @tparam Ty2 第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    class compressed_pair {
    public:
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
         * @brief The parameter type used to pass a first element value.
         *
         * \lang simp-chinese
         * @brief 用于传递第一个元素值的参数类型。
         */
        using first_param_type = typename type_traits::implements::call_traits<first_type>::param_type;

        /**
         * \lang english
         * @brief The parameter type used to pass a second element value.
         *
         * \lang simp-chinese
         * @brief 用于传递第二个元素值的参数类型。
         */
        using second_param_type = typename type_traits::implements::call_traits<second_type>::param_type;

        /**
         * \lang english
         * @brief The reference type of the first element.
         *
         * \lang simp-chinese
         * @brief 第一个元素的引用类型。
         */
        using first_reference = typename type_traits::implements::call_traits<first_type>::reference;

        /**
         * \lang english
         * @brief The reference type of the second element.
         *
         * \lang simp-chinese
         * @brief 第二个元素的引用类型。
         */
        using second_reference = typename type_traits::implements::call_traits<second_type>::reference;

        /**
         * \lang english
         * @brief The const reference type of the first element.
         *
         * \lang simp-chinese
         * @brief 第一个元素的常量引用类型。
         */
        using first_const_reference = typename type_traits::implements::call_traits<first_type>::const_reference;

        /**
         * \lang english
         * @brief The const reference type of the second element.
         *
         * \lang simp-chinese
         * @brief 第二个元素的常量引用类型。
         */
        using second_const_reference = typename type_traits::implements::call_traits<second_type>::const_reference;

        /**
         * \lang english
         * @brief Default constructor. Value-initializes both elements.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。对两个元素进行值初始化。
         */
        constexpr compressed_pair() = default;

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         */
        constexpr compressed_pair(const compressed_pair &) = default;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         */
        constexpr compressed_pair(compressed_pair &&) = default;

        /**
         * \lang english
         * @brief Constructs both elements from the given values.
         *
         * @param first Value for the first element.
         * @param second Value for the second element.
         *
         * \lang simp-chinese
         * @brief 从给定值构造两个元素。
         *
         * @param first 第一个元素的值。
         * @param second 第二个元素的值。
         */
        constexpr compressed_pair(first_param_type first, second_param_type second) : first(first), second(second) {
        }

        /**
         * \lang english
         * @brief Constructs the first element from the given value; the second element
         *        is initialized from the same value.
         *
         * @param first Value for both elements.
         *
         * \lang simp-chinese
         * @brief 从给定值构造第一个元素；第二个元素以相同的值初始化。
         *
         * @param first 两个元素的值。
         */
        explicit constexpr compressed_pair(first_param_type first) : first(first), second(first) {
        }

        /**
         * \lang english
         * @brief Piecewise construct constructor. Forwards the elements of the two
         *        tuples to the respective element constructors.
         *
         * @tparam Args1 Types of the first element's constructor arguments.
         * @tparam Args2 Types of the second element's constructor arguments.
         * @param first_args Tuple containing arguments for the first element.
         * @param second_args Tuple containing arguments for the second element.
         *
         * \lang simp-chinese
         * @brief 分段构造构造函数。将两个元组的元素分别转发给对应元素的构造函数。
         *
         * @tparam Args1 第一个元素构造函数参数的类型。
         * @tparam Args2 第二个元素构造函数参数的类型。
         * @param first_args 包含第一个元素参数的元组。
         * @param second_args 包含第二个元素参数的元组。
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair(piecewise_construct_t, tuple<Args1...> first_args, tuple<Args2...> second_args) :
            compressed_pair(piecewise_construct, utility::move(first_args), utility::move(second_args),
                            type_traits::helper::index_sequence_for<Args1...>{},
                            type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * \lang english
         * @brief Gets a reference to the first element.
         *
         * @return A reference to the first element.
         *
         * \lang simp-chinese
         * @brief 获取第一个元素的引用。
         *
         * @return 第一个元素的引用。
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * \lang english
         * @brief Gets a const reference to the first element.
         *
         * @return A const reference to the first element.
         *
         * \lang simp-chinese
         * @brief 获取第一个元素的常量引用。
         *
         * @return 第一个元素的常量引用。
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * \lang english
         * @brief Gets a reference to the second element.
         *
         * @return A reference to the second element.
         *
         * \lang simp-chinese
         * @brief 获取第二个元素的引用。
         *
         * @return 第二个元素的引用。
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * \lang english
         * @brief Gets a const reference to the second element.
         *
         * @return A const reference to the second element.
         *
         * \lang simp-chinese
         * @brief 获取第二个元素的常量引用。
         *
         * @return 第二个元素的常量引用。
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * \lang english
         * @brief Swaps the contents with another compressed_pair.
         *
         * @param other The other pair to swap with.
         *
         * \lang simp-chinese
         * @brief 与另一个 compressed_pair 交换内容。
         *
         * @param other 要交换的另一个对。
         */
        constexpr rain_fn swap(compressed_pair &other) -> void {
            using std::swap;
            swap(first, other.first);
            swap(second, other.second);
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @param other The other pair to copy from.
         * @return Reference to this pair.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @param other 要拷贝的另一个对。
         * @return 此对的引用。
         */
        constexpr rain_fn operator=(const compressed_pair &other) -> compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param other The other pair to move from.
         * @return Reference to this pair.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param other 要移动的另一个对。
         * @return 此对的引用。
         */
        constexpr rain_fn operator=(compressed_pair &&other) noexcept(type_traits::properties::is_nothrow_move_assignable_v<Ty1> &&
                                                                      type_traits::properties::is_nothrow_move_assignable_v<Ty2>)
            -> compressed_pair & {
            utility::construct_in_place(this->get_first(), utility::move(other.get_first()));
            utility::construct_in_place(this->get_second(), utility::move(other.get_second()));
            return *this;
        }

        /**
         * \lang english
         * @brief The first element.
         *
         * @note Does not exist if Ty1 is empty and non-final; in that case Ty1 is
         *       stored as an empty base class and accessed via get_first().
         *
         * \lang simp-chinese
         * @brief 第一个元素。
         *
         * @note 若 Ty1 为空且非 final，则该成员不存在；此时 Ty1 以空基类形式存储，
         *       并通过 get_first() 访问。
         */
        first_type first;

        /**
         * \lang english
         * @brief The second element.
         *
         * @note Does not exist if Ty2 is empty and non-final; in that case Ty2 is
         *       stored as an empty base class and accessed via get_second().
         *
         * \lang simp-chinese
         * @brief 第二个元素。
         *
         * @note 若 Ty2 为空且非 final，则该成员不存在；此时 Ty2 以空基类形式存储，
         *       并通过 get_second() 访问。
         */
        second_type second;
    };

    /**
     * \lang english
     * @brief Swaps two compressed pairs.
     *
     * @tparam Ty1 The type of the first element.
     * @tparam Ty2 The type of the second element.
     * @param left The first pair.
     * @param right The second pair.
     *
     * \lang simp-chinese
     * @brief 交换两个压缩对。
     *
     * @tparam Ty1 第一个元素的类型。
     * @tparam Ty2 第二个元素的类型。
     * @param left 第一个对。
     * @param right 第二个对。
     */
    template <typename Ty1, typename Ty2>
    constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &left, compressed_pair<Ty1, Ty2> &right) -> void {
        left.swap(right);
    }
}

#endif

namespace rainy::container {
    using rainy::core::container::compressed_pair;
    using rainy::core::container::swap;
}

#endif
