#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/atom.hpp>
#include <string>

using namespace rainy::foundation::container;

TEST_CASE("atom - trivial type uses lock-free path", "[atom]") {
    atom<int> value(1);
    CHECK(atom<int>::is_lock_free());
    CHECK(value.load() == 1);
    value.store(2);
    CHECK(value.load() == 2);
    CHECK(static_cast<int>(value) == 2);
}

TEST_CASE("atom - exchange", "[atom]") {
    atom<int> value(10);
    CHECK(value.exchange(20) == 10);
    CHECK(value.load() == 20);
}

TEST_CASE("atom - compare_exchange success and failure", "[atom]") {
    atom<int> value(5);
    int expected = 5;
    CHECK(value.compare_exchange(expected, 6));
    CHECK(value.load() == 6);
    int stale = 5;
    CHECK_FALSE(value.compare_exchange(stale, 7));
    CHECK(stale == 6);
}

TEST_CASE("atom - non-trivial type is mutex guarded", "[atom]") {
    atom<std::string> value(std::string("hello"));
    CHECK_FALSE(atom<std::string>::is_lock_free());
    CHECK(value.load() == "hello");
    value.store("world");
    CHECK(value.load() == "world");
}

TEST_CASE("atom - non-trivial exchange", "[atom]") {
    atom<std::string> value(std::string("a"));
    CHECK(value.exchange("b") == "a");
    CHECK(value.load() == "b");
}

TEST_CASE("atom - compare_exchange is strong", "[atom]") {
    atom<int> value(5);
    for (int i = 0; i < 1000; ++i) {
        int current = value.load();
        CHECK(value.compare_exchange(current, current));
        CHECK(current == value.load());
    }
    int stale = 4;
    CHECK_FALSE(value.compare_exchange(stale, 6));
    CHECK(stale == 5);
    CHECK(value.load() == 5);
}
