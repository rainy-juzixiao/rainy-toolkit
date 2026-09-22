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
#include <rainy/foundation/container/any.hpp>

#include <sstream>
#include <string>
#include <vector>

using namespace rainy::foundation::container;

TEST_CASE("any - operators - equality int", "[any]") {
    any a = 42;
    any b = 42;
    any c = 100;
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("any - operators - inequality int", "[any]") {
    any a = 42;
    any b = 100;
    CHECK(a != b);
    CHECK_FALSE(a != a);
}

TEST_CASE("any - operators - less than int", "[any]") {
    any a = 10;
    any b = 20;
    CHECK(a < b);
    CHECK_FALSE(b < a);
}

TEST_CASE("any - operators - less equal int", "[any]") {
    any a = 10;
    any b = 10;
    any c = 20;
    CHECK(a <= b);
    CHECK(a <= c);
    CHECK_FALSE(c <= a);
}

TEST_CASE("any - operators - greater than int", "[any]") {
    any a = 30;
    any b = 20;
    CHECK(a > b);
    CHECK_FALSE(b > a);
}

TEST_CASE("any - operators - greater equal int", "[any]") {
    any a = 20;
    any b = 20;
    any c = 30;
    CHECK(a >= b);
    CHECK(c >= a);
    CHECK_FALSE(a >= c);
}

TEST_CASE("any - operators - comparison with negative numbers", "[any]") {
    any a = -10;
    any b = 5;
    any c = -10;
    CHECK(a < b);
    CHECK(a == c);
    CHECK(b > a);
}

TEST_CASE("any - operators - double comparison", "[any]") {
    any a = 3.14;
    any b = 3.14;
    any c = 2.718;
    CHECK(a == b);
    CHECK(a != c);
    CHECK(c < a);
    CHECK(a > c);
}

TEST_CASE("any - operators - string comparison", "[any]") {
    any a = std::string("apple");
    any b = std::string("apple");
    any c = std::string("banana");
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a < c);
}

TEST_CASE("any - operators - string comparison empty", "[any]") {
    any a = std::string("");
    any b = std::string("a");
    CHECK(a != b);
    CHECK(a < b);
}

TEST_CASE("any - operators - subscript by index on int vector", "[any]") {
    any vec = std::vector<int>{10, 20, 30, 40, 50};
    CHECK(vec[0].as<int>() == 10);
    CHECK(vec[1].as<int>() == 20);
    CHECK(vec[2].as<int>() == 30);
    CHECK(vec[3].as<int>() == 40);
    CHECK(vec[4].as<int>() == 50);
}

TEST_CASE("any - operators - subscript by index on string vector", "[any]") {
    any vec = std::vector<std::string>{"a", "b", "c"};
    CHECK(vec[0].as<std::string>() == "a");
    CHECK(vec[1].as<std::string>() == "b");
    CHECK(vec[2].as<std::string>() == "c");
}

TEST_CASE("any - operators - const subscript by index", "[any]") {
    const any vec = std::vector<int>{100, 200, 300};
    CHECK(vec[0].as<const int>() == 100);
    CHECK(vec[1].as<const int>() == 200);
    CHECK(vec[2].as<const int>() == 300);
}

TEST_CASE("any - operators - dereference pointer", "[any]") {
    int value = 42;
    int *ptr = &value;
    any a = ptr;
    any b = *a;
    CHECK(b.has_value() == true);
}

TEST_CASE("any - operators - stream output int", "[any]") {
    any a = 42;
    std::ostringstream oss;
    oss << a;
    CHECK(oss.str() == "42");
}

TEST_CASE("any - operators - stream output negative int", "[any]") {
    any a = -100;
    std::ostringstream oss;
    oss << a;
    CHECK(oss.str() == "-100");
}

TEST_CASE("any - operators - stream output double", "[any]") {
    any a = 3.14;
    std::ostringstream oss;
    oss << a;
    CHECK(oss.str() == "3.14");
}

TEST_CASE("any - operators - stream output string", "[any]") {
    any a = std::string("hello");
    std::ostringstream oss;
    oss << a;
    CHECK(oss.str() == "hello");
}

TEST_CASE("any - operators - stream output empty any", "[any]") {
    any a;
    std::ostringstream oss;
    oss << a;
    CHECK(oss.str().empty());
}

TEST_CASE("any - operators - compound operations return value", "[any]") {
    any a = 10;
    any b = 5;
    a += b;
    CHECK(a.has_value() == true);
}

TEST_CASE("any - operators - NEGATIVE - comparison with empty any", "[any]") {
    any a;
    any b = 42;
    CHECK_FALSE(a == b);
    CHECK_FALSE(a != b);
    CHECK_FALSE(a < b);
    CHECK_FALSE(a > b);
}

TEST_CASE("any - operators - NEGATIVE - empty any compared to empty any", "[any]") {
    any a;
    any b;
    CHECK_FALSE(a == b);
    CHECK_FALSE(a != b);
}
