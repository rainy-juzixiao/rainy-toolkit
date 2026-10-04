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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_PARSER_HPP
#include <rainy/foundation/willow/implements/protobuf/codec.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/serializer.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf::implements {
    using namespace rainy::foundation::exceptions::willow::protobuf;

    template <typename BasicDocument>
    void assign_protobuf_integer(BasicDocument &slot, std::int64_t value) {
        using integer_type = typename BasicDocument::integer_type;
        if (value < static_cast<std::int64_t>(utility::numeric_limits<integer_type>::min()) ||
            value > static_cast<std::int64_t>(utility::numeric_limits<integer_type>::max())) {
            throw_protobuf_parse_error("integer value out of document range");
        }
        slot = BasicDocument(static_cast<integer_type>(value));
    }

    template <typename BasicDocument>
    void assign_protobuf_uinteger(BasicDocument &slot, std::uint64_t value) {
        using integer_type = typename BasicDocument::integer_type;
        if (value > static_cast<std::uint64_t>(utility::numeric_limits<integer_type>::max())) {
            throw_protobuf_parse_error("integer value out of document range");
        }
        slot = BasicDocument(static_cast<integer_type>(value));
    }

    template <typename BasicDocument>
    void assign_protobuf_string(BasicDocument &slot, const std::uint8_t *data, std::size_t size) {
        using char_type = typename BasicDocument::char_type;
        using string_type = typename BasicDocument::string_type;
        if constexpr (sizeof(char_type) != 1) {
            throw_protobuf_parse_error("protobuf string requires 1-byte character document");
        } else {
            string_type str{};
            str.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                str.push_back(static_cast<char_type>(data[i]));
            }
            slot = BasicDocument(str);
        }
    }

    template <typename Element, typename BasicDocument>
    void assign_packed_varint(std::uint64_t raw, BasicDocument &elem) {
        if constexpr (type_traits::type_relations::is_same_v<Element, proto::int32>) {
            assign_protobuf_integer(elem, static_cast<std::int64_t>(static_cast<std::int32_t>(static_cast<std::int64_t>(raw))));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::int64>) {
            assign_protobuf_integer(elem, static_cast<std::int64_t>(raw));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::uint32>) {
            if (raw > utility::numeric_limits<std::uint32_t>::max()) {
                throw_protobuf_parse_error("uint32 value out of range");
            }
            assign_protobuf_uinteger(elem, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::uint64>) {
            assign_protobuf_uinteger(elem, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sint32>) {
            assign_protobuf_integer(elem, decode_zigzag32(static_cast<std::uint32_t>(raw)));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sint64>) {
            assign_protobuf_integer(elem, decode_zigzag64(raw));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::boolean>) {
            elem = BasicDocument(raw != 0);
        } else {
            throw_protobuf_parse_error("unsupported packed varint type");
        }
    }

    template <typename Element, typename BasicDocument>
    void assign_packed_fixed32(std::uint32_t raw, BasicDocument &elem) {
        if constexpr (type_traits::type_relations::is_same_v<Element, proto::fixed32>) {
            assign_protobuf_uinteger(elem, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sfixed32>) {
            std::int32_t value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(std::int32_t));
            assign_protobuf_integer(elem, value);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::floating>) {
            float value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(float));
            elem = BasicDocument(static_cast<typename BasicDocument::float_type>(value));
        } else {
            throw_protobuf_parse_error("unsupported packed fixed32 type");
        }
    }

    template <typename Element, typename BasicDocument>
    void assign_packed_fixed64(std::uint64_t raw, BasicDocument &elem) {
        if constexpr (type_traits::type_relations::is_same_v<Element, proto::fixed64>) {
            if (raw > static_cast<std::uint64_t>(utility::numeric_limits<typename BasicDocument::integer_type>::max())) {
                throw_protobuf_parse_error("integer value out of document range");
            }
            assign_protobuf_uinteger(elem, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sfixed64>) {
            std::int64_t value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(std::int64_t));
            assign_protobuf_integer(elem, value);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::doubling>) {
            double value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(double));
            elem = BasicDocument(static_cast<typename BasicDocument::float_type>(value));
        } else {
            throw_protobuf_parse_error("unsupported packed fixed64 type");
        }
    }

    template <typename Kind, typename BasicDocument>
    void decode_single_scalar(const std::uint8_t *&ptr, const std::uint8_t *end, wire_type wire, BasicDocument &slot);

    template <typename Kind, typename BasicDocument>
    void decode_single_scalar_unchecked(const std::uint8_t *&ptr, const std::uint8_t *end, BasicDocument &slot);

    template <typename Concept, typename BasicDocument>
    struct protobuf_parser {
        static void decode_into(const std::uint8_t *data, std::size_t size, BasicDocument &doc) {
            static_assert(is_protobuf_message_v<Concept>, "decode requires a protobuf concept type");
            static_assert(valid_field_numbers_v<Concept>, "protobuf field numbers must be unique and within 1..536870911");
            if (doc.is_null()) {
                doc = BasicDocument(document_type::object);
            }
            if (!doc.is_object()) {
                throw_protobuf_parse_error("protobuf message requires object document");
            }
            const std::uint8_t *ptr = data;
            const std::uint8_t *end = data + size;
            while (ptr < end) {
                std::uint32_t number = 0;
                wire_type wire = wire_type::varint;
                if (!decode_tag(ptr, end, number, wire)) {
                    throw_protobuf_parse_error("truncated tag");
                }
                if (!dispatch_field<Concept>(number, wire, ptr, end, doc)) {
                    if (!skip_field(wire, ptr, end)) {
                        throw_protobuf_parse_error("truncated field payload");
                    }
                }
            }
            if (ptr != end) {
                throw_protobuf_parse_error("trailing bytes after message");
            }
        }

    private:
        template <typename Owner>
        static bool dispatch_field(std::uint32_t number, wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end,
                                   BasicDocument &doc) {
            return dispatch_fields<Owner>(number, wire, ptr, end, doc, std::make_index_sequence<field_count_v<Owner>>{});
        }

        template <typename Owner, std::size_t... Is>
        static bool dispatch_fields(std::uint32_t number, wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end,
                                    BasicDocument &doc, std::index_sequence<Is...>) {
            bool handled = false;
            ((number == field_number_v<Owner, Is> ? (decode_field<Owner, Is>(wire, ptr, end, doc), handled = true, 0) : 0),
             ...);
            return handled;
        }

        template <typename Owner, std::size_t Index>
        static void decode_field(wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end, BasicDocument &doc) {
            using field_type = field_at_t<Owner, Index>;
            using kind = typename field_type::kind;
            constexpr core::text::string_view name = field_name_of<field_type>();
            auto key = make_protobuf_key<BasicDocument>(name);
            if constexpr (is_repeated_kind_v<kind>) {
                BasicDocument &slot = doc[key];
                if (slot.is_null()) {
                    slot = BasicDocument(document_type::array);
                }
                if (!slot.is_array()) {
                    throw_protobuf_parse_error("protobuf repeated field requires array document");
                }
                decode_repeated<kind>(wire, ptr, end, slot);
            } else if constexpr (is_message_kind_v<kind>) {
                if (wire != wire_type::len) {
                    throw_protobuf_parse_error("message field requires LEN wire type");
                }
                std::size_t length = 0;
                if (!decode_length(ptr, end, length)) {
                    throw_protobuf_parse_error("truncated message length");
                }
                if (length > static_cast<std::size_t>(end - ptr)) {
                    throw_protobuf_parse_error("truncated message payload");
                }
                BasicDocument &slot = doc[key];
                if (slot.is_null()) {
                    slot = BasicDocument(document_type::object);
                }
                if (!slot.is_object()) {
                    throw_protobuf_parse_error("protobuf message field requires object document");
                }
                protobuf_parser<message_concept_t<kind>, BasicDocument>::decode_into(ptr, length, slot);
                ptr += length;
            } else {
                BasicDocument &slot = doc[key];
                decode_single_scalar<kind>(ptr, end, wire, slot);
            }
        }

        template <typename Repeated>
        static void decode_repeated(wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end, BasicDocument &array) {
            using element = repeated_element_t<Repeated>;
            if constexpr (is_message_kind_v<element>) {
                if (wire != wire_type::len) {
                    throw_protobuf_parse_error("message element requires LEN wire type");
                }
                std::size_t length = 0;
                if (!decode_length(ptr, end, length)) {
                    throw_protobuf_parse_error("truncated message length");
                }
                if (length > static_cast<std::size_t>(end - ptr)) {
                    throw_protobuf_parse_error("truncated message payload");
                }
                BasicDocument elem(document_type::object);
                protobuf_parser<message_concept_t<element>, BasicDocument>::decode_into(ptr, length, elem);
                ptr += length;
                array.as_array().push_back(utility::move(elem));
            } else if constexpr (type_traits::type_relations::is_same_v<element, proto::string> || type_traits::type_relations::is_same_v<element, proto::bytes>) {
                if (wire != wire_type::len) {
                    throw_protobuf_parse_error("string element requires LEN wire type");
                }
                BasicDocument elem{};
                decode_single_scalar_unchecked<element>(ptr, end, elem);
                array.as_array().push_back(utility::move(elem));
            } else {
                if (constexpr wire_type element_wire = wire_of_v<element>; wire == element_wire) {
                    BasicDocument elem{};
                    decode_single_scalar_unchecked<element>(ptr, end, elem);
                    array.as_array().push_back(utility::move(elem));
                } else if (wire == wire_type::len && is_packable_v<element>) {
                    std::size_t length = 0;
                    if (!decode_length(ptr, end, length)) {
                        throw_protobuf_parse_error("truncated packed length");
                    }
                    if (length > static_cast<std::size_t>(end - ptr)) {
                        throw_protobuf_parse_error("truncated packed payload");
                    }
                    const std::uint8_t *block_end = ptr + length;
                    while (ptr < block_end) {
                        BasicDocument elem{};
                        decode_packed_scalar<element>(ptr, block_end, elem);
                        array.as_array().push_back(utility::move(elem));
                    }
                } else {
                    throw_protobuf_parse_error("unexpected wire type for repeated field");
                }
            }
        }

        template <typename Element>
        static void decode_packed_scalar(const std::uint8_t *&ptr, const std::uint8_t *end, BasicDocument &elem) {
            if constexpr (constexpr wire_type element_wire = wire_of_v<Element>; element_wire == wire_type::varint) {
                std::uint64_t raw = 0;
                if (!decode_varint(ptr, end, raw)) {
                    throw_protobuf_parse_error("truncated packed varint");
                }
                assign_packed_varint<Element>(raw, elem);
            } else if constexpr (element_wire == wire_type::i32) {
                std::uint32_t raw = 0;
                if (!decode_fixed32(ptr, end, raw)) {
                    throw_protobuf_parse_error("truncated packed fixed32");
                }
                assign_packed_fixed32<Element>(raw, elem);
            } else if constexpr (element_wire == wire_type::i64) {
                std::uint64_t raw = 0;
                if (!decode_fixed64(ptr, end, raw)) {
                    throw_protobuf_parse_error("truncated packed fixed64");
                }
                assign_packed_fixed64<Element>(raw, elem);
            } else {
                throw_protobuf_parse_error("element type is not packable");
            }
        }

    };

    template <typename Kind, typename BasicDocument>
    void decode_single_scalar_unchecked(const std::uint8_t *&ptr, const std::uint8_t *end, BasicDocument &slot) {
        if constexpr (wire_of_v<Kind> == wire_type::varint) {
            std::uint64_t raw = 0;
            if (!decode_varint(ptr, end, raw)) {
                throw_protobuf_parse_error("truncated varint payload");
            }
            assign_packed_varint<Kind>(raw, slot);
        } else if constexpr (wire_of_v<Kind> == wire_type::i32) {
            std::uint32_t raw = 0;
            if (!decode_fixed32(ptr, end, raw)) {
                throw_protobuf_parse_error("truncated fixed32 payload");
            }
            assign_packed_fixed32<Kind>(raw, slot);
        } else if constexpr (wire_of_v<Kind> == wire_type::i64) {
            std::uint64_t raw = 0;
            if (!decode_fixed64(ptr, end, raw)) {
                throw_protobuf_parse_error("truncated fixed64 payload");
            }
            assign_packed_fixed64<Kind>(raw, slot);
        } else {
            std::size_t length = 0;
            if (!decode_length(ptr, end, length)) {
                throw_protobuf_parse_error("truncated length prefix");
            }
            if (length > static_cast<std::size_t>(end - ptr)) {
                throw_protobuf_parse_error("truncated length payload");
            }
            assign_protobuf_string(slot, ptr, length);
            ptr += length;
        }
    }

    template <typename Kind, typename BasicDocument>
    void decode_single_scalar(const std::uint8_t *&ptr, const std::uint8_t *end, wire_type wire, BasicDocument &slot) {
        if (wire != wire_of_v<Kind>) {
            throw_protobuf_parse_error("unexpected wire type for field");
        }
        decode_single_scalar_unchecked<Kind>(ptr, end, slot);
    }
}

#endif

#endif
