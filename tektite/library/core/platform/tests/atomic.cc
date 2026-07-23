#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <rainy/core/layer.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdint>

using namespace rainy::core::layer;

TEST_CASE("interlocked_increment_explicit basic", "[atomic]") {
    volatile long val = 0;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire,
        memory_order_release, memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        val = 0;
        long ret = interlocked_increment_explicit(&val, order);
        REQUIRE(ret == 1);
        REQUIRE(val == 1);
    }
}

TEST_CASE("interlocked_decrement_explicit basic", "[atomic]") {
    volatile long val = 10;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire,
        memory_order_release, memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        val = 10;
        long ret = interlocked_decrement_explicit(&val, order);
        REQUIRE(ret == 9);
        REQUIRE(val == 9);
    }
}

TEST_CASE("interlocked_increment8_explicit multithreaded", "[atomic][thread]") {
    volatile std::int8_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 100;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment8_explicit(&val, memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == static_cast<std::int8_t>(num_threads * iters_per_thread));
}

TEST_CASE("interlocked_increment16_explicit multithreaded", "[atomic][thread]") {
    volatile std::int16_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment16_explicit(&val, memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == static_cast<std::int16_t>(num_threads * iters_per_thread));
}

TEST_CASE("interlocked_increment32_explicit multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment32_explicit(&val, memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == num_threads * iters_per_thread);
}

TEST_CASE("interlocked_increment64_explicit multithreaded", "[atomic][thread]") {
    volatile std::int64_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment64_explicit(&val, memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == static_cast<std::int64_t>(num_threads) * iters_per_thread);
}

TEST_CASE("interlocked_exchange_add_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 0;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_acquire, memory_order_release,
        memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        val = 10;
        std::intptr_t ret = interlocked_exchange_add_explicit(&val, 5, order);
        REQUIRE(ret == 10);
        REQUIRE(val == 15);
    }
}

TEST_CASE("interlocked_exchange_add_explicit multithreaded", "[atomic][thread]") {
    volatile std::intptr_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_exchange_add_explicit(&val, 1, memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == num_threads * iters_per_thread);
}

TEST_CASE("interlocked_exchange_subtract_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 20;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_acquire, memory_order_release,
        memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        val = 20;
        std::intptr_t ret = interlocked_exchange_subtract_explicit(&val, 5, order);
        REQUIRE(ret == 20);
        REQUIRE(val == 15);
    }
}

TEST_CASE("interlocked_exchange_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 0;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_acquire, memory_order_release,
        memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        val = 42;
        std::intptr_t ret = interlocked_exchange_explicit(&val, 99, order);
        REQUIRE(ret == 42);
        REQUIRE(val == 99);
    }
}

TEST_CASE("interlocked_compare_exchange_explicit basic", "[atomic]") {
    volatile long val = 10;
    memory_order success_orders[] = {
        memory_order_relaxed, memory_order_acquire, memory_order_release,
        memory_order_acq_rel, memory_order_seq_cst
    };
    memory_order failure_orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_seq_cst
    };
    for (auto succ : success_orders) {
        for (auto fail : failure_orders) {
            val = 10;
            bool ok = interlocked_compare_exchange_explicit(&val, 20, 10, succ, fail);
            REQUIRE(ok == true);
            REQUIRE(val == 20);
            ok = interlocked_compare_exchange_explicit(&val, 30, 10, succ, fail);
            REQUIRE(ok == false);
            REQUIRE(val == 20);
        }
    }
}

TEST_CASE("interlocked_compare_exchange32_explicit basic", "[atomic]") {
    volatile std::int32_t val = 10;
    bool ok = interlocked_compare_exchange32_explicit(&val, 20, 10,
        memory_order_seq_cst, memory_order_seq_cst);
    REQUIRE(ok == true);
    REQUIRE(val == 20);
    ok = interlocked_compare_exchange32_explicit(&val, 30, 10,
        memory_order_seq_cst, memory_order_seq_cst);
    REQUIRE(ok == false);
    REQUIRE(val == 20);
}

TEST_CASE("interlocked_compare_exchange64_explicit basic", "[atomic]") {
    volatile std::int64_t val = 100;
    bool ok = interlocked_compare_exchange64_explicit(&val, 200, 100,
        memory_order_seq_cst, memory_order_seq_cst);
    REQUIRE(ok == true);
    REQUIRE(val == 200);
}

TEST_CASE("interlocked_and_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 0xFF;
    std::intptr_t ret = interlocked_and_explicit(&val, 0x0F, memory_order_seq_cst);
    REQUIRE(ret == 0xFF);
    REQUIRE(val == 0x0F);
}

TEST_CASE("interlocked_and8_explicit basic", "[atomic]") {
    volatile std::int8_t val = 0x7F;
    std::int8_t ret = interlocked_and8_explicit(&val, 0x0F, memory_order_seq_cst);
    REQUIRE(ret == 0x7F);
    REQUIRE(val == 0x0F);
}

TEST_CASE("interlocked_and16_explicit basic", "[atomic]") {
    volatile std::int16_t val = 0xFF00;
    std::int16_t ret = interlocked_and16_explicit(&val, 0x0FF0, memory_order_seq_cst);
    REQUIRE(ret == static_cast<std::int16_t>(0xFF00));
    REQUIRE(val == 0x0F00);
}

TEST_CASE("interlocked_and32_explicit basic", "[atomic]") {
    volatile std::int32_t val = 0xFFFF0000;
    std::int32_t ret = interlocked_and32_explicit(&val, 0x0F0F0F0F, memory_order_seq_cst);
    REQUIRE(ret == static_cast<std::int32_t>(0xFFFF0000));
    REQUIRE(val == 0x0F0F0000);
}

TEST_CASE("interlocked_and64_explicit basic", "[atomic]") {
    volatile std::int64_t val = 0xFFFFFFFF00000000LL;
    std::int64_t ret = interlocked_and64_explicit(&val, 0x0F0F0F0F0F0F0F0FLL, memory_order_seq_cst);
    REQUIRE(ret == 0xFFFFFFFF00000000LL);
    REQUIRE(val == 0x0F0F0F0F00000000LL);
}

TEST_CASE("interlocked_or_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 0x0F;
    std::intptr_t ret = interlocked_or_explicit(&val, 0xF0, memory_order_seq_cst);
    REQUIRE(ret == 0x0F);
    REQUIRE(val == 0xFF);
}

TEST_CASE("interlocked_or8_explicit basic", "[atomic]") {
    volatile std::int8_t val = 0x0F;
    std::int8_t ret = interlocked_or8_explicit(&val, 0x70, memory_order_seq_cst);
    REQUIRE(ret == 0x0F);
    REQUIRE(val == 0x7F);
}

TEST_CASE("interlocked_or16_explicit basic", "[atomic]") {
    volatile std::int16_t val = 0x00FF;
    std::int16_t ret = interlocked_or16_explicit(&val, 0xFF00, memory_order_seq_cst);
    REQUIRE(ret == 0x00FF);
    REQUIRE(val == static_cast<std::int16_t>(0xFFFF));
}

TEST_CASE("interlocked_or32_explicit basic", "[atomic]") {
    volatile std::int32_t val = 0x0F0F0F0F;
    std::int32_t ret = interlocked_or32_explicit(&val, 0xF0F0F0F0, memory_order_seq_cst);
    REQUIRE(ret == 0x0F0F0F0F);
    REQUIRE(val == static_cast<std::int32_t>(0xFFFFFFFF));
}

TEST_CASE("interlocked_or64_explicit basic", "[atomic]") {
    volatile std::int64_t val = 0x0F0F0F0F0F0F0F0FLL;
    std::int64_t ret = interlocked_or64_explicit(&val, 0xF0F0F0F0F0F0F0F0LL, memory_order_seq_cst);
    REQUIRE(ret == 0x0F0F0F0F0F0F0F0FLL);
    REQUIRE(val == static_cast<std::int64_t>(0xFFFFFFFFFFFFFFFFLL));
}

TEST_CASE("interlocked_xor_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 0xFF;
    std::intptr_t ret = interlocked_xor_explicit(&val, 0x0F, memory_order_seq_cst);
    REQUIRE(ret == 0xFF);
    REQUIRE(val == 0xF0);
}

TEST_CASE("interlocked_xor8_explicit basic", "[atomic]") {
    volatile std::int8_t val = 0x7F;
    std::int8_t ret = interlocked_xor8_explicit(&val, 0x0F, memory_order_seq_cst);
    REQUIRE(ret == 0x7F);
    REQUIRE(val == 0x70);
}

TEST_CASE("interlocked_xor16_explicit basic", "[atomic]") {
    volatile std::int16_t val = 0xFFFF;
    std::int16_t ret = interlocked_xor16_explicit(&val, 0x00FF, memory_order_seq_cst);
    REQUIRE(ret == static_cast<std::int16_t>(0xFFFF));
    REQUIRE(val == static_cast<std::int16_t>(0xFF00));
}

TEST_CASE("interlocked_xor32_explicit basic", "[atomic]") {
    volatile std::int32_t val = 0xFFFFFFFF;
    std::int32_t ret = interlocked_xor32_explicit(&val, 0x0000FFFF, memory_order_seq_cst);
    REQUIRE(ret == static_cast<std::int32_t>(0xFFFFFFFF));
    REQUIRE(val == static_cast<std::int32_t>(0xFFFF0000));
}

TEST_CASE("interlocked_xor64_explicit basic", "[atomic]") {
    volatile std::int64_t val = 0xFFFFFFFFFFFFFFFFLL;
    std::int64_t ret = interlocked_xor64_explicit(&val, 0x00000000FFFFFFFFLL, memory_order_seq_cst);
    REQUIRE(ret == static_cast<std::int64_t>(0xFFFFFFFFFFFFFFFFLL));
    REQUIRE(val == static_cast<std::int64_t>(0xFFFFFFFF00000000LL));
}

TEST_CASE("iso_volatile_load_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 42;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_seq_cst
    };
    for (auto order : orders) {
        std::intptr_t ret = iso_volatile_load_explicit(&val, order);
        REQUIRE(ret == 42);
    }
}

TEST_CASE("iso_volatile_load8_explicit basic", "[atomic]") {
    volatile std::int8_t val = 7;
    std::int8_t ret = iso_volatile_load8_explicit(&val, memory_order_seq_cst);
    REQUIRE(ret == 7);
}

TEST_CASE("iso_volatile_load16_explicit basic", "[atomic]") {
    volatile std::int16_t val = 1024;
    std::int16_t ret = iso_volatile_load16_explicit(&val, memory_order_seq_cst);
    REQUIRE(ret == 1024);
}

TEST_CASE("iso_volatile_load32_explicit basic", "[atomic]") {
    volatile std::int32_t val = 65536;
    std::int32_t ret = iso_volatile_load32_explicit(&val, memory_order_seq_cst);
    REQUIRE(ret == 65536);
}

TEST_CASE("iso_volatile_load64_explicit basic", "[atomic]") {
    volatile std::int64_t val = 1LL << 40;
    std::int64_t ret = iso_volatile_load64_explicit(&val, memory_order_seq_cst);
    REQUIRE(ret == 1LL << 40);
}

TEST_CASE("iso_volatile_store_explicit basic", "[atomic]") {
    volatile std::intptr_t val = 0;
    void* ptr_val = nullptr;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_release, memory_order_seq_cst
    };
    std::intptr_t value = 99;
    for (auto order : orders) {
        iso_volatile_store_explicit(&val, reinterpret_cast<void*>(&value), order);
        REQUIRE(val == 99);
    }
}

TEST_CASE("iso_volatile_store8_explicit basic", "[atomic]") {
    volatile std::int8_t val = 0;
    iso_volatile_store8_explicit(&val, 127, memory_order_seq_cst);
    REQUIRE(val == 127);
}

TEST_CASE("iso_volatile_store16_explicit basic", "[atomic]") {
    volatile std::int16_t val = 0;
    iso_volatile_store16_explicit(&val, 32767, memory_order_seq_cst);
    REQUIRE(val == 32767);
}

TEST_CASE("iso_volatile_store32_explicit basic", "[atomic]") {
    volatile std::int32_t val = 0;
    iso_volatile_store32_explicit(&val, 2147483647, memory_order_seq_cst);
    REQUIRE(val == 2147483647);
}

TEST_CASE("iso_volatile_store64_explicit basic", "[atomic]") {
    volatile std::int64_t val = 0;
    iso_volatile_store64_explicit(&val, 9223372036854775807LL, memory_order_seq_cst);
    REQUIRE(val == 9223372036854775807LL);
}

TEST_CASE("interlocked_exchange_pointer_explicit basic", "[atomic]") {
    int a = 1, b = 2;
    volatile void* ptr = &a;
    void* ret = interlocked_exchange_pointer_explicit(&ptr, &b, memory_order_seq_cst);
    REQUIRE(ret == &a);
    REQUIRE(ptr == &b);
}

TEST_CASE("interlocked_compare_exchange_pointer_explicit basic", "[atomic]") {
    int a = 1, b = 2, c = 3;
    volatile void* ptr = &a;
    void* ret = interlocked_compare_exchange_pointer_explicit(&ptr, &b, &a,
        memory_order_seq_cst, memory_order_seq_cst);
    REQUIRE(ret == &a);
    REQUIRE(ptr == &b);
    ret = interlocked_compare_exchange_pointer_explicit(&ptr, &c, &a,
        memory_order_seq_cst, memory_order_seq_cst);
    REQUIRE(ret == &b);
    REQUIRE(ptr == &b);
}

TEST_CASE("interlocked_and_or_xor multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0xFFFFFFFF;
    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::atomic<bool> start{false};
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            while (!start.load(std::memory_order_acquire));
            if (i == 0) {
                for (int j = 0; j < 1000; ++j) {
                    interlocked_and32_explicit(&val, 0x0F0F0F0F, memory_order_relaxed);
                    interlocked_or32_explicit(&val, 0xF0F0F0F0, memory_order_relaxed);
                }
            } else {
                for (int j = 0; j < 1000; ++j) {
                    interlocked_xor32_explicit(&val, 0xAAAAAAAA, memory_order_relaxed);
                    interlocked_xor32_explicit(&val, 0xAAAAAAAA, memory_order_relaxed);
                }
            }
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();
    volatile std::int32_t result = iso_volatile_load32_explicit(&val, memory_order_seq_cst);
    (void)result;
    SUCCEED();
}

TEST_CASE("interlocked_increment all memory orders correctness", "[atomic][memory_order]") {
    volatile long val = 0;
    memory_order orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire,
        memory_order_release, memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        val = 0;
        std::thread t1([&]() {
            for (int i = 0; i < 5000; ++i) {
                interlocked_increment_explicit(&val, order);
            }
        });
        std::thread t2([&]() {
            for (int i = 0; i < 5000; ++i) {
                interlocked_increment_explicit(&val, order);
            }
        });
        t1.join();
        t2.join();
        REQUIRE(val == 10000);
    }
}

TEST_CASE("interlocked_exchange_add all sizes", "[atomic][size]") {
    volatile std::int8_t v8 = 0;
    REQUIRE(interlocked_exchange_add8_explicit(&v8, 5, memory_order_seq_cst) == 0);
    REQUIRE(v8 == 5);
    volatile std::int16_t v16 = 10;
    REQUIRE(interlocked_exchange_add16_explicit(&v16, 5, memory_order_seq_cst) == 10);
    REQUIRE(v16 == 15);
    volatile std::int32_t v32 = 100;
    REQUIRE(interlocked_exchange_add32_explicit(&v32, 50, memory_order_seq_cst) == 100);
    REQUIRE(v32 == 150);
    volatile std::int64_t v64 = 1000;
    REQUIRE(interlocked_exchange_add64_explicit(&v64, 500, memory_order_seq_cst) == 1000);
    REQUIRE(v64 == 1500);
}

TEST_CASE("interlocked_exchange_subtract all sizes", "[atomic][size]") {
    volatile std::int8_t v8 = 10;
    REQUIRE(interlocked_exchange_subtract8_explicit(&v8, 3, memory_order_seq_cst) == 10);
    REQUIRE(v8 == 7);
    volatile std::int16_t v16 = 100;
    REQUIRE(interlocked_exchange_subtract16_explicit(&v16, 30, memory_order_seq_cst) == 100);
    REQUIRE(v16 == 70);
    volatile std::int32_t v32 = 1000;
    REQUIRE(interlocked_exchange_subtract32_explicit(&v32, 300, memory_order_seq_cst) == 1000);
    REQUIRE(v32 == 700);
    volatile std::int64_t v64 = 10000;
    REQUIRE(interlocked_exchange_subtract64_explicit(&v64, 3000, memory_order_seq_cst) == 10000);
    REQUIRE(v64 == 7000);
}

TEST_CASE("interlocked_exchange all sizes", "[atomic][size]") {
    volatile std::int8_t v8 = 1;
    REQUIRE(interlocked_exchange8_explicit(&v8, 2, memory_order_seq_cst) == 1);
    REQUIRE(v8 == 2);
    volatile std::int16_t v16 = 10;
    REQUIRE(interlocked_exchange16_explicit(&v16, 20, memory_order_seq_cst) == 10);
    REQUIRE(v16 == 20);
    volatile std::int32_t v32 = 100;
    REQUIRE(interlocked_exchange32_explicit(&v32, 200, memory_order_seq_cst) == 100);
    REQUIRE(v32 == 200);
    volatile std::int64_t v64 = 1000;
    REQUIRE(interlocked_exchange64_explicit(&v64, 2000, memory_order_seq_cst) == 1000);
    REQUIRE(v64 == 2000);
}

using namespace rainy::core::layer;

TEST_CASE("interlocked_increment basic", "[atomic]") {
    volatile long val = 0;
    long ret = interlocked_increment(&val);
    REQUIRE(ret == 1);
    REQUIRE(val == 1);
}

TEST_CASE("interlocked_increment8 basic", "[atomic]") {
    volatile std::int8_t val = 5;
    std::int8_t ret = interlocked_increment8(&val);
    REQUIRE(ret == 6);
    REQUIRE(val == 6);
}

TEST_CASE("interlocked_increment16 basic", "[atomic]") {
    volatile std::int16_t val = 100;
    std::int16_t ret = interlocked_increment16(&val);
    REQUIRE(ret == 101);
    REQUIRE(val == 101);
}

TEST_CASE("interlocked_increment32 basic", "[atomic]") {
    volatile std::int32_t val = 1000;
    std::int32_t ret = interlocked_increment32(&val);
    REQUIRE(ret == 1001);
    REQUIRE(val == 1001);
}

TEST_CASE("interlocked_increment64 basic", "[atomic]") {
    volatile std::int64_t val = 10000;
    std::int64_t ret = interlocked_increment64(&val);
    REQUIRE(ret == 10001);
    REQUIRE(val == 10001);
}

TEST_CASE("interlocked_decrement basic", "[atomic]") {
    volatile long val = 10;
    long ret = interlocked_decrement(&val);
    REQUIRE(ret == 9);
    REQUIRE(val == 9);
}

TEST_CASE("interlocked_decrement8 basic", "[atomic]") {
    volatile std::int8_t val = 10;
    std::int8_t ret = interlocked_decrement8(&val);
    REQUIRE(ret == 9);
    REQUIRE(val == 9);
}

TEST_CASE("interlocked_decrement16 basic", "[atomic]") {
    volatile std::int16_t val = 200;
    std::int16_t ret = interlocked_decrement16(&val);
    REQUIRE(ret == 199);
    REQUIRE(val == 199);
}

TEST_CASE("interlocked_decrement32 basic", "[atomic]") {
    volatile std::int32_t val = 2000;
    std::int32_t ret = interlocked_decrement32(&val);
    REQUIRE(ret == 1999);
    REQUIRE(val == 1999);
}

TEST_CASE("interlocked_decrement64 basic", "[atomic]") {
    volatile std::int64_t val = 20000;
    std::int64_t ret = interlocked_decrement64(&val);
    REQUIRE(ret == 19999);
    REQUIRE(val == 19999);
}

TEST_CASE("interlocked_increment multithreaded", "[atomic][thread]") {
    volatile long val = 0;
    const int num_threads = 8;
    const int iters_per_thread = 5000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == num_threads * iters_per_thread);
}

TEST_CASE("interlocked_increment8 multithreaded", "[atomic][thread]") {
    volatile std::int8_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 100;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment8(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == static_cast<std::int8_t>(num_threads * iters_per_thread));
}

TEST_CASE("interlocked_increment16 multithreaded", "[atomic][thread]") {
    volatile std::int16_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment16(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == static_cast<std::int16_t>(num_threads * iters_per_thread));
}

TEST_CASE("interlocked_increment32 multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment32(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == num_threads * iters_per_thread);
}

TEST_CASE("interlocked_increment64 multithreaded", "[atomic][thread]") {
    volatile std::int64_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_increment64(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == static_cast<std::int64_t>(num_threads) * iters_per_thread);
}

TEST_CASE("interlocked_decrement multithreaded", "[atomic][thread]") {
    volatile long val = 40000;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_decrement(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == 0);
}

TEST_CASE("interlocked_decrement8 multithreaded", "[atomic][thread]") {
    volatile std::int8_t val = 100;
    const int num_threads = 4;
    const int iters_per_thread = 25;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_decrement8(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == 0);
}

TEST_CASE("interlocked_decrement32 multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 40000;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_decrement32(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == 0);
}

TEST_CASE("interlocked_decrement64 multithreaded", "[atomic][thread]") {
    volatile std::int64_t val = 40000;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_decrement64(&val);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == 0);
}

TEST_CASE("interlocked_exchange_add8 basic", "[atomic]") {
    volatile std::int8_t val = 5;
    std::int8_t ret = interlocked_exchange_add8(&val, 3);
    REQUIRE(ret == 5);
    REQUIRE(val == 8);
}

TEST_CASE("interlocked_exchange_add16 basic", "[atomic]") {
    volatile std::int16_t val = 100;
    std::int16_t ret = interlocked_exchange_add16(&val, 50);
    REQUIRE(ret == 100);
    REQUIRE(val == 150);
}

TEST_CASE("interlocked_exchange_add32 basic", "[atomic]") {
    volatile std::int32_t val = 1000;
    std::int32_t ret = interlocked_exchange_add32(&val, 500);
    REQUIRE(ret == 1000);
    REQUIRE(val == 1500);
}

TEST_CASE("interlocked_exchange_add64 basic", "[atomic]") {
    volatile std::int64_t val = 10000;
    std::int64_t ret = interlocked_exchange_add64(&val, 5000);
    REQUIRE(ret == 10000);
    REQUIRE(val == 15000);
}

TEST_CASE("interlocked_exchange_add multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_exchange_add32(&val, 1);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == num_threads * iters_per_thread);
}

TEST_CASE("interlocked_exchange_subtract basic", "[atomic]") {
    volatile std::intptr_t val = 50;
    std::intptr_t ret = interlocked_exchange_subtract(&val, 20);
    REQUIRE(ret == 50);
    REQUIRE(val == 30);
}

TEST_CASE("interlocked_exchange_subtract8 basic", "[atomic]") {
    volatile std::int8_t val = 20;
    std::int8_t ret = interlocked_exchange_subtract8(&val, 7);
    REQUIRE(ret == 20);
    REQUIRE(val == 13);
}

TEST_CASE("interlocked_exchange_subtract16 basic", "[atomic]") {
    volatile std::int16_t val = 200;
    std::int16_t ret = interlocked_exchange_subtract16(&val, 75);
    REQUIRE(ret == 200);
    REQUIRE(val == 125);
}

TEST_CASE("interlocked_exchange_subtract32 basic", "[atomic]") {
    volatile std::int32_t val = 2000;
    std::int32_t ret = interlocked_exchange_subtract32(&val, 750);
    REQUIRE(ret == 2000);
    REQUIRE(val == 1250);
}

TEST_CASE("interlocked_exchange_subtract64 basic", "[atomic]") {
    volatile std::int64_t val = 20000;
    std::int64_t ret = interlocked_exchange_subtract64(&val, 7500);
    REQUIRE(ret == 20000);
    REQUIRE(val == 12500);
}

TEST_CASE("interlocked_exchange_subtract multithreaded", "[atomic][thread]") {
    volatile std::intptr_t val = 40000;
    const int num_threads = 4;
    const int iters_per_thread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters_per_thread; ++j) {
                interlocked_exchange_subtract(&val, 1);
            }
        });
    }
    for (auto& t : threads) t.join();
    REQUIRE(val == 0);
}

TEST_CASE("iso_volatile_load basic", "[atomic]") {
    volatile std::intptr_t val = 42;
    std::intptr_t ret = iso_volatile_load(&val);
    REQUIRE(ret == 42);
}

TEST_CASE("iso_volatile_load8 basic", "[atomic]") {
    volatile std::int8_t val = 15;
    std::int8_t ret = iso_volatile_load8(&val);
    REQUIRE(ret == 15);
}

TEST_CASE("iso_volatile_load16 basic", "[atomic]") {
    volatile std::int16_t val = 2048;
    std::int16_t ret = iso_volatile_load16(&val);
    REQUIRE(ret == 2048);
}

TEST_CASE("iso_volatile_load32 basic", "[atomic]") {
    volatile std::int32_t val = 131072;
    std::int32_t ret = iso_volatile_load32(&val);
    REQUIRE(ret == 131072);
}

TEST_CASE("iso_volatile_load64 basic", "[atomic]") {
    volatile std::int64_t val = 1LL << 42;
    std::int64_t ret = iso_volatile_load64(&val);
    REQUIRE(ret == 1LL << 42);
}

TEST_CASE("interlocked_exchange basic", "[atomic]") {
    volatile std::intptr_t val = 10;
    std::intptr_t ret = interlocked_exchange(&val, 99);
    REQUIRE(ret == 10);
    REQUIRE(val == 99);
}

TEST_CASE("interlocked_exchange8 basic", "[atomic]") {
    volatile std::int8_t val = 1;
    std::int8_t ret = interlocked_exchange8(&val, 127);
    REQUIRE(ret == 1);
    REQUIRE(val == 127);
}

TEST_CASE("interlocked_exchange16 basic", "[atomic]") {
    volatile std::int16_t val = 100;
    std::int16_t ret = interlocked_exchange16(&val, 200);
    REQUIRE(ret == 100);
    REQUIRE(val == 200);
}

TEST_CASE("interlocked_exchange32 basic", "[atomic]") {
    volatile std::int32_t val = 1000;
    std::int32_t ret = interlocked_exchange32(&val, 2000);
    REQUIRE(ret == 1000);
    REQUIRE(val == 2000);
}

TEST_CASE("interlocked_exchange64 basic", "[atomic]") {
    volatile std::int64_t val = 10000;
    std::int64_t ret = interlocked_exchange64(&val, 20000);
    REQUIRE(ret == 10000);
    REQUIRE(val == 20000);
}

TEST_CASE("interlocked_exchange_pointer basic", "[atomic]") {
    int a = 1, b = 2;
    volatile void* ptr = &a;
    void* ret = interlocked_exchange_pointer(&ptr, &b);
    REQUIRE(ret == &a);
    REQUIRE(ptr == &b);
}

TEST_CASE("interlocked_exchange multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0;
    std::atomic<bool> start{false};
    std::atomic<int> exchanges{0};
    const int num_threads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            while (!start.load(std::memory_order_acquire));
            for (int j = 0; j < 1000; ++j) {
                std::int32_t old = interlocked_exchange32(&val, i * 1000 + j);
                (void)old;
                exchanges.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();
    REQUIRE(exchanges.load() == num_threads * 1000);
}

TEST_CASE("interlocked_compare_exchange basic", "[atomic]") {
    volatile long val = 10;
    bool ok = interlocked_compare_exchange(&val, 20, 10);
    REQUIRE(ok == true);
    REQUIRE(val == 20);
    ok = interlocked_compare_exchange(&val, 30, 10);
    REQUIRE(ok == false);
    REQUIRE(val == 20);
}

TEST_CASE("interlocked_compare_exchange8 basic", "[atomic]") {
    volatile std::int8_t val = 5;
    bool ok = interlocked_compare_exchange8(&val, 10, 5);
    REQUIRE(ok == true);
    REQUIRE(val == 10);
    ok = interlocked_compare_exchange8(&val, 15, 5);
    REQUIRE(ok == false);
    REQUIRE(val == 10);
}

TEST_CASE("interlocked_compare_exchange16 basic", "[atomic]") {
    volatile std::int16_t val = 100;
    bool ok = interlocked_compare_exchange16(&val, 200, 100);
    REQUIRE(ok == true);
    REQUIRE(val == 200);
    ok = interlocked_compare_exchange16(&val, 300, 100);
    REQUIRE(ok == false);
    REQUIRE(val == 200);
}

TEST_CASE("interlocked_compare_exchange32 basic", "[atomic]") {
    volatile std::int32_t val = 1000;
    bool ok = interlocked_compare_exchange32(&val, 2000, 1000);
    REQUIRE(ok == true);
    REQUIRE(val == 2000);
    ok = interlocked_compare_exchange32(&val, 3000, 1000);
    REQUIRE(ok == false);
    REQUIRE(val == 2000);
}

TEST_CASE("interlocked_compare_exchange64 basic", "[atomic]") {
    volatile std::int64_t val = 10000;
    bool ok = interlocked_compare_exchange64(&val, 20000, 10000);
    REQUIRE(ok == true);
    REQUIRE(val == 20000);
    ok = interlocked_compare_exchange64(&val, 30000, 10000);
    REQUIRE(ok == false);
    REQUIRE(val == 20000);
}

TEST_CASE("interlocked_compare_exchange_pointer basic", "[atomic]") {
    int a = 1, b = 2, c = 3;
    volatile void* ptr = &a;
    void* ret = interlocked_compare_exchange_pointer(&ptr, &b, &a);
    REQUIRE(ret == &a);
    REQUIRE(ptr == &b);
    ret = interlocked_compare_exchange_pointer(&ptr, &c, &a);
    REQUIRE(ret == &b);
    REQUIRE(ptr == &b);
}

TEST_CASE("interlocked_compare_exchange multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0;
    std::atomic<bool> start{false};
    const int num_threads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            while (!start.load(std::memory_order_acquire));
            for (int j = 0; j < 1000; ++j) {
                std::int32_t expected = i * 1000 + j;
                while (!interlocked_compare_exchange32(&val, expected + 1, expected)) {
                    expected = iso_volatile_load32(&val);
                }
            }
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();
    REQUIRE(val == num_threads * 1000);
}

TEST_CASE("interlocked_and basic", "[atomic]") {
    volatile std::intptr_t val = 0xFF;
    std::intptr_t ret = interlocked_and(&val, 0x0F);
    REQUIRE(ret == 0xFF);
    REQUIRE(val == 0x0F);
}

TEST_CASE("interlocked_and8 basic", "[atomic]") {
    volatile std::int8_t val = 0x7F;
    std::int8_t ret = interlocked_and8(&val, 0x0F);
    REQUIRE(ret == 0x7F);
    REQUIRE(val == 0x0F);
}

TEST_CASE("interlocked_and16 basic", "[atomic]") {
    volatile std::int16_t val = 0xFF00;
    std::int16_t ret = interlocked_and16(&val, 0x0FF0);
    REQUIRE(ret == static_cast<std::int16_t>(0xFF00));
    REQUIRE(val == 0x0F00);
}

TEST_CASE("interlocked_and32 basic", "[atomic]") {
    volatile std::int32_t val = 0xFFFF0000;
    std::int32_t ret = interlocked_and32(&val, 0x0F0F0F0F);
    REQUIRE(ret == static_cast<std::int32_t>(0xFFFF0000));
    REQUIRE(val == 0x0F0F0000);
}

TEST_CASE("interlocked_and64 basic", "[atomic]") {
    volatile std::int64_t val = 0xFFFFFFFF00000000LL;
    std::int64_t ret = interlocked_and64(&val, 0x0F0F0F0F0F0F0F0FLL);
    REQUIRE(ret == 0xFFFFFFFF00000000LL);
    REQUIRE(val == 0x0F0F0F0F00000000LL);
}

TEST_CASE("interlocked_or basic", "[atomic]") {
    volatile std::intptr_t val = 0x0F;
    std::intptr_t ret = interlocked_or(&val, 0xF0);
    REQUIRE(ret == 0x0F);
    REQUIRE(val == 0xFF);
}

TEST_CASE("interlocked_or8 basic", "[atomic]") {
    volatile std::int8_t val = 0x0F;
    std::int8_t ret = interlocked_or8(&val, 0x70);
    REQUIRE(ret == 0x0F);
    REQUIRE(val == 0x7F);
}

TEST_CASE("interlocked_or16 basic", "[atomic]") {
    volatile std::int16_t val = 0x00FF;
    std::int16_t ret = interlocked_or16(&val, 0xFF00);
    REQUIRE(ret == 0x00FF);
    REQUIRE(val == static_cast<std::int16_t>(0xFFFF));
}

TEST_CASE("interlocked_or32 basic", "[atomic]") {
    volatile std::int32_t val = 0x0F0F0F0F;
    std::int32_t ret = interlocked_or32(&val, 0xF0F0F0F0);
    REQUIRE(ret == 0x0F0F0F0F);
    REQUIRE(val == static_cast<std::int32_t>(0xFFFFFFFF));
}

TEST_CASE("interlocked_or64 basic", "[atomic]") {
    volatile std::int64_t val = 0x0F0F0F0F0F0F0F0FLL;
    std::int64_t ret = interlocked_or64(&val, 0xF0F0F0F0F0F0F0F0LL);
    REQUIRE(ret == 0x0F0F0F0F0F0F0F0FLL);
    REQUIRE(val == static_cast<std::int64_t>(0xFFFFFFFFFFFFFFFFLL));
}

TEST_CASE("interlocked_xor basic", "[atomic]") {
    volatile std::intptr_t val = 0xFF;
    std::intptr_t ret = interlocked_xor(&val, 0x0F);
    REQUIRE(ret == 0xFF);
    REQUIRE(val == 0xF0);
}

TEST_CASE("interlocked_xor8 basic", "[atomic]") {
    volatile std::int8_t val = 0x7F;
    std::int8_t ret = interlocked_xor8(&val, 0x0F);
    REQUIRE(ret == 0x7F);
    REQUIRE(val == 0x70);
}

TEST_CASE("interlocked_xor16 basic", "[atomic]") {
    volatile std::int16_t val = 0xFFFF;
    std::int16_t ret = interlocked_xor16(&val, 0x00FF);
    REQUIRE(ret == static_cast<std::int16_t>(0xFFFF));
    REQUIRE(val == static_cast<std::int16_t>(0xFF00));
}

TEST_CASE("interlocked_xor32 basic", "[atomic]") {
    volatile std::int32_t val = 0xFFFFFFFF;
    std::int32_t ret = interlocked_xor32(&val, 0x0000FFFF);
    REQUIRE(ret == static_cast<std::int32_t>(0xFFFFFFFF));
    REQUIRE(val == static_cast<std::int32_t>(0xFFFF0000));
}

TEST_CASE("interlocked_xor64 basic", "[atomic]") {
    volatile std::int64_t val = 0xFFFFFFFFFFFFFFFFLL;
    std::int64_t ret = interlocked_xor64(&val, 0x00000000FFFFFFFFLL);
    REQUIRE(ret == static_cast<std::int64_t>(0xFFFFFFFFFFFFFFFFLL));
    REQUIRE(val == static_cast<std::int64_t>(0xFFFFFFFF00000000LL));
}

TEST_CASE("interlocked_bitops multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0xFFFFFFFF;
    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::atomic<bool> start{false};
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            while (!start.load(std::memory_order_acquire));
            if (i % 2 == 0) {
                for (int j = 0; j < 1000; ++j) {
                    interlocked_and32(&val, 0x0F0F0F0F);
                    interlocked_or32(&val, 0xF0F0F0F0);
                }
            } else {
                for (int j = 0; j < 1000; ++j) {
                    interlocked_xor32(&val, 0xAAAAAAAA);
                    interlocked_xor32(&val, 0xAAAAAAAA);
                }
            }
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();
    volatile std::int32_t result = iso_volatile_load32(&val);
    (void)result;
    SUCCEED();
}

TEST_CASE("iso_volatile_store basic", "[atomic]") {
    volatile std::intptr_t val = 0;
    int value = 99;
    iso_volatile_store(&val, reinterpret_cast<void*>(&value));
    REQUIRE(val == 99);
}

TEST_CASE("iso_volatile_store8 basic", "[atomic]") {
    volatile std::int8_t val = 0;
    iso_volatile_store8(&val, 127);
    REQUIRE(val == 127);
}

TEST_CASE("iso_volatile_store16 basic", "[atomic]") {
    volatile std::int16_t val = 0;
    iso_volatile_store16(&val, 32767);
    REQUIRE(val == 32767);
}

TEST_CASE("iso_volatile_store32 basic", "[atomic]") {
    volatile std::int32_t val = 0;
    iso_volatile_store32(&val, 2147483647);
    REQUIRE(val == 2147483647);
}

TEST_CASE("iso_volatile_store64 basic", "[atomic]") {
    volatile std::int64_t val = 0;
    iso_volatile_store64(&val, 9223372036854775807LL);
    REQUIRE(val == 9223372036854775807LL);
}

TEST_CASE("iso_volatile_load_store multithreaded", "[atomic][thread]") {
    volatile std::int32_t val = 0;
    std::atomic<bool> start{false};
    const int num_threads = 4;
    const int iters = 10000;
    std::vector<std::thread> writers;
    std::vector<std::thread> readers;
    std::atomic<int> read_count{0};
    std::atomic<int> errors{0};
    for (int i = 0; i < num_threads / 2; ++i) {
        writers.emplace_back([&]() {
            while (!start.load(std::memory_order_acquire));
            for (int j = 0; j < iters; ++j) {
                iso_volatile_store32(&val, j);
            }
        });
    }
    for (int i = 0; i < num_threads / 2; ++i) {
        readers.emplace_back([&]() {
            while (!start.load(std::memory_order_acquire));
            for (int j = 0; j < iters; ++j) {
                volatile std::int32_t v = iso_volatile_load32(&val);
                if (v < 0) {
                    errors.fetch_add(1, std::memory_order_relaxed);
                }
                read_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();
    REQUIRE(errors.load() == 0);
    REQUIRE(read_count.load() == (num_threads / 2) * iters);
}

TEST_CASE("atomic_thread_fence basic", "[atomic][fence]") {
    std::atomic<int> flag{0};
    int data = 0;
    std::thread t1([&]() {
        data = 42;
        atomic_thread_fence(memory_order_release);
        flag.store(1, std::memory_order_relaxed);
    });
    std::thread t2([&]() {
        while (flag.load(std::memory_order_relaxed) == 0) {}
        atomic_thread_fence(memory_order_acquire);
        REQUIRE(data == 42);
    });
    t1.join();
    t2.join();
}

TEST_CASE("atomic_thread_fence all orders", "[atomic][fence]") {
    memory_order orders[] = {
        memory_order_relaxed, memory_order_consume, memory_order_acquire,
        memory_order_release, memory_order_acq_rel, memory_order_seq_cst
    };
    for (auto order : orders) {
        atomic_thread_fence(order);
        SUCCEED();
    }
}

TEST_CASE("read_write_barrier basic", "[atomic][barrier]") {
    std::atomic<int> flag{0};
    int data = 0;
    std::thread t1([&]() {
        data = 42;
        write_barrier();
        read_write_barrier();
        flag.store(1, std::memory_order_relaxed);
    });
    std::thread t2([&]() {
        while (flag.load(std::memory_order_relaxed) == 0) {}
        read_barrier();
        read_write_barrier();
        REQUIRE(data == 42);
    });
    t1.join();
    t2.join();
}

TEST_CASE("read_write_barrier standalone", "[atomic][barrier]") {
    read_barrier();
    write_barrier();
    read_write_barrier();
    SUCCEED();
}

TEST_CASE("barrier multithreaded ordering", "[atomic][barrier][thread]") {
    const int num_threads = 4;
    std::atomic<int> ready{0};
    std::atomic<int> data{0};
    std::atomic<int> observed{0};
    std::thread writer([&]() {
        data.store(1, std::memory_order_relaxed);
        write_barrier();
        read_write_barrier();
        ready.store(1, std::memory_order_relaxed);
    });
    std::vector<std::thread> readers;
    for (int i = 0; i < num_threads - 1; ++i) {
        readers.emplace_back([&]() {
            while (ready.load(std::memory_order_relaxed) == 0) {}
            read_barrier();
            read_write_barrier();
            observed.fetch_add(data.load(std::memory_order_relaxed), std::memory_order_relaxed);
        });
    }
    writer.join();
    for (auto& t : readers) t.join();
    REQUIRE(observed.load() == num_threads - 1);
}