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
#include <rainy/foundation/willow/xml.hpp>

#include <string>

using namespace rainy::foundation::willow;
using rainy::foundation::exceptions::willow::xml::xml_parse_error;

namespace {
    std::string nested_element(const std::size_t depth) {
        std::string source;
        source.reserve(depth * 7);
        for (std::size_t i = 0; i < depth; ++i) {
            source += "<a>";
        }
        for (std::size_t i = 0; i < depth; ++i) {
            source += "</a>";
        }
        return source;
    }
}

TEST_CASE("xml parse - deeply nested input is rejected", "[willow][xml][depth]") {
    REQUIRE_THROWS_AS(xml::parse(nested_element(300).c_str()), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse(nested_element(200000).c_str()), xml_parse_error);
}

TEST_CASE("xml parse - shallow nesting is accepted", "[willow][xml][depth]") {
    REQUIRE_NOTHROW(xml::parse("<a><b>1</b></a>"));
}
