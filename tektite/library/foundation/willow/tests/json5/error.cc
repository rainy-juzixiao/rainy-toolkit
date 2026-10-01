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
#include <rainy/foundation/willow/json5.hpp>

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json5;
using rainy::foundation::exceptions::willow::json5::json5_parse_error;

TEST_CASE("json5 error - invalid numbers", "[willow][json5]") {
    REQUIRE_THROWS_AS(json5::parse("01"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("00"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("07"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("0x"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("0xg"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("+"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("-"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("."), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("-."), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(".e5"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("1e"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("1e+"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("-x"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("0x1.5"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("1.2.3"), json5_parse_error);
}

TEST_CASE("json5 error - invalid escapes", "[willow][json5]") {
    REQUIRE_THROWS_AS(json5::parse(R"("\1")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\8")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\01")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\00")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\xZZ")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\x4")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\u12")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\uD800")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(R"("\uD800\uD800")"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("\"abc\ndef\""), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("\"abc\ref\""), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("\"unterminated"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("'unterminated"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("\"mismatch'"), json5_parse_error);
}

TEST_CASE("json5 error - invalid comments and structure", "[willow][json5]") {
    REQUIRE_THROWS_AS(json5::parse(""), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("/"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("/ x"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("/* abc"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("["), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("[,,]"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("[1,,]"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{,}"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{a:1,,}"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{1:2}"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{a 1}"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{a,}"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{a}"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("[1 2]"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("[1, 2"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{a:1"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("[-]"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("nul"), json5_parse_error);
}

TEST_CASE("json5 error - trailing garbage", "[willow][json5]") {
    REQUIRE_THROWS_AS(json5::parse("true false"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("1 2"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("42 42"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("truex"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("nullx"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("Infinityx"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("NaNx"), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse("{\"a\"}"), json5_parse_error);
}
