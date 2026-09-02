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
#include <array>
#include <vector>
#include <string>

namespace ra = rainy::algorithm;

TEST_CASE("is_permutation - identical ranges", "[algorithm][permutation]") {
    std::array<int, 5> a = {1, 2, 3, 4, 5};
    std::array<int, 5> b = {1, 2, 3, 4, 5};
    REQUIRE(ra::is_permutation(a.begin(), a.end(), b.begin()));

    std::array<int, 5> c = {5, 4, 3, 2, 1};
    REQUIRE(ra::is_permutation(a.begin(), a.end(), c.begin()));
}

TEST_CASE("is_permutation - not permutations", "[algorithm][permutation]") {
    std::array<int, 4> a = {1, 2, 3, 4};
    std::array<int, 4> b = {1, 2, 3, 5};
    REQUIRE_FALSE(ra::is_permutation(a.begin(), a.end(), b.begin()));

    std::array<int, 4> c = {1, 2, 3, 3};
    REQUIRE_FALSE(ra::is_permutation(a.begin(), a.end(), c.begin()));
}

TEST_CASE("is_permutation - empty and single element", "[algorithm][permutation]") {
    std::vector<int> empty_a;
    std::vector<int> empty_b;
    REQUIRE(ra::is_permutation(empty_a.begin(), empty_a.end(), empty_b.begin()));

    std::array<int, 1> single = {42};
    REQUIRE(ra::is_permutation(single.begin(), single.end(), single.begin()));
}

TEST_CASE("is_permutation - duplicates", "[algorithm][permutation]") {
    std::array<int, 6> a = {1, 2, 2, 3, 3, 3};
    std::array<int, 6> b = {3, 2, 1, 3, 2, 3};
    REQUIRE(ra::is_permutation(a.begin(), a.end(), b.begin()));

    std::array<int, 6> c = {1, 2, 2, 3, 3, 4};
    REQUIRE_FALSE(ra::is_permutation(a.begin(), a.end(), c.begin()));
}

TEST_CASE("next_permutation - basic", "[algorithm][permutation]") {
    std::array<int, 3> a = {1, 2, 3};
    REQUIRE(ra::next_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{1, 3, 2});

    REQUIRE(ra::next_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{2, 1, 3});

    REQUIRE(ra::next_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{2, 3, 1});

    REQUIRE(ra::next_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{3, 1, 2});

    REQUIRE(ra::next_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{3, 2, 1});

    REQUIRE_FALSE(ra::next_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{1, 2, 3});
}

TEST_CASE("next_permutation - single element and empty", "[algorithm][permutation]") {
    std::array<int, 1> single = {42};
    REQUIRE_FALSE(ra::next_permutation(single.begin(), single.end()));

    std::vector<int> empty;
    REQUIRE_FALSE(ra::next_permutation(empty.begin(), empty.end()));
}

TEST_CASE("next_permutation - with custom comparator", "[algorithm][permutation]") {
    std::array<int, 3> a = {3, 2, 1};
    REQUIRE(ra::next_permutation(a.begin(), a.end(), [](int l, int r) { return l > r; }));
    REQUIRE(a == std::array<int, 3>{3, 1, 2});
}

TEST_CASE("prev_permutation - basic", "[algorithm][permutation]") {
    std::array<int, 3> a = {3, 2, 1};
    REQUIRE(ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{3, 1, 2});

    REQUIRE(ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{2, 3, 1});

    REQUIRE(ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{2, 1, 3});

    REQUIRE(ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{1, 3, 2});

    REQUIRE(ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{1, 2, 3});

    REQUIRE_FALSE(ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(a == std::array<int, 3>{3, 2, 1});
}

TEST_CASE("prev_permutation - single element and empty", "[algorithm][permutation]") {
    std::array<int, 1> single = {42};
    REQUIRE_FALSE(ra::prev_permutation(single.begin(), single.end()));

    std::vector<int> empty;
    REQUIRE_FALSE(ra::prev_permutation(empty.begin(), empty.end()));
}

TEST_CASE("next_permutation - larger set", "[algorithm][permutation]") {
    std::array<int, 4> a = {1, 2, 3, 4};
    int count = 0;
    do {
        ++count;
    } while (ra::next_permutation(a.begin(), a.end()));
    REQUIRE(count == 24);
}

TEST_CASE("prev_permutation - larger set", "[algorithm][permutation]") {
    std::array<int, 4> a = {4, 3, 2, 1};
    int count = 0;
    do {
        ++count;
    } while (ra::prev_permutation(a.begin(), a.end()));
    REQUIRE(count == 24);
}