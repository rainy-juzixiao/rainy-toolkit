#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/singleton.hpp>

using namespace rainy::foundation::container;

namespace {
    struct counter {
        int value = 0;

        counter() {
            ++constructions;
        }

        static int constructions;
    };

    int counter::constructions = 0;
}

TEST_CASE("singleton - returns same instance", "[singleton]") {
    counter &first = singleton<counter>::instance();
    counter &second = singleton<counter>::instance();
    CHECK(&first == &second);
}

TEST_CASE("singleton - constructs only once", "[singleton]") {
    singleton<counter>::instance();
    const int after_first = counter::constructions;
    singleton<counter>::instance();
    singleton<counter>::instance();
    CHECK(counter::constructions == after_first);
}

TEST_CASE("singleton - instance is usable", "[singleton]") {
    singleton<counter>::instance().value = 42;
    CHECK(singleton<counter>::instance().value == 42);
}

TEST_CASE("singleton - distinct types are distinct instances", "[singleton]") {
    struct alpha {
        int value = 1;
    };
    struct beta {
        int value = 2;
    };
    CHECK(&singleton<alpha>::instance() != reinterpret_cast<void *>(&singleton<beta>::instance()));
    CHECK(singleton<alpha>::instance().value == 1);
    CHECK(singleton<beta>::instance().value == 2);
}

TEST_CASE("singleton - throwing constructor releases the registry", "[singleton]") {
    struct throwing {
        throwing() {
            throw rainy::core::exceptions::runtime::runtime_error("boom");
        }
    };
    struct after_throwing {
        int value = 7;
    };
    CHECK_THROWS(singleton<throwing>::instance());
    CHECK(singleton<after_throwing>::instance().value == 7);
}
