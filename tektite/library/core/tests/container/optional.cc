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
#include <rainy/core/container/optional.hpp>


using rainy::container::make_optional;
using rainy::container::nullopt;
using rainy::container::optional;

struct tracked {
    static inline int alive = 0;
    int value;
    explicit tracked(int v) : value(v) {
        ++alive;
    }
    tracked(const tracked &other) : value(other.value) {
        ++alive;
    }
    tracked(tracked &&other) noexcept : value(other.value) {
        ++alive;
    }
    tracked &operator=(const tracked &other) {
        value = other.value;
        return *this;
    }
    ~tracked() {
        --alive;
    }
};

TEST_CASE("optional construction", "[container][optional]") {
    SECTION("empty optional") {
        optional<int> empty;
        REQUIRE_FALSE(empty.has_value());
        REQUIRE(empty == nullopt);
        optional<int> from_nullopt(nullopt);
        REQUIRE_FALSE(from_nullopt.has_value());
    }
    SECTION("engaged optional") {
        optional<int> direct(std::in_place, 42);
        REQUIRE(direct.has_value());
        REQUIRE(*direct == 42);

        optional<int> copied(7);
        REQUIRE(copied.value_or(0) == 7);

        auto made = make_optional<std::string>("rainy");
        REQUIRE(*made == "rainy");
    }
}

TEST_CASE("optional element access", "[container][optional]") {
    optional<std::string> value("hello");
    REQUIRE(value->size() == 5);
    REQUIRE(*value == "hello");
    value->append("!");
    REQUIRE(*value == "hello!");
    *value = "world";
    REQUIRE(value.value() == "world");
}

TEST_CASE("optional value_or and value", "[container][optional]") {
    optional<int> engaged(10);
    optional<int> empty;
    REQUIRE(engaged.value_or(-1) == 10);
    REQUIRE(empty.value_or(-1) == -1);
    REQUIRE_THROWS_AS(empty.value(), rainy::core::exceptions::runtime::bad_optional_access);
}

TEST_CASE("optional reassignment and reset", "[container][optional]") {
    optional<tracked> value(std::in_place, 1);
    REQUIRE(tracked::alive == 1);

    value.reset();
    REQUIRE_FALSE(value.has_value());
    REQUIRE(tracked::alive == 0);

    value = tracked{2};
    REQUIRE(value.has_value());
    REQUIRE(value->value == 2);
    REQUIRE(tracked::alive == 1);

    value = nullopt;
    REQUIRE_FALSE(value.has_value());
    REQUIRE(tracked::alive == 0);
}

TEST_CASE("optional move semantics", "[container][optional]") {
    optional<std::string> source("moved");
    optional<std::string> target(std::move(source));
    REQUIRE(target.has_value());
    REQUIRE(*target == "moved");

    optional<std::string> assigned;
    assigned = std::move(target);
    REQUIRE(*assigned == "moved");
}

TEST_CASE("optional comparisons", "[container][optional]") {
    optional<int> a(1);
    optional<int> b(2);
    optional<int> empty;

    REQUIRE(a == a);
    REQUIRE(a != b);
    REQUIRE(a < b);
    REQUIRE(b > a);
    REQUIRE(a <= b);
    REQUIRE(b >= a);

    REQUIRE(empty == nullopt);
    REQUIRE(empty != a);
    REQUIRE(a != nullopt);
    REQUIRE(empty < a);
    REQUIRE(a > empty);

    REQUIRE(a == 1);
    REQUIRE(a != 2);
    REQUIRE(a < 2);
    REQUIRE(a >= 1);
}

TEST_CASE("optional swap", "[container][optional]") {
    using rainy::container::swap;
    optional<int> left(1);
    optional<int> right(2);
    swap(left, right);
    REQUIRE(*left == 2);
    REQUIRE(*right == 1);

    optional<int> empty;
    swap(left, empty);
    REQUIRE_FALSE(left.has_value());
    REQUIRE(*empty == 2);
}
