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
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <numeric>

#include <rainy/foundation/io/net/io_context.hpp>

using namespace rainy::foundation::io::net;
using namespace std::chrono_literals;

namespace {
    /// RAII guard: 在后台线程跑 ctx.run()，析构时 stop + join
    struct background_runner {
        explicit background_runner(io_context &ctx)
            : ctx_(ctx)
            , thread_([this] { ctx_.run(); }) {
        }

        ~background_runner() {
            ctx_.stop();
            if (thread_.joinable()) {
                thread_.join();
            }
        }

        background_runner(const background_runner &) = delete;
        background_runner &operator=(const background_runner &) = delete;

    private:
        io_context  &ctx_;
        std::thread  thread_;
    };

    /// 等待 condition 为 true，最多等 max_wait，步进 step
    template <typename Pred>
    bool wait_for_condition(Pred &&pred,
                            std::chrono::milliseconds max_wait = 500ms,
                            std::chrono::milliseconds step     = 1ms) {
        auto deadline = std::chrono::steady_clock::now() + max_wait;
        while (!pred()) {
            if (std::chrono::steady_clock::now() >= deadline) return false;
            std::this_thread::sleep_for(step);
        }
        return true;
    }
}

SCENARIO("io_context can be default constructed and reflects initial state",
         "[io_context][construction]") {

    GIVEN("a default-constructed io_context") {
        io_context ctx;

        THEN("it is not stopped initially") {
            REQUIRE_FALSE(ctx.stopped());
        }

        THEN("an executor can be obtained from it") {
            auto ex = ctx.get_executor();
            // executor must refer back to the same context
            REQUIRE(&ex.context() == &ctx);
        }
    }

    GIVEN("an io_context constructed with a concurrency hint") {
        io_context ctx{4};

        THEN("it is not stopped initially") {
            REQUIRE_FALSE(ctx.stopped());
        }
    }
}

SCENARIO("io_context stop and restart lifecycle", "[io_context][stop][restart]") {
    GIVEN("a freshly constructed io_context") {
        io_context ctx;

        WHEN("stop() is called") {
            ctx.stop();

            THEN("stopped() returns true") {
                REQUIRE(ctx.stopped());
            }

            AND_WHEN("restart() is called after stop()") {
                ctx.restart();

                THEN("stopped() returns false again") {
                    REQUIRE_FALSE(ctx.stopped());
                }
            }
        }
    }

    GIVEN("an io_context that has been stopped") {
        io_context ctx;
        ctx.stop();

        WHEN("run() is called on a stopped context") {
            auto n = ctx.run();

            THEN("run() returns immediately with 0 completions") {
                REQUIRE(n == 0);
            }
        }

        WHEN("poll() is called on a stopped context") {
            auto n = ctx.poll();

            THEN("poll() returns immediately with 0 completions") {
                REQUIRE(n == 0);
            }
        }
    }
}

// =============================================================================
// FEATURE: run() exits when there is no outstanding work
// =============================================================================

SCENARIO("io_context run() exits automatically when there is no work",
         "[io_context][run][work]") {

    GIVEN("an io_context with no posted work") {
        io_context ctx;

        WHEN("run() is called") {
            auto n = ctx.run();

            THEN("run() returns immediately") {
                REQUIRE(n == 0);
            }

            THEN("the context is not in stopped state") {
                // run() draining naturally does NOT set stopped flag
                REQUIRE_FALSE(ctx.stopped());
            }
        }
    }
}

// =============================================================================
// FEATURE: executor post / dispatch single handler
// =============================================================================

SCENARIO("executor post schedules a handler for deferred execution",
         "[io_context][executor][post]") {

    GIVEN("an io_context and its executor") {
        io_context ctx;
        auto ex = ctx.get_executor();

        WHEN("a single handler is posted") {
            std::atomic<bool> executed{false};
            ex.post([&executed] { executed = true; },
                    std::allocator<void>{});

            AND_WHEN("run() is called") {
                auto n = ctx.run();

                THEN("the handler is executed exactly once") {
                    REQUIRE(executed.load());
                }

                THEN("run() reports one completion") {
                    REQUIRE(n == 1);
                }
            }
        }
    }
}

SCENARIO("executor dispatch executes handler inline when called from within run()",
         "[io_context][executor][dispatch]") {

    GIVEN("an io_context and its executor") {
        io_context ctx;
        auto ex = ctx.get_executor();

        WHEN("dispatch is called from outside the event loop") {
            std::atomic<bool> executed{false};
            ex.dispatch([&executed] { executed = true; },
                        std::allocator<void>{});

            ctx.run();

            THEN("the handler is eventually executed") {
                REQUIRE(executed.load());
            }
        }

        WHEN("dispatch is called from inside the event loop (inline execution)") {
            std::atomic<int> order{0};
            int dispatch_order = -1;
            int post_order     = -1;

            // post an outer handler that then dispatches an inner one
            ex.post([&] {
                // Inside the event loop: dispatch should run inline immediately
                ex.dispatch([&] {
                    dispatch_order = order.fetch_add(1);
                }, std::allocator<void>{});
                // This runs after dispatch returned
                post_order = order.fetch_add(1);
            }, std::allocator<void>{});

            ctx.run();

            THEN("dispatched handler ran before the remainder of the outer handler") {
                REQUIRE(dispatch_order == 0);
                REQUIRE(post_order     == 1);
            }
        }
    }
}

// =============================================================================
// FEATURE: executor defer
// =============================================================================

SCENARIO("executor defer schedules a handler similar to post",
         "[io_context][executor][defer]") {

    GIVEN("an io_context and its executor") {
        io_context ctx;
        auto ex = ctx.get_executor();

        WHEN("a handler is deferred") {
            std::atomic<bool> executed{false};
            ex.defer([&executed] { executed = true; },
                     std::allocator<void>{});

            ctx.run();

            THEN("the handler is executed") {
                REQUIRE(executed.load());
            }
        }
    }
}

// =============================================================================
// FEATURE: multiple posted handlers maintain FIFO order
// =============================================================================

SCENARIO("multiple posted handlers are executed in FIFO order",
         "[io_context][executor][post][order]") {
    GIVEN("an io_context with multiple handlers posted before run()") {
        io_context ctx;
        auto ex = ctx.get_executor();

        std::vector<int> order;
        constexpr int N = 8;

        for (int i = 0; i < N; ++i) {
            ex.post([i, &order] { order.push_back(i); },
                    std::allocator<void>{});
        }

        WHEN("run() processes all handlers") {
            auto n = ctx.run();

            THEN("all handlers were executed") {
                REQUIRE(static_cast<int>(order.size()) == N);
            }

            THEN("handlers ran in the order they were posted") {
                std::vector<int> expected(N);
                std::iota(expected.begin(), expected.end(), 0);
                REQUIRE(order == expected);
            }

            THEN("completion count equals number of handlers") {
                REQUIRE(n == static_cast<std::size_t>(N));
            }
        }
    }
}

SCENARIO("poll() processes only currently-ready handlers without blocking",
         "[io_context][poll]") {
    GIVEN("an io_context with handlers posted before poll()") {
        io_context ctx;
        auto ex = ctx.get_executor();

        std::atomic<int> count{0};
        ex.post([&count] { ++count; }, std::allocator<void>{});
        ex.post([&count] { ++count; }, std::allocator<void>{});

        WHEN("poll() is called") {
            auto n = ctx.poll();

            THEN("all ready handlers are processed") {
                REQUIRE(count.load() == 2);
                REQUIRE(n == 2);
            }
        }
    }

    GIVEN("an io_context with no posted handlers") {
        io_context ctx;

        WHEN("poll() is called") {
            auto n = ctx.poll();

            THEN("it returns immediately with 0") {
                REQUIRE(n == 0);
            }
        }
    }
}

SCENARIO("poll_one() processes at most one ready handler",
         "[io_context][poll_one]") {

    GIVEN("an io_context with two handlers posted") {
        io_context ctx;
        auto ex = ctx.get_executor();

        std::atomic<int> count{0};
        ex.post([&count] { ++count; }, std::allocator<void>{});
        ex.post([&count] { ++count; }, std::allocator<void>{});

        WHEN("poll_one() is called once") {
            auto n = ctx.poll_one();

            THEN("exactly one handler ran") {
                REQUIRE(count.load() == 1);
                REQUIRE(n == 1);
            }

            AND_WHEN("poll_one() is called a second time") {
                auto n2 = ctx.poll_one();

                THEN("the second handler also ran") {
                    REQUIRE(count.load() == 2);
                    REQUIRE(n2 == 1);
                }
            }
        }
    }
}

SCENARIO("run_one() processes exactly one handler then returns",
         "[io_context][run_one]") {

    GIVEN("an io_context with two handlers posted") {
        io_context ctx;
        auto ex = ctx.get_executor();

        std::atomic<int> count{0};
        ex.post([&count] { ++count; }, std::allocator<void>{});
        ex.post([&count] { ++count; }, std::allocator<void>{});

        WHEN("run_one() is called once") {
            auto n = ctx.run_one();

            THEN("exactly one handler ran") {
                REQUIRE(count.load() == 1);
                REQUIRE(n == 1);
            }
        }
    }

    GIVEN("an io_context with no work") {
        io_context ctx;

        WHEN("run_one() is called") {
            auto n = ctx.run_one();

            THEN("it returns 0 immediately") {
                REQUIRE(n == 0);
            }
        }
    }
}

SCENARIO("run_for() returns when the timeout expires with no work",
         "[io_context][run_for]") {

    GIVEN("an io_context with no posted work") {
        io_context ctx;

        WHEN("run_for() is called with a short timeout") {
            auto start = std::chrono::steady_clock::now();
            auto n     = ctx.run_for(20ms);
            auto elapsed = std::chrono::steady_clock::now() - start;

            THEN("it returns 0") {
                REQUIRE(n == 0);
            }

            THEN("it did not block significantly beyond the timeout") {
                // Allow generous 5x slack for loaded CI environments
                REQUIRE(elapsed < 200ms);
            }
        }
    }

    GIVEN("an io_context with a handler posted during run_for()") {
        io_context ctx;
        auto ex = ctx.get_executor();

        std::atomic<bool> executed{false};

        WHEN("run_for() is called and a handler is posted from another thread") {
            std::thread poster([&] {
                std::this_thread::sleep_for(10ms);
                ex.post([&executed] { executed = true; },
                        std::allocator<void>{});
            });

            auto n = ctx.run_for(500ms);
            poster.join();

            THEN("the handler was executed") {
                REQUIRE(executed.load());
                REQUIRE(n >= 1);
            }
        }
    }
}

// =============================================================================
// FEATURE: run_one_for() — time-bounded single completion
// =============================================================================

SCENARIO("run_one_for() returns when the timeout expires with no work",
         "[io_context][run_one_for]") {

    GIVEN("an io_context with no work") {
        io_context ctx;

        WHEN("run_one_for() is called with a short timeout") {
            auto start   = std::chrono::steady_clock::now();
            auto n       = ctx.run_one_for(20ms);
            auto elapsed = std::chrono::steady_clock::now() - start;

            THEN("it returns 0") {
                REQUIRE(n == 0);
            }

            THEN("elapsed time is within reasonable bounds") {
                REQUIRE(elapsed < 200ms);
            }
        }
    }
}

// =============================================================================
// FEATURE: on_work_started / on_work_finished keeps run() alive
// =============================================================================

SCENARIO("on_work_started prevents run() from exiting prematurely",
         "[io_context][executor][work_count]") {

    GIVEN("an io_context whose executor has outstanding work") {
        io_context ctx;
        auto ex = ctx.get_executor();

        WHEN("on_work_started is called before run() and work is finished later") {
            ex.on_work_started();   // simulate one outstanding async op

            std::atomic<bool> run_returned{false};
            std::atomic<bool> handler_ran{false};

            std::thread runner([&] {
                ctx.run();
                run_returned = true;
            });

            // Give run() a moment to start and verify it is blocking
            std::this_thread::sleep_for(30ms);
            REQUIRE_FALSE(run_returned.load());

            // Now post a handler and finish work
            ex.post([&handler_ran] { handler_ran = true; },
                    std::allocator<void>{});
            ex.on_work_finished();

            runner.join();

            THEN("run() eventually returned") {
                REQUIRE(run_returned.load());
            }

            THEN("the posted handler was executed") {
                REQUIRE(handler_ran.load());
            }
        }
    }
}

// =============================================================================
// FEATURE: stop() interrupts a blocking run()
// =============================================================================

SCENARIO("stop() causes a blocking run() to return",
         "[io_context][run][stop]") {

    GIVEN("an io_context with outstanding work keeping run() alive") {
        io_context ctx;
        auto ex = ctx.get_executor();

        ex.on_work_started();   // prevent run() from exiting naturally

        WHEN("stop() is called from another thread") {
            std::atomic<bool> run_returned{false};

            std::thread runner([&] {
                ctx.run();
                run_returned = true;
            });

            std::this_thread::sleep_for(30ms);
            REQUIRE_FALSE(run_returned.load());

            ctx.stop();
            runner.join();

            THEN("run() returned after stop()") {
                REQUIRE(run_returned.load());
            }

            THEN("the context reports stopped") {
                REQUIRE(ctx.stopped());
            }

            // Release the artificial work count so restart works cleanly
            ex.on_work_finished();
        }
    }
}

// =============================================================================
// FEATURE: restart() allows run() to be called again after stop()
// =============================================================================

SCENARIO("restart() resets the stopped flag so run() can be used again",
         "[io_context][restart]") {

    GIVEN("an io_context that was stopped") {
        io_context ctx;
        auto ex = ctx.get_executor();
        ctx.stop();

        WHEN("restart() is called and a handler is posted") {
            ctx.restart();

            std::atomic<bool> executed{false};
            ex.post([&executed] { executed = true; },
                    std::allocator<void>{});

            auto n = ctx.run();

            THEN("the handler is executed") {
                REQUIRE(executed.load());
            }

            THEN("run() reports at least one completion") {
                REQUIRE(n >= 1);
            }
        }
    }
}

// =============================================================================
// FEATURE: running_in_this_thread()
// =============================================================================

SCENARIO("running_in_this_thread() reflects whether the caller is inside run()",
         "[io_context][executor][running_in_this_thread]") {

    GIVEN("an io_context and its executor") {
        io_context ctx;
        auto ex = ctx.get_executor();

        WHEN("queried from outside the event loop") {
            THEN("running_in_this_thread() returns false") {
                REQUIRE_FALSE(ex.running_in_this_thread());
            }
        }

        WHEN("queried from inside a posted handler") {
            std::atomic<bool> inside_loop{false};

            ex.post([&] {
                inside_loop = ex.running_in_this_thread();
            }, std::allocator<void>{});

            ctx.run();

            THEN("running_in_this_thread() returned true inside the handler") {
                REQUIRE(inside_loop.load());
            }
        }
    }
}

// =============================================================================
// FEATURE: multi-threaded concurrent run()
// =============================================================================

SCENARIO("multiple threads can call run() concurrently and share the work",
         "[io_context][run][threading]") {

    GIVEN("an io_context with many handlers posted") {
        io_context ctx;
        auto ex = ctx.get_executor();

        constexpr int HANDLER_COUNT  = 100;
        constexpr int THREAD_COUNT   = 4;
        std::atomic<int> executed{0};

        for (int i = 0; i < HANDLER_COUNT; ++i) {
            ex.post([&executed] { ++executed; }, std::allocator<void>{});
        }

        WHEN("multiple threads call run() simultaneously") {
            std::vector<std::thread> threads;
            threads.reserve(THREAD_COUNT);

            for (int t = 0; t < THREAD_COUNT; ++t) {
                threads.emplace_back([&ctx] { ctx.run(); });
            }
            for (auto &th : threads) th.join();

            THEN("every handler was executed exactly once") {
                REQUIRE(executed.load() == HANDLER_COUNT);
            }
        }
    }
}

// =============================================================================
// FEATURE: handler posted from within another handler
// =============================================================================

SCENARIO("a handler posted from within another handler is executed in the same run()",
         "[io_context][executor][chaining]") {

    GIVEN("an io_context") {
        io_context ctx;
        auto ex = ctx.get_executor();

        WHEN("a handler posts another handler and run() is called once") {
            std::atomic<int> depth{0};

            std::function<void()> recurse;
            recurse = [&] {
                int d = depth.fetch_add(1);
                if (d < 3) {
                    ex.post(recurse, std::allocator<void>{});
                }
            };

            ex.post(recurse, std::allocator<void>{});
            auto n = ctx.run();

            THEN("all chained handlers executed") {
                REQUIRE(depth.load() == 4);  // 0,1,2,3
            }

            THEN("run() returns the total number of completions") {
                REQUIRE(n == 4);
            }
        }
    }
}

// =============================================================================
// FEATURE: exception safety — handler throwing does not corrupt context
// =============================================================================

// NOTE: Per Networking TS / Asio convention, handlers must not throw.
//       Throwing propagates out of run() and terminates the loop for that
//       thread. The context itself must remain usable after restart().

SCENARIO("an exception thrown from a handler propagates through run()",
         "[io_context][exception]") {

    GIVEN("an io_context with a throwing handler posted") {
        io_context ctx;
        auto ex = ctx.get_executor();

        ex.post([] { throw std::runtime_error("handler error"); },
                std::allocator<void>{});

        WHEN("run() is called") {
            THEN("the exception propagates out of run()") {
                REQUIRE_THROWS_AS(ctx.run(), std::runtime_error);
            }

            AND_WHEN("the context is restarted after the exception") {
                try { ctx.run(); } catch (...) {}
                ctx.restart();

                std::atomic<bool> ok{false};
                ex.post([&ok] { ok = true; }, std::allocator<void>{});
                ctx.run();

                THEN("the context is still usable") {
                    REQUIRE(ok.load());
                }
            }
        }
    }
}

// =============================================================================
// FEATURE: get_executor() returns independent copies that share the context
// =============================================================================

SCENARIO("multiple executor copies obtained from the same io_context share state",
         "[io_context][executor][copy]") {

    GIVEN("an io_context with two independently obtained executors") {
        io_context ctx;
        auto ex1 = ctx.get_executor();
        auto ex2 = ctx.get_executor();

        WHEN("one executor posts and the other calls run() indirectly") {
            std::atomic<int> count{0};

            ex1.post([&count] { ++count; }, std::allocator<void>{});
            ex2.post([&count] { ++count; }, std::allocator<void>{});

            auto n = ctx.run();

            THEN("both handlers are executed") {
                REQUIRE(count.load() == 2);
                REQUIRE(n == 2);
            }

            THEN("both executors refer to the same context") {
                REQUIRE(&ex1.context() == &ex2.context());
            }
        }
    }
}