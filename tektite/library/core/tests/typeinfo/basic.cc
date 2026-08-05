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
#include <rainy/core/typeinfo.hpp>

using namespace rainy::core;

TEST_CASE("typeinfo basic create") {
    constexpr auto type = typeinfo::create<int>();

    STATIC_REQUIRE(type.name() == type_name<int>());
    STATIC_REQUIRE(type.hash_code() == typeinfo::get_type_hash<int>());
}

TEST_CASE("typeinfo equality") {
    constexpr auto a = typeinfo::create<int>();
    constexpr auto b = typeinfo::create<int>();
    constexpr auto c = typeinfo::create<float>();

    STATIC_REQUIRE(a == b);
    STATIC_REQUIRE(a != c);
    STATIC_REQUIRE(a.is_same(b));
    STATIC_REQUIRE_FALSE(a.is_same(c));
}

TEST_CASE("typeinfo void") {
    STATIC_REQUIRE(typeinfo::create<void>().is_void());
    STATIC_REQUIRE_FALSE(typeinfo::create<int>().is_void());
}

TEST_CASE("type hash macro") {
    REQUIRE(rainy_typehash(int) == typeinfo::get_type_hash<int>());
    REQUIRE(rainy_typeid(int) == typeinfo::create<int>());
}
