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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_DIRECT_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_DIRECT_HPP
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/willow/implements/bson/codec.hpp>
#include <rainy/foundation/willow/implements/bson/config.hpp>
#include <rainy/foundation/willow/implements/bson/exceptions.hpp>
#include <rainy/foundation/willow/implements/bson/version.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::willow::bson::types {
    struct string_ {};
    struct int32 {};
    struct int64 {};
    struct double_ {};
    struct boolean {};
    struct null_ {};
    struct binary {};

    template <typename Concept>
    struct message {};

    template <typename Element>
    struct repeated {};
}

namespace rainy::foundation::willow::bson {
    template <typename Ty>
    struct is_repeated_kind : type_traits::helper::false_type {};

    template <typename Element>
    struct is_repeated_kind<types::repeated<Element>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_repeated_kind_v = is_repeated_kind<Ty>::value;

    template <typename Ty>
    struct is_message_kind : type_traits::helper::false_type {};

    template <typename Concept>
    struct is_message_kind<types::message<Concept>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_message_kind_v = is_message_kind<Ty>::value;

    template <typename Ty>
    struct repeated_element;

    template <typename Element>
    struct repeated_element<types::repeated<Element>> {
        using type = Element;
    };

    template <typename Ty>
    using repeated_element_t = typename repeated_element<Ty>::type;

    template <typename Ty>
    struct message_concept;

    template <typename Concept>
    struct message_concept<types::message<Concept>> {
        using type = Concept;
    };

    template <typename Ty>
    using message_concept_t = typename message_concept<Ty>::type;
}

#if RAINY_WILLOW_BSON_HAS_CXX20_FIELDS

namespace rainy::foundation::willow::bson {
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

    /**
     * \lang english
     * @brief One struct member registered for bson direct mode.
     *
     * A bson map is keyed by name, so a field carries its name and no number, unlike the protobuf field which
     * carries both a name and a number.
     *
     * @tparam Name The bson element key
     * @tparam Kind The bson value kind, from bson::types
     * @tparam Member The pointer to member the codec reads and writes
     *
     * \lang simp-chinese
     * @brief 为 bson direct 模式注册的一个结构体成员。
     *
     * bson 映射以名称作键，因此字段只携带名称而不带编号，这一点与同时携带名称和编号的 protobuf 字段不同。
     *
     * @tparam Name bson 元素键
     * @tparam Kind bson 取值种类，来自 bson::types
     * @tparam Member 编解码器读写的成员指针
     */
    template <fixed_name Name, typename Kind, auto Member>
    struct member_field {
        static constexpr auto name = Name;
        using kind = Kind;
        static constexpr auto member = Member;
    };

    template <typename Ty>
    struct is_direct_field : type_traits::helper::false_type {};

    template <fixed_name Name, typename Kind, auto Member>
    struct is_direct_field<member_field<Name, Kind, Member>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_direct_field_v = is_direct_field<Ty>::value;

    template <typename Ty, typename = void>
    struct is_direct_message : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_direct_message<Ty, type_traits::other_trans::void_t<decltype(Ty::bson_fields)>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_direct_message_v = is_direct_message<Ty>::value;

    template <typename Concept>
    struct message_fields {
    private:
        using raw = type_traits::modifers::remove_cvref_t<decltype(Concept::bson_fields)>;

    public:
        using type = raw;
        static constexpr std::size_t size = std::tuple_size<raw>::value;
    };

    template <typename Concept>
    inline constexpr std::size_t field_count_v = message_fields<Concept>::size;

    template <typename Concept, std::size_t Index>
    struct field_at {
        using type = std::tuple_element_t<Index, typename message_fields<Concept>::type>;
        static_assert(is_direct_field_v<type>, "bson_fields must only contain member_field<...> entries");
    };

    template <typename Concept, std::size_t Index>
    using field_at_t = typename field_at<Concept, Index>::type;

    template <typename Concept, std::size_t Index>
    RAINY_NODISCARD constexpr core::text::string_view field_name_view() noexcept {
        return field_at_t<Concept, Index>::name.view();
    }

    template <typename Concept, std::size_t... Is>
    constexpr bool check_unique_names(type_traits::helper::index_sequence<Is...>) {
        constexpr core::text::string_view names[] = {field_name_view<Concept, Is>()...};
        for (std::size_t i = 0; i < sizeof...(Is); ++i) {
            if (names[i].empty()) {
                return false;
            }
            for (std::size_t j = i + 1; j < sizeof...(Is); ++j) {
                if (names[i] == names[j]) {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename Concept>
    inline constexpr bool valid_field_names_v =
        check_unique_names<Concept>(type_traits::helper::make_index_sequence<field_count_v<Concept>>{});

    template <typename Concept, typename = void>
    struct direct_message : type_traits::helper::false_type {};

    template <typename Concept>
    struct direct_message<Concept, type_traits::other_trans::void_t<decltype(Concept::bson_fields)>>
        : type_traits::helper::integral_constant<bool, valid_field_names_v<Concept>> {};

    template <typename Concept>
    inline constexpr bool direct_message_v = direct_message<Concept>::value;
}

#endif

namespace rainy::foundation::willow::bson::implements {
    using namespace rainy::foundation::exceptions::willow::bson;

    using bson::byte_buffer;

#if RAINY_WILLOW_BSON_HAS_CXX20_FIELDS
    template <typename Kind>
    struct element_byte;

    template <>
    struct element_byte<types::double_> : type_traits::helper::integral_constant<std::uint8_t, 0x01> {};
    template <>
    struct element_byte<types::string_> : type_traits::helper::integral_constant<std::uint8_t, 0x02> {};
    template <typename Concept>
    struct element_byte<types::message<Concept>> : type_traits::helper::integral_constant<std::uint8_t, 0x03> {};
    template <typename Element>
    struct element_byte<types::repeated<Element>> : type_traits::helper::integral_constant<std::uint8_t, 0x04> {};
    template <>
    struct element_byte<types::binary> : type_traits::helper::integral_constant<std::uint8_t, 0x05> {};
    template <>
    struct element_byte<types::boolean> : type_traits::helper::integral_constant<std::uint8_t, 0x08> {};
    template <>
    struct element_byte<types::null_> : type_traits::helper::integral_constant<std::uint8_t, 0x0A> {};
    template <>
    struct element_byte<types::int32> : type_traits::helper::integral_constant<std::uint8_t, 0x10> {};
    template <>
    struct element_byte<types::int64> : type_traits::helper::integral_constant<std::uint8_t, 0x12> {};

    template <typename Kind>
    inline constexpr std::uint8_t element_byte_v = element_byte<Kind>::value;

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
        static constexpr bool value =
            direct_string_like_v<Kind, Ty> && requires(raw &v, const value_type *p, std::size_t n) { v.assign(p, n); };
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

    template <typename Concept>
    bool direct_message_is_default(const Concept &value);

    template <typename Kind, typename Ty>
    bool direct_member_is_default(const Ty &member) {
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        if constexpr (is_repeated_kind_v<Kind>) {
            static_assert(direct_container_v<Kind, Ty>, "bson repeated member requires a container with emplace_back()");
            return member.empty();
        } else if constexpr (is_message_kind_v<Kind>) {
            static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                          "bson message member type must match message<Concept>");
            return direct_message_is_default<message_concept_t<Kind>>(member);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::string_> ||
                             type_traits::type_relations::is_same_v<Kind, types::binary>) {
            static_assert(direct_string_like_v<Kind, Ty>, "bson string/binary member requires data()/size()/empty()");
            return member.empty();
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::boolean>) {
            static_assert(type_traits::type_relations::is_same_v<raw, bool>, "bson bool member requires bool");
            return !member;
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::null_>) {
            return true;
        } else {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                          "bson scalar member requires an arithmetic type");
            return member == static_cast<raw>(0);
        }
    }

    template <typename Concept, std::size_t... Is>
    bool direct_message_is_default_impl(const Concept &value, type_traits::helper::index_sequence<Is...>) {
        return (direct_member_is_default<typename field_at_t<Concept, Is>::kind>(value.*(field_at_t<Concept, Is>::member)) &&
                ...);
    }

    template <typename Concept>
    bool direct_message_is_default(const Concept &value) {
        return direct_message_is_default_impl(value, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
    }

    /**
     * \lang english
     * @brief Writes a struct straight to bson bytes, one registered member at a time.
     *
     * Fields holding their default value are skipped, matching the protobuf direct codec, so an empty struct
     * produces an empty document rather than a document full of zeroes.
     *
     * @tparam Concept A type carrying a bson_fields tuple of member_field entries
     *
     * \lang simp-chinese
     * @brief 把结构体直接写成 bson 字节，每次一个已注册成员。
     *
     * 持有默认值的字段被跳过，与 protobuf 的 direct 编解码器一致，因此空结构体产生空文档，而不是满是零的文档。
     *
     * @tparam Concept 携带 member_field 项组成的 bson_fields 元组的类型
     */
    template <typename Concept>
    struct direct_serializer {
        static void encode_into(const Concept &value, byte_buffer &out) {
            const std::size_t length_offset = out.size();
            push_le32(out, 0);
            encode_fields(value, out, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            out.push_back(0);
            const std::size_t length = out.size() - length_offset;
            patch_le32(out, length_offset, static_cast<std::uint32_t>(length));
        }

    private:
        template <std::size_t... Is>
        static void encode_fields(const Concept &value, byte_buffer &out, type_traits::helper::index_sequence<Is...>) {
            (encode_field<Is>(value, out), ...);
        }

        template <std::size_t Index>
        static void encode_field(const Concept &value, byte_buffer &out) {
            using field_type = field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            const auto &member = value.*(field_type::member);
            if (direct_member_is_default<kind>(member)) {
                return;
            }
            out.push_back(element_byte_v<kind>);
            append_field_name<Index>(out);
            out.push_back(0);
            encode_member<kind>(member, out);
        }

        template <std::size_t Index>
        static void append_field_name(byte_buffer &out) {
            const core::text::string_view name = field_name_view<Concept, Index>();
            const auto *data = reinterpret_cast<const std::uint8_t *>(name.data());
            for (std::size_t i = 0; i < name.size(); ++i) {
                if (data[i] == 0) {
                    throw_bson_serialize_error("a bson field name must not contain a 0x00 byte");
                }
            }
            out.insert(out.end(), data, data + name.size());
        }

        template <typename Kind, typename Ty>
        static void encode_member(const Ty &member, byte_buffer &out) {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            if constexpr (is_repeated_kind_v<Kind>) {
                static_assert(direct_container_v<Kind, Ty>, "bson repeated member requires a container with emplace_back()");
                encode_array<Kind>(member, out);
            } else if constexpr (is_message_kind_v<Kind>) {
                static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                              "bson message member type must match message<Concept>");
                direct_serializer<message_concept_t<Kind>>::encode_into(member, out);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::string_>) {
                static_assert(direct_string_like_v<Kind, Ty>, "bson string member requires data()/size()/empty()");
                encode_string(member, out);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::binary>) {
                static_assert(direct_string_like_v<Kind, Ty>, "bson binary member requires data()/size()/empty()");
                encode_binary(member, out);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::boolean>) {
                static_assert(type_traits::type_relations::is_same_v<raw, bool>, "bson bool member requires bool");
                out.push_back(member ? 1 : 0);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::null_>) {
                (void) member;
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::double_>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "bson double member requires an arithmetic type");
                const double value = static_cast<double>(member);
                std::uint64_t bits = 0;
                core::builtin::copy_memory(&bits, &value, sizeof(double));
                push_le64(out, bits);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::int32>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "bson int32 member requires an arithmetic type");
                const auto value = static_cast<std::int64_t>(member);
                if (value < static_cast<std::int64_t>((utility::numeric_limits<std::int32_t>::min)()) ||
                    value > static_cast<std::int64_t>((utility::numeric_limits<std::int32_t>::max)())) {
                    throw_bson_serialize_error("bson int32 value out of range");
                }
                push_le32(out, static_cast<std::uint32_t>(static_cast<std::int32_t>(value)));
            } else {
                static_assert(type_traits::type_relations::is_same_v<Kind, types::int64>,
                              "unknown bson direct field kind");
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "bson int64 member requires an arithmetic type");
                push_le64(out, static_cast<std::uint64_t>(static_cast<std::int64_t>(member)));
            }
        }

        template <typename Ty>
        static void encode_string(const Ty &member, byte_buffer &out) {
            const std::size_t size = member.size();
            const auto *data = reinterpret_cast<const std::uint8_t *>(member.data());
            for (std::size_t i = 0; i < size; ++i) {
                if (data[i] == 0) {
                    throw_bson_serialize_error("a bson string must not contain a 0x00 byte");
                }
            }
            if (size > static_cast<std::size_t>((utility::numeric_limits<std::int32_t>::max)()) - 1) {
                throw_bson_serialize_error("a bson string exceeds the 32-bit length limit");
            }
            push_le32(out, static_cast<std::uint32_t>(size + 1));
            out.insert(out.end(), data, data + size);
            out.push_back(0);
        }

        template <typename Ty>
        static void encode_binary(const Ty &member, byte_buffer &out) {
            const std::size_t size = member.size();
            if (size > static_cast<std::size_t>((utility::numeric_limits<std::int32_t>::max)())) {
                throw_bson_serialize_error("a bson binary exceeds the 32-bit length limit");
            }
            push_le32(out, static_cast<std::uint32_t>(size));
            out.push_back(static_cast<std::uint8_t>(binary_subtype::generic));
            const auto *data = reinterpret_cast<const std::uint8_t *>(member.data());
            out.insert(out.end(), data, data + size);
        }

        template <typename Kind, typename Ty>
        static void encode_array(const Ty &member, byte_buffer &out) {
            const std::size_t length_offset = out.size();
            push_le32(out, 0);
            std::size_t index = 0;
            char index_buffer[24];
            for (const auto &element: member) {
                using element_kind = repeated_element_t<Kind>;
                out.push_back(element_byte_v<element_kind>);
                const auto result = core::text::to_chars(index_buffer, index_buffer + sizeof(index_buffer),
                                                         static_cast<unsigned long long>(index));
                out.insert(out.end(), index_buffer, result.ptr);
                ++index;
                out.push_back(0);
                encode_member<element_kind>(element, out);
            }
            out.push_back(0);
            patch_le32(out, length_offset, static_cast<std::uint32_t>(out.size() - length_offset));
        }

        static void patch_le32(byte_buffer &out, const std::size_t offset, const std::uint32_t value) {
            out[offset] = static_cast<std::uint8_t>(value & 0xffu);
            out[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xffu);
            out[offset + 2] = static_cast<std::uint8_t>((value >> 16) & 0xffu);
            out[offset + 3] = static_cast<std::uint8_t>((value >> 24) & 0xffu);
        }
    };

    /**
     * \lang english
     * @brief Reads bson bytes straight into a struct, one registered member at a time.
     *
     * A field absent from the input keeps its previous value, and a field the concept does not map is skipped
     * through the shared element skipper, so an unknown key never derails a decode.
     *
     * @tparam Concept A type carrying a bson_fields tuple of member_field entries
     *
     * \lang simp-chinese
     * @brief 把 bson 字节直接读入结构体，每次一个已注册成员。
     *
     * 输入中缺失的字段保留其原值，concept 未映射的字段经共享的元素跳过器越过，因此未知键不会打断解码。
     *
     * @tparam Concept 携带 member_field 项组成的 bson_fields 元组的类型
     */
    template <typename Concept>
    struct direct_parser {
        static void decode_into(const std::uint8_t *data, const std::size_t size, Concept &out) {
            if (size < 5) {
                throw_bson_parse_error("a bson document needs at least five bytes");
            }
            const std::uint8_t *ptr = data;
            const std::uint8_t *const end = data + size;
            std::uint32_t raw = 0;
            if (!take_le32(ptr, end, raw)) {
                throw_bson_parse_error("truncated bson document length");
            }
            const std::int32_t length = static_cast<std::int32_t>(raw);
            if (length < 5 || static_cast<std::size_t>(length) != size) {
                throw_bson_parse_error("a bson document length does not match the input");
            }
            const std::uint8_t *const document_end = data + length;
            decode_fields(ptr, document_end, out, 0);
            if (ptr >= document_end || *ptr != 0) {
                throw_bson_parse_error("a bson document is missing its terminator");
            }
            ++ptr;
            if (ptr != document_end) {
                throw_bson_parse_error("a bson document length does not match its contents");
            }
        }

    private:
        template <typename Message>
        static void decode_fields(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out,
                                  const std::size_t depth) {
            if (depth >= willow::implements::max_document_nesting_depth) {
                throw_bson_parse_error("document nesting is too deep");
            }
            while (ptr < end) {
                if (*ptr == 0) {
                    break;
                }
                const std::uint8_t type_byte = *ptr++;
                const std::uint8_t *key_data = nullptr;
                std::size_t key_size = 0;
                if (!take_cstring(ptr, end, key_data, key_size)) {
                    throw_bson_parse_error("truncated bson element key");
                }
                const core::text::string_view key(reinterpret_cast<const char *>(key_data), key_size);
                dispatch_field<Message>(ptr, end, out, key, static_cast<element_type>(type_byte),
                                        type_traits::helper::make_index_sequence<field_count_v<Message>>{}, depth);
            }
        }

        template <typename Message, std::size_t... Is>
        static void dispatch_field(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out,
                                   const core::text::string_view &key, const element_type kind,
                                   type_traits::helper::index_sequence<Is...>, const std::size_t depth) {
            const bool matched = (try_assign_field<Message, Is>(ptr, end, out, key, kind, depth) || ...);
            if (!matched) {
                skip_element_body(kind, ptr, end);
            }
        }

        template <typename Message, std::size_t Index>
        static bool try_assign_field(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out,
                                     const core::text::string_view &key, const element_type kind,
                                     const std::size_t depth) {
            using field_type = field_at_t<Message, Index>;
            using field_kind = typename field_type::kind;
            if (!(field_name_view<Message, Index>() == key)) {
                return false;
            }
            if (static_cast<std::uint8_t>(kind) != element_byte_v<field_kind>) {
                throw_bson_parse_error("a bson element type does not match the registered field kind");
            }
            decode_member<field_kind>(ptr, end, out.*(field_type::member), depth);
            return true;
        }

        template <typename Kind, typename Ty>
        static void decode_member(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member,
                                  const std::size_t depth) {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            if constexpr (is_repeated_kind_v<Kind>) {
                static_assert(direct_container_v<Kind, Ty>, "bson repeated member requires a container with emplace_back()");
                decode_array<Kind>(ptr, end, member, depth);
            } else if constexpr (is_message_kind_v<Kind>) {
                static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                              "bson message member type must match message<Concept>");
                decode_message<message_concept_t<Kind>>(ptr, end, member, depth);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::string_>) {
                static_assert(direct_assignable_string_v<Kind, Ty>, "bson string member requires assign(data(), size())");
                decode_string(ptr, end, member);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::binary>) {
                static_assert(direct_assignable_string_v<Kind, Ty>, "bson binary member requires assign(data(), size())");
                decode_binary(ptr, end, member);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::boolean>) {
                static_assert(type_traits::type_relations::is_same_v<raw, bool>, "bson bool member requires bool");
                std::uint8_t value = 0;
                if (!take_u8(ptr, end, value)) {
                    throw_bson_parse_error("truncated bson boolean");
                }
                if (value > 1) {
                    throw_bson_parse_error("a bson boolean must be 0 or 1");
                }
                member = value != 0;
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::null_>) {
                (void) member;
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::double_>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "bson double member requires an arithmetic type");
                std::uint64_t bits = 0;
                if (!take_le64(ptr, end, bits)) {
                    throw_bson_parse_error("truncated bson double");
                }
                double value = 0;
                core::builtin::copy_memory(&value, &bits, sizeof(double));
                assign_floating(value, member);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::int32>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "bson int32 member requires an arithmetic type");
                std::uint32_t value = 0;
                if (!take_le32(ptr, end, value)) {
                    throw_bson_parse_error("truncated bson int32");
                }
                assign_integer(static_cast<std::int64_t>(static_cast<std::int32_t>(value)), member);
            } else {
                static_assert(type_traits::type_relations::is_same_v<Kind, types::int64>,
                              "unknown bson direct field kind");
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "bson int64 member requires an arithmetic type");
                std::uint64_t value = 0;
                if (!take_le64(ptr, end, value)) {
                    throw_bson_parse_error("truncated bson int64");
                }
                assign_integer(static_cast<std::int64_t>(value), member);
            }
        }

        template <typename Ty>
        static void assign_floating(const double value, Ty &member) {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            if constexpr (type_traits::primary_types::is_floating_point_v<raw>) {
                member = static_cast<raw>(value);
            } else if constexpr (type_traits::properties::is_unsigned_v<raw>) {
                if (!(value >= 0.0) || value > static_cast<double>((utility::numeric_limits<raw>::max)())) {
                    throw_bson_parse_error("a bson double is out of the member range");
                }
                member = static_cast<raw>(value);
            } else {
                if (value < static_cast<double>((utility::numeric_limits<raw>::min)()) ||
                    value > static_cast<double>((utility::numeric_limits<raw>::max)())) {
                    throw_bson_parse_error("a bson double is out of the member range");
                }
                member = static_cast<raw>(value);
            }
        }

        template <typename Integer, typename Ty>
        static void assign_integer(const Integer value, Ty &member) {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            if constexpr (type_traits::primary_types::is_floating_point_v<raw>) {
                member = static_cast<raw>(value);
            } else if constexpr (type_traits::properties::is_unsigned_v<raw>) {
                if constexpr (type_traits::properties::is_signed_v<Integer>) {
                    if (value < 0) {
                        throw_bson_parse_error("a bson integer is out of the member range");
                    }
                }
                if (static_cast<std::uint64_t>(value) > static_cast<std::uint64_t>((utility::numeric_limits<raw>::max)())) {
                    throw_bson_parse_error("a bson integer is out of the member range");
                }
                member = static_cast<raw>(value);
            } else {
                if (static_cast<std::int64_t>(value) < static_cast<std::int64_t>((utility::numeric_limits<raw>::min)()) ||
                    static_cast<std::int64_t>(value) > static_cast<std::int64_t>((utility::numeric_limits<raw>::max)())) {
                    throw_bson_parse_error("a bson integer is out of the member range");
                }
                member = static_cast<raw>(value);
            }
        }

        template <typename Ty>
        static void decode_string(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member) {
            const std::uint8_t *data = nullptr;
            std::size_t size = 0;
            read_length_prefixed(ptr, end, data, size);
            using value_type = typename type_traits::modifers::remove_cvref_t<Ty>::value_type;
            member.assign(reinterpret_cast<const value_type *>(data), size);
        }

        /**
         * \lang english
         * @brief Reads a binary element, whose body is an int32 length, a subtype byte and the payload.
         *
         * \lang simp-chinese
         * @brief 读取二进制元素，其值为 int32 长度、子类型字节与载荷。
         */
        template <typename Ty>
        static void decode_binary(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member) {
            std::uint32_t raw = 0;
            if (!take_le32(ptr, end, raw)) {
                throw_bson_parse_error("truncated bson binary length");
            }
            if (static_cast<std::int32_t>(raw) < 0) {
                throw_bson_parse_error("a bson binary length must not be negative");
            }
            const std::size_t size = static_cast<std::size_t>(raw);
            if (static_cast<std::size_t>(end - ptr) < size + 1) {
                throw_bson_parse_error("a bson binary runs past the end of the input");
            }
            ++ptr;
            using value_type = typename type_traits::modifers::remove_cvref_t<Ty>::value_type;
            member.assign(reinterpret_cast<const value_type *>(ptr), size);
            ptr += size;
        }

        template <typename Concept2>
        static void decode_message(const std::uint8_t *&ptr, const std::uint8_t *end, Concept2 &member,
                                   const std::size_t depth) {
            std::uint32_t raw = 0;
            if (!take_le32(ptr, end, raw)) {
                throw_bson_parse_error("truncated bson document length");
            }
            const std::int32_t length = static_cast<std::int32_t>(raw);
            if (length < 5 || static_cast<std::size_t>(end - ptr) < static_cast<std::size_t>(length) - 4) {
                throw_bson_parse_error("a nested bson document runs past the end of the input");
            }
            const std::uint8_t *const document_end = ptr + (length - 4);
            decode_fields(ptr, document_end, member, depth + 1);
            if (ptr >= document_end || *ptr != 0) {
                throw_bson_parse_error("a nested bson document is missing its terminator");
            }
            ++ptr;
            if (ptr != document_end) {
                throw_bson_parse_error("a nested bson document length does not match its contents");
            }
        }

        template <typename Kind, typename Ty>
        static void decode_array(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member,
                                 const std::size_t depth) {
            std::uint32_t raw = 0;
            if (!take_le32(ptr, end, raw)) {
                throw_bson_parse_error("truncated bson array length");
            }
            const std::int32_t length = static_cast<std::int32_t>(raw);
            if (length < 5 || static_cast<std::size_t>(end - ptr) < static_cast<std::size_t>(length) - 4) {
                throw_bson_parse_error("a bson array runs past the end of the input");
            }
            const std::uint8_t *const array_end = ptr + (length - 4);
            using element_kind = repeated_element_t<Kind>;
            while (ptr < array_end) {
                if (*ptr == 0) {
                    break;
                }
                const std::uint8_t type_byte = *ptr++;
                const std::uint8_t *index_data = nullptr;
                std::size_t index_size = 0;
                if (!take_cstring(ptr, array_end, index_data, index_size)) {
                    throw_bson_parse_error("truncated bson array index");
                }
                if (element_byte_v<element_kind> != type_byte) {
                    skip_element_body(static_cast<element_type>(type_byte), ptr, array_end);
                    continue;
                }
                auto &slot = member.emplace_back();
                decode_member<element_kind>(ptr, array_end, slot, depth + 1);
            }
            if (ptr >= array_end || *ptr != 0) {
                throw_bson_parse_error("a bson array is missing its terminator");
            }
            ++ptr;
            if (ptr != array_end) {
                throw_bson_parse_error("a bson array length does not match its contents");
            }
        }
    };
}

#endif

#endif

#endif
