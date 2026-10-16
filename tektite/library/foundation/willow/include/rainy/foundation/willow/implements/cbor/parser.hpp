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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_PARSER_HPP
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/willow/implements/cbor/codec.hpp>
#include <rainy/foundation/willow/implements/cbor/config.hpp>
#include <rainy/foundation/willow/implements/cbor/exceptions.hpp>
#include <rainy/foundation/willow/implements/cbor/version.hpp>

#if RAINY_WILLOW_CBOR_AVAILABLE

namespace rainy::foundation::willow::cbor::implements {
    using namespace rainy::foundation::exceptions::willow::cbor;

    template <typename BasicDocument>
    void assign_cbor_text(cbor_document<BasicDocument> &slot, const std::uint8_t *data, const std::size_t size,
                          const bool byte_string) {
        using char_type = typename BasicDocument::char_type;
        using string_type = typename BasicDocument::string_type;
        if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
            string_type str{reinterpret_cast<const char *>(data), size};
            slot = cbor_document<BasicDocument>(str);
        }
#if RAINY_HAS_CXX20
        else if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
            string_type str{};
            str.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                str.push_back(static_cast<char8_t>(data[i]));
            }
            slot = cbor_document<BasicDocument>(str);
        }
#endif
        else {
            throw_cbor_parse_error("cbor strings require a 1-byte character document");
        }
        if (byte_string) {
            slot.mark_byte_string();
        }
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type narrow_key_to_string(const char *data, const std::size_t size) {
        using string_type = typename BasicDocument::string_type;
        return willow::implements::convert_document_string<string_type>(core::text::string(data, size));
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type cbor_key_to_string(const cbor_document<BasicDocument> &key) {
        char buffer[32];
        if (key.is_string()) {
            return key.as_string();
        }
        if (key.is_integer()) {
            const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), static_cast<long long>(key.as_integer()));
            return narrow_key_to_string<BasicDocument>(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }
        if (key.is_float()) {
            const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), static_cast<double>(key.as_float()));
            return narrow_key_to_string<BasicDocument>(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }
        if (key.is_bool()) {
            return key.as_bool() ? narrow_key_to_string<BasicDocument>("true", 4)
                                 : narrow_key_to_string<BasicDocument>("false", 5);
        }
        if (key.is_null()) {
            return narrow_key_to_string<BasicDocument>("null", 4);
        }
        throw_cbor_parse_error("cbor map key must be a scalar");
    }

    /**
     * \lang english
     * @brief The cbor reader, turning wire bytes into a document.
     *
     * A text string and a byte string both read back as a string, with the byte string flag telling them apart,
     * and an unassigned simple value or a semantic tag keeps its number on the node.
     *
     * @tparam BasicDocument The untagged document type
     *
     * \lang simp-chinese
     * @brief cbor 读取器，把线上字节转换为文档。
     *
     * 文本串与字节串都读作字符串，由字节串标志区分二者，未分配简单值或语义标签在节点上保留其编号。
     *
     * @tparam BasicDocument 未打标签的文档类型
     */
    template <typename BasicDocument>
    struct cbor_reader {
        using doc_type = cbor_document<BasicDocument>;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using string_type = typename BasicDocument::string_type;

        static void decode_into(const std::uint8_t *data, const std::size_t size, doc_type &out) {
            if (data == nullptr && size != 0) {
                throw_cbor_parse_error("null input with nonzero size");
            }
            if (size == 0) {
                throw_cbor_parse_error("a cbor item needs at least one byte");
            }
            const std::uint8_t *ptr = data;
            const std::uint8_t *const end = data + size;
            decode_value(ptr, end, out);
            if (ptr != end) {
                throw_cbor_parse_error("trailing bytes after the cbor item");
            }
        }

    private:
        static void assign_integer(doc_type &out, const std::int64_t value) {
            if (value < static_cast<std::int64_t>((utility::numeric_limits<integer_type>::min)()) ||
                value > static_cast<std::int64_t>((utility::numeric_limits<integer_type>::max)())) {
                throw_cbor_parse_error("integer value out of document range");
            }
            out = doc_type(static_cast<integer_type>(value));
        }

        static void assign_unsigned(doc_type &out, const std::uint64_t value) {
            if (value > static_cast<std::uint64_t>((utility::numeric_limits<integer_type>::max)())) {
                throw_cbor_parse_error("integer value out of document range");
            }
            out = doc_type(static_cast<integer_type>(value));
        }

        static void read_head(const std::uint8_t *&ptr, const std::uint8_t *end, major_type &major, std::uint8_t &additional,
                              std::uint64_t &argument) {
            std::uint8_t head = 0;
            if (!take_u8(ptr, end, head)) {
                throw_cbor_parse_error("unexpected end of cbor input");
            }
            major = static_cast<major_type>(head >> 5);
            additional = static_cast<std::uint8_t>(head & 0x1fu);
            argument = 0;
            if (additional == additional_indefinite) {
                return;
            }
            if (!take_argument(additional, ptr, end, argument)) {
                throw_cbor_parse_error("truncated cbor argument");
            }
        }

        static void decode_value(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            switch (major) {
                case major_type::unsigned_int:
                    if (additional == additional_indefinite) {
                        throw_cbor_parse_error("an integer may not use the indefinite form");
                    }
                    assign_unsigned(out, argument);
                    return;
                case major_type::negative_int:
                    if (additional == additional_indefinite) {
                        throw_cbor_parse_error("an integer may not use the indefinite form");
                    }
                    if (argument > static_cast<std::uint64_t>((utility::numeric_limits<std::int64_t>::max)())) {
                        throw_cbor_parse_error("a negative cbor integer is out of range");
                    }
                    assign_integer(out, -1 - static_cast<std::int64_t>(argument));
                    return;
                case major_type::byte_string:
                    decode_string(ptr, end, out, additional, argument, true);
                    return;
                case major_type::text_string:
                    decode_string(ptr, end, out, additional, argument, false);
                    return;
                case major_type::array:
                    decode_array(ptr, end, out, additional, argument);
                    return;
                case major_type::map:
                    decode_map(ptr, end, out, additional, argument);
                    return;
                case major_type::tag: {
                    if (additional == additional_indefinite) {
                        throw_cbor_parse_error("a tag may not use the indefinite form");
                    }
                    decode_tag(ptr, end, out, argument);
                    return;
                }
                case major_type::simple:
                    decode_simple(ptr, end, out, additional, argument);
                    return;
                default:
                    throw_cbor_parse_error("unknown cbor major type");
            }
        }

        static void decode_string(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out,
                                  const std::uint8_t additional, const std::uint64_t argument, const bool byte_string) {
            if (additional == additional_indefinite) {
                decode_indefinite_string(ptr, end, out, byte_string);
                return;
            }
            if (argument > static_cast<std::uint64_t>(end - ptr)) {
                throw_cbor_parse_error("a cbor string runs past the end of the input");
            }
            const auto size = static_cast<std::size_t>(argument);
            assign_cbor_text<BasicDocument>(out, ptr, size, byte_string);
            ptr += size;
        }

        static void decode_indefinite_string(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out,
                                             const bool byte_string) {
            string_type accumulated{};
            while (true) {
                if (ptr >= end) {
                    throw_cbor_parse_error("an indefinite cbor string is not terminated");
                }
                if (*ptr == 0xff) {
                    ++ptr;
                    break;
                }
                major_type chunk_major{};
                std::uint8_t chunk_additional = 0;
                std::uint64_t chunk_argument = 0;
                read_head(ptr, end, chunk_major, chunk_additional, chunk_argument);
                const bool chunk_byte = chunk_major == major_type::byte_string;
                if (chunk_byte != byte_string) {
                    throw_cbor_parse_error("an indefinite cbor string holds a chunk of the wrong type");
                }
                if (chunk_additional == additional_indefinite) {
                    throw_cbor_parse_error("a cbor string chunk may not itself be indefinite");
                }
                if (chunk_argument > static_cast<std::uint64_t>(end - ptr)) {
                    throw_cbor_parse_error("a cbor string chunk runs past the end of the input");
                }
                const auto size = static_cast<std::size_t>(chunk_argument);
                append_narrow(accumulated, ptr, size);
                ptr += size;
            }
            out = doc_type(accumulated);
            if (byte_string) {
                out.mark_byte_string();
            }
        }

        static void append_narrow(string_type &target, const std::uint8_t *data, const std::size_t size) {
            for (std::size_t i = 0; i < size; ++i) {
                target.push_back(static_cast<typename string_type::value_type>(data[i]));
            }
        }

        static void decode_array(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out,
                                 const std::uint8_t additional, const std::uint64_t argument) {
            out = doc_type(document_type::array);
            auto &array = out.as_array();
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor array is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    doc_type element;
                    decode_value(ptr, end, element);
                    array.push_back(utility::move(element));
                }
            }
            if (argument > static_cast<std::uint64_t>(end - ptr) + 1) {
                throw_cbor_parse_error("a cbor array declares more elements than the input holds");
            }
            array.reserve(static_cast<std::size_t>(argument));
            for (std::uint64_t i = 0; i < argument; ++i) {
                doc_type element;
                decode_value(ptr, end, element);
                array.push_back(utility::move(element));
            }
        }

        static void decode_map(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out,
                               const std::uint8_t additional, const std::uint64_t argument) {
            out = doc_type(document_type::object);
            auto &object = out.as_object();
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor map is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    decode_map_entry(ptr, end, object);
                }
            }
            if (argument > static_cast<std::uint64_t>(end - ptr) + 1) {
                throw_cbor_parse_error("a cbor map declares more entries than the input holds");
            }
            for (std::uint64_t i = 0; i < argument; ++i) {
                decode_map_entry(ptr, end, object);
            }
        }

        static void decode_map_entry(const std::uint8_t *&ptr, const std::uint8_t *end, typename doc_type::object_type &object) {
            doc_type key_node;
            decode_value(ptr, end, key_node);
            string_type key = cbor_key_to_string<BasicDocument>(key_node);
            doc_type value;
            decode_value(ptr, end, value);
            object.emplace(utility::move(key), utility::move(value));
        }

        static void decode_tag(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out, const std::uint64_t tag_number) {
            doc_type inner;
            decode_value(ptr, end, inner);
            out = utility::move(inner);
            out.set_tag(tag_number);
        }

        static void decode_simple(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out,
                                  const std::uint8_t additional, const std::uint64_t argument) {
            if (additional == additional_indefinite) {
                throw_cbor_parse_error("the break byte is not a value");
            }
            if (additional < 20) {
                out = doc_type(document_type::null);
                out.set_simple(static_cast<std::uint8_t>(argument));
                return;
            }
            switch (additional) {
                case 20:
                    out = doc_type(false);
                    return;
                case 21:
                    out = doc_type(true);
                    return;
                case 22:
                    out = doc_type(document_type::null);
                    return;
                case 23:
                    out = doc_type(document_type::null);
                    out.set_simple(static_cast<std::uint8_t>(simple_value::undefined));
                    return;
                case 24: {
                    out = doc_type(document_type::null);
                    out.set_simple(static_cast<std::uint8_t>(argument));
                    return;
                }
                case 25: {
                    out = doc_type(static_cast<float_type>(half_to_double(static_cast<std::uint16_t>(argument))));
                    return;
                }
                case 26: {
                    float value = 0;
                    const auto bits = static_cast<std::uint32_t>(argument);
                    core::builtin::copy_memory(&value, &bits, sizeof(float));
                    out = doc_type(static_cast<float_type>(value));
                    return;
                }
                case 27: {
                    double value = 0;
                    core::builtin::copy_memory(&value, &argument, sizeof(double));
                    out = doc_type(static_cast<float_type>(value));
                    return;
                }
                default:
                    throw_cbor_parse_error("unknown cbor simple value");
            }
        }
    };
}

#endif

#endif
