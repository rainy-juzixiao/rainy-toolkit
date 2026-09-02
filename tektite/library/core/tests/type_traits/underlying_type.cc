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
#include <rainy/core/type_traits/underlying_type.hpp>
#include <type_traits>

using namespace rainy::type_traits::other_trans;

enum class SmallEnum : char { A = 1, B = 2 };
enum class LargeEnum : unsigned long long { X = 1ULL << 60 };
enum PlainEnum { RED, GREEN, BLUE };

enum class BitmaskFlags { A = 1 << 0, B = 1 << 1, C = 1 << 2 };
RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(BitmaskFlags)

enum class BitmaskFlags2 { A = 1 << 0, B = 1 << 1, AB = 3 };
RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(BitmaskFlags2)

enum class BitmaskFlags3 : unsigned char { A = 1 << 0 };
RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(BitmaskFlags3)

TEST_CASE("underlying_type - enum class with char", "[underlying_type][enum]") {
    using ut = underlying_type_t<SmallEnum>;
    REQUIRE((std::is_same_v<ut, char>));
}

TEST_CASE("underlying_type - enum class with unsigned long long", "[underlying_type][enum]") {
    using ut = underlying_type_t<LargeEnum>;
    REQUIRE((std::is_same_v<ut, unsigned long long>));
}

TEST_CASE("underlying_type - plain enum", "[underlying_type][enum]") {
    using ut = underlying_type_t<PlainEnum>;
    REQUIRE((std::is_integral_v<ut>));
}

TEST_CASE("bitmask operators - bitwise OR", "[bitmask][operators]") {
    BitmaskFlags combined = BitmaskFlags::A | BitmaskFlags::B;
    using ut = underlying_type_t<BitmaskFlags>;
    REQUIRE(static_cast<ut>(combined) ==
            (static_cast<ut>(BitmaskFlags::A) | static_cast<ut>(BitmaskFlags::B)));
}

TEST_CASE("bitmask operators - bitwise AND", "[bitmask][operators]") {
    BitmaskFlags2 combined = BitmaskFlags2::AB;
    BitmaskFlags2 result = combined & BitmaskFlags2::A;
    REQUIRE(static_cast<int>(result) == static_cast<int>(BitmaskFlags2::A));
}

TEST_CASE("bitmask operators - bitwise XOR", "[bitmask][operators]") {
    BitmaskFlags result = BitmaskFlags::A ^ BitmaskFlags::B;
    using ut = underlying_type_t<BitmaskFlags>;
    REQUIRE(static_cast<ut>(result) ==
            (static_cast<ut>(BitmaskFlags::A) ^ static_cast<ut>(BitmaskFlags::B)));
}

TEST_CASE("bitmask operators - bitwise NOT", "[bitmask][operators]") {
    BitmaskFlags3 result = ~BitmaskFlags3::A;
    using ut = underlying_type_t<BitmaskFlags3>;
    REQUIRE(static_cast<ut>(result) == static_cast<ut>(~static_cast<ut>(BitmaskFlags3::A)));
}

TEST_CASE("bitmask operators - assignment OR", "[bitmask][operators]") {
    BitmaskFlags f = BitmaskFlags::A;
    f |= BitmaskFlags::B;
    REQUIRE(static_cast<int>(f) ==
            (static_cast<int>(BitmaskFlags::A) | static_cast<int>(BitmaskFlags::B)));
}

TEST_CASE("bitmask operators - assignment AND", "[bitmask][operators]") {
    BitmaskFlags2 f = BitmaskFlags2::AB;
    f &= BitmaskFlags2::A;
    REQUIRE(static_cast<int>(f) == static_cast<int>(BitmaskFlags2::A));
}

TEST_CASE("bitmask operators - assignment XOR", "[bitmask][operators]") {
    BitmaskFlags f = BitmaskFlags::A;
    f ^= BitmaskFlags::A;
    REQUIRE(static_cast<int>(f) == 0);
}