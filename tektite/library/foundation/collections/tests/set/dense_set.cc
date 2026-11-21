#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/dense_set.hpp>
#include <string>

using namespace rainy::foundation::collections;

TEST_CASE("dense_set - default construction", "[dense_set]") {
    dense_set<int> s;
    CHECK(s.empty());
    CHECK(s.size() == 0);
    CHECK(s.begin() == s.end());
}

TEST_CASE("dense_set - insert and find", "[dense_set]") {
    dense_set<int> s;
    auto [it, ok] = s.insert(1);
    CHECK(ok);
    CHECK(*it == 1);
    auto [it2, ok2] = s.insert(1);
    CHECK_FALSE(ok2);
    CHECK(s.size() == 1);
    CHECK(s.find(1) != s.end());
    CHECK(s.find(2) == s.end());
    CHECK(s.contains(1));
    CHECK_FALSE(s.contains(2));
    CHECK(s.count(1) == 1);
}

TEST_CASE("dense_set - erase", "[dense_set]") {
    dense_set<int> s;
    for (int i = 0; i < 16; ++i) {
        s.insert(i);
    }
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

TEST_CASE("dense_set - iteration after erase", "[dense_set]") {
    dense_set<int> s;
    for (int i = 0; i < 32; ++i) {
        s.insert(i);
    }
    for (int i = 0; i < 32; i += 2) {
        s.erase(i);
    }
    CHECK(s.size() == 16);
    int seen = 0;
    for (int v: s) {
        CHECK(v % 2 == 1);
        ++seen;
    }
    CHECK(seen == 16);
}

TEST_CASE("dense_set - transparent lookup", "[dense_set]") {
    dense_set<std::string> s;
    s.insert("hello");
    CHECK(s.find("hello") != s.end());
    CHECK(s.contains("hello"));
}

TEST_CASE("dense_set - copy move and initializer list", "[dense_set]") {
    dense_set<int> s{1, 2, 3, 3, 2};
    CHECK(s.size() == 3);
    dense_set<int> c = s;
    CHECK(c.size() == 3);
    dense_set<int> mv = std::move(s);
    CHECK(mv.size() == 3);
    CHECK(mv.contains(1));
    CHECK(mv.contains(2));
    CHECK(mv.contains(3));
}

TEST_CASE("dense_set - rehash", "[dense_set]") {
    dense_set<int> s;
    for (int i = 0; i < 400; ++i) {
        s.insert(i);
    }
    CHECK(s.size() == 400);
    s.rehash(1024);
    for (int i = 0; i < 400; ++i) {
        CHECK(s.contains(i));
    }
}
