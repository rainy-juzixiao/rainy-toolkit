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
using rainy::foundation::exceptions::willow::json5::json5_parse_error;

namespace {
    std::string nested_array(const std::size_t depth) {
        std::string source;
        source.reserve(depth * 2);
        source.append(depth, '[');
        source.append(depth, ']');
        return source;
    }
}

TEST_CASE("json5 parse - deeply nested input is rejected", "[willow][json5][depth]") {
    REQUIRE_THROWS_AS(json5::parse(nested_array(300).c_str()), json5_parse_error);
    REQUIRE_THROWS_AS(json5::parse(nested_array(200000).c_str()), json5_parse_error);
}

TEST_CASE("json5 parse - shallow nesting is accepted", "[willow][json5][depth]") {
    REQUIRE_NOTHROW(json5::parse("[[[1]]]"));
}
