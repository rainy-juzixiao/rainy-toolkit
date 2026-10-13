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
#include <rainy/foundation/text/lexical_cast/traits.hpp>

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
