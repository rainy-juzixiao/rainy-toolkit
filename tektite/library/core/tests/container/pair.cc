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
#include <rainy/core/container/pair.hpp>

using rainy::container::make_pair;
using rainy::container::pair;

TEST_CASE("pair construction", "[container][pair]") {
    SECTION("default and value construction") {
        pair<int, double> default_pair;
        REQUIRE(default_pair.first == 0);
        REQUIRE(default_pair.second == 0.0);

        pair<int, double> value_pair(1, 2.5);
        REQUIRE(value_pair.first == 1);
        REQUIRE(value_pair.second == 2.5);
    }
    SECTION("converting construction") {
        pair<double, long> converted(pair<int, int>(1, 2));
        REQUIRE(converted.first == 1.0);
        REQUIRE(converted.second == 2);

        pair<int, double> from_values(1.5, 3);
        REQUIRE(from_values.first == 1);
        REQUIRE(from_values.second == 3.0);
    }
    SECTION("piecewise construction") {
        pair<std::string, std::string> piecewise(std::piecewise_construct, std::tuple("rainy"), std::tuple(3, 'x'));
        REQUIRE(piecewise.first == "rainy");
        REQUIRE(piecewise.second == "xxx");
    }
}

TEST_CASE("make_pair", "[container][pair]") {
    auto value = make_pair(1, 2.5);
    STATIC_REQUIRE(std::is_same_v<decltype(value), pair<int, double>>);
    REQUIRE(value.first == 1);
    REQUIRE(value.second == 2.5);
}

TEST_CASE("pair copy and move", "[container][pair]") {
    pair<std::string, int> source("hello", 1);
    pair<std::string, int> copied(source);
    REQUIRE(copied.first == "hello");
    REQUIRE(copied.second == 1);

    pair<std::string, int> moved(std::move(source));
    REQUIRE(moved.first == "hello");
    REQUIRE(moved.second == 1);

    pair<std::string, int> assigned;
    assigned = std::move(moved);
    REQUIRE(assigned.first == "hello");
    REQUIRE(assigned.second == 1);
}

TEST_CASE("pair swap and comparison", "[container][pair]") {
    pair<int, int> left(1, 2);
    pair<int, int> right(1, 3);
    left.swap(right);
    REQUIRE(left.second == 3);
    REQUIRE(right.second == 2);
}
