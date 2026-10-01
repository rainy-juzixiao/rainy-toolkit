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
#ifndef RAINY_FOUNDATION_WILLOW_JSON5_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_JSON5_LEXER_HPP
#include <cctype>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/json5/config.hpp>
#include <rainy/foundation/willow/implements/json5/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>
#include <type_traits>

namespace rainy::foundation::willow::json5::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct json5_lexer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        json5_lexer(Adapter adapter) : adapter_(adapter) {
            string_buffer_.reserve(96);
            number_buffer_.reserve(64);
            read_next();
        }

        char_int_type read_next() {
            if (has_pushback_) {
                has_pushback_ = false;
                current = pushback_;
                return current;
            }
            current = adapter_.get_char();
            return current;
        }

        void unread(const char_int_type ch) {
            pushback_ = ch;
            has_pushback_ = true;
        }

        void set_key_mode(const bool enabled) {
            key_mode_ = enabled;
        }

        static std::uint32_t code_unit(const char_int_type ch) {
            return static_cast<std::uint32_t>(
                static_cast<typename std::make_unsigned<char_type>::type>(char_traits::to_char_type(ch)));
        }

        static bool is_json5_space(const char_int_type ch) {
            const auto unit = code_unit(ch);
            return unit == 0x09 || unit == 0x0A || unit == 0x0B || unit == 0x0C || unit == 0x0D || unit == 0x20 || unit == 0xA0 ||
                   unit == 0x1680 || (unit >= 0x2000 && unit <= 0x200A) || unit == 0x2028 || unit == 0x2029 || unit == 0x202F ||
                   unit == 0x205F || unit == 0x3000 || unit == 0xFEFF;
        }

        static bool is_line_terminator(const char_int_type ch) {
            const auto unit = code_unit(ch);
            return unit == 0x0A || unit == 0x0D || unit == 0x2028 || unit == 0x2029;
        }

        static bool is_digit(const char_int_type ch) {
            return ch >= '0' && ch <= '9';
        }

        static bool is_hex_digit(const char_int_type ch) {
            return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
        }

        static bool is_ident_start(const char_int_type ch) {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$';
        }

        static bool is_ident_part(const char_int_type ch) {
            return is_ident_start(ch) || is_digit(ch);
        }

        void skip_spaces() {
            while (true) {
                if (is_json5_space(current)) {
                    read_next();
                    continue;
                }
                if (current == '/') {
                    read_next();
                    if (current == '/') {
                        while (current != char_traits::eof() && !is_line_terminator(current)) {
                            read_next();
                        }
                        continue;
                    }
                    if (current == '*') {
                        read_next();
                        while (true) {
                            if (current == char_traits::eof()) {
                                exceptions::willow::json5::throw_json5_parse_error("unterminated comment");
                            }
                            if (current == '*') {
                                read_next();
                                if (current == '/') {
                                    read_next();
                                    break;
                                }
                                continue;
                            }
                            read_next();
                        }
                        continue;
                    }
                    exceptions::willow::json5::throw_json5_parse_error("unexpected character");
                }
                return;
            }
        }

        token_type scan() {
            skip_spaces();
            if (key_mode_ && is_ident_start(current)) {
                return scan_identifier();
            }
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
                case 'I':
                    literal_checker<'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'>::check(*this);
                    number_value_ = std::numeric_limits<float_type>::infinity();
                    return token_type::value_float;
                case 'N':
                    literal_checker<'N', 'a', 'N'>::check(*this);
                    number_value_ = std::numeric_limits<float_type>::quiet_NaN();
                    return token_type::value_float;
                case '\"':
                case '\'':
                    return scan_string();
                case '-':
                case '+':
                case '.':
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
                    exceptions::willow::json5::throw_json5_parse_error("unexpected character");
            }
            read_next();

            return result;
        }

        template <char_type... Ch>
        struct literal_checker {
            static void check(json5_lexer &) {
            }
        };

        template <char_type This, char_type... Rest>
        struct literal_checker<This, Rest...> {
            static void check(json5_lexer &s) {
                if (char_traits::to_char_type(s.current) != This) {
                    exceptions::willow::json5::throw_json5_parse_error("unexpected literal");
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

        token_type scan_identifier() {
            string_buffer_.clear();
            while (is_ident_part(current)) {
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
            return token_type::value_string;
        }

        token_type scan_string() {
            const char_type quote = char_traits::to_char_type(current);
            string_buffer_.clear();
            while (true) {
                const auto ch = read_next();
                if (ch == char_traits::eof()) {
                    exceptions::willow::json5::throw_json5_parse_error("unexpected end in string");
                }
                if (ch == '\n' || ch == '\r') {
                    exceptions::willow::json5::throw_json5_parse_error("unescaped line terminator in string");
                }
                if (ch == '\\') {
                    scan_escape_sequence();
                    continue;
                }
                if (char_traits::to_char_type(ch) == quote) {
                    read_next();
                    return token_type::value_string;
                }
                string_buffer_.push_back(char_traits::to_char_type(ch));
            }
        }

        void scan_escape_sequence() {
            switch (const auto ch = read_next(); ch) {
                case char_traits::eof():
                    exceptions::willow::json5::throw_json5_parse_error("unexpected end in string");
                case '\"':
                    add_char('\"');
                    return;
                case '\'':
                    add_char('\'');
                    return;
                case '\\':
                    add_char('\\');
                    return;
                case '/':
                    add_char('/');
                    return;
                case 'b':
                    add_char('\b');
                    return;
                case 'f':
                    add_char('\f');
                    return;
                case 'n':
                    add_char('\n');
                    return;
                case 'r':
                    add_char('\r');
                    return;
                case 't':
                    add_char('\t');
                    return;
                case 'v':
                    add_char('\v');
                    return;
                case '0': {
                    read_next();
                    if (is_digit(current)) {
                        exceptions::willow::json5::throw_json5_parse_error("octal escape sequence is not supported");
                    }
                    unread(current);
                    add_char('\0');
                    return;
                }
                case 'x': {
                    const std::uint32_t code = read_hex_digits(2);
                    unicode_writer<string_type> uw(string_buffer_);
                    uw.add_code(code);
                    return;
                }
                case 'u': {
                    const std::uint32_t lead = read_hex_digits(4);
                    if (unicode_surrogate_lead_begin <= lead && lead <= unicode_surrogate_lead_end) {
                        if (read_next() != '\\' || read_next() != 'u') {
                            exceptions::willow::json5::throw_json5_parse_error("lead surrogate must be followed by trail surrogate");
                        }
                        const std::uint32_t trail = read_hex_digits(4);
                        if (!(unicode_surrogate_trail_begin <= trail && trail <= unicode_surrogate_trail_end)) {
                            exceptions::willow::json5::throw_json5_parse_error(
                                "surrogate U+D800...U+DBFF must be followed by U+DC00...U+DFFF");
                        }
                        unicode_writer<string_type> uw(string_buffer_);
                        uw.add_surrogates(lead, trail);
                    } else {
                        unicode_writer<string_type> uw(string_buffer_);
                        uw.add_code(lead);
                    }
                    return;
                }
                case '\n':
                    return;
                case '\r': {
                    read_next();
                    if (current != '\n' && current != char_traits::eof()) {
                        unread(current);
                    }
                    return;
                }
                default: {
                    if (is_line_terminator(ch)) {
                        return;
                    }
                    if (is_digit(ch)) {
                        exceptions::willow::json5::throw_json5_parse_error("octal escape sequence is not supported");
                    }
                    add_char(ch);
                    return;
                }
            }
        }

        std::uint32_t read_hex_digits(const int count) {
            std::uint32_t code = 0;
            for (int i = 0; i < count; ++i) {
                const auto ch = read_next();
                code <<= 4;
                if (ch >= '0' && ch <= '9') {
                    code |= static_cast<std::uint32_t>(ch - '0');
                } else if (ch >= 'a' && ch <= 'f') {
                    code |= static_cast<std::uint32_t>(ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    code |= static_cast<std::uint32_t>(ch - 'A' + 10);
                } else {
                    exceptions::willow::json5::throw_json5_parse_error("escape sequence must be followed by hex digits");
                }
            }
            return code;
        }

        token_type scan_number() {
            number_buffer_.clear();
            bool negative = false;
            if (current == '-' || current == '+') {
                negative = current == '-';
                if (negative) {
                    push_number_char();
                }
                read_next();
                if (current == 'I') {
                    literal_checker<'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'>::check(*this);
                    number_value_ =
                        negative ? -std::numeric_limits<float_type>::infinity() : std::numeric_limits<float_type>::infinity();
                    return token_type::value_float;
                }
                if (current == 'N') {
                    literal_checker<'N', 'a', 'N'>::check(*this);
                    number_value_ = std::numeric_limits<float_type>::quiet_NaN();
                    return token_type::value_float;
                }
            }
            bool is_float = false;
            if (current == '.') {
                is_float = true;
                number_buffer_.push_back('0');
                push_number_char();
                read_next();
                if (!is_digit(current)) {
                    exceptions::willow::json5::throw_json5_parse_error("invalid number");
                }
                while (is_digit(current)) {
                    push_number_char();
                    read_next();
                }
            } else if (current == '0') {
                push_number_char();
                read_next();
                if (current == 'x' || current == 'X') {
                    return scan_hex_number();
                }
                if (is_digit(current)) {
                    exceptions::willow::json5::throw_json5_parse_error("leading zeros are not allowed");
                }
                if (current == '.') {
                    is_float = true;
                    push_number_char();
                    read_next();
                    if (!is_digit(current)) {
                        number_buffer_.pop_back();
                    } else {
                        while (is_digit(current)) {
                            push_number_char();
                            read_next();
                        }
                    }
                }
            } else if (is_digit(current)) {
                while (is_digit(current)) {
                    push_number_char();
                    read_next();
                }
                if (current == '.') {
                    is_float = true;
                    push_number_char();
                    read_next();
                    if (!is_digit(current)) {
                        number_buffer_.pop_back();
                    } else {
                        while (is_digit(current)) {
                            push_number_char();
                            read_next();
                        }
                    }
                }
            } else {
                exceptions::willow::json5::throw_json5_parse_error("invalid number");
            }
            if (current == 'e' || current == 'E') {
                is_float = true;
                push_number_char();
                read_next();
                if (current == '+' || current == '-') {
                    push_number_char();
                    read_next();
                }
                if (!is_digit(current)) {
                    exceptions::willow::json5::throw_json5_parse_error("invalid exponent in number");
                }
                while (is_digit(current)) {
                    push_number_char();
                    read_next();
                }
            }
            return is_float ? scan_float_token() : scan_integer_token();
        }

        token_type scan_hex_number() {
            number_buffer_.pop_back();
            read_next();
            bool has_digits = false;
            while (is_hex_digit(current)) {
                push_number_char();
                has_digits = true;
                read_next();
            }
            if (!has_digits) {
                exceptions::willow::json5::throw_json5_parse_error("hex literal must contain at least one digit");
            }
            const char *begin = number_buffer_.data();
            const char *end = begin + number_buffer_.size();
            integer_type integer_value{};
            const auto int_result = core::text::from_chars(begin, end, integer_value, 16);
            if (int_result.ec == std::errc{} && int_result.ptr == end) {
                integer_value_ = integer_value;
                return token_type::value_integer;
            }
            const auto float_result = core::text::from_chars(begin, end, number_value_, core::text::chars_format::hex);
            if (float_result.ec == std::errc{} || float_result.ec == std::errc::result_out_of_range) {
                return token_type::value_float;
            }
            exceptions::willow::json5::throw_json5_parse_error("invalid hex number");
        }

        token_type scan_float_token() {
            const char *begin = number_buffer_.data();
            const char *end = begin + number_buffer_.size();
            const auto result = core::text::from_chars(begin, end, number_value_);
            if (result.ec != std::errc{} && result.ec != std::errc::result_out_of_range) {
                exceptions::willow::json5::throw_json5_parse_error("invalid number");
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

        void add_char(const char_int_type ch) {
            string_buffer_.push_back(char_traits::to_char_type(ch));
        }

        void push_number_char() {
            number_buffer_.push_back(static_cast<char>(current));
        }

    private:
        integer_type integer_value_{};
        float_type number_value_{};
        core::collections::vector<char> number_buffer_;
        string_type string_buffer_;
        Adapter adapter_;
        char_int_type current;
        char_int_type pushback_{};
        bool has_pushback_{false};
        bool key_mode_{false};
    };
}

#endif
