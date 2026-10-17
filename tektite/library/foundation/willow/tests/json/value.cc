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

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json;

TEST_CASE("json value - default is null", "[willow][json]") {
    json::facade<document> j;
    REQUIRE(j.is_null());
    REQUIRE(j.type() == document_type::null);
    REQUIRE(j.type_name() == "null");
    REQUIRE(j.empty());
}

TEST_CASE("json value - integer construction", "[willow][json]") {
    json::facade<document> j = 42;
    REQUIRE(j.is_integer());
    REQUIRE(j.is_number());
    REQUIRE(!j.is_float());
    REQUIRE(j.type() == document_type::number_integer);
    REQUIRE(j.as_integer() == 42);
    REQUIRE(j.get<int>() == 42);
    REQUIRE(j.type_name() == "integer");
}

TEST_CASE("json value - float construction", "[willow][json]") {
    json::facade<document> j = 3.14;
    REQUIRE(j.is_float());
    REQUIRE(j.is_number());
    REQUIRE(!j.is_integer());
    REQUIRE(j.type() == document_type::number_float);
    REQUIRE(j.as_float() == 3.14);
    REQUIRE(j.type_name() == "float");
}

TEST_CASE("json value - bool construction", "[willow][json]") {
    json::facade<document> j = true;
    REQUIRE(j.is_bool());
    REQUIRE(j.as_bool());
    REQUIRE(j.type_name() == "boolean");

    json::facade<document> f = false;
    REQUIRE(f.is_bool());
    REQUIRE(!f.as_bool());
}

TEST_CASE("json value - string construction", "[willow][json]") {
    json::facade<document> j = "hello";
    REQUIRE(j.is_string());
    REQUIRE(j.as_string() == "hello");
    REQUIRE(j.as_string_view() == "hello");
    REQUIRE(j.type_name() == "string");
}

TEST_CASE("json value - numeric cross conversion", "[willow][json]") {
    json::facade<document> integer = 10;
    REQUIRE(integer.as_float() == 10.0);

    json::facade<document> floating = 2.0;
    REQUIRE(floating.as_integer() == 2);
}

TEST_CASE("json value - get typed accessors", "[willow][json]") {
    json::facade<document> j_int = 7;
    REQUIRE(j_int.get<int>() == 7);

    json::facade<document> j_bool = true;
    REQUIRE(j_bool.get<bool>());
}

TEST_CASE("json value - as_* type errors throw willow_type_error", "[willow][json]") {
    json::facade<document> j = 42;
    REQUIRE_THROWS_AS(j.as_string(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(j.as_bool(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(j.as_array(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(j.as_object(), rainy::foundation::exceptions::willow::willow_type_error);

    json::facade<document> s = "x";
    REQUIRE_THROWS_AS(s.as_integer(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(s.as_float(), rainy::foundation::exceptions::willow::willow_type_error);
}

TEST_CASE("json value - initializer list infers array", "[willow][json]") {
    json::facade<document> j{1, 2, 3};
    REQUIRE(j.is_array());
    REQUIRE(j.size() == 3);
    REQUIRE(j[0].as_integer() == 1);
    REQUIRE(j[2].as_integer() == 3);
}

TEST_CASE("json value - initializer list of pairs infers object", "[willow][json]") {
    json::facade<document> j{{"name", std::string("rainy")}, {"version", 1}};
    REQUIRE(j.is_object());
    REQUIRE(j.size() == 2);
    REQUIRE(j["name"].as_string() == "rainy");
    REQUIRE(j["version"].as_integer() == 1);
}

TEST_CASE("json value - object/array static factories", "[willow][json]") {
    json::facade<document> obj = json::facade<document>::object({{"k", 1}});
    REQUIRE(obj.is_object());
    REQUIRE(obj["k"].as_integer() == 1);

    json::facade<document> arr = json::facade<document>::array({1, 2});
    REQUIRE(arr.is_array());
    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0].as_integer() == 1);
}

TEST_CASE("json value - comparison operators", "[willow][json]") {
    REQUIRE(json::facade<document>(1) == json::facade<document>(1));
    REQUIRE(json::facade<document>(1) != json::facade<document>(2));
    REQUIRE(json::facade<document>(1) < json::facade<document>(2));
    REQUIRE(json::facade<document>(2) > json::facade<document>(1));
    REQUIRE(json::facade<document>(1) <= json::facade<document>(1));
    REQUIRE(json::facade<document>(1) >= json::facade<document>(1));
    REQUIRE(json::facade<document>("a") == json::facade<document>("a"));
    REQUIRE(json::facade<document>{} == json::facade<document>{});
}

TEST_CASE("json value - explicit conversion operator", "[willow][json]") {
    json::facade<document> j = 42;
    REQUIRE(static_cast<int>(j) == 42);

    json::facade<document> s = "xy";
    REQUIRE(s.as_string() == "xy");
}
