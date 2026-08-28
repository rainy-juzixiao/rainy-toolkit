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
#include <rainy/core/utility/hash.hpp>

using rainy::utility::hash;

TEST_CASE("hash of integral and enum types", "[utility][hash]") {
    SECTION("integers") {
        hash<int> int_hash;
        hash<unsigned long long> ull_hash;
        STATIC_REQUIRE(std::is_same_v<typename hash<int>::result_type, std::size_t>);
        REQUIRE(int_hash(0) == int_hash(0));
        REQUIRE(int_hash(1) != int_hash(2));
        REQUIRE(int_hash(-1) == int_hash(-1));
    }
    SECTION("bool and enum") {
        enum class color { red, green };
        hash<bool> bool_hash;
        hash<color> color_hash;
        REQUIRE(bool_hash(true) != bool_hash(false));
        REQUIRE(color_hash(color::red) != color_hash(color::green));
    }
    SECTION("pointers") {
        hash<const void *> ptr_hash;
        int value = 0;
        REQUIRE(ptr_hash(&value) == ptr_hash(&value));
        REQUIRE(ptr_hash(nullptr) == ptr_hash(nullptr));
        int other = 0;
        REQUIRE(ptr_hash(&value) != ptr_hash(&other));
    }
}

TEST_CASE("hash of floating point types", "[utility][hash]") {
    hash<float> float_hash;
    hash<double> double_hash;
    REQUIRE(float_hash(1.0f) == float_hash(1.0f));
    REQUIRE(float_hash(1.0f) != float_hash(2.0f));
    REQUIRE(double_hash(3.14) == double_hash(3.14));
    REQUIRE(double_hash(3.14) != double_hash(2.71));
}

TEST_CASE("hash of strings", "[utility][hash]") {
    hash<std::string> string_hash;
    hash<std::string_view> view_hash;
    REQUIRE(string_hash("rainy") == string_hash("rainy"));
    REQUIRE(string_hash("rainy") != string_hash("toolkit"));
    REQUIRE(string_hash("rainy") == view_hash(std::string_view("rainy")));
}

namespace testing {
    struct int_wrapper {
        int value;
    };
} // namespace testing

template <>
struct rainy::utility::hash<testing::int_wrapper> {
    using argument_type = testing::int_wrapper;
    using result_type = std::size_t;

    result_type operator()(const argument_type &value) const {
        return hash<int>{}(value.value);
    }
};

TEST_CASE("custom hash specialization", "[utility][hash]") {
    hash<testing::int_wrapper> wrapper_hash;
    REQUIRE(wrapper_hash({1}) == wrapper_hash({1}));
    REQUIRE(wrapper_hash({1}) != wrapper_hash({2}));
}
