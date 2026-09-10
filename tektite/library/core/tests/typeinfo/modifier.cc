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

TEST_CASE("remove const") {
    STATIC_REQUIRE(typeinfo::create<const int>().remove_const() == typeinfo::create<int>());
}

TEST_CASE("remove volatile") {
    STATIC_REQUIRE(typeinfo::create<volatile int>().remove_volatile() == typeinfo::create<int>());
}

TEST_CASE("remove cv") {
    STATIC_REQUIRE(typeinfo::create<const volatile int>().remove_cv() == typeinfo::create<int>());
}

TEST_CASE("remove reference") {
    STATIC_REQUIRE(typeinfo::create<int &>().remove_reference() == typeinfo::create<int>());
    STATIC_REQUIRE(typeinfo::create<int &&>().remove_reference() == typeinfo::create<int>());
}

TEST_CASE("remove cvref") {
    STATIC_REQUIRE(typeinfo::create<const volatile int &>().remove_cvref() == typeinfo::create<int>());
}

TEST_CASE("remove pointer") {
    STATIC_REQUIRE(typeinfo::create<int *>().remove_pointer() == typeinfo::create<int>());
}

TEST_CASE("decay") {
    STATIC_REQUIRE(typeinfo::create<const int &>().decay() == typeinfo::create<int>());
    STATIC_REQUIRE(typeinfo::create<int[5]>().decay() == typeinfo::create<int *>());
}
