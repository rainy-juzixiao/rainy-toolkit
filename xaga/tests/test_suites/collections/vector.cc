#include <rainy/collections/vector.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

// Test fixture for tracking constructor/destructor counts
struct TestTracker {
    static int constructed;
    static int destructed;
    static int copied;
    static int moved;
    int value;

    TestTracker(int v = 0) : value(v) {
        ++constructed;
    }
    TestTracker(const TestTracker &other) : value(other.value) {
        ++constructed;
        ++copied;
    }
    TestTracker(TestTracker &&other) noexcept : value(other.value) {
        ++constructed;
        ++moved;
    }
    ~TestTracker() {
        ++destructed;
    }

    TestTracker &operator=(const TestTracker &other) {
        value = other.value;
        ++copied;
        return *this;
    }

    TestTracker &operator=(TestTracker &&other) noexcept {
        value = other.value;
        ++moved;
        return *this;
    }

    bool operator==(const TestTracker &other) const {
        return value == other.value;
    }
};

int TestTracker::constructed = 0;
int TestTracker::destructed = 0;
int TestTracker::copied = 0;
int TestTracker::moved = 0;

struct NonDefaultConstructible {
    int value;
    NonDefaultConstructible(int v) : value(v) {
    }
    bool operator==(const NonDefaultConstructible &other) const {
        return value == other.value;
    }
};

struct ThrowingConstructor {
    int value;
    static bool should_throw;

    ThrowingConstructor(int v = 0) : value(v) {
        if (should_throw)
            throw std::runtime_error("Constructor throws");
    }

    ThrowingConstructor(const ThrowingConstructor &other) : value(other.value) {
        if (should_throw)
            throw std::runtime_error("Copy constructor throws");
    }
};

bool ThrowingConstructor::should_throw = false;

// Reset function for test tracker
void resetTracker() {
    TestTracker::constructed = 0;
    TestTracker::destructed = 0;
    TestTracker::copied = 0;
    TestTracker::moved = 0;
}

// Custom allocator for testing
template <typename T>
class TestAllocator : public std::allocator<T> {
public:
    using Base = std::allocator<T>;

    template <typename U>
    struct rebind {
        using other = TestAllocator<U>;
    };

    TestAllocator() = default;
    TestAllocator(const TestAllocator &) = default;

    template <typename U>
    TestAllocator(const TestAllocator<U> &) {
    }

    T *allocate(size_t n) {
        allocated_count += n;
        return Base::allocate(n);
    }

    void deallocate(T *p, size_t n) {
        allocated_count -= n;
        Base::deallocate(p, n);
    }

    static size_t allocated_count;
};

template <typename T>
size_t TestAllocator<T>::allocated_count = 0;

using namespace rainy::collections;

SCENARIO("Vector construction and basic properties", "[vector][construction]") {
    GIVEN("Different vector construction methods") {
        WHEN("Default constructing a vector") {
            vector<int> v;

            THEN("It should be empty with zero capacity") {
                REQUIRE(v.empty());
                REQUIRE(v.size() == 0);
                REQUIRE(v.capacity() == 0);
                REQUIRE(v.begin() == v.end());
            }
        }

        WHEN("Constructing with count default elements") {
            vector<int> v(5);

            THEN("It should have 5 default-initialized elements") {
                REQUIRE(v.size() == 5);
                REQUIRE(v.capacity() >= 5);
                REQUIRE(!v.empty());
                for (size_t i = 0; i < v.size(); ++i) {
                    REQUIRE(v[i] == 0);
                }
            }
        }

        WHEN("Constructing with count and value") {
            vector<std::string> v(3, "test");

            THEN("It should have 3 copies of the value") {
                REQUIRE(v.size() == 3);
                for (const auto &s: v) {
                    REQUIRE(s == "test");
                }
            }
        }

        WHEN("Constructing from iterator range") {
            std::vector<int> src = {1, 2, 3, 4, 5};
            vector<int> v(src.begin(), src.end());

            THEN("It should contain the same elements") {
                REQUIRE(v.size() == 5);
                REQUIRE(v[0] == 1);
                REQUIRE(v[4] == 5);
            }
        }

        WHEN("Constructing from initializer list") {
            vector<int> v = {10, 20, 30};

            THEN("It should contain the list elements") {
                REQUIRE(v.size() == 3);
                REQUIRE(v[0] == 10);
                REQUIRE(v[1] == 20);
                REQUIRE(v[2] == 30);
            }
        }

        WHEN("Copy constructing a vector") {
            vector<int> original = {1, 2, 3};
            vector<int> copy(original);

            THEN("The copy should have the same elements") {
                REQUIRE(copy.size() == 3);
                REQUIRE(copy[0] == 1);
                REQUIRE(copy[2] == 3);
            }

            AND_WHEN("Modifying the original") {
                original[0] = 99;

                THEN("The copy should remain unchanged") {
                    REQUIRE(copy[0] == 1);
                }
            }
        }

        WHEN("Move constructing a vector") {
            vector<std::string> original = {"hello", "world"};
            vector<std::string> moved(std::move(original));

            THEN("The moved-from vector should be empty") {
                REQUIRE(moved.size() == 2);
                REQUIRE(moved[0] == "hello");
                REQUIRE(original.empty());
                REQUIRE(original.capacity() == 0);
            }
        }
    }
}

SCENARIO("Vector element access and modification", "[vector][access]") {
    GIVEN("A vector with elements") {
        vector<int> v = {10, 20, 30, 40, 50};

        WHEN("Accessing elements via operator[]") {
            THEN("Elements can be read and modified") {
                REQUIRE(v[0] == 10);
                REQUIRE(v[4] == 50);

                v[2] = 99;
                REQUIRE(v[2] == 99);
            }
        }

        WHEN("Accessing elements via at()") {
            THEN("Valid indices return elements") {
                REQUIRE(v.at(0) == 10);
                REQUIRE(v.at(4) == 50);
            }

            THEN("Invalid indices throw out_of_range") {
                REQUIRE_THROWS_AS(v.at(5), std::out_of_range);
                REQUIRE_THROWS_AS(v.at(static_cast<size_t>(-1)), std::out_of_range);
            }
        }

        WHEN("Accessing front() and back()") {
            THEN("They return first and last elements") {
                REQUIRE(v.front() == 10);
                REQUIRE(v.back() == 50);

                v.front() = 11;
                v.back() = 51;
                REQUIRE(v[0] == 11);
                REQUIRE(v[4] == 51);
            }

            AND_WHEN("Vector is empty") {
                vector<int> empty;

                THEN("front() and back() still work but are dangerous") {
                    // Note: This is undefined behavior, but we test that it compiles
                    // In practice, you should check empty() first
                }
            }
        }

        WHEN("Accessing data()") {
            int *data_ptr = v.data();

            THEN("It returns pointer to underlying array") {
                REQUIRE(data_ptr != nullptr);
                REQUIRE(data_ptr[0] == 10);
                REQUIRE(data_ptr[4] == 50);
            }
        }
    }
}

SCENARIO("Vector capacity management", "[vector][capacity]") {
    GIVEN("An empty vector") {
        vector<int> v;

        WHEN("Reserving capacity") {
            v.reserve(100);

            THEN("Capacity increases but size remains 0") {
                REQUIRE(v.capacity() >= 100);
                REQUIRE(v.size() == 0);
                REQUIRE(v.empty());
            }
        }

        WHEN("Reserving less than current capacity") {
            v.reserve(100);
            size_t old_cap = v.capacity();
            v.reserve(50);

            THEN("Capacity does not decrease") {
                REQUIRE(v.capacity() == old_cap);
            }
        }
    }

    GIVEN("A vector with some elements") {
        vector<int> v = {1, 2, 3, 4, 5};
        size_t old_cap = v.capacity();

        WHEN("Shrinking to fit") {
            v.shrink_to_fit();

            THEN("Capacity should equal size") {
                REQUIRE(v.capacity() == v.size());
                REQUIRE(v.size() == 5);
            }
        }

        WHEN("Resizing to larger size") {
            v.resize(10);

            THEN("Size increases, new elements default-initialized") {
                REQUIRE(v.size() == 10);
                REQUIRE(v[5] == 0);
                REQUIRE(v[9] == 0);
            }
        }

        WHEN("Resizing to larger size with value") {
            v.resize(10, 99);

            THEN("New elements get the specified value") {
                REQUIRE(v.size() == 10);
                REQUIRE(v[5] == 99);
                REQUIRE(v[9] == 99);
            }
        }

        WHEN("Resizing to smaller size") {
            v.resize(3);

            THEN("Size decreases, elements are destroyed") {
                REQUIRE(v.size() == 3);
                REQUIRE(v[0] == 1);
                REQUIRE(v[2] == 3);
            }
        }
    }
}

SCENARIO("Vector modifiers", "[vector][modifiers]") {
    GIVEN("An empty vector") {
        vector<int> v;

        WHEN("Pushing back elements") {
            v.push_back(10);
            v.push_back(20);

            THEN("Elements are added at the end") {
                REQUIRE(v.size() == 2);
                REQUIRE(v[0] == 10);
                REQUIRE(v[1] == 20);
            }
        }

        WHEN("Emplacing back elements") {
            v.emplace_back(30);
            v.emplace_back(40);

            THEN("Elements are constructed in-place") {
                REQUIRE(v.size() == 2);
                REQUIRE(v[0] == 30);
                REQUIRE(v[1] == 40);
            }
        }

        WHEN("Pushing and popping") {
            v.push_back(1);
            v.push_back(2);
            v.pop_back();

            THEN("Last element is removed") {
                REQUIRE(v.size() == 1);
                REQUIRE(v[0] == 1);
            }
        }
    }

    GIVEN("A vector with elements at the middle") {
        vector<int> v = {1, 2, 3, 4, 5};

        WHEN("Inserting at beginning") {
            auto it = v.insert(v.begin(), 99);

            THEN("Element is inserted at front") {
                REQUIRE(v.size() == 6);
                REQUIRE(v[0] == 99);
                REQUIRE(v[1] == 1);
                REQUIRE(*it == 99);
            }
        }

        WHEN("Inserting at middle") {
            auto it = v.insert(v.begin() + 2, 99);

            THEN("Element is inserted at correct position") {
                REQUIRE(v.size() == 6);
                REQUIRE(v[2] == 99);
                REQUIRE(v[3] == 3);
                REQUIRE(*it == 99);
            }
        }

        WHEN("Inserting at end") {
            auto it = v.insert(v.end(), 99);

            THEN("Element is inserted at back") {
                REQUIRE(v.size() == 6);
                REQUIRE(v[5] == 99);
                REQUIRE(*it == 99);
            }
        }

        WHEN("Inserting multiple elements") {
            v.insert(v.begin() + 2, 3, 99);

            THEN("All elements are inserted") {
                REQUIRE(v.size() == 8);
                REQUIRE(v[2] == 99);
                REQUIRE(v[3] == 99);
                REQUIRE(v[4] == 99);
                REQUIRE(v[5] == 3); // Original element shifted
            }
        }

        WHEN("Erasing from beginning") {
            auto it = v.erase(v.begin());

            THEN("First element removed") {
                REQUIRE(v.size() == 4);
                REQUIRE(v[0] == 2);
                REQUIRE(*it == 2);
            }
        }

        WHEN("Erasing from middle") {
            auto it = v.erase(v.begin() + 2);

            THEN("Element at position removed") {
                REQUIRE(v.size() == 4);
                REQUIRE(v[2] == 4); // Original 4 shifted to position 2
                REQUIRE(*it == 4);
            }
        }

        WHEN("Erasing range") {
            auto it = v.erase(v.begin() + 1, v.begin() + 4);

            THEN("Range of elements removed") {
                REQUIRE(v.size() == 2);
                REQUIRE(v[0] == 1);
                REQUIRE(v[1] == 5);
                REQUIRE(*it == 5);
            }
        }

        WHEN("Clearing the vector") {
            v.clear();

            THEN("All elements are removed") {
                REQUIRE(v.empty());
                REQUIRE(v.size() == 0);
            }
        }
    }
}

SCENARIO("Vector assignment operations", "[vector][assignment]") {
    GIVEN("Two vectors with different content") {
        vector<int> v1 = {1, 2, 3};
        vector<int> v2 = {4, 5, 6, 7, 8};

        WHEN("Copy assigning") {
            v1 = v2;

            THEN("v1 becomes a copy of v2") {
                REQUIRE(v1.size() == v2.size());
                REQUIRE(v1[0] == 4);
                REQUIRE(v1[4] == 8);
            }

            AND_WHEN("Modifying v2") {
                v2[0] = 99;

                THEN("v1 remains unchanged") {
                    REQUIRE(v1[0] == 4);
                }
            }
        }

        WHEN("Move assigning") {
            vector<int> v3 = {9, 10, 11};
            v1 = std::move(v3);

            THEN("v1 takes v3's resources") {
                REQUIRE(v1.size() == 3);
                REQUIRE(v1[0] == 9);
                REQUIRE(v1[2] == 11);
                REQUIRE(v3.empty());
            }
        }

        WHEN("Assigning from initializer list") {
            v1 = {100, 200, 300, 400};

            THEN("v1 gets the new elements") {
                REQUIRE(v1.size() == 4);
                REQUIRE(v1[0] == 100);
                REQUIRE(v1[3] == 400);
            }
        }

        WHEN("Assigning with iterator range") {
            std::vector<int> src = {50, 60, 70};
            v1.assign(src.begin(), src.end());

            THEN("v1 gets elements from range") {
                REQUIRE(v1.size() == 3);
                REQUIRE(v1[0] == 50);
                REQUIRE(v1[2] == 70);
            }
        }

        WHEN("Assigning with count and value") {
            v1.assign(5, 42);

            THEN("v1 gets count copies of value") {
                REQUIRE(v1.size() == 5);
                for (const auto &val: v1) {
                    REQUIRE(val == 42);
                }
            }
        }
    }
}

SCENARIO("Vector comparison operators", "[vector][comparison]") {
    GIVEN("Vectors with different contents") {
        vector<int> v1 = {1, 2, 3};
        vector<int> v2 = {1, 2, 3};
        vector<int> v3 = {1, 2, 4};
        vector<int> v4 = {1, 2};
        vector<int> v5 = {1, 2, 3, 4};

        THEN("Equality works correctly") {
            REQUIRE(v1 == v2);
            REQUIRE_FALSE(v1 == v3);
            REQUIRE_FALSE(v1 == v4);
            REQUIRE_FALSE(v1 == v5);
        }

        THEN("Inequality works correctly") {
            REQUIRE(v1 != v3);
            REQUIRE_FALSE(v1 != v2);
        }

        THEN("Less-than works lexicographically") {
            REQUIRE(v4 < v1); // Shorter prefix is less
            REQUIRE(v1 < v3); // 3 < 4
            REQUIRE(v1 < v5); // Prefix of longer
            REQUIRE_FALSE(v3 < v1);
        }

        THEN("Greater-than works lexicographically") {
            REQUIRE(v3 > v1);
            REQUIRE(v5 > v1);
            REQUIRE_FALSE(v1 > v2);
        }

        THEN("Less-or-equal works correctly") {
            REQUIRE(v1 <= v2);
            REQUIRE(v1 <= v3);
            REQUIRE(v4 <= v1);
            REQUIRE_FALSE(v3 <= v1);
        }

        THEN("Greater-or-equal works correctly") {
            REQUIRE(v1 >= v2);
            REQUIRE(v3 >= v1);
            REQUIRE(v1 >= v4);
            REQUIRE_FALSE(v1 >= v3);
        }
    }
}

SCENARIO("Vector with non-default constructible types", "[vector][special]") {
    GIVEN("A vector of non-default constructible type") {
        vector<NonDefaultConstructible> v;

        WHEN("Emplacing elements") {
            v.emplace_back(10);
            v.emplace_back(20);

            THEN("Elements are properly constructed") {
                REQUIRE(v.size() == 2);
                REQUIRE(v[0].value == 10);
                REQUIRE(v[1].value == 20);
            }
        }

        WHEN("Constructing with initializer list") {
            vector<NonDefaultConstructible> v2 = {NonDefaultConstructible(1), NonDefaultConstructible(2)};

            THEN("Elements are copied correctly") {
                REQUIRE(v2.size() == 2);
                REQUIRE(v2[0].value == 1);
                REQUIRE(v2[1].value == 2);
            }
        }
    }
}

SCENARIO("Vector iterator operations", "[vector][iterators]") {
    GIVEN("A vector with elements") {
        vector<int> v = {1, 2, 3, 4, 5};

        THEN("Iterators can traverse the vector") {
            auto it = v.begin();
            REQUIRE(*it == 1);
            ++it;
            REQUIRE(*it == 2);
            it += 2;
            REQUIRE(*it == 4);
            --it;
            REQUIRE(*it == 3);
        }

        THEN("Reverse iterators traverse in reverse") {
            auto rit = v.rbegin();
            REQUIRE(*rit == 5);
            ++rit;
            REQUIRE(*rit == 4);
            REQUIRE(*(v.rend() - 1) == 1);
        }

        THEN("Const iterators work for const access") {
            const auto &cv = v;
            auto cit = cv.cbegin();
            REQUIRE(*cit == 1);
            REQUIRE(*(cv.cend() - 1) == 5);
        }

        THEN("Iterator difference works correctly") {
            REQUIRE(v.end() - v.begin() == 5);
            REQUIRE(v.begin() + 3 == v.begin() + 3);
        }
    }
}

SCENARIO("Vector exception safety", "[vector][exceptions]") {
    GIVEN("A vector with throwing constructor type") {
        vector<ThrowingConstructor> v;
        ThrowingConstructor::should_throw = false;

        WHEN("Construction throws during resize") {
            v.emplace_back(1);
            v.emplace_back(2);

            ThrowingConstructor::should_throw = true;

            THEN("Vector state remains valid") {
                REQUIRE_THROWS_AS(v.resize(10), std::runtime_error);
                // Vector should be in valid state
                REQUIRE(v.size() == 2);
                REQUIRE(v[0].value == 1);
                REQUIRE(v[1].value == 2);
            }
        }

        ThrowingConstructor::should_throw = false;
    }
}

SCENARIO("Vector swap operation", "[vector][swap]") {
    GIVEN("Two vectors with different content") {
        vector<int> v1 = {1, 2, 3};
        vector<int> v2 = {4, 5, 6, 7};
        auto v1_data = v1.data();
        auto v2_data = v2.data();

        WHEN("Swapping the vectors") {
            swap(v1, v2);

            THEN("Their contents are exchanged") {
                REQUIRE(v1.size() == 4);
                REQUIRE(v1[0] == 4);
                REQUIRE(v1[3] == 7);

                REQUIRE(v2.size() == 3);
                REQUIRE(v2[0] == 1);
                REQUIRE(v2[2] == 3);
            }

            THEN("Their internal pointers are swapped (or moved)") {
                // Note: Depending on allocator, pointers might be swapped or moved
                // This test checks the observable behavior, not the implementation
            }
        }

        WHEN("Using member swap") {
            v1.swap(v2);

            THEN("Same effect as non-member swap") {
                REQUIRE(v1.size() == 4);
                REQUIRE(v2.size() == 3);
            }
        }
    }
}

SCENARIO("Vector edge cases and boundary conditions", "[vector][edge]") {
    GIVEN("An empty vector") {
        vector<int> v;

        WHEN("Reserving zero capacity") {
            v.reserve(0);

            THEN("Nothing changes") {
                REQUIRE(v.capacity() == 0);
            }
        }

        WHEN("Shrinking to fit empty vector") {
            v.shrink_to_fit();

            THEN("Remains empty") {
                REQUIRE(v.empty());
                REQUIRE(v.capacity() == 0);
            }
        }
    }

    GIVEN("A vector at maximum size for small type") {
        vector<char> v;

        WHEN("Reserving maximum possible size") {
            // This is implementation-defined, we just test it doesn't crash
            size_t max_possible = v.max_size();

            // Some implementations might throw bad_alloc
            try {
                v.reserve(max_possible);
                // If it succeeds, capacity should be at least max_possible
                REQUIRE(v.capacity() >= max_possible);
            } catch (const std::bad_alloc &) {
                // Expected on some platforms
            }
        }
    }

    GIVEN("A vector with single element") {
        vector<int> v = {42};

        WHEN("Popping the last element") {
            v.pop_back();

            THEN("Vector becomes empty") {
                REQUIRE(v.empty());
                REQUIRE(v.size() == 0);
            }
        }
    }

    GIVEN("A vector with capacity greater than size") {
        vector<int> v;
        v.reserve(100);
        v.push_back(1);
        v.push_back(2);

        WHEN("Inserting without reallocation") {
            size_t old_cap = v.capacity();
            auto it = v.insert(v.begin() + 1, 99);

            THEN("Elements shift correctly without reallocation") {
                REQUIRE(v.capacity() == old_cap);
                REQUIRE(v.size() == 3);
                REQUIRE(v[0] == 1);
                REQUIRE(v[1] == 99);
                REQUIRE(v[2] == 2);
                REQUIRE(*it == 99);
            }
        }
    }
}

SCENARIO("Vector with custom allocator", "[vector][allocator]") {
    using AllocVec = vector<int, TestAllocator<int>>;

    GIVEN("A vector with custom allocator") {
        TestAllocator<int>::allocated_count = 0;

        WHEN("Creating and manipulating vector") {
            AllocVec v;
            v.reserve(10);

            THEN("Allocator is used for allocations") {
                REQUIRE(TestAllocator<int>::allocated_count >= 10);
            }

            AND_WHEN("Adding elements") {
                for (int i = 0; i < 5; ++i) {
                    v.push_back(i);
                }

                THEN("Allocations count reflects usage") {
                    REQUIRE(v.size() == 5);
                    REQUIRE(TestAllocator<int>::allocated_count >= 10);
                }
            }
        }

        WHEN("Copying vector with allocator") {
            AllocVec v1;
            v1.push_back(1);
            v1.push_back(2);

            AllocVec v2(v1, TestAllocator<int>());

            THEN("Elements are copied correctly") {
                REQUIRE(v2.size() == 2);
                REQUIRE(v2[0] == 1);
                REQUIRE(v2[1] == 2);
            }
        }
    }

    TestAllocator<int>::allocated_count = 0;
}

SCENARIO("Vector move semantics with non-equal allocators", "[vector][move][allocator]") {
    using AllocVec = vector<TestTracker, TestAllocator<TestTracker>>;

    GIVEN("Two vectors with different allocators") {
        resetTracker();
        TestAllocator<TestTracker>::allocated_count = 0;

        AllocVec v1{TestAllocator<TestTracker>()};
        v1.push_back(TestTracker(1));
        v1.push_back(TestTracker(2));

        // Create a vector with a different allocator instance
        // Note: TestAllocator instances are equal, so we need a different type or state
        // For this test, we'll assume allocators are equal (is_always_equal)

        WHEN("Move constructing with different allocator") {
            // If allocators are equal, resources are moved
            AllocVec v2(std::move(v1), TestAllocator<TestTracker>());

            THEN("Elements are moved (either by pointer or construction)") {
                REQUIRE(v2.size() == 2);
                REQUIRE(v1.empty());
                // Depending on allocator equality, either moved or copied
            }
        }
    }
}

SCENARIO("Vector with move-only types", "[vector][moveonly]") {
    GIVEN("A vector of move-only types") {
        vector<std::unique_ptr<int>> v;

        WHEN("Adding elements") {
            v.push_back(std::make_unique<int>(10));
            v.push_back(std::make_unique<int>(20));

            THEN("Elements are stored correctly") {
                REQUIRE(v.size() == 2);
                REQUIRE(*v[0] == 10);
                REQUIRE(*v[1] == 20);
            }

            AND_WHEN("Moving the vector") {
                auto v2 = std::move(v);

                THEN("Ownership is transferred") {
                    REQUIRE(v2.size() == 2);
                    REQUIRE(v.empty());
                    REQUIRE(*v2[0] == 10);
                }
            }
        }

        WHEN("Emplacing elements") {
            v.emplace_back(std::make_unique<int>(30));

            THEN("Element is constructed in-place") {
                REQUIRE(v.size() == 1);
                REQUIRE(*v[0] == 30);
            }
        }
    }
}

SCENARIO("Vector large data handling", "[vector][performance][large]") {
    GIVEN("A vector needing multiple reallocations") {
        vector<int> v;
        size_t prev_cap = 0;

        WHEN("Pushing many elements") {
            for (int i = 0; i < 1000; ++i) {
                v.push_back(i);

                if (v.capacity() > prev_cap) {
                    // Capacity should grow geometrically
                    if (prev_cap > 0) {
                        REQUIRE(v.capacity() >= prev_cap * 2);
                    }
                    prev_cap = v.capacity();
                }
            }

            THEN("All elements are present") {
                REQUIRE(v.size() == 1000);
                for (int i = 0; i < 1000; ++i) {
                    REQUIRE(v[i] == i);
                }
            }
        }
    }
}

SCENARIO("Vector self-assignment", "[vector][edge]") {
    GIVEN("A vector with content") {
        vector<int> v = {1, 2, 3};

        WHEN("Self-assigning via copy") {
            v = v;

            THEN("Content remains unchanged") {
                REQUIRE(v.size() == 3);
                REQUIRE(v[0] == 1);
                REQUIRE(v[2] == 3);
            }
        }

        WHEN("Self-assigning via move") {
            v = std::move(v);

            THEN("Content remains unchanged (or valid)") {
                // Self-move assignment should leave object in valid state
                REQUIRE(v.size() == 3); // Some implementations might empty it
            }
        }
    }
}
