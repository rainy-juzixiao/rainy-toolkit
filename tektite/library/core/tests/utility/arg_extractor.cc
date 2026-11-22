/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/core/utility/arg_extractor.hpp>

#include <string>

using rainy::utility::extract_args_to_array;

TEST_CASE("arg_extractor collects arguments of one type", "[arg_extractor]") {
    SECTION("single match") {
        auto values = extract_args_to_array<int>(1, 2.5, "text");
        STATIC_REQUIRE(decltype(values)::size() == 1);
        REQUIRE(values[0] == 1);
    }

    SECTION("several matches keep declaration order") {
        auto values = extract_args_to_array<int>(10, 'a', 20, 30);
        STATIC_REQUIRE(decltype(values)::size() == 3);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }

    SECTION("no match yields an empty array") {
        auto values = extract_args_to_array<double>(1, 2, 3);
        STATIC_REQUIRE(decltype(values)::size() == 0);
        REQUIRE(values.size() == 0);
    }

    SECTION("matching is exact on the decayed type") {
        auto lvalues = extract_args_to_array<std::string>(std::string("a"), 3, std::string("b"));
        STATIC_REQUIRE(decltype(lvalues)::size() == 2);
        REQUIRE(lvalues[0] == std::string("a"));
        REQUIRE(lvalues[1] == std::string("b"));
    }

    SECTION("mixed lvalue and rvalue arguments") {
        int first = 7;
        auto values = extract_args_to_array<int>(first, 8);
        STATIC_REQUIRE(decltype(values)::size() == 2);
        REQUIRE(values[0] == 7);
        REQUIRE(values[1] == 8);
    }
}

TEST_CASE("arg_extractor through the class template", "[arg_extractor]") {
    auto values = rainy::utility::arg_extractor<long, long, int, long>::extract_to_array(1L, 2, 3L);
    STATIC_REQUIRE(decltype(values)::size() == 2);
    REQUIRE(values[0] == 1L);
    REQUIRE(values[1] == 3L);
}
