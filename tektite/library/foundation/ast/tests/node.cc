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
#include <rainy/foundation/ast/source_location.hpp>
#include <string_view>

using namespace rainy::foundation::ast;

TEST_CASE("ast source_location - default and comparison", "[ast][source_location]") {
    const source_location origin{};
    REQUIRE(origin.line == 1);
    REQUIRE(origin.column == 1);
    REQUIRE(origin.offset == 0);

    const source_location position{3, 7, 42};
    REQUIRE(position.line == 3);
    REQUIRE(position.column == 7);
    REQUIRE(position.offset == 42);
    REQUIRE(position == source_location(3, 7, 42));
    REQUIRE(position != origin);
}

TEST_CASE("ast source_span - emptiness and equality", "[ast][source_location]") {
    const source_span span{source_location(1, 1, 0), source_location(1, 5, 4)};
    REQUIRE_FALSE(span.empty());
    REQUIRE(span == source_span(source_location(1, 1, 0), source_location(1, 5, 4)));

    const source_span collapsed{source_location(2, 2, 9), source_location(2, 2, 9)};
    REQUIRE(collapsed.empty());
    REQUIRE(span != collapsed);
}

TEST_CASE("ast node - kind and children", "[ast][node]") {
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};

    ast_node *root = allocator.create<ast_node>(node_kind::object, &resource);
    ast_node *first = allocator.create<ast_node>(node_kind::literal, &resource);
    ast_node *second = allocator.create<ast_node>(node_kind::array, &resource);

    root->add_child(first);
    root->add_child(second);

    REQUIRE(root->kind == node_kind::object);
    REQUIRE(root->child_count() == 2);
    REQUIRE(root->child(0) == first);
    REQUIRE(root->child(1) == second);
    REQUIRE(root->child(2) == nullptr);
    REQUIRE(first->child_count() == 0);
    REQUIRE(std::string_view(root->kind_name()) == "object");
}

TEST_CASE("ast node - crtp base stamps the kind", "[ast][node]") {
    struct literal_node : ast_node_base<literal_node, node_kind::literal> {};

    literal_node node;
    REQUIRE(node.kind == node_kind::literal);
    REQUIRE(literal_node::static_kind == node_kind::literal);
    REQUIRE(std::string_view(node.self().kind_name()) == "literal");
}

TEST_CASE("ast kind helpers - generic vs user ranges", "[ast][node_kind]") {
    REQUIRE(is_generic_kind(node_kind::literal));
    REQUIRE_FALSE(is_generic_kind(node_kind::invalid));
    REQUIRE_FALSE(is_generic_kind(node_kind::generic));
    REQUIRE(is_user_kind(node_kind::user));
    REQUIRE(is_user_kind(node_kind::max));
    REQUIRE_FALSE(is_user_kind(node_kind::literal));
    REQUIRE(std::string_view(kind_name(node_kind::identifier)) == "identifier");
}
