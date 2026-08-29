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
#include <rainy/core/collections/views/algorithm/set_operations.hpp>

#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

TEST_CASE("includes checks subsequence containment", "[algorithm][set]") {
    std::vector<int> big{1, 2, 3, 4, 5};
    std::vector<int> inside{2, 3};
    std::vector<int> outside{2, 6};

    REQUIRE(ra::includes(big.begin(), big.end(), inside.begin(), inside.end()));
    REQUIRE_FALSE(ra::includes(big.begin(), big.end(), outside.begin(), outside.end()));
    REQUIRE(ra::includes(big.begin(), big.end(), big.begin(), big.end()));
}

TEST_CASE("merge combines two sorted ranges", "[algorithm][set]") {
    std::vector<int> left{1, 3, 5};
    std::vector<int> right{2, 4, 6};
    std::vector<int> result(6, 0);

    auto end = ra::merge(left.begin(), left.end(), right.begin(), right.end(), result.begin());
    REQUIRE(end == result.end());
    REQUIRE((result == std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST_CASE("inplace_merge joins two sorted halves", "[algorithm][set]") {
    std::vector<int> data{1, 3, 5, 2, 4, 6};
    ra::inplace_merge(data.begin(), data.begin() + 3, data.end());
    REQUIRE((data == std::vector<int>{1, 2, 3, 4, 5, 6}));

    std::vector<int> with_comp{5, 3, 1, 6, 4, 2};
    ra::inplace_merge(with_comp.begin(), with_comp.begin() + 3, with_comp.end(), [](int left, int right) { return left > right; });
    REQUIRE((with_comp == std::vector<int>{6, 5, 4, 3, 2, 1}));
}

TEST_CASE("set union keeps every element once", "[algorithm][set]") {
    std::vector<int> left{1, 2, 4};
    std::vector<int> right{2, 3, 4};
    std::vector<int> result;
    ra::set_union(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("set intersection keeps common elements", "[algorithm][set]") {
    std::vector<int> left{1, 2, 3, 4};
    std::vector<int> right{2, 4, 6};
    std::vector<int> result;
    ra::set_intersection(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result));
    REQUIRE((result == std::vector<int>{2, 4}));
}

TEST_CASE("set difference keeps left-only elements", "[algorithm][set]") {
    std::vector<int> left{1, 2, 3, 4};
    std::vector<int> right{2, 4, 6};
    std::vector<int> result;
    ra::set_difference(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1, 3}));
}

TEST_CASE("set symmetric difference keeps non-common elements", "[algorithm][set]") {
    std::vector<int> left{1, 2, 3, 4};
    std::vector<int> right{2, 4, 6};
    std::vector<int> result;
    ra::set_symmetric_difference(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1, 3, 6}));
}

TEST_CASE("set operations respect a custom comparator", "[algorithm][set]") {
    auto greater = [](int left, int right) { return left > right; };
    std::vector<int> left{4, 2, 1};
    std::vector<int> right{4, 3, 2};
    std::vector<int> result;
    ra::set_union(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result), greater);
    REQUIRE((result == std::vector<int>{4, 3, 2, 1}));
}

TEST_CASE("ranges set operations delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> left{1, 2, 4};
    std::vector<int> right{2, 3, 4};

    REQUIRE(rv::includes(left, left));
    REQUIRE_FALSE(rv::includes(left, right));

    std::vector<int> result;
    rv::set_union(left, right, std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1, 2, 3, 4}));

    result.clear();
    rv::set_intersection(left, right, std::back_inserter(result));
    REQUIRE((result == std::vector<int>{2, 4}));

    result.clear();
    rv::set_difference(left, right, std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1}));

    result.clear();
    rv::set_symmetric_difference(left, right, std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1, 3}));

    result.clear();
    rv::merge(left, right, std::back_inserter(result));
    REQUIRE((result == std::vector<int>{1, 2, 2, 3, 4, 4}));
}
