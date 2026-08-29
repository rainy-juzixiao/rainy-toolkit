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
#include <rainy/core/memory/nebula_ptr.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <numeric>
#include <string>
#include <vector>


namespace fm = rainy::core::memory;

TEST_CASE("default_deleter default construction", "[nebula_ptr][default_deleter]") {
    fm::default_deleter<int> d;
    (void)d;
}

TEST_CASE("default_deleter placeholder construction", "[nebula_ptr][default_deleter]") {
    fm::default_deleter<int> d(std::in_place);
    (void)d;
}

TEST_CASE("default_deleter converting construction", "[nebula_ptr][default_deleter]") {
    // Derived-to-base conversion of deleter
    fm::default_deleter<int> d;
    fm::default_deleter<const int> d_const(d);
    (void)d_const;

    // array → array conversion
    fm::default_deleter<int[]> d_arr;
    fm::default_deleter<const int[]> d_carr(d_arr);
    (void)d_carr;

    // non-array → array conversion
    fm::default_deleter<int[]> d_from_nonarr(d);
    (void)d_from_nonarr;
}

TEST_CASE("nebula_ptr default construction is empty", "[nebula_ptr]") {
    fm::nebula_ptr<int> p;
    REQUIRE_FALSE(p);
    REQUIRE(p.empty());
    REQUIRE(p.get() == nullptr);
}

TEST_CASE("nebula_ptr nullptr construction", "[nebula_ptr]") {
    fm::nebula_ptr<int> p(nullptr);
    REQUIRE_FALSE(p);
    REQUIRE(p.empty());
}

TEST_CASE("nebula_ptr pointer construction", "[nebula_ptr]") {
    auto* raw = new int(42);
    fm::nebula_ptr<int> p(raw);
    REQUIRE(p);
    REQUIRE_FALSE(p.empty());
    REQUIRE(p.get() == raw);
    REQUIRE(*p == 42);
    // destructor will delete raw
}

TEST_CASE("nebula_ptr pointer with deleter construction", "[nebula_ptr]") {
    auto* raw = new int(99);
    fm::default_deleter<int> d;
    fm::nebula_ptr<int> p(raw, d);
    REQUIRE(p);
    REQUIRE(*p == 99);
}

TEST_CASE("nebula_ptr move construction", "[nebula_ptr]") {
    auto* raw = new int(7);
    fm::nebula_ptr<int> p1(raw);
    fm::nebula_ptr<int> p2(std::move(p1));

    // p1 is now empty
    REQUIRE_FALSE(p1);
    REQUIRE(p1.get() == nullptr);

    // p2 owns the resource
    REQUIRE(p2);
    REQUIRE(p2.get() == raw);
    REQUIRE(*p2 == 7);
}

TEST_CASE("nebula_ptr move assignment", "[nebula_ptr]") {
    auto* raw1 = new int(10);
    auto* raw2 = new int(20);

    fm::nebula_ptr<int> p1(raw1);
    fm::nebula_ptr<int> p2(raw2);

    p2 = std::move(p1);
    // p2 now owns raw1 (raw2 should have been deleted)
    REQUIRE(p2.get() == raw1);
    REQUIRE(*p2 == 10);
    REQUIRE_FALSE(p1);
}

TEST_CASE("nebula_ptr nullptr assignment", "[nebula_ptr]") {
    auto* raw = new int(5);
    fm::nebula_ptr<int> p(raw);
    REQUIRE(p);

    p = nullptr;
    REQUIRE_FALSE(p);
    REQUIRE(p.get() == nullptr);
}

TEST_CASE("nebula_ptr raw pointer assignment", "[nebula_ptr]") {
    fm::nebula_ptr<int> p(new int(1));
    auto* raw2 = new int(2);

    p = raw2; // deletes old resource, takes new one
    REQUIRE(*p == 2);
}

TEST_CASE("nebula_ptr get returns pointer", "[nebula_ptr]") {
    auto* raw = new int(100);
    fm::nebula_ptr<int> p(raw);
    REQUIRE(p.get() == raw);
}

TEST_CASE("nebula_ptr operator->", "[nebula_ptr]") {
    struct S { int x; };
    auto* raw = new S{42};
    fm::nebula_ptr<S> p(raw);
    REQUIRE(p->x == 42);
}

TEST_CASE("nebula_ptr operator*", "[nebula_ptr]") {
    auto* raw = new int(77);
    fm::nebula_ptr<int> p(raw);
    REQUIRE(*p == 77);
}

TEST_CASE("nebula_ptr as_reference", "[nebula_ptr]") {
    auto* raw = new double(3.14);
    fm::nebula_ptr<double> p(raw);
    REQUIRE(p.as_reference() == Catch::Approx(3.14));
}

TEST_CASE("nebula_ptr get_deleter returns reference to deleter", "[nebula_ptr]") {
    fm::nebula_ptr<int> p(new int(0));
    auto& d = p.get_deleter();
    (void)d; // deleter is default_deleter<int>
}

TEST_CASE("nebula_ptr reset with pointer", "[nebula_ptr]") {
    auto* old = new int(1);
    auto* newp = new int(2);
    fm::nebula_ptr<int> p(old);
    p.reset(newp);
    REQUIRE(p.get() == newp);
    REQUIRE(*p == 2);
    // old is deleted inside reset
}

TEST_CASE("nebula_ptr reset with nullptr", "[nebula_ptr]") {
    auto* raw = new int(1);
    fm::nebula_ptr<int> p(raw);
    p.reset(); // reset to null
    REQUIRE_FALSE(p);
}

TEST_CASE("nebula_ptr release transfers ownership", "[nebula_ptr]") {
    auto* raw = new int(42);
    fm::nebula_ptr<int> p(raw);
    int* released = p.release();
    REQUIRE(released == raw);
    REQUIRE_FALSE(p);
    REQUIRE(p.get() == nullptr);

    // must manually delete since we released
    delete released;
}

TEST_CASE("nebula_ptr swap", "[nebula_ptr]") {
    auto* raw1 = new int(1);
    auto* raw2 = new int(2);
    fm::nebula_ptr<int> p1(raw1);
    fm::nebula_ptr<int> p2(raw2);

    p1.swap(p2);
    REQUIRE(p1.get() == raw2);
    REQUIRE(p2.get() == raw1);
}

TEST_CASE("nebula_ptr derived-to-base conversion", "[nebula_ptr]") {
    struct Base { virtual ~Base() = default; virtual int value() const { return 0; } };
    struct Derived : Base { int value() const override { return 42; } };

    fm::nebula_ptr<Derived> derived(new Derived());
    fm::nebula_ptr<Base> base(std::move(derived));
    REQUIRE(base);
    REQUIRE(base->value() == 42);
    REQUIRE_FALSE(derived);
}

TEST_CASE("nebula_ptr destructor deletes owned resource", "[nebula_ptr]") {
    static int alive = 0;
    struct Tracker {
        Tracker() { ++alive; }
        ~Tracker() { --alive; }
    };

    REQUIRE(alive == 0);
    {
        fm::nebula_ptr<Tracker> p(new Tracker());
        REQUIRE(alive == 1);
    }
    REQUIRE(alive == 0);
}

TEST_CASE("nebula_ptr array default construction", "[nebula_ptr][array_spec]") {
    fm::nebula_ptr<int[]> p;
    REQUIRE_FALSE(p);
    REQUIRE(p.size() == 0);
    REQUIRE(p.length() == 0);
}

TEST_CASE("nebula_ptr array from pointer with length", "[nebula_ptr][array_spec]") {
    auto* arr = new int[5]{10, 20, 30, 40, 50};
    fm::nebula_ptr<int[]> p(arr, 5);
    REQUIRE(p);
    REQUIRE(p.size() == 5);
    REQUIRE(p.length() == 5);
    REQUIRE(p[0] == 10);
    REQUIRE(p[4] == 50);
}

TEST_CASE("nebula_ptr array operator[]", "[nebula_ptr][array_spec]") {
    auto* raw = new int[3]{100, 200, 300};
    fm::nebula_ptr<int[]> p(raw, 3);
    REQUIRE(p[0] == 100);
    REQUIRE(p[1] == 200);
    REQUIRE(p[2] == 300);

    p[1] = 999;
    REQUIRE(p[1] == 999);
}

TEST_CASE("nebula_ptr array at() with range check", "[nebula_ptr][array_spec]") {
    auto* raw = new int[3]{5, 6, 7};
    fm::nebula_ptr<int[]> p(raw, 3);
    REQUIRE(p.at(0) == 5);
    REQUIRE(p.at(2) == 7);
    // Out-of-range access should throw
    REQUIRE_THROWS_AS(p.at(5), rainy::core::exceptions::logic::out_of_range);
}

TEST_CASE("nebula_ptr array begin/end", "[nebula_ptr][array_spec]") {
    auto* raw = new int[4]{2, 4, 6, 8};
    fm::nebula_ptr<int[]> p(raw, 4);
    REQUIRE(p.begin() == raw);
    REQUIRE(p.end() == raw + 4);
    REQUIRE(std::distance(p.begin(), p.end()) == 4);
    REQUIRE(std::accumulate(p.begin(), p.end(), 0) == 20);
}

TEST_CASE("nebula_ptr array fill", "[nebula_ptr][array_spec]") {
    auto* raw = new int[5]{};
    fm::nebula_ptr<int[]> p(raw, 5);
    p.fill(7);
    for (std::size_t i = 0; i < 5; ++i) {
        REQUIRE(p[i] == 7);
    }
}

TEST_CASE("nebula_ptr array fill_with_ilist", "[nebula_ptr][array_spec]") {
    auto* raw = new int[5]{};
    fm::nebula_ptr<int[]> p(raw, 5);
    p.fill_with_ilist({10, 20, 30});
    REQUIRE(p[0] == 10);
    REQUIRE(p[1] == 20);
    REQUIRE(p[2] == 30);
    REQUIRE(p[3] == 0);
    REQUIRE(p[4] == 0);
}

TEST_CASE("nebula_ptr array fill_with_ilist overflow", "[nebula_ptr][array_spec]") {
    auto* raw = new int[3]{};
    fm::nebula_ptr<int[]> p(raw, 3);
    p.fill_with_ilist({1, 2, 3, 4, 5}); // only copies min(3, 5) = 3
    REQUIRE(p[0] == 1);
    REQUIRE(p[1] == 2);
    REQUIRE(p[2] == 3);
}

TEST_CASE("nebula_ptr array reset with pointer", "[nebula_ptr][array_spec]") {
    auto* old = new int[2]{1, 2};
    fm::nebula_ptr<int[]> p(old, 2);
    auto* newp = new int[3]{3, 4, 5};
    p.reset(newp, 3);
    REQUIRE(p.size() == 3);
    REQUIRE(p[0] == 3);
    REQUIRE(p[2] == 5);
}

TEST_CASE("nebula_ptr array move assignment", "[nebula_ptr][array_spec]") {
    auto* raw1 = new int[2]{1, 2};
    auto* raw2 = new int[3]{3, 4, 5};
    fm::nebula_ptr<int[]> p1(raw1, 2);
    fm::nebula_ptr<int[]> p2(raw2, 3);

    p2 = std::move(p1);
    REQUIRE(p2.size() == 2);
    REQUIRE(p2[0] == 1);
    REQUIRE(p2[1] == 2);
    REQUIRE_FALSE(p1);
}

TEST_CASE("nebula_ptr array nullptr assignment", "[nebula_ptr][array_spec]") {
    auto* raw = new int[3]{1, 2, 3};
    fm::nebula_ptr<int[]> p(raw, 3);
    p = nullptr;
    REQUIRE_FALSE(p);
    REQUIRE(p.size() == 0);
}

TEST_CASE("nebula_ptr array destructor deletes array", "[nebula_ptr][array_spec]") {
    static int alive = 0;
    struct Tracker {
        Tracker() { ++alive; }
        ~Tracker() { --alive; }
    };

    REQUIRE(alive == 0);
    {
        auto* arr = new Tracker[5];
        fm::nebula_ptr<Tracker[]> p(arr, 5);
        REQUIRE(alive == 5);
    }
    REQUIRE(alive == 0);
}

TEST_CASE("make_nebula for single object", "[nebula_ptr][make_nebula]") {
    auto p = fm::make_nebula<int>(42);
    REQUIRE(p);
    REQUIRE(*p == 42);
}

TEST_CASE("make_nebula for custom type", "[nebula_ptr][make_nebula]") {
    struct Point {
        Point(int x, int y) : x_(x), y_(y) {}
        int x_, y_;
    };

    auto p = fm::make_nebula<Point>(3, 4);
    REQUIRE(p->x_ == 3);
    REQUIRE(p->y_ == 4);
}

TEST_CASE("make_nebula for string", "[nebula_ptr][make_nebula]") {
    auto p = fm::make_nebula<std::string>("hello world");
    REQUIRE(*p == "hello world");
    REQUIRE(p->size() == 11);
}

TEST_CASE("make_nebula array default-constructs elements", "[nebula_ptr][make_nebula]") {
    auto p = fm::make_nebula<int[]>(3);
    REQUIRE(p);
    REQUIRE(p.size() == 3);
    // value-initialized to 0
    REQUIRE(p[0] == 0);
    REQUIRE(p[2] == 0);
}

TEST_CASE("make_nebula array with initializer list", "[nebula_ptr][make_nebula]") {
    auto p = fm::make_nebula<int[]>(5, {10, 20, 30});
    REQUIRE(p);
    REQUIRE(p.size() == 5);
    REQUIRE(p[0] == 10);
    REQUIRE(p[1] == 20);
    REQUIRE(p[2] == 30);
    REQUIRE(p[3] == 0);
    REQUIRE(p[4] == 0);
}

TEST_CASE("make_nebula array with initializer list larger than size", "[nebula_ptr][make_nebula]") {
    auto p = fm::make_nebula<int[]>(3, {1, 2, 3, 4, 5});
    REQUIRE(p.size() == 3);
    REQUIRE(p[0] == 1);
    REQUIRE(p[1] == 2);
    REQUIRE(p[2] == 3);
}

TEST_CASE("make_nebula array with initializer list empty", "[nebula_ptr][make_nebula]") {
    auto p = fm::make_nebula<int[]>(3);
    REQUIRE(p.size() == 3);
    // default-initialized → indeterminate for ints; just verify size
}

TEST_CASE("make_unique single object", "[nebula_ptr][make_unique]") {
    auto p = fm::make_unique<int>(99);
    REQUIRE(p);
    REQUIRE(*p == 99);
}

TEST_CASE("make_unique array", "[nebula_ptr][make_unique]") {
    auto p = fm::make_unique<int[]>(4);
    REQUIRE(p);
    REQUIRE(p.size() == 4);
}

TEST_CASE("make_unique array with initializer list", "[nebula_ptr][make_unique]") {
    auto p = fm::make_unique<int[]>(5, {1, 2, 3});
    REQUIRE(p[0] == 1);
    REQUIRE(p[2] == 3);
    REQUIRE(p[3] == 0);
}

TEST_CASE("unique_ptr is identical to nebula_ptr", "[nebula_ptr][unique_ptr]") {
    STATIC_REQUIRE(std::is_same_v<fm::unique_ptr<int>, fm::nebula_ptr<int>>);
    STATIC_REQUIRE(std::is_same_v<fm::unique_ptr<int[]>, fm::nebula_ptr<int[]>>);
}

TEST_CASE("nebula_ptr with const type", "[nebula_ptr]") {
    auto* raw = new const int(42);
    fm::nebula_ptr<const int> p(raw);
    REQUIRE(*p == 42);
    REQUIRE(p.get() == raw);
}

TEST_CASE("nebula_ptr array constness", "[nebula_ptr][array_spec]") {
    auto* raw = new int[2]{1, 2};
    const fm::nebula_ptr<int[]> p(raw, 2);
    REQUIRE(p[0] == 1);
    REQUIRE(p[1] == 2);
    REQUIRE(p.begin() == raw);
    REQUIRE(p.end() == raw + 2);
}
