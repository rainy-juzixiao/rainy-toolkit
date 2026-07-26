#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/memory/temporary_buffer.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

using namespace rainy;
using namespace rainy::core;
using namespace rainy::core::memory;

struct TestPOD {
    int value;
    double data;

    bool operator==(const TestPOD &other) const {
        return value == other.value && data == other.data;
    }
};

static_assert(type_traits::properties::is_pod_v<TestPOD>, "TestPOD must be POD");

TEST_CASE("temporary_buffer construction and destruction", "[temporary_buffer]") {
    SECTION("Construct with existing buffer") {
        constexpr std::ptrdiff_t count = 5;
        constexpr std::ptrdiff_t capacity = 10;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));

        temporary_buffer<int> buf(raw_buffer, count, capacity);

        REQUIRE(buf.begin() == raw_buffer);
        REQUIRE(buf.end() == raw_buffer + count);
        REQUIRE(buf.get_buffer() == raw_buffer);
        REQUIRE(static_cast<bool>(buf) == true);
        REQUIRE(static_cast<int *>(buf) == raw_buffer);

        buf.return_buffer();
    }

    SECTION("Move constructor") {
        constexpr std::ptrdiff_t count = 5;
        constexpr std::ptrdiff_t capacity = 10;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));

        temporary_buffer<int> original(raw_buffer, count, capacity);
        temporary_buffer<int> moved(std::move(original));

        REQUIRE(moved.get_buffer() == raw_buffer);
        REQUIRE(moved.begin() == raw_buffer);
        REQUIRE(moved.end() == raw_buffer + count);
        REQUIRE(original.get_buffer() == nullptr);
        REQUIRE(static_cast<bool>(original) == false);

        moved.return_buffer();
    }

    SECTION("Move assignment") {
        constexpr std::ptrdiff_t count = 5;
        constexpr std::ptrdiff_t capacity = 10;
        auto *raw_buffer1 = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));
        auto *raw_buffer2 = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));

        temporary_buffer<int> buf1(raw_buffer1, count, capacity);
        temporary_buffer<int> buf2(raw_buffer2, count, capacity);

        buf1 = std::move(buf2);

        REQUIRE(buf1.get_buffer() == raw_buffer2);
        REQUIRE(buf2.get_buffer() == nullptr);
        REQUIRE(static_cast<bool>(buf2) == false);

        buf1.return_buffer();
    }
}

TEST_CASE("temporary_buffer with POD types", "[temporary_buffer]") {
    SECTION("Integer buffer") {
        constexpr std::ptrdiff_t count = 10;
        constexpr std::ptrdiff_t capacity = 12;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));

        for (std::ptrdiff_t i = 0; i < count; ++i) {
            raw_buffer[i] = static_cast<int>(i * 2);
        }

        temporary_buffer<int> buf(raw_buffer, count, capacity);

        auto it = buf.begin();
        REQUIRE(*it == 0);
        ++it;
        REQUIRE(*it == 2);

        std::vector<int> expected = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
        std::vector<int> actual(buf.begin(), buf.end());
        REQUIRE(actual == expected);

        const auto &const_buf = buf;
        auto const_it = const_buf.begin();
        REQUIRE(*const_it == 0);

        buf.return_buffer();
    }
}

TEST_CASE("temporary_buffer reallocation", "[temporary_buffer]") {
    SECTION("Reallocate to larger size") {
        constexpr std::ptrdiff_t initial_count = 5;
        constexpr std::ptrdiff_t initial_capacity = 8;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * initial_capacity, alignof(int)));

        for (std::ptrdiff_t i = 0; i < initial_count; ++i) {
            raw_buffer[i] = static_cast<int>(i);
        }

        temporary_buffer<int> buf(raw_buffer, initial_count, initial_capacity);

        const auto old_buffer = buf.get_buffer();
        const std::size_t new_size = 20;

        buf.reallocate(new_size);

        REQUIRE(buf.get_buffer() != old_buffer);
        REQUIRE(buf.get_buffer() != nullptr);
        REQUIRE(buf.begin() == buf.get_buffer());
        REQUIRE(buf.end() == buf.get_buffer() + static_cast<std::ptrdiff_t>(new_size));

        for (std::ptrdiff_t i = 0; i < std::min(initial_count, static_cast<std::ptrdiff_t>(new_size)); ++i) {
            REQUIRE(buf.get_buffer()[i] == static_cast<int>(i));
        }

        buf.return_buffer();
    }

    SECTION("Reallocate to smaller or equal size should not reallocate") {
        constexpr std::ptrdiff_t initial_count = 10;
        constexpr std::ptrdiff_t initial_capacity = 15;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * initial_capacity, alignof(int)));

        temporary_buffer<int> buf(raw_buffer, initial_count, initial_capacity);
        const auto old_buffer = buf.get_buffer();

        const std::size_t smaller_size = 5;
        buf.reallocate(smaller_size);

        REQUIRE(buf.get_buffer() == old_buffer);
        REQUIRE(buf.begin() == old_buffer);
        REQUIRE(buf.end() == old_buffer + static_cast<std::ptrdiff_t>(smaller_size));
        REQUIRE(static_cast<std::ptrdiff_t>(smaller_size) == buf.end() - buf.begin());

        const std::size_t same_size = static_cast<std::size_t>(initial_count);
        buf.reallocate(same_size);

        REQUIRE(buf.get_buffer() == old_buffer);
        REQUIRE(static_cast<std::ptrdiff_t>(same_size) == buf.end() - buf.begin());

        buf.return_buffer();
    }

    SECTION("Reallocate to zero should not reallocate") {
        constexpr std::ptrdiff_t initial_count = 5;
        constexpr std::ptrdiff_t initial_capacity = 10;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * initial_capacity, alignof(int)));

        temporary_buffer<int> buf(raw_buffer, initial_count, initial_capacity);
        const auto old_buffer = buf.get_buffer();

        buf.reallocate(0);

        REQUIRE(buf.get_buffer() == old_buffer);
        REQUIRE(buf.end() - buf.begin() == 0);

        buf.return_buffer();
    }
}

TEST_CASE("temporary_buffer edge cases", "[temporary_buffer]") {
    SECTION("Multiple return_buffer calls") {
        constexpr std::ptrdiff_t count = 5;
        constexpr std::ptrdiff_t capacity = 10;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));

        temporary_buffer<int> buf(raw_buffer, count, capacity);

        buf.return_buffer();
        REQUIRE(buf.get_buffer() == nullptr);
        REQUIRE(buf.begin() == nullptr);
        REQUIRE(buf.end() == nullptr);

        buf.return_buffer();
        REQUIRE(buf.get_buffer() == nullptr);
    }
}

TEST_CASE("get_temporary_buffer helper function", "[temporary_buffer]") {
    SECTION("Get buffer for small count") {
        const std::ptrdiff_t count = 10;
        auto buf = get_temporary_buffer<int>(count);

        REQUIRE(buf.get_buffer() != nullptr);
        REQUIRE(buf.begin() != nullptr);
        REQUIRE(buf.end() == buf.begin() + count);
        REQUIRE(static_cast<bool>(buf) == true);

        REQUIRE(buf.end() - buf.begin() == count);

        buf.return_buffer();
    }

    SECTION("Get buffer for custom POD type") {
        const std::ptrdiff_t count = 5;
        auto buf = get_temporary_buffer<TestPOD>(count);

        REQUIRE(buf.get_buffer() != nullptr);
        REQUIRE(buf.end() - buf.begin() == count);

        for (std::ptrdiff_t i = 0; i < count; ++i) {
            buf.get_buffer()[i] = TestPOD{static_cast<int>(i), static_cast<double>(i)};
        }

        for (std::ptrdiff_t i = 0; i < count; ++i) {
            REQUIRE(buf.get_buffer()[i].value == static_cast<int>(i));
            REQUIRE(buf.get_buffer()[i].data == Catch::Approx(static_cast<double>(i)));
        }

        buf.return_buffer();
    }
}

TEST_CASE("return_temporary_buffer helper function", "[temporary_buffer]") {
    SECTION("Return valid buffer") {
        const std::ptrdiff_t count = 10;
        auto buf = get_temporary_buffer<int>(count);

        REQUIRE(buf.get_buffer() != nullptr);

        return_temporary_buffer(buf);

        REQUIRE(buf.get_buffer() == nullptr);
        REQUIRE(buf.begin() == nullptr);
        REQUIRE(buf.end() == nullptr);
        REQUIRE(static_cast<bool>(buf) == false);
    }
}

TEST_CASE("temporary_buffer const correctness", "[temporary_buffer]") {
    SECTION("Const iterator access") {
        constexpr std::ptrdiff_t count = 5;
        constexpr std::ptrdiff_t capacity = 10;
        auto *raw_buffer = static_cast<int *>(layer::allocate(sizeof(int) * capacity, alignof(int)));

        for (std::ptrdiff_t i = 0; i < count; ++i) {
            raw_buffer[i] = static_cast<int>(i);
        }

        const temporary_buffer<int> buf(raw_buffer, count, capacity);

        auto it = buf.begin();
        REQUIRE(*it == 0);
        ++it;
        REQUIRE(*it == 1);

        const int *const_ptr = buf.get_buffer();
        REQUIRE(const_ptr == raw_buffer);

        const int *implicit_ptr = buf;
        REQUIRE(implicit_ptr == raw_buffer);
    }
}

TEST_CASE("temporary_buffer iteration and algorithms", "[temporary_buffer]") {
    SECTION("Use with STL algorithms") {
        const std::ptrdiff_t count = 10;
        auto buf = get_temporary_buffer<int>(count);

        std::iota(buf.begin(), buf.end(), 1);

        int sum = std::accumulate(buf.begin(), buf.end(), 0);
        REQUIRE(sum == 55);

        auto it = std::find(buf.begin(), buf.end(), 5);
        REQUIRE(it != buf.end());
        REQUIRE(*it == 5);

        auto *reversed_buf = static_cast<int *>(layer::allocate(sizeof(int) * count, alignof(int)));
        temporary_buffer<int> reversed(reversed_buf, count, count);
        std::copy(buf.begin(), buf.end(), reversed.begin());
        std::reverse(reversed.begin(), reversed.end());

        REQUIRE(*reversed.begin() == 10);
        REQUIRE(*(reversed.end() - 1) == 1);

        buf.return_buffer();
        reversed.return_buffer();
    }
}

TEST_CASE("temporary_buffer reallocation preserves data", "[temporary_buffer]") {
    SECTION("Reallocate to larger size preserves all data") {
        const std::ptrdiff_t initial_count = 15;
        auto buf = get_temporary_buffer<int>(initial_count);

        for (std::ptrdiff_t i = 0; i < initial_count; ++i) {
            buf.get_buffer()[i] = static_cast<int>(i * i);
        }

        const std::size_t new_size = 30;
        buf.reallocate(new_size);

        for (std::ptrdiff_t i = 0; i < initial_count; ++i) {
            REQUIRE(buf.get_buffer()[i] == static_cast<int>(i * i));
        }

        for (std::ptrdiff_t i = initial_count; i < static_cast<std::ptrdiff_t>(new_size); ++i) {
            buf.get_buffer()[i] = static_cast<int>(i * 3);
        }

        buf.return_buffer();
    }

    SECTION("Reallocate to smaller size preserves only requested data") {
        const std::ptrdiff_t initial_count = 20;
        auto buf = get_temporary_buffer<int>(initial_count);

        for (std::ptrdiff_t i = 0; i < initial_count; ++i) {
            buf.get_buffer()[i] = static_cast<int>(i + 100);
        }

        const std::size_t new_size = 10;
        buf.reallocate(new_size);

        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(new_size); ++i) {
            REQUIRE(buf.get_buffer()[i] == static_cast<int>(i + 100));
        }

        REQUIRE(buf.end() - buf.begin() == static_cast<std::ptrdiff_t>(new_size));

        buf.return_buffer();
    }
}
