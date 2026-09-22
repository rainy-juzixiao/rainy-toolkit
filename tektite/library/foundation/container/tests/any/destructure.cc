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

#include <string>
#include <tuple>
#include <utility>

using namespace rainy::foundation::container;

TEST_CASE("any - destructure - into tuple from pair", "[any]") {
    auto p = std::make_pair(42, std::string("hello"));
    any a = p;
    auto t = a.destructure<int, std::string>();
    CHECK(std::get<0>(t) == 42);
    CHECK(std::get<1>(t) == "hello");
}

TEST_CASE("any - destructure - into tuple from tuple", "[any]") {
    auto tup = std::make_tuple(1, 2.0, std::string("three"));
    any a = tup;
    auto result = a.destructure<int, double, std::string>();
    CHECK(std::get<0>(result) == 1);
    CHECK(std::get<1>(result) == 2.0);
    CHECK(std::get<2>(result) == "three");
}

TEST_CASE("any - destructure - into tuple pair of ints", "[any]") {
    auto p = std::make_pair(100, 200);
    any a = p;
    auto t = a.destructure<int, int>();
    CHECK(std::get<0>(t) == 100);
    CHECK(std::get<1>(t) == 200);
}

TEST_CASE("any - destructure - into receiver tuple", "[any]") {
    auto p = std::make_tuple(std::string("a"), std::string("b"), std::string("c"));
    any a = p;
    std::tuple<std::string, std::string, std::string> receiver;
    bool ok = a.destructure(receiver);
    CHECK(ok == true);
    CHECK(std::get<0>(receiver) == "a");
    CHECK(std::get<1>(receiver) == "b");
    CHECK(std::get<2>(receiver) == "c");
}

TEST_CASE("any - destructure - into callable sum", "[any]") {
    auto p = std::make_pair(7, 8);
    any a = p;
    int sum = 0;
    bool ok = a.destructure([&sum](int x, int y) { sum = x + y; });
    CHECK(ok == true);
    CHECK(sum == 15);
}

TEST_CASE("any - destructure - into callable with string", "[any]") {
    auto p = std::make_pair(42, std::string("world"));
    any a = p;
    std::string result;
    bool ok = a.destructure([&result](int n, const std::string &s) {
        result = std::to_string(n) + " " + s;
    });
    CHECK(ok == true);
    CHECK(result == "42 world");
}

TEST_CASE("any - destructure - into callable three args", "[any]") {
    auto t = std::make_tuple(10, 20, 30);
    any a = t;
    int sum = 0;
    bool ok = a.destructure([&sum](int x, int y, int z) { sum = x + y + z; });
    CHECK(ok == true);
    CHECK(sum == 60);
}

TEST_CASE("any - destructure - NEGATIVE - non-tuple-like any to tuple", "[any]") {
    any a = 42;
    std::tuple<int, int> receiver;
    bool ok = a.destructure(receiver);
    CHECK(ok == false);
}

TEST_CASE("any - destructure - NEGATIVE - non-tuple-like any to callable", "[any]") {
    any a = std::string("not_a_pair");
    bool ok = a.destructure([](int x, int y) { return x + y; });
    CHECK(ok == false);
}
