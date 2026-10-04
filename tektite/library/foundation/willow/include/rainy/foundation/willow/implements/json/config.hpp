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
#ifndef RAINY_FOUNDATION_WILLOW_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_CONFIG_HPP
#include <rainy/foundation/willow/implements/common/config.hpp>

namespace rainy::foundation::willow::json::implements {
    enum class token_type {
        uninitialized,
        literal_true,
        literal_false,
        literal_null,
        value_string,
        value_integer,
        value_float,
        begin_array,
        end_array,
        begin_object,
        end_object,
        name_separator,
        value_separator,
        end_of_input
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

namespace rainy::foundation::willow::json {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    template <typename BasicDocument>
    struct json_document : private BasicDocument {
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
        using initializer_list = typename BasicDocument::initializer_list;
        using iterator = typename BasicDocument::iterator;
        using const_iterator = typename BasicDocument::const_iterator;
        using reverse_iterator = typename BasicDocument::reverse_iterator;
        using const_reverse_iterator = typename BasicDocument::const_reverse_iterator;
        using node_type = typename BasicDocument::node_type;

        using BasicDocument::BasicDocument;

        json_document() = default;

        json_document(const BasicDocument &value) : BasicDocument(value) {
        }

        json_document(BasicDocument &&value) : BasicDocument(utility::move(value)) {
        }

        template <typename OtherDocument>
        json_document(from_other_document_t, const OtherDocument &value) :
            BasicDocument(willow::implements::convert_document<BasicDocument>(value)) {
        }

        BasicDocument &as_document() noexcept {
            return *this;
        }

        const BasicDocument &as_document() const noexcept {
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

        void swap(json_document &right) noexcept {
            static_cast<BasicDocument &>(*this).swap(static_cast<BasicDocument &>(right));
        }

        json_document &operator+=(json_document &&right) {
            static_cast<BasicDocument &>(*this) += static_cast<BasicDocument &&>(static_cast<BasicDocument &>(right));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const BasicDocument &>(*this).template get<Ty>();
        }

        friend bool operator==(const json_document &left, const json_document &right) {
            return static_cast<const BasicDocument &>(left) == static_cast<const BasicDocument &>(right);
        }

        friend bool operator!=(const json_document &left, const json_document &right) {
            return static_cast<const BasicDocument &>(left) != static_cast<const BasicDocument &>(right);
        }

        friend bool operator<(const json_document &left, const json_document &right) {
            return static_cast<const BasicDocument &>(left) < static_cast<const BasicDocument &>(right);
        }

        friend bool operator<=(const json_document &left, const json_document &right) {
            return static_cast<const BasicDocument &>(left) <= static_cast<const BasicDocument &>(right);
        }

        friend bool operator>(const json_document &left, const json_document &right) {
            return static_cast<const BasicDocument &>(left) > static_cast<const BasicDocument &>(right);
        }

        friend bool operator>=(const json_document &left, const json_document &right) {
            return static_cast<const BasicDocument &>(left) >= static_cast<const BasicDocument &>(right);
        }
    };

    template <typename BasicDocument = document>
    using facade = json_document<BasicDocument>;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::json> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type =
            json::json_document<basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif
