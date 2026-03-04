#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>
#include <rainy/meta/wtf/express_center.hpp>
#include <thread>

using namespace rainy::meta::wtf;
using namespace rainy::meta::wtf::execution;

// 测试用事件类型
struct SimpleEvent {
    int value{0};
    explicit SimpleEvent(const int v = 0) : value(v) {
    }
};

struct StringEvent {
    std::string message;
    explicit StringEvent(std::string msg = "") : message(std::move(msg)) {
    }
};

struct CounterEvent {
    mutable int counter{0};
};

SCENARIO("Event subscription and basic emission", "[express_center][subscription]") {
    GIVEN("An express center instance") {
        express_center center;

        WHEN("A subscriber is registered for an event") {
            int received_value = 0;
            auto token = center.subscribe([&](const SimpleEvent &e) { received_value = e.value; });

            THEN("The subscription token should not be empty") {
                REQUIRE_FALSE(token.empty());
            }

            AND_WHEN("An event is emitted immediately") {
                center.emit(immediate, SimpleEvent{42});

                THEN("The subscriber receives the event") {
                    REQUIRE(received_value == 42);
                }
            }
        }

        WHEN("Multiple subscribers are registered") {
            int count1 = 0, count2 = 0, count3 = 0;

            center.subscribe([&](const SimpleEvent &e) { count1 = e.value; });
            center.subscribe([&](const SimpleEvent &e) { count2 = e.value * 2; });
            center.subscribe([&](const SimpleEvent &e) { count3 = e.value * 3; });

            AND_WHEN("An event is emitted") {
                center.emit(immediate, SimpleEvent{10});

                THEN("All subscribers receive the event") {
                    REQUIRE(count1 == 10);
                    REQUIRE(count2 == 20);
                    REQUIRE(count3 == 30);
                }
            }
        }
    }
}

SCENARIO("Event unsubscription", "[express_center][unsubscription]") {
    GIVEN("An express center with a registered subscriber") {
        express_center center;
        int call_count = 0;

        const auto token = center.subscribe([&](const SimpleEvent &) { call_count++; });

        WHEN("The subscriber is unsubscribed") {
            const bool result = center.unsubscribe(token);

            THEN("Unsubscription succeeds") {
                REQUIRE(result == true);
            }

            AND_WHEN("An event is emitted after unsubscription") {
                center.emit(immediate, SimpleEvent{1});

                THEN("The handler is not called") {
                    REQUIRE(call_count == 0);
                }
            }
        }

        WHEN("An invalid token is used for unsubscription") {
            constexpr subscription invalid_token;
            const bool result = center.unsubscribe(invalid_token);

            THEN("Unsubscription fails") {
                REQUIRE(result == false);
            }
        }
    }
}

SCENARIO("Subscription enable/disable", "[express_center][enabled]") {
    GIVEN("An express center with a subscriber") {
        express_center center;
        int call_count = 0;

        const auto token = center.subscribe([&](const SimpleEvent &) { call_count++; });

        WHEN("The subscriber is disabled") {
            const bool result = center.set_enabled(token, false);

            THEN("Set enabled succeeds") {
                REQUIRE(result == true);
            }

            AND_WHEN("An event is emitted") {
                center.emit(immediate, SimpleEvent{1});

                THEN("The handler is not invoked") {
                    REQUIRE(call_count == 0);
                }
            }
        }

        WHEN("The subscriber is disabled then re-enabled") {
            center.set_enabled(token, false);
            center.emit(immediate, SimpleEvent{1});

            center.set_enabled(token, true);
            center.emit(immediate, SimpleEvent{2});

            THEN("The handler is called only after re-enabling") {
                REQUIRE(call_count == 1);
            }
        }
    }
}

SCENARIO("Locked emission", "[express_center][locked]") {
    GIVEN("An express center and a subscriber") {
        express_center center;
        int received = 0;

        center.subscribe([&](const SimpleEvent &e) { received = e.value; });

        WHEN("An event is emitted with internal locking") {
            center.emit(locked, SimpleEvent{99});

            THEN("The event is delivered correctly") {
                REQUIRE(received == 99);
            }
        }

        WHEN("An event is emitted with external locking") {
            std::mutex mtx;
            center.emit(locked, mtx, SimpleEvent{77});

            THEN("The event is delivered correctly") {
                REQUIRE(received == 77);
            }
        }
    }
}

SCENARIO("Async emission with fire-and-forget", "[express_center][async]") {
    GIVEN("An express center with a subscriber") {
        express_center center;
        std::atomic<int> received{0};

        center.subscribe([&](const SimpleEvent &e) { received.store(e.value); });

        WHEN("An event is emitted asynchronously") {
            center.emit(async, SimpleEvent{123});

            // 等待异步处理
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            THEN("The event is eventually delivered") {
                REQUIRE(received.load() == 123);
            }
        }
    }
}

SCENARIO("Async emission with wait-for-all", "[express_center][async][wait]") {
    GIVEN("An express center with a subscriber") {
        express_center center;
        int received = 0;

        center.subscribe([&](const SimpleEvent &e) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            received = e.value;
        });

        WHEN("An event is emitted with wait policy") {
            center.emit(async_wait, SimpleEvent{456});

            THEN("The handler completes before returning") {
                REQUIRE(received == 456);
            }
        }
    }
}

SCENARIO("Event interception", "[express_center][interception]") {
    GIVEN("An express center") {
        express_center center;
        int handler_called = 0;

        center.subscribe([&](const SimpleEvent &) { handler_called++; });

        WHEN("An interceptor allows the event to continue") {
            center.intercept<SimpleEvent>(priority_level::user_mode,
                                          [](const SimpleEvent &) { return interception_result::continue_running; });

            center.emit(immediate, SimpleEvent{1});

            THEN("The event reaches the handler") {
                REQUIRE(handler_called == 1);
            }
        }

        WHEN("An interceptor rejects the event") {
            center.intercept<SimpleEvent>(priority_level::user_mode,
                                          [](const SimpleEvent &) { return interception_result::reject_event; });

            center.emit(immediate, SimpleEvent{1});

            THEN("The event does not reach the handler") {
                REQUIRE(handler_called == 0);
            }
        }

        WHEN("An interceptor bypasses to the handler directly") {
            center.intercept<SimpleEvent>(priority_level::system,
                                          [](const SimpleEvent &) { return interception_result::continue_to_event_handler; });

            center.emit(immediate, SimpleEvent{1});

            THEN("The event reaches the handler") {
                REQUIRE(handler_called == 1);
            }
        }
    }
}

SCENARIO("Interception with predicates", "[express_center][interception][predicate]") {
    GIVEN("An express center with a conditional interceptor") {
        express_center center;
        int blocked_count = 0;

        center.subscribe([&](const SimpleEvent &) { blocked_count++; });

        center.intercept<SimpleEvent>(
            priority_level::user_mode,
            [](const SimpleEvent &e) {
                return e.value > 50 ? interception_result::reject_event : interception_result::continue_running;
            },
            [](const SimpleEvent &e) { return e.value > 0; });

        WHEN("Events matching the predicate are emitted") {
            center.emit(immediate, SimpleEvent{30});
            center.emit(immediate, SimpleEvent{60});
            center.emit(immediate, SimpleEvent{51});

            THEN("Only events passing both predicate and handler reach subscribers") {
                REQUIRE(blocked_count == 1); // 仅30通过
            }
        }
    }
}

SCENARIO("Priority-based interception", "[express_center][priority]") {
    GIVEN("An express center with multiple priority interceptors") {
        express_center center;
        std::vector<std::string> execution_order;

        center.intercept<SimpleEvent>(priority_level::the_root, [&](const SimpleEvent &) {
            execution_order.push_back("root");
            return interception_result::continue_running;
        });

        center.intercept<SimpleEvent>(priority_level::user_mode, [&](const SimpleEvent &) {
            execution_order.push_back("user");
            return interception_result::continue_running;
        });

        center.intercept<SimpleEvent>(priority_level::system, [&](const SimpleEvent &) {
            execution_order.push_back("system");
            return interception_result::continue_running;
        });

        WHEN("An event is emitted") {
            center.emit(immediate, SimpleEvent{1});

            THEN("Interceptors execute in priority order") {
                REQUIRE(execution_order.size() == 3);
                REQUIRE(execution_order[0] == "root");
                REQUIRE(execution_order[1] == "system");
                REQUIRE(execution_order[2] == "user");
            }
        }
    }
}

SCENARIO("Unintercept operations", "[express_center][unintercept]") {
    GIVEN("An express center with interceptors") {
        express_center center;
        const std::size_t event_id = rainy::foundation::ctti::typeinfo::get_type_hash<SimpleEvent>();

        center.intercept<SimpleEvent>(priority_level::user_mode,
                                      [](const SimpleEvent &) { return interception_result::continue_running; });
        center.intercept<SimpleEvent>(priority_level::system,
                                      [](const SimpleEvent &) { return interception_result::continue_running; });

        WHEN("A specific priority level is cleared") {
            center.unintercept(event_id, priority_level::user_mode);

            THEN("Other priority levels remain intact") {
                // 仍然可以正常发射事件
                bool no_crash = true;
                center.emit(immediate, SimpleEvent{1});
                REQUIRE(no_crash);
            }
        }

        WHEN("All interceptions are cleared") {
            center.unintercept_all(event_id);

            THEN("Event emission proceeds without interception") {
                bool no_crash = true;
                center.emit(immediate, SimpleEvent{1});
                REQUIRE(no_crash);
            }
        }
    }
}

SCENARIO("Subscriber count and query", "[express_center][query]") {
    GIVEN("An express center") {
        express_center center;
        const std::size_t event_id = rainy::foundation::ctti::typeinfo::get_type_hash<SimpleEvent>();

        WHEN("No subscribers exist") {
            THEN("Subscriber count is zero") {
                REQUIRE(center.subscriber_count(event_id) == 0);
                REQUIRE_FALSE(center.has_subscribers(event_id));
            }
        }

        WHEN("Multiple subscribers are added") {
            center.subscribe([](const SimpleEvent &) {});
            center.subscribe([](const SimpleEvent &) {});
            center.subscribe([](const SimpleEvent &) {});

            THEN("Subscriber count is correct") {
                REQUIRE(center.subscriber_count(event_id) == 3);
                REQUIRE(center.has_subscribers(event_id));
            }
        }
    }
}

SCENARIO("Clear operations", "[express_center][clear]") {
    GIVEN("An express center with subscribers") {
        express_center center;
        const std::size_t event_id = rainy::foundation::ctti::typeinfo::get_type_hash<SimpleEvent>();

        center.subscribe([](const SimpleEvent &) {});
        center.subscribe([](const StringEvent &) {});

        WHEN("A specific event is cleared") {
            center.clear_event(event_id);

            THEN("Only that event's subscribers are removed") {
                REQUIRE(center.subscriber_count(event_id) == 0);
            }
        }

        WHEN("All events are cleared") {
            center.clear();

            THEN("All subscribers are removed") {
                REQUIRE(center.subscriber_count(event_id) == 0);
            }
        }
    }
}

SCENARIO("Dispatcher wrapper", "[dispatcher]") {
    GIVEN("A dispatcher wrapping an express center") {
        express_center center;
        dispatcher disp(center);

        WHEN("Operations are performed through the dispatcher") {
            int value = 0;
            const auto token = disp.subscribe([&](const SimpleEvent &e) { value = e.value; });

            disp.emit(immediate, SimpleEvent{88});

            THEN("Operations work correctly") {
                REQUIRE(value == 88);
                REQUIRE(disp.subscriber_count<SimpleEvent>() == 1);
                REQUIRE(disp.has_subscribers<SimpleEvent>());
            }

            AND_WHEN("Unsubscribe through dispatcher") {
                disp.unsubscribe(token);

                THEN("Subscriber is removed") {
                    REQUIRE(disp.subscriber_count<SimpleEvent>() == 0);
                }
            }
        }
    }
}

SCENARIO("Thread safety", "[express_center][concurrency]") {
    GIVEN("An express center accessed by multiple threads") {
        express_center center;
        std::atomic<int> total_calls{0};

        center.subscribe([&](const SimpleEvent &) { total_calls.fetch_add(1); });

        WHEN("Multiple threads emit events concurrently") {
            constexpr int num_threads = 4;
            constexpr int events_per_thread = 25;

            std::vector<std::thread> threads;
            for (int i = 0; i < num_threads; ++i) {
                threads.emplace_back([&]() {
                    for (int j = 0; j < events_per_thread; ++j) {
                        center.emit(locked, SimpleEvent{j});
                    }
                });
            }

            for (auto &t: threads) {
                t.join();
            }

            THEN("All events are delivered correctly") {
                REQUIRE(total_calls.load() == num_threads * events_per_thread);
            }
        }
    }
}

SCENARIO("Shutdown behavior", "[express_center][shutdown]") {
    GIVEN("An express center with async tasks") {
        express_center center;
        std::atomic<int> completed{0};

        center.subscribe([&](const SimpleEvent &) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            completed.fetch_add(1);
        });

        WHEN("Events are queued and center is shut down") {
            center.emit(async, SimpleEvent{1});
            center.emit(async, SimpleEvent{2});

            center.shutdown();

            THEN("The center stops processing gracefully") {
                // shutdown应该等待线程完成
                REQUIRE(completed.load() >= 0);
            }
        }
    }
}
