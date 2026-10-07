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
#ifndef RAINY_FOUNDATION_WILLOW_INI_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_INI_LEXER_HPP
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/ini/config.hpp>
#include <rainy/foundation/willow/implements/ini/exceptions.hpp>

namespace rainy::foundation::willow::ini::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct ini_lexer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        ini_lexer(Adapter adapter) : adapter_(adapter) {
            line_buffer_.reserve(128);
            key_buffer_.reserve(64);
            value_buffer_.reserve(64);
            read_next();
        }

        char_int_type read_next() {
            current = adapter_.get_char();
            return current;
        }

        token_type scan() {
            while (true) {
                if (current == char_traits::eof()) {
                    return token_type::end_of_input;
                }
                read_logical_line();
                trim_line();
                if (line_buffer_.empty()) {
                    continue;
                }
                const char_type lead = line_buffer_.front();
                if (lead == char_type(';') || lead == char_type('#')) {
                    continue;
                }
                if (lead == char_type('[')) {
                    scan_section();
                    return token_type::section;
                }
                scan_key_value();
                return token_type::key_value;
            }
        }

        string_type token_to_section() const {
            return key_buffer_;
        }

        string_type token_to_key() const {
            return key_buffer_;
        }

        string_type token_to_value() const {
            return value_buffer_;
        }

    private:
        void read_logical_line() {
            line_buffer_.clear();
            while (current != char_traits::eof()) {
                const char_type ch = char_traits::to_char_type(current);
                if (ch == char_type('\n')) {
                    read_next();
                    return;
                }
                if (ch == char_type('\r')) {
                    read_next();
                    if (current == char_type('\n')) {
                        read_next();
                    }
                    return;
                }
                line_buffer_.push_back(ch);
                read_next();
            }
        }

        void trim_line() {
            std::size_t begin = 0;
            while (begin < line_buffer_.size() && is_blank(line_buffer_[begin])) {
                ++begin;
            }
            std::size_t end = line_buffer_.size();
            while (end > begin && is_blank(line_buffer_[end - 1])) {
                --end;
            }
            if (begin == 0 && end == line_buffer_.size()) {
                return;
            }
            string_type trimmed;
            trimmed.reserve(end - begin);
            for (std::size_t i = begin; i < end; ++i) {
                trimmed.push_back(line_buffer_[i]);
            }
            line_buffer_ = utility::move(trimmed);
        }

        static bool is_blank(const char_type ch) {
            return ch == char_type(' ') || ch == char_type('\t');
        }

        void scan_section() {
            const std::size_t close = line_buffer_.find(char_type(']'));
            if (close == string_type::npos) {
                exceptions::willow::ini::throw_ini_parse_error("section header is missing ']'");
            }
            string_type name;
            name.reserve(close > 1 ? close - 1 : 0);
            for (std::size_t i = 1; i < close; ++i) {
                name.push_back(line_buffer_[i]);
            }
            trim_in_place(name);
            if (name.empty()) {
                exceptions::willow::ini::throw_ini_parse_error("section name is empty");
            }
            for (std::size_t i = close + 1; i < line_buffer_.size(); ++i) {
                const char_type ch = line_buffer_[i];
                if (is_blank(ch)) {
                    continue;
                }
                if (ch == char_type(';') || ch == char_type('#')) {
                    for (std::size_t j = i + 1; j < line_buffer_.size(); ++j) {
                        if (!is_blank(line_buffer_[j])) {
                            break;
                        }
                    }
                    key_buffer_ = utility::move(name);
                    return;
                }
                exceptions::willow::ini::throw_ini_parse_error("unexpected text after section header");
            }
            key_buffer_ = utility::move(name);
        }

        void scan_key_value() {
            const std::size_t equal = line_buffer_.find(char_type('='));
            if (equal == string_type::npos) {
                exceptions::willow::ini::throw_ini_parse_error("expected '=' in key-value line");
            }
            string_type key;
            key.reserve(equal);
            for (std::size_t i = 0; i < equal; ++i) {
                key.push_back(line_buffer_[i]);
            }
            string_type value;
            if (equal + 1 < line_buffer_.size()) {
                value.reserve(line_buffer_.size() - equal - 1);
                for (std::size_t i = equal + 1; i < line_buffer_.size(); ++i) {
                    value.push_back(line_buffer_[i]);
                }
            }
            trim_in_place(key);
            trim_in_place(value);
            if (key.empty()) {
                exceptions::willow::ini::throw_ini_parse_error("key is empty");
            }
            if (key.find(char_type('\n')) != string_type::npos || value.find(char_type('\n')) != string_type::npos) {
                exceptions::willow::ini::throw_ini_parse_error("key and value must stay on one line");
            }
            key_buffer_ = utility::move(key);
            value_buffer_ = utility::move(value);
        }

        static void trim_in_place(string_type &str) {
            std::size_t begin = 0;
            while (begin < str.size() && is_blank(str[begin])) {
                ++begin;
            }
            std::size_t end = str.size();
            while (end > begin && is_blank(str[end - 1])) {
                --end;
            }
            if (begin == 0 && end == str.size()) {
                return;
            }
            string_type trimmed;
            trimmed.reserve(end - begin);
            for (std::size_t i = begin; i < end; ++i) {
                trimmed.push_back(str[i]);
            }
            str = utility::move(trimmed);
        }

    private:
        string_type line_buffer_;
        string_type key_buffer_;
        string_type value_buffer_;
        Adapter adapter_;
        char_int_type current;
    };
}

#endif
