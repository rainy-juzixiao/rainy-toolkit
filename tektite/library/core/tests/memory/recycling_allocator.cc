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
#include <rainy/core/memory/recycling_allocator.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <type_traits>

namespace fm = rainy::core::memory;

TEST_CASE("recycling_allocator allocates storage with the element alignment", "[recycling_allocator]") {
    struct alignas(64) aligned_value { int value; };

    fm::recycling_allocator<aligned_value> allocator;
    auto *ptr = allocator.allocate(2);
    REQUIRE(ptr != nullptr);
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr) % alignof(aligned_value) == 0);

    std::construct_at(ptr, aligned_value{1});
    std::construct_at(ptr + 1, aligned_value{2});
    REQUIRE(ptr[0].value == 1);
    REQUIRE(ptr[1].value == 2);
    std::destroy_at(ptr + 1);
    std::destroy_at(ptr);
    allocator.deallocate(ptr, 2);
}

TEST_CASE("recycling_allocator has standard allocator rebind and equality semantics", "[recycling_allocator]") {
    using int_allocator = fm::recycling_allocator<int>;
    using double_allocator = int_allocator::rebind<double>::other;

    STATIC_REQUIRE(std::is_same_v<int_allocator::value_type, int>);
    STATIC_REQUIRE(std::is_same_v<double_allocator, fm::recycling_allocator<double>>);
    STATIC_REQUIRE(int_allocator::is_always_equal::value);
    STATIC_REQUIRE(int_allocator::propagate_on_container_move_assignment::value);

    int_allocator integers;
    double_allocator doubles(integers);
    REQUIRE(integers == doubles);
    REQUIRE_FALSE(integers != doubles);
}

TEST_CASE("recycling_allocator reuses storage within a thread context", "[recycling_allocator]") {
    struct alignas(64) cached_value { int value; };
    rainy::core::concurrency::thread_context thread_context;
    rainy::core::concurrency::implements::thread_info_base thread_info;
    rainy::core::concurrency::thread_context::thread_call_stack::context scope(&thread_context, thread_info);

    fm::recycling_allocator<cached_value> allocator;
    cached_value *const first = allocator.allocate(1);
    allocator.deallocate(first, 1);
    cached_value *const second = allocator.allocate(1);
    REQUIRE(second == first);
    REQUIRE(reinterpret_cast<std::uintptr_t>(second) % alignof(cached_value) == 0);
    allocator.deallocate(second, 1);
}

TEST_CASE("get_recycling_allocator preserves custom allocators and replaces std allocators", "[recycling_allocator]") {
    fm::recycling_allocator<int> custom;
    const auto preserved = fm::get_recycling_allocator<fm::recycling_allocator<int>, void>::get(custom);
    STATIC_REQUIRE(std::is_same_v<std::remove_cv_t<decltype(preserved)>, fm::recycling_allocator<int>>);

    const auto recycled = fm::get_recycling_allocator<std::allocator<int>, void>::get(std::allocator<int>{});
    STATIC_REQUIRE(std::is_same_v<std::remove_cv_t<decltype(recycled)>, fm::recycling_allocator<int, void>>);
}
