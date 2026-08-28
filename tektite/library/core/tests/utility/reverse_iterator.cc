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
#include <rainy/core/utility/reverse_iterator.hpp>

using rainy::utility::reverse_iterator;

TEST_CASE("reverse_iterator traversal", "[utility][reverse_iterator]") {
    std::vector<int> data{1, 2, 3};
    reverse_iterator<std::vector<int>::iterator> rbegin(data.end());
    reverse_iterator<std::vector<int>::iterator> rend(data.begin());

    std::vector<int> collected;
    for (auto it = rbegin; it != rend; ++it) {
        collected.push_back(*it);
    }
    REQUIRE((collected == std::vector<int>{3, 2, 1}));
}

TEST_CASE("reverse_iterator access and navigation", "[utility][reverse_iterator]") {
    std::vector<int> data{1, 2, 3, 4};
    auto rit = reverse_iterator<std::vector<int>::iterator>(data.end());

    REQUIRE(*rit == 4);
    REQUIRE(rit[1] == 3);

    ++rit;
    REQUIRE(*rit == 3);
    rit++;
    REQUIRE(*rit == 2);
    --rit;
    REQUIRE(*rit == 3);
    rit--;
    REQUIRE(*rit == 4);

    auto advanced = rit + 2;
    REQUIRE(*advanced == 2);
    auto rewound = advanced - 2;
    REQUIRE(*rewound == 4);
    REQUIRE(advanced - rit == 2);
}

TEST_CASE("reverse_iterator base and comparison", "[utility][reverse_iterator]") {
    std::vector<int> data{1, 2, 3};
    auto rit = reverse_iterator<std::vector<int>::iterator>(data.end());

    REQUIRE(rit.base() == data.end());
    REQUIRE(rit == reverse_iterator<std::vector<int>::iterator>(data.end()));
    REQUIRE(rit != reverse_iterator<std::vector<int>::iterator>(data.begin()));
    REQUIRE(rit < reverse_iterator<std::vector<int>::iterator>(data.begin()));
    REQUIRE(rit >= reverse_iterator<std::vector<int>::iterator>(data.end()));
    REQUIRE_FALSE(rit < reverse_iterator<std::vector<int>::iterator>(data.end()));
}
