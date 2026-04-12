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
#include <atomic>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <thread>
#include <future>

using namespace rainy::foundation;
using namespace rainy::foundation::concurrency;

template <typename LockType>
void test_basic_lifecycle() {
    LockType m;

    REQUIRE(m.native_handle() != nullptr);
}

template <typename LockType>
void test_basic_lock_unlock() {
    LockType m;
    m.lock();
    m.unlock();
    SUCCEED();
}

template <typename LockType>
void test_try_lock_behavior() {
    LockType m;
    const bool first = m.try_lock();
    REQUIRE(first == true);

    const bool second = m.try_lock();
    if constexpr (std::is_same_v<LockType, recursive_mutex> || std::is_same_v<LockType, recursive_timed_mutex>) {
        // 可递归锁允许第二次成功
        REQUIRE(second == true);
    } else {
        // 普通锁第二次应失败
        REQUIRE(second == false);
    }
    m.unlock();
    if (first && second) {
        m.unlock();
    }
}

template <typename LockType>
void test_multithread_mutex() {
    LockType m;
    std::atomic<int> counter{0};
    constexpr int iterations = 3000;

    auto worker = [&]() {
        for (int i = 0; i < iterations; ++i) {
            m.lock();
            ++counter;
            m.unlock();
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);
    std::thread t3(worker);
    std::thread t4(worker);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    REQUIRE(counter == iterations * 4);
}

#if RAINY_USING_MACOS
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

static void print_stacktrace() {
    void *bt[128];
    int n = backtrace(bt, 128);
    char **syms = backtrace_symbols(bt, n);
    fprintf(stderr, "\n=== stack trace (%d frames) ===\n", n);
    for (int i = 0; i < n; ++i) {
        // 尝试从符号里提取 mangled name 并 demangle
        // macOS backtrace_symbols 格式：
        // "0   libfoo.dylib  0x000000010 _ZNSt... + 42"
        char *begin = nullptr, *end = nullptr, *sym = syms[i];
        for (char *p = sym; *p; ++p) {
            if (*p == '_' && (p == sym || *(p-1) == ' ')) begin = p;
            else if (begin && *p == ' ')                 { end = p; break; }
        }
        if (begin && end) {
            *end = '\0';
            int status = 0;
            char *demangled = abi::__cxa_demangle(begin, nullptr, nullptr, &status);
            *end = ' ';
            if (status == 0 && demangled) {
                fprintf(stderr, "  [%2d] %s\n", i, demangled);
                free(demangled);
                continue;
            }
        }
        fprintf(stderr, "  [%2d] %s\n", i, sym);
    }
    free(syms);
    fprintf(stderr, "=== end of stack trace ===\n\n");
}

static void on_terminate() {
    fprintf(stderr, "\n*** std::terminate called ***\n");
    // 如果有当前异常，打印它
    if (auto eptr = std::current_exception()) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception &e) {
            fprintf(stderr, "  active exception: %s\n", e.what());
        } catch (...) {
            fprintf(stderr, "  active exception: (unknown type)\n");
        }
    } else {
        fprintf(stderr, "  no active exception (likely pure virtual call, "
                        "mutex destroyed while locked, or explicit abort)\n");
    }
    print_stacktrace();
    // 输出后让默认行为继续（生成 core dump）
    std::abort();
}

static void on_signal(int sig) {
    fprintf(stderr, "\n*** signal %d received ***\n", sig);
    print_stacktrace();
    signal(sig, SIG_DFL);
    raise(sig);
}
#endif

template <typename LockType>
void test_timed_lock() {
#if RAINY_USING_MACOS
    std::set_terminate(on_terminate);
    signal(SIGABRT, on_signal);
#endif
    if constexpr (std::is_same_v<LockType, timed_mutex> || std::is_same_v<LockType, recursive_timed_mutex>) {
        {
            LockType m;
            std::atomic<bool> started{false};
            std::promise<void> locked_promise;

            std::thread t([&]() {
                m.lock();
                started = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                m.unlock();
            });
            //  等线程真正持锁
            while (!started) {
                std::this_thread::yield();
            }
            // 短超时 → 必须失败
            const auto t0 = std::chrono::steady_clock::now();
            bool acquired = m.try_lock_for(std::chrono::milliseconds(50));
            const auto elapsed = std::chrono::steady_clock::now() - t0;
            REQUIRE(acquired == false);
            REQUIRE(elapsed >= std::chrono::milliseconds(50));
            acquired = m.try_lock_for(std::chrono::milliseconds(300));
            REQUIRE(acquired == true);
            t.join();
            m.unlock();
        }
        {
            LockType m;
            std::atomic<bool> t2_started{false};
            std::thread t2([&]() {
                m.lock();
                t2_started = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m.unlock();
            });
            while (!t2_started) {
                std::this_thread::yield();
            }
            auto tp = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
            bool acquired = m.try_lock_until(tp);
            REQUIRE(acquired == false);
            tp = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
            acquired = m.try_lock_until(tp);
            REQUIRE(acquired == true);
            t2.join();
            m.unlock();
        }
    }
}

TEMPLATE_TEST_CASE("Lock types unified BDD tests", "[lock][bdd]", mutex, recursive_mutex, timed_mutex, recursive_timed_mutex) {
    GIVEN("a lock instance") {
        WHEN("testing basic lifecycle") {
            test_basic_lifecycle<TestType>();
        }
        WHEN("testing basic lock/unlock") {
            test_basic_lock_unlock<TestType>();
        }
        WHEN("testing try_lock behavior") {
            test_try_lock_behavior<TestType>();
        }
        WHEN("testing multithreaded mutual exclusion") {
            test_multithread_mutex<TestType>();
        }
        WHEN("testing timed locks if supported") {
            test_timed_lock<TestType>();
        }
    }
}

SCENARIO("create_synchronized_task executes callable under mutex", "[create_synchronized_task][bdd]") {
    GIVEN("a mutex and shared counter") {
        mutex m;
        int counter = 0;
        WHEN("calling synchronized task") {
            auto result = synchronized_invoke(m, [&]() {
                ++counter;
                return 42;
            });
            THEN("callable executes and returns value") {
                REQUIRE(counter == 1);
                REQUIRE(result == 42);
            }
        }
    }
}

SCENARIO("create_synchronized_task provides mutual exclusion", "[create_synchronized_task][bdd][thread]") {
    GIVEN("a shared counter protected by synchronized task") {
        mutex m;
        int counter = 0;
        constexpr int iterations = 3000;
        auto worker = [&]() {
            for (int i = 0; i < iterations; ++i) {
                synchronized_invoke(m, [&]() { ++counter; });
            }
        };
        WHEN("multiple threads run synchronized tasks") {
            std::thread t1(worker);
            std::thread t2(worker);
            std::thread t3(worker);
            std::thread t4(worker);
            t1.join();
            t2.join();
            t3.join();
            t4.join();
            THEN("increments are not lost") {
                REQUIRE(counter == iterations * 4);
            }
        }
    }
}

SCENARIO("create_synchronized_task forwards arguments correctly", "[create_synchronized_task][bdd]") {
    GIVEN("a mutex") {
        mutex m;
        WHEN("passing arguments into callable") {
            auto sum = synchronized_invoke(m, [](int a, int b) { return a + b; }, 2, 3);
            THEN("arguments are forwarded") {
                REQUIRE(sum == 5);
            }
        }
    }
}

SCENARIO("call_once executes callable only once", "[call_once][bdd]") {
    GIVEN("a once_flag and counter") {
        once_flag flag{};
        int counter = 0;
        WHEN("call_once is invoked multiple times") {
            call_once(flag, [&]() { ++counter; });
            call_once(flag, [&]() { ++counter; });
            call_once(flag, [&]() { ++counter; });
            THEN("callable runs exactly once") {
                REQUIRE(counter == 1);
            }
        }
    }
}

SCENARIO("call_once is thread-safe", "[call_once][bdd][thread]") {
    GIVEN("a once_flag shared across threads") {
        once_flag flag{};
        std::atomic<int> counter{0};
        auto worker = [&]() { call_once(flag, [&]() { ++counter; }); };
        WHEN("many threads invoke call_once concurrently") {
            std::vector<std::thread> threads;
            for (int i = 0; i < 16; ++i) {
                threads.emplace_back(worker);
            }
            for (auto &t: threads) {
                t.join();
            }
            THEN("callable executes exactly once") {
                REQUIRE(counter == 1);
            }
        }
    }
}

SCENARIO("call_once retries if callable throws", "[call_once][bdd]") {
    GIVEN("a once_flag and throwing callable") {
        once_flag flag{};
        int attempts = 0;
        auto throwing = [&]() {
            ++attempts;
            if (attempts == 1)
                throw std::runtime_error("fail");
        };
        WHEN("first call throws and second retries") {
            REQUIRE_THROWS(call_once(flag, throwing));
            REQUIRE_NOTHROW(call_once(flag, throwing));
            THEN("callable executed twice total") {
                REQUIRE(attempts == 2);
            }
        }
    }
}
