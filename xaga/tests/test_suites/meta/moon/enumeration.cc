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
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rainy/meta/moon/enumeration.hpp>

using namespace rainy::meta::moon;

enum class Color {
    Red,
    Green,
    Blue,
    Yellow,
    Purple
};

enum class Numbers : int {
    Zero = 0,
    One = 1,
    Two = 2,
    Ten = 10,
    Hundred = 100
};

enum class Weekday {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

enum SimpleEnum {
    SIMPLE_A,
    SIMPLE_B,
    SIMPLE_C
};

enum EmptyEnum {
};

enum class ScopedEmptyEnum {
};

enum class FlagsEnum {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    Execute = 1 << 2,
    All = Read | Write | Execute
};

// 带间隙的枚举
enum class GappedEnum : int {
    First = 0,
    Second = 5,
    Third = 10,
    Fourth = 20
};

enum class DuplicateEnum {
    A = 0,
    B = 0,
    C = 1
};

enum class SignedEnum : int {
    Negative = -10,
    Zero = 0,
    Positive = 10
};

TEST_CASE("enum_count", "[enumeration][enum_count]") {
    SECTION("scoped enums") {
        REQUIRE(enum_count<Color>() == 5);
        REQUIRE(enum_count<Numbers>() == 5);
        REQUIRE(enum_count<Weekday>() == 7);
        REQUIRE(enum_count<FlagsEnum>() == 5);
    }

    SECTION("unscoped enums") {
        REQUIRE(enum_count<SimpleEnum>() == 3);
    }

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
    SECTION("empty enums") {
        REQUIRE(enum_count<EmptyEnum>() == 0);
        REQUIRE(enum_count<ScopedEmptyEnum>() == 0);
    }
#endif

    SECTION("gapped enums") {
        REQUIRE(enum_count<GappedEnum>() == 4);
    }

    SECTION("duplicate values") {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        REQUIRE(enum_count<DuplicateEnum>() == 3);
#else
        // 重复值只应被计数一次，在C++17回退版本中
        REQUIRE(enum_count<DuplicateEnum>() == 2);
#endif
    }

    SECTION("signed enums") {
        REQUIRE(enum_count<SignedEnum>() == 3);
    }
}

TEST_CASE("enum_values", "[enumeration][enum_values]") {
    SECTION("basic enum values") {
        constexpr auto values = enum_values<Color>();
        REQUIRE(values.size() == 5);
        REQUIRE(values[0] == Color::Red);
        REQUIRE(values[1] == Color::Green);
        REQUIRE(values[2] == Color::Blue);
        REQUIRE(values[3] == Color::Yellow);
        REQUIRE(values[4] == Color::Purple);
    }

    SECTION("numbered enum values") {
        constexpr auto values = enum_values<Numbers>();
        REQUIRE(values.size() == 5);
        REQUIRE(values[0] == Numbers::Zero);
        REQUIRE(values[1] == Numbers::One);
        REQUIRE(values[2] == Numbers::Two);
        REQUIRE(values[3] == Numbers::Ten);
        REQUIRE(values[4] == Numbers::Hundred);
    }

    SECTION("gapped enum values") {
        constexpr auto values = enum_values<GappedEnum>();
        REQUIRE(values.size() == 4);
        REQUIRE(values[0] == GappedEnum::First);
        REQUIRE(values[1] == GappedEnum::Second);
        REQUIRE(values[2] == GappedEnum::Third);
        REQUIRE(values[3] == GappedEnum::Fourth);
    }

    SECTION("empty enum values") {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        constexpr auto values = enum_values<EmptyEnum>();
        REQUIRE(values.size() == 0);
#endif
    }
}

TEST_CASE("enum_value_by_index", "[enumeration][enum_value]") {
    SECTION("compile-time index") {
        constexpr auto red = enum_value<Color, 0>();
        constexpr auto green = enum_value<Color, 1>();
        constexpr auto blue = enum_value<Color, 2>();

        REQUIRE(red == Color::Red);
        REQUIRE(green == Color::Green);
        REQUIRE(blue == Color::Blue);
    }

    SECTION("runtime index") {
        REQUIRE(enum_value<Color>(0) == Color::Red);
        REQUIRE(enum_value<Color>(1) == Color::Green);
        REQUIRE(enum_value<Color>(2) == Color::Blue);
        REQUIRE(enum_value<Color>(3) == Color::Yellow);
        REQUIRE(enum_value<Color>(4) == Color::Purple);
    }

    SECTION("out of bounds - undefined behavior") {
        // 注意：越界访问会导致未定义行为，这里不测试
    }
}

TEST_CASE("enum_names", "[enumeration][enum_names]") {
    SECTION("scoped enum names") {
        constexpr auto names = enum_names<Color>();
        REQUIRE(names.size() == 5);
        REQUIRE(names[0] == "Red");
        REQUIRE(names[1] == "Green");
        REQUIRE(names[2] == "Blue");
        REQUIRE(names[3] == "Yellow");
        REQUIRE(names[4] == "Purple");
    }

    SECTION("numbered enum names") {
        constexpr auto names = enum_names<Numbers>();
        REQUIRE(names.size() == 5);
        REQUIRE(names[0] == "Zero");
        REQUIRE(names[1] == "One");
        REQUIRE(names[2] == "Two");
        REQUIRE(names[3] == "Ten");
        REQUIRE(names[4] == "Hundred");
    }

    SECTION("unscoped enum names") {
        constexpr auto names = enum_names<SimpleEnum>();
        REQUIRE(names.size() == 3);
        REQUIRE(names[0] == "SIMPLE_A");
        REQUIRE(names[1] == "SIMPLE_B");
        REQUIRE(names[2] == "SIMPLE_C");
    }

    SECTION("empty enum names") {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        constexpr auto names = enum_names<EmptyEnum>();
        REQUIRE(names.size() == 0);
#endif
    }
}

TEST_CASE("enum_entries", "[enumeration][enum_entries]") {
    SECTION("entries pair") {
        constexpr auto entries = enum_entries<Color>();
        REQUIRE(entries.size() == 5);

        REQUIRE(entries[0].first == Color::Red);
        REQUIRE(entries[0].second == "Red");

        REQUIRE(entries[1].first == Color::Green);
        REQUIRE(entries[1].second == "Green");

        REQUIRE(entries[2].first == Color::Blue);
        REQUIRE(entries[2].second == "Blue");
    }

    SECTION("modify through entries is constexpr") {
        constexpr auto entries = enum_entries<Numbers>();
        static_assert(entries[0].first == Numbers::Zero);
        static_assert(entries[0].second == "Zero");
        static_assert(entries[4].first == Numbers::Hundred);
        static_assert(entries[4].second == "Hundred");
    }
}

TEST_CASE("enum_name", "[enumeration][enum_name]") {
    SECTION("get name from enum value") {
        REQUIRE(enum_name(Color::Red) == "Red");
        REQUIRE(enum_name(Color::Green) == "Green");
        REQUIRE(enum_name(Color::Blue) == "Blue");
        REQUIRE(enum_name(Numbers::Ten) == "Ten");
        REQUIRE(enum_name(Numbers::Hundred) == "Hundred");
    }

    SECTION("compile-time enum name") {
        REQUIRE(enum_name<Color, Color::Red>() == "Red");
        REQUIRE(enum_name<Color, Color::Green>() == "Green");
        REQUIRE(enum_name<Numbers, Numbers::Zero>() == "Zero");
    }

    SECTION("invalid enum value returns empty") {
        // 注意：无效的枚举值可能未定义行为
        // 这里假设函数可以安全处理
    }
}

TEST_CASE("enum_cast", "[enumeration][enum_cast]") {
    SECTION("cast from string to enum") {
        auto red = enum_cast<Color>("Red");
        REQUIRE(red.has_value());
        REQUIRE(red.value() == Color::Red);

        auto green = enum_cast<Color>("Green");
        REQUIRE(green.has_value());
        REQUIRE(green.value() == Color::Green);

        auto invalid = enum_cast<Color>("InvalidColor");
        REQUIRE_FALSE(invalid.has_value());
    }

    SECTION("cast from string with custom predicate") {
        auto red = enum_cast<Color>("red", [](char a, char b) { return std::tolower(a) == std::tolower(b); });
        REQUIRE(red.has_value());
        REQUIRE(red.value() == Color::Red);

        auto green = enum_cast<Color>("GREEN", [](char a, char b) { return std::tolower(a) == std::tolower(b); });
        REQUIRE(green.has_value());
        REQUIRE(green.value() == Color::Green);
    }

    SECTION("cast from underlying value") {
        auto zero = enum_cast<Numbers>(0);
        REQUIRE(zero.has_value());
        REQUIRE(zero.value() == Numbers::Zero);

        auto one = enum_cast<Numbers>(1);
        REQUIRE(one.has_value());
        REQUIRE(one.value() == Numbers::One);

        auto ten = enum_cast<Numbers>(10);
        REQUIRE(ten.has_value());
        REQUIRE(ten.value() == Numbers::Ten);

        auto invalid = enum_cast<Numbers>(999);
        REQUIRE_FALSE(invalid.has_value());
    }

    SECTION("cast for unscoped enum") {
        auto a = enum_cast<SimpleEnum>("SIMPLE_A");
        REQUIRE(a.has_value());
        REQUIRE(a.value() == SIMPLE_A);
    }
}

TEST_CASE("enum_integer", "[enumeration][enum_integer]") {
    SECTION("convert enum to underlying integer") {
        REQUIRE(enum_integer(Color::Red) == 0);
        REQUIRE(enum_integer(Color::Green) == 1);
        REQUIRE(enum_integer(Color::Blue) == 2);

        REQUIRE(enum_integer(Numbers::Zero) == 0);
        REQUIRE(enum_integer(Numbers::One) == 1);
        REQUIRE(enum_integer(Numbers::Two) == 2);
        REQUIRE(enum_integer(Numbers::Ten) == 10);
        REQUIRE(enum_integer(Numbers::Hundred) == 100);
    }
}

TEST_CASE("enum_underlying", "[enumeration][enum_underlying]") {
    SECTION("enum_underlying is alias for enum_integer") {
        REQUIRE(enum_underlying(Color::Red) == enum_integer(Color::Red));
        REQUIRE(enum_underlying(Numbers::Ten) == enum_integer(Numbers::Ten));
    }
}

TEST_CASE("enum_index", "[enumeration][enum_index]") {
    SECTION("get index of enum value") {
        auto idx_red = enum_index(Color::Red);
        REQUIRE(idx_red.has_value());
        REQUIRE(idx_red.value() == 0);

        auto idx_green = enum_index(Color::Green);
        REQUIRE(idx_green.has_value());
        REQUIRE(idx_green.value() == 1);

        auto idx_blue = enum_index(Color::Blue);
        REQUIRE(idx_blue.has_value());
        REQUIRE(idx_blue.value() == 2);

        auto idx_purple = enum_index(Color::Purple);
        REQUIRE(idx_purple.has_value());
        REQUIRE(idx_purple.value() == 4);
    }

    SECTION("compile-time index") {
        constexpr auto idx = enum_index<Color, Color::Red>();
        REQUIRE(idx.has_value());
        REQUIRE(idx.value() == 0);

        constexpr auto idx_green = enum_index<Color, Color::Green>();
        REQUIRE(idx_green.has_value());
        REQUIRE(idx_green.value() == 1);
    }

    SECTION("index for numbers enum") {
        auto idx_zero = enum_index(Numbers::Zero);
        REQUIRE(idx_zero.has_value());
        REQUIRE(idx_zero.value() == 0);

        auto idx_hundred = enum_index(Numbers::Hundred);
        REQUIRE(idx_hundred.has_value());
        REQUIRE(idx_hundred.value() == 4);
    }
}

TEST_CASE("enum_contains", "[enumeration][enum_contains]") {
    SECTION("check enum contains value") {
        REQUIRE(enum_contains(Color::Red));
        REQUIRE(enum_contains(Color::Green));
        REQUIRE(enum_contains(Color::Blue));
        REQUIRE(enum_contains(Color::Yellow));
        REQUIRE(enum_contains(Color::Purple));
    }

    SECTION("compile-time contains check") {
        REQUIRE(enum_contains<Color, Color::Red>());
        REQUIRE(enum_contains<Color, Color::Green>());
        REQUIRE(enum_contains<Numbers, Numbers::Zero>());
        REQUIRE(enum_contains<Numbers, Numbers::Hundred>());
    }

    SECTION("check underlying value") {
        REQUIRE(enum_contains<Numbers>(0));
        REQUIRE(enum_contains<Numbers>(1));
        REQUIRE(enum_contains<Numbers>(2));
        REQUIRE(enum_contains<Numbers>(10));
        REQUIRE(enum_contains<Numbers>(100));
        REQUIRE_FALSE(enum_contains<Numbers>(999));
    }

    SECTION("check string name") {
        REQUIRE(enum_contains<Color>("Red"));
        REQUIRE(enum_contains<Color>("Green"));
        REQUIRE(enum_contains<Color>("Blue"));
        REQUIRE_FALSE(enum_contains<Color>("Cyan"));
    }

    SECTION("check string name with predicate") {
        REQUIRE(enum_contains<Color>("RED", [](char a, char b) { return std::tolower(a) == std::tolower(b); }));
        REQUIRE(enum_contains<Color>("BLUE", [](char a, char b) { return std::tolower(a) == std::tolower(b); }));
    }
}

TEST_CASE("enum_flags_name", "[enumeration][enum_flags_name]") {
    SECTION("single flag") {
        std::string name = enum_flags_name(FlagsEnum::Read);
        REQUIRE(name == "Read");

        name = enum_flags_name(FlagsEnum::Write);
        REQUIRE(name == "Write");

        name = enum_flags_name(FlagsEnum::Execute);
        REQUIRE(name == "Execute");
    }

    SECTION("combined flags") {
        FlagsEnum combined = static_cast<FlagsEnum>(static_cast<int>(FlagsEnum::Read) | static_cast<int>(FlagsEnum::Write));
        std::string name = enum_flags_name(combined);
        REQUIRE(name == "Read|Write");

        FlagsEnum all = FlagsEnum::All;
        std::string all_name = enum_flags_name(all);
        REQUIRE(all_name == "Read|Write|Execute");
    }

    SECTION("custom separator") {
        FlagsEnum combined = static_cast<FlagsEnum>(static_cast<int>(FlagsEnum::Read) | static_cast<int>(FlagsEnum::Execute));
        std::string name = enum_flags_name(combined, ',');
        REQUIRE(name == "Read,Execute");

        std::string name_with_space = enum_flags_name(combined, ' ');
        REQUIRE(name_with_space == "Read Execute");
    }

    SECTION("empty flags return empty string") {
        std::string name = enum_flags_name(FlagsEnum::None);
        REQUIRE(name.empty());
    }

    SECTION("invalid flag combination") {
        // 无效的位组合应返回空字符串
        FlagsEnum invalid = static_cast<FlagsEnum>(0x10);
        std::string name = enum_flags_name(invalid);
        REQUIRE(name.empty());
    }
}

TEST_CASE("is_enum_value", "[enumeration][is_enum_value]") {
    SECTION("check if value is valid enum") {
        REQUIRE(is_enum_value_v<Color, static_cast<int>(Color::Red)>);
        REQUIRE(is_enum_value_v<Color, static_cast<int>(Color::Green)>);
        REQUIRE(is_enum_value_v<Numbers, 0>);
        REQUIRE(is_enum_value_v<Numbers, 1>);
        REQUIRE(is_enum_value_v<Numbers, 2>);
        REQUIRE(is_enum_value_v<Numbers, 10>);
        REQUIRE(is_enum_value_v<Numbers, 100>);
    }

    SECTION("invalid values") {
        // 无效的底层值不应被视为有效枚举值
        // 注意：这依赖于具体的实现
    }

    SECTION("type trait") {
        REQUIRE(is_enum_value<Color, static_cast<int>(Color::Red)>::value);
        REQUIRE(is_enum_value<Numbers, 0>::value);
    }
}

TEST_CASE("enum_type_name", "[enumeration][enum_type_name]") {
    SECTION("get enum type name") {
        auto name = enum_type_name<Color>();
        REQUIRE_FALSE(name.empty());
        REQUIRE(name.find("Color") != std::string_view::npos);

        auto numbers_name = enum_type_name<Numbers>();
        REQUIRE_FALSE(numbers_name.empty());
        REQUIRE(numbers_name.find("Numbers") != std::string_view::npos);
    }

    SECTION("empty enum type name") {
        // 对于空枚举，类型名仍应返回
        auto name = enum_type_name<EmptyEnum>();
        REQUIRE_FALSE(name.empty());
    }
}

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

enum class AnnotatedEnum {
    First[[= rainy::annotations::moon::rename("renamed_first")]],
    Second[[= rainy::annotations::moon::rename("renamed_second")]],
    Third[[= rainy::annotations::moon::with_prefix("pre_")]],
    Fourth[[= rainy::annotations::moon::with_suffix("_suf")]],
    Ignored[[= rainy::annotations::moon::ignore]],
    Normal
};

enum class[[ = rainy::annotations::moon::with_prefix("enum_"), = rainy::annotations::moon::with_suffix("_type") ]] TypeAnnotatedEnum {
    Value1,
    Value2
};

TEST_CASE("C++26 static reflection - enum annotations", "[enumeration][cxx26][annotations]") {
    SECTION("rename annotation") {
        constexpr auto names = enum_names<AnnotatedEnum>();
        REQUIRE(names.size() == 5); // Ignored 被忽略

        bool found_renamed_first = false;
        bool found_renamed_second = false;
        for (const auto &name: names) {
            if (name == "renamed_first") {
                found_renamed_first = true;
            }
            if (name == "renamed_second") {
                found_renamed_second = true;
            }
        }
        REQUIRE(found_renamed_first);
        REQUIRE(found_renamed_second);
    }

    SECTION("prefix and suffix annotations") {
        constexpr auto names = enum_names<AnnotatedEnum>();

        bool found_prefixed = false;
        bool found_suffixed = false;
        for (const auto &name: names) {
            if (name.find("pre_") != std::string_view::npos) {
                found_prefixed = true;
            }
            if (name.find("_suf") != std::string_view::npos) {
                found_suffixed = true;
            }
        }
        REQUIRE(found_prefixed);
        REQUIRE(found_suffixed);
    }

    SECTION("ignore annotation") {
        constexpr auto names = enum_names<AnnotatedEnum>();
        // Ignored 不应出现在名称列表中
        for (const auto &name: names) {
            REQUIRE(name != "Ignored");
        }
    }

    SECTION("normal member without annotation") {
        constexpr auto names = enum_names<AnnotatedEnum>();
        bool found_normal = false;
        for (const auto &name: names) {
            if (name == "Normal") {
                found_normal = true;
            }
        }
        REQUIRE(found_normal);
    }

    SECTION("type-level annotations") {
        constexpr auto names = enum_names<TypeAnnotatedEnum>();
        for (const auto &name: names) {
            REQUIRE(name.find("enum_") != std::string_view::npos);
            REQUIRE(name.find("_type") != std::string_view::npos);
        }
    }
}

#endif

TEST_CASE("compile-time constexpr evaluation", "[enumeration][constexpr]") {
    SECTION("enum_count is constexpr") {
        constexpr size_t color_count = enum_count<Color>();
        constexpr size_t numbers_count = enum_count<Numbers>();
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        constexpr size_t empty_count = enum_count<EmptyEnum>();
#endif
        static_assert(color_count == 5);
        static_assert(numbers_count == 5);
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        static_assert(empty_count == 0);
#endif
    }

    SECTION("enum_values is constexpr") {
        constexpr auto values = enum_values<Color>();
        static_assert(values.size() == 5);
        static_assert(values[0] == Color::Red);
    }

    SECTION("enum_names is constexpr") {
        constexpr auto names = enum_names<Color>();
        static_assert(names.size() == 5);
        static_assert(names[0] == "Red");
    }

    SECTION("enum_entries is constexpr") {
        constexpr auto entries = enum_entries<Color>();
        static_assert(entries.size() == 5);
        static_assert(entries[0].first == Color::Red);
        static_assert(entries[0].second == "Red");
    }

    SECTION("enum_name is constexpr") {
        constexpr auto name = enum_name<Color, Color::Red>();
        static_assert(name == "Red");
    }
}

TEMPLATE_TEST_CASE("template enum operations", "[enumeration][template]", Color, Numbers, Weekday, SimpleEnum) {
    SECTION("enum_count for template type") {
        constexpr auto count = enum_count<TestType>();
        REQUIRE(count > 0);
    }

    SECTION("enum_values returns non-empty array") {
        constexpr auto values = enum_values<TestType>();
        REQUIRE(values.size() == enum_count<TestType>());
    }

    SECTION("enum_names returns non-empty array") {
        constexpr auto names = enum_names<TestType>();
        REQUIRE(names.size() == enum_count<TestType>());
    }
}

TEST_CASE("enum round-trip conversion", "[enumeration][roundtrip]") {
    SECTION("value -> name -> value") {
        for (const auto &val: enum_values<Color>()) {
            auto name = enum_name(val);
            auto roundtrip = enum_cast<Color>(name);
            REQUIRE(roundtrip.has_value());
            REQUIRE(roundtrip.value() == val);
        }
    }

    SECTION("index -> value -> index") {
        for (std::size_t i = 0; i < enum_count<Color>(); ++i) {
            auto val = enum_value<Color>(i);
            auto idx = enum_index(val);
            REQUIRE(idx.has_value());
            REQUIRE(idx.value() == i);
        }
    }

    SECTION("underlying -> value -> underlying") {
        auto values = enum_values<Numbers>();
        for (const auto &val: values) {
            auto underlying = enum_integer(val);
            auto roundtrip = enum_cast<Numbers>(underlying);
            REQUIRE(roundtrip.has_value());
            REQUIRE(roundtrip.value() == val);
        }
    }
}
