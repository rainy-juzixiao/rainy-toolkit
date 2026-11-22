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
#ifndef RAINY_FOUNDATION_TEXT_CONVERTER_HPP
#define RAINY_FOUNDATION_TEXT_CONVERTER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/text/implements/lexical_cast/caster.hpp>
#include <rainy/foundation/text/implements/lexical_cast/encoding.hpp>
#include <rainy/foundation/text/implements/lexical_cast/source.hpp>
#include <rainy/foundation/text/implements/lexical_cast/traits.hpp>

namespace rainy::foundation::text::implements {
    /**
     * \lang english
     * @brief Computes an exactly representable power of two in the floating point domain.
     * @tparam Ty The floating point type
     * @param exponent The exponent of the power of two
     * @return The value of the power of two
     *
     * \lang simp-chinese
     * @brief 在浮点域中计算可精确表示的 2 的幂。
     * @tparam Ty 浮点类型
     * @param exponent 2 的幂的指数
     * @return 该 2 的幂的值
     */
    template <typename Ty>
    RAINY_INLINE_CONSTEXPR rain_fn exact_power_of_two(int exponent) noexcept -> Ty {
        Ty result{1};
        for (int index = 0; index < exponent; ++index) {
            result *= Ty{2};
        }
        return result;
    }

    /**
     * \lang english
     * @brief Indicates whether a floating point value fits into an integral type without overflow.
     * @tparam Target The integral type
     * @tparam Source The floating point type
     * @param value The value to test
     * @return true when the value is in range
     *
     * \lang simp-chinese
     * @brief 判断浮点值是否能在不溢出的前提下放入整数类型。
     * @tparam Target 整数类型
     * @tparam Source 浮点类型
     * @param value 待判断的值
     * @return 值在范围内返回 true
     */
    template <typename Target, typename Source>
    bool float_in_range(Source value) noexcept {
        if (value != value) {
            return false;
        }
        constexpr int digits =
            static_cast<int>(sizeof(Target) * 8) - (type_traits::properties::is_signed_v<Target> ? 1 : 0);
        if constexpr (type_traits::properties::is_signed_v<Target>) {
            const Source bound = exact_power_of_two<Source>(digits);
            return value >= -bound && value < bound;
        } else {
            return value >= Source{0} && value < exact_power_of_two<Source>(digits);
        }
    }

    /**
     * \lang english
     * @brief Converts an arithmetic source into an arithmetic target without any text round trip.
     *
     * The conversion fails when it would lose information: the integral sources and targets must round trip,
     * a floating point source of a narrower type must round trip as well, and a floating point source must
     * hold an exactly integral value to reach an integral target.
     *
     * @tparam Target The arithmetic target type
     * @tparam Source The arithmetic source type
     *
     * \lang simp-chinese
     * @brief 在不经文本往返的情况下把算术源转换为算术目标。
     *
     * 会丢失信息的转换视为失败：整数源与整数目标必须往返，窄化后的浮点源也必须往返，
     * 浮点源转换为整数目标时必须持有整数值。
     *
     * @tparam Target 算术目标类型
     * @tparam Source 算术源类型
     */
    template <typename Target, typename Source>
    struct charconv_converter {
        /**
         * \lang english
         * @brief Performs the conversion.
         * @param source The value to convert
         * @param result The output value, written on success
         * @return true on success
         *
         * \lang simp-chinese
         * @brief 执行转换。
         * @param source 待转换的值
         * @param result 输出值，成功时写入
         * @return 成功返回 true
         */
        static bool convert(const Source &source, Target &result) noexcept {
            if constexpr (is_bool_v<Source>) {
                return charconv_converter<Target, int>::convert(static_cast<int>(source), result);
            } else if constexpr (is_float_v<Source> && !is_float_v<Target>) {
                if (!float_in_range<Target>(source)) {
                    return false;
                }
                const Target converted = static_cast<Target>(source);
                if (static_cast<Source>(converted) != source) {
                    return false;
                }
                result = converted;
                return true;
            } else if constexpr (is_float_v<Source> && is_float_v<Target>) {
                const Target converted = static_cast<Target>(source);
                const Source restored = static_cast<Source>(converted);
                if (restored == source || (restored != restored && source != source)) {
                    result = converted;
                    return true;
                }
                const Source magnitude = source < Source{0} ? -source : source;
                if (converted == Target{0} && magnitude < static_cast<Source>((utility::numeric_limits<Target>::min)())) {
                    result = converted;
                    return true;
                }
                return false;
            } else if constexpr (is_float_v<Target>) {
                result = static_cast<Target>(source);
                return true;
            } else {
                const Target converted = static_cast<Target>(source);
                if (static_cast<Source>(converted) != source) {
                    return false;
                }
                result = converted;
                return true;
            }
        }
    };

    /**
     * \lang english
     * @brief Parses a character sequence into an arithmetic target.
     *
     * The whole sequence must be consumed: a leading blank, a trailing garbage and a truncated exponent are
     * all rejected. A single leading plus sign is accepted because charconv only knows the minus sign.
     *
     * @tparam Target The arithmetic target type
     * @tparam Source The source type
     *
     * \lang simp-chinese
     * @brief 把字符序列解析为算术目标。
     *
     * 必须消费整个序列：前导空白、尾部残留与被截断的指数都判为失败。
     * 允许单个前导正号，因为 charconv 只认识负号。
     *
     * @tparam Target 算术目标类型
     * @tparam Source 源类型
     */
    template <typename Target, typename Source>
    struct charconv_parser {
        /**
         * \lang english
         * @brief Performs the parsing.
         * @param source The text to parse
         * @param result The output value, written on success
         * @return true on success
         *
         * \lang simp-chinese
         * @brief 执行解析。
         * @param source 待解析的文本
         * @param result 输出值，成功时写入
         * @return 成功返回 true
         */
        static bool convert(const Source &source, Target &result) {
            using char_type = typename text_view<Source>::char_type;
            text_view<Source> text{source};
            if (!text) {
                return false;
            }
            const auto view = text.view();
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                return parse(core::text::string_view{view.data(), view.size()}, result);
            } else {
                core::text::string utf8;
                if (!to_utf8(view, utf8)) {
                    return false;
                }
                return parse(utf8, result);
            }
        }

    private:
        static bool parse(core::text::string_view input, Target &result) noexcept {
            if (input.empty()) {
                return false;
            }
            const char *begin = input.data();
            const char *end = begin + input.size();
            if (*begin == '+') {
                ++begin;
                if (begin != end && (*begin == '+' || *begin == '-')) {
                    return false;
                }
            }
            if constexpr (is_float_v<Target>) {
                auto parsed = core::text::from_chars(begin, end, result);
                return parsed.ec == std::errc{} && parsed.ptr == end;
            } else {
                auto parsed = core::text::from_chars(begin, end, result, 10);
                return parsed.ec == std::errc{} && parsed.ptr == end;
            }
        }
    };

    /**
     * \lang english
     * @brief Reads a character sequence into a bool target.
     *
     * Only the digits zero and one are accepted, leading zeros and a single leading sign included.
     * The spellings true and false are rejected.
     *
     * @tparam Target The bool target type
     * @tparam Source The source type
     *
     * \lang simp-chinese
     * @brief 把字符序列读入 bool 目标。
     *
     * 只接受数字 0 与 1，包括前导零与单个前导符号。
     * true 与 false 两种写法一律拒绝。
     *
     * @tparam Target bool 目标类型
     * @tparam Source 源类型
     */
    template <typename Target, typename Source>
    struct boolean_converter {
        /**
         * \lang english
         * @brief Performs the parsing.
         * @param source The text to parse
         * @param result The output value, written on success
         * @return true on success
         *
         * \lang simp-chinese
         * @brief 执行解析。
         * @param source 待解析的文本
         * @param result 输出值，成功时写入
         * @return 成功返回 true
         */
        static bool convert(const Source &source, Target &result) noexcept {
            using char_type = typename text_view<Source>::char_type;
            text_view<Source> text{source};
            if (!text) {
                return false;
            }
            const auto view = text.view();
            if (view.empty()) {
                return false;
            }
            const char_type *begin = view.data();
            const char_type *last = begin + view.size() - 1;
            const bool value = *last == static_cast<char_type>('1');
            if (!value && *last != static_cast<char_type>('0')) {
                return false;
            }
            if (begin != last && (*begin == static_cast<char_type>('+') || (*begin == static_cast<char_type>('-') && !value))) {
                ++begin;
            }
            while (begin != last) {
                if (*begin != static_cast<char_type>('0')) {
                    return false;
                }
                ++begin;
            }
            result = value;
            return true;
        }
    };

    /**
     * \lang english
     * @brief Reads a character sequence into a character target.
     *
     * The sequence must hold exactly one code unit. Narrowing between the character types is rejected, a
     * wider source is decoded through UTF-8 instead.
     *
     * @tparam Target The character target type
     * @tparam Source The source type
     *
     * \lang simp-chinese
     * @brief 把字符序列读入字符目标。
     *
     * 序列必须恰好包含一个码元。字符类型之间的窄化一律拒绝，更宽的源改为经 UTF-8 解码。
     *
     * @tparam Target 字符目标类型
     * @tparam Source 源类型
     */
    template <typename Target, typename Source>
    struct character_converter {
        /**
         * \lang english
         * @brief Performs the conversion.
         * @param source The text to convert
         * @param result The output character, written on success
         * @return true on success
         *
         * \lang simp-chinese
         * @brief 执行转换。
         * @param source 待转换的文本
         * @param result 输出字符，成功时写入
         * @return 成功返回 true
         */
        static bool convert(const Source &source, Target &result) {
            using char_type = typename text_view<Source>::char_type;
            text_view<Source> text{source};
            if (!text) {
                return false;
            }
            const auto view = text.view();
            if (view.size() != 1) {
                return false;
            }
            if constexpr (type_traits::type_relations::is_same_v<char_type, Target> || sizeof(char_type) == sizeof(Target)) {
                result = static_cast<Target>(view[0]);
                return true;
            } else {
                core::text::basic_string<Target> converted;
                if (!convert_encoding(view, converted) || converted.size() != 1) {
                    return false;
                }
                result = static_cast<Target>(converted[0]);
                return true;
            }
        }
    };

    /**
     * \lang english
     * @brief Writes a source value into a string like target.
     *
     * A view target cannot own the result of an encoding conversion, so a cross character type conversion
     * into a view target is reported as a failure.
     *
     * @tparam Target The string like target type
     * @tparam Source The source type
     *
     * \lang simp-chinese
     * @brief 把源值写入类字符串目标。
     *
     * 视图目标无法持有编码转换的结果，因此跨字符类型转换到视图目标会报告失败。
     *
     * @tparam Target 类字符串目标类型
     * @tparam Source 源类型
     */
    template <typename Target, typename Source>
    struct string_converter {
        /**
         * \lang english
         * @brief Performs the conversion.
         * @param source The value to convert
         * @param result The output string, written on success
         * @return true on success
         *
         * \lang simp-chinese
         * @brief 执行转换。
         * @param source 待转换的值
         * @param result 输出字符串，成功时写入
         * @return 成功返回 true
         */
        static bool convert(const Source &source, Target &result) {
            using char_type = typename text_view<Source>::char_type;
            using target_char = typename string_traits<Target>::char_type;
            if constexpr (string_traits<Target>::is_view && !is_char_sequence_v<Source>) {
                return false;
            }
            text_view<Source> text{source};
            if (!text) {
                return false;
            }
            const auto view = text.view();
            if constexpr (type_traits::type_relations::is_same_v<char_type, target_char>) {
                assign_string(result, view.data(), view.size());
                return true;
            } else if constexpr (string_traits<Target>::is_view) {
                return false;
            } else {
                core::text::basic_string<target_char> converted;
                if (!convert_encoding(view, converted)) {
                    return false;
                }
                assign_string(result, converted.data(), converted.size());
                return true;
            }
        }
    };

    /**
     * \lang english
     * @brief Forwards a character sequence to a user provided lexical_caster.
     *
     * @tparam Target The user defined target type
     * @tparam Source The source type
     *
     * \lang simp-chinese
     * @brief 把字符序列转发给用户提供的 lexical_caster。
     *
     * @tparam Target 用户自定义目标类型
     * @tparam Source 源类型
     */
    template <typename Target, typename Source>
    struct custom_caster {
        using char_type = char_sequence_char_t<Source>;

        static bool convert(const Source &source, Target &result) {
            return lexical_caster<Target, char_type>::convert(source_char_view(source), result);
        }
    };

    /**
     * \lang english
     * @brief Formats a user defined value into a string like target through a user provided lexical_caster.
     *
     * A view target cannot own the produced text, so it is reported as a failure.
     *
     * @tparam Target The string like target type
     * @tparam Source The user defined source type
     *
     * \lang simp-chinese
     * @brief 通过用户提供的 lexical_caster 把用户自定义值格式化为类字符串目标。
     *
     * 视图目标无法持有生成的文本，因此报告为失败。
     *
     * @tparam Target 类字符串目标类型
     * @tparam Source 用户自定义源类型
     */
    template <typename Target, typename Source>
    struct custom_string_converter {
        using target_char = typename string_traits<Target>::char_type;

        static bool convert(const Source &source, Target &result) {
            if constexpr (string_traits<Target>::is_view) {
                return false;
            } else {
                core::text::basic_string<target_char> converted = lexical_caster<Source, target_char>::to_string(source);
                assign_string(result, converted.data(), converted.size());
                return true;
            }
        }
    };
}

#endif
