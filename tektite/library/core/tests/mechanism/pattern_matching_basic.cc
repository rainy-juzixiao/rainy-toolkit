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
#include <rainy/core/mechanism/pattern_matching.hpp>

namespace pm = rainy::core::mechanism::pattern_matching;

TEST_CASE("pattern_matching wildcard and literal", "[pattern_matching][basic]") {
    SECTION("literal match hits the first arm") {
        const int result = pm::match(42)((pm::pattern | 42) = [] { return 1; },
                                         (pm::pattern | pm::_) = [] { return 2; });
        REQUIRE(result == 1);
    }
    SECTION("literal miss falls through to the wildcard") {
        const int result = pm::match(7)((pm::pattern | 42) = [] { return 1; },
                                        (pm::pattern | pm::_) = [] { return 2; });
        REQUIRE(result == 2);
    }
    SECTION("wildcard accepts any type") {
        const int from_int = pm::match(3)((pm::pattern | pm::_) = [] { return 1; });
        const int from_char = pm::match('c')((pm::pattern | pm::_) = [] { return 2; });
        REQUIRE(from_int == 1);
        REQUIRE(from_char == 2);
    }
}

TEST_CASE("pattern_matching or_ combinator", "[pattern_matching][basic]") {
    SECTION("matches any listed alternative") {
        auto handler = [](int value) {
            return pm::match(value)((pm::pattern | pm::or_(1, 7, 9)) = [] { return 1; },
                                    (pm::pattern | pm::_) = [] { return 0; });
        };
        REQUIRE(handler(1) == 1);
        REQUIRE(handler(7) == 1);
        REQUIRE(handler(9) == 1);
        REQUIRE(handler(2) == 0);
    }
    SECTION("wildcard first short-circuits") {
        const int result = pm::match(5)((pm::pattern | pm::or_(pm::_, 42)) = [] { return 1; },
                                        (pm::pattern | 42) = [] { return 2; });
        REQUIRE(result == 1);
    }
}

TEST_CASE("pattern_matching and_ and meet", "[pattern_matching][basic]") {
    SECTION("meet applies a predicate") {
        auto is_even = pm::meet([](int value) { return value % 2 == 0; });
        const int even = pm::match(4)((pm::pattern | is_even) = [] { return 1; },
                                      (pm::pattern | pm::_) = [] { return 0; });
        const int odd = pm::match(3)((pm::pattern | is_even) = [] { return 1; },
                                     (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(even == 1);
        REQUIRE(odd == 0);
    }
    SECTION("and_ requires every predicate") {
        auto positive = pm::meet([](int value) { return value > 0; });
        auto small = pm::meet([](int value) { return value < 10; });
        auto both = pm::and_(positive, small);
        REQUIRE(pm::match(6)((pm::pattern | both) = [] { return 1; },
                             (pm::pattern | pm::_) = [] { return 0; }) == 1);
        REQUIRE(pm::match(-1)((pm::pattern | both) = [] { return 1; },
                              (pm::pattern | pm::_) = [] { return 0; }) == 0);
        REQUIRE(pm::match(11)((pm::pattern | both) = [] { return 1; },
                              (pm::pattern | pm::_) = [] { return 0; }) == 0);
    }
}

TEST_CASE("pattern_matching not_ combinator", "[pattern_matching][basic]") {
    SECTION("matches everything but the inner pattern") {
        REQUIRE(pm::match(7)((pm::pattern | pm::not_(42)) = [] { return 1; },
                             (pm::pattern | pm::_) = [] { return 0; }) == 1);
        REQUIRE(pm::match(42)((pm::pattern | pm::not_(42)) = [] { return 1; },
                              (pm::pattern | pm::_) = [] { return 0; }) == 0);
    }
    SECTION("negating the wildcard never matches") {
        REQUIRE(pm::match(5)((pm::pattern | pm::not_(pm::_)) = [] { return 1; },
                             (pm::pattern | pm::_) = [] { return 0; }) == 0);
    }
}

TEST_CASE("pattern_matching app transforms before matching", "[pattern_matching][basic]") {
    SECTION("first element of a pair") {
        const int result = pm::match(rainy::core::container::pair<int, int>{3, 4})(
            (pm::pattern | pm::app([](auto &&p) { return p.first; }, 3)) = [] { return 1; },
            (pm::pattern | pm::_) = [] { return 0; });
        REQUIRE(result == 1);
    }
    SECTION("captures the transformed value") {
        pm::id<int> captured;
        const int result = pm::match(21)(
            (pm::pattern | pm::app([](int value) { return value * 2; }, captured.at(pm::_))) = [&] { return *captured; });
        REQUIRE(result == 42);
    }
}