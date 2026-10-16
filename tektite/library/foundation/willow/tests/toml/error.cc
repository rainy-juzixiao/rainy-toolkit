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
#include <rainy/foundation/willow/toml.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::toml;
using rainy::foundation::exceptions::willow::toml::toml_parse_error;

TEST_CASE("toml error - a line without a key or a header is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("just some text\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("= 1\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("key 1\n"), toml_parse_error);
}

TEST_CASE("toml error - an unclosed table header is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("[server\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("[server]\n[a.b\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("[[fruit]\n"), toml_parse_error);
}

TEST_CASE("toml error - an empty table header is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("[]\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("[ ]\n"), toml_parse_error);
}

TEST_CASE("toml error - a duplicate key is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = 1\na = 2\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("[s]\na = 1\na = 2\n"), toml_parse_error);
}

TEST_CASE("toml error - a duplicate table is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("[a]\nx = 1\n[a]\ny = 2\n"), toml_parse_error);
}

TEST_CASE("toml error - redefining a value as a table is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = 1\n[a]\nx = 2\n"), toml_parse_error);
}

TEST_CASE("toml error - extending an inline table is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = { x = 1 }\n[a.b]\ny = 2\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = { x = 1 }\na.b = 2\n"), toml_parse_error);
}

TEST_CASE("toml error - a malformed number is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = 01\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = 1.\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = .5\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = 1e\n"), toml_parse_error);
}

TEST_CASE("toml error - a malformed string is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = \"unterminated\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = \"bad \\x escape\"\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = 'unterminated\n"), toml_parse_error);
}

TEST_CASE("toml error - a malformed datetime is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = 1979-13-01\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = 1979-05-27T25:00:00\n"), toml_parse_error);
}

TEST_CASE("toml error - an unclosed array is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = [1, 2\n"), toml_parse_error);
}

TEST_CASE("toml error - an unclosed inline table is rejected", "[willow][toml]") {
    REQUIRE_THROWS_AS(toml::parse("a = { x = 1\n"), toml_parse_error);
    REQUIRE_THROWS_AS(toml::parse("a = { x = 1, }\n"), toml_parse_error);
}
