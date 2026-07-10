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
#include <../include/rainy/core/memory/allocator.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <set>
#include <vector>

#include "catch2/catch_approx.hpp"

namespace fm = rainy::core::memory;

TEST_CASE("allocation_result default construction", "[allocator][allocation_result]") {
    fm::allocation_result<int*> result{};
    REQUIRE(result.ptr == nullptr);
    REQUIRE(result.count == 0);
}

TEST_CASE("allocation_result value construction", "[allocator][allocation_result]") {
    int dummy{};
    fm::allocation_result<int*> result{&dummy, 42};
    REQUIRE(result.ptr == &dummy);
    REQUIRE(result.count == 42);
}

TEST_CASE("allocation_result converting construction", "[allocator][allocation_result]") {
    int dummy{};
    fm::allocation_result<int*> src{&dummy, 7};
    fm::allocation_result<const int*> dst{src};
    REQUIRE(dst.ptr == &dummy);
    REQUIRE(dst.count == 7);
}

TEST_CASE("allocation_result move converting construction", "[allocator][allocation_result]") {
    int dummy{};
    fm::allocation_result<int*> src{&dummy, 3};
    fm::allocation_result<const int*> dst{std::move(src)};
    REQUIRE(dst.ptr == &dummy);
    REQUIRE(dst.count == 3);
}

TEST_CASE("allocator type aliases", "[allocator]") {
    using Alloc = fm::allocator<int>;

    STATIC_REQUIRE(std::is_same_v<Alloc::value_type, int>);
    STATIC_REQUIRE(std::is_same_v<Alloc::size_type, std::size_t>);
    STATIC_REQUIRE(std::is_same_v<Alloc::difference_type, std::ptrdiff_t>);
    STATIC_REQUIRE(Alloc::propagate_on_container_move_assignment::value);
    STATIC_REQUIRE(Alloc::is_always_equal::value);
}

TEST_CASE("allocator basic allocate and deallocate", "[allocator]") {
    fm::allocator<int> alloc;

    SECTION("allocate single element") {
        int* p = alloc.allocate(1);
        REQUIRE(p != nullptr);
        *p = 42;
        REQUIRE(*p == 42);
        alloc.deallocate(p, 1);
    }

    SECTION("allocate multiple elements") {
        int* p = alloc.allocate(10);
        REQUIRE(p != nullptr);
        for (int i = 0; i < 10; ++i) {
            p[i] = i;
        }
        for (int i = 0; i < 10; ++i) {
            REQUIRE(p[i] == i);
        }
        alloc.deallocate(p, 10);
    }
}

TEST_CASE("allocator allocate_at_least", "[allocator]") {
    fm::allocator<double> alloc;
    auto result = alloc.allocate_at_least(5);
    REQUIRE(result.ptr != nullptr);
    REQUIRE(result.count == 5);
    result.ptr[0] = 1.0;
    result.ptr[4] = 5.0;
    REQUIRE(result.ptr[0] == Catch::Approx(1.0));
    alloc.deallocate(result.ptr, result.count);
}

TEST_CASE("allocator default construction is constexpr", "[allocator]") {
    constexpr fm::allocator<int> alloc{};
    (void)alloc;
}

TEST_CASE("allocator copy construction", "[allocator]") {
    fm::allocator<int> a;
    fm::allocator<int> b(a);
    // is_always_equal, so a == b always
    REQUIRE(a == b);
}

TEST_CASE("allocator rebind construction", "[allocator]") {
    fm::allocator<int> int_alloc;
    fm::allocator<double> double_alloc(int_alloc); // rebind via converting ctor
    REQUIRE(int_alloc == double_alloc);
}

TEST_CASE("allocator equality comparison", "[allocator]") {
    fm::allocator<int> a;
    fm::allocator<int> b;
    REQUIRE(a == b);
}

TEST_CASE("allocator inequality comparison", "[allocator]") {
    fm::allocator<int> a;
    fm::allocator<double> b;
    // is_always_equal → always equal
    REQUIRE_FALSE(a != b);
}

TEST_CASE("allocator_traits for std::allocator", "[allocator][allocator_traits]") {
    using Traits = fm::allocator_traits<std::allocator<int>>;
    STATIC_REQUIRE(std::is_same_v<Traits::allocator_type, std::allocator<int>>);
    STATIC_REQUIRE(std::is_same_v<Traits::value_type, int>);
    STATIC_REQUIRE(std::is_same_v<Traits::pointer, int*>);
    STATIC_REQUIRE(std::is_same_v<Traits::const_pointer, const int*>);
    STATIC_REQUIRE(std::is_same_v<Traits::void_pointer, void*>);
    STATIC_REQUIRE(std::is_same_v<Traits::const_void_pointer, const void*>);
    STATIC_REQUIRE(std::is_same_v<Traits::size_type, std::size_t>);
    STATIC_REQUIRE(std::is_same_v<Traits::difference_type, std::ptrdiff_t>);

    STATIC_REQUIRE_FALSE(Traits::propagate_on_container_copy_assignment::value);
    STATIC_REQUIRE(Traits::propagate_on_container_move_assignment::value);
    STATIC_REQUIRE_FALSE(Traits::propagate_on_container_swap::value);
    STATIC_REQUIRE(Traits::is_always_equal::value);

    // rebind
    STATIC_REQUIRE(std::is_same_v<Traits::template rebind_alloc<double>, std::allocator<double>>);
    STATIC_REQUIRE(std::is_same_v<Traits::template rebind_traits<double>, fm::allocator_traits<std::allocator<double>>>);
}

TEST_CASE("allocator_traits for custom allocator", "[allocator][allocator_traits]") {
    using Traits = fm::allocator_traits<fm::allocator<int>>;
    STATIC_REQUIRE(std::is_same_v<Traits::allocator_type, fm::allocator<int>>);
    STATIC_REQUIRE(std::is_same_v<Traits::value_type, int>);
    STATIC_REQUIRE(std::is_same_v<Traits::pointer, int*>);
    STATIC_REQUIRE(std::is_same_v<Traits::const_pointer, const int*>);
    STATIC_REQUIRE(std::is_same_v<Traits::void_pointer, void*>);
    STATIC_REQUIRE(std::is_same_v<Traits::const_void_pointer, const void*>);

    // custom allocator uses the normal_allocator_traits branch
    STATIC_REQUIRE(Traits::is_always_equal::value);
    STATIC_REQUIRE(Traits::propagate_on_container_move_assignment::value);
}

TEST_CASE("allocator_traits allocate/deallocate (std allocator)", "[allocator][allocator_traits]") {
    using Traits = fm::allocator_traits<std::allocator<int>>;
    std::allocator<int> alloc;
    int* p = Traits::allocate(alloc, 5);
    REQUIRE(p != nullptr);
    p[0] = 1;
    p[4] = 5;
    REQUIRE(p[4] == 5);
    Traits::deallocate(alloc, p, 5);
}

TEST_CASE("allocator_traits construct/destroy", "[allocator][allocator_traits]") {
    using Traits = fm::allocator_traits<std::allocator<int>>;
    std::allocator<int> alloc;
    int* p = Traits::allocate(alloc, 1);

    Traits::construct(alloc, p, 42);
    REQUIRE(*p == 42);
    Traits::destroy(alloc, p);
    Traits::deallocate(alloc, p, 1);
}

TEST_CASE("allocator_traits select_on_container_copy_construction", "[allocator][allocator_traits]") {
    using Traits = fm::allocator_traits<std::allocator<int>>;
    std::allocator<int> alloc;
    auto selected = Traits::select_on_container_copy_construction(alloc);
    REQUIRE(selected == alloc);
}

TEST_CASE("is_std_allocator detection", "[allocator]") {
    STATIC_REQUIRE(fm::implements::is_std_allocator<std::allocator<int>>);
    STATIC_REQUIRE(fm::implements::is_std_allocator<std::allocator<double>>);
    STATIC_REQUIRE_FALSE(fm::implements::is_std_allocator<fm::allocator<int>>);
    STATIC_REQUIRE_FALSE(fm::implements::is_std_allocator<int>);
}

TEST_CASE("has_select_on_container_copy_construction detection", "[allocator]") {
    // std::allocator doesn't have select_on_container_copy_construction
    STATIC_REQUIRE_FALSE(fm::implements::has_select_on_container_copy_construction<std::allocator<int>>::value);
    // custom allocator doesn't either by default
    STATIC_REQUIRE_FALSE(fm::implements::has_select_on_container_copy_construction<fm::allocator<int>>::value);
}

TEST_CASE("block_allocator type aliases", "[allocator][block_allocator]") {
    using Block = fm::block_allocator<int, 8>;

    STATIC_REQUIRE(std::is_same_v<Block::value_type, int>);
    STATIC_REQUIRE(std::is_same_v<Block::pointer, int*>);
    STATIC_REQUIRE(std::is_same_v<Block::size_type, std::size_t>);
    STATIC_REQUIRE(std::is_same_v<Block::reference, int&>);
    STATIC_REQUIRE(std::is_same_v<Block::const_reference, const int&>);
}

TEST_CASE("block_allocator falls back to heap when count exceeds block", "[allocator][block_allocator]") {
    fm::block_allocator<int, 4> alloc;

    // Requesting more than N should fall back to heap allocator
    int* p = alloc.allocate(10);
    REQUIRE(p != nullptr);
    p[9] = 123;
    REQUIRE(p[9] == 123);
    alloc.deallocate(p, 10);
}

TEST_CASE("block_allocator uses block storage for first allocation", "[allocator][block_allocator]") {
    fm::block_allocator<int, 4> alloc;

    int* p = alloc.allocate(4);
    REQUIRE(p != nullptr);
    p[0] = 10;
    p[3] = 40;
    REQUIRE(p[3] == 40);
    alloc.deallocate(p, 4);
}

TEST_CASE("block_allocator second allocation falls back to heap", "[allocator][block_allocator]") {
    fm::block_allocator<int, 4> alloc;

    int* first = alloc.allocate(4);
    REQUIRE(first != nullptr);

    // Second allocation should go to heap even if count <= N
    int* second = alloc.allocate(2);
    REQUIRE(second != nullptr);
    REQUIRE(second != first);

    alloc.deallocate(first, 4);
    alloc.deallocate(second, 2);
}

TEST_CASE("block_allocator deallocate restores block for reuse", "[allocator][block_allocator]") {
    fm::block_allocator<int, 4> alloc;

    int* first = alloc.allocate(4);
    alloc.deallocate(first, 4);

    // After dealloc, the block should be available again
    int* second = alloc.allocate(4);
    REQUIRE(second == first); // same block re-used
    alloc.deallocate(second, 4);
}

TEST_CASE("block_allocator construct and destroy", "[allocator][block_allocator]") {
    fm::block_allocator<int, 2> alloc;

    int* p = alloc.allocate(1);
    alloc.construct(p, 77);
    REQUIRE(*p == 77);
    alloc.destroy(p);
    alloc.deallocate(p, 1);
}

TEST_CASE("block_allocator allocate_at_least", "[allocator][block_allocator]") {
    fm::block_allocator<double, 3> alloc;
    auto result = alloc.allocate_at_least(2);
    REQUIRE(result.ptr != nullptr);
    REQUIRE(result.count == 2);
    result.ptr[0] = 1.5;
    result.ptr[1] = 2.5;
    REQUIRE(result.ptr[1] == Catch::Approx(2.5));
    alloc.deallocate(result.ptr, result.count);
}

TEST_CASE("alloc_construct_ptr allocates and deallocates on scope exit", "[allocator]") {
    fm::allocator<int> alloc;
    {
        fm::implements::alloc_construct_ptr<fm::allocator<int>> raii(alloc);
        raii.allocate();
        REQUIRE(raii.ptr != nullptr);
        *raii.ptr = 100;

        // release transfers ownership without deallocation
        int* released = raii.release();
        REQUIRE(released != nullptr);
        REQUIRE(*released == 100);
        REQUIRE(raii.ptr == nullptr);

        alloc.deallocate(released, 1);
    }
    // RAII destructor called but ptr is null so no double-free
}

TEST_CASE("alloc_construct_ptr destructor deallocates on exception path", "[allocator]") {
    fm::allocator<int> alloc;
    {
        fm::implements::alloc_construct_ptr<fm::allocator<int>> raii(alloc);
        raii.allocate();
        *raii.ptr = 42;
        // no release → destructor deallocates
    }
    // successfully cleaned up — would ASAN/UBSan catch double-free if buggy
}

TEST_CASE("custom allocator works with std::vector", "[allocator]") {
    using Vec = std::vector<int, fm::allocator<int>>;
    Vec v{1, 2, 3, 4, 5};
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[4] == 5);

    v.push_back(6);
    REQUIRE(v.size() == 6);
    REQUIRE(v[5] == 6);
}

TEST_CASE("custom allocator works with std::set", "[allocator]") {
    using Set = std::set<int, std::less<int>, fm::allocator<int>>;
    Set s{3, 1, 4, 1, 5, 9};
    REQUIRE(s.size() == 5);
    REQUIRE(s.count(1) == 1);
    REQUIRE(s.count(6) == 0);
}
