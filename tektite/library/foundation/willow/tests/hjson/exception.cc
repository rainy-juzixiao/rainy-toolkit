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

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;
namespace wex = rainy::foundation::exceptions::willow::hjson;
namespace wnx = rainy::foundation::exceptions::willow;

TEST_CASE("hjson exception - parse failure is hjson_parse_error", "[willow][hjson][exception]") {
    try {
        (void)hjson::parse("{");
        FAIL("expected hjson_parse_error");
    } catch (const wex::hjson_exception &ex) {
        REQUIRE(dynamic_cast<const wex::hjson_parse_error *>(&ex) != nullptr);
        REQUIRE(ex.what() != nullptr);
        REQUIRE(std::string(ex.what()).find("hjson") != std::string::npos);
    }
}

TEST_CASE("hjson exception - catch hjson_exception and willow base", "[willow][hjson][exception]") {
    REQUIRE_THROWS_AS(hjson::parse("a: {"), wex::hjson_exception);
    REQUIRE_THROWS_AS(hjson::parse("a: {"), wnx::willow_exception);
}

TEST_CASE("hjson exception - model type error derives from willow_exception", "[willow][hjson][exception]") {
    const hjson::facade<document> doc = 42;
    try {
        (void)doc.as_string();
        FAIL("expected willow_type_error");
    } catch (const wnx::willow_exception &ex) {
        REQUIRE(dynamic_cast<const wnx::willow_type_error *>(&ex) != nullptr);
        REQUIRE(ex.what() != nullptr);
    }
}

TEST_CASE("hjson exception - wrong container access is willow_invalid_key", "[willow][hjson][exception]") {
    const hjson::facade<document> doc = 42;
    try {
        (void)doc["k"];
        FAIL("expected willow_invalid_key");
    } catch (const wnx::willow_exception &ex) {
        REQUIRE(dynamic_cast<const wnx::willow_invalid_key *>(&ex) != nullptr);
    }
}
