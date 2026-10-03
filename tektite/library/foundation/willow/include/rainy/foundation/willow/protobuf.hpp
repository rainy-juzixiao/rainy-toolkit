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
#ifndef RAINY_FOUNDATION_WILLOW_PROTOBUF_HPP
#define RAINY_FOUNDATION_WILLOW_PROTOBUF_HPP
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/direct.hpp>
#include <rainy/foundation/willow/implements/protobuf/parser.hpp>
#include <rainy/foundation/willow/implements/protobuf/serializer.hpp>

#if RAINY_HAS_CXX20

namespace rainy::foundation::willow::protobuf {
    using willow::basic_document;
    using willow::document;
    using willow::document64;
    using willow::document_type;

    template <typename Concept, typename BasicDocument>
    struct protobuf_document : public BasicDocument {
        static_assert(protobuf_message<Concept>, "protobuf facade requires a concept type with protobuf_fields");
        static_assert(valid_field_numbers_v<Concept>, "protobuf field numbers must be unique and within 1..536870911");

        using document_base = BasicDocument;

        protobuf_document() = default;

        protobuf_document(const BasicDocument &value) : BasicDocument(value) {
        }

        protobuf_document(BasicDocument &&value) : BasicDocument(static_cast<BasicDocument &&>(value)) {
        }

        const BasicDocument &as_document() const noexcept {
            return static_cast<const BasicDocument &>(*this);
        }

        BasicDocument &as_document() noexcept {
            return static_cast<BasicDocument &>(*this);
        }

        friend bool operator==(const protobuf_document &left, const protobuf_document &right) {
            return static_cast<const BasicDocument &>(left) == static_cast<const BasicDocument &>(right);
        }

        friend bool operator!=(const protobuf_document &left, const protobuf_document &right) {
            return !(left == right);
        }
    };

    template <typename Concept, typename BasicDocument = document>
    using facade = protobuf_document<Concept, BasicDocument>;

    template <typename Concept, typename BasicDocument>
    byte_buffer encode(const BasicDocument &doc) {
        static_assert(protobuf_message<Concept>, "encode requires a protobuf concept type");
        byte_buffer out{};
        implements::protobuf_serializer<Concept, BasicDocument>::encode_into(doc, out);
        return out;
    }

    template <typename Concept, typename BasicDocument>
    byte_buffer encode(const protobuf_document<Concept, BasicDocument> &doc) {
        return encode<Concept>(doc.as_document());
    }

    template <typename Concept, typename BasicDocument = document>
    BasicDocument decode(const std::uint8_t *data, std::size_t size) {
        static_assert(protobuf_message<Concept>, "decode requires a protobuf concept type");
        BasicDocument doc(document_type::object);
        if (data == nullptr && size != 0) {
            implements::throw_protobuf_parse_error("null input with nonzero size");
        }
        if (size == 0) {
            return doc;
        }
        implements::protobuf_parser<Concept, BasicDocument>::decode_into(data, size, doc);
        return doc;
    }

    template <typename Concept, typename BasicDocument = document>
    BasicDocument decode(const byte_buffer &bytes) {
        if (bytes.empty()) {
            return BasicDocument(document_type::object);
        }
        return decode<Concept, BasicDocument>(bytes.data(), bytes.size());
    }

    template <typename Concept, typename BasicDocument = document>
    facade<Concept, BasicDocument> decode_facade(const std::uint8_t *data, std::size_t size) {
        return facade<Concept, BasicDocument>(decode<Concept, BasicDocument>(data, size));
    }

    template <typename Concept, typename BasicDocument = document>
    facade<Concept, BasicDocument> decode_facade(const byte_buffer &bytes) {
        return facade<Concept, BasicDocument>(decode<Concept, BasicDocument>(bytes));
    }

    template <typename Concept>
    byte_buffer encode_direct(const Concept &value) {
        static_assert(direct_message<Concept>, "encode_direct requires a concept type with member_field entries");
        byte_buffer out{};
        out.reserve(128);
        implements::direct_serializer<Concept>::encode_into(value, out);
        return out;
    }

    template <typename Concept>
    Concept decode_direct(const std::uint8_t *data, std::size_t size) {
        static_assert(direct_message<Concept>, "decode_direct requires a concept type with member_field entries");
        Concept value{};
        if (data == nullptr && size != 0) {
            implements::throw_protobuf_parse_error("null input with nonzero size");
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
    void decode_direct_into(const std::uint8_t *data, std::size_t size, Concept &value) {
        static_assert(direct_message<Concept>, "decode_direct_into requires a concept type with member_field entries");
        if (data == nullptr && size != 0) {
            implements::throw_protobuf_parse_error("null input with nonzero size");
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
