#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/slot.hpp>
#include <string>

using namespace rainy::foundation::container;

TEST_CASE("slot - default is empty", "[slot]") {
    slot<int> value;
    CHECK_FALSE(value.has_value());
    CHECK_FALSE(static_cast<bool>(value));
}

TEST_CASE("slot - construct with value", "[slot]") {
    slot<int> value(5);
    CHECK(value.has_value());
    CHECK(value.get() == 5);
    CHECK(*value == 5);
}

TEST_CASE("slot - emplace", "[slot]") {
    slot<std::string> value;
    value.emplace("hello");
    CHECK(value.has_value());
    CHECK(*value == "hello");
    value.emplace(3, 'x');
    CHECK(*value == "xxx");
}

TEST_CASE("slot - reset", "[slot]") {
    slot<int> value(5);
    value.reset();
    CHECK_FALSE(value.has_value());
}

TEST_CASE("slot - value throws when empty", "[slot]") {
    slot<int> value;
    CHECK_THROWS_AS(value.value(), rainy::core::exceptions::logic::out_of_range);
}

TEST_CASE("slot - value_or", "[slot]") {
    slot<int> empty;
    CHECK(empty.value_or(9) == 9);
    slot<int> filled(3);
    CHECK(filled.value_or(9) == 3);
}

TEST_CASE("slot - copy and move", "[slot]") {
    slot<std::string> source(std::string("data"));
    slot<std::string> copied = source;
    CHECK(*copied == "data");
    CHECK(*source == "data");
    slot<std::string> moved = std::move(source);
    CHECK(*moved == "data");
    CHECK_FALSE(source.has_value());
}

TEST_CASE("slot - arrow access", "[slot]") {
    struct widget {
        int field = 4;
    };
    slot<widget> value(widget{});
    CHECK(value->field == 4);
}

TEST_CASE("slot - equality", "[slot]") {
    slot<int> a(1);
    slot<int> b(1);
    slot<int> c(2);
    slot<int> empty;
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a != empty);
    CHECK(empty == slot<int>{});
}

TEST_CASE("slot - self swap keeps the value", "[slot]") {
    slot<std::string> value(std::string("kept"));
    value.swap(value);
    CHECK(value.has_value());
    CHECK(*value == "kept");
}

TEST_CASE("slot - swap occupied and empty", "[slot]") {
    slot<std::string> occupied(std::string("data"));
    slot<std::string> empty;
    occupied.swap(empty);
    CHECK_FALSE(occupied.has_value());
    CHECK(empty.has_value());
    CHECK(*empty == "data");
    empty.swap(occupied);
    CHECK(occupied.has_value());
    CHECK_FALSE(empty.has_value());
}
