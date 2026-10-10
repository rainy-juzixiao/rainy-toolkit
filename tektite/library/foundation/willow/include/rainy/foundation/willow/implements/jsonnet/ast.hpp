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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_AST_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_AST_HPP
#include <cstdint>
#include <memory_resource>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/ast/ast_dump.hpp>
#include <rainy/foundation/ast/node.hpp>
#include <rainy/foundation/ast/node_allocator.hpp>
#include <rainy/foundation/ast/node_kind.hpp>
#include <rainy/foundation/willow/implements/jsonnet/version.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet {
    using ast_string = core::text::basic_string<char, core::text::char_traits<char>, std::pmr::polymorphic_allocator<char>>;

    /**
     * \lang english
     * @brief The jsonnet-specific node kinds, layered above the generic vocabulary.
     *
     * \lang simp-chinese
     * @brief jsonnet 专有的节点种类，叠加在通用词汇之上。
     */
    enum class jsonnet_node_kind : std::uint16_t {
        null_literal = 1,
        boolean_literal,
        number_literal,
        string_literal,
        identifier,
        self_reference,
        super_reference,
        dollar,
        unary,
        binary,
        conditional,
        field_access,
        index,
        slice,
        call,
        argument,
        named_argument,
        function,
        parameter,
        object,
        object_field,
        object_comprehension,
        array,
        array_comprehension,
        local,
        bind,
        assert,
        error,
        import_expression,
        import_string,
        import_binary,
        comprehension_for,
        comprehension_if,
    };

    constexpr ast::node_kind to_node_kind(const jsonnet_node_kind kind) noexcept {
        return static_cast<ast::node_kind>(static_cast<std::uint16_t>(kind) | static_cast<std::uint16_t>(ast::node_kind::generic));
    }

    constexpr bool is_jsonnet_kind(const ast::node_kind kind) noexcept {
        return (static_cast<std::uint16_t>(kind) & static_cast<std::uint16_t>(ast::node_kind::generic)) != 0;
    }

    constexpr jsonnet_node_kind jsonnet_kind_of(const ast::node_kind kind) noexcept {
        return static_cast<jsonnet_node_kind>(static_cast<std::uint16_t>(kind) & 0x3FFFu);
    }

    const char *jsonnet_kind_name(ast::node_kind kind) noexcept;

    inline const char *node_name(const ast::node_kind kind) noexcept {
        return is_jsonnet_kind(kind) ? jsonnet_kind_name(kind) : ast::kind_name(kind);
    }

    enum class field_visibility : std::uint8_t {
        visible,
        hidden,
        forced_visible,
    };

    enum node_flags : std::uint16_t {
        flag_none = 0,
        flag_integer = 1u << 0,
        flag_inherits = 1u << 1,
        flag_method = 1u << 2,
        flag_computed_name = 1u << 3,
        flag_has_default = 1u << 4,
        flag_slice_begin = 1u << 5,
        flag_slice_end = 1u << 6,
        flag_slice_step = 1u << 7,
        flag_tailstrict = 1u << 8,
        flag_has_else = 1u << 9,
    };

    struct jsonnet_node : ast::ast_node {
        jsonnet_node_kind json_kind{jsonnet_node_kind::null_literal};
        field_visibility visibility{field_visibility::visible};
        std::uint16_t flags{flag_none};
        ast_string text{};
        double number{0.0};
        bool boolean{false};

        jsonnet_node() = default;

        jsonnet_node(const jsonnet_node_kind kind, std::pmr::memory_resource *resource) :
            ast::ast_node(to_node_kind(kind), resource), json_kind(kind) {
        }

        bool has_flag(const std::uint16_t flag) const noexcept {
            return (flags & flag) != 0;
        }

        void set_flag(const std::uint16_t flag) noexcept {
            flags |= flag;
        }

        jsonnet_node *at(const std::size_t index) const noexcept {
            return static_cast<jsonnet_node *>(child(index));
        }
    };

    using jsonnet_node_allocator = ast::node_allocator<jsonnet_node>;

    inline jsonnet_node *make_node(jsonnet_node_allocator &allocator, const jsonnet_node_kind kind,
                                   std::pmr::memory_resource *resource) {
        jsonnet_node *node = allocator.create<jsonnet_node>(kind, resource);
        node->text = ast_string{resource};
        return node;
    }
}

#endif

#endif
