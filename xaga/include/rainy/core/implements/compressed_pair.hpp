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
#ifndef RAINY_CORE_IMPLEMENTS_COMPRESSED_PAIR_HPP
#define RAINY_CORE_IMPLEMENTS_COMPRESSED_PAIR_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>

namespace rainy::utility {
    /**
     * @brief A compressed pair that optimizes storage when one or both types are empty.
     *        Uses empty base optimization to reduce memory footprint.
     *
     *        压缩对，当一个或两个类型为空时优化存储。
     *        使用空基类优化来减少内存占用。
     *
     * @tparam Ty1 The type of the first element
     *             第一个元素的类型
     * @tparam Ty2 The type of the second element
     *             第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    class compressed_pair;
}

namespace rainy::utility::implements {
    // Internal implementation details - members are documented as they are exposed through public API
    template <typename Ty, bool = std::is_final_v<Ty>>
    struct compressed_pair_empty : std::false_type {};

    template <typename Ty>
    struct compressed_pair_empty<Ty, false> : std::is_empty<Ty> {};

    template <typename Ty1, typename Ty2, bool is_same, bool first_empty, bool second_empty>
    struct compressed_pair_switch;

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, false, false> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 0;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, true, true> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 3;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, true, false> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 1;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, false, false, true> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 2;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, true, true, true> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 4;
    };

    template <typename Ty1, typename Ty2>
    struct compressed_pair_switch<Ty1, Ty2, true, false, false> {
        /**
         * @brief Version identifier for compressed pair implementation
         *        压缩对实现的版本标识符
         */
        RAINY_CONSTEXPR static int value = 5;
    };

    template <typename Ty1, typename Ty2, int Version>
    class compressed_pair_impl;

    template <class Ty1, typename Ty2>
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

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values
         *        从给定值构造两个元素
         *
         * @param x Value for the first element
         *          第一个元素的值
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first(x), second(y) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param x Value for the first element
         *          第一个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type x) : first(x) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param y Value for the second element
         *          第二个元素的值
         */
        explicit constexpr compressed_pair_impl(second_param_type y) : second(y) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element
         *        获取第一个元素的引用
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * @brief Gets a const reference to the first element
         *        获取第一个元素的常量引用
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair_impl
         *        与另一个 compressed_pair_impl 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            using std::swap;
            swap(first, pair.first);
            swap(second, pair.second);
        }

        first_type first;
        second_type second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            first(rainy::utility::get<I1>(utility::move(first_args))...), second(std::get<I2>(utility::move(second_args))...) {
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

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (first via base class)
         *        从给定值构造两个元素（第一个通过基类）
         *
         * @param first Value for the first element
         *              第一个元素的值
         * @param second Value for the second element
         *               第二个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second(second) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param first Value for the first element
         *              第一个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param second Value for the second element
         *               第二个元素的值
         */
        explicit constexpr compressed_pair_impl(second_param_type second) : second(second) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element (via base class)
         *        获取第一个元素的引用（通过基类）
         */
        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the first element (via base class)
         *        获取第一个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair_impl
         *        与另一个 compressed_pair_impl 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            using std::swap;
            swap(second, pair.second);
        }

        second_type second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            type_traits::cv_modify::remove_cv_t<Ty1>(std::get<I1>(utility::move(first_args))...),
            second(std::get<I2>(utility::move(second_args))...) {
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

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (second via base class)
         *        从给定值构造两个元素（第二个通过基类）
         *
         * @param x Value for the first element
         *          第一个元素的值
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : second_type(y), first(x) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param x Value for the first element
         *          第一个元素的值
         */
        constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr explicit compressed_pair_impl(second_param_type y) : second_type(y) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element
         *        获取第一个元素的引用
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * @brief Gets a const reference to the first element
         *        获取第一个元素的常量引用
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * @brief Gets a reference to the second element (via base class)
         *        获取第二个元素的引用（通过基类）
         */
        constexpr rain_fn get_second() -> second_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the second element (via base class)
         *        获取第二个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return *this;
        }

        /**
         * @brief Swaps the contents with another compressed_pair_impl
         *        与另一个 compressed_pair_impl 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair_impl &pair) -> void {
            std::swap(first, pair.first);
        }

        first_type first;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            first(std::get<I1>(utility::move(first_args))...),
            type_traits::cv_modify::remove_cv_t<Ty2>(std::get<I2>(utility::move(second_args))...) {
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

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (both via base classes)
         *        从给定值构造两个元素（都通过基类）
         *
         * @param first Value for the first element
         *              第一个元素的值
         * @param second Value for the second element
         *               第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second_type(second) {
        }

        /**
         * @brief Constructs only the first element, second is default constructed
         *        只构造第一个元素，第二个默认构造
         *
         * @param first Value for the first element
         *              第一个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first) : first_type(first) {
        }

        /**
         * @brief Constructs only the second element, first is default constructed
         *        只构造第二个元素，第一个默认构造
         *
         * @param second Value for the second element
         *               第二个元素的值
         */
        explicit constexpr compressed_pair_impl(second_param_type second) : second_type(second) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element (via base class)
         *        获取第一个元素的引用（通过基类）
         */
        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the first element (via base class)
         *        获取第一个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        /**
         * @brief Gets a reference to the second element (via base class)
         *        获取第二个元素的引用（通过基类）
         */
        constexpr rain_fn get_second() -> second_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the second element (via base class)
         *        获取第二个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return *this;
        }

        /**
         * @brief Swaps the contents with another compressed_pair (no-op as both are empty)
         *        与另一个 compressed_pair 交换内容（空操作，因为两者都为空）
         *
         * @param  The other pair to swap with
         *         要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &) -> void {
        }

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            type_traits::cv_modify::remove_cv_t<Ty1>(std::get<I1>(utility::move(first_args))...),
            type_traits::cv_modify::remove_cv_t<Ty2>(std::get<I2>(utility::move(second_args))...) {
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

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values (first via base class)
         *        从给定值构造两个元素（第一个通过基类）
         *
         * @param x Value for the first element
         *          第一个元素的值
         * @param y Value for the second element
         *          第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first_type(x), second(y) {
        }

        /**
         * @brief Constructs only the first element, second uses the same value
         *        只构造第一个元素，第二个使用相同的值
         *
         * @param x Value for both elements
         *          两个元素的值
         */
        explicit compressed_pair_impl(first_param_type x) : first_type(x), second(x) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element (via base class)
         *        获取第一个元素的引用（通过基类）
         */
        constexpr rain_fn get_first() -> first_reference {
            return *this;
        }

        /**
         * @brief Gets a const reference to the first element (via base class)
         *        获取第一个元素的常量引用（通过基类）
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return *this;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair
         *        与另一个 compressed_pair 交换内容
         *
         * @param pair The other pair to swap with
         *             要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &pair) -> void {
            std::swap(second, pair.second);
        }

        Ty2 second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            type_traits::cv_modify::remove_cv_t<Ty1>(std::get<I1>(utility::move(first_args))...),
            second(std::get<I2>(utility::move(second_args))...) {
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

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        constexpr compressed_pair_impl() = default;

        /**
         * @brief Constructs both elements from the given values
         *        从给定值构造两个元素
         *
         * @param first Value for the first element
         *              第一个元素的值
         * @param second Value for the second element
         *               第二个元素的值
         */
        constexpr compressed_pair_impl(first_param_type first, second_param_type second) : first(first), second(second) {
        }

        /**
         * @brief Constructs only the first element, second uses the same value
         *        只构造第一个元素，第二个使用相同的值
         *
         * @param first Value for both elements
         *              两个元素的值
         */
        explicit constexpr compressed_pair_impl(first_param_type first) : first(first), second(first) {
        }

        /**
         * @brief Piecewise construct constructor
         *        分段构造构造函数
         *
         * @tparam Args1 Types for the first element's constructor arguments
         *               第一个元素构造函数参数的类型
         * @tparam Args2 Types for the second element's constructor arguments
         *               第二个元素构造函数参数的类型
         * @param first_args Tuple containing arguments for the first element
         *                   包含第一个元素参数的元组
         * @param second_args Tuple containing arguments for the second element
         *                    包含第二个元素参数的元组
         */
        template <typename... Args1, typename... Args2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args) :
            compressed_pair_impl(utility::piecewise_construct, utility::move(first_args), utility::move(second_args),
                                 type_traits::helper::index_sequence_for<Args1...>{},
                                 type_traits::helper::index_sequence_for<Args2...>{}) {
        }

        /**
         * @brief Gets a reference to the first element
         *        获取第一个元素的引用
         */
        constexpr rain_fn get_first() -> first_reference {
            return first;
        }

        /**
         * @brief Gets a const reference to the first element
         *        获取第一个元素的常量引用
         */
        constexpr rain_fn get_first() const -> first_const_reference {
            return first;
        }

        /**
         * @brief Gets a reference to the second element
         *        获取第二个元素的引用
         */
        constexpr rain_fn get_second() -> second_reference {
            return second;
        }

        /**
         * @brief Gets a const reference to the second element
         *        获取第二个元素的常量引用
         */
        constexpr rain_fn get_second() const -> second_const_reference {
            return second;
        }

        /**
         * @brief Swaps the contents with another compressed_pair
         *        与另一个 compressed_pair 交换内容
         *
         * @param y The other pair to swap with
         *          要交换的另一个对
         */
        constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &y) -> void {
            std::swap(first, y.first);
            std::swap(second, y.second);
        }

        Ty1 first;
        Ty2 second;

    private:
        template <typename... Args1, typename... Args2, std::size_t... I1, std::size_t... I2>
        constexpr compressed_pair_impl(utility::piecewise_construct_t, utility::tuple<Args1...> first_args,
                                       utility::tuple<Args2...> second_args, type_traits::helper::index_sequence<I1...>,
                                       type_traits::helper::index_sequence<I2...>) :
            first(rainy::utility::get<I1>(utility::move(first_args))...), second(std::get<I2>(utility::move(second_args))...) {
        }
    };
}

namespace rainy::utility {
    /**
     * @brief A compressed pair that optimizes storage when one or both types are empty.
     *        Uses empty base optimization to reduce memory footprint.
     *
     *        压缩对，当一个或两个类型为空时优化存储。
     *        使用空基类优化来减少内存占用。
     *
     * @tparam Ty1 The type of the first element
     *             第一个元素的类型
     * @tparam Ty2 The type of the second element
     *             第二个元素的类型
     */
    template <typename Ty1, typename Ty2>
    class compressed_pair
        : public implements::compressed_pair_impl<
              Ty1, Ty2,
              implements::compressed_pair_switch<Ty1, Ty2,
                                                 type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty1>,
                                                                                    type_traits::cv_modify::remove_cv_t<Ty2>>,
                                                 implements::compressed_pair_empty<Ty1>::value,
                                                 implements::compressed_pair_empty<Ty2>::value>::value> {
    public:
        using base = implements::compressed_pair_impl<
            Ty1, Ty2,
            implements::compressed_pair_switch<
                Ty1, Ty2,
                type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty1>, type_traits::cv_modify::remove_cv_t<Ty2>>,
                implements::compressed_pair_empty<Ty1>::value, implements::compressed_pair_empty<Ty2>::value>::value>;

        /**
         * @brief Inherit all constructors from the base implementation
         *        继承基类实现的所有构造函数
         */
        using base::base;

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        constexpr compressed_pair(const compressed_pair &) = default;

        /**
         * @brief Move constructor
         *        移动构造函数
         */
        constexpr compressed_pair(compressed_pair &&) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         *
         * @param other The other pair to copy from
         *              要拷贝的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(const compressed_pair &other)->compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other pair to move from
         *              要移动的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(compressed_pair &&other) noexcept(std::is_nothrow_move_assignable_v<Ty1> &&
                                                                      std::is_nothrow_move_assignable_v<Ty2>)
            ->compressed_pair & {
            utility::construct_in_place(this->get_first(), utility::move(other.get_first()));
            utility::construct_in_place(this->get_second(), utility::move(other.get_second()));
            return *this;
        }
    };

    /**
     * @brief Specialization for when both types are the same.
     *        Handles the case where Ty1 and Ty2 are identical.
     *
     *        当两个类型相同时的特化。
     *        处理 Ty1 和 Ty2 相同的情况。
     *
     * @tparam Ty The type of both elements
     *            两个元素的类型
     */
    template <typename Ty>
    class compressed_pair<Ty, Ty>
        : public implements::compressed_pair_impl<
              Ty, Ty,
              implements::compressed_pair_switch<
                  Ty, Ty,
                  type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, type_traits::cv_modify::remove_cv_t<Ty>>,
                  implements::compressed_pair_empty<Ty>::value, implements::compressed_pair_empty<Ty>::value>::value> {
    public:
        using base = implements::compressed_pair_impl<
            Ty, Ty,
            implements::compressed_pair_switch<
                Ty, Ty,
                type_traits::implements::is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, type_traits::cv_modify::remove_cv_t<Ty>>,
                implements::compressed_pair_empty<Ty>::value, implements::compressed_pair_empty<Ty>::value>::value>;

        /**
         * @brief Inherit all constructors from the base implementation
         *        继承基类实现的所有构造函数
         */
        using base::base;

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        constexpr compressed_pair(const compressed_pair &) = default;

        /**
         * @brief Move constructor
         *        移动构造函数
         */
        constexpr compressed_pair(compressed_pair &&) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         *
         * @param other The other pair to copy from
         *              要拷贝的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(const compressed_pair &other)->compressed_pair & {
            this->get_first() = other.get_first();
            this->get_second() = other.get_second();
            return *this;
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other pair to move from
         *              要移动的另一个对
         * @return Reference to this pair
         *         此对的引用
         */
        constexpr rain_fn operator=(compressed_pair &&other) noexcept(std::is_nothrow_move_assignable_v<Ty>)->compressed_pair & {
            this->get_first() = utility::move(other.get_first());
            this->get_second() = utility::move(other.get_second());
            return *this;
        }
    };

    /**
     * @brief Swaps two compressed pairs
     *        交换两个压缩对
     *
     * @tparam Ty1 The type of the first element
     *             第一个元素的类型
     * @tparam Ty2 The type of the second element
     *             第二个元素的类型
     * @param left The first pair
     *             第一个对
     * @param right The second pair
     *              第二个对
     */
    template <typename Ty1, typename Ty2>
    constexpr rain_fn swap(compressed_pair<Ty1, Ty2> &left, compressed_pair<Ty1, Ty2> &right) -> void {
        left.swap(right);
    }
}

#endif