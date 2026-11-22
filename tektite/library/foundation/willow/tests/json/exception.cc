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
#include <rainy/foundation/willow/json.hpp>

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json;
namespace wex = rainy::foundation::exceptions::willow::json;
namespace wnx = rainy::foundation::exceptions::willow;

TEST_CASE("json exception - model type error derives from willow_exception", "[willow][json]") {
    json::facade<document> j = 42;
    try {
        (void)j.as_string();
        FAIL("expected willow_type_error");
    } catch (const wnx::willow_exception &ex) {
        REQUIRE(dynamic_cast<const wnx::willow_type_error *>(&ex) != nullptr);
        REQUIRE(ex.what() != nullptr);
        REQUIRE(std::string(ex.what()).find("must be string") != std::string::npos);
    }
}

TEST_CASE("json exception - parse failure is json_parse_error", "[willow][json]") {
    try {
        (void)json::parse("{");
        FAIL("expected json_parse_error");
    } catch (const wex::json_exception &ex) {
        REQUIRE(dynamic_cast<const wex::json_parse_error *>(&ex) != nullptr);
    }
}

TEST_CASE("json exception - wrong container access is willow_invalid_key", "[willow][json]") {
    json::facade<document> j = 42;
    try {
        (void)j["k"];
        FAIL("expected willow_invalid_key");
    } catch (const wnx::willow_exception &ex) {
        REQUIRE(dynamic_cast<const wnx::willow_invalid_key *>(&ex) != nullptr);
    }
}

TEST_CASE("json exception - catch language and model base exceptions", "[willow][json]") {
    REQUIRE_THROWS_AS(json::parse("oops"), wex::json_exception);

    json::facade<document> j = "s";
    REQUIRE_THROWS_AS(j.as_bool(), wnx::willow_exception);
}
