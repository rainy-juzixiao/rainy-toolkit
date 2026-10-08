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
#ifndef RAINY_FOUNDATION_WILLOW_HJSON_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_HJSON_LEXER_HPP
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/simd.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/hjson/config.hpp>
#include <rainy/foundation/willow/implements/hjson/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>
#include <type_traits>

namespace rainy::foundation::willow::hjson::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct hjson_lexer {
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

        hjson_lexer(Adapter adapter, const char_type *span_data = nullptr, std::size_t span_size = 0) :
            adapter_(adapter), span_data_(span_data), span_size_(span_data != nullptr ? span_size : 0) {
            string_buffer_.reserve(96);
            number_buffer_.reserve(64);
            read_next();
        }

        char_int_type read_next() {
            if (!pushback_.empty()) {
                current = pushback_.back();
                pushback_.pop_back();
            } else if (span_data_ != nullptr) {
                current = span_pos_ < span_size_ ? span_data_[span_pos_++] : char_traits::eof();
            } else {
                current = adapter_.get_char();
            }
            if (current == '\n' || current == '\r') {
                column_ = 0;
            } else {
                ++column_;
            }
            return current;
        }

        void unread(const char_int_type ch) {
            if (span_data_ != nullptr) {
                (void)ch;
                --span_pos_;
                return;
            }
            pushback_.push_back(ch);
        }

        void set_key_mode(const bool enabled) {
            key_mode_ = enabled;
        }

        token_type probe_root() {
            key_mode_ = true;
            probe_colon_ = false;
            const token_type probe = scan();
            key_mode_ = false;
            if (probe != token_type::value_string) {
                return probe;
            }
            white();
            probe_colon_ = current == ':';
            return probe;
        }

        token_type reclassify_root_value() {
            trim_buffer();
            if (equals_ascii("true")) {
                return token_type::literal_true;
            }
            if (equals_ascii("false")) {
                return token_type::literal_false;
            }
            if (equals_ascii("null")) {
                return token_type::literal_null;
            }
            if (classify_number()) {
                return number_is_float_ ? token_type::value_float : token_type::value_integer;
            }
            return token_type::value_string;
        }

        bool probe_sees_colon() const {
            return probe_colon_;
        }

        static std::uint32_t code_unit(const char_int_type ch) {
            return static_cast<std::uint32_t>(
                static_cast<typename std::make_unsigned<char_type>::type>(char_traits::to_char_type(ch)));
        }

        static bool is_hjson_space(const char_int_type ch) {
            const auto unit = code_unit(ch);
            return unit == 0x09 || unit == 0x0A || unit == 0x0D || unit == 0x20;
        }

        static bool is_digit(const char_int_type ch) {
            return ch >= '0' && ch <= '9';
        }

        static bool is_punctuator(const char_int_type ch) {
            return ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == ':' || ch == ',';
        }

        void white() {
            while (true) {
                if (current == ' ' || current == '\t') {
                    if (span_data_ != nullptr) {
                        skip_inline_spaces_span();
                        continue;
                    }
                    read_next();
                    continue;
                }
                if (is_hjson_space(current)) {
                    read_next();
                    continue;
                }
                if (current == '#') {
                    while (current != char_traits::eof() && current != '\n' && current != '\r') {
                        read_next();
                    }
                    continue;
                }
                if (current == '/') {
                    read_next();
                    if (current == '/') {
                        while (current != char_traits::eof() && current != '\n' && current != '\r') {
                            read_next();
                        }
                        continue;
                    }
                    if (current == '*') {
                        read_next();
                        while (true) {
                            if (current == char_traits::eof()) {
                                exceptions::willow::hjson::throw_hjson_parse_error("unterminated comment");
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
                    unread(current);
                    current = '/';
                    return;
                }
                return;
            }
        }

        token_type scan() {
            white();
            if (key_mode_) {
                if (current == char_traits::eof() || current == '\0') {
                    return token_type::end_of_input;
                }
                if (!is_punctuator(current) && current != '"' && current != '\'') {
                    return scan_key();
                }
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
                case '"':
                case '\'':
                    return scan_string();
                case '\0':
                case char_traits::eof():
                    return token_type::end_of_input;
                default:
                    return scan_quoteless();
            }
            read_next();
            return result;
        }

        string_type token_to_string() const {
            return string_buffer_;
        }

        integer_type token_to_integer() const {
            return integer_value_;
        }

        float_type token_to_float() const {
            return number_value_;
        }

        static bool try_parse_number(const char *begin, const char *end, integer_type &integer_out, float_type &float_out,
                                     bool &is_float) {
            if (begin == end) {
                return false;
            }
            integer_type integer_value{};
            const auto int_result = core::text::from_chars(begin, end, integer_value);
            if (int_result.ec == std::errc{} && int_result.ptr == end) {
                integer_out = integer_value;
                is_float = false;
                return true;
            }
            float_type float_value{};
            const auto float_result = core::text::from_chars(begin, end, float_value);
            if ((float_result.ec == std::errc{} || float_result.ec == std::errc::result_out_of_range) &&
                float_result.ptr == end) {
                float_out = float_value;
                is_float = true;
                return true;
            }
            return false;
        }

    private:
        token_type scan_key() {
            string_buffer_.clear();
            while (current != char_traits::eof() && !is_hjson_space(current) && !is_punctuator(current)) {
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
            return token_type::value_string;
        }

        token_type scan_string() {
            const char_type quote = char_traits::to_char_type(current);
            const std::size_t opener_column = column_ > 0 ? column_ - 1 : 0;
            if (quote == char_type('\'')) {
                read_next();
                if (current == char_type('\'')) {
                    read_next();
                    if (current == char_type('\'')) {
                        read_next();
                        return scan_multiline_string(opener_column);
                    }
                    string_buffer_.clear();
                    return token_type::value_string;
                }
                unread(current);
                current = char_type('\'');
                string_buffer_.clear();
                while (true) {
                    const auto ch = read_next();
                    if (ch == char_traits::eof()) {
                        exceptions::willow::hjson::throw_hjson_parse_error("unexpected end in string");
                    }
                    if (ch == '\n' || ch == '\r') {
                        exceptions::willow::hjson::throw_hjson_parse_error("unescaped line terminator in string");
                    }
                    if (ch == '\\') {
                        scan_escape_sequence();
                        continue;
                    }
                    if (ch == '\'') {
                        read_next();
                        return token_type::value_string;
                    }
                    string_buffer_.push_back(char_traits::to_char_type(ch));
                }
            }
            string_buffer_.clear();
            while (true) {
                const auto ch = read_next();
                if (ch == char_traits::eof()) {
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected end in string");
                }
                if (ch == '\n' || ch == '\r') {
                    exceptions::willow::hjson::throw_hjson_parse_error("unescaped line terminator in string");
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

        token_type scan_multiline_string(const std::size_t opener_column) {
            string_buffer_.clear();
            while (current != char_traits::eof() && current != '\n' && current != '\r') {
                if (!is_hjson_space(current)) {
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected text after multiline string opener");
                }
                read_next();
            }
            normalize_newline();
            bool pending_newline = false;
            while (true) {
                if (current == char_traits::eof()) {
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected end in multiline string");
                }
                std::size_t indent = 0;
                while ((current == ' ' || current == '\t') && indent < opener_column) {
                    ++indent;
                    read_next();
                }
                if (current == '\'') {
                    std::size_t quotes = 0;
                    while (current == '\'' && quotes < 3) {
                        ++quotes;
                        read_next();
                    }
                    if (quotes == 3) {
                        return token_type::value_string;
                    }
                    flush_pending(pending_newline);
                    for (std::size_t q = 0; q < quotes; ++q) {
                        string_buffer_.push_back(char_traits::to_char_type('\''));
                    }
                    read_line_body();
                    continue;
                }
                if (current == '\n' || current == '\r') {
                    normalize_newline();
                    pending_newline = true;
                    continue;
                }
                flush_pending(pending_newline);
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
                read_line_body();
            }
        }

        void flush_pending(bool &pending_newline) {
            if (pending_newline) {
                string_buffer_.push_back(char_traits::to_char_type('\n'));
                pending_newline = false;
            }
        }

        void read_line_body() {
            while (current != char_traits::eof() && current != '\n' && current != '\r') {
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
        }

        void normalize_newline() {
            if (current == '\r') {
                read_next();
                if (current == '\n') {
                    read_next();
                }
            } else {
                read_next();
            }
        }

        void scan_escape_sequence() {
            switch (const auto ch = read_next(); ch) {
                case char_traits::eof():
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected end in string");
                case '"':
                    add_char('"');
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
                case 'u': {
                    const std::uint32_t lead = read_hex_digits(4);
                    if (unicode_surrogate_lead_begin <= lead && lead <= unicode_surrogate_lead_end) {
                        if (read_next() != '\\' || read_next() != 'u') {
                            exceptions::willow::hjson::throw_hjson_parse_error(
                                "lead surrogate must be followed by trail surrogate");
                        }
                        const std::uint32_t trail = read_hex_digits(4);
                        if (!(unicode_surrogate_trail_begin <= trail && trail <= unicode_surrogate_trail_end)) {
                            exceptions::willow::hjson::throw_hjson_parse_error(
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
                default:
                    add_char(ch);
                    return;
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
                    exceptions::willow::hjson::throw_hjson_parse_error("escape sequence must be followed by hex digits");
                }
            }
            return code;
        }

        token_type scan_quoteless() {
            string_buffer_.clear();
            if (span_data_ != nullptr) {
                scan_quoteless_span();
            } else {
                while (true) {
                    if (current == char_traits::eof() || is_terminator_char(current)) {
                        break;
                    }
                    if (current == '/') {
                        read_next();
                        if (current == '/' || current == '*') {
                            unread(current);
                            current = '/';
                            break;
                        }
                        string_buffer_.push_back(char_traits::to_char_type('/'));
                        continue;
                    }
                    string_buffer_.push_back(char_traits::to_char_type(current));
                    read_next();
                }
            }
            trim_buffer();
            if (equals_ascii("true")) {
                return token_type::literal_true;
            }
            if (equals_ascii("false")) {
                return token_type::literal_false;
            }
            if (equals_ascii("null")) {
                return token_type::literal_null;
            }
            if (classify_number()) {
                return number_is_float_ ? token_type::value_float : token_type::value_integer;
            }
            return token_type::value_string;
        }

        bool equals_ascii(const char *ascii) const {
            std::size_t length = 0;
            while (ascii[length] != '\0') {
                ++length;
            }
            if (string_buffer_.size() != length) {
                return false;
            }
            for (std::size_t i = 0; i < length; ++i) {
                if (string_buffer_[i] != char_traits::to_char_type(static_cast<char_int_type>(ascii[i]))) {
                    return false;
                }
            }
            return true;
        }

        bool classify_number() {
            number_buffer_.clear();
            for (const auto ch: string_buffer_) {
                const auto unit = code_unit(static_cast<char_int_type>(ch));
                if (unit > 0x7F) {
                    return false;
                }
                number_buffer_.push_back(static_cast<char>(unit));
            }
            if (number_buffer_.empty()) {
                return false;
            }
            const char *begin = number_buffer_.data();
            const char *end = begin + number_buffer_.size();
            if (try_parse_number(begin, end, integer_value_, number_value_, number_is_float_)) {
                return true;
            }
            return false;
        }

        void trim_buffer() {
            std::size_t begin = 0;
            while (begin < string_buffer_.size() && (string_buffer_[begin] == char_type(' ') ||
                                                     string_buffer_[begin] == char_type('\t'))) {
                ++begin;
            }
            std::size_t end = string_buffer_.size();
            while (end > begin && (string_buffer_[end - 1] == char_type(' ') ||
                                   string_buffer_[end - 1] == char_type('\t'))) {
                --end;
            }
            if (begin == 0 && end == string_buffer_.size()) {
                return;
            }
            string_type trimmed;
            trimmed.reserve(end - begin);
            for (std::size_t i = begin; i < end; ++i) {
                trimmed.push_back(string_buffer_[i]);
            }
            string_buffer_ = utility::move(trimmed);
        }

        void add_char(const char_int_type ch) {
            string_buffer_.push_back(char_traits::to_char_type(ch));
        }

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
                    ++column_;
                    return;
                }
                span_pos_ += static_cast<std::size_t>(width);
            }
            while (span_pos_ < span_size_ && is_hjson_space(span_data_[span_pos_])) {
                ++span_pos_;
            }
            current = span_pos_ < span_size_ ? span_data_[span_pos_++] : char_traits::eof();
            if (current == '\n' || current == '\r') {
                column_ = 0;
            } else {
                ++column_;
            }
        }

        void skip_inline_spaces_span() {
            constexpr core::simd_size_type width = simd_char_type::size();
            while (span_size_ - span_pos_ >= static_cast<std::size_t>(width)) {
                const auto chunk = core::unchecked_load<simd_char_type>(span_data_ + span_pos_, width);
                const auto blanks = (chunk == simd_char(' ')) | (chunk == simd_char('\t'));
                if (core::any_of(!blanks)) {
                    core::simd_size_type skipped = 0;
                    for (; skipped < width; ++skipped) {
                        if (!blanks[skipped]) {
                            break;
                        }
                    }
                    span_pos_ += static_cast<std::size_t>(skipped);
                    current = span_data_[span_pos_++];
                    column_ += static_cast<std::size_t>(skipped) + 1;
                    return;
                }
                span_pos_ += static_cast<std::size_t>(width);
                column_ += static_cast<std::size_t>(width);
            }
            while (span_pos_ < span_size_ && (span_data_[span_pos_] == char_type(' ') ||
                                              span_data_[span_pos_] == char_type('\t'))) {
                ++span_pos_;
                ++column_;
            }
            current = span_pos_ < span_size_ ? span_data_[span_pos_++] : char_traits::eof();
            if (current == '\n' || current == '\r') {
                column_ = 0;
            } else {
                ++column_;
            }
        }

        static constexpr bool is_terminator_char(const char_int_type ch) noexcept {
            return ch == '\n' || ch == '\r' || ch == ',' || ch == '}' || ch == ']' || ch == '#';
        }

        char_int_type scan_quoteless_span() {
            const char_type *const begin = span_data_ + span_pos_ - 1;
            const char_type *scan = begin;
            const char_type *const end = span_data_ + span_size_;
            while (scan != end) {
                const char_type byte = *scan;
                if (byte == char_type('\n') || byte == char_type('\r') || byte == char_type(',') || byte == char_type('}') ||
                    byte == char_type(']') || byte == char_type('#')) {
                    break;
                }
                if (byte == char_type('/')) {
                    const char_type next = scan + 1 != end ? scan[1] : char_type('\0');
                    if (next == char_type('/') || next == char_type('*')) {
                        break;
                    }
                }
                ++scan;
            }
            string_buffer_.append(begin, static_cast<std::size_t>(scan - begin));
            span_pos_ = static_cast<std::size_t>(scan - span_data_);
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
        string_type string_buffer_;
        core::collections::vector<char> number_buffer_;
        core::collections::vector<char_int_type> pushback_;
        Adapter adapter_;
        const char_type *span_data_{nullptr};
        std::size_t span_size_{0};
        std::size_t span_pos_{0};
        char_int_type current;
        std::size_t column_{0};
        bool key_mode_{false};
        bool probe_colon_{false};
        bool number_is_float_{false};
    };
}

#endif
