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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_CONFIG_HPP
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/xml/exceptions.hpp>

namespace rainy::foundation::willow::xml::implements {
    enum class token_type {
        uninitialized,
        element_open,
        element_end,
        text,
        cdata,
        comment,
        declaration,
        processing_instruction,
        doctype,
        end_of_input
    };
}

namespace rainy::foundation::willow::xml {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    struct xml_node_tag {};

    template <typename BasicDocument>
    struct with_xml_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_xml_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, xml_node_tag>;
    };

    template <typename BasicDocument>
    struct without_xml_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_xml_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, xml_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct xml_document;

    template <typename BasicDocument>
    using xml_document_base = typename with_xml_node_tag<BasicDocument>::type;

    /**
     * \lang english
     * @brief The xml declaration carried by the root of a parsed document.
     *
     * @note Standalone is kept verbatim because xml does not constrain it to a fixed set of spellings.
     *
     * \lang simp-chinese
     * @brief 解析得到的文档根所携带的 xml 声明。
     *
     * @note standalone 原样保留，因为 xml 并未将其限定为固定的拼写集合。
     */
    template <typename StringType>
    struct xml_declaration {
        using string_type = StringType;

        bool present{false};
        string_type version;
        string_type encoding;
        string_type standalone;
    };

    /**
     * \lang english
     * @brief An element attribute, keeping its name and value together.
     *
     * \lang simp-chinese
     * @brief 元素属性，把名称与取值放在一起。
     */
    template <typename StringType>
    struct xml_attribute {
        using string_type = StringType;

        string_type name;
        string_type value;
    };
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, xml::xml_node_tag> {
        using type = xml::xml_document<typename xml::without_xml_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::xml {
    /**
     * \lang english
     * @brief The xml document type, carrying the per element payload the document floor cannot express.
     *
     * An element name is only meaningful to its parent, so it lives on the node rather than as an object key;
     * attributes, the cdata authoring flag and the root xml declaration ride alongside it. All of it hangs off
     * the node state, so an untagged document pays a single null pointer for the whole mechanism.
     *
     * @tparam BasicDocument The untagged document type
     *
     * \lang simp-chinese
     * @brief xml 文档类型，携带文档地板无法表达的各元素负载。
     *
     * 元素名只对其父节点有意义，因此挂在节点上而非作为对象键；属性、cdata 撰写标记与根的 xml 声明与之并列。
     * 这些全部挂在节点状态上，因此未打标签的文档只为整套机制付出一个空指针。
     *
     * @tparam BasicDocument 未打标签的文档类型
     */
    template <typename BasicDocument>
    struct xml_document : public xml_document_base<BasicDocument> {
        using base_type = xml_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_xml_node_tag<BasicDocument>::type>,
                      "xml_document requires an untagged basic_document");

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
        using declaration_type = xml_declaration<string_type>;
        using attribute_type = xml_attribute<string_type>;
        using attribute_list = core::collections::vector<attribute_type>;

        using base_type::base_type;

        xml_document() = default;

        xml_document(const base_type &right) : base_type(right) {
        }

        xml_document(base_type &&right) : base_type(utility::move(right)) {
        }

        template <typename OtherDocument>
        xml_document(from_other_document_t, const OtherDocument &right) :
            base_type(willow::implements::convert_document<base_type>(right)) {
        }

        xml_document(const xml_document &right) :
            base_type(static_cast<const base_type &>(right)), state_(clone_state(right.state_)) {
        }

        xml_document(xml_document &&right) noexcept :
            base_type(utility::move(static_cast<base_type &>(right))), state_(right.state_) {
            right.state_ = nullptr;
        }

        ~xml_document() {
            release_state();
        }

        xml_document &operator=(const xml_document &right) {
            if (this != &right) {
                base_type::operator=(static_cast<const base_type &>(right));
                release_state();
                state_ = clone_state(right.state_);
            }
            return *this;
        }

        xml_document &operator=(xml_document &&right) noexcept {
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

        void swap(xml_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(state_, other.state_);
        }

        xml_document &operator+=(xml_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        /**
         * \lang english
         * @brief Indicates whether this element carries a name.
         *
         * The document root does not; every element below it does.
         *
         * \lang simp-chinese
         * @brief 指示该元素是否携带名称。
         *
         * 文档根不带；其下的每个元素都带。
         */
        bool has_name() const noexcept {
            return state_ != nullptr && !state_->name.empty();
        }

        /**
         * \lang english
         * @brief Returns the element name.
         *
         * \lang simp-chinese
         * @brief 返回元素名。
         */
        const string_type &name() const {
            if (state_ == nullptr || state_->name.empty()) {
                exceptions::willow::xml::throw_xml_type_error("node has no element name");
            }
            return state_->name;
        }

        void set_name(const string_type &value) {
            state().name = value;
        }

        /**
         * \lang english
         * @brief Returns the attributes in document order.
         *
         * \lang simp-chinese
         * @brief 按文档顺序返回属性。
         */
        const attribute_list &attributes() const {
            static const attribute_list empty{};
            return state_ != nullptr ? state_->attributes : empty;
        }

        attribute_list &attributes() {
            return state().attributes;
        }

        bool has_attribute(const string_type &attribute_name) const {
            return find_attribute(attribute_name) != nullptr;
        }

        const string_type *find_attribute(const string_type &attribute_name) const {
            if (state_ == nullptr) {
                return nullptr;
            }
            for (const attribute_type &attribute: state_->attributes) {
                if (attribute.name == attribute_name) {
                    return &attribute.value;
                }
            }
            return nullptr;
        }

        void add_attribute(const string_type &attribute_name, const string_type &attribute_value) {
            attribute_type &attribute = state().attributes.emplace_back();
            attribute.name = attribute_name;
            attribute.value = attribute_value;
        }

        /**
         * \lang english
         * @brief Indicates whether the text of this node was authored as a cdata section.
         *
         * \lang simp-chinese
         * @brief 指示该节点的文本是否以 cdata 段撰写。
         */
        bool is_cdata() const noexcept {
            return state_ != nullptr && state_->cdata;
        }

        void mark_cdata() {
            state().cdata = true;
        }

        void clear_cdata() noexcept {
            if (state_ != nullptr) {
                state_->cdata = false;
            }
        }

        /**
         * \lang english
         * @brief Indicates whether this node carries an xml declaration.
         *
         * Only the document root carries one.
         *
         * \lang simp-chinese
         * @brief 指示该节点是否携带 xml 声明。
         *
         * 仅文档根会携带。
         */
        bool has_xml_declaration() const noexcept {
            return state_ != nullptr && state_->declaration.present;
        }

        /**
         * \lang english
         * @brief Returns the xml declaration.
         *
         * \lang simp-chinese
         * @brief 返回 xml 声明。
         */
        const declaration_type &declaration() const {
            if (state_ == nullptr || !state_->declaration.present) {
                exceptions::willow::xml::throw_xml_type_error("node has no xml declaration");
            }
            return state_->declaration;
        }

        void set_declaration(const declaration_type &declaration) {
            declaration_type &target = state().declaration;
            target = declaration;
            target.present = true;
        }

        friend bool operator==(const xml_document &left, const xml_document &right) {
            if (!(static_cast<const base_type &>(left) == static_cast<const base_type &>(right))) {
                return false;
            }
            if (left.has_name() != right.has_name()) {
                return false;
            }
            if (left.has_name() && !(left.state_->name == right.state_->name)) {
                return false;
            }
            if (left.is_cdata() != right.is_cdata()) {
                return false;
            }
            if (left.attribute_count() != right.attribute_count()) {
                return false;
            }
            for (const attribute_type &attribute: left.attributes()) {
                const string_type *other = right.find_attribute(attribute.name);
                if (other == nullptr || !(*other == attribute.value)) {
                    return false;
                }
            }
            if (left.has_xml_declaration() != right.has_xml_declaration()) {
                return false;
            }
            if (!left.has_xml_declaration()) {
                return true;
            }
            const declaration_type &lhs = left.state_->declaration;
            const declaration_type &rhs = right.state_->declaration;
            return lhs.version == rhs.version && lhs.encoding == rhs.encoding && lhs.standalone == rhs.standalone;
        }

        friend bool operator!=(const xml_document &left, const xml_document &right) {
            return !(left == right);
        }

        friend bool operator<(const xml_document &left, const xml_document &right) {
            return static_cast<const base_type &>(left) < static_cast<const base_type &>(right);
        }

        friend bool operator<=(const xml_document &left, const xml_document &right) {
            return static_cast<const base_type &>(left) <= static_cast<const base_type &>(right);
        }

        friend bool operator>(const xml_document &left, const xml_document &right) {
            return static_cast<const base_type &>(left) > static_cast<const base_type &>(right);
        }

        friend bool operator>=(const xml_document &left, const xml_document &right) {
            return static_cast<const base_type &>(left) >= static_cast<const base_type &>(right);
        }

    private:
        struct node_state {
            string_type name;
            attribute_list attributes;
            bool cdata{false};
            declaration_type declaration{};
        };

        size_type attribute_count() const noexcept {
            return state_ != nullptr ? state_->attributes.size() : 0;
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
    using facade = xml_document<BasicDocument>;

    template <typename Ty, typename = void>
    struct has_xml_declaration : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_xml_declaration<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().has_xml_declaration())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_xml_declaration_v = has_xml_declaration<Ty>::value;

    template <typename Ty, typename = void>
    struct has_xml_attributes : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_xml_attributes<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().attributes()),
                                                                    decltype(std::declval<const Ty &>().has_name())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_xml_attributes_v = has_xml_attributes<Ty>::value;

    template <typename Ty, typename = void>
    struct has_xml_cdata : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_xml_cdata<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().is_cdata())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_xml_cdata_v = has_xml_cdata<Ty>::value;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::xml> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type =
            xml::xml_document<basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif
