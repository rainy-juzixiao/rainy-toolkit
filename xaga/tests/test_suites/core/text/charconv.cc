#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <cstring>
#include <limits>
#include <rainy/core/core.hpp>
#include <string>

using namespace rainy;
using namespace rainy::foundation::text;

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// ============================================================================
// 整数 to_chars 极限测试
// ============================================================================

SCENARIO("Integer to_chars - All signed types boundaries", "[to_chars][integer][extreme]") {
    char buffer[256];

    WHEN("Testing char boundaries") {
        THEN("signed char min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed char>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed char>::max());
            REQUIRE(r2.ec == std::errc{});
        }

        THEN("unsigned char min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned char>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned char>::max());
            REQUIRE(r2.ec == std::errc{});
        }

        THEN("char min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<char>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<char>::max());
            REQUIRE(r2.ec == std::errc{});
        }
    }

    WHEN("Testing short boundaries") {
        THEN("signed short min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed short>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed short>::max());
            REQUIRE(r2.ec == std::errc{});
        }

        THEN("unsigned short min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned short>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned short>::max());
            REQUIRE(r2.ec == std::errc{});
        }
    }

    WHEN("Testing int boundaries") {
        THEN("signed int min/max/zero") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed int>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed int>::max());
            REQUIRE(r2.ec == std::errc{});
            auto r3 = to_chars(buffer, buffer + sizeof(buffer), 0);
            REQUIRE(r3.ec == std::errc{});
        }

        THEN("unsigned int min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned int>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned int>::max());
            REQUIRE(r2.ec == std::errc{});
        }
    }

    WHEN("Testing long boundaries") {
        THEN("signed long min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed long>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed long>::max());
            REQUIRE(r2.ec == std::errc{});
        }

        THEN("unsigned long min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned long>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned long>::max());
            REQUIRE(r2.ec == std::errc{});
        }
    }

    WHEN("Testing long long boundaries") {
        THEN("signed long long min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed long long>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<signed long long>::max());
            REQUIRE(r2.ec == std::errc{});
        }

        THEN("unsigned long long min/max") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned long long>::min());
            REQUIRE(r1.ec == std::errc{});
            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<unsigned long long>::max());
            REQUIRE(r2.ec == std::errc{});
        }
    }
}

SCENARIO("Integer to_chars - All bases with all types", "[to_chars][integer][bases]") {
    char buffer[256];

    WHEN("Testing all valid bases (2-36)") {
        for (int base = 2; base <= 36; ++base) {
            THEN("Base " + std::to_string(base) + " works for various values") {
                auto r1 = to_chars(buffer, buffer + sizeof(buffer), 0, base);
                REQUIRE(r1.ec == std::errc{});

                auto r2 = to_chars(buffer, buffer + sizeof(buffer), 1, base);
                REQUIRE(r2.ec == std::errc{});

                auto r3 = to_chars(buffer, buffer + sizeof(buffer), 255, base);
                REQUIRE(r3.ec == std::errc{});

                auto r4 = to_chars(buffer, buffer + sizeof(buffer), -1, base);
                REQUIRE(r4.ec == std::errc{});
            }
        }
    }

    WHEN("Testing power-of-2 bases specifically (path coverage)") {
        std::array<int, 5> pow2_bases = {2, 4, 8, 16, 32};
        for (int base: pow2_bases) {
            THEN("Power-of-2 base " + std::to_string(base)) {
                auto r1 = to_chars(buffer, buffer + sizeof(buffer), 65535, base);
                REQUIRE(r1.ec == std::errc{});

                auto r2 = to_chars(buffer, buffer + sizeof(buffer), -32768, base);
                REQUIRE(r2.ec == std::errc{});
            }
        }
    }

    WHEN("Testing non-power-of-2 bases (different path)") {
        std::array<int, 5> bases = {3, 5, 7, 10, 13};
        for (int base: bases) {
            THEN("Non-power-of-2 base " + std::to_string(base)) {
                auto r1 = to_chars(buffer, buffer + sizeof(buffer), 1000, base);
                REQUIRE(r1.ec == std::errc{});

                auto r2 = to_chars(buffer, buffer + sizeof(buffer), -500, base);
                REQUIRE(r2.ec == std::errc{});
            }
        }
    }

    WHEN("Testing base <= 8 vs base > 8 paths") {
        THEN("Base 2-8 uses one path") {
            for (int base = 2; base <= 8; ++base) {
                auto r = to_chars(buffer, buffer + sizeof(buffer), 255, base);
                REQUIRE(r.ec == std::errc{});
            }
        }

        THEN("Base 9-36 uses another path") {
            for (int base = 9; base <= 36; ++base) {
                auto r = to_chars(buffer, buffer + sizeof(buffer), 255, base);
                REQUIRE(r.ec == std::errc{});
            }
        }
    }
}

SCENARIO("Integer to_chars - Buffer boundary tests", "[to_chars][integer][buffer]") {
    WHEN("Buffer size exactly matches output") {
        THEN("Single digit fits in 1 byte buffer") {
            char buf[1];
            auto r = to_chars(buf, buf + 1, 5);
            if (r.ec == std::errc{}) {
                REQUIRE(r.ptr == buf + 1);
            }
        }

        THEN("Two digit number needs 2 bytes") {
            char buf[2];
            auto r = to_chars(buf, buf + 2, 42);
            if (r.ec == std::errc{}) {
                REQUIRE(r.ptr == buf + 2);
            }
        }

        THEN("Negative single digit needs 2 bytes") {
            char buf[2];
            auto r = to_chars(buf, buf + 2, -5);
            if (r.ec == std::errc{}) {
                REQUIRE(r.ptr == buf + 2);
            }
        }
    }

    WHEN("Buffer too small") {
        THEN("0 byte buffer fails") {
            char buf[1];
            auto r = to_chars(buf, buf, 42);
            REQUIRE(r.ec != std::errc{});
        }

        THEN("Insufficient buffer for negative") {
            char buf[1];
            auto r = to_chars(buf, buf + 1, -42);
            REQUIRE(r.ec != std::errc{});
        }

        THEN("Insufficient buffer for large number") {
            char buf[5];
            auto r = to_chars(buf, buf + 5, 123456789);
            REQUIRE(r.ec != std::errc{});
        }
    }

    WHEN("Testing with nullptr (undefined but shouldn't crash)") {
        THEN("begin == end nullptr") {
            char *null_ptr = nullptr;
            auto r = to_chars(null_ptr, null_ptr, 42);
            // Should handle gracefully
        }
    }
}

// ============================================================================
// 整数 from_chars 极限测试
// ============================================================================

SCENARIO("Integer from_chars - All signed types boundaries", "[from_chars][integer][extreme]") {
    using utility::begin;
    using utility::end;

    constexpr string_view sc_max = "127";
    constexpr string_view sc_min = "-128";
    constexpr string_view sc_ovf = "128";
    constexpr string_view sc_unf = "-129";

    constexpr string_view uc_max = "255";
    constexpr string_view uc_ovf = "256";
    constexpr string_view uc_neg = "-1";

    constexpr string_view s_max = "32767";
    constexpr string_view s_min = "-32768";
    constexpr string_view s_ovf = "32768";

    constexpr string_view us_max = "65535";
    constexpr string_view us_ovf = "65536";

    constexpr string_view i_max = "2147483647";
    constexpr string_view i_min = "-2147483648";
    constexpr string_view i_ovf = "2147483648";
    constexpr string_view i_unf = "-2147483649";

    constexpr string_view ui_max = "4294967295";
    constexpr string_view ui_ovf = "4294967296";

    constexpr string_view ll_max = "9223372036854775807";
    constexpr string_view ll_min = "-9223372036854775808";
    constexpr string_view ll_ovf = "9223372036854775808";

    constexpr string_view ull_max = "18446744073709551615";
    constexpr string_view ull_ovf = "18446744073709551616";

    WHEN("Parsing to signed char") {
        THEN("Max value") {
            signed char val = 0;
            auto r = from_chars(begin(sc_max), end(sc_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 127);
        }

        THEN("Min value") {
            signed char val = 0;
            auto r = from_chars(begin(sc_min), end(sc_min), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -128);
        }

        THEN("Overflow") {
            signed char val = 0;
            auto r = from_chars(begin(sc_ovf), end(sc_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Underflow") {
            signed char val = 0;
            auto r = from_chars(begin(sc_unf), end(sc_unf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned char") {
        THEN("Max value") {
            unsigned char val = 0;
            auto r = from_chars(begin(uc_max), end(uc_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Overflow") {
            unsigned char val = 0;
            auto r = from_chars(begin(uc_ovf), end(uc_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Negative on unsigned") {
            unsigned char val = 0;
            auto r = from_chars(begin(uc_neg), end(uc_neg), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Parsing to short") {
        THEN("Max value") {
            short val = 0;
            auto r = from_chars(begin(s_max), end(s_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 32767);
        }

        THEN("Min value") {
            short val = 0;
            auto r = from_chars(begin(s_min), end(s_min), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -32768);
        }

        THEN("Overflow") {
            short val = 0;
            auto r = from_chars(begin(s_ovf), end(s_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned short") {
        THEN("Max value") {
            unsigned short val = 0;
            auto r = from_chars(begin(us_max), end(us_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 65535);
        }

        THEN("Overflow") {
            unsigned short val = 0;
            auto r = from_chars(begin(us_ovf), end(us_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to int") {
        THEN("Max int32") {
            int val = 0;
            auto r = from_chars(begin(i_max), end(i_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 2147483647);
        }

        THEN("Min int32") {
            int val = 0;
            auto r = from_chars(begin(i_min), end(i_min), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == static_cast<int>(-2147483648));
        }

        THEN("Overflow by 1") {
            int val = 0;
            auto r = from_chars(begin(i_ovf), end(i_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Underflow by 1") {
            int val = 0;
            auto r = from_chars(begin(i_unf), end(i_unf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned int") {
        THEN("Max uint32") {
            unsigned int val = 0;
            auto r = from_chars(begin(ui_max), end(ui_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 4294967295U);
        }

        THEN("Overflow") {
            unsigned int val = 0;
            auto r = from_chars(begin(ui_ovf), end(ui_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to long long") {
        THEN("Max int64") {
            long long val = 0;
            auto r = from_chars(begin(ll_max), end(ll_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 9223372036854775807LL);
        }

        THEN("Min int64") {
            long long val = 0;
            auto r = from_chars(begin(ll_min), end(ll_min), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -9223372036854775807LL - 1);
        }

        THEN("Overflow") {
            long long val = 0;
            auto r = from_chars(begin(ll_ovf), end(ll_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned long long") {
        THEN("Max uint64") {
            unsigned long long val = 0;
            auto r = from_chars(begin(ull_max), end(ull_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 18446744073709551615ULL);
        }

        THEN("Overflow") {
            unsigned long long val = 0;
            auto r = from_chars(begin(ull_ovf), end(ull_ovf), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }
}

SCENARIO("Integer from_chars - All bases exhaustive", "[from_chars][integer][bases]") {
    using utility::begin;
    using utility::end;

    constexpr string_view zero = "0";
    constexpr string_view one = "1";

    // Power-of-2 bases
    constexpr string_view bin10 = "10";
    constexpr string_view bin255 = "11111111";
    constexpr string_view base4_255 = "3333";
    constexpr string_view oct511 = "777";
    constexpr string_view hex_ff = "ff";
    constexpr string_view hex_FF = "FF";
    constexpr string_view hex_long = "aAbBcCdDeEfF";
    constexpr string_view base32_vv = "vv";

    // Non-power-of-2 bases <= 10
    constexpr string_view base3_2222 = "2222";
    constexpr string_view base5_444 = "444";
    constexpr string_view base10_9999 = "9999";

    // Non-power-of-2 bases > 10
    constexpr string_view base11_aa = "aa";
    constexpr string_view base13_cc = "cc";
    constexpr string_view base36_zz = "zz";

    WHEN("Testing each base 2-36") {
        for (int base = 2; base <= 36; ++base) {
            THEN("Base " + std::to_string(base) + " parses correctly") {
                int val = 0;

                auto r0 = from_chars(begin(zero), end(zero), val, base);
                REQUIRE(r0.ec == std::errc{});
                REQUIRE(val == 0);

                auto r1 = from_chars(begin(one), end(one), val, base);
                REQUIRE(r1.ec == std::errc{});
                REQUIRE(val == 1);

                // base-1 digit
                if (base <= 10) {
                    char digit[2] = {char('0' + base - 1), 0};
                    auto r = from_chars(digit, digit + 1, val, base);
                    REQUIRE(r.ec == std::errc{});
                    REQUIRE(val == base - 1);
                } else {
                    char digit[2] = {char('a' + base - 11), 0};
                    auto r = from_chars(digit, digit + 1, val, base);
                    REQUIRE(r.ec == std::errc{});
                    REQUIRE(val == base - 1);
                }
            }
        }
    }

    WHEN("Testing power-of-2 bases <= 8") {
        THEN("Base 2 edge cases") {
            int val = 0;
            auto r1 = from_chars(begin(bin10), end(bin10), val, 2);
            REQUIRE(r1.ec == std::errc{});
            REQUIRE(val == 2);

            auto r2 = from_chars(begin(bin255), end(bin255), val, 2);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Base 4 edge cases") {
            int val = 0;
            auto r = from_chars(begin(base4_255), end(base4_255), val, 4);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Base 8 edge cases") {
            int val = 0;
            auto r = from_chars(begin(oct511), end(oct511), val, 8);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 511);
        }
    }

    WHEN("Testing power-of-2 bases > 8") {
        THEN("Base 16 hex digits") {
            int val = 0;
            auto r1 = from_chars(begin(hex_ff), end(hex_ff), val, 16);
            REQUIRE(r1.ec == std::errc{});
            REQUIRE(val == 255);

            auto r2 = from_chars(begin(hex_FF), end(hex_FF), val, 16);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(val == 255);

            auto r3 = from_chars(begin(hex_long), end(hex_long), val, 16);
            REQUIRE(r3.ec == std::errc::result_out_of_range);
        }

        THEN("Base 32 edge cases") {
            int val = 0;
            auto r = from_chars(begin(base32_vv), end(base32_vv), val, 32);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 1023);
        }
    }

    WHEN("Testing non-power-of-2 bases <= 10") {
        THEN("Base 3") {
            int val = 0;
            auto r = from_chars(begin(base3_2222), end(base3_2222), val, 3);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 80);
        }

        THEN("Base 5") {
            int val = 0;
            auto r = from_chars(begin(base5_444), end(base5_444), val, 5);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 124);
        }

        THEN("Base 10") {
            int val = 0;
            auto r = from_chars(begin(base10_9999), end(base10_9999), val, 10);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 9999);
        }
    }

    WHEN("Testing non-power-of-2 bases > 10") {
        THEN("Base 11") {
            int val = 0;
            auto r = from_chars(begin(base11_aa), end(base11_aa), val, 11);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 120);
        }

        THEN("Base 13") {
            int val = 0;
            auto r = from_chars(begin(base13_cc), end(base13_cc), val, 13);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 168);
        }

        THEN("Base 36 with all digits") {
            int val = 0;
            auto r = from_chars(begin(base36_zz), end(base36_zz), val, 36);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 1295);
        }
    }
}

SCENARIO("Integer from_chars - Invalid input coverage", "[from_chars][integer][invalid]") {
    using utility::begin;
    using utility::end;

    constexpr string_view empty = "";
    constexpr string_view ws3 = "   ";
    constexpr string_view lead_ws = " 42";
    constexpr string_view abc = "abc";
    constexpr string_view special = "@#$";
    constexpr string_view plus_42 = "+42";
    constexpr string_view digits_then_letters = "123abc";
    constexpr string_view neg_then_letters = "-45xyz";
    constexpr string_view oct_invalid = "1239";
    constexpr string_view only_minus = "-";
    constexpr string_view multi_minus = "--5";
    constexpr string_view minus_middle = "12-34";

    WHEN("Empty or whitespace input") {
        THEN("Empty string") {
            int val = 99;
            auto r = from_chars(begin(empty), end(empty), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
            REQUIRE(r.ptr == begin(empty));
        }

        THEN("Only whitespace") {
            int val = 99;
            auto r = from_chars(begin(ws3), end(ws3), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Leading whitespace") {
            int val = 99;
            auto r = from_chars(begin(lead_ws), end(lead_ws), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Invalid characters") {
        THEN("Pure alphabetic") {
            int val = 99;
            auto r = from_chars(begin(abc), end(abc), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Special characters") {
            int val = 99;
            auto r = from_chars(begin(special), end(special), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Plus sign (not supported)") {
            int val = 99;
            auto r = from_chars(begin(plus_42), end(plus_42), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Partial valid input") {
        THEN("Digits then letters") {
            int val = 0;
            auto r = from_chars(begin(digits_then_letters), end(digits_then_letters), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 123);
            REQUIRE(r.ptr == begin(digits_then_letters) + 3);
        }

        THEN("Negative then invalid") {
            int val = 0;
            auto r = from_chars(begin(neg_then_letters), end(neg_then_letters), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -45);
            REQUIRE(r.ptr == begin(neg_then_letters) + 3);
        }

        THEN("Stop at first invalid in base") {
            int val = 0;
            auto r = from_chars(begin(oct_invalid), end(oct_invalid), val, 8); // 9 invalid in octal
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 83); // 123 in octal
            REQUIRE(r.ptr == begin(oct_invalid) + 3);
        }
    }

    WHEN("Minus sign edge cases") {
        THEN("Only minus") {
            int val = 99;
            auto r = from_chars(begin(only_minus), end(only_minus), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Multiple minus") {
            int val = 99;
            auto r = from_chars(begin(multi_minus), end(multi_minus), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Minus in middle") {
            int val = 0;
            auto r = from_chars(begin(minus_middle), end(minus_middle), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 12);
            REQUIRE(r.ptr == begin(minus_middle) + 2);
        }
    }
}

SCENARIO("Integer from_chars - Overflow detection exhaustive", "[from_chars][integer][overflow]") {
    using utility::begin;
    using utility::end;

    // Signed values
    constexpr string_view sc_1000 = "1000";
    constexpr string_view sc_neg1000 = "-1000";
    constexpr string_view sc_max = "127";
    constexpr string_view sc_over = "128";

    // Unsigned values
    constexpr string_view uc_256 = "256";
    constexpr string_view uc_max = "255";
    constexpr string_view us_large = "999999999";

    // Base-specific overflows
    constexpr string_view bin_256 = "100000000"; // binary 256
    constexpr string_view hex_256 = "100"; // hex 256
    constexpr string_view base36_ovf = "zzzzzz";

    WHEN("Testing signed overflow paths") {
        THEN("Multiplication overflow in signed") {
            signed char val = 0;
            auto r = from_chars(begin(sc_1000), end(sc_1000), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Negative multiplication overflow") {
            signed char val = 0;
            auto r = from_chars(begin(sc_neg1000), end(sc_neg1000), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Just at boundary (no overflow)") {
            signed char val = 0;
            auto r = from_chars(begin(sc_max), end(sc_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 127);
        }

        THEN("One past boundary") {
            signed char val = 0;
            auto r = from_chars(begin(sc_over), end(sc_over), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Testing unsigned overflow paths") {
        THEN("Unsigned type narrower than accumulator") {
            unsigned char val = 0;
            auto r = from_chars(begin(uc_256), end(uc_256), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("At exact max") {
            unsigned char val = 0;
            auto r = from_chars(begin(uc_max), end(uc_max), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Large overflow") {
            unsigned short val = 0;
            auto r = from_chars(begin(us_large), end(us_large), val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Testing overflow in different bases") {
        THEN("Binary overflow") {
            unsigned char val = 0;
            auto r = from_chars(begin(bin_256), end(bin_256), val, 2); // 256
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Hex overflow") {
            unsigned char val = 0;
            auto r = from_chars(begin(hex_256), end(hex_256), val, 16); // 256
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Base 36 overflow") {
            unsigned short val = 0;
            auto r = from_chars(begin(base36_ovf), end(base36_ovf), val, 36);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }
}

// ============================================================================
// 浮点数 to_chars 极限测试
// ============================================================================

SCENARIO("Float to_chars - All special values", "[to_chars][float][special]") {
    char buffer[256];

    WHEN("Testing float special values") {
        THEN("Positive zero") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 0.0f);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Negative zero") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), -0.0f);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Positive infinity") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::infinity());
            REQUIRE(r.ec == std::errc{});
            std::string str(buffer, r.ptr);
            REQUIRE((str == "inf" || str == "Inf" || str == "infinity"));
        }

        THEN("Negative infinity") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), -std::numeric_limits<float>::infinity());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Quiet NaN") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::quiet_NaN());
            REQUIRE(r.ec == std::errc{});
            std::string str(buffer, r.ptr);
            REQUIRE((str.find("nan") != std::string::npos || str.find("NaN") != std::string::npos));
        }

        THEN("Signaling NaN") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::signaling_NaN());
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Testing double special values") {
        THEN("All special values for double") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<double>::infinity());
            REQUIRE(r1.ec == std::errc{});

            auto r2 = to_chars(buffer, buffer + sizeof(buffer), -std::numeric_limits<double>::infinity());
            REQUIRE(r2.ec == std::errc{});

            auto r3 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<double>::quiet_NaN());
            REQUIRE(r3.ec == std::errc{});
        }
    }

    WHEN("Testing long double special values") {
        THEN("All special values for long double") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<long double>::infinity());
            REQUIRE(r1.ec == std::errc{});

            auto r2 = to_chars(buffer, buffer + sizeof(buffer), -std::numeric_limits<long double>::infinity());
            REQUIRE(r2.ec == std::errc{});

            auto r3 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<long double>::quiet_NaN());
            REQUIRE(r3.ec == std::errc{});
        }
    }
}

SCENARIO("Float to_chars - Extreme magnitude values", "[to_chars][float][magnitude]") {
    char buffer[256];

    WHEN("Testing float boundaries") {
        THEN("Maximum float") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::max());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Minimum positive normalized float") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::min());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Minimum positive denormalized float") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::denorm_min());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Lowest (most negative) float") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::lowest());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Epsilon") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<float>::epsilon());
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Testing double boundaries") {
        THEN("Maximum double") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<double>::max());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Minimum positive double") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<double>::min());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Denormalized double") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<double>::denorm_min());
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Lowest double") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<double>::lowest());
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Testing long double boundaries") {
        THEN("All boundary values") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<long double>::max());
            REQUIRE(r1.ec == std::errc{});

            auto r2 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<long double>::min());
            REQUIRE(r2.ec == std::errc{});

            auto r3 = to_chars(buffer, buffer + sizeof(buffer), std::numeric_limits<long double>::denorm_min());
            REQUIRE(r3.ec == std::errc{});
        }
    }
}

SCENARIO("Float to_chars - All format combinations", "[to_chars][float][formats]") {
    char buffer[256];

    WHEN("Testing scientific format") {
        THEN("No precision specified") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 1.234567e20f, chars_format::scientific);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("With various precisions") {
            for (int prec = 0; prec <= 10; ++prec) {
                auto r = to_chars(buffer, buffer + sizeof(buffer), 3.14159f, chars_format::scientific, prec);
                REQUIRE(r.ec == std::errc{});
            }
        }

        THEN("Very large exponent") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 1e35f, chars_format::scientific);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Very small exponent") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 1e-35f, chars_format::scientific);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Testing fixed format") {
        THEN("No precision") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 123.456f, chars_format::fixed);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Zero precision") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 123.456f, chars_format::fixed, 0);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("High precision") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 3.14159265359f, chars_format::fixed, 15);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Small value in fixed") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 0.000001f, chars_format::fixed);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Testing hex format") {
        THEN("Various float values in hex") {
            std::array<float, 5> values = {1.0f, 2.5f, 16.75f, 0.125f, -3.14f};
            for (float val: values) {
                auto r = to_chars(buffer, buffer + sizeof(buffer), val, chars_format::hex);
                REQUIRE(r.ec == std::errc{});
            }
        }

        THEN("Hex with precision") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 3.14159f, chars_format::hex, 10);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Testing general format") {
        THEN("General chooses appropriate format") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 12345.6789f, chars_format::general);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("General with precision") {
            for (int prec = 1; prec <= 10; ++prec) {
                auto r = to_chars(buffer, buffer + sizeof(buffer), 3.14159f, chars_format::general, prec);
                REQUIRE(r.ec == std::errc{});
            }
        }
    }

    WHEN("Testing format combinations") {
        THEN("Fixed | Scientific") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 123.456f, chars_format::fixed | chars_format::scientific);
            REQUIRE(r.ec == std::errc{});
        }
    }
}

SCENARIO("Float to_chars - Precision edge cases", "[to_chars][float][precision]") {
    char buffer[256];

    WHEN("Testing extreme precisions") {
        THEN("Precision 0") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 3.14159f, chars_format::fixed, 0);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Very high precision (beyond float capability)") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 3.14159f, chars_format::fixed, 50);
            // Should handle gracefully
        }

        THEN("Negative precision (if accepted)") {
            // May not be valid, but test behavior
        }
    }

    WHEN("Testing rounding at precision boundary") {
        THEN("Values that round up") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 1.995f, chars_format::fixed, 2);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Values that round down") {
            auto r = to_chars(buffer, buffer + sizeof(buffer), 1.994f, chars_format::fixed, 2);
            REQUIRE(r.ec == std::errc{});
        }
    }
}

// ============================================================================
// 浮点数 from_chars 极限测试
// ============================================================================

SCENARIO("Float from_chars - All special value strings", "[from_chars][float][special]") {
    using utility::begin;
    using utility::end;

    // Infinity variants
    constexpr string_view inf_lc = "inf";
    constexpr string_view inf_cap = "Inf";
    constexpr string_view inf_uc = "INF";
    constexpr string_view infinity = "infinity";
    constexpr string_view neg_inf = "-inf";

    // NaN variants
    constexpr string_view nan_lc = "nan";
    constexpr string_view nan_mixed = "NaN";
    constexpr string_view nan_uc = "NAN";

    // Zero variants
    constexpr string_view zero = "0.0";
    constexpr string_view neg_zero = "-0.0";
    constexpr string_view zero_e = "0e0";

    WHEN("Parsing infinity variants") {
        THEN("inf lowercase") {
            float val = 0.0f;
            auto r = from_chars(begin(inf_lc), end(inf_lc), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
            REQUIRE(val > 0);
        }

        THEN("Inf capitalized") {
            float val = 0.0f;
            auto r = from_chars(begin(inf_cap), end(inf_cap), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
        }

        THEN("INF uppercase") {
            float val = 0.0f;
            auto r = from_chars(begin(inf_uc), end(inf_uc), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
        }

        THEN("infinity full word") {
            float val = 0.0f;
            auto r = from_chars(begin(infinity), end(infinity), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
        }

        THEN("-inf negative") {
            float val = 0.0f;
            auto r = from_chars(begin(neg_inf), end(neg_inf), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
            REQUIRE(val < 0);
        }
    }

    WHEN("Parsing NaN variants") {
        THEN("nan lowercase") {
            float val = 0.0f;
            auto r = from_chars(begin(nan_lc), end(nan_lc), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isnan(val));
        }

        THEN("NaN mixed case") {
            float val = 0.0f;
            auto r = from_chars(begin(nan_mixed), end(nan_mixed), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isnan(val));
        }

        THEN("NAN uppercase") {
            float val = 0.0f;
            auto r = from_chars(begin(nan_uc), end(nan_uc), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isnan(val));
        }
    }

    WHEN("Parsing zero variants") {
        THEN("0.0") {
            float val = 1.0f;
            auto r = from_chars(begin(zero), end(zero), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 0.0f);
        }

        THEN("-0.0") {
            float val = 1.0f;
            auto r = from_chars(begin(neg_zero), end(neg_zero), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::signbit(val));
        }

        THEN("0e0") {
            float val = 1.0f;
            auto r = from_chars(begin(zero_e), end(zero_e), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 0.0f);
        }
    }
}

SCENARIO("Float from_chars - Extreme magnitudes", "[from_chars][float][magnitude]") {
    using utility::begin;
    using utility::end;

    // Very large floats
    constexpr string_view fl_max = "3.4e38";
    constexpr string_view fl_ovf_bound = "3.5e38";
    constexpr string_view fl_def_ovf = "1e100";

    // Very small floats
    constexpr string_view fl_min_norm = "1.17549e-38";
    constexpr string_view fl_denorm = "1e-45";
    constexpr string_view fl_underflow = "1e-100";

    // Double extremes
    constexpr string_view db_max = "1.7e308";
    constexpr string_view db_ovf = "1e500";
    constexpr string_view db_denorm = "5e-324";

    WHEN("Parsing very large values") {
        THEN("Near float max") {
            float val = 0.0f;
            auto r = from_chars(begin(fl_max), end(fl_max), val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("At overflow boundary") {
            float val = 0.0f;
            auto r = from_chars(begin(fl_ovf_bound), end(fl_ovf_bound), val);
            // May overflow to infinity
        }

        THEN("Definite overflow") {
            float val = 0.0f;
            auto r = from_chars(begin(fl_def_ovf), end(fl_def_ovf), val);
            if (r.ec == std::errc{}) {
                REQUIRE(std::isinf(val));
            }
        }
    }

    WHEN("Parsing very small values") {
        THEN("Near float min normalized") {
            float val = 0.0f;
            auto r = from_chars(begin(fl_min_norm), end(fl_min_norm), val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Denormalized range") {
            float val = 0.0f;
            auto r = from_chars(begin(fl_denorm), end(fl_denorm), val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Underflow to zero") {
            float val = 1.0f;
            auto r = from_chars(begin(fl_underflow), end(fl_underflow), val);
            if (r.ec == std::errc{}) {
                REQUIRE(val == 0.0f);
            }
        }
    }

    WHEN("Parsing double extremes") {
        THEN("Near double max") {
            double val = 0.0;
            auto r = from_chars(begin(db_max), end(db_max), val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Double overflow") {
            double val = 0.0;
            auto r = from_chars(begin(db_ovf), end(db_ovf), val);
            if (r.ec == std::errc{}) {
                REQUIRE(std::isinf(val));
            }
        }

        THEN("Double denormalized") {
            double val = 0.0;
            auto r = from_chars(begin(db_denorm), end(db_denorm), val);
            REQUIRE(r.ec == std::errc{});
        }
    }
}

SCENARIO("Float from_chars - All format strings", "[from_chars][float][formats]") {
    using utility::begin;
    using utility::end;

    // Scientific notation
    constexpr string_view sci_e_lc = "1.5e10";
    constexpr string_view sci_E_uc = "1.5E10";
    constexpr string_view sci_e_plus = "1.5e+10";
    constexpr string_view sci_e_minus = "1.5e-10";
    constexpr string_view sci_no_decimal = "15e9";
    constexpr string_view sci_no_fraction = "1.e10";

    // Fixed point
    constexpr string_view int_only = "123";
    constexpr string_view frac_only = ".456";
    constexpr string_view both_parts = "123.456";
    constexpr string_view leading_zeros = "00123.45600";
    constexpr string_view neg_frac = "-123.456";

    // Hexadecimal floats
    constexpr string_view hex_lc_prefix = "0x1.8p3";
    constexpr string_view hex_uc_prefix = "0X1.8P3";
    constexpr string_view hex_digits_lc = "0x1.abcp3";
    constexpr string_view hex_digits_uc = "0x1.ABCp3";
    constexpr string_view hex_neg = "-0x1.8p3";

    WHEN("Parsing scientific notation variants") {
        THEN("Lowercase e") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_e_lc), end(sci_e_lc), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(1.5e10f, 0.001f));
        }

        THEN("Uppercase E") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_E_uc), end(sci_E_uc), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(1.5e10f, 0.001f));
        }

        THEN("Positive exponent with +") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_e_plus), end(sci_e_plus), val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Negative exponent") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_e_minus), end(sci_e_minus), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(1.5e-10f, 0.001e-10f));
        }

        THEN("No decimal point") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_no_decimal), end(sci_no_decimal), val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("No mantissa fraction") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_no_fraction), end(sci_no_fraction), val);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Parsing fixed point variants") {
        THEN("Integer part only") {
            float val = 0.0f;
            auto r = from_chars(begin(int_only), end(int_only), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 123.0f);
        }

        THEN("Fraction part only") {
            float val = 0.0f;
            auto r = from_chars(begin(frac_only), end(frac_only), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(0.456f, 0.001f));
        }

        THEN("Both parts") {
            float val = 0.0f;
            auto r = from_chars(begin(both_parts), end(both_parts), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(123.456f, 0.001f));
        }

        THEN("Leading zeros") {
            float val = 0.0f;
            auto r = from_chars(begin(leading_zeros), end(leading_zeros), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(123.456f, 0.001f));
        }

        THEN("Negative with fraction") {
            float val = 0.0f;
            auto r = from_chars(begin(neg_frac), end(neg_frac), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(-123.456f, 0.001f));
        }
    }

    WHEN("Parsing hexadecimal float") {
        THEN("0x prefix lowercase") {
            float val = 0.0f;
            auto r = from_chars(begin(hex_lc_prefix), end(hex_lc_prefix), val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(12.0f, 0.001f));
        }

        THEN("0X prefix uppercase") {
            float val = 0.0f;
            auto r = from_chars(begin(hex_uc_prefix), end(hex_uc_prefix), val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Hex digits A-F lowercase") {
            float val = 0.0f;
            auto r = from_chars(begin(hex_digits_lc), end(hex_digits_lc), val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Hex digits A-F uppercase") {
            float val = 0.0f;
            auto r = from_chars(begin(hex_digits_uc), end(hex_digits_uc), val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Negative hex float") {
            float val = 0.0f;
            auto r = from_chars(begin(hex_neg), end(hex_neg), val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val < 0);
        }
    }
}

SCENARIO("Float from_chars - Invalid input exhaustive", "[from_chars][float][invalid]") {
    using utility::begin;
    using utility::end;

    constexpr string_view empty = "";
    constexpr string_view ws3 = "   ";
    constexpr string_view alpha = "abc";
    constexpr string_view special = "@#$";
    constexpr string_view num_then_text = "12.5abc";
    constexpr string_view sci_then_text = "1.5e10xyz";
    constexpr string_view malformed_e = "1.5e";
    constexpr string_view multi_e = "1.5e10e5";
    constexpr string_view multi_dot = "1.2.3";

    WHEN("Empty or whitespace") {
        THEN("Empty string") {
            float val = 99.0f;
            auto r = from_chars(begin(empty), end(empty), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Only whitespace") {
            float val = 99.0f;
            auto r = from_chars(begin(ws3), end(ws3), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Invalid characters") {
        THEN("Pure alphabetic") {
            float val = 99.0f;
            auto r = from_chars(begin(alpha), end(alpha), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Special characters") {
            float val = 99.0f;
            auto r = from_chars(begin(special), end(special), val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Partial valid input") {
        THEN("Number then text") {
            float val = 0.0f;
            auto r = from_chars(begin(num_then_text), end(num_then_text), val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(12.5f, 0.001f));
            REQUIRE(r.ptr == begin(num_then_text) + 4);
        }

        THEN("Scientific then invalid") {
            float val = 0.0f;
            auto r = from_chars(begin(sci_then_text), end(sci_then_text), val);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Malformed scientific notation") {
        THEN("e without exponent") {
            float val = 0.0f;
            auto r = from_chars(begin(malformed_e), end(malformed_e), val);
            // Behavior depends on implementation
        }

        THEN("Multiple e's") {
            float val = 0.0f;
            auto r = from_chars(begin(multi_e), end(multi_e), val);
            // Should stop at first complete valid parse
        }
    }

    WHEN("Multiple decimal points") {
        THEN("Two dots") {
            float val = 0.0f;
            auto r = from_chars(begin(multi_dot), end(multi_dot), val);
            if (r.ec == std::errc{}) {
                REQUIRE(r.ptr < end(multi_dot));
            }
        }
    }
}

// ============================================================================
// 往返测试
// ============================================================================

SCENARIO("Round-trip conversion - Integer extremes", "[roundtrip][integer]") {
    char buffer[256];

    WHEN("Round-tripping all integer types at boundaries") {
        THEN("signed char boundaries") {
            constexpr signed char orig1 = std::numeric_limits<signed char>::min();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig1);
            signed char parsed1 = 0;
            auto r2 = from_chars(buffer, r1.ptr, parsed1);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(parsed1 == orig1);

            constexpr signed char orig2 = (std::numeric_limits<signed char>::max)();
            r1 = to_chars(buffer, buffer + sizeof(buffer), orig2);
            parsed1 = 0;
            r2 = from_chars(buffer, r1.ptr, parsed1);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(parsed1 == orig2);
        }

        THEN("unsigned long long max") {
            constexpr unsigned long long orig = std::numeric_limits<unsigned long long>::max();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig);
            unsigned long long parsed = 0;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(parsed == orig);
        }

        THEN("signed long long min/max") {
            constexpr long long orig_min = std::numeric_limits<long long>::min();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig_min);
            long long parsed = 0;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(parsed == orig_min);

            constexpr long long orig_max = std::numeric_limits<long long>::max();
            r1 = to_chars(buffer, buffer + sizeof(buffer), orig_max);
            parsed = 0;
            r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(parsed == orig_max);
        }
    }

    WHEN("Round-tripping in all bases") {
        for (int base = 2; base <= 36; ++base) {
            THEN("Base " + std::to_string(base) + " preserves value") {
                int orig = 12345;
                auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig, base);
                int parsed = 0;
                auto r2 = from_chars(buffer, r1.ptr, parsed, base);
                REQUIRE(r2.ec == std::errc{});
                REQUIRE(parsed == orig);
            }
        }
    }
}

SCENARIO("Round-trip conversion - Float extremes", "[roundtrip][float]") {
    char buffer[256];
    WHEN("Round-tripping float boundary values") {
        THEN("Float max") {
            constexpr float orig = (std::numeric_limits<float>::max)();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig, std::chars_format::scientific, 10);
            float parsed = 0.0f;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE_THAT(parsed, WithinRel(orig, 0.01f));
        }

        THEN("Float min normalized") {
            constexpr float orig = (std::numeric_limits<float>::min)();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig, chars_format::scientific, 10);
            float parsed = 0.0f;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE_THAT(parsed, WithinRel(orig, 0.01f));
        }

        THEN("Float denorm_min") {
            constexpr float orig = std::numeric_limits<float>::denorm_min();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig, chars_format::scientific, 10);
            float parsed = 0.0f;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
        }
    }

    WHEN("Round-tripping double with high precision") {
        THEN("PI with max precision") {
            double orig = 3.141592653589793238462643383279502884;
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig, chars_format::fixed, 15);
            double parsed = 0.0;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE_THAT(parsed, WithinRel(orig, 1e-14));
        }

        THEN("e with max precision") {
            double orig = 2.718281828459045235360287471352662498;
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig, chars_format::scientific, 15);
            double parsed = 0.0;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE_THAT(parsed, WithinRel(orig, 1e-14));
        }
    }

    WHEN("Round-tripping special values") {
        THEN("Infinity preserved") {
            constexpr float orig = std::numeric_limits<float>::infinity();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig);
            float parsed = 0.0f;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(std::isinf(parsed));
            REQUIRE(parsed > 0);
        }

        THEN("Negative infinity preserved") {
            constexpr float orig = -std::numeric_limits<float>::infinity();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig);
            float parsed = 0.0f;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(std::isinf(parsed));
            REQUIRE(parsed < 0);
        }

        THEN("NaN preserved") {
            constexpr float orig = std::numeric_limits<float>::quiet_NaN();
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig);
            float parsed = 0.0f;
            auto r2 = from_chars(buffer, r1.ptr, parsed);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(std::isnan(parsed));
        }
    }
}

// ============================================================================
// 压力测试和随机测试
// ============================================================================

SCENARIO("Stress test - Random values", "[stress][random]") {
    char buffer[256];

    WHEN("Testing many random integers") {
        THEN("1000 random values round-trip correctly") {
            std::srand(42); // Reproducible
            for (int i = 0; i < 1000; ++i) {
                int orig = std::rand();
                if (std::rand() % 2)
                    orig = -orig;

                auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig);
                REQUIRE(r1.ec == std::errc{});

                int parsed = 0;
                auto r2 = from_chars(buffer, r1.ptr, parsed);
                REQUIRE(r2.ec == std::errc{});
                REQUIRE(parsed == orig);
            }
        }
    }

    WHEN("Testing many random floats") {
        THEN("1000 random floats round-trip approximately") {
            std::srand(42);
            for (int i = 0; i < 1000; ++i) {
                float orig = (float) std::rand() / RAND_MAX * 1000.0f;
                if (std::rand() % 2)
                    orig = -orig;

                auto r1 = to_chars(buffer, buffer + sizeof(buffer), orig);
                REQUIRE(r1.ec == std::errc{});

                float parsed = 0.0f;
                auto r2 = from_chars(buffer, r1.ptr, parsed);
                REQUIRE(r2.ec == std::errc{});
                REQUIRE_THAT(parsed, WithinRel(orig, 0.01f));
            }
        }
    }
}

SCENARIO("Concurrency safety considerations", "[concurrency]") {
    WHEN("Multiple conversions with separate buffers") {
        THEN("Results are independent") {
            char buf1[64], buf2[64], buf3[64];

            auto r1 = to_chars(buf1, buf1 + sizeof(buf1), 123);
            auto r2 = to_chars(buf2, buf2 + sizeof(buf2), 456);
            auto r3 = to_chars(buf3, buf3 + sizeof(buf3), 789);

            REQUIRE(r1.ec == std::errc{});
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(r3.ec == std::errc{});

            REQUIRE(std::string(buf1, r1.ptr) == "123");
            REQUIRE(std::string(buf2, r2.ptr) == "456");
            REQUIRE(std::string(buf3, r3.ptr) == "789");
        }
    }
}

// ============================================================================
// 性能和行为特征测试
// ============================================================================

SCENARIO("Behavior characteristics", "[behavior]") {
    char buffer[256];

    WHEN("Testing determinism") {
        THEN("Same input always produces same output") {
            auto r1 = to_chars(buffer, buffer + sizeof(buffer), 3.14159f);
            std::string str1(buffer, r1.ptr);

            auto r2 = to_chars(buffer, buffer + sizeof(buffer), 3.14159f);
            std::string str2(buffer, r2.ptr);

            REQUIRE(str1 == str2);
        }
    }

    WHEN("Testing null-termination behavior") {
        THEN("to_chars does NOT null-terminate") {
            std::memset(buffer, 'X', sizeof(buffer));
            auto r = to_chars(buffer, buffer + 10, 42);
            // Should not write beyond r.ptr
            // Next byte should still be 'X'
            if (r.ptr < buffer + sizeof(buffer)) {
                REQUIRE(*r.ptr == 'X');
            }
        }
    }
}
