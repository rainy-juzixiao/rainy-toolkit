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
#include <rainy/core/container/bitset.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

using bitset8 = rainy::core::container::bitset<8>;
using bitset16 = rainy::core::container::bitset<16>;
using bitset0 = rainy::core::container::bitset<0>;

TEST_CASE("bitset default constructor", "[bitset]") {
    bitset8 bs;
    REQUIRE(bs.none());
    REQUIRE_FALSE(bs.any());
    REQUIRE(bs.count() == 0);
    REQUIRE(bs.size() == 8);
}

TEST_CASE("bitset value constructor", "[bitset]") {
    SECTION("from unsigned long long") {
        bitset8 bs(0b10101010);
        REQUIRE(bs.test(0) == false);
        REQUIRE(bs.test(1) == true);
        REQUIRE(bs.test(7) == true);
        REQUIRE(bs.count() == 4);
    }

    SECTION("value 0") {
        bitset8 bs(0ULL);
        REQUIRE(bs.none());
    }

    SECTION("all bits set") {
        bitset8 bs(0xFFULL);
        REQUIRE(bs.all());
        REQUIRE(bs.count() == 8);
    }
}

TEST_CASE("bitset set and reset", "[bitset]") {
    bitset8 bs;

    SECTION("set all") {
        bs.set();
        REQUIRE(bs.all());
        REQUIRE(bs.count() == 8);
    }

    SECTION("set single bit") {
        bs.set(3);
        REQUIRE(bs.test(3));
        REQUIRE(bs.count() == 1);
    }

    SECTION("set single bit to false") {
        bs.set(3, false);
        REQUIRE_FALSE(bs.test(3));
    }

    SECTION("reset all") {
        bs.set();
        bs.reset();
        REQUIRE(bs.none());
    }

    SECTION("reset single bit") {
        bs.set();
        bs.reset(5);
        REQUIRE_FALSE(bs.test(5));
        REQUIRE(bs.count() == 7);
    }
}

TEST_CASE("bitset flip", "[bitset]") {
    bitset8 bs(0b10101010);

    SECTION("flip all") {
        bs.flip();
        REQUIRE(bs == bitset8(0b01010101));
    }

    SECTION("flip single bit") {
        bs.flip(0);
        REQUIRE(bs.test(0));
        bs.flip(0);
        REQUIRE_FALSE(bs.test(0));
    }
}

TEST_CASE("bitset operator[]", "[bitset]") {
    bitset8 bs;

    SECTION("const access") {
        const bitset8 &cbs = bs;
        REQUIRE_FALSE(cbs[0]);
    }

    SECTION("mutable access via reference") {
        bs[0] = true;
        REQUIRE(bs.test(0));

        bs[0] = false;
        REQUIRE_FALSE(bs.test(0));
    }

    SECTION("reference flip") {
        bs[3] = true;
        bs[3].flip();
        REQUIRE_FALSE(bs.test(3));
    }

    SECTION("reference assignment from reference") {
        bs[0] = true;
        bs[1] = bs[0];
        REQUIRE(bs.test(1));
    }
}

TEST_CASE("bitset bitwise operators", "[bitset]") {
    bitset8 a(0b11001100);
    bitset8 b(0b10101010);

    SECTION("operator&=") {
        a &= b;
        REQUIRE(a == bitset8(0b10001000));
    }

    SECTION("operator|=") {
        a |= b;
        REQUIRE(a == bitset8(0b11101110));
    }

    SECTION("operator^=") {
        a ^= b;
        REQUIRE(a == bitset8(0b01100110));
    }

    SECTION("operator~") {
        auto result = ~a;
        REQUIRE(result == bitset8(0b00110011));
    }

    SECTION("operator&") {
        auto result = a & b;
        REQUIRE(result == bitset8(0b10001000));
    }

    SECTION("operator|") {
        auto result = a | b;
        REQUIRE(result == bitset8(0b11101110));
    }

    SECTION("operator^") {
        auto result = a ^ b;
        REQUIRE(result == bitset8(0b01100110));
    }
}

TEST_CASE("bitset shift operators", "[bitset]") {
    SECTION("shift left") {
        bitset8 bs(0b00000001);
        bs <<= 3;
        REQUIRE(bs == bitset8(0b00001000));
    }

    SECTION("shift left overflow") {
        bitset8 bs(0b10000000);
        bs <<= 1;
        REQUIRE(bs.none());
    }

    SECTION("shift right") {
        bitset8 bs(0b10000000);
        bs >>= 3;
        REQUIRE(bs == bitset8(0b00010000));
    }

    SECTION("shift right overflow") {
        bitset8 bs(0b00000001);
        bs >>= 1;
        REQUIRE(bs.none());
    }

    SECTION("shift by N") {
        bitset8 bs(0b00000001);
        bs <<= 8;
        REQUIRE(bs.none());
    }

    SECTION("shift left const") {
        bitset8 bs(0b00000001);
        auto result = bs << 3;
        REQUIRE(result == bitset8(0b00001000));
        REQUIRE(bs == bitset8(0b00000001));
    }

    SECTION("shift right const") {
        bitset8 bs(0b10000000);
        auto result = bs >> 3;
        REQUIRE(result == bitset8(0b00010000));
        REQUIRE(bs == bitset8(0b10000000));
    }
}

TEST_CASE("bitset to_ulong and to_ullong", "[bitset]") {
    SECTION("to_ulong") {
        bitset8 bs(0b11001100);
        REQUIRE(bs.to_ulong() == 0b11001100);
    }

    SECTION("to_ullong") {
        bitset8 bs(0b11001100);
        REQUIRE(bs.to_ullong() == 0b11001100);
    }

    SECTION("to_ullong larger") {
        bitset16 bs(0b1010101010101010);
        REQUIRE(bs.to_ullong() == 0b1010101010101010);
    }

    SECTION("to_ulong overflow throws") {
        rainy::core::container::bitset<128> bs;
        bs.set(100);
        REQUIRE_THROWS_AS(bs.to_ulong(), std::overflow_error);
    }

    SECTION("to_ullong overflow throws") {
        rainy::core::container::bitset<128> bs;
        bs.set(100);
        REQUIRE_THROWS_AS(bs.to_ullong(), std::overflow_error);
    }

    SECTION("zero bitset to_ulong") {
        bitset0 bs;
        REQUIRE(bs.to_ulong() == 0);
    }

    SECTION("zero bitset to_ullong") {
        bitset0 bs;
        REQUIRE(bs.to_ullong() == 0);
    }
}

TEST_CASE("bitset to_string", "[bitset]") {
    SECTION("basic to_string") {
        bitset8 bs(0b10101010);
        REQUIRE(bs.to_string() == "10101010");
    }

    SECTION("to_string with custom chars") {
        bitset8 bs(0b10101010);
        REQUIRE(bs.to_string('x', 'y') == "yxyxyxyx");
    }

    SECTION("zero bitset to_string") {
        bitset8 bs;
        REQUIRE(bs.to_string() == "00000000");
    }

    SECTION("all ones to_string") {
        bitset8 bs;
        bs.set();
        REQUIRE(bs.to_string() == "11111111");
    }
}

TEST_CASE("bitset test and query", "[bitset]") {
    bitset8 bs;

    SECTION("test out of range throws") {
        REQUIRE_THROWS_AS(bs.test(8), std::out_of_range);
    }

    SECTION("all with partial bits") {
        bitset8 bs(0xFF);
        REQUIRE(bs.all());
    }

    SECTION("any") {
        REQUIRE_FALSE(bs.any());
        bs.set(0);
        REQUIRE(bs.any());
    }

    SECTION("none") {
        REQUIRE(bs.none());
        bs.set(0);
        REQUIRE_FALSE(bs.none());
    }
}

TEST_CASE("bitset comparison", "[bitset]") {
    SECTION("equal") {
        bitset8 a(0b10101010);
        bitset8 b(0b10101010);
        REQUIRE(a == b);
    }

    SECTION("not equal") {
        bitset8 a(0b10101010);
        bitset8 b(0b01010101);
        REQUIRE(a != b);
    }

    SECTION("self comparison") {
        bitset8 a(0b10101010);
        REQUIRE(a == a);
    }
}

TEST_CASE("bitset string constructor", "[bitset]") {
    SECTION("from string") {
        rainy::core::text::string str = "10101010";
        bitset8 bs(str);
        REQUIRE(bs == bitset8(0b10101010));
    }

    SECTION("from string with trailing zeros") {
        rainy::core::text::string str = "11110000";
        bitset8 bs(str);
        REQUIRE(bs.test(7));
        REQUIRE_FALSE(bs.test(0));
    }

    SECTION("from string_view") {
        rainy::core::text::string_view sv = "11001100";
        bitset8 bs(sv);
        REQUIRE(bs == bitset8(0b11001100));
    }

    SECTION("from C string") {
        bitset8 bs("10101010");
        REQUIRE(bs == bitset8(0b10101010));
    }

    SECTION("from string with custom chars") {
        rainy::core::text::string str = "yxyxyxyx";
        bitset8 bs(str, 0, rainy::core::text::string::npos, 'x', 'y');
        REQUIRE(bs == bitset8(0b10101010));
    }

    SECTION("invalid char throws") {
        rainy::core::text::string str = "10201010";
        REQUIRE_THROWS_AS(bitset8(str), std::invalid_argument);
    }
}

TEST_CASE("bitset hash", "[bitset]") {
    SECTION("rainy utility hash") {
        bitset8 a(0b10101010);
        bitset8 b(0b10101010);
        bitset8 c(0b01010101);
        rainy::utility::hash<bitset8> hasher;
        REQUIRE(hasher(a) == hasher(b));
        REQUIRE(hasher(a) != hasher(c));
    }

    SECTION("std hash") {
        bitset8 a(0b10101010);
        bitset8 b(0b10101010);
        bitset8 c(0b01010101);
        std::hash<bitset8> hasher;
        REQUIRE(hasher(a) == hasher(b));
        REQUIRE(hasher(a) != hasher(c));
    }

    SECTION("hash of zero bitset") {
        bitset8 a;
        bitset8 b;
        rainy::utility::hash<bitset8> hasher;
        REQUIRE(hasher(a) == hasher(b));
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)