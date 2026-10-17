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
    facade<document> parse_text(const char *source) {
        return jsonnet::parse(source);
    }

    rainy::core::text::string dump(const char *source) {
        return jsonnet::dump(parse_text(source));
    }
}

TEST_CASE("jsonnet evaluate - slice bounds and steps must be integers", "[jsonnet][evaluate]") {
    REQUIRE_THROWS_AS(dump("[10, 20][0:1.5]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(dump("[10, 20][::1.5]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(dump("[10, 20][0:std.pow(10, 400)]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(dump("[10, 20][std.pow(10, 400):]"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - reverse slice still works", "[jsonnet][evaluate]") {
    REQUIRE(dump("[0, 1, 2, 3][::-1]") == "[3,2,1,0]");
}

TEST_CASE("jsonnet evaluate - function call arity is checked", "[jsonnet][evaluate]") {
    REQUIRE_THROWS_AS(dump("(function(x) x)(1, 2)"), jsonnet_evaluate_error);
    REQUIRE(dump("(function(x) x)(x=1)") == "1");
    REQUIRE_THROWS_AS(dump("(function(x, y) x + y)(1)"), jsonnet_evaluate_error);
    REQUIRE(dump("(function(a, b=a) b)(1)") == "1");
    REQUIRE_THROWS_AS(dump("(function(x) x)(x=1, x=2)"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(dump("(function(x) x)(y=1)"), jsonnet_evaluate_error);
}
