#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <rainy/foundation/concurrency/future.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace rainy::foundation::concurrency;
using namespace std::chrono_literals;
using rainy::core::concurrency::atomic;

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

TEST_CASE("A promise delivers a value to its associated monad_future", "[promise][monad_future][basic]") {

    SECTION("a promise<int> and its associated monad_future") {
        promise<int> p;
        auto f = p.get_monad_future();

        REQUIRE(f.valid());

        SECTION("set_value(42) is called from another thread") {
            resolve_after(p, 42);

            SECTION("monad_future.get() returns 42 and the monad_future becomes invalid") {
                int result = f.get();
                CHECK(result == 42);
                CHECK_FALSE(f.valid());
            }
        }
    }

    SECTION("a promise<std::string> and its associated monad_future") {
        promise<std::string> p;
        auto f = p.get_monad_future();

        SECTION("set_value is called with a string") {
            p.set_value("hello rainy");

            SECTION("monad_future.get() returns that string") {
                CHECK(f.get() == "hello rainy");
            }
        }
    }
}

TEST_CASE("A promise<void> signals completion to its associated monad_future", "[promise][monad_future][void]") {

    SECTION("a promise<void> and its associated monad_future") {
        promise<void> p;
        auto f = p.get_monad_future();

        SECTION("set_value() is called after a short delay") {
            resolve_after(p);

            SECTION("monad_future.get() does not throw and the monad_future becomes invalid") {
                REQUIRE_NOTHROW(f.get());
                CHECK_FALSE(f.valid());
            }
        }
    }
}

TEST_CASE("A promise can deliver an exception to its monad_future", "[promise][monad_future][exception]") {

    SECTION("a promise<int> and its associated monad_future") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("set_exception is called with a std::runtime_error") {
            reject_after(p, std::make_exception_ptr(std::runtime_error("oops")));

            SECTION("monad_future.get() rethrows that exception") {
                REQUIRE_THROWS_AS(f.get(), std::runtime_error);
            }
        }

        SECTION("set_exception is called with a std::logic_error") {
            p.set_exception(std::make_exception_ptr(std::logic_error("bad state")));

            SECTION("monad_future.get() rethrows std::logic_error with the original message") {
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

TEST_CASE("Destroying a promise without resolving injects broken_promise", "[promise][broken]") {

    SECTION("a monad_future whose promise is destroyed before being resolved") {
        monad_future<int> f = [&]() -> monad_future<int> {
            promise<int> p;
            return p.get_monad_future();
        }();

        SECTION("monad_future.get() throws std::future_error with broken_promise") {
            try {
                f.get();
                FAIL("expected std::future_error");
            } catch (const std::future_error &e) {
                CHECK(e.code() == std::future_errc::broken_promise);
            }
        }
    }
}

TEST_CASE("A promise cannot be resolved more than once", "[promise][error]") {

    SECTION("a promise<int> that has already been resolved with set_value") {
        promise<int> p;
        auto f = p.get_monad_future();
        p.set_value(1);

        SECTION("set_value is called a second time") {
            SECTION("it throws std::future_error(promise_already_satisfied)") {
                try {
                    p.set_value(2);
                    FAIL("expected std::future_error");
                } catch (const std::future_error &e) {
                    CHECK(e.code() == std::future_errc::promise_already_satisfied);
                }
            }
        }

        SECTION("set_exception is called after set_value") {
            SECTION("it throws std::future_error(promise_already_satisfied)") {
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

TEST_CASE("get_monad_future() may only be called once per promise", "[promise][error]") {

    SECTION("a promise whose get_monad_future() has already been called") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("get_monad_future() is called a second time") {
            SECTION("it throws std::future_error(future_already_retrieved)") {
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

TEST_CASE("A promise can be move-constructed and move-assigned", "[promise][move]") {

    SECTION("a promise<int> that has been move-constructed into another promise") {
        promise<int> p1;
        auto f = p1.get_monad_future();
        promise<int> p2(std::move(p1));

        SECTION("the new owner resolves via set_value") {
            p2.set_value(7);

            SECTION("the original monad_future receives the value") {
                CHECK(f.get() == 7);
            }
        }
    }

    SECTION("a promise<int> that has been move-assigned") {
        promise<int> p1;
        auto f = p1.get_monad_future();
        promise<int> p2;
        p2 = std::move(p1);

        SECTION("the assigned promise resolves") {
            p2.set_value(13);

            SECTION("the original monad_future receives the value") {
                CHECK(f.get() == 13);
            }
        }
    }
}

TEST_CASE("A default-constructed monad_future is invalid", "[monad_future][valid]") {

    SECTION("a default-constructed monad_future<int>") {
        monad_future<int> f;

        SECTION("valid() returns false") {
            CHECK_FALSE(f.valid());
        }

        SECTION("calling get() throws std::future_error(no_state)") {
            try {
                f.get();
                FAIL("expected std::future_error");
            } catch (const std::future_error &e) {
                CHECK(e.code() == std::future_errc::no_state);
            }
        }

        SECTION("calling wait() throws std::future_error(no_state)") {
            REQUIRE_THROWS_AS(f.wait(), std::future_error);
        }
    }
}

TEST_CASE("A monad_future can only be moved, not copied", "[monad_future][move]") {

    SECTION("a valid monad_future<int>") {
        promise<int> p;
        auto f1 = p.get_monad_future();

        SECTION("it is move-constructed into f2") {
            auto f2 = std::move(f1);

            SECTION("f1 becomes invalid and f2 is valid") {
                CHECK_FALSE(f1.valid());
                CHECK(f2.valid());
            }

            SECTION("the promise is resolved") {
                p.set_value(99);

                SECTION("f2.get() returns the value") {
                    CHECK(f2.get() == 99);
                }
            }
        }
    }
}

TEST_CASE("monad_future::wait() blocks until the promise is resolved", "[monad_future][wait]") {

    SECTION("an unresolved promise<int> and its monad_future") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("wait() is called and the promise resolves after 30ms") {
            auto start = std::chrono::steady_clock::now();
            resolve_after(p, 0, 30ms);
            f.wait();
            auto elapsed = std::chrono::steady_clock::now() - start;

            SECTION("wait() returns only after the resolve") {
                CHECK(elapsed >= 25ms);
                CHECK(f.is_ready());
            }
        }
    }
}

TEST_CASE("monad_future::wait_for() respects the timeout duration", "[monad_future][wait_for]") {

    SECTION("a promise<int> that will never be resolved within the timeout window") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("wait_for() is called with 20ms but the promise resolves after 200ms") {
            resolve_after(p, 0, 200ms);
            auto status = f.wait_for(20ms);

            SECTION("wait_for() returns future_status::timeout") {
                CHECK(status == future_status::timeout);
            }
        }
    }

    SECTION("a promise<int> that resolves before the timeout") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("wait_for() is called with 200ms and the promise resolves after 10ms") {
            resolve_after(p, 42, 10ms);
            auto status = f.wait_for(200ms);

            SECTION("wait_for() returns future_status::ready") {
                CHECK(status == future_status::ready);
            }
        }
    }
}

TEST_CASE("monad_future::wait_until() respects the absolute deadline", "[monad_future][wait_until]") {

    SECTION("a promise<int> that resolves after the deadline has passed") {
        promise<int> p;
        auto f = p.get_monad_future();
        auto deadline = std::chrono::steady_clock::now() + 20ms;

        resolve_after(p, 0, 200ms);

        SECTION("wait_until() is called with that deadline") {
            auto status = f.wait_until(deadline);

            SECTION("it returns future_status::timeout") {
                CHECK(status == future_status::timeout);
            }
        }
    }
}

TEST_CASE("A monad_future can be converted to a shared_future via share()", "[shared_future]") {

    SECTION("a valid monad_future<int>") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("share() is called") {
            auto sf = f.share();

            SECTION("the original monad_future becomes invalid") {
                CHECK_FALSE(f.valid());
            }

            SECTION("the shared_future is valid") {
                CHECK(sf.valid());
            }

            SECTION("the promise is resolved") {
                p.set_value(55);

                SECTION("sf.get() returns the value") {
                    CHECK(sf.get() == 55);
                }

                SECTION("sf.get() can be called multiple times without throwing") {
                    CHECK(sf.get() == 55);
                    CHECK(sf.get() == 55);
                    CHECK(sf.get() == 55);
                }
            }
        }
    }
}

TEST_CASE("Multiple threads can read from a shared_future concurrently", "[shared_future][concurrency]") {

    SECTION("a shared_future<int> shared across several threads") {
        promise<int> p;
        auto sf = p.get_monad_future().share();

        constexpr int num_readers = 8;
        std::vector<int> results(num_readers, -1);
        std::vector<std::thread> readers;

        for (int i = 0; i < num_readers; ++i) {
            readers.emplace_back([&sf, &results, i]() { results[i] = sf.get(); });
        }

        SECTION("the promise is resolved with 42") {
            p.set_value(42);
            for (auto &t: readers)
                t.join();

            SECTION("every reader receives 42") {
                for (int i = 0; i < num_readers; ++i) {
                    CHECK(results[i] == 42);
                }
            }
        }
    }
}

TEST_CASE("A shared_future propagates exceptions to all readers", "[shared_future][exception]") {

    SECTION("a shared_future<int> and a promise that will be rejected") {
        promise<int> p;
        auto sf = p.get_monad_future().share();
        auto sf2 = sf;

        SECTION("set_exception is called") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("shared fail")));

            SECTION("the first reader gets the exception") {
                REQUIRE_THROWS_AS(sf.get(), std::runtime_error);
            }

            SECTION("subsequent reads also throw the same exception") {
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

TEST_CASE("then() transforms the result of a fulfilled monad_future", "[monad_future][then]") {

    SECTION("a monad_future<int> that resolves to 10") {
        promise<int> p;
        auto f = p.get_monad_future();

        SECTION("then() chains a doubling function") {
            auto doubled = f.then([](int v) { return v * 2; });
            p.set_value(10);

            SECTION("the chained monad_future resolves to 20") {
                CHECK(doubled.get() == 20);
            }
        }
    }

    SECTION("a monad_future<std::string> that resolves to a short string") {
        promise<std::string> p;
        auto f = p.get_monad_future();

        SECTION("then() chains a length function") {
            auto len = f.then([](std::string s) -> std::size_t { return s.size(); });
            p.set_value("hello");

            SECTION("the chained monad_future resolves to the string length") {
                CHECK(len.get() == 5u);
            }
        }
    }
}

TEST_CASE("then() can change the value type across multiple steps", "[monad_future][then][chain]") {

    SECTION("a promise<int> and a three-step transformation chain") {
        promise<int> p;

        auto result = p.get_monad_future()
                          .then([](int v) { return v * 3; }) // int  -> int
                          .then([](int v) { return std::to_string(v); }) // int  -> string
                          .then([](std::string s) { return s + "!"; }); // string -> string

        SECTION("the promise is resolved with 4") {
            p.set_value(4);

            SECTION("the final monad_future resolves to \"12!\"") {
                CHECK(result.get() == "12!");
            }
        }
    }
}

TEST_CASE("then skips callback on rejected monad_future", "[monad_future][then][exception]") {
    SECTION("a promise<int> and a then() chain") {
        promise<int> p;
        bool callback_ran = false;

        auto chained = p.get_monad_future().then([&callback_ran](int) -> int {
            callback_ran = true;
            return 0;
        });

        SECTION("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("upstream error")));

            SECTION("the callback is never invoked") {
                REQUIRE_THROWS_AS(chained.get(), std::runtime_error);
                CHECK_FALSE(callback_ran);
            }
        }
    }
}

TEST_CASE("then() propagates exceptions thrown inside the callback", "[monad_future][then][exception]") {

    SECTION("a monad_future<int> with a then() callback that throws") {
        promise<int> p;

        auto chained = p.get_monad_future().then([](int) -> int { throw std::logic_error("callback exploded"); });

        SECTION("the promise is resolved") {
            p.set_value(1);

            SECTION("the chained monad_future is rejected with the thrown exception") {
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

TEST_CASE("then works on already resolved future", "[monad_future][then]") {
    SECTION("a monad_future<int> that is resolved before then() is registered") {
        promise<int> p;
        p.set_value(7);
        auto f = p.get_monad_future();

        SECTION("then() is attached after resolution") {
            auto result = f.then([](int v) { return v + 1; });

            SECTION("the chained monad_future resolves immediately to 8") {
                CHECK(result.get() == 8);
            }
        }
    }
}

TEST_CASE("then() with an explicit executor submits work through that executor", "[monad_future][then][executor]") {

    SECTION("a recording_executor and a resolved monad_future<int>") {
        promise<int> p;
        p.set_value(5);
        auto f = p.get_monad_future();
        recording_executor exec;

        SECTION("then() is called with the recording executor") {
            auto result = f.then(exec, [](int v) { return v * 10; });

            SECTION("the callback has been queued but not yet run") {
                CHECK(exec.queue_size() == 1u);
                CHECK_FALSE(result.is_ready());

                SECTION("the executor is flushed") {
                    exec.flush();

                    SECTION("the chained monad_future resolves to 50") {
                        CHECK(result.get() == 50);
                    }
                }
            }
        }
    }
}

TEST_CASE("catch_error recovers rejected monad_future with fallback value", "[monad_future][catch_error]") {
    SECTION("a promise<int> and a catch_error() handler that returns -1") {
        promise<int> p;
        auto recovered = p.get_monad_future().catch_error([](std::exception_ptr) { return -1; });

        SECTION("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("fail")));

            SECTION("the recovered monad_future resolves to -1") {
                CHECK(recovered.get() == -1);
            }
        }
    }
}

TEST_CASE("catch_error passes fulfilled value", "[monad_future][catch_error]") {
    SECTION("a promise<int> with a catch_error() handler attached") {
        promise<int> p;
        bool handler_ran = false;

        auto result = p.get_monad_future().catch_error([&handler_ran](std::exception_ptr) -> int {
            handler_ran = true;
            return -1;
        });

        SECTION("the promise is resolved normally") {
            p.set_value(42);

            SECTION("the handler is never called and the value passes through") {
                CHECK(result.get() == 42);
                CHECK_FALSE(handler_ran);
            }
        }
    }
}

TEST_CASE("catch_error() propagates when the handler itself throws", "[monad_future][catch_error]") {

    SECTION("a rejected monad_future and a catch_error() handler that re-throws") {
        promise<int> p;
        auto result =
            p.get_monad_future().catch_error([](std::exception_ptr) -> int { throw std::runtime_error("handler also failed"); });

        SECTION("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("original")));

            SECTION("the result monad_future is rejected with the handler's exception") {
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

TEST_CASE("catch_error() can be chained with then() to form recovery pipelines", "[monad_future][catch_error][chain]") {

    SECTION("a chain: then() followed by catch_error() followed by then()") {
        promise<int> p;

        auto result = p.get_monad_future()
                          .then([](int v) -> int {
                              if (v < 0)
                                  throw std::range_error("negative");
                              return v * 2;
                          })
                          .catch_error([](std::exception_ptr) -> int {
                              return 0;
                          })
                          .then([](int v) -> std::string { return "result=" + std::to_string(v); });

        SECTION("the promise is resolved with a negative value (triggers the error path)") {
            p.set_value(-3);

            SECTION("the pipeline recovers and the final monad_future contains the fallback string") {
                CHECK(result.get() == "result=0");
            }
        }

        SECTION("a second chain is set up and resolved with a positive value") {
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

            SECTION("the pipeline produces the doubled value") {
                CHECK(result2.get() == "result=10");
            }
        }
    }
}

TEST_CASE("finally() always runs its callback regardless of outcome", "[monad_future][finally]") {

    SECTION("a fulfilled monad_future<int> with a finally() callback") {
        bool ran = false;
        promise<int> p;
        auto result = p.get_monad_future().finally([&ran] { ran = true; });

        SECTION("the promise is resolved") {
            p.set_value(10);

            SECTION("the finally callback runs and the value passes through") {
                CHECK(result.get() == 10);
                CHECK(ran);
            }
        }
    }

    SECTION("a rejected monad_future<int> with a finally() callback") {
        bool ran = false;
        promise<int> p;
        auto result = p.get_monad_future().finally([&ran] { ran = true; });

        SECTION("the promise is rejected") {
            p.set_exception(std::make_exception_ptr(std::runtime_error("err")));

            SECTION("the finally callback still runs and the exception propagates") {
                REQUIRE_THROWS_AS(result.get(), std::runtime_error);
                CHECK(ran);
            }
        }
    }
}

TEST_CASE("finally() swallows exceptions thrown by its own callback", "[monad_future][finally]") {

    SECTION("a fulfilled monad_future<int> whose finally() callback throws") {
        promise<int> p;
        auto result = p.get_monad_future().finally([] { throw std::runtime_error("cleanup failed"); });

        SECTION("the promise is resolved") {
            p.set_value(7);

            SECTION("the cleanup exception is swallowed and the value passes through") {
                CHECK(result.get() == 7);
            }
        }
    }
}

TEST_CASE("finally() can be composed in a full pipeline", "[monad_future][finally][chain]") {
    SECTION("a chain: then() → finally() → catch_error()") {
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

        SECTION("the promise is resolved with 0 (error path)") {
            p.set_value(0);

            SECTION("finally and catch both run, result is -1") {
                CHECK(result.get() == -1);
                REQUIRE(log.size() == 2u);
                CHECK(log[0] == "finally");
                CHECK(log[1] == "catch");
            }
        }
    }
}

TEST_CASE("multiple threads waiting on shared future all wake up", "[concurrency]") {
    SECTION("a promise<int> and several threads blocked in wait()") {
        promise<int> p;
        auto sf = p.get_monad_future().share();

        constexpr int num_waiters = 16;
        atomic<int> ready_count{0};
        std::vector<std::thread> waiters;

        for (int i = 0; i < num_waiters; ++i) {
            waiters.emplace_back([&sf, &ready_count]() {
                sf.wait();
                ready_count.fetch_add(1, rainy::core::layer::memory_order_relaxed);
            });
        }

        SECTION("the promise is resolved with 1") {
            p.set_value(1);
            for (auto &t: waiters)
                t.join();

            SECTION("all waiting threads unblock") {
                CHECK(ready_count.load() == num_waiters);
            }
        }
    }
}

TEST_CASE("Continuations added from multiple threads are all executed", "[concurrency][then]") {

    SECTION("a shared_state with continuations registered concurrently before resolution") {
        promise<int> p;
        auto sf = p.get_monad_future().share();

        constexpr int num_continuations = 32;
        atomic<int> fired{0};
        std::vector<std::thread> registrars;

        for (int i = 0; i < num_continuations; ++i) {
            registrars.emplace_back([&sf, &fired]() {
                auto _ = sf.then([&fired](int) { fired.fetch_add(1, rainy::core::layer::memory_order_relaxed); });
                (void) _;
            });
        }

        SECTION("the promise is resolved after all registrars have started") {
            std::this_thread::sleep_for(5ms);
            p.set_value(0);
            for (auto &t: registrars) {
                t.join();
            }
            std::this_thread::sleep_for(10ms);
            SECTION("every continuation fires exactly once") {
                CHECK(fired.load() == num_continuations);
            }
        }
    }
}

TEST_CASE("promise::swap exchanges state between two promises", "[promise][swap]") {

    SECTION("two promises p1 and p2 each with an associated monad_future") {
        promise<int> p1, p2;
        auto f1 = p1.get_monad_future();
        auto f2 = p2.get_monad_future();

        SECTION("p1 and p2 are swapped") {
            swap(p1, p2);

            SECTION("p1 (now holding p2's old state) resolves with 10") {
                p1.set_value(10);

                SECTION("f2 receives 10 and f1 is still pending") {
                    CHECK(f2.get() == 10);
                    CHECK_FALSE(f1.is_ready());
                }
            }
        }
    }
}

TEST_CASE("std::future receives value from std::promise", "[future][basic]") {
    SECTION("a promise and a future") {
        promise<int> promise;
        future<int> fut = promise.get_future();

        SECTION("value is set in another thread") {
            std::thread worker([&] {
                std::this_thread::sleep_for(20ms);
                promise.set_value(42);
            });

            SECTION("future returns the correct value") {
                REQUIRE(fut.get() == 42);
            }

            worker.join();
        }
    }
}

TEST_CASE("std::future can only be consumed once", "[future][ownership]") {
    SECTION("a ready future") {
        promise<int> promise;
        auto fut = promise.get_future();
        promise.set_value(10);

        SECTION("get is called") {
            REQUIRE(fut.get() == 10);

            SECTION("future becomes invalid") {
                REQUIRE_FALSE(fut.valid());
            }

            SECTION("calling get again throws") {
                REQUIRE_THROWS_AS(fut.get(), std::future_error);
            }
        }
    }
}

TEST_CASE("future validity changes after get", "[future][state]") {
    SECTION("a future with shared state") {
        promise<int> p;
        auto fut = p.get_future();

        REQUIRE(fut.valid());

        SECTION("result is consumed") {
            p.set_value(1);
            fut.get();

            SECTION("future is no longer valid") {
                REQUIRE_FALSE(fut.valid());
            }
        }
    }
}

TEST_CASE("future wait blocks until value is ready", "[future][wait]") {
    SECTION("a delayed producer thread") {
        promise<int> p;
        auto fut = p.get_future();

        std::thread worker([&] {
            std::this_thread::sleep_for(50ms);
            p.set_value(5);
        });

        SECTION("wait is called") {
            fut.wait();

            SECTION("result is ready") {
                REQUIRE(fut.get() == 5);
            }
        }

        worker.join();
    }
}

#if !RAINY_USING_MACOS
TEST_CASE("future wait_for reports timeout and readiness", "[future][wait_for]") {
    SECTION("a future fulfilled later") {
        promise<int> p;
        auto fut = p.get_future();

        std::thread worker([&] {
            std::this_thread::sleep_for(80ms);
            p.set_value(9);
        });

        SECTION("checking early") {
            auto status = fut.wait_for(10ms);

            SECTION("future is not ready") {
                REQUIRE(status == future_status::timeout);
            }
        }

        SECTION("waiting long enough") {
            auto status = fut.wait_for(200ms);

            SECTION("future becomes ready") {
                REQUIRE(status == future_status::ready);
            }
        }

        worker.join();
    }
}
#endif

TEST_CASE("exception propagates through future", "[future][exception]") {
    SECTION("a promise delivering an exception") {
        promise<int> p;
        auto fut = p.get_future();

        std::thread worker([&] {
            try {
                throw std::runtime_error("failure");
            } catch (...) {
                p.set_exception(std::current_exception());
            }
        });

        SECTION("getting the result") {
            SECTION("exception is rethrown") {
                REQUIRE_THROWS_AS(fut.get(), std::runtime_error);
            }
        }

        worker.join();
    }
}

TEST_CASE("future can be converted to shared_future", "[shared_future]") {
    SECTION("a promise") {
        promise<int> p;
        auto fut = p.get_future();

        shared_future<int> shared = fut.share();

        SECTION("value is fulfilled") {
            p.set_value(77);

            SECTION("shared future reads value") {
                REQUIRE(shared.get() == 77);
            }
        }
    }
}

TEST_CASE("shared_future allows repeated get", "[shared_future][reuse]") {
    SECTION("a ready shared_future") {
        promise<int> p;
        auto shared = p.get_future().share();

        p.set_value(3);

        SECTION("get is called multiple times") {
            int a = shared.get();
            int b = shared.get();

            SECTION("values are identical") {
                REQUIRE(a == 3);
                REQUIRE(b == 3);
            }
        }
    }
}

TEST_CASE("shared_future supports concurrent readers", "[shared_future][thread]") {
    SECTION("a shared future") {
        promise<int> p;
        auto shared = p.get_future().share();

        atomic<int> r1{0};
        atomic<int> r2{0};

        std::thread t1([&] { r1 = shared.get(); });
        std::thread t2([&] { r2 = shared.get(); });

        SECTION("value becomes available") {
            std::this_thread::sleep_for(20ms);
            p.set_value(100);

            t1.join();
            t2.join();

            SECTION("all readers observe same value") {
                REQUIRE(r1 == 100);
                REQUIRE(r2 == 100);
            }
        }
    }
}

TEST_CASE("broken promise triggers exception", "[future][broken]") {
    SECTION("a future whose promise is destroyed") {
        future<int> fut;
        {
            promise<int> p;
            fut = p.get_future();
        } // promise destroyed here
        SECTION("getting the value") {
            SECTION("future reports broken_promise") {
                REQUIRE_THROWS_AS(fut.get(), std::future_error);
            }
        }
    }
}

#ifndef RAINY_USING_MSVC
#pragma warning(pop)
#endif
