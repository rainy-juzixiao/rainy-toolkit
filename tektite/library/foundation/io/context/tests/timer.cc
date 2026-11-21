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
#include <rainy/foundation/io/timer.hpp>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

using namespace rainy::foundation::io;
using namespace std::chrono_literals;

namespace {
    struct background_runner {
        explicit background_runner(io_context &ctx) : ctx_(ctx), thread_([this] { ctx_.run(); }) {
        }

        ~background_runner() {
            ctx_.stop();
            if (thread_.joinable())
                thread_.join();
        }

        background_runner(const background_runner &) = delete;
        background_runner &operator=(const background_runner &) = delete;

    private:
        io_context &ctx_;
        std::thread thread_;
    };

    template <typename Pred>
    bool spin_wait(Pred &&pred, std::chrono::milliseconds max_wait = 2000ms, std::chrono::milliseconds step = 1ms) {
        auto deadline = std::chrono::steady_clock::now() + max_wait;
        while (!pred()) {
            if (std::chrono::steady_clock::now() >= deadline)
                return false;
            std::this_thread::sleep_for(step);
        }
        return true;
    }
}

TEST_CASE("wait_traits converts time_point and duration correctly", "[timer][wait_traits]") {
    SECTION("wait_traits for steady_clock") {
        using traits = wait_traits<std::chrono::steady_clock>;
        SECTION("to_wait_duration is called with a future time_point") {
            auto future = std::chrono::steady_clock::now() + 100ms;
            auto d = traits::to_wait_duration(future);
            SECTION("the returned duration is positive and roughly 100ms") {
                REQUIRE(d > std::chrono::steady_clock::duration::zero());
                REQUIRE(d <= 110ms);
            }
        }
        SECTION("to_wait_duration is called with a past time_point") {
            auto past = std::chrono::steady_clock::now() - 100ms;
            auto d = traits::to_wait_duration(past);
            SECTION("the returned duration is zero (already expired)") {
                REQUIRE(d == std::chrono::steady_clock::duration::zero());
            }
        }
        SECTION("to_wait_duration is called with a duration directly") {
            auto input = 200ms;
            auto output = traits::to_wait_duration(std::chrono::steady_clock::duration{input});

            SECTION("the duration is returned unchanged") {
                REQUIRE(output == input);
            }
        }
    }
}

TEST_CASE("steady_timer expiry is set correctly at construction", "[timer][construction]") {
    SECTION("an io_context") {
        io_context ctx;
        SECTION("timer is constructed with a duration") {
            auto before = std::chrono::steady_clock::now();
            steady_timer t(ctx, 100ms);
            auto after = std::chrono::steady_clock::now();
            SECTION("expiry() is between now+100ms and now+110ms") {
                REQUIRE(t.expiry() >= before + 100ms);
                REQUIRE(t.expiry() <= after + 110ms);
            }
        }
        SECTION("timer is constructed with an absolute time_point") {
            auto tp = std::chrono::steady_clock::now() + 500ms;
            steady_timer t(ctx, tp);
            SECTION("expiry() matches the given time_point exactly") {
                REQUIRE(t.expiry() == tp);
            }
        }
        SECTION("timer is default-constructed (no expiry)") {
            steady_timer t(ctx);
            SECTION("expiry() is the epoch (time_point{})") {
                using tp = std::chrono::steady_clock::time_point;
                REQUIRE(t.expiry() == tp{});
            }
        }
    }
}

TEST_CASE("expires_at and expires_after update the expiry and cancel pending waits", "[timer][expiry]") {
    SECTION("a steady_timer with an initial 10-second expiry") {
        io_context ctx;
        steady_timer t(ctx, 10s);
        SECTION("expires_after is called with 50ms") {
            auto before = std::chrono::steady_clock::now();
            auto cancelled = t.expires_after(50ms);
            SECTION("no previous async waits were cancelled (none were pending)") {
                REQUIRE(cancelled == 0);
            }
            SECTION("expiry is updated to approximately now + 50ms") {
                REQUIRE(t.expiry() >= before + 50ms);
                REQUIRE(t.expiry() <= before + 60ms);
            }
        }
        SECTION("expires_at is called with a specific time_point") {
            auto tp = std::chrono::steady_clock::now() + 200ms;
            auto cancelled = t.expires_at(tp);
            SECTION("no previous async waits were cancelled") {
                REQUIRE(cancelled == 0);
            }
            SECTION("expiry matches the given time_point") {
                REQUIRE(t.expiry() == tp);
            }
        }
    }
    SECTION("a steady_timer with one pending async_wait") {
        io_context ctx;
        steady_timer t(ctx, 10s); // far future, won't fire during test
        std::atomic<bool> called{false};
        std::error_code received_ec{};
        t.async_wait([&](std::error_code ec) {
            received_ec = ec;
            called = true;
        });
        SECTION("expires_after is called while a wait is pending") {
            auto cancelled = t.expires_after(50ms);
            SECTION("one pending wait was cancelled") {
                REQUIRE(cancelled == 1);
            }
            SECTION("ctx.run() processes the cancellation callback") {
                ctx.run();
                SECTION("the cancelled handler was called with operation_canceled") {
                    REQUIRE(called.load());
                    REQUIRE(received_ec == std::make_error_code(std::errc::operation_canceled));
                }
            }
        }
    }
}

TEST_CASE("async_wait fires the handler after the timer expires", "[timer][async_wait]") {
    SECTION("a steady_timer set to expire in 50ms") {
        io_context ctx;
        steady_timer t(ctx, 50ms);
        std::atomic<bool> fired{false};
        std::error_code received_ec{};
        SECTION("async_wait is registered and ctx.run() is called") {
            t.async_wait([&](std::error_code ec) {
                received_ec = ec;
                fired = true;
            });
            auto start = std::chrono::steady_clock::now();
            ctx.run();
            auto elapsed = std::chrono::steady_clock::now() - start;
            SECTION("the handler was called with no error") {
                REQUIRE(fired.load());
                REQUIRE_FALSE(received_ec);
            }
            SECTION("at least 50ms elapsed before run() returned") {
                REQUIRE(elapsed >= 40ms); // allow 10ms under-measurement slack
            }

            SECTION("run() returned after the handler completed") {
                // run() exits naturally: work_count_ back to 0
                REQUIRE(ctx.stopped() == false);
            }
        }
    }
    SECTION("a system_timer set to expire in 30ms") {
        io_context ctx;
        system_timer t(ctx, 30ms);
        std::atomic<bool> fired{false};
        SECTION("async_wait is registered and ctx.run() is called") {
            t.async_wait([&](std::error_code ec) {
                if (!ec)
                    fired = true;
            });
            ctx.run();
            SECTION("the handler was called") {
                REQUIRE(fired.load());
            }
        }
    }
}

TEST_CASE("async_wait fires immediately when the timer has already expired", "[timer][async_wait][expired]") {
    SECTION("a steady_timer set to a past time_point") {
        io_context ctx;
        steady_timer t(ctx, std::chrono::steady_clock::now() - 1s);
        std::atomic<bool> fired{false};
        SECTION("async_wait is registered") {
            t.async_wait([&](std::error_code ec) {
                if (!ec) {
                    fired = true;
                }
            });
            auto start = std::chrono::steady_clock::now();
            ctx.run();
            auto elapsed = std::chrono::steady_clock::now() - start;
            SECTION("the handler is called") {
                REQUIRE(fired.load());
            }
            SECTION("run() returns quickly (well under 200ms)") {
                REQUIRE(elapsed < 200ms);
            }
        }
    }
}

TEST_CASE("cancel() cancels all pending async_wait operations", "[timer][cancel]") {
    SECTION("a steady_timer with a far-future expiry and one pending wait") {
        io_context ctx;
        steady_timer t(ctx, 60s);
        std::atomic<bool> called{false};
        std::error_code received_ec{};
        t.async_wait([&](std::error_code ec) {
            received_ec = ec;
            called = true;
        });
        SECTION("cancel() is called") {
            auto n = t.cancel();
            SECTION("cancel() reports one cancelled operation") {
                REQUIRE(n == 1);
            }
            SECTION("ctx.run() processes the cancellation") {
                ctx.run();
                SECTION("the handler was called with operation_canceled") {
                    REQUIRE(called.load());
                    REQUIRE(received_ec == std::make_error_code(std::errc::operation_canceled));
                }
            }
        }
    }
    SECTION("a steady_timer with two pending async_waits") {
        io_context ctx;
        steady_timer t(ctx, 60s);
        std::atomic<int> cancel_count{0};
        t.async_wait([&](std::error_code ec) {
            if (ec == std::make_error_code(std::errc::operation_canceled))
                ++cancel_count;
        });
        t.async_wait([&](std::error_code ec) {
            if (ec == std::make_error_code(std::errc::operation_canceled))
                ++cancel_count;
        });
        SECTION("cancel() is called") {
            auto n = t.cancel();
            ctx.run();
            SECTION("both operations were cancelled") {
                REQUIRE(n == 2);
                REQUIRE(cancel_count.load() == 2);
            }
        }
    }
    SECTION("a steady_timer with no pending waits") {
        io_context ctx;
        steady_timer t(ctx, 60s);
        SECTION("cancel() is called") {
            auto n = t.cancel();
            SECTION("it returns 0") {
                REQUIRE(n == 0);
            }
        }
    }
}

TEST_CASE("cancel_one() cancels only the earliest pending async_wait", "[timer][cancel_one]") {
    SECTION("a steady_timer with two pending async_waits") {
        io_context ctx;
        steady_timer t(ctx, 60s);
        std::atomic<int> cancel_count{0};
        std::atomic<int> success_count{0};
        t.async_wait([&](std::error_code ec) {
            if (ec) {
                ++cancel_count;
            } else {
                ++success_count;
            }
        });
        t.async_wait([&](std::error_code ec) {
            if (ec) {
                ++cancel_count;
            } else {
                ++success_count;
            }
        });
        SECTION("cancel_one() is called once") {
            auto n = t.cancel_one();
            SECTION("exactly one operation was reported cancelled") {
                REQUIRE(n == 1);
            }
            SECTION("the remaining wait is also cancelled to let run() exit") {
                t.cancel();
                ctx.run();
                SECTION("exactly one handler received operation_canceled from cancel_one") {
                    // cancel_one cancelled 1, then cancel() cancelled the other 1
                    REQUIRE(cancel_count.load() == 2);
                    REQUIRE(success_count.load() == 0);
                }
            }
        }
    }
}

TEST_CASE("basic_waitable_timer supports move semantics", "[timer][move]") {
    SECTION("a steady_timer with a 50ms expiry") {
        io_context ctx;
        steady_timer t1(ctx, 50ms);
        SECTION("it is move-constructed into t2") {
            auto expiry = t1.expiry();
            steady_timer t2(std::move(t1));
            SECTION("t2 has the same expiry") {
                REQUIRE(t2.expiry() == expiry);
            }

            SECTION("async_wait is registered on t2 and ctx.run() runs") {
                std::atomic<bool> fired{false};
                t2.async_wait([&](std::error_code ec) {
                    if (!ec) {
                        fired = true;
                    }
                });
                ctx.run();
                SECTION("the handler fires correctly via t2") {
                    REQUIRE(fired.load());
                }
            }
        }
    }
    SECTION("two steady_timers, t1 with 50ms expiry and t2 with 60s expiry") {
        io_context ctx;
        steady_timer t1(ctx, 50ms);
        steady_timer t2(ctx, 60s);
        std::atomic<bool> t2_cancelled{false};
        t2.async_wait([&](std::error_code ec) {
            if (ec == std::make_error_code(std::errc::operation_canceled)) {
                t2_cancelled = true;
            }
        });
        SECTION("t1 is move-assigned into t2") {
            // move-assign: t2's pending wait should be cancelled
            t2 = std::move(t1);
            ctx.run();

            SECTION("t2's original pending wait was cancelled by the move") {
                REQUIRE(t2_cancelled.load());
            }
        }
    }
}

TEST_CASE("wait() blocks the calling thread until expiry", "[timer][wait][sync]") {
    SECTION("a steady_timer set to expire in 50ms") {
        io_context ctx;
        steady_timer t(ctx, 50ms);
        SECTION("wait() is called on a background thread") {
            auto start = std::chrono::steady_clock::now();
            std::error_code ec;
            t.wait(ec);
            auto elapsed = std::chrono::steady_clock::now() - start;
            SECTION("no error is reported") {
                REQUIRE_FALSE(ec);
            }

            SECTION("at least 50ms elapsed") {
                REQUIRE(elapsed >= 40ms);
            }
        }
    }
    SECTION("a steady_timer already expired") {
        io_context ctx;
        steady_timer t(ctx, std::chrono::steady_clock::now() - 1s);
        SECTION("wait() is called") {
            auto start = std::chrono::steady_clock::now();
            std::error_code ec;
            t.wait(ec);
            auto elapsed = std::chrono::steady_clock::now() - start;
            SECTION("it returns immediately with no error") {
                REQUIRE_FALSE(ec);
                REQUIRE(elapsed < 50ms);
            }
        }
    }
}

TEST_CASE("multiple timers sharing one io_context all fire correctly", "[timer][multi]") {
    SECTION("three steady_timers with different expiries on the same io_context") {
        io_context ctx;
        steady_timer t1(ctx, 20ms);
        steady_timer t2(ctx, 60ms);
        steady_timer t3(ctx, 100ms);
        std::vector<int> order;
        std::mutex order_mutex;

        auto make_handler = [&](int id) {
            return [&, id](std::error_code ec) {
                if (!ec) {
                    std::lock_guard<std::mutex> lock(order_mutex);
                    order.push_back(id);
                }
            };
        };

        t1.async_wait(make_handler(1));
        t2.async_wait(make_handler(2));
        t3.async_wait(make_handler(3));

        SECTION("ctx.run() processes all three timers") {
            ctx.run();

            SECTION("all three handlers fired in expiry order") {
                REQUIRE(order.size() == 3);
                REQUIRE(std::count(order.begin(), order.end(), 1) == 1);
                REQUIRE(std::count(order.begin(), order.end(), 2) == 1);
                REQUIRE(std::count(order.begin(), order.end(), 3) == 1);
            }
        }
    }
}

TEST_CASE("a timer can be rescheduled and awaited again after firing", "[timer][reschedule]") {
    SECTION("a steady_timer that has already fired once") {
        io_context ctx;
        steady_timer t(ctx, 20ms);
        std::atomic<int> fire_count{0};
        t.async_wait([&](std::error_code ec) {
            if (!ec)
                ++fire_count;
        });
        ctx.run();
        REQUIRE(fire_count.load() == 1);
        SECTION("the timer is rescheduled with expires_after and awaited again") {
            ctx.restart();
            t.expires_after(20ms);
            t.async_wait([&](std::error_code ec) {
                if (!ec)
                    ++fire_count;
            });
            ctx.run();
            SECTION("the handler fires a second time") {
                REQUIRE(fire_count.load() == 2);
            }
        }
    }
}

TEST_CASE("a timer can be cancelled from within a different handler's callback", "[timer][cancel][handler]") {
    SECTION("a short timer and a long timer sharing one io_context") {
        io_context ctx;
        steady_timer short_t(ctx, 20ms);
        steady_timer long_t(ctx, 60s);
        std::atomic<bool> short_fired{false};
        std::atomic<bool> long_cancelled{false};
        long_t.async_wait([&](std::error_code ec) {
            if (ec == std::make_error_code(std::errc::operation_canceled))
                long_cancelled = true;
        });
        short_t.async_wait([&](std::error_code ec) {
            if (!ec) {
                short_fired = true;
                long_t.cancel(); // cancel the long timer from inside a handler
            }
        });
        SECTION("ctx.run() runs both timers") {
            ctx.run();
            SECTION("the short timer fired normally") {
                REQUIRE(short_fired.load());
            }

            SECTION("the long timer was cancelled by the short timer's handler") {
                REQUIRE(long_cancelled.load());
            }
        }
    }
}

TEST_CASE("get_executor() returns an executor associated with the owning io_context", "[timer][executor]") {
    SECTION("a steady_timer bound to an io_context") {
        io_context ctx;
        steady_timer t(ctx, 100ms);
        SECTION("get_executor() is called") {
            auto ex = t.get_executor();
            SECTION("the executor refers to the same io_context") {
                REQUIRE(&ex.context() == &ctx);
            }
        }
    }
}
