#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/ring_buffer.hpp>

using namespace rainy::foundation::collections;

TEST_CASE("circular_buffer - overwrite oldest on push_back", "[circular_buffer]") {
    circular_buffer<int> cb(3);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    CHECK(cb.full());
    cb.push_back(4);
    CHECK(cb.size() == 3);
    CHECK(cb[0] == 2);
    CHECK(cb[1] == 3);
    CHECK(cb[2] == 4);
    CHECK(cb.front() == 2);
    CHECK(cb.back() == 4);
}

TEST_CASE("circular_buffer - overwrite oldest on push_front", "[circular_buffer]") {
    circular_buffer<int> cb(3);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_front(0);
    CHECK(cb.size() == 3);
    CHECK(cb[0] == 0);
    CHECK(cb[1] == 1);
    CHECK(cb[2] == 2);
    CHECK(cb.back() == 2);
}

TEST_CASE("circular_buffer - many pushes keep last n", "[circular_buffer]") {
    circular_buffer<int> cb(4);
    for (int i = 0; i < 100; ++i) {
        cb.push_back(i);
    }
    REQUIRE(cb.size() == 4);
    CHECK(cb[0] == 96);
    CHECK(cb[1] == 97);
    CHECK(cb[2] == 98);
    CHECK(cb[3] == 99);
    int expect = 96;
    for (int v: cb) {
        CHECK(v == expect);
        ++expect;
    }
}
