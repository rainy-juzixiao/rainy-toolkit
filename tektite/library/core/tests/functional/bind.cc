#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <rainy/core/functional/bind.hpp>

#include "../../../../../../../../../../usr/x86_64-w64-mingw32/include/c++/16.1.0/functional"

using namespace rainy::functional;
using namespace rainy::functional::placeholders;

int add(int a, int b) {
    return a + b;
}

TEST_CASE("binder basic functionality", "[bind]") {
    auto add = [](int a, int b) { return a + b; };
    auto bound = bind(add, 10, 20);
    REQUIRE(bound() == 30);
}

TEST_CASE("binder with placeholders", "[bind]") {
    auto add = [](int a, int b, int c) { return a + b + c; };
    auto bound = bind(add, _1, 5, _2);
    REQUIRE(bound(10, 20) == 35);
}

TEST_CASE("binder with multiple placeholders", "[bind]") {
    auto sub = [](int a, int b, int c, int d) { return a - b - c - d; };
    auto bound = bind(sub, _4, _3, _2, _1);
    REQUIRE(bound(1, 2, 3, 4) == -2);
}

TEST_CASE("binder with nested binder", "[bind]") {
    auto addinternal = [](int a, int b) { return a + b; };
    auto mulinternal = [](int a, int b) { return a * b; };
    auto inner = bind(addinternal, 3, 4);
    auto outer = bind(mulinternal, inner, 2);
    REQUIRE(outer() == 14);
}

TEST_CASE("binder with placeholder and nested binder", "[bind]") {
    auto add = [](int a, int b) { return a + b; };
    auto mul = [](int a, int b) { return a * b; };
    auto inner = bind(add, _1, 5);
    auto outer = bind(mul, inner, 2);
    REQUIRE(outer(3) == 16);
}

TEST_CASE("binder with lambda capturing", "[bind]") {
    int factor = 3;
    auto mul = [factor](int x) { return x * factor; };
    auto bound = bind(mul, 7);
    REQUIRE(bound() == 21);
}

TEST_CASE("binder with function pointer", "[bind]") {
    auto bound = bind(add, 100, 200);
    REQUIRE(bound() == 300);
}

TEST_CASE("binder with move-only types", "[bind]") {
    auto make = []() { return std::make_unique<int>(42); };
    auto bound = bind(make);
    auto result = bound();
    REQUIRE(*result == 42);
}

TEST_CASE("bind_front basic", "[bind_front]") {
    auto add = [](int a, int b, int c) { return a + b + c; };
    auto bound = bind_front(add, 1, 2);
    REQUIRE(bound(3) == 6);
}

TEST_CASE("bind_front with one argument", "[bind_front]") {
    auto mul = [](int a, int b) { return a * b; };
    auto bound = bind_front(mul, 5);
    REQUIRE(bound(6) == 30);
}

TEST_CASE("bind_front with zero arguments", "[bind_front]") {
    auto identity = [](int x) { return x; };
    auto bound = bind_front(identity);
    REQUIRE(bound(99) == 99);
}

TEST_CASE("binder with placeholders in different order", "[bind]") {
    auto concat = [](int a, int b, int c, int d) { return a * 1000 + b * 100 + c * 10 + d; };
    auto bound = bind(concat, _3, _1, _4, _2);
    REQUIRE(bound(2, 4, 1, 3) == 1234);
}

TEST_CASE("binder with duplicate placeholders", "[bind]") {
    auto sum3 = [](int a, int b, int c) { return a + b + c; };
    auto bound = bind(sum3, _1, _2, _1);
    REQUIRE(bound(5, 10) == 20);
}

TEST_CASE("binder with placeholders and bound values", "[bind]") {
    auto func = [](int a, int b, int c, int d) { return a + b + c + d; };
    auto bound = bind(func, 1, _2, 3, _4);
    REQUIRE(bound(10, 20, 30, 40) == 1 + 20 + 3 + 40);
}

TEST_CASE("bind_front with placeholder not allowed", "[bind_front]") {
    auto add = [](int a, int b) { return a + b; };
    auto bound = bind_front(add, 1);
    REQUIRE(bound(2) == 3);
}

TEST_CASE("binder with empty bound args", "[bind]") {
    auto func = []() { return 42; };
    auto bound = bind(func);
    REQUIRE(bound() == 42);
}

TEST_CASE("bind_front with empty bound args", "[bind_front]") {
    auto func = [](int x) { return x + 1; };
    auto bound = bind_front(func);
    REQUIRE(bound(41) == 42);
}

TEST_CASE("binder with custom type", "[bind]") {
    struct Point {
        int x, y;
        Point operator+(const Point &other) const {
            return {x + other.x, y + other.y};
        }
    };
    auto add = [](Point a, Point b) { return a + b; };
    auto bound = bind(add, Point{1, 2}, Point{3, 4});
    auto result = bound();
    REQUIRE(result.x == 4);
    REQUIRE(result.y == 6);
}

TEST_CASE("binder with reference to function", "[bind]") {
    auto bound = bind(add, 15, 25);
    REQUIRE(bound() == 40);
}

int mul(int a, int b) {
    return a * b;
}

TEST_CASE("bind_front with reference to function", "[bind_front]") {
    auto bound = bind_front(mul, 6);
    REQUIRE(bound(7) == 42);
}
