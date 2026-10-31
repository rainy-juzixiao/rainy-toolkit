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
#ifndef RAINY_FOUNDATION_TEXT_UNICODE_HPP
#define RAINY_FOUNDATION_TEXT_UNICODE_HPP
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/text/implements/lexical_cast/traits.hpp>
#include <rainy/foundation/text/unicode/errors.hpp>

namespace rainy::foundation::text::unicode {

    /**
     * \lang english
     * @brief The number of code units each encoding uses per element.
     *
     * \lang simp-chinese
     * @brief 各编码每个元素占用的码元数量语义标识。
     */
    enum class encoding_form {
        /**
         * \lang english
         * @brief The UTF-8 encoding form, one to four bytes per code point.
         *
         * \lang simp-chinese
         * @brief UTF-8 编码形式，每个码点一至四个字节。
         */
        utf8,
        /**
         * \lang english
         * @brief The UTF-16 encoding form, one or two code units per code point.
         *
         * \lang simp-chinese
         * @brief UTF-16 编码形式，每个码点一或两个码元。
         */
        utf16,
        /**
         * \lang english
         * @brief The UTF-32 encoding form, one code unit per code point.
         *
         * \lang simp-chinese
         * @brief UTF-32 编码形式，每个码点一个码元。
         */
        utf32
    };

    /**
     * \lang english
     * @brief The replacement code point emitted by the permissive entry points.
     *
     * \lang simp-chinese
     * @brief 宽容入口发出的替换码点。
     */
    inline constexpr char32_t replacement_code_point = U'�';

    /**
     * \lang english
     * @brief The encoding form a character type belongs to.
     *
     * @tparam CharType The character type to inspect
     *
     * \lang simp-chinese
     * @brief 字符类型所属的编码形式。
     *
     * @tparam CharType 待检查的字符类型
     */
    template <typename CharType>
    RAINY_INLINE_CONSTEXPR encoding_form encoding_form_of_v =
        implements::is_byte_char_v<CharType> ? encoding_form::utf8
        : type_traits::type_relations::is_same_v<CharType, char16_t>
            ? encoding_form::utf16
            : type_traits::type_relations::is_same_v<CharType, char32_t> ? encoding_form::utf32 : encoding_form::utf16;

    template <>
    RAINY_INLINE_CONSTEXPR encoding_form encoding_form_of_v<wchar_t> =
        WCHAR_MAX <= 0xFFFF ? encoding_form::utf16 : encoding_form::utf32;
}

namespace rainy::foundation::text::unicode::implements {
    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_wide_char_v = type_traits::type_relations::is_same_v<CharType, wchar_t>;

    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_char_kind_v = ::rainy::foundation::text::implements::is_char_v<CharType>;

    template <typename CharType>
    using wide_alias_of_t =
        type_traits::other_trans::conditional_t<type_traits::type_relations::is_same_v<CharType, char16_t>, char16_t,
                                                type_traits::other_trans::conditional_t<
                                                    type_traits::type_relations::is_same_v<CharType, char32_t>, char32_t, wchar_t>>;

    template <typename CharType, typename = void>
    struct sequence_traits {
        static constexpr bool value = false;
        using char_type = char;
    };

    template <typename CharType>
    struct sequence_traits<CharType, type_traits::other_trans::enable_if_t<is_char_kind_v<CharType>, void>> {
        static constexpr bool value = true;
        using char_type = CharType;
    };

    template <typename CharType, typename Traits>
    struct sequence_traits<core::text::basic_string_view<CharType, Traits>> {
        static constexpr bool value = true;
        using char_type = CharType;
    };

    template <typename CharType, typename Traits, typename Allocator>
    struct sequence_traits<core::text::basic_string<CharType, Traits, Allocator>> {
        static constexpr bool value = true;
        using char_type = CharType;
    };

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_sequence_v = sequence_traits<Ty>::value;

    template <typename Ty>
    using sequence_char_t = typename sequence_traits<Ty>::char_type;

    /**
     * \lang english
     * @brief Decodes one code point from a UTF-8 sequence.
     * @param data Pointer to the leading byte
     * @param size The number of remaining bytes
     * @param code_point On output, the decoded code point
     * @param consumed On output, the number of bytes consumed
     * @return The error found, none on success
     *
     * \lang simp-chinese
     * @brief 从 UTF-8 序列解码一个码点。
     * @param data 指向首字节的指针
     * @param size 剩余字节数
     * @param code_point 输出解码出的码点
     * @param consumed 输出已消费的字节数
     * @return 发现的错误，成功时为 none
     */
    template <typename CharType>
    encoding_error decode_utf8(const CharType *data, std::size_t size, char32_t &code_point, std::size_t &consumed) {
        const unsigned char lead = static_cast<unsigned char>(data[0]);
        std::size_t needed = 0;
        char32_t value = 0;
        if (lead < 0x80) {
            value = lead;
            needed = 0;
        } else if (lead < 0xC0) {
            return encoding_error::invalid_leading_element;
        } else if (lead < 0xE0) {
            value = lead & 0x1F;
            needed = 1;
        } else if (lead < 0xF0) {
            value = lead & 0x0F;
            needed = 2;
        } else if (lead < 0xF8) {
            value = lead & 0x07;
            needed = 3;
        } else {
            return encoding_error::invalid_leading_element;
        }
        if (size < needed + 1) {
            return encoding_error::truncated_sequence;
        }
        for (std::size_t index = 1; index <= needed; ++index) {
            const unsigned char unit = static_cast<unsigned char>(data[index]);
            if ((unit & 0xC0) != 0x80) {
                return encoding_error::invalid_continuation_element;
            }
            value = (value << 6) | (unit & 0x3F);
        }
        if (needed >= 1 && value <= (needed == 1 ? 0x7F : needed == 2 ? 0x7FF : 0xFFFF)) {
            return encoding_error::overlong_sequence;
        }
        if (value >= 0xD800 && value <= 0xDFFF) {
            return encoding_error::invalid_code_point;
        }
        if (value > 0x10FFFF) {
            return encoding_error::invalid_code_point;
        }
        code_point = value;
        consumed = needed + 1;
        return encoding_error::none;
    }

    /**
     * \lang english
     * @brief Decodes one code point from a UTF-16 sequence.
     * @param data Pointer to the leading code unit
     * @param size The number of remaining code units
     * @param code_point On output, the decoded code point
     * @param consumed On output, the number of code units consumed
     * @return The error found, none on success
     *
     * \lang simp-chinese
     * @brief 从 UTF-16 序列解码一个码点。
     * @param data 指向首码元的指针
     * @param size 剩余码元数
     * @param code_point 输出解码出的码点
     * @param consumed 输出已消费的码元数
     * @return 发现的错误，成功时为 none
     */
    template <typename CharType>
    encoding_error decode_utf16(const CharType *data, std::size_t size, char32_t &code_point, std::size_t &consumed) {
        const char32_t lead = static_cast<char32_t>(static_cast<unsigned short>(data[0]));
        if (lead >= 0xDC00 && lead <= 0xDFFF) {
            return encoding_error::invalid_leading_element;
        }
        if (lead >= 0xD800 && lead <= 0xDBFF) {
            if (size < 2) {
                return encoding_error::truncated_sequence;
            }
            const char32_t trail = static_cast<char32_t>(static_cast<unsigned short>(data[1]));
            if (trail < 0xDC00 || trail > 0xDFFF) {
                return encoding_error::invalid_continuation_element;
            }
            code_point = 0x10000 + ((lead - 0xD800) << 10) + (trail - 0xDC00);
            consumed = 2;
            return encoding_error::none;
        }
        code_point = lead;
        consumed = 1;
        return encoding_error::none;
    }

    /**
     * \lang english
     * @brief Decodes one code point from a UTF-32 sequence.
     * @param data Pointer to the code unit
     * @param code_point On output, the decoded code point
     * @param consumed On output, the number of code units consumed
     * @return The error found, none on success
     *
     * \lang simp-chinese
     * @brief 从 UTF-32 序列解码一个码点。
     * @param data 指向码元的指针
     * @param code_point 输出解码出的码点
     * @param consumed 输出已消费的码元数
     * @return 发现的错误，成功时为 none
     */
    template <typename CharType>
    encoding_error decode_utf32(const CharType *data, char32_t &code_point, std::size_t &consumed) {
        const char32_t value = static_cast<char32_t>(data[0]);
        if (value >= 0xD800 && value <= 0xDFFF) {
            return encoding_error::invalid_code_point;
        }
        if (value > 0x10FFFF) {
            return encoding_error::invalid_code_point;
        }
        code_point = value;
        consumed = 1;
        return encoding_error::none;
    }

    /**
     * \lang english
     * @brief Dispatches to the decoder matching the character type.
     *
     * \lang simp-chinese
     * @brief 按字符类型分派到对应的解码器。
     */
    template <typename CharType>
    encoding_error decode_one(const CharType *data, std::size_t size, char32_t &code_point, std::size_t &consumed) {
        constexpr encoding_form form = encoding_form_of_v<CharType>;
        if constexpr (form == encoding_form::utf8) {
            return decode_utf8(data, size, code_point, consumed);
        } else if constexpr (form == encoding_form::utf16) {
            return decode_utf16(data, size, code_point, consumed);
        } else {
            return decode_utf32(data, code_point, consumed);
        }
    }

    /**
     * \lang english
     * @brief Encodes one code point as UTF-8.
     * @param code_point The code point to encode
     * @param output The output string, appended to
     * @return false when the code point is a surrogate or outside the Unicode range
     *
     * \lang simp-chinese
     * @brief 把一个码点编码为 UTF-8。
     * @param code_point 待编码的码点
     * @param output 追加输出的字符串
     * @return 码点为代理项或超出 Unicode 范围时返回 false
     */
    inline bool encode_utf8(char32_t code_point, core::text::string &output) {
        if (code_point >= 0xD800 && code_point <= 0xDFFF) {
            return false;
        }
        if (code_point > 0x10FFFF) {
            return false;
        }
        char buffer[4];
        std::size_t count = 0;
        if (code_point < 0x80) {
            buffer[count++] = static_cast<char>(code_point);
        } else if (code_point < 0x800) {
            buffer[count++] = static_cast<char>(0xC0 | (code_point >> 6));
            buffer[count++] = static_cast<char>(0x80 | (code_point & 0x3F));
        } else if (code_point < 0x10000) {
            buffer[count++] = static_cast<char>(0xE0 | (code_point >> 12));
            buffer[count++] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
            buffer[count++] = static_cast<char>(0x80 | (code_point & 0x3F));
        } else {
            buffer[count++] = static_cast<char>(0xF0 | (code_point >> 18));
            buffer[count++] = static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
            buffer[count++] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
            buffer[count++] = static_cast<char>(0x80 | (code_point & 0x3F));
        }
        output.append(buffer, count);
        return true;
    }

    /**
     * \lang english
     * @brief Encodes one code point as UTF-16 code units.
     * @param code_point The code point to encode
     * @param output The output buffer, sized for two code units
     * @param count On output, the number of code units written
     * @return false when the code point is a surrogate or outside the Unicode range
     *
     * \lang simp-chinese
     * @brief 把一个码点编码为 UTF-16 码元。
     * @param code_point 待编码的码点
     * @param output 输出缓冲区，至少容纳两个码元
     * @param count 输出写入的码元数
     * @return 码点为代理项或超出 Unicode 范围时返回 false
     */
    inline bool encode_utf16(char32_t code_point, char16_t *output, std::size_t &count) {
        if (code_point >= 0xD800 && code_point <= 0xDFFF) {
            return false;
        }
        if (code_point > 0x10FFFF) {
            return false;
        }
        if (code_point < 0x10000) {
            output[0] = static_cast<char16_t>(code_point);
            count = 1;
        } else {
            const char32_t adjusted = code_point - 0x10000;
            output[0] = static_cast<char16_t>(0xD800 | (adjusted >> 10));
            output[1] = static_cast<char16_t>(0xDC00 | (adjusted & 0x3FF));
            count = 2;
        }
        return true;
    }

    template <typename FromChar, typename ToChar>
    void append_one(char32_t code_point, core::text::basic_string<ToChar> &output) {
        constexpr encoding_form form = encoding_form_of_v<ToChar>;
        if constexpr (form == encoding_form::utf8) {
            encode_utf8(code_point, output);
        } else if constexpr (form == encoding_form::utf16) {
            char16_t buffer[2];
            std::size_t count = 0;
            encode_utf16(code_point, buffer, count);
            output.append(buffer, count);
        } else {
            output.push_back(static_cast<ToChar>(code_point));
        }
    }
}

namespace rainy::foundation::text::unicode {
    /**
     * \lang english
     * @brief Reports the encoding form of a character sequence.
     * @return The encoding form of the sequence's character type
     *
     * \lang simp-chinese
     * @brief 报告字符序列的编码形式。
     * @return 序列字符类型的编码形式
     */
    template <typename Sequence, type_traits::other_trans::enable_if_t<implements::is_sequence_v<Sequence>, int> = 0>
    constexpr encoding_form encoding_form_of() {
        return encoding_form_of_v<implements::sequence_char_t<Sequence>>;
    }

    /**
     * \lang english
     * @brief Converts a sequence between the UTF encoding forms, reporting the first error.
     * @param input The sequence to convert
     * @param output The converted sequence, appended to; left partially filled on error
     * @param error On output, none on success, the first error otherwise
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 在各 UTF 编码形式之间转换序列，报告首个错误。
     * @param input 待转换的序列
     * @param output 追加输出的序列，出错时为部分填充
     * @param error 输出错误，成功时为 none
     * @return 成功返回 true
     */
    template <typename FromChar, typename ToChar>
    bool convert_encoding(core::text::basic_string_view<FromChar> input, core::text::basic_string<ToChar> &output,
                          encoding_error &error) {
        error = encoding_error::none;
        const FromChar *data = input.data();
        const std::size_t size = input.size();
        std::size_t offset = 0;
        while (offset < size) {
            char32_t code_point = 0;
            std::size_t consumed = 0;
            error = implements::decode_one(data + offset, size - offset, code_point, consumed);
            if (error != encoding_error::none) {
                return false;
            }
            implements::append_one<FromChar, ToChar>(code_point, output);
            offset += consumed;
        }
        return true;
    }

    /**
     * \lang english
     * @brief Converts a sequence between the UTF encoding forms, replacing invalid parts with U+FFFD.
     * @param input The sequence to convert
     * @param output The converted sequence, appended to
     * @return The error found, none on success; the corresponding position still becomes U+FFFD
     *
     * \lang simp-chinese
     * @brief 在各 UTF 编码形式之间转换序列，非法部分以 U+FFFD 替换。
     * @param input 待转换的序列
     * @param output 追加输出的序列
     * @return 发现的错误，成功时为 none；对应位置仍变为 U+FFFD
     */
    template <typename FromChar, typename ToChar>
    encoding_error convert_encoding_replace(core::text::basic_string_view<FromChar> input, core::text::basic_string<ToChar> &output) {
        encoding_error error = encoding_error::none;
        const FromChar *data = input.data();
        const std::size_t size = input.size();
        std::size_t offset = 0;
        while (offset < size) {
            char32_t code_point = 0;
            std::size_t consumed = 0;
            const encoding_error step = implements::decode_one(data + offset, size - offset, code_point, consumed);
            if (step == encoding_error::none) {
                implements::append_one<FromChar, ToChar>(code_point, output);
                offset += consumed;
            } else {
                if (error == encoding_error::none) {
                    error = step;
                }
                implements::append_one<FromChar, ToChar>(replacement_code_point, output);
                if (step == encoding_error::invalid_continuation_element) {
                    std::size_t lookahead = offset + 1;
                    while (lookahead < size) {
                        char32_t probe = 0;
                        std::size_t probe_consumed = 0;
                        if (implements::decode_one(data + lookahead, size - lookahead, probe, probe_consumed) == encoding_error::none) {
                            break;
                        }
                        ++lookahead;
                    }
                    offset = lookahead;
                } else {
                    offset += step == encoding_error::truncated_sequence ? size - offset : 1;
                }
            }
        }
        return error;
    }

    /**
     * \lang english
     * @brief Converts a sequence to UTF-8.
     * @param input The sequence to convert
     * @param output The converted UTF-8 string, appended to
     * @param error On output, none on success, the first error otherwise
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把序列转换为 UTF-8。
     * @param input 待转换的序列
     * @param output 追加输出的 UTF-8 字符串
     * @param error 输出错误，成功时为 none
     * @return 成功返回 true
     */
    template <typename FromChar>
    bool to_utf8(core::text::basic_string_view<FromChar> input, core::text::string &output, encoding_error &error) {
        return convert_encoding(input, output, error);
    }

    /**
     * \lang english
     * @brief Converts a sequence to UTF-16.
     * @param input The sequence to convert
     * @param output The converted UTF-16 string, appended to
     * @param error On output, none on success, the first error otherwise
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把序列转换为 UTF-16。
     * @param input 待转换的序列
     * @param output 追加输出的 UTF-16 字符串
     * @param error 输出错误，成功时为 none
     * @return 成功返回 true
     */
    template <typename FromChar>
    bool to_utf16(core::text::basic_string_view<FromChar> input, core::text::u16string &output, encoding_error &error) {
        return convert_encoding(input, output, error);
    }

    /**
     * \lang english
     * @brief Converts a sequence to UTF-32.
     * @param input The sequence to convert
     * @param output The converted UTF-32 string, appended to
     * @param error On output, none on success, the first error otherwise
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把序列转换为 UTF-32。
     * @param input 待转换的序列
     * @param output 追加输出的 UTF-32 字符串
     * @param error 输出错误，成功时为 none
     * @return 成功返回 true
     */
    template <typename FromChar>
    bool to_utf32(core::text::basic_string_view<FromChar> input, core::text::u32string &output, encoding_error &error) {
        return convert_encoding(input, output, error);
    }

    /**
     * \lang english
     * @brief Reports whether a UTF-8 sequence is well formed.
     * @param input The sequence to inspect
     * @return true when no surrogate, overlong, out of range or malformed sequence appears
     *
     * \lang simp-chinese
     * @brief 报告 UTF-8 序列是否良构。
     * @param input 待检查的序列
     * @return 不含代理项、过长、越界或畸形序列时返回 true
     */
    inline bool validate_utf8(core::text::string_view input) {
        encoding_error ignored = encoding_error::none;
        core::text::string sink;
        return convert_encoding(input, sink, ignored);
    }

    /**
     * \lang english
     * @brief Reports whether a UTF-16 sequence is well formed.
     * @param input The sequence to inspect
     * @return true when no unpaired surrogate or out of range code unit appears
     *
     * \lang simp-chinese
     * @brief 报告 UTF-16 序列是否良构。
     * @param input 待检查的序列
     * @return 不含未配对代理项或越界码元时返回 true
     */
    inline bool validate_utf16(core::text::u16string_view input) {
        encoding_error ignored = encoding_error::none;
        core::text::u16string sink;
        return convert_encoding(input, sink, ignored);
    }

    /**
     * \lang english
     * @brief Reports whether a UTF-32 sequence is well formed.
     * @param input The sequence to inspect
     * @return true when no surrogate or out of range code point appears
     *
     * \lang simp-chinese
     * @brief 报告 UTF-32 序列是否良构。
     * @param input 待检查的序列
     * @return 不含代理项或越界码点时返回 true
     */
    inline bool validate_utf32(core::text::u32string_view input) {
        encoding_error ignored = encoding_error::none;
        core::text::u32string sink;
        return convert_encoding(input, sink, ignored);
    }

    /**
     * \lang english
     * @brief Counts the code points of a sequence.
     * @param input The sequence to count
     * @return The number of code points; a malformed sequence still contributes one count per consumed step
     *
     * \lang simp-chinese
     * @brief 统计序列的码点数。
     * @param input 待统计的序列
     * @return 码点数量；畸形序列的每个消费步仍计一次
     */
    inline std::size_t codepoint_length(core::text::string_view input) {
        std::size_t count = 0;
        const char *data = input.data();
        const std::size_t size = input.size();
        std::size_t offset = 0;
        while (offset < size) {
            char32_t code_point = 0;
            std::size_t consumed = 0;
            const encoding_error step = implements::decode_one(data + offset, size - offset, code_point, consumed);
            if (step == encoding_error::none) {
                offset += consumed;
            } else {
                offset += step == encoding_error::invalid_continuation_element ? 1 : 1;
            }
            ++count;
        }
        return count;
    }

    /**
     * \lang english
     * @brief An input iterator over the code points of a UTF sequence.
     *
     * Malformed positions yield U+FFFD and consume one element, mirroring the
     * replace policy of the permissive converters.
     *
     * @tparam CharType The character type of the underlying sequence
     *
     * \lang simp-chinese
     * @brief 遍历 UTF 序列码点的输入迭代器。
     *
     * 畸形位置产出 U+FFFD 并消费一个元素，与宽容转换器的替换策略一致。
     *
     * @tparam CharType 底层序列的字符类型
     */
    template <typename CharType>
    class codepoint_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = char32_t;
        using difference_type = std::ptrdiff_t;
        using pointer = const char32_t *;
        using reference = const char32_t &;

        /**
         * \lang english
         * @brief Constructs an end iterator.
         *
         * \lang simp-chinese
         * @brief 构造末端迭代器。
         */
        codepoint_iterator() : current(nullptr), remaining(0), cached(replacement_code_point) {
        }

        /**
         * \lang english
         * @brief Constructs an iterator over a sequence.
         * @param data Pointer to the first character
         * @param size The number of characters
         *
         * \lang simp-chinese
         * @brief 构造遍历序列的迭代器。
         * @param data 指向首字符的指针
         * @param size 字符数量
         */
        codepoint_iterator(const CharType *data, std::size_t size) :
            current(size ? data : nullptr), remaining(size), cached(replacement_code_point) {
            if (remaining) {
                load();
            }
        }

        reference operator*() const {
            return cached;
        }

        pointer operator->() const {
            return &cached;
        }

        codepoint_iterator &operator++() {
            if (remaining) {
                load();
            } else {
                current = nullptr;
                cached = replacement_code_point;
            }
            return *this;
        }

        codepoint_iterator operator++(int) {
            codepoint_iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * \lang english
         * @brief Reports whether the code point last loaded came from a malformed sequence.
         * @return true when the current position was invalid and replaced
         *
         * \lang simp-chinese
         * @brief 报告最近加载的码点是否来自畸形序列。
         * @return 当前位置非法且被替换时返回 true
         */
        bool errored() const {
            return last_error != encoding_error::none;
        }

        /**
         * \lang english
         * @brief Returns the error found while loading the current code point.
         * @return The error, none when the current position was well formed
         *
         * \lang simp-chinese
         * @brief 返回加载当前码点时发现的错误。
         * @return 错误，当前位置良构时为 none
         */
        encoding_error error() const {
            return last_error;
        }

        friend bool operator==(const codepoint_iterator &left, const codepoint_iterator &right) {
            return left.current == right.current && left.remaining == right.remaining;
        }

        friend bool operator!=(const codepoint_iterator &left, const codepoint_iterator &right) {
            return !(left == right);
        }

    private:
        void load() {
            char32_t code_point = 0;
            std::size_t consumed = 0;
            last_error = implements::decode_one(current, remaining, code_point, consumed);
            if (last_error != encoding_error::none) {
                code_point = replacement_code_point;
                consumed = 1;
            }
            cached = code_point;
            current += consumed;
            remaining -= consumed;
        }

        const CharType *current;
        std::size_t remaining;
        char32_t cached;
        encoding_error last_error;
    };

    /**
     * \lang english
     * @brief Builds a code point iterator for a string like sequence.
     * @param sequence The sequence to traverse
     * @return An iterator positioned at the first code point
     *
     * \lang simp-chinese
     * @brief 为类字符串序列构建码点迭代器。
     * @param sequence 待遍历的序列
     * @return 定位于首个码点的迭代器
     */
    template <typename Sequence, type_traits::other_trans::enable_if_t<implements::is_sequence_v<Sequence>, int> = 0>
    codepoint_iterator<implements::sequence_char_t<Sequence>> codepoint_begin(const Sequence &sequence) {
        using char_type = implements::sequence_char_t<Sequence>;
        return codepoint_iterator<char_type>(sequence.data(), sequence.size());
    }

    /**
     * \lang english
     * @brief Builds the end code point iterator for a string like sequence.
     * @return The end iterator
     *
     * \lang simp-chinese
     * @brief 构建类字符串序列的末端码点迭代器。
     * @return 末端迭代器
     */
    template <typename CharType>
    codepoint_iterator<CharType> codepoint_end() {
        return codepoint_iterator<CharType>();
    }
}

namespace rainy::text {
    using foundation::text::unicode::codepoint_begin;
    using foundation::text::unicode::codepoint_end;
    using foundation::text::unicode::codepoint_iterator;
    using foundation::text::unicode::codepoint_length;
    using foundation::text::unicode::convert_encoding;
    using foundation::text::unicode::convert_encoding_replace;
    using foundation::text::unicode::encoding_form;
    using foundation::text::unicode::encoding_form_of;
    using foundation::text::unicode::encoding_form_of_v;
    using foundation::text::unicode::replacement_code_point;
    using foundation::text::unicode::to_utf16;
    using foundation::text::unicode::to_utf32;
    using foundation::text::unicode::to_utf8;
    using foundation::text::unicode::validate_utf16;
    using foundation::text::unicode::validate_utf32;
    using foundation::text::unicode::validate_utf8;
}

#endif
