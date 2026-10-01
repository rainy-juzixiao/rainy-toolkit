/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_WILLOW_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_SERIALIZER_HPP
#include <iomanip>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/json/config.hpp>
#include <rainy/foundation/willow/implements/json/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::json::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument>
    struct json_serializer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using args = serializer_args<BasicDocument>;

        static constexpr std::size_t flush_threshold = 4096;

        json_serializer(output_adapter<char_type> adapter, const args &args) :
            out_(utility::move(adapter)), arg_(args), indent_char_(args.indent_char) {
            out_buffer_.reserve(flush_threshold);
            if (arg_.indent > 0) {
                indent_string_.assign(256, indent_char_);
            }
        }

        void dump(const BasicDocument &json) {
            dump_value(json, 0);
            flush_buffer();
        }

    private:
        void put(const char_type ch) {
            out_buffer_.push_back(ch);
            if (out_buffer_.size() >= flush_threshold) {
                flush_buffer();
            }
        }

        void put(const char_type *str, const std::size_t size) {
            if (size == 0) {
                return;
            }
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

        void flush_buffer() {
            if (!out_buffer_.empty()) {
                out_->write(out_buffer_.data(), out_buffer_.size());
                out_buffer_.clear();
            }
        }

        template <typename Node>
        void dump_value(const Node &json, const unsigned int current_indent) {
            switch (json.type()) {
                case document_type::object: {
                    dump_object(json, current_indent);
                    break;
                }
                case document_type::array: {
                    dump_array(json, current_indent);
                    break;
                }
                case document_type::string: {
                    dump_string(json.as_string());
                    break;
                }
                case document_type::boolean: {
                    dump_boolean(json.as_bool());
                    break;
                }
                case document_type::number_integer: {
                    dump_integer(json.as_integer());
                    break;
                }
                case document_type::number_float: {
                    dump_float(json.as_float());
                    break;
                }
                case document_type::null: {
                    dump_null();
                    break;
                }
            }
        }

        template <typename Node>
        void dump_object(const Node &json, const unsigned int current_indent) {
            const auto &object = json.as_object();
            if (object.empty()) {
                write_literal<'{', '}'>();
                return;
            }
            const bool pretty_print = arg_.indent > 0;
            const unsigned int new_indent = current_indent + arg_.indent;
            put(to_char_type('{'));
            if (pretty_print) {
                put(to_char_type('\n'));
            }
            auto iter = object.cbegin();
            const auto size = object.size();
            for (std::size_t i = 0; i < size; ++i, ++iter) {
                if (pretty_print) {
                    write_indent(new_indent);
                }
                put(to_char_type('"'));
                dump_escaped_string(iter->first);
                put(to_char_type('"'));
                put(to_char_type(':'));
                if (pretty_print) {
                    put(to_char_type(' '));
                }
                dump_value(iter->second, new_indent);
                if (i != size - 1) {
                    put(to_char_type(','));
                    if (pretty_print) {
                        put(to_char_type('\n'));
                    }
                }
            }
            if (pretty_print) {
                put(to_char_type('\n'));
                write_indent(current_indent);
            }
            put(to_char_type('}'));
        }

        template <typename Node>
        void dump_array(const Node &json, const unsigned int current_indent) {
            const auto &array = json.as_array();
            if (array.empty()) {
                write_literal<'[', ']'>();
                return;
            }
            const bool pretty_print = arg_.indent > 0;
            const unsigned int new_indent = current_indent + arg_.indent;
            put(to_char_type('['));
            if (pretty_print) {
                put(to_char_type('\n'));
            }
            const auto size = array.size();
            for (std::size_t i = 0; i < size; ++i) {
                if (pretty_print) {
                    write_indent(new_indent);
                }
                dump_value(array[i], new_indent);
                if (i != size - 1) {
                    put(to_char_type(','));
                    if (pretty_print) {
                        put(to_char_type('\n'));
                    }
                }
            }
            if (pretty_print) {
                put(to_char_type('\n'));
                write_indent(current_indent);
            }
            put(to_char_type(']'));
        }

        void dump_string(const string_type &str) {
            put(to_char_type('"'));
            dump_escaped_string(str);
            put(to_char_type('"'));
        }

        void dump_escaped_string(const string_type &str) {
            const char_type *data = str.data();
            const std::size_t size = str.size();
            std::size_t run = 0;
            std::size_t i = 0;
            while (i < size) {
                const auto ch = data[i];
                const auto unit = static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(ch));
                const bool clean = unit > 0x1F && unit != static_cast<std::uint32_t>('"') &&
                                   unit != static_cast<std::uint32_t>('\\') &&
                                   !(arg_.escape_unicode && unit >= 0x7F);
                if (clean) {
                    ++i;
                    continue;
                }
                put(data + run, i - run);
                switch (unit) {
                    case '\t':
                        put(to_char_type('\\'));
                        put(to_char_type('t'));
                        break;
                    case '\r':
                        put(to_char_type('\\'));
                        put(to_char_type('r'));
                        break;
                    case '\n':
                        put(to_char_type('\\'));
                        put(to_char_type('n'));
                        break;
                    case '\b':
                        put(to_char_type('\\'));
                        put(to_char_type('b'));
                        break;
                    case '\f':
                        put(to_char_type('\\'));
                        put(to_char_type('f'));
                        break;
                    case '"':
                        put(to_char_type('\\'));
                        put(to_char_type('"'));
                        break;
                    case '\\':
                        put(to_char_type('\\'));
                        put(to_char_type('\\'));
                        break;
                    default:
                        if (unit <= 0x1F || (arg_.escape_unicode && unit >= 0x7F)) {
                            escape_unicode(unit);
                        } else {
                            put(static_cast<char_type>(unit));
                        }
                        break;
                }
                ++i;
                run = i;
            }
            put(data + run, size - run);
        }

        void escape_unicode(std::uint32_t codepoint) {
            if (codepoint <= 0xFFFF) {
                put(to_char_type('\\'));
                put(to_char_type('u'));
                append_hex(static_cast<std::uint16_t>(codepoint), 4);
            } else {
                codepoint -= 0x10000;
                const auto high = static_cast<std::uint16_t>(0xD800 + (codepoint >> 10));
                const auto low = static_cast<std::uint16_t>(0xDC00 + (codepoint & 0x3FF));
                put(to_char_type('\\'));
                put(to_char_type('u'));
                append_hex(high, 4);
                put(to_char_type('\\'));
                put(to_char_type('u'));
                append_hex(low, 4);
            }
        }

        void append_hex(std::uint16_t value, int width) {
            static const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
            for (int i = width - 1; i >= 0; --i) {
                put(to_char_type(hex_chars[(value >> (4 * i)) & 0xF]));
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
            bool negative = value < 0;
            auto abs_value =
                static_cast<integer_type>(negative ? -static_cast<std::int64_t>(value) : static_cast<std::uint64_t>(value));
            char_type buffer[32]{};
            std::size_t pos = sizeof(buffer) / sizeof(buffer[0]);
            buffer[--pos] = '\0';
            while (abs_value > 0) {
                buffer[--pos] = to_char_type('0' + abs_value % 10);
                abs_value /= 10;
            }
            if (negative) {
                buffer[--pos] = to_char_type('-');
            }
            put(buffer + pos, sizeof(buffer) / sizeof(buffer[0]) - pos - 1);
        }

        void dump_float(float_type value) {
            if (!std::isfinite(value)) {
                dump_null();
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

        void write_indent(unsigned int indent_level) {
            if (indent_level > 0 && indent_level <= indent_string_.size()) {
                put(indent_string_.data(), indent_level);
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
        string_type indent_string_;
        core::collections::vector<char_type> out_buffer_;
    };
}

#endif
