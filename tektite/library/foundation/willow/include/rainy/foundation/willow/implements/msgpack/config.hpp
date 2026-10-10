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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_CONFIG_HPP
#include <cstdint>
#include <utility>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/type_traits/underlying_type.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/msgpack/exceptions.hpp>

#if RAINY_WILLOW_MSGPACK_AVAILABLE

namespace rainy::foundation::willow::msgpack {
    using byte_buffer = core::collections::vector<std::uint8_t>;

    enum class feature : std::uint32_t {
        none = 0,
        ext = 1u << 0,
        timestamp = 1u << 1,
        all = ext | timestamp,
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(feature)

    struct msgpack_options {
        feature features = feature::none;
    };
}

namespace rainy::foundation::willow::msgpack {
    struct msgpack_node_tag {};

    template <typename BasicDocument>
    struct with_msgpack_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_msgpack_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc,
                                    msgpack_node_tag>;
    };

    template <typename BasicDocument>
    struct without_msgpack_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_msgpack_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, msgpack_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct msgpack_document;

    template <typename BasicDocument>
    using msgpack_document_base = typename with_msgpack_node_tag<BasicDocument>::type;
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, msgpack::msgpack_node_tag> {
        using type = msgpack::msgpack_document<typename msgpack::without_msgpack_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::msgpack {
    using willow::basic_document;
    using willow::document;
    using willow::document64;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    template <typename BasicDocument>
    struct msgpack_document : public msgpack_document_base<BasicDocument> {
        using base_type = msgpack_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_msgpack_node_tag<BasicDocument>::type>,
                      "msgpack_document requires an untagged basic_document");

        template <typename Ty>
        using allocator_type = typename base_type::template allocator_type<Ty>;

        using size_type = typename base_type::size_type;
        using difference_type = typename base_type::difference_type;
        using char_type = typename base_type::char_type;
        using string_type = typename base_type::string_type;
        using integer_type = typename base_type::integer_type;
        using float_type = typename base_type::float_type;
        using boolean_type = typename base_type::boolean_type;
        using array_type = typename base_type::array_type;
        using object_type = typename base_type::object_type;
        using initializer_list = typename base_type::initializer_list;
        using iterator = typename base_type::iterator;
        using const_iterator = typename base_type::const_iterator;
        using reverse_iterator = typename base_type::reverse_iterator;
        using const_reverse_iterator = typename base_type::const_reverse_iterator;
        using node_type = typename base_type::node_type;

        using base_type::base_type;

        msgpack_document() = default;

        msgpack_document(const base_type &right) : base_type(right) {
        }

        msgpack_document(base_type &&right) : base_type(utility::move(right)) {
        }

        template <typename OtherDocument>
        msgpack_document(from_other_document_t, const OtherDocument &right) :
            base_type(willow::implements::convert_document<base_type>(right)) {
        }

        msgpack_document(const msgpack_document &right) :
            base_type(static_cast<const base_type &>(right)), state_(clone_state(right.state_)) {
        }

        msgpack_document(msgpack_document &&right) noexcept :
            base_type(utility::move(static_cast<base_type &>(right))), state_(right.state_) {
            right.state_ = nullptr;
        }

        ~msgpack_document() {
            release_state();
        }

        msgpack_document &operator=(const msgpack_document &right) {
            if (this != &right) {
                base_type::operator=(static_cast<const base_type &>(right));
                release_state();
                state_ = clone_state(right.state_);
            }
            return *this;
        }

        msgpack_document &operator=(msgpack_document &&right) noexcept {
            if (this != &right) {
                base_type::operator=(utility::move(static_cast<base_type &>(right)));
                release_state();
                state_ = right.state_;
                right.state_ = nullptr;
            }
            return *this;
        }

        base_type &base() noexcept {
            return *this;
        }

        const base_type &base() const noexcept {
            return *this;
        }

        base_type &as_document() noexcept {
            return *this;
        }

        const base_type &as_document() const noexcept {
            return *this;
        }

        using base_type::array;
        using base_type::as_array;
        using base_type::as_bool;
        using base_type::as_float;
        using base_type::as_integer;
        using base_type::as_object;
        using base_type::as_string;
        using base_type::as_string_view;
        using base_type::begin;
        using base_type::cbegin;
        using base_type::cend;
        using base_type::clear;
        using base_type::contains;
        using base_type::count;
        using base_type::crbegin;
        using base_type::crend;
        using base_type::emplace_back;
        using base_type::empty;
        using base_type::end;
        using base_type::erase;
        using base_type::find;
        using base_type::get;
        using base_type::is_array;
        using base_type::is_bool;
        using base_type::is_float;
        using base_type::is_integer;
        using base_type::is_null;
        using base_type::is_number;
        using base_type::is_object;
        using base_type::is_primitive;
        using base_type::is_string;
        using base_type::object;
        using base_type::push_back;
        using base_type::rbegin;
        using base_type::rend;
        using base_type::size;
        using base_type::type_name;
        using base_type::operator[];

        void swap(msgpack_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(state_, other.state_);
        }

        msgpack_document &operator+=(msgpack_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        void set_ext(const std::int8_t type, byte_buffer data) {
            state().ext_type = type;
            state().ext_data = utility::move(data);
        }

        bool has_ext() const noexcept {
            return state_ != nullptr;
        }

        std::int8_t ext_type() const {
            if (state_ == nullptr) {
                exceptions::willow::msgpack::throw_msgpack_type_error("node has no extension payload");
            }
            return state_->ext_type;
        }

        const byte_buffer &ext_data() const {
            if (state_ == nullptr) {
                exceptions::willow::msgpack::throw_msgpack_type_error("node has no extension payload");
            }
            return state_->ext_data;
        }

        bool is_timestamp() const noexcept {
            return state_ != nullptr && state_->ext_type == -1;
        }

        std::int64_t timestamp_seconds() const {
            const byte_buffer &payload = ext_data();
            if (payload.size() == 4) {
                return static_cast<std::int64_t>(decode_be_uint32(payload.data()));
            }
            if (payload.size() == 8) {
                const std::uint64_t data64 = decode_be_uint64(payload.data());
                return static_cast<std::int64_t>(data64 & 0x00000003ffffffffULL);
            }
            if (payload.size() == 12) {
                return decode_be_int64(payload.data() + 4);
            }
            exceptions::willow::msgpack::throw_msgpack_parse_error("invalid timestamp payload length");
        }

        std::uint32_t timestamp_nanoseconds() const {
            const byte_buffer &payload = ext_data();
            if (payload.size() == 4) {
                return 0;
            }
            if (payload.size() == 8) {
                return static_cast<std::uint32_t>(decode_be_uint64(payload.data()) >> 34);
            }
            if (payload.size() == 12) {
                return decode_be_uint32(payload.data());
            }
            exceptions::willow::msgpack::throw_msgpack_parse_error("invalid timestamp payload length");
        }

        friend bool operator==(const msgpack_document &left, const msgpack_document &right) {
            return static_cast<const base_type &>(left) == static_cast<const base_type &>(right);
        }

        friend bool operator!=(const msgpack_document &left, const msgpack_document &right) {
            return static_cast<const base_type &>(left) != static_cast<const base_type &>(right);
        }

        friend bool operator<(const msgpack_document &left, const msgpack_document &right) {
            return static_cast<const base_type &>(left) < static_cast<const base_type &>(right);
        }

        friend bool operator<=(const msgpack_document &left, const msgpack_document &right) {
            return static_cast<const base_type &>(left) <= static_cast<const base_type &>(right);
        }

        friend bool operator>(const msgpack_document &left, const msgpack_document &right) {
            return static_cast<const base_type &>(left) > static_cast<const base_type &>(right);
        }

        friend bool operator>=(const msgpack_document &left, const msgpack_document &right) {
            return static_cast<const base_type &>(left) >= static_cast<const base_type &>(right);
        }

    private:
        struct node_state {
            std::int8_t ext_type;
            byte_buffer ext_data;
        };

        static std::uint32_t decode_be_uint32(const std::uint8_t *data) noexcept {
            return (static_cast<std::uint32_t>(data[0]) << 24) | (static_cast<std::uint32_t>(data[1]) << 16) |
                   (static_cast<std::uint32_t>(data[2]) << 8) | static_cast<std::uint32_t>(data[3]);
        }

        static std::uint64_t decode_be_uint64(const std::uint8_t *data) noexcept {
            return (static_cast<std::uint64_t>(decode_be_uint32(data)) << 32) |
                   static_cast<std::uint64_t>(decode_be_uint32(data + 4));
        }

        static std::int64_t decode_be_int64(const std::uint8_t *data) noexcept {
            return static_cast<std::int64_t>(decode_be_uint64(data));
        }

        static node_state *clone_state(const node_state *right) {
            if (right == nullptr) {
                return nullptr;
            }
            return willow::implements::value<base_type>::template create<node_state>(*right);
        }

        void release_state() noexcept {
            if (state_ != nullptr) {
                willow::implements::value<base_type>::template destroy<node_state>(state_);
                state_ = nullptr;
            }
        }

        node_state &state() {
            if (state_ == nullptr) {
                state_ = willow::implements::value<base_type>::template create<node_state>();
            }
            return *state_;
        }

        node_state *state_{nullptr};
    };

    template <typename BasicDocument = document>
    using facade = msgpack_document<BasicDocument>;

    template <typename Ty, typename = void>
    struct has_msgpack_ext : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_msgpack_ext<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().has_ext())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_msgpack_ext_v = has_msgpack_ext<Ty>::value;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::msgpack> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type = msgpack::msgpack_document<
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif

#endif
