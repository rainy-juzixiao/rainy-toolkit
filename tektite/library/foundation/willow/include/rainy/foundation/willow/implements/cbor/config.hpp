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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_CONFIG_HPP
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits/underlying_type.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/cbor/exceptions.hpp>
#include <rainy/foundation/willow/implements/cbor/version.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>

#if RAINY_WILLOW_CBOR_AVAILABLE

namespace rainy::foundation::willow::cbor {
    using byte_buffer = core::collections::vector<std::uint8_t>;

    /**
     * \lang english
     * @brief The cbor major type, the top three bits of the initial byte.
     *
     * \lang simp-chinese
     * @brief cbor 主类型，即首字节的高三位。
     */
    enum class major_type : std::uint8_t {
        unsigned_int = 0,
        negative_int = 1,
        byte_string = 2,
        text_string = 3,
        array = 4,
        map = 5,
        tag = 6,
        simple = 7,
    };

    /**
     * \lang english
     * @brief The cbor simple values the floor can hold, plus the unassigned ones it keeps verbatim.
     *
     * \lang simp-chinese
     * @brief 地板能承载的 cbor 简单值，以及它原样保留的未分配值。
     */
    enum class simple_value : std::uint8_t {
        false_ = 20,
        true_ = 21,
        null_ = 22,
        undefined = 23,
    };
}

namespace rainy::foundation::willow::cbor {
    struct cbor_node_tag {};

    template <typename BasicDocument>
    struct with_cbor_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_cbor_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type =
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, cbor_node_tag>;
    };

    template <typename BasicDocument>
    struct without_cbor_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_cbor_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, cbor_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct cbor_document;

    template <typename BasicDocument>
    using cbor_document_base = typename with_cbor_node_tag<BasicDocument>::type;
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, cbor::cbor_node_tag> {
        using type = cbor::cbor_document<typename cbor::without_cbor_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::cbor {
    using willow::basic_document;
    using willow::document;
    using willow::document64;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    /**
     * \lang english
     * @brief The cbor document type, carrying what the document floor cannot express.
     *
     * The floor has one string kind, while cbor distinguishes a text string from a byte string, and the floor
     * has no place for a semantic tag or an unassigned simple value. Those keep their identity on the node: a
     * byte string and a text string both read back as a string, and the flag tells them apart. A node without
     * such state pays a single null pointer.
     *
     * @tparam BasicDocument The untagged document type
     *
     * \lang simp-chinese
     * @brief cbor 文档类型，携带文档地板无法表达的内容。
     *
     * 地板只有一种字符串，而 cbor 区分文本串与字节串，且地板没有语义标签或未分配简单值的位置。这些在节点上保留
     * 其身份：字节串与文本串都读作字符串，由标志位区分二者。不具备该状态的节点只付出一个空指针。
     *
     * @tparam BasicDocument 未打标签的文档类型
     */
    template <typename BasicDocument>
    struct cbor_document : public cbor_document_base<BasicDocument> {
        using base_type = cbor_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_cbor_node_tag<BasicDocument>::type>,
                      "cbor_document requires an untagged basic_document");

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

        cbor_document() = default;

        cbor_document(const base_type &right) : base_type(right) {
        }

        cbor_document(base_type &&right) : base_type(utility::move(right)) {
        }

        template <typename OtherDocument>
        cbor_document(from_other_document_t, const OtherDocument &right) :
            base_type(willow::implements::convert_document<base_type>(right)) {
        }

        cbor_document(const cbor_document &right) :
            base_type(static_cast<const base_type &>(right)), state_(clone_state(right.state_)) {
        }

        cbor_document(cbor_document &&right) noexcept :
            base_type(utility::move(static_cast<base_type &>(right))), state_(right.state_) {
            right.state_ = nullptr;
        }

        ~cbor_document() {
            release_state();
        }

        cbor_document &operator=(const cbor_document &right) {
            if (this != &right) {
                base_type::operator=(static_cast<const base_type &>(right));
                release_state();
                state_ = clone_state(right.state_);
            }
            return *this;
        }

        cbor_document &operator=(cbor_document &&right) noexcept {
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

        void swap(cbor_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(state_, other.state_);
        }

        cbor_document &operator+=(cbor_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        /**
         * \lang english
         * @brief Indicates whether the string of this node was authored as a byte string.
         *
         * \lang simp-chinese
         * @brief 指示该节点的字符串是否以字节串撰写。
         */
        bool is_byte_string() const noexcept {
            return state_ != nullptr && state_->byte_string;
        }

        void mark_byte_string() {
            state().byte_string = true;
        }

        void clear_byte_string() noexcept {
            if (state_ != nullptr) {
                state_->byte_string = false;
            }
        }

        /**
         * \lang english
         * @brief Indicates whether this node carries a semantic tag or an unassigned simple value.
         *
         * \lang simp-chinese
         * @brief 指示该节点是否携带语义标签或未分配简单值。
         */
        bool has_native_value() const noexcept {
            return state_ != nullptr && state_->native;
        }

        /**
         * \lang english
         * @brief Returns the semantic tag number of a tagged node.
         *
         * \lang simp-chinese
         * @brief 返回已打标签节点的语义标签编号。
         */
        std::uint64_t tag() const {
            if (state_ == nullptr || !state_->native) {
                exceptions::willow::cbor::throw_cbor_type_error("node has no semantic tag");
            }
            return state_->tag_number;
        }

        /**
         * \lang english
         * @brief Returns the raw initial byte of a node carrying an unassigned simple value.
         *
         * \lang simp-chinese
         * @brief 返回携带未分配简单值的节点的原始首字节。
         */
        std::uint8_t simple() const {
            if (state_ == nullptr || !state_->native) {
                exceptions::willow::cbor::throw_cbor_type_error("node has no simple value");
            }
            return state_->simple_value;
        }

        void set_tag(const std::uint64_t tag_number) {
            state().native = true;
            state().tagged = true;
            state().tag_number = tag_number;
            state().simple_value = 0;
        }

        void set_simple(const std::uint8_t simple_value) {
            state().native = true;
            state().tagged = false;
            state().simple_value = simple_value;
            state().tag_number = 0;
        }

        /**
         * \lang english
         * @brief Indicates whether the native value of this node is a semantic tag rather than a simple value.
         *
         * \lang simp-chinese
         * @brief 指示该节点的原生取值是语义标签而非简单值。
         */
        bool is_tagged() const noexcept {
            return state_ != nullptr && state_->native && state_->tagged;
        }

        /**
         * \lang english
         * @brief Drops the native tag or simple value, keeping the underlying value.
         *
         * \lang simp-chinese
         * @brief 丢弃原生标签或简单值，保留底层取值。
         */
        void clear_native_value() noexcept {
            if (state_ != nullptr) {
                state_->native = false;
                state_->tagged = false;
                state_->tag_number = 0;
                state_->simple_value = 0;
            }
        }

        friend bool operator==(const cbor_document &left, const cbor_document &right) {
            if (!(static_cast<const base_type &>(left) == static_cast<const base_type &>(right))) {
                return false;
            }
            if (left.is_byte_string() != right.is_byte_string()) {
                return false;
            }
            if (left.has_native_value() != right.has_native_value()) {
                return false;
            }
            if (!left.has_native_value()) {
                return true;
            }
            return left.state_->tag_number == right.state_->tag_number &&
                   left.state_->simple_value == right.state_->simple_value;
        }

        friend bool operator!=(const cbor_document &left, const cbor_document &right) {
            return !(left == right);
        }

        friend bool operator<(const cbor_document &left, const cbor_document &right) {
            return static_cast<const base_type &>(left) < static_cast<const base_type &>(right);
        }

        friend bool operator<=(const cbor_document &left, const cbor_document &right) {
            return static_cast<const base_type &>(left) <= static_cast<const base_type &>(right);
        }

        friend bool operator>(const cbor_document &left, const cbor_document &right) {
            return static_cast<const base_type &>(left) > static_cast<const base_type &>(right);
        }

        friend bool operator>=(const cbor_document &left, const cbor_document &right) {
            return static_cast<const base_type &>(left) >= static_cast<const base_type &>(right);
        }

    private:
        struct node_state {
            bool byte_string{false};
            bool native{false};
            bool tagged{false};
            std::uint64_t tag_number{0};
            std::uint8_t simple_value{0};
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
    using facade = cbor_document<BasicDocument>;

    template <typename Ty, typename = void>
    struct has_cbor_native_value : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_cbor_native_value<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().has_native_value())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_cbor_native_value_v = has_cbor_native_value<Ty>::value;

    template <typename Ty, typename = void>
    struct has_cbor_byte_string : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_cbor_byte_string<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().is_byte_string())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_cbor_byte_string_v = has_cbor_byte_string<Ty>::value;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::cbor> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type = cbor::cbor_document<
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif

#endif
