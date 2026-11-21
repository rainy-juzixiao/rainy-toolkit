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
#include <rainy/foundation/willow/json.hpp>

#include <iostream>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json;

TEST_CASE("json container - object subscript creates on demand", "[willow][json]") {
    json::facade<document> j;
    REQUIRE(j.is_null());
    j["a"] = 1;
    REQUIRE(j.is_object());
    REQUIRE(j["a"].as_integer() == 1);
    REQUIRE(j.count("a") == 1);
    REQUIRE(j.count("b") == 0);
}

TEST_CASE("json container - array subscript", "[willow][json]") {
    json::facade<document> j = json::facade<document>::array({10, 20});
    REQUIRE(j[0].as_integer() == 10);
    REQUIRE(j[1].as_integer() == 20);

    j[5] = 1;
    REQUIRE(j.size() == 6);
    REQUIRE(j[5].as_integer() == 1);
}

TEST_CASE("json container - const array subscript out of range throws", "[willow][json]") {
    const json::facade<document> j = json::facade<document>::array({1});
    REQUIRE_THROWS_AS(j[5], std::out_of_range);
}

TEST_CASE("json container - const object missing key throws", "[willow][json]") {
    const json::facade<document> j = json::facade<document>::object({{"k", 1}});
    REQUIRE_THROWS_AS(j["missing"], std::out_of_range);
}

TEST_CASE("json container - subscript on wrong type throws willow_invalid_key", "[willow][json]") {
    using rainy::foundation::exceptions::willow::willow_invalid_key;
    json::facade<document> j = 42;
    REQUIRE_THROWS_AS(j["key"], willow_invalid_key);

    json::facade<document> s = "x";
    REQUIRE_THROWS_AS(s[0], willow_invalid_key);
}

TEST_CASE("json container - size and empty", "[willow][json]") {
    json::facade<document> obj = json::parse("{}");
    REQUIRE(obj.size() == 0);
    REQUIRE(obj.empty());

    json::facade<document> arr = json::parse("[1,2]");
    REQUIRE(arr.size() == 2);
    REQUIRE(!arr.empty());
    json::facade<document> primitive = 42;
    REQUIRE(primitive.size() == 1);
    REQUIRE(!primitive.empty());
}

TEST_CASE("json container - count and contains-style lookup", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"alpha":1,"beta":2})");
    REQUIRE(j.count("alpha") == 1);
    REQUIRE(j.count("gamma") == 0);

    json::facade<document> arr = json::parse("[1]");
    REQUIRE(arr.count("alpha") == 0);
}

TEST_CASE("json container - erase by key", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"a":1,"b":2})");
    const auto erased = j.erase("a");
    REQUIRE(erased == 1);
    REQUIRE(j.count("a") == 0);
    REQUIRE(j.size() == 1);
}

TEST_CASE("json container - erase by index", "[willow][json]") {
    json::facade<document> j = json::facade<document>::array({1, 2, 3});
    j.erase(static_cast<json::facade<document>::size_type>(1));
    REQUIRE(j.size() == 2);
    REQUIRE(j[0].as_integer() == 1);
    REQUIRE(j[1].as_integer() == 3);
}

TEST_CASE("json container - push_back and emplace_back", "[willow][json]") {
    json::facade<document> j;
    j.push_back(1);
    REQUIRE(j.is_array());
    REQUIRE(j.size() == 1);

    j.push_back("s");
    REQUIRE(j.size() == 2);
    REQUIRE(j[1].is_string());

    j.emplace_back(3);
    REQUIRE(j.size() == 3);
    REQUIRE(j[2].as_integer() == 3);
}

TEST_CASE("json container - clear", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"a":1})");
    j.clear();
    REQUIRE(j.is_null());

    json::facade<document> arr = json::parse("[1,2]");
    arr.clear();
    REQUIRE(arr.is_null());
}

TEST_CASE("json container - range for over array", "[willow][json]") {
    json::facade<document> j = json::facade<document>::array({1, 2, 3});
    int sum = 0;
    for (const auto &element: j) {
        sum += element.value().as_integer();
    }
    REQUIRE(sum == 6);
}

TEST_CASE("json container - reverse iteration over array", "[willow][json]") {
    json::facade<document> j = json::facade<document>::array({1, 2});
    auto it = j.rbegin();
    REQUIRE(it->value().as_integer() == 2);
    ++it;
    REQUIRE(it->value().as_integer() == 1);
    ++it;
    REQUIRE(it == j.rend());
}

TEST_CASE("json container - copy and move semantics", "[willow][json]") {
    json::facade<document> a = json::parse(R"({"k":[1,2]})");
    json::facade<document> b = a;
    REQUIRE(b == a);
    b["k"] = 9;
    REQUIRE(a["k"][0].as_integer() == 1);
    REQUIRE(b["k"].as_integer() == 9);

    json::facade<document> c = std::move(b);
    REQUIRE(c["k"].as_integer() == 9);
}
