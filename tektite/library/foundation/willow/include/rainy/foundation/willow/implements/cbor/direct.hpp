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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_DIRECT_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_DIRECT_HPP
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/willow/implements/cbor/codec.hpp>
#include <rainy/foundation/willow/implements/cbor/config.hpp>
#include <rainy/foundation/willow/implements/cbor/exceptions.hpp>
#include <rainy/foundation/willow/implements/cbor/serializer.hpp>
#include <rainy/foundation/willow/implements/cbor/version.hpp>

#if RAINY_WILLOW_CBOR_AVAILABLE

namespace rainy::foundation::willow::cbor::types {
    struct text_string {};
    struct byte_string {};
    struct int32 {};
    struct int64 {};
    struct uint32 {};
    struct uint64 {};
    struct double_ {};
    struct boolean {};
    struct null_ {};

    template <typename Concept>
    struct message {};

    template <typename Element>
    struct repeated {};
}

namespace rainy::foundation::willow::cbor {
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

#if RAINY_WILLOW_CBOR_HAS_CXX20_FIELDS

namespace rainy::foundation::willow::cbor {
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
     * @brief One struct member registered for cbor direct mode.
     *
     * A cbor map is keyed by name, so a field carries its name and no number, unlike the protobuf field which
     * carries both a name and a number.
     *
     * @tparam Name The cbor map key
     * @tparam Kind The cbor value kind, from cbor::types
     * @tparam Member The pointer to member the codec reads and writes
     *
     * \lang simp-chinese
     * @brief 为 cbor direct 模式注册的一个结构体成员。
     *
     * cbor 映射以名称作键，因此字段只携带名称而不带编号，这一点与同时携带名称和编号的 protobuf 字段不同。
     *
     * @tparam Name cbor 映射键
     * @tparam Kind cbor 取值种类，来自 cbor::types
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
    struct is_direct_message<Ty, type_traits::other_trans::void_t<decltype(Ty::cbor_fields)>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_direct_message_v = is_direct_message<Ty>::value;

    template <typename Concept>
    struct message_fields {
    private:
        using raw = type_traits::modifers::remove_cvref_t<decltype(Concept::cbor_fields)>;

    public:
        using type = raw;
        static constexpr std::size_t size = std::tuple_size<raw>::value;
    };

    template <typename Concept>
    inline constexpr std::size_t field_count_v = message_fields<Concept>::size;

    template <typename Concept, std::size_t Index>
    struct field_at {
        using type = std::tuple_element_t<Index, typename message_fields<Concept>::type>;
        static_assert(is_direct_field_v<type>, "cbor_fields must only contain member_field<...> entries");
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

    template <typename Concept>
    concept direct_message = requires { Concept::cbor_fields; } && valid_field_names_v<Concept>;
}

#endif

namespace rainy::foundation::willow::cbor::implements {
    using namespace rainy::foundation::exceptions::willow::cbor;

    using cbor::byte_buffer;

#if RAINY_WILLOW_CBOR_HAS_CXX20_FIELDS
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

    template <typename Kind>
    inline constexpr bool is_integer_kind_v = type_traits::type_relations::is_same_v<Kind, types::int32> ||
                                              type_traits::type_relations::is_same_v<Kind, types::int64> ||
                                              type_traits::type_relations::is_same_v<Kind, types::uint32> ||
                                              type_traits::type_relations::is_same_v<Kind, types::uint64>;

    template <typename Concept>
    bool direct_message_is_default(const Concept &value);

    template <typename Kind, typename Ty>
    bool direct_member_is_default(const Ty &member) {
        using raw = type_traits::modifers::remove_cvref_t<Ty>;
        if constexpr (is_repeated_kind_v<Kind>) {
            static_assert(direct_container_v<Kind, Ty>, "cbor repeated member requires a container with emplace_back()");
            return member.empty();
        } else if constexpr (is_message_kind_v<Kind>) {
            static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                          "cbor message member type must match message<Concept>");
            return direct_message_is_default<message_concept_t<Kind>>(member);
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::text_string> ||
                             type_traits::type_relations::is_same_v<Kind, types::byte_string>) {
            static_assert(direct_string_like_v<Kind, Ty>, "cbor string member requires data()/size()/empty()");
            return member.empty();
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::boolean>) {
            static_assert(type_traits::type_relations::is_same_v<raw, bool>, "cbor bool member requires bool");
            return !member;
        } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::null_>) {
            return true;
        } else {
            static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                          "cbor scalar member requires an arithmetic type");
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

    template <typename Kind>
    struct is_byte_string_kind : type_traits::helper::bool_constant<type_traits::type_relations::is_same_v<Kind, types::byte_string>> {};

    /**
     * \lang english
     * @brief Writes a struct straight to cbor bytes, one registered member at a time.
     *
     * A struct becomes a cbor map. Fields holding their default value are skipped, matching the protobuf direct
     * codec, so an empty struct produces an empty map rather than a map full of zeroes.
     *
     * @tparam Concept A type carrying a cbor_fields tuple of member_field entries
     *
     * \lang simp-chinese
     * @brief 把结构体直接写成 cbor 字节，每次一个已注册成员。
     *
     * 结构体变为 cbor 映射。持有默认值的字段被跳过，与 protobuf 的 direct 编解码器一致，因此空结构体产生空映射，
     * 而不是满是零的映射。
     *
     * @tparam Concept 携带 member_field 项组成的 cbor_fields 元组的类型
     */
    template <typename Concept>
    struct direct_serializer {
        static void encode_into(const Concept &value, byte_buffer &out) {
            push_head(out, major_type::map, static_cast<std::uint64_t>(count_present(value,
                    type_traits::helper::make_index_sequence<field_count_v<Concept>>{})));
            encode_fields(value, out, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

    private:
        template <std::size_t... Is>
        static std::size_t count_present(const Concept &value, type_traits::helper::index_sequence<Is...>) {
            std::size_t count = 0;
            const bool present[] = {!direct_member_is_default<typename field_at_t<Concept, Is>::kind>(
                value.*(field_at_t<Concept, Is>::member))...};
            for (const bool flag: present) {
                count += flag ? 1 : 0;
            }
            return count;
        }

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
            const core::text::string_view name = field_name_view<Concept, Index>();
            push_head(out, major_type::text_string, static_cast<std::uint64_t>(name.size()));
            const auto *name_bytes = reinterpret_cast<const std::uint8_t *>(name.data());
            out.insert(out.end(), name_bytes, name_bytes + name.size());
            encode_member<kind>(member, out);
        }

        template <typename Kind, typename Ty>
        static void encode_member(const Ty &member, byte_buffer &out) {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            if constexpr (is_repeated_kind_v<Kind>) {
                static_assert(direct_container_v<Kind, Ty>, "cbor repeated member requires a container with emplace_back()");
                push_head(out, major_type::array, static_cast<std::uint64_t>(member.size()));
                for (const auto &element: member) {
                    encode_member<repeated_element_t<Kind>>(element, out);
                }
            } else if constexpr (is_message_kind_v<Kind>) {
                static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                              "cbor message member type must match message<Concept>");
                direct_serializer<message_concept_t<Kind>>::encode_into(member, out);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::text_string> ||
                                 type_traits::type_relations::is_same_v<Kind, types::byte_string>) {
                static_assert(direct_string_like_v<Kind, Ty>, "cbor string member requires data()/size()/empty()");
                encode_string<Kind>(member, out);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::boolean>) {
                static_assert(type_traits::type_relations::is_same_v<raw, bool>, "cbor bool member requires bool");
                out.push_back(static_cast<std::uint8_t>(
                    0xe0u | static_cast<std::uint8_t>(member ? simple_value::true_ : simple_value::false_)));
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::null_>) {
                (void) member;
                out.push_back(static_cast<std::uint8_t>(0xe0u | static_cast<std::uint8_t>(simple_value::null_)));
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::double_>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor double member requires an arithmetic type");
                push_float_narrowest(out, static_cast<double>(member));
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::int32>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor int32 member requires an arithmetic type");
                const auto value = static_cast<std::int64_t>(member);
                if (value < static_cast<std::int64_t>((utility::numeric_limits<std::int32_t>::min)()) ||
                    value > static_cast<std::int64_t>((utility::numeric_limits<std::int32_t>::max)())) {
                    throw_cbor_serialize_error("cbor int32 value out of range");
                }
                encode_signed(out, value);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::uint32>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor uint32 member requires an arithmetic type");
                const auto value = static_cast<std::uint64_t>(member);
                if (value > static_cast<std::uint64_t>((utility::numeric_limits<std::uint32_t>::max)())) {
                    throw_cbor_serialize_error("cbor uint32 value out of range");
                }
                push_head(out, major_type::unsigned_int, value);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::uint64>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor uint64 member requires an arithmetic type");
                push_head(out, major_type::unsigned_int, static_cast<std::uint64_t>(member));
            } else {
                static_assert(type_traits::type_relations::is_same_v<Kind, types::int64>,
                              "unknown cbor direct field kind");
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor int64 member requires an arithmetic type");
                encode_signed(out, static_cast<std::int64_t>(member));
            }
        }

        static void encode_signed(byte_buffer &out, const std::int64_t value) {
            if (value >= 0) {
                push_head(out, major_type::unsigned_int, static_cast<std::uint64_t>(value));
            } else {
                push_head(out, major_type::negative_int, static_cast<std::uint64_t>(-1 - value));
            }
        }

        template <typename Kind, typename Ty>
        static void encode_string(const Ty &member, byte_buffer &out) {
            const std::size_t size = member.size();
            constexpr bool byte_string = is_byte_string_kind<Kind>::value;
            push_head(out, byte_string ? major_type::byte_string : major_type::text_string, static_cast<std::uint64_t>(size));
            const auto *data = reinterpret_cast<const std::uint8_t *>(member.data());
            out.insert(out.end(), data, data + size);
        }
    };

    /**
     * \lang english
     * @brief Reads cbor bytes straight into a struct, one registered member at a time.
     *
     * A field absent from the input keeps its previous value, and a field the concept does not map is skipped
     * through the shared value skipper, so an unknown key never derails a decode.
     *
     * @tparam Concept A type carrying a cbor_fields tuple of member_field entries
     *
     * \lang simp-chinese
     * @brief 把 cbor 字节直接读入结构体，每次一个已注册成员。
     *
     * 输入中缺失的字段保留其原值，concept 未映射的字段经共享的取值跳过器越过，因此未知键不会打断解码。
     *
     * @tparam Concept 携带 member_field 项组成的 cbor_fields 元组的类型
     */
    template <typename Concept>
    struct direct_parser {
        static void decode_into(const std::uint8_t *data, const std::size_t size, Concept &out) {
            const std::uint8_t *ptr = data;
            const std::uint8_t *const end = data + size;
            decode_map_fields<Concept>(ptr, end, out);
            if (ptr != end) {
                throw_cbor_parse_error("trailing bytes after the cbor map");
            }
        }

        template <typename Message>
        static void decode_map_fields(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            if (major != major_type::map) {
                throw_cbor_parse_error("a cbor direct value must be a map");
            }
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor map is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    decode_entry<Message>(ptr, end, out);
                }
            }
            for (std::uint64_t i = 0; i < argument; ++i) {
                decode_entry<Message>(ptr, end, out);
            }
        }

    private:
        static void read_head(const std::uint8_t *&ptr, const std::uint8_t *end, major_type &major, std::uint8_t &additional,
                              std::uint64_t &argument) {
            std::uint8_t head = 0;
            if (!take_u8(ptr, end, head)) {
                throw_cbor_parse_error("unexpected end of cbor input");
            }
            major = static_cast<major_type>(head >> 5);
            additional = static_cast<std::uint8_t>(head & 0x1fu);
            argument = 0;
            if (additional == additional_indefinite) {
                return;
            }
            if (!take_argument(additional, ptr, end, argument)) {
                throw_cbor_parse_error("truncated cbor argument");
            }
        }

        template <typename Message>
        static void decode_entry(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            if (major != major_type::text_string) {
                throw_cbor_parse_error("a cbor direct map key must be a text string");
            }
            if (additional == additional_indefinite) {
                throw_cbor_parse_error("a cbor direct map key must not be indefinite");
            }
            if (argument > static_cast<std::uint64_t>(end - ptr)) {
                throw_cbor_parse_error("a cbor direct map key runs past the end of the input");
            }
            const core::text::string_view key(reinterpret_cast<const char *>(ptr), static_cast<std::size_t>(argument));
            ptr += argument;
            dispatch_entry<Message>(ptr, end, out, key,
                                    type_traits::helper::make_index_sequence<field_count_v<Message>>{});
        }

        template <typename Message, std::size_t... Is>
        static void dispatch_entry(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out,
                                   const core::text::string_view &key, type_traits::helper::index_sequence<Is...>) {
            const bool matched = (try_assign<Message, Is>(ptr, end, out, key) || ...);
            if (!matched) {
                skip_value(ptr, end);
            }
        }

        template <typename Message, std::size_t Index>
        static bool try_assign(const std::uint8_t *&ptr, const std::uint8_t *end, Message &out,
                               const core::text::string_view &key) {
            using field_type = field_at_t<Message, Index>;
            using field_kind = typename field_type::kind;
            if (!(field_name_view<Message, Index>() == key)) {
                return false;
            }
            decode_member<field_kind>(ptr, end, out.*(field_type::member));
            return true;
        }

        template <typename Kind, typename Ty>
        static void decode_member(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member) {
            using raw = type_traits::modifers::remove_cvref_t<Ty>;
            if constexpr (is_repeated_kind_v<Kind>) {
                static_assert(direct_container_v<Kind, Ty>, "cbor repeated member requires a container with emplace_back()");
                decode_array<Kind>(ptr, end, member);
            } else if constexpr (is_message_kind_v<Kind>) {
                static_assert(type_traits::type_relations::is_same_v<raw, message_concept_t<Kind>>,
                              "cbor message member type must match message<Concept>");
                direct_parser<message_concept_t<Kind>>::decode_map_fields(ptr, end, member);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::text_string> ||
                                 type_traits::type_relations::is_same_v<Kind, types::byte_string>) {
                static_assert(direct_assignable_string_v<Kind, Ty>, "cbor string member requires assign(data(), size())");
                decode_string<Kind>(ptr, end, member);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::boolean>) {
                static_assert(type_traits::type_relations::is_same_v<raw, bool>, "cbor bool member requires bool");
                std::uint8_t head = 0;
                if (!take_u8(ptr, end, head)) {
                    throw_cbor_parse_error("truncated cbor boolean");
                }
                const auto true_byte = static_cast<std::uint8_t>(0xe0u | static_cast<std::uint8_t>(simple_value::true_));
                const auto false_byte = static_cast<std::uint8_t>(0xe0u | static_cast<std::uint8_t>(simple_value::false_));
                if (head == true_byte) {
                    member = true;
                } else if (head == false_byte) {
                    member = false;
                } else {
                    throw_cbor_parse_error("a cbor boolean must be true or false");
                }
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::null_>) {
                (void) member;
                skip_value(ptr, end);
            } else if constexpr (type_traits::type_relations::is_same_v<Kind, types::double_>) {
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor double member requires an arithmetic type");
                member = static_cast<raw>(decode_double(ptr, end));
            } else {
                static_assert(is_integer_kind_v<Kind>, "unknown cbor direct field kind");
                static_assert(type_traits::composite_types::is_arithmetic_v<raw>,
                              "cbor integer member requires an arithmetic type");
                assign_integer(ptr, end, member);
            }
        }

        template <typename Ty>
        static void assign_integer(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            if (additional == additional_indefinite) {
                throw_cbor_parse_error("an integer may not use the indefinite form");
            }
            if (major == major_type::unsigned_int) {
                if (argument > static_cast<std::uint64_t>((utility::numeric_limits<Ty>::max)())) {
                    throw_cbor_parse_error("a cbor integer is out of the member range");
                }
                member = static_cast<Ty>(argument);
                return;
            }
            if (major == major_type::negative_int) {
                if (argument > static_cast<std::uint64_t>((utility::numeric_limits<std::int64_t>::max)())) {
                    throw_cbor_parse_error("a negative cbor integer is out of range");
                }
                const auto value = -1 - static_cast<std::int64_t>(argument);
                if (value < static_cast<std::int64_t>((utility::numeric_limits<Ty>::min)())) {
                    throw_cbor_parse_error("a cbor integer is out of the member range");
                }
                member = static_cast<Ty>(value);
                return;
            }
            throw_cbor_parse_error("a cbor integer field received a non integer value");
        }

        static double decode_double(const std::uint8_t *&ptr, const std::uint8_t *end) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            if (major == major_type::unsigned_int) {
                return static_cast<double>(argument);
            }
            if (major == major_type::negative_int) {
                return -1.0 - static_cast<double>(argument);
            }
            if (major != major_type::simple) {
                throw_cbor_parse_error("a cbor float field received a non numeric value");
            }
            switch (additional) {
                case 25:
                    return half_to_double(static_cast<std::uint16_t>(argument));
                case 26: {
                    float value = 0;
                    const auto bits = static_cast<std::uint32_t>(argument);
                    core::builtin::copy_memory(&value, &bits, sizeof(float));
                    return static_cast<double>(value);
                }
                case 27: {
                    double value = 0;
                    core::builtin::copy_memory(&value, &argument, sizeof(double));
                    return value;
                }
                default:
                    throw_cbor_parse_error("a cbor float field received a non float value");
            }
        }

        template <typename Kind, typename Ty>
        static void decode_string(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            const bool byte_string = is_byte_string_kind<Kind>::value;
            const major_type expected = byte_string ? major_type::byte_string : major_type::text_string;
            if (major != expected) {
                throw_cbor_parse_error("a cbor string field received a string of the wrong type");
            }
            using value_type = typename type_traits::modifers::remove_cvref_t<Ty>::value_type;
            if (additional == additional_indefinite) {
                typename type_traits::modifers::remove_cvref_t<Ty> accumulated{};
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor string is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        break;
                    }
                    major_type chunk_major{};
                    std::uint8_t chunk_additional = 0;
                    std::uint64_t chunk_argument = 0;
                    read_head(ptr, end, chunk_major, chunk_additional, chunk_argument);
                    if (chunk_major != expected || chunk_additional == additional_indefinite) {
                        throw_cbor_parse_error("an indefinite cbor string holds a chunk of the wrong type");
                    }
                    if (chunk_argument > static_cast<std::uint64_t>(end - ptr)) {
                        throw_cbor_parse_error("a cbor string chunk runs past the end of the input");
                    }
                    accumulated.append(reinterpret_cast<const value_type *>(ptr), static_cast<std::size_t>(chunk_argument));
                    ptr += chunk_argument;
                }
                member = utility::move(accumulated);
                return;
            }
            if (argument > static_cast<std::uint64_t>(end - ptr)) {
                throw_cbor_parse_error("a cbor string runs past the end of the input");
            }
            member.assign(reinterpret_cast<const value_type *>(ptr), static_cast<std::size_t>(argument));
            ptr += argument;
        }

        template <typename Kind, typename Ty>
        static void decode_array(const std::uint8_t *&ptr, const std::uint8_t *end, Ty &member) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            if (major != major_type::array) {
                throw_cbor_parse_error("a cbor repeated field received a non array value");
            }
            using element_kind = repeated_element_t<Kind>;
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor array is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    auto &slot = member.emplace_back();
                    decode_member<element_kind>(ptr, end, slot);
                }
            }
            for (std::uint64_t i = 0; i < argument; ++i) {
                auto &slot = member.emplace_back();
                decode_member<element_kind>(ptr, end, slot);
            }
        }

        static void skip_value(const std::uint8_t *&ptr, const std::uint8_t *end) {
            major_type major{};
            std::uint8_t additional = 0;
            std::uint64_t argument = 0;
            read_head(ptr, end, major, additional, argument);
            switch (major) {
                case major_type::unsigned_int:
                case major_type::negative_int:
                case major_type::tag:
                    if (additional == additional_indefinite) {
                        throw_cbor_parse_error("an integer or tag may not use the indefinite form");
                    }
                    if (major == major_type::tag) {
                        skip_value(ptr, end);
                    }
                    return;
                case major_type::byte_string:
                case major_type::text_string:
                    skip_string(ptr, end, additional, argument);
                    return;
                case major_type::array:
                    skip_array(ptr, end, additional, argument);
                    return;
                case major_type::map:
                    skip_map(ptr, end, additional, argument);
                    return;
                case major_type::simple:
                    skip_simple(ptr, end, additional, argument);
                    return;
                default:
                    throw_cbor_parse_error("unknown cbor major type");
            }
        }

        static void skip_string(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t additional,
                                const std::uint64_t argument) {
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor string is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    major_type chunk_major{};
                    std::uint8_t chunk_additional = 0;
                    std::uint64_t chunk_argument = 0;
                    read_head(ptr, end, chunk_major, chunk_additional, chunk_argument);
                    if (chunk_major != major_type::byte_string && chunk_major != major_type::text_string) {
                        throw_cbor_parse_error("an indefinite cbor string holds a chunk of the wrong type");
                    }
                    if (chunk_argument > static_cast<std::uint64_t>(end - ptr)) {
                        throw_cbor_parse_error("a cbor string chunk runs past the end of the input");
                    }
                    ptr += chunk_argument;
                }
            }
            if (argument > static_cast<std::uint64_t>(end - ptr)) {
                throw_cbor_parse_error("a cbor string runs past the end of the input");
            }
            ptr += argument;
        }

        static void skip_array(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t additional,
                               const std::uint64_t argument) {
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor array is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    skip_value(ptr, end);
                }
            }
            for (std::uint64_t i = 0; i < argument; ++i) {
                skip_value(ptr, end);
            }
        }

        static void skip_map(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t additional,
                             const std::uint64_t argument) {
            if (additional == additional_indefinite) {
                while (true) {
                    if (ptr >= end) {
                        throw_cbor_parse_error("an indefinite cbor map is not terminated");
                    }
                    if (*ptr == 0xff) {
                        ++ptr;
                        return;
                    }
                    skip_value(ptr, end);
                    skip_value(ptr, end);
                }
            }
            for (std::uint64_t i = 0; i < argument; ++i) {
                skip_value(ptr, end);
                skip_value(ptr, end);
            }
        }

        static void skip_simple(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t additional,
                                const std::uint64_t argument) {
            if (additional == additional_indefinite) {
                throw_cbor_parse_error("the break byte is not a value");
            }
            (void) argument;
            (void) ptr;
            (void) end;
        }
    };
}

#endif

#endif

#endif
