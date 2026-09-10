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
#include <rainy/core/collections/views/algorithm/heap.hpp>

#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

TEST_CASE("make_heap and is_heap", "[algorithm][heap]") {
    std::vector<int> data{3, 1, 4, 1, 5, 9, 2, 6};
    ra::make_heap(data.begin(), data.end());
    REQUIRE(ra::is_heap(data.begin(), data.end()));
    REQUIRE(data.front() == 9);

    std::vector<int> empty;
    ra::make_heap(empty.begin(), empty.end());
    REQUIRE(ra::is_heap(empty.begin(), empty.end()));

    std::vector<int> single{42};
    ra::make_heap(single.begin(), single.end());
    REQUIRE(ra::is_heap(single.begin(), single.end()));

    std::vector<int> with_comp{3, 1, 4};
    ra::make_heap(with_comp.begin(), with_comp.end(), [](int left, int right) { return left > right; });
    REQUIRE(with_comp.front() == 1);
    REQUIRE(ra::is_heap(with_comp.begin(), with_comp.end(), [](int left, int right) { return left > right; }));
}

TEST_CASE("is_heap_until finds the longest heap prefix", "[algorithm][heap]") {
    std::vector<int> data{9, 5, 1};
    REQUIRE(ra::is_heap_until(data.begin(), data.end()) == data.end());

    std::vector<int> broken{9, 5, 1, 7};
    REQUIRE(ra::is_heap_until(broken.begin(), broken.end()) == broken.begin() + 3);
}

TEST_CASE("push_heap and pop_heap maintain the invariant", "[algorithm][heap]") {
    std::vector<int> data;
    for (int value : {3, 1, 4, 1, 5, 9}) {
        data.push_back(value);
        ra::push_heap(data.begin(), data.end());
        REQUIRE(ra::is_heap(data.begin(), data.end()));
    }
    REQUIRE(data.front() == 9);

    ra::pop_heap(data.begin(), data.end());
    REQUIRE(data.back() == 9);
    data.pop_back();
    REQUIRE(ra::is_heap(data.begin(), data.end()));
    REQUIRE(data.front() == 5);

    ra::pop_heap(data.begin(), data.end());
    REQUIRE(data.back() == 5);
    data.pop_back();
    REQUIRE(ra::is_heap(data.begin(), data.end()));
}

TEST_CASE("sort_heap drains the heap in ascending order", "[algorithm][heap]") {
    std::vector<int> data{3, 1, 4, 1, 5, 9, 2, 6};
    ra::make_heap(data.begin(), data.end());
    ra::sort_heap(data.begin(), data.end());
    REQUIRE((data == std::vector<int>{1, 1, 2, 3, 4, 5, 6, 9}));
    REQUIRE_FALSE(ra::is_heap(data.begin(), data.end()));
}

TEST_CASE("heaps work with a custom comparator", "[algorithm][heap]") {
    std::vector<int> data{3, 1, 4, 1, 5};
    auto greater = [](int left, int right) { return left > right; };

    ra::make_heap(data.begin(), data.end(), greater);
    REQUIRE(ra::is_heap(data.begin(), data.end(), greater));
    REQUIRE(data.front() == 1);

    ra::sort_heap(data.begin(), data.end(), greater);
    REQUIRE((data == std::vector<int>{5, 4, 3, 1, 1}));
}

TEST_CASE("ranges heap algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data{3, 1, 4, 1, 5, 9};
    rv::make_heap(data);
    REQUIRE(rv::is_heap(data));
    REQUIRE(data.front() == 9);

    data.push_back(7);
    rv::push_heap(data);
    REQUIRE(rv::is_heap(data));
    REQUIRE(data.front() == 9);

    rv::pop_heap(data);
    REQUIRE(data.back() == 9);
    data.pop_back();

    rv::sort_heap(data);
    REQUIRE((data == std::vector<int>{1, 1, 3, 4, 5, 7}));
    std::vector<int> heap_ok{9, 5, 1};
    REQUIRE(rv::is_heap_until(heap_ok) == heap_ok.end());
    std::vector<int> heap_broken{9, 5, 1, 7};
    REQUIRE(rv::is_heap_until(heap_broken) == heap_broken.begin() + 3);
}
