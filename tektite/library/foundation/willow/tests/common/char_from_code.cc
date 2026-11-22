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
#include <rainy/foundation/willow/implements/common/operations.hpp>
#include <rainy/foundation/willow/jsonnet.hpp>

using namespace rainy::foundation::willow;
using rainy::foundation::exceptions::willow::willow_type_error;
namespace ops = rainy::foundation::willow::implements::operations;

namespace {
    document run(const char *source) {
        jsonnet::facade<document> parsed = jsonnet::parse(source);
        return jsonnet::evaluate(parsed);
    }
}

TEST_CASE("operations - char_from_code rejects non-scalar values", "[willow][operations][unicode]") {
    REQUIRE_THROWS_AS(ops::char_from_code<document>(-1), willow_type_error);
    REQUIRE_THROWS_AS(ops::char_from_code<document>(0xD800), willow_type_error);
    REQUIRE_THROWS_AS(ops::char_from_code<document>(0xDFFF), willow_type_error);
    REQUIRE_THROWS_AS(ops::char_from_code<document>(0x110000), willow_type_error);
}

TEST_CASE("operations - char_from_code encodes Unicode scalar values", "[willow][operations][unicode]") {
    REQUIRE(ops::char_from_code<document>(0x41).as_string() == "A");
    REQUIRE(ops::char_from_code<document>(0x1F600).as_string() == "\xF0\x9F\x98\x80");
    REQUIRE(ops::char_from_code<document>(0x10FFFF).as_string() == "\xF4\x8F\xBF\xBF");
}

TEST_CASE("jsonnet std - char rejects non-scalar values", "[jsonnet][stdlib]") {
    REQUIRE_THROWS_AS(run("std.char(-1)"), rainy::foundation::exceptions::willow::jsonnet::jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("std.char(55296)"), rainy::foundation::exceptions::willow::jsonnet::jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("std.char(1114112)"), rainy::foundation::exceptions::willow::jsonnet::jsonnet_evaluate_error);
}

TEST_CASE("jsonnet std - char encodes Unicode scalar values", "[jsonnet][stdlib]") {
    REQUIRE(run("std.char(65)").as_string() == "A");
    REQUIRE(run("std.char(128512)").as_string() == "\xF0\x9F\x98\x80");
}
