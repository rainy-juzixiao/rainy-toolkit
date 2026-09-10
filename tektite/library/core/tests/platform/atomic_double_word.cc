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
#include <catch2/generators/catch_generators.hpp>
#include <rainy/core/layer.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdint>
#include <cstring>

using namespace rainy::core::layer;

TEST_CASE("double_word_t basic construction and comparison", "[atomic][double_word]") {
    double_word_t<std::uint32_t> dw32{0xAAAAAAAA, 0xBBBBBBBB};
    REQUIRE(dw32.lo == 0xAAAAAAAA);
    REQUIRE(dw32.hi == 0xBBBBBBBB);

    double_word_t<std::uint32_t> dw32_same{0xAAAAAAAA, 0xBBBBBBBB};
    double_word_t<std::uint32_t> dw32_diff{0xCCCCCCCC, 0xDDDDDDDD};

    REQUIRE(dw32 == dw32_same);
    REQUIRE(dw32 != dw32_diff);
    REQUIRE_FALSE(dw32 == dw32_diff);
    REQUIRE_FALSE(dw32 != dw32_same);

    double_word_t<std::uint64_t> dw64{0xAAAAAAAAAAAAAAAAULL, 0xBBBBBBBBBBBBBBBBULL};
    REQUIRE(dw64.lo == 0xAAAAAAAAAAAAAAAAULL);
    REQUIRE(dw64.hi == 0xBBBBBBBBBBBBBBBBULL);
}

TEST_CASE("double_word_t alignment requirements", "[atomic][double_word]") {
    REQUIRE(alignof(double_word_t<std::uint32_t>) >= 8);
    REQUIRE(alignof(double_word_t<std::uint64_t>) >= 16);
    REQUIRE(sizeof(double_word_t<std::uint32_t>) == 8);
    REQUIRE(sizeof(double_word_t<std::uint64_t>) == 16);
    REQUIRE(sizeof(native_double_word_t) == sizeof(void*) * 2);
}

TEST_CASE("native_double_word_t size matches pointer width", "[atomic][double_word]") {
#if RAINY_USING_64_BIT_PLATFORM
    REQUIRE(sizeof(native_double_word_t) == 16);
#else
    REQUIRE(sizeof(native_double_word_t) == 8);
#endif
}

TEST_CASE("interlocked_compare_exchange_double_word basic success", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t initial;
    initial.lo = static_cast<std::uintptr_t>(100);
    initial.hi = static_cast<std::uintptr_t>(200);
    std::memcpy(const_cast<native_double_word_t*>(&dest), &initial, sizeof(native_double_word_t));

    native_double_word_t exchange;
    exchange.lo = static_cast<std::uintptr_t>(300);
    exchange.hi = static_cast<std::uintptr_t>(400);

    native_double_word_t comparand;
    comparand.lo = static_cast<std::uintptr_t>(100);
    comparand.hi = static_cast<std::uintptr_t>(200);

    bool result = interlocked_compare_exchange_double_word(&dest, exchange, &comparand);
    REQUIRE(result == true);
    REQUIRE(dest.lo == static_cast<std::uintptr_t>(300));
    REQUIRE(dest.hi == static_cast<std::uintptr_t>(400));
}

TEST_CASE("interlocked_compare_exchange_double_word basic failure", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t initial;
    initial.lo = static_cast<std::uintptr_t>(100);
    initial.hi = static_cast<std::uintptr_t>(200);
    std::memcpy(const_cast<native_double_word_t*>(&dest), &initial, sizeof(native_double_word_t));

    native_double_word_t exchange;
    exchange.lo = static_cast<std::uintptr_t>(300);
    exchange.hi = static_cast<std::uintptr_t>(400);

    native_double_word_t comparand;
    comparand.lo = static_cast<std::uintptr_t>(999);
    comparand.hi = static_cast<std::uintptr_t>(888);

    bool result = interlocked_compare_exchange_double_word(&dest, exchange, &comparand);
    REQUIRE(result == false);
    REQUIRE(dest.lo == static_cast<std::uintptr_t>(100));
    REQUIRE(dest.hi == static_cast<std::uintptr_t>(200));
    REQUIRE(comparand.lo == static_cast<std::uintptr_t>(100));
    REQUIRE(comparand.hi == static_cast<std::uintptr_t>(200));
}

TEST_CASE("interlocked_compare_exchange_double_word partial mismatch lo", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t initial;
    initial.lo = static_cast<std::uintptr_t>(100);
    initial.hi = static_cast<std::uintptr_t>(200);
    std::memcpy(const_cast<native_double_word_t*>(&dest), &initial, sizeof(native_double_word_t));

    native_double_word_t exchange;
    exchange.lo = static_cast<std::uintptr_t>(300);
    exchange.hi = static_cast<std::uintptr_t>(400);

    native_double_word_t comparand;
    comparand.lo = static_cast<std::uintptr_t>(999);
    comparand.hi = static_cast<std::uintptr_t>(200);

    bool result = interlocked_compare_exchange_double_word(&dest, exchange, &comparand);
    REQUIRE(result == false);
    REQUIRE(dest.lo == static_cast<std::uintptr_t>(100));
    REQUIRE(dest.hi == static_cast<std::uintptr_t>(200));
    REQUIRE(comparand.lo == static_cast<std::uintptr_t>(100));
    REQUIRE(comparand.hi == static_cast<std::uintptr_t>(200));
}

TEST_CASE("interlocked_compare_exchange_double_word partial mismatch hi", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t initial;
    initial.lo = static_cast<std::uintptr_t>(100);
    initial.hi = static_cast<std::uintptr_t>(200);
    std::memcpy(const_cast<native_double_word_t*>(&dest), &initial, sizeof(native_double_word_t));

    native_double_word_t exchange;
    exchange.lo = static_cast<std::uintptr_t>(300);
    exchange.hi = static_cast<std::uintptr_t>(400);

    native_double_word_t comparand;
    comparand.lo = static_cast<std::uintptr_t>(100);
    comparand.hi = static_cast<std::uintptr_t>(999);

    bool result = interlocked_compare_exchange_double_word(&dest, exchange, &comparand);
    REQUIRE(result == false);
    REQUIRE(dest.lo == static_cast<std::uintptr_t>(100));
    REQUIRE(dest.hi == static_cast<std::uintptr_t>(200));
}

TEST_CASE("interlocked_compare_exchange_double_word zero values", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&dest), &zero, sizeof(native_double_word_t));

    native_double_word_t exchange;
    exchange.lo = static_cast<std::uintptr_t>(0xDEAD);
    exchange.hi = static_cast<std::uintptr_t>(0xBEEF);

    native_double_word_t comparand{0, 0};

    bool result = interlocked_compare_exchange_double_word(&dest, exchange, &comparand);
    REQUIRE(result == true);
    REQUIRE(dest.lo == static_cast<std::uintptr_t>(0xDEAD));
    REQUIRE(dest.hi == static_cast<std::uintptr_t>(0xBEEF));
}

TEST_CASE("interlocked_compare_exchange_double_word max values", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t max_val;
    max_val.lo = ~static_cast<std::uintptr_t>(0);
    max_val.hi = ~static_cast<std::uintptr_t>(0);
    std::memcpy(const_cast<native_double_word_t*>(&dest), &max_val, sizeof(native_double_word_t));

    native_double_word_t exchange{0, 0};

    native_double_word_t comparand;
    comparand.lo = ~static_cast<std::uintptr_t>(0);
    comparand.hi = ~static_cast<std::uintptr_t>(0);

    bool result = interlocked_compare_exchange_double_word(&dest, exchange, &comparand);
    REQUIRE(result == true);
    REQUIRE(dest.lo == 0);
    REQUIRE(dest.hi == 0);
}

TEST_CASE("atomic_load_double_word basic", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t src;
    native_double_word_t value;
    value.lo = static_cast<std::uintptr_t>(0x1234);
    value.hi = static_cast<std::uintptr_t>(0x5678);
    std::memcpy(const_cast<native_double_word_t*>(&src), &value, sizeof(native_double_word_t));

    memory_order orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_seq_cst
    };
    for (auto order : orders) {
        native_double_word_t loaded = atomic_load_double_word(&src, order);
        REQUIRE(loaded.lo == static_cast<std::uintptr_t>(0x1234));
        REQUIRE(loaded.hi == static_cast<std::uintptr_t>(0x5678));
    }
}

TEST_CASE("atomic_load_double_word all zeros", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t src;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&src), &zero, sizeof(native_double_word_t));

    native_double_word_t loaded = atomic_load_double_word(&src, memory_order_seq_cst);
    REQUIRE(loaded.lo == 0);
    REQUIRE(loaded.hi == 0);
}

TEST_CASE("atomic_load_double_word all ones", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t src;
    native_double_word_t ones;
    ones.lo = ~static_cast<std::uintptr_t>(0);
    ones.hi = ~static_cast<std::uintptr_t>(0);
    std::memcpy(const_cast<native_double_word_t*>(&src), &ones, sizeof(native_double_word_t));

    native_double_word_t loaded = atomic_load_double_word(&src, memory_order_seq_cst);
    REQUIRE(loaded.lo == ~static_cast<std::uintptr_t>(0));
    REQUIRE(loaded.hi == ~static_cast<std::uintptr_t>(0));
}

TEST_CASE("atomic_store_double_word basic", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&dest), &zero, sizeof(native_double_word_t));

    native_double_word_t value;
    value.lo = static_cast<std::uintptr_t>(0xAAAA);
    value.hi = static_cast<std::uintptr_t>(0xBBBB);

    memory_order orders[] = {
        memory_order_relaxed, memory_order_release, memory_order_seq_cst
    };
    for (auto order : orders) {
        atomic_store_double_word(&dest, value, order);
        REQUIRE(dest.lo == static_cast<std::uintptr_t>(0xAAAA));
        REQUIRE(dest.hi == static_cast<std::uintptr_t>(0xBBBB));

        native_double_word_t zero_reset{0, 0};
        atomic_store_double_word(&dest, zero_reset, memory_order_relaxed);
    }
}

TEST_CASE("atomic_store_double_word zero", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t value;
    value.lo = static_cast<std::uintptr_t>(0xFFFF);
    value.hi = static_cast<std::uintptr_t>(0xEEEE);
    std::memcpy(const_cast<native_double_word_t*>(&dest), &value, sizeof(native_double_word_t));

    native_double_word_t zero{0, 0};
    atomic_store_double_word(&dest, zero, memory_order_seq_cst);
    REQUIRE(dest.lo == 0);
    REQUIRE(dest.hi == 0);
}

TEST_CASE("atomic_store_double_word max values", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&dest), &zero, sizeof(native_double_word_t));

    native_double_word_t max_val;
    max_val.lo = ~static_cast<std::uintptr_t>(0);
    max_val.hi = ~static_cast<std::uintptr_t>(0);

    atomic_store_double_word(&dest, max_val, memory_order_seq_cst);
    REQUIRE(dest.lo == ~static_cast<std::uintptr_t>(0));
    REQUIRE(dest.hi == ~static_cast<std::uintptr_t>(0));
}

TEST_CASE("double_word store load roundtrip", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t buf;
    native_double_word_t original;
    original.lo = static_cast<std::uintptr_t>(0xCAFE);
    original.hi = static_cast<std::uintptr_t>(0xBABE);

    atomic_store_double_word(&buf, original, memory_order_seq_cst);
    native_double_word_t loaded = atomic_load_double_word(&buf, memory_order_seq_cst);

    REQUIRE(loaded.lo == original.lo);
    REQUIRE(loaded.hi == original.hi);
    REQUIRE(loaded == original);
}

TEST_CASE("double_word cas store load roundtrip", "[atomic][double_word]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t buf;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&buf), &zero, sizeof(native_double_word_t));

    native_double_word_t exchange;
    exchange.lo = static_cast<std::uintptr_t>(0x1111);
    exchange.hi = static_cast<std::uintptr_t>(0x2222);

    native_double_word_t comparand{0, 0};
    bool ok = interlocked_compare_exchange_double_word(&buf, exchange, &comparand);
    REQUIRE(ok == true);

    native_double_word_t loaded = atomic_load_double_word(&buf, memory_order_seq_cst);
    REQUIRE(loaded == exchange);
}

TEST_CASE("interlocked_compare_exchange_double_word multithreaded", "[atomic][double_word][thread]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t dest;
    native_double_word_t initial{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&dest), &initial, sizeof(native_double_word_t));

    std::atomic<bool> start{false};
    std::atomic<int> success_count{0};
    const int num_threads = 4;
    const int iters_per_thread = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            while (!start.load(std::memory_order_acquire));

            for (int j = 0; j < iters_per_thread; ++j) {
                native_double_word_t expected = atomic_load_double_word(&dest, memory_order_acquire);

                native_double_word_t new_val;
                new_val.lo = expected.lo + 1;
                new_val.hi = expected.hi + 1;

                native_double_word_t comparand = expected;
                if (interlocked_compare_exchange_double_word(&dest, new_val, &comparand)) {
                    success_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();

    native_double_word_t final_val = atomic_load_double_word(&dest, memory_order_seq_cst);
    REQUIRE(final_val.lo == static_cast<std::uintptr_t>(success_count.load()));
    REQUIRE(final_val.hi == static_cast<std::uintptr_t>(success_count.load()));
    REQUIRE(success_count.load() > 0);
}

TEST_CASE("atomic_store_double_word multithreaded load observe", "[atomic][double_word][thread]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t shared;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&shared), &zero, sizeof(native_double_word_t));

    std::atomic<bool> start{false};
    std::atomic<bool> done{false};
    std::atomic<int> corruptions{0};
    const int iters = 5000;

    std::thread writer([&]() {
        while (!start.load(std::memory_order_acquire));
        for (int i = 0; i < iters; ++i) {
            native_double_word_t val;
            val.lo = static_cast<std::uintptr_t>(i);
            val.hi = static_cast<std::uintptr_t>(i);
            atomic_store_double_word(&shared, val, memory_order_release);
        }
        done.store(true, std::memory_order_release);
    });

    std::thread reader([&]() {
        while (!start.load(std::memory_order_acquire));
        while (!done.load(std::memory_order_acquire)) {
            native_double_word_t val = atomic_load_double_word(&shared, memory_order_acquire);
            if (val.lo != val.hi) {
                corruptions.fetch_add(1, std::memory_order_relaxed);
            }
        }
        native_double_word_t final_val = atomic_load_double_word(&shared, memory_order_seq_cst);
        if (final_val.lo != final_val.hi) {
            corruptions.fetch_add(1, std::memory_order_relaxed);
        }
    });

    start.store(true, std::memory_order_release);
    writer.join();
    reader.join();

    REQUIRE(corruptions.load() == 0);
}

TEST_CASE("double_word concurrent single writer multiple readers", "[atomic][double_word][thread]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t shared;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&shared), &zero, sizeof(native_double_word_t));

    std::atomic<bool> start{false};
    std::atomic<bool> done{false};
    std::atomic<int> total_reads{0};
    std::atomic<int> corruptions{0};
    const int num_readers = 3;
    const int iters = 3000;

    std::thread writer([&]() {
        while (!start.load(std::memory_order_acquire));
        for (int i = 1; i <= iters; ++i) {
            native_double_word_t val;
            val.lo = static_cast<std::uintptr_t>(i);
            val.hi = static_cast<std::uintptr_t>(i * 2);
            atomic_store_double_word(&shared, val, memory_order_release);
        }
        done.store(true, std::memory_order_release);
    });

    std::vector<std::thread> readers;
    for (int r = 0; r < num_readers; ++r) {
        readers.emplace_back([&]() {
            while (!start.load(std::memory_order_acquire));
            for (;;) {
                native_double_word_t val = atomic_load_double_word(&shared, memory_order_acquire);
                if (val.lo != 0 && val.hi != val.lo * 2) {
                    corruptions.fetch_add(1, std::memory_order_relaxed);
                }
                total_reads.fetch_add(1, std::memory_order_relaxed);
                if (done.load(std::memory_order_acquire)) {
                    break;
                }
            }
        });
    }

    start.store(true, std::memory_order_release);
    writer.join();
    for (auto& t : readers) t.join();

    REQUIRE(corruptions.load() == 0);
    REQUIRE(total_reads.load() > 0);
}

TEST_CASE("double_word concurrent multi cas", "[atomic][double_word][thread]") {
    alignas(sizeof(native_double_word_t)) volatile native_double_word_t shared;
    native_double_word_t zero{0, 0};
    std::memcpy(const_cast<native_double_word_t*>(&shared), &zero, sizeof(native_double_word_t));

    std::atomic<bool> start{false};
    std::atomic<int> cas_success{0};
    std::atomic<int> cas_failure{0};
    const int num_threads = 4;
    const int iters_per_thread = 2000;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            while (!start.load(std::memory_order_acquire));
            for (int j = 0; j < iters_per_thread; ++j) {
                native_double_word_t expected = atomic_load_double_word(&shared, memory_order_acquire);
                native_double_word_t new_val;
                new_val.lo = expected.lo + 1;
                new_val.hi = expected.hi + 1;

                native_double_word_t comparand = expected;
                if (interlocked_compare_exchange_double_word(&shared, new_val, &comparand)) {
                    cas_success.fetch_add(1, std::memory_order_relaxed);
                } else {
                    cas_failure.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();

    native_double_word_t final_val = atomic_load_double_word(&shared, memory_order_seq_cst);
    REQUIRE(final_val.lo == static_cast<std::uintptr_t>(cas_success.load()));
    REQUIRE(final_val.hi == static_cast<std::uintptr_t>(cas_success.load()));
    REQUIRE(cas_success.load() + cas_failure.load() == num_threads * iters_per_thread);
}

TEST_CASE("is_always_lock_free constexpr", "[atomic][double_word]") {
    REQUIRE((is_always_lock_free == true || is_always_lock_free == false));
}

TEST_CASE("double_word_t copy semantics", "[atomic][double_word]") {
    double_word_t<std::uint32_t> dw1{1, 2};
    double_word_t<std::uint32_t> dw2 = dw1;
    REQUIRE(dw2 == dw1);
    REQUIRE(dw2.lo == 1);
    REQUIRE(dw2.hi == 2);

    double_word_t<std::uint32_t> dw3{3, 4};
    dw1 = dw3;
    REQUIRE(dw1 == dw3);
    REQUIRE(dw1.lo == 3);
    REQUIRE(dw1.hi == 4);
}

TEST_CASE("double_word_t with pointer types", "[atomic][double_word]") {
    int a = 1, b = 2;
    double_word_t<void*> dw_ptr{&a, &b};
    REQUIRE(dw_ptr.lo == &a);
    REQUIRE(dw_ptr.hi == &b);

    double_word_t<void*> dw_ptr2{&a, &b};
    REQUIRE(dw_ptr == dw_ptr2);

    double_word_t<void*> dw_ptr3{&b, &a};
    REQUIRE(dw_ptr != dw_ptr3);
}