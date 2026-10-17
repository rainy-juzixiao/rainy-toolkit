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
#ifndef RAINY_FOUNDATION_TEXT_CASTER_HPP
#define RAINY_FOUNDATION_TEXT_CASTER_HPP
#include <rainy/core/platform.hpp>

namespace rainy::foundation::text {
    /**
     * \lang english
     * @brief The extension point of lexical_cast for user defined target types.
     *
     * Specialize this template to teach lexical_cast how to read and how to write a type that the
     * library does not know about. A specialization must set `value` to true and provide the two
     * static members below:
     *
     * @code
     * template <>
     * struct lexical_caster<point, char> {
     *     static constexpr bool value = true;
     *     static bool convert(const core::text::basic_string_view<char> &input, point &result);
     *     static core::text::basic_string<char> to_string(const point &value);
     * };
     * @endcode
     *
     * @tparam Target The user defined type being converted
     * @tparam CharT The character type of the text representation
     *
     * \lang simp-chinese
     * @brief lexical_cast 面向用户自定义目标类型的扩展点。
     *
     * 特化该模板即可告诉 lexical_cast 如何读写库不认识的类型。特化必须把 `value` 置为 true，
     * 并提供下面两个静态成员：
     *
     * @code
     * template <>
     * struct lexical_caster<point, char> {
     *     static constexpr bool value = true;
     *     static bool convert(const core::text::basic_string_view<char> &input, point &result);
     *     static core::text::basic_string<char> to_string(const point &value);
     * };
     * @endcode
     *
     * @tparam Target 被转换的用户自定义类型
     * @tparam CharT 文本表示的字符类型
     */
    template <typename Target, typename CharT, typename = void>
    struct lexical_caster {
        static constexpr bool value = false;
    };

    /**
     * \lang english
     * @brief Indicates whether a lexical_caster specialization is available for the given pair of types.
     *
     * @tparam Target The target type
     * @tparam CharT The character type of the text representation
     *
     * \lang simp-chinese
     * @brief 指示给定类型对是否存在 lexical_caster 特化。
     *
     * @tparam Target 目标类型
     * @tparam CharT 文本表示的字符类型
     */
    template <typename Target, typename CharT>
    RAINY_CONSTEXPR_BOOL has_lexical_caster_v = lexical_caster<Target, CharT>::value;
}

#endif
