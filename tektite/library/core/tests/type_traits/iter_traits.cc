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
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/ranges_traits.hpp>

using namespace rainy::type_traits::extras::iterators;
using namespace rainy::type_traits::extras::ranges;
using rainy::type_traits::type_relations::is_same_v;

struct full_iterator {
    using value_type = int;
    using difference_type = long;
    using pointer = int *;
    using reference = int &;
    using iterator_category = std::forward_iterator_tag;
};

struct minimal_iterator {
    using value_type = double;
};

struct full_range {
    using value_type = int;
    using reference = int &;
    using const_reference = const int &;
    using difference_type = long;
    using iterator = full_iterator;
    using const_iterator = full_iterator;
};

TEST_CASE("iterator member type detection", "[type_traits][iter_traits]") {
    STATIC_REQUIRE(has_value_type<full_iterator>::value);
    STATIC_REQUIRE(has_value_type<minimal_iterator>::value);
    STATIC_REQUIRE_FALSE(has_value_type<int>::value);

    STATIC_REQUIRE(has_difference_type<full_iterator>::value);
    STATIC_REQUIRE_FALSE(has_difference_type<minimal_iterator>::value);

    STATIC_REQUIRE(has_pointer<full_iterator>::value);
    STATIC_REQUIRE_FALSE(has_pointer<minimal_iterator>::value);

    STATIC_REQUIRE(has_reference<full_iterator>::value);
    STATIC_REQUIRE_FALSE(has_reference<minimal_iterator>::value);

    STATIC_REQUIRE(has_iterator_category<full_iterator>::value);
    STATIC_REQUIRE_FALSE(has_iterator_category<minimal_iterator>::value);

    STATIC_REQUIRE_FALSE(has_element_type<full_iterator>::value);
}

TEST_CASE("iterator_traits", "[type_traits][iter_traits]") {
    using traits = iterator_traits<full_iterator>;
    STATIC_REQUIRE(is_same_v<typename traits::value_type, int>);
    STATIC_REQUIRE(is_same_v<typename traits::difference_type, long>);
    STATIC_REQUIRE(is_same_v<typename traits::pointer, int *>);
    STATIC_REQUIRE(is_same_v<typename traits::reference, int &>);
    STATIC_REQUIRE(
        is_same_v<typename traits::iterator_category, std::forward_iterator_tag>);
}

TEST_CASE("range traits", "[type_traits][ranges_traits]") {
    using traits = range_traits<full_range>;
    STATIC_REQUIRE(is_same_v<typename traits::value_type, int>);
    STATIC_REQUIRE(is_same_v<typename traits::reference, int &>);
    STATIC_REQUIRE(is_same_v<typename traits::const_reference, const int &>);
    STATIC_REQUIRE(is_same_v<typename traits::difference_type, long>);
    STATIC_REQUIRE(is_same_v<typename traits::iterator, full_iterator>);

    STATIC_REQUIRE(is_same_v<range_reference_t<full_range>, int &>);
    STATIC_REQUIRE(is_same_v<range_const_reference_t<full_range>, const int &>);
    STATIC_REQUIRE(is_same_v<range_difference_t<full_range>, long>);
    STATIC_REQUIRE(is_same_v<range_value_type_t<full_range>, int>);
}
