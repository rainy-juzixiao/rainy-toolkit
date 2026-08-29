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

TEST_CASE("type compatible")
{
    STATIC_REQUIRE(typeinfo::create<int>().is_compatible(typeinfo::create<int>()));
    STATIC_REQUIRE(typeinfo::create<const int>().is_compatible(typeinfo::create<int>()));
    STATIC_REQUIRE(typeinfo::create<int&>().is_compatible(typeinfo::create<int>()));

    STATIC_REQUIRE_FALSE(typeinfo::create<float>().is_compatible(typeinfo::create<int>()));
}