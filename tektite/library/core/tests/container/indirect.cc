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
#include <rainy/core/container/indirect.hpp>

using rainy::container::defered_init;
using rainy::container::indirect;
using rainy::container::make_indirect;

TEST_CASE("indirect construction", "[container][indirect]") {
    SECTION("from value") {
        indirect<int> value(42);
        REQUIRE_FALSE(value.empty());
        REQUIRE(*value == 42);
        REQUIRE_FALSE(value.valueless_after_move());
    }
    SECTION("make_indirect") {
        auto value = make_indirect<std::string>(3, 'x');
        REQUIRE(*value == "xxx");

        auto copied = make_indirect<int>(7);
        REQUIRE(*copied == 7);
    }
    SECTION("empty via defered_init") {
        indirect<int> empty(defered_init);
        REQUIRE(empty.empty());
        REQUIRE(empty.valueless_after_move());
    }
}

TEST_CASE("indirect element access", "[container][indirect]") {
    indirect<std::string> value(std::in_place, "hello");
    REQUIRE(value->size() == 5);
    REQUIRE(*value == "hello");
    *value = "world";
    REQUIRE(*value == "world");
}

TEST_CASE("indirect copy and move", "[container][indirect]") {
    indirect<std::string> source("data");
    indirect<std::string> copied(source);
    REQUIRE(*copied == "data");
    REQUIRE(*source == "data");

    indirect<std::string> moved(std::move(source));
    REQUIRE(*moved == "data");
    REQUIRE(source.valueless_after_move());

    indirect<std::string> assigned;
    assigned = std::move(moved);
    REQUIRE(*assigned == "data");
    REQUIRE(moved.valueless_after_move());
}

TEST_CASE("indirect reset and reassignment", "[container][indirect]") {
    indirect<int> value(1);
    value.reset();
    REQUIRE(value.empty());

    value = 5;
    REQUIRE_FALSE(value.empty());
    REQUIRE(*value == 5);
}

TEST_CASE("indirect comparisons", "[container][indirect]") {
    indirect<int> a(1);
    indirect<int> b(1);
    indirect<int> c(2);
    indirect<int> empty(defered_init);

    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(a < c);
    REQUIRE(c > a);
    REQUIRE(a <= b);
    REQUIRE(c >= a);

    REQUIRE_FALSE(empty == a);
    REQUIRE(empty != a);
    REQUIRE(a != nullptr);
    REQUIRE(empty == nullptr);
}

TEST_CASE("indirect swap", "[container][indirect]") {
    using rainy::container::swap;
    indirect<int> left(1);
    indirect<int> right(2);
    swap(left, right);
    REQUIRE(*left == 2);
    REQUIRE(*right == 1);
}
