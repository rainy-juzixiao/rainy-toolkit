#include <catch2/catch_all.hpp>
#include <rainy/core/memory/shared_ptr.hpp>
#include <rainy/foundation/container/not_null.hpp>

using namespace rainy::foundation::container;

TEST_CASE("not_null - construct from pointer", "[not_null]") {
    int value = 42;
    not_null<int *> ptr(&value);
    CHECK(ptr.get() == &value);
    CHECK(*ptr == 42);
    *ptr = 7;
    CHECK(value == 7);
}

TEST_CASE("not_null - construct from nullptr is rejected", "[not_null]") {
    CHECK_FALSE((std::is_constructible_v<not_null<int *>, std::nullptr_t>) );
}

TEST_CASE("not_null - implicit conversion to pointer", "[not_null]") {
    int value = 1;
    not_null<int *> ptr(&value);
    int *raw = ptr;
    CHECK(raw == &value);
}

TEST_CASE("not_null - member access", "[not_null]") {
    struct widget {
        int field = 5;
    };
    widget w;
    not_null<widget *> ptr(&w);
    CHECK(ptr->field == 5);
}

TEST_CASE("not_null - copy and equality", "[not_null]") {
    int value = 3;
    not_null<int *> a(&value);
    not_null<int *> b = a;
    CHECK(a == b);
    int other = 4;
    not_null<int *> c(&other);
    CHECK(a != c);
}

TEST_CASE("not_null - smart pointer", "[not_null]") {
    auto sp = rainy::core::memory::make_shared<int>(9);
    not_null<rainy::core::memory::shared_ptr<int>> ptr(sp);
    CHECK(ptr.get() == sp);
    CHECK(*ptr == 9);
}

TEST_CASE("not_null - no pointer arithmetic", "[not_null]") {
    int arr[3] = {1, 2, 3};
    not_null<int *> ptr(arr);
    CHECK(*ptr == 1);
    CHECK_FALSE(std::is_assignable_v<not_null<int *> &, std::nullptr_t>);
}

TEST_CASE("not_null - CTAD", "[not_null]") {
    int value = 11;
    not_null ptr(&value);
    CHECK(*ptr == 11);
}
