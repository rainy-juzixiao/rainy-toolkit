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
#include <rainy/foundation/willow/yaml.hpp>

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::yaml;
using rainy::foundation::exceptions::willow::yaml::yaml_alias_error;
using rainy::foundation::exceptions::willow::yaml::yaml_parse_error;
using rainy::foundation::exceptions::willow::yaml::yaml_scalar_error;

TEST_CASE("yaml error - unterminated quoted scalars", "[willow][yaml][error]") {
    REQUIRE_THROWS_AS(yaml::parse("\"unterminated"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse("'unterminated"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse("a: \"unterminated\n"), yaml_scalar_error);
}

TEST_CASE("yaml error - mapping key without a separator", "[willow][yaml][error]") {
    REQUIRE_THROWS_AS(yaml::parse("a: 1\nb\n"), yaml_parse_error);
    REQUIRE_THROWS_AS(yaml::parse("k: v\n  continuation\n"), yaml_parse_error);
}

TEST_CASE("yaml error - trailing tokens after the document", "[willow][yaml][error]") {
    REQUIRE_THROWS_AS(yaml::parse("a\nb\n"), yaml_parse_error);
    REQUIRE_THROWS_AS(yaml::parse("1\n2\n"), yaml_parse_error);
    REQUIRE_THROWS_AS(yaml::parse("a: 1\nb: 2\n---\nc: 3\n"), yaml_parse_error);
}

TEST_CASE("yaml error - malformed flow collections", "[willow][yaml][error]") {
    REQUIRE_THROWS_AS(yaml::parse("[1, 2\n"), yaml_parse_error);
    REQUIRE_THROWS_AS(yaml::parse("{a: 1\n"), yaml_parse_error);
    REQUIRE_THROWS_AS(yaml::parse("{a: 1 b: 2}\n"), yaml_parse_error);
    REQUIRE_THROWS_AS(yaml::parse("{a 1}\n"), yaml_parse_error);
}

TEST_CASE("yaml error - unknown alias", "[willow][yaml][error]") {
    REQUIRE_THROWS_AS(yaml::parse("*missing\n"), yaml_alias_error);
}

TEST_CASE("yaml error - error messages carry the failure site", "[willow][yaml][error]") {
    try {
        yaml::parse("k: v\n  continuation\n");
        FAIL("expected a parse error");
    } catch (const yaml_parse_error &ex) {
        const std::string message = ex.what();
        REQUIRE(message.find("expected ':' after mapping key") != std::string::npos);
    }

    try {
        yaml::parse("\"unterminated");
        FAIL("expected a scalar error");
    } catch (const yaml_scalar_error &ex) {
        const std::string message = ex.what();
        REQUIRE(message.find("unexpected end in quoted scalar") != std::string::npos);
    }
}
