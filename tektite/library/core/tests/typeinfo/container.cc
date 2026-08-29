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
#include <vector>
#include <map>

using namespace rainy::core;

TEST_CASE("sequential container")
{
    constexpr auto type = typeinfo::create<std::vector<int>>();

    STATIC_REQUIRE(type.is_sequential_container());
    STATIC_REQUIRE_FALSE(type.is_associative_container());
}

TEST_CASE("associative container")
{
    constexpr auto type = typeinfo::create<std::map<int, int>>();

    STATIC_REQUIRE(type.is_associative_container());
    STATIC_REQUIRE_FALSE(type.is_sequential_container());
}