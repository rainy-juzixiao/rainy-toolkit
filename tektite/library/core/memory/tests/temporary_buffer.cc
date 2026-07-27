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
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/memory/temporary_buffer.hpp>

using namespace rainy;
using namespace rainy::core::memory;

TEST_CASE("temporary_buffer basic allocation", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(10);

    REQUIRE(buffer);
    REQUIRE(buffer.get_buffer() != nullptr);

    buffer[0] = 42;
    buffer[1] = 100;

    REQUIRE(buffer[0] == 42);
    REQUIRE(buffer[1] == 100);
}

TEST_CASE("temporary_buffer iterator access", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(5);

    for (int i = 0; i < 5; ++i) {
        buffer.begin()[i] = i * 2;
    }

    int index = 0;
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        REQUIRE(*it == index * 2);
        ++index;
    }
}

TEST_CASE("temporary_buffer bool conversion", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(4);

    REQUIRE(static_cast<bool>(buffer));

    buffer.return_buffer();

    REQUIRE_FALSE(static_cast<bool>(buffer));
    REQUIRE(buffer.get_buffer() == nullptr);
}

TEST_CASE("temporary_buffer reallocate expand", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(3);

    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;

    auto old_buffer = buffer.get_buffer();

    buffer.reallocate(8);

    REQUIRE(buffer);
    REQUIRE(buffer.get_buffer() != nullptr);
    REQUIRE(buffer[0] == 1);
    REQUIRE(buffer[1] == 2);
    REQUIRE(buffer[2] == 3);
    REQUIRE(buffer.get_buffer() != old_buffer);
}

TEST_CASE("temporary_buffer reallocate shrink", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(8);

    for (int i = 0; i < 8; ++i) {
        buffer[i] = i;
    }

    auto old_buffer = buffer.get_buffer();

    buffer.reallocate(4);

    REQUIRE(buffer.get_buffer() == old_buffer);
    REQUIRE(buffer[0] == 0);
    REQUIRE(buffer[3] == 3);
}

TEST_CASE("temporary_buffer reallocate zero", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(5);

    buffer.reallocate(0);

    REQUIRE(buffer);
}

TEST_CASE("temporary_buffer move constructor", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(6);
    auto ptr = buffer.get_buffer();

    buffer[0] = 99;

    auto moved = std::move(buffer);

    REQUIRE(moved.get_buffer() == ptr);
    REQUIRE(moved[0] == 99);
    REQUIRE(moved);

    REQUIRE_FALSE(buffer);
    REQUIRE(buffer.get_buffer() == nullptr);
}

TEST_CASE("temporary_buffer move assignment", "[temporary_buffer]") {
    auto first = get_temporary_buffer<int>(4);
    auto second = get_temporary_buffer<int>(8);

    auto ptr = second.get_buffer();

    second[0] = 123;

    first = std::move(second);

    REQUIRE(first.get_buffer() == ptr);
    REQUIRE(first[0] == 123);
    REQUIRE(first);

    REQUIRE_FALSE(second);
}

TEST_CASE("temporary_buffer const access", "[temporary_buffer]") {
    const auto buffer = get_temporary_buffer<int>(3);

    REQUIRE(buffer.get_buffer() != nullptr);
    REQUIRE(buffer.begin() == buffer.get_buffer());
    REQUIRE(buffer.end() == buffer.get_buffer() + 3);
}

TEST_CASE("return_temporary_buffer works", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(10);

    REQUIRE(buffer);

    return_temporary_buffer(buffer);

    REQUIRE_FALSE(buffer);
    REQUIRE(buffer.get_buffer() == nullptr);
}

TEST_CASE("temporary_buffer raw pointer conversion", "[temporary_buffer]") {
    auto buffer = get_temporary_buffer<int>(2);

    int *ptr = buffer;

    REQUIRE(ptr == buffer.get_buffer());

    const auto &const_buffer = buffer;
    const int *const_ptr = const_buffer;

    REQUIRE(const_ptr == buffer.get_buffer());
}
