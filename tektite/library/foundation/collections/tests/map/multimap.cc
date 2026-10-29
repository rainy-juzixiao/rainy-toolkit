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
#include <rainy/foundation/collections/map.hpp>
#include <string>

using rainy::collections::multimap;

TEST_CASE("multimap - insert duplicates", "[multimap]") {
    multimap<int, std::string> m;
    m.insert({1, "one"});
    m.insert({1, "uno"});
    m.insert({2, "two"});
    CHECK(m.size() == 3);
    CHECK(m.count(1) == 2);
    CHECK(m.count(2) == 1);
    CHECK(m.count(3) == 0);
}

TEST_CASE("multimap - equal_range covers duplicates", "[multimap]") {
    multimap<int, int> m;
    for (int i = 0; i < 4; ++i) {
        m.insert({1, i});
    }
    m.insert({2, 9});
    auto range = m.equal_range(1);
    int seen = 0;
    for (auto it = range.first; it != range.second; ++it) {
        CHECK(it->first == 1);
        ++seen;
    }
    CHECK(seen == 4);
}

TEST_CASE("multimap - erase removes all duplicates", "[multimap]") {
    multimap<int, int> m;
    for (int i = 0; i < 5; ++i) {
        m.insert({7, i});
    }
    m.insert({8, 0});
    CHECK(m.erase(7) == 5);
    CHECK(m.size() == 1);
    CHECK_FALSE(m.contains(7));
}

TEST_CASE("multimap - iteration sorted with duplicates adjacent", "[multimap]") {
    multimap<int, int> m;
    m.insert({2, 0});
    m.insert({1, 1});
    m.insert({2, 2});
    m.insert({1, 3});
    int last = -1;
    for (auto &kv: m) {
        CHECK(kv.first >= last);
        last = kv.first;
    }
    CHECK(m.begin()->first == 1);
    auto it = m.begin();
    ++it;
    CHECK(it->first == 1);
    ++it;
    CHECK(it->first == 2);
}

TEST_CASE("multimap - copy and move", "[multimap]") {
    multimap<int, int> m;
    for (int i = 0; i < 5; ++i) {
        m.insert({i % 2, i});
    }
    multimap<int, int> c = m;
    CHECK(c.size() == m.size());
    CHECK(c.count(0) == 3);
    CHECK(c.count(1) == 2);
    multimap<int, int> mv = std::move(m);
    CHECK(mv.size() == c.size());
}

TEST_CASE("multimap - emplace", "[multimap]") {
    multimap<int, std::string> m;
    m.emplace(1, "a");
    m.emplace(1, "b");
    CHECK(m.size() == 2);
    CHECK(m.count(1) == 2);
}

TEST_CASE("multimap - comparison", "[multimap]") {
    multimap<int, int> a = {{1, 1}, {1, 2}};
    multimap<int, int> b = {{1, 1}, {1, 2}};
    multimap<int, int> c = {{1, 1}, {2, 2}};
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a < c);
}
