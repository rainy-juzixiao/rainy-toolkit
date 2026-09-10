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
#include <rainy/core/utility/bit.hpp>

using namespace rainy::utility;

TEST_CASE("popcount and single bit checks", "[utility][bit]") {
    STATIC_REQUIRE(popcount(0u) == 0);
    STATIC_REQUIRE(popcount(0b1011u) == 3);
    STATIC_REQUIRE(popcount(0xFFu) == 8);
    REQUIRE(popcount(0xF0F0'F0F0'F0F0'F0F0ull) == 32);

    STATIC_REQUIRE(has_single_bit(0u) == false);
    STATIC_REQUIRE(has_single_bit(1u) == true);
    STATIC_REQUIRE(has_single_bit(64u) == true);
    STATIC_REQUIRE(has_single_bit(96u) == false);
}

TEST_CASE("power of two helpers", "[utility][bit]") {
    STATIC_REQUIRE(next_power_of_two(1u) == 1);
    STATIC_REQUIRE(next_power_of_two(5u) == 8);
    STATIC_REQUIRE(next_power_of_two(8u) == 8);
    REQUIRE(next_power_of_two(1000ull) == 1024);

    STATIC_REQUIRE(bit_width(0u) == 0);
    STATIC_REQUIRE(bit_width(1u) == 1);
    STATIC_REQUIRE(bit_width(255u) == 8);
    REQUIRE(bit_width(1024ull) == 11);

    STATIC_REQUIRE(bit_floor(0u) == 0);
    STATIC_REQUIRE(bit_floor(7u) == 4);
    STATIC_REQUIRE(bit_floor(8u) == 8);
    REQUIRE(bit_floor(1000ull) == 512);
}

TEST_CASE("count leading and trailing zeros", "[utility][bit]") {
    REQUIRE(countl_zero(1u) == 31);
    REQUIRE(countl_zero(0x8000'0000u) == 0);
    REQUIRE(countr_zero(1u) == 0);
    REQUIRE(countr_zero(0x8000'0000u) == 31);
    REQUIRE(countr_zero(0u) == 32);
}

TEST_CASE("bitwise modulo and rotation", "[utility][bit]") {
    REQUIRE(mod(10u, 8) == 2);
    REQUIRE(mod(16u, 8) == 0);
    REQUIRE(mod(15u, 16) == 15);

    REQUIRE(rotate_left(0x8000'0000u, 1) == 1u);
    REQUIRE(rotate_left(1u, 4) == 16u);
    REQUIRE(rotate_right(1u, 1) == 0x8000'0000u);
    REQUIRE(rotate_right(16u, 4) == 1u);
}
