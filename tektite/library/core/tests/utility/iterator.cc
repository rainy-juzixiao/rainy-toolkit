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
#include <rainy/core/utility/iterator.hpp>

using namespace rainy::utility;

TEST_CASE("input_iterator_pointer - wraps a value", "[iterator][pointer]") {
    input_iterator_pointer ptr{42};
    REQUIRE(*ptr == 42);
}

TEST_CASE("input_iterator_pointer - arrow operator", "[iterator][pointer]") {
    struct Point { int x; int y; };
    input_iterator_pointer p{Point{1, 2}};
    REQUIRE(p->x == 1);
    REQUIRE(p->y == 2);
}

TEST_CASE("input_iterator_pointer - move from lvalue", "[iterator][pointer]") {
    int val = 42;
    input_iterator_pointer ptr{std::move(val)};
    REQUIRE(*ptr == 42);
}