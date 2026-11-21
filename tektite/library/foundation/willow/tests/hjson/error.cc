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
#include <catch2/catch_all.hpp>
#include <rainy/foundation/willow/hjson.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;
using rainy::foundation::exceptions::willow::hjson::hjson_parse_error;

TEST_CASE("hjson error - unterminated quoted string is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("\"unterminated"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("a: \"unterminated\n"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("'unterminated"), hjson_parse_error);
}

TEST_CASE("hjson error - unterminated multiline string is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("a: '''\ntext\n"), hjson_parse_error);
}

TEST_CASE("hjson error - unclosed containers are rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("{"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("["), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("{a: 1"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("[1, 2"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("{a: [1}"), hjson_parse_error);
}

TEST_CASE("hjson error - missing separator is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("a 1\n"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("{a 1}"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("{a: }"), hjson_parse_error);
}

TEST_CASE("hjson error - unterminated block comment is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("/* never closed\na: 1\n"), hjson_parse_error);
}

TEST_CASE("hjson error - trailing garbage after value is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("1 2"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("[1] trailing"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("a: 1 }"), hjson_parse_error);
}

TEST_CASE("hjson error - unexpected close brace at root is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("}"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("]"), hjson_parse_error);
}

TEST_CASE("hjson error - colon without key is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse(": 1\n"), hjson_parse_error);
}

TEST_CASE("hjson error - malformed unicode escapes are rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse(R"(a: "\uZZZZ")"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse(R"(a: "\u12")"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse(R"(a: "\u")"), hjson_parse_error);
}

TEST_CASE("hjson error - surrogate pairs must be well formed", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse(R"(a: "\ud83d")"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse(R"(a: "\ud83dA")"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse(R"(a: "\ud83d\ud83d")"), hjson_parse_error);
}

TEST_CASE("hjson error - mismatched container terminators are rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("{a: [1, 2}"), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse("[{a: 1]"), hjson_parse_error);
}

TEST_CASE("hjson error - repeated separators are rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("a:: 1\n"), hjson_parse_error);
}

TEST_CASE("hjson error - content after a closed root container is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("{a: 1} trailing"), hjson_parse_error);
}

TEST_CASE("hjson error - a container following a scalar value is rejected", "[willow][hjson][error]") {
    REQUIRE_THROWS_AS(hjson::parse("a: 1 {b: 2}\n"), hjson_parse_error);
}
