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
#include <rainy/core/type_traits/templates.hpp>

using namespace rainy::type_traits::extras::templates;
using rainy::type_traits::type_relations::is_same_v;

template <typename First, typename... Rest>
struct sample_template {};

struct with_difference_type {
    using difference_type = long;
};

struct with_rebind {
    template <typename>
    struct rebind {
        using type = int;
    };
};

TEST_CASE("get_first_parameter", "[type_traits][templates]") {
    STATIC_REQUIRE(is_same_v<get_first_parameter<sample_template<int, double>>::type, int>);
    STATIC_REQUIRE(is_same_v<get_first_parameter<sample_template<char>>::type, char>);
}

TEST_CASE("replace_first_parameter", "[type_traits][templates]") {
    STATIC_REQUIRE(
        is_same_v<replace_first_parameter<long, sample_template<int, double>>::type, sample_template<long, double>>);
    STATIC_REQUIRE(is_same_v<replace_first_parameter<long, sample_template<int>>::type, sample_template<long>>);
}

TEST_CASE("get_ptr_difference_type", "[type_traits][templates]") {
    STATIC_REQUIRE(is_same_v<get_ptr_difference_type<int *>::type, ptrdiff_t>);
    STATIC_REQUIRE(is_same_v<get_ptr_difference_type<with_difference_type>::type, long>);
}

TEST_CASE("get_rebind_alias", "[type_traits][templates]") {
    STATIC_REQUIRE(is_same_v<get_rebind_alias<sample_template<int>, double>::type, sample_template<double>>);
    STATIC_REQUIRE(is_same_v<get_rebind_alias<with_rebind, double>::type, with_rebind::rebind<double>>);
}

TEST_CASE("replace_last_parameter", "[type_traits][templates]") {
    STATIC_REQUIRE(
        is_same_v<replace_last_parameter<sample_template<int, char>, double>::type, sample_template<int, double>>);
}
