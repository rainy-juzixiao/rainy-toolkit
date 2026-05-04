#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <rainy/core/core.hpp>
#include <mutex>
#include <thread>
#include <vector>

template <typename T>
struct SafeValues;

template <>
struct SafeValues<std::int8_t> {
    static constexpr std::int8_t start = 0;
    static constexpr std::int8_t delta = 1;
    static constexpr int threads = 4;
    static constexpr int iterations = 10;
};
template <>
struct SafeValues<std::int16_t> {
    static constexpr std::int16_t start = 0;
    static constexpr std::int16_t delta = 1;
    static constexpr int threads = 4;
    static constexpr int iterations = 100;
};
template <>
struct SafeValues<std::int32_t> {
    static constexpr std::int32_t start = 0;
    static constexpr std::int32_t delta = 1;
    static constexpr int threads = 4;
    static constexpr int iterations = 10000;
};
template <>
struct SafeValues<std::int64_t> {
    static constexpr std::int64_t start = 0;
    static constexpr std::int64_t delta = 1;
    static constexpr int threads = 4;
    static constexpr int iterations = 10000;
};

using namespace rainy::core::pal;

TEMPLATE_TEST_CASE("Interlocked increment and decrement thread safety", "[atomic][multithread]", std::int8_t, std::int16_t,
                   std::int32_t, std::int64_t) {
    using Type = TestType;
    volatile Type value = SafeValues<Type>::start;
    GIVEN("A safe initial value for type") {
        WHEN("Multiple threads increment concurrently") {
            std::vector<std::thread> threads;
            for (int i = 0; i < SafeValues<Type>::threads; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_increment8(reinterpret_cast<volatile std::int8_t *>(&value));
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_increment16(reinterpret_cast<volatile std::int16_t *>(&value));
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_increment32(reinterpret_cast<volatile std::int32_t *>(&value));
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_increment64(reinterpret_cast<volatile std::int64_t *>(&value));
                        }
                    }
                });
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("Final value is threads * iterations") {
                REQUIRE(value == SafeValues<Type>::threads * SafeValues<Type>::iterations);
            }
        }
        WHEN("Multiple threads decrement concurrently after incrementing") {
            value = SafeValues<Type>::start;
            std::vector<std::thread> threads;
            for (int i = 0; i < SafeValues<Type>::threads; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_increment8(reinterpret_cast<volatile std::int8_t *>(&value));
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_increment16(reinterpret_cast<volatile std::int16_t *>(&value));
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_increment32(reinterpret_cast<volatile std::int32_t *>(&value));
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_increment64(reinterpret_cast<volatile std::int64_t *>(&value));
                        }
                    }
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_decrement8(reinterpret_cast<volatile std::int8_t *>(&value));
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_decrement16(reinterpret_cast<volatile std::int16_t *>(&value));
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_decrement32(reinterpret_cast<volatile std::int32_t *>(&value));
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_decrement64(reinterpret_cast<volatile std::int64_t *>(&value));
                        }
                    }
                });
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("Final value returns to start") {
                REQUIRE(value == SafeValues<Type>::start);
            }
        }
    }
}

TEMPLATE_TEST_CASE("Interlocked exchange-add/subtract thread safety", "[atomic][multithread]", std::int8_t, std::int16_t, std::int32_t,
                   std::int64_t) {
    using Type = TestType;
    volatile Type value = SafeValues<Type>::start;

    GIVEN("A safe initial value") {
        WHEN("Multiple threads use exchange-add concurrently") {
            std::vector<std::thread> threads;
            for (int i = 0; i < SafeValues<Type>::threads; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_exchange_add8(reinterpret_cast<volatile std::int8_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_exchange_add16(reinterpret_cast<volatile std::int16_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_exchange_add32(reinterpret_cast<volatile std::int32_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_exchange_add64(reinterpret_cast<volatile std::int64_t *>(&value), SafeValues<Type>::delta);
                        }
                    }
                });
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("Final value equals threads * iterations * delta") {
                REQUIRE(value == SafeValues<Type>::threads * SafeValues<Type>::iterations * SafeValues<Type>::delta);
            }
        }

        WHEN("Multiple threads use exchange-subtract after adding") {
            value = SafeValues<Type>::start;
            std::vector<std::thread> threads;
            for (int i = 0; i < SafeValues<Type>::threads; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_exchange_add8(reinterpret_cast<volatile std::int8_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_exchange_add16(reinterpret_cast<volatile std::int16_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_exchange_add32(reinterpret_cast<volatile std::int32_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_exchange_add64(reinterpret_cast<volatile std::int64_t *>(&value), SafeValues<Type>::delta);
                        }
                    }
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_exchange_subtract8(reinterpret_cast<volatile std::int8_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_exchange_subtract16(reinterpret_cast<volatile std::int16_t *>(&value),
                                                            SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_exchange_subtract32(reinterpret_cast<volatile std::int32_t *>(&value),
                                                            SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_exchange_subtract64(reinterpret_cast<volatile std::int64_t *>(&value),
                                                            SafeValues<Type>::delta);
                        }
                    }
                });
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("Final value returns to start") {
                REQUIRE(value == SafeValues<Type>::start);
            }
        }
    }
}

TEMPLATE_TEST_CASE("iso_volatile_load thread safety", "[atomic][multithread]", std::intptr_t, std::int8_t, std::int16_t, std::int32_t,
                   std::int64_t) {
    using Type = TestType;
    volatile Type value = SafeValues<Type>::start;

    GIVEN("A safe initial value") {
        WHEN("Multiple threads concurrently read the value") {
            std::vector<std::thread> threads;
            std::vector<Type> results(SafeValues<Type>::threads);
            for (int i = 0; i < SafeValues<Type>::threads; ++i) {
                threads.emplace_back(
                    [&](int idx) {
                        for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                            if constexpr (sizeof(Type) == 1) {
                                results[idx] = iso_volatile_load8(reinterpret_cast<volatile std::int8_t *>(&value)); // NOLINT
                            } else if constexpr (sizeof(Type) == 2) {
                                results[idx] = iso_volatile_load16(reinterpret_cast<volatile std::int16_t *>(&value)); // NOLINT
                            } else if constexpr (sizeof(Type) == 4) {
                                results[idx] = iso_volatile_load32(reinterpret_cast<volatile std::int32_t *>(&value)); // NOLINT
                            } else if constexpr (sizeof(Type) == 8) {
                                results[idx] = iso_volatile_load64(reinterpret_cast<volatile std::int64_t *>(&value)); // NOLINT
                            }
                        }
                    },
                    i);
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("All threads should read the same final value") {
                for (auto &result: results) {
                    REQUIRE(result == SafeValues<Type>::start);
                }
            }
        }
        WHEN("Multiple threads concurrently read and then modify the value") {
            value = SafeValues<Type>::start;
            std::vector<std::thread> threads;
            for (int i = 0; i < SafeValues<Type>::threads; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < SafeValues<Type>::iterations; ++j) {
                        if constexpr (sizeof(Type) == 1) {
                            iso_volatile_load8(reinterpret_cast<volatile std::int8_t *>(&value));
                        } else if constexpr (sizeof(Type) == 2) {
                            iso_volatile_load16(reinterpret_cast<volatile std::int16_t *>(&value));
                        } else if constexpr (sizeof(Type) == 4) {
                            iso_volatile_load32(reinterpret_cast<volatile std::int32_t *>(&value));
                        } else if constexpr (sizeof(Type) == 8) {
                            iso_volatile_load64(reinterpret_cast<volatile std::int64_t *>(&value));
                        }
                        // 修改数据，确保没有冲突
                        if constexpr (sizeof(Type) == 1) {
                            interlocked_exchange_add8(reinterpret_cast<volatile std::int8_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 2) {
                            interlocked_exchange_add16(reinterpret_cast<volatile std::int16_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 4) {
                            interlocked_exchange_add32(reinterpret_cast<volatile std::int32_t *>(&value), SafeValues<Type>::delta);
                        } else if constexpr (sizeof(Type) == 8) {
                            interlocked_exchange_add64(reinterpret_cast<volatile std::int64_t *>(&value), SafeValues<Type>::delta);
                        }
                    }
                });
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("The final value should be threads * iterations * delta added to the original value") {
                REQUIRE(value ==
                        SafeValues<Type>::start + SafeValues<Type>::threads * SafeValues<Type>::iterations * SafeValues<Type>::delta);
            }
        }
    }
}

TEMPLATE_TEST_CASE("interlocked_exchange provides atomic replacement semantics", "[atomic][multithread][exchange]", std::int8_t,
                   std::int16_t, std::int32_t, std::int64_t) {
    using T = TestType;
    constexpr int threads = 4;
    constexpr int iterations = 100;
    // 每个线程写一个唯一、不会溢出的值
    constexpr T thread_values[threads] = {
        static_cast<T>(1),
        static_cast<T>(2),
        static_cast<T>(3),
        static_cast<T>(4),
    };
    volatile T value = static_cast<T>(0);
    GIVEN("A shared variable accessed via interlocked_exchange") {
        WHEN("Multiple threads repeatedly exchange distinct constant values") {
            std::vector<std::thread> workers;
            std::vector<T> observed_old_values;
            std::mutex record_mutex;
            for (const auto &thread_value: thread_values) {
                workers.emplace_back([&] {
                    for (int j = 0; j < iterations; ++j) {
                        T old;
                        if constexpr (sizeof(T) == 1) {
                            old = interlocked_exchange8(reinterpret_cast<volatile std::int8_t *>(&value), thread_value);
                        } else if constexpr (sizeof(T) == 2) {
                            old = interlocked_exchange16(reinterpret_cast<volatile std::int16_t *>(&value), thread_value);
                        } else if constexpr (sizeof(T) == 4) {
                            old = interlocked_exchange32(reinterpret_cast<volatile std::int32_t *>(&value), thread_value);
                        } else {
                            old = interlocked_exchange64(reinterpret_cast<volatile std::int64_t *>(&value), thread_value);
                        }
                        std::lock_guard lock(record_mutex); // NOLINT
                        observed_old_values.push_back(old);
                    }
                });
            }
            for (auto &t: workers) {
                t.join();
            }
            THEN("The final value must be one of the exchanged values") {
                bool match = false;
                for (auto v: thread_values) {
                    match |= (value == v);
                }
                REQUIRE(match);
            }
            THEN("All observed old values must be valid previously-written values") {
                for (auto old: observed_old_values) {
                    bool valid = (old == static_cast<T>(0));
                    for (auto v: thread_values) {
                        valid |= (old == v);
                    }
                    REQUIRE(valid);
                }
            }
        }
    }
}

TEST_CASE("interlocked_exchange_pointer provides atomic pointer replacement semantics", "[atomic][multithread][exchange][pointer]") {
    constexpr int threads = 4;
    constexpr int iterations = 100;
    int objects[threads] = {1, 2, 3, 4}; // 使用稳定地址，避免 ABA / 生命周期问题
    void *thread_values[threads] = {
        &objects[0],
        &objects[1],
        &objects[2],
        &objects[3],
    };
    volatile void *shared = nullptr;
    GIVEN("A shared pointer accessed via interlocked_exchange_pointer") {
        WHEN("Multiple threads repeatedly exchange distinct pointer values") {
            std::vector<std::thread> workers;
            std::vector<void *> observed_old_values;
            std::mutex record_mutex;
            for (auto &thread_value: thread_values) {
                workers.emplace_back([&] {
                    for (int j = 0; j < iterations; ++j) {
                        std::lock_guard lock(record_mutex); // NOLINT
                        void *old = interlocked_exchange_pointer(const_cast<volatile void **>(&shared), thread_value); // NOLINT
                        observed_old_values.push_back(old);
                    }
                });
            }
            for (auto &t: workers) {
                t.join();
            }
            THEN("The final pointer must be one of the exchanged pointer values") {
                bool match = false;
                for (auto &p: thread_values) {
                    match |= (shared == p);
                }
                REQUIRE(match);
            }

            THEN("All observed old pointers must be valid previously-written values or null") {
                for (const auto *old: observed_old_values) {
                    bool valid = (old == nullptr);
                    for (auto &p: thread_values) {
                        valid |= (old == p);
                    }
                    REQUIRE(valid);
                }
            }
        }
    }
}
