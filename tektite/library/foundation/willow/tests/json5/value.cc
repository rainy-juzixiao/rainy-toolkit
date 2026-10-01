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

#include <cmath>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json5;

TEST_CASE("json5 value - decoded string content", "[willow][json5]") {
    REQUIRE(json5::parse("'a\\tb'").as_string() == "a\tb");
    REQUIRE(json5::parse("\"line1\\\nline2\"").as_string() == "line1line2");
    REQUIRE(json5::parse("'\\x41\\x42'").as_string() == "AB");
    REQUIRE(json5::parse("'\\u0041'").as_string() == "A");
    REQUIRE(json5::parse("'a\\\\b'").as_string() == "a\\b");
    REQUIRE(json5::parse("'don\\'t'").as_string() == "don't");
}

TEST_CASE("json5 value - integer and float classification", "[willow][json5]") {
    REQUIRE(json5::parse("42").is_integer());
    REQUIRE(json5::parse("+42").is_integer());
    REQUIRE(json5::parse("0x10").is_integer());
    REQUIRE(json5::parse("0x10").as_integer() == 16);
    REQUIRE(json5::parse("-0x10").as_integer() == -16);
    REQUIRE(json5::parse("42.0").is_float());
    REQUIRE(json5::parse("+.5").is_float());
    REQUIRE(json5::parse("5.").is_float());
    REQUIRE(json5::parse("1e2").is_float());
    REQUIRE(json5::parse("1e2").as_float() == 100.0);
    REQUIRE(json5::parse("0xFFFFFFFFFFFFFFFF").is_float());
}

TEST_CASE("json5 value - special float literals", "[willow][json5]") {
    json5::facade<document> inf = json5::parse("Infinity");
    REQUIRE(inf.is_float());
    REQUIRE(std::isinf(inf.as_float()));
    REQUIRE(inf.as_float() > 0);

    json5::facade<document> ninf = json5::parse("-Infinity");
    REQUIRE(ninf.is_float());
    REQUIRE(std::isinf(ninf.as_float()));
    REQUIRE(ninf.as_float() < 0);

    json5::facade<document> nan = json5::parse("NaN");
    REQUIRE(nan.is_float());
    REQUIRE(std::isnan(nan.as_float()));
}

TEST_CASE("json5 value - container predicates", "[willow][json5]") {
    json5::facade<document> j = json5::parse("{list:[1,'x',{k:.5,}],}");
    REQUIRE(j.is_object());
    REQUIRE(j.size() == 1);
    REQUIRE(j["list"].is_array());
    REQUIRE(j["list"].size() == 3);
    REQUIRE(j["list"][0].is_integer());
    REQUIRE(j["list"][1].is_string());
    REQUIRE(j["list"][2].is_object());
    REQUIRE(j["list"][2]["k"].is_float());
    REQUIRE(j["list"][2]["k"].as_float() == 0.5);
}

TEST_CASE("json5 value - boolean and null predicates", "[willow][json5]") {
    REQUIRE(json5::parse("true").is_bool());
    REQUIRE(json5::parse("false").is_bool());
    REQUIRE(json5::parse("null").is_null());
    REQUIRE(!json5::parse("null").is_bool());
    REQUIRE(!json5::parse("0").is_null());
}
