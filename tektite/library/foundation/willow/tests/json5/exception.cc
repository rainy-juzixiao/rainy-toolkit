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
namespace wex5 = rainy::foundation::exceptions::willow::json5;
namespace wnx = rainy::foundation::exceptions::willow;

TEST_CASE("json5 exception - parse failure is json5_parse_error", "[willow][json5]") {
    try {
        (void)json5::parse("{");
        FAIL("expected json5_parse_error");
    } catch (const wex5::json5_exception &ex) {
        REQUIRE(dynamic_cast<const wex5::json5_parse_error *>(&ex) != nullptr);
        REQUIRE(ex.what() != nullptr);
        REQUIRE(std::string(ex.what()).find("json5") != std::string::npos);
    }
}

TEST_CASE("json5 exception - catch json5_exception and willow base", "[willow][json5]") {
    REQUIRE_THROWS_AS(json5::parse("oops"), wex5::json5_exception);
    REQUIRE_THROWS_AS(json5::parse("01"), wnx::willow_exception);
}

TEST_CASE("json5 exception - model type error derives from willow_exception", "[willow][json5]") {
    json5::facade<document> j = 42;
    try {
        (void)j.as_string();
        FAIL("expected willow_type_error");
    } catch (const wnx::willow_exception &ex) {
        REQUIRE(dynamic_cast<const wnx::willow_type_error *>(&ex) != nullptr);
        REQUIRE(ex.what() != nullptr);
    }
}

TEST_CASE("json5 exception - wrong container access is willow_invalid_key", "[willow][json5]") {
    json5::facade<document> j = 42;
    try {
        (void)j["k"];
        FAIL("expected willow_invalid_key");
    } catch (const wnx::willow_exception &ex) {
        REQUIRE(dynamic_cast<const wnx::willow_invalid_key *>(&ex) != nullptr);
    }
}
