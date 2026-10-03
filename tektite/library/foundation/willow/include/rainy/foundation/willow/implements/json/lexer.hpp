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
#ifndef RAINY_FOUNDATION_WILLOW_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_LEXER_HPP
#include <iomanip>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/simd.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/json/config.hpp>
#include <rainy/foundation/willow/implements/json/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::json::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct json_lexer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using simd_char_type = core::vec<char_type>;

        json_lexer(Adapter adapter, const char_type *span_data = nullptr, std::size_t span_size = 0) :
            adapter_(adapter), span_data_(span_data), span_size_(span_data != nullptr ? span_size : 0) {
            string_buffer_.reserve(96);
            number_buffer_.reserve(64);
            read_next();
        }

        char_int_type read_next() {
            if (span_data_ != nullptr) {
                current = span_pos_ < span_size_ ? span_data_[span_pos_++] : char_traits::eof();
                return current;
            }
            current = adapter_.get_char();
            return current;
        }

        void skip_spaces() {
            if (span_data_ != nullptr) {
                if (!is_space_char(current)) {
                    return;
                }
                skip_spaces_span();
                return;
            }
            while (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
                read_next();
            }
        }

        token_type scan() {
            skip_spaces();
            token_type result;
            switch (current) {
                case '[':
                    result = token_type::begin_array;
                    break;
                case ']':
                    result = token_type::end_array;
                    break;
                case '{':
                    result = token_type::begin_object;
                    break;
                case '}':
                    result = token_type::end_object;
                    break;
                case ':':
                    result = token_type::name_separator;
                    break;
                case ',':
                    result = token_type::value_separator;
                    break;
                case 't':
                    return scan_literal<'t', 'r', 'u', 'e'>(token_type::literal_true);
                case 'f':
                    return scan_literal<'f', 'a', 'l', 's', 'e'>(token_type::literal_false);
                case 'n':

                    return scan_literal<'n', 'u', 'l', 'l'>(token_type::literal_null);
                case '\"':
                    return scan_string();
                case '-':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    return scan_number();
                case '\0':
                case char_traits::eof():
                    return token_type::end_of_input;
                default:
                    exceptions::willow::json::throw_json_parse_error("unexpected character");
            }
            read_next();

            return result;
        }

        template <char_type... Ch>
        struct literal_checker {
            static void check(json_lexer &) {
            }
        };

        template <char_type This, char_type... Rest>
        struct literal_checker<This, Rest...> {
            static void check(json_lexer &s) {
                if (char_traits::to_char_type(s.current) != This) {
                    exceptions::willow::json::throw_json_parse_error("unexpected literal");
                }
                s.read_next();
                literal_checker<Rest...>::check(s);
            }
        };

        template <char_type... chars>
        token_type scan_literal(token_type result) {
            literal_checker<chars...>::check(*this);
            return result;
        }

        token_type scan_string() {
            if (current != '\"') {
                exceptions::willow::json::throw_json_parse_error("string must start with '\"'");
            }
            string_buffer_.clear();
            while (true) {
                const auto ch = span_data_ != nullptr ? scan_string_span() : read_next();
                switch (ch) {
                    case char_traits::eof(): {
                        exceptions::willow::json::throw_json_parse_error("unexpected end");
                    }
                    case '\"': {
                        read_next();
                        return token_type::value_string;
                    }
                    case 0x00:
                    case 0x01:
                    case 0x02:
                    case 0x03:
                    case 0x04:
                    case 0x05:
                    case 0x06:
                    case 0x07:
                    case 0x08:
                    case 0x09:
                    case 0x0A:
                    case 0x0B:
                    case 0x0C:
                    case 0x0D:
                    case 0x0E:
                    case 0x0F:
                    case 0x10:
                    case 0x11:
                    case 0x12:
                    case 0x13:
                    case 0x14:
                    case 0x15:
                    case 0x16:
                    case 0x17:
                    case 0x18:
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    case 0x1F:
                        exceptions::willow::json::throw_json_parse_error("invalid control character");
                    case '\\': {
                        switch (read_next()) {
                            case '\"':
                                add_char('\"');
                                break;
                            case '\\':
                                add_char('\\');
                                break;
                            case '/':
                                add_char('/');
                                break;
                            case 'b':
                                add_char('\b');
                                break;
                            case 'f':
                                add_char('\f');
                                break;
                            case 'n':
                                add_char('\n');
                                break;
                            case 'r':
                                add_char('\r');
                                break;
                            case 't':
                                add_char('\t');
                                break;
                            case 'u': {
                                const uint32_t code = read_one_escaped_code();
                                if (unicode_surrogate_lead_begin <= code && code <= unicode_surrogate_lead_end) {
                                    if (read_next() != '\\' || read_next() != 'u') {
                                        exceptions::willow::json::throw_json_parse_error(
                                            "lead surrogate must be followed by trail surrogate");
                                    }
                                    const auto lead_surrogate = code;
                                    const auto trail_surrogate = read_one_escaped_code();
                                    if (!(unicode_surrogate_lead_begin <= trail_surrogate &&
                                          trail_surrogate <= unicode_surrogate_lead_end)) {
                                        exceptions::willow::json::throw_json_parse_error(
                                            "surrogate U+D800...U+DBFF must be followed by U+DC00...U+DFFF");
                                    }
                                    unicode_writer<string_type> uw(this->string_buffer_);
                                    uw.add_surrogates(lead_surrogate, trail_surrogate);
                                } else {
                                    unicode_writer<string_type> uw(this->string_buffer_);
                                    uw.add_code(code);
                                }
                                break;
                            }
                            default: {
                                exceptions::willow::json::throw_json_parse_error("invalid character");
                            }
                        }
                        break;
                    }
                    default: {
                        add_char(ch);
                    }
                }
            }
        }

        token_type scan_number() {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                if (span_data_ != nullptr) {
                    return scan_number_span();
                }
            }
            number_buffer_.clear();
            if (current == '-') {
                push_number_char();
                read_next();
                if (!is_digit_char(current)) {
                    exceptions::willow::json::throw_json_parse_error("invalid integer");
                }
            }
            if (current == '0') {
                push_number_char();
                read_next();
            } else {
                consume_digits();
            }
            bool is_float = false;
            if (current == '.') {
                is_float = true;
                push_number_char();
                read_next();
                if (!is_digit_char(current)) {
                    exceptions::willow::json::throw_json_parse_error("invalid float number");
                }
                consume_digits();
            }
            if (current == 'e' || current == 'E') {
                is_float = true;
                push_number_char();
                read_next();
                if (current == '+' || current == '-') {
                    push_number_char();
                    read_next();
                }
                if (!is_digit_char(current)) {
                    exceptions::willow::json::throw_json_parse_error("invalid exponent number");
                }
                consume_digits();
            }
            return is_float ? scan_float_token() : scan_integer_token();
        }

        token_type scan_number_span() {
            const char_type *const begin = span_data_ + span_pos_ - 1;
            const char_type *scan = begin;
            const char_type *const end = span_data_ + span_size_;

            if (scan != end && *scan == '-') {
                ++scan;
            }
            if (scan == end || !is_digit_char(*scan)) {
                exceptions::willow::json::throw_json_parse_error("invalid integer");
            }
            if (*scan == '0') {
                ++scan;
                if (scan != end && is_digit_char(*scan)) {
                    exceptions::willow::json::throw_json_parse_error("invalid integer");
                }
            } else {
                while (scan != end && is_digit_char(*scan)) {
                    ++scan;
                }
            }
            bool is_float = false;
            if (scan != end && *scan == '.') {
                is_float = true;
                ++scan;
                if (scan == end || !is_digit_char(*scan)) {
                    exceptions::willow::json::throw_json_parse_error("invalid float number");
                }
                while (scan != end && is_digit_char(*scan)) {
                    ++scan;
                }
            }
            if (scan != end && (*scan == 'e' || *scan == 'E')) {
                is_float = true;
                ++scan;
                if (scan != end && (*scan == '+' || *scan == '-')) {
                    ++scan;
                }
                if (scan == end || !is_digit_char(*scan)) {
                    exceptions::willow::json::throw_json_parse_error("invalid exponent number");
                }
                while (scan != end && is_digit_char(*scan)) {
                    ++scan;
                }
            }

            span_pos_ = static_cast<std::size_t>(scan - span_data_);
            current = scan != end ? span_data_[span_pos_++] : char_traits::eof();

            if (is_float) {
                float_type value{};
                const auto result = core::text::from_chars(begin, scan, value);
                if (result.ec != std::errc{} && result.ec != std::errc::result_out_of_range) {
                    exceptions::willow::json::throw_json_parse_error("invalid number");
                }
                number_value_ = value;
                return token_type::value_float;
            }
            integer_type value{};
            const auto result = core::text::from_chars(begin, scan, value);
            if (result.ec == std::errc{}) {
                integer_value_ = value;
                return token_type::value_integer;
            }
            float_type float_value{};
            const auto float_result = core::text::from_chars(begin, scan, float_value);
            if (float_result.ec != std::errc{} && float_result.ec != std::errc::result_out_of_range) {
                exceptions::willow::json::throw_json_parse_error("invalid number");
            }
            number_value_ = float_value;
            return token_type::value_float;
        }

        token_type scan_float_token() {
            const char *begin = number_buffer_.data();
            const char *end = begin + number_buffer_.size();
            const auto result = core::text::from_chars(begin, end, number_value_);
            if (result.ec != std::errc{} && result.ec != std::errc::result_out_of_range) {
                exceptions::willow::json::throw_json_parse_error("invalid number");
            }
            return token_type::value_float;
        }

        token_type scan_integer_token() {
            const char *begin = number_buffer_.data();
            const char *end = begin + number_buffer_.size();
            integer_type integer_value{};
            if (core::text::from_chars(begin, end, integer_value).ec == std::errc{}) {
                integer_value_ = integer_value;
                return token_type::value_integer;
            }
            return scan_float_token();
        }

        integer_type token_to_integer() const {
            return integer_value_;
        }

        float_type token_to_float() const {
            return number_value_;
        }

        string_type token_to_string() const {
            return string_buffer_;
        }

        uint32_t read_one_escaped_code() {
            uint32_t code = 0;
            for (const auto factor: {12, 8, 4, 0}) {
                const auto ch = read_next();
                if (ch >= '0' && ch <= '9') {
                    code += ((ch - '0') << factor);
                } else if (ch >= 'A' && ch <= 'F') {
                    code += ((ch - 'A' + 10) << factor);
                } else if (ch >= 'a' && ch <= 'f') {
                    code += ((ch - 'a' + 10) << factor);
                } else {
                    exceptions::willow::json::throw_json_parse_error("'\\u' must be followed by 4 hex digits");
                }
            }
            return code;
        }

        void add_char(const char_int_type ch) {
            string_buffer_.push_back(char_traits::to_char_type(ch));
        }

        void push_number_char() {
            number_buffer_.push_back(static_cast<char>(current));
        }

        void consume_digits() {
            while (is_digit_char(current)) {
                push_number_char();
                read_next();
            }
        }

    private:
        static constexpr bool is_space_char(const char_int_type ch) noexcept {
            return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
        }

        static constexpr bool is_digit_char(const char_int_type ch) noexcept {
            return ch >= '0' && ch <= '9';
        }

        /**
         * @brief Broadcasts a character into a simd vector.
         *
         * @note The character is converted to char_type before the broadcast because basic_vec only accepts scalars
         * whose conversion to its value type is value preserving. Passing a plain char literal to a wchar_t vector
         * is rejected on MSVC, where char is signed and wchar_t is unsigned.
         */
        static constexpr simd_char_type simd_char(const char_type ch) noexcept {
            return simd_char_type{ch};
        }

        void skip_spaces_span() {
            constexpr core::simd_size_type width = simd_char_type::size();
            while (span_size_ - span_pos_ >= static_cast<std::size_t>(width)) {
                const auto chunk = core::unchecked_load<simd_char_type>(span_data_ + span_pos_, width);
                const auto spaces =
                    (chunk == simd_char(' ')) | (chunk == simd_char('\t')) | (chunk == simd_char('\n')) | (chunk == simd_char('\r'));
                if (core::any_of(!spaces)) {
                    for (core::simd_size_type i = 0; i < width; ++i) {
                        if (!spaces[i]) {
                            span_pos_ += static_cast<std::size_t>(i);
                            break;
                        }
                    }
                    current = span_data_[span_pos_++];
                    return;
                }
                span_pos_ += static_cast<std::size_t>(width);
            }
            while (span_pos_ < span_size_ && is_space_char(span_data_[span_pos_])) {
                ++span_pos_;
            }
            current = span_pos_ < span_size_ ? span_data_[span_pos_++] : char_traits::eof();
        }

        char_int_type scan_string_span() {
            constexpr core::simd_size_type width = simd_char_type::size();
            while (span_size_ - span_pos_ >= static_cast<std::size_t>(width)) {
                const simd_char_type quote_char = simd_char('\"');
                const simd_char_type escape_char = simd_char('\\');
                const simd_char_type control_begin = simd_char('\0');
                const simd_char_type control_end = simd_char(' ');
                const auto chunk = core::unchecked_load<simd_char_type>(span_data_ + span_pos_, width);
                const auto special =
                    (chunk == quote_char) | (chunk == escape_char) | ((chunk >= control_begin) & (chunk < control_end));
                if (core::any_of(special)) {
                    core::simd_size_type plain = 0;
                    while (!special[plain]) {
                        ++plain;
                    }
                    if (plain > 0) {
                        string_buffer_.append(span_data_ + span_pos_, static_cast<std::size_t>(plain));
                        span_pos_ += static_cast<std::size_t>(plain);
                    }
                    current = span_data_[span_pos_++];
                    return current;
                }
                string_buffer_.append(span_data_ + span_pos_, static_cast<std::size_t>(width));
                span_pos_ += static_cast<std::size_t>(width);
            }
            const char_type *const begin = span_data_ + span_pos_;
            const char_type *scan = begin;
            const char_type *const end = span_data_ + span_size_;
            while (scan != end) {
                const char_type byte = *scan;
                if (byte == char_type('\"') || byte == char_type('\\') || (byte >= char_type('\0') && byte < char_type(' '))) {
                    break;
                }
                ++scan;
            }
            if (scan != begin) {
                string_buffer_.append(begin, static_cast<std::size_t>(scan - begin));
                span_pos_ += static_cast<std::size_t>(scan - begin);
            }
            if (scan == end) {
                current = char_traits::eof();
                return current;
            }
            current = span_data_[span_pos_++];
            return current;
        }

    private:
        integer_type integer_value_{};
        float_type number_value_{};
        core::collections::vector<char> number_buffer_;
        string_type string_buffer_;
        Adapter adapter_;
        const char_type *span_data_{nullptr};
        std::size_t span_size_{0};
        std::size_t span_pos_{0};
        char_int_type current;
    };
}

#endif
