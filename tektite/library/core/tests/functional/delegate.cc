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
#include <rainy/core/functional/delegate.hpp>

using namespace rainy::functional;

static int add(int a, int b) {
    return a + b;
}

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

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = default;
    NonCopyable &operator=(NonCopyable &&) = default;
    int operator()(int x) const {
        return x * 2;
    }
};

TEST_CASE("delegate default construction", "[delegate]") {
    delegate<int(int)> d;
    REQUIRE(d.empty());
    REQUIRE_FALSE(d);
}

TEST_CASE("delegate from lambda", "[delegate]") {
    delegate<int(int)> d = [](int x) { return x * 2; };
    REQUIRE_FALSE(d.empty());
    REQUIRE(d(5) == 10);
}

TEST_CASE("delegate from function pointer", "[delegate]") {
    delegate<int(int, int)> d = TestClass::static_add;
    REQUIRE(d(3, 4) == 7);
}

TEST_CASE("delegate from member function pointer", "[delegate]") {
    delegate d = &TestClass::add;
    TestClass obj;
    REQUIRE(d(obj, 10, 20) == 30);
}

TEST_CASE("delegate from const member function pointer", "[delegate]") {
    delegate<int (TestClass::*)() const> d = &TestClass::get;
    const TestClass obj{42};
    REQUIRE(d(obj) == 42);
}

TEST_CASE("delegate copy construction", "[delegate]") {
    delegate<int(int)> d1 = [](int x) { return x + 1; };
    delegate<int(int)> d2(d1);
    REQUIRE(d2(10) == 11);
}

TEST_CASE("delegate move construction", "[delegate]") {
    delegate<int(int)> d1 = [](int x) { return x + 1; };
    delegate<int(int)> d2(std::move(d1));
    REQUIRE(d2(10) == 11);
    REQUIRE(d1.empty());
}

TEST_CASE("delegate copy assignment", "[delegate]") {
    delegate<int(int)> d1 = [](int x) { return x * 3; };
    delegate<int(int)> d2;
    d2 = d1;
    REQUIRE(d2(4) == 12);
}

TEST_CASE("delegate move assignment", "[delegate]") {
    delegate<int(int)> d1 = [](int x) { return x * 4; };
    delegate<int(int)> d2;
    d2 = std::move(d1);
    REQUIRE(d2(5) == 20);
    REQUIRE(d1.empty());
}

TEST_CASE("delegate assignment from lambda", "[delegate]") {
    delegate<int(int)> d;
    d = [](int x) { return x - 1; };
    REQUIRE(d(10) == 9);
}

TEST_CASE("delegate assignment from nullptr", "[delegate]") {
    delegate<int(int)> d = [](int x) { return x; };
    REQUIRE_FALSE(d.empty());
    d = nullptr;
    REQUIRE(d.empty());
}

TEST_CASE("delegate reset", "[delegate]") {
    delegate<int(int)> d = [](int x) { return x; };
    d.reset();
    REQUIRE(d.empty());
}

TEST_CASE("delegate swap", "[delegate]") {
    delegate<int(int)> d1 = [](int x) { return x + 1; };
    delegate<int(int)> d2 = [](int x) { return x * 2; };
    d1.swap(d2);
    REQUIRE(d1(5) == 10);
    REQUIRE(d2(5) == 6);
}

TEST_CASE("delegate void return type", "[delegate]") {
    int counter = 0;
    delegate<void(int)> d = [&counter](int x) { counter += x; };
    d(5);
    REQUIRE(counter == 5);
    d(3);
    REQUIRE(counter == 8);
}

TEST_CASE("delegate with multiple arguments", "[delegate]") {
    delegate<int(int, int, int)> d = [](int a, int b, int c) { return a + b + c; };
    REQUIRE(d(1, 2, 3) == 6);
}

TEST_CASE("delegate with void member function", "[delegate]") {
    delegate<void (TestClass::*)(int)> d = &TestClass::set;
    TestClass obj;
    d(obj, 77);
    REQUIRE(obj.value == 77);
}

TEST_CASE("delegate target type info", "[delegate]") {
    auto lambda = [](int x) { return x + 1; };
    delegate<int(int)> d(lambda);
    REQUIRE(d.target_type() == rainy_typeid(decltype(lambda)));
}

TEST_CASE("delegate target pointer for function", "[delegate]") {
    delegate<int(int, int)> d = add;
    auto *ptr = d.target<int (*)(int, int)>();
    REQUIRE(ptr != nullptr);
    REQUIRE((*ptr)(3, 4) == 7);
}

TEST_CASE("delegate target pointer for lambda", "[delegate]") {
    auto lambda = [](int x) { return x * 3; };
    delegate<int(int)> d = lambda;
    auto *ptr = d.target<decltype(lambda)>();
    REQUIRE(ptr != nullptr);
    REQUIRE((*ptr)(5) == 15);
}

TEST_CASE("delegate target pointer for member function", "[delegate]") {
    delegate<int (TestClass::*)(int, int)> d = &TestClass::add;
    auto *ptr = d.target<decltype(&TestClass::add)>();
    REQUIRE(ptr != nullptr);
    TestClass obj;
    REQUIRE((obj.*(*ptr))(2, 3) == 5);
}

TEST_CASE("delegate is_const", "[delegate]") {
    delegate<int (TestClass::*)() const> d = &TestClass::get;
    REQUIRE(d.is_const());
    delegate<void (TestClass::*)(int)> d2 = &TestClass::set;
    REQUIRE_FALSE(d2.is_const());
}

TEST_CASE("delegate is_static", "[delegate]") {
    delegate<int(int, int)> d = TestClass::static_add;
    REQUIRE(d.is_static());
    delegate<int (TestClass::*)(int, int)> d2 = &TestClass::add;
    REQUIRE_FALSE(d2.is_static());
}

TEST_CASE("delegate with non-copyable callable", "[delegate]") {
    NonCopyable nc;
    delegate<int(int)> d = std::move(nc);
    REQUIRE(d(5) == 10);
}

TEST_CASE("move_only_delegate construction", "[move_only_delegate]") {
    move_only_delegate<int(int)> d = [](int x) { return x * 2; };
    REQUIRE(d(7) == 14);
}

TEST_CASE("move_only_delegate move construction", "[move_only_delegate]") {
    move_only_delegate<int(int)> d1 = [](int x) { return x + 3; };
    move_only_delegate<int(int)> d2(std::move(d1));
    REQUIRE(d2(10) == 13);
    REQUIRE(d1.empty());
}

TEST_CASE("move_only_delegate move assignment", "[move_only_delegate]") {
    move_only_delegate<int(int)> d1 = [](int x) { return x * 5; };
    move_only_delegate<int(int)> d2;
    d2 = std::move(d1);
    REQUIRE(d2(2) == 10);
    REQUIRE(d1.empty());
}

TEST_CASE("move_only_delegate copy disabled", "[move_only_delegate]") {
    move_only_delegate<int(int)> d1 = [](int x) { return x; };
    static_assert(!std::is_copy_constructible_v<move_only_delegate<int(int)>>);
    static_assert(!std::is_copy_assignable_v<move_only_delegate<int(int)>>);
}

TEST_CASE("move_only_delegate from non-copyable", "[move_only_delegate]") {
    NonCopyable nc;
    move_only_delegate<int(int)> d(std::move(nc));
    REQUIRE(d(6) == 12);
}

TEST_CASE("delegate exception on empty call", "[delegate]") {
    delegate<void()> d;
    REQUIRE_THROWS_AS(d(), exceptions::invalid_delegate);
    REQUIRE_THROWS_AS(d.invoke(), exceptions::invalid_delegate);
}

TEST_CASE("delegate exception on empty call with return type", "[delegate]") {
    delegate<int()> d;
    REQUIRE_THROWS_AS(d(), exceptions::invalid_delegate);
    REQUIRE_THROWS_AS(d.invoke(), exceptions::invalid_delegate);
}

TEST_CASE("delegate with noexcept function", "[delegate]") {
    auto fn = [](int x) noexcept { return x * 2; };
    delegate<int(int)> d = fn;
    REQUIRE(d.is_noexcept());
    REQUIRE(d(5) == 10);
}

TEST_CASE("delegate with lvalue qualified member", "[delegate]") {
    struct S {
        int value = 0;
        int &get() & {
            return value;
        }
    };
    delegate<int  &(S::*)() &> d = &S::get;
    S obj;
    d(obj) = 42;
    REQUIRE(obj.value == 42);
    REQUIRE(d.is_invoke_for_lvalue());
}

TEST_CASE("delegate with rvalue qualified member", "[delegate]") {
    struct S {
        int value = 0;
        int get() && {
            return value + 1;
        }
    };
    delegate<int (S::*)() &&> d = &S::get;
    S obj{5};
    // REQUIRE(d(obj) == 6);
    // REQUIRE(d.is_invoke_for_rvalue());
}

TEST_CASE("delegate with volatile qualified member", "[delegate]") {
    struct S {
        int get() volatile {
            return 42;
        }
    };
    delegate<int (S::*)() volatile> d = &S::get;
    volatile S obj;
    REQUIRE(d(obj) == 42);
    REQUIRE(d.is_volatile());
}

TEST_CASE("delegate multiple assignment", "[delegate]") {
    delegate<int(int)> d;
    d = [](int x) { return x + 1; };
    REQUIRE(d(1) == 2);
    d = [](int x) { return x * 2; };
    REQUIRE(d(3) == 6);
    d = nullptr;
    REQUIRE(d.empty());
    d = [](int x) { return x - 1; };
    REQUIRE(d(5) == 4);
}

TEST_CASE("delegate with std::function compatible", "[delegate]") {
    std::function<int(int)> f = [](int x) { return x + 5; };
    delegate<int(int)> d = f;
    REQUIRE(d(10) == 15);
}

TEST_CASE("delegate bool conversion", "[delegate]") {
    delegate<int(int)> d;
    REQUIRE_FALSE(static_cast<bool>(d));
    d = [](int x) { return x; };
    REQUIRE(static_cast<bool>(d));
}

TEST_CASE("move_only_delegate from move-only lambda", "[move_only_delegate]") {
    auto lambda = [p = std::make_unique<int>(42)](int x) { return *p + x; };
    move_only_delegate<int(int)> d(std::move(lambda));
    REQUIRE(d(8) == 50);
}

TEST_CASE("delegate has flags", "[delegate]") {
    struct S {
        int f() const noexcept {
            return 1;
        }
        static int g() {
            return 2;
        }
    };
    delegate<int (S::*)() const noexcept> d1 = &S::f;
    REQUIRE(d1.is_const());
    REQUIRE(d1.is_noexcept());
    delegate<int()> d2 = S::g;
    REQUIRE(d2.is_static());
}

TEST_CASE("delegate function_signature", "[delegate]") {
    delegate<int(int)> d = [](int x) { return x; };
    REQUIRE(d.function_signature() == rainy_typeid(int(int)));
}

TEST_CASE("delegate return_type", "[delegate]") {
    delegate<int(int)> d = [](int x) { return x; };
    REQUIRE(d.return_type() == rainy_typeid(int));
    delegate<void()> d2 = []() {};
    REQUIRE(d2.return_type() == rainy_typeid(void));
}

TEST_CASE("delegate which_belongs", "[delegate]") {
    delegate<int (TestClass::*)(int, int)> d = &TestClass::add;
    REQUIRE(d.which_belongs() == rainy_typeid(TestClass));
    delegate<int(int, int)> d2 = TestClass::static_add;
    REQUIRE(d2.which_belongs() == rainy_typeid(void));
}

TEST_CASE("delegate with std::bind result", "[delegate]") {
    TestClass obj{100};
    auto bound = std::bind(&TestClass::get, std::cref(obj));
    delegate<int()> d = bound;
    REQUIRE(d() == 100);
}

TEST_CASE("move_only_delegate reset", "[move_only_delegate]") {
    move_only_delegate<int(int)> d = [](int x) { return x + 1; };
    REQUIRE_FALSE(d.empty());
    d.reset();
    REQUIRE(d.empty());
}

TEST_CASE("move_only_delegate assignment from nullptr", "[move_only_delegate]") {
    move_only_delegate<int(int)> d = [](int x) { return x; };
    d = nullptr;
    REQUIRE(d.empty());
}
