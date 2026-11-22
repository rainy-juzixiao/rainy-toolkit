#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/cell.hpp>
#include <string>

using namespace rainy::foundation::container;

TEST_CASE("cell - construct and get", "[cell]") {
    cell<int> value(5);
    CHECK(value.get() == 5);
}

TEST_CASE("cell - set through non-const reference", "[cell]") {
    cell<int> value(1);
    value.set(10);
    CHECK(value.get() == 10);
}

TEST_CASE("cell - replace returns old value", "[cell]") {
    cell<int> value(5);
    CHECK(value.replace(10) == 5);
    CHECK(value.get() == 10);
}

TEST_CASE("cell - update with function", "[cell]") {
    cell<int> value(5);
    value.update([](int current) { return current + 1; });
    CHECK(value.get() == 6);
}

TEST_CASE("cell - swap", "[cell]") {
    cell<int> left(1);
    cell<int> right(2);
    left.swap(right);
    CHECK(left.get() == 2);
    CHECK(right.get() == 1);
}

TEST_CASE("cell - take leaves default", "[cell]") {
    cell<int> value(42);
    CHECK(value.take() == 42);
    CHECK(value.get() == 0);
}

TEST_CASE("cell - into_inner and get_mut", "[cell]") {
    cell<std::string> value(std::string("hello"));
    value.get_mut() += " world";
    CHECK(value.get() == "hello world");
    CHECK(value.into_inner() == "hello world");
}

TEST_CASE("cell - as_ptr", "[cell]") {
    cell<int> value(7);
    *value.as_ptr() = 9;
    CHECK(value.get() == 9);
}

TEST_CASE("cell - not copyable", "[cell]") {
    CHECK_FALSE(std::is_copy_constructible_v<cell<int>>);
    CHECK_FALSE(std::is_copy_assignable_v<cell<int>>);
}

namespace {
    struct move_constructible_only {
        int value = 0;

        move_constructible_only() = default;

        move_constructible_only(const move_constructible_only &) = delete;

        move_constructible_only &operator=(const move_constructible_only &) = delete;

        move_constructible_only(move_constructible_only &&) noexcept = default;

        move_constructible_only &operator=(move_constructible_only &&) = delete;
    };
}

TEST_CASE("cell - take works without assignment", "[cell]") {
    cell<move_constructible_only> value(move_constructible_only{});
    value.get_mut().value = 5;
    move_constructible_only taken = value.take();
    CHECK(taken.value == 5);
    CHECK(value.get_mut().value == 0);
}
