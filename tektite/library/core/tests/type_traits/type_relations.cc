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
#include <rainy/core/type_traits/type_relations.hpp>

using namespace rainy::type_traits::type_relations;

namespace testing {
    struct base {};
    struct derived : base {};
}

TEST_CASE("is_base_of", "[type_traits][type_relations]") {
    STATIC_REQUIRE(is_base_of_v<testing::base, testing::derived>);
    STATIC_REQUIRE(is_base_of_v<testing::base, testing::base>);
    STATIC_REQUIRE_FALSE(is_base_of_v<testing::derived, testing::base>);
    STATIC_REQUIRE(is_base_of_v<const testing::base, testing::derived>);
}

TEST_CASE("is_any_of", "[type_traits][type_relations]") {
    STATIC_REQUIRE(is_any_of_v<int, int, double>);
    STATIC_REQUIRE(is_any_of_v<double, int, double>);
    STATIC_REQUIRE_FALSE(is_any_of_v<char, int, double>);
    STATIC_REQUIRE_FALSE(is_any_of_v<char>);
}

TEST_CASE("is_convertible", "[type_traits][type_relations]") {
    STATIC_REQUIRE(is_convertible_v<testing::derived &, testing::base &>);
    STATIC_REQUIRE(is_convertible_v<int, double>);
    STATIC_REQUIRE_FALSE(is_convertible_v<testing::base *, testing::derived *>);
    STATIC_REQUIRE(is_nothrow_convertible_v<int, double>);
    STATIC_REQUIRE_FALSE(is_nothrow_convertible_v<testing::base, testing::derived>);
}
