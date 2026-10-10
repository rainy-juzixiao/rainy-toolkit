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
#ifndef RAINY_FOUNDATION_AST_NODE_HPP
#define RAINY_FOUNDATION_AST_NODE_HPP
#include <memory_resource>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/ast/node_allocator.hpp>
#include <rainy/foundation/ast/node_kind.hpp>
#include <rainy/foundation/ast/source_location.hpp>

namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief The base of every syntax node produced by a front-end.
     *
     * @brief A node carries its kind, its source span, and a uniform child list. Keeping children on
     *        the base lets the generic traversal and dump algorithms walk any front-end tree without
     *        knowing its concrete node layout.
     *
     * \lang simp-chinese
     * @brief 前端产生的每个语法节点的基类。
     *
     * @brief 节点携带种类、源区间与统一的子节点列表。子节点放在基类上，使通用遍历与导出算法
     *        无需了解前端的具体节点布局即可遍历任意前端语法树。
     */
    struct ast_node {
        node_kind kind{node_kind::invalid};
        source_span location{};
        node_buffer<ast_node *> children{};

        constexpr ast_node() noexcept = default;

        constexpr explicit ast_node(const node_kind kind) noexcept : kind(kind) {
        }

        ast_node(const node_kind kind, std::pmr::memory_resource *resource) : kind(kind), children(resource) {
        }

        void add_child(ast_node *child) {
            children.push_back(child);
        }

        std::size_t child_count() const noexcept {
            return children.size();
        }

        ast_node *child(const std::size_t index) const noexcept {
            return index < children.size() ? children[index] : nullptr;
        }

        const char *kind_name() const noexcept {
            return ast::kind_name(kind);
        }
    };

    /**
     * \lang english
     * @brief CRTP helper that stamps a compile-time kind onto a node type.
     *
     * \lang simp-chinese
     * @brief 将编译期种类盖印到节点类型上的 CRTP 辅助。
     */
    template <typename Derived, node_kind Kind>
    struct ast_node_base : ast_node {
        static constexpr node_kind static_kind = Kind;

        constexpr ast_node_base() noexcept : ast_node(Kind) {
        }

        explicit ast_node_base(std::pmr::memory_resource *resource) : ast_node(Kind, resource) {
        }

        constexpr Derived &self() noexcept {
            return static_cast<Derived &>(*this);
        }

        constexpr const Derived &self() const noexcept {
            return static_cast<const Derived &>(*this);
        }
    };
}

#endif
