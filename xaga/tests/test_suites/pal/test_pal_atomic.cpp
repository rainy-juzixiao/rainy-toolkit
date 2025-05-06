#include <atomic>
#include <gtest/gtest.h>
#include <rainy/core/core.hpp>
#include <thread>
#include <vector>

using namespace rainy::core::pal;

class InterlockedTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }

    template <typename Func>
    void run_in_threads(Func &&func, size_t num_threads) {
        std::vector<std::thread> threads;
        for (size_t i = 0; i < num_threads; ++i) {
            threads.push_back(std::thread(func));
        }
        for (auto &t: threads) {
            t.join();
        }
    }
};

TEST_F(InterlockedTest, TestInterlockedIncrement) {
    volatile long value = 0;
    auto increment_task = [&value]() { interlocked_increment(&value); };

    run_in_threads(increment_task, 10); // 创建10个线程并行执行
    EXPECT_EQ(value, 10); // 期望value等于10
}

TEST_F(InterlockedTest, TestInterlockedDecrement) {
    volatile long value = 2;
    auto decrement_task = [&value]() { interlocked_decrement(&value); };

    run_in_threads(decrement_task, 2); // 创建2个线程并行执行
    EXPECT_EQ(value, 0); // 期望value等于0
}

TEST_F(InterlockedTest, TestInterlockedExchangeAdd) {
    volatile std::intptr_t value = 10;
    auto exchange_add_task = [&value]() { interlocked_exchange_add(&value, 5); };

    run_in_threads(exchange_add_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 25); // 期望value等于25
}

TEST_F(InterlockedTest, TestInterlockedExchangeSubtract) {
    volatile std::intptr_t value = 10;
    auto exchange_subtract_task = [&value]() { interlocked_exchange_subtract(&value, 3); };

    run_in_threads(exchange_subtract_task, 2); // 创建2个线程并行执行
    EXPECT_EQ(value, 4); // 期望value等于4
}

TEST_F(InterlockedTest, TestIsoVolatileLoad32) {
    volatile int value = 42;
    auto load_task = [&value]() { iso_volatile_load32(&value); };

    run_in_threads(load_task, 5); // 创建5个线程并行执行
    EXPECT_EQ(value, 42u); // 期望value等于42
}

TEST_F(InterlockedTest, TestIsoVolatileLoad64) {
    volatile long long value = 42;
    auto load_task = [&value]() { iso_volatile_load64(&value); };

    run_in_threads(load_task, 5); // 创建5个线程并行执行
    EXPECT_EQ(value, 42ull); // 期望value等于42
}

TEST_F(InterlockedTest, TestInterlockedExchange) {
    volatile long value = 10;
    auto exchange_task = [&value]() { interlocked_exchange(&value, 20); };

    run_in_threads(exchange_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 20); // 期望value等于20
}

TEST_F(InterlockedTest, TestInterlockedExchangePointer) {
    int a = 10, b = 20;
    void *ptr = &a;
    auto exchange_pointer_task = [&ptr, &a, &b]() { interlocked_exchange_pointer((volatile void **) &ptr, &b); };

    run_in_threads(exchange_pointer_task, 2); // 创建2个线程并行执行
    EXPECT_EQ(ptr, &b); // 期望ptr等于b
}

TEST_F(InterlockedTest, TestInterlockedCompareExchange) {
    volatile long value = 10;
    auto compare_exchange_task = [&value]() { interlocked_compare_exchange(&value, 20, 10); };

    run_in_threads(compare_exchange_task, 2); // 创建2个线程并行执行
    EXPECT_EQ(value, 20); // 期望value等于20
}

TEST_F(InterlockedTest, TestInterlockedCompareExchangePointer) {
    int a = 10, b = 20;
    void *ptr = &a;
    auto compare_exchange_pointer_task = [&ptr, &a, &b]() { interlocked_compare_exchange_pointer((volatile void **) &ptr, &b, &a); };

    run_in_threads(compare_exchange_pointer_task, 2); // 创建2个线程并行执行
    EXPECT_EQ(ptr, &b); // 期望ptr等于b
}

TEST_F(InterlockedTest, TestInterlockedCompareExchange64) {
    volatile std::int64_t value = 10;
    auto compare_exchange64_task = [&value]() { interlocked_compare_exchange64(&value, 20, 10); };

    run_in_threads(compare_exchange64_task, 2); // 创建2个线程并行执行
    EXPECT_EQ(value, 20); // 期望value等于20
}

TEST_F(InterlockedTest, TestInterlockedAnd) {
    volatile long value = 0b1100;
    auto and_task = [&value]() { interlocked_and(&value, 0b1010); };

    run_in_threads(and_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 0b1000); // 期望value等于0b1000
}

TEST_F(InterlockedTest, TestInterlockedOr) {
    volatile long value = 0b0100;
    auto or_task = [&value]() { interlocked_or(&value, 0b1010); };

    run_in_threads(or_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 0b1110); // 期望value等于0b1110
}

TEST_F(InterlockedTest, TestInterlockedXor) {
    volatile long value = 0b1100;
    auto xor_task = [&value]() { interlocked_xor(&value, 0b1010); };

    run_in_threads(xor_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 0b0110); // 期望value等于0b0110
}

TEST_F(InterlockedTest, TestIsoVolatileStore32) {
    volatile int value = 0;
    auto store_task = [&value]() { iso_volatile_store32(&value, 42); };

    run_in_threads(store_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 42); // 期望value等于42
}

TEST_F(InterlockedTest, TestIsoVolatileStore64) {
    volatile long long value = 0;
    auto store_task = [&value]() { iso_volatile_store64(&value, 42ull); };

    run_in_threads(store_task, 3); // 创建3个线程并行执行
    EXPECT_EQ(value, 42ull); // 期望value等于42
}

TEST_F(InterlockedTest, TestIsoMemoryFence) {
    auto fence_task = []() { iso_memory_fence(); };

    run_in_threads(fence_task, 3); // 创建3个线程并行执行
    SUCCEED(); // 仅测试函数调用是否成功
}
