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
#include <map>
#include <random>
#include <string>

using rainy::collections::map;

TEST_CASE("map - empty container", "[map]") {
    map<int, int> m;
    CHECK(m.empty());
    CHECK(m.size() == 0);
    CHECK(m.begin() == m.end());
    CHECK(m.find(1) == m.end());
    CHECK_FALSE(m.contains(1));
    CHECK(m.count(1) == 0);
}

TEST_CASE("map - insert and lookup", "[map]") {
    map<int, std::string> m;
    auto r1 = m.insert({3, "three"});
    auto r2 = m.insert({1, "one"});
    auto r3 = m.insert({2, "two"});
    CHECK(r1.second);
    CHECK(r2.second);
    CHECK(r3.second);
    CHECK(m.size() == 3);
    CHECK(m[1] == "one");
    CHECK(m.at(2) == "two");
    CHECK(m.contains(3));
    CHECK_FALSE(m.contains(4));
}

TEST_CASE("map - duplicate key rejected", "[map]") {
    map<int, std::string> m = {{1, "one"}, {2, "two"}};
    auto r = m.insert({2, "TWO"});
    CHECK_FALSE(r.second);
    CHECK(r.first->second == "two");
    CHECK(m.size() == 2);
}

TEST_CASE("map - iteration is sorted", "[map]") {
    map<int, int> m;
    for (int i = 9; i >= 0; --i) {
        m.insert({i, i * i});
    }
    int expected = 0;
    for (auto &kv: m) {
        CHECK(kv.first == expected);
        CHECK(kv.second == expected * expected);
        ++expected;
    }
    CHECK(expected == 10);
}

TEST_CASE("map - reverse iteration", "[map]") {
    map<int, int> m;
    for (int i = 0; i < 5; ++i) {
        m.insert({i, i});
    }
    auto it = m.end();
    int expected = 4;
    while (it != m.begin()) {
        --it;
        CHECK(it->first == expected);
        --expected;
    }
    CHECK(expected == -1);
}

TEST_CASE("map - erase by key and iterator", "[map]") {
    map<int, int> m;
    for (int i = 0; i < 10; ++i) {
        m.insert({i, i});
    }
    CHECK(m.erase(5) == 1);
    CHECK(m.erase(5) == 0);
    CHECK(m.size() == 9);
    CHECK_FALSE(m.contains(5));

    auto it = m.find(0);
    auto next = m.erase(it);
    CHECK(next->first == 1);
    CHECK(m.size() == 8);
    CHECK_FALSE(m.contains(0));
}

TEST_CASE("map - lower/upper bound and equal_range", "[map]") {
    map<int, int> m;
    for (int i = 0; i < 10; i += 2) {
        m.insert({i, i});
    }
    CHECK(m.lower_bound(3)->first == 4);
    CHECK(m.upper_bound(3)->first == 4);
    CHECK(m.lower_bound(4)->first == 4);
    CHECK(m.upper_bound(4)->first == 6);
    auto range = m.equal_range(4);
    CHECK(range.first->first == 4);
    CHECK(range.second->first == 6);
    CHECK(m.lower_bound(100) == m.end());
    CHECK(m.upper_bound(100) == m.end());
}

TEST_CASE("map - operator[] default constructs", "[map]") {
    map<std::string, int> m;
    CHECK(m["a"] == 0);
    m["a"] = 5;
    CHECK(m["a"] == 5);
    CHECK(m.size() == 1);
}

TEST_CASE("map - at throws out_of_range", "[map]") {
    map<int, int> m = {{1, 1}};
    CHECK_THROWS_AS(m.at(2), rainy::core::exceptions::logic::out_of_range);
}

TEST_CASE("map - try_emplace and insert_or_assign", "[map]") {
    map<int, std::string> m;
    auto r1 = m.try_emplace(1, "one");
    CHECK(r1.second);
    auto r2 = m.try_emplace(1, "ONE");
    CHECK_FALSE(r2.second);
    CHECK(m[1] == "one");

    auto r3 = m.insert_or_assign(1, "uno");
    CHECK_FALSE(r3.second);
    CHECK(m[1] == "uno");
    auto r4 = m.insert_or_assign(2, "dos");
    CHECK(r4.second);
    CHECK(m[2] == "dos");
}

TEST_CASE("map - copy and move", "[map]") {
    map<int, int> m;
    for (int i = 0; i < 20; ++i) {
        m.insert({i, i});
    }
    map<int, int> c = m;
    CHECK(c.size() == m.size());
    for (int i = 0; i < 20; ++i) {
        CHECK(c.at(i) == i);
    }
    map<int, int> mv = std::move(m);
    CHECK(mv.size() == c.size());
    CHECK(mv.at(7) == 7);
}

TEST_CASE("map - many elements keep ordering", "[map]") {
    map<int, int> m;
    for (int i = 0; i < 1000; ++i) {
        m.insert({i, i});
    }
    CHECK(m.size() == 1000);
    int expected = 0;
    for (auto &kv: m) {
        CHECK(kv.first == expected);
        ++expected;
    }
    for (int i = 0; i < 1000; i += 2) {
        m.erase(i);
    }
    CHECK(m.size() == 500);
    for (auto &kv: m) {
        CHECK(kv.first % 2 == 1);
    }
}

TEST_CASE("map - merge", "[map]") {
    map<int, int> a = {{1, 1}, {2, 2}, {3, 3}};
    map<int, int> b = {{2, 20}, {4, 4}};
    a.merge(b);
    CHECK(a.size() == 4);
    CHECK(a.at(2) == 2);
    CHECK(a.at(4) == 4);
    CHECK(b.size() == 1);
    CHECK(b.contains(2));
}

TEST_CASE("map - node extract and insert", "[map]") {
    map<int, std::string> m = {{1, "one"}, {2, "two"}};
    auto handle = m.extract(1);
    CHECK_FALSE(handle.empty());
    CHECK(handle.key() == 1);
    CHECK(handle.mapped() == "one");
    CHECK(m.size() == 1);
    CHECK_FALSE(m.contains(1));

    auto result = m.insert(std::move(handle));
    CHECK(result.inserted);
    CHECK(m.size() == 2);
    CHECK(m.at(1) == "one");
}

TEST_CASE("map - comparison operators", "[map]") {
    map<int, int> a = {{1, 1}, {2, 2}};
    map<int, int> b = {{1, 1}, {2, 2}};
    map<int, int> c = {{1, 1}, {3, 3}};
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a < c);
    CHECK(c > a);
    CHECK(a <= b);
    CHECK(a >= b);
}

TEST_CASE("map - swap and assignment", "[map]") {
    map<int, int> a = {{1, 1}, {2, 2}};
    map<int, int> b = {{9, 9}};
    a.swap(b);
    CHECK(a.size() == 1);
    CHECK(b.size() == 2);
    CHECK(a.contains(9));
    CHECK(b.contains(1));
    CHECK(b.begin()->first == 1);
    CHECK((++b.begin())->first == 2);

    map<int, int> c;
    c = b;
    CHECK(c.size() == 2);
    CHECK(c.at(2) == 2);

    map<int, int> d;
    d = std::move(c);
    CHECK(d.size() == 2);
    CHECK(d.contains(1));

    map<int, int> e = {{4, 4}, {5, 5}, {6, 6}};
    e = e;
    CHECK(e.size() == 3);
    CHECK(e.at(4) == 4);
}

TEST_CASE("map - differential against std::map", "[map]") {
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> key_dist(0, 200);
    std::uniform_int_distribution<int> op_dist(0, 9);

    map<int, int> ours;
    std::map<int, int> reference;

    for (int step = 0; step < 20000; ++step) {
        const int key = key_dist(rng);
        const int op = op_dist(rng);
        if (op < 5) {
            const int value = static_cast<int>(rng() % 1000);
            ours.insert({key, value});
            reference.insert({key, value});
        } else if (op < 7) {
            ours.erase(key);
            reference.erase(key);
        } else if (op < 9) {
            ours[key] = static_cast<int>(rng() % 1000);
            reference[key] = ours.at(key);
        } else {
            auto ours_it = ours.find(key);
            auto ref_it = reference.find(key);
            REQUIRE((ours_it == ours.end()) == (ref_it == reference.end()));
        }
        REQUIRE(ours.size() == reference.size());
    }

    REQUIRE(ours.size() == reference.size());
    auto ours_it = ours.begin();
    auto ref_it = reference.begin();
    for (; ref_it != reference.end(); ++ref_it, ++ours_it) {
        REQUIRE(ours_it != ours.end());
        CHECK(ours_it->first == ref_it->first);
        CHECK(ours_it->second == ref_it->second);
    }
    CHECK(ours_it == ours.end());
}
