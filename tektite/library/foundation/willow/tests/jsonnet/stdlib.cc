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

namespace {
    document run(const char *source) {
        facade<document> parsed = jsonnet::parse(source);
        return evaluate(parsed);
    }
}

TEST_CASE("jsonnet std - length", "[jsonnet][stdlib]") {
    REQUIRE(run("std.length(\"abc\")").as_integer() == 3);
    REQUIRE(run("std.length([1, 2, 3])").as_integer() == 3);
    REQUIRE(run("std.length({ a: 1, b: 2 })").as_integer() == 2);
    REQUIRE(run("std.length(\"\")").as_integer() == 0);
    REQUIRE_THROWS_AS(run("std.length(42)"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet std - type", "[jsonnet][stdlib]") {
    REQUIRE(run("std.type(null)").as_string() == "null");
    REQUIRE(run("std.type(true)").as_string() == "boolean");
    REQUIRE(run("std.type(1)").as_string() == "number");
    REQUIRE(run("std.type(\"x\")").as_string() == "string");
    REQUIRE(run("std.type([])").as_string() == "array");
    REQUIRE(run("std.type({})").as_string() == "object");
    REQUIRE(run("std.type(function() 1)").as_string() == "function");
}

TEST_CASE("jsonnet std - toString", "[jsonnet][stdlib]") {
    REQUIRE(run("std.toString(42)").as_string() == "42");
    REQUIRE(run("std.toString(true)").as_string() == "true");
    REQUIRE(run("std.toString(\"x\")").as_string() == "x");
    REQUIRE(run("std.toString([1, 2])").as_string() == "[1, 2]");
}

TEST_CASE("jsonnet std - codepoint and char", "[jsonnet][stdlib]") {
    REQUIRE(run("std.codepoint(\"A\")").as_integer() == 65);
    REQUIRE(run("std.char(65)").as_string() == "A");
    REQUIRE(run("std.char(20013)").as_string() == "\xe4\xb8\xad");
}

TEST_CASE("jsonnet std - math functions", "[jsonnet][stdlib]") {
    REQUIRE(run("std.floor(2.7)").as_integer() == 2);
    REQUIRE(run("std.ceil(2.1)").as_integer() == 3);
    REQUIRE(run("std.abs(-3)").as_integer() == 3);
    REQUIRE(run("std.sqrt(16)").as_integer() == 4);
    REQUIRE(run("std.pow(2, 10)").as_integer() == 1024);
    REQUIRE(run("std.mod(7, 3)").as_integer() == 1);
    REQUIRE(run("std.mod(-7, 3)").as_integer() == 2);
    REQUIRE(run("std.min([3, 1, 2])").as_integer() == 1);
    REQUIRE(run("std.max([3, 1, 2])").as_integer() == 3);
}

TEST_CASE("jsonnet std - primitiveEquals", "[jsonnet][stdlib]") {
    REQUIRE(run("std.primitiveEquals(1, 1)").as_bool());
    REQUIRE(run("std.primitiveEquals(1, 2)").as_bool() == false);
    REQUIRE(run("std.primitiveEquals(\"a\", \"a\")").as_bool());
}

TEST_CASE("jsonnet std - join", "[jsonnet][stdlib]") {
    REQUIRE(run("std.join(\",\", [\"a\", \"b\", \"c\"])").as_string() == "a,b,c");
    REQUIRE(run("std.join(\"-\", [\"x\"])").as_string() == "x");
    REQUIRE(run("std.join(\",\", [])").as_string() == "");
    REQUIRE(run("std.join([0], [[1], [2], [3]])").size() == 5);
}
