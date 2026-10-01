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
#ifndef RAINY_FOUNDATION_WILLOW_YAML_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_YAML_CONFIG_HPP

#include <rainy/foundation/willow/implements/common/config.hpp>
#include <cstring>

namespace rainy::foundation::willow::yaml::implements {
    enum class token_type {
        uninitialized,
        literal_true,
        literal_false,
        literal_null,
        literal_null_tilde,
        value_string,
        value_integer,
        value_float,
        begin_flow_mapping,
        end_flow_mapping,
        begin_flow_sequence,
        end_flow_sequence,
        key_separator,
        value_separator,
        block_entry_indicator,
        anchor_indicator,
        alias_indicator,
        tag_indicator,
        directive_indicator,
        document_start,
        document_end,
        block_scalar_literal,
        block_scalar_folded,
        explicit_key,
        end_of_input
    };

    enum class scalar_style {
        plain,
        single_quoted,
        double_quoted,
        literal,
        folded,
        tagged,
        aliased
    };

    using willow::implements::merge_surrogates;
    using willow::implements::unicode_surrogate_base;
    using willow::implements::unicode_surrogate_lead_begin;
    using willow::implements::unicode_surrogate_lead_end;
    using willow::implements::unicode_surrogate_max_sur;
    using willow::implements::unicode_surrogate_bits;
    using willow::implements::unicode_surrogate_trail_begin;
    using willow::implements::unicode_surrogate_trail_end;
}

namespace rainy::foundation::willow::yaml {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::serializer_args;

    template <typename CharType>
    using document_stream = core::collections::vector<
        basic_document<collections::unordered_map, core::collections::vector, core::text::basic_string<CharType>>>;

    enum class node_notation {
        unspecified,
        block,
        flow,
        literal,
        folded
    };

    inline node_notation notation_from_name(const char *name, std::size_t length) noexcept {
        if (length == 4 && std::memcmp(name, "flow", 4) == 0) {
            return node_notation::flow;
        }
        if (length == 5 && std::memcmp(name, "block", 5) == 0) {
            return node_notation::block;
        }
        if (length == 6 && std::memcmp(name, "folded", 6) == 0) {
            return node_notation::folded;
        }
        if (length == 7 && std::memcmp(name, "literal", 7) == 0) {
            return node_notation::literal;
        }
        return node_notation::unspecified;
    }

    template <typename BasicDocument>
    struct yaml_document : private BasicDocument {
        template <typename Ty>
        using allocator_type = typename BasicDocument::template allocator_type<Ty>;

        using size_type = typename BasicDocument::size_type;
        using difference_type = typename BasicDocument::difference_type;
        using char_type = typename BasicDocument::char_type;
        using string_type = typename BasicDocument::string_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using state_type = typename BasicDocument::state_type;
        using initializer_list = typename BasicDocument::initializer_list;
        using iterator = typename BasicDocument::iterator;
        using const_iterator = typename BasicDocument::const_iterator;
        using reverse_iterator = typename BasicDocument::reverse_iterator;
        using const_reverse_iterator = typename BasicDocument::const_reverse_iterator;

        using BasicDocument::BasicDocument;

        yaml_document() = default;

        yaml_document(const BasicDocument &right) : BasicDocument(right) {
        }

        yaml_document(BasicDocument &&right) : BasicDocument(utility::move(right)) {
        }

        void set_notation(const node_notation notation) noexcept {
            notation_ = notation;
        }

        node_notation notation() const noexcept {
            return notation_;
        }

        BasicDocument &base() noexcept {
            return *this;
        }

        const BasicDocument &base() const noexcept {
            return *this;
        }

        using BasicDocument::is_object;
        using BasicDocument::is_array;
        using BasicDocument::is_string;
        using BasicDocument::is_bool;
        using BasicDocument::is_integer;
        using BasicDocument::is_float;
        using BasicDocument::is_number;
        using BasicDocument::is_null;
        using BasicDocument::is_primitive;
        using BasicDocument::type;
        using BasicDocument::type_name;
        using BasicDocument::has_state;
        using BasicDocument::state;
        using BasicDocument::begin;
        using BasicDocument::end;
        using BasicDocument::cbegin;
        using BasicDocument::cend;
        using BasicDocument::rbegin;
        using BasicDocument::rend;
        using BasicDocument::crbegin;
        using BasicDocument::crend;
        using BasicDocument::size;
        using BasicDocument::empty;
        using BasicDocument::contains;
        using BasicDocument::find;
        using BasicDocument::count;
        using BasicDocument::erase;
        using BasicDocument::push_back;
        using BasicDocument::emplace_back;
        using BasicDocument::clear;
        using BasicDocument::object;
        using BasicDocument::array;
        using BasicDocument::as_bool;
        using BasicDocument::as_integer;
        using BasicDocument::as_float;
        using BasicDocument::as_array;
        using BasicDocument::as_string;
        using BasicDocument::as_object;
        using BasicDocument::as_string_view;
        using BasicDocument::get;
        using BasicDocument::operator[];

        void swap(yaml_document &other) noexcept {
            static_cast<BasicDocument &>(*this).swap(static_cast<BasicDocument &>(other));
        }

        yaml_document &operator+=(yaml_document &&elem) {
            static_cast<BasicDocument &>(*this) += static_cast<BasicDocument &&>(static_cast<BasicDocument &>(elem));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const BasicDocument &>(*this).template get<Ty>();
        }

        string_type &anchor() {
            return this->state()["yaml.anchor"];
        }

        string_type &alias() {
            return this->state()["yaml.alias"];
        }

        string_type &tag() {
            return this->state()["yaml.tag"];
        }

        string_type &directive() {
            return this->state()["yaml.directive"];
        }

        string_type &style() {
            return this->state()["yaml.style"];
        }

        const string_type &anchor() const {
            return this->state().properties.at("yaml.anchor");
        }

        const string_type &alias() const {
            return this->state().properties.at("yaml.alias");
        }

        const string_type &tag() const {
            return this->state().properties.at("yaml.tag");
        }

        const string_type &directive() const {
            return this->state().properties.at("yaml.directive");
        }

        const string_type &style() const {
            if (notation_ == node_notation::unspecified) {
                return this->state().properties.at("yaml.style");
            }
            style_cache_ = style_name();
            return style_cache_;
        }

        string_type style_name() const {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                switch (notation_) {
                    case node_notation::block:
                        return "block";
                    case node_notation::flow:
                        return "flow";
                    case node_notation::literal:
                        return "literal";
                    case node_notation::folded:
                        return "folded";
                    default:
                        return string_type();
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                switch (notation_) {
                    case node_notation::block:
                        return L"block";
                    case node_notation::flow:
                        return L"flow";
                    case node_notation::literal:
                        return L"literal";
                    case node_notation::folded:
                        return L"folded";
                    default:
                        return string_type();
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                switch (notation_) {
                    case node_notation::block:
                        return u"block";
                    case node_notation::flow:
                        return u"flow";
                    case node_notation::literal:
                        return u"literal";
                    case node_notation::folded:
                        return u"folded";
                    default:
                        return string_type();
                }
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                switch (notation_) {
                    case node_notation::block:
                        return U"block";
                    case node_notation::flow:
                        return U"flow";
                    case node_notation::literal:
                        return U"literal";
                    case node_notation::folded:
                        return U"folded";
                    default:
                        return string_type();
                }
            } else {
#if RAINY_HAS_CXX20
                if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                    switch (notation_) {
                        case node_notation::block:
                            return u8"block";
                        case node_notation::flow:
                            return u8"flow";
                        case node_notation::literal:
                            return u8"literal";
                        case node_notation::folded:
                            return u8"folded";
                        default:
                            return string_type();
                    }
                }
#endif
                return string_type();
            }
        }

        bool has_anchor() const {
            return this->has_state() && this->state().contains("yaml.anchor");
        }

        bool has_alias() const {
            return this->has_state() && this->state().contains("yaml.alias");
        }

        bool has_tag() const {
            return this->has_state() && this->state().contains("yaml.tag");
        }

        bool has_directive() const {
            return this->has_state() && this->state().contains("yaml.directive");
        }

        bool has_style() const {
            return notation_ != node_notation::unspecified;
        }

        friend bool operator==(const yaml_document &left, const yaml_document &right) {
            return static_cast<const BasicDocument &>(left) == static_cast<const BasicDocument &>(right);
        }

        friend bool operator!=(const yaml_document &left, const yaml_document &right) {
            return static_cast<const BasicDocument &>(left) != static_cast<const BasicDocument &>(right);
        }

        friend bool operator<(const yaml_document &left, const yaml_document &right) {
            return static_cast<const BasicDocument &>(left) < static_cast<const BasicDocument &>(right);
        }

        friend bool operator<=(const yaml_document &left, const yaml_document &right) {
            return static_cast<const BasicDocument &>(left) <= static_cast<const BasicDocument &>(right);
        }

        friend bool operator>(const yaml_document &left, const yaml_document &right) {
            return static_cast<const BasicDocument &>(left) > static_cast<const BasicDocument &>(right);
        }

        friend bool operator>=(const yaml_document &left, const yaml_document &right) {
            return static_cast<const BasicDocument &>(left) >= static_cast<const BasicDocument &>(right);
        }

    private:
        node_notation notation_{node_notation::unspecified};
        mutable string_type style_cache_{};
    };

    template <typename BasicDocument = document>
    using facade = yaml_document<BasicDocument>;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::yaml> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type =
            yaml::yaml_document<basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif