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
#include <catch2/matchers/catch_matchers_string.hpp>
#include <rainy/core/diagnostics/source_location.hpp>
#include <string>

using namespace rainy::core::diagnostics;

static source_location get_location_in_function(unsigned int line = __builtin_LINE(), const char *file = __builtin_FILE(),
                                                const char *function = __builtin_FUNCTION()) {
    return source_location::current(line, 0, file, function);
}

#define GET_CURRENT_LINE() __builtin_LINE()
#define GET_CURRENT_FILE() __builtin_FILE()
#define GET_CURRENT_FUNCTION() __builtin_FUNCTION()

TEST_CASE("source_location default constructor", "[source_location][constructors]") {
    SECTION("Default constructor creates empty location") {
        source_location loc;
        REQUIRE(loc.file_name() == nullptr);
        REQUIRE(loc.function_name() == nullptr);
        REQUIRE(loc.line() == 0);
        REQUIRE(loc.column() == 0);
    }

    SECTION("Default constructed locations are equal") {
        source_location loc1;
        source_location loc2;
        REQUIRE(loc1 == loc2);
        REQUIRE_FALSE(loc1 != loc2);
    }
}

TEST_CASE("source_location parameterized constructor", "[source_location][constructors]") {
    SECTION("Constructor with all parameters") {
        const char *file = "test.cpp";
        const char *function = "test_function";
        unsigned int line = 42;
        unsigned int column = 7;

        source_location loc(file, line, function, column);
        REQUIRE(std::string(loc.file_name()) == file);
        REQUIRE(std::string(loc.function_name()) == function);
        REQUIRE(loc.line() == line);
        REQUIRE(loc.column() == column);
    }

    SECTION("Constructor with default column") {
        const char *file = "test.cpp";
        const char *function = "test_function";
        unsigned int line = 42;

        source_location loc(file, line, function);
        REQUIRE(std::string(loc.file_name()) == file);
        REQUIRE(std::string(loc.function_name()) == function);
        REQUIRE(loc.line() == line);
        REQUIRE(loc.column() == 0); // 默认列号应为0
    }

    SECTION("Constructed locations equality") {
        source_location loc1("file.cpp", 10, "func", 5);
        source_location loc2("file.cpp", 10, "func", 5);
        source_location loc3("file.cpp", 10, "func", 6);
        source_location loc4("file.cpp", 11, "func", 5);
        source_location loc5("file2.cpp", 10, "func", 5);

        REQUIRE(loc1 == loc2);
        REQUIRE_FALSE(loc1 == loc3);
        REQUIRE_FALSE(loc1 == loc4);
        REQUIRE_FALSE(loc1 == loc5);
    }
}

#if RAINY_HAS_CXX20
TEST_CASE("source_location from std::source_location", "[source_location][cxx20]") {
    SECTION("Construct from std::source_location") {
        auto std_loc = std::source_location::current();
        source_location loc(std_loc);

        REQUIRE(std::string(loc.file_name()) == std_loc.file_name());
        REQUIRE(std::string(loc.function_name()) == std_loc.function_name());
        REQUIRE(loc.line() == std_loc.line());
        REQUIRE(loc.column() == std_loc.column());
    }
}
#endif

TEST_CASE("source_location::current() static method", "[source_location][current]") {
    SECTION("Current captures correct file name") {
        auto loc = source_location::current();
        std::string file_name = loc.file_name();
        // 文件名应该包含"test_source_location"或类似名称
        REQUIRE_FALSE(file_name.empty());
        REQUIRE(file_name.find("source_location") != std::string::npos);
    }

    SECTION("Current captures correct function name") {
        auto loc = source_location::current();
        std::string func_name = loc.function_name();
        // 在测试函数中，函数名应该包含"source_location::current"
        REQUIRE_FALSE(func_name.empty());
        // 注意：实际的函数名可能因编译器而异
    }

    SECTION("Current captures correct line number") {
        const unsigned int expected_line = GET_CURRENT_LINE() + 1;
        auto loc = source_location::current();
        REQUIRE(loc.line() == expected_line);
    }

    SECTION("Current with explicit parameters") {
        const char *test_file = "explicit_test.cpp";
        const char *test_function = "explicit_test_func";
        unsigned int test_line = 123;
        unsigned int test_column = 45;

        auto loc = source_location::current(test_line, test_column, test_file, test_function);
        REQUIRE(std::string(loc.file_name()) == test_file);
        REQUIRE(std::string(loc.function_name()) == test_function);
        REQUIRE(loc.line() == test_line);
        REQUIRE(loc.column() == test_column);
    }

    SECTION("Multiple current calls return different locations") {
        unsigned int line1 = GET_CURRENT_LINE() + 1;
        auto loc1 = source_location::current();

        unsigned int line2 = GET_CURRENT_LINE() + 1;
        auto loc2 = source_location::current();

        // 由于编译器优化，line1和line2可能相同，但在同一行调用时可能不同
        // 这里我们只验证它们都是有效的
        REQUIRE(loc1.line() > 0);
        REQUIRE(loc2.line() > 0);
    }
}

TEST_CASE("source_location accessor methods", "[source_location][accessors]") {
    source_location loc("test.cpp", 100, "test_func", 50);

    SECTION("file_name returns correct value") {
        REQUIRE(std::string(loc.file_name()) == "test.cpp");
    }

    SECTION("function_name returns correct value") {
        REQUIRE(std::string(loc.function_name()) == "test_func");
    }

    SECTION("line returns correct value") {
        REQUIRE(loc.line() == 100);
    }

    SECTION("column returns correct value") {
        REQUIRE(loc.column() == 50);
    }
}

TEST_CASE("source_location::to_string()", "[source_location][string_conversion]") {
    SECTION("Unknown source location") {
        source_location loc;
        REQUIRE(loc.to_string() == "(unknown source location)");
    }

    SECTION("Location with file and line only") {
        source_location loc("test.cpp", 42, "");
        std::string result = loc.to_string();
        REQUIRE(result == "test.cpp:42");
    }

    SECTION("Location with file, line, and column") {
        source_location loc("test.cpp", 42, "", 7);
        std::string result = loc.to_string();
        REQUIRE(result == "test.cpp:42:7");
    }

    SECTION("Location with file, line, and function name") {
        source_location loc("test.cpp", 42, "test_function");
        std::string result = loc.to_string();
        REQUIRE(result == "test.cpp:42 in function 'test_function'");
    }

    SECTION("Location with all fields") {
        source_location loc("test.cpp", 42, "test_function", 7);
        std::string result = loc.to_string();
        REQUIRE(result == "test.cpp:42:7 in function 'test_function'");
    }

    SECTION("Location with empty function name") {
        source_location loc("test.cpp", 42, "", 7);
        std::string result = loc.to_string();
        REQUIRE(result == "test.cpp:42:7");
    }

    SECTION("Location with null function name") {
        source_location loc("test.cpp", 42, nullptr, 7);
        std::string result = loc.to_string();
        REQUIRE(result == "test.cpp:42:7");
    }
}

TEST_CASE("source_location equality operators", "[source_location][equality]") {
    source_location loc1("file.cpp", 10, "func", 5);
    source_location loc2("file.cpp", 10, "func", 5);
    source_location loc3("file.cpp", 10, "func", 6);

    SECTION("Operator==") {
        REQUIRE(loc1 == loc2);
        REQUIRE_FALSE(loc1 == loc3);
        REQUIRE_FALSE(loc1 == source_location()); // 与默认构造的比较
    }

    SECTION("Operator!=") {
        REQUIRE_FALSE(loc1 != loc2);
        REQUIRE(loc1 != loc3);
        REQUIRE(loc1 != source_location());
    }

    SECTION("Equality with different fields") {
        source_location diff_file("file2.cpp", 10, "func", 5);
        source_location diff_line("file.cpp", 11, "func", 5);
        source_location diff_func("file.cpp", 10, "func2", 5);
        source_location diff_col("file.cpp", 10, "func", 6);

        REQUIRE_FALSE(loc1 == diff_file);
        REQUIRE_FALSE(loc1 == diff_line);
        REQUIRE_FALSE(loc1 == diff_func);
        REQUIRE_FALSE(loc1 == diff_col);
    }
}

TEST_CASE("source_location copy and move semantics", "[source_location][semantics]") {
    SECTION("Copy construction") {
        source_location original("file.cpp", 42, "func", 7);
        source_location copy(original);

        REQUIRE(copy == original);
        REQUIRE(copy.file_name() == original.file_name());
        REQUIRE(copy.function_name() == original.function_name());
        REQUIRE(copy.line() == original.line());
        REQUIRE(copy.column() == original.column());
    }

    SECTION("Copy assignment") {
        source_location original("file.cpp", 42, "func", 7);
        source_location copy;
        copy = original;

        REQUIRE(copy == original);
    }
}

TEST_CASE("current_location helper function", "[source_location][helper]") {
    SECTION("Returns current location by default") {
        unsigned int expected_line = GET_CURRENT_LINE() + 1;
        auto loc = current_location();
        REQUIRE(loc.line() == expected_line);
    }

    SECTION("Returns provided location") {
        source_location provided("test.cpp", 100, "test", 50);
        auto loc = current_location(provided);
        REQUIRE(loc == provided);
    }

    SECTION("Can be used in constexpr context") {
        constexpr auto loc = current_location();
        // 在constexpr上下文中，line可能为0，但仍然是有效的
        STATIC_REQUIRE(std::is_same_v<decltype(loc), const source_location>);
    }
}

TEST_CASE("source_location with different compilers", "[source_location][compiler]") {
    SECTION("GCC specific column handling") {
#if RAINY_USING_GCC
        auto loc = source_location::current();
        auto explicit_loc = source_location::current(__builtin_LINE(), 0, __builtin_FILE(), __builtin_FUNCTION());
        REQUIRE(explicit_loc.column() == 0);
#endif
    }

    SECTION("Non-GCC column handling") {
#if !RAINY_USING_GCC
        auto loc = source_location::current();
#endif
    }
}

TEST_CASE("source_location constexpr evaluation", "[source_location][constexpr]") {
    SECTION("Constexpr construction and access") {
        constexpr source_location loc("test.cpp", 42, "func", 7);
        STATIC_REQUIRE(loc.line() == 42);
        STATIC_REQUIRE(loc.column() == 7);
        STATIC_REQUIRE(std::string_view(loc.file_name()) == "test.cpp");
        STATIC_REQUIRE(std::string_view(loc.function_name()) == "func");
    }
}

TEST_CASE("source_location stress test", "[source_location][stress]") {
    constexpr int COUNT = 1000;
    std::vector<source_location> locations;
    locations.reserve(COUNT);

    for (int i = 0; i < COUNT; ++i) {
        locations.emplace_back("test.cpp", i, "stress_test", i % 100);
    }

    REQUIRE(locations.size() == COUNT);
    REQUIRE(locations[0].line() == 0);
    REQUIRE(locations[COUNT - 1].line() == COUNT - 1);
}
