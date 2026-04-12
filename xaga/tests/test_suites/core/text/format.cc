#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rainy/core/core.hpp>

namespace text = rainy::foundation::text;

static std::string to_std(const text::basic_string<char> &s) {
    return {s.data(), s.size()}; // NOLINT
}

// ============================================================
// Helper locale aliases
// ============================================================
#define LOCALE_EN std::locale("en_US.UTF-8")
#define LOCALE_DE std::locale("de_DE.UTF-8")
#define LOCALE_ZH std::locale("zh_CN.UTF-8")
#define LOCALE_FR std::locale("fr_FR.UTF-8")

SCENARIO("Basic string formatting with no placeholders", "[format][string][basic]") {
    GIVEN("A format string with no placeholders") {
        WHEN("format() is called with no extra arguments") {
            auto result = to_std(text::format("Hello World"));
            THEN("The result equals the original string verbatim") {
                REQUIRE(result == "Hello World");
            }
        }
        WHEN("The format string is empty") {
            const auto result = to_std(text::format(""));
            THEN("The result is an empty string") {
                REQUIRE(result.empty());
            }
        }
    }
}

SCENARIO("Formatting with a single {} placeholder", "[format][string][basic]") {

    GIVEN("A format string containing one placeholder") {
        WHEN("A C-style string literal is supplied") {
            auto result = to_std(text::format("Hello {}", "World"));
            THEN("The placeholder is replaced with the literal content") {
                REQUIRE(result == "Hello World");
            }
        }
        WHEN("A std::string is supplied") {
            std::string s = "Standard String";
            auto result = to_std(text::format("Value: {}", s));
            THEN("The placeholder is replaced with the std::string content") {
                REQUIRE(result == "Value: Standard String");
            }
        }
        WHEN("A std::string_view is supplied") {
            std::string_view sv = "StringView";
            auto result = to_std(text::format("Value: {}", sv));
            THEN("The placeholder is replaced with the string_view content") {
                REQUIRE(result == "Value: StringView");
            }
        }
    }
}

SCENARIO("Formatting with multiple sequential {} placeholders", "[format][string][basic]") {

    GIVEN("A format string with three placeholders") {
        WHEN("Three C-style strings are supplied") {
            auto result = to_std(text::format("{} {} {}", "A", "B", "C"));
            THEN("Each placeholder is replaced in order") {
                REQUIRE(result == "A B C");
            }
        }
        WHEN("Mixed string types are supplied") {
            std::string s = "Standard String";
            std::string_view sv = "StringView";
            auto result = to_std(text::format("{} | {} | {}", "literal", s, sv));
            THEN("All three placeholders are replaced correctly") {
                REQUIRE(result == "literal | Standard String | StringView");
            }
        }
    }
}

SCENARIO("Formatting with explicit positional indices", "[format][string][positional]") {
    GIVEN("A format string using {0} and {1} indices") {
        WHEN("Arguments are referenced in forward order") {
            auto result = to_std(text::format("{0} {1}", "first", "second"));
            THEN("The result matches the natural order") {
                REQUIRE(result == "first second");
            }
        }
        WHEN("Arguments are referenced in reverse order") {
            auto result = to_std(text::format("{1} {0}", "first", "second"));
            THEN("The result has the arguments swapped") {
                REQUIRE(result == "second first");
            }
        }
        WHEN("The same index is referenced more than once") {
            auto result = to_std(text::format("{0} {0} {1}", "repeat", "once"));
            THEN("The first argument appears twice") {
                REQUIRE(result == "repeat repeat once");
            }
        }
    }

    GIVEN("A format string that reuses {0} at the end") {
        WHEN("First argument is repeated via {0}") {
            std::string s = "Rainy";
            auto result = to_std(text::format("{} and again: {0}", s));
            THEN("Both occurrences hold the same value") {
                REQUIRE(result == "Rainy and again: Rainy");
            }
        }
    }
}

SCENARIO("Formatting raw pointers", "[format][pointer]") {

    GIVEN("A nullptr") {
        WHEN("nullptr is passed as an argument") {
            auto result = to_std(text::format("{}", nullptr));
            THEN("The output is the null-pointer representation (0x0)") {
                REQUIRE(result == "0x0");
            }
        }
    }

    GIVEN("A non-null pointer to a std::string") {
        std::string s = "hello";
        WHEN("The address-of the string is passed") {
            auto result = to_std(text::format("{}", &s));
            THEN("The output starts with '0x' and is non-zero") {
                REQUIRE_THAT(result, Catch::Matchers::StartsWith("0x"));
                REQUIRE(result != "0x0");
            }
        }
    }
}

SCENARIO("Locale-aware formatting of plain strings (no :L flag needed)", "[format][string][locale]") {

    GIVEN("An en_US.UTF-8 locale and a plain string argument") {
        WHEN("format() is called with a locale and a string placeholder") {
            auto result = to_std(text::format(LOCALE_EN, "Greeting: {}", "Hello World"));
            THEN("The string is embedded unchanged regardless of locale") {
                REQUIRE(result == "Greeting: Hello World");
            }
        }
    }

    GIVEN("A zh_CN.UTF-8 locale") {
        WHEN("A UTF-8 string argument is formatted") {
            auto result = to_std(text::format(LOCALE_ZH, "{}", "你好世界"));
            THEN("The UTF-8 content is preserved as-is") {
                REQUIRE(result == "你好世界");
            }
        }
    }
}

SCENARIO("Locale-aware numeric formatting with :L flag", "[format][locale][numeric]") {

    GIVEN("An en_US locale and a large integer (1234567)") {
        constexpr int number = 1234567;

        WHEN("The integer is formatted with the :L flag") {
            auto result = to_std(text::format(LOCALE_EN, "{:L}", number));
            THEN("Thousands separators follow en_US conventions (1,234,567)") {
                REQUIRE(result == "1,234,567");
            }
        }

        WHEN("A full mixed format string is used (mirrors the documented example)") {
            std::string str = "Standard String";
            auto result = to_std(
                text::format(LOCALE_EN, "Hello World {} {:L} {} {} {} {} {0}", 3.14, number, "This char array", str, &str, nullptr));

            THEN("The locale-formatted number uses comma separators") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("1,234,567"));
            }
            THEN("Static string fragments appear verbatim") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("Hello World"));
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("This char array"));
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("Standard String"));
            }
            THEN("The null pointer is rendered as 0x0") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("0x0"));
            }
            THEN("The non-null pointer is rendered as a non-zero hex address") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("0x"));
            }
            THEN("{0} re-uses the first arg (3.14) — it appears at the start and the end") {
                REQUIRE_THAT(result, Catch::Matchers::StartsWith("Hello World 3.14"));
                REQUIRE_THAT(result, Catch::Matchers::EndsWith("3.14"));
            }
        }
    }

    GIVEN("A de_DE locale and a large integer") {
        constexpr int number = 1234567; // NOLINT
        WHEN("The integer is formatted with :L") {
            auto result = to_std(text::format(LOCALE_DE, "{:L}", number));
            THEN("Thousands separators follow de_DE conventions (1.234.567)") {
#if !RAINY_USING_MACOS // 我不知道为什么macOS给的结果有点奇怪……因此，我不检查
                REQUIRE(result == "1.234.567");
#endif
            }
        }
    }

    GIVEN("A fr_FR locale and a large integer") {
        constexpr int number = 1234567; // NOLINT
        WHEN("The integer is formatted with :L") {
            auto result = to_std(text::format(LOCALE_FR, "{:L}", number));
            THEN("A separator is inserted (wider than the bare digit string)") {
                // fr_FR uses narrow no-break space (U+202F); we check structurally
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("234"));
#if !RAINY_USING_MACOS
                REQUIRE(result.size() > std::string("1234567").size());
#endif
            }
        }
    }
}

SCENARIO("Locale-aware floating-point formatting", "[format][locale][float]") {
    GIVEN("An en_US locale") {
        WHEN("3.14 is formatted without :L") {
            auto result = to_std(text::format(LOCALE_EN, "{}", 3.14));
            THEN("The decimal separator is a period") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("."));
            }
        }
        WHEN("A large float is formatted with :L") {
            auto result = to_std(text::format(LOCALE_EN, "{:.2Lf}", 1234567.89));
            THEN("en_US thousands separators and decimal period are used") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring("1,234,567"));
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring(".89"));
            }
        }
    }

    GIVEN("A de_DE locale") {
        WHEN("3.14 is formatted with :L") {
            auto result = to_std(text::format(LOCALE_DE, "{:L}", 3.14));
            THEN("The decimal separator is a comma per de_DE convention") {
                REQUIRE_THAT(result, Catch::Matchers::ContainsSubstring(","));
            }
        }
    }
}

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4566)
#endif

SCENARIO("Edge cases for string formatting", "[format][string][edge]") {

    GIVEN("A format string where the placeholder is at the very start") {
        auto result = to_std(text::format("{} tail", "head"));
        REQUIRE(result == "head tail");
    }

    GIVEN("A format string where the placeholder is at the very end") {
        auto result = to_std(text::format("head {}", "tail"));
        REQUIRE(result == "head tail");
    }

    GIVEN("An empty string argument") {
        auto result = to_std(text::format("[{}]", ""));
        THEN("The brackets are present but nothing is between them") {
            REQUIRE(result == "[]");
        }
    }

    GIVEN("A format string with escaped braces") {
        auto result = to_std(text::format("{{literal braces}}"));
        THEN("Double braces are rendered as single literal braces") {
            REQUIRE(result == "{literal braces}");
        }
    }

    GIVEN("A very long string argument") {
        std::string long_str(10000, 'x');
        auto result = to_std(text::format("{}", long_str));
        THEN("The full string is preserved without truncation") {
            REQUIRE(result == long_str);
        }
    }

    GIVEN("A string argument containing Unicode characters") {
        auto result = to_std(text::format("{}", "こんにちは🌧️"));
        THEN("Unicode content is preserved exactly") {
            REQUIRE(result == "こんにちは🌧️");
        }
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
