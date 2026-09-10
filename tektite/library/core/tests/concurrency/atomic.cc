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
#include <rainy/core/concurrency/atomic.hpp>

#include <chrono>
#include <cstdint>
#include <thread>
#include <type_traits>
#include <vector>

using namespace rainy::core;
using namespace rainy::core::concurrency;

namespace {
    constexpr memory_order all_orders[] = {memory_order::relaxed, memory_order::consume, memory_order::acquire,
                                             memory_order::release, memory_order::acq_rel, memory_order::seq_cst};
}

TEMPLATE_TEST_CASE("atomic integral basic operations", "[atomic][integral]", char, signed char, unsigned char, short,
                   unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long,
                   std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, std::int32_t, std::uint32_t,
                   std::int64_t, std::uint64_t, std::size_t, std::ptrdiff_t) {
    SECTION("type properties") {
        STATIC_REQUIRE(std::is_trivially_copyable_v<TestType>);
        STATIC_REQUIRE(std::is_copy_constructible_v<atomic<TestType>> == false);
        STATIC_REQUIRE(std::is_copy_assignable_v<atomic<TestType>> == false);
        STATIC_REQUIRE(atomic<TestType>::is_always_lock_free);
        using value_type = typename atomic<TestType>::value_type;
        STATIC_REQUIRE(std::is_same_v<value_type, TestType>);
    }

    SECTION("construction") {
        atomic<TestType> a;
        REQUIRE(a.load() == TestType{0});
        atomic<TestType> b{TestType(5)};
        REQUIRE(b.load() == TestType(5));
    }

    SECTION("is_lock_free") {
        atomic<TestType> a;
        REQUIRE(a.is_lock_free());
        const volatile atomic<TestType> &cv = a;
        REQUIRE(cv.is_lock_free());
    }

    SECTION("store/load with all memory orders") {
        atomic<TestType> a{TestType(7)};
        for (const auto order : all_orders) {
            a.store(TestType(10), order);
            REQUIRE(a.load(order) == TestType(10));
            a.store(TestType(3), order);
            REQUIRE(a.load(order) == TestType(3));
        }
    }

    SECTION("default store/load") {
        atomic<TestType> a{TestType(1)};
        a.store(TestType(9));
        REQUIRE(a.load() == TestType(9));
    }

    SECTION("operator= and conversion") {
        atomic<TestType> a;
        a = TestType(42);
        REQUIRE(a.load() == TestType(42));
        TestType v = a;
        REQUIRE(v == TestType(42));
        const atomic<TestType> &ca = a;
        TestType cv = ca;
        REQUIRE(cv == TestType(42));
    }

    SECTION("exchange with all memory orders") {
        atomic<TestType> a{TestType(1)};
        for (const auto order : all_orders) {
            REQUIRE(a.exchange(TestType(2), order) == TestType(1));
            REQUIRE(a.load() == TestType(2));
            REQUIRE(a.exchange(TestType(1), order) == TestType(2));
            REQUIRE(a.load() == TestType(1));
        }
    }

    SECTION("default exchange") {
        atomic<TestType> a{TestType(1)};
        REQUIRE(a.exchange(TestType(5)) == TestType(1));
        REQUIRE(a.load() == TestType(5));
    }

    SECTION("compare_exchange_strong") {
        atomic<TestType> a{TestType(10)};
        TestType expected = TestType(10);
        for (const auto success : all_orders) {
            for (const auto failure : all_orders) {
                REQUIRE(a.compare_exchange_strong(expected, TestType(20), success, failure));
                REQUIRE(a.load() == TestType(20));
                expected = TestType(99);
                REQUIRE_FALSE(a.compare_exchange_strong(expected, TestType(30), success, failure));
                REQUIRE(expected == TestType(20)); // 失败时 expected 被更新为当前值
                REQUIRE(a.load() == TestType(20));
                REQUIRE(a.compare_exchange_strong(expected, TestType(10), success, failure));
                expected = TestType(10);
            }
        }
    }

    SECTION("compare_exchange_weak") {
        atomic<TestType> a{TestType(10)};
        TestType expected = TestType(10);
        bool ok = false;
        for (int i = 0; i < 100 && !ok; ++i) { // 弱 CAS 允许虚假失败，循环重试
            ok = a.compare_exchange_weak(expected, TestType(20));
        }
        REQUIRE(ok);
        REQUIRE(a.load() == TestType(20));
        expected = TestType(99);
        REQUIRE_FALSE(a.compare_exchange_weak(expected, TestType(30)));
        REQUIRE(expected == TestType(20));
    }

    SECTION("fetch_add/fetch_sub with all memory orders") {
        atomic<TestType> a{TestType(5)};
        for (const auto order : all_orders) {
            REQUIRE(a.fetch_add(TestType(1), order) == TestType(5));
            REQUIRE(a.load() == TestType(6));
            REQUIRE(a.fetch_sub(TestType(1), order) == TestType(6));
            REQUIRE(a.load() == TestType(5));
        }
        REQUIRE(a.fetch_add(TestType(2)) == TestType(5));
        REQUIRE(a.load() == TestType(7));
        REQUIRE(a.fetch_sub(TestType(3)) == TestType(7));
        REQUIRE(a.load() == TestType(4));
    }

    SECTION("fetch_and/fetch_or/fetch_xor with all memory orders") {
        atomic<TestType> a{TestType(0x05)};
        for (const auto order : all_orders) {
            REQUIRE(a.fetch_and(TestType(0x0F), order) == TestType(0x05));
            REQUIRE(a.load() == TestType(0x05));
            REQUIRE(a.fetch_or(TestType(0x10), order) == TestType(0x05));
            REQUIRE(a.load() == TestType(0x15));
            REQUIRE(a.fetch_xor(TestType(0x03), order) == TestType(0x15));
            REQUIRE(a.load() == TestType(0x16));
            REQUIRE(a.fetch_xor(TestType(0x03), order) == TestType(0x16));
            REQUIRE(a.load() == TestType(0x15));
            REQUIRE(a.fetch_and(TestType(0x0F), order) == TestType(0x15));
            REQUIRE(a.load() == TestType(0x05));
            REQUIRE(a.fetch_or(TestType(0x00), order) == TestType(0x05));
            REQUIRE(a.load() == TestType(0x05));
        }
    }

    SECTION("fetch_max/fetch_min with all memory orders") {
        atomic<TestType> a{TestType(22)};
        for (const auto order : all_orders) {
            a.store(TestType(22), memory_order::relaxed); // 每轮迭代重置初始值
            REQUIRE(a.fetch_max(TestType(7), order) == TestType(22)); // 22 > 7，不更新
            REQUIRE(a.load() == TestType(22));
            REQUIRE(a.fetch_min(TestType(100), order) == TestType(22)); // 22 < 100，不更新
            REQUIRE(a.load() == TestType(22));
            REQUIRE(a.fetch_max(TestType(50), order) == TestType(22));
            REQUIRE(a.load() == TestType(50));
            REQUIRE(a.fetch_min(TestType(30), order) == TestType(50));
            REQUIRE(a.load() == TestType(30));
            REQUIRE(a.fetch_max(TestType(30), order) == TestType(30));
            REQUIRE(a.load() == TestType(30));
            REQUIRE(a.fetch_min(TestType(30), order) == TestType(30));
            REQUIRE(a.load() == TestType(30));
        }
    }

    SECTION("increment/decrement operators") {
        atomic<TestType> a{TestType(5)};
        REQUIRE(a++ == TestType(5)); // 后置返回旧值
        REQUIRE(a.load() == TestType(6));
        REQUIRE(++a == TestType(7)); // 前置返回新值
        REQUIRE(a-- == TestType(7));
        REQUIRE(a.load() == TestType(6));
        REQUIRE(--a == TestType(5));
        REQUIRE(a.load() == TestType(5));
    }

    SECTION("compound assignment operators") {
        atomic<TestType> a{TestType(5)};
        REQUIRE((a += TestType(2)) == TestType(7));
        REQUIRE((a -= TestType(3)) == TestType(4));
        REQUIRE((a &= TestType(0xFF)) == TestType(4));
        REQUIRE((a |= TestType(0x08)) == TestType(12));
        REQUIRE((a ^= TestType(0x03)) == TestType(15));
        REQUIRE(a.load() == TestType(15));
    }

    SECTION("volatile qualified operations") {
        volatile atomic<TestType> a{TestType(5)};
        REQUIRE(a.load() == TestType(5));
        REQUIRE(a.load(memory_order::acquire) == TestType(5));
        a.store(TestType(6), memory_order::release);
        REQUIRE(a.load() == TestType(6));
        REQUIRE(a.exchange(TestType(7), memory_order::acq_rel) == TestType(6));
        REQUIRE(a.fetch_add(TestType(1), memory_order::relaxed) == TestType(7));
        REQUIRE(a.fetch_sub(TestType(1), memory_order::seq_cst) == TestType(8));
        REQUIRE(++a == TestType(8));
        REQUIRE(a++ == TestType(8));
        REQUIRE(a.load() == TestType(9));
        REQUIRE((a += TestType(1)) == TestType(10));
        TestType expected = TestType(10);
        REQUIRE(a.compare_exchange_strong(expected, TestType(11), memory_order::seq_cst, memory_order::relaxed));
        REQUIRE(a.load() == TestType(11));
        a = TestType(3);
        REQUIRE(a.load() == TestType(3));
        TestType cv = a;
        REQUIRE(cv == TestType(3));
    }
}

TEMPLATE_TEST_CASE("atomic floating point operations", "[atomic][floating]", float, double) {
    SECTION("type properties") {
        STATIC_REQUIRE(atomic<TestType>::is_always_lock_free);
        using value_type = typename atomic<TestType>::value_type;
        STATIC_REQUIRE(std::is_same_v<value_type, TestType>);
    }

    SECTION("store/load/exchange with all memory orders") {
        atomic<TestType> a{TestType(1.5)};
        for (const auto order : all_orders) {
            a.store(TestType(2.5), order);
            REQUIRE(a.load(order) == TestType(2.5));
            REQUIRE(a.exchange(TestType(1.5), order) == TestType(2.5));
            REQUIRE(a.load() == TestType(1.5));
        }
    }

    SECTION("fetch_add/fetch_sub with all memory orders") {
        atomic<TestType> a{TestType(1.5)};
        for (const auto order : all_orders) {
            REQUIRE(a.fetch_add(TestType(2.5), order) == TestType(1.5));
            REQUIRE(a.load() == TestType(4.0));
            REQUIRE(a.fetch_sub(TestType(1.0), order) == TestType(4.0));
            REQUIRE(a.load() == TestType(3.0));
            REQUIRE(a.fetch_sub(TestType(1.5), order) == TestType(3.0));
            REQUIRE(a.load() == TestType(1.5));
        }
    }

    SECTION("compare_exchange_strong") {
        atomic<TestType> a{TestType(1.5)};
        TestType expected = TestType(1.5);
        REQUIRE(a.compare_exchange_strong(expected, TestType(2.5)));
        REQUIRE(a.load() == TestType(2.5));
        expected = TestType(99.0);
        REQUIRE_FALSE(a.compare_exchange_strong(expected, TestType(3.5)));
        REQUIRE(expected == TestType(2.5));
    }

    SECTION("compound assignment and operators") {
        atomic<TestType> a{TestType(1.5)};
        REQUIRE((a += TestType(2.0)) == TestType(3.5));
        REQUIRE((a -= TestType(0.5)) == TestType(3.0));
        a = TestType(4.0);
        REQUIRE(a.load() == TestType(4.0));
        TestType v = a;
        REQUIRE(v == TestType(4.0));
    }

    SECTION("volatile qualified operations") {
        volatile atomic<TestType> a{TestType(1.5)};
        REQUIRE(a.load(memory_order::acquire) == TestType(1.5));
        a.store(TestType(2.5), memory_order::release);
        REQUIRE(a.fetch_add(TestType(0.5), memory_order::seq_cst) == TestType(2.5));
        REQUIRE(a.load() == TestType(3.0));
        REQUIRE((a -= TestType(0.5)) == TestType(2.5));
    }
}

TEST_CASE("atomic<bool>", "[atomic][bool]") {
    SECTION("basic store/load") {
        atomic<bool> a;
        REQUIRE_FALSE(a.load());
        a.store(true);
        REQUIRE(a.load());
        a.store(false, memory_order::release);
        REQUIRE_FALSE(a.load(memory_order::acquire));
        a = true;
        REQUIRE(a.load());
        bool v = a;
        REQUIRE(v);
    }

    SECTION("exchange") {
        atomic<bool> a{false};
        REQUIRE_FALSE(a.exchange(true));
        REQUIRE(a.load());
        REQUIRE(a.exchange(false, memory_order::acq_rel));
        REQUIRE_FALSE(a.load());
    }

    SECTION("compare_exchange") {
        atomic<bool> a{false};
        bool expected = false;
        REQUIRE(a.compare_exchange_strong(expected, true));
        REQUIRE(a.load());
        expected = false;
        REQUIRE_FALSE(a.compare_exchange_strong(expected, false));
        REQUIRE(expected); // 失败时 expected 被更新为 true
        expected = true;
        REQUIRE(a.compare_exchange_weak(expected, false));
        REQUIRE_FALSE(a.load());
    }

    SECTION("volatile qualified operations") {
        volatile atomic<bool> a{false};
        REQUIRE_FALSE(a.load());
        a.store(true, memory_order::release);
        REQUIRE(a.exchange(false) == true);
        REQUIRE_FALSE(a.load());
    }
}

TEST_CASE("atomic pointer types", "[atomic][pointer]") {
    int arr[4] = {1, 2, 3, 4};

    SECTION("store/load/exchange") {
        atomic<int *> p{arr};
        REQUIRE(p.load() == arr);
        p.store(arr + 1);
        REQUIRE(p.load() == arr + 1);
        REQUIRE(p.exchange(arr) == arr + 1);
        REQUIRE(p.load() == arr);
        p = arr + 3;
        REQUIRE(p.load() == arr + 3);
    }

    SECTION("fetch_add/fetch_sub with all memory orders") {
        atomic<int *> p{arr};
        for (const auto order : all_orders) {
            REQUIRE(p.fetch_add(1, order) == arr);
            REQUIRE(p.load() == arr + 1);
            REQUIRE(p.fetch_sub(1, order) == arr + 1);
            REQUIRE(p.load() == arr);
        }
    }

    SECTION("compare_exchange") {
        atomic<int *> p{arr};
        int *expected = arr;
        REQUIRE(p.compare_exchange_strong(expected, arr + 2));
        REQUIRE(p.load() == arr + 2);
        expected = arr;
        REQUIRE_FALSE(p.compare_exchange_strong(expected, arr + 3));
        REQUIRE(expected == arr + 2);
    }

    SECTION("increment/decrement and compound assignment") {
        atomic<int *> p{arr};
        REQUIRE(p++ == arr);
        REQUIRE(p.load() == arr + 1);
        REQUIRE(++p == arr + 2);
        REQUIRE(p-- == arr + 2);
        REQUIRE(p.load() == arr + 1);
        REQUIRE(--p == arr);
        REQUIRE((p += 2) == arr + 2);
        REQUIRE((p -= 1) == arr + 1);
        REQUIRE(p.load() == arr + 1);
    }

    SECTION("fetch_max/fetch_min") {
        atomic<int *> p{arr};
        REQUIRE(p.fetch_max(arr + 2) == arr); // arr < arr + 2，更新
        REQUIRE(p.load() == arr + 2);
        REQUIRE(p.fetch_min(arr + 1) == arr + 2); // arr + 2 > arr + 1，更新
        REQUIRE(p.load() == arr + 1);
        REQUIRE(p.fetch_max(arr + 3, memory_order::acq_rel) == arr + 1);
        REQUIRE(p.load() == arr + 3);
    }

    SECTION("volatile qualified operations") {
        volatile atomic<int *> p{arr};
        REQUIRE(p.load() == arr);
        p.store(arr + 1, memory_order::release);
        REQUIRE(p.fetch_add(1, memory_order::seq_cst) == arr + 1);
        REQUIRE(p.load() == arr + 2);
    }
}

TEST_CASE("atomic custom trivially copyable type", "[atomic][custom]") {
    struct two_int {
        std::int32_t a;
        std::int32_t b;
    };
    STATIC_REQUIRE(std::is_trivially_copyable_v<two_int>);

    atomic<two_int> v{two_int{1, 2}};
    REQUIRE(v.load().a == 1);
    REQUIRE(v.load().b == 2);

    v.store(two_int{3, 4});
    REQUIRE(v.load().a == 3);

    two_int old = v.exchange(two_int{5, 6});
    REQUIRE(old.a == 3);
    REQUIRE(v.load().a == 5);

    two_int expected = two_int{5, 6};
    REQUIRE(v.compare_exchange_strong(expected, two_int{7, 8}));
    REQUIRE(v.load().a == 7);
    expected = two_int{0, 0};
    REQUIRE_FALSE(v.compare_exchange_strong(expected, two_int{9, 10}));
    REQUIRE(expected.a == 7); // 失败时 expected 被更新

    v = two_int{11, 12};
    REQUIRE(v.load().b == 12);
}

TEST_CASE("atomic typedef aliases", "[atomic][alias]") {
    atomic_bool b{true};
    REQUIRE(b.load());

    atomic_char c{'x'};
    REQUIRE(c.load() == 'x');

    atomic_int i{1};
    REQUIRE(i.fetch_add(1) == 1);
    REQUIRE(i.load() == 2);

    atomic_uint ui{2u};
    REQUIRE(ui.load() == 2u);

    atomic_llong ll{3};
    REQUIRE(ll.fetch_add(2) == 3);
    REQUIRE(ll.load() == 5);

    atomic_ullong ull{4u};
    REQUIRE(ull.load() == 4u);

    atomic_size_t st{5};
    REQUIRE(st.load() == 5);

    atomic_int8_t i8{6};
    atomic_uint8_t u8{7u};
    atomic_int16_t i16{8};
    atomic_uint16_t u16{9u};
    atomic_int32_t i32{10};
    atomic_uint32_t u32{11u};
    atomic_int64_t i64{12};
    atomic_uint64_t u64{13u};
    REQUIRE(i8.load() == 6);
    REQUIRE(u8.load() == 7u);
    REQUIRE(i16.load() == 8);
    REQUIRE(u16.load() == 9u);
    REQUIRE(i32.load() == 10);
    REQUIRE(u32.load() == 11u);
    REQUIRE(i64.load() == 12);
    REQUIRE(u64.load() == 13u);

    atomic_int_least32_t il32{14};
    atomic_uint_least32_t ul32{15u};
    atomic_int_fast64_t if64{16};
    atomic_uint_fast64_t uf64{17u};
    atomic_intptr_t ip{18};
    atomic_uintptr_t up{19u};
    atomic_ptrdiff_t pd{20};
    atomic_intmax_t im{21};
    atomic_uintmax_t um{22u};
    REQUIRE(il32.load() == 14);
    REQUIRE(ul32.load() == 15u);
    REQUIRE(if64.load() == 16);
    REQUIRE(uf64.load() == 17u);
    REQUIRE(ip.load() == 18);
    REQUIRE(up.load() == 19u);
    REQUIRE(pd.load() == 20);
    REQUIRE(im.load() == 21);
    REQUIRE(um.load() == 22u);
#if RAINY_HAS_CXX20
    atomic_char8_t c8{23};
    REQUIRE(c8.load() == 23);
#endif
}

TEST_CASE("atomic free functions", "[atomic][free-function]") {
    SECTION("is_lock_free") {
        atomic<int> a;
        REQUIRE(atomic_is_lock_free(&a));
        volatile atomic<int> va;
        REQUIRE(atomic_is_lock_free(&va));
    }

    SECTION("store/load") {
        atomic<int> a{0};
        atomic_store(&a, 10);
        REQUIRE(atomic_load(&a) == 10);
        atomic_store_explicit(&a, 11, memory_order::release);
        REQUIRE(atomic_load_explicit(&a, memory_order::acquire) == 11);

        volatile atomic<int> va{0};
        atomic_store(&va, 12);
        REQUIRE(atomic_load(&va) == 12);
        atomic_store_explicit(&va, 13, memory_order::seq_cst);
        REQUIRE(atomic_load_explicit(&va, memory_order::seq_cst) == 13);
    }

    SECTION("exchange") {
        atomic<int> a{1};
        REQUIRE(atomic_exchange(&a, 2) == 1);
        REQUIRE(atomic_load(&a) == 2);
        REQUIRE(atomic_exchange_explicit(&a, 3, memory_order::acq_rel) == 2);

        volatile atomic<int> va{1};
        REQUIRE(atomic_exchange(&va, 4) == 1);
        REQUIRE(atomic_exchange_explicit(&va, 5, memory_order::relaxed) == 4);
    }

    SECTION("compare_exchange_weak/strong") {
        atomic<int> a{10};
        int expected = 10;
        REQUIRE(atomic_compare_exchange_strong(&a, &expected, 20));
        REQUIRE(atomic_load(&a) == 20);
        expected = 99;
        REQUIRE_FALSE(atomic_compare_exchange_strong(&a, &expected, 30));
        REQUIRE(expected == 20);

        expected = 20;
        REQUIRE(atomic_compare_exchange_strong_explicit(&a, &expected, 25, memory_order::acq_rel, memory_order::relaxed));
        REQUIRE(atomic_load(&a) == 25);
        expected = 25;
        REQUIRE(atomic_compare_exchange_weak(&a, &expected, 26));
        REQUIRE(atomic_load(&a) == 26);
        expected = 26;
        REQUIRE(atomic_compare_exchange_weak_explicit(&a, &expected, 27, memory_order::release, memory_order::consume));

        volatile atomic<int> va{10};
        expected = 10;
        REQUIRE(atomic_compare_exchange_strong(&va, &expected, 11));
        REQUIRE(atomic_load(&va) == 11);
    }

    SECTION("fetch_add/fetch_sub") {
        atomic<int> a{5};
        REQUIRE(atomic_fetch_add(&a, 1) == 5);
        REQUIRE(atomic_load(&a) == 6);
        REQUIRE(atomic_fetch_sub(&a, 4) == 6);
        REQUIRE(atomic_load(&a) == 2);
        REQUIRE(atomic_fetch_add_explicit(&a, 10, memory_order::relaxed) == 2);
        REQUIRE(atomic_fetch_sub_explicit(&a, 5, memory_order::seq_cst) == 12);

        volatile atomic<int> va{5};
        REQUIRE(atomic_fetch_add(&va, 2) == 5);
        REQUIRE(atomic_fetch_sub_explicit(&va, 1, memory_order::acq_rel) == 7);
    }

    SECTION("fetch_and/fetch_or/fetch_xor") {
        atomic<int> a{0x05};
        REQUIRE(atomic_fetch_and(&a, 0x0F) == 0x05);
        REQUIRE(atomic_fetch_or(&a, 0x10) == 0x05);
        REQUIRE(atomic_load(&a) == 0x15);
        REQUIRE(atomic_fetch_xor(&a, 0x03) == 0x15);
        REQUIRE(atomic_load(&a) == 0x16);
        REQUIRE(atomic_fetch_and_explicit(&a, 0x0F, memory_order::relaxed) == 0x16);
        REQUIRE(atomic_fetch_or_explicit(&a, 0x00, memory_order::seq_cst) == 0x06);
        REQUIRE(atomic_fetch_xor_explicit(&a, 0x06, memory_order::release) == 0x06);
        REQUIRE(atomic_load(&a) == 0x00);
    }

    SECTION("fetch_max/fetch_min") {
        atomic<int> a{22};
        REQUIRE(atomic_fetch_max(&a, 50) == 22);
        REQUIRE(atomic_load(&a) == 50);
        REQUIRE(atomic_fetch_min(&a, 30) == 50);
        REQUIRE(atomic_load(&a) == 30);
        REQUIRE(atomic_fetch_max_explicit(&a, 30, memory_order::acq_rel) == 30);
        REQUIRE(atomic_fetch_min_explicit(&a, 40, memory_order::relaxed) == 30);
        REQUIRE(atomic_load(&a) == 30);
    }
}

TEST_CASE("atomic_flag basic operations", "[atomic][atomic-flag]") {
    SECTION("default construction is clear") {
        atomic_flag flag;
        REQUIRE_FALSE(flag.test());
    }

    SECTION("test_and_set/clear cycle") {
        atomic_flag flag;
        REQUIRE_FALSE(flag.test_and_set()); // 之前是清除状态
        REQUIRE(flag.test());
        REQUIRE(flag.test_and_set()); // 之前是设置状态
        flag.clear();
        REQUIRE_FALSE(flag.test());
        REQUIRE_FALSE(flag.test_and_set());
        REQUIRE(flag.test());
    }

    SECTION("all memory orders") {
        atomic_flag flag;
        for (const auto order : all_orders) {
            REQUIRE_FALSE(flag.test(order));
            REQUIRE_FALSE(flag.test_and_set(order));
            REQUIRE(flag.test(order));
            flag.clear(order);
            REQUIRE_FALSE(flag.test(order));
        }
    }

    SECTION("default memory orders") {
        atomic_flag flag;
        REQUIRE_FALSE(flag.test_and_set());
        REQUIRE(flag.test());
        flag.clear();
        REQUIRE_FALSE(flag.test());
    }

    SECTION("volatile qualified operations") {
        volatile atomic_flag flag;
        REQUIRE_FALSE(flag.test());
        REQUIRE_FALSE(flag.test_and_set(memory_order::acquire));
        REQUIRE(flag.test(memory_order::seq_cst));
        flag.clear(memory_order::release);
        REQUIRE_FALSE(flag.test());
    }
}

TEST_CASE("atomic_flag free functions", "[atomic][atomic-flag][free-function]") {
    SECTION("non-volatile overloads") {
        atomic_flag flag;
        REQUIRE_FALSE(atomic_flag_test(&flag));
        REQUIRE_FALSE(atomic_flag_test_and_set(&flag, memory_order::acquire));
        REQUIRE(atomic_flag_test(&flag, memory_order::seq_cst));
        atomic_flag_clear(&flag, memory_order::release);
        REQUIRE_FALSE(atomic_flag_test(&flag));
        atomic_flag_test_and_set(&flag);
        atomic_flag_clear(&flag);
        REQUIRE_FALSE(atomic_flag_test(&flag));
    }

    SECTION("volatile overloads") {
        volatile atomic_flag flag;
        REQUIRE_FALSE(atomic_flag_test(&flag));
        REQUIRE_FALSE(atomic_flag_test_and_set(&flag, memory_order::acquire));
        REQUIRE(atomic_flag_test(&flag, memory_order::seq_cst));
        atomic_flag_clear(&flag, memory_order::release);
        REQUIRE_FALSE(atomic_flag_test(&flag));
    }
}

TEST_CASE("atomic wait/notify_one", "[atomic][wait][multithread]") {
    atomic<int> value{0};
    bool woke_up = false;
    std::thread waiter([&] {
        value.wait(0); // 值仍为 0 时阻塞
        woke_up = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    value.store(1, memory_order::release);
    value.notify_one();
    waiter.join();
    REQUIRE(woke_up);
}

TEST_CASE("atomic wait/notify_all", "[atomic][wait][multithread]") {
    atomic<int> gate{0};
    atomic<int> woken{0};
    std::thread waiter1([&] {
        gate.wait(0);
        woken.fetch_add(1, memory_order::relaxed);
    });
    std::thread waiter2([&] {
        gate.wait(0);
        woken.fetch_add(1, memory_order::relaxed);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    gate.store(1, memory_order::release);
    gate.notify_all();
    waiter1.join();
    waiter2.join();
    REQUIRE(woken.load() == 2);
}

TEST_CASE("atomic wait/notify free functions", "[atomic][wait][free-function][multithread]") {
    atomic<int> value{0};
    bool woke_up = false;
    std::thread waiter([&] {
        atomic_wait(&value, 0);
        woke_up = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    atomic_store_explicit(&value, 1, memory_order::release);
    atomic_notify_one(&value);
    waiter.join();
    REQUIRE(woke_up);

    atomic<int> gate{0};
    atomic<int> woken{0};
    std::thread w1([&] {
        atomic_wait_explicit(&gate, 0, memory_order::acquire);
        woken.fetch_add(1);
    });
    std::thread w2([&] {
        atomic_wait_explicit(&gate, 0, memory_order::acquire);
        woken.fetch_add(1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    atomic_store_explicit(&gate, 1, memory_order::release);
    atomic_notify_all(&gate);
    w1.join();
    w2.join();
    REQUIRE(woken.load() == 2);
}

TEST_CASE("atomic_flag wait/notify", "[atomic][atomic-flag][wait][multithread]") {
    atomic_flag flag;
    bool woke_up = false;
    std::thread waiter([&] {
        flag.wait(false); // 标志为 false 时阻塞
        woke_up = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    flag.test_and_set(memory_order::acquire);
    flag.notify_one();
    waiter.join();
    REQUIRE(woke_up);

    atomic_flag gate;
    atomic<int> woken{0};
    std::thread w1([&] {
        gate.wait(false);
        woken.fetch_add(1);
    });
    std::thread w2([&] {
        gate.wait(false);
        woken.fetch_add(1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    gate.test_and_set();
    gate.notify_all();
    w1.join();
    w2.join();
    REQUIRE(woken.load() == 2);
}

TEST_CASE("multithreaded fetch_add stress", "[atomic][multithread]") {
    atomic<long long> counter{0};
    constexpr int thread_count = 8;
    constexpr int iterations = 10000;
    std::vector<std::thread> pool;
    pool.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        pool.emplace_back([&counter] {
            for (int j = 0; j < iterations; ++j) {
                counter.fetch_add(1, memory_order::relaxed);
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    REQUIRE(counter.load() == static_cast<long long>(thread_count) * iterations);
}

TEST_CASE("multithreaded fetch_add returns distinct old values", "[atomic][multithread]") {
    atomic<int> counter{0};
    constexpr int thread_count = 8;
    constexpr int iterations = 5000;
    constexpr int total = thread_count * iterations;
    std::vector<std::vector<int>> seen(thread_count);
    std::vector<std::thread> pool;
    pool.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        pool.emplace_back([&counter, &seen, i] {
            seen[i].reserve(iterations);
            for (int j = 0; j < iterations; ++j) {
                seen[i].push_back(counter.fetch_add(1, memory_order::relaxed));
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    std::vector<char> seen_set(static_cast<std::size_t>(total), 0);
    for (const auto &list : seen) {
        for (const int old : list) {
            REQUIRE(old >= 0);
            REQUIRE(old < total);
            REQUIRE_FALSE(seen_set[static_cast<std::size_t>(old)]);
            seen_set[static_cast<std::size_t>(old)] = 1;
        }
    }
}

TEST_CASE("multithreaded atomic_flag spinlock", "[atomic][atomic-flag][multithread]") {
    atomic_flag lock;
    int counter = 0;
    constexpr int thread_count = 8;
    constexpr int iterations = 1000;
    std::vector<std::thread> pool;
    pool.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        pool.emplace_back([&lock, &counter] {
            for (int j = 0; j < iterations; ++j) {
                while (lock.test_and_set(memory_order::acquire)) {
                }
                ++counter; // 临界区
                lock.clear(memory_order::release);
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    REQUIRE(counter == thread_count * iterations);
}

TEST_CASE("multithreaded cas counter", "[atomic][multithread]") {
    atomic<int> counter{0};
    constexpr int thread_count = 4;
    constexpr int iterations = 5000;
    std::vector<std::thread> pool;
    pool.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        pool.emplace_back([&counter] {
            for (int j = 0; j < iterations; ++j) {
                int expected = counter.load(memory_order::relaxed);
                while (!counter.compare_exchange_weak(expected, expected + 1, memory_order::acq_rel, memory_order::relaxed)) {
                }
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    REQUIRE(counter.load() == thread_count * iterations);
}

TEST_CASE("multithreaded exchange atomicity (lock-free stack)", "[atomic][multithread]") {
    struct node {
        int value;
        node *next;
    };
    constexpr int thread_count = 8;
    constexpr int iterations = 1000;
    constexpr int total = thread_count * iterations;
    std::vector<node> nodes(static_cast<std::size_t>(total));
    atomic<node *> head{nullptr};
    std::vector<std::thread> pool;
    pool.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        pool.emplace_back([&nodes, &head, i] {
            for (int j = 0; j < iterations; ++j) {
                node *n = &nodes[static_cast<std::size_t>(i * iterations + j)];
                n->value = i * iterations + j;
                n->next = head.exchange(n, memory_order::relaxed); // 原子头插
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    std::vector<char> seen(static_cast<std::size_t>(total), 0);
    int count = 0;
    for (node *n = head.load(); n != nullptr; n = n->next) {
        ++count;
        REQUIRE(n->value >= 0);
        REQUIRE(n->value < total);
        REQUIRE_FALSE(seen[static_cast<std::size_t>(n->value)]);
        seen[static_cast<std::size_t>(n->value)] = 1;
    }
    REQUIRE(count == total);
}

TEST_CASE("multithreaded release/acquire handshake", "[atomic][multithread]") {
    atomic<bool> ready{false};
    int payload = 0;
    std::thread producer([&] {
        payload = 42; // 普通写，release store 之后对消费者可见
        ready.store(true, memory_order::release);
    });
    while (!ready.load(memory_order::acquire)) {
    }
    producer.join();
    REQUIRE(payload == 42);
}

TEST_CASE("fenced_block", "[atomic][fenced-block]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<fenced_block>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<fenced_block>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<fenced_block>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<fenced_block>);

    SECTION("half does not issue any fence") {
        fenced_block fb(fenced_block::half);
        (void)fb;
    }

    SECTION("full issues acquire fence on construction and release fence on destruction") {
        fenced_block fb(fenced_block::full);
        (void)fb;
    }

    SECTION("full fence protects data passing") {
        atomic<int> ready{0};
        int data = 0;
        std::thread writer([&] {
            data = 42;
            {
                fenced_block fb(fenced_block::full); // release fence on destruction
                (void)fb;
            }
            ready.store(1, memory_order::release);
        });
        while (ready.load(memory_order::acquire) == 0) {
        }
        writer.join();
        REQUIRE(data == 42);
    }
}

TEST_CASE("atomic_thread_fence / atomic_signal_fence", "[atomic][fence]") {
    concurrency::atomic_thread_fence(memory_order::relaxed);
    concurrency::atomic_thread_fence(memory_order::consume);
    concurrency::atomic_thread_fence(memory_order::acquire);
    concurrency::atomic_thread_fence(memory_order::release);
    concurrency::atomic_thread_fence(memory_order::acq_rel);
    concurrency::atomic_thread_fence(memory_order::seq_cst);

    concurrency::atomic_signal_fence(memory_order::relaxed);
    concurrency::atomic_signal_fence(memory_order::acquire);
    concurrency::atomic_signal_fence(memory_order::seq_cst);
}
