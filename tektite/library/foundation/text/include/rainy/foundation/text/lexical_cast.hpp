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
#ifndef RAINY_FOUNDATION_TEXT_LEXICAL_CAST_HPP
#define RAINY_FOUNDATION_TEXT_LEXICAL_CAST_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/text/implements/lexical_cast/bad_lexical_cast.hpp>
#include <rainy/foundation/text/implements/lexical_cast/dispatch.hpp>

namespace rainy::foundation::text {
    /**
     * \lang english
     * @brief Converts a text representation into a value, reporting the failure instead of throwing.
     *
     * The state of result is undefined when the function returns false.
     *
     * @tparam Target The target type
     * @tparam Source The source type
     * @param arg The text representation or the value to convert
     * @param result The output value, written on success
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把文本表示转换为值，失败时只报告而不抛出异常。
     *
     * 函数返回 false 时 result 的状态未定义。
     *
     * @tparam Target 目标类型
     * @tparam Source 源类型
     * @param arg 待转换的文本表示或值
     * @param result 输出值，成功时写入
     * @return 成功返回 true
     */
    template <typename Target, typename Source>
    bool try_lexical_convert(const Source &arg, Target &result) {
        return implements::try_convert<Target>(arg, result);
    }

    /**
     * \lang english
     * @brief Converts a counted character range into a value, reporting the failure instead of throwing.
     *
     * The range needs no null terminator, so a substring is accepted as well.
     *
     * @tparam Target The target type
     * @tparam CharType The character type
     * @param chars Pointer to the first character of the range
     * @param count The number of characters in the range
     * @param result The output value, written on success
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 把带长度的字符区间转换为值，失败时只报告而不抛出异常。
     *
     * 该区间无需以空字符结尾，因此也可以传入子串。
     *
     * @tparam Target 目标类型
     * @tparam CharType 字符类型
     * @param chars 指向区间首字符的指针
     * @param count 区间内的字符数量
     * @param result 输出值，成功时写入
     * @return 成功返回 true
     */
    template <typename Target, typename CharType>
    bool try_lexical_convert(const CharType *chars, std::size_t count, Target &result) {
        return try_lexical_convert(core::text::basic_string_view<CharType>{chars, count}, result);
    }
    /**
     * \lang english
     * @brief Converts a text representation into a value, throwing when the text does not fit the target.
     *
     * @tparam Target The target type
     * @tparam Source The source type
     * @param arg The text representation or the value to convert
     * @return The converted value
     * @throws bad_lexical_cast when the conversion fails
     *
     * \lang simp-chinese
     * @brief 把文本表示转换为值，文本不符合目标时抛出异常。
     *
     * @tparam Target 目标类型
     * @tparam Source 源类型
     * @param arg 待转换的文本表示或值
     * @return 转换得到的值
     * @throws 转换失败时抛出 bad_lexical_cast
     */
    template <typename Target, typename Source>
    RAINY_NODISCARD Target lexical_cast(const Source &arg) {
        Target result{};
        if (!try_lexical_convert(arg, result)) {
            exceptions::logic::throw_bad_lexical_cast("lexical_cast", "the source value cannot be interpreted as the target type");
        }
        return result;
    }
    /**
     * \lang english
     * @brief Converts a counted character range into a value, throwing when the range does not fit the target.
     *
     * The range needs no null terminator, so a substring is accepted as well.
     *
     * @tparam Target The target type
     * @tparam CharType The character type
     * @param chars Pointer to the first character of the range
     * @param count The number of characters in the range
     * @return The converted value
     * @throws bad_lexical_cast when the conversion fails
     *
     * \lang simp-chinese
     * @brief 把带长度的字符区间转换为值，区间不符合目标时抛出异常。
     *
     * 该区间无需以空字符结尾，因此也可以传入子串。
     *
     * @tparam Target 目标类型
     * @tparam CharType 字符类型
     * @param chars 指向区间首字符的指针
     * @param count 区间内的字符数量
     * @return 转换得到的值
     * @throws 转换失败时抛出 bad_lexical_cast
     */
    template <typename Target, typename CharType>
    RAINY_NODISCARD Target lexical_cast(const CharType *chars, std::size_t count) {
        return lexical_cast<Target>(core::text::basic_string_view<CharType>{chars, count});
    }
    /**
     * \lang english
     * @brief Formats a value as text.
     *
     * @tparam Ty The type of the value
     * @tparam Char The character type of the produced text
     * @param value The value to format
     * @return The text representation of value
     * @throws bad_lexical_cast when no text representation exists
     *
     * \lang simp-chinese
     * @brief 把值格式化为文本。
     *
     * @tparam Ty 值的类型
     * @tparam Char 所生成文本的字符类型
     * @param value 待格式化的值
     * @return value 的文本表示
     * @throws 不存在文本表示时抛出 bad_lexical_cast
     */
    template <typename Ty, typename Char = char>
    RAINY_NODISCARD core::text::basic_string<Char> lexical_to_string(const Ty &value) {
        return lexical_cast<core::text::basic_string<Char>>(value);
    }
}

namespace rainy::text {
    using foundation::text::lexical_cast;
    using foundation::text::lexical_to_string;
    using foundation::text::try_lexical_convert;
}

#endif
