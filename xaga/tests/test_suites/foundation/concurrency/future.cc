#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <atomic>
#include <chrono>
#include <future> // only for std::future_error / std::future_errc
#include <rainy/foundation/concurrency/future.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace rainy::foundation::concurrency;
using namespace std::chrono_literals;

#ifndef RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26800)
#endif

namespace {
    template <typename T>
    void resolve_after(promise<T> &p, T value, std::chrono::milliseconds delay = 20ms) {
        auto shared_p = std::make_shared<promise<T>>(std::move(p));
        std::thread([shared_p, v = std::move(value), delay]() mutable {
            std::this_thread::sleep_for(delay);
            shared_p->set_value(std::move(v));
        }).detach();
    }

    void resolve_after(promise<void> &p, std::chrono::milliseconds delay = 20ms) {
        auto shared_p = std::make_shared<promise<void>>(std::move(p));
        std::thread([shared_p, delay]() mutable {
            std::this_thread::sleep_for(delay);
            shared_p->set_value();
        }).detach();
    }

    template <typename T>
    void reject_after(promise<T> &p, std::exception_ptr ep, std::chrono::milliseconds delay = 20ms) {
        auto shared_p = std::make_shared<promise<T>>(std::move(p));
        std::thread([shared_p, ep, delay]() mutable {
            std::this_thread::sleep_for(delay);
            shared_p->set_exception(ep);
        }).detach();
    }

    struct recording_executor {
        struct state {
            std::vector<std::function<void()>> queue;
        };
        std::shared_ptr<state> s = std::make_shared<state>();

        void submit(std::function<void()> fn) {
            s->queue.push_back(std::move(fn));
        }

        void flush() {
            for (auto &fn: s->queue)
                fn();
            s->queue.clear();
        }

        std::size_t queue_size() const {
            return s->queue.size();
        }
    };

}

SCENARIO("A promise delivers a value to its associated monad_future", "[promise][monad_future][basic]") {

    GIVEN("a promise<int> and its associated monad_future") {
        promise<int> p;
        auto f = p.get_monad_future();

        REQUIRE(f.valid());

        WHEN("set_value(42) is called from another thread") {
            resolve_after(p, 42);

            THEN("monad_future.get() returns 42 and the monad_future becomes invalid") {
                int result = f.get();
                CHECK(result == 42);
                CHECK_FALSE(f.valid());
            }
        }
    }

    GIVEN("a promise<std::string> and its associated monad_future") {
        promise<std::string> p;
        auto f = p.get_monad_future();

        WHEN("set_value is called with a string") {
            p.set_value("hello rainy");

            THEN("monad_future.get() returns that string") {
                CHECK(f.get() == "hello rainy");
            }
        }
    }
}

SCENARIO("A promise<void> signals completion to its associated monad_future", "[promise][monad_future][void]") {

    GIVEN("a promise<void> and its associated monad_future") {
        promise<void> p;
        auto f = p.get_monad_future();

        WHEN("set_value() is called after a short delay") {
            resolve_after(p);

            THEN("monad_future.get() does not throw and the monad_future becomes invalid") {
                REQUIRE_NOTHROW(f.get());
                CHECK_FALSE(f.valid());
            }
        }
    }
}

SCENARIO("A promise can deliver an exception to its monad_future", "[promise][monad_future][exception]") {

    GIVEN("a promise<int> and its associated monad_future") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("set_exception is called with a std::runtime_error") {
            reject_after(p, std::make_exception_ptr(std::runtime_error("oops")));

            THEN("monad_future.get() rethrows that exception") {
                REQUIRE_THROWS_AS(f.get(), std::runtime_error);
            }
        }

        WHEN("set_exception is called with a std::logic_error") {
            p.set_exception(std::make_exception_ptr(std::logic_error("bad state")));

            THEN("monad_future.get() rethrows std::logic_error with the original message") {
                try {
                    f.get();
                    FAIL("expected an exception");
                } catch (const std::logic_error &e) {
                    CHECK(std::string(e.what()) == "bad state");
                }
            }
        }
    }
}

SCENARIO("Destroying a promise without resolving injects broken_promise", "[promise][broken]") {

    GIVEN("a monad_future whose promise is destroyed before being resolved") {
        monad_future<int> f = [&]() -> monad_future<int> {
            promise<int> p;
            return p.get_monad_future();
        }();

        THEN("monad_future.get() throws std::future_error with broken_promise") {
            try {
                f.get();
                FAIL("expected std::future_error");
            } catch (const std::future_error &e) {
                CHECK(e.code() == std::future_errc::broken_promise);
            }
        }
    }
}

SCENARIO("A promise cannot be resolved more than once", "[promise][error]") {

    GIVEN("a promise<int> that has already been resolved with set_value") {
        promise<int> p;
        auto f = p.get_monad_future();
        p.set_value(1);

        WHEN("set_value is called a second time") {
            THEN("it throws std::future_error(promise_already_satisfied)") {
                try {
                    p.set_value(2);
                    FAIL("expected std::future_error");
                } catch (const std::future_error &e) {
                    CHECK(e.code() == std::future_errc::promise_already_satisfied);
                }
            }
        }

        WHEN("set_exception is called after set_value") {
            THEN("it throws std::future_error(promise_already_satisfied)") {
                try {
                    p.set_exception(std::make_exception_ptr(std::runtime_error("")));
                    FAIL("expected std::future_error");
                } catch (const std::future_error &e) {
                    CHECK(e.code() == std::future_errc::promise_already_satisfied);
                }
            }
        }
    }
}

SCENARIO("get_monad_future() may only be called once per promise", "[promise][error]") {

    GIVEN("a promise whose get_monad_future() has already been called") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("get_monad_future() is called a second time") {
            THEN("it throws std::future_error(future_already_retrieved)") {
                try {
                    auto f2 = p.get_monad_future();
                    FAIL("expected std::future_error");
                } catch (const std::future_error &e) {
                    CHECK(e.code() == std::future_errc::future_already_retrieved);
                }
            }
        }
    }
}

SCENARIO("A promise can be move-constructed and move-assigned", "[promise][move]") {

    GIVEN("a promise<int> that has been move-constructed into another promise") {
        promise<int> p1;
        auto f = p1.get_monad_future();
        promise<int> p2(std::move(p1));

        WHEN("the new owner resolves via set_value") {
            p2.set_value(7);

            THEN("the original monad_future receives the value") {
                CHECK(f.get() == 7);
            }
        }
    }

    GIVEN("a promise<int> that has been move-assigned") {
        promise<int> p1;
        auto f = p1.get_monad_future();
        promise<int> p2;
        p2 = std::move(p1);

        WHEN("the assigned promise resolves") {
            p2.set_value(13);

            THEN("the original monad_future receives the value") {
                CHECK(f.get() == 13);
            }
        }
    }
}

SCENARIO("A default-constructed monad_future is invalid", "[monad_future][valid]") {

    GIVEN("a default-constructed monad_future<int>") {
        monad_future<int> f;

        THEN("valid() returns false") {
            CHECK_FALSE(f.valid());
        }

        THEN("calling get() throws std::future_error(no_state)") {
            try {
                f.get();
                FAIL("expected std::future_error");
            } catch (const std::future_error &e) {
                CHECK(e.code() == std::future_errc::no_state);
            }
        }

        THEN("calling wait() throws std::future_error(no_state)") {
            REQUIRE_THROWS_AS(f.wait(), std::future_error);
        }
    }
}

SCENARIO("A monad_future can only be moved, not copied", "[monad_future][move]") {

    GIVEN("a valid monad_future<int>") {
        promise<int> p;
        auto f1 = p.get_monad_future();

        WHEN("it is move-constructed into f2") {
            auto f2 = std::move(f1);

            THEN("f1 becomes invalid and f2 is valid") {
                CHECK_FALSE(f1.valid());
                CHECK(f2.valid());
            }

            AND_WHEN("the promise is resolved") {
                p.set_value(99);

                THEN("f2.get() returns the value") {
                    CHECK(f2.get() == 99);
                }
            }
        }
    }
}

SCENARIO("monad_future::wait() blocks until the promise is resolved", "[monad_future][wait]") {

    GIVEN("an unresolved promise<int> and its monad_future") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("wait() is called and the promise resolves after 30ms") {
            auto start = std::chrono::steady_clock::now();
            resolve_after(p, 0, 30ms);
            f.wait();
            auto elapsed = std::chrono::steady_clock::now() - start;

            THEN("wait() returns only after the resolve") {
                CHECK(elapsed >= 25ms);
                CHECK(f.is_ready());
            }
        }
    }
}

SCENARIO("monad_future::wait_for() respects the timeout duration", "[monad_future][wait_for]") {

    GIVEN("a promise<int> that will never be resolved within the timeout window") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("wait_for() is called with 20ms but the promise resolves after 200ms") {
            resolve_after(p, 0, 200ms);
            auto status = f.wait_for(20ms);

            THEN("wait_for() returns future_status::timeout") {
                CHECK(status == future_status::timeout);
            }
        }
    }

    GIVEN("a promise<int> that resolves before the timeout") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("wait_for() is called with 200ms and the promise resolves after 10ms") {
            resolve_after(p, 42, 10ms);
            auto status = f.wait_for(200ms);

            THEN("wait_for() returns future_status::ready") {
                CHECK(status == future_status::ready);
            }
        }
    }
}

SCENARIO("monad_future::wait_until() respects the absolute deadline", "[monad_future][wait_until]") {

    GIVEN("a promise<int> that resolves after the deadline has passed") {
        promise<int> p;
        auto f = p.get_monad_future();
        auto deadline = std::chrono::steady_clock::now() + 20ms;

        resolve_after(p, 0, 200ms);

        WHEN("wait_until() is called with that deadline") {
            auto status = f.wait_until(deadline);

            THEN("it returns future_status::timeout") {
                CHECK(status == future_status::timeout);
            }
        }
    }
}

SCENARIO("A monad_future can be converted to a shared_future via share()", "[shared_future]") {

    GIVEN("a valid monad_future<int>") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("share() is called") {
            auto sf = f.share();

            THEN("the original monad_future becomes invalid") {
                CHECK_FALSE(f.valid());
            }

            THEN("the shared_future is valid") {
                CHECK(sf.valid());
            }

            AND_WHEN("the promise is resolved") {
                p.set_value(55);

                THEN("sf.get() returns the value") {
                    CHECK(sf.get() == 55);
                }

                THEN("sf.get() can be called multiple times without throwing") {
                    CHECK(sf.get() == 55);
                    CHECK(sf.get() == 55);
                    CHECK(sf.get() == 55);
                }
            }
        }
    }
}

SCENARIO("Multiple threads can read from a shared_future concurrently", "[shared_future][concurrency]") {

    GIVEN("a shared_future<int> shared across several threads") {
        promise<int> p;
        auto sf = p.get_monad_future().share();

        constexpr int num_readers = 8;
        std::vector<int> results(num_readers, -1);
        std::vector<std::thread> readers;

        for (int i = 0; i < num_readers; ++i) {
            readers.emplace_back([&sf, &results, i]() { results[i] = sf.get(); });
        }

        WHEN("the promise is resolved with 42") {
            p.set_value(42);
            for (auto &t: readers)
                t.join();

            THEN("every reader receives 42") {
                for (int i = 0; i < num_readers; ++i) {
                    CHECK(results[i] == 42);
                }
            }
        }
    }
}

SCENARIO("A shared_future propagates exceptions to all readers", "[shared_future][exception]") {

    GIVEN("a shared_future<int> and a promise that will be rejected") {
        promise<int> p;
        auto sf = p.get_monad_future().share();
        auto sf2 = sf; // copy

        WHEN("set_exception is called") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("shared fail")));

            THEN("the first reader gets the exception") {
                REQUIRE_THROWS_AS(sf.get(), std::runtime_error);
            }

            THEN("subsequent reads also throw the same exception") {
                try {
                    sf2.get();
                    FAIL("expected exception");
                } catch (const std::runtime_error &e) {
                    CHECK(std::string(e.what()) == "shared fail");
                }
            }
        }
    }
}

SCENARIO("then() transforms the result of a fulfilled monad_future", "[monad_future][then]") {

    GIVEN("a monad_future<int> that resolves to 10") {
        promise<int> p;
        auto f = p.get_monad_future();

        WHEN("then() chains a doubling function") {
            auto doubled = f.then([](int v) { return v * 2; });
            p.set_value(10);

            THEN("the chained monad_future resolves to 20") {
                CHECK(doubled.get() == 20);
            }
        }
    }

    GIVEN("a monad_future<std::string> that resolves to a short string") {
        promise<std::string> p;
        auto f = p.get_monad_future();

        WHEN("then() chains a length function") {
            auto len = f.then([](std::string s) -> std::size_t { return s.size(); });
            p.set_value("hello");

            THEN("the chained monad_future resolves to the string length") {
                CHECK(len.get() == 5u);
            }
        }
    }
}

SCENARIO("then() can change the value type across multiple steps", "[monad_future][then][chain]") {

    GIVEN("a promise<int> and a three-step transformation chain") {
        promise<int> p;

        auto result = p.get_monad_future()
                          .then([](int v) { return v * 3; }) // int  -> int
                          .then([](int v) { return std::to_string(v); }) // int  -> string
                          .then([](std::string s) { return s + "!"; }); // string -> string

        WHEN("the promise is resolved with 4") {
            p.set_value(4);

            THEN("the final monad_future resolves to \"12!\"") {
                CHECK(result.get() == "12!");
            }
        }
    }
}

SCENARIO("then skips callback on rejected monad_future", "[monad_future][then][exception]") {
    GIVEN("a promise<int> and a then() chain") {
        promise<int> p;
        bool callback_ran = false;

        auto chained = p.get_monad_future().then([&callback_ran](int) -> int {
            callback_ran = true;
            return 0;
        });

        WHEN("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("upstream error")));

            THEN("the callback is never invoked") {
                REQUIRE_THROWS_AS(chained.get(), std::runtime_error);
                CHECK_FALSE(callback_ran);
            }
        }
    }
}

SCENARIO("then() propagates exceptions thrown inside the callback", "[monad_future][then][exception]") {

    GIVEN("a monad_future<int> with a then() callback that throws") {
        promise<int> p;

        auto chained = p.get_monad_future().then([](int) -> int { throw std::logic_error("callback exploded"); });

        WHEN("the promise is resolved") {
            p.set_value(1);

            THEN("the chained monad_future is rejected with the thrown exception") {
                try {
                    chained.get();
                    FAIL("expected std::logic_error");
                } catch (const std::logic_error &e) {
                    CHECK(std::string(e.what()) == "callback exploded");
                }
            }
        }
    }
}

SCENARIO("then works on already resolved future", "[monad_future][then]") {
    GIVEN("a monad_future<int> that is resolved before then() is registered") {
        promise<int> p;
        p.set_value(7);
        auto f = p.get_monad_future();

        WHEN("then() is attached after resolution") {
            auto result = f.then([](int v) { return v + 1; });

            THEN("the chained monad_future resolves immediately to 8") {
                CHECK(result.get() == 8);
            }
        }
    }
}

SCENARIO("then() with an explicit executor submits work through that executor", "[monad_future][then][executor]") {

    GIVEN("a recording_executor and a resolved monad_future<int>") {
        promise<int> p;
        p.set_value(5);
        auto f = p.get_monad_future();
        recording_executor exec;

        WHEN("then() is called with the recording executor") {
            auto result = f.then(exec, [](int v) { return v * 10; });

            THEN("the callback has been queued but not yet run") {
                CHECK(exec.queue_size() == 1u);
                CHECK_FALSE(result.is_ready());

                AND_WHEN("the executor is flushed") {
                    exec.flush();

                    THEN("the chained monad_future resolves to 50") {
                        CHECK(result.get() == 50);
                    }
                }
            }
        }
    }
}

SCENARIO("catch_error recovers rejected monad_future with fallback value", "[monad_future][catch_error]") {
    GIVEN("a promise<int> and a catch_error() handler that returns -1") {
        promise<int> p;
        auto recovered = p.get_monad_future().catch_error([](std::exception_ptr) { return -1; });

        WHEN("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("fail")));

            THEN("the recovered monad_future resolves to -1") {
                CHECK(recovered.get() == -1);
            }
        }
    }
}

SCENARIO("catch_error passes fulfilled value", "[monad_future][catch_error]") {
    GIVEN("a promise<int> with a catch_error() handler attached") {
        promise<int> p;
        bool handler_ran = false;

        auto result = p.get_monad_future().catch_error([&handler_ran](std::exception_ptr) -> int {
            handler_ran = true;
            return -1;
        });

        WHEN("the promise is resolved normally") {
            p.set_value(42);

            THEN("the handler is never called and the value passes through") {
                CHECK(result.get() == 42);
                CHECK_FALSE(handler_ran);
            }
        }
    }
}

SCENARIO("catch_error() propagates when the handler itself throws", "[monad_future][catch_error]") {

    GIVEN("a rejected monad_future and a catch_error() handler that re-throws") {
        promise<int> p;
        auto result =
            p.get_monad_future().catch_error([](std::exception_ptr) -> int { throw std::runtime_error("handler also failed"); });

        WHEN("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("original")));

            THEN("the result monad_future is rejected with the handler's exception") {
                try {
                    result.get();
                    FAIL("expected std::runtime_error");
                } catch (const std::runtime_error &e) {
                    CHECK(std::string(e.what()) == "handler also failed");
                }
            }
        }
    }
}

SCENARIO("catch_error() can be chained with then() to form recovery pipelines", "[monad_future][catch_error][chain]") {

    GIVEN("a chain: then() followed by catch_error() followed by then()") {
        promise<int> p;

        auto result = p.get_monad_future()
                          .then([](int v) -> int {
                              if (v < 0)
                                  throw std::range_error("negative");
                              return v * 2;
                          })
                          .catch_error([](std::exception_ptr) -> int {
                              return 0; // recover with sentinel
                          })
                          .then([](int v) -> std::string { return "result=" + std::to_string(v); });

        WHEN("the promise is resolved with a negative value (triggers the error path)") {
            p.set_value(-3);

            THEN("the pipeline recovers and the final monad_future contains the fallback string") {
                CHECK(result.get() == "result=0");
            }
        }

        AND_WHEN("a second chain is set up and resolved with a positive value") {
            promise<int> p2;
            auto result2 = p2.get_monad_future()
                               .then([](int v) -> int {
                                   if (v < 0)
                                       throw std::range_error("negative");
                                   return v * 2;
                               })
                               .catch_error([](std::exception_ptr) -> int { return 0; })
                               .then([](int v) -> std::string { return "result=" + std::to_string(v); });

            p2.set_value(5);

            THEN("the pipeline produces the doubled value") {
                CHECK(result2.get() == "result=10");
            }
        }
    }
}

SCENARIO("finally() always runs its callback regardless of outcome", "[monad_future][finally]") {

    GIVEN("a fulfilled monad_future<int> with a finally() callback") {
        bool ran = false;
        promise<int> p;
        auto result = p.get_monad_future().finally([&ran] { ran = true; });

        WHEN("the promise is resolved") {
            p.set_value(10);

            THEN("the finally callback runs and the value passes through") {
                CHECK(result.get() == 10);
                CHECK(ran);
            }
        }
    }

    GIVEN("a rejected monad_future<int> with a finally() callback") {
        bool ran = false;
        promise<int> p;
        auto result = p.get_monad_future().finally([&ran] { ran = true; });

        WHEN("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("err")));

            THEN("the finally callback still runs and the exception propagates") {
                REQUIRE_THROWS_AS(result.get(), std::runtime_error);
                CHECK(ran);
            }
        }
    }
}

SCENARIO("finally() swallows exceptions thrown by its own callback", "[monad_future][finally]") {

    GIVEN("a fulfilled monad_future<int> whose finally() callback throws") {
        promise<int> p;
        auto result = p.get_monad_future().finally([] { throw std::runtime_error("cleanup failed"); });

        WHEN("the promise is resolved") {
            p.set_value(7);

            THEN("the cleanup exception is swallowed and the value passes through") {
                CHECK(result.get() == 7);
            }
        }
    }
}

SCENARIO("finally() can be composed in a full pipeline", "[monad_future][finally][chain]") {
    GIVEN("a chain: then() → finally() → catch_error()") {
        std::vector<std::string> log;

        promise<int> p;
        auto result = p.get_monad_future()
                          .then([](int v) -> int {
                              if (v == 0)
                                  throw std::invalid_argument("zero");
                              return v + 1;
                          })
                          .finally([&log] { log.push_back("finally"); })
                          .catch_error([&log](std::exception_ptr) -> int {
                              log.push_back("catch");
                              return -1;
                          });

        WHEN("the promise is resolved with 0 (error path)") {
            p.set_value(0);

            THEN("finally and catch both run, result is -1") {
                CHECK(result.get() == -1);
                REQUIRE(log.size() == 2u);
                CHECK(log[0] == "finally");
                CHECK(log[1] == "catch");
            }
        }
    }
}

SCENARIO("set_value_at_thread_exit defers value until thread exit", "[promise][thread_exit]") {
    GIVEN("a promise<int> whose value is set via set_value_at_thread_exit") {
        promise<int> p;
        auto f = p.get_monad_future();
        std::atomic<bool> thread_has_exited{false};

        std::thread([&p, &thread_has_exited]() {
            p.set_value_at_thread_exit(123);
            thread_has_exited.store(true, std::memory_order_release);
        }).join();

        THEN("after the thread exits, monad_future.get() returns the deferred value") {
            CHECK(thread_has_exited.load());
            CHECK(f.get() == 123);
        }
    }
}

SCENARIO("set_exception_at_thread_exit defers exception until thread exit", "[promise][thread_exit]") {
    GIVEN("a promise<int> whose exception is set via set_exception_at_thread_exit") {
        promise<int> p;
        auto f = p.get_monad_future();

        std::thread([&p]() { p.set_exception_at_thread_exit(std::make_exception_ptr(std::runtime_error("deferred error"))); }).join();

        THEN("after the thread exits, monad_future.get() rethrows the deferred exception") {
            try {
                f.get();
                FAIL("expected std::runtime_error");
            } catch (const std::runtime_error &e) {
                CHECK(std::string(e.what()) == "deferred error");
            }
        }
    }
}

SCENARIO("multiple threads waiting on shared future all wake up", "[concurrency]") {
    GIVEN("a promise<int> and several threads blocked in wait()") {
        promise<int> p;
        auto sf = p.get_monad_future().share();

        constexpr int num_waiters = 16;
        std::atomic<int> ready_count{0};
        std::vector<std::thread> waiters;

        for (int i = 0; i < num_waiters; ++i) {
            waiters.emplace_back([&sf, &ready_count]() {
                sf.wait();
                ready_count.fetch_add(1, std::memory_order_relaxed);
            });
        }

        WHEN("the promise is resolved with 1") {
            p.set_value(1);
            for (auto &t: waiters)
                t.join();

            THEN("all waiting threads unblock") {
                CHECK(ready_count.load() == num_waiters);
            }
        }
    }
}

SCENARIO("Continuations added from multiple threads are all executed", "[concurrency][then]") {

    GIVEN("a shared_state with continuations registered concurrently before resolution") {
        promise<int> p;
        auto sf = p.get_monad_future().share();

        constexpr int num_continuations = 32;
        std::atomic<int> fired{0};
        std::vector<std::thread> registrars;

        for (int i = 0; i < num_continuations; ++i) {
            registrars.emplace_back([&sf, &fired]() {
                auto _ = sf.then([&fired](int) { fired.fetch_add(1, std::memory_order_relaxed); });
                (void) _;
            });
        }

        WHEN("the promise is resolved after all registrars have started") {
            std::this_thread::sleep_for(5ms);
            p.set_value(0);
            for (auto &t: registrars) {
                t.join();
            }
            std::this_thread::sleep_for(10ms);
            THEN("every continuation fires exactly once") {
                CHECK(fired.load() == num_continuations);
            }
        }
    }
}

SCENARIO("promise::swap exchanges state between two promises", "[promise][swap]") {

    GIVEN("two promises p1 and p2 each with an associated monad_future") {
        promise<int> p1, p2;
        auto f1 = p1.get_monad_future();
        auto f2 = p2.get_monad_future();

        WHEN("p1 and p2 are swapped") {
            swap(p1, p2);

            AND_WHEN("p1 (now holding p2's old state) resolves with 10") {
                p1.set_value(10);

                THEN("f2 receives 10 and f1 is still pending") {
                    CHECK(f2.get() == 10);
                    CHECK_FALSE(f1.is_ready());
                }
            }
        }
    }
}

SCENARIO("std::future receives value from std::promise", "[future][basic]") {
    GIVEN("a promise and a future") {
        promise<int> promise;
        future<int> fut = promise.get_future();

        WHEN("value is set in another thread") {
            std::thread worker([&] {
                std::this_thread::sleep_for(20ms);
                promise.set_value(42);
            });

            THEN("future returns the correct value") {
                REQUIRE(fut.get() == 42);
            }

            worker.join();
        }
    }
}

SCENARIO("std::future can only be consumed once", "[future][ownership]") {
    GIVEN("a ready future") {
        promise<int> promise;
        auto fut = promise.get_future();
        promise.set_value(10);

        WHEN("get is called") {
            REQUIRE(fut.get() == 10);

            THEN("future becomes invalid") {
                REQUIRE_FALSE(fut.valid());
            }

            AND_THEN("calling get again throws") {
                REQUIRE_THROWS_AS(fut.get(), std::future_error);
            }
        }
    }
}

SCENARIO("future validity changes after get", "[future][state]") {
    GIVEN("a future with shared state") {
        promise<int> p;
        auto fut = p.get_future();

        REQUIRE(fut.valid());

        WHEN("result is consumed") {
            p.set_value(1);
            fut.get();

            THEN("future is no longer valid") {
                REQUIRE_FALSE(fut.valid());
            }
        }
    }
}

SCENARIO("future wait blocks until value is ready", "[future][wait]") {
    GIVEN("a delayed producer thread") {
        promise<int> p;
        auto fut = p.get_future();

        std::thread worker([&] {
            std::this_thread::sleep_for(50ms);
            p.set_value(5);
        });

        WHEN("wait is called") {
            fut.wait();

            THEN("result is ready") {
                REQUIRE(fut.get() == 5);
            }
        }

        worker.join();
    }
}

SCENARIO("future wait_for reports timeout and readiness", "[future][wait_for]") {
    GIVEN("a future fulfilled later") {
        promise<int> p;
        auto fut = p.get_future();

        std::thread worker([&] {
            std::this_thread::sleep_for(80ms);
            p.set_value(9);
        });

        WHEN("checking early") {
            auto status = fut.wait_for(10ms);

            THEN("future is not ready") {
                REQUIRE(status == future_status::timeout);
            }
        }

        WHEN("waiting long enough") {
            auto status = fut.wait_for(200ms);

            THEN("future becomes ready") {
                REQUIRE(status == future_status::ready);
            }
        }

        worker.join();
    }
}

SCENARIO("exception propagates through future", "[future][exception]") {
    GIVEN("a promise delivering an exception") {
        promise<int> p;
        auto fut = p.get_future();

        std::thread worker([&] {
            try {
                throw std::runtime_error("failure");
            } catch (...) {
                p.set_exception(std::current_exception());
            }
        });

        WHEN("getting the result") {
            THEN("exception is rethrown") {
                REQUIRE_THROWS_AS(fut.get(), std::runtime_error);
            }
        }

        worker.join();
    }
}

SCENARIO("future can be converted to shared_future", "[shared_future]") {
    GIVEN("a promise") {
        promise<int> p;
        auto fut = p.get_future();

        shared_future<int> shared = fut.share();

        WHEN("value is fulfilled") {
            p.set_value(77);

            THEN("shared future reads value") {
                REQUIRE(shared.get() == 77);
            }
        }
    }
}

SCENARIO("shared_future allows repeated get", "[shared_future][reuse]") {
    GIVEN("a ready shared_future") {
        promise<int> p;
        auto shared = p.get_future().share();

        p.set_value(3);

        WHEN("get is called multiple times") {
            int a = shared.get();
            int b = shared.get();

            THEN("values are identical") {
                REQUIRE(a == 3);
                REQUIRE(b == 3);
            }
        }
    }
}

SCENARIO("shared_future supports concurrent readers", "[shared_future][thread]") {
    GIVEN("a shared future") {
        promise<int> p;
        auto shared = p.get_future().share();

        std::atomic<int> r1{0};
        std::atomic<int> r2{0};

        std::thread t1([&] { r1 = shared.get(); });
        std::thread t2([&] { r2 = shared.get(); });

        WHEN("value becomes available") {
            std::this_thread::sleep_for(20ms);
            p.set_value(100);

            t1.join();
            t2.join();

            THEN("all readers observe same value") {
                REQUIRE(r1 == 100);
                REQUIRE(r2 == 100);
            }
        }
    }
}

SCENARIO("broken promise triggers exception", "[future][broken]") {
    GIVEN("a future whose promise is destroyed") {
        future<int> fut;
        {
            promise<int> p;
            fut = p.get_future();
        } // promise destroyed here
        WHEN("getting the value") {
            THEN("future reports broken_promise") {
                REQUIRE_THROWS_AS(fut.get(), std::future_error);
            }
        }
    }
}

#ifndef RAINY_USING_MSVC
#pragma warning(pop)
#endif
