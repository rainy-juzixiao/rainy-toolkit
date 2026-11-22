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
#include <rainy/core/utility/move_iterator.hpp>
#include <rainy/core/utility/reverse_iterator.hpp>

#include <string>
#include <type_traits>
#include <vector>

namespace {
    struct tracked_type {
        static inline int copies = 0;
        static inline int moves = 0;

        int payload{};

        tracked_type() = default;
        explicit tracked_type(int value) : payload(value) {
        }
        tracked_type(const tracked_type &other) : payload(other.payload) {
            ++copies;
        }
        tracked_type(tracked_type &&other) noexcept : payload(other.payload) {
            ++moves;
        }

        static void reset() {
            copies = 0;
            moves = 0;
        }
    };
}

TEST_CASE("move_iterator - reference type is an rvalue reference", "[utility][move_iterator]") {
    static_assert(std::is_same_v<rainy::utility::move_iterator<std::vector<int>::iterator>::reference, int &&>);
    static_assert(std::is_same_v<rainy::utility::move_iterator<std::vector<int>::iterator>::value_type, int>);
    static_assert(std::is_same_v<rainy::utility::move_iterator<std::vector<int>::iterator>::iterator_type, std::vector<int>::iterator>);
    REQUIRE(true);
}

TEST_CASE("move_iterator - traversal reads the underlying range in order", "[utility][move_iterator]") {
    std::vector<std::string> data{"a", "b", "c"};
    auto first = rainy::utility::make_move_iterator(data.begin());
    auto last = rainy::utility::make_move_iterator(data.end());

    std::string joined;
    for (auto it = first; it != last; ++it) {
        joined += *it;
    }
    REQUIRE(joined == "abc");
}

TEST_CASE("move_iterator - dereference yields an rvalue", "[utility][move_iterator]") {
    std::vector<tracked_type> data{tracked_type{1}, tracked_type{2}, tracked_type{3}};
    std::vector<tracked_type> target;
    target.reserve(data.size());

    tracked_type::reset();
    for (auto it = rainy::utility::make_move_iterator(data.begin()); it != rainy::utility::make_move_iterator(data.end()); ++it) {
        target.emplace_back(*it);
    }

    REQUIRE(target.size() == 3);
    REQUIRE(tracked_type::copies == 0);
    REQUIRE(tracked_type::moves == 3);
    REQUIRE(target[0].payload == 1);
    REQUIRE(target[2].payload == 3);
}

TEST_CASE("move_iterator - base access preserves and moves out the iterator", "[utility][move_iterator]") {
    std::vector<int> data{4, 5, 6};
    rainy::utility::move_iterator<std::vector<int>::iterator> it{data.begin()};
    REQUIRE(it.base() == data.begin());
    REQUIRE(it.base() == data.begin());
    auto moved = rainy::utility::move(it).base();
    REQUIRE(moved == data.begin());
}

TEST_CASE("move_iterator - comparison and arithmetic follow the underlying iterator", "[utility][move_iterator]") {
    std::vector<int> data{1, 2, 3, 4};
    auto begin = rainy::utility::make_move_iterator(data.begin());
    auto end = rainy::utility::make_move_iterator(data.end());
    auto it = begin;

    REQUIRE(begin == it);
    REQUIRE_FALSE(begin != it);
    REQUIRE(begin < end);
    REQUIRE(end > begin);
    REQUIRE(begin <= it);
    REQUIRE(begin >= it);
    REQUIRE(end - begin == 4);

    ++it;
    REQUIRE(*it == 2);
    --it;
    REQUIRE(*it == 1);

    it += 2;
    REQUIRE(*it == 3);
    it -= 1;
    REQUIRE(*it == 2);
    REQUIRE(it[1] == 3);
    REQUIRE(*(it + 1) == 3);
    REQUIRE(*(end - 3) == 2);
}

TEST_CASE("move_iterator - converting constructor and assignment", "[utility][move_iterator]") {
    std::vector<int> data{7, 8, 9};
    rainy::utility::move_iterator<std::vector<int>::iterator> source{data.begin()};
    rainy::utility::move_iterator<std::vector<int>::const_iterator> target{source};
    REQUIRE(target.base() == data.cbegin());
    REQUIRE(*target == 7);

    rainy::utility::move_iterator<std::vector<int>::const_iterator> assigned{data.cend()};
    assigned = source;
    REQUIRE(assigned.base() == data.cbegin());
}

TEST_CASE("move_iterator - interoperates with make_reverse_iterator", "[utility][move_iterator]") {
    std::vector<int> data{1, 2, 3};
    auto rbegin = rainy::utility::make_reverse_iterator(rainy::utility::make_move_iterator(data.end()));
    REQUIRE(*rbegin == 3);
    REQUIRE(rbegin[1] == 2);
}
