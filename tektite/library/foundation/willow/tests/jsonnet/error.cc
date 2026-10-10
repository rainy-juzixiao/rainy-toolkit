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

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::jsonnet;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_evaluate_error;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_parse_error;

namespace {
    document run(const char *source) {
        facade<document> parsed = jsonnet::parse(source);
        return evaluate(parsed);
    }
}

TEST_CASE("jsonnet error - parse failures", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(jsonnet::parse(""), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("{"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("[1 2]"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("local = 1; 2"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("if then 1"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("a."), jsonnet_parse_error);
    REQUIRE_THROWS_AS(jsonnet::parse("\"unterminated"), jsonnet_parse_error);
}

TEST_CASE("jsonnet parser - trailing commas are accepted", "[jsonnet][error]") {
    REQUIRE(jsonnet::parse("[1,]").has_ast());
    REQUIRE(jsonnet::parse("{ a: 1, }").has_ast());
    REQUIRE(jsonnet::parse("f(1,)").has_ast());
}

TEST_CASE("jsonnet error - undefined variables", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("x"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("x + 1"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("local y = 1; x"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet error - type errors", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("1 + true"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("1 + {}"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{} + 1"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("true + true"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("-true"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("if 1 then 2 else 3"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet error - division and modulo by zero", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("1 / 0"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("1 % 0"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet error - missing fields and indices", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("{ a: 1 }.b"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("[1, 2][5]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("1[0]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{}[0]"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet error - calling a non-function", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("1(2)"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{ a: 1 }.a()"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet error - error expression carries its message", "[jsonnet][error]") {
    try {
        run("error \"custom message\"");
        FAIL("expected an evaluate error");
    } catch (const jsonnet_evaluate_error &error) {
        REQUIRE(std::string(error.what()).find("custom message") != std::string::npos);
    }
}

TEST_CASE("jsonnet error - assert without message uses a default", "[jsonnet][error]") {
    try {
        run("assert false; 1");
        FAIL("expected an evaluate error");
    } catch (const jsonnet_evaluate_error &error) {
        REQUIRE(std::string(error.what()).find("assertion failed") != std::string::npos);
    }
}

TEST_CASE("jsonnet error - self outside an object", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("self"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("super"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet error - functions cannot be manifested", "[jsonnet][error]") {
    REQUIRE_THROWS_AS(run("function(x) x"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{ f(x): x }"), jsonnet_evaluate_error);
}
