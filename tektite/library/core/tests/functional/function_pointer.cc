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
#include <catch2/matchers/catch_matchers.hpp>
#include <rainy/core/functional/function_pointer.hpp>

using namespace rainy::functional;

struct TestClass {
    int value = 0;
    int add(int a, int b) {
        return a + b;
    }
    int get() const {
        return value;
    }
    void set(int v) {
        value = v;
    }
    static int static_add(int a, int b) {
        return a + b;
    }
};

int free_add(int a, int b) {
    return a + b;
}
int free_mul(int a, int b) {
    return a * b;
}

TEST_CASE("function_pointer default construction", "[function_pointer]") {
    function_pointer<int(int)> fp;
    REQUIRE(fp.empty());
    REQUIRE_FALSE(fp);
    REQUIRE(fp.get() == nullptr);
}

TEST_CASE("function_pointer from function pointer", "[function_pointer]") {
    function_pointer<int(int, int)> fp = free_add;
    REQUIRE_FALSE(fp.empty());
    REQUIRE(fp(3, 4) == 7);
    REQUIRE(fp.get() == &free_add);
}

TEST_CASE("function_pointer from static member function", "[function_pointer]") {
    function_pointer<int(int, int)> fp = TestClass::static_add;
    REQUIRE_FALSE(fp.empty());
    REQUIRE(fp(5, 6) == 11);
}

TEST_CASE("function_pointer from member function pointer", "[function_pointer]") {
    function_pointer<int (TestClass::*)(int, int)> fp = &TestClass::add;
    TestClass obj;
    REQUIRE_FALSE(fp.empty());
    REQUIRE(fp(obj, 10, 20) == 30);
}

TEST_CASE("function_pointer from const member function", "[function_pointer]") {
    function_pointer<int (TestClass::*)() const> fp = &TestClass::get;
    const TestClass obj{42};
    REQUIRE(fp(obj) == 42);
}

TEST_CASE("function_pointer from void member function", "[function_pointer]") {
    function_pointer<void (TestClass::*)(int)> fp = &TestClass::set;
    TestClass obj;
    fp(obj, 77);
    REQUIRE(obj.value == 77);
}

TEST_CASE("function_pointer copy construction", "[function_pointer]") {
    function_pointer<int(int, int)> fp1 = free_add;
    function_pointer<int(int, int)> fp2(fp1);
    REQUIRE(fp2(3, 4) == 7);
    REQUIRE(fp2.get() == fp1.get());
}

TEST_CASE("function_pointer copy assignment", "[function_pointer]") {
    function_pointer<int(int, int)> fp1 = free_add;
    function_pointer<int(int, int)> fp2;
    fp2 = fp1;
    REQUIRE(fp2(3, 4) == 7);
    REQUIRE(fp2.get() == fp1.get());
}

TEST_CASE("function_pointer assignment from function pointer", "[function_pointer]") {
    function_pointer<int(int, int)> fp;
    fp = free_mul;
    REQUIRE(fp(3, 4) == 12);
    fp = free_add;
    REQUIRE(fp(3, 4) == 7);
}

TEST_CASE("function_pointer reset", "[function_pointer]") {
    function_pointer<int(int, int)> fp = free_add;
    REQUIRE_FALSE(fp.empty());
    fp.reset();
    REQUIRE(fp.empty());
    REQUIRE(fp.get() == nullptr);
}

TEST_CASE("function_pointer swap", "[function_pointer]") {
    function_pointer<int(int, int)> fp1 = free_add;
    function_pointer<int(int, int)> fp2 = free_mul;
    fp1.swap(fp2);
    REQUIRE(fp1(3, 4) == 12);
    REQUIRE(fp2(3, 4) == 7);
}

TEST_CASE("function_pointer free swap", "[function_pointer]") {
    function_pointer<int(int, int)> fp1 = free_add;
    function_pointer<int(int, int)> fp2 = free_mul;
    swap(fp1, fp2);
    REQUIRE(fp1(3, 4) == 12);
    REQUIRE(fp2(3, 4) == 7);
}

static int fp_void_counter = 0;
static void fp_void_increment(int x) {
    fp_void_counter += x;
}

TEST_CASE("function_pointer void return type", "[function_pointer]") {
    function_pointer<void(int)> fp = fp_void_increment;
    fp(5);
    REQUIRE(fp_void_counter == 5);
    fp(3);
    REQUIRE(fp_void_counter == 8);
}

TEST_CASE("function_pointer with multiple arguments", "[function_pointer]") {
    function_pointer<int(int, int, int)> fp = [](int a, int b, int c) { return a + b + c; };
    REQUIRE(fp(1, 2, 3) == 6);
}

TEST_CASE("function_pointer get pointer", "[function_pointer]") {
    function_pointer<int(int, int)> fp = free_add;
    auto ptr = fp.get();
    REQUIRE(ptr == &free_add);
    REQUIRE(ptr(10, 20) == 30);
}

TEST_CASE("function_pointer explicit conversion to bool", "[function_pointer]") {
    function_pointer<int(int)> fp;
    REQUIRE_FALSE(static_cast<bool>(fp));
    fp = [](int x) { return x; };
    REQUIRE(static_cast<bool>(fp));
}

TEST_CASE("function_pointer conversion to pointer", "[function_pointer]") {
    function_pointer<int(int, int)> fp = free_add;
    int (*ptr)(int, int) = fp;
    REQUIRE(ptr == &free_add);
    REQUIRE(ptr(5, 6) == 11);
}

TEST_CASE("function_pointer null pointer conversion", "[function_pointer]") {
    function_pointer<int(int, int)> fp;
    int (*ptr)(int, int) = fp;
    REQUIRE(ptr == nullptr);
}

TEST_CASE("function_pointer target_type", "[function_pointer]") {
    function_pointer<int(int, int)> fp = free_add;
    REQUIRE(fp.target_type() == rainy_typeid(int (int, int)));
}

TEST_CASE("function_pointer type flags", "[function_pointer]") {
    function_pointer<int(int, int)> fp = free_add;
    REQUIRE((fp.type() & method_flags::static_specified) == method_flags::static_specified);
}

TEST_CASE("function_pointer with lambda", "[function_pointer]") {
    auto lambda = [](int x) { return x * 3; };
    function_pointer<int(int)> fp = lambda;
    REQUIRE(fp(5) == 15);
}

TEST_CASE("function_pointer with const lambda", "[function_pointer]") {
    const auto lambda = [](int x) { return x + 1; };
    function_pointer<int(int)> fp = lambda;
    REQUIRE(fp(9) == 10);
}

TEST_CASE("function_pointer with noexcept function", "[function_pointer]") {
    auto fn = [](int x) noexcept { return x * 2; };
    function_pointer<int(int)> fp = fn;
    REQUIRE(fp(5) == 10);
}

TEST_CASE("function_pointer invoke with empty throws", "[function_pointer]") {
    function_pointer<int(int, int)> fp;
    REQUIRE_THROWS_AS(fp(1, 2), rainy::core::exceptions::runtime::nullpointer_exception);
}

TEST_CASE("function_pointer invoke member with empty throws", "[function_pointer]") {
    function_pointer<int (TestClass::*)(int, int)> fp;
    TestClass obj;
    REQUIRE_THROWS_AS(fp(obj, 1, 2), rainy::core::exceptions::runtime::nullpointer_exception);
}

TEST_CASE("function_pointer reassign different functions", "[function_pointer]") {
    function_pointer<int(int, int)> fp;
    fp = free_add;
    REQUIRE(fp(2, 3) == 5);
    fp = free_mul;
    REQUIRE(fp(2, 3) == 6);
    fp = nullptr;
    REQUIRE(fp.empty());
    fp = [](int a, int b) { return a - b; };
    REQUIRE(fp(5, 3) == 2);
}

TEST_CASE("function_pointer with std::function", "[function_pointer]") {
    std::function<int(int)> f = [](int x) { return x + 5; };
    REQUIRE(f.target<int (*)(int)>() == nullptr);
}

TEST_CASE("function_pointer with free function pointer type", "[function_pointer]") {
    using FpType = int (*)(int, int);
    function_pointer<FpType> fp = free_add;
    REQUIRE(fp(7, 8) == 15);
    REQUIRE(fp.get() == free_add);
}

TEST_CASE("function_pointer with static member pointer type", "[function_pointer]") {
    using FpType = int (*)(int, int);
    function_pointer<FpType> fp = TestClass::static_add;
    REQUIRE(fp(4, 5) == 9);
}

TEST_CASE("function_pointer with member pointer type", "[function_pointer]") {
    using MemFnType = int (TestClass::*)(int, int);
    function_pointer<MemFnType> fp = &TestClass::add;
    TestClass obj{0};
    REQUIRE(fp(obj, 6, 7) == 13);
}

TEST_CASE("function_pointer with const member pointer type", "[function_pointer]") {
    using MemFnType = int (TestClass::*)() const;
    function_pointer<MemFnType> fp = &TestClass::get;
    const TestClass obj{99};
    REQUIRE(fp(obj) == 99);
}

TEST_CASE("function_pointer with void member pointer type", "[function_pointer]") {
    using MemFnType = void (TestClass::*)(int);
    function_pointer<MemFnType> fp = &TestClass::set;
    TestClass obj;
    fp(obj, 123);
    REQUIRE(obj.value == 123);
}

TEST_CASE("function_pointer lvalue qualified member", "[function_pointer]") {
    struct S {
        int value = 0;
        int &get() & {
            return value;
        }
    };
    using MemFnType = int &(S::*) () &;
    function_pointer<MemFnType> fp = &S::get;
    S obj;
    fp(obj) = 42;
    REQUIRE(obj.value == 42);
}

TEST_CASE("function_pointer rvalue qualified member", "[function_pointer]") {
    struct S {
        int value = 0;
        int get() && {
            return value + 1;
        }
    };
    using MemFnType = int (S::*)() &&;
    function_pointer<MemFnType> fp = &S::get;
    S obj{5};
    REQUIRE(fp(obj) == 6);
}

TEST_CASE("function_pointer volatile qualified member", "[function_pointer]") {
    struct S {
        int get() volatile {
            return 42;
        }
    };
    using MemFnType = int (S::*)() volatile;
    function_pointer<MemFnType> fp = &S::get;
    volatile S obj;
    REQUIRE(fp(obj) == 42);
}

static int global = 10;

int &ref_return(int) {
    return global;
}

TEST_CASE("function_pointer with function returning reference", "[function_pointer]") {

    function_pointer<int &(int)> fp = ref_return;
    fp(0) = 20;
    REQUIRE(global == 20);
}

TEST_CASE("function_pointer with const function pointer", "[function_pointer]") {
    const function_pointer<int(int, int)> fp = free_add;
    REQUIRE(fp(2, 3) == 5);
}

TEST_CASE("function_pointer from nullptr", "[function_pointer]") {
    function_pointer<int(int, int)> fp = nullptr;
    REQUIRE(fp.empty());
    REQUIRE(fp.get() == nullptr);
    REQUIRE_FALSE(fp);
}
