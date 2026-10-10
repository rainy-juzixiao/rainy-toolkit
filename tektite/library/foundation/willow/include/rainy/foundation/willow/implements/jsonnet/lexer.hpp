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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_LEXER_HPP
#include <cstddef>
#include <cstdint>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/char_traits.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/ast/source_location.hpp>
#include <rainy/foundation/willow/implements/jsonnet/config.hpp>
#include <rainy/foundation/willow/writer.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    using namespace rainy::foundation::exceptions::willow;
    using willow::implements::unicode_surrogate_lead_begin;
    using willow::implements::unicode_surrogate_lead_end;
    using willow::implements::unicode_surrogate_trail_begin;
    using willow::implements::unicode_surrogate_trail_end;

    enum class token_type {
        uninitialized,
        end_of_input,
        identifier,
        literal_null,
        literal_true,
        literal_false,
        literal_number,
        literal_string,
        kw_assert,
        kw_else,
        kw_error,
        kw_for,
        kw_function,
        kw_if,
        kw_import,
        kw_importstr,
        kw_importbin,
        kw_in,
        kw_local,
        kw_tailstrict,
        kw_then,
        kw_self,
        kw_super,
        brace_left,
        brace_right,
        bracket_left,
        bracket_right,
        paren_left,
        paren_right,
        comma,
        dot,
        semicolon,
        dollar,
        colon,
        double_colon,
        triple_colon,
        op_plus,
        op_minus,
        op_star,
        op_slash,
        op_percent,
        op_shift_left,
        op_shift_right,
        op_less,
        op_less_equal,
        op_greater,
        op_greater_equal,
        op_equal,
        op_not_equal,
        op_ampersand,
        op_caret,
        op_pipe,
        op_and,
        op_or,
        op_not,
        op_tilde,
    };

    const char *token_name(token_type type) noexcept;

    struct token {
        token_type type{token_type::uninitialized};
        core::text::string text{};
        double number{0.0};
        bool is_integer{false};
        ast::source_span span{};
    };

    struct jsonnet_lexer {
        using char_traits = core::text::char_traits<char>;

        jsonnet_lexer(const char *data, const std::size_t size) : data_(data), size_(size) {
        }

        token scan() {
            skip_trivia();
            token result;
            result.span.begin = location();
            if (position_ >= size_) {
                result.type = token_type::end_of_input;
                result.span.end = location();
                return result;
            }
            const char ch = data_[position_];
            if (is_identifier_start(ch)) {
                scan_identifier(result);
            } else if (is_digit(ch)) {
                scan_number(result);
            } else if (ch == '"' || ch == '\'') {
                scan_string(result, ch, false);
            } else if (ch == '@' && position_ + 1 < size_ && (data_[position_ + 1] == '"' || data_[position_ + 1] == '\'')) {
                advance();
                scan_string(result, data_[position_], true);
            } else if (ch == '|' && match_text_block_opener()) {
                scan_text_block(result);
            } else {
                scan_symbol(result, ch);
            }
            result.span.end = location();
            return result;
        }

        const char *data() const noexcept {
            return data_;
        }

        std::size_t size() const noexcept {
            return size_;
        }

    private:
        static bool is_digit(const char ch) noexcept {
            return ch >= '0' && ch <= '9';
        }

        static bool is_identifier_start(const char ch) noexcept {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
        }

        static bool is_identifier_part(const char ch) noexcept {
            return is_identifier_start(ch) || is_digit(ch);
        }

        static bool is_space(const char ch) noexcept {
            return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
        }

        static bool is_operator_char(const char ch) noexcept {
            switch (ch) {
                case '!':
                case '$':
                case ':':
                case '~':
                case '+':
                case '-':
                case '&':
                case '|':
                case '^':
                case '=':
                case '<':
                case '>':
                case '*':
                case '/':
                case '%':
                    return true;
                default:
                    return false;
            }
        }

        ast::source_location location() const noexcept {
            return ast::source_location{line_, column_, static_cast<std::uint32_t>(position_)};
        }

        char peek(const std::size_t ahead = 0) const noexcept {
            const std::size_t index = position_ + ahead;
            return index < size_ ? data_[index] : '\0';
        }

        char advance() noexcept {
            const char ch = data_[position_++];
            if (ch == '\n') {
                ++line_;
                column_ = 1;
            } else {
                ++column_;
            }
            return ch;
        }

        void skip_trivia() {
            while (position_ < size_) {
                const char ch = data_[position_];
                if (is_space(ch)) {
                    advance();
                    continue;
                }
                if (ch == '#') {
                    skip_line_comment();
                    continue;
                }
                if (ch == '/' && peek(1) == '/') {
                    skip_line_comment();
                    continue;
                }
                if (ch == '/' && peek(1) == '*') {
                    skip_block_comment();
                    continue;
                }
                break;
            }
        }

        void skip_line_comment() {
            while (position_ < size_ && data_[position_] != '\n') {
                advance();
            }
        }

        void skip_block_comment() {
            advance();
            advance();
            while (position_ < size_) {
                if (data_[position_] == '*' && peek(1) == '/') {
                    advance();
                    advance();
                    return;
                }
                advance();
            }
            exceptions::willow::jsonnet::throw_jsonnet_parse_error("unterminated block comment");
        }

        void scan_identifier(token &result) {
            const std::size_t begin = position_;
            while (position_ < size_ && is_identifier_part(data_[position_])) {
                advance();
            }
            const auto *start = data_ + begin;
            const auto length = position_ - begin;
            const core::text::basic_string_view<char> text{start, length};
            result.text.assign(start, length);
            result.type = keyword_of(text);
        }

        static token_type keyword_of(const core::text::basic_string_view<char> text) noexcept {
            struct entry {
                const char *name;
                token_type type;
            };
            static constexpr entry keywords[] = {
                {"assert", token_type::kw_assert},   {"else", token_type::kw_else},
                {"error", token_type::kw_error},     {"false", token_type::literal_false},
                {"for", token_type::kw_for},         {"function", token_type::kw_function},
                {"if", token_type::kw_if},           {"import", token_type::kw_import},
                {"importbin", token_type::kw_importbin}, {"importstr", token_type::kw_importstr},
                {"in", token_type::kw_in},           {"local", token_type::kw_local},
                {"null", token_type::literal_null},  {"self", token_type::kw_self},
                {"super", token_type::kw_super},     {"tailstrict", token_type::kw_tailstrict},
                {"then", token_type::kw_then},       {"true", token_type::literal_true},
            };
            for (const entry &candidate: keywords) {
                if (text == core::text::basic_string_view<char>{candidate.name}) {
                    return candidate.type;
                }
            }
            return token_type::identifier;
        }

        void scan_number(token &result) {
            core::text::string cleaned;
            if (peek() == '0') {
                cleaned.push_back(advance());
            } else {
                while (position_ < size_ && (is_digit(data_[position_]) || data_[position_] == '_')) {
                    const char ch = advance();
                    if (ch != '_') {
                        cleaned.push_back(ch);
                    }
                }
            }
            bool is_float = false;
            if (peek() == '.' && is_digit(peek(1))) {
                is_float = true;
                cleaned.push_back(advance());
                while (position_ < size_ && (is_digit(data_[position_]) || data_[position_] == '_')) {
                    const char ch = advance();
                    if (ch != '_') {
                        cleaned.push_back(ch);
                    }
                }
            }
            if (peek() == 'e' || peek() == 'E') {
                is_float = true;
                cleaned.push_back(advance());
                if (peek() == '+' || peek() == '-') {
                    cleaned.push_back(advance());
                }
                if (!is_digit(peek())) {
                    exceptions::willow::jsonnet::throw_jsonnet_parse_error("invalid exponent in number");
                }
                while (position_ < size_ && (is_digit(data_[position_]) || data_[position_] == '_')) {
                    const char ch = advance();
                    if (ch != '_') {
                        cleaned.push_back(ch);
                    }
                }
            }
            double value{};
            const auto result_code =
                core::text::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), value);
            if (result_code.ec != std::errc{} && result_code.ec != std::errc::result_out_of_range) {
                exceptions::willow::jsonnet::throw_jsonnet_parse_error("invalid number");
            }
            result.type = token_type::literal_number;
            result.number = value;
            result.is_integer = !is_float;
            result.text = utility::move(cleaned);
        }

        void scan_string(token &result, const char quote, const bool verbatim) {
            advance();
            core::text::string buffer;
            while (true) {
                if (position_ >= size_) {
                    exceptions::willow::jsonnet::throw_jsonnet_parse_error("unterminated string");
                }
                const char ch = data_[position_];
                if (ch == quote) {
                    advance();
                    if (verbatim && peek() == quote) {
                        buffer.push_back(quote);
                        advance();
                        continue;
                    }
                    break;
                }
                if (!verbatim && ch == '\\') {
                    advance();
                    scan_escape(buffer);
                    continue;
                }
                buffer.push_back(advance());
            }
            result.type = token_type::literal_string;
            result.text = utility::move(buffer);
        }

        void scan_escape(core::text::string &buffer) {
            if (position_ >= size_) {
                exceptions::willow::jsonnet::throw_jsonnet_parse_error("unterminated escape sequence");
            }
            const char ch = advance();
            switch (ch) {
                case '"':
                    buffer.push_back('"');
                    break;
                case '\'':
                    buffer.push_back('\'');
                    break;
                case '\\':
                    buffer.push_back('\\');
                    break;
                case '/':
                    buffer.push_back('/');
                    break;
                case 'b':
                    buffer.push_back('\b');
                    break;
                case 'f':
                    buffer.push_back('\f');
                    break;
                case 'n':
                    buffer.push_back('\n');
                    break;
                case 'r':
                    buffer.push_back('\r');
                    break;
                case 't':
                    buffer.push_back('\t');
                    break;
                case 'u': {
                    const std::uint32_t code = read_hex4();
                    if (unicode_surrogate_lead_begin <= code && code <= unicode_surrogate_lead_end) {
                        if (peek() == '\\' && peek(1) == 'u') {
                            advance();
                            advance();
                            const std::uint32_t trail = read_hex4();
                            if (unicode_surrogate_trail_begin <= trail && trail <= unicode_surrogate_trail_end) {
                                unicode_writer<core::text::string> writer{buffer};
                                writer.add_surrogates(code, trail);
                                return;
                            }
                            unicode_writer<core::text::string> writer{buffer};
                            writer.add_code(code);
                            writer.add_code(trail);
                            return;
                        }
                    }
                    unicode_writer<core::text::string> writer{buffer};
                    writer.add_code(code);
                    break;
                }
                default:
                    exceptions::willow::jsonnet::throw_jsonnet_parse_error("invalid escape sequence");
            }
        }

        std::uint32_t read_hex4() {
            std::uint32_t code = 0;
            for (int i = 0; i < 4; ++i) {
                if (position_ >= size_) {
                    exceptions::willow::jsonnet::throw_jsonnet_parse_error("incomplete unicode escape");
                }
                const char ch = advance();
                code <<= 4;
                if (ch >= '0' && ch <= '9') {
                    code |= static_cast<std::uint32_t>(ch - '0');
                } else if (ch >= 'a' && ch <= 'f') {
                    code |= static_cast<std::uint32_t>(ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    code |= static_cast<std::uint32_t>(ch - 'A' + 10);
                } else {
                    exceptions::willow::jsonnet::throw_jsonnet_parse_error("invalid unicode escape digit");
                }
            }
            return code;
        }

        bool match_text_block_opener() const noexcept {
            if (peek() != '|' || peek(1) != '|' || peek(2) != '|') {
                return false;
            }
            std::size_t index = 3;
            if (peek(index) == '-') {
                ++index;
            }
            const char after = peek(index);
            return after == '\n' || after == '\r' || after == '\0';
        }

        void scan_text_block(token &result) {
            advance();
            advance();
            advance();
            bool chomp = false;
            if (peek() == '-') {
                chomp = true;
                advance();
            }
            if (peek() == '\r') {
                advance();
            }
            if (peek() == '\n') {
                advance();
            }
            core::text::string raw;
            core::text::string prefix;
            bool prefix_known = false;
            while (position_ < size_) {
                const std::size_t line_begin = position_;
                while (position_ < size_ && data_[position_] != '\n') {
                    advance();
                }
                const bool has_newline = position_ < size_;
                const std::size_t line_end = position_;
                const core::text::basic_string_view<char> line{data_ + line_begin, line_end - line_begin};
                if (has_newline) {
                    advance();
                }
                if (is_terminator(line)) {
                    break;
                }
                if (!prefix_known && !is_blank(line)) {
                    prefix = leading_whitespace(line);
                    prefix_known = true;
                }
                if (prefix_known) {
                    raw.append(strip_prefix(line, prefix));
                } else {
                    raw.append(line.data(), line.size());
                }
                if (has_newline) {
                    raw.push_back('\n');
                }
            }
            if (chomp && !raw.empty() && raw.back() == '\n') {
                raw.pop_back();
            }
            result.type = token_type::literal_string;
            result.text = utility::move(raw);
        }

        static bool is_blank(const core::text::basic_string_view<char> line) noexcept {
            for (const char ch: line) {
                if (ch != ' ' && ch != '\t' && ch != '\r') {
                    return false;
                }
            }
            return true;
        }

        static core::text::string leading_whitespace(const core::text::basic_string_view<char> line) {
            core::text::string prefix;
            for (const char ch: line) {
                if (ch == ' ' || ch == '\t') {
                    prefix.push_back(ch);
                } else {
                    break;
                }
            }
            return prefix;
        }

        static core::text::basic_string_view<char> strip_prefix(const core::text::basic_string_view<char> line,
                                                                const core::text::string &prefix) noexcept {
            if (prefix.empty() || line.size() < prefix.size()) {
                return line;
            }
            for (std::size_t i = 0; i < prefix.size(); ++i) {
                if (line[i] != prefix[i]) {
                    return line;
                }
            }
            return core::text::basic_string_view<char>{line.data() + prefix.size(), line.size() - prefix.size()};
        }

        static bool is_terminator(const core::text::basic_string_view<char> line) noexcept {
            std::size_t i = 0;
            while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
                ++i;
            }
            return i + 3 <= line.size() && line[i] == '|' && line[i + 1] == '|' && line[i + 2] == '|' &&
                   i + 3 == line.size();
        }

        void scan_symbol(token &result, const char ch) {
            switch (ch) {
                case '{':
                    result.type = token_type::brace_left;
                    advance();
                    return;
                case '}':
                    result.type = token_type::brace_right;
                    advance();
                    return;
                case '[':
                    result.type = token_type::bracket_left;
                    advance();
                    return;
                case ']':
                    result.type = token_type::bracket_right;
                    advance();
                    return;
                case '(':
                    result.type = token_type::paren_left;
                    advance();
                    return;
                case ')':
                    result.type = token_type::paren_right;
                    advance();
                    return;
                case ',':
                    result.type = token_type::comma;
                    advance();
                    return;
                case '.':
                    result.type = token_type::dot;
                    advance();
                    return;
                case ';':
                    result.type = token_type::semicolon;
                    advance();
                    return;
                case '$':
                    result.type = token_type::dollar;
                    advance();
                    return;
                default:
                    break;
            }
            if (!is_operator_char(ch)) {
                exceptions::willow::jsonnet::throw_jsonnet_parse_error("unexpected character");
            }
            scan_operator(result);
        }

        void scan_operator(token &result) {
            if (match3(':', ':', ':')) {
                result.type = token_type::triple_colon;
                advance();
                advance();
                advance();
                return;
            }
            if (match2(':', ':')) {
                result.type = token_type::double_colon;
                advance();
                advance();
                return;
            }
            if (match2('=', '=')) {
                result.type = token_type::op_equal;
                advance();
                advance();
                return;
            }
            if (match2('!', '=')) {
                result.type = token_type::op_not_equal;
                advance();
                advance();
                return;
            }
            if (match2('<', '=')) {
                result.type = token_type::op_less_equal;
                advance();
                advance();
                return;
            }
            if (match2('>', '=')) {
                result.type = token_type::op_greater_equal;
                advance();
                advance();
                return;
            }
            if (match2('<', '<')) {
                result.type = token_type::op_shift_left;
                advance();
                advance();
                return;
            }
            if (match2('>', '>')) {
                result.type = token_type::op_shift_right;
                advance();
                advance();
                return;
            }
            if (match2('&', '&')) {
                result.type = token_type::op_and;
                advance();
                advance();
                return;
            }
            if (match2('|', '|')) {
                result.type = token_type::op_or;
                advance();
                advance();
                return;
            }
            const char ch = advance();
            switch (ch) {
                case ':':
                    result.type = token_type::colon;
                    break;
                case '+':
                    result.type = token_type::op_plus;
                    break;
                case '-':
                    result.type = token_type::op_minus;
                    break;
                case '*':
                    result.type = token_type::op_star;
                    break;
                case '/':
                    result.type = token_type::op_slash;
                    break;
                case '%':
                    result.type = token_type::op_percent;
                    break;
                case '<':
                    result.type = token_type::op_less;
                    break;
                case '>':
                    result.type = token_type::op_greater;
                    break;
                case '=':
                    result.type = token_type::op_equal;
                    break;
                case '&':
                    result.type = token_type::op_ampersand;
                    break;
                case '^':
                    result.type = token_type::op_caret;
                    break;
                case '|':
                    result.type = token_type::op_pipe;
                    break;
                case '!':
                    result.type = token_type::op_not;
                    break;
                case '~':
                    result.type = token_type::op_tilde;
                    break;
                default:
                    exceptions::willow::jsonnet::throw_jsonnet_parse_error("unexpected character");
            }
        }

        bool match2(const char a, const char b) const noexcept {
            return peek() == a && peek(1) == b;
        }

        bool match3(const char a, const char b, const char c) const noexcept {
            return peek() == a && peek(1) == b && peek(2) == c;
        }

        const char *data_{nullptr};
        std::size_t size_{0};
        std::size_t position_{0};
        std::uint32_t line_{1};
        std::uint32_t column_{1};
    };
}

#endif

#endif
