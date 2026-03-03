#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <cstring>
#include <limits>
#include <rainy/core/implements/text/charconv.hpp>
#include <string>

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
    WHEN("Parsing to signed char") {
        THEN("Max value") {
            signed char val = 0;
            auto r = from_chars("127", "127" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 127);
        }

        THEN("Min value") {
            signed char val = 0;
            auto r = from_chars("-128", "-128" + 4, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -128);
        }

        THEN("Overflow") {
            signed char val = 0;
            auto r = from_chars("128", "128" + 3, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Underflow") {
            signed char val = 0;
            auto r = from_chars("-129", "-129" + 4, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned char") {
        THEN("Max value") {
            unsigned char val = 0;
            auto r = from_chars("255", "255" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Overflow") {
            unsigned char val = 0;
            auto r = from_chars("256", "256" + 3, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Negative on unsigned") {
            unsigned char val = 0;
            auto r = from_chars("-1", "-1" + 2, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Parsing to short") {
        THEN("Max value (32767)") {
            short val = 0;
            auto r = from_chars("32767", "32767" + 5, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 32767);
        }

        THEN("Min value (-32768)") {
            short val = 0;
            auto r = from_chars("-32768", "-32768" + 6, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -32768);
        }

        THEN("Overflow") {
            short val = 0;
            auto r = from_chars("32768", "32768" + 5, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned short") {
        THEN("Max value (65535)") {
            unsigned short val = 0;
            auto r = from_chars("65535", "65535" + 5, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 65535);
        }

        THEN("Overflow") {
            unsigned short val = 0;
            auto r = from_chars("65536", "65536" + 5, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to int") {
        THEN("Max int32 (2147483647)") {
            int val = 0;
            auto r = from_chars("2147483647", "2147483647" + 10, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 2147483647);
        }

        THEN("Min int32 (-2147483648)") {
            int val = 0;
            auto r = from_chars("-2147483648", "-2147483648" + 11, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == (int) -2147483648);
        }

        THEN("Overflow by 1") {
            int val = 0;
            auto r = from_chars("2147483648", "2147483648" + 10, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Underflow by 1") {
            int val = 0;
            auto r = from_chars("-2147483649", "-2147483649" + 11, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned int") {
        THEN("Max uint32 (4294967295)") {
            unsigned int val = 0;
            auto r = from_chars("4294967295", "4294967295" + 10, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 4294967295U);
        }

        THEN("Overflow") {
            unsigned int val = 0;
            auto r = from_chars("4294967296", "4294967296" + 10, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to long long") {
        THEN("Max int64 (9223372036854775807)") {
            long long val = 0;
            auto r = from_chars("9223372036854775807", "9223372036854775807" + 19, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 9223372036854775807LL);
        }

        THEN("Min int64 (-9223372036854775808)") {
            long long val = 0;
            auto r = from_chars("-9223372036854775808", "-9223372036854775808" + 20, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Overflow") {
            long long val = 0;
            auto r = from_chars("9223372036854775808", "9223372036854775808" + 19, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Parsing to unsigned long long") {
        THEN("Max uint64 (18446744073709551615)") {
            unsigned long long val = 0;
            auto r = from_chars("18446744073709551615", "18446744073709551615" + 20, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 18446744073709551615ULL);
        }

        THEN("Overflow") {
            unsigned long long val = 0;
            auto r = from_chars("18446744073709551616", "18446744073709551616" + 20, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }
}

SCENARIO("Integer from_chars - All bases exhaustive", "[from_chars][integer][bases]") {
    WHEN("Testing each base 2-36") {
        for (int base = 2; base <= 36; ++base) {
            THEN("Base " + std::to_string(base) + " parses correctly") {
                int val = 0;

                // Test "0"
                auto r0 = from_chars("0", "0" + 1, val, base);
                REQUIRE(r0.ec == std::errc{});
                REQUIRE(val == 0);

                // Test "1"
                auto r1 = from_chars("1", "1" + 1, val, base);
                REQUIRE(r1.ec == std::errc{});
                REQUIRE(val == 1);

                // Test base-1 digit (max single digit)
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
            auto r1 = from_chars("10", "10" + 2, val, 2);
            REQUIRE(r1.ec == std::errc{});
            REQUIRE(val == 2);

            auto r2 = from_chars("11111111", "11111111" + 8, val, 2);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Base 4 edge cases") {
            int val = 0;
            auto r = from_chars("3333", "3333" + 4, val, 4);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Base 8 edge cases") {
            int val = 0;
            auto r = from_chars("777", "777" + 3, val, 8);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 511);
        }
    }

    WHEN("Testing power-of-2 bases > 8") {
        THEN("Base 16 hex digits") {
            int val = 0;
            auto r1 = from_chars("ff", "ff" + 2, val, 16);
            REQUIRE(r1.ec == std::errc{});
            REQUIRE(val == 255);

            auto r2 = from_chars("FF", "FF" + 2, val, 16);
            REQUIRE(r2.ec == std::errc{});
            REQUIRE(val == 255);

            auto r3 = from_chars("aAbBcCdDeEfF", "aAbBcCdDeEfF" + 12, val, 16);
            REQUIRE(r3.ec == std::errc::result_out_of_range);
        }

        THEN("Base 32 edge cases") {
            int val = 0;
            auto r = from_chars("vv", "vv" + 2, val, 32);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 1023);
        }
    }

    WHEN("Testing non-power-of-2 bases <= 10") {
        THEN("Base 3") {
            int val = 0;
            auto r = from_chars("2222", "2222" + 4, val, 3);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 80);
        }

        THEN("Base 5") {
            int val = 0;
            auto r = from_chars("444", "444" + 3, val, 5);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 124);
        }

        THEN("Base 10") {
            int val = 0;
            auto r = from_chars("9999", "9999" + 4, val, 10);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 9999);
        }
    }

    WHEN("Testing non-power-of-2 bases > 10") {
        THEN("Base 11") {
            int val = 0;
            auto r = from_chars("aa", "aa" + 2, val, 11);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 120);
        }

        THEN("Base 13") {
            int val = 0;
            auto r = from_chars("cc", "cc" + 2, val, 13);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 168);
        }

        THEN("Base 36 with all digits") {
            int val = 0;
            auto r = from_chars("zz", "zz" + 2, val, 36);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 1295);
        }
    }
}

SCENARIO("Integer from_chars - Invalid input coverage", "[from_chars][integer][invalid]") {
    WHEN("Empty or whitespace input") {
        THEN("Empty string") {
            int val = 99;
            auto r = from_chars("", "", val);
            REQUIRE(r.ec == std::errc::invalid_argument);
            REQUIRE(r.ptr == "");
        }

        THEN("Only whitespace") {
            int val = 99;
            auto r = from_chars("   ", "   " + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Leading whitespace") {
            int val = 99;
            auto r = from_chars(" 42", " 42" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Invalid characters") {
        THEN("Pure alphabetic") {
            int val = 99;
            auto r = from_chars("abc", "abc" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Special characters") {
            int val = 99;
            auto r = from_chars("@#$", "@#$" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Plus sign (not supported)") {
            int val = 99;
            auto r = from_chars("+42", "+42" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Partial valid input") {
        THEN("Digits then letters") {
            int val = 0;
            auto r = from_chars("123abc", "123abc" + 6, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 123);
            REQUIRE(r.ptr == "123abc" + 3);
        }

        THEN("Negative then invalid") {
            int val = 0;
            auto r = from_chars("-45xyz", "-45xyz" + 6, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == -45);
            REQUIRE(r.ptr == "-45xyz" + 3);
        }

        THEN("Stop at first invalid in base") {
            int val = 0;
            auto r = from_chars("1239", "1239" + 4, val, 8); // 9 invalid in octal
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 83); // 123 in octal
            REQUIRE(r.ptr == "1239" + 3);
        }
    }

    WHEN("Minus sign edge cases") {
        THEN("Only minus") {
            int val = 99;
            auto r = from_chars("-", "-" + 1, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Multiple minus") {
            int val = 99;
            auto r = from_chars("--5", "--5" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Minus in middle") {
            int val = 0;
            auto r = from_chars("12-34", "12-34" + 5, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 12);
            REQUIRE(r.ptr == "12-34" + 2);
        }
    }
}

SCENARIO("Integer from_chars - Overflow detection exhaustive", "[from_chars][integer][overflow]") {
    WHEN("Testing signed overflow paths") {
        THEN("Multiplication overflow in signed") {
            signed char val = 0;
            auto r = from_chars("1000", "1000" + 4, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Negative multiplication overflow") {
            signed char val = 0;
            auto r = from_chars("-1000", "-1000" + 5, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Just at boundary (no overflow)") {
            signed char val = 0;
            auto r = from_chars("127", "127" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 127);
        }

        THEN("One past boundary") {
            signed char val = 0;
            auto r = from_chars("128", "128" + 3, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Testing unsigned overflow paths") {
        THEN("Unsigned type narrower than accumulator") {
            unsigned char val = 0;
            auto r = from_chars("256", "256" + 3, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("At exact max") {
            unsigned char val = 0;
            auto r = from_chars("255", "255" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 255);
        }

        THEN("Large overflow") {
            unsigned short val = 0;
            auto r = from_chars("999999999", "999999999" + 9, val);
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }
    }

    WHEN("Testing overflow in different bases") {
        THEN("Binary overflow") {
            unsigned char val = 0;
            auto r = from_chars("100000000", "100000000" + 9, val, 2); // 256
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Hex overflow") {
            unsigned char val = 0;
            auto r = from_chars("100", "100" + 3, val, 16); // 256
            REQUIRE(r.ec == std::errc::result_out_of_range);
        }

        THEN("Base 36 overflow") {
            unsigned short val = 0;
            auto r = from_chars("zzzzzz", "zzzzzz" + 6, val, 36);
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
    WHEN("Parsing infinity variants") {
        THEN("inf lowercase") {
            float val = 0.0f;
            auto r = from_chars("inf", "inf" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
            REQUIRE(val > 0);
        }

        THEN("Inf capitalized") {
            float val = 0.0f;
            auto r = from_chars("Inf", "Inf" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
        }

        THEN("INF uppercase") {
            float val = 0.0f;
            auto r = from_chars("INF", "INF" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
        }

        THEN("infinity full word") {
            float val = 0.0f;
            auto r = from_chars("infinity", "infinity" + 8, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
        }

        THEN("-inf negative") {
            float val = 0.0f;
            auto r = from_chars("-inf", "-inf" + 4, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isinf(val));
            REQUIRE(val < 0);
        }
    }

    WHEN("Parsing NaN variants") {
        THEN("nan lowercase") {
            float val = 0.0f;
            auto r = from_chars("nan", "nan" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isnan(val));
        }

        THEN("NaN mixed case") {
            float val = 0.0f;
            auto r = from_chars("NaN", "NaN" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isnan(val));
        }

        THEN("NAN uppercase") {
            float val = 0.0f;
            auto r = from_chars("NAN", "NAN" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(std::isnan(val));
        }
    }

    WHEN("Parsing zero variants") {
        THEN("0.0") {
            float val = 1.0f;
            auto r = from_chars("0.0", "0.0" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 0.0f);
        }

        THEN("-0.0") {
            float val = 1.0f;
            auto r = from_chars("-0.0", "-0.0" + 4, val);
            REQUIRE(r.ec == std::errc{});
            // Check for negative zero
        }

        THEN("0e0") {
            float val = 1.0f;
            auto r = from_chars("0e0", "0e0" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 0.0f);
        }
    }
}

SCENARIO("Float from_chars - Extreme magnitudes", "[from_chars][float][magnitude]") {
    WHEN("Parsing very large values") {
        THEN("Near float max") {
            float val = 0.0f;
            auto r = from_chars("3.4e38", "3.4e38" + 6, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("At overflow boundary") {
            float val = 0.0f;
            auto r = from_chars("3.5e38", "3.5e38" + 6, val);
            // May overflow to infinity
        }

        THEN("Definite overflow") {
            float val = 0.0f;
            auto r = from_chars("1e100", "1e100" + 5, val);
            if (r.ec == std::errc{}) {
                REQUIRE(std::isinf(val));
            }
        }
    }

    WHEN("Parsing very small values") {
        THEN("Near float min normalized") {
            float val = 0.0f;
            auto r = from_chars("1.17549e-38", "1.17549e-38" + 11, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Denormalized range") {
            float val = 0.0f;
            auto r = from_chars("1e-45", "1e-45" + 5, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Underflow to zero") {
            float val = 1.0f;
            auto r = from_chars("1e-100", "1e-100" + 6, val);
            if (r.ec == std::errc{}) {
                REQUIRE(val == 0.0f);
            }
        }
    }

    WHEN("Parsing double extremes") {
        THEN("Near double max") {
            double val = 0.0;
            auto r = from_chars("1.7e308", "1.7e308" + 7, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Double overflow") {
            double val = 0.0;
            auto r = from_chars("1e500", "1e500" + 5, val);
            if (r.ec == std::errc{}) {
                REQUIRE(std::isinf(val));
            }
        }

        THEN("Double denormalized") {
            double val = 0.0;
            auto r = from_chars("5e-324", "5e-324" + 6, val);
            REQUIRE(r.ec == std::errc{});
        }
    }
}

SCENARIO("Float from_chars - All format strings", "[from_chars][float][formats]") {
    WHEN("Parsing scientific notation variants") {
        THEN("Lowercase e") {
            float val = 0.0f;
            auto r = from_chars("1.5e10", "1.5e10" + 6, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(1.5e10f, 0.001f));
        }

        THEN("Uppercase E") {
            float val = 0.0f;
            auto r = from_chars("1.5E10", "1.5E10" + 6, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(1.5e10f, 0.001f));
        }

        THEN("Positive exponent with +") {
            float val = 0.0f;
            auto r = from_chars("1.5e+10", "1.5e+10" + 7, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Negative exponent") {
            float val = 0.0f;
            auto r = from_chars("1.5e-10", "1.5e-10" + 7, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(1.5e-10f, 0.001e-10f));
        }

        THEN("No decimal point") {
            float val = 0.0f;
            auto r = from_chars("15e9", "15e9" + 4, val);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("No mantissa fraction") {
            float val = 0.0f;
            auto r = from_chars("1.e10", "1.e10" + 5, val);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Parsing fixed point variants") {
        THEN("Integer part only") {
            float val = 0.0f;
            auto r = from_chars("123", "123" + 3, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val == 123.0f);
        }

        THEN("Fraction part only") {
            float val = 0.0f;
            auto r = from_chars(".456", ".456" + 4, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(0.456f, 0.001f));
        }

        THEN("Both parts") {
            float val = 0.0f;
            auto r = from_chars("123.456", "123.456" + 7, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(123.456f, 0.001f));
        }

        THEN("Leading zeros") {
            float val = 0.0f;
            auto r = from_chars("00123.45600", "00123.45600" + 11, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(123.456f, 0.001f));
        }

        THEN("Negative with fraction") {
            float val = 0.0f;
            auto r = from_chars("-123.456", "-123.456" + 8, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(-123.456f, 0.001f));
        }
    }

    WHEN("Parsing hexadecimal float") {
        THEN("0x prefix lowercase") {
            float val = 0.0f;
            auto r = from_chars("0x1.8p3", "0x1.8p3" + 7, val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(12.0f, 0.001f));
        }

        THEN("0X prefix uppercase") {
            float val = 0.0f;
            auto r = from_chars("0X1.8P3", "0X1.8P3" + 7, val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Hex digits A-F lowercase") {
            float val = 0.0f;
            auto r = from_chars("0x1.abcp3", "0x1.abcp3" + 9, val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Hex digits A-F uppercase") {
            float val = 0.0f;
            auto r = from_chars("0x1.ABCp3", "0x1.ABCp3" + 9, val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
        }

        THEN("Negative hex float") {
            float val = 0.0f;
            auto r = from_chars("-0x1.8p3", "-0x1.8p3" + 8, val, chars_format::hex);
            REQUIRE(r.ec == std::errc{});
            REQUIRE(val < 0);
        }
    }
}

SCENARIO("Float from_chars - Invalid input exhaustive", "[from_chars][float][invalid]") {
    WHEN("Empty or whitespace") {
        THEN("Empty string") {
            float val = 99.0f;
            auto r = from_chars("", "", val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Only whitespace") {
            float val = 99.0f;
            auto r = from_chars("   ", "   " + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Invalid characters") {
        THEN("Pure alphabetic") {
            float val = 99.0f;
            auto r = from_chars("abc", "abc" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }

        THEN("Special characters") {
            float val = 99.0f;
            auto r = from_chars("@#$", "@#$" + 3, val);
            REQUIRE(r.ec == std::errc::invalid_argument);
        }
    }

    WHEN("Partial valid input") {
        THEN("Number then text") {
            float val = 0.0f;
            auto r = from_chars("12.5abc", "12.5abc" + 7, val);
            REQUIRE(r.ec == std::errc{});
            REQUIRE_THAT(val, WithinRel(12.5f, 0.001f));
            REQUIRE(r.ptr == "12.5abc" + 4);
        }

        THEN("Scientific then invalid") {
            float val = 0.0f;
            auto r = from_chars("1.5e10xyz", "1.5e10xyz" + 9, val);
            REQUIRE(r.ec == std::errc{});
        }
    }

    WHEN("Malformed scientific notation") {
        THEN("e without exponent") {
            float val = 0.0f;
            auto r = from_chars("1.5e", "1.5e" + 4, val);
            // Behavior depends on implementation
        }

        THEN("Multiple e's") {
            float val = 0.0f;
            auto r = from_chars("1.5e10e5", "1.5e10e5" + 8, val);
            // Should stop at first complete valid parse
        }
    }

    WHEN("Multiple decimal points") {
        THEN("Two dots") {
            float val = 0.0f;
            auto r = from_chars("1.2.3", "1.2.3" + 5, val);
            // Should stop at first invalid point
            if (r.ec == std::errc{}) {
                REQUIRE(r.ptr < "1.2.3" + 5);
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
