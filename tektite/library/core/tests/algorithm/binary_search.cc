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
#include <rainy/core/algorithm.hpp>
#include <rainy/core/collections/views/algorithm/binary_search.hpp>

#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

TEST_CASE("upper_bound finds the first greater element", "[algorithm][binary_search]") {
    std::vector<int> data{1, 2, 2, 3, 5};

    REQUIRE(ra::upper_bound(data.begin(), data.end(), 2) == data.begin() + 3);
    REQUIRE(ra::upper_bound(data.begin(), data.end(), 0) == data.begin());
    REQUIRE(ra::upper_bound(data.begin(), data.end(), 5) == data.end());
    REQUIRE(ra::upper_bound(data.begin(), data.end(), 9) == data.end());

    REQUIRE(ra::upper_bound(data.begin(), data.end(), 2, [](int left, int right) { return left < right; }) ==
            data.begin() + 3);
}

TEST_CASE("equal_range delimits equal elements", "[algorithm][binary_search]") {
    std::vector<int> data{1, 2, 2, 2, 4};

    auto range = ra::equal_range(data.begin(), data.end(), 2);
    REQUIRE(range.first == data.begin() + 1);
    REQUIRE(range.second == data.begin() + 4);

    auto missing = ra::equal_range(data.begin(), data.end(), 3);
    REQUIRE(missing.first == missing.second);
    REQUIRE(missing.first == data.begin() + 4);

    auto comp_range = ra::equal_range(data.begin(), data.end(), 2, [](int left, int right) { return left < right; });
    REQUIRE(comp_range.first == data.begin() + 1);
    REQUIRE(comp_range.second == data.begin() + 4);
}

TEST_CASE("ranges binary search algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data{1, 2, 2, 3, 5};

    REQUIRE(rv::upper_bound(data, 2) == data.begin() + 3);
    REQUIRE(rv::lower_bound(data, 2) == data.begin() + 1);
    REQUIRE(rv::binary_search(data, 3));
    REQUIRE_FALSE(rv::binary_search(data, 4));

    auto range = rv::equal_range(data, 2);
    REQUIRE(range.first == data.begin() + 1);
    REQUIRE(range.second == data.begin() + 3);
}
