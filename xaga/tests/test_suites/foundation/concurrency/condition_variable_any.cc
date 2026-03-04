#include <atomic>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <stop_token>
#include <thread>

using namespace rainy::foundation::concurrency;
using namespace std::chrono_literals;

SCENARIO("Constructing and destructing condition_variable_any", "[condition_variable_any]") {
    GIVEN("A newly constructed condition variable") {
        condition_variable_any cv;

        THEN("It has a valid native handle") {
            REQUIRE(cv.native_handle() != nullptr);
        }
    }
}

SCENARIO("notify_one wakes a waiting thread", "[condition_variable_any]") {
    GIVEN("A condition variable_any and a mutex") {
        condition_variable_any cv;
        mutex mtx;
        std::atomic<bool> ready{false};

        WHEN("A thread waits and another thread calls notify_one") {
            std::thread t([&]() {
                unique_lock<mutex> lock(mtx);
                ready = true;
                cv.wait(lock);
            });

            while (!ready) std::this_thread::yield();

            THEN("notify_one unblocks the waiting thread") {
                std::this_thread::sleep_for(50ms);
                cv.notify_one();
                t.join();
                REQUIRE_FALSE(t.joinable());
            }
        }
    }
}

SCENARIO("notify_all wakes all waiting threads", "[condition_variable_any]") {
    GIVEN("A condition variable_any and multiple threads waiting") {
        condition_variable_any cv;
        mutex mtx;
        std::atomic<bool> ready1{false}, ready2{false};

        std::thread t1([&]() {
            unique_lock<mutex> lock(mtx);
            ready1 = true;
            cv.wait(lock);
        });

        std::thread t2([&]() {
            unique_lock<mutex> lock(mtx);
            ready2 = true;
            cv.wait(lock);
        });

        while (!ready1 || !ready2) std::this_thread::yield();

        WHEN("notify_all is called") {
            std::this_thread::sleep_for(50ms);
            cv.notify_all();

            THEN("All threads are unblocked") {
                t1.join();
                t2.join();
                REQUIRE_FALSE(t1.joinable());
                REQUIRE_FALSE(t2.joinable());
            }
        }
    }
}

SCENARIO("wait with predicate avoids spurious wakeup", "[condition_variable_any]") {
    GIVEN("A condition variable_any and a shared state") {
        condition_variable_any cv;
        mutex mtx;
        bool flag = false;

        WHEN("A thread waits with a predicate and another thread sets the flag") {
            std::thread t([&]() {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [&]{ return flag; });
            });

            std::this_thread::sleep_for(50ms);

            {
                unique_lock<mutex> lock(mtx);
                flag = true;
            }
            cv.notify_one();

            THEN("The waiting thread proceeds after the predicate is true") {
                t.join();
                REQUIRE_FALSE(t.joinable());
            }
        }
    }
}

SCENARIO("wait_for returns timeout when duration expires", "[condition_variable_any]") {
    GIVEN("A condition variable_any and a mutex") {
        condition_variable_any cv;
        mutex mtx;

        WHEN("wait_for is called with a short timeout") {
            unique_lock<mutex> lock(mtx);
            auto start = std::chrono::steady_clock::now();
            cv_status status = cv.wait_for(lock, 10ms);
            auto end = std::chrono::steady_clock::now();

            THEN("It returns cv_status::timeout") {
                REQUIRE(status == cv_status::timeout);
                REQUIRE((end - start) >= 10ms);
            }
        }
    }
}

SCENARIO("wait_until returns no_timeout when notified before deadline", "[condition_variable_any]") {
    GIVEN("A condition variable_any and a mutex") {
        condition_variable_any cv;
        mutex mtx;
        bool ready = false;

        WHEN("A thread waits until a future time point and another thread notifies") {
            std::thread t([&]() {
                unique_lock<mutex> lock(mtx);
                cv.wait_until(lock, std::chrono::steady_clock::now() + 500ms, [&]{ return ready; });
            });

            std::this_thread::sleep_for(50ms);

            {
                unique_lock<mutex> lock(mtx);
                ready = true;
            }
            cv.notify_one();

            THEN("The waiting thread proceeds before timeout") {
                t.join();
                REQUIRE_FALSE(t.joinable());
            }
        }
    }
}

SCENARIO("wait with stop_token can be interrupted", "[condition_variable_any]") {
    GIVEN("A condition variable_any, a mutex and a stop source") {
        condition_variable_any cv;
        mutex mtx;
        std::stop_source stop_src;
        bool flag = false;
        bool result_from_thread = true; // 用于保存线程返回值

        WHEN("A thread waits with a stop_token and stop is requested") {
            std::thread t([&]() {
                unique_lock<mutex> lock(mtx);
                result_from_thread = cv.wait(lock, stop_src.get_token(), [&]{ return flag; });
            });

            std::this_thread::sleep_for(50ms);
            stop_src.request_stop();

            THEN("The waiting thread returns false due to stop request") {
                t.join();
                REQUIRE_FALSE(result_from_thread);  // 在主线程断言
                REQUIRE_FALSE(t.joinable());
            }
        }
    }
}

