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
#include <rainy/core/type_traits/logical.hpp>
#include <rainy/core/type_traits/primary_types.hpp>
#include <rainy/core/type_traits/underlying_type.hpp>

using namespace rainy::type_traits::logical_traits;
using rainy::type_traits::other_trans::underlying_type_t;

namespace testing {
    enum class scoped : unsigned char {};
} // namespace testing

TEST_CASE("conjunction", "[type_traits][logical]") {
    STATIC_REQUIRE(conjunction<>::value);
    STATIC_REQUIRE(conjunction<std::true_type>::value);
    STATIC_REQUIRE(conjunction<std::true_type, std::true_type>::value);
    STATIC_REQUIRE_FALSE(conjunction<std::true_type, std::false_type>::value);

    STATIC_REQUIRE(conjunction_v<std::is_const<const int>, std::is_integral<int>>);
    STATIC_REQUIRE_FALSE(conjunction_v<std::is_const<const int>, std::is_integral<double>>);
}

TEST_CASE("disjunction", "[type_traits][logical]") {
    STATIC_REQUIRE_FALSE(disjunction<>::value);
    STATIC_REQUIRE(disjunction<std::true_type, std::false_type>::value);
    STATIC_REQUIRE_FALSE(disjunction<std::false_type, std::false_type>::value);

    STATIC_REQUIRE(disjunction_v<std::is_integral<double>, std::is_floating_point<double>>);
    STATIC_REQUIRE_FALSE(disjunction_v<std::is_integral<double>, std::is_const<double>>);
}

TEST_CASE("negation", "[type_traits][logical]") {
    STATIC_REQUIRE_FALSE(negation<std::true_type>::value);
    STATIC_REQUIRE(negation<std::false_type>::value);
    STATIC_REQUIRE(negation_v<std::is_void<int>>);
}

TEST_CASE("underlying_type", "[type_traits][logical]") {
    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<underlying_type_t<testing::scoped>, unsigned char>);
}
