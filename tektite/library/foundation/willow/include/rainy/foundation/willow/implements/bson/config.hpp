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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_CONFIG_HPP
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits/underlying_type.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/bson/exceptions.hpp>
#include <rainy/foundation/willow/implements/bson/version.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::willow::bson {
    using byte_buffer = core::collections::vector<std::uint8_t>;

    /**
     * \lang english
     * @brief The bson element type byte, as it appears on the wire.
     *
     * The enumerator values are the element type bytes themselves, so the kind of a decoded node and the byte
     * that produced it are the same number.
     *
     * \lang simp-chinese
     * @brief bson 元素类型字节，与线上表示一致。
     *
     * 枚举值就是元素类型字节本身，因此解码所得节点的种类与产生它的字节是同一个数。
     */
    enum class element_type : std::uint8_t {
        double_ = 0x01,
        string_ = 0x02,
        document = 0x03,
        array = 0x04,
        binary = 0x05,
        undefined = 0x06,
        object_id = 0x07,
        boolean = 0x08,
        datetime = 0x09,
        null_ = 0x0A,
        regex = 0x0B,
        db_pointer = 0x0C,
        code = 0x0D,
        symbol = 0x0E,
        code_with_scope = 0x0F,
        int32 = 0x10,
        timestamp = 0x11,
        int64 = 0x12,
        decimal128 = 0x13,
        max_key = 0x7F,
        min_key = 0xFF,
    };

    /**
     * \lang english
     * @brief The bson binary subtype byte.
     *
     * \lang simp-chinese
     * @brief bson 二进制子类型字节。
     */
    enum class binary_subtype : std::uint8_t {
        generic = 0x00,
        function = 0x01,
        byte_array_deprecated = 0x02,
        uuid_deprecated = 0x03,
        uuid = 0x04,
        md5 = 0x05,
        encrypted = 0x06,
        column = 0x07,
        sensitive = 0x08,
        vector = 0x09,
        user_defined = 0x80,
    };
}

namespace rainy::foundation::willow::bson {
    struct bson_node_tag {};

    template <typename BasicDocument>
    struct with_bson_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_bson_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type =
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, bson_node_tag>;
    };

    template <typename BasicDocument>
    struct without_bson_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_bson_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, bson_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct bson_document;

    template <typename BasicDocument>
    using bson_document_base = typename with_bson_node_tag<BasicDocument>::type;
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, bson::bson_node_tag> {
        using type = bson::bson_document<typename bson::without_bson_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::bson {
    using willow::basic_document;
    using willow::document;
    using willow::document64;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    /**
     * \lang english
     * @brief The bson document type, carrying the element types the document floor cannot express.
     *
     * The floor has seven kinds; bson has twenty one element types. The seven that map onto a floor kind do so
     * directly, and the rest keep their element type byte and their raw value body on the node, so they survive
     * a decode and encode byte for byte. A node without such state pays a single null pointer.
     *
     * @tparam BasicDocument The untagged document type
     *
     * \lang simp-chinese
     * @brief bson 文档类型，携带文档地板无法表达的元素类型。
     *
     * 地板有七种 kind，bson 有二十一种元素类型。能映射到地板 kind 的七种直接映射，其余的在节点上保留其元素类型
     * 字节与原始值体，从而逐字节往返。不具备该状态的节点只付出一个空指针。
     *
     * @tparam BasicDocument 未打标签的文档类型
     */
    template <typename BasicDocument>
    struct bson_document : public bson_document_base<BasicDocument> {
        using base_type = bson_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_bson_node_tag<BasicDocument>::type>,
                      "bson_document requires an untagged basic_document");

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

        bson_document() = default;

        bson_document(const base_type &right) : base_type(right) {
        }

        bson_document(base_type &&right) : base_type(utility::move(right)) {
        }

        template <typename OtherDocument>
        bson_document(from_other_document_t, const OtherDocument &right) :
            base_type(willow::implements::convert_document<base_type>(right)) {
        }

        bson_document(const bson_document &right) :
            base_type(static_cast<const base_type &>(right)), state_(clone_state(right.state_)) {
        }

        bson_document(bson_document &&right) noexcept :
            base_type(utility::move(static_cast<base_type &>(right))), state_(right.state_) {
            right.state_ = nullptr;
        }

        ~bson_document() {
            release_state();
        }

        bson_document &operator=(const bson_document &right) {
            if (this != &right) {
                base_type::operator=(static_cast<const base_type &>(right));
                release_state();
                state_ = clone_state(right.state_);
            }
            return *this;
        }

        bson_document &operator=(bson_document &&right) noexcept {
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

        void swap(bson_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(state_, other.state_);
        }

        bson_document &operator+=(bson_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        /**
         * \lang english
         * @brief Indicates whether this node carries an element type the floor cannot express.
         *
         * \lang simp-chinese
         * @brief 指示该节点是否携带地板无法表达的元素类型。
         */
        bool has_native_element() const noexcept {
            return state_ != nullptr;
        }

        /**
         * \lang english
         * @brief Returns the element type byte of a node carrying native state.
         *
         * \lang simp-chinese
         * @brief 返回携带原生状态的节点的元素类型字节。
         */
        element_type native_element() const {
            if (state_ == nullptr) {
                exceptions::willow::bson::throw_bson_type_error("node has no native bson element");
            }
            return state_->kind;
        }

        /**
         * \lang english
         * @brief Returns the raw value body of a node carrying native state.
         *
         * The bytes are the value body only, without the element type byte or the key.
         *
         * \lang simp-chinese
         * @brief 返回携带原生状态的节点的原始值体。
         *
         * 字节仅为值体，不含元素类型字节与键。
         */
        const byte_buffer &native_payload() const {
            if (state_ == nullptr) {
                exceptions::willow::bson::throw_bson_type_error("node has no native bson element");
            }
            return state_->payload;
        }

        void set_native_element(const element_type kind, byte_buffer payload) {
            state().kind = kind;
            state().payload = utility::move(payload);
        }

        friend bool operator==(const bson_document &left, const bson_document &right) {
            if (!(static_cast<const base_type &>(left) == static_cast<const base_type &>(right))) {
                return false;
            }
            if (left.has_native_element() != right.has_native_element()) {
                return false;
            }
            if (!left.has_native_element()) {
                return true;
            }
            return left.state_->kind == right.state_->kind && left.state_->payload == right.state_->payload;
        }

        friend bool operator!=(const bson_document &left, const bson_document &right) {
            return !(left == right);
        }

        friend bool operator<(const bson_document &left, const bson_document &right) {
            return static_cast<const base_type &>(left) < static_cast<const base_type &>(right);
        }

        friend bool operator<=(const bson_document &left, const bson_document &right) {
            return static_cast<const base_type &>(left) <= static_cast<const base_type &>(right);
        }

        friend bool operator>(const bson_document &left, const bson_document &right) {
            return static_cast<const base_type &>(left) > static_cast<const base_type &>(right);
        }

        friend bool operator>=(const bson_document &left, const bson_document &right) {
            return static_cast<const base_type &>(left) >= static_cast<const base_type &>(right);
        }

    private:
        struct node_state {
            element_type kind{element_type::null_};
            byte_buffer payload;
        };

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
    using facade = bson_document<BasicDocument>;

    template <typename Ty, typename = void>
    struct has_bson_native_element : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_bson_native_element<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().has_native_element())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_bson_native_element_v = has_bson_native_element<Ty>::value;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::bson> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type = bson::bson_document<
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif

#endif
