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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_IMPLEMENTS_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_IMPLEMENTS_HPP
#include <rainy/core/implements/exceptions.hpp>
#include <rainy/core/implements/text/string_view.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::exceptions::runtime {
    class format_error : public runtime_error {
    public:
        explicit format_error(const std::string &message, const source &location = source::current()) :
            runtime_error(message, location) {
        }
    };

    RAINY_INLINE rain_fn throw_format_error(const char *message,
                                            const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(format_error{message, location});
    }
}

namespace rainy::foundation::text::implements {
    enum class align_type {
        none,
        left, // '<'
        right, // '>'
        center // '^'
    };

    enum class sign_type {
        none, // 默认
        plus, // '+'
        minus, // '-'
        space // ' '
    };

    template <typename CharTypeype>
    struct format_specs {
        align_type align = align_type::none;
        CharTypeype fill = static_cast<CharTypeype>(' ');
        int width = 0;
        int precision = -1;
        bool use_locale = false;
        char type = '\0';

        // 动态宽度和精度
        bool dynamic_width = false;
        bool dynamic_precision = false;
        std::size_t width_arg_id = 0;
        std::size_t precision_arg_id = 0;
    };

    // 解析整数（用于宽度、精度、参数ID）
    template <typename CharType>
    constexpr std::size_t parse_nonnegative_int(typename basic_string_view<CharType>::const_iterator &it,
                                                typename basic_string_view<CharType>::const_iterator end, int max_value = INT_MAX) {

        std::size_t value = 0;
        while (it != end && *it >= '0' && *it <= '9') {
            if (value > static_cast<std::size_t>(max_value) / 10) {
                exceptions::runtime::throw_format_error("number is too big");
            }
            value = value * 10 + (*it - '0');
            ++it;
        }
        return value;
    }

    // 检查是否为对齐符号
    constexpr bool is_align_char(char c) noexcept {
        return c == '<' || c == '>' || c == '^';
    }

    template <typename CharType>
    struct stack_buffer {
        void push(CharType c) noexcept {
            data[len++] = c;
        }
        void push(const CharType *s, int n) noexcept {
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                std::memcpy(data + len, s, static_cast<std::size_t>(n));
            } else {
                std::wmemcpy(data + len, s, static_cast<std::size_t>(n));
            }
            len += n;
        }

        void push_uint_reverse(uint64_t v, int digits) noexcept {
            int start = len;
            len += digits;
            int pos = len - 1;
            while (v >= 100) {
                // 两位一组，使用 Grisu 风格的 2-digit 表
                int r = static_cast<int>(v % 100);
                v /= 100;
                data[pos--] = CharType('0') + r % 10;
                data[pos--] = CharType('0') + r / 10;
            }
            if (v >= 10) {
                data[pos--] = CharType('0') + static_cast<int>(v % 10);
                data[pos] = CharType('0') + static_cast<int>(v / 10);
            } else {
                data[pos] = CharType('0') + static_cast<int>(v);
            }
            (void) start;
        }

        CharType data[64];
        int32_t len = 0;
    };
}

#endif
