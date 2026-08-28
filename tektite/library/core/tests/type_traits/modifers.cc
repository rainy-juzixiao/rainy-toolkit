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
#include <rainy/core/type_traits/modifers.hpp>

using namespace rainy::type_traits::modifers;

TEST_CASE("array modifiers", "[type_traits][modifers]") {
    SECTION("remove_extent") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_extent_t<int[2]>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_extent_t<int[2][3]>, int[3]>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_extent_t<int>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_extent_t<int[]>, int>);
    }
    SECTION("remove_all_extents") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_all_extents_t<int[2][3]>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_all_extents_t<int[2][3][4]>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_all_extents_t<int>, int>);
    }
    SECTION("rank") {
        STATIC_REQUIRE(rank<int>::value == 0);
        STATIC_REQUIRE(rank<int[2]>::value == 1);
        STATIC_REQUIRE(rank<int[2][3]>::value == 2);
        STATIC_REQUIRE(rank<int[][3]>::value == 2);
    }
    SECTION("extent") {
        STATIC_REQUIRE(extent<int[2]>::value == 2);
        STATIC_REQUIRE(extent<int[2][3], 1>::value == 3);
        STATIC_REQUIRE(extent<int[]>::value == 0);
        STATIC_REQUIRE(extent<int, 0>::value == 0);
    }
}

TEST_CASE("reference modifiers", "[type_traits][modifers]") {
    SECTION("remove_reference") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_reference_t<int &>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_reference_t<int &&>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_reference_t<int>, int>);
    }
    SECTION("add_reference") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_lvalue_reference_t<int>, int &>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_rvalue_reference_t<int>, int &&>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_lvalue_reference_t<int &>, int &>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_rvalue_reference_t<int &&>, int &&>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_const_lvalue_ref_t<int>, const int &>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_const_rvalue_ref_t<int>, const int &&>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_cvlref_t<int>, const volatile int &>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_cvrref_t<int>, const volatile int &&>);
    }
}

TEST_CASE("pointer modifiers", "[type_traits][modifers]") {
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_pointer_t<int>, int *>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_pointer_t<int *>, int **>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_pointer_t<int *>, int>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_pointer_t<int **>, int *>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_pointer_t<int>, int>);
}

TEST_CASE("cv modifiers", "[type_traits][modifers]") {
    SECTION("add const/volatile/cv") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_const_t<int>, const int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_volatile_t<int>, volatile int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<add_cv_t<int>, const volatile int>);
    }
    SECTION("remove const/volatile/cv") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_const_t<const int>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_const_t<const int *>, const int *>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_volatile_t<volatile int>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_cv_t<const volatile int>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_cv_t<const int *>, const int *>);
    }
    SECTION("remove_cvref") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_cvref_t<const int &>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_cvref_t<volatile int &&>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<remove_cvref_t<int>, int>);
    }
    SECTION("constness_as") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<constness_as_t<int, const double>, const int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<constness_as_t<int, double>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<constness_as_t<const int, double>, int>);
    }
}
