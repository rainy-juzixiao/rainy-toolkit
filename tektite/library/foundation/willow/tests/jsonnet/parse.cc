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
#include <rainy/foundation/willow/jsonnet.hpp>

namespace rast = rainy::foundation::ast;
namespace rcore = rainy::core;

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::jsonnet;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_parse_error;

namespace {
    facade<document> parse_text(const char *source) {
        return jsonnet::parse(source);
    }

    const jsonnet_node *root_of(const facade<document> &doc) {
        return doc.ast();
    }

    rcore::text::string dump1(const facade<document> &doc) {
        rast::dump_options options;
        options.indent = 1;
        options.annotations = true;
        return dump_ast(doc, options);
    }
}

TEST_CASE("jsonnet parser - literals", "[jsonnet][parser]") {
    REQUIRE(root_of(parse_text("null"))->json_kind == jsonnet_node_kind::null_literal);
    REQUIRE(root_of(parse_text("true"))->boolean == true);
    REQUIRE(root_of(parse_text("false"))->boolean == false);
    REQUIRE(root_of(parse_text("42"))->number == 42.0);
    REQUIRE(root_of(parse_text("42"))->has_flag(flag_integer));
    REQUIRE(root_of(parse_text("4.5"))->number == 4.5);
    REQUIRE_FALSE(root_of(parse_text("4.5"))->has_flag(flag_integer));
    REQUIRE(root_of(parse_text("\"hi\""))->text == "hi");
}

TEST_CASE("jsonnet parser - binary precedence and associativity", "[jsonnet][parser]") {
    const auto text = dump1(parse_text("1 + 2 * 3"));
    REQUIRE(text == "binary @1:1\n"
                     " number @1:1\n"
                     " binary @1:5\n"
                     "  number @1:5\n"
                     "  number @1:9\n");

    const auto text2 = dump1(parse_text("1 - 2 - 3"));
    REQUIRE(text2 == "binary @1:1\n"
                      " binary @1:1\n"
                      "  number @1:1\n"
                      "  number @1:5\n"
                      " number @1:9\n");
}

TEST_CASE("jsonnet parser - unary and postfix", "[jsonnet][parser]") {
    REQUIRE(dump1(parse_text("-1")) == "unary @1:1\n number @1:2\n");
    REQUIRE(dump1(parse_text("a.b.c")) ==
            "field_access @1:1\n"
            " field_access @1:1\n"
            "  identifier @1:1\n"
            "  string @1:3\n"
            " string @1:5\n");
    REQUIRE(dump1(parse_text("a[0]")) == "index @1:1\n identifier @1:1\n number @1:3\n");
}

TEST_CASE("jsonnet parser - slices", "[jsonnet][parser]") {
    const auto doc = parse_text("a[1:2:3]");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::slice);
    REQUIRE(node->has_flag(flag_slice_begin));
    REQUIRE(node->has_flag(flag_slice_end));
    REQUIRE(node->has_flag(flag_slice_step));
    REQUIRE(node->child_count() == 4);
}

TEST_CASE("jsonnet parser - calls with positional and named arguments", "[jsonnet][parser]") {
    const auto doc = parse_text("f(1, x = 2)");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::call);
    REQUIRE(node->at(1)->json_kind == jsonnet_node_kind::argument);
    REQUIRE(node->at(2)->json_kind == jsonnet_node_kind::named_argument);
    REQUIRE(node->at(2)->text == "x");
}

TEST_CASE("jsonnet parser - local bindings are recursive", "[jsonnet][parser]") {
    const auto doc = parse_text("local x = 1, y = x; y");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::local);
    REQUIRE(node->at(0)->json_kind == jsonnet_node_kind::bind);
    REQUIRE(node->at(0)->text == "x");
    REQUIRE(node->at(1)->text == "y");
    REQUIRE(node->at(2)->json_kind == jsonnet_node_kind::identifier);
}

TEST_CASE("jsonnet parser - conditional with and without else", "[jsonnet][parser]") {
    REQUIRE(root_of(parse_text("if a then b else c"))->has_flag(flag_has_else));
    REQUIRE_FALSE(root_of(parse_text("if a then b"))->has_flag(flag_has_else));
    REQUIRE(root_of(parse_text("if a then b"))->child_count() == 3);
    REQUIRE(root_of(parse_text("if a then b"))->at(2)->json_kind == jsonnet_node_kind::null_literal);
}

TEST_CASE("jsonnet parser - functions and parameters", "[jsonnet][parser]") {
    const auto doc = parse_text("function(a, b = 1) a");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::function);
    REQUIRE(node->at(0)->json_kind == jsonnet_node_kind::parameter);
    REQUIRE(node->at(0)->text == "a");
    REQUIRE_FALSE(node->at(0)->has_flag(flag_has_default));
    REQUIRE(node->at(1)->has_flag(flag_has_default));
}

TEST_CASE("jsonnet parser - objects with visibility and methods", "[jsonnet][parser]") {
    const auto doc = parse_text("{ a: 1, b:: 2, c::: 3, d(x): x }");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::object);
    REQUIRE(node->child_count() == 4);
    REQUIRE(node->at(0)->visibility == field_visibility::visible);
    REQUIRE(node->at(1)->visibility == field_visibility::hidden);
    REQUIRE(node->at(2)->visibility == field_visibility::forced_visible);
    REQUIRE(node->at(3)->has_flag(flag_method));
}

TEST_CASE("jsonnet parser - computed and inherited fields", "[jsonnet][parser]") {
    const auto doc = parse_text("{ [\"k\"]: 1, a +: 2 }");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->at(0)->has_flag(flag_computed_name));
    REQUIRE(node->at(1)->has_flag(flag_inherits));
}

TEST_CASE("jsonnet parser - object composition is sugar for +", "[jsonnet][parser]") {
    const auto doc = parse_text("a { b: 1 }");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::binary);
    REQUIRE(node->text == "+");
}

TEST_CASE("jsonnet parser - arrays and comprehensions", "[jsonnet][parser]") {
    REQUIRE(root_of(parse_text("[1, 2, 3]"))->json_kind == jsonnet_node_kind::array);
    REQUIRE(root_of(parse_text("[]"))->child_count() == 0);
    const auto doc = parse_text("[x for x in y if x > 1]");
    const jsonnet_node *comp = root_of(doc);
    REQUIRE(comp->json_kind == jsonnet_node_kind::array_comprehension);
    REQUIRE(comp->at(1)->json_kind == jsonnet_node_kind::comprehension_for);
    REQUIRE(comp->at(2)->json_kind == jsonnet_node_kind::comprehension_if);
}

TEST_CASE("jsonnet parser - object comprehension", "[jsonnet][parser]") {
    const auto doc = parse_text("{ [k]: v for k in ks }");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::object_comprehension);
    REQUIRE(node->at(1)->json_kind == jsonnet_node_kind::comprehension_for);
}

TEST_CASE("jsonnet parser - assert, error and import", "[jsonnet][parser]") {
    REQUIRE(root_of(parse_text("assert x; y"))->json_kind == jsonnet_node_kind::assert);
    REQUIRE(root_of(parse_text("assert x : \"m\"; y"))->child_count() == 3);
    REQUIRE(root_of(parse_text("error \"boom\""))->json_kind == jsonnet_node_kind::error);
    REQUIRE(root_of(parse_text("import \"a.libsonnet\""))->json_kind == jsonnet_node_kind::import_expression);
    REQUIRE(root_of(parse_text("importstr \"a.txt\""))->json_kind == jsonnet_node_kind::import_string);
    REQUIRE(root_of(parse_text("importbin \"a.bin\""))->json_kind == jsonnet_node_kind::import_binary);
    REQUIRE(root_of(parse_text("import \"a.libsonnet\""))->text == "a.libsonnet");
}

TEST_CASE("jsonnet parser - self, super, dollar and in super", "[jsonnet][parser]") {
    REQUIRE(root_of(parse_text("self"))->json_kind == jsonnet_node_kind::self_reference);
    REQUIRE(root_of(parse_text("super"))->json_kind == jsonnet_node_kind::super_reference);
    REQUIRE(root_of(parse_text("$"))->json_kind == jsonnet_node_kind::dollar);
    const auto doc = parse_text("\"k\" in super");
    const jsonnet_node *node = root_of(doc);
    REQUIRE(node->json_kind == jsonnet_node_kind::binary);
    REQUIRE(node->at(1)->json_kind == jsonnet_node_kind::super_reference);
}

TEST_CASE("jsonnet parser - errors on malformed input", "[jsonnet][parser]") {
    REQUIRE_THROWS_AS(jsonnet::parse("1 +"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("{ a 1 }"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("[1, 2"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("local x = 1"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("1 2"), jsonnet_parse_error);
}

TEST_CASE("jsonnet parser - expression tree survives the document", "[jsonnet][parser]") {
    facade<document> doc = parse_text("{ a: [1, 2, 3], b: { c: true } }");
    REQUIRE(doc.has_ast());
    const jsonnet_node *root = doc.ast();
    REQUIRE(root->json_kind == jsonnet_node_kind::object);
    facade<document> copy = doc;
    REQUIRE(copy.has_ast());
    REQUIRE(copy.ast() == doc.ast());
}
