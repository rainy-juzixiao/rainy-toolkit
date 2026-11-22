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
using rainy::foundation::exceptions::willow::hjson::hjson_parse_error;

namespace {
    std::string nested_array(const std::size_t depth) {
        std::string source;
        source.reserve(depth * 2);
        source.append(depth, '[');
        source.append(depth, ']');
        return source;
    }
}

TEST_CASE("hjson parse - deeply nested input is rejected", "[willow][hjson][depth]") {
    REQUIRE_THROWS_AS(hjson::parse(nested_array(300).c_str()), hjson_parse_error);
    REQUIRE_THROWS_AS(hjson::parse(nested_array(200000).c_str()), hjson_parse_error);
}

TEST_CASE("hjson parse - shallow nesting is accepted", "[willow][hjson][depth]") {
    REQUIRE_NOTHROW(hjson::parse("[[[1]]]"));
}
