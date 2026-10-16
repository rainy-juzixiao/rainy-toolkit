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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_SERIALIZER_HPP
#include <cmath>
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/toml/config.hpp>
#include <rainy/foundation/willow/implements/toml/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::toml::implements {
    using namespace rainy::foundation::exceptions::willow;

    /**
     * \lang english
     * @brief The toml serializer, writing a document as toml text.
     *
     * A table writes its scalar and array entries first and its sub tables afterwards, because toml
     * requires every key of a table to appear before the header of a sub table. Arrays are written
     * inline, a table inside an array becoming an inline table, which round trips even though the
     * array of tables header form is only read, never produced.
     *
     * @tparam BasicDocument The document type
     *
     * \lang simp-chinese
     * @brief toml 序列化器，把文档写成 toml 文本。
     *
     * 表先写其标量与数组项，再写子表，因为 toml 要求一张表的所有键都出现在子表表头之前。数组以内联形式写出，
     * 数组中的表变为内联表；这样即使表数组表头形式只读不写，也能往返。
     *
     * @tparam BasicDocument 文档类型
     */
    template <typename BasicDocument>
    struct toml_serializer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using args = serializer_args<BasicDocument>;

        static constexpr std::size_t flush_threshold = 4096;

        toml_serializer(output_adapter<char_type> adapter, const args &arg) : out_(utility::move(adapter)), arg_(arg) {
            out_buffer_.reserve(flush_threshold);
            path_buffer_.reserve(256);
        }

        void dump(const BasicDocument &toml) {
            if (!toml.is_object()) {
                exceptions::willow::toml::throw_toml_serialize_error("a toml document must be an object");
            }
            dump_table(toml, false);
            flush_buffer();
        }

    private:
        void dump_table(const BasicDocument &table, const bool emit_header) {
            const auto &object = table.as_object();
            bool has_scalar_entry = false;
            bool has_child_table = false;
            for (const auto &entry: object) {
                if (entry.second.is_object()) {
                    has_child_table = true;
                } else {
                    has_scalar_entry = true;
                }
            }
            const bool wrote_header = emit_header && (has_scalar_entry || !has_child_table);
            if (wrote_header) {
                put(to_char_type('\n'));
                put(to_char_type('['));
                put(path_buffer_.data(), path_buffer_.size());
                put(to_char_type(']'));
                put(to_char_type('\n'));
            }
            for (const auto &entry: object) {
                if (entry.second.is_object()) {
                    continue;
                }
                dump_key(entry.first);
                put(to_char_type(' '));
                put(to_char_type('='));
                put(to_char_type(' '));
                dump_value(entry.second);
                put(to_char_type('\n'));
            }
            for (const auto &entry: object) {
                if (!entry.second.is_object()) {
                    continue;
                }
                const std::size_t saved = path_buffer_.size();
                if (saved != 0) {
                    path_buffer_.push_back(char_type('.'));
                }
                path_buffer_.append(entry.first);
                dump_table(entry.second, true);
                path_buffer_.resize(saved);
            }
        }

        void dump_key(const string_type &key) {
            if (is_bare_key(key)) {
                put(key.data(), key.size());
                return;
            }
            dump_basic_string(key);
        }

        static bool is_bare_key(const string_type &key) {
            if (key.empty()) {
                return false;
            }
            for (const char_type ch: key) {
                const auto unit = static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(ch));
                const bool bare = (unit >= 'A' && unit <= 'Z') || (unit >= 'a' && unit <= 'z') || (unit >= '0' && unit <= '9') ||
                                  unit == '_' || unit == '-';
                if (!bare) {
                    return false;
                }
            }
            return true;
        }

        void dump_value(const BasicDocument &value) {
            if constexpr (has_toml_datetime_v<BasicDocument>) {
                if (value.has_datetime()) {
                    dump_datetime(value.as_datetime());
                    return;
                }
            }
            switch (value.type()) {
                case document_type::string:
                    dump_basic_string(value.as_string());
                    break;
                case document_type::boolean:
                    dump_boolean(value.as_bool());
                    break;
                case document_type::number_integer:
                    dump_integer(value.as_integer());
                    break;
                case document_type::number_float:
                    dump_float(value.as_float());
                    break;
                case document_type::array:
                    dump_array(value);
                    break;
                case document_type::object:
                    dump_inline_table(value);
                    break;
                case document_type::null:
                    exceptions::willow::toml::throw_toml_serialize_error("toml has no null value");
            }
        }

        void dump_array(const BasicDocument &value) {
            const auto &array = value.as_array();
            put(to_char_type('['));
            const std::size_t size = array.size();
            for (std::size_t i = 0; i < size; ++i) {
                if (i != 0) {
                    put(to_char_type(','));
                    put(to_char_type(' '));
                }
                dump_value(array[i]);
            }
            put(to_char_type(']'));
        }

        void dump_inline_table(const BasicDocument &value) {
            const auto &object = value.as_object();
            put(to_char_type('{'));
            bool first = true;
            for (const auto &entry: object) {
                if (!first) {
                    put(to_char_type(','));
                    put(to_char_type(' '));
                }
                dump_key(entry.first);
                put(to_char_type(' '));
                put(to_char_type('='));
                put(to_char_type(' '));
                dump_value(entry.second);
                first = false;
            }
            put(to_char_type('}'));
        }

        void dump_basic_string(const string_type &str) {
            put(to_char_type('\"'));
            for (const char_type ch: str) {
                const auto unit = static_cast<std::uint32_t>(static_cast<type_traits::helper::make_unsigned_t<char_type>>(ch));
                switch (unit) {
                    case '\b':
                        put_escape('b');
                        break;
                    case '\t':
                        put_escape('t');
                        break;
                    case '\n':
                        put_escape('n');
                        break;
                    case '\f':
                        put_escape('f');
                        break;
                    case '\r':
                        put_escape('r');
                        break;
                    case '\"':
                        put_escape('\"');
                        break;
                    case '\\':
                        put_escape('\\');
                        break;
                    default:
                        if (unit < 0x20 || unit == 0x7F) {
                            escape_unicode(unit);
                        } else {
                            put(ch);
                        }
                        break;
                }
            }
            put(to_char_type('\"'));
        }

        void put_escape(const char_type ch) {
            put(to_char_type('\\'));
            put(ch);
        }

        void escape_unicode(std::uint32_t codepoint) {
            put(to_char_type('\\'));
            put(to_char_type('u'));
            static const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
            for (int i = 3; i >= 0; --i) {
                put(to_char_type(hex_chars[(codepoint >> (4 * i)) & 0xF]));
            }
        }

        void dump_boolean(const boolean_type value) {
            if (value) {
                put_literal("true");
            } else {
                put_literal("false");
            }
        }

        void dump_datetime(const datetime_fields &fields) {
            if (fields.kind != datetime_kind::local_time) {
                put_padded(fields.year, 4);
                put(to_char_type('-'));
                put_padded(fields.month, 2);
                put(to_char_type('-'));
                put_padded(fields.day, 2);
            }
            if (fields.kind != datetime_kind::local_date) {
                if (fields.kind != datetime_kind::local_time) {
                    put(fields.use_space_separator ? to_char_type(' ') : to_char_type('T'));
                }
                put_padded(fields.hour, 2);
                put(to_char_type(':'));
                put_padded(fields.minute, 2);
                put(to_char_type(':'));
                put_padded(fields.second, 2);
                if (fields.nanosecond != 0) {
                    put(to_char_type('.'));
                    put_fraction(fields.nanosecond);
                }
            }
            if (fields.has_offset) {
                if (fields.use_zulu) {
                    put(to_char_type('Z'));
                } else {
                    const std::int32_t offset = fields.offset_minutes;
                    put(offset < 0 ? to_char_type('-') : to_char_type('+'));
                    const std::int32_t magnitude = offset < 0 ? -offset : offset;
                    put_padded(magnitude / 60, 2);
                    put(to_char_type(':'));
                    put_padded(magnitude % 60, 2);
                }
            }
        }

        void put_padded(std::int32_t value, int width) {
            for (int i = width - 1; i >= 0; --i) {
                std::int32_t divisor = 1;
                for (int j = 0; j < i; ++j) {
                    divisor *= 10;
                }
                put(to_char_type('0' + (value / divisor) % 10));
            }
        }

        void put_fraction(std::int32_t nanosecond) {
            int width = 9;
            while (width > 1 && nanosecond % 10 == 0) {
                nanosecond /= 10;
                --width;
            }
            put_padded(nanosecond, width);
        }

        void dump_integer(const integer_type value) {
            char buffer[32]{};
            const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), static_cast<long long>(value));
            put_narrow(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }

        void dump_float(const float_type value) {
            if (std::isnan(value)) {
                if (std::signbit(value)) {
                    put(to_char_type('-'));
                }
                put_literal("nan");
                return;
            }
            if (std::isinf(value)) {
                if (value < 0) {
                    put(to_char_type('-'));
                }
                put_literal("inf");
                return;
            }
            char buffer[64]{};
            const auto result =
                core::text::to_chars(buffer, buffer + sizeof(buffer), value, core::text::chars_format::general, arg_.precision);
            if (result.ec != std::errc{}) {
                exceptions::willow::toml::throw_toml_serialize_error("a float could not be formatted");
            }
            put_narrow(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }

        void put_narrow(const char *const str, const std::size_t size) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                put(str, size);
            } else {
                char_type text[64];
                const std::size_t count = size < 64 ? size : 64;
                for (std::size_t i = 0; i < count; ++i) {
                    text[i] = static_cast<char_type>(str[i]);
                }
                put(text, count);
            }
        }

        void put_literal(const char *const literal) {
            std::size_t size = 0;
            while (literal[size] != '\0') {
                ++size;
            }
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                put(literal, size);
            } else {
                char_type text[8];
                for (std::size_t i = 0; i < size; ++i) {
                    text[i] = static_cast<char_type>(literal[i]);
                }
                put(text, size);
            }
        }

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

        static char_type to_char_type(const char_type ch) {
            return char_traits::to_char_type(static_cast<char_int_type>(ch));
        }

    private:
        output_adapter<char_type> out_;
        const args &arg_;
        string_type path_buffer_;
        core::collections::vector<char_type> out_buffer_;
    };
}

#endif
