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
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/mechanism/pattern_matching.hpp>

namespace pm = rainy::core::mechanism::pattern_matching;
namespace rc = rainy::core::container;

TEST_CASE("pattern_matching id capture and reuse", "[pattern_matching][advanced]") {
    SECTION("a repeated id requires equal values") {
        pm::id<int> same;
        REQUIRE(pm::match(rc::tuple<int, int>{2, 2})((pm::pattern | pm::ds(same, same)) = [] { return 1; },
                                                     (pm::pattern | pm::_) = [] { return 0; }) == 1);
        REQUIRE(pm::match(rc::tuple<int, int>{2, 3})((pm::pattern | pm::ds(same, same)) = [] { return 1; },
                                                     (pm::pattern | pm::_) = [] { return 0; }) == 0);
    }
    SECTION("id binds the first matching value") {
        pm::id<int> bound;
        const int result = pm::match(rc::tuple<int, int>{5, 5})((pm::pattern | pm::ds(bound.at(pm::_), bound)) = [&] { return *bound; });
        REQUIRE(result == 5);
    }
}

TEST_CASE("pattern_matching when applies a post check", "[pattern_matching][advanced]") {
    SECTION("a zero-argument guard reads through a captured id") {
        pm::id<int> bound;
        REQUIRE(pm::match(5)((pm::pattern | bound.at(pm::_) | pm::when([&] { return *bound < 10; })) =
                                 [&] { return *bound; }) == 5);
        bool matched = false;
        pm::match(15)((pm::pattern | bound.at(pm::_) | pm::when([&] { return *bound < 10; })) = [&] { matched = true; },
                      (pm::pattern | pm::_) = [&] { matched = false; });
        REQUIRE_FALSE(matched);
    }
}

TEST_CASE("pattern_matching expression operators", "[pattern_matching][advanced]") {
    SECTION("comparison against the wildcard") {
        REQUIRE(pm::match(5)((pm::pattern | (pm::_ == 5)) = [] { return 1; },
                             (pm::pattern | pm::_) = [] { return 0; }) == 1);
        REQUIRE(pm::match(4)((pm::pattern | (pm::_ == 5)) = [] { return 1; },
                             (pm::pattern | pm::_) = [] { return 0; }) == 0);
    }
    SECTION("arithmetic on the wildcard") {
        REQUIRE(pm::match(10)((pm::pattern | (pm::_ * 2 == 20)) = [] { return 1; },
                              (pm::pattern | pm::_) = [] { return 0; }) == 1);
    }
    SECTION("boolean combination") {
        REQUIRE(pm::match(5)((pm::pattern | (pm::_ > 0 && pm::_ < 10)) = [] { return 1; },
                             (pm::pattern | pm::_) = [] { return 0; }) == 1);
        REQUIRE(pm::match(50)((pm::pattern | (pm::_ > 0 && pm::_ < 10)) = [] { return 1; },
                              (pm::pattern | pm::_) = [] { return 0; }) == 0);
    }
    SECTION("unary operators") {
        REQUIRE(pm::match(-3)((pm::pattern | (-pm::_)) = [] { return 1; },
                              (pm::pattern | pm::_) = [] { return 0; }) == 1);
        REQUIRE(pm::match(false)((pm::pattern | (!pm::_)) = [] { return 1; },
                                 (pm::pattern | pm::_) = [] { return 0; }) == 1);
    }
}

TEST_CASE("pattern_matching no-match behavior", "[pattern_matching][advanced]") {
    SECTION("expression form throws when nothing matches") {
        bool thrown = false;
        try {
            const int result = pm::match(1)((pm::pattern | 2) = [] { return 5; });
            (void)result;
        } catch (const rainy::core::exceptions::logic::logic_error &) {
            thrown = true;
        }
        REQUIRE(thrown);
    }
    SECTION("statement form skips silently") {
        bool ran = false;
        pm::match(1)((pm::pattern | 2) = [&] { ran = true; });
        REQUIRE_FALSE(ran);
        pm::match(2)((pm::pattern | 2) = [&] { ran = true; });
        REQUIRE(ran);
    }
}

namespace {
    constexpr int constexpr_match(int value) {
        return pm::match(value)((pm::pattern | 42) = [] { return 1; }, (pm::pattern | pm::_) = [] { return 0; });
    }
    static_assert(constexpr_match(42) == 1);
    static_assert(constexpr_match(7) == 0);
} // namespace

TEST_CASE("pattern_matching works in constant evaluation", "[pattern_matching][advanced][constexpr]") {
    STATIC_REQUIRE(constexpr_match(42) == 1);
    STATIC_REQUIRE(constexpr_match(7) == 0);
}