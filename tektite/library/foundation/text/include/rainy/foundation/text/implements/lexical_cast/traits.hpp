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
#ifndef RAINY_FOUNDATION_TEXT_TRAITS_HPP
#define RAINY_FOUNDATION_TEXT_TRAITS_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits/decay.hpp>
#include <rainy/core/type_traits/modifers.hpp>
#include <rainy/core/type_traits/primary_types.hpp>

namespace rainy::foundation::text::implements {
    /**
     * \lang english
     * @brief Normalizes a source type before the converter selection.
     *
     * References and cv qualifiers are dropped, while a character array decays into a constant character
     * pointer so that a string literal stays usable.
     *
     * @tparam Ty The type to normalize
     *
     * \lang simp-chinese
     * @brief 在挑选转换器之前归一化源类型。
     *
     * 去掉引用与 cv 限定符，字符数组退化为常量字符指针，从而字符串字面量依然可用。
     *
     * @tparam Ty 待归一化的类型
     */
    template <typename Ty>
    struct source_type_traits {
        using type = type_traits::other_trans::decay_t<Ty>;
    };

    /**
     * \lang english
     * @brief Normalization of a sized character array.
     *
     * @tparam Ty The element type
     * @tparam Size The number of elements
     *
     * \lang simp-chinese
     * @brief 定长字符数组的归一化。
     *
     * @tparam Ty 元素类型
     * @tparam Size 元素数量
     */
    template <typename Ty, std::size_t Size>
    struct source_type_traits<Ty[Size]> {
        using type = type_traits::other_trans::decay_t<const Ty[Size]>;
    };

    /**
     * \lang english
     * @brief Normalization of an unsized character array.
     *
     * @tparam Ty The element type
     *
     * \lang simp-chinese
     * @brief 无长度字符数组的归一化。
     *
     * @tparam Ty 元素类型
     */
    template <typename Ty>
    struct source_type_traits<Ty[]> {
        using type = type_traits::other_trans::decay_t<const Ty[]>;
    };

    /**
     * \lang english
     * @brief The normalized form of a source type.
     *
     * @tparam Ty The type to normalize
     *
     * \lang simp-chinese
     * @brief 源类型的归一化结果。
     *
     * @tparam Ty 待归一化的类型
     */
    template <typename Ty>
    using source_type_t = typename source_type_traits<Ty>::type;

    /**
     * \lang english
     * @brief Indicates whether Ty is a plain character type.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为普通字符类型。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_char_v =
        type_traits::type_relations::is_same_v<Ty, char> || type_traits::type_relations::is_same_v<Ty, wchar_t> ||
        type_traits::type_relations::is_same_v<Ty, char16_t> || type_traits::type_relations::is_same_v<Ty, char32_t>
#if RAINY_HAS_CXX20
        || type_traits::type_relations::is_same_v<Ty, char8_t>
#endif
        ;

    /**
     * \lang english
     * @brief Indicates whether Ty is a character type, the narrow signed and unsigned characters included.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为字符类型，包含有符号与无符号的窄字符。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_character_v = is_char_v<Ty> || type_traits::type_relations::is_same_v<Ty, signed char> ||
                                          type_traits::type_relations::is_same_v<Ty, unsigned char>;

    /**
     * \lang english
     * @brief Indicates whether Ty is a single byte type.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为单字节类型。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_byte_char_v =
        type_traits::type_relations::is_same_v<Ty, char> || type_traits::type_relations::is_same_v<Ty, signed char> ||
        type_traits::type_relations::is_same_v<Ty, unsigned char>
#if RAINY_HAS_CXX20
        || type_traits::type_relations::is_same_v<Ty, char8_t>
#endif
        ;

    /**
     * \lang english
     * @brief Indicates whether Ty is bool.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为 bool。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_bool_v = type_traits::type_relations::is_same_v<Ty, bool>;

    /**
     * \lang english
     * @brief Indicates whether Ty is an integral type that charconv can handle.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为 charconv 可处理的整数类型。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integer_v = type_traits::primary_types::is_integral_v<Ty> && !is_character_v<Ty> && !is_bool_v<Ty>;

    /**
     * \lang english
     * @brief Indicates whether Ty is a floating point type.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为浮点类型。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_float_v = type_traits::primary_types::is_floating_point_v<Ty> && !is_character_v<Ty>;

    /**
     * \lang english
     * @brief Indicates whether Ty is an arithmetic type handled by charconv.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为 charconv 处理的算术类型。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_number_v = is_integer_v<Ty> || is_float_v<Ty>;

    /**
     * \lang english
     * @brief Indicates whether Ty is rendered as text by the built in converters.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否会被内置转换器渲染成文本。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_number_source_v = is_number_v<Ty> || is_bool_v<Ty>;

    /**
     * \lang english
     * @brief Describes the string like types accepted as source and produced as target.
     *
     * @tparam Ty The type to inspect
     *
     * \lang simp-chinese
     * @brief 描述可作为源与目标的类字符串类型。
     *
     * @tparam Ty 待检查的类型
     */
    template <typename Ty>
    struct string_traits {
        static constexpr bool value = false;
        static constexpr bool is_view = false;
        using char_type = char;
    };

    /**
     * \lang english
     * @brief Specialization for the toolkit string.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Allocator The allocator type
     *
     * \lang simp-chinese
     * @brief 工具库字符串的特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Allocator 分配器类型
     */
    template <typename CharType, typename Traits, typename Allocator>
    struct string_traits<core::text::basic_string<CharType, Traits, Allocator>> {
        static constexpr bool value = true;
        static constexpr bool is_view = false;
        using char_type = CharType;
    };

    /**
     * \lang english
     * @brief Specialization for the toolkit string view.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 工具库字符串视图的特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     */
    template <typename CharType, typename Traits>
    struct string_traits<core::text::basic_string_view<CharType, Traits>> {
        static constexpr bool value = true;
        static constexpr bool is_view = true;
        using char_type = CharType;
    };

    /**
     * \lang english
     * @brief Specialization for the standard library string.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Allocator The allocator type
     *
     * \lang simp-chinese
     * @brief 标准库字符串的特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Allocator 分配器类型
     */
    template <typename CharType, typename Traits, typename Allocator>
    struct string_traits<std::basic_string<CharType, Traits, Allocator>> {
        static constexpr bool value = true;
        static constexpr bool is_view = false;
        using char_type = CharType;
    };

    /**
     * \lang english
     * @brief Specialization for the standard library string view.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 标准库字符串视图的特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     */
    template <typename CharType, typename Traits>
    struct string_traits<std::basic_string_view<CharType, Traits>> {
        static constexpr bool value = true;
        static constexpr bool is_view = true;
        using char_type = CharType;
    };

    /**
     * \lang english
     * @brief Indicates whether Ty is one of the supported string like types.
     *
     * @tparam Ty The type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否为受支持的类字符串类型。
     *
     * @tparam Ty 待判断的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_string_like_v = string_traits<Ty>::value;

    /**
     * \lang english
     * @brief Describes every type that can be read as a character sequence.
     *
     * The character sequence sources are the string like types, the character types, the null terminated
     * character strings and the character arrays.
     *
     * @tparam Ty The decayed type to inspect
     *
     * \lang simp-chinese
     * @brief 描述所有可按字符序列读取的类型。
     *
     * 字符序列源包括类字符串类型、字符类型、以空字符结尾的字符串与字符数组。
     *
     * @tparam Ty 待检查的退化类型
     */
    template <typename Ty, typename = void>
    struct char_sequence_traits {
        static constexpr bool value = false;
        using char_type = char;
    };

    /**
     * \lang english
     * @brief Specialization for the character types.
     *
     * @tparam Ty The character type
     *
     * \lang simp-chinese
     * @brief 字符类型的特化。
     *
     * @tparam Ty 字符类型
     */
    template <typename Ty>
    struct char_sequence_traits<Ty, type_traits::other_trans::enable_if_t<is_character_v<Ty>, void>> {
        static constexpr bool value = true;
        using char_type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for the string like types.
     *
     * @tparam Ty The string like type
     *
     * \lang simp-chinese
     * @brief 类字符串类型的特化。
     *
     * @tparam Ty 类字符串类型
     */
    template <typename Ty>
    struct char_sequence_traits<Ty, type_traits::other_trans::enable_if_t<string_traits<Ty>::value, void>> {
        static constexpr bool value = true;
        using char_type = typename string_traits<Ty>::char_type;
    };

    /**
     * \lang english
     * @brief Specialization for the null terminated character strings.
     *
     * @tparam Ty The pointer type
     *
     * \lang simp-chinese
     * @brief 以空字符结尾的字符串的特化。
     *
     * @tparam Ty 指针类型
     */
    template <typename Ty>
    struct char_sequence_traits<
        Ty, type_traits::other_trans::enable_if_t<
                type_traits::primary_types::is_pointer_v<Ty> &&
                    is_character_v<type_traits::modifers::remove_cv_t<type_traits::modifers::remove_pointer_t<Ty>>>,
                void>> {
        static constexpr bool value = true;
        using char_type = type_traits::modifers::remove_cv_t<type_traits::modifers::remove_pointer_t<Ty>>;
    };

    /**
     * \lang english
     * @brief Indicates whether Ty can be read as a character sequence.
     *
     * @tparam Ty The decayed type to test
     *
     * \lang simp-chinese
     * @brief 判断 Ty 是否可按字符序列读取。
     *
     * @tparam Ty 待判断的退化类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_char_sequence_v = char_sequence_traits<Ty>::value;

    /**
     * \lang english
     * @brief The character type of a character sequence.
     *
     * @tparam Ty The character sequence type
     *
     * \lang simp-chinese
     * @brief 字符序列的字符类型。
     *
     * @tparam Ty 字符序列类型
     */
    template <typename Ty>
    using char_sequence_char_t = typename char_sequence_traits<Ty>::char_type;

    template <typename Source, typename = void>
    struct text_char_type {
        using type = char;
    };

    template <typename Source>
    struct text_char_type<Source, type_traits::other_trans::enable_if_t<is_char_sequence_v<Source>, void>> {
        using type = char_sequence_char_t<Source>;
    };
}

#endif
