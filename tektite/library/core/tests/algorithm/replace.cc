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

#include <vector>

namespace ra = rainy::core::algorithm;

TEST_CASE("replace replaces matching elements in-place", "[algorithm][replace]") {
    std::vector<int> data{1, 2, 1, 3, 1};
    ra::replace(data.begin(), data.end(), 1, 42);
    REQUIRE((data == std::vector<int>{42, 2, 42, 3, 42}));

    SECTION("no match leaves the range untouched") {
        std::vector<int> unchanged{1, 2, 3};
        ra::replace(unchanged.begin(), unchanged.end(), 99, 42);
        REQUIRE((unchanged == std::vector<int>{1, 2, 3}));
    }
    SECTION("works with raw arrays") {
        int arr[4] = {7, 7, 1, 7};
        ra::replace(arr, arr + 4, 7, 0);
        REQUIRE(arr[0] == 0);
        REQUIRE(arr[1] == 0);
        REQUIRE(arr[2] == 1);
        REQUIRE(arr[3] == 0);
    }
}

TEST_CASE("replace_if replaces elements satisfying a predicate", "[algorithm][replace]") {
    std::vector<int> data{1, 8, 3, 10, 5};
    ra::replace_if(data.begin(), data.end(), [](int value) { return value > 5; }, 0);
    REQUIRE((data == std::vector<int>{1, 0, 3, 0, 5}));

    ra::replace_if(data.begin(), data.end(), [](int value) { return value < 0; }, -1);
    REQUIRE((data == std::vector<int>{1, 0, 3, 0, 5}));
}

TEST_CASE("replace_copy writes replaced copies to the destination", "[algorithm][replace]") {
    const std::vector<int> source{1, 2, 1, 3};
    std::vector<int> target(source.size(), 0);

    auto result = ra::replace_copy(source.begin(), source.end(), target.begin(), 1, 42);
    REQUIRE(result == target.end());
    REQUIRE((target == std::vector<int>{42, 2, 42, 3}));
    REQUIRE((source == std::vector<int>{1, 2, 1, 3}));
}

TEST_CASE("replace_copy_if writes predicate-replaced copies", "[algorithm][replace]") {
    const std::vector<int> source{1, 8, 3, 10};
    std::vector<int> target(source.size(), 0);

    auto result = ra::replace_copy_if(source.begin(), source.end(), target.begin(), [](int value) { return value > 5; }, 0);
    REQUIRE(result == target.end());
    REQUIRE((target == std::vector<int>{1, 0, 3, 0}));
}
