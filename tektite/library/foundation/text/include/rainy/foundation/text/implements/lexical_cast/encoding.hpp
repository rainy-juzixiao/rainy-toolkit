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
#ifndef RAINY_FOUNDATION_TEXT_ENCODING_HPP
#define RAINY_FOUNDATION_TEXT_ENCODING_HPP
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/char_traits.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/text/wstring_convert.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/text/implements/lexical_cast/traits.hpp>

namespace rainy::foundation::text::implements {
    template <typename CharType>
    using utf8_converter =
        core::text::wstring_convert<core::text::codecvt_utf8<CharType>, core::text::basic_string, CharType, core::text::char_traits,
                                    core::memory::allocator<CharType>, core::memory::allocator<char>>;

    /**
     * \lang english
     * @brief Copies a range of single byte characters into a string of single byte characters.
     * @tparam Target The string type
     * @tparam FromChar The character type of the range
     * @param target The output string
     * @param data Pointer to the first character of the range
     * @param size The number of characters in the range
     *
     * \lang simp-chinese
     * @brief 把单字节字符区间拷贝进单字节字符的字符串。
     * @tparam Target 字符串类型
     * @tparam FromChar 区间的字符类型
     * @param target 输出字符串
     * @param data 指向区间首字符的指针
     * @param size 区间内的字符数量
     */
    template <typename Target, typename FromChar>
    void assign_bytes(Target &target, const FromChar *data, std::size_t size) {
        target.assign(reinterpret_cast<const typename Target::value_type *>(data), size);
    }

    /**
     * \lang english
     * @brief Indicates whether a range of code units is a well formed UTF-8 sequence.
     * @param data Pointer to the first byte
     * @param size The number of bytes
     * @return true when no overlong, surrogate, out of range or truncated sequence appears
     *
     * \lang simp-chinese
     * @brief 判断码元区间是否为良构的 UTF-8 序列。
     * @param data 指向首字节的指针
     * @param size 字节数
     * @return 不含过长、代理项、越界或截断序列时返回 true
     */
    inline bool is_valid_utf8_sequence(const char *data, std::size_t size) noexcept {
        std::size_t index = 0;
        while (index < size) {
            const unsigned char lead = static_cast<unsigned char>(data[index]);
            if (lead < 0x80) {
                ++index;
                continue;
            }
            std::size_t needed = 0;
            char32_t value = 0;
            if (lead >= 0xC2 && lead <= 0xDF) {
                value = lead & 0x1F;
                needed = 1;
            } else if (lead >= 0xE0 && lead <= 0xEF) {
                value = lead & 0x0F;
                needed = 2;
            } else if (lead >= 0xF0 && lead <= 0xF4) {
                value = lead & 0x07;
                needed = 3;
            } else {
                return false;
            }
            if (size - index < needed + 1) {
                return false;
            }
            for (std::size_t offset = 1; offset <= needed; ++offset) {
                const unsigned char unit = static_cast<unsigned char>(data[index + offset]);
                if ((unit & 0xC0) != 0x80) {
                    return false;
                }
                value = (value << 6) | (unit & 0x3F);
            }
            if ((needed == 2 && value < 0x800) || (needed == 3 && value < 0x10000)) {
                return false;
            }
            if ((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF) {
                return false;
            }
            index += needed + 1;
        }
        return true;
    }

    /**
     * \lang english
     * @brief Indicates whether a range of wide code units holds only valid code points.
     * @tparam CharType The code unit type
     * @param data Pointer to the first code unit
     * @param size The number of code units
     * @return true when no lone surrogate or out of range code point appears
     *
     * \lang simp-chinese
     * @brief 判断宽码元区间是否只含有有效码点。
     * @tparam CharType 码元类型
     * @param data 指向首码元的指针
     * @param size 码元数
     * @return 不含孤立代理项或越界码点时返回 true
     */
    template <typename CharType>
    bool is_valid_wide_sequence(const CharType *data, std::size_t size) noexcept {
        if constexpr (sizeof(CharType) == 2) {
            for (std::size_t index = 0; index < size; ++index) {
                const char32_t unit = static_cast<char32_t>(static_cast<unsigned short>(data[index]));
                if (unit >= 0xD800 && unit <= 0xDBFF) {
                    if (index + 1 >= size) {
                        return false;
                    }
                    const char32_t trail = static_cast<char32_t>(static_cast<unsigned short>(data[index + 1]));
                    if (trail < 0xDC00 || trail > 0xDFFF) {
                        return false;
                    }
                    ++index;
                } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
                    return false;
                }
            }
        } else {
            for (std::size_t index = 0; index < size; ++index) {
                const char32_t unit = static_cast<char32_t>(data[index]);
                if (unit > 0x10FFFF || (unit >= 0xD800 && unit <= 0xDFFF)) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * \lang english
     * @brief Converts a character sequence to its UTF-8 representation.
     * @param input The sequence to convert
     * @param output The converted sequence, left untouched on failure
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把字符序列转换为 UTF-8 表示。
     * @param input 待转换的序列
     * @param output 转换结果，失败时保持不变
     * @return 成功返回 true
     */
    template <typename CharType>
    bool to_utf8(core::text::basic_string_view<CharType> input, core::text::string &output) {
        if constexpr (is_byte_char_v<CharType>) {
            assign_bytes(output, input.data(), input.size());
            return true;
        } else {
            if (!is_valid_wide_sequence(input.data(), input.size())) {
                return false;
            }
            utf8_converter<CharType> converter;
            core::text::string converted = converter.to_bytes(input.data(), input.data() + input.size());
            if (!input.empty() && converted.empty()) {
                return false;
            }
            output = utility::move(converted);
            return true;
        }
    }

    /**
     * \lang english
     * @brief Converts a UTF-8 sequence to the given character type.
     * @param input The UTF-8 sequence to convert
     * @param output The converted sequence, left untouched on failure
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把 UTF-8 序列转换为给定字符类型。
     * @param input 待转换的 UTF-8 序列
     * @param output 转换结果，失败时保持不变
     * @return 成功返回 true
     */
    template <typename CharType>
    bool from_utf8(const core::text::string &input, core::text::basic_string<CharType> &output) {
        if constexpr (is_byte_char_v<CharType>) {
            assign_bytes(output, input.data(), input.size());
            return true;
        } else {
            if (!is_valid_utf8_sequence(input.data(), input.size())) {
                return false;
            }
            utf8_converter<CharType> converter;
            core::text::basic_string<CharType> converted = converter.from_bytes(input.data(), input.data() + input.size());
            if (!input.empty() && converted.empty()) {
                return false;
            }
            output = utility::move(converted);
            return true;
        }
    }

    /**
     * \lang english
     * @brief Converts a character sequence to another character type through UTF-8.
     * @param input The sequence to convert
     * @param output The converted sequence
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 经 UTF-8 把字符序列转换为另一种字符类型。
     * @param input 待转换的序列
     * @param output 转换结果
     * @return 成功返回 true
     */
    template <typename FromChar, typename ToChar>
    bool convert_encoding(core::text::basic_string_view<FromChar> input, core::text::basic_string<ToChar> &output) {
        if constexpr (type_traits::type_relations::is_same_v<FromChar, ToChar>) {
            output.assign(input.data(), input.size());
            return true;
        } else {
            core::text::string utf8;
            if (!to_utf8(input, utf8)) {
                return false;
            }
            return from_utf8(utf8, output);
        }
    }

    /**
     * \lang english
     * @brief Writes a character range into a string like target.
     *
     * A view target receives a view over the range, an owning target receives a copy of it.
     *
     * @tparam Target The string like target type
     * @tparam CharType The character type of the range
     * @param result The output string
     * @param data Pointer to the first character of the range
     * @param size The number of characters in the range
     *
     * \lang simp-chinese
     * @brief 把字符区间写入类字符串目标。
     *
     * 视图目标得到指向该区间的视图，拥有型目标得到该区间的拷贝。
     *
     * @tparam Target 类字符串目标类型
     * @tparam CharType 区间的字符类型
     * @param result 输出字符串
     * @param data 指向区间首字符的指针
     * @param size 区间内的字符数量
     */
    template <typename Target, typename CharType>
    void assign_string(Target &result, const CharType *data, std::size_t size) {
        if constexpr (string_traits<Target>::is_view) {
            result = Target{data, size};
        } else {
            result.assign(data, size);
        }
    }
}

#endif
