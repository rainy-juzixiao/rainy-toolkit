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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_SERIALIZER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/bson/codec.hpp>
#include <rainy/foundation/willow/implements/bson/config.hpp>
#include <rainy/foundation/willow/implements/bson/exceptions.hpp>
#include <rainy/foundation/willow/implements/bson/version.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::willow::bson::implements {
    using namespace rainy::foundation::exceptions::willow::bson;

    /**
     * \lang english
     * @brief The bson writer, turning a document into wire bytes.
     *
     * A document becomes a bson document, an array becomes a bson array, and the scalar kinds pick the narrowest
     * element type that holds them: an integer within the 32-bit range is written as int32, anything wider as
     * int64. A node carrying native state is written back byte for byte, so a decoded element survives a re
     * encode untouched.
     *
     * @tparam Document The document type
     *
     * \lang simp-chinese
     * @brief bson 写入器，把文档转换为线上字节。
     *
     * 文档变为 bson 文档，数组变为 bson 数组，标量种类选择能容纳它们的最窄元素类型：32 位范围内的整数写为
     * int32，更宽的写为 int64。携带原生状态的节点逐字节写回，因此已解码的元素在重新编码时原样保留。
     *
     * @tparam Document 文档类型
     */
    template <typename Document>
    struct bson_writer {
        using char_type = typename Document::char_type;
        using string_type = typename Document::string_type;

        static void encode_into(const Document &doc, byte_buffer &out) {
            if (!doc.is_object()) {
                throw_bson_serialize_error("a bson document must be an object");
            }
            encode_document(doc, out);
        }

    private:
        static void encode_document(const Document &doc, byte_buffer &out) {
            const std::size_t length_offset = out.size();
            push_le32(out, 0);
            for (const auto &entry: doc.as_object()) {
                encode_entry(entry.first, entry.second, out);
            }
            out.push_back(0);
            const std::size_t length = out.size() - length_offset;
            patch_le32(out, length_offset, static_cast<std::uint32_t>(length));
        }

        static void encode_array(const Document &doc, byte_buffer &out) {
            const std::size_t length_offset = out.size();
            push_le32(out, 0);
            std::size_t index = 0;
            char index_buffer[24];
            for (const auto &element: doc.as_array()) {
                out.push_back(element_byte_of(element));
                const auto result = core::text::to_chars(index_buffer, index_buffer + sizeof(index_buffer),
                                                         static_cast<unsigned long long>(index));
                out.insert(out.end(), index_buffer, result.ptr);
                ++index;
                out.push_back(0);
                encode_value(element, out);
            }
            out.push_back(0);
            const std::size_t length = out.size() - length_offset;
            patch_le32(out, length_offset, static_cast<std::uint32_t>(length));
        }

        static void encode_entry(const string_type &key, const Document &value, byte_buffer &out) {
            out.push_back(element_byte_of(value));
            append_key(key, out);
            out.push_back(0);
            encode_value(value, out);
        }

        static void append_key(const string_type &key, byte_buffer &out) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                const auto *data = reinterpret_cast<const std::uint8_t *>(key.data());
                for (std::size_t i = 0; i < key.size(); ++i) {
                    if (data[i] == 0) {
                        throw_bson_serialize_error("a bson key must not contain a 0x00 byte");
                    }
                }
                out.insert(out.end(), data, data + key.size());
            } else {
                throw_bson_serialize_error("bson keys require a 1-byte character document");
            }
        }

        static std::uint8_t element_byte_of(const Document &doc) {
            if constexpr (has_bson_native_element_v<Document>) {
                if (doc.has_native_element()) {
                    return static_cast<std::uint8_t>(doc.native_element());
                }
            }
            switch (doc.type()) {
                case document_type::number_float:
                    return static_cast<std::uint8_t>(element_type::double_);
                case document_type::string:
                    return static_cast<std::uint8_t>(element_type::string_);
                case document_type::object:
                    return static_cast<std::uint8_t>(element_type::document);
                case document_type::array:
                    return static_cast<std::uint8_t>(element_type::array);
                case document_type::boolean:
                    return static_cast<std::uint8_t>(element_type::boolean);
                case document_type::null:
                    return static_cast<std::uint8_t>(element_type::null_);
                case document_type::number_integer:
                    return fits_int32(doc.as_integer()) ? static_cast<std::uint8_t>(element_type::int32)
                                                        : static_cast<std::uint8_t>(element_type::int64);
                default:
                    break;
            }
            throw_bson_serialize_error("unsupported document value for bson");
        }

        static bool fits_int32(const typename Document::integer_type value) {
            return value >= static_cast<typename Document::integer_type>((utility::numeric_limits<std::int32_t>::min)()) &&
                   value <= static_cast<typename Document::integer_type>((utility::numeric_limits<std::int32_t>::max)());
        }

        static void encode_value(const Document &doc, byte_buffer &out) {
            if constexpr (has_bson_native_element_v<Document>) {
                if (doc.has_native_element()) {
                    const byte_buffer &payload = doc.native_payload();
                    out.insert(out.end(), payload.begin(), payload.end());
                    return;
                }
            }
            switch (doc.type()) {
                case document_type::number_float: {
                    double value = static_cast<double>(doc.as_float());
                    std::uint64_t bits = 0;
                    core::builtin::copy_memory(&bits, &value, sizeof(double));
                    push_le64(out, bits);
                    return;
                }
                case document_type::string: {
                    const string_type &str = doc.as_string();
                    if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                        push_le32(out, static_cast<std::uint32_t>(str.size() + 1));
                        const auto *data = reinterpret_cast<const std::uint8_t *>(str.data());
                        for (std::size_t i = 0; i < str.size(); ++i) {
                            if (data[i] == 0) {
                                throw_bson_serialize_error("a bson string must not contain a 0x00 byte");
                            }
                        }
                        out.insert(out.end(), data, data + str.size());
                        out.push_back(0);
                    } else {
                        throw_bson_serialize_error("bson strings require a 1-byte character document");
                    }
                    return;
                }
                case document_type::object:
                    encode_document(doc, out);
                    return;
                case document_type::array:
                    encode_array(doc, out);
                    return;
                case document_type::boolean:
                    out.push_back(doc.as_bool() ? 1 : 0);
                    return;
                case document_type::null:
                    return;
                case document_type::number_integer: {
                    const auto value = doc.as_integer();
                    if (fits_int32(value)) {
                        push_le32(out, static_cast<std::uint32_t>(static_cast<std::int32_t>(value)));
                    } else {
                        push_le64(out, static_cast<std::uint64_t>(static_cast<std::int64_t>(value)));
                    }
                    return;
                }
                default:
                    break;
            }
            throw_bson_serialize_error("unsupported document value for bson");
        }

        static void patch_le32(byte_buffer &out, const std::size_t offset, const std::uint32_t value) {
            out[offset] = static_cast<std::uint8_t>(value & 0xffu);
            out[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xffu);
            out[offset + 2] = static_cast<std::uint8_t>((value >> 16) & 0xffu);
            out[offset + 3] = static_cast<std::uint8_t>((value >> 24) & 0xffu);
        }
    };
}

#endif

#endif
