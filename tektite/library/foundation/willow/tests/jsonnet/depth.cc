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

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::jsonnet;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_evaluate_error;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_exception;

namespace {
    document run(const std::string &source) {
        return evaluate(jsonnet::parse(source.c_str()));
    }
}

TEST_CASE("jsonnet evaluate - deeply nested expressions are rejected", "[jsonnet][depth]") {
    std::string source(300, '[');
    source += '1';
    source.append(300, ']');
    REQUIRE_THROWS_AS(run(source), jsonnet_exception);
}

TEST_CASE("jsonnet evaluate - oversized indices are rejected", "[jsonnet][depth]") {
    REQUIRE_THROWS_AS(run("[1][100000000000000000000]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("\"a\"[-100000000000000000000]"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("[1][1e400]"), jsonnet_evaluate_error);
}

TEST_CASE("jsonnet evaluate - shifts avoid undefined behaviour", "[jsonnet][depth]") {
    REQUIRE(run("-1 << 1").as_integer() == -2);
    REQUIRE(run("1 << 63").as_float() == -9223372036854775808.0);
    REQUIRE(run("1 << 64").as_integer() == 1);
}

TEST_CASE("jsonnet evaluate - percent c encodes code points", "[jsonnet][depth]") {
    REQUIRE(run("\"%c\" % 65").as_string() == "A");
    REQUIRE(run("\"%c\" % 20013").as_string() == "\xe4\xb8\xad");
    REQUIRE_THROWS_AS(run("\"%c\" % 1114112"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("\"%c\" % 55296"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("\"%c\" % -1"), jsonnet_evaluate_error);
    REQUIRE_THROWS_AS(run("\"%c\" % 1.5"), jsonnet_evaluate_error);
}
