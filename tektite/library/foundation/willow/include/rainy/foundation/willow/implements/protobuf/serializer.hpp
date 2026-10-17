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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_SERIALIZER_HPP
#include <rainy/foundation/willow/implements/protobuf/codec.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>
#include <utility>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf::implements {
    using namespace rainy::foundation::exceptions::willow::protobuf;

    template <typename Lhs, typename Rhs>
    constexpr bool protobuf_cmp_greater(Lhs lhs, Rhs rhs) noexcept {
#if RAINY_HAS_CXX20
        return std::cmp_greater(lhs, rhs);
#else
        if constexpr (type_traits::properties::is_signed_v<Lhs> == type_traits::properties::is_signed_v<Rhs>) {
            return lhs > rhs;
        } else if constexpr (type_traits::properties::is_signed_v<Lhs>) {
            using unsigned_lhs = typename type_traits::helper::make_unsigned<Lhs>::type;
            return lhs < 0 ? false : static_cast<unsigned_lhs>(lhs) > static_cast<unsigned_lhs>(rhs);
        } else {
            using unsigned_rhs = typename type_traits::helper::make_unsigned<Rhs>::type;
            return rhs < 0 ? true : static_cast<unsigned_rhs>(lhs) > static_cast<unsigned_rhs>(rhs);
        }
#endif
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type make_protobuf_key(core::text::string_view name) {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        string_type key{};
        key.reserve(name.size());
        for (char ch: name) {
            key.push_back(static_cast<char_type>(ch));
        }
        return key;
    }

    template <typename BasicDocument>
    const BasicDocument *find_protobuf_node(const BasicDocument &doc, core::text::string_view name) {
        if (!doc.is_object()) {
            return nullptr;
        }
        auto key = make_protobuf_key<BasicDocument>(name);
        auto it = doc.find(key);
        if (it == doc.cend()) {
            return nullptr;
        }
        return &(it->value());
    }

    template <typename BasicDocument>
    void append_string_bytes(const BasicDocument &node, byte_buffer &payload) {
        using char_type = typename BasicDocument::char_type;
        if constexpr (sizeof(char_type) != 1) {
            throw_protobuf_serialize_error("protobuf string requires 1-byte character document");
        } else {
            const auto &str = node.as_string();
            const auto *data = reinterpret_cast<const std::uint8_t *>(str.data());
            payload.insert(payload.end(), data, data + str.size());
        }
    }

    template <typename Kind, typename BasicDocument>
    void encode_scalar_value(const BasicDocument &node, byte_buffer &out);

    template <typename BasicDocument, typename Nested>
    void encode_nested_message(const BasicDocument &node, byte_buffer &out);

    template <typename Concept, typename BasicDocument>
    struct protobuf_serializer {
        static void encode_into(const BasicDocument &doc, byte_buffer &out) {
            static_assert(is_protobuf_message_v<Concept>, "encode requires a protobuf concept type");
            static_assert(valid_field_numbers_v<Concept>, "protobuf field numbers must be unique and within 1..536870911");
            if (doc.is_null()) {
                return;
            }
            if (!doc.is_object()) {
                throw_protobuf_serialize_error("protobuf message requires object document");
            }
            encode_fields<Concept>(doc, out, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

    private:
        template <typename Owner, std::size_t... Is>
        static void encode_fields(const BasicDocument &doc, byte_buffer &out, type_traits::helper::index_sequence<Is...>) {
            (encode_field<Owner, Is>(doc, out), ...);
        }

        template <typename Owner, std::size_t Index>
        static void encode_field(const BasicDocument &doc, byte_buffer &out) {
            using field_type = field_at_t<Owner, Index>;
            using kind = typename field_type::kind;
            constexpr std::uint32_t number = field_number_v<Owner, Index>;
            constexpr core::text::string_view name = field_name_of<field_type>();
            const BasicDocument *node = find_protobuf_node(doc, name);
            if (node == nullptr || node->is_null()) {
                return;
            }
            if constexpr (is_repeated_kind_v<kind>) {
                encode_repeated<kind>(*node, number, out);
            } else if constexpr (is_message_kind_v<kind>) {
                encode_tag(number, wire_type::len, out);
                byte_buffer nested{};
                encode_nested_message<BasicDocument, message_concept_t<kind>>(*node, nested);
                encode_length(nested.size(), out);
                out.insert(out.end(), nested.begin(), nested.end());
            } else {
                encode_tag(number, wire_of_v<kind>, out);
                encode_scalar_value<kind>(*node, out);
            }
        }

        template <typename Repeated, typename BasicDocument_>
        static void encode_repeated(const BasicDocument_ &node, const std::uint32_t number, byte_buffer &out) {
            using element = repeated_element_t<Repeated>;
            if (!node.is_array()) {
                throw_protobuf_serialize_error("protobuf repeated field requires array document");
            }
            for (const auto &elem: node.as_array()) {
                if constexpr (is_message_kind_v<element>) {
                    encode_tag(number, wire_type::len, out);
                    byte_buffer nested{};
                    encode_nested_message<BasicDocument_, message_concept_t<element>>(elem, nested);
                    encode_length(nested.size(), out);
                    out.insert(out.end(), nested.begin(), nested.end());
                } else {
                    encode_tag(number, wire_of_v<element>, out);
                    encode_scalar_value<element>(elem, out);
                }
            }
        }
    };

    template <typename BasicDocument, typename Nested>
    void encode_nested_message(const BasicDocument &node, byte_buffer &out) {
        if (!node.is_object()) {
            throw_protobuf_serialize_error("protobuf message field requires object document");
        }
        protobuf_serializer<Nested, BasicDocument>::encode_into(node, out);
    }

    template <typename Kind, typename BasicDocument>
    void encode_scalar_value(const BasicDocument &node, byte_buffer &out) {
        if constexpr (type_traits::type_relations::is_same_v<Kind, proto::int32>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("int32 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < utility::numeric_limits<std::int32_t>::min() || raw > utility::numeric_limits<std::int32_t>::max()) {
                throw_protobuf_serialize_error("int32 value out of range");
            }
            encode_varint(static_cast<std::uint64_t>(raw), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::int64>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("int64 requires integer document");
            }
            encode_varint(static_cast<std::uint64_t>(static_cast<std::int64_t>(node.as_integer())), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::uint32>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("uint32 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < 0 || protobuf_cmp_greater(raw ,utility::numeric_limits<std::uint32_t>::max())) {
                throw_protobuf_serialize_error("uint32 value out of range");
            }
            encode_varint(static_cast<std::uint64_t>(raw), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::uint64>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("uint64 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < 0) {
                throw_protobuf_serialize_error("uint64 value out of range");
            }
            encode_varint(static_cast<std::uint64_t>(raw), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sint32>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("sint32 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < utility::numeric_limits<std::int32_t>::min() || raw > utility::numeric_limits<std::int32_t>::max()) {
                throw_protobuf_serialize_error("sint32 value out of range");
            }
            encode_varint(encode_zigzag32(static_cast<std::int32_t>(raw)), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sint64>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("sint64 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            encode_varint(encode_zigzag64(static_cast<std::int64_t>(raw)), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::boolean>) {
            if (!node.is_bool()) {
                throw_protobuf_serialize_error("bool requires boolean document");
            }
            encode_varint(node.as_bool() ? 1u : 0u, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::fixed32>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("fixed32 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < 0 || protobuf_cmp_greater(raw ,utility::numeric_limits<std::uint32_t>::max())) {
                throw_protobuf_serialize_error("fixed32 value out of range");
            }
            encode_fixed32(static_cast<std::uint32_t>(raw), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sfixed32>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("sfixed32 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < utility::numeric_limits<std::int32_t>::min() || raw > utility::numeric_limits<std::int32_t>::max()) {
                throw_protobuf_serialize_error("sfixed32 value out of range");
            }
            std::uint32_t bits = 0;
            core::builtin::copy_memory(&bits, &raw, sizeof(std::int32_t));
            encode_fixed32(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::floating>) {
            if (!node.is_number()) {
                throw_protobuf_serialize_error("float requires numeric document");
            }
            const auto value = node.is_integer() ? static_cast<float>(node.as_integer()) : node.as_float();
            std::uint32_t bits = 0;
            core::builtin::copy_memory(&bits, &value, sizeof(float));
            encode_fixed32(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::fixed64>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("fixed64 requires integer document");
            }
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            if (raw < 0) {
                throw_protobuf_serialize_error("fixed64 value out of range");
            }
            encode_fixed64(static_cast<std::uint64_t>(raw), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sfixed64>) {
            if (!node.is_integer()) {
                throw_protobuf_serialize_error("sfixed64 requires integer document");
            }
            std::uint64_t bits = 0;
            const auto raw = static_cast<std::int64_t>(node.as_integer());
            core::builtin::copy_memory(&bits, &raw, sizeof(std::int64_t));
            encode_fixed64(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::doubling>) {
            if (!node.is_number()) {
                throw_protobuf_serialize_error("double requires numeric document");
            }
            const auto value = node.is_integer() ? static_cast<double>(node.as_integer()) : node.as_float();
            std::uint64_t bits = 0;
            core::builtin::copy_memory(&bits, &value, sizeof(double));
            encode_fixed64(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::string> ||
                             type_traits::type_relations::is_same_v<Kind, proto::bytes>) {
            if (!node.is_string()) {
                throw_protobuf_serialize_error("string/bytes requires string document");
            }
            byte_buffer payload{};
            append_string_bytes<BasicDocument>(node, payload);
            encode_length(payload.size(), out);
            out.insert(out.end(), payload.begin(), payload.end());
        } else {
            static_assert(!type_traits::type_relations::is_same_v<Kind, Kind>, "unsupported protobuf kind");
        }
    }
}

#endif

#endif
