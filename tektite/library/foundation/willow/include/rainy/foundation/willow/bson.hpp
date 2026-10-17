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
#ifndef RAINY_FOUNDATION_WILLOW_BSON_HPP
#define RAINY_FOUNDATION_WILLOW_BSON_HPP
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/bson/config.hpp>
#include <rainy/foundation/willow/implements/bson/direct.hpp>
#include <rainy/foundation/willow/implements/bson/parser.hpp>
#include <rainy/foundation/willow/implements/bson/serializer.hpp>
#include <rainy/foundation/willow/implements/bson/version.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::willow::bson {
    using willow::basic_document;
    using willow::document;
    using willow::document64;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;

    template <typename BasicDocument>
    void encode(const BasicDocument &doc, byte_buffer &out) {
        implements::bson_writer<BasicDocument>::encode_into(doc, out);
    }

    template <typename BasicDocument>
    void encode(const bson_document<BasicDocument> &doc, byte_buffer &out) {
        implements::bson_writer<bson_document<BasicDocument>>::encode_into(doc, out);
    }

    template <typename BasicDocument>
    byte_buffer encode(const BasicDocument &doc) {
        byte_buffer out{};
        encode(doc, out);
        return out;
    }

    template <typename BasicDocument>
    byte_buffer encode(const bson_document<BasicDocument> &doc) {
        byte_buffer out{};
        encode(doc, out);
        return out;
    }

    template <typename BasicDocument = document>
    void decode_into(const std::uint8_t *data, const std::size_t size, bson_document<BasicDocument> &out) {
        implements::bson_reader<BasicDocument>::decode_into(data, size, out);
    }

    template <typename BasicDocument = document>
    void decode_into(const byte_buffer &bytes, bson_document<BasicDocument> &out) {
        decode_into<BasicDocument>(bytes.data(), bytes.size(), out);
    }

    template <typename BasicDocument = document>
    bson_document<BasicDocument> decode(const std::uint8_t *data, const std::size_t size) {
        bson_document<BasicDocument> doc;
        decode_into<BasicDocument>(data, size, doc);
        return doc;
    }

    template <typename BasicDocument = document>
    bson_document<BasicDocument> decode(const byte_buffer &bytes) {
        return decode<BasicDocument>(bytes.data(), bytes.size());
    }

    /**
     * \lang english
     * @brief Writes a struct straight to bson bytes through its bson_fields registration.
     *
     * \lang simp-chinese
     * @brief 经其 bson_fields 注册把结构体直接写成 bson 字节。
     */
    template <typename Concept>
    byte_buffer encode_direct(const Concept &value) {
        static_assert(is_direct_message_v<Concept>, "encode_direct requires a concept type with member_field entries");
        byte_buffer out{};
        out.reserve(128);
        implements::direct_serializer<Concept>::encode_into(value, out);
        return out;
    }

    /**
     * \lang english
     * @brief Reads bson bytes straight into a struct through its bson_fields registration.
     *
     * \lang simp-chinese
     * @brief 经其 bson_fields 注册把 bson 字节直接读入结构体。
     */
    template <typename Concept>
    Concept decode_direct(const std::uint8_t *data, const std::size_t size) {
        static_assert(is_direct_message_v<Concept>, "decode_direct requires a concept type with member_field entries");
        Concept value{};
        if (data == nullptr && size != 0) {
            implements::throw_bson_parse_error("null input with nonzero size");
        }
        if (size == 0) {
            return value;
        }
        implements::direct_parser<Concept>::decode_into(data, size, value);
        return value;
    }

    template <typename Concept>
    Concept decode_direct(const byte_buffer &bytes) {
        if (bytes.empty()) {
            return Concept{};
        }
        return decode_direct<Concept>(bytes.data(), bytes.size());
    }

    template <typename Concept>
    void decode_direct_into(const std::uint8_t *data, const std::size_t size, Concept &value) {
        static_assert(is_direct_message_v<Concept>, "decode_direct_into requires a concept type with member_field entries");
        if (data == nullptr && size != 0) {
            implements::throw_bson_parse_error("null input with nonzero size");
        }
        if (size == 0) {
            return;
        }
        implements::direct_parser<Concept>::decode_into(data, size, value);
    }

    template <typename Concept>
    void decode_direct_into(const byte_buffer &bytes, Concept &value) {
        if (bytes.empty()) {
            return;
        }
        decode_direct_into<Concept>(bytes.data(), bytes.size(), value);
    }
}

#endif

#endif
