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
#ifndef RAINY_FOUNDATION_WILLOW_YAML_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_YAML_SERIALIZER_HPP
#include <iomanip>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/yaml/config.hpp>
#include <rainy/foundation/willow/implements/yaml/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::yaml::implements {
    using namespace rainy::foundation::exceptions::willow;

    enum class yaml_flow_style {
        block,
        flow,
        auto_detect
    };

    template <typename BasicDocument>
    struct yaml_serializer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using args = serializer_args<BasicDocument>;
        using node_type = facade<BasicDocument>;

        static constexpr std::size_t flush_threshold = 4096;

        yaml_serializer(output_adapter<char_type> adapter, const args &args) :
            out_(utility::move(adapter)), arg_(args), indent_char_(args.indent_char), indent_width_(args.indent > 0 ? args.indent : 2) {
            string_buffer_.reserve(256);
            out_buffer_.reserve(flush_threshold);
            if (arg_.indent > 0) {
                indent_string_.assign(256, indent_char_);
            }
        }

        void dump(const node_type &yaml) {
            dump_node(yaml, 0, false);
            terminate_line();
            flush_buffer();
        }

    private:
        void terminate_line() {
            if (last_char_ != to_char_type('\n')) {
                put(to_char_type('\n'));
            }
        }

        void flush_buffer() {
            if (!out_buffer_.empty()) {
                out_->write(out_buffer_.data(), out_buffer_.size());
                out_buffer_.clear();
            }
        }

        void put(const char_type ch) {
            last_char_ = ch;
            out_buffer_.push_back(ch);
            if (out_buffer_.size() >= flush_threshold) {
                flush_buffer();
            }
        }

        void put(const char_type *str, const std::size_t size) {
            if (size == 0) {
                return;
            }
            last_char_ = str[size - 1];
            if (size >= flush_threshold) {
                flush_buffer();
                out_->write(str, size);
                return;
            }
            if (out_buffer_.size() + size > flush_threshold) {
                flush_buffer();
            }
            out_buffer_.insert(out_buffer_.end(), str, str + size);
        }

        template <typename Node>
        yaml_flow_style get_style(const Node &doc) const {
            if (style_is_flow(doc)) {
                return yaml_flow_style::flow;
            }
            if (has_explicit_block_style(doc)) {
                return yaml_flow_style::block;
            }
            if (arg_.indent > 0) {
                return yaml_flow_style::block;
            }
            return yaml_flow_style::flow;
        }

        template <typename Node>
        bool style_is_flow(const Node &doc) const {
            return style_key_is(doc, "flow", 4);
        }

        template <typename Node>
        bool has_explicit_block_style(const Node &doc) const {
            return style_key_is(doc, "block", 5);
        }

        template <typename Node>
        bool style_key_is(const Node &doc, const char *const narrow, const std::size_t length) const {
            if (!doc.has_style()) {
                return false;
            }
            const auto &name = doc.style();
            if (name.size() != length) {
                return false;
            }
            for (std::size_t i = 0; i < length; ++i) {
                if (static_cast<char>(name[i]) != narrow[i]) {
                    return false;
                }
            }
            return true;
        }

        template <typename Node>
        bool has_yaml_anchor(const Node &doc) const {
            return doc.has_anchor();
        }

        template <typename Node>
        bool has_yaml_alias(const Node &doc) const {
            return doc.has_alias();
        }

        template <typename Node>
        bool has_yaml_tag(const Node &doc) const {
            return doc.has_tag();
        }

        template <typename Node>
        bool has_node_prefix(const Node &yaml) const {
            return !has_yaml_alias(yaml) && (has_yaml_tag(yaml) || has_yaml_anchor(yaml));
        }

        template <typename Node>
        void write_node_prefix(const Node &yaml, const bool trailing_space = true) {
            if (has_yaml_tag(yaml)) {
                put(to_char_type('!'));
                const auto &tag = yaml.tag();
                put(tag.data(), tag.size());
                if (trailing_space) {
                    put(to_char_type(' '));
                }
            }
            if (has_yaml_anchor(yaml)) {
                put(to_char_type('&'));
                const auto &anchor = yaml.anchor();
                put(anchor.data(), anchor.size());
                if (trailing_space) {
                    put(to_char_type(' '));
                }
            }
        }

        template <typename Node>
        void dump_node(const Node &yaml, const unsigned int current_indent, const bool in_sequence,
                       const bool emit_prefix = true) {
            if (has_yaml_alias(yaml)) {
                put(to_char_type('*'));
                const auto &alias = yaml.alias();
                put(alias.data(), alias.size());
                if (!in_sequence) {
                    put(to_char_type('\n'));
                }
                return;
            }

            if (emit_prefix) {
                write_node_prefix(yaml);
            }

            switch (yaml.type()) {
                case document_type::object:
                    dump_object(yaml, current_indent, in_sequence);
                    break;
                case document_type::array:
                    dump_array(yaml, current_indent, in_sequence);
                    break;
                case document_type::string:
                    dump_string(yaml.as_string(), current_indent);
                    break;
                case document_type::boolean:
                    dump_boolean(yaml.as_bool());
                    break;
                case document_type::number_integer:
                    dump_integer(yaml.as_integer());
                    break;
                case document_type::number_float:
                    dump_float(yaml.as_float());
                    break;
                case document_type::null:
                    dump_null();
                    break;
            }
        }

        template <typename Node>
        void dump_object(const Node &yaml, const unsigned int current_indent, const bool in_sequence) {
            const auto &object = yaml.as_object();
            if (object.empty()) {
                write_indent(current_indent);
                put(to_char_type('{'));
                put(to_char_type('}'));
                if (!in_sequence) {
                    put(to_char_type('\n'));
                }
                return;
            }

            const auto style = get_style(yaml);
            if (style == yaml_flow_style::flow) {
                if (!in_sequence) {
                    write_indent(current_indent);
                }
                dump_flow_mapping(yaml);
                if (!in_sequence) {
                    put(to_char_type('\n'));
                }
                return;
            }

            for (const auto &entry: object) {
                const auto &key = entry.first;
                const node_type &value = entry.second;
                write_indent(current_indent);
                write_key(key);
                put(to_char_type(':'));
                if (needs_inline_value(value)) {
                    put(to_char_type(' '));
                    dump_node(value, current_indent, false);
                    if (!value.is_string() && !has_yaml_alias(value)) {
                        put(to_char_type('\n'));
                    }
                } else {
                    if (has_node_prefix(value)) {
                        put(to_char_type(' '));
                        write_node_prefix(value, false);
                    }
                    put(to_char_type('\n'));
                    dump_node(value, current_indent + 1, false, false);
                }
            }
        }

        template <typename Node>
        void dump_flow_mapping(const Node &yaml) {
            const auto &object = yaml.as_object();
            put(to_char_type('{'));
            auto iter = object.cbegin();
            for (std::size_t i = 0; i < object.size(); ++i, ++iter) {
                if (i > 0) {
                    put(to_char_type(','));
                    put(to_char_type(' '));
                }
                write_flow_key(iter->first);
                put(to_char_type(':'));
                put(to_char_type(' '));
                dump_flow_node(iter->second);
            }
            put(to_char_type('}'));
        }

        template <typename Node>
        void dump_array(const Node &yaml, const unsigned int current_indent, const bool in_sequence) {
            const auto &array = yaml.as_array();
            if (array.empty()) {
                write_indent(current_indent);
                put(to_char_type('['));
                put(to_char_type(']'));
                if (!in_sequence) {
                    put(to_char_type('\n'));
                }
                return;
            }

            const auto style = get_style(yaml);
            if (style == yaml_flow_style::flow) {
                if (!in_sequence) {
                    write_indent(current_indent);
                }
                dump_flow_sequence(yaml);
                if (!in_sequence) {
                    put(to_char_type('\n'));
                }
                return;
            }

            for (const auto &raw_item: array) {
                const node_type &item = raw_item;
                write_indent(current_indent);
                put(to_char_type('-'));
                if (is_nested_container(item) && !has_yaml_alias(item) && get_style(item) == yaml_flow_style::flow) {
                    put(to_char_type(' '));
                    dump_node(item, current_indent + 1, true);
                    put(to_char_type('\n'));
                } else if (is_nested_container(item) && !has_yaml_alias(item)) {
                    if (has_node_prefix(item)) {
                        put(to_char_type(' '));
                        write_node_prefix(item, false);
                    }
                    put(to_char_type('\n'));
                    dump_node(item, current_indent + 1, false, false);
                } else {
                    put(to_char_type(' '));
                    dump_node(item, current_indent + 1, true);
                    if (!item.is_string() || has_yaml_alias(item)) {
                        put(to_char_type('\n'));
                    }
                }
            }
        }

        template <typename Node>
        void dump_flow_sequence(const Node &yaml) {
            const auto &array = yaml.as_array();
            put(to_char_type('['));
            for (std::size_t i = 0; i < array.size(); ++i) {
                if (i > 0) {
                    put(to_char_type(','));
                    put(to_char_type(' '));
                }
                dump_flow_node(array[i]);
            }
            put(to_char_type(']'));
        }

        template <typename Node>
        void dump_flow_node(const Node &yaml) {
            if (has_yaml_alias(yaml)) {
                put(to_char_type('*'));
                const auto &alias = yaml.alias();
                put(alias.data(), alias.size());
                return;
            }
            write_node_prefix(yaml);
            switch (yaml.type()) {
                case document_type::object:
                    dump_flow_mapping(yaml);
                    break;
                case document_type::array:
                    dump_flow_sequence(yaml);
                    break;
                case document_type::string:
                    dump_flow_string(yaml.as_string());
                    break;
                case document_type::boolean:
                    dump_boolean(yaml.as_bool());
                    break;
                case document_type::number_integer:
                    dump_integer(yaml.as_integer());
                    break;
                case document_type::number_float:
                    dump_float(yaml.as_float());
                    break;
                case document_type::null:
                    dump_null();
                    break;
            }
        }

        template <typename Node>
        bool is_nested_container(const Node &doc) const {
            return doc.is_object() || doc.is_array();
        }

        template <typename Node>
        bool needs_inline_value(const Node &doc) const {
            if (has_yaml_alias(doc)) {
                return true;
            }
            switch (doc.type()) {
                case document_type::object:
                case document_type::array:
                    return false;
                default:
                    return true;
            }
        }

        void write_indent(unsigned int indent_level) {
            const unsigned int spaces = indent_level * indent_width_;
            if (spaces > 0 && spaces <= indent_string_.size()) {
                put(indent_string_.data(), spaces);
            } else if (spaces > indent_string_.size()) {
                indent_string_.assign(spaces, indent_char_);
                put(indent_string_.data(), spaces);
            }
        }

        void write_key(const string_type &key) {
            write_string_scalar(key);
        }

        void write_flow_key(const string_type &key) {
            write_string_scalar(key);
        }

        bool is_plain_safe(const string_type &str) const {
            if (str.empty()) {
                return false;
            }
            if (is_reserved_word(str)) {
                return false;
            }
            const char_type first = str.front();
            if (is_indicator_char(first)) {
                return false;
            }
            if (first == ' ' || first == '\t') {
                return false;
            }
            if (str.back() == ' ' || str.back() == '\t') {
                return false;
            }
            for (const auto &ch: str) {
                if (ch == '\n' || ch == '\r') {
                    return false;
                }
                if (ch == ':' || ch == '#' || ch == ',' || ch == '[' || ch == ']' || ch == '{' || ch == '}') {
                    return false;
                }
            }
            return true;
        }

        bool is_indicator_char(const char_type ch) const {
            switch (ch) {
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
                case '"':
                case '%':
                case '@':
                case '`':
                    return true;
                default:
                    return false;
            }
        }

        bool is_reserved_word(const string_type &str) const {
            const auto length = str.size();
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                switch (length) {
                    case 1:
                        return str == "~";
                    case 2:
                        return str == "no" || str == "on";
                    case 3:
                        return str == "yes" || str == "off";
                    case 4:
                        return str == "true" || str == "True" || str == "TRUE" || str == "null" || str == "Null" || str == "NULL";
                    case 5:
                        return str == "false" || str == "False" || str == "FALSE";
                    default:
                        return false;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                switch (length) {
                    case 1:
                        return str == L"~";
                    case 2:
                        return str == L"no" || str == L"on";
                    case 3:
                        return str == L"yes" || str == L"off";
                    case 4:
                        return str == L"true" || str == L"True" || str == L"TRUE" || str == L"null" || str == L"Null" ||
                               str == L"NULL";
                    case 5:
                        return str == L"false" || str == L"False" || str == L"FALSE";
                    default:
                        return false;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                switch (length) {
                    case 1:
                        return str == u"~";
                    case 2:
                        return str == u"no" || str == u"on";
                    case 3:
                        return str == u"yes" || str == u"off";
                    case 4:
                        return str == u"true" || str == u"True" || str == u"TRUE" || str == u"null" || str == u"Null" ||
                               str == u"NULL";
                    case 5:
                        return str == u"false" || str == u"False" || str == u"FALSE";
                    default:
                        return false;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                switch (length) {
                    case 1:
                        return str == U"~";
                    case 2:
                        return str == U"no" || str == U"on";
                    case 3:
                        return str == U"yes" || str == U"off";
                    case 4:
                        return str == U"true" || str == U"True" || str == U"TRUE" || str == U"null" || str == U"Null" ||
                               str == U"NULL";
                    case 5:
                        return str == U"false" || str == U"False" || str == U"FALSE";
                    default:
                        return false;
                }
            } else {
#if RAINY_HAS_CXX20
                if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                    switch (length) {
                        case 1:
                            return str == u8"~";
                        case 2:
                            return str == u8"no" || str == u8"on";
                        case 3:
                            return str == u8"yes" || str == u8"off";
                        case 4:
                            return str == u8"true" || str == u8"True" || str == u8"TRUE" || str == u8"null" ||
                                   str == u8"Null" || str == u8"NULL";
                        case 5:
                            return str == u8"false" || str == u8"False" || str == u8"FALSE";
                        default:
                            return false;
                    }
                }
#endif
                return false;
            }
        }

        bool is_number_like(const string_type &str) const {
            if (str.empty()) {
                return false;
            }
            string_type cleaned;
            cleaned.reserve(str.size());
            for (const auto &ch: str) {
                if (ch == '_') {
                    continue;
                }
                if (ch == '+' || ch == '-') {
                    if (!cleaned.empty()) {
                        return false;
                    }
                    cleaned.push_back(ch);
                    continue;
                }
                cleaned.push_back(ch);
            }
            if (cleaned.empty()) {
                return false;
            }
            const bool signed_value = cleaned[0] == '-' || cleaned[0] == '+';
            std::size_t i = signed_value ? 1 : 0;
            if (cleaned.size() > i + 2 && cleaned[i] == '0' && (cleaned[i + 1] == 'x' || cleaned[i + 1] == 'X')) {
                for (std::size_t k = i + 2; k < cleaned.size(); ++k) {
                    const auto c = cleaned[k];
                    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                        return false;
                    }
                }
                return true;
            }
            if (cleaned.size() > i + 2 && cleaned[i] == '0' && cleaned[i + 1] == 'o') {
                for (std::size_t k = i + 2; k < cleaned.size(); ++k) {
                    const auto c = cleaned[k];
                    if (c < '0' || c > '7') {
                        return false;
                    }
                }
                return true;
            }
            bool has_digit = false;
            bool has_dot = false;
            for (; i < cleaned.size(); ++i) {
                const auto c = cleaned[i];
                if (c >= '0' && c <= '9') {
                    has_digit = true;
                } else if (c == '.') {
                    has_dot = true;
                } else if (c == 'e' || c == 'E') {
                    return has_digit;
                } else {
                    return false;
                }
            }
            return has_digit || has_dot;
        }

        bool needs_quotes(const string_type &str) const {
            if (arg_.escape_unicode) {
                for (const auto &ch: str) {
                    if (static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(ch)) >= 0x7F) {
                        return true;
                    }
                }
            }
            if (!is_plain_safe(str)) {
                return true;
            }
            if (is_number_like(str)) {
                return true;
            }
            return false;
        }

        void write_string_scalar(const string_type &str) {
            if (needs_quotes(str)) {
                dump_quoted_string(str);
            } else {
                write_raw_string(str);
            }
        }

        void write_raw_string(const string_type &str) {
            for (const auto &ch: str) {
                string_buffer_.push_back(ch);
            }
            if (!string_buffer_.empty()) {
                put(string_buffer_.data(), string_buffer_.size());
                string_buffer_.clear();
            }
        }

        void dump_string(const string_type &str, const unsigned int current_indent) {
            if (str.find(to_char_type('\n')) != string_type::npos) {
                dump_block_scalar(str, current_indent);
                return;
            }
            if (needs_quotes(str)) {
                dump_quoted_string(str);
            } else {
                write_raw_string(str);
            }
            put(to_char_type('\n'));
        }

        void dump_flow_string(const string_type &str) {
            if (needs_quotes(str)) {
                dump_quoted_string(str);
            } else {
                write_raw_string(str);
            }
        }

        void dump_block_scalar(const string_type &str, const unsigned int current_indent) {
            std::size_t trailing = 0;
            while (trailing < str.size() && str[str.size() - 1 - trailing] == to_char_type('\n')) {
                ++trailing;
            }
            put(to_char_type('|'));
            if (trailing == 0) {
                put(to_char_type('-'));
            } else if (trailing >= 2) {
                put(to_char_type('+'));
            }
            put(to_char_type('\n'));
            const std::size_t body_end = str.size() - (trailing > 0 ? 1 : 0);
            std::size_t pos = 0;
            while (pos <= body_end) {
                const auto end = str.find(to_char_type('\n'), pos);
                write_indent(current_indent + 1);
                if (end == string_type::npos || end >= body_end) {
                    put(str.data() + pos, body_end - pos);
                    put(to_char_type('\n'));
                    break;
                }
                put(str.data() + pos, end - pos);
                put(to_char_type('\n'));
                pos = end + 1;
            }
        }

        void dump_quoted_string(const string_type &str) {
            put(to_char_type('"'));
            string_buffer_.clear();
            for (std::size_t index = 0; index < str.size(); ++index) {
                const auto unit =
                    static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(str[index]));
                if constexpr (is_byte_char_type()) {
                    if (arg_.escape_unicode && unit >= 0x7F) {
                        std::uint32_t codepoint = 0;
                        if (decode_utf8(str, index, codepoint)) {
                            escape_unicode(codepoint);
                            flush_quoted_buffer();
                            continue;
                        }
                    }
                }
                switch (unit) {
                    case '\t':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('t'));
                        break;
                    case '\r':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('r'));
                        break;
                    case '\n':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('n'));
                        break;
                    case '\b':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('b'));
                        break;
                    case '\f':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('f'));
                        break;
                    case '"':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('"'));
                        break;
                    case '\\':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('\\'));
                        break;
                    default:
                        if (unit <= 0x1F || (arg_.escape_unicode && unit >= 0x7F)) {
                            escape_unicode(unit);
                        } else {
                            string_buffer_.push_back(static_cast<char_type>(unit));
                        }
                        break;
                }
                flush_quoted_buffer();
            }
            if (!string_buffer_.empty()) {
                put(string_buffer_.data(), string_buffer_.size());
                string_buffer_.clear();
            }
            put(to_char_type('"'));
        }

        static constexpr bool is_byte_char_type() {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                return true;
            } else {
#if RAINY_HAS_CXX20
                return type_traits::type_relations::is_same_v<char_type, char8_t>;
#else
                return false;
#endif
            }
        }

        static bool decode_utf8(const string_type &str, std::size_t &index, std::uint32_t &codepoint) {
            const auto lead =
                static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(str[index]));
            std::size_t continuation = 0;
            if ((lead & 0xE0) == 0xC0) {
                codepoint = lead & 0x1F;
                continuation = 1;
            } else if ((lead & 0xF0) == 0xE0) {
                codepoint = lead & 0x0F;
                continuation = 2;
            } else if ((lead & 0xF8) == 0xF0) {
                codepoint = lead & 0x07;
                continuation = 3;
            } else {
                return false;
            }
            if (index + continuation >= str.size()) {
                return false;
            }
            for (std::size_t offset = 1; offset <= continuation; ++offset) {
                const auto next =
                    static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(str[index + offset]));
                if ((next & 0xC0) != 0x80) {
                    return false;
                }
                codepoint = (codepoint << 6) | (next & 0x3F);
            }
            index += continuation;
            return true;
        }

        void flush_quoted_buffer() {
            if (string_buffer_.size() > 400) {
                put(string_buffer_.data(), string_buffer_.size());
                string_buffer_.clear();
            }
        }

        void escape_unicode(std::uint32_t codepoint) {
            if (codepoint <= 0xFFFF) {
                string_buffer_.push_back(to_char_type('\\'));
                string_buffer_.push_back(to_char_type('u'));
                append_hex(static_cast<std::uint16_t>(codepoint), 4);
            } else {
                codepoint -= 0x10000;
                const auto high = static_cast<std::uint16_t>(0xD800 + (codepoint >> 10));
                const auto low = static_cast<std::uint16_t>(0xDC00 + (codepoint & 0x3FF));
                string_buffer_.push_back(to_char_type('\\'));
                string_buffer_.push_back(to_char_type('u'));
                append_hex(high, 4);
                string_buffer_.push_back(to_char_type('\\'));
                string_buffer_.push_back(to_char_type('u'));
                append_hex(low, 4);
            }
        }

        void append_hex(std::uint16_t value, int width) {
            static const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
            for (int i = width - 1; i >= 0; --i) {
                string_buffer_.push_back(to_char_type(hex_chars[(value >> (4 * i)) & 0xF]));
            }
        }

        void dump_boolean(boolean_type value) {
            if (value) {
                write_literal<'t', 'r', 'u', 'e'>();
            } else {
                write_literal<'f', 'a', 'l', 's', 'e'>();
            }
        }

        void dump_null() {
            write_literal<'n', 'u', 'l', 'l'>();
        }

        void dump_integer(integer_type value) {
            if (value == 0) {
                put(to_char_type('0'));
                return;
            }
            const bool negative = value < 0;
            auto magnitude = static_cast<std::uint64_t>(value);
            if (negative) {
                magnitude = ~magnitude + 1;
            }
            char_type buffer[32]{};
            std::size_t pos = sizeof(buffer) / sizeof(buffer[0]);
            while (magnitude > 0) {
                buffer[--pos] = to_char_type(static_cast<char_type>('0' + magnitude % 10));
                magnitude /= 10;
            }
            if (negative) {
                buffer[--pos] = to_char_type('-');
            }
            put(buffer + pos, sizeof(buffer) / sizeof(buffer[0]) - pos);
        }

        void dump_float(float_type value) {
            if (!std::isfinite(value)) {
                if (value != value) {
                    write_literal<'.', 'N', 'a', 'N'>();
                } else if (value > 0) {
                    write_literal<'.', 'i', 'n', 'f'>();
                } else {
                    write_literal<'-', '.', 'i', 'n', 'f'>();
                }
                return;
            }
            char buffer[64]{};
            const core::text::to_chars_result result =
                core::text::to_chars(buffer, buffer + sizeof(buffer), value, core::text::chars_format::general, arg_.precision);
            if (result.ec != std::errc{}) {
                dump_null();
                return;
            }
            const std::size_t length = static_cast<std::size_t>(result.ptr - buffer);
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                put(buffer, length);
            } else {
                char_type text[64];
                for (std::size_t i = 0; i < length; ++i) {
                    text[i] = static_cast<char_type>(buffer[i]);
                }
                put(text, length);
            }
        }

        template <char_type... ch>
        void write_literal() {
            static constexpr core::collections::array<char_type, sizeof...(ch)> literal = {ch...};
            put(std::data(literal), sizeof...(ch));
        }

        static char_type to_char_type(char_type c) {
            return char_traits::to_char_type(static_cast<char_int_type>(c));
        }

    private:
        output_adapter<char_type> out_;
        const args &arg_;
        char_type indent_char_;
        unsigned int indent_width_;
        string_type indent_string_;
        core::collections::vector<char_type> string_buffer_;
        core::collections::vector<char_type> out_buffer_;
        char_type last_char_{char_type('\n')};
    };
}

#endif
