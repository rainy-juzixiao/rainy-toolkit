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
#ifndef RAINY_CORE_TYPE_TRAITS_HELPER_HPP
#define RAINY_CORE_TYPE_TRAITS_HELPER_HPP
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/modifers.hpp>

namespace rainy::type_traits::helper {
    /**
     * @brief Wrapper template for holding a static value of type Ty.
     *        用于持有类型Ty的静态值的包装器模板。
     *
     * @tparam Ty The type to wrap
     *            要包装的类型
     */
    template <typename Ty>
    struct wrapper {
        inline static cv_modify::remove_cvref_t<Ty> value;
    };

    /**
     * @brief Returns a reference to a fake object of type Ty.
     *        返回类型Ty的伪对象的引用。
     *
     * This function is useful in unevaluated contexts where a reference to an object
     * of type Ty is needed without actually constructing one.
     *
     * 此函数在未求值上下文中很有用，当需要类型Ty的对象的引用而不实际构造一个时。
     *
     * @tparam Ty The type of the fake object
     *            伪对象的类型
     * @return Reference to a static fake object
     *         静态伪对象的引用
     */
    template <typename Ty>
    constexpr cv_modify::remove_cvref_t<Ty> &get_fake_object() noexcept {
        return wrapper<cv_modify::remove_cvref_t<Ty>>::value;
    }

    /**
     * @brief Identity template that holds the type Ty.
     *        持有类型Ty的恒等模板。
     *
     * @tparam Ty The type to hold
     *            要持有的类型
     */
    template <typename Ty>
    struct identity {
        using type = Ty;
    };

    /**
     * @brief Alias template for identity, providing direct access to the held type.
     *        identity的别名模板，提供对持有类型的直接访问。
     *
     * @tparam Ty The type to hold
     *            要持有的类型
     */
    template <typename Ty>
    using identity_t = typename identity<Ty>::type;

    /**
     * @brief Compile-time sequence of integer values.
     *        整数值的编译时序列。
     *
     * @tparam Ty The integral type of the values
     *            值的整型类型
     * @tparam Vals The sequence of values
     *              值的序列
     */
    template <typename Ty, Ty... Vals>
    struct integer_sequence {
        static_assert(implements::is_integral_v<Ty>, "integer_sequence<T, I...> requires T to be an integral type.");

        using value_type = Ty;

        /**
         * @brief Returns the number of elements in the sequence.
         *        返回序列中的元素数量。
         *
         * @return The size of the sequence
         *         序列的大小
         */
        RAINY_NODISCARD static constexpr std::size_t size() noexcept {
            return sizeof...(Vals);
        }
    };

    /**
     * @brief Creates an integer_sequence of type T with values 0, 1, ..., N-1.
     *        创建类型T的integer_sequence，值为0, 1, ..., N-1。
     *
     * @tparam T The integral type
     *           整型类型
     * @tparam N The size of the sequence
     *           序列的大小
     */
    template <typename T, T N>
    using make_integer_sequence = typename core::builtin::make_integer_seq<integer_sequence, T, N>::type;

    /**
     * @brief Compile-time sequence of size_t indices.
     *        size_t索引的编译时序列。
     *
     * @tparam Vals The index values
     *              索引值
     */
    template <std::size_t... Vals>
    using index_sequence = integer_sequence<std::size_t, Vals...>;

    /**
     * @brief Creates an index_sequence of size Size with values 0, 1, ..., Size-1.
     *        创建大小为Size的index_sequence，值为0, 1, ..., Size-1。
     *
     * @tparam Size The size of the sequence
     *              序列的大小
     */
    template <std::size_t Size>
    using make_index_sequence = make_integer_sequence<std::size_t, Size>;

    /**
     * @brief Creates an index_sequence with the same size as the parameter pack Types.
     *        创建与参数包Types大小相同的index_sequence。
     *
     * @tparam Types The parameter pack
     *               参数包
     */
    template <typename... Types>
    using index_sequence_for = make_index_sequence<sizeof...(Types)>;

    /**
     * @brief Base class that makes derived classes non-copyable but movable.
     *        使派生类不可拷贝但可移动的基类。
     */
    class RAINY_TOOLKIT_API non_copyable {
    protected:
        non_copyable() = default;
        ~non_copyable() = default;
        non_copyable(const non_copyable &) = delete;
        non_copyable &operator=(const non_copyable &) = delete;
        non_copyable(non_copyable &&) = default;
        non_copyable &operator=(non_copyable &&) = default;
    };

    /**
     * @brief Base class that makes derived classes non-movable.
     *        使派生类不可移动的基类。
     */
    class RAINY_TOOLKIT_API non_moveable {
    protected:
        non_moveable() = default;
        ~non_moveable() = default;
        non_moveable(non_moveable &&) = delete;
        non_moveable &operator=(non_moveable &&) = delete;
    };

    /**
     * @brief Makes an unsigned version of an integral or enum type.
     *        生成整型或枚举类型的无符号版本。
     *
     * @tparam Ty The integral or enum type to convert
     *            要转换的整型或枚举类型
     */
    template <typename Ty>
    struct make_unsigned {
        static_assert(implements::is_integral_v<Ty> || implements::_is_enum_v<Ty>,
                      "make_unsigned<T> requires Ty to be an integral or enum type");

        using type = implements::_conditional_t<
            implements::_is_const_v<Ty>,
            typename implements::_add_const<implements::make_unsigned_raw<implements::remove_cv_t<Ty>>>::type,
            implements::make_unsigned_raw<implements::remove_cv_t<Ty>>>;
    };

    /**
     * @brief Alias template for make_unsigned, providing direct access to the unsigned type.
     *        make_unsigned的别名模板，提供对无符号类型的直接访问。
     *
     * @tparam Ty The integral or enum type to convert
     *            要转换的整型或枚举类型
     */
    template <typename Ty>
    using make_unsigned_t = typename make_unsigned<Ty>::type;

    /**
     * @brief Makes a signed version of an integral or enum type.
     *        生成整型或枚举类型的有符号版本。
     *
     * @tparam Ty The integral or enum type to convert
     *            要转换的整型或枚举类型
     */
    template <typename Ty>
    struct make_signed {
        static_assert(implements::is_integral_v<Ty> || implements::_is_enum_v<Ty>,
                      "make_signed<T> requires Ty to be an integral or enum type");

        using type =
            implements::_conditional_t<implements::_is_const_v<Ty>,
                                       typename implements::_add_const<implements::make_signed_raw<implements::remove_cv_t<Ty>>>::type,
                                       implements::make_signed_raw<implements::remove_cv_t<Ty>>>;
    };

    /**
     * @brief Alias template for make_signed, providing direct access to the signed type.
     *        make_signed的别名模板，提供对有符号类型的直接访问。
     *
     * @tparam Ty The integral or enum type to convert
     *            要转换的整型或枚举类型
     */
    template <typename Ty>
    using make_signed_t = typename make_signed<Ty>::type;
}

#endif