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
#ifndef RAINY_FOUNDATION_TEXT_ESCAPE_HPP
#define RAINY_FOUNDATION_TEXT_ESCAPE_HPP
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/foundation/text/unicode/unicode.hpp>

namespace rainy::foundation::text::escape {
    /**
     * \lang english
     * @brief The reason an unescape step failed.
     *
     * \lang simp-chinese
     * @brief 一次反转义步骤失败的原因。
     */
    enum class escape_error {
        /**
         * \lang english
         * @brief No error occurred.
         *
         * \lang simp-chinese
         * @brief 没有发生错误。
         */
        none,
        /**
         * \lang english
         * @brief The sequence ended in the middle of an escape.
         *
         * \lang simp-chinese
         * @brief 序列在转义的中间结束。
         */
        unexpected_end,
        /**
         * \lang english
         * @brief The character following a backslash is not a known escape.
         *
         * \lang simp-chinese
         * @brief 反斜杠之后的字符不是已知转义。
         */
        unknown_escape,
        /**
         * \lang english
         * @brief A hexadecimal escape contained a character that is not a hex digit.
         *
         * \lang simp-chinese
         * @brief 十六进制转义含有非十六进制数字的字符。
         */
        invalid_hex_digit,
        /**
         * \lang english
         * @brief A \uXXXX escape or surrogate pair does not form a valid code point.
         *
         * \lang simp-chinese
         * @brief \uXXXX 转义或代理对未构成有效码点。
         */
        invalid_code_point
    };

    namespace implements {
        template <typename CharType>
        RAINY_CONSTEXPR_BOOL is_wide_v = type_traits::type_relations::is_same_v<CharType, wchar_t>;

        template <typename CharType>
        constexpr unsigned long code_unit_value(CharType unit) {
            return static_cast<unsigned long>(unit);
        }

        constexpr bool is_hex_digit(unsigned long unit) {
            return (unit >= '0' && unit <= '9') || (unit >= 'a' && unit <= 'f') || (unit >= 'A' && unit <= 'F');
        }

        constexpr unsigned long hex_digit_value(unsigned long unit) {
            if (unit >= '0' && unit <= '9') {
                return unit - '0';
            }
            if (unit >= 'a' && unit <= 'f') {
                return unit - 'a' + 10;
            }
            return unit - 'A' + 10;
        }

        template <typename CharType>
        CharType hex_digit_char(unsigned long value) {
            return static_cast<CharType>(value < 10 ? '0' + value : 'a' + (value - 10));
        }

        template <typename CharType, typename OutputIt>
        void put_hex4(OutputIt &out, unsigned long value) {
            for (int shift = 12; shift >= 0; shift -= 4) {
                *out++ = hex_digit_char<CharType>((value >> shift) & 0xF);
            }
        }

        template <typename CharType, typename OutputIt>
        void put_hex2(OutputIt &out, unsigned long value) {
            *out++ = hex_digit_char<CharType>((value >> 4) & 0xF);
            *out++ = hex_digit_char<CharType>(value & 0xF);
        }

        template <typename CharType>
        void put_hex4(core::text::basic_string<CharType> &out, unsigned long value) {
            for (int shift = 12; shift >= 0; shift -= 4) {
                out.push_back(hex_digit_char<CharType>((value >> shift) & 0xF));
            }
        }

        template <typename CharType>
        void put_hex2(core::text::basic_string<CharType> &out, unsigned long value) {
            out.push_back(hex_digit_char<CharType>((value >> 4) & 0xF));
            out.push_back(hex_digit_char<CharType>(value & 0xF));
        }

        template <typename CharType>
        void put_code_point(core::text::basic_string<CharType> &out, char32_t code_point) {
            unicode::implements::append_one<CharType, CharType>(code_point, out);
        }
    }

    /**
     * \lang english
     * @brief Escapes a sequence using the C dialect, appending to a string.
     *
     * The named escapes are \a \b \f \n \r \t \v \\ \" \' and \0 for a null
     * character. Any other code unit below 0x20, and 0x7F, becomes \xHH. All
     * other code units are copied unchanged.
     *
     * @tparam CharType The character type of the sequence
     * @param input The sequence to escape
     * @param output The escaped sequence, appended to
     *
     * \lang simp-chinese
     * @brief 用 C 方言转义序列，追加到字符串。
     *
     * 具名转义为 \a \b \f \n \r \t \v \\ \" \' 以及空字符的 \0。其余小于 0x20 的
     * 码元以及 0x7F 变为 \xHH。其它码元原样拷贝。
     *
     * @tparam CharType 序列的字符类型
     * @param input 待转义的序列
     * @param output 追加输出的转义序列
     */
    template <typename CharType>
    void escape_c(core::text::basic_string_view<CharType> input, core::text::basic_string<CharType> &output) {
        for (const CharType unit : input) {
            const unsigned long value = implements::code_unit_value(unit);
            switch (value) {
                case '\a': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('a')); break;
                case '\b': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('b')); break;
                case '\f': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('f')); break;
                case '\n': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('n')); break;
                case '\r': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('r')); break;
                case '\t': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('t')); break;
                case '\v': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('v')); break;
                case '\\': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('\\')); break;
                case '\'': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('\'')); break;
                case '"': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('"')); break;
                case 0: output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('0')); break;
                default:
                    if (value < 0x20 || value == 0x7F) {
                        output.push_back(static_cast<CharType>('\\'));
                        output.push_back(static_cast<CharType>('x'));
                        implements::put_hex2(output, value);
                    } else {
                        output.push_back(unit);
                    }
                    break;
            }
        }
    }

    /**
     * \lang english
     * @brief Escapes a sequence using the C dialect into an output iterator.
     *
     * @tparam CharType The character type of the sequence
     * @tparam OutputIt The output iterator type
     * @param input The sequence to escape
     * @param out The output iterator to write to
     * @return The advanced output iterator
     *
     * \lang simp-chinese
     * @brief 用 C 方言把序列转义到输出迭代器。
     *
     * @tparam CharType 序列的字符类型
     * @tparam OutputIt 输出迭代器类型
     * @param input 待转义的序列
     * @param out 写入的输出迭代器
     * @return 推进后的输出迭代器
     */
    template <typename CharType, typename OutputIt>
    OutputIt escape_c(core::text::basic_string_view<CharType> input, OutputIt out) {
        for (const CharType unit : input) {
            const unsigned long value = implements::code_unit_value(unit);
            if (value == '\a' || value == '\b' || value == '\f' || value == '\n' || value == '\r' || value == '\t' ||
                value == '\v') {
                static constexpr char letters[] = {'a', 'b', 'f', 'n', 'r', 't', 'v'};
                static constexpr unsigned long values[] = {'\a', '\b', '\f', '\n', '\r', '\t', '\v'};
                *out++ = static_cast<CharType>('\\');
                for (std::size_t index = 0; index < 7; ++index) {
                    if (values[index] == value) {
                        *out++ = static_cast<CharType>(letters[index]);
                        break;
                    }
                }
            } else if (value == '\\' || value == '\'' || value == '"') {
                *out++ = static_cast<CharType>('\\');
                *out++ = static_cast<CharType>(value);
            } else if (value == 0) {
                *out++ = static_cast<CharType>('\\');
                *out++ = static_cast<CharType>('0');
            } else if (value < 0x20 || value == 0x7F) {
                *out++ = static_cast<CharType>('\\');
                *out++ = static_cast<CharType>('x');
                implements::put_hex2<CharType>(out, value);
            } else {
                *out++ = static_cast<CharType>(unit);
            }
        }
        return out;
    }

    /**
     * \lang english
     * @brief Escapes a sequence using the JSON dialect, appending to a string.
     *
     * Following RFC 8259, the quotation mark, the backslash and every code unit
     * below 0x20 are escaped. The short escapes \b \f \n \r \t are used where
     * they exist, otherwise the code unit becomes \u00XX.
     *
     * @tparam CharType The character type of the sequence
     * @param input The sequence to escape
     * @param output The escaped sequence, appended to
     *
     * \lang simp-chinese
     * @brief 用 JSON 方言转义序列，追加到字符串。
     *
     * 依照 RFC 8259，双引号、反斜杠与所有小于 0x20 的码元都被转义。存在短转义
     * \b \f \n \r \t 时使用短转义，否则码元变为 \u00XX。
     *
     * @tparam CharType 序列的字符类型
     * @param input 待转义的序列
     * @param output 追加输出的转义序列
     */
    template <typename CharType>
    void escape_json(core::text::basic_string_view<CharType> input, core::text::basic_string<CharType> &output) {
        for (const CharType unit : input) {
            const unsigned long value = implements::code_unit_value(unit);
            switch (value) {
                case '"': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('"')); break;
                case '\\': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('\\')); break;
                case '\b': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('b')); break;
                case '\f': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('f')); break;
                case '\n': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('n')); break;
                case '\r': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('r')); break;
                case '\t': output.push_back(static_cast<CharType>('\\')); output.push_back(static_cast<CharType>('t')); break;
                default:
                    if (value < 0x20) {
                        output.push_back(static_cast<CharType>('\\'));
                        output.push_back(static_cast<CharType>('u'));
                        output.push_back(static_cast<CharType>('0'));
                        output.push_back(static_cast<CharType>('0'));
                        implements::put_hex2(output, value);
                    } else {
                        output.push_back(unit);
                    }
                    break;
            }
        }
    }

    /**
     * \lang english
     * @brief Escapes a sequence using the JSON dialect into an output iterator.
     *
     * @tparam CharType The character type of the sequence
     * @tparam OutputIt The output iterator type
     * @param input The sequence to escape
     * @param out The output iterator to write to
     * @return The advanced output iterator
     *
     * \lang simp-chinese
     * @brief 用 JSON 方言把序列转义到输出迭代器。
     *
     * @tparam CharType 序列的字符类型
     * @tparam OutputIt 输出迭代器类型
     * @param input 待转义的序列
     * @param out 写入的输出迭代器
     * @return 推进后的输出迭代器
     */
    template <typename CharType, typename OutputIt>
    OutputIt escape_json(core::text::basic_string_view<CharType> input, OutputIt out) {
        for (const CharType unit : input) {
            const unsigned long value = implements::code_unit_value(unit);
            using out_type = CharType;
            if (value == '"' || value == '\\') {
                *out++ = static_cast<out_type>('\\');
                *out++ = static_cast<out_type>(value);
            } else if (value == '\b' || value == '\f' || value == '\n' || value == '\r' || value == '\t') {
                static constexpr char letters[] = {'b', 'f', 'n', 'r', 't'};
                static constexpr unsigned long values[] = {'\b', '\f', '\n', '\r', '\t'};
                *out++ = static_cast<out_type>('\\');
                for (std::size_t index = 0; index < 5; ++index) {
                    if (values[index] == value) {
                        *out++ = static_cast<out_type>(letters[index]);
                        break;
                    }
                }
            } else if (value < 0x20) {
                *out++ = static_cast<out_type>('\\');
                *out++ = static_cast<out_type>('u');
                *out++ = static_cast<out_type>('0');
                *out++ = static_cast<out_type>('0');
                implements::put_hex2<CharType>(out, value);
            } else {
                *out++ = static_cast<out_type>(unit);
            }
        }
        return out;
    }

    /**
     * \lang english
     * @brief Reads four hexadecimal digits and produces their value.
     *
     * @tparam CharType The character type of the sequence
     * @param data Pointer to the first digit
     * @param size The number of remaining characters
     * @param value On output, the parsed value
     * @return The error found, none on success
     *
     * \lang simp-chinese
     * @brief 读取四个十六进制数字并给出其值。
     *
     * @tparam CharType 序列的字符类型
     * @param data 指向首个数字的指针
     * @param size 剩余字符数
     * @param value 输出解析出的值
     * @return 发现的错误，成功时为 none
     */
    template <typename CharType>
    escape_error read_hex4(const CharType *data, std::size_t size, unsigned long &value) {
        if (size < 4) {
            return escape_error::unexpected_end;
        }
        value = 0;
        for (std::size_t index = 0; index < 4; ++index) {
            const unsigned long unit = implements::code_unit_value(data[index]);
            if (!implements::is_hex_digit(unit)) {
                return escape_error::invalid_hex_digit;
            }
            value = (value << 4) | implements::hex_digit_value(unit);
        }
        return escape_error::none;
    }

    /**
     * \lang english
     * @brief Reverses a C dialect escape, appending to a string.
     *
     * The sequence is parsed strictly: an escape that is not recognised, a
     * hexadecimal escape without two digits or a \uXXXX escape that is not a
     * valid code point reports an error and leaves the output partially filled.
     *
     * @tparam CharType The character type of the sequence
     * @param input The escaped sequence
     * @param output The unescaped sequence, appended to
     * @param error On output, none on success, the first error otherwise
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 反转 C 方言转义，追加到字符串。
     *
     * 序列严格解析：无法识别的转义、缺少两位数字的十六进制转义、或非有效码点的
     * \uXXXX 转义会报告错误，输出为部分填充。
     *
     * @tparam CharType 序列的字符类型
     * @param input 已转义的序列
     * @param output 追加输出的反转义序列
     * @param error 输出错误，成功时为 none
     * @return 成功返回 true
     */
    template <typename CharType>
    bool unescape_c(core::text::basic_string_view<CharType> input, core::text::basic_string<CharType> &output,
                    escape_error &error) {
        error = escape_error::none;
        const CharType *data = input.data();
        const std::size_t size = input.size();
        std::size_t offset = 0;
        while (offset < size) {
            const unsigned long unit = implements::code_unit_value(data[offset]);
            if (unit != '\\') {
                output.push_back(data[offset]);
                ++offset;
                continue;
            }
            if (offset + 1 >= size) {
                error = escape_error::unexpected_end;
                return false;
            }
            const unsigned long next = implements::code_unit_value(data[offset + 1]);
            switch (next) {
                case 'a': output.push_back(static_cast<CharType>('\a')); offset += 2; break;
                case 'b': output.push_back(static_cast<CharType>('\b')); offset += 2; break;
                case 'f': output.push_back(static_cast<CharType>('\f')); offset += 2; break;
                case 'n': output.push_back(static_cast<CharType>('\n')); offset += 2; break;
                case 'r': output.push_back(static_cast<CharType>('\r')); offset += 2; break;
                case 't': output.push_back(static_cast<CharType>('\t')); offset += 2; break;
                case 'v': output.push_back(static_cast<CharType>('\v')); offset += 2; break;
                case '\\': output.push_back(static_cast<CharType>('\\')); offset += 2; break;
                case '\'': output.push_back(static_cast<CharType>('\'')); offset += 2; break;
                case '"': output.push_back(static_cast<CharType>('"')); offset += 2; break;
                case '0': output.push_back(static_cast<CharType>(0)); offset += 2; break;
                case 'x': {
                    if (offset + 3 >= size) {
                        error = escape_error::unexpected_end;
                        return false;
                    }
                    unsigned long value = 0;
                    for (std::size_t index = 0; index < 2; ++index) {
                        const unsigned long digit = implements::code_unit_value(data[offset + 2 + index]);
                        if (!implements::is_hex_digit(digit)) {
                            error = escape_error::invalid_hex_digit;
                            return false;
                        }
                        value = (value << 4) | implements::hex_digit_value(digit);
                    }
                    output.push_back(static_cast<CharType>(value));
                    offset += 4;
                    break;
                }
                case 'u': {
                    unsigned long value = 0;
                    error = read_hex4(data + offset + 2, size - (offset + 2), value);
                    if (error != escape_error::none) {
                        return false;
                    }
                    char32_t code_point = static_cast<char32_t>(value);
                    offset += 6;
                    if (value >= 0xD800 && value <= 0xDBFF) {
                        if (offset + 1 >= size || implements::code_unit_value(data[offset]) != '\\' ||
                            implements::code_unit_value(data[offset + 1]) != 'u') {
                            error = escape_error::invalid_code_point;
                            return false;
                        }
                        unsigned long trail = 0;
                        error = read_hex4(data + offset + 2, size - (offset + 2), trail);
                        if (error != escape_error::none) {
                            return false;
                        }
                        if (trail < 0xDC00 || trail > 0xDFFF) {
                            error = escape_error::invalid_code_point;
                            return false;
                        }
                        code_point = static_cast<char32_t>(0x10000 + ((value - 0xD800) << 10) + (trail - 0xDC00));
                        offset += 6;
                    } else if (value >= 0xDC00 && value <= 0xDFFF) {
                        error = escape_error::invalid_code_point;
                        return false;
                    }
                    implements::put_code_point(output, code_point);
                    break;
                }
                default:
                    error = escape_error::unknown_escape;
                    return false;
            }
        }
        return true;
    }

    /**
     * \lang english
     * @brief Reverses a JSON dialect escape, appending to a string.
     *
     * The escape set of RFC 8259 is accepted: \" \\ \/ \b \f \n \r \t and
     * \uXXXX including surrogate pairs. A \uXXXX escape that is not a valid
     * code point, or any other escape, reports an error.
     *
     * @tparam CharType The character type of the sequence
     * @param input The escaped sequence
     * @param output The unescaped sequence, appended to
     * @param error On output, none on success, the first error otherwise
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 反转 JSON 方言转义，追加到字符串。
     *
     * 接受 RFC 8259 的转义集：\" \\ \/ \b \f \n \r \t 以及含代理对的 \uXXXX。
     * 非有效码点的 \uXXXX 转义或其它转义会报告错误。
     *
     * @tparam CharType 序列的字符类型
     * @param input 已转义的序列
     * @param output 追加输出的反转义序列
     * @param error 输出错误，成功时为 none
     * @return 成功返回 true
     */
    template <typename CharType>
    bool unescape_json(core::text::basic_string_view<CharType> input, core::text::basic_string<CharType> &output,
                       escape_error &error) {
        error = escape_error::none;
        const CharType *data = input.data();
        const std::size_t size = input.size();
        std::size_t offset = 0;
        while (offset < size) {
            const unsigned long unit = implements::code_unit_value(data[offset]);
            if (unit != '\\') {
                output.push_back(data[offset]);
                ++offset;
                continue;
            }
            if (offset + 1 >= size) {
                error = escape_error::unexpected_end;
                return false;
            }
            const unsigned long next = implements::code_unit_value(data[offset + 1]);
            switch (next) {
                case '"': output.push_back(static_cast<CharType>('"')); offset += 2; break;
                case '\\': output.push_back(static_cast<CharType>('\\')); offset += 2; break;
                case '/': output.push_back(static_cast<CharType>('/')); offset += 2; break;
                case 'b': output.push_back(static_cast<CharType>('\b')); offset += 2; break;
                case 'f': output.push_back(static_cast<CharType>('\f')); offset += 2; break;
                case 'n': output.push_back(static_cast<CharType>('\n')); offset += 2; break;
                case 'r': output.push_back(static_cast<CharType>('\r')); offset += 2; break;
                case 't': output.push_back(static_cast<CharType>('\t')); offset += 2; break;
                case 'u': {
                    unsigned long value = 0;
                    error = read_hex4(data + offset + 2, size - (offset + 2), value);
                    if (error != escape_error::none) {
                        return false;
                    }
                    char32_t code_point = static_cast<char32_t>(value);
                    offset += 6;
                    if (value >= 0xD800 && value <= 0xDBFF) {
                        if (offset + 1 >= size || implements::code_unit_value(data[offset]) != '\\' ||
                            implements::code_unit_value(data[offset + 1]) != 'u') {
                            error = escape_error::invalid_code_point;
                            return false;
                        }
                        unsigned long trail = 0;
                        error = read_hex4(data + offset + 2, size - (offset + 2), trail);
                        if (error != escape_error::none) {
                            return false;
                        }
                        if (trail < 0xDC00 || trail > 0xDFFF) {
                            error = escape_error::invalid_code_point;
                            return false;
                        }
                        code_point = static_cast<char32_t>(0x10000 + ((value - 0xD800) << 10) + (trail - 0xDC00));
                        offset += 6;
                    } else if (value >= 0xDC00 && value <= 0xDFFF) {
                        error = escape_error::invalid_code_point;
                        return false;
                    }
                    implements::put_code_point(output, code_point);
                    break;
                }
                default:
                    error = escape_error::unknown_escape;
                    return false;
            }
        }
        return true;
    }
}

namespace rainy::text {
    using foundation::text::escape::escape_c;
    using foundation::text::escape::escape_error;
    using foundation::text::escape::escape_json;
    using foundation::text::escape::read_hex4;
    using foundation::text::escape::unescape_c;
    using foundation::text::escape::unescape_json;
}

#endif
