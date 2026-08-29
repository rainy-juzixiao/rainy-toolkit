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
#include <rainy/core/text/string_view.hpp>

using rainy::core::text::string_view;

TEST_CASE("string_view construction and observation", "[text][string_view]") {
    SECTION("from literals and std::string") {
        string_view literal("rainy");
        REQUIRE(literal.size() == 5);
        REQUIRE(literal.length() == 5);
        REQUIRE_FALSE(literal.empty());
        REQUIRE(std::string(literal.data(), literal.size()) == "rainy");

        std::string source("toolkit");
        string_view from_string(source);
        REQUIRE(from_string.size() == 7);
    }
    SECTION("element access") {
        string_view value("abcdef");
        REQUIRE(value[0] == 'a');
        REQUIRE(value.at(1) == 'b');
        REQUIRE(value.front() == 'a');
        REQUIRE(value.back() == 'f');
    }
}

TEST_CASE("string_view modifiers", "[text][string_view]") {
    string_view value("rainy-toolkit");
    value.remove_prefix(6);
    REQUIRE(value == "toolkit");
    value.remove_suffix(3);
    REQUIRE(value == "tool");

    string_view whole("abcdef");
    REQUIRE(whole.substr(2) == "cdef");
    REQUIRE(whole.substr(1, 3) == "bcd");
}

TEST_CASE("string_view comparison", "[text][string_view]") {
    string_view left("abc");
    string_view right("abd");

    REQUIRE(left == left);
    REQUIRE(left != right);
    REQUIRE(left < right);
    REQUIRE(left <= right);
    REQUIRE(right > left);
    REQUIRE(right >= left);
    REQUIRE(left == "abc");
    REQUIRE(left != "abd");
}

TEST_CASE("string_view prefix, suffix and containment", "[text][string_view]") {
    string_view value("rainy-toolkit");
    REQUIRE(value.starts_with("rainy"));
    REQUIRE(value.starts_with('r'));
    REQUIRE_FALSE(value.starts_with("toolkit"));
    REQUIRE(value.ends_with("toolkit"));
    REQUIRE(value.ends_with('t'));
    REQUIRE_FALSE(value.ends_with("rainy"));
    REQUIRE(value.contains("y-t"));
    REQUIRE(value.contains('-'));
    REQUIRE_FALSE(value.contains("xyz"));
}

TEST_CASE("string_view search", "[text][string_view]") {
    string_view value("rainy-toolkit-rainy");

    REQUIRE(value.find("toolkit") == 6);
    REQUIRE(value.find("missing") == string_view::npos);
    REQUIRE(value.find('y') == 4);
    REQUIRE(value.find('y', 5) == 18);

    REQUIRE(value.rfind("rainy") == 14);
    REQUIRE(value.rfind('-') == 13);

    REQUIRE(value.find_first_of("tk") == 6);
    REQUIRE(value.find_last_of("tk") == 12);
    REQUIRE(value.find_first_not_of("rainy-") == 6);
    REQUIRE(value.find_last_not_of("rainy-") == 12);
}
