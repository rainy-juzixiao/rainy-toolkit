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
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <memory>
#include <rainy/core/annotations/lifetime_annotation.hpp>
#include <string>

using namespace rainy;
using namespace rainy::utility;
using namespace rainy::annotations::lifetime;

struct TestType {
    int value;
    std::string str;
    bool moved_from = false;

    TestType() : value(0), str("default") {
    }
    explicit TestType(int v) : value(v), str("test") {
    }
    TestType(int v, std::string s) : value(v), str(std::move(s)) {
    }

    TestType(const TestType &other) : value(other.value), str(other.str), moved_from(false) {
    }
    TestType(TestType &&other) noexcept : value(other.value), str(std::move(other.str)), moved_from(true) {
        other.value = 0;
        other.str.clear();
    }

    TestType &operator=(const TestType &other) {
        if (this != &other) {
            value = other.value;
            str = other.str;
            moved_from = false;
        }
        return *this;
    }

    TestType &operator=(TestType &&other) noexcept {
        if (this != &other) {
            value = other.value;
            str = std::move(other.str);
            moved_from = true;
            other.value = 0;
            other.str.clear();
        }
        return *this;
    }

    ~TestType() = default;

    bool operator==(const TestType &other) const {
        return value == other.value && str == other.str;
    }
};

struct MovableOnlyType {
    int value;
    MovableOnlyType() : value(0) {
    }
    explicit MovableOnlyType(int v) : value(v) {
    }
    MovableOnlyType(const MovableOnlyType &) = delete;
    MovableOnlyType(MovableOnlyType &&) noexcept = default;
    MovableOnlyType &operator=(const MovableOnlyType &) = delete;
    MovableOnlyType &operator=(MovableOnlyType &&) noexcept = default;
};

struct NonMovableType {
    int value;
    NonMovableType() : value(0) {
    }
    explicit NonMovableType(int v) : value(v) {
    }
    NonMovableType(const NonMovableType &) = default;
    NonMovableType(NonMovableType &&) = delete;
    NonMovableType &operator=(const NonMovableType &) = default;
    NonMovableType &operator=(NonMovableType &&) = delete;
};

struct CopyOnlyType {
    int value;
    CopyOnlyType() : value(0) {
    }
    explicit CopyOnlyType(int v) : value(v) {
    }
    CopyOnlyType(const CopyOnlyType &) = default;
    CopyOnlyType(CopyOnlyType &&) = delete;
    CopyOnlyType &operator=(const CopyOnlyType &) = default;
    CopyOnlyType &operator=(CopyOnlyType &&) = delete;
};

TEST_CASE("in annotation type alias works correctly", "[lifetime][in]") {
    STATIC_REQUIRE(std::is_same_v<in<int>, const int>);
    STATIC_REQUIRE(std::is_same_v<in<std::string>, const std::string &>);
    STATIC_REQUIRE(std::is_same_v<in<TestType>, const TestType &>);
}

TEST_CASE("move_from annotation type alias works correctly", "[lifetime][move_from]") {
    STATIC_REQUIRE(std::is_same_v<move_from<int>, int &&>);
    STATIC_REQUIRE(std::is_same_v<move_from<TestType>, TestType &&>);
    STATIC_REQUIRE(std::is_same_v<move_from<std::string>, std::string &&>);
}

TEST_CASE("read_only annotation type alias works correctly", "[lifetime][read_only]") {
    STATIC_REQUIRE(std::is_same_v<read_only<int>, const int &>);
    STATIC_REQUIRE(std::is_same_v<read_only<TestType>, const TestType &>);
}

TEST_CASE("static_read_only annotation type alias works correctly", "[lifetime][static_read_only]") {
    STATIC_REQUIRE(std::is_same_v<static_read_only<int>, read_only<int>>);
    STATIC_REQUIRE(std::is_same_v<static_read_only<TestType>, read_only<TestType>>);
}

TEST_CASE("deferred_init default construction and destruction", "[lifetime][deferred_init]") {
    deferred_init<TestType> di;
    REQUIRE_FALSE(di.is_init());
}

TEST_CASE("deferred_init value throws when not initialized", "[lifetime][deferred_init]") {
    deferred_init<TestType> di;
    REQUIRE_THROWS_AS(di.value(), core::exceptions::runtime::runtime_error);
}

TEST_CASE("deferred_init construct and value works", "[lifetime][deferred_init]") {
    deferred_init<TestType> di;
    di.construct(42, "hello");
    REQUIRE(di.is_init());
    REQUIRE(di.value().value == 42);
    REQUIRE(di.value().str == "hello");
}

TEST_CASE("deferred_init construct throws when already initialized", "[lifetime][deferred_init]") {
    deferred_init<TestType> di;
    di.construct(42);
    REQUIRE_THROWS_AS(di.construct(43), core::exceptions::runtime::runtime_error);
}

TEST_CASE("deferred_init destructor cleans up properly", "[lifetime][deferred_init]") {
    bool destroyed = false;
    struct TrackedDestructor {
        bool *flag;
        TrackedDestructor(bool *f) : flag(f) {
        }
        ~TrackedDestructor() {
            *flag = true;
        }
    };
    {
        deferred_init<TrackedDestructor> di;
        di.construct(&destroyed);
        REQUIRE_FALSE(destroyed);
    }
    REQUIRE(destroyed);
}

TEST_CASE("deferred_init with move-only types", "[lifetime][deferred_init]") {
    deferred_init<MovableOnlyType> di;
    di.construct(42);
    REQUIRE(di.value().value == 42);
}

TEST_CASE("deferred_init with non-copyable types", "[lifetime][deferred_init]") {
    deferred_init<MovableOnlyType> di;
    di.construct(100);
    REQUIRE(di.value().value == 100);
}

TEST_CASE("out constructs from raw pointer", "[lifetime][out]") {
    TestType obj{42, "test"};
    out<TestType> ot(&obj);
    REQUIRE(&ot.value() == &obj);
    REQUIRE(ot.value().value == 42);
    REQUIRE(ot.value().str == "test");
}

TEST_CASE("out throws when constructed with null pointer", "[lifetime][out]") {
    TestType *null_ptr = nullptr;
    REQUIRE_THROWS_AS(out<TestType>(null_ptr), core::exceptions::runtime::runtime_error);
}

TEST_CASE("out constructs from deferred_init", "[lifetime][out]") {
    deferred_init<TestType> di;
    out<TestType> ot(&di);
    di.construct(42, "test");
    REQUIRE(ot.value().value == 42);
    REQUIRE(ot.value().str == "test");
}

TEST_CASE("out constructs from another out", "[lifetime][out]") {
    TestType obj{42, "test"};
    out<TestType> ot1(&obj);
    out<TestType> ot2(&ot1);
    REQUIRE(&ot2.value() == &obj);
}

TEST_CASE("out construct with arguments on uninitialized deferred_init", "[lifetime][out]") {
    deferred_init<TestType> di;
    out<TestType> ot(&di);
    ot.construct(42, "constructed");
    REQUIRE(di.is_init());
    REQUIRE(di.value().value == 42);
    REQUIRE(di.value().str == "constructed");
}

TEST_CASE("out construct on already initialized deferred_init assigns", "[lifetime][out]") {
    deferred_init<TestType> di;
    di.construct(10, "initial");
    out<TestType> ot(&di);
    ot.construct(20, "updated");
    REQUIRE(di.value().value == 20);
    REQUIRE(di.value().str == "updated");
}

TEST_CASE("out construct on raw pointer assigns", "[lifetime][out]") {
    TestType obj{10, "initial"};
    out<TestType> ot(&obj);
    ot.construct(20, "updated");
    REQUIRE(obj.value == 20);
    REQUIRE(obj.str == "updated");
}

TEST_CASE("out called_construct flag works", "[lifetime][out]") {
    deferred_init<TestType> di;
    out<TestType> ot(&di);
    REQUIRE_FALSE(ot.called_construct());
    ot.construct(42);
    REQUIRE(ot.called_construct());
}

TEST_CASE("out destructor cleans up deferred_init on exception", "[lifetime][out]") {
    deferred_init<TestType> di;
    bool destroyed = false;
    struct TrackedDestructor {
        bool *flag;
        TrackedDestructor(bool *f) : flag(f) {
        }
        ~TrackedDestructor() {
            *flag = true;
        }
    };
    {
        deferred_init<TrackedDestructor> di2;
        out<TrackedDestructor> ot(&di2);
        try {
            ot.construct(&destroyed);
            throw core::exceptions::runtime::runtime_error("test exception");
        } catch (...) {
        }
        REQUIRE_FALSE(destroyed);
    }
}

TEST_CASE("out value throws if not initialized", "[lifetime][out]") {
    deferred_init<TestType> di;
    out<TestType> ot(&di);
    REQUIRE_THROWS_AS(ot.value(), core::exceptions::runtime::runtime_error);
}

TEST_CASE("borrow_out constructs from raw pointer", "[lifetime][borrow]") {
    TestType obj{42, "borrow"};
    borrow_out<TestType> bo(&obj);
    REQUIRE(bo.value().value == 42);
    REQUIRE(bo.value().str == "borrow");
}

TEST_CASE("borrow_out constructs from deferred_init", "[lifetime][borrow]") {
    deferred_init<TestType> di;
    borrow_out<TestType> bo(&di);
    di.construct(100, "deferred");
    REQUIRE(bo.value().value == 100);
    REQUIRE(bo.value().str == "deferred");
}

TEST_CASE("borrow_out constructs from out", "[lifetime][borrow]") {
    TestType obj{99, "from_out"};
    out<TestType> ot(&obj);
    borrow_out<TestType> bo(&ot);
    REQUIRE(bo.value().value == 99);
    REQUIRE(bo.value().str == "from_out");
}

TEST_CASE("borrow_out copy constructor shares control block", "[lifetime][borrow]") {
    TestType obj{1, "shared"};
    borrow_out<TestType> bo1(&obj);
    borrow_out<TestType> bo2(bo1);
    REQUIRE(bo1.value().value == 1);
    REQUIRE(bo2.value().value == 1);
}

TEST_CASE("borrow_out copy assignment shares control block", "[lifetime][borrow]") {
    TestType obj1{1, "first"};
    TestType obj2{2, "second"};
    borrow_out<TestType> bo1(&obj1);
    borrow_out<TestType> bo2(&obj2);
    bo2 = bo1;
    REQUIRE(bo2.value().value == 1);
    REQUIRE(bo2.value().str == "first");
}

TEST_CASE("borrow_out mut returns refwrap", "[lifetime][borrow]") {
    TestType obj{42, "mutable"};
    borrow_out<TestType> bo(&obj);
    auto mut_ref = bo.mut();
    TestType &ref = mut_ref;
    ref.value = 100;
    ref.str = "modified";
    REQUIRE(bo.value().value == 100);
    REQUIRE(bo.value().str == "modified");
}

TEST_CASE("borrow_out const_ref returns crefwrap", "[lifetime][borrow]") {
    TestType obj{42, "const"};
    borrow_out<TestType> bo(&obj);
    auto const_ref = bo.const_ref();
    const TestType &ref = const_ref;
    REQUIRE(ref.value == 42);
    REQUIRE(ref.str == "const");
}

TEST_CASE("borrow_out mut throws when mutable reference already active", "[lifetime][borrow]") {
    TestType obj{42, "test"};
    borrow_out<TestType> bo(&obj);
    auto mut1 = bo.mut();
    TestType &ph = mut1;
    REQUIRE_THROWS_AS(bo.mut(), core::exceptions::runtime::runtime_error);
}

TEST_CASE("borrow_out const_ref throws when mutable reference active", "[lifetime][borrow]") {
    TestType obj{42, "test"};
    borrow_out<TestType> bo(&obj);
    auto mut = bo.mut();
    TestType &ph = mut;
    REQUIRE_THROWS_AS(bo.const_ref(), core::exceptions::runtime::runtime_error);
}

TEST_CASE("borrow_out mut throws when immutable references exist", "[lifetime][borrow]") {
    TestType obj{42, "test"};
    borrow_out<TestType> bo(&obj);
    auto const_ref = bo.const_ref();
    const TestType &ph = const_ref;
    REQUIRE_THROWS_AS(bo.mut(), core::exceptions::runtime::runtime_error);
}

TEST_CASE("take construct from lvalue reference", "[lifetime][take]") {
    TestType obj{42, "original"};
    take<TestType> tk(obj);
    REQUIRE(tk.get().moved_from);
    REQUIRE(tk.get().value == 42);
    REQUIRE(tk.get().str == "original");
}

TEST_CASE("take construct from rvalue reference", "[lifetime][take]") {
    take<TestType> tk(TestType{42, "rvalue"});
    REQUIRE(tk.get().value == 42);
    REQUIRE(tk.get().str == "rvalue");
}

TEST_CASE("take move constructor", "[lifetime][take]") {
    TestType obj{42, "moved"};
    take<TestType> tk1(obj);
    take<TestType> tk2(std::move(tk1));
    REQUIRE(tk2.get().value == 42);
    REQUIRE(tk2.get().str == "moved");
}

TEST_CASE("take conversion to reference", "[lifetime][take]") {
    TestType obj{42, "test"};
    take<TestType> tk(obj);
    TestType &ref = tk;
    ref.value = 100;
    REQUIRE(tk.get().value == 100);
}

TEST_CASE("take address-of operator", "[lifetime][take]") {
    TestType obj{42, "test"};
    take<TestType> tk(obj);
    TestType *ptr = &tk;
    REQUIRE(ptr == &tk.get());
}

TEST_CASE("take get returns reference", "[lifetime][take]") {
    TestType obj{42, "test"};
    take<TestType> tk(obj);
    TestType &ref = tk.get();
    ref.value = 100;
    REQUIRE(tk.get().value == 100);
}

TEST_CASE("take assignment operator", "[lifetime][take]") {
    TestType obj1{1, "first"};
    TestType obj2{2, "second"};
    take<TestType> tk(obj1);
    tk = obj2;
    REQUIRE(tk.get().value == 2);
    REQUIRE(tk.get().str == "second");
}

TEST_CASE("take swap", "[lifetime][take]") {
    TestType obj1{1, "first"};
    TestType obj2{2, "second"};
    take<TestType> tk1(obj1);
    take<TestType> tk2(obj2);
    tk1.swap(tk2);
    REQUIRE(tk1.get().value == 2);
    REQUIRE(tk1.get().str == "second");
    REQUIRE(tk2.get().value == 1);
    REQUIRE(tk2.get().str == "first");
}

TEST_CASE("take move assignment", "[lifetime][take]") {
    TestType obj1{1, "first"};
    TestType obj2{2, "second"};
    take<TestType> tk1(obj1);
    take<TestType> tk2(obj2);
    tk1 = std::move(tk2);
    REQUIRE(tk1.get().value == 2);
    REQUIRE(tk1.get().str == "second");
}

TEST_CASE("take with move-only type works", "[lifetime][take]") {
    MovableOnlyType obj{42};
    take<MovableOnlyType> tk(std::move(obj));
    REQUIRE(tk.get().value == 42);
}

TEST_CASE("take with invocable type", "[lifetime][take]") {
    auto lambda = [](int x) { return x * 2; };
    take<decltype(lambda)> tk(lambda);
    REQUIRE(tk(21) == 42);
}

TEST_CASE("uninitialized default construction works", "[lifetime][uninitialized]") {
    uninitialized<int> ui;
    ui.value = 42;
    REQUIRE(ui.value == 42);
}

TEST_CASE("uninitialized copy and move", "[lifetime][uninitialized]") {
    uninitialized<int> ui1;
    ui1.value = 42;
    uninitialized<int> ui2(ui1);
    REQUIRE(ui2.value == 42);
    uninitialized<int> ui3(std::move(ui2));
    REQUIRE(ui3.value == 42);
}

TEST_CASE("uninitialized const specialization is deleted", "[lifetime][uninitialized]") {
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<uninitialized<const int>>);
}

TEST_CASE("uninitialized reference specialization is deleted", "[lifetime][uninitialized]") {
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<uninitialized<int &>>);
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<uninitialized<const int &>>);
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<uninitialized<int &&>>);
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<uninitialized<const int &&>>);
}

TEST_CASE("utility aliases match annotations", "[lifetime][utility]") {
    STATIC_REQUIRE(std::is_same_v<in<int>, rainy::utility::in<int>>);
    STATIC_REQUIRE(std::is_same_v<move_from<int>, rainy::utility::move_from<int>>);
    STATIC_REQUIRE(std::is_same_v<read_only<int>, rainy::utility::read_only<int>>);
    STATIC_REQUIRE(std::is_same_v<static_read_only<int>, rainy::utility::static_read_only<int>>);
    STATIC_REQUIRE(std::is_same_v<take<int>, rainy::utility::take<int>>);
}
