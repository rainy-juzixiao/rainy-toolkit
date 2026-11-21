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
#ifndef RAINY_FOUNDATION_TEXT_SOURCE_HPP
#define RAINY_FOUNDATION_TEXT_SOURCE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/char_traits.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/text/implements/lexical_cast/encoding.hpp>
#include <rainy/foundation/text/implements/lexical_cast/traits.hpp>

namespace rainy::foundation::text::implements {
    /**
     * \lang english
     * @brief Builds a character sequence view over a string like, character, C string or character array source.
     * @param source The source to view
     * @return The view over the characters of source
     *
     * \lang simp-chinese
     * @brief 为类字符串、字符、C 字符串或字符数组源构造字符序列视图。
     * @param source 待观察的源
     * @return 指向 source 字符的视图
     */
    template <typename Source>
    rain_fn source_char_view(const Source &source) -> core::text::basic_string_view<char_sequence_char_t<Source>> {
        if constexpr (string_traits<Source>::value) {
            return {source.data(), source.size()};
        } else if constexpr (is_character_v<Source>) {
            return {&source, 1};
        } else {
            return {source, core::text::char_traits<char_sequence_char_t<Source>>::length(source)};
        }
    }

    /**
     * \lang english
     * @brief The text representation of a source value, backed by a caller visible buffer.
     *
     * The character sequence sources are viewed in place, the arithmetic sources are formatted into an
     * internal buffer. An arithmetic source whose text does not fit the buffer is reported as a failure.
     *
     * @tparam Source The decayed source type
     *
     * \lang simp-chinese
     * @brief 源值的文本表示，其缓冲区对调用方可见。
     *
     * 字符序列源就地观察，算术源格式化到内部缓冲区。文本放不进缓冲区的算术源会被报告为失败。
     *
     * @tparam Source 退化后的源类型
     */
    template <typename Source>
    class text_view {
    public:
        using char_type = typename text_char_type<Source>::type;

        /**
         * \lang english
         * @brief Builds the text representation of source.
         * @param source The source value
         *
         * \lang simp-chinese
         * @brief 构造 source 的文本表示。
         * @param source 源值
         */
        explicit text_view(const Source &source) {
            if constexpr (is_char_sequence_v<Source>) {
                view_ = source_char_view(source);
            } else if constexpr (is_bool_v<Source>) {
                view_ = source ? core::text::string_view{"1", 1} : core::text::string_view{"0", 1};
            } else {
                auto result = core::text::to_chars(buffer_, buffer_ + sizeof(buffer_), source);
                if (result.ec == std::errc{}) {
                    view_ = core::text::string_view{buffer_, static_cast<std::size_t>(result.ptr - buffer_)};
                } else {
                    valid_ = false;
                }
            }
        }

        /**
         * \lang english
         * @brief Indicates whether a text representation is available.
         *
         * \lang simp-chinese
         * @brief 指示是否存在可用的文本表示。
         */
        explicit operator bool() const noexcept {
            return valid_;
        }

        /**
         * \lang english
         * @brief Returns the text representation.
         *
         * \lang simp-chinese
         * @brief 返回文本表示。
         */
        RAINY_NODISCARD core::text::basic_string_view<char_type> view() const noexcept {
            return view_;
        }

    private:
        char buffer_[128]{};
        core::text::basic_string_view<char_type> view_{};
        bool valid_{true};
    };
}

#endif
