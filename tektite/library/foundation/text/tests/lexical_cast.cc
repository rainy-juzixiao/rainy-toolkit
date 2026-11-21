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
#include <cstdint>
#include <rainy/core/text/format.hpp>
#include <rainy/foundation/text/lexical_cast.hpp>
#include <string>
#include <string_view>

using rainy::core::text::basic_string;
using rainy::core::text::string;
using rainy::core::text::string_view;
using rainy::core::text::u16string;
using rainy::core::text::u32string;
using rainy::core::text::wstring;
using rainy::foundation::exceptions::logic::bad_lexical_cast;
using rainy::foundation::text::lexical_cast;
using rainy::foundation::text::lexical_to_string;
using rainy::foundation::text::try_lexical_convert;

namespace {
    template <typename Target, typename Source>
    bool rejects(const Source &source) {
        Target result{};
        return !try_lexical_convert(source, result);
    }
}

struct test_point {
    int x{};
    int y{};
};

bool operator==(const test_point &left, const test_point &right) {
    return left.x == right.x && left.y == right.y;
}

namespace rainy::foundation::text {
    template <>
    struct lexical_caster<test_point, char> {
        static constexpr bool value = true;

        static bool convert(const string_view &input, test_point &result) {
            auto separator = input.find(',');
            if (separator == string_view::npos) {
                return false;
            }
            int x = 0;
            int y = 0;
            if (!try_lexical_convert(string_view{input.data(), separator}, x) ||
                !try_lexical_convert(string_view{input.data() + separator + 1, input.size() - separator - 1}, y)) {
                return false;
            }
            result = test_point{x, y};
            return true;
        }

        static basic_string<char> to_string(const test_point &value) {
            return basic_string<char>{rainy::core::text::format("{},{}", value.x, value.y)};
        }
    };
}

TEST_CASE("lexical_cast parses the whole input", "[text][lexical_cast]") {
    SECTION("a plain integer") {
        REQUIRE(lexical_cast<int>("123") == 123);
        REQUIRE(lexical_cast<int>(string_view{"123"}) == 123);
        REQUIRE(lexical_cast<int>(std::string{"123"}) == 123);
        REQUIRE(lexical_cast<int>(std::string_view{"123"}) == 123);
        REQUIRE(lexical_cast<int>(string{"123"}) == 123);
    }

    SECTION("a leading blank fails") {
        REQUIRE(rejects<int>(" 123"));
        REQUIRE(rejects<int>("\t123"));
    }

    SECTION("a trailing blank fails") {
        REQUIRE(rejects<int>("123 "));
    }

    SECTION("enclosing delimiters fail") {
        REQUIRE(rejects<int>("[123]"));
        REQUIRE(rejects<int>("{123}"));
        REQUIRE(rejects<int>("(123)"));
        REQUIRE(rejects<int>("\"123\""));
        REQUIRE(rejects<int>("'1'"));
    }

    SECTION("a trailing garbage fails") {
        REQUIRE(rejects<int>("123abc"));
        REQUIRE(rejects<int>("1.5"));
    }

    SECTION("a counted range needs no null terminator") {
        REQUIRE(lexical_cast<int>("12 34", 2) == 12);
        int result = 0;
        REQUIRE_FALSE(try_lexical_convert("xx42", 4, result));
        REQUIRE(try_lexical_convert("42xx", 2, result));
        REQUIRE(result == 42);
    }
}

TEST_CASE("lexical_cast handles the empty input", "[text][lexical_cast]") {
    REQUIRE(rejects<int>(""));
    REQUIRE(rejects<double>(""));
    REQUIRE(rejects<long double>(""));
    REQUIRE(rejects<bool>(""));
    REQUIRE(rejects<char>(""));
    REQUIRE(rejects<unsigned char>(""));

    SECTION("a string target accepts it") {
        REQUIRE(lexical_cast<string>("").empty());
        REQUIRE(lexical_cast<std::string>("").empty());
    }
}

TEST_CASE("lexical_cast handles the integer boundaries", "[text][lexical_cast]") {
    REQUIRE(lexical_cast<int>("0") == 0);
    REQUIRE(lexical_cast<int>("+1") == 1);
    REQUIRE(lexical_cast<int>("-1") == -1);
    REQUIRE(lexical_cast<int>("000123") == 123);
    REQUIRE(lexical_cast<int>("2147483647") == 2147483647);
    REQUIRE(lexical_cast<int>("-2147483648") == -2147483648);

    SECTION("an overflow fails") {
        REQUIRE(rejects<int>("2147483648"));
        REQUIRE(rejects<int>("-2147483649"));
        REQUIRE(rejects<std::int16_t>("32768"));
        REQUIRE(lexical_cast<std::int16_t>("32767") == 32767);
    }

    SECTION("a malformed sign fails") {
        REQUIRE(rejects<int>("+"));
        REQUIRE(rejects<int>("-"));
        REQUIRE(rejects<int>("++1"));
        REQUIRE(rejects<int>("+-1"));
        REQUIRE(rejects<int>("1-"));
        REQUIRE(rejects<double>("+-1"));
        REQUIRE(lexical_cast<double>("+.5") == 0.5);
    }

    SECTION("a negative value never reaches an unsigned target") {
        REQUIRE(rejects<unsigned int>("-1"));
    }

    SECTION("the unsigned range is covered") {
        REQUIRE(lexical_cast<unsigned int>("4294967295") == 4294967295u);
        REQUIRE(rejects<unsigned int>("4294967296"));
    }
}

TEST_CASE("lexical_cast handles the floating point values", "[text][lexical_cast]") {
    REQUIRE(lexical_cast<double>("0.5") == 0.5);
    REQUIRE(lexical_cast<double>("-0.5") == -0.5);
    REQUIRE(lexical_cast<double>("1e10") == 1e10);
    REQUIRE(lexical_cast<double>("1.5e10") == 1.5e10);
    REQUIRE(lexical_cast<double>("1.5E10") == 1.5e10);
    REQUIRE(lexical_cast<double>("1.5e+10") == 1.5e10);
    REQUIRE(lexical_cast<double>("1.5e-10") == 1.5e-10);
    REQUIRE(lexical_cast<float>("1.5") == 1.5f);

    SECTION("a truncated exponent fails") {
        REQUIRE(rejects<double>("1e"));
        REQUIRE(rejects<double>("1e+"));
        REQUIRE(rejects<double>("1e-"));
        REQUIRE(rejects<double>("1.0E"));
        REQUIRE(rejects<double>("1.0e-"));
    }

    SECTION("the hexadecimal form is not supported") {
        REQUIRE(rejects<double>("0x1.8p3"));
    }

    SECTION("a round trip keeps the value") {
        REQUIRE(lexical_cast<double>(lexical_to_string(0.1)) == 0.1);
        REQUIRE(lexical_cast<float>(lexical_to_string(0.1f)) == 0.1f);
        REQUIRE(lexical_cast<long double>(lexical_to_string(1.0L / 3.0L)) == 1.0L / 3.0L);
    }

    SECTION("the long double precision survives") {
        const long double precise = 1.2345678901234567890L;
        REQUIRE(lexical_cast<long double>(lexical_to_string(precise)) == precise);
#if RAINY_HAS_EXTENDED_LONG_DOUBLE
        REQUIRE(lexical_cast<long double>("1e-4932") > 0.0L);
#else
        REQUIRE(rejects<long double>("1e-4932"));
#endif
    }

    SECTION("an out of range value fails") {
        REQUIRE(rejects<double>("1e9999"));
    }
}

TEST_CASE("lexical_cast handles the infinity and the not a number", "[text][lexical_cast]") {
    REQUIRE(lexical_cast<double>("inf") > 0);
    REQUIRE(lexical_cast<double>("INF") > 0);
    REQUIRE(lexical_cast<double>("-inf") < 0);
    REQUIRE(lexical_cast<double>("-INF") < 0);
    REQUIRE(lexical_cast<double>("infinity") > 0);
    REQUIRE(lexical_cast<double>("-INFINITY") < 0);
    REQUIRE(lexical_cast<double>("+inf") > 0);

    REQUIRE(lexical_cast<double>("nan") != lexical_cast<double>("nan"));
    REQUIRE(lexical_cast<double>("NAN") != lexical_cast<double>("NAN"));
    REQUIRE(lexical_cast<double>("-nan") != lexical_cast<double>("nan"));
    REQUIRE(lexical_cast<double>("+nan") != lexical_cast<double>("nan"));
    REQUIRE(lexical_cast<double>("nan(ind)") != lexical_cast<double>("nan"));
    REQUIRE(lexical_cast<double>("nan(snan)") != lexical_cast<double>("nan"));

    SECTION("the parenthesis have to be closed") {
        REQUIRE(rejects<double>("NAN(x"));
    }

    SECTION("the output is lower case") {
        REQUIRE(lexical_to_string(lexical_cast<double>("inf")) == "inf");
        REQUIRE(lexical_to_string(lexical_cast<double>("-inf")) == "-inf");
        REQUIRE(lexical_to_string(lexical_cast<double>("nan")) == "nan");
        REQUIRE(lexical_to_string(lexical_cast<double>("-nan")).find("nan") != string::npos);
    }
}

TEST_CASE("lexical_cast accepts only zero and one for a bool", "[text][lexical_cast]") {
    REQUIRE(lexical_cast<bool>("0") == false);
    REQUIRE(lexical_cast<bool>("1") == true);
    REQUIRE(lexical_cast<bool>("00") == false);
    REQUIRE(lexical_cast<bool>("01") == true);
    REQUIRE(lexical_cast<bool>("+1") == true);
    REQUIRE(lexical_cast<bool>("-0") == false);

    REQUIRE(rejects<bool>("true"));
    REQUIRE(rejects<bool>("false"));
    REQUIRE(rejects<bool>("True"));
    REQUIRE(rejects<bool>("FALSE"));
    REQUIRE(rejects<bool>("t"));
    REQUIRE(rejects<bool>("F"));
    REQUIRE(rejects<bool>("2"));
    REQUIRE(rejects<bool>("10"));
    REQUIRE(rejects<bool>("-1"));
    REQUIRE(rejects<bool>("1 1"));

    SECTION("a bool is rendered as zero or one") {
        REQUIRE(lexical_to_string(true) == "1");
        REQUIRE(lexical_to_string(false) == "0");
        REQUIRE(lexical_cast<int>(true) == 1);
        REQUIRE(lexical_cast<double>(false) == 0.0);
        REQUIRE(lexical_cast<bool>(true));
        REQUIRE_FALSE(lexical_cast<bool>(false));
    }
}

TEST_CASE("lexical_cast handles the character targets", "[text][lexical_cast]") {
    REQUIRE(lexical_cast<char>("a") == 'a');
    REQUIRE(lexical_cast<char>(L"a") == 'a');
    REQUIRE(lexical_cast<wchar_t>("a") == L'a');
    REQUIRE(lexical_cast<char32_t>(rainy::core::text::u32string_view{U"a"}) == U'a');
    REQUIRE(lexical_cast<unsigned char>("\x7f") == 0x7f);
    REQUIRE(lexical_cast<signed char>("\x7f") == 0x7f);

    SECTION("a single character source is accepted") {
        REQUIRE(lexical_cast<int>('7') == 7);
        REQUIRE(lexical_cast<int>(L'7') == 7);
        REQUIRE(lexical_cast<string>('a') == "a");
        REQUIRE(lexical_cast<bool>('1'));
    }

    SECTION("more than one code unit fails") {
        REQUIRE(rejects<char>("ab"));
        REQUIRE(rejects<char>(""));
        REQUIRE(rejects<char>(L"ab"));
    }

    SECTION("the narrow characters are characters, not numbers") {
        REQUIRE(rejects<std::int8_t>("127"));
        REQUIRE(rejects<std::uint8_t>("255"));
        REQUIRE(rejects<char>("127"));
    }

    SECTION("a multi digit number is not a single character") {
        REQUIRE(rejects<char>(53));
        REQUIRE(rejects<wchar_t>(53));
        REQUIRE(lexical_cast<char>(5) == '5');
    }
}

TEST_CASE("lexical_cast converts between the character types", "[text][lexical_cast]") {
    const string utf8 = "héllo";
    const auto wide = lexical_cast<wstring>(string_view{utf8.data(), utf8.size()});
    REQUIRE(wide.size() == 5);
    REQUIRE(wide[1] == static_cast<wchar_t>(0xe9));
    REQUIRE(lexical_cast<string>(wide) == utf8);

    const auto u16 = lexical_cast<u16string>(wide);
    REQUIRE(u16.size() == 5);
    REQUIRE(u16[1] == static_cast<char16_t>(0xe9));
    REQUIRE(lexical_cast<wstring>(u16) == wide);

    const auto u32 = lexical_cast<u32string>(wide);
    REQUIRE(u32.size() == 5);
    REQUIRE(u32[1] == static_cast<char32_t>(0xe9));
    REQUIRE(lexical_cast<u32string>(u16) == u32);

    SECTION("the same character type needs no conversion") {
        REQUIRE(lexical_cast<string>(string{"abc"}) == "abc");
        REQUIRE(lexical_cast<wstring>(wstring{L"abc"}) == L"abc");
    }

    SECTION("a view target cannot own a conversion") {
        REQUIRE(rejects<string_view>(wide));
    }

    SECTION("the single byte character types are interchangeable") {
#if RAINY_HAS_CXX20
        const rainy::core::text::u8string bytes{rainy::core::text::u8string_view{u8"ab"}};
        REQUIRE(lexical_to_string(bytes) == "ab");
        REQUIRE(lexical_cast<rainy::core::text::u8string>("ab") == bytes);
#endif
    }

    SECTION("the standard library strings are supported") {
        REQUIRE(lexical_cast<std::string>(string{"abc"}) == "abc");
        REQUIRE(lexical_cast<std::string_view>(std::string{"abc"}) == "abc");
        REQUIRE(lexical_cast<std::wstring>(std::wstring{L"abc"}) == L"abc");
        REQUIRE(lexical_cast<std::string>(L"abc") == "abc");
    }
}

TEST_CASE("lexical_cast converts between the arithmetic values", "[text][lexical_cast]") {
    REQUIRE(lexical_cast<int>(7) == 7);
    REQUIRE(lexical_cast<int>(7.0) == 7);
    REQUIRE(lexical_cast<double>(7) == 7.0);
    REQUIRE(lexical_cast<short>(7) == 7);
    REQUIRE(lexical_cast<unsigned>(-1) == 4294967295u);

    SECTION("a lossy conversion fails") {
        REQUIRE(rejects<int>(3.7));
        REQUIRE(rejects<int>(3.0e300));
        REQUIRE(rejects<std::int8_t>(1000));
        REQUIRE(rejects<float>(0.1));
    }

    SECTION("a floating point value that fits the integral range is kept") {
        REQUIRE(lexical_cast<int>(1073741824.0) == 1073741824);
        REQUIRE(lexical_cast<int>(2000000000.0) == 2000000000);
        REQUIRE(lexical_cast<int>(2147483647.0) == 2147483647);
        REQUIRE(lexical_cast<int>(-2147483648.0) == -2147483648);
        REQUIRE(lexical_cast<unsigned>(4294967295.0) == 4294967295u);
        REQUIRE(rejects<int>(2147483648.0));
        REQUIRE(rejects<unsigned>(-1.0));
    }

    SECTION("a value is rendered as text") {
        REQUIRE(lexical_to_string(42) == "42");
        REQUIRE(lexical_to_string(-42) == "-42");
        REQUIRE(lexical_to_string(3.5) == "3.5");
        REQUIRE(lexical_to_string(0.1) == "0.1");
        REQUIRE(lexical_cast<string>(42) == "42");
        REQUIRE(lexical_cast<std::string>(42) == "42");
    }
}

TEST_CASE("lexical_cast is exported through rainy::text", "[text][lexical_cast]") {
    REQUIRE(rainy::text::lexical_cast<int>("42") == 42);
    REQUIRE(rainy::text::lexical_to_string(3.5) == "3.5");

    int result = 0;
    REQUIRE(rainy::text::try_lexical_convert("7", result));
    REQUIRE(result == 7);
}

TEST_CASE("lexical_cast throws on a failure", "[text][lexical_cast]") {
    REQUIRE_THROWS_AS(lexical_cast<int>("abc"), bad_lexical_cast);
    REQUIRE_THROWS_AS(lexical_cast<int>(" 123"), bad_lexical_cast);
    REQUIRE_THROWS_AS(lexical_cast<bool>("true"), bad_lexical_cast);
    REQUIRE_THROWS_AS(lexical_cast<int8_t>("127"), bad_lexical_cast);
}

TEST_CASE("try_lexical_convert reports instead of throwing", "[text][lexical_cast]") {
    SECTION("a success writes the value") {
        int result = 0;
        REQUIRE(try_lexical_convert("123", result));
        REQUIRE(result == 123);
    }

    SECTION("a failure returns false") {
        int result = 123;
        REQUIRE_FALSE(try_lexical_convert("abc", result));
    }

    SECTION("the non owning targets keep the source alive") {
        const string source{"abc"};
        string_view result;
        REQUIRE(try_lexical_convert(source, result));
        REQUIRE(result == "abc");
    }

    SECTION("a value that needs a buffer never reaches a view target") {
        string_view result;
        REQUIRE_FALSE(try_lexical_convert(42, result));
        REQUIRE_FALSE(try_lexical_convert(true, result));
    }
}

TEST_CASE("lexical_cast serves the user defined types", "[text][lexical_cast]") {
    REQUIRE(rainy::foundation::text::has_lexical_caster_v<test_point, char>);

    SECTION("the text is read into the type") {
        const test_point point = lexical_cast<test_point>("1,2");
        REQUIRE(point.x == 1);
        REQUIRE(point.y == 2);
    }

    SECTION("a malformed text fails") {
        REQUIRE(rejects<test_point>("1;2"));
        REQUIRE(rejects<test_point>("1"));
        REQUIRE(rejects<test_point>("a,2"));
        REQUIRE_THROWS_AS(lexical_cast<test_point>("1;2"), bad_lexical_cast);
    }

    SECTION("the type is rendered as text") {
        REQUIRE(lexical_to_string(test_point{3, 4}) == "3,4");
        REQUIRE(lexical_cast<string>(test_point{3, 4}) == "3,4");
        REQUIRE(rejects<string_view>(test_point{3, 4}));
    }

    SECTION("a round trip keeps the value") {
        REQUIRE(lexical_cast<test_point>(lexical_to_string(test_point{7, -8})) == test_point{7, -8});
    }
}
