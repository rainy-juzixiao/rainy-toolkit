#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/dense_tombstone_map.hpp>
#include <string>

using namespace rainy::foundation::collections;

TEST_CASE("dense_tombstone_map - insert find erase", "[dense_tombstone_map]") {
    dense_tombstone_map<int, int> m;
    for (int i = 0; i < 16; ++i) {
        m.insert({i, i * 10});
    }
    CHECK(m.size() == 16);
    CHECK(m.erase(5) == 1);
    CHECK(m.erase(5) == 0);
    CHECK(m.size() == 15);
    CHECK_FALSE(m.contains(5));
    for (int i = 0; i < 16; ++i) {
        if (i == 5) {
            continue;
        }
        REQUIRE(m.contains(i));
        CHECK(m.at(i) == i * 10);
    }
}

TEST_CASE("dense_tombstone_map - interleaved insert and erase", "[dense_tombstone_map]") {
    dense_tombstone_map<int, int> m;
    for (int round = 0; round < 8; ++round) {
        for (int i = 0; i < 50; ++i) {
            m.insert_or_assign(round * 100 + i, i);
        }
        for (int i = 0; i < 50; i += 3) {
            m.erase(round * 100 + i);
        }
    }
    for (int round = 0; round < 8; ++round) {
        for (int i = 0; i < 50; ++i) {
            const int key = round * 100 + i;
            if (i % 3 == 0) {
                CHECK_FALSE(m.contains(key));
            } else {
                REQUIRE(m.contains(key));
                CHECK(m.at(key) == i);
            }
        }
    }
}

TEST_CASE("dense_tombstone_map - iteration skips dead slots", "[dense_tombstone_map]") {
    dense_tombstone_map<int, int> m;
    for (int i = 0; i < 64; ++i) {
        m.insert({i, i});
    }
    for (int i = 0; i < 64; i += 2) {
        m.erase(i);
    }
    int seen = 0;
    for (auto &kv: m) {
        CHECK(kv.first % 2 == 1);
        ++seen;
    }
    CHECK(seen == 32);
    CHECK(m.size() == 32);
}

TEST_CASE("dense_tombstone_map - operator[] and at", "[dense_tombstone_map]") {
    dense_tombstone_map<std::string, int> m;
    m["a"] = 1;
    m["b"] = 2;
    CHECK(m.at("a") == 1);
    CHECK(m["b"] == 2);
    CHECK_THROWS_AS(m.at("missing"), rainy::core::exceptions::logic::out_of_range);
}

TEST_CASE("dense_tombstone_map - copy move", "[dense_tombstone_map]") {
    dense_tombstone_map<int, int> m;
    for (int i = 0; i < 20; ++i) {
        m.insert({i, i});
    }
    for (int i = 0; i < 20; i += 2) {
        m.erase(i);
    }
    dense_tombstone_map<int, int> c = m;
    CHECK(c.size() == m.size());
    for (int i = 0; i < 20; ++i) {
        CHECK(c.contains(i) == m.contains(i));
    }
    dense_tombstone_map<int, int> mv = std::move(m);
    CHECK(mv.size() == c.size());
}
