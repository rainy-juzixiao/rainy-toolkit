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

TEST_CASE("jsonnet evaluate - literals and arithmetic", "[jsonnet][evaluate]") {
    REQUIRE(run("null").is_null());
    REQUIRE(run("true").as_bool() == true);
    REQUIRE(run("1 + 2").as_integer() == 3);
    REQUIRE(run("2 * 3 + 4").as_integer() == 10);
    REQUIRE(run("2 + 3 * 4").as_integer() == 14);
    REQUIRE(run("10 - 2 - 3").as_integer() == 5);
    REQUIRE(run("7 / 2").as_float() == 3.5);
    REQUIRE(run("7 % 3").as_integer() == 1);
    REQUIRE(run("-7 % 3").as_integer() == 2);
    REQUIRE(run("2 << 3").as_integer() == 16);
    REQUIRE(run("16 >> 2").as_integer() == 4);
    REQUIRE(run("5 & 3").as_integer() == 1);
    REQUIRE(run("5 | 2").as_integer() == 7);
    REQUIRE(run("5 ^ 1").as_integer() == 4);
}

TEST_CASE("jsonnet evaluate - comparisons and booleans", "[jsonnet][evaluate]") {
    REQUIRE(run("1 < 2").as_bool());
    REQUIRE(run("2 <= 2").as_bool());
    REQUIRE(run("3 > 4").as_bool() == false);
    REQUIRE(run("1 == 1").as_bool());
    REQUIRE(run("1 != 2").as_bool());
    REQUIRE(run("\"a\" < \"b\"").as_bool());
    REQUIRE(run("true && false").as_bool() == false);
    REQUIRE(run("true || false").as_bool());
    REQUIRE(run("!true").as_bool() == false);
    REQUIRE(run("1 == 1.0").as_bool());
}

TEST_CASE("jsonnet evaluate - boolean operators short-circuit", "[jsonnet][evaluate]") {
    REQUIRE(run("false && error \"boom\"").as_bool() == false);
    REQUIRE(run("true || error \"boom\"").as_bool());
}

TEST_CASE("jsonnet evaluate - strings and concatenation", "[jsonnet][evaluate]") {
    REQUIRE(run("\"a\" + \"b\"").as_string() == "ab");
    REQUIRE(run("\"n=\" + 42").as_string() == "n=42");
    REQUIRE(run("\"x\" + true").as_string() == "xtrue");
    REQUIRE(run("\"a\" + [1, 2]").as_string() == "a[1, 2]");
}

TEST_CASE("jsonnet evaluate - conditionals", "[jsonnet][evaluate]") {
    REQUIRE(run("if true then 1 else 2").as_integer() == 1);
    REQUIRE(run("if false then 1 else 2").as_integer() == 2);
    REQUIRE(run("if false then 1").is_null());
}

TEST_CASE("jsonnet evaluate - arrays and indexing", "[jsonnet][evaluate]") {
    REQUIRE(run("[1, 2, 3]").size() == 3);
    REQUIRE(run("[1, 2, 3][1]").as_integer() == 2);
    REQUIRE(run("[[1, 2], [3, 4]][1][0]").as_integer() == 3);
    REQUIRE(run("[1, 2] + [3]").size() == 3);
    REQUIRE(run("[]").size() == 0);
    REQUIRE_THROWS_AS(run("[1][5]"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - string indexing", "[jsonnet][evaluate]") {
    REQUIRE(run("\"abc\"[0]").as_string() == "a");
    REQUIRE(run("\"abc\"[2]").as_string() == "c");
    REQUIRE_THROWS_AS(run("\"abc\"[3]"), jsonnet_evaluate_error);
    REQUIRE(run("\"\\u00e9\"[0]").as_string() == "\xc3\xa9");
    REQUIRE(run("\"\\u00e9x\"[1]").as_string() == "x");
    REQUIRE_THROWS_AS(run("\"\\u00e9\"[1]"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - slices", "[jsonnet][evaluate]") {
    REQUIRE(run("[0, 1, 2, 3, 4][1:3]").size() == 2);
    REQUIRE(run("[0, 1, 2, 3, 4][1:3][0]").as_integer() == 1);
    REQUIRE(run("[0, 1, 2, 3, 4][::2]").size() == 3);
    REQUIRE(run("[0, 1, 2, 3, 4][-2:]").size() == 2);
    REQUIRE(run("\"abcde\"[1:3]").as_string() == "bc");
    REQUIRE(run("[0, 1, 2, 3, 4][4:0:-1]").size() == 4);
}

TEST_CASE("jsonnet evaluate - reverse slices", "[jsonnet][evaluate]") {
    REQUIRE(run("[0, 1, 2, 3][::-1]").size() == 4);
    REQUIRE(run("[0, 1, 2, 3][::-1][0]").as_integer() == 3);
    REQUIRE(run("[0, 1, 2, 3][1::-1]").size() == 2);
    REQUIRE(run("[0, 1, 2, 3][:2:-1]").size() == 1);
    REQUIRE(run("\"abcd\"[::-1]").as_string() == "dcba");
}

TEST_CASE("jsonnet evaluate - string slices count code points", "[jsonnet][evaluate]") {
    REQUIRE(run("\"\\u00e9\"[0:1]").as_string() == "\xc3\xa9");
    REQUIRE(run("\"\\u00e9x\"[1:2]").as_string() == "x");
    REQUIRE(run("\"\\u00e9\\u00e8x\"[::-1]").as_string() == "x\xc3\xa8\xc3\xa9");
}

TEST_CASE("jsonnet evaluate - fractional indices are rejected", "[jsonnet][evaluate]") {
    REQUIRE(run("[10, 20][1.0]").as_integer() == 20);
    REQUIRE_THROWS_AS(run("[10, 20][1.9]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("\"abc\"[1.9]"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - object equality compares fields", "[jsonnet][evaluate]") {
    REQUIRE(run("{ a: 1 } == { a: 1 }").as_bool());
    REQUIRE(run("{ a: 1 } == { b: 2 }").as_bool() == false);
    REQUIRE(run("{ a: 1 } == { a: 2 }").as_bool() == false);
    REQUIRE(run("{ a: 1, b: 2 } == { b: 2, a: 1 }").as_bool());
    REQUIRE(run("{ a: { x: 1 } } == { a: { x: 1 } }").as_bool());
    REQUIRE(run("{ a: 1 } != { b: 1 }").as_bool());
}

TEST_CASE("jsonnet evaluate - defaults see earlier arguments", "[jsonnet][evaluate]") {
    REQUIRE(run("(function(a, b=a) b)(1)").as_integer() == 1);
    REQUIRE(run("(function(a, b=a + 1) b)(1)").as_integer() == 2);
    REQUIRE(run("(function(a, b=2) b)(1)").as_integer() == 2);
}

TEST_CASE("jsonnet evaluate - local bindings are recursive and lazy", "[jsonnet][evaluate]") {
    REQUIRE(run("local x = 1; x + 1").as_integer() == 2);
    REQUIRE(run("local x = 1, y = x + 1; y").as_integer() == 2);
    REQUIRE(run("local f(n) = if n <= 1 then 1 else n * f(n - 1); f(5)").as_integer() == 120);
}

TEST_CASE("jsonnet evaluate - functions with defaults and named arguments", "[jsonnet][evaluate]") {
    REQUIRE(run("local f(a, b) = a + b; f(1, 2)").as_integer() == 3);
    REQUIRE(run("local f(a, b = 10) = a + b; f(1)").as_integer() == 11);
    REQUIRE(run("local f(a, b) = a - b; f(b = 1, a = 5)").as_integer() == 4);
    REQUIRE(run("(function(x) x * 2)(21)").as_integer() == 42);
    REQUIRE_THROWS_AS(run("local f(a) = a; f()"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - objects and field access", "[jsonnet][evaluate]") {
    REQUIRE(run("{ a: 1, b: 2 }.a").as_integer() == 1);
    
    REQUIRE(run("{ \"k\": 5 }[\"k\"]").as_integer() == 5);
    REQUIRE(run("{ a: { b: 3 } }.a.b").as_integer() == 3);
    REQUIRE(run("{ [\"x\" + \"y\"]: 1 }.xy").as_integer() == 1);
    REQUIRE(run("{ f(x): x + 1 }.f(41)").as_integer() == 42);
    REQUIRE_THROWS_AS(run("{ a: 1 }.missing"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - object composition and self", "[jsonnet][evaluate]") {
    REQUIRE(run("({ a: 1 } + { b: 2 }).a").as_integer() == 1);
    REQUIRE(run("({ a: 1 } + { b: 2 }).b").as_integer() == 2);
    REQUIRE(run("({ a: 1 } + { a: 2 }).a").as_integer() == 2);
    REQUIRE(run("{ a: 1, b: self.a + 1 }.b").as_integer() == 2);
    REQUIRE(run("({ a: 1 } + { b: self.a + 10 }).b").as_integer() == 11);
    REQUIRE(run("({ a: 1 } + { b: self.a + 10, a: 5 }).b").as_integer() == 15);
}

TEST_CASE("jsonnet evaluate - super and inherited fields", "[jsonnet][evaluate]") {
    REQUIRE(run("({ a: 1 } + { a: super.a + 1 }).a").as_integer() == 2);
    REQUIRE(run("({ a: 1, b: 2 } + { b: super.b * 10 }).b").as_integer() == 20);
    REQUIRE(run("({ a: 1 } + { a +: 5 }).a").as_integer() == 6);
}

TEST_CASE("jsonnet evaluate - field visibility", "[jsonnet][evaluate]") {
    const document obj = run("{ visible: 1, hidden:: 2 }");
    REQUIRE(obj.contains("visible"));
    REQUIRE_FALSE(obj.contains("hidden"));
    REQUIRE(obj.size() == 1);
}

TEST_CASE("jsonnet evaluate - array comprehensions", "[jsonnet][evaluate]") {
    REQUIRE(run("[x for x in [1, 2, 3]]").size() == 3);
    REQUIRE(run("[x * 2 for x in [1, 2, 3]]")[2].as_integer() == 6);
    REQUIRE(run("[x for x in [1, 2, 3, 4] if x % 2 == 0]").size() == 2);
    REQUIRE(run("[x + y for x in [1, 2] for y in [10, 20]]").size() == 4);
    REQUIRE(run("[x + y for x in [1, 2] for y in [10, 20]]")[3].as_integer() == 22);
}

TEST_CASE("jsonnet evaluate - object comprehensions", "[jsonnet][evaluate]") {
    REQUIRE(run("{ [k]: 1 for k in [\"a\", \"b\"] }").size() == 2);
    REQUIRE(run("{ [k]: k + \"!\" for k in [\"a\"] }.a").as_string() == "a!");
    REQUIRE(run("{ [if x > 1 then \"big\" else null]: x for x in [1, 2] }").size() == 1);
}

TEST_CASE("jsonnet evaluate - in operator", "[jsonnet][evaluate]") {
    REQUIRE(run("\"a\" in { a: 1 }").as_bool());
    REQUIRE(run("\"z\" in { a: 1 }").as_bool() == false);
    
}

TEST_CASE("jsonnet evaluate - assert and error", "[jsonnet][evaluate]") {
    REQUIRE(run("assert true; 1").as_integer() == 1);
    REQUIRE_THROWS_AS(run("assert false; 1"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("assert false : \"custom\"; 1"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("error \"boom\""), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - string formatting", "[jsonnet][evaluate]") {
    REQUIRE(run("\"%d\" % 42").as_string() == "42");
    REQUIRE(run("\"%s=%s\" % [\"a\", \"b\"]").as_string() == "a=b");
    REQUIRE(run("\"%s\" % [1, 2, 3]").as_string() == "1");
    REQUIRE(run("\"100%%\" % []").as_string() == "100%");
    REQUIRE(run("\"%s\" % \"x\"").as_string() == "x");
}

TEST_CASE("jsonnet evaluate - laziness avoids unused errors", "[jsonnet][evaluate]") {
    REQUIRE(run("{ a: 1, b: error \"unused\" }.a").as_integer() == 1);
    REQUIRE(run("local x = error \"unused\"; 5").as_integer() == 5);
    REQUIRE(run("[1, error \"unused\"][0]").as_integer() == 1);
}

TEST_CASE("jsonnet evaluate - object locals see self and dollar", "[jsonnet][evaluate]") {
    REQUIRE(run("{ local x = self.a, a: 1, b: x }.b").as_integer() == 1);
    REQUIRE(run("{ a: 1, b: { c: $.a } }.b.c").as_integer() == 1);
    REQUIRE(run("{ a: 1, b: { a: 2, c: $.a } }.b.c").as_integer() == 1);
    REQUIRE(run("{ a: 10, g: (function(x) self.a + x)(1) }.g").as_integer() == 11);
}

TEST_CASE("jsonnet evaluate - object assertions run on manifestation", "[jsonnet][evaluate]") {
    REQUIRE_THROWS_AS(run("{ assert false : \"boom\", a: 1 }"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{ a: 1, assert self.a == 2 : \"boom\" }"), jsonnet_evaluate_error);
    REQUIRE(run("{ a: 1, assert self.a == 1 }.a").as_integer() == 1);
}

TEST_CASE("jsonnet evaluate - equality is structural for nested containers", "[jsonnet][evaluate]") {
    REQUIRE(run("[{a: 1}] == [{a: 1}]").as_bool());
    REQUIRE(run("[{a: 1}] == [{a: 2}]").as_bool() == false);
    REQUIRE(run("{ a: [1, {b: 2}] } == { a: [1, {b: 2}] }").as_bool());
    REQUIRE(run("{ a: [1, {b: 2}] } == { a: [1, {b: 3}] }").as_bool() == false);
}

TEST_CASE("jsonnet evaluate - ordering follows the jsonnet comparison rules", "[jsonnet][evaluate]") {
    REQUIRE(run("[1, 2] < [1, 3]").as_bool());
    REQUIRE(run("[1] < [1, 0]").as_bool());
    REQUIRE_THROWS_AS(run("1 < \"a\""), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("true < false"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - duplicate fields are rejected in comprehensions", "[jsonnet][evaluate]") {
    REQUIRE_THROWS_AS(run("{ [k]: 1 for k in [\"a\", \"a\"] }"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{ a: 1, a: 2 }"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("{ [\"a\"]: 1, [\"a\"]: 2 }"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - string conversion uses the shortest representation", "[jsonnet][evaluate]") {
    REQUIRE(run("\"\" + 0.1").as_string() == "0.1");
    REQUIRE(run("std.toString(1.5e-7)").as_string() == "1.5e-07");
}

TEST_CASE("jsonnet evaluate - non-finite numbers cannot be manifested", "[jsonnet][evaluate]") {
    REQUIRE_THROWS_AS(run("1e400"), jsonnet_evaluate_error);
}
