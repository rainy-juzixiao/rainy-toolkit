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
#ifndef RAINY_FOUNDATION_AST_VISITOR_HPP
#define RAINY_FOUNDATION_AST_VISITOR_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/foundation/ast/node.hpp>

namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief Detects whether a visitor can accept a node of a given type.
     *
     * \lang simp-chinese
     * @brief 检测访问者是否可接收给定类型的节点。
     */
    template <typename Visitor, typename Node, typename = void>
    struct has_visit : type_traits::helper::false_type {};

    template <typename Visitor, typename Node>
    struct has_visit<Visitor, Node,
                     type_traits::other_trans::void_t<decltype(utility::declval<Visitor &>().visit(
                         utility::declval<Node>()))>> : type_traits::helper::true_type {};

    template <typename Visitor, typename Node>
    RAINY_CONSTEXPR_BOOL has_visit_v = has_visit<Visitor, Node>::value;

    template <typename Visitor, typename Node>
    RAINY_INLINE void invoke_visit(Visitor &visitor, Node node) {
        if constexpr (has_visit_v<Visitor, Node>) {
            visitor.visit(node);
        } else {
            (void) visitor;
            (void) node;
        }
    }

    /**
     * \lang english
     * @brief Visits every node of a subtree in pre-order.
     *
     * @brief @c visitor.visit(node) is invoked when well-formed and the traversal then descends into
     *        the node's children. A visitor that declares no @c visit overload still walks the tree.
     *
     * \lang simp-chinese
     * @brief 以前序遍历子树中的每个节点。
     *
     * @brief 当 @c visitor.visit(node) 合法时调用它，随后下降到该节点的子节点。未声明 @c visit
     *        重载的访问者仍会遍历整棵树。
     */
    template <typename Visitor>
    void traverse(Visitor &visitor, ast_node *node) {
        if (node == nullptr) {
            return;
        }
        invoke_visit(visitor, node);
        for (ast_node *child: node->children) {
            traverse(visitor, child);
        }
    }

    template <typename Visitor>
    void traverse(Visitor &visitor, ast_node &node) {
        traverse(visitor, &node);
    }

    template <typename Visitor>
    void traverse(Visitor &visitor, const ast_node *node) {
        if (node == nullptr) {
            return;
        }
        invoke_visit(visitor, node);
        for (const ast_node *child: node->children) {
            traverse(visitor, child);
        }
    }

    template <typename Visitor>
    void traverse(Visitor &visitor, const ast_node &node) {
        traverse(visitor, &node);
    }

    /**
     * \lang english
     * @brief Visits the direct children of a node in order.
     *
     * \lang simp-chinese
     * @brief 按序访问节点的直接子节点。
     */
    template <typename Visitor>
    void traverse_children(Visitor &visitor, ast_node *node) {
        if (node == nullptr) {
            return;
        }
        for (ast_node *child: node->children) {
            invoke_visit(visitor, child);
        }
    }

    template <typename Visitor>
    void traverse_children(Visitor &visitor, const ast_node *node) {
        if (node == nullptr) {
            return;
        }
        for (const ast_node *child: node->children) {
            invoke_visit(visitor, child);
        }
    }
}

#endif
