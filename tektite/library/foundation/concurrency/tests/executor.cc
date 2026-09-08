#include <catch2/catch_test_macros.hpp>

#include <rainy/foundation/concurrency/executor.hpp>

using namespace rainy::foundation::concurrency;
using rainy::core::concurrency::atomic;

struct PooledExecutorFixture {
    pinned_actor_pool pool;
    executor ex;

    PooledExecutorFixture() : ex(pool) {
    }
};

TEST_CASE_METHOD(PooledExecutorFixture, "executor basic submit/get", "[executor]") {
    SECTION("a pooled executor") {
        SECTION("submitting simple values") {
            auto f = ex.submit([] { return 42; });
            auto fb = ex.submit([] { return true; });
            auto fs = ex.submit([] { return std::string("hello"); });
            SECTION("results are returned correctly") {
                REQUIRE(f.get() == 42);
                REQUIRE(fb.get());
                REQUIRE(fs.get() == "hello");
            }
        }
    }
}

TEST_CASE_METHOD(PooledExecutorFixture, "executor submit with arguments", "[executor]") {
    SECTION("an executor") {
        SECTION("submitting callable with parameters") {

            auto f = ex.submit([](int a, int b) { return a + b; }, 10, 20);

            auto f2 = ex.submit([](std::string a, std::string b) { return a + b; }, std::string("foo"), std::string("bar"));

            SECTION("arguments are forwarded correctly") {
                REQUIRE(f.get() == 30);
                REQUIRE(f2.get() == "foobar");
            }
        }
    }
}

TEST_CASE_METHOD(PooledExecutorFixture, "future then chaining", "[executor][then]") {
    SECTION("an executor") {
        SECTION("chaining string transformations") {

            auto f = ex.submit([] { return std::string("rainy"); })
                         .then([](std::string s) { return s + "::foundation"; })
                         .then([](std::string s) { return s.size(); });

            SECTION("chain result is correct") {
                REQUIRE(f.get() == std::size_t(17));
            }
        }

        SECTION("chaining numeric transformations") {

            auto f = ex.submit([] { return 1; }).then([](int x) { return x + 1; }).then([](int x) { return x * 3; }).then([](int x) {
                return x - 1;
            });

            SECTION("all steps execute sequentially") {
                REQUIRE(f.get() == 5);
            }
        }
    }
}

TEST_CASE_METHOD(PooledExecutorFixture, "catch_error recovers from exceptions", "[executor][error]") {
    SECTION("an executor") {
        SECTION("upstream throws") {

            auto f =
                ex.submit([]() -> int { throw std::runtime_error("boom"); }).catch_error([](std::exception_ptr) -> int { return -1; });
            ex.wait_all();

            SECTION("error is recovered") {
                REQUIRE(f.get() == -1);
            }
        }

        SECTION("no exception occurs") {

            auto f = ex.submit([] { return 99; }).catch_error([](std::exception_ptr) -> int { return -1; });
            ex.wait_all();

            SECTION("value passes through") {
                REQUIRE(f.get() == 99);
            }
        }

        SECTION("catch_error is followed by then") {

            auto f = ex.submit([]() -> int { throw std::logic_error("err"); })
                         .catch_error([](std::exception_ptr) -> int { return 10; })
                         .then([](int x) { return x * 2; });
            ex.wait_all();

            SECTION("chain continues with recovered value") {
                REQUIRE(f.get() == 20);
            }
        }
    }
}

TEST_CASE_METHOD(PooledExecutorFixture, "finally executes regardless of outcome", "[executor][finally]") {
    SECTION("an executor and a counter") {
        atomic<int> counter{0};

        SECTION("execution succeeds") {

            auto f = ex.submit([] { return 7; }).finally([&] { counter.fetch_add(1); });

            SECTION("result passes through and finally runs") {
                REQUIRE(f.get() == 7);
                REQUIRE(counter.load() == 1);
            }
        }

        SECTION("execution throws") {

            auto f = ex.submit([]() -> int { throw std::runtime_error("x"); }).finally([&] { counter.fetch_add(1); });

            SECTION("exception propagates but finally still runs") {
                REQUIRE_THROWS(f.get());
                REQUIRE(counter.load() == 1);
            }
        }
    }
}

TEST_CASE_METHOD(PooledExecutorFixture, "full async chain works end-to-end", "[executor][chain]") {
    SECTION("an executor") {
        atomic<bool> finally_called{false};

        SECTION("running full pipeline") {

            auto f = ex.submit([] { return std::string("rainy"); })
                         .then([](std::string s) { return s + "::foundation"; })
                         .then([](std::string s) { return s.size(); })
                         .catch_error([](std::exception_ptr) -> std::size_t { return 0; })
                         .finally([&] { finally_called.store(true); });

            SECTION("result and finally are correct") {
                REQUIRE(f.get() == std::size_t(17));
                REQUIRE(finally_called.load());
            }
        }
    }
}


TEST_CASE_METHOD(PooledExecutorFixture, "wait_all completes all submitted tasks", "[executor][bulk]") {
    SECTION("an executor") {
        constexpr int N = 10000;
        atomic<int> done{0};

        SECTION("many tasks are submitted") {

            for (int i = 0; i < N; ++i) {
                ex.submit([&] { done.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
            }

            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

            ex.wait_all();

            SECTION("all tasks finish within deadline") {
                REQUIRE(std::chrono::steady_clock::now() < deadline);
                REQUIRE(done.load() == N);
            }
        }
    }
}


TEST_CASE_METHOD(PooledExecutorFixture, "executor supports concurrent submit", "[executor][concurrency]") {
    SECTION("an executor") {
        constexpr int PRODUCERS = 8;
        constexpr int TASKS_PER = 500;
        constexpr int TOTAL = PRODUCERS * TASKS_PER;

        atomic<int> counter{0};

        SECTION("multiple threads submit simultaneously") {

            std::vector<std::thread> producers;
            producers.reserve(PRODUCERS);

            for (int p = 0; p < PRODUCERS; ++p) {
                producers.emplace_back([&] {
                    for (int i = 0; i < TASKS_PER; ++i) {
                        ex.submit([&] { counter.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
                    }
                });
            }

            for (auto &t: producers) {
                t.join();
            }
            ex.wait_all();

            SECTION("every task runs exactly once") {
                REQUIRE(counter.load() == TOTAL);
            }
        }
    }
}


TEST_CASE("work stealing balances load", "[actor_pool][steal]") {
    SECTION("a dedicated actor pool") {
        const auto pool = std::make_unique<dedicated_actor_pool>(4);
        constexpr int N = 2000;
        atomic<int> done{0};
        SECTION("all tasks target a single actor") {
            for (int i = 0; i < N; ++i) {
                pool->submit_to(0, [&] { done.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
            }
            pool->wait_all();
            SECTION("all tasks complete") {
                REQUIRE(done.load() == N);
            }
        }
    }
}


TEST_CASE("same actor preserves submission order", "[actor_pool][ordering]") {

    SECTION("a dedicated actor pool") {
        const auto pool = std::make_unique<dedicated_actor_pool>(4);

        constexpr int N = 10000;
        std::vector<int> order;
        std::mutex mtx;

        SECTION("tasks are submitted to same actor") {

            for (int i = 0; i < N; ++i) {
                int val = i;
                pool->submit_to(0, [&, val] {
                    std::lock_guard<std::mutex> lk(mtx);
                    order.push_back(val);
                });
            }

            pool->wait_all();

            SECTION("execution order matches submission order") {
                REQUIRE(order.size() == N);
                REQUIRE(std::is_sorted(order.begin(), order.end()));
            }
        }
    }
}

struct PinnedActorPoolFixture {
    pinned_actor_pool pool;
    executor ex;

    PinnedActorPoolFixture(std::size_t threads = 2) : pool(threads), ex(pool) {
    }
};


TEST_CASE_METHOD(PinnedActorPoolFixture, "pinned_actor_pool basic submit/get", "[pinned_actor_pool]") {
    SECTION("a pinned actor pool with executor") {
        SECTION("submitting simple tasks") {
            auto f1 = ex.submit([] { return 123; });
            auto f2 = ex.submit([] { return true; });
            auto f3 = ex.submit([] { return std::string("hello"); });

            SECTION("results are returned correctly") {
                REQUIRE(f1.get() == 123);
                REQUIRE(f2.get());
                REQUIRE(f3.get() == "hello");
            }
        }
    }
}


TEST_CASE_METHOD(PinnedActorPoolFixture, "submit_to directs task to specific actor", "[pinned_actor_pool][submit_to]") {
    SECTION("a pinned actor pool") {
        atomic<int> counter{0};

        SECTION("submitting tasks to a specific actor") {
            for (std::size_t i = 0; i < pool.thread_count(); ++i) {
                ex.submit([&, i] { counter.fetch_add(static_cast<int>(i + 1), rainy::core::layer::memory_order_relaxed); });
            }

            pool.wait_all();

            SECTION("all tasks complete") {
                REQUIRE(counter.load() == static_cast<int>((pool.thread_count() * (pool.thread_count() + 1)) / 2));
            }
        }

        SECTION("submitting tasks with submit_to out-of-bounds actor_id") {
            auto f = ex.submit([&] { return 99; });
            pool.submit_to(pool.thread_count() + 10, [] {}); // should fallback to round-robin
            pool.wait_all();

            SECTION("task executes normally") {
                REQUIRE(f.get() == 99);
            }
        }
    }
}


TEST_CASE_METHOD(PinnedActorPoolFixture, "order of tasks to same actor is preserved", "[pinned_actor_pool][ordering]") {
    SECTION("a pinned actor pool") {
        constexpr int N = 1000;
        std::vector<int> order;
        std::mutex mtx;

        SECTION("tasks are submitted to same actor sequentially") {
            for (int i = 0; i < N; ++i) {
                ex.submit_to(0, [&, i] {
                    std::lock_guard<std::mutex> lk(mtx);
                    order.push_back(i);
                });
            }

            pool.wait_all();

            SECTION("execution order matches submission order") {
                REQUIRE(order.size() == N);
                REQUIRE(std::is_sorted(order.begin(), order.end()));
            }
        }
    }
}

TEST_CASE_METHOD(PinnedActorPoolFixture, "wait_all completes bulk tasks", "[pinned_actor_pool][bulk]") {
    SECTION("a pinned actor pool") {
        constexpr int N = 10000;
        atomic<int> done{0};

        SECTION("many tasks are submitted concurrently") {
            for (int i = 0; i < N; ++i) {
                ex.submit([&] { done.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
            }

            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

            pool.wait_all();

            SECTION("all tasks finish within deadline") {
                REQUIRE(std::chrono::steady_clock::now() < deadline);
                REQUIRE(done.load() == N);
            }
        }
    }
}

TEST_CASE_METHOD(PinnedActorPoolFixture, "handles concurrent submissions correctly", "[pinned_actor_pool][concurrency]") {
    SECTION("a pinned actor pool") {
        constexpr int PRODUCERS = 4;
        constexpr int TASKS_PER = 500;
        constexpr int TOTAL = PRODUCERS * TASKS_PER;

        atomic<int> counter{0};

        SECTION("multiple threads submit tasks simultaneously") {
            std::vector<std::thread> producers;
            producers.reserve(PRODUCERS);

            for (int p = 0; p < PRODUCERS; ++p) {
                producers.emplace_back([&] {
                    for (int i = 0; i < TASKS_PER; ++i) {
                        ex.submit([&] { counter.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
                    }
                });
            }

            for (auto &t: producers) {
                t.join();
            }
            pool.wait_all();

            SECTION("all tasks are executed exactly once") {
                REQUIRE(counter.load() == TOTAL);
            }
        }
    }
}

struct PriorityActorPoolFixture {
    priority_actor_pool pool;
    executor ex;

    PriorityActorPoolFixture(std::size_t threads = 4) : pool(threads), ex(pool) {
    }
};

TEST_CASE_METHOD(PriorityActorPoolFixture, "priority_actor_pool basic submit/get", "[priority_actor_pool][basic]") {

    SECTION("a priority actor pool") {
        SECTION("submitting simple tasks") {

            auto f1 = ex.submit([] { return 42; });
            auto f2 = ex.submit([] { return std::string("rainy"); });

            SECTION("results are returned correctly") {
                REQUIRE(f1.get() == 42);
                REQUIRE(f2.get() == "rainy");
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "default submit uses normal priority", "[priority_actor_pool][priority]") {

    SECTION("a priority pool") {
        atomic<int> counter{0};

        SECTION("tasks are submitted without explicit priority") {

            for (int i = 0; i < 1000; ++i) {
                ex.submit([&] { counter.fetch_add(1); });
            }

            pool.wait_all();

            SECTION("all tasks execute normally") {
                REQUIRE(counter.load() == 1000);
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "high priority tasks execute before backlog", "[priority_actor_pool][priority][ordering]") {

    SECTION("a priority pool with backlog") {

        atomic<int> low_started{0};
        atomic<bool> high_done{false};

        SECTION("low tasks are queued before high task") {

            for (int i = 0; i < 2000; ++i) {
                pool.submit([&] { low_started.fetch_add(1, rainy::core::layer::memory_order_relaxed); }, actor_priority::low);
            }

            auto f = ex.submit(actor_priority::high, [&] {
                high_done.store(true, rainy::core::layer::memory_order_release);
                return 1;
            });

            REQUIRE(f.get() == 1);

            SECTION("high priority runs without waiting for all low tasks") {
                REQUIRE(high_done.load());
                REQUIRE(low_started.load() <= 2000);
                pool.wait_all(); // 在断言后、作用域结束前，等所有 low task 跑完
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "low priority tasks are not starved", "[priority_actor_pool][fairness]") {
    SECTION("continuous high priority workload") {

        constexpr int LOW_TASKS = 200;
        atomic<int> low_done{0};

        SECTION("low priority tasks exist alongside high priority tasks") {

            for (int i = 0; i < LOW_TASKS; ++i) {
                pool.submit([&] { low_done.fetch_add(1); }, actor_priority::low);
            }

            for (int i = 0; i < 2000; ++i) {
                pool.submit([] {}, actor_priority::high);
            }

            pool.wait_all();

            SECTION("low tasks eventually execute") {
                REQUIRE(low_done.load() == LOW_TASKS);
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "work stealing occurs within same tier", "[priority_actor_pool][steal]") {

    SECTION("a priority pool") {

        constexpr int N = 3000;
        atomic<int> done{0};

        SECTION("all tasks target one actor in normal tier") {

            for (int i = 0; i < N; ++i) {
                pool.submit_to(0, [&] { done.fetch_add(1, rainy::core::layer::memory_order_relaxed); }, actor_priority::normal);
            }

            pool.wait_all();

            SECTION("tasks complete via intra-tier stealing") {
                REQUIRE(done.load() == N);
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "submit_to out-of-range falls back to routing", "[priority_actor_pool][submit_to]") {

    SECTION("a priority pool") {

        atomic<int> counter{0};

        SECTION("actor_id exceeds tier size") {

            pool.submit_to(pool.actors_per_tier() + 10, [&] { counter.fetch_add(1); }, actor_priority::high);

            pool.wait_all();

            SECTION("task still executes") {
                REQUIRE(counter.load() == 1);
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "executor chaining works with priority pool", "[priority_actor_pool][executor][chain]") {

    SECTION("an executor on priority pool") {

        SECTION("running chained async computation") {

            auto f =
                ex.submit(actor_priority::high, [] { return 5; }).then([](int x) { return x * 2; }).then([](int x) { return x + 3; });

            SECTION("chain produces correct result") {
                REQUIRE(f.get() == 13);
            }
        }
    }
}

TEST_CASE_METHOD(PriorityActorPoolFixture, "supports concurrent submissions", "[priority_actor_pool][concurrency]") {
    SECTION("a priority pool") {
        constexpr int PRODUCERS = 6;
        constexpr int TASKS_PER = 400;
        constexpr int TOTAL = PRODUCERS * TASKS_PER;

        atomic<int> counter{0};

        SECTION("multiple threads submit simultaneously") {

            std::vector<std::thread> producers;

            for (int p = 0; p < PRODUCERS; ++p) {
                producers.emplace_back([&] {
                    for (int i = 0; i < TASKS_PER; ++i) {
                        ex.submit([&] { counter.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
                    }
                });
            }

            for (auto &t: producers) {
                t.join();
            }

            pool.wait_all();

            SECTION("all tasks execute exactly once") {
                REQUIRE(counter.load() == TOTAL);
            }
        }
    }
}

struct BlockingActorPoolFixture {
    blocking_actor_pool pool;
    executor ex;

    BlockingActorPoolFixture(std::size_t base_threads = 2, std::size_t max_threads = 8) : pool(base_threads, max_threads), ex(pool) {
    }
};

void simulate_blocking_work(std::chrono::milliseconds duration) {
    std::this_thread::sleep_for(duration);
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool basic submit and wait", "[blocking_actor_pool][basic]") {
    SECTION("a blocking actor pool with default configuration") {
        SECTION("submitting non-blocking simple tasks") {
            auto f1 = ex.submit([] { return 100; });
            auto f2 = ex.submit([] { return std::string("blocking test"); });

            SECTION("results are returned correctly") {
                REQUIRE(f1.get() == 100);
                REQUIRE(f2.get() == "blocking test");
            }
        }

        SECTION("submitting tasks that simulate blocking I/O") {
            auto start = std::chrono::steady_clock::now();

            auto f1 = ex.submit([] {
                simulate_blocking_work(std::chrono::milliseconds(100));
                return 42;
            });

            auto f2 = ex.submit([] {
                simulate_blocking_work(std::chrono::milliseconds(50));
                return 84;
            });

            SECTION("tasks complete with correct results") {
                REQUIRE(f1.get() == 42);
                REQUIRE(f2.get() == 84);

                auto end = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                REQUIRE(elapsed.count() < 300);
            }
        }

        SECTION("submitting a task that returns void") {
            atomic<bool> executed{false};

            auto fut = ex.submit([&executed] {
                simulate_blocking_work(std::chrono::milliseconds(10));
                executed = true;
            });

            SECTION("void future works correctly") {
                REQUIRE_NOTHROW(fut.get());
                REQUIRE(executed.load() == true);
            }
        }
    }
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool wait_all semantics", "[blocking_actor_pool][synchronization]") {
    SECTION("a blocking actor pool with multiple tasks") {
        atomic<int> counter{0};

        SECTION("submitting tasks and calling wait_all") {
            for (int i = 0; i < 5; ++i) {
                ex.submit([&counter] {
                    simulate_blocking_work(std::chrono::milliseconds(20));
                    counter.fetch_add(1, rainy::core::layer::memory_order_relaxed);
                });
            }

            pool.wait_all();

            SECTION("all tasks complete before wait_all returns") {
                REQUIRE(counter.load() == 5);
            }
        }

        SECTION("wait_all is called with no pending tasks") {
            SECTION("it returns immediately") {
                REQUIRE_NOTHROW(pool.wait_all());
            }
        }
    }
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool dynamic thread scaling", "[blocking_actor_pool][scaling]") {
    SECTION("a blocking actor pool with base_threads=2, max_threads=8") {

        SECTION("submitting many blocking tasks") {
            std::vector<monad_future<int>> futures;
            auto initial_traits = pool.traits();

            REQUIRE(initial_traits.concurrency == 2);

            for (int i = 0; i < 8; ++i) {
                futures.push_back(ex.submit([i] {
                    simulate_blocking_work(std::chrono::milliseconds(200));
                    return i;
                }));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            SECTION("thread count increases to handle the load") {
                auto mid_traits = pool.traits();
                REQUIRE(mid_traits.concurrency > 2);
                REQUIRE(mid_traits.concurrency <= 8);
            }

            for (auto &f: futures) {
                f.get();
            }
        }
    }
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool traits reporting", "[blocking_actor_pool][traits]") {
    SECTION("a blocking actor pool") {
        auto traits = pool.traits();

        SECTION("traits correctly identify the pool characteristics") {
            REQUIRE(traits.is_multi_threaded == true);
            REQUIRE(traits.is_ordered == false);
            REQUIRE(traits.supports_stealing == false);
            REQUIRE(traits.supports_affinity == false);
            REQUIRE(traits.mode == actor_pool_mode::blocking);
        }

        SECTION("submitting tasks increases active threads") {
            auto before = pool.traits().concurrency;

            std::vector<monad_future<void>> futures;
            for (int i = 0; i < 6; ++i) {
                futures.push_back(ex.submit([] { simulate_blocking_work(std::chrono::milliseconds(300)); }));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            SECTION("traits.concurrency reflects current active threads") {
                auto after = pool.traits().concurrency;
                REQUIRE(after > before);
            }

            for (auto &f: futures) {
                f.get();
            }
        }
    }
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool concurrent task execution", "[blocking_actor_pool][concurrency]") {
    SECTION("a blocking actor pool") {
        atomic<int> concurrent_execution{0};
        atomic<int> max_concurrent{0};

        SECTION("submitting tasks that track concurrency") {
            std::vector<monad_future<void>> futures;

            for (int i = 0; i < 8; ++i) {
                futures.push_back(ex.submit([&concurrent_execution, &max_concurrent] {
                    int current = concurrent_execution.fetch_add(1, rainy::core::layer::memory_order_acq_rel) + 1;

                    int observed_max = max_concurrent.load(rainy::core::layer::memory_order_relaxed);
                    while (current > observed_max) {
                        max_concurrent.compare_exchange_weak(observed_max, current, rainy::core::layer::memory_order_acq_rel);
                    }

                    simulate_blocking_work(std::chrono::milliseconds(100));

                    concurrent_execution.fetch_sub(1, rainy::core::layer::memory_order_acq_rel);
                }));
            }

            pool.wait_all();

            SECTION("multiple tasks execute concurrently") {
                REQUIRE(max_concurrent.load() > 2);
                INFO("Max concurrent executions: " << max_concurrent.load());
            }
        }
    }
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool exception handling", "[blocking_actor_pool][exceptions]") {
    SECTION("a blocking actor pool") {
        SECTION("a task throws an exception") {
            auto fut = ex.submit([]() -> int {
                throw std::runtime_error("blocking task error");
                return 42;
            });

            SECTION("exception is propagated through future") {
                REQUIRE_THROWS_AS(fut.get(), std::runtime_error);
            }
        }

        SECTION("multiple tasks throw different exceptions") {
            auto fut1 = ex.submit([]() -> int { throw std::logic_error("logic error"); });
            auto fut2 = ex.submit([]() -> int { throw std::runtime_error("runtime error"); });

            SECTION("each exception is properly propagated") {
                REQUIRE_THROWS_AS(fut1.get(), std::logic_error);
                REQUIRE_THROWS_AS(fut2.get(), std::runtime_error);
            }
        }
    }
}

TEST_CASE_METHOD(BlockingActorPoolFixture, "blocking_actor_pool task ordering semantics", "[blocking_actor_pool][ordering]") {
    SECTION("a blocking actor pool") {
        std::vector<int> execution_order;
        std::mutex order_mutex;

        SECTION("submitting tasks in sequence") {
            std::vector<monad_future<void>> futures;

            for (int i = 0; i < 10; ++i) {
                futures.push_back(ex.submit([i, &execution_order, &order_mutex] {
                    simulate_blocking_work(std::chrono::milliseconds(rand() % 50));
                    std::lock_guard<std::mutex> lock(order_mutex);
                    execution_order.push_back(i);
                }));
            }

            pool.wait_all();

            SECTION("tasks may not execute in submission order") {
                bool out_of_order = false;
                for (size_t i = 0; i < execution_order.size(); ++i) {
                    if (execution_order[i] != static_cast<int>(i)) {
                        out_of_order = true;
                        break;
                    }
                }
                REQUIRE(out_of_order == true);
                REQUIRE(execution_order.size() == 10);
            }
        }
    }
}

TEST_CASE("async and async_isolated execute functions correctly", "[async]") {

    SECTION("A simple function returning an integer") {
        auto simple_func = []() { return 42; };

        SECTION("async is called with the function") {
            auto fut = async(simple_func);

            SECTION("the future should hold the correct result") {
                REQUIRE(fut.get() == 42);
            }
        }

        SECTION("async_isolated is called with the function") {
            auto fut = async_isolated(simple_func);

            SECTION("the future should hold the correct result") {
                REQUIRE(fut.get() == 42);
            }
        }
    }

    SECTION("A function with parameters") {
        auto add = [](int a, int b) { return a + b; };

        SECTION("async is called with parameters") {
            auto fut = async(add, 2, 3);

            SECTION("the future should compute the correct sum") {
                REQUIRE(fut.get() == 5);
            }
        }

        SECTION("async_isolated is called with parameters") {
            auto fut = async_isolated(add, 10, 5);

            SECTION("the future should compute the correct sum") {
                REQUIRE(fut.get() == 15);
            }
        }
    }

    SECTION("A function that modifies external state") {
        int value = 0;
        auto increment = [&value]() { value += 1; };

        SECTION("async is called") {
            auto fut = async(increment);
            fut.get();

            SECTION("the external state should be updated") {
                REQUIRE(value == 1);
            }
        }

        SECTION("async_isolated is called") {
            auto fut = async_isolated(increment);
            fut.get();

            SECTION("the external state should be updated") {
                REQUIRE(value == 1);
            }
        }
    }
}
