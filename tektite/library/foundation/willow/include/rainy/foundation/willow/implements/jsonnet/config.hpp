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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_CONFIG_HPP
#include <memory>
#include <memory_resource>
#include <rainy/core/memory/shared_ptr.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/jsonnet/ast.hpp>
#include <rainy/foundation/willow/implements/jsonnet/exceptions.hpp>
#include <rainy/foundation/willow/implements/jsonnet/version.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet {
    /**
     * \lang english
     * @brief Owns the syntax tree produced by the parser.
     *
     * \lang simp-chinese
     * @brief 拥有解析器产出的语法树。
     */
    struct ast_arena {
        std::pmr::monotonic_buffer_resource resource{};
        jsonnet_node_allocator allocator{&resource};
        jsonnet_node *root{nullptr};
    };

    using ast_handle = core::memory::shared_ptr<ast_arena>;

    inline ast_handle make_ast_handle() {
        return core::memory::make_shared<ast_arena>();
    }
}

namespace rainy::foundation::willow::jsonnet {
    struct jsonnet_node_tag {};

    template <typename BasicDocument>
    struct with_jsonnet_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_jsonnet_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc,
                                    jsonnet_node_tag>;
    };

    template <typename BasicDocument>
    struct without_jsonnet_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_jsonnet_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, jsonnet_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct jsonnet_document;

    template <typename BasicDocument>
    using jsonnet_document_base = typename with_jsonnet_node_tag<BasicDocument>::type;
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, jsonnet::jsonnet_node_tag> {
        using type = jsonnet::jsonnet_document<typename jsonnet::without_jsonnet_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::jsonnet {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    template <typename BasicDocument>
    struct jsonnet_document : public jsonnet_document_base<BasicDocument> {
        using base_type = jsonnet_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_jsonnet_node_tag<BasicDocument>::type>,
                      "jsonnet_document requires an untagged basic_document");

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

        jsonnet_document() = default;

        jsonnet_document(const base_type &right) : base_type(right) {
        }

        jsonnet_document(base_type &&right) : base_type(utility::move(right)) {
        }

        template <typename OtherDocument>
        jsonnet_document(from_other_document_t, const OtherDocument &right) :
            base_type(willow::implements::convert_document<base_type>(right)) {
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

        void swap(jsonnet_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(ast_, other.ast_);
        }

        jsonnet_document &operator+=(jsonnet_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        friend bool operator==(const jsonnet_document &left, const jsonnet_document &right) {
            return static_cast<const base_type &>(left) == static_cast<const base_type &>(right);
        }

        friend bool operator!=(const jsonnet_document &left, const jsonnet_document &right) {
            return static_cast<const base_type &>(left) != static_cast<const base_type &>(right);
        }

        friend bool operator<(const jsonnet_document &left, const jsonnet_document &right) {
            return static_cast<const base_type &>(left) < static_cast<const base_type &>(right);
        }

        friend bool operator<=(const jsonnet_document &left, const jsonnet_document &right) {
            return static_cast<const base_type &>(left) <= static_cast<const base_type &>(right);
        }

        friend bool operator>(const jsonnet_document &left, const jsonnet_document &right) {
            return static_cast<const base_type &>(left) > static_cast<const base_type &>(right);
        }

        friend bool operator>=(const jsonnet_document &left, const jsonnet_document &right) {
            return static_cast<const base_type &>(left) >= static_cast<const base_type &>(right);
        }

        bool has_ast() const noexcept {
            return ast_ != nullptr && ast_->root != nullptr;
        }

        const jsonnet_node *ast() const noexcept {
            return ast_ != nullptr ? ast_->root : nullptr;
        }

        jsonnet_node *ast() noexcept {
            return ast_ != nullptr ? ast_->root : nullptr;
        }

        const ast_handle &handle() const noexcept {
            return ast_;
        }

        void set_ast(ast_handle handle) {
            ast_ = utility::move(handle);
        }

    private:
        ast_handle ast_{};
    };

    template <typename BasicDocument = document>
    using facade = jsonnet_document<BasicDocument>;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::jsonnet> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type = jsonnet::jsonnet_document<
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif

#endif
