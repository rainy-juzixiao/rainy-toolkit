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
#include <catch2/catch_test_macros.hpp>
#include <memory_resource>
#include <rainy/foundation/ast/node.hpp>
#include <rainy/foundation/ast/node_allocator.hpp>
#include <rainy/foundation/ast/visitor.hpp>

using namespace rainy::foundation::ast;

namespace {
    struct counting_visitor {
        int visited = 0;

        void visit(ast_node *) {
            ++visited;
        }
    };

    struct silent_visitor {};

    ast_node *make_tree(std::pmr::memory_resource *resource, node_allocator<ast_node> &allocator) {
        ast_node *root = allocator.create<ast_node>(node_kind::object, resource);
        ast_node *child_a = allocator.create<ast_node>(node_kind::literal, resource);
        ast_node *child_b = allocator.create<ast_node>(node_kind::array, resource);
        ast_node *grandchild = allocator.create<ast_node>(node_kind::identifier, resource);
        child_b->add_child(grandchild);
        root->add_child(child_a);
        root->add_child(child_b);
        return root;
    }
}

TEST_CASE("ast visitor - pre-order visits every node", "[ast][visitor]") {
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = make_tree(&resource, allocator);

    counting_visitor visitor;
    traverse(visitor, root);
    REQUIRE(visitor.visited == 4);
}

TEST_CASE("ast visitor - a visitor without visit still walks the tree", "[ast][visitor]") {
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = make_tree(&resource, allocator);

    silent_visitor visitor;
    traverse(visitor, root);
    SUCCEED();
}

TEST_CASE("ast visitor - null nodes are ignored", "[ast][visitor]") {
    counting_visitor visitor;
    traverse(visitor, static_cast<ast_node *>(nullptr));
    REQUIRE(visitor.visited == 0);
}

TEST_CASE("ast visitor - children-only traversal does not descend", "[ast][visitor]") {
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = make_tree(&resource, allocator);

    counting_visitor visitor;
    traverse_children(visitor, root);
    REQUIRE(visitor.visited == 2);
}
