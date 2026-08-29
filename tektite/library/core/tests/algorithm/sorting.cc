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
#include <rainy/core/collections/views/algorithm/sorting.hpp>

#include <numeric>
#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

namespace {
    auto shuffled = [] {
        std::vector<int> data(200);
        std::iota(data.begin(), data.end(), 0);
        for (std::size_t i = 0; i < data.size(); ++i) {
            std::swap(data[i], data[(i * 7919) % data.size()]);
        }
        return data;
    };
}

TEST_CASE("is_sorted and is_sorted_until", "[algorithm][sorting]") {
    std::vector<int> sorted{1, 2, 2, 3};
    REQUIRE(ra::is_sorted(sorted.begin(), sorted.end()));
    REQUIRE(ra::is_sorted_until(sorted.begin(), sorted.end()) == sorted.end());

    std::vector<int> unsorted{1, 3, 2};
    REQUIRE_FALSE(ra::is_sorted(unsorted.begin(), unsorted.end()));
    REQUIRE(ra::is_sorted_until(unsorted.begin(), unsorted.end()) == unsorted.begin() + 2);

    std::vector<int> descending{5, 3, 1};
    REQUIRE(ra::is_sorted(descending.begin(), descending.end(), [](int left, int right) { return left > right; }));
}

TEST_CASE("sort handles a large shuffled range", "[algorithm][sorting]") {
    std::vector<int> data = shuffled();
    ra::sort(data.begin(), data.end());
    REQUIRE(ra::is_sorted(data.begin(), data.end()));
    REQUIRE(data.front() == 0);
    REQUIRE(data.back() == 199);

    std::vector<int> by_abs = shuffled();
    ra::sort(by_abs.begin(), by_abs.end(), [](int left, int right) { return left > right; });
    REQUIRE(by_abs.front() == 199);
    REQUIRE(by_abs.back() == 0);

    std::vector<int> empty;
    ra::sort(empty.begin(), empty.end());
    REQUIRE(empty.empty());
    std::vector<int> single{1};
    ra::sort(single.begin(), single.end());
    REQUIRE(single.front() == 1);
}

TEST_CASE("stable_sort preserves the order of equal elements", "[algorithm][sorting]") {
    struct entry {
        int key;
        int index;
    };
    std::vector<entry> data{{1, 0}, {0, 1}, {1, 2}, {0, 3}, {1, 4}};
    ra::stable_sort(data.begin(), data.end(), [](const entry &left, const entry &right) { return left.key < right.key; });
    REQUIRE(data[0].index == 1);
    REQUIRE(data[1].index == 3);
    REQUIRE(data[2].index == 0);
    REQUIRE(data[3].index == 2);
    REQUIRE(data[4].index == 4);

    std::vector<int> big = shuffled();
    ra::stable_sort(big.begin(), big.end());
    REQUIRE(ra::is_sorted(big.begin(), big.end()));
}

TEST_CASE("partial_sort places the smallest prefix in order", "[algorithm][sorting]") {
    std::vector<int> data = shuffled();
    ra::partial_sort(data.begin(), data.begin() + 10, data.end());
    REQUIRE(ra::is_sorted(data.begin(), data.begin() + 10));
    for (int i = 0; i < 10; ++i) {
        REQUIRE(data[i] == i);
    }

    std::vector<int> whole{5, 3, 1, 4, 2};
    ra::partial_sort(whole.begin(), whole.end(), whole.end());
    REQUIRE((whole == std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("partial_sort_copy fills the destination prefix", "[algorithm][sorting]") {
    std::vector<int> data = shuffled();
    std::vector<int> target(5, 0);
    auto result = ra::partial_sort_copy(data.begin(), data.end(), target.begin(), target.end());
    REQUIRE(result == target.end());
    REQUIRE((target == std::vector<int>{0, 1, 2, 3, 4}));

    std::vector<int> wide(300, 0);
    result = ra::partial_sort_copy(data.begin(), data.end(), wide.begin(), wide.end());
    REQUIRE(result == wide.begin() + 200);
    REQUIRE(ra::is_sorted(wide.begin(), wide.begin() + 200));
}

TEST_CASE("nth_element settles the nth position", "[algorithm][sorting]") {
    std::vector<int> data = shuffled();
    ra::nth_element(data.begin(), data.begin() + 50, data.end());
    REQUIRE(data[50] == 50);
    for (int i = 0; i < 50; ++i) {
        REQUIRE(data[i] <= 50);
    }
    for (int i = 51; i < 200; ++i) {
        REQUIRE(data[i] >= 50);
    }
}

TEST_CASE("ranges sorting algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data = shuffled();
    rv::sort(data);
    REQUIRE(rv::is_sorted(data));

    std::vector<int> to_nth = shuffled();
    rv::nth_element(to_nth, to_nth.begin() + 100);
    REQUIRE(to_nth[100] == 100);

    std::vector<int> prefix = shuffled();
    rv::partial_sort(prefix, prefix.begin() + 5);
    REQUIRE((std::vector<int>{prefix.begin(), prefix.begin() + 5} == std::vector<int>{0, 1, 2, 3, 4}));

    std::vector<int> to_copy = shuffled();
    std::vector<int> target(3, 0);
    rv::partial_sort_copy(to_copy, target.begin(), target.end());
    REQUIRE((target == std::vector<int>{0, 1, 2}));

    std::vector<int> check{1, 2, 4, 3};
    REQUIRE_FALSE(rv::is_sorted(check));
    REQUIRE(*rv::is_sorted_until(check) == 3);
}
