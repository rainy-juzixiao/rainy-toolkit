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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_DIRECT_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_DIRECT_HPP
#include <rainy/foundation/willow/implements/protobuf/codec.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <type_traits>
#include <utility>

#if RAINY_HAS_CXX20

namespace rainy::foundation::willow::protobuf {
    template <typename Concept, std::size_t Index>
    struct direct_field_at {
        using type = std::tuple_element_t<Index, typename message_fields<Concept>::type>;
        static_assert(is_direct_field_v<type>, "direct protobuf_fields must only contain member_field<...> entries");
    };

    template <typename Concept, std::size_t Index>
    using direct_field_at_t = typename direct_field_at<Concept, Index>::type;

    template <typename Owner, typename Member>
    Owner direct_owner_of(Member Owner::*) {
        return Owner{};
    }

    template <typename Concept, std::size_t Index>
    constexpr bool direct_field_owner_ok() {
        using field_type = direct_field_at_t<Concept, Index>;
        return type_traits::type_relations::is_same_v<decltype(direct_owner_of(field_type::member)), Concept>;
    }

    template <typename Concept, std::size_t... Is>
    constexpr bool check_direct_fields(type_traits::helper::index_sequence<Is...>) {
        return ((is_direct_field_v<direct_field_at_t<Concept, Is>> && direct_field_owner_ok<Concept, Is>()) && ...);
    }

    template <typename Kind, typename Ty>
    struct direct_string_like {
    private:
        using raw = type_traits::modifers::remove_cvref_t<Ty>;

    public:
        static constexpr bool value = requires(const raw &v) {
            typename raw::value_type;
            { v.data() } -> std::convertible_to<const typename raw::value_type *>;
            { v.size() } -> std::convertible_to<std::size_t>;
            { v.empty() } -> std::convertible_to<bool>;
        } && sizeof(typename raw::value_type) == 1;
    };

    template <typename Kind, typename Ty>
    inline constexpr bool direct_string_like_v = direct_string_like<Kind, Ty>::value;

    template <typename Kind, typename Ty>
    struct direct_assignable_string {
    private:
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        using value_type = typename raw::value_type;

    public:
        static constexpr bool value = direct_string_like_v<Kind, Ty> && requires(raw &v, const value_type *p,
                                                                                 std::size_t n) { v.assign(p, n); };
    };

    template <typename Kind, typename Ty>
    inline constexpr bool direct_assignable_string_v = direct_assignable_string<Kind, Ty>::value;

    template <typename Kind, typename Ty>
    struct direct_container {
    private:
        using raw = type_traits::modifers::remove_cvref_t<Ty>;

    public:
        static constexpr bool value = requires(raw &v) {
            typename raw::value_type;
            { v.size() } -> std::convertible_to<std::size_t>;
            { v.empty() } -> std::convertible_to<bool>;
            { v.begin() };
            { v.end() };
            { v.emplace_back() } -> std::convertible_to<typename raw::value_type &>;
        };
    };

    template <typename Kind, typename Ty>
    inline constexpr bool direct_container_v = direct_container<Kind, Ty>::value;
}

namespace rainy::foundation::willow::protobuf {
    template <typename Ty>
    concept direct_message = requires { Ty::protobuf_fields; } &&
                             (std::tuple_size_v<type_traits::modifers::remove_cvref_t<decltype(Ty::protobuf_fields)>> >= 0) &&
                             check_direct_fields<Ty>(type_traits::helper::make_index_sequence<field_count_v<Ty>>{}) &&
                             valid_field_numbers_v<Ty>;
}

namespace rainy::foundation::willow::protobuf::implements {
    using namespace rainy::foundation::exceptions::willow::protobuf;

    using protobuf::byte_buffer;

    template <typename Concept>
    bool direct_message_is_default(const Concept &value);

    template <typename Kind, typename Ty>
    bool direct_member_is_default(const Ty &member) {
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        if constexpr (is_repeated_kind_v<Kind>) {
            static_assert(direct_container_v<Kind, Ty>, "protobuf repeated member requires a container with emplace_back()");
            return member.empty();
        } else if constexpr (is_message_kind_v<Kind>) {
            static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                          "protobuf message member type must match message<Concept>");
            return direct_message_is_default<message_concept_t<Kind>>(member);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::string> ||
                             type_traits::type_relations::is_same_v<Kind, proto::bytes>) {
            static_assert(direct_string_like_v<Kind, Ty>, "protobuf string/bytes member requires data()/size()/empty()");
            return member.empty();
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::boolean>) {
            static_assert(type_traits::type_relations::is_same_v<raw, bool>, "protobuf bool member requires bool");
            return !member;
        } else {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "protobuf scalar member requires an arithmetic type");
            return member == static_cast<raw>(0);
        }
    }

    template <typename Concept, std::size_t... Is>
    bool direct_message_is_default_impl(const Concept &value, type_traits::helper::index_sequence<Is...>) {
        return (direct_member_is_default<typename direct_field_at_t<Concept, Is>::kind>(
                    value.*(direct_field_at_t<Concept, Is>::member)) &&
                ...);
    }

    template <typename Concept>
    bool direct_message_is_default(const Concept &value) {
        static_assert(direct_message<Concept>, "direct mode requires a concept type with member_field entries");
        return direct_message_is_default_impl(value, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
    }

    template <typename Kind, typename Ty>
    void encode_direct_scalar(const Ty &member, byte_buffer &out) {
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        if constexpr (type_traits::type_relations::is_same_v<Kind, proto::int32>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "int32 member requires an arithmetic type");
            const auto value = static_cast<std::int64_t>(member);
            if (value < utility::numeric_limits<std::int32_t>::min() || value > utility::numeric_limits<std::int32_t>::max()) {
                throw_protobuf_serialize_error("int32 value out of range");
            }
            encode_varint(static_cast<std::uint64_t>(value), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::int64>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "int64 member requires an arithmetic type");
            const auto value = static_cast<std::int64_t>(member);
            if constexpr (type_traits::properties::is_unsigned_v<raw>) {
                if (static_cast<std::uint64_t>(member) >
                    static_cast<std::uint64_t>(utility::numeric_limits<std::int64_t>::max())) {
                    throw_protobuf_serialize_error("int64 value out of range");
                }
            }
            encode_varint(static_cast<std::uint64_t>(value), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::uint32>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "uint32 member requires an arithmetic type");
            if (member < static_cast<raw>(0) ||
                static_cast<std::uint64_t>(member) > utility::numeric_limits<std::uint32_t>::max()) {
                throw_protobuf_serialize_error("uint32 value out of range");
            }
            encode_varint(static_cast<std::uint64_t>(member), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::uint64>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "uint64 member requires an arithmetic type");
            if (member < static_cast<raw>(0)) {
                throw_protobuf_serialize_error("uint64 value out of range");
            }
            encode_varint(static_cast<std::uint64_t>(member), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sint32>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "sint32 member requires an arithmetic type");
            const auto value = static_cast<std::int64_t>(member);
            if (value < utility::numeric_limits<std::int32_t>::min() || value > utility::numeric_limits<std::int32_t>::max()) {
                throw_protobuf_serialize_error("sint32 value out of range");
            }
            encode_varint(encode_zigzag32(static_cast<std::int32_t>(value)), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sint64>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "sint64 member requires an arithmetic type");
            const auto value = static_cast<std::int64_t>(member);
            if constexpr (type_traits::properties::is_unsigned_v<raw>) {
                if (static_cast<std::uint64_t>(member) >
                    static_cast<std::uint64_t>(utility::numeric_limits<std::int64_t>::max())) {
                    throw_protobuf_serialize_error("sint64 value out of range");
                }
            }
            encode_varint(encode_zigzag64(value), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::boolean>) {
            static_assert(type_traits::type_relations::is_same_v<raw, bool>, "protobuf bool member requires bool");
            encode_varint(member ? 1u : 0u, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::fixed32>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "fixed32 member requires an arithmetic type");
            if (member < static_cast<raw>(0) ||
                static_cast<std::uint64_t>(member) > utility::numeric_limits<std::uint32_t>::max()) {
                throw_protobuf_serialize_error("fixed32 value out of range");
            }
            encode_fixed32(static_cast<std::uint32_t>(member), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sfixed32>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "sfixed32 member requires an arithmetic type");
            const auto value = static_cast<std::int64_t>(member);
            if (value < utility::numeric_limits<std::int32_t>::min() || value > utility::numeric_limits<std::int32_t>::max()) {
                throw_protobuf_serialize_error("sfixed32 value out of range");
            }
            std::uint32_t bits = 0;
            const auto narrowed = static_cast<std::int32_t>(value);
            core::builtin::copy_memory(&bits, &narrowed, sizeof(std::int32_t));
            encode_fixed32(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::floating>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "float member requires an arithmetic type");
            const auto value = static_cast<float>(member);
            std::uint32_t bits = 0;
            core::builtin::copy_memory(&bits, &value, sizeof(float));
            encode_fixed32(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::fixed64>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "fixed64 member requires an arithmetic type");
            if (member < static_cast<raw>(0)) {
                throw_protobuf_serialize_error("fixed64 value out of range");
            }
            encode_fixed64(static_cast<std::uint64_t>(member), out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sfixed64>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "sfixed64 member requires an arithmetic type");
            const auto value = static_cast<std::int64_t>(member);
            if constexpr (std::is_unsigned_v<raw>) {
                if (static_cast<std::uint64_t>(member) >
                    static_cast<std::uint64_t>(utility::numeric_limits<std::int64_t>::max())) {
                    throw_protobuf_serialize_error("sfixed64 value out of range");
                }
            }
            std::uint64_t bits = 0;
            core::builtin::copy_memory(&bits, &value, sizeof(std::int64_t));
            encode_fixed64(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::doubling>) {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>, "double member requires an arithmetic type");
            const auto value = static_cast<double>(member);
            std::uint64_t bits = 0;
            core::builtin::copy_memory(&bits, &value, sizeof(double));
            encode_fixed64(bits, out);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::string> ||
                             type_traits::type_relations::is_same_v<Kind, proto::bytes>) {
            static_assert(direct_string_like_v<Kind, Ty>, "protobuf string/bytes member requires data()/size()/empty()");
            using value_type = typename type_traits::modifers::remove_cvref_t<Ty>::value_type;
            static_assert(sizeof(value_type) == 1, "protobuf string/bytes member requires 1-byte characters");
            encode_length(member.size(), out);
            const auto *data = reinterpret_cast<const std::uint8_t *>(member.data());
            out.insert(out.end(), data, data + member.size());
        } else {
            static_assert(!type_traits::type_relations::is_same_v<Kind, Kind>, "unsupported protobuf kind");
        }
    }

    template <typename Concept, std::size_t Index>
    std::size_t direct_field_measure(const Concept &value);

    template <typename Kind, typename Ty>
    std::size_t measure_direct_scalar(const Ty &member) {
        if constexpr (type_traits::type_relations::is_same_v<Kind, proto::int32> ||
                      type_traits::type_relations::is_same_v<Kind, proto::int64> ||
                      type_traits::type_relations::is_same_v<Kind, proto::uint32> ||
                      type_traits::type_relations::is_same_v<Kind, proto::uint64>) {
            return varint_size(static_cast<std::uint64_t>(static_cast<std::int64_t>(member)));
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sint32>) {
            return varint_size(encode_zigzag32(static_cast<std::int32_t>(static_cast<std::int64_t>(member))));
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::sint64>) {
            return varint_size(encode_zigzag64(static_cast<std::int64_t>(member)));
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::boolean>) {
            return 1;
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::fixed32> ||
                             type_traits::type_relations::is_same_v<Kind, proto::sfixed32> ||
                             type_traits::type_relations::is_same_v<Kind, proto::floating>) {
            return 4;
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::fixed64> ||
                             type_traits::type_relations::is_same_v<Kind, proto::sfixed64> ||
                             type_traits::type_relations::is_same_v<Kind, proto::doubling>) {
            return 8;
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, proto::string> ||
                             type_traits::type_relations::is_same_v<Kind, proto::bytes>) {
            return varint_size(static_cast<std::uint64_t>(member.size())) + member.size();
        } else {
            static_assert(!type_traits::type_relations::is_same_v<Kind, Kind>, "unsupported protobuf kind");
            return 0;
        }
    }

    template <typename Concept, std::size_t... Is>
    std::size_t direct_message_measure_impl(const Concept &value, type_traits::helper::index_sequence<Is...>) {
        std::size_t size = 0;
        ((size += direct_field_measure<Concept, Is>(value)), ...);
        return size;
    }

    template <typename Concept>
    std::size_t direct_message_measure(const Concept &value);

    template <typename Concept, std::size_t Index>
    std::size_t direct_field_measure(const Concept &value) {
        using field_type = direct_field_at_t<Concept, Index>;
        using kind = typename field_type::kind;
        constexpr std::uint32_t number = field_number_v<Concept, Index>;
        const auto &member = value.*(field_type::member);
        if (direct_member_is_default<kind>(member)) {
            return 0;
        }
        if constexpr (is_repeated_kind_v<kind>) {
            using element = repeated_element_t<kind>;
            std::size_t size = 0;
            for (const auto &elem : member) {
                if constexpr (is_message_kind_v<element>) {
                    const std::size_t nested = direct_message_measure<message_concept_t<element>>(elem);
                    size += encode_tag_size(number, wire_type::len) + varint_size(nested) + nested;
                } else {
                    size += encode_tag_size(number, wire_of_v<element>) + measure_direct_scalar<element>(elem);
                }
            }
            return size;
        } else if constexpr (is_message_kind_v<kind>) {
            const std::size_t nested = direct_message_measure<message_concept_t<kind>>(member);
            return encode_tag_size(number, wire_type::len) + varint_size(nested) + nested;
        } else {
            return encode_tag_size(number, wire_of_v<kind>) + measure_direct_scalar<kind>(member);
        }
    }

    template <typename Concept>
    std::size_t direct_message_measure(const Concept &value) {
        static_assert(direct_message<Concept>, "direct mode requires a concept type with member_field entries");
        return direct_message_measure_impl(value, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
    }

    template <typename Concept>
    struct direct_serializer {
        static void encode_into(const Concept &value, byte_buffer &out) {
            static_assert(direct_message<Concept>, "encode_direct requires a concept type with member_field entries");
            static_assert(valid_field_numbers_v<Concept>, "protobuf field numbers must be unique and within 1..536870911");
            encode_fields(value, out, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

    private:
        template <std::size_t... Is>
        static void encode_fields(const Concept &value, byte_buffer &out, type_traits::helper::index_sequence<Is...>) {
            (encode_field<Is>(value, out), ...);
        }

        template <std::size_t Index>
        static void encode_field(const Concept &value, byte_buffer &out) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            constexpr std::uint32_t number = field_number_v<Concept, Index>;
            const auto &member = value.*(field_type::member);
            if (direct_member_is_default<kind>(member)) {
                return;
            }
            if constexpr (is_repeated_kind_v<kind>) {
                encode_repeated<kind>(member, number, out);
            } else if constexpr (is_message_kind_v<kind>) {
                encode_tag(number, wire_type::len, out);
                byte_buffer nested{};
                nested.reserve(direct_message_measure<message_concept_t<kind>>(member));
                direct_serializer<message_concept_t<kind>>::encode_into(member, nested);
                encode_length(nested.size(), out);
                out.insert(out.end(), nested.begin(), nested.end());
            } else {
                encode_tag(number, wire_of_v<kind>, out);
                encode_direct_scalar<kind>(member, out);
            }
        }

        template <typename Repeated, typename Container>
        static void encode_repeated(const Container &members, const std::uint32_t number, byte_buffer &out) {
            using element = repeated_element_t<Repeated>;
            for (const auto &elem : members) {
                if constexpr (is_message_kind_v<element>) {
                    encode_tag(number, wire_type::len, out);
                    byte_buffer nested{};
                    nested.reserve(direct_message_measure<message_concept_t<element>>(elem));
                    direct_serializer<message_concept_t<element>>::encode_into(elem, nested);
                    encode_length(nested.size(), out);
                    out.insert(out.end(), nested.begin(), nested.end());
                } else {
                    encode_tag(number, wire_of_v<element>, out);
                    encode_direct_scalar<element>(elem, out);
                }
            }
        }
    };

    template <typename Ty>
    void assign_direct_integer(Ty &slot, std::int64_t value) {
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        static_assert(type_traits::composite_types::is_arithmetic_v<raw> && !type_traits::type_relations::is_same_v<raw, bool>,
                      "protobuf integer member requires a non-bool arithmetic type");
        if constexpr (type_traits::primary_types::is_floating_point_v<raw>) {
            slot = static_cast<raw>(value);
        } else {
            if (value < static_cast<std::int64_t>(utility::numeric_limits<raw>::min()) ||
                value > static_cast<std::int64_t>(utility::numeric_limits<raw>::max())) {
                throw_protobuf_parse_error("integer value out of member range");
            }
            slot = static_cast<raw>(value);
        }
    }

    template <typename Ty>
    void assign_direct_uinteger(Ty &slot, std::uint64_t value) {
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        static_assert(type_traits::composite_types::is_arithmetic_v<raw> && !type_traits::type_relations::is_same_v<raw, bool>,
                      "protobuf integer member requires a non-bool arithmetic type");
        if constexpr (type_traits::primary_types::is_floating_point_v<raw>) {
            slot = static_cast<raw>(value);
        } else if constexpr (type_traits::properties::is_signed_v<raw>) {
            if (value > static_cast<std::uint64_t>(utility::numeric_limits<raw>::max())) {
                throw_protobuf_parse_error("integer value out of member range");
            }
            slot = static_cast<raw>(value);
        } else {
            if (value > static_cast<std::uint64_t>(utility::numeric_limits<raw>::max())) {
                throw_protobuf_parse_error("integer value out of member range");
            }
            slot = static_cast<raw>(value);
        }
    }

    template <typename Element, typename Ty>
    void assign_direct_packed_varint(std::uint64_t raw, Ty &slot) {
        if constexpr (type_traits::type_relations::is_same_v<Element, proto::int32>) {
            assign_direct_integer(slot, static_cast<std::int64_t>(static_cast<std::int32_t>(static_cast<std::int64_t>(raw))));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::int64>) {
            assign_direct_integer(slot, static_cast<std::int64_t>(raw));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::uint32>) {
            if (raw > utility::numeric_limits<std::uint32_t>::max()) {
                throw_protobuf_parse_error("uint32 value out of range");
            }
            assign_direct_uinteger(slot, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::uint64>) {
            assign_direct_uinteger(slot, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sint32>) {
            assign_direct_integer(slot, decode_zigzag32(static_cast<std::uint32_t>(raw)));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sint64>) {
            assign_direct_integer(slot, decode_zigzag64(raw));
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::boolean>) {
            using member_raw = type_traits::modifers::remove_cvref_t<Ty>;
            static_assert(type_traits::type_relations::is_same_v<member_raw, bool>, "protobuf bool member requires bool");
            slot = (raw != 0);
        } else {
            throw_protobuf_parse_error("unsupported packed varint type");
        }
    }

    template <typename Element, typename Ty>
    void assign_direct_packed_fixed32(std::uint32_t raw, Ty &slot) {
        if constexpr (type_traits::type_relations::is_same_v<Element, proto::fixed32>) {
            assign_direct_uinteger(slot, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sfixed32>) {
            std::int32_t value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(std::int32_t));
            assign_direct_integer(slot, value);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::floating>) {
            float value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(float));
            slot = static_cast<type_traits::modifers::remove_cvref_t<Ty>>(value);
        } else {
            throw_protobuf_parse_error("unsupported packed fixed32 type");
        }
    }

    template <typename Element, typename Ty>
    void assign_direct_packed_fixed64(std::uint64_t raw, Ty &slot) {
        if constexpr (type_traits::type_relations::is_same_v<Element, proto::fixed64>) {
            assign_direct_uinteger(slot, raw);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::sfixed64>) {
            std::int64_t value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(std::int64_t));
            assign_direct_integer(slot, value);
        } else if constexpr (type_traits::type_relations::is_same_v<Element, proto::doubling>) {
            double value = 0;
            core::builtin::copy_memory(&value, &raw, sizeof(double));
            slot = static_cast<type_traits::modifers::remove_cvref_t<Ty>>(value);
        } else {
            throw_protobuf_parse_error("unsupported packed fixed64 type");
        }
    }

    template <typename Kind, typename Ty>
    void decode_direct_single_unchecked(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &slot) {
        if constexpr (wire_of_v<Kind> == wire_type::varint) {
            std::uint64_t raw = 0;
            if (!decode_varint(ptr, end, raw)) {
                throw_protobuf_parse_error("truncated varint payload");
            }
            assign_direct_packed_varint<Kind>(raw, slot);
        } else if constexpr (wire_of_v<Kind> == wire_type::i32) {
            std::uint32_t raw = 0;
            if (!decode_fixed32(ptr, end, raw)) {
                throw_protobuf_parse_error("truncated fixed32 payload");
            }
            assign_direct_packed_fixed32<Kind>(raw, slot);
        } else if constexpr (wire_of_v<Kind> == wire_type::i64) {
            std::uint64_t raw = 0;
            if (!decode_fixed64(ptr, end, raw)) {
                throw_protobuf_parse_error("truncated fixed64 payload");
            }
            assign_direct_packed_fixed64<Kind>(raw, slot);
        } else {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            static_assert(direct_assignable_string_v<Kind, Ty>,
                          "protobuf string/bytes member requires assign(pointer, size)");
            std::size_t length = 0;
            if (!decode_length(ptr, end, length)) {
                throw_protobuf_parse_error("truncated length prefix");
            }
            if (length > static_cast<std::size_t>(end - ptr)) {
                throw_protobuf_parse_error("truncated length payload");
            }
            using value_type = typename raw::value_type;
            static_assert(sizeof(value_type) == 1, "protobuf string/bytes member requires 1-byte characters");
            slot.assign(reinterpret_cast<const value_type *>(ptr), length);
            ptr += length;
        }
    }

    template <typename Kind, typename Ty>
    void decode_direct_single(const std::uint8_t *&ptr, const std::uint8_t *end, wire_type wire, Ty &slot) {
        if (wire != wire_of_v<Kind>) {
            throw_protobuf_parse_error("unexpected wire type for field");
        }
        decode_direct_single_unchecked<Kind>(ptr, end, slot);
    }

    template <typename Concept>
    struct direct_parser {
        static void decode_into(const std::uint8_t *data, std::size_t size, Concept &value) {
            static_assert(direct_message<Concept>, "decode_direct requires a concept type with member_field entries");
            static_assert(valid_field_numbers_v<Concept>, "protobuf field numbers must be unique and within 1..536870911");
            const std::uint8_t *ptr = data;
            const std::uint8_t *end = data + size;
            while (ptr < end) {
                std::uint32_t number = 0;
                wire_type wire = wire_type::varint;
                if (!decode_tag(ptr, end, number, wire)) {
                    throw_protobuf_parse_error("truncated tag");
                }
                if (!dispatch_field(number, wire, ptr, end, value)) {
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
        template <std::size_t... Is>
        static bool dispatch_fields(std::uint32_t number, wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end,
                                    Concept &value, type_traits::helper::index_sequence<Is...>) {
            bool handled = false;
            ((number == field_number_v<Concept, Is> ? (decode_field<Is>(number, wire, ptr, end, value), handled = true, 0)
                                                       : 0),
             ...);
            return handled;
        }

        static bool dispatch_field(std::uint32_t number, wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end,
                                   Concept &value) {
            return dispatch_fields(number, wire, ptr, end, value,
                                   type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

        template <std::size_t Index>
        static void decode_field(std::uint32_t number, wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end,
                                 Concept &value) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            auto &slot = value.*(field_type::member);
            if constexpr (is_repeated_kind_v<kind>) {
                static_assert(direct_container_v<kind, decltype(slot)>,
                              "protobuf repeated member requires a container with emplace_back()");
                decode_repeated<kind>(wire, ptr, end, slot, number);
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
                direct_parser<message_concept_t<kind>>::decode_into(ptr, length, slot);
                ptr += length;
            } else {
                decode_direct_single<kind>(ptr, end, wire, slot);
            }
        }

        template <typename Repeated, typename Container>
        static void decode_repeated(wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end, Container &slots,
                                    std::uint32_t number) {
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
                auto &elem = slots.emplace_back();
                direct_parser<message_concept_t<element>>::decode_into(ptr, length, elem);
                ptr += length;
            } else if constexpr (type_traits::type_relations::is_same_v<element, proto::string> ||
                                 type_traits::type_relations::is_same_v<element, proto::bytes>) {
                if (wire != wire_type::len) {
                    throw_protobuf_parse_error("string element requires LEN wire type");
                }
                auto &elem = slots.emplace_back();
                decode_direct_single_unchecked<element>(ptr, end, elem);
            } else {
                if (constexpr wire_type element_wire = wire_of_v<element>; wire == element_wire) {
                    auto &elem = slots.emplace_back();
                    decode_direct_single_unchecked<element>(ptr, end, elem);
                    decode_repeated_run<element>(number, ptr, end, slots);
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
                        auto &elem = slots.emplace_back();
                        decode_packed_scalar<element>(ptr, block_end, elem);
                    }
                } else {
                    throw_protobuf_parse_error("unexpected wire type for repeated field");
                }
            }
        }

        template <typename Element, typename Container>
        static void decode_repeated_run(std::uint32_t number, const std::uint8_t *&ptr, const std::uint8_t *end,
                                        Container &slots) {
            constexpr wire_type element_wire = wire_of_v<Element>;
            if (number >= 16) {
                return;
            }
            const std::uint8_t tag_byte =
                static_cast<std::uint8_t>((number << 3) | static_cast<std::uint32_t>(element_wire));
            const std::uint8_t *p = ptr;
            if constexpr (element_wire == wire_type::varint) {
                while (p < end && *p == tag_byte) {
                    const std::uint8_t *q = p + 1;
                    std::uint64_t raw = 0;
                    if (q < end && (*q & 0x80u) == 0) {
                        raw = *q++;
                    } else if (q + 1 < end && (q[1] & 0x80u) == 0) {
                        raw = (static_cast<std::uint64_t>(*q & 0x7Fu)) | (static_cast<std::uint64_t>(q[1]) << 7);
                        q += 2;
                    } else {
                        break;
                    }
                    auto &elem = slots.emplace_back();
                    assign_direct_packed_varint<Element>(raw, elem);
                    p = q;
                }
            } else if constexpr (element_wire == wire_type::i32) {
                while (p < end && *p == tag_byte && static_cast<std::size_t>(end - p) >= 5) {
                    const std::uint32_t raw = static_cast<std::uint32_t>(p[1]) |
                                              (static_cast<std::uint32_t>(p[2]) << 8) |
                                              (static_cast<std::uint32_t>(p[3]) << 16) |
                                              (static_cast<std::uint32_t>(p[4]) << 24);
                    auto &elem = slots.emplace_back();
                    assign_direct_packed_fixed32<Element>(raw, elem);
                    p += 5;
                }
            } else if constexpr (element_wire == wire_type::i64) {
                while (p < end && *p == tag_byte && static_cast<std::size_t>(end - p) >= 9) {
                    std::uint64_t raw = 0;
                    for (int i = 0; i < 8; ++i) {
                        raw |= static_cast<std::uint64_t>(p[1 + i]) << (8 * i);
                    }
                    auto &elem = slots.emplace_back();
                    assign_direct_packed_fixed64<Element>(raw, elem);
                    p += 9;
                }
            } else {
                return;
            }
            ptr = p;
        }

        template <typename Element, typename Ty>
        static void decode_packed_scalar(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &elem) {
            if constexpr (constexpr wire_type element_wire = wire_of_v<Element>; element_wire == wire_type::varint) {
                std::uint64_t raw = 0;
                if (!decode_varint(ptr, end, raw)) {
                    throw_protobuf_parse_error("truncated packed varint");
                }
                assign_direct_packed_varint<Element>(raw, elem);
            } else if constexpr (element_wire == wire_type::i32) {
                std::uint32_t raw = 0;
                if (!decode_fixed32(ptr, end, raw)) {
                    throw_protobuf_parse_error("truncated packed fixed32");
                }
                assign_direct_packed_fixed32<Element>(raw, elem);
            } else if constexpr (element_wire == wire_type::i64) {
                std::uint64_t raw = 0;
                if (!decode_fixed64(ptr, end, raw)) {
                    throw_protobuf_parse_error("truncated packed fixed64");
                }
                assign_direct_packed_fixed64<Element>(raw, elem);
            } else {
                throw_protobuf_parse_error("element type is not packable");
            }
        }
    };
}

#endif

#endif
