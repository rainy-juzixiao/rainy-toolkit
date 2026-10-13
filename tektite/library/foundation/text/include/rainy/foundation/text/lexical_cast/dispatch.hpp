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
#ifndef RAINY_FOUNDATION_TEXT_DISPATCH_HPP
#define RAINY_FOUNDATION_TEXT_DISPATCH_HPP
#include <rainy/foundation/text/lexical_cast/caster.hpp>
#include <rainy/foundation/text/lexical_cast/converter.hpp>
#include <rainy/foundation/text/lexical_cast/traits.hpp>

namespace rainy::foundation::text::implements {
    /**
     * \lang english
     * @brief Selects the converter that serves a pair of target and source types.
     *
     * The primary template covers the pairs that have no conversion path at all and diagnoses them at
     * compile time.
     *
     * @tparam Target The decayed target type
     * @tparam Source The decayed source type
     *
     * \lang simp-chinese
     * @brief 为目标类型与源类型的组合挑选转换器。
     *
     * 主模板对应完全没有转换路径的组合，并在编译期给出诊断。
     *
     * @tparam Target 退化后的目标类型
     * @tparam Source 退化后的源类型
     */
    template <typename Target, typename Source, typename = void>
    struct caster_for {
        static_assert(type_traits::implements::always_false<Source>,
                      "lexical_cast has no conversion path for these types, specialize lexical_caster to add one");

        static bool convert(const Source &, Target &) noexcept {
            return false;
        }
    };

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_number_source_v<Source> && is_number_v<Target>, void>>
        : charconv_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_number_source_v<Source> && is_bool_v<Target>, void>>
        : boolean_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_number_source_v<Source> && is_character_v<Target>, void>>
        : character_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_number_source_v<Source> && is_string_like_v<Target>, void>>
        : string_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_char_sequence_v<Source> && is_number_v<Target>, void>>
        : charconv_parser<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_char_sequence_v<Source> && is_bool_v<Target>, void>>
        : boolean_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_char_sequence_v<Source> && is_character_v<Target>, void>>
        : character_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source, type_traits::other_trans::enable_if_t<is_char_sequence_v<Source> && is_string_like_v<Target>, void>>
        : string_converter<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<Target, Source,
                  type_traits::other_trans::enable_if_t<
                      is_char_sequence_v<Source> && has_lexical_caster_v<Target, char_sequence_char_t<Source>>, void>>
        : custom_caster<Target, Source> {};

    template <typename Target, typename Source>
    struct caster_for<
        Target, Source,
        type_traits::other_trans::enable_if_t<!is_number_source_v<Source> && !is_char_sequence_v<Source> && is_string_like_v<Target> &&
                                                  has_lexical_caster_v<Source, typename string_traits<Target>::char_type>,
                                              void>> : custom_string_converter<Target, Source> {};

    /**
     * \lang english
     * @brief Runs the conversion that is selected for the given pair of types.
     * @param source The value to convert
     * @param result The output value, written on success
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 执行为给定类型组合挑选的转换。
     * @param source 待转换的值
     * @param result 输出值，成功时写入
     * @return 成功返回 true
     */
    template <typename Target, typename Source>
    bool try_convert(const Source &source, Target &result) {
        using target_type = type_traits::modifers::remove_cv_t<Target>;
        using source_type = source_type_t<Source>;
        return caster_for<target_type, source_type>::convert(source, result);
    }
}

#endif
