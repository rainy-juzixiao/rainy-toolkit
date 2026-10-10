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
#include <rainy/foundation/ast/ast_dump.hpp>
#include <rainy/foundation/ast/node.hpp>
#include <rainy/foundation/ast/node_allocator.hpp>

using namespace rainy::foundation::ast;

namespace {
    ast_node *make_dump_tree(std::pmr::memory_resource *resource, node_allocator<ast_node> &allocator) {
        ast_node *root = allocator.create<ast_node>(node_kind::object, resource);
        root->location = source_span{source_location(1, 1, 0), source_location(1, 2, 1)};
        ast_node *child = allocator.create<ast_node>(node_kind::literal, resource);
        child->location = source_span{source_location(1, 2, 1), source_location(1, 3, 2)};
        root->add_child(child);
        return root;
    }
}

TEST_CASE("ast dump - renders kind names with indentation", "[ast][dump]") {
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = make_dump_tree(&resource, allocator);

    dump_options options;
    options.indent = 2;
    options.annotations = false;

    const auto text = dump_to_string(root, options);
    REQUIRE(text == "object\n  literal\n");
}

TEST_CASE("ast dump - annotations include the begin position", "[ast][dump]") {
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = make_dump_tree(&resource, allocator);

    dump_options options;
    options.indent = 4;
    options.annotations = true;

    const auto text = dump_to_string(root, options);
    REQUIRE(text == "object @1:1\n    literal @1:2\n");
}

TEST_CASE("ast dump - a null root produces no output", "[ast][dump]") {
    const auto text = dump_to_string(static_cast<ast_node *>(nullptr));
    REQUIRE(text.empty());
}
