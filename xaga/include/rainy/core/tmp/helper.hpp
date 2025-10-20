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
#ifndef RAINY_CORE_TMP_HELPER_HPP
#define RAINY_CORE_TMP_HELPER_HPP
#include <rainy/core/tmp/implements.hpp>
#include <rainy/core/tmp/modifers.hpp>

// type_traits的帮助程序类和 typedef
namespace rainy::type_traits::helper {
    /**
     * @brief 从类型和值生成整型常量。
     * @tparam Ty 类型
     * @tparam Data 值
     */
    template <typename Ty, Ty Data>
    struct integral_constant {
        using value_type = Ty;
        using type = integral_constant;

        constexpr explicit operator value_type() const noexcept {
            return value;
        }
        constexpr value_type operator()() const noexcept {
            return value;
        }

        static constexpr Ty value = Data;
    };

    /**
     * @brief 将 bool 用作 Ty 参数的 integral_constant 的显式部分特化
     */
    template <bool Boolean>
    using bool_constant = integral_constant<bool, Boolean>;

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    template <typename>
    struct char_space : integral_constant<char, ' '> {};

    template <>
    struct char_space<wchar_t> : integral_constant<wchar_t, ' '> {};

    template <>
    struct char_space<char16_t> : integral_constant<char16_t, u' '> {};

    template <>
    struct char_space<char32_t> : integral_constant<char32_t, u' '> {};

    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_space_v = char_space<CharType>::value;

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    template <>
    struct char_space<char8_t> : integral_constant<char8_t, ' '> {};
#endif
    template <typename CharType>
    struct char_null : integral_constant<char, '\0'> {};

    template <>
    struct char_null<wchar_t> : integral_constant<wchar_t, '\0'> {};

    template <>
    struct char_null<char16_t> : integral_constant<char16_t, u'\0'> {};

    template <>
    struct char_null<char32_t> : integral_constant<char32_t, u'\0'> {};

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    template <>
    struct char_null<char8_t> : integral_constant<char8_t, '\0'> {};
#endif

    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_null_v = char_null<CharType>::value;

    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_wchar_t = implements::is_same_v<CharType, wchar_t>;

    template <typename Ty>
    struct wrapper {
        inline static cv_modify::remove_cvref_t<Ty> value;
    };

    template <typename Ty>
    constexpr cv_modify::remove_cvref_t<Ty> &get_fake_object() noexcept {
        return wrapper<cv_modify::remove_cvref_t<Ty>>::value;
    }

    template <typename Ty>
    struct identity {
        using type = Ty;
    };

    template <typename Ty>
    using identity_t = typename identity<Ty>::type;

    template <typename Ty, Ty... Vals>
    struct integer_sequence {
        static_assert(implements::is_integral_v<Ty>, "integer_sequence<T, I...> requires T to be an integral type.");

        using value_type = Ty;

        RAINY_NODISCARD static constexpr std::size_t size() noexcept {
            return sizeof...(Vals);
        }
    };

    template <typename T, T N>
    using make_integer_sequence = typename core::builtin::make_integer_seq<integer_sequence, T, N>::type;

    template <std::size_t... Vals>
    using index_sequence = integer_sequence<std::size_t, Vals...>;

    template <std::size_t Size>
    using make_index_sequence = make_integer_sequence<std::size_t, Size>;

    template <typename... Types>
    using index_sequence_for = make_index_sequence<sizeof...(Types)>;

    class RAINY_TOOLKIT_API non_copyable {
    protected:
        non_copyable() = default;
        ~non_copyable() = default;
        non_copyable(const non_copyable &) = delete;
        non_copyable &operator=(const non_copyable &) = delete;
        non_copyable(non_copyable &&) = default;
        non_copyable &operator=(non_copyable &&) = default;
    };

    class RAINY_TOOLKIT_API non_moveable {
    protected:
        non_moveable() = default;
        ~non_moveable() = default;
        non_moveable(non_moveable &&) = delete;
        non_moveable &operator=(non_moveable &&) = delete;
    };

    template <typename Ty>
    struct make_unsigned {
        static_assert(implements::is_integral_v<Ty> || implements::_is_enum_v<Ty>,
                      "make_unsigned<T> requires Ty to be an integral or enum type");

        using type = implements::_conditional_t<
            implements::_is_const_v<Ty>,
            typename implements::_add_const<implements::make_unsigned_raw<implements::remove_cv_t<Ty>>>::type,
            implements::make_unsigned_raw<implements::remove_cv_t<Ty>>>;
    };

    template <typename Ty>
    using make_unsigned_t = typename make_unsigned<Ty>::type;

    template <typename Ty>
    struct make_signed {
        static_assert(implements::is_integral_v<Ty> || implements::_is_enum_v<Ty>,
                      "make_signed<T> requires Ty to be an integral or enum type");

        using type =
            implements::_conditional_t<implements::_is_const_v<Ty>,
                                       typename implements::_add_const<implements::make_signed_raw<implements::remove_cv_t<Ty>>>::type,
                                       implements::make_signed_raw<implements::remove_cv_t<Ty>>>;
    };

    template <typename Ty>
    using make_signed_t = typename make_signed<Ty>::type;
}

#endif