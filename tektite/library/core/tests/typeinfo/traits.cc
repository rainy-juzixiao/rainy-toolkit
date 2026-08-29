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

struct TestClass
{
    int value;
};

enum class TestEnum
{
    A,
    B
};

union TestUnion
{
    int i;
    float f;
};

TEST_CASE("fundamental traits")
{
    STATIC_REQUIRE(typeinfo::create<int>().is_integer());
    STATIC_REQUIRE(typeinfo::create<int>().is_arithmetic());

    STATIC_REQUIRE(typeinfo::create<float>().is_floating_point());
    STATIC_REQUIRE(typeinfo::create<float>().is_arithmetic());
}

TEST_CASE("pointer traits")
{
    STATIC_REQUIRE(typeinfo::create<int*>().is_pointer());
}

TEST_CASE("reference traits")
{
    STATIC_REQUIRE(typeinfo::create<int&>().is_reference());
    STATIC_REQUIRE(typeinfo::create<int&>().is_lvalue_reference());
    STATIC_REQUIRE(typeinfo::create<int&&>().is_rvalue_reference());
}

TEST_CASE("cv traits")
{
    STATIC_REQUIRE(typeinfo::create<const int>().is_const());
    STATIC_REQUIRE(typeinfo::create<volatile int>().is_volatile());
    STATIC_REQUIRE(typeinfo::create<const volatile int>().is_const_volatile());
}

TEST_CASE("class traits")
{
    STATIC_REQUIRE(typeinfo::create<TestClass>().is_class());
}

TEST_CASE("enum union traits")
{
    STATIC_REQUIRE(typeinfo::create<TestEnum>().has_traits(traits::is_enum));
    STATIC_REQUIRE(typeinfo::create<TestUnion>().has_traits(traits::is_union));
}