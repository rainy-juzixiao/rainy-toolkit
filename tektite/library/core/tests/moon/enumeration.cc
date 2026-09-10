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
#include <rainy/core/meta/enumeration.hpp>

namespace {
    enum class test_enum {
        value1,
        value2,
        value3
    };

    enum class test_enum_flags {
        flag1 = 1,
        flag2 = 2,
        flag4 = 4,
        flag8 = 8
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(test_enum_flags);

    enum class test_enum_empty {
    };

    enum class test_enum_underlying : unsigned int {
        val1 = 10,
        val2 = 20,
        val3 = 30
    };
}

TEST_CASE("enum_count", "[enumeration]") {
    REQUIRE(rainy::core::meta::enum_count<::test_enum>() == 3);
    REQUIRE(rainy::core::meta::enum_count<::test_enum_empty>() == 0);
    REQUIRE(rainy::core::meta::enum_count<::test_enum_underlying>() == 3);
}

TEST_CASE("enum_values", "[enumeration]") {
    constexpr auto values = rainy::core::meta::enum_values<::test_enum>();
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == ::test_enum::value1);
    REQUIRE(values[1] == ::test_enum::value2);
    REQUIRE(values[2] == ::test_enum::value3);

    constexpr auto empty_values = rainy::core::meta::enum_values<::test_enum_empty>();
    REQUIRE(empty_values.size() == 0);
}

TEST_CASE("enum_value_index", "[enumeration]") {
    constexpr auto val1 = rainy::core::meta::enum_value<::test_enum, 0>();
    constexpr auto val2 = rainy::core::meta::enum_value<::test_enum, 1>();
    constexpr auto val3 = rainy::core::meta::enum_value<::test_enum, 2>();

    REQUIRE(val1 == ::test_enum::value1);
    REQUIRE(val2 == ::test_enum::value2);
    REQUIRE(val3 == ::test_enum::value3);

    constexpr auto val_dynamic = rainy::core::meta::enum_value<::test_enum>(1);
    REQUIRE(val_dynamic == ::test_enum::value2);
}

TEST_CASE("enum_entries", "[enumeration]") {
    constexpr auto entries = rainy::core::meta::enum_entries<::test_enum>();
    REQUIRE(entries.size() == 3);

    const auto &entry0 = entries[0];
    REQUIRE(entry0.first == ::test_enum::value1);
    REQUIRE_FALSE(entry0.second.empty());

    const auto &entry1 = entries[1];
    REQUIRE(entry1.first == ::test_enum::value2);
    REQUIRE_FALSE(entry1.second.empty());

    const auto &entry2 = entries[2];
    REQUIRE(entry2.first == ::test_enum::value3);
    REQUIRE_FALSE(entry2.second.empty());
}

TEST_CASE("enum_name", "[enumeration]") {
    using ::test_enum;

    constexpr auto name1 = rainy::core::meta::enum_name<test_enum>(test_enum::value1);
    REQUIRE_FALSE(name1.empty());

    constexpr auto name2 = rainy::core::meta::enum_name<test_enum, test_enum::value2>();
    REQUIRE_FALSE(name2.empty());

    // Test enum_name for invalid value (using static_cast to int and back to enum)
    constexpr auto invalid = static_cast<test_enum>(999);
    constexpr auto name_invalid = rainy::core::meta::enum_name<test_enum>(invalid);
    REQUIRE(name_invalid.empty());
}

TEST_CASE("enum_names", "[enumeration]") {
    constexpr auto names = rainy::core::meta::enum_names<::test_enum>();
    REQUIRE(names.size() == 3);
    REQUIRE_FALSE(names[0].empty());
    REQUIRE_FALSE(names[1].empty());
    REQUIRE_FALSE(names[2].empty());

    constexpr auto empty_names = rainy::core::meta::enum_names<::test_enum_empty>();
    REQUIRE(empty_names.size() == 0);
}

TEST_CASE("enum_cast", "[enumeration]") {
    using ::test_enum;

    constexpr auto result1 = rainy::core::meta::enum_cast<test_enum>("value1");
    REQUIRE(result1.has_value());
    REQUIRE(result1.value() == test_enum::value1);

    constexpr auto result2 = rainy::core::meta::enum_cast<test_enum>("value2");
    REQUIRE(result2.has_value());
    REQUIRE(result2.value() == test_enum::value2);

    constexpr auto result3 = rainy::core::meta::enum_cast<test_enum>("invalid");
    REQUIRE_FALSE(result3.has_value());

    // Test case insensitive
    constexpr auto result_case = rainy::core::meta::enum_cast<test_enum>("VALUE1", rainy::core::meta::case_insensitive);
    REQUIRE(result_case.has_value());
    REQUIRE(result_case.value() == test_enum::value1);

    // Test underlying type cast
    constexpr auto result_underlying = rainy::core::meta::enum_cast<test_enum>(0);
    REQUIRE(result_underlying.has_value());
    REQUIRE(result_underlying.value() == test_enum::value1);

    constexpr auto result_underlying_invalid = rainy::core::meta::enum_cast<test_enum>(999);
    REQUIRE_FALSE(result_underlying_invalid.has_value());
}

TEST_CASE("enum_contains", "[enumeration]") {
    using ::test_enum;

    REQUIRE(rainy::core::meta::enum_contains(test_enum::value1));
    REQUIRE(rainy::core::meta::enum_contains(test_enum::value2));
    REQUIRE(rainy::core::meta::enum_contains(test_enum::value3));

    constexpr auto invalid = static_cast<test_enum>(999);
    REQUIRE_FALSE(rainy::core::meta::enum_contains(invalid));

    REQUIRE(rainy::core::meta::enum_contains<test_enum, test_enum::value1>());
    REQUIRE_FALSE(rainy::core::meta::enum_contains<test_enum>(999));

    REQUIRE(rainy::core::meta::enum_contains<test_enum>(0));
    REQUIRE(rainy::core::meta::enum_contains<test_enum>(1));
    REQUIRE(rainy::core::meta::enum_contains<test_enum>(2));
    REQUIRE_FALSE(rainy::core::meta::enum_contains<test_enum>(999));

    REQUIRE(rainy::core::meta::enum_contains<test_enum>("value1"));
    REQUIRE(rainy::core::meta::enum_contains<test_enum>("value2"));
    REQUIRE_FALSE(rainy::core::meta::enum_contains<test_enum>("invalid"));

    REQUIRE(rainy::core::meta::enum_contains<test_enum>("VALUE1", rainy::core::meta::case_insensitive));
}

TEST_CASE("enum_flags_name", "[enumeration]") {
    using ::test_enum_flags;

    auto name1 = rainy::core::meta::enum_flags_name(test_enum_flags::flag1);
    REQUIRE_FALSE(name1.empty());

    constexpr auto combined = test_enum_flags::flag1 | test_enum_flags::flag2;
    auto name_combined = rainy::core::meta::enum_flags_name(combined);
    REQUIRE_FALSE(name_combined.empty());

    constexpr auto combined2 = test_enum_flags::flag1 | test_enum_flags::flag4;
    auto name_combined2 = rainy::core::meta::enum_flags_name(combined2);
    REQUIRE_FALSE(name_combined2.empty());

    constexpr auto invalid = static_cast<test_enum_flags>(16);
    auto name_invalid = rainy::core::meta::enum_flags_name(invalid);
    REQUIRE(name_invalid.empty());
}

TEST_CASE("enum_type_name", "[enumeration]") {
    constexpr auto type_name = rainy::core::meta::enum_type_name<::test_enum>();
    REQUIRE_FALSE(type_name.empty());

    constexpr auto empty_type_name = rainy::core::meta::enum_type_name<::test_enum_empty>();
    REQUIRE_FALSE(empty_type_name.empty());
}

TEST_CASE("is_enum_value", "[enumeration]") {
    using ::test_enum;

    constexpr bool is_valid1 = rainy::core::meta::is_enum_value_v<test_enum, static_cast<int>(test_enum::value1)>;
    REQUIRE(is_valid1);

    constexpr bool is_valid2 = rainy::core::meta::is_enum_value_v<test_enum, static_cast<int>(test_enum::value2)>;
    REQUIRE(is_valid2);

    constexpr bool is_invalid = rainy::core::meta::is_enum_value_v<test_enum, 999>;
    REQUIRE_FALSE(is_invalid);
}

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

namespace anno {
    enum class test_ignore_enum {
        value1[[= rainy::annotations::moon::ignore]],
        value2,
        value3
    };

    enum class all_ignore {
        item1[[= rainy::annotations::moon::ignore]] = 1,
        item2[[= rainy::annotations::moon::ignore]] = 2,
        item3[[= rainy::annotations::moon::ignore]] = 3,
        item4[[= rainy::annotations::moon::ignore]] = 4
    };

    enum class test_enum_empty {
    };

    enum class test_enum_namestyle {
        val1[[= rainy::annotations::moon::all_caps]] = 10,
        valTwo[[= rainy::annotations::moon::use_snake_case]] = 20,
        val[[= rainy::annotations::moon::with_suffix("3")]] = 30
    };

    enum class[[= rainy::annotations::moon::all_caps]] test_enum_namestyle_global {
        val1 = 10,
        val2[[= rainy::annotations::moon::no_name_style]] = 20,
        three[[= rainy::annotations::moon::with_prefix("val")]] = 30
    };

    enum class test_enum_rename {
        val1[[= rainy::annotations::moon::rename("one")]] = 1,
        val2[[= rainy::annotations::moon::rename("two")]] = 2,
        val3[[= rainy::annotations::moon::rename("three")]] = 3
    };
}

TEST_CASE("enum_count with ignore", "[enumeration][annotation]") {
    STATIC_REQUIRE(rainy::core::meta::enum_count<anno::test_ignore_enum>() == 2);
    STATIC_REQUIRE(rainy::core::meta::enum_count<anno::all_ignore>() == 0);
    STATIC_REQUIRE(rainy::core::meta::enum_count<anno::test_enum_empty>() == 0);
}

TEST_CASE("enum_name with namestyle", "[enumeration][annotation]") {
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_namestyle>(anno::test_enum_namestyle::val1) == "VAL1");
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_namestyle>(anno::test_enum_namestyle::valTwo) == "val_two");
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_namestyle>(anno::test_enum_namestyle::val) == "val3");
}

TEST_CASE("enum_name with global namestyle", "[enumeration][annotation]") {
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_namestyle_global>(anno::test_enum_namestyle_global::val1) == "VAL1");
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_namestyle_global>(anno::test_enum_namestyle_global::val2) == "val2");
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_namestyle_global>(anno::test_enum_namestyle_global::three) == "valTHREE");
}

TEST_CASE("enum_name with rename", "[enumeration][annotation]") {
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_rename>(anno::test_enum_rename::val1) == "one");
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_rename>(anno::test_enum_rename::val2) == "two");
    STATIC_REQUIRE(rainy::core::meta::enum_name<anno::test_enum_rename>(anno::test_enum_rename::val3) == "three");
}

#endif
