#include <catch2/catch_test_macros.hpp>

#include <rainy/foundation/concurrency/executor.hpp>

using namespace rainy::foundation::concurrency;

struct PooledExecutorFixture {
    pinned_actor_pool pool;
    executor ex;

    PooledExecutorFixture()
        : ex(pool) {}
};

SCENARIO_METHOD(PooledExecutorFixture,
                "executor basic submit/get",
                "[executor]") {
    GIVEN("a pooled executor") {
        WHEN("submitting simple values") {
            auto f  = ex.submit([] { return 42; });
            auto fb = ex.submit([] { return true; });
            auto fs = ex.submit([] { return std::string("hello"); });
            THEN("results are returned correctly") {
                REQUIRE(f.get() == 42);
                REQUIRE(fb.get());
                REQUIRE(fs.get() == "hello");
            }
        }
    }
}

SCENARIO_METHOD(PooledExecutorFixture, "executor submit with arguments", "[executor][bdd]") {
    GIVEN("an executor") {
        WHEN("submitting callable with parameters") {

            auto f = ex.submit([](int a, int b) { return a + b; }, 10, 20);

            auto f2 = ex.submit([](std::string a, std::string b) { return a + b; }, std::string("foo"), std::string("bar"));

            THEN("arguments are forwarded correctly") {
                REQUIRE(f.get() == 30);
                REQUIRE(f2.get() == "foobar");
            }
        }
    }
}

SCENARIO_METHOD(PooledExecutorFixture, "future then chaining", "[executor][then]") {
    GIVEN("an executor") {
        WHEN("chaining string transformations") {

            auto f = ex.submit([] { return std::string("rainy"); })
                         .then([](std::string s) { return s + "::foundation"; })
                         .then([](std::string s) { return s.size(); });

            THEN("chain result is correct") {
                REQUIRE(f.get() == std::size_t(17));
            }
        }

        WHEN("chaining numeric transformations") {

            auto f = ex.submit([] { return 1; }).then([](int x) { return x + 1; }).then([](int x) { return x * 3; }).then([](int x) {
                return x - 1;
            });

            THEN("all steps execute sequentially") {
                REQUIRE(f.get() == 5);
            }
        }
    }
}

SCENARIO_METHOD(PooledExecutorFixture, "catch_error recovers from exceptions", "[executor][error]") {
    GIVEN("an executor") {
        WHEN("upstream throws") {

            auto f =
                ex.submit([]() -> int { throw std::runtime_error("boom"); }).catch_error([](std::exception_ptr) -> int { return -1; });
            ex.wait_all();

            THEN("error is recovered") {
                REQUIRE(f.get() == -1);
            }
        }

        WHEN("no exception occurs") {

            auto f = ex.submit([] { return 99; }).catch_error([](std::exception_ptr) -> int { return -1; });
            ex.wait_all();

            THEN("value passes through") {
                REQUIRE(f.get() == 99);
            }
        }

        WHEN("catch_error is followed by then") {

            auto f = ex.submit([]() -> int { throw std::logic_error("err"); })
                         .catch_error([](std::exception_ptr) -> int { return 10; })
                         .then([](int x) { return x * 2; });
            ex.wait_all();

            THEN("chain continues with recovered value") {
                REQUIRE(f.get() == 20);
            }
        }
    }
}

SCENARIO_METHOD(PooledExecutorFixture ,"finally executes regardless of outcome", "[executor][finally]") {
    GIVEN("an executor and a counter") {
        std::atomic<int> counter{0};

        WHEN("execution succeeds") {

            auto f = ex.submit([] { return 7; }).finally([&] { counter.fetch_add(1); });

            THEN("result passes through and finally runs") {
                REQUIRE(f.get() == 7);
                REQUIRE(counter.load() == 1);
            }
        }

        WHEN("execution throws") {

            auto f = ex.submit([]() -> int { throw std::runtime_error("x"); }).finally([&] { counter.fetch_add(1); });

            THEN("exception propagates but finally still runs") {
                REQUIRE_THROWS(f.get());
                REQUIRE(counter.load() == 1);
            }
        }
    }
}

SCENARIO_METHOD(PooledExecutorFixture, "full async chain works end-to-end", "[executor][chain]") {
    GIVEN("an executor") {
        std::atomic<bool> finally_called{false};

        WHEN("running full pipeline") {

            auto f = ex.submit([] { return std::string("rainy"); })
                         .then([](std::string s) { return s + "::foundation"; })
                         .then([](std::string s) { return s.size(); })
                         .catch_error([](std::exception_ptr) -> std::size_t { return 0; })
                         .finally([&] { finally_called.store(true); });

            THEN("result and finally are correct") {
                REQUIRE(f.get() == std::size_t(17));
                REQUIRE(finally_called.load());
            }
        }
    }
}

// check
SCENARIO_METHOD(PooledExecutorFixture, "wait_all completes all submitted tasks", "[executor][bulk]") {
    GIVEN("an executor") {
        constexpr int N = 10000;
        std::atomic<int> done{0};

        WHEN("many tasks are submitted") {

            for (int i = 0; i < N; ++i)
                ex.submit([&] { done.fetch_add(1, std::memory_order_relaxed); });

            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

            ex.wait_all();

            THEN("all tasks finish within deadline") {
                REQUIRE(std::chrono::steady_clock::now() < deadline);
                REQUIRE(done.load() == N);
            }
        }
    }
}

// check
SCENARIO_METHOD(PooledExecutorFixture, "executor supports concurrent submit", "[executor][concurrency]") {
    GIVEN("an executor") {
        constexpr int PRODUCERS = 8;
        constexpr int TASKS_PER = 500;
        constexpr int TOTAL = PRODUCERS * TASKS_PER;

        std::atomic<int> counter{0};

        WHEN("multiple threads submit simultaneously") {

            std::vector<std::thread> producers;
            producers.reserve(PRODUCERS);

            for (int p = 0; p < PRODUCERS; ++p) {
                producers.emplace_back([&] {
                    for (int i = 0; i < TASKS_PER; ++i)
                        ex.submit([&] { counter.fetch_add(1, std::memory_order_relaxed); });
                });
            }

            for (auto &t: producers) {
                t.join();
            }
            ex.wait_all();

            THEN("every task runs exactly once") {
                REQUIRE(counter.load() == TOTAL);
            }
        }
    }
}

// check
SCENARIO("work stealing balances load", "[actor_pool][steal]") {
    GIVEN("a dedicated actor pool") {
        const auto pool = std::make_unique<dedicated_actor_pool>(4);
        constexpr int N = 2000;
        std::atomic<int> done{0};
        WHEN("all tasks target a single actor") {
            for (int i = 0; i < N; ++i) {
                pool->submit_to(0, [&] { done.fetch_add(1, std::memory_order_relaxed); });
            }
            pool->wait_all();
            THEN("all tasks complete") {
                REQUIRE(done.load() == N);
            }
        }
    }
}

// check
SCENARIO("same actor preserves submission order", "[actor_pool][ordering]") {

    GIVEN("a dedicated actor pool") {
        const auto pool = std::make_unique<dedicated_actor_pool>(4);

        constexpr int N = 10000;
        std::vector<int> order;
        std::mutex mtx;

        WHEN("tasks are submitted to same actor") {

            for (int i = 0; i < N; ++i) {
                int val = i;
                pool->submit_to(0, [&, val] {
                    std::lock_guard<std::mutex> lk(mtx);
                    order.push_back(val);
                });
            }

            pool->wait_all();

            THEN("execution order matches submission order") {
                REQUIRE(order.size() == N);
                REQUIRE(std::is_sorted(order.begin(), order.end()));
            }
        }
    }
}

struct PinnedActorPoolFixture {
    pinned_actor_pool pool;
    executor ex;

    PinnedActorPoolFixture(std::size_t threads = 2)
        : pool(threads), ex(pool) {}
};

// check
SCENARIO_METHOD(PinnedActorPoolFixture, "pinned_actor_pool basic submit/get", "[pinned_actor_pool]") {
    GIVEN("a pinned actor pool with executor") {
        WHEN("submitting simple tasks") {
            auto f1 = ex.submit([] { return 123; });
            auto f2 = ex.submit([] { return true; });
            auto f3 = ex.submit([] { return std::string("hello"); });

            THEN("results are returned correctly") {
                REQUIRE(f1.get() == 123);
                REQUIRE(f2.get());
                REQUIRE(f3.get() == "hello");
            }
        }
    }
}

// check
SCENARIO_METHOD(PinnedActorPoolFixture, "submit_to directs task to specific actor", "[pinned_actor_pool][submit_to]") {
    GIVEN("a pinned actor pool") {
        std::atomic<int> counter{0};

        WHEN("submitting tasks to a specific actor") {
            for (std::size_t i = 0; i < pool.thread_count(); ++i) {
                ex.submit([&, i] { counter.fetch_add(static_cast<int>(i + 1), std::memory_order_relaxed); });
            }

            pool.wait_all();

            THEN("all tasks complete") {
                REQUIRE(counter.load() == static_cast<int>((pool.thread_count() * (pool.thread_count() + 1)) / 2));
            }
        }

        WHEN("submitting tasks with submit_to out-of-bounds actor_id") {
            auto f = ex.submit([&] { return 99; });
            pool.submit_to(pool.thread_count() + 10, []{}); // should fallback to round-robin
            pool.wait_all();

            THEN("task executes normally") {
                REQUIRE(f.get() == 99);
            }
        }
    }
}

// check
SCENARIO_METHOD(PinnedActorPoolFixture, "order of tasks to same actor is preserved", "[pinned_actor_pool][ordering]") {
    GIVEN("a pinned actor pool") {
        constexpr int N = 1000;
        std::vector<int> order;
        std::mutex mtx;

        WHEN("tasks are submitted to same actor sequentially") {
            for (int i = 0; i < N; ++i) {
                ex.submit_to(0, [&, i] {
                    std::lock_guard<std::mutex> lk(mtx);
                    order.push_back(i);
                });
            }

            pool.wait_all();

            THEN("execution order matches submission order") {
                REQUIRE(order.size() == N);
                REQUIRE(std::is_sorted(order.begin(), order.end()));
            }
        }
    }
}

// fix this leak
SCENARIO_METHOD(PinnedActorPoolFixture, "wait_all completes bulk tasks", "[pinned_actor_pool][bulk]") {
    GIVEN("a pinned actor pool") {
        constexpr int N = 10000;
        std::atomic<int> done{0};

        WHEN("many tasks are submitted concurrently") {
            for (int i = 0; i < N; ++i) {
                ex.submit([&] { done.fetch_add(1, std::memory_order_relaxed); });
            }

            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

            pool.wait_all();

            THEN("all tasks finish within deadline") {
                REQUIRE(std::chrono::steady_clock::now() < deadline);
                REQUIRE(done.load() == N);
            }
        }
    }
}

// check
SCENARIO_METHOD(PinnedActorPoolFixture, "handles concurrent submissions correctly", "[pinned_actor_pool][concurrency]") {
    GIVEN("a pinned actor pool") {
        constexpr int PRODUCERS = 4;
        constexpr int TASKS_PER = 500;
        constexpr int TOTAL = PRODUCERS * TASKS_PER;

        std::atomic<int> counter{0};

        WHEN("multiple threads submit tasks simultaneously") {
            std::vector<std::thread> producers;
            producers.reserve(PRODUCERS);

            for (int p = 0; p < PRODUCERS; ++p) {
                producers.emplace_back([&] {
                    for (int i = 0; i < TASKS_PER; ++i) {
                        ex.submit([&] { counter.fetch_add(1, std::memory_order_relaxed); });
                    }
                });
            }

            for (auto &t : producers) t.join();
            pool.wait_all();

            THEN("all tasks are executed exactly once") {
                REQUIRE(counter.load() == TOTAL);
            }
        }
    }
}

struct PriorityActorPoolFixture {
    priority_actor_pool pool;
    executor ex;

    PriorityActorPoolFixture(std::size_t threads = 4)
        : pool(threads), ex(pool) {}
};

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "priority_actor_pool basic submit/get",
                "[priority_actor_pool][basic]") {

    GIVEN("a priority actor pool") {
        WHEN("submitting simple tasks") {

            auto f1 = ex.submit([] { return 42; });
            auto f2 = ex.submit([] { return std::string("rainy"); });

            THEN("results are returned correctly") {
                REQUIRE(f1.get() == 42);
                REQUIRE(f2.get() == "rainy");
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "default submit uses normal priority",
                "[priority_actor_pool][priority]") {

    GIVEN("a priority pool") {
        std::atomic<int> counter{0};

        WHEN("tasks are submitted without explicit priority") {

            for (int i = 0; i < 1000; ++i)
                ex.submit([&] { counter.fetch_add(1); });

            pool.wait_all();

            THEN("all tasks execute normally") {
                REQUIRE(counter.load() == 1000);
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "high priority tasks execute before backlog",
                "[priority_actor_pool][priority][ordering]") {

    GIVEN("a priority pool with backlog") {

        std::atomic<int> low_started{0};
        std::atomic<bool> high_done{false};

        WHEN("low tasks are queued before high task") {

            for (int i = 0; i < 2000; ++i) {
                pool.submit(
                    [&] {
                        low_started.fetch_add(1, std::memory_order_relaxed);
                    },
                    actor_priority::low);
            }

            auto f = ex.submit(actor_priority::high, [&] {
                high_done.store(true, std::memory_order_release);
                return 1;
            });

            REQUIRE(f.get() == 1);

            THEN("high priority runs without waiting for all low tasks") {
                REQUIRE(high_done.load());
                REQUIRE(low_started.load() <= 2000);
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "low priority tasks are not starved",
                "[priority_actor_pool][fairness]") {

    GIVEN("continuous high priority workload") {

        constexpr int LOW_TASKS = 200;
        std::atomic<int> low_done{0};

        WHEN("low priority tasks exist alongside high priority tasks") {

            for (int i = 0; i < LOW_TASKS; ++i) {
                pool.submit(
                    [&] { low_done.fetch_add(1); },
                    actor_priority::low);
            }

            for (int i = 0; i < 2000; ++i) {
                pool.submit([] {}, actor_priority::high);
            }

            pool.wait_all();

            THEN("low tasks eventually execute") {
                REQUIRE(low_done.load() == LOW_TASKS);
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "work stealing occurs within same tier",
                "[priority_actor_pool][steal]") {

    GIVEN("a priority pool") {

        constexpr int N = 3000;
        std::atomic<int> done{0};

        WHEN("all tasks target one actor in normal tier") {

            for (int i = 0; i < N; ++i) {
                pool.submit_to(
                    0,
                    [&] { done.fetch_add(1, std::memory_order_relaxed); },
                    actor_priority::normal);
            }

            pool.wait_all();

            THEN("tasks complete via intra-tier stealing") {
                REQUIRE(done.load() == N);
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "submit_to out-of-range falls back to routing",
                "[priority_actor_pool][submit_to]") {

    GIVEN("a priority pool") {

        std::atomic<int> counter{0};

        WHEN("actor_id exceeds tier size") {

            pool.submit_to(
                pool.actors_per_tier() + 10,
                [&] { counter.fetch_add(1); },
                actor_priority::high);

            pool.wait_all();

            THEN("task still executes") {
                REQUIRE(counter.load() == 1);
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "executor chaining works with priority pool",
                "[priority_actor_pool][executor][chain]") {

    GIVEN("an executor on priority pool") {

        WHEN("running chained async computation") {

            auto f =
                ex.submit(actor_priority::high, [] { return 5; })
                  .then([](int x) { return x * 2; })
                  .then([](int x) { return x + 3; });

            THEN("chain produces correct result") {
                REQUIRE(f.get() == 13);
            }
        }
    }
}

// check
SCENARIO_METHOD(PriorityActorPoolFixture,
                "supports concurrent submissions",
                "[priority_actor_pool][concurrency]") {

    GIVEN("a priority pool") {

        constexpr int PRODUCERS = 6;
        constexpr int TASKS_PER = 400;
        constexpr int TOTAL = PRODUCERS * TASKS_PER;

        std::atomic<int> counter{0};

        WHEN("multiple threads submit simultaneously") {

            std::vector<std::thread> producers;

            for (int p = 0; p < PRODUCERS; ++p) {
                producers.emplace_back([&] {
                    for (int i = 0; i < TASKS_PER; ++i) {
                        ex.submit([&] {
                            counter.fetch_add(1, std::memory_order_relaxed);
                        });
                    }
                });
            }

            for (auto &t : producers) {
                t.join();
            }

            pool.wait_all();

            THEN("all tasks execute exactly once") {
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

SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool basic submit and wait", "[blocking_actor_pool][basic]") {
    GIVEN("a blocking actor pool with default configuration") {
        WHEN("submitting non-blocking simple tasks") {
            auto f1 = ex.submit([] { return 100; });
            auto f2 = ex.submit([] { return std::string("blocking test"); });

            THEN("results are returned correctly") {
                REQUIRE(f1.get() == 100);
                REQUIRE(f2.get() == "blocking test");
            }
        }

        WHEN("submitting tasks that simulate blocking I/O") {
            auto start = std::chrono::steady_clock::now();

            auto f1 = ex.submit([] {
                simulate_blocking_work(std::chrono::milliseconds(100));
                return 42;
            });

            auto f2 = ex.submit([] {
                simulate_blocking_work(std::chrono::milliseconds(50));
                return 84;
            });

            THEN("tasks complete with correct results") {
                REQUIRE(f1.get() == 42);
                REQUIRE(f2.get() == 84);

                auto end = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                REQUIRE(elapsed.count() < 150);
            }
        }

        WHEN("submitting a task that returns void") {
            std::atomic<bool> executed{false};

            auto fut = ex.submit([&executed] {
                simulate_blocking_work(std::chrono::milliseconds(10));
                executed = true;
            });

            THEN("void future works correctly") {
                REQUIRE_NOTHROW(fut.get());
                REQUIRE(executed.load() == true);
            }
        }
    }
}

SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool wait_all semantics", "[blocking_actor_pool][synchronization]") {
    GIVEN("a blocking actor pool with multiple tasks") {
        std::atomic<int> counter{0};

        WHEN("submitting tasks and calling wait_all") {
            for (int i = 0; i < 5; ++i) {
                ex.submit([&counter] {
                    simulate_blocking_work(std::chrono::milliseconds(20));
                    counter.fetch_add(1, std::memory_order_relaxed);
                });
            }

            pool.wait_all();

            THEN("all tasks complete before wait_all returns") {
                REQUIRE(counter.load() == 5);
            }
        }

        WHEN("wait_all is called with no pending tasks") {
            THEN("it returns immediately") {
                REQUIRE_NOTHROW(pool.wait_all());
            }
        }
    }
}

// 只验证扩容，不验证缩容
SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool dynamic thread scaling", "[blocking_actor_pool][scaling]") {
    GIVEN("a blocking actor pool with base_threads=2, max_threads=8") {

        WHEN("submitting many blocking tasks") {
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

            THEN("thread count increases to handle the load") {
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

SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool traits reporting", "[blocking_actor_pool][traits]") {
    GIVEN("a blocking actor pool") {
        auto traits = pool.traits();

        THEN("traits correctly identify the pool characteristics") {
            REQUIRE(traits.is_multi_threaded == true);
            REQUIRE(traits.is_ordered == false);
            REQUIRE(traits.supports_stealing == false);
            REQUIRE(traits.supports_affinity == false);
            REQUIRE(traits.mode == actor_pool_mode::blocking);
        }

        WHEN("submitting tasks increases active threads") {
            auto before = pool.traits().concurrency;

            std::vector<monad_future<void>> futures;
            for (int i = 0; i < 6; ++i) {
                futures.push_back(ex.submit([] { simulate_blocking_work(std::chrono::milliseconds(300)); }));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            THEN("traits.concurrency reflects current active threads") {
                auto after = pool.traits().concurrency;
                REQUIRE(after > before);
            }

            for (auto &f: futures) {
                f.get();
            }
        }
    }
}

SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool concurrent task execution", "[blocking_actor_pool][concurrency]") {
    GIVEN("a blocking actor pool") {
        std::atomic<int> concurrent_execution{0};
        std::atomic<int> max_concurrent{0};

        WHEN("submitting tasks that track concurrency") {
            std::vector<monad_future<void>> futures;

            for (int i = 0; i < 8; ++i) {
                futures.push_back(ex.submit([&concurrent_execution, &max_concurrent] {
                    int current = concurrent_execution.fetch_add(1, std::memory_order_acq_rel) + 1;

                    int observed_max = max_concurrent.load(std::memory_order_relaxed);
                    while (current > observed_max) {
                        max_concurrent.compare_exchange_weak(observed_max, current, std::memory_order_acq_rel);
                    }

                    simulate_blocking_work(std::chrono::milliseconds(100));

                    concurrent_execution.fetch_sub(1, std::memory_order_acq_rel);
                }));
            }

            pool.wait_all();

            THEN("multiple tasks execute concurrently") {
                REQUIRE(max_concurrent.load() > 2);
                INFO("Max concurrent executions: " << max_concurrent.load());
            }
        }
    }
}

SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool exception handling", "[blocking_actor_pool][exceptions]") {
    GIVEN("a blocking actor pool") {
        WHEN("a task throws an exception") {
            auto fut = ex.submit([]() -> int {
                throw std::runtime_error("blocking task error");
                return 42;
            });

            THEN("exception is propagated through future") {
                REQUIRE_THROWS_AS(fut.get(), std::runtime_error);
            }
        }

        WHEN("multiple tasks throw different exceptions") {
            auto fut1 = ex.submit([]() -> int { throw std::logic_error("logic error"); });
            auto fut2 = ex.submit([]() -> int { throw std::runtime_error("runtime error"); });

            THEN("each exception is properly propagated") {
                REQUIRE_THROWS_AS(fut1.get(), std::logic_error);
                REQUIRE_THROWS_AS(fut2.get(), std::runtime_error);
            }
        }
    }
}

SCENARIO_METHOD(BlockingActorPoolFixture, "blocking_actor_pool task ordering semantics", "[blocking_actor_pool][ordering]") {
    GIVEN("a blocking actor pool") {
        std::vector<int> execution_order;
        std::mutex order_mutex;

        WHEN("submitting tasks in sequence") {
            std::vector<monad_future<void>> futures;

            for (int i = 0; i < 10; ++i) {
                futures.push_back(ex.submit([i, &execution_order, &order_mutex] {
                    simulate_blocking_work(std::chrono::milliseconds(rand() % 50));
                    std::lock_guard<std::mutex> lock(order_mutex);
                    execution_order.push_back(i);
                }));
            }

            pool.wait_all();

            THEN("tasks may not execute in submission order") {
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

SCENARIO("async and async_isolated execute functions correctly", "[async]") {

    GIVEN("A simple function returning an integer") {
        auto simple_func = []() { return 42; };

        WHEN("async is called with the function") {
            auto fut = async(simple_func);

            THEN("the future should hold the correct result") {
                REQUIRE(fut.get() == 42);
            }
        }

        WHEN("async_isolated is called with the function") {
            auto fut = async_isolated(simple_func);

            THEN("the future should hold the correct result") {
                REQUIRE(fut.get() == 42);
            }
        }
    }

    GIVEN("A function with parameters") {
        auto add = [](int a, int b) { return a + b; };

        WHEN("async is called with parameters") {
            auto fut = async(add, 2, 3);

            THEN("the future should compute the correct sum") {
                REQUIRE(fut.get() == 5);
            }
        }

        WHEN("async_isolated is called with parameters") {
            auto fut = async_isolated(add, 10, 5);

            THEN("the future should compute the correct sum") {
                REQUIRE(fut.get() == 15);
            }
        }
    }

    GIVEN("A function that modifies external state") {
        int value = 0;
        auto increment = [&value]() { value += 1; };

        WHEN("async is called") {
            auto fut = async(increment);
            fut.get();

            THEN("the external state should be updated") {
                REQUIRE(value == 1);
            }
        }

        WHEN("async_isolated is called") {
            auto fut = async_isolated(increment);
            fut.get();

            THEN("the external state should be updated") {
                REQUIRE(value == 1);
            }
        }
    }
}
