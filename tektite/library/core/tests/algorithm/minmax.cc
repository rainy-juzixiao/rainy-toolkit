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
#include <rainy/core/collections/views/algorithm/minmax.hpp>
#include <rainy/core/collections/views/algorithm/permutation.hpp>

#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

TEST_CASE("min and max compare two values", "[algorithm][minmax]") {
        int low = 1;
    int high = 2;
    REQUIRE(ra::min(low, high) == 1);
    REQUIRE(ra::min(3, 3) == 3);
    REQUIRE(ra::max(1, 2) == 2);
    REQUIRE(ra::min(3, 3) == 3);
    REQUIRE(ra::max(3, 3) == 3);

    REQUIRE(ra::min(1, 2, [](int left, int right) { return left > right; }) == 2);
    REQUIRE(ra::max(1, 2, [](int left, int right) { return left > right; }) == 1);

    int first_value = 1;
    int second_value = 2;
    auto both = ra::minmax(first_value, second_value);
    REQUIRE(both.first == 1);
    REQUIRE(both.second == 2);
}

TEST_CASE("clamp bounds a value", "[algorithm][minmax]") {
    REQUIRE(ra::clamp(5, 1, 10) == 5);
    REQUIRE(ra::clamp(0, 1, 10) == 1);
    REQUIRE(ra::clamp(99, 1, 10) == 10);
    REQUIRE(ra::clamp(5, 10, 1, [](int left, int right) { return left > right; }) == 5);
}

TEST_CASE("element extrema locate positions", "[algorithm][minmax]") {
    std::vector<int> data{3, 1, 4, 1, 5, 9, 2};

    REQUIRE(*ra::min_element(data.begin(), data.end()) == 1);
    REQUIRE(ra::min_element(data.begin(), data.end()) == data.begin() + 1);
    REQUIRE(*ra::max_element(data.begin(), data.end()) == 9);

    auto both = ra::minmax_element(data.begin(), data.end());
    REQUIRE(*both.first == 1);
    REQUIRE(*both.second == 9);

    std::vector<int> empty;
    REQUIRE(ra::min_element(empty.begin(), empty.end()) == empty.end());
    REQUIRE(ra::max_element(empty.begin(), empty.end()) == empty.end());
    auto empty_both = ra::minmax_element(empty.begin(), empty.end());
    REQUIRE(empty_both.first == empty.end());
    REQUIRE(empty_both.second == empty.end());
}

TEST_CASE("is_permutation compares multisets of elements", "[algorithm][permutation]") {
    std::vector<int> left{1, 2, 3};
    std::vector<int> right{3, 1, 2};
    std::vector<int> other{3, 1, 1};

    REQUIRE(ra::is_permutation(left.begin(), left.end(), right.begin()));
    REQUIRE_FALSE(ra::is_permutation(left.begin(), left.end(), other.begin()));

    std::vector<int> duplicates{1, 2, 1};
    std::vector<int> duplicates_other{1, 1, 2};
    REQUIRE(ra::is_permutation(duplicates.begin(), duplicates.end(), duplicates_other.begin()));

    std::vector<int> empty1;
    std::vector<int> empty2;
    REQUIRE(ra::is_permutation(empty1.begin(), empty1.end(), empty2.begin()));
}

TEST_CASE("next_permutation enumerates in lexicographic order", "[algorithm][permutation]") {
    std::vector<int> data{1, 2, 3};

    REQUIRE(ra::next_permutation(data.begin(), data.end()));
    REQUIRE((data == std::vector<int>{1, 3, 2}));
    REQUIRE(ra::next_permutation(data.begin(), data.end()));
    REQUIRE((data == std::vector<int>{2, 1, 3}));

    data = {3, 2, 1};
    REQUIRE_FALSE(ra::next_permutation(data.begin(), data.end()));
    REQUIRE((data == std::vector<int>{1, 2, 3}));
}

TEST_CASE("prev_permutation enumerates in reverse lexicographic order", "[algorithm][permutation]") {
    std::vector<int> data{3, 2, 1};

    REQUIRE(ra::prev_permutation(data.begin(), data.end()));
    REQUIRE((data == std::vector<int>{3, 1, 2}));
    REQUIRE(ra::prev_permutation(data.begin(), data.end()));
    REQUIRE((data == std::vector<int>{2, 3, 1}));

    data = {1, 2, 3};
    REQUIRE_FALSE(ra::prev_permutation(data.begin(), data.end()));
    REQUIRE((data == std::vector<int>{3, 2, 1}));
}

TEST_CASE("ranges minmax and permutation algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data{3, 1, 4, 1, 5};

    REQUIRE(*rv::min_element(data) == 1);
    REQUIRE(*rv::max_element(data) == 5);
    REQUIRE(rv::min(3, 1) == 1);
    REQUIRE(rv::max(3, 1) == 3);
    REQUIRE(rv::clamp(42, 0, 9) == 9);

    auto both = rv::minmax_element(data);
    REQUIRE(*both.first == 1);
    REQUIRE(*both.second == 5);

    std::vector<int> perm{1, 2, 3};
    REQUIRE(rv::is_permutation(perm, std::vector<int>{2, 3, 1}.begin()));
    REQUIRE(rv::next_permutation(perm));
    REQUIRE((perm == std::vector<int>{1, 3, 2}));
    REQUIRE(rv::prev_permutation(perm));
    REQUIRE((perm == std::vector<int>{1, 2, 3}));
}
