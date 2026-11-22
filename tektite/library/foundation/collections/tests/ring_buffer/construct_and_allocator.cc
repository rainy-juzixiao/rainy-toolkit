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
#include <catch2/catch_all.hpp>
#include <rainy/foundation/collections/ring_buffer.hpp>

#include <stdexcept>
#include <utility>
#include <vector>

using namespace rainy::foundation::collections;

namespace {
    struct allocation_registry {
        static inline std::vector<std::pair<void *, int>> owners;
        static inline int mismatches = 0;

        static void record(void *ptr, int id) {
            owners.emplace_back(ptr, id);
        }

        static void release(void *ptr, int id) {
            for (auto it = owners.begin(); it != owners.end(); ++it) {
                if (it->first == ptr) {
                    if (it->second != id) {
                        ++mismatches;
                    }
                    owners.erase(it);
                    return;
                }
            }
            ++mismatches;
        }

        static void reset() {
            owners.clear();
            mismatches = 0;
        }
    };

    template <typename T, bool Pocma = false, bool Pocs = false>
    struct test_allocator {
        using value_type = T;
        using propagate_on_container_move_assignment = std::integral_constant<bool, Pocma>;
        using propagate_on_container_swap = std::integral_constant<bool, Pocs>;
        using is_always_equal = std::false_type;

        int id = 0;

        test_allocator() = default;

        explicit test_allocator(int identifier) : id(identifier) {
        }

        template <typename U>
        test_allocator(const test_allocator<U, Pocma, Pocs> &other) : id(other.id) {
        }

        T *allocate(const std::size_t count) {
            auto *ptr = static_cast<T *>(::operator new(count * sizeof(T)));
            allocation_registry::record(ptr, id);
            return ptr;
        }

        void deallocate(T *ptr, std::size_t) {
            allocation_registry::release(ptr, id);
            ::operator delete(ptr);
        }

        template <typename U, typename... Args>
        void construct(U *ptr, Args &&...args) {
            ::new (static_cast<void *>(ptr)) U(std::forward<Args>(args)...);
        }

        template <typename U>
        struct rebind {
            using other = test_allocator<U, Pocma, Pocs>;
        };
    };

    template <typename T, bool Pocma, bool Pocs>
    bool operator==(const test_allocator<T, Pocma, Pocs> &left, const test_allocator<T, Pocma, Pocs> &right) noexcept {
        return left.id == right.id;
    }

    template <typename T, bool Pocma, bool Pocs>
    bool operator!=(const test_allocator<T, Pocma, Pocs> &left, const test_allocator<T, Pocma, Pocs> &right) noexcept {
        return !(left == right);
    }
}

TEST_CASE("ring_buffer - range construction", "[ring_buffer][construct]") {
    const std::vector<int> source{1, 2, 3, 4, 5};
    ring_buffer<int> rb(source.begin(), source.end());
    REQUIRE(rb.size() == source.size());
    CHECK(rb.capacity() == source.size());
    CHECK(rb.front() == 1);
    CHECK(rb.back() == 5);
    for (std::size_t i = 0; i < source.size(); ++i) {
        CHECK(rb[i] == source[i]);
    }
}

TEST_CASE("ring_buffer - initializer_list construction", "[ring_buffer][construct]") {
    ring_buffer<int> rb{10, 20, 30};
    REQUIRE(rb.size() == 3);
    CHECK(rb.capacity() == 3);
    CHECK(rb[0] == 10);
    CHECK(rb[1] == 20);
    CHECK(rb[2] == 30);
}

TEST_CASE("ring_buffer - empty range construction", "[ring_buffer][construct]") {
    const std::vector<int> empty;
    ring_buffer<int> from_range(empty.begin(), empty.end());
    CHECK(from_range.empty());
    CHECK(from_range.size() == 0);
    CHECK(from_range.capacity() == 0);
    CHECK(from_range.begin() == from_range.end());

    ring_buffer<int> from_ilist(std::initializer_list<int>{});
    CHECK(from_ilist.empty());
    CHECK(from_ilist.size() == 0);
    CHECK(from_ilist.capacity() == 0);
}

TEST_CASE("ring_buffer - copy construction after wraparound", "[ring_buffer][construct]") {
    ring_buffer<int> rb(4);
    for (int value = 1; value <= 4; ++value) {
        rb.push_back(value);
    }
    rb.pop_front();
    rb.push_back(5);
    REQUIRE(rb[0] == 2);
    REQUIRE(rb[3] == 5);

    ring_buffer<int> copy(rb);
    REQUIRE(copy.size() == 4);
    CHECK(copy.capacity() == 4);
    const int expected[] = {2, 3, 4, 5};
    for (std::size_t i = 0; i < 4; ++i) {
        CHECK(copy[i] == expected[i]);
    }
}

TEST_CASE("ring_buffer - move assignment with equal non-propagating allocators", "[ring_buffer][allocator]") {
    using alloc = test_allocator<int, false, false>;
    allocation_registry::reset();
    {
        ring_buffer<int, alloc> source(3, alloc(7));
        source.push_back(1);
        source.push_back(2);

        ring_buffer<int, alloc> target(4, alloc(7));
        target.push_back(9);
        target = std::move(source);

        CHECK(target.get_allocator().id == 7);
        REQUIRE(target.size() == 2);
        CHECK(target[0] == 1);
        CHECK(target[1] == 2);
    }
    CHECK(allocation_registry::mismatches == 0);
    CHECK(allocation_registry::owners.empty());
}

TEST_CASE("ring_buffer - move assignment with unequal non-propagating allocators", "[ring_buffer][allocator]") {
    using alloc = test_allocator<int, false, false>;
    allocation_registry::reset();
    {
        ring_buffer<int, alloc> source(3, alloc(2));
        source.push_back(1);
        source.push_back(2);
        source.push_back(3);

        ring_buffer<int, alloc> target(4, alloc(1));
        target.push_back(8);
        target = std::move(source);

        CHECK(target.get_allocator().id == 1);
        REQUIRE(target.size() == 3);
        CHECK(target[0] == 1);
        CHECK(target[1] == 2);
        CHECK(target[2] == 3);
    }
    CHECK(allocation_registry::mismatches == 0);
    CHECK(allocation_registry::owners.empty());
}

TEST_CASE("ring_buffer - move assignment with propagating allocators", "[ring_buffer][allocator]") {
    using alloc = test_allocator<int, true, false>;
    allocation_registry::reset();
    {
        ring_buffer<int, alloc> source(3, alloc(2));
        source.push_back(1);
        source.push_back(2);

        ring_buffer<int, alloc> target(4, alloc(1));
        target.push_back(8);
        target = std::move(source);

        CHECK(target.get_allocator().id == 2);
        REQUIRE(target.size() == 2);
        CHECK(target[0] == 1);
        CHECK(target[1] == 2);
        CHECK(source.empty());
    }
    CHECK(allocation_registry::mismatches == 0);
    CHECK(allocation_registry::owners.empty());
}

TEST_CASE("ring_buffer - swap does not exchange non-propagating allocators", "[ring_buffer][allocator]") {
    using alloc = test_allocator<int, false, false>;
    allocation_registry::reset();
    {
        ring_buffer<int, alloc> left(3, alloc(1));
        left.push_back(1);
        left.push_back(2);

        ring_buffer<int, alloc> right(3, alloc(2));
        right.push_back(7);
        right.push_back(8);
        right.push_back(9);

        left.swap(right);

        CHECK(left.get_allocator().id == 1);
        CHECK(right.get_allocator().id == 2);
        REQUIRE(left.size() == 3);
        CHECK(left[0] == 7);
        CHECK(left[2] == 9);
        REQUIRE(right.size() == 2);
        CHECK(right[0] == 1);
        CHECK(right[1] == 2);
    }
}

TEST_CASE("ring_buffer - swap exchanges propagating allocators", "[ring_buffer][allocator]") {
    using alloc = test_allocator<int, false, true>;
    allocation_registry::reset();
    {
        ring_buffer<int, alloc> left(3, alloc(1));
        left.push_back(1);

        ring_buffer<int, alloc> right(3, alloc(2));
        right.push_back(7);

        left.swap(right);

        CHECK(left.get_allocator().id == 2);
        CHECK(right.get_allocator().id == 1);
        REQUIRE(left.size() == 1);
        CHECK(left[0] == 7);
        REQUIRE(right.size() == 1);
        CHECK(right[0] == 1);
    }
    CHECK(allocation_registry::mismatches == 0);
    CHECK(allocation_registry::owners.empty());
}

namespace {
    struct throwing_value {
        static inline int copies_before_throw = -1;
        static inline int constructed = 0;
        static inline int destroyed = 0;
        int value{0};

        throwing_value() = default;

        explicit throwing_value(int v) : value(v) {
        }

        throwing_value(const throwing_value &right) : value(right.value) {
            if (copies_before_throw == 0) {
                throw std::runtime_error("copy failed");
            }
            if (copies_before_throw > 0) {
                --copies_before_throw;
            }
            ++constructed;
        }

        throwing_value(throwing_value &&) = delete;
        throwing_value &operator=(const throwing_value &) = delete;
        throwing_value &operator=(throwing_value &&) = delete;

        ~throwing_value() {
            ++destroyed;
        }
    };
}

TEST_CASE("ring_buffer - range construction releases storage when a copy throws", "[ring_buffer][construct][exception]") {
    throwing_value source[3]{throwing_value{1}, throwing_value{2}, throwing_value{3}};
    throwing_value::copies_before_throw = 2;
    throwing_value::constructed = 0;
    throwing_value::destroyed = 0;

    CHECK_THROWS_AS((ring_buffer<throwing_value>(source, source + 3)), std::runtime_error);
    CHECK(throwing_value::constructed == throwing_value::destroyed);
}
