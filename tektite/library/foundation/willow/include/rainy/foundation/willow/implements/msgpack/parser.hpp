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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_PARSER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/willow/implements/msgpack/codec.hpp>
#include <rainy/foundation/willow/implements/msgpack/config.hpp>
#include <rainy/foundation/willow/implements/msgpack/exceptions.hpp>
#include <rainy/foundation/willow/implements/msgpack/version.hpp>

#if RAINY_WILLOW_MSGPACK_AVAILABLE

namespace rainy::foundation::willow::msgpack::implements {
    using namespace rainy::foundation::exceptions::willow::msgpack;

    template <typename BasicDocument>
    void assign_msgpack_string(msgpack_document<BasicDocument> &slot, const std::uint8_t *data, const std::size_t size) {
        using char_type = typename BasicDocument::char_type;
        using string_type = typename BasicDocument::string_type;
        if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
            string_type str{reinterpret_cast<const char *>(data), size};
            slot = msgpack_document<BasicDocument>(str);
        }
#if RAINY_HAS_CXX20
        else if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
            string_type str{};
            str.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                str.push_back(static_cast<char8_t>(data[i]));
            }
            slot = msgpack_document<BasicDocument>(str);
        }
#endif
        else {
            throw_msgpack_parse_error("msgpack strings require a 1-byte character document");
        }
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type narrow_key_to_string(const char *data, const std::size_t size) {
        using string_type = typename BasicDocument::string_type;
        return willow::implements::convert_document_string<string_type>(core::text::string(data, size));
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type msgpack_key_to_string(const msgpack_document<BasicDocument> &key) {
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
        throw_msgpack_parse_error("msgpack map key must be a scalar");
    }

    template <typename BasicDocument>
    struct msgpack_parser {
        using doc_type = msgpack_document<BasicDocument>;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using string_type = typename BasicDocument::string_type;

        static void decode_into(const std::uint8_t *data, const std::size_t size, doc_type &out, const msgpack_options &options) {
            if (data == nullptr && size != 0) {
                throw_msgpack_parse_error("null input with nonzero size");
            }
            const std::uint8_t *ptr = data;
            const std::uint8_t *end = data + size;
            decode_value(ptr, end, out, options);
            if (ptr != end) {
                throw_msgpack_parse_error("trailing bytes after msgpack value");
            }
        }

    private:
        static void assign_uint(doc_type &out, const std::uint64_t value) {
            if (value > static_cast<std::uint64_t>((utility::numeric_limits<integer_type>::max)())) {
                throw_msgpack_parse_error("integer value out of document range");
            }
            out = doc_type(static_cast<integer_type>(value));
        }

        static void assign_int(doc_type &out, const std::int64_t value) {
            if (value < static_cast<std::int64_t>((utility::numeric_limits<integer_type>::min)()) ||
                value > static_cast<std::int64_t>((utility::numeric_limits<integer_type>::max)())) {
                throw_msgpack_parse_error("integer value out of document range");
            }
            out = doc_type(static_cast<integer_type>(value));
        }

        static void decode_str(doc_type &out, const std::uint8_t *&ptr, const std::uint8_t *end, const std::size_t length) {
            if (static_cast<std::size_t>(end - ptr) < length) {
                throw_msgpack_parse_error("truncated string payload");
            }
            assign_msgpack_string<BasicDocument>(out, ptr, length);
            ptr += length;
        }

        static void decode_array(doc_type &out, const std::uint8_t *&ptr, const std::uint8_t *end, const std::size_t count,
                                 const msgpack_options &options) {
            out = doc_type(document_type::array);
            auto &array = out.as_array();
            array.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                doc_type element;
                decode_value(ptr, end, element, options);
                array.push_back(utility::move(element));
            }
        }

        static void decode_map(doc_type &out, const std::uint8_t *&ptr, const std::uint8_t *end, const std::size_t count,
                               const msgpack_options &options) {
            out = doc_type(document_type::object);
            auto &object = out.as_object();
            for (std::size_t i = 0; i < count; ++i) {
                doc_type key_node;
                decode_value(ptr, end, key_node, options);
                string_type key = msgpack_key_to_string<BasicDocument>(key_node);
                doc_type value;
                decode_value(ptr, end, value, options);
                object.emplace(utility::move(key), utility::move(value));
            }
        }

        static void decode_ext(doc_type &out, const std::uint8_t *&ptr, const std::uint8_t *end, const std::size_t length,
                               const msgpack_options &options) {
            std::uint8_t raw_type = 0;
            if (!take_u8(ptr, end, raw_type)) {
                throw_msgpack_parse_error("truncated extension type");
            }
            const std::int8_t ext_type = static_cast<std::int8_t>(raw_type);
            if (ext_type == -1) {
                if ((options.features & feature::timestamp) == feature::none) {
                    throw_msgpack_parse_error("timestamp encountered but the timestamp feature is disabled");
                }
            } else if ((options.features & feature::ext) == feature::none) {
                throw_msgpack_parse_error("extension encountered but the ext feature is disabled");
            }
            if (static_cast<std::size_t>(end - ptr) < length) {
                throw_msgpack_parse_error("truncated extension payload");
            }
            byte_buffer payload{};
            payload.reserve(length);
            for (std::size_t i = 0; i < length; ++i) {
                payload.push_back(ptr[i]);
            }
            ptr += length;
            out = doc_type(document_type::null);
            out.set_ext(static_cast<std::int8_t>(raw_type), utility::move(payload));
        }

        static void decode_value(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out, const msgpack_options &options) {
            if (ptr >= end) {
                throw_msgpack_parse_error("unexpected end of msgpack input");
            }
            const std::uint8_t head = *ptr++;
            if (head <= 0x7f) {
                assign_uint(out, head);
                return;
            }
            if (head >= 0xe0) {
                assign_int(out, static_cast<std::int8_t>(head));
                return;
            }
            if (head <= 0x8f) {
                decode_map(out, ptr, end, head & 0x0f, options);
                return;
            }
            if (head <= 0x9f) {
                decode_array(out, ptr, end, head & 0x0f, options);
                return;
            }
            if (head <= 0xbf) {
                decode_str(out, ptr, end, head & 0x1f);
                return;
            }
            switch (head) {
                case 0xc0:
                    out = doc_type(document_type::null);
                    return;
                case 0xc1:
                    throw_msgpack_parse_error("reserved format byte 0xc1");
                case 0xc2:
                    out = doc_type(false);
                    return;
                case 0xc3:
                    out = doc_type(true);
                    return;
                case 0xc4:
                case 0xc5:
                case 0xc6: {
                    std::size_t length = 0;
                    if (!read_length(ptr, end, head, length)) {
                        throw_msgpack_parse_error("truncated binary length");
                    }
                    decode_str(out, ptr, end, length);
                    return;
                }
                case 0xc7:
                case 0xc8:
                case 0xc9: {
                    std::size_t length = 0;
                    if (!read_length(ptr, end, head, length)) {
                        throw_msgpack_parse_error("truncated extension length");
                    }
                    decode_ext(out, ptr, end, length, options);
                    return;
                }
                case 0xca: {
                    std::uint32_t bits = 0;
                    if (!take_be32(ptr, end, bits)) {
                        throw_msgpack_parse_error("truncated float32 payload");
                    }
                    float value = 0;
                    core::builtin::copy_memory(&value, &bits, sizeof(float));
                    out = doc_type(static_cast<float_type>(value));
                    return;
                }
                case 0xcb: {
                    std::uint64_t bits = 0;
                    if (!take_be64(ptr, end, bits)) {
                        throw_msgpack_parse_error("truncated float64 payload");
                    }
                    double value = 0;
                    core::builtin::copy_memory(&value, &bits, sizeof(double));
                    out = doc_type(static_cast<float_type>(value));
                    return;
                }
                case 0xcc: {
                    std::uint8_t value = 0;
                    if (!take_u8(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated uint8 payload");
                    }
                    assign_uint(out, value);
                    return;
                }
                case 0xcd: {
                    std::uint16_t value = 0;
                    if (!take_be16(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated uint16 payload");
                    }
                    assign_uint(out, value);
                    return;
                }
                case 0xce: {
                    std::uint32_t value = 0;
                    if (!take_be32(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated uint32 payload");
                    }
                    assign_uint(out, value);
                    return;
                }
                case 0xcf: {
                    std::uint64_t value = 0;
                    if (!take_be64(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated uint64 payload");
                    }
                    assign_uint(out, value);
                    return;
                }
                case 0xd0: {
                    std::uint8_t value = 0;
                    if (!take_u8(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated int8 payload");
                    }
                    assign_int(out, static_cast<std::int8_t>(value));
                    return;
                }
                case 0xd1: {
                    std::uint16_t value = 0;
                    if (!take_be16(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated int16 payload");
                    }
                    assign_int(out, static_cast<std::int16_t>(value));
                    return;
                }
                case 0xd2: {
                    std::uint32_t value = 0;
                    if (!take_be32(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated int32 payload");
                    }
                    assign_int(out, static_cast<std::int32_t>(value));
                    return;
                }
                case 0xd3: {
                    std::uint64_t value = 0;
                    if (!take_be64(ptr, end, value)) {
                        throw_msgpack_parse_error("truncated int64 payload");
                    }
                    assign_int(out, static_cast<std::int64_t>(value));
                    return;
                }
                case 0xd4:
                case 0xd5:
                case 0xd6:
                case 0xd7:
                case 0xd8:
                    decode_ext(out, ptr, end, fixext_length(head), options);
                    return;
                case 0xd9:
                case 0xda:
                case 0xdb: {
                    std::size_t length = 0;
                    if (!read_length(ptr, end, head, length)) {
                        throw_msgpack_parse_error("truncated string length");
                    }
                    decode_str(out, ptr, end, length);
                    return;
                }
                case 0xdc:
                case 0xdd: {
                    std::size_t count = 0;
                    if (!read_length(ptr, end, head, count)) {
                        throw_msgpack_parse_error("truncated array length");
                    }
                    decode_array(out, ptr, end, count, options);
                    return;
                }
                case 0xde:
                case 0xdf: {
                    std::size_t count = 0;
                    if (!read_length(ptr, end, head, count)) {
                        throw_msgpack_parse_error("truncated map length");
                    }
                    decode_map(out, ptr, end, count, options);
                    return;
                }
                default:
                    throw_msgpack_parse_error("unknown msgpack format byte");
            }
        }

        static bool read_length(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t head,
                                std::size_t &length) {
            switch (head) {
                case 0xc4:
                case 0xc7:
                case 0xd9: {
                    std::uint8_t value = 0;
                    if (!take_u8(ptr, end, value)) {
                        return false;
                    }
                    length = value;
                    return true;
                }
                case 0xc5:
                case 0xc8:
                case 0xda:
                case 0xdc:
                case 0xde: {
                    std::uint16_t value = 0;
                    if (!take_be16(ptr, end, value)) {
                        return false;
                    }
                    length = value;
                    return true;
                }
                case 0xc6:
                case 0xc9:
                case 0xdb:
                case 0xdd:
                case 0xdf: {
                    std::uint32_t value = 0;
                    if (!take_be32(ptr, end, value)) {
                        return false;
                    }
                    length = value;
                    return true;
                }
                default:
                    return false;
            }
        }
    };
}

#endif

#endif
