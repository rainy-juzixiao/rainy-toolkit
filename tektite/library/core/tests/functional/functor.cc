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
#include <catch2/matchers/catch_matchers.hpp>
#include <numeric>
#include <rainy/core/functional/functor.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace rainy::functional;

TEST_CASE("plus with int", "[functor]") {
    plus<int> op;
    REQUIRE(op(3, 5) == 8);
    REQUIRE(op(-2, 7) == 5);
    REQUIRE(op(0, 0) == 0);
}

TEST_CASE("plus with double", "[functor]") {
    plus<double> op;
    REQUIRE(op(3.5, 2.5) == 6.0);
    REQUIRE_THAT(op(-1.2, 3.3), Catch::Matchers::WithinAbs(2.1, 1e-12));
}

TEST_CASE("plus<void> with mixed types", "[functor]") {
    plus<void> op;
    REQUIRE(op(3, 5.5) == 8.5);
    REQUIRE(op(2.5f, 3) == 5.5f);
    REQUIRE(op(10, 20L) == 30L);
}

TEST_CASE("minus with int", "[functor]") {
    minus<int> op;
    REQUIRE(op(10, 3) == 7);
    REQUIRE(op(5, 8) == -3);
    REQUIRE(op(0, 5) == -5);
}

TEST_CASE("minus<void> with mixed types", "[functor]") {
    minus<void> op;
    REQUIRE(op(10.5, 3) == 7.5);
    REQUIRE(op(5, 2.5f) == 2.5f);
    REQUIRE(op(20L, 5) == 15L);
}

TEST_CASE("multiplies with int", "[functor]") {
    multiplies<int> op;
    REQUIRE(op(3, 5) == 15);
    REQUIRE(op(-2, 6) == -12);
    REQUIRE(op(0, 100) == 0);
}

TEST_CASE("multiplies<void> with mixed types", "[functor]") {
    multiplies<void> op;
    REQUIRE(op(3.5, 2) == 7.0);
    REQUIRE(op(2.5f, 4) == 10.0f);
    REQUIRE(op(10L, 3) == 30L);
}

TEST_CASE("divides with int", "[functor]") {
    divides<int> op;
    REQUIRE(op(10, 2) == 5);
    REQUIRE(op(7, 3) == 2);
    REQUIRE(op(-10, 2) == -5);
}

TEST_CASE("divides<void> with mixed types", "[functor]") {
    divides<void> op;
    REQUIRE(op(10.0, 2) == 5.0);
    REQUIRE(op(7.5f, 2) == 3.75f);
}

TEST_CASE("modulus with int", "[functor]") {
    modulus<int> op;
    REQUIRE(op(10, 3) == 1);
    REQUIRE(op(8, 4) == 0);
    REQUIRE(op(-7, 3) == -1);
}

TEST_CASE("modulus<void> with mixed types", "[functor]") {
    modulus<void> op;
    REQUIRE(op(10L, 3) == 1L);
    REQUIRE(op(7, 2L) == 1L);
}

TEST_CASE("negate with int", "[functor]") {
    negate<int> op;
    REQUIRE(op(5) == -5);
    REQUIRE(op(-3) == 3);
    REQUIRE(op(0) == 0);
}

TEST_CASE("negate<void> with mixed types", "[functor]") {
    negate<void> op;
    REQUIRE(op(5.5) == -5.5);
    REQUIRE(op(-2.5f) == 2.5f);
    REQUIRE(op(10L) == -10L);
}

TEST_CASE("equal with int", "[functor]") {
    equal<int> op;
    REQUIRE(op(5, 5) == true);
    REQUIRE(op(5, 3) == false);
    REQUIRE(op(-1, -1) == true);
}

TEST_CASE("equal<void> with mixed types", "[functor]") {
    equal<void> op;
    REQUIRE(op(5, 5.0) == true);
    REQUIRE(op(5, 5.5) == false);
    REQUIRE(op(3L, 3) == true);
}

TEST_CASE("not_equal with int", "[functor]") {
    not_equal<int> op;
    REQUIRE(op(5, 5) == false);
    REQUIRE(op(5, 3) == true);
    REQUIRE(op(-1, 1) == true);
}

TEST_CASE("not_equal<void> with mixed types", "[functor]") {
    not_equal<void> op;
    REQUIRE(op(5, 5.0) == false);
    REQUIRE(op(5, 5.5) == true);
    REQUIRE(op(3L, 4) == true);
}

TEST_CASE("less with int", "[functor]") {
    less<int> op;
    REQUIRE(op(3, 5) == true);
    REQUIRE(op(5, 3) == false);
    REQUIRE(op(5, 5) == false);
}

TEST_CASE("less<void> with mixed types", "[functor]") {
    less<void> op;
    REQUIRE(op(3.5, 5) == true);
    REQUIRE(op(5, 3.5) == false);
    REQUIRE(op(5L, 5.0) == false);
}

TEST_CASE("less_equal with int", "[functor]") {
    less_equal<int> op;
    REQUIRE(op(3, 5) == true);
    REQUIRE(op(5, 3) == false);
    REQUIRE(op(5, 5) == true);
}

TEST_CASE("less_equal<void> with mixed types", "[functor]") {
    less_equal<void> op;
    REQUIRE(op(3.5, 5) == true);
    REQUIRE(op(5, 3.5) == false);
    REQUIRE(op(5L, 5.0) == true);
}

TEST_CASE("greater with int", "[functor]") {
    greater<int> op;
    REQUIRE(op(5, 3) == true);
    REQUIRE(op(3, 5) == false);
    REQUIRE(op(5, 5) == false);
}

TEST_CASE("greater<void> with mixed types", "[functor]") {
    greater<void> op;
    REQUIRE(op(5.5, 3) == true);
    REQUIRE(op(3, 5.5) == false);
    REQUIRE(op(5L, 5.0) == false);
}

TEST_CASE("greater_equal with int", "[functor]") {
    greater_equal<int> op;
    REQUIRE(op(5, 3) == true);
    REQUIRE(op(3, 5) == false);
    REQUIRE(op(5, 5) == true);
}

TEST_CASE("greater_equal<void> with mixed types", "[functor]") {
    greater_equal<void> op;
    REQUIRE(op(5.5, 3) == true);
    REQUIRE(op(3, 5.5) == false);
    REQUIRE(op(5L, 5.0) == true);
}

TEST_CASE("logical_and with bool", "[functor]") {
    logical_and<bool> op;
    REQUIRE(op(true, true) == true);
    REQUIRE(op(true, false) == false);
    REQUIRE(op(false, true) == false);
    REQUIRE(op(false, false) == false);
}

TEST_CASE("logical_and<void> with mixed types", "[functor]") {
    logical_and<void> op;
    REQUIRE(op(true, 1) == true);
    REQUIRE(op(true, 0) == false);
    REQUIRE(op(1, true) == true);
    REQUIRE(op(0, true) == false);
}

TEST_CASE("logical_or with bool", "[functor]") {
    logical_or<bool> op;
    REQUIRE(op(true, true) == true);
    REQUIRE(op(true, false) == true);
    REQUIRE(op(false, true) == true);
    REQUIRE(op(false, false) == false);
}

TEST_CASE("logical_or<void> with mixed types", "[functor]") {
    logical_or<void> op;
    REQUIRE(op(true, 0) == true);
    REQUIRE(op(false, 1) == true);
    REQUIRE(op(0, false) == false);
    REQUIRE(op(1, false) == true);
}

TEST_CASE("logical_not with bool", "[functor]") {
    logical_not<bool> op;
    REQUIRE(op(true) == false);
    REQUIRE(op(false) == true);
}

TEST_CASE("logical_not<void> with mixed types", "[functor]") {
    logical_not<void> op;
    REQUIRE(op(1) == false);
    REQUIRE(op(0) == true);
    REQUIRE(op(-1) == false);
}

TEST_CASE("plus with custom type", "[functor]") {
    struct Point {
        int x, y;
        Point operator+(const Point &other) const {
            return {x + other.x, y + other.y};
        }
    };
    plus<Point> op;
    Point p1{1, 2}, p2{3, 4};
    auto result = op(p1, p2);
    REQUIRE(result.x == 4);
    REQUIRE(result.y == 6);
}

TEST_CASE("minus with custom type", "[functor]") {
    struct Point {
        int x, y;
        Point operator-(const Point &other) const {
            return {x - other.x, y - other.y};
        }
    };
    minus<Point> op;
    Point p1{5, 6}, p2{2, 3};
    auto result = op(p1, p2);
    REQUIRE(result.x == 3);
    REQUIRE(result.y == 3);
}

TEST_CASE("equal with string", "[functor]") {
    equal<std::string> op;
    REQUIRE(op("hello", "hello") == true);
    REQUIRE(op("hello", "world") == false);
}

TEST_CASE("less with string", "[functor]") {
    less<std::string> op;
    REQUIRE(op("apple", "banana") == true);
    REQUIRE(op("banana", "apple") == false);
    REQUIRE(op("apple", "apple") == false);
}

TEST_CASE("multiplies<void> with int and double", "[functor]") {
    multiplies<void> op;
    REQUIRE(op(3, 2.5) == 7.5);
    REQUIRE(op(2.5f, 4) == 10.0f);
}

TEST_CASE("divides<void> with int and double", "[functor]") {
    divides<void> op;
    REQUIRE(op(10, 3.0) == 10.0 / 3.0);
    REQUIRE(op(7.5f, 2) == 3.75f);
}

TEST_CASE("modulus<void> with int and long", "[functor]") {
    modulus<void> op;
    REQUIRE(op(10, 3L) == 1L);
    REQUIRE(op(7L, 3) == 1L);
}

TEST_CASE("logical_and with int", "[functor]") {
    logical_and<int> op;
    REQUIRE(op(1, 1) == true);
    REQUIRE(op(1, 0) == false);
    REQUIRE(op(0, 1) == false);
    REQUIRE(op(0, 0) == false);
}

TEST_CASE("logical_or with int", "[functor]") {
    logical_or<int> op;
    REQUIRE(op(1, 1) == true);
    REQUIRE(op(1, 0) == true);
    REQUIRE(op(0, 1) == true);
    REQUIRE(op(0, 0) == false);
}

TEST_CASE("negate with custom type", "[functor]") {
    struct Number {
        int value;
        Number operator-() const {
            return {-value};
        }
    };
    negate<Number> op;
    Number n{5};
    auto result = op(n);
    REQUIRE(result.value == -5);
}

TEST_CASE("logical_not with custom type", "[functor]") {
    struct BoolLike {
        bool value;
        operator bool() const {
            return value;
        }
    };
    logical_not<BoolLike> op;
    BoolLike b1{true}, b2{false};
    REQUIRE(op(b1) == false);
    REQUIRE(op(b2) == true);
}

TEST_CASE("comparison functors with floating point", "[functor]") {
    equal<double> eq;
    REQUIRE(eq(3.0, 3.0) == true);
    REQUIRE(eq(3.0, 3.0000001) == false);

    less<double> lt;
    REQUIRE(lt(2.5, 3.5) == true);
    REQUIRE(lt(3.5, 2.5) == false);
}

TEST_CASE("plus with const references", "[functor]") {
    plus<int> op;
    const int a = 5, b = 7;
    REQUIRE(op(a, b) == 12);
}

TEST_CASE("minus with const references", "[functor]") {
    minus<int> op;
    const int a = 10, b = 3;
    REQUIRE(op(a, b) == 7);
}

TEST_CASE("multiplies with const references", "[functor]") {
    multiplies<int> op;
    const int a = 4, b = 6;
    REQUIRE(op(a, b) == 24);
}

TEST_CASE("functor as comparator in algorithm", "[functor]") {
    std::vector<int> v = {5, 2, 8, 1, 9, 3};
    std::sort(v.begin(), v.end(), less<int>());
    REQUIRE(v == std::vector<int>{1, 2, 3, 5, 8, 9});

    std::sort(v.begin(), v.end(), greater<int>());
    REQUIRE(v == std::vector<int>{9, 8, 5, 3, 2, 1});
}

TEST_CASE("functor in accumulate", "[functor]") {
    std::vector<int> v = {1, 2, 3, 4, 5};
    int sum = std::accumulate(v.begin(), v.end(), 0, plus<int>());
    REQUIRE(sum == 15);

    int product = std::accumulate(v.begin(), v.end(), 1, multiplies<int>());
    REQUIRE(product == 120);
}

TEST_CASE("functor with different integer types", "[functor]") {
    plus<void> add;
    REQUIRE(add(5, 3L) == 8L);
    REQUIRE(add(5L, 3) == 8L);
    REQUIRE(add(5, 3) == 8);

    minus<void> sub;
    REQUIRE(sub(10L, 3) == 7L);
    REQUIRE(sub(10, 3L) == 7L);
}

TEST_CASE("logical functors with different types", "[functor]") {
    logical_and<void> land;
    REQUIRE(land(5, 3) == true);
    REQUIRE(land(5, 0) == false);
    REQUIRE(land(0L, 3) == false);

    logical_or<void> lor;
    REQUIRE(lor(5, 0) == true);
    REQUIRE(lor(0L, 3) == true);
    REQUIRE(lor(0, 0L) == false);
}

TEST_CASE("negate with different types", "[functor]") {
    negate<void> neg;
    REQUIRE(neg(5) == -5);
    REQUIRE(neg(5.5) == -5.5);
    REQUIRE(neg(-3.5f) == 3.5f);
}
