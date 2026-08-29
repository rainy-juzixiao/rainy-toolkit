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
#include <limits>
#include <rainy/core/type_traits/limits.hpp>

using rainy::utility::numeric_limits;

TEST_CASE("numeric_limits of integers", "[type_traits][limits]") {
    SECTION("int") {
        STATIC_REQUIRE(numeric_limits<int>::is_specialized);
        STATIC_REQUIRE(numeric_limits<int>::is_signed);
        STATIC_REQUIRE(numeric_limits<int>::is_integer);
        STATIC_REQUIRE(numeric_limits<int>::is_bounded);
        STATIC_REQUIRE(numeric_limits<int>::digits == 31);
        STATIC_REQUIRE(numeric_limits<int>::max() == std::numeric_limits<int>::max());
        STATIC_REQUIRE(numeric_limits<int>::lowest() == std::numeric_limits<int>::lowest());
        STATIC_REQUIRE(numeric_limits<int>::min() == std::numeric_limits<int>::min());
    }
    SECTION("unsigned int") {
        STATIC_REQUIRE(numeric_limits<unsigned int>::is_specialized);
        STATIC_REQUIRE_FALSE(numeric_limits<unsigned int>::is_signed);
        STATIC_REQUIRE(numeric_limits<unsigned int>::min() == 0);
        STATIC_REQUIRE(numeric_limits<unsigned int>::max() == std::numeric_limits<unsigned int>::max());
    }
    SECTION("bool") {
        STATIC_REQUIRE(numeric_limits<bool>::is_specialized);
        STATIC_REQUIRE_FALSE(numeric_limits<bool>::is_signed);
        STATIC_REQUIRE(numeric_limits<bool>::digits == 1);
    }
}

TEST_CASE("numeric_limits of floating points", "[type_traits][limits]") {
    STATIC_REQUIRE(numeric_limits<double>::is_specialized);
    STATIC_REQUIRE(numeric_limits<double>::is_signed);
    STATIC_REQUIRE_FALSE(numeric_limits<double>::is_integer);
    STATIC_REQUIRE(numeric_limits<double>::has_infinity);
    STATIC_REQUIRE(numeric_limits<double>::has_quiet_NaN);
    STATIC_REQUIRE(numeric_limits<double>::digits == 53);
    STATIC_REQUIRE(numeric_limits<double>::max() == std::numeric_limits<double>::max());
    STATIC_REQUIRE(numeric_limits<double>::lowest() == std::numeric_limits<double>::lowest());
    STATIC_REQUIRE(numeric_limits<double>::infinity() == std::numeric_limits<double>::infinity());
    STATIC_REQUIRE(numeric_limits<double>::epsilon() == std::numeric_limits<double>::epsilon());

    STATIC_REQUIRE(numeric_limits<float>::digits == 24);
    STATIC_REQUIRE(numeric_limits<float>::max() == std::numeric_limits<float>::max());
}

TEST_CASE("numeric_limits cv-qualified propagation", "[type_traits][limits]") {
    STATIC_REQUIRE(numeric_limits<const int>::max() == std::numeric_limits<int>::max());
    STATIC_REQUIRE(numeric_limits<volatile int>::is_signed);
}
