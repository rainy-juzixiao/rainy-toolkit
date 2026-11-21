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
#include <rainy/core/collections/array.hpp>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/mechanism/pattern_matching.hpp>

namespace pm = rainy::core::mechanism::pattern_matching;
namespace rc = rainy::core::container;
namespace rcol = rainy::core::collections;

TEST_CASE("pattern_matching ds destructures tuples", "[pattern_matching][ds]") {
    SECTION("exact shape with wildcards") {
        rc::tuple<int, int, int> value{1, 2, 3};
        const int result = pm::match(value)((pm::pattern | pm::ds(1, pm::_, 3)) = [] { return 1; },
                                            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 1);
    }
    SECTION("shape mismatch falls through") {
        rc::tuple<int, int, int> value{1, 2, 3};
        const int result = pm::match(value)((pm::pattern | pm::ds(1, pm::_, 4)) = [] { return 1; },
                                            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 0);
    }
    SECTION("captures an element") {
        rc::tuple<int, int, int> value{1, 2, 3};
        pm::id<int> middle;
        const int result = pm::match(value)((pm::pattern | pm::ds(1, middle.at(pm::_), 3)) = [&] { return *middle; });
        REQUIRE(result == 2);
    }
}

TEST_CASE("pattern_matching ds destructures rainy arrays", "[pattern_matching][ds]") {
    SECTION("exact shape") {
        rcol::array<int, 3> value{1, 2, 3};
        const int result = pm::match(value)((pm::pattern | pm::ds(1, pm::_, 3)) = [] { return 1; },
                                            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 1);
    }
    SECTION("rest binding via ooo") {
        rcol::array<int, 4> value{1, 2, 3, 4};
        pm::id<pm::implements::subrange_t<rcol::array<int, 4>>> rest;
        const int result = pm::match(value)((pm::pattern | pm::ds(1, pm::ooo(rest))) = [&] { return static_cast<int>(*(*rest).begin()); },
                                            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 2);
    }
    SECTION("rest range length") {
        rcol::array<int, 5> value{1, 2, 3, 4, 5};
        pm::id<pm::implements::subrange_t<rcol::array<int, 5>>> rest;
        const int result = pm::match(value)((pm::pattern | pm::ds(1, pm::_, pm::ooo(rest))) =
                                                [&] { return static_cast<int>((*rest).size()); },
                                            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 3);
    }
}

TEST_CASE("pattern_matching ds destructures rainy pairs", "[pattern_matching][ds]") {
    SECTION("exact shape") {
        rc::pair<int, char> value{1, 'a'};
        const int result = pm::match(value)((pm::pattern | pm::ds(1, 'a')) = [] { return 1; },
                                            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 1);
    }
    SECTION("captures both halves") {
        rc::pair<int, char> value{1, 'a'};
        pm::id<int> first_id;
        pm::id<char> second_id;
        const int result = pm::match(value)((pm::pattern | pm::ds(first_id.at(pm::_), second_id.at(pm::_))) =
                                                [&] { return *first_id + static_cast<int>(*second_id); });
        REQUIRE(result == 1 + 'a');
    }
}

TEST_CASE("pattern_matching ds destructures multiple match values", "[pattern_matching][ds]") {
    SECTION("multi-value match is a reference tuple") {
        const int result = pm::match(1, 2, 3)((pm::pattern | pm::ds(1, pm::_, 3)) = [] { return 1; },
                                              (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 1);
    }
    SECTION("handlers can mutate the matched values through the tuple") {
        int first = 1;
        int second = 2;
        pm::match(first, second)((pm::pattern | pm::ds(1, pm::_)) = [&] {
            first = 10;
            second = 20;
        });
        REQUIRE(first == 10);
        REQUIRE(second == 20);
    }
}