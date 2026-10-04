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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_CONFIG_HPP
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf {
    using byte_buffer = core::collections::vector<std::uint8_t>;
}

namespace rainy::foundation::willow::protobuf::proto {
    struct int32 {};
    struct int64 {};
    struct uint32 {};
    struct uint64 {};
    struct sint32 {};
    struct sint64 {};
    struct boolean {};
    struct fixed32 {};
    struct sfixed32 {};
    struct floating {};
    struct fixed64 {};
    struct sfixed64 {};
    struct doubling {};
    struct string {};
    struct bytes {};

    template <typename Concept>
    struct message {};

    template <typename Element>
    struct repeated {};
}

namespace rainy::foundation::willow::protobuf {
    template <typename Ty>
    struct is_repeated_kind : type_traits::helper::false_type {};

    template <typename Element>
    struct is_repeated_kind<proto::repeated<Element>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_repeated_kind_v = is_repeated_kind<Ty>::value;

    template <typename Ty>
    struct is_message_kind : type_traits::helper::false_type {};

    template <typename Concept>
    struct is_message_kind<proto::message<Concept>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_message_kind_v = is_message_kind<Ty>::value;

    template <typename Ty>
    struct repeated_element;

    template <typename Element>
    struct repeated_element<proto::repeated<Element>> {
        using type = Element;
    };

    template <typename Ty>
    using repeated_element_t = typename repeated_element<Ty>::type;

    template <typename Ty>
    struct message_concept;

    template <typename Concept>
    struct message_concept<proto::message<Concept>> {
        using type = Concept;
    };

    template <typename Ty>
    using message_concept_t = typename message_concept<Ty>::type;

    enum class wire_type : std::uint32_t {
        varint = 0,
        i64 = 1,
        len = 2,
        sgroup = 3,
        egroup = 4,
        i32 = 5,
    };

    template <typename Kind>
    struct wire_of;

    template <>
    struct wire_of<proto::int32> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::int64> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::uint32> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::uint64> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::sint32> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::sint64> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::boolean> : type_traits::helper::integral_constant<wire_type, wire_type::varint> {};
    template <>
    struct wire_of<proto::fixed64> : type_traits::helper::integral_constant<wire_type, wire_type::i64> {};
    template <>
    struct wire_of<proto::sfixed64> : type_traits::helper::integral_constant<wire_type, wire_type::i64> {};
    template <>
    struct wire_of<proto::doubling> : type_traits::helper::integral_constant<wire_type, wire_type::i64> {};
    template <>
    struct wire_of<proto::string> : type_traits::helper::integral_constant<wire_type, wire_type::len> {};
    template <>
    struct wire_of<proto::bytes> : type_traits::helper::integral_constant<wire_type, wire_type::len> {};
    template <>
    struct wire_of<proto::fixed32> : type_traits::helper::integral_constant<wire_type, wire_type::i32> {};
    template <>
    struct wire_of<proto::sfixed32> : type_traits::helper::integral_constant<wire_type, wire_type::i32> {};
    template <>
    struct wire_of<proto::floating> : type_traits::helper::integral_constant<wire_type, wire_type::i32> {};

    template <typename Concept>
    struct wire_of<proto::message<Concept>> : type_traits::helper::integral_constant<wire_type, wire_type::len> {};

    template <typename Element>
    struct wire_of<proto::repeated<Element>> : wire_of<Element> {};

    template <typename Kind>
    inline constexpr wire_type wire_of_v = wire_of<Kind>::value;

    template <typename Kind>
    struct is_packable : type_traits::helper::false_type {};

    template <>
    struct is_packable<proto::int32> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::int64> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::uint32> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::uint64> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::sint32> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::sint64> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::boolean> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::fixed32> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::sfixed32> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::floating> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::fixed64> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::sfixed64> : type_traits::helper::true_type {};
    template <>
    struct is_packable<proto::doubling> : type_traits::helper::true_type {};

    template <typename Kind>
    inline constexpr bool is_packable_v = is_packable<Kind>::value;

    inline constexpr std::uint32_t max_field_number = 536870911;
}

#if RAINY_WILLOW_PROTOBUF_HAS_CXX20_FIELDS

namespace rainy::foundation::willow::protobuf {
    template <std::size_t N>
    struct fixed_name {
        char data[N]{};

        constexpr fixed_name(const char (&str)[N]) noexcept {
            for (std::size_t i = 0; i < N; ++i) {
                data[i] = str[i];
            }
        }

        constexpr std::size_t size() const noexcept {
            return N == 0 ? 0 : N - 1;
        }

        constexpr core::text::string_view view() const noexcept {
            return core::text::string_view(data, size());
        }

        constexpr bool operator==(const fixed_name &) const noexcept = default;
    };

    template <std::size_t N>
    fixed_name(const char (&)[N]) -> fixed_name<N>;
}

namespace rainy::foundation::willow::protobuf {
    template <fixed_name Name, typename Kind, std::uint32_t Number>
    struct field {
        static constexpr auto name = Name;
        using kind = Kind;
        static constexpr std::uint32_t number = Number;
    };

    template <typename Ty>
    struct is_field_nttp : type_traits::helper::false_type {};

    template <fixed_name Name, typename Kind, std::uint32_t Number>
    struct is_field_nttp<field<Name, Kind, Number>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_field_nttp_v = is_field_nttp<Ty>::value;

    template <fixed_name Name, typename Kind, std::uint32_t Number, auto Member>
    struct member_field {
        static constexpr auto name = Name;
        using kind = Kind;
        static constexpr std::uint32_t number = Number;
        static constexpr auto member = Member;
    };

    template <typename Ty>
    struct is_direct_field_nttp : type_traits::helper::false_type {};

    template <fixed_name Name, typename Kind, std::uint32_t Number, auto Member>
    struct is_direct_field_nttp<member_field<Name, Kind, Number, Member>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_direct_field_nttp_v = is_direct_field_nttp<Ty>::value;
}

#endif

#if RAINY_WILLOW_PROTOBUF_HAS_CXX17_FIELDS

namespace rainy::foundation::willow::protobuf {
    template <typename Kind, std::uint32_t Number>
    struct cxx17_field {
        using kind = Kind;
        static constexpr std::uint32_t number = Number;
    };

    template <typename Kind, std::uint32_t Number, auto Member>
    struct cxx17_member_field {
        using kind = Kind;
        static constexpr std::uint32_t number = Number;
        static constexpr auto member = Member;
    };

    template <typename Ty, typename = void>
    struct has_protobuf_kind : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_protobuf_kind<Ty, type_traits::other_trans::void_t<typename Ty::kind>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename = void>
    struct has_protobuf_number : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_protobuf_number<
        Ty, type_traits::other_trans::void_t<type_traits::helper::integral_constant<std::uint32_t, Ty::number>>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename = void>
    struct has_protobuf_member : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_protobuf_member<Ty, type_traits::other_trans::void_t<decltype(Ty::member)>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename = void>
    struct has_cxx17_field_name : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_cxx17_field_name<Ty,
                                type_traits::other_trans::void_t<decltype(Ty::protobuf_field_name())>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename = void>
    struct has_nttp_field_name : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_nttp_field_name<Ty, type_traits::other_trans::void_t<decltype(Ty::name.view())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    struct is_field_macro
        : type_traits::helper::bool_constant<has_protobuf_kind<Ty>::value && has_protobuf_number<Ty>::value &&
                                             has_cxx17_field_name<Ty>::value> {};

    template <typename Ty>
    inline constexpr bool is_field_macro_v = is_field_macro<Ty>::value;

    template <typename Ty>
    struct is_direct_field_macro
        : type_traits::helper::bool_constant<is_field_macro<Ty>::value && has_protobuf_member<Ty>::value> {};

    template <typename Ty>
    inline constexpr bool is_direct_field_macro_v = is_direct_field_macro<Ty>::value;

    template <typename Ty>
    RAINY_NODISCARD constexpr core::text::string_view field_name_of() noexcept {
        if constexpr (has_nttp_field_name<Ty>::value) {
            return Ty::name.view();
        } else {
            return Ty::protobuf_field_name();
        }
    }
}

#define RAINY_WILLOW_PROTOBUF_CXX17_FIELD_DECL(FieldType, NameStr, Kind, Number)                                        \
    struct FieldType : ::rainy::foundation::willow::protobuf::cxx17_field<Kind, Number> {                               \
        static constexpr ::rainy::core::text::string_view protobuf_field_name() noexcept {                              \
            return ::rainy::core::text::string_view(NameStr, sizeof(NameStr) - 1);                                       \
        }                                                                                                               \
    }

#define RAINY_WILLOW_PROTOBUF_CXX17_MEMBER_FIELD_DECL(FieldType, NameStr, Kind, Number, MemberPtr)                     \
    struct FieldType : ::rainy::foundation::willow::protobuf::cxx17_member_field<Kind, Number, MemberPtr> {             \
        static constexpr ::rainy::core::text::string_view protobuf_field_name() noexcept {                              \
            return ::rainy::core::text::string_view(NameStr, sizeof(NameStr) - 1);                                       \
        }                                                                                                               \
    }

#endif

namespace rainy::foundation::willow::protobuf {
#if RAINY_WILLOW_PROTOBUF_HAS_CXX20_FIELDS
    template <typename Ty>
    struct is_field
        : type_traits::helper::bool_constant<is_field_nttp_v<Ty> || is_field_macro_v<Ty>> {};

    template <typename Ty>
    inline constexpr bool is_field_v = is_field<Ty>::value;

    template <typename Ty>
    struct is_direct_field
        : type_traits::helper::bool_constant<is_direct_field_nttp_v<Ty> || is_direct_field_macro_v<Ty>> {};

    template <typename Ty>
    inline constexpr bool is_direct_field_v = is_direct_field<Ty>::value;
#else
    template <typename Ty>
    struct is_field : is_field_macro<Ty> {};

    template <typename Ty>
    inline constexpr bool is_field_v = is_field<Ty>::value;

    template <typename Ty>
    struct is_direct_field : is_direct_field_macro<Ty> {};

    template <typename Ty>
    inline constexpr bool is_direct_field_v = is_direct_field<Ty>::value;
#endif

    template <typename Ty, typename = void>
    struct is_protobuf_message : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_protobuf_message<Ty, type_traits::other_trans::void_t<decltype(Ty::protobuf_fields)>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_protobuf_message_v = is_protobuf_message<Ty>::value;

    template <typename Concept>
    struct message_fields {
    private:
        using raw = type_traits::modifers::remove_cvref_t<decltype(Concept::protobuf_fields)>;

    public:
        using type = raw;
        static constexpr std::size_t size = std::tuple_size<raw>::value;
    };

    template <typename Concept>
    inline constexpr std::size_t field_count_v = message_fields<Concept>::size;

    template <typename Concept, std::size_t Index>
    struct field_at {
        using type = std::tuple_element_t<Index, typename message_fields<Concept>::type>;
        static_assert(is_field_v<type> || is_direct_field_v<type>,
                      "protobuf_fields must only contain field<...> or member_field<...> entries");
    };

    template <typename Concept, std::size_t Index>
    using field_at_t = typename field_at<Concept, Index>::type;

    template <typename Concept, std::size_t Index>
    struct field_number {
        static constexpr std::uint32_t value = field_at_t<Concept, Index>::number;
    };

    template <typename Concept, std::size_t Index>
    inline constexpr std::uint32_t field_number_v = field_number<Concept, Index>::value;

    template <typename Concept, std::size_t Index>
    RAINY_NODISCARD constexpr core::text::string_view field_name_view() noexcept {
        return field_name_of<field_at_t<Concept, Index>>();
    }

    template <typename Concept, std::size_t... Is>
    constexpr bool check_field_numbers(type_traits::helper::index_sequence<Is...>) {
        constexpr std::uint32_t numbers[] = {field_number_v<Concept, Is>...};
        for (std::size_t i = 0; i < sizeof...(Is); ++i) {
            if (numbers[i] == 0 || numbers[i] > max_field_number) {
                return false;
            }
            for (std::size_t j = i + 1; j < sizeof...(Is); ++j) {
                if (numbers[i] == numbers[j]) {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename Concept>
    inline constexpr bool valid_field_numbers_v =
        check_field_numbers<Concept>(type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
}

#if RAINY_WILLOW_PROTOBUF_HAS_CXX20_FIELDS

namespace rainy::foundation::willow::protobuf {
    template <typename Ty>
    concept protobuf_message = requires { Ty::protobuf_fields; } &&
                               (std::tuple_size_v<type_traits::modifers::remove_cvref_t<decltype(Ty::protobuf_fields)>> >= 0);
}

#endif

#endif

#endif
