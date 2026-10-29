#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/dense_map.hpp>
#include <string>

using namespace rainy::foundation::collections;

TEST_CASE("dense_map - default construction", "[dense_map]") {
    dense_map<int, int> m;
    CHECK(m.empty());
    CHECK(m.size() == 0);
    CHECK(m.begin() == m.end());
}

TEST_CASE("dense_map - insert and find", "[dense_map]") {
    dense_map<int, int> m;
    auto [it, ok] = m.insert({1, 100});
    CHECK(ok);
    CHECK(it->first == 1);
    CHECK(it->second == 100);
    CHECK(m.size() == 1);

    auto [it2, ok2] = m.insert({1, 999});
    CHECK_FALSE(ok2);
    CHECK(it2->second == 100);
    CHECK(m.size() == 1);

    CHECK(m.find(1) != m.end());
    CHECK(m.find(2) == m.end());
    CHECK(m.contains(1));
    CHECK_FALSE(m.contains(2));
    CHECK(m.count(1) == 1);
    CHECK(m.count(2) == 0);
}

TEST_CASE("dense_map - operator[] and at", "[dense_map]") {
    dense_map<std::string, int> m;
    m["a"] = 1;
    m["b"] = 2;
    CHECK(m["a"] == 1);
    CHECK(m.at("b") == 2);
    m["a"] = 10;
    CHECK(m["a"] == 10);
    CHECK_THROWS_AS(m.at("missing"), rainy::core::exceptions::logic::out_of_range);
}

TEST_CASE("dense_map - emplace and try_emplace", "[dense_map]") {
    dense_map<int, std::string> m;
    auto [it, ok] = m.emplace(1, "one");
    CHECK(ok);
    CHECK(it->second == "one");
    auto [it2, ok2] = m.try_emplace(1, "uno");
    CHECK_FALSE(ok2);
    CHECK(it2->second == "one");
    auto [it3, ok3] = m.try_emplace(2, "two");
    CHECK(ok3);
    CHECK(it3->second == "two");
}

TEST_CASE("dense_map - insert_or_assign", "[dense_map]") {
    dense_map<int, int> m;
    auto [it, ok] = m.insert_or_assign(1, 1);
    CHECK(ok);
    CHECK(it->second == 1);
    auto [it2, ok2] = m.insert_or_assign(1, 2);
    CHECK_FALSE(ok2);
    CHECK(it2->second == 2);
}

TEST_CASE("dense_map - erase by key keeps other entries", "[dense_map]") {
    dense_map<int, int> m;
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

TEST_CASE("dense_map - erase range and iteration", "[dense_map]") {
    dense_map<int, int> m;
    for (int i = 0; i < 32; ++i) {
        m.insert({i, i});
    }
    for (int i = 0; i < 32; i += 2) {
        m.erase(i);
    }
    CHECK(m.size() == 16);
    int seen = 0;
    for (auto &kv: m) {
        CHECK(kv.first % 2 == 1);
        ++seen;
    }
    CHECK(seen == 16);
}

namespace {
    struct constant_hash {
        std::size_t operator()(int) const noexcept {
            return 0;
        }
    };
}

TEST_CASE("dense_map - backshift erase with all keys colliding", "[dense_map]") {
    dense_map<int, int, constant_hash> m;
    for (int i = 0; i < 64; ++i) {
        m.insert({i, i});
    }
    CHECK(m.size() == 64);
    CHECK(m.bucket_size(m.bucket(0)) == 64);

    m.erase(0);
    CHECK_FALSE(m.contains(0));
    CHECK(m.size() == 63);

    m.erase(63);
    CHECK_FALSE(m.contains(63));
    CHECK(m.size() == 62);

    m.erase(32);
    CHECK_FALSE(m.contains(32));
    CHECK(m.size() == 61);

    for (int i = 0; i < 64; ++i) {
        if (i == 0 || i == 63 || i == 32) {
            continue;
        }
        REQUIRE(m.contains(i));
        CHECK(m.at(i) == i);
    }
    int seen = 0;
    for (auto &kv: m) {
        CHECK(m.at(kv.first) == kv.first);
        ++seen;
    }
    CHECK(seen == 61);

    for (int i = 0; i < 64; ++i) {
        m.erase(i);
    }
    CHECK(m.empty());
}

TEST_CASE("dense_map - backshift erase adjacent same chain", "[dense_map]") {
    dense_map<int, int, constant_hash> m;
    for (int i = 0; i < 8; ++i) {
        m.insert({i, i * 100});
    }
    for (int i = 7; i >= 0; --i) {
        CHECK(m.erase(i) == 1);
        CHECK(m.size() == static_cast<std::size_t>(i));
        for (int j = 0; j < i; ++j) {
            REQUIRE(m.contains(j));
            CHECK(m.at(j) == j * 100);
        }
    }
    CHECK(m.empty());
}

TEST_CASE("dense_map - rehash and reserve", "[dense_map]") {
    dense_map<int, int> m;
    const auto before = m.bucket_count();
    m.reserve(1000);
    CHECK(m.bucket_count() >= before);
    for (int i = 0; i < 500; ++i) {
        m.insert({i, i});
    }
    CHECK(m.size() == 500);
    CHECK(m.load_factor() <= m.max_load_factor());
    for (int i = 0; i < 500; ++i) {
        CHECK(m.contains(i));
    }
    m.rehash(2048);
    CHECK(m.bucket_count() >= 2048);
    for (int i = 0; i < 500; ++i) {
        CHECK(m.contains(i));
    }
}

TEST_CASE("dense_map - transparent lookup", "[dense_map]") {
    dense_map<std::string, int> m;
    m["hello"] = 1;
    CHECK(m.find("hello") != m.end());
    CHECK(m.contains("hello"));
    CHECK(m.count("hello") == 1);
}

TEST_CASE("dense_map - copy and move", "[dense_map]") {
    dense_map<int, int> m;
    for (int i = 0; i < 20; ++i) {
        m.insert({i, i});
    }
    dense_map<int, int> c = m;
    CHECK(c.size() == m.size());
    for (int i = 0; i < 20; ++i) {
        CHECK(c.at(i) == i);
    }
    dense_map<int, int> mv = std::move(m);
    CHECK(mv.size() == 20);
    for (int i = 0; i < 20; ++i) {
        CHECK(mv.at(i) == i);
    }
}

TEST_CASE("dense_map - initializer list and bucket interface", "[dense_map]") {
    dense_map<int, int> m{{1, 1}, {2, 2}, {3, 3}};
    CHECK(m.size() == 3);
    CHECK(m.bucket_size(m.bucket(1)) >= 1);
    for (std::size_t b = 0; b < m.bucket_count(); ++b) {
        for (auto it = m.begin(b); it != m.end(b); ++it) {
            CHECK(m.contains(it->first));
        }
    }
}
