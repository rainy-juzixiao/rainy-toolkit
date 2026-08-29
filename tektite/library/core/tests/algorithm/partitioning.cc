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
#include <rainy/core/collections/views/algorithm/partitioning.hpp>

#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

namespace {
    auto is_even = [](int value) { return value % 2 == 0; };
}

TEST_CASE("is_partitioned reports partition state", "[algorithm][partitioning]") {
    std::vector<int> partitioned{2, 4, 1, 3};
    REQUIRE(ra::is_partitioned(partitioned.begin(), partitioned.end(), is_even));

    std::vector<int> not_partitioned{2, 1, 4, 3};
    REQUIRE_FALSE(ra::is_partitioned(not_partitioned.begin(), not_partitioned.end(), is_even));

    std::vector<int> all_true{2, 4};
    REQUIRE(ra::is_partitioned(all_true.begin(), all_true.end(), is_even));
    std::vector<int> all_false{1, 3};
    REQUIRE(ra::is_partitioned(all_false.begin(), all_false.end(), is_even));
}

TEST_CASE("partition reorders the range", "[algorithm][partitioning]") {
    std::vector<int> data{1, 2, 3, 4, 5, 6};
    auto point = ra::partition(data.begin(), data.end(), is_even);

    REQUIRE(ra::is_partitioned(data.begin(), data.end(), is_even));
    REQUIRE(point == data.begin() + 3);
    std::vector<int> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    REQUIRE((sorted == std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST_CASE("partition_copy splits into two destinations", "[algorithm][partitioning]") {
    const std::vector<int> data{1, 2, 3, 4, 5, 6};
    std::vector<int> evens;
    std::vector<int> odds;

    auto ends = ra::partition_copy(data.begin(), data.end(), std::back_inserter(evens), std::back_inserter(odds), is_even);
    REQUIRE((evens == std::vector<int>{2, 4, 6}));
    REQUIRE((odds == std::vector<int>{1, 3, 5}));
}

TEST_CASE("stable_partition preserves relative order", "[algorithm][partitioning]") {
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8};
    auto point = ra::stable_partition(data.begin(), data.end(), is_even);

    REQUIRE(point == data.begin() + 4);
    REQUIRE((data == std::vector<int>{2, 4, 6, 8, 1, 3, 5, 7}));
}

TEST_CASE("partition_point locates the boundary", "[algorithm][partitioning]") {
    std::vector<int> data{2, 4, 6, 1, 3, 5};
    auto point = ra::partition_point(data.begin(), data.end(), is_even);
    REQUIRE(point == data.begin() + 3);

    std::vector<int> all_even{2, 4, 6};
    REQUIRE(ra::partition_point(all_even.begin(), all_even.end(), is_even) == all_even.end());
    std::vector<int> none_even{1, 3, 5};
    REQUIRE(ra::partition_point(none_even.begin(), none_even.end(), is_even) == none_even.begin());
}

TEST_CASE("ranges partitioning algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8};

    REQUIRE(rv::is_partitioned(std::vector<int>{2, 4, 1, 3}, is_even));

    auto point = rv::stable_partition(data, is_even);
    REQUIRE(point == data.begin() + 4);
    REQUIRE((data == std::vector<int>{2, 4, 6, 8, 1, 3, 5, 7}));

    std::vector<int> evens;
    std::vector<int> odds;
    rv::partition_copy(data, std::back_inserter(evens), std::back_inserter(odds), is_even);
    REQUIRE((evens == std::vector<int>{2, 4, 6, 8}));
    REQUIRE((odds == std::vector<int>{1, 3, 5, 7}));

    std::vector<int> to_partition{1, 2, 3, 4};
    rv::partition(to_partition, is_even);
    REQUIRE(rv::is_partitioned(to_partition, is_even));
    REQUIRE(rv::partition_point(to_partition, is_even) == to_partition.begin() + 2);
}
