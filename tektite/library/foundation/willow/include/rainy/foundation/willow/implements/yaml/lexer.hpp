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
#ifndef RAINY_FOUNDATION_WILLOW_YAML_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_YAML_LEXER_HPP
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/simd.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/yaml/config.hpp>
#include <rainy/foundation/willow/implements/yaml/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::yaml::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename Adapter, typename = void>
    struct is_contiguous_adapter : type_traits::helper::false_type {};

    template <typename Adapter>
    struct is_contiguous_adapter<
        Adapter, type_traits::other_trans::void_t<decltype(utility::declval<const Adapter &>().data()),
                                                  decltype(utility::declval<const Adapter &>().size()),
                                                  decltype(utility::declval<const Adapter &>().position()),
                                                  decltype(utility::declval<Adapter &>().set_position(std::size_t{}))>>
        : type_traits::helper::true_type {};

    template <typename BasicDocument, typename Adapter>
    struct yaml_lexer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        enum class plain_keyword {
            none,
            true_value,
            false_value,
            null_value
        };

        template <std::size_t N>
        static bool equals_word(const string_type &str, const char_type (&word)[N]) noexcept {
            return str.size() == N - 1 && char_traits::compare(str.data(), word, N - 1) == 0;
        }

        static constexpr bool contiguous_adapter = is_contiguous_adapter<Adapter>::value;

        using simd_char_type = core::vec<char_type>;

        static constexpr std::size_t block_scan_threshold = static_cast<std::size_t>(simd_char_type::size());

        static constexpr simd_char_type simd_char(const char_type ch) noexcept {
            return simd_char_type{ch};
        }

        static constexpr bool is_plain_scalar_stop(const char_type ch, const bool flow) noexcept {
            if (ch == char_type('\n') || ch == char_type('\r') || ch == char_type(':') || ch == char_type('#')) {
                return true;
            }
            if (flow) {
                return ch == char_type(',') || ch == char_type(']') || ch == char_type('}');
            }
            return false;
        }

        const char_type *find_plain_scalar_stop(const char_type *first, const char_type *last) const {
            constexpr core::simd_size_type width = simd_char_type::size();
            const bool flow = in_flow();
            const char_type *scan = first;
            while (static_cast<std::size_t>(last - scan) >= static_cast<std::size_t>(width)) {
                const auto chunk = core::unchecked_load<simd_char_type>(scan, width);
                auto stop = (chunk == simd_char('\n')) | (chunk == simd_char('\r')) | (chunk == simd_char(':')) |
                            (chunk == simd_char('#'));
                if (flow) {
                    stop = stop | (chunk == simd_char(',')) | (chunk == simd_char(']')) | (chunk == simd_char('}'));
                }
                if (core::any_of(stop)) {
                    core::simd_size_type index = 0;
                    while (!stop[index]) {
                        ++index;
                    }
                    return scan + index;
                }
                scan += width;
            }
            while (scan != last && !is_plain_scalar_stop(*scan, flow)) {
                ++scan;
            }
            return scan;
        }

        const char_type *buffer_begin() const noexcept {
            return adapter_.data();
        }

        const char_type *buffer_end() const noexcept {
            return adapter_.data() + adapter_.size();
        }

        std::size_t buffer_position() const noexcept {
            return adapter_.position();
        }

        void jump_to(const char_type *target) noexcept {
            const auto offset = static_cast<std::size_t>(target - buffer_begin());
            if (offset < adapter_.size()) {
                current = char_traits::to_int_type(*target);
                adapter_.set_position(offset + 1);
            } else {
                current = char_traits::eof();
                adapter_.set_position(adapter_.size());
            }
        }

        bool can_block_scan() const noexcept {
            if constexpr (contiguous_adapter) {
                return !has_pushback_ && current != char_traits::eof();
            }
            return false;
        }

        yaml_lexer(Adapter adapter) : adapter_(adapter) {
            string_buffer_.reserve(96);
            read_next();
            current_line_indent_ = measure_indent();
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

        char_int_type peek_next() {
            const auto ch = adapter_.get_char();
            unread(ch);
            return ch;
        }

        void enter_flow() {
            ++flow_depth_;
        }

        void leave_flow() {
            if (flow_depth_ > 0) {
                --flow_depth_;
            }
        }

        bool in_flow() const {
            return flow_depth_ > 0;
        }

        static std::uint32_t code_unit(const char_int_type ch) {
            return static_cast<std::uint32_t>(
                static_cast<type_traits::helper::make_unsigned_t<char_type>>(char_traits::to_char_type(ch)));
        }

        static bool is_yaml_space(const char_int_type ch) {
            const auto unit = code_unit(ch);
            return unit == 0x20 || unit == 0x09;
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

        static bool is_oct_digit(const char_int_type ch) {
            return ch >= '0' && ch <= '7';
        }

        static bool is_indicator(const char_int_type ch) {
            switch (code_unit(ch)) {
                case '-':
                case '?':
                case ':':
                case ',':
                case '[':
                case ']':
                case '{':
                case '}':
                case '#':
                case '&':
                case '*':
                case '!':
                case '|':
                case '>':
                case '\'':
                case '\"':
                case '%':
                case '@':
                case '`':
                    return true;
                default:
                    return false;
            }
        }

        static bool is_anchor_char(const char_int_type ch) {
            return !is_yaml_space(ch) && !is_line_terminator(ch) && ch != char_traits::eof() && !is_indicator(ch);
        }

        static bool is_tag_char(const char_int_type ch) {
            return !is_yaml_space(ch) && !is_line_terminator(ch) && ch != char_traits::eof() && ch != ',';
        }

        int measure_indent() {
            int indent = 0;
            while (is_yaml_space(current)) {
                ++indent;
                read_next();
            }
            return indent;
        }

        int current_line_indent() const {
            return current_line_indent_;
        }

        void skip_spaces() {
            while (is_yaml_space(current)) {
                read_next();
            }
        }

        void skip_comment() {
            if (current == '#') {
                while (current != char_traits::eof() && !is_line_terminator(current)) {
                    read_next();
                }
            }
        }

        void skip_blank_and_comments() {
            while (true) {
                skip_spaces();
                skip_comment();
                if (is_line_terminator(current)) {
                    read_next();
                    int indent = 0;
                    while (is_yaml_space(current)) {
                        ++indent;
                        read_next();
                    }
                    current_line_indent_ = indent;
                    continue;
                }
                break;
            }
        }

        void consume_line_terminator() {
            if (current == '\r') {
                read_next();
                if (current == '\n') {
                    read_next();
                }
                return;
            }
            read_next();
        }

        string_type read_block_scalar(const bool literal) {
            enum class chomping { clip, strip, keep };
            chomping mode = chomping::clip;
            int explicit_indent = -1;
            while (true) {
                const char_int_type ch = current;
                if (ch >= '1' && ch <= '9') {
                    if (explicit_indent < 0) {
                        explicit_indent = static_cast<int>(ch - '0');
                    }
                    read_next();
                    continue;
                }
                if (ch == '+' || ch == '-') {
                    mode = (ch == '+') ? chomping::keep : chomping::strip;
                    read_next();
                    continue;
                }
                break;
            }
            while (current != char_traits::eof() && !is_line_terminator(current)) {
                read_next();
            }
            if (current != char_traits::eof()) {
                consume_line_terminator();
            }

            const int parent = block_parent_indent_;
            int detected = explicit_indent > 0 ? parent + explicit_indent : -1;
            core::collections::vector<string_type> lines;
            bool final_break = false;

            while (current != char_traits::eof()) {
                int raw_indent = 0;
                while (is_yaml_space(current)) {
                    ++raw_indent;
                    read_next();
                }
                current_line_indent_ = raw_indent;
                if (current == char_traits::eof()) {
                    break;
                }
                if (is_line_terminator(current)) {
                    consume_line_terminator();
                    final_break = true;
                    lines.emplace_back();
                    continue;
                }
                if (raw_indent <= parent) {
                    break;
                }
                if (detected < 0) {
                    detected = raw_indent;
                } else if (raw_indent < detected) {
                    break;
                }
                string_type line;
                for (int i = 0; i < raw_indent - detected; ++i) {
                    line.push_back(char_traits::to_char_type(' '));
                }
                while (current != char_traits::eof() && !is_line_terminator(current)) {
                    line.push_back(char_traits::to_char_type(current));
                    read_next();
                }
                lines.push_back(utility::move(line));
                final_break = false;
                if (current != char_traits::eof()) {
                    consume_line_terminator();
                    final_break = true;
                }
            }

            string_type value;
            for (std::size_t i = 0; i < lines.size(); ++i) {
                if (i > 0) {
                    value.push_back(char_traits::to_char_type('\n'));
                }
                value += lines[i];
            }
            if (final_break) {
                value.push_back(char_traits::to_char_type('\n'));
            }

            std::size_t trailing = 0;
            while (trailing < value.size() && value[value.size() - 1 - trailing] == char_traits::to_char_type('\n')) {
                ++trailing;
            }
            if (trailing > 0) {
                if (mode == chomping::strip) {
                    value.erase(value.size() - trailing);
                } else if (mode == chomping::clip) {
                    value.erase(value.size() - trailing);
                    value.push_back(char_traits::to_char_type('\n'));
                }
            }

            if (!literal) {
                std::size_t lead = 0;
                while (lead < value.size() && value[lead] == char_traits::to_char_type('\n')) {
                    ++lead;
                }
                std::size_t trail = 0;
                while (trail < value.size() - lead && value[value.size() - 1 - trail] == char_traits::to_char_type('\n')) {
                    ++trail;
                }
                const std::size_t middle_end = value.size() - trail;
                string_type folded;
                folded.reserve(value.size());
                for (std::size_t i = 0; i < lead; ++i) {
                    folded.push_back(value[i]);
                }
                std::size_t position = lead;
                while (position < middle_end) {
                    if (value[position] != char_traits::to_char_type('\n')) {
                        folded.push_back(value[position]);
                        ++position;
                        continue;
                    }
                    std::size_t run = 0;
                    while (position + run < middle_end && value[position + run] == char_traits::to_char_type('\n')) {
                        ++run;
                    }
                    if (run == 1) {
                        folded.push_back(char_traits::to_char_type(' '));
                    } else {
                        for (std::size_t i = 1; i < run; ++i) {
                            folded.push_back(char_traits::to_char_type('\n'));
                        }
                    }
                    position += run;
                }
                for (std::size_t i = 0; i < trail; ++i) {
                    folded.push_back(char_traits::to_char_type('\n'));
                }
                value = utility::move(folded);
            }
            return value;
        }

        token_type scan() {
            skip_blank_and_comments();

            if (current == char_traits::eof() || current == '\0') {
                return token_type::end_of_input;
            }

            switch (char_traits::to_char_type(current)) {
                case '[':
                    read_next();
                    return token_type::begin_flow_sequence;
                case ']':
                    read_next();
                    return token_type::end_flow_sequence;
                case '{':
                    read_next();
                    return token_type::begin_flow_mapping;
                case '}':
                    read_next();
                    return token_type::end_flow_mapping;
                case ',':
                    read_next();
                    return token_type::value_separator;
                case ':':
                    read_next();
                    return token_type::key_separator;
                case '?':
                    read_next();
                    return token_type::explicit_key;
                case '-':
                    return scan_dash_token();
                case '.':
                    return scan_document_end();
                case '#':
                    skip_comment();
                    return scan();
                case '&':
                    return scan_anchor();
                case '*':
                    return scan_alias();
                case '!':
                    return scan_tag();
                case '%':
                    return scan_directive();
                case '|':
                    block_parent_indent_ = current_line_indent_;
                    read_next();
                    return token_type::block_scalar_literal;
                case '>':
                    block_parent_indent_ = current_line_indent_;
                    read_next();
                    return token_type::block_scalar_folded;
                case '\"':
                case '\'':
                    return scan_quoted_scalar();
                case '~':
                    read_next();
                    return token_type::literal_null_tilde;
                default:
                    return scan_plain_scalar();
            }
        }

        token_type scan_dash_token() {
            read_next();
            if (is_yaml_space(current) || is_line_terminator(current) || current == char_traits::eof()) {
                return token_type::block_entry_indicator;
            }
            if (current == '-') {
                read_next();
                if (current == '-') {
                    read_next();
                    return token_type::document_start;
                }
                string_buffer_.clear();
                string_buffer_.push_back('-');
                string_buffer_.push_back('-');
                return scan_plain_scalar_continue();
            }
            string_buffer_.clear();
            string_buffer_.push_back('-');
            return scan_plain_scalar_continue();
        }

        token_type scan_document_end() {
            read_next();
            if (current == '.' && peek_next() == '.') {
                read_next();
                read_next();
                return token_type::document_end;
            }
            string_buffer_.clear();
            string_buffer_.push_back('.');
            return scan_plain_scalar_continue();
        }

        token_type scan_anchor() {
            read_next();
            anchor_name_.clear();
            while (is_anchor_char(current)) {
                anchor_name_.push_back(char_traits::to_char_type(current));
                read_next();
            }
            return token_type::anchor_indicator;
        }

        token_type scan_alias() {
            read_next();
            string_buffer_.clear();
            while (is_anchor_char(current)) {
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
            return token_type::alias_indicator;
        }

        token_type scan_tag() {
            read_next();
            string_buffer_.clear();
            if (current == '<') {
                read_next();
                while (current != '>' && current != char_traits::eof()) {
                    string_buffer_.push_back(char_traits::to_char_type(current));
                    read_next();
                }
                if (current == '>') {
                    read_next();
                }
            } else {
                while (is_tag_char(current)) {
                    string_buffer_.push_back(char_traits::to_char_type(current));
                    read_next();
                }
            }
            return token_type::tag_indicator;
        }

        token_type scan_directive() {
            read_next();
            string_buffer_.clear();
            while (!is_line_terminator(current) && current != char_traits::eof()) {
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
            return token_type::directive_indicator;
        }

        token_type scan_quoted_scalar() {
            const char_type quote = char_traits::to_char_type(current);
            read_next();
            string_buffer_.clear();

            while (current != char_traits::eof()) {
                if (char_traits::to_char_type(current) == quote) {
                    read_next();
                    if (quote == '\'' && current == '\'') {
                        string_buffer_.push_back('\'');
                        read_next();
                        continue;
                    }
                    return token_type::value_string;
                }
                if (quote == '\"' && current == '\\') {
                    read_next();
                    scan_escape_sequence();
                    continue;
                }
                if (is_line_terminator(current)) {
                    read_next();
                    while (is_yaml_space(current)) {
                        read_next();
                    }
                    string_buffer_.push_back(' ');
                    continue;
                }
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }

            exceptions::willow::yaml::throw_yaml_scalar_error("unexpected end in quoted scalar");
        }

        void scan_escape_sequence() {
            const char_int_type ch = current;
            switch (ch) {
                case char_traits::eof():
                    exceptions::willow::yaml::throw_yaml_scalar_error("unexpected end in escape sequence");
                case '0':
                    add_char('\0');
                    break;
                case 'a':
                    add_char('\a');
                    break;
                case 'b':
                    add_char('\b');
                    break;
                case 't':
                    add_char('\t');
                    break;
                case 'n':
                    add_char('\n');
                    break;
                case 'v':
                    add_char('\v');
                    break;
                case 'f':
                    add_char('\f');
                    break;
                case 'r':
                    add_char('\r');
                    break;
                case 'e':
                    add_char('\x1B');
                    break;
                case ' ':
                    add_char(' ');
                    break;
                case '\"':
                    add_char('\"');
                    break;
                case '/':
                    add_char('/');
                    break;
                case '\\':
                    add_char('\\');
                    break;
                case 'N':
                    add_code_point(0x85);
                    break;
                case '_':
                    add_code_point(0xA0);
                    break;
                case 'L':
                    add_code_point(0x2028);
                    break;
                case 'P':
                    add_code_point(0x2029);
                    break;
                case 'x': {
                    read_next();
                    add_code_point(read_hex_digits(2));
                    return;
                }
                case 'u': {
                    read_next();
                    const std::uint32_t lead = read_hex_digits(4);
                    if (unicode_surrogate_lead_begin <= lead && lead <= unicode_surrogate_lead_end) {
                        if (current != '\\') {
                            exceptions::willow::yaml::throw_yaml_scalar_error("lead surrogate must be followed by trail surrogate");
                        }
                        read_next();
                        if (current != 'u') {
                            exceptions::willow::yaml::throw_yaml_scalar_error("lead surrogate must be followed by trail surrogate");
                        }
                        read_next();
                        const std::uint32_t trail = read_hex_digits(4);
                        if (!(unicode_surrogate_trail_begin <= trail && trail <= unicode_surrogate_trail_end)) {
                            exceptions::willow::yaml::throw_yaml_scalar_error(
                                "surrogate U+D800...U+DBFF must be followed by U+DC00...U+DFFF");
                        }
                        unicode_writer<string_type> uw(string_buffer_);
                        uw.add_surrogates(lead, trail);
                    } else {
                        add_code_point(lead);
                    }
                    return;
                }
                case 'U': {
                    read_next();
                    add_code_point(read_hex_digits(8));
                    return;
                }
                default:
                    add_char(ch);
                    break;
            }
            read_next();
        }

        std::uint32_t read_hex_digits(const int count) {
            std::uint32_t code = 0;
            for (int i = 0; i < count; ++i) {
                const char_int_type ch = current;
                code <<= 4;
                if (ch >= '0' && ch <= '9') {
                    code |= static_cast<std::uint32_t>(ch - '0');
                } else if (ch >= 'a' && ch <= 'f') {
                    code |= static_cast<std::uint32_t>(ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    code |= static_cast<std::uint32_t>(ch - 'A' + 10);
                } else {
                    exceptions::willow::yaml::throw_yaml_scalar_error("escape sequence must be followed by hex digits");
                }
                read_next();
            }
            return code;
        }

        token_type scan_plain_scalar() {
            string_buffer_.clear();
            return scan_plain_scalar_continue();
        }

        token_type scan_plain_scalar_continue() {
            if constexpr (contiguous_adapter) {
                if (string_buffer_.empty() && can_block_scan() && scan_plain_integer_span()) {
                    return token_type::value_integer;
                }
            }

            std::size_t consumed = 0;
            while (current != char_traits::eof() && !is_line_terminator(current)) {
                if (in_flow() && (char_traits::to_char_type(current) == ',' ||
                                  char_traits::to_char_type(current) == ']' ||
                                  char_traits::to_char_type(current) == '}')) {
                    break;
                }
                if (char_traits::to_char_type(current) == ':' && is_key_boundary()) {
                    break;
                }
                if (char_traits::to_char_type(current) == '#' && is_comment_boundary()) {
                    break;
                }
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
                if constexpr (contiguous_adapter) {
                    if ((++consumed & (block_scan_threshold - 1)) == 0 && can_block_scan() && block_has_stop_free_run()) {
                        scan_plain_scalar_span();
                        break;
                    }
                }
            }

            return finish_plain_scalar();
        }

        bool scan_plain_integer_span() {
            const char_type *last = buffer_end();
            const char_type *scan = buffer_begin() + buffer_position() - 1;
            if (scan == last || *scan < char_type('0') || *scan > char_type('9')) {
                return false;
            }

            std::uint64_t magnitude = 0;
            bool overflow = false;
            while (scan != last && *scan >= char_type('0') && *scan <= char_type('9')) {
                const auto digit = static_cast<std::uint64_t>(*scan - char_type('0'));
                if (magnitude > (maximum_integer_magnitude() - digit) / 10) {
                    overflow = true;
                } else if (!overflow) {
                    magnitude = magnitude * 10 + digit;
                }
                ++scan;
            }

            if (overflow || !is_scalar_terminator_at(scan, last)) {
                return false;
            }

            integer_value_ = static_cast<integer_type>(magnitude);
            number_is_float_ = false;
            jump_to(scan);
            return true;
        }

        bool is_scalar_terminator_at(const char_type *position, const char_type *last) const noexcept {
            if (position == last) {
                return true;
            }
            const char_type ch = *position;
            if (is_line_terminator(char_traits::to_int_type(ch))) {
                return true;
            }
            if (ch == char_type(':')) {
                return key_boundary_at(position, last);
            }
            if (ch == char_type('#')) {
                return comment_boundary_at(position, last);
            }
            if (in_flow()) {
                return ch == char_type(',') || ch == char_type(']') || ch == char_type('}');
            }
            return false;
        }

        token_type finish_plain_scalar() {
            trim_trailing_spaces();

            if (string_buffer_.empty()) {
                return token_type::literal_null;
            }

            return classify_plain_scalar();
        }

        bool block_has_stop_free_run() const {
            constexpr core::simd_size_type width = simd_char_type::size();
            const char_type *last = buffer_end();
            const char_type *start = buffer_begin() + buffer_position() - 1;
            if (static_cast<std::size_t>(last - start) < static_cast<std::size_t>(width)) {
                return false;
            }
            const auto chunk = core::unchecked_load<simd_char_type>(start, width);
            auto stop = (chunk == simd_char('\n')) | (chunk == simd_char('\r')) | (chunk == simd_char(':')) |
                        (chunk == simd_char('#'));
            if (in_flow()) {
                stop = stop | (chunk == simd_char(',')) | (chunk == simd_char(']')) | (chunk == simd_char('}'));
            }
            return core::none_of(stop);
        }

        void scan_plain_scalar_span() {
            const char_type *last = buffer_end();

            string_buffer_.push_back(char_traits::to_char_type(current));

            const char_type *cursor = buffer_begin() + buffer_position();
            while (cursor < last) {
                const char_type *stop = find_plain_scalar_stop(cursor, last);
                if (stop != cursor) {
                    string_buffer_.append(cursor, static_cast<std::size_t>(stop - cursor));
                    cursor = stop;
                }
                if (cursor == last) {
                    break;
                }
                const char_type ch = *cursor;
                if (ch == ':' && !key_boundary_at(cursor, last)) {
                    string_buffer_.push_back(ch);
                    ++cursor;
                    continue;
                }
                if (ch == '#' && !comment_boundary_at(cursor, last)) {
                    string_buffer_.push_back(ch);
                    ++cursor;
                    continue;
                }
                break;
            }

            jump_to(cursor);
        }

        bool key_boundary_at(const char_type *position, const char_type *last) const noexcept {
            if (position + 1 >= last) {
                return true;
            }
            const auto next = char_traits::to_int_type(*(position + 1));
            return is_yaml_space(next) || is_line_terminator(next);
        }

        bool comment_boundary_at(const char_type *position, const char_type *last) const noexcept {
            return key_boundary_at(position, last);
        }

        bool is_key_boundary() {
            const auto next = peek_next();
            return is_yaml_space(next) || is_line_terminator(next) || next == char_traits::eof();
        }

        bool is_comment_boundary() {
            const auto next = peek_next();
            return is_yaml_space(next) || is_line_terminator(next) || next == char_traits::eof();
        }

        bool next_is_key_separator() {
            if (current == char_traits::eof() || char_traits::to_char_type(current) != char_type(':')) {
                return false;
            }
            if (in_flow()) {
                return true;
            }
            if constexpr (contiguous_adapter) {
                if (has_pushback_) {
                    return true;
                }
                const char_type *scan = buffer_begin() + buffer_position();
                const char_type *const last = buffer_end();
                if (scan == last) {
                    return true;
                }
                const auto after = char_traits::to_int_type(*scan);
                return is_yaml_space(after) || is_line_terminator(after);
            } else {
                const auto next = peek_next();
                return is_yaml_space(next) || is_line_terminator(next);
            }
        }

        void trim_trailing_spaces() {
            while (!string_buffer_.empty() && (string_buffer_.back() == ' ' || string_buffer_.back() == '\t')) {
                string_buffer_.pop_back();
            }
        }

        static plain_keyword match_plain_keyword(const string_type &s) {
            const auto length = s.size();
            if (length == 0) {
                return plain_keyword::null_value;
            }
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                switch (length) {
                    case 2:
                        if (equals_word(s, "on") || equals_word(s, "On") || equals_word(s, "ON")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, "no") || equals_word(s, "No") || equals_word(s, "NO")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 3:
                        if (equals_word(s, "yes") || equals_word(s, "Yes") || equals_word(s, "YES")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, "off") || equals_word(s, "Off") || equals_word(s, "OFF")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 4:
                        if (s[0] == 't' || s[0] == 'T') {
                            if (equals_word(s, "true") || equals_word(s, "True") || equals_word(s, "TRUE")) {
                                return plain_keyword::true_value;
                            }
                        } else if (s[0] == 'n' || s[0] == 'N') {
                            if (equals_word(s, "null") || equals_word(s, "Null") || equals_word(s, "NULL")) {
                                return plain_keyword::null_value;
                            }
                        }
                        break;
                    case 5:
                        if (equals_word(s, "false") || equals_word(s, "False") || equals_word(s, "FALSE")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                switch (length) {
                    case 2:
                        if (equals_word(s, L"on") || equals_word(s, L"On") || equals_word(s, L"ON")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, L"no") || equals_word(s, L"No") || equals_word(s, L"NO")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 3:
                        if (equals_word(s, L"yes") || equals_word(s, L"Yes") || equals_word(s, L"YES")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, L"off") || equals_word(s, L"Off") || equals_word(s, L"OFF")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 4:
                        if (equals_word(s, L"true") || equals_word(s, L"True") || equals_word(s, L"TRUE")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, L"null") || equals_word(s, L"Null") || equals_word(s, L"NULL")) {
                            return plain_keyword::null_value;
                        }
                        break;
                    case 5:
                        if (equals_word(s, L"false") || equals_word(s, L"False") || equals_word(s, L"FALSE")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                switch (length) {
                    case 2:
                        if (equals_word(s, u"on") || equals_word(s, u"On") || equals_word(s, u"ON")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, u"no") || equals_word(s, u"No") || equals_word(s, u"NO")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 3:
                        if (equals_word(s, u"yes") || equals_word(s, u"Yes") || equals_word(s, u"YES")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, u"off") || equals_word(s, u"Off") || equals_word(s, u"OFF")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 4:
                        if (equals_word(s, u"true") || equals_word(s, u"True") || equals_word(s, u"TRUE")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, u"null") || equals_word(s, u"Null") || equals_word(s, u"NULL")) {
                            return plain_keyword::null_value;
                        }
                        break;
                    case 5:
                        if (equals_word(s, u"false") || equals_word(s, u"False") || equals_word(s, u"FALSE")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                switch (length) {
                    case 2:
                        if (equals_word(s, U"on") || equals_word(s, U"On") || equals_word(s, U"ON")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, U"no") || equals_word(s, U"No") || equals_word(s, U"NO")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 3:
                        if (equals_word(s, U"yes") || equals_word(s, U"Yes") || equals_word(s, U"YES")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, U"off") || equals_word(s, U"Off") || equals_word(s, U"OFF")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    case 4:
                        if (equals_word(s, U"true") || equals_word(s, U"True") || equals_word(s, U"TRUE")) {
                            return plain_keyword::true_value;
                        }
                        if (equals_word(s, U"null") || equals_word(s, U"Null") || equals_word(s, U"NULL")) {
                            return plain_keyword::null_value;
                        }
                        break;
                    case 5:
                        if (equals_word(s, U"false") || equals_word(s, U"False") || equals_word(s, U"FALSE")) {
                            return plain_keyword::false_value;
                        }
                        break;
                    default:
                        break;
                }
            } else {
#if RAINY_HAS_CXX20
                if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                    switch (length) {
                        case 2:
                            if (equals_word(s, u8"on") || equals_word(s, u8"On") || equals_word(s, u8"ON")) {
                                return plain_keyword::true_value;
                            }
                            if (equals_word(s, u8"no") || equals_word(s, u8"No") || equals_word(s, u8"NO")) {
                                return plain_keyword::false_value;
                            }
                            break;
                        case 3:
                            if (equals_word(s, u8"yes") || equals_word(s, u8"Yes") || equals_word(s, u8"YES")) {
                                return plain_keyword::true_value;
                            }
                            if (equals_word(s, u8"off") || equals_word(s, u8"Off") || equals_word(s, u8"OFF")) {
                                return plain_keyword::false_value;
                            }
                            break;
                        case 4:
                            if (equals_word(s, u8"true") || equals_word(s, u8"True") || equals_word(s, u8"TRUE")) {
                                return plain_keyword::true_value;
                            }
                            if (equals_word(s, u8"null") || equals_word(s, u8"Null") || equals_word(s, u8"NULL")) {
                                return plain_keyword::null_value;
                            }
                            break;
                        case 5:
                            if (equals_word(s, u8"false") || equals_word(s, u8"False") || equals_word(s, u8"FALSE")) {
                                return plain_keyword::false_value;
                            }
                            break;
                        default:
                            break;
                    }
                }
#endif
            }
            return plain_keyword::none;
        }

        token_type classify_plain_scalar() {
            switch (match_plain_keyword(string_buffer_)) {
                case plain_keyword::true_value:
                    return token_type::literal_true;
                case plain_keyword::false_value:
                    return token_type::literal_false;
                case plain_keyword::null_value:
                    return token_type::literal_null;
                default:
                    break;
            }
            if (scan_plain_number()) {
                return number_is_float_ ? token_type::value_float : token_type::value_integer;
            }
            return token_type::value_string;
        }

        bool scan_plain_number() {
            const auto &s = string_buffer_;
            if (s.empty()) {
                return false;
            }

            std::size_t idx = 0;
            bool negative = false;
            if (s[idx] == '-' || s[idx] == '+') {
                negative = s[idx] == '-';
                ++idx;
                if (idx >= s.size()) {
                    return false;
                }
            }

            if (idx < s.size() && s[idx] == '0' && idx + 1 < s.size() && (s[idx + 1] == 'x' || s[idx + 1] == 'X')) {
                idx += 2;
                bool has_digits = false;
                std::uint64_t value = 0;
                while (idx < s.size() && is_hex_digit(static_cast<char_int_type>(s[idx]))) {
                    value = value * 16 + hex_value(s[idx]);
                    has_digits = true;
                    ++idx;
                }
                if (!has_digits || idx != s.size()) {
                    return false;
                }
                integer_value_ = negative ? -static_cast<integer_type>(value) : static_cast<integer_type>(value);
                number_is_float_ = false;
                return true;
            }

            if (idx < s.size() && s[idx] == '0' && idx + 1 < s.size() && s[idx + 1] == 'o') {
                idx += 2;
                bool has_digits = false;
                std::uint64_t value = 0;
                while (idx < s.size() && is_oct_digit(static_cast<char_int_type>(s[idx]))) {
                    value = value * 8 + static_cast<std::uint64_t>(s[idx] - '0');
                    has_digits = true;
                    ++idx;
                }
                if (!has_digits || idx != s.size()) {
                    return false;
                }
                integer_value_ = negative ? -static_cast<integer_type>(value) : static_cast<integer_type>(value);
                number_is_float_ = false;
                return true;
            }

            {
                bool plain_digits = idx < s.size();
                std::uint64_t magnitude = 0;
                bool overflow = false;
                for (std::size_t i = idx; plain_digits && i < s.size(); ++i) {
                    const char_type c = s[i];
                    if (c < char_type('0') || c > char_type('9')) {
                        plain_digits = false;
                        break;
                    }
                    const auto digit = static_cast<std::uint64_t>(c - char_type('0'));
                    if (magnitude > (maximum_integer_magnitude() - digit) / 10) {
                        overflow = true;
                    } else if (!overflow) {
                        magnitude = magnitude * 10 + digit;
                    }
                }
                if (plain_digits && !overflow) {
                    integer_value_ = negative ? -static_cast<integer_type>(magnitude) : static_cast<integer_type>(magnitude);
                    number_is_float_ = false;
                    return true;
                }
            }

            bool has_dot = false;
            bool has_exp = false;
            std::size_t digit_start = idx;
            for (; idx < s.size(); ++idx) {
                const char_type c = s[idx];
                if (c >= '0' && c <= '9') {
                    continue;
                }
                if (c == '_') {
                    continue;
                }
                if (c == '.' && !has_dot && !has_exp) {
                    has_dot = true;
                    continue;
                }
                if ((c == 'e' || c == 'E') && !has_exp) {
                    has_exp = true;
                    if (idx + 1 < s.size() && (s[idx + 1] == '+' || s[idx + 1] == '-')) {
                        ++idx;
                    }
                    continue;
                }
                return false;
            }

            if (digit_start == s.size()) {
                return false;
            }

            core::collections::vector<char> cleaned;
            const char *begin = nullptr;
            const char *end = nullptr;
            if (s.find(char_type('_')) == string_type::npos) {
                begin = reinterpret_cast<const char *>(s.data());
                end = begin + s.size();
            } else {
                cleaned.reserve(s.size());
                for (const char_type c: s) {
                    if (c != '_') {
                        cleaned.push_back(static_cast<char>(c));
                    }
                }
                begin = cleaned.data();
                end = begin + cleaned.size();
            }

            if (has_dot || has_exp) {
                float_type fv{};
                const auto result = core::text::from_chars(begin, end, fv);
                if (result.ec != std::errc{} && result.ec != std::errc::result_out_of_range) {
                    return false;
                }
                number_value_ = fv;
                number_is_float_ = true;
                return true;
            }

            integer_type iv{};
            const auto result = core::text::from_chars(begin, end, iv);
            if (result.ec == std::errc{}) {
                integer_value_ = iv;
                number_is_float_ = false;
                return true;
            }

            float_type fv{};
            const auto fresult = core::text::from_chars(begin, end, fv);
            if (fresult.ec == std::errc{} || fresult.ec == std::errc::result_out_of_range) {
                number_value_ = fv;
                number_is_float_ = true;
                return true;
            }
            return false;
        }

        static constexpr std::uint64_t maximum_integer_magnitude() noexcept {
            return static_cast<std::uint64_t>(utility::numeric_limits<integer_type>::max());
        }

        static std::uint64_t hex_value(const char c) {
            if (c >= '0' && c <= '9') {
                return static_cast<std::uint64_t>(c - '0');
            }
            if (c >= 'a' && c <= 'f') {
                return static_cast<std::uint64_t>(c - 'a' + 10);
            }
            return static_cast<std::uint64_t>(c - 'A' + 10);
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

        string_type token_to_anchor() const {
            return anchor_name_;
        }

        void add_char(const char_int_type ch) {
            string_buffer_.push_back(char_traits::to_char_type(ch));
        }

        void add_code_point(const std::uint32_t code) {
            unicode_writer<string_type> uw(string_buffer_);
            uw.add_code(code);
        }

    private:
        integer_type integer_value_{};
        float_type number_value_{};
        bool number_is_float_{false};
        string_type anchor_name_;
        string_type string_buffer_;
        Adapter adapter_;
        char_int_type current;
        char_int_type pushback_{};
        bool has_pushback_{false};
        int flow_depth_{0};
        int current_line_indent_{0};
        int block_parent_indent_{0};
    };
}

#endif
