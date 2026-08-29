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
#include <rainy/core/collections/views/algorithm/nonmodifying.hpp>

#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

TEST_CASE("for_each and for_each_n apply a function", "[algorithm][nonmodifying]") {
    std::vector<int> data{1, 2, 3, 4};
    int sum = 0;
    auto fx = ra::for_each(data.begin(), data.end(), [&](int value) { sum += value; });
    REQUIRE(sum == 10);
    fx(0);

    REQUIRE(*ra::for_each_n(data.begin(), 2, [&](int) {}) == 3);
}

TEST_CASE("count and count_if count matching elements", "[algorithm][nonmodifying]") {
    std::vector<int> data{1, 2, 2, 3, 2};
    REQUIRE(ra::count(data.begin(), data.end(), 2) == 3);
    REQUIRE(ra::count(data.begin(), data.end(), 9) == 0);
    REQUIRE(ra::count_if(data.begin(), data.end(), [](int v) { return v > 1; }) == 4);
}

TEST_CASE("mismatch locates the first difference", "[algorithm][nonmodifying]") {
    std::vector<int> left{1, 2, 3};
    std::vector<int> right{1, 2, 4};

    auto hit = ra::mismatch(left.begin(), left.end(), right.begin());
    REQUIRE(*hit.first == 3);
    REQUIRE(*hit.second == 4);

    auto same = ra::mismatch(left.begin(), left.end(), left.begin());
    REQUIRE(same.first == left.end());
}

TEST_CASE("search finds subsequences", "[algorithm][nonmodifying]") {
    std::vector<int> data{1, 2, 3, 1, 2, 3, 1, 2};
    std::vector<int> sub{2, 3, 1};

    REQUIRE(ra::search(data.begin(), data.end(), sub.begin(), sub.end()) == data.begin() + 1);
    REQUIRE(ra::search(data.begin(), data.end(), data.begin(), data.begin()) == data.begin());

    std::vector<int> missing{9, 9};
    REQUIRE(ra::search(data.begin(), data.end(), missing.begin(), missing.end()) == data.end());
}

TEST_CASE("find_end locates the last subsequence occurrence", "[algorithm][nonmodifying]") {
    std::vector<int> data{1, 2, 3, 1, 2, 3, 1, 2};
    std::vector<int> sub{1, 2, 3};

    REQUIRE(ra::find_end(data.begin(), data.end(), sub.begin(), sub.end()) == data.begin() + 3);
    REQUIRE(ra::find_end(data.begin(), data.end(), data.begin(), data.begin()) == data.end());

    std::vector<int> missing{5, 6};
    REQUIRE(ra::find_end(data.begin(), data.end(), missing.begin(), missing.end()) == data.end());
}

TEST_CASE("find_first_of finds set members", "[algorithm][nonmodifying]") {
    std::vector<int> data{4, 2, 7, 9};
    std::vector<int> set{7, 2};

    REQUIRE(ra::find_first_of(data.begin(), data.end(), set.begin(), set.end()) == data.begin() + 1);

    std::vector<int> empty_set;
    REQUIRE(ra::find_first_of(data.begin(), data.end(), empty_set.begin(), empty_set.end()) == data.end());
}

TEST_CASE("adjacent_find finds equal neighbors", "[algorithm][nonmodifying]") {
    std::vector<int> data{1, 2, 2, 3};
    REQUIRE(ra::adjacent_find(data.begin(), data.end()) == data.begin() + 1);

    std::vector<int> unique_data{1, 2, 3};
    REQUIRE(ra::adjacent_find(unique_data.begin(), unique_data.end()) == unique_data.end());

    std::vector<int> predicate_data{1, 2, 4, 5};
    REQUIRE(ra::adjacent_find(predicate_data.begin(), predicate_data.end(), [](int a, int b) { return b - a > 1; }) ==
            predicate_data.begin() + 1);
}

TEST_CASE("search_n finds repeated values", "[algorithm][nonmodifying]") {
    std::vector<int> data{1, 5, 5, 5, 2};

    REQUIRE(ra::search_n(data.begin(), data.end(), 3, 5) == data.begin() + 1);
    REQUIRE(ra::search_n(data.begin(), data.end(), 4, 5) == data.end());
    REQUIRE(ra::search_n(data.begin(), data.end(), 0, 5) == data.begin());
    REQUIRE(ra::search_n(data.begin(), data.end(), 2, 1, [](int element, int value) { return element != value; }) ==
            data.begin() + 1);
}

TEST_CASE("ranges nonmodifying algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data{1, 2, 2, 3, 2};
    std::vector<int> sub{2, 3};
    std::vector<int> set{3, 2};

    REQUIRE(rv::count(data, 2) == 3);
    REQUIRE(rv::count_if(data, [](int v) { return v > 2; }) == 1);

    int sum = 0;
    rv::for_each(data, [&](int value) { sum += value; });
    REQUIRE(sum == 10);
    REQUIRE(*rv::for_each_n(data, 2, [&](int) {}) == 2);

    auto hit = rv::mismatch(data, data.begin());
    REQUIRE(hit.first == data.end());

    REQUIRE(*rv::adjacent_find(data) == 2);
    REQUIRE(*rv::search(data, sub) == 2);
    REQUIRE(rv::search(data, data) == data.begin());
    REQUIRE(*rv::find_end(data, sub) == 2);
    REQUIRE(rv::find_end(data, data) == data.begin());
    REQUIRE(*rv::find_first_of(data, set) == 2);
    REQUIRE(rv::search_n(data, 2, 2) == data.begin() + 1);
    REQUIRE(rv::search_n(data, 3, 2) == data.end());
}
