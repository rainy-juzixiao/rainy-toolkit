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

#include <cstring>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>

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
    using willow::implements::unicode_surrogate_bits;
    using willow::implements::unicode_surrogate_lead_begin;
    using willow::implements::unicode_surrogate_lead_end;
    using willow::implements::unicode_surrogate_max_sur;
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

    struct yaml_node_tag {};

    template <typename BasicDocument>
    struct with_yaml_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_yaml_node_tag<basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, yaml_node_tag>;
    };

    template <typename BasicDocument>
    struct without_yaml_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_yaml_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, yaml_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct yaml_document;

    template <typename BasicDocument>
    using yaml_document_base = typename with_yaml_node_tag<BasicDocument>::type;
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, yaml::yaml_node_tag> {
        using type = yaml::yaml_document<typename yaml::without_yaml_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::yaml {
    template <typename BasicDocument>
    struct yaml_document : public yaml_document_base<BasicDocument> {
        using base_type = yaml_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_yaml_node_tag<BasicDocument>::type>,
                      "yaml_document requires an untagged basic_document");

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

        using base_type::base_type;

        yaml_document() = default;

        yaml_document(const base_type &right) : base_type(right) {
        }

        yaml_document(base_type &&right) : base_type(utility::move(right)) {
        }

        yaml_document(const yaml_document &right) :
            base_type(static_cast<const base_type &>(right)), state_(clone_state(right.state_)) {
        }

        yaml_document(yaml_document &&right) noexcept :
            base_type(utility::move(static_cast<base_type &>(right))), state_(right.state_) {
            right.state_ = nullptr;
        }

        ~yaml_document() {
            release_state();
        }

        yaml_document &operator=(const yaml_document &right) {
            if (this != &right) {
                base_type::operator=(static_cast<const base_type &>(right));
                if (right.state_ != nullptr) {
                    release_state();
                    state_ = clone_state(right.state_);
                }
            }
            return *this;
        }

        yaml_document &operator=(yaml_document &&right) noexcept {
            if (this != &right) {
                base_type::operator=(utility::move(static_cast<base_type &>(right)));
                if (right.state_ != nullptr) {
                    release_state();
                    state_ = right.state_;
                    right.state_ = nullptr;
                }
            }
            return *this;
        }

        base_type &base() noexcept {
            return *this;
        }

        const base_type &base() const noexcept {
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

        void swap(yaml_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(state_, other.state_);
        }

        yaml_document &operator+=(yaml_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        string_type &anchor() {
            return state().anchor;
        }

        string_type &alias() {
            return state().alias;
        }

        string_type &tag() {
            return state().tag;
        }

        string_type &directive() {
            return state().directive;
        }

        string_type &style() {
            return state().style;
        }

        const string_type &anchor() const {
            return require_field(state_ ? &state_->anchor : nullptr, "anchor");
        }

        const string_type &alias() const {
            return require_field(state_ ? &state_->alias : nullptr, "alias");
        }

        const string_type &tag() const {
            return require_field(state_ ? &state_->tag : nullptr, "tag");
        }

        const string_type &directive() const {
            return require_field(state_ ? &state_->directive : nullptr, "directive");
        }

        const string_type &style() const {
            return require_field(state_ ? &state_->style : nullptr, "style");
        }

        void set_style(const char *const narrow) {
            state().style = narrow_style(narrow);
        }

        void set_style(const bool condition, const char *const when_true, const char *const when_false) {
            state().style = narrow_style(condition ? when_true : when_false);
        }

        string_type narrow_style(const char *const narrow) const {
            string_type result;
            for (const char *p = narrow; *p != 0; ++p) {
                result.push_back(static_cast<char_type>(*p));
            }
            return result;
        }

        bool has_anchor() const {
            return state_ != nullptr && !state_->anchor.empty();
        }

        bool has_alias() const {
            return state_ != nullptr && !state_->alias.empty();
        }

        bool has_tag() const {
            return state_ != nullptr && !state_->tag.empty();
        }

        bool has_directive() const {
            return state_ != nullptr && !state_->directive.empty();
        }

        bool has_style() const {
            return state_ != nullptr && !state_->style.empty();
        }

    private:
        struct node_state {
            string_type anchor;
            string_type alias;
            string_type tag;
            string_type directive;
            string_type style;
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

        const string_type &require_field(const string_type *field, const char *const what) const {
            if (field == nullptr || field->empty()) {
                core::exceptions::logic::throw_out_of_range(what);
            }
            return *field;
        }

        node_state *state_{nullptr};
    };

    template <typename BasicDocument = document>
    using facade = yaml_document<BasicDocument>;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::yaml> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t, typename FloatingType = double,
                  typename BooleanType = bool, template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type =
            yaml::yaml_document<basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif
