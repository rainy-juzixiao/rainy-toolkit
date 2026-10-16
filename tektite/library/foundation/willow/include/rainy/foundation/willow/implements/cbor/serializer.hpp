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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_SERIALIZER_HPP
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/cbor/codec.hpp>
#include <rainy/foundation/willow/implements/cbor/config.hpp>
#include <rainy/foundation/willow/implements/cbor/exceptions.hpp>
#include <rainy/foundation/willow/implements/cbor/version.hpp>

#if RAINY_WILLOW_CBOR_AVAILABLE

namespace rainy::foundation::willow::cbor::implements {
    using namespace rainy::foundation::exceptions::willow::cbor;

    /**
     * \lang english
     * @brief The cbor writer, turning a document into wire bytes.
     *
     * Every argument is written in its shortest form and every float in the narrowest of half, single and
     * double that reproduces it exactly, which is the canonical encoding rfc 8949 prefers.
     *
     * @tparam Document The document type
     *
     * \lang simp-chinese
     * @brief cbor 写入器，把文档转换为线上字节。
     *
     * 每个参数都以最短形式写出，每个浮点数都取半精度、单精度与双精度中能精确再现它的最窄者，这是 RFC 8949
     * 偏好的规范编码。
     *
     * @tparam Document 文档类型
     */
    template <typename Document>
    struct cbor_writer {
        using char_type = typename Document::char_type;
        using string_type = typename Document::string_type;

        static void encode_into(const Document &doc, byte_buffer &out) {
            encode_value(doc, out);
        }

    private:
        static void encode_value(const Document &doc, byte_buffer &out) {
            if constexpr (has_cbor_native_value_v<Document>) {
                if (doc.has_native_value()) {
                    encode_native(doc, out);
                    return;
                }
            }
            switch (doc.type()) {
                case document_type::null:
                    out.push_back(static_cast<std::uint8_t>(0xe0u | static_cast<std::uint8_t>(simple_value::null_)));
                    return;
                case document_type::boolean:
                    out.push_back(static_cast<std::uint8_t>(
                        0xe0u | static_cast<std::uint8_t>(doc.as_bool() ? simple_value::true_ : simple_value::false_)));
                    return;
                case document_type::number_integer:
                    encode_integer(out, static_cast<std::int64_t>(doc.as_integer()));
                    return;
                case document_type::number_float:
                    encode_float(out, static_cast<double>(doc.as_float()));
                    return;
                case document_type::string:
                    encode_string(doc, out);
                    return;
                case document_type::array: {
                    const auto &array = doc.as_array();
                    const std::size_t size = array.size();
                    if (size <= static_cast<std::size_t>((utility::numeric_limits<std::uint32_t>::max)())) {
                        push_head(out, major_type::array, static_cast<std::uint64_t>(size));
                    } else if (size <= static_cast<std::size_t>((utility::numeric_limits<std::uint64_t>::max)())) {
                        push_head(out, major_type::array, static_cast<std::uint64_t>(size));
                    } else {
                        throw_cbor_serialize_error("a cbor array exceeds the 64-bit length limit");
                    }
                    for (const auto &element: array) {
                        encode_value(element, out);
                    }
                    return;
                }
                case document_type::object: {
                    const auto &object = doc.as_object();
                    push_head(out, major_type::map, static_cast<std::uint64_t>(object.size()));
                    for (const auto &entry: object) {
                        encode_key(entry.first, out);
                        encode_value(entry.second, out);
                    }
                    return;
                }
                default:
                    break;
            }
            throw_cbor_serialize_error("unsupported document value for cbor");
        }

        static void encode_native(const Document &doc, byte_buffer &out) {
            if (doc.is_tagged()) {
                push_head(out, major_type::tag, doc.tag());
                if constexpr (has_cbor_native_value_v<Document>) {
                    Document inner = doc;
                    inner.clear_native_value();
                    encode_value(inner, out);
                }
                return;
            }
            if (doc.is_null()) {
                const std::uint8_t value = doc.simple();
                if (value < 24) {
                    out.push_back(static_cast<std::uint8_t>(0xe0u | value));
                } else {
                    out.push_back(0xf8);
                    out.push_back(value);
                }
                return;
            }
            throw_cbor_serialize_error("unsupported native cbor value");
        }

        static void encode_key(const string_type &key, byte_buffer &out) {
            encode_text(key.data(), key.size(), out);
        }

        static void encode_string(const Document &doc, byte_buffer &out) {
            const string_type &str = doc.as_string();
            bool byte_string = false;
            if constexpr (has_cbor_byte_string_v<Document>) {
                byte_string = doc.is_byte_string();
            }
            encode_text(str.data(), str.size(), out, byte_string);
        }

        template <typename Char>
        static void encode_text(const Char *data, const std::size_t size, byte_buffer &out, const bool byte_string = false) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char> ||
                          type_traits::type_relations::is_same_v<char_type, char8_t>) {
                if (size > static_cast<std::size_t>((utility::numeric_limits<std::uint64_t>::max)())) {
                    throw_cbor_serialize_error("a cbor string exceeds the 64-bit length limit");
                }
                push_head(out, byte_string ? major_type::byte_string : major_type::text_string, static_cast<std::uint64_t>(size));
                const auto *bytes = reinterpret_cast<const std::uint8_t *>(data);
                out.insert(out.end(), bytes, bytes + size);
            } else {
                throw_cbor_serialize_error("cbor strings require a 1-byte character document");
            }
        }

        static void encode_integer(byte_buffer &out, const std::int64_t value) {
            if (value >= 0) {
                push_head(out, major_type::unsigned_int, static_cast<std::uint64_t>(value));
            } else {
                push_head(out, major_type::negative_int, static_cast<std::uint64_t>(-1 - value));
            }
        }

        /**
         * \lang english
         * @brief Writes a float in the narrowest of half, single and double that reproduces it exactly.
         *
         * \lang simp-chinese
         * @brief 以半精度、单精度与双精度中能精确再现该浮点数的最窄形式写出它。
         */
        static void encode_float(byte_buffer &out, const double value) {
            push_float_narrowest(out, value);
        }
    };
}

#endif

#endif
