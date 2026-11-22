#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/ring_buffer.hpp>

using namespace rainy::foundation::collections;

TEST_CASE("ring_buffer - empty and capacity", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    CHECK(rb.empty());
    CHECK_FALSE(rb.full());
    CHECK(rb.size() == 0);
    CHECK(rb.capacity() == 4);
}

TEST_CASE("ring_buffer - push_back and pop_front", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    CHECK(rb.size() == 3);
    CHECK(rb.front() == 1);
    CHECK(rb.back() == 3);
    rb.pop_front();
    CHECK(rb.front() == 2);
    CHECK(rb.size() == 2);
}

TEST_CASE("ring_buffer - push_front and pop_back", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    rb.push_front(1);
    rb.push_front(2);
    rb.push_front(3);
    CHECK(rb.size() == 3);
    CHECK(rb.front() == 3);
    CHECK(rb.back() == 1);
    rb.pop_back();
    CHECK(rb.back() == 2);
}

TEST_CASE("ring_buffer - throw_on_full", "[ring_buffer]") {
    ring_buffer<int> rb(2);
    rb.push_back(1);
    rb.push_back(2);
    CHECK(rb.full());
    CHECK_THROWS_AS(rb.push_back(3), rainy::core::exceptions::logic::length_error);
}

TEST_CASE("ring_buffer - wraparound index and iteration", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    for (int i = 0; i < 10; ++i) {
        if (rb.full()) {
            rb.pop_front();
        }
        rb.push_back(i);
    }
    REQUIRE(rb.size() == 4);
    CHECK(rb[0] == 6);
    CHECK(rb[1] == 7);
    CHECK(rb[2] == 8);
    CHECK(rb[3] == 9);
    int expect = 6;
    for (int v: rb) {
        CHECK(v == expect);
        ++expect;
    }
}

TEST_CASE("ring_buffer - at bounds", "[ring_buffer]") {
    ring_buffer<int> rb(3);
    rb.push_back(10);
    rb.push_back(20);
    CHECK(rb.at(0) == 10);
    CHECK(rb.at(1) == 20);
    CHECK_THROWS_AS(rb.at(2), rainy::core::exceptions::logic::out_of_range);
}

TEST_CASE("ring_buffer - reverse iteration", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    int expect = 3;
    for (auto it = rb.rbegin(); it != rb.rend(); ++it) {
        CHECK(*it == expect);
        --expect;
    }
    CHECK(expect == 0);
}

TEST_CASE("ring_buffer - copy and move", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    ring_buffer<int> c = rb;
    CHECK(c.size() == 3);
    CHECK(c[0] == 1);
    CHECK(c[2] == 3);
    ring_buffer<int> mv = std::move(rb);
    CHECK(mv.size() == 3);
    CHECK(mv[0] == 1);
}

TEST_CASE("ring_buffer - clear", "[ring_buffer]") {
    ring_buffer<int> rb(4);
    rb.push_back(1);
    rb.push_back(2);
    rb.clear();
    CHECK(rb.empty());
    CHECK(rb.size() == 0);
}
