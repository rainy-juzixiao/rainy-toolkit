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
#include <rainy/core/concurrency/tss_ptr.hpp>

using rainy::core::concurrency::tss_ptr;

TEST_CASE("tss_ptr default construction", "[tss_ptr]") {
    tss_ptr<int> ptr;
    REQUIRE(static_cast<int*>(ptr) == nullptr);
}

TEST_CASE("tss_ptr assignment and retrieval", "[tss_ptr]") {
    tss_ptr<int> ptr;
    int value = 42;
    ptr = &value;
    REQUIRE(static_cast<int*>(ptr) == &value);
    REQUIRE(*static_cast<int*>(ptr) == 42);
}

TEST_CASE("tss_ptr assignment to nullptr", "[tss_ptr]") {
    tss_ptr<int> ptr;
    int value = 42;
    ptr = &value;
    REQUIRE(static_cast<int*>(ptr) != nullptr);
    ptr = nullptr;
    REQUIRE(static_cast<int*>(ptr) == nullptr);
}

TEST_CASE("tss_ptr multiple assignments", "[tss_ptr]") {
    tss_ptr<int> ptr;
    int value1 = 10;
    int value2 = 20;
    ptr = &value1;
    REQUIRE(*static_cast<int*>(ptr) == 10);
    ptr = &value2;
    REQUIRE(*static_cast<int*>(ptr) == 20);
}

TEST_CASE("tss_ptr with different types", "[tss_ptr]") {
    tss_ptr<double> ptr;
    double value = 3.14;
    ptr = &value;
    REQUIRE(*static_cast<double*>(ptr) == 3.14);
}

TEST_CASE("tss_ptr implicit conversion to pointer", "[tss_ptr]") {
    tss_ptr<int> ptr;
    int value = 100;
    ptr = &value;
    int* raw_ptr = ptr;
    REQUIRE(raw_ptr == &value);
    REQUIRE(*raw_ptr == 100);
}

TEST_CASE("tss_ptr with struct type", "[tss_ptr]") {
    struct TestStruct {
        int a;
        double b;
    };
    tss_ptr<TestStruct> ptr;
    TestStruct ts{5, 2.5};
    ptr = &ts;
    REQUIRE(static_cast<TestStruct*>(ptr)->a == 5);
    REQUIRE(static_cast<TestStruct*>(ptr)->b == 2.5);
}

TEST_CASE("tss_ptr chained operations", "[tss_ptr]") {
    tss_ptr<int> ptr;
    int value = 7;
    ptr = &value;
    int* p1 = ptr;
    int* p2 = ptr;
    REQUIRE(p1 == p2);
    REQUIRE(*p1 == 7);
}

TEST_CASE("tss_ptr after destruction", "[tss_ptr]") {
    tss_ptr<int>* ptr = new tss_ptr<int>();
    int value = 5;
    *ptr = &value;
    REQUIRE(static_cast<int*>(*ptr) == &value);
    delete ptr;
}

TEST_CASE("tss_ptr reusing after assignment", "[tss_ptr]") {
    tss_ptr<int> ptr;
    int a = 1, b = 2, c = 3;
    ptr = &a;
    REQUIRE(*static_cast<int*>(ptr) == 1);
    ptr = &b;
    REQUIRE(*static_cast<int*>(ptr) == 2);
    ptr = &c;
    REQUIRE(*static_cast<int*>(ptr) == 3);
}

TEST_CASE("tss_ptr with const values", "[tss_ptr]") {
    tss_ptr<const int> ptr;
    const int value = 99;
    ptr = &value;
    REQUIRE(*static_cast<const int*>(ptr) == 99);
}

TEST_CASE("tss_ptr with array type", "[tss_ptr]") {
    tss_ptr<int[]> ptr;
    int arr[5] = {1, 2, 3, 4, 5};
    ptr = arr;
    int* p = ptr;
    REQUIRE(p[0] == 1);
    REQUIRE(p[4] == 5);
}