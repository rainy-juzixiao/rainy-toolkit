#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/dense_tombstone_set.hpp>
#include <string>

using namespace rainy::foundation::collections;

TEST_CASE("dense_tombstone_set - insert find erase", "[dense_tombstone_set]") {
    dense_tombstone_set<int> s;
    for (int i = 0; i < 16; ++i) {
        s.insert(i);
    }
    CHECK(s.size() == 16);
    CHECK(s.erase(5) == 1);
    CHECK(s.erase(5) == 0);
    CHECK(s.size() == 15);
    CHECK_FALSE(s.contains(5));
    for (int i = 0; i < 16; ++i) {
        if (i == 5) {
            continue;
        }
        CHECK(s.contains(i));
    }
}

TEST_CASE("dense_tombstone_set - interleaved insert erase", "[dense_tombstone_set]") {
    dense_tombstone_set<int> s;
    for (int round = 0; round < 8; ++round) {
        for (int i = 0; i < 50; ++i) {
            s.insert(round * 100 + i);
        }
        for (int i = 0; i < 50; i += 3) {
            s.erase(round * 100 + i);
        }
    }
    for (int round = 0; round < 8; ++round) {
        for (int i = 0; i < 50; ++i) {
            const int key = round * 100 + i;
            if (i % 3 == 0) {
                CHECK_FALSE(s.contains(key));
            } else {
                CHECK(s.contains(key));
            }
        }
    }
}

TEST_CASE("dense_tombstone_set - iteration and copy", "[dense_tombstone_set]") {
    dense_tombstone_set<int> s;
    for (int i = 0; i < 64; ++i) {
        s.insert(i);
    }
    for (int i = 0; i < 64; i += 2) {
        s.erase(i);
    }
    int seen = 0;
    for (int v: s) {
        CHECK(v % 2 == 1);
        ++seen;
    }
    CHECK(seen == 32);
    dense_tombstone_set<int> c = s;
    CHECK(c.size() == 32);
    for (int v: c) {
        CHECK(v % 2 == 1);
    }
}

TEST_CASE("dense_tombstone_set - transparent lookup", "[dense_tombstone_set]") {
    dense_tombstone_set<std::string> s;
    s.insert("hello");
    CHECK(s.find("hello") != s.end());
    CHECK(s.contains("hello"));
}
