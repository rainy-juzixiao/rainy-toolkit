#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <thread>

using namespace rainy::foundation::concurrency;
using namespace std::chrono_literals;

SCENARIO("Constructing and destructing condition_variable", "[condition_variable]") {
    GIVEN("A newly constructed condition variable") {
        condition_variable cv; // NOLINT

        THEN("It has a valid native handle") {
            REQUIRE(cv.native_handle() != nullptr);
        }
    }
}

SCENARIO("Wait and notify_one functionality", "[condition_variable]") {
    GIVEN("A condition variable and a mutex") {
        condition_variable cv;
        mutex mtx;
        std::atomic<bool> ready{false};

        WHEN("A thread waits and another thread calls notify_one") {
            std::thread t([&]() {
                unique_lock<mutex> lock(mtx);
                ready = true;
                cv.wait(lock);
            });

            // 等待子线程进入 wait
            while (!ready) std::this_thread::yield();

            THEN("notify_one unblocks the waiting thread") {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                cv.notify_one();
                t.join();
                REQUIRE(t.joinable() == false);
            }
        }
    }
}

SCENARIO("Wait and notify_all functionality", "[condition_variable]") {
    GIVEN("A condition variable and multiple threads waiting") {
        condition_variable cv;
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

        // 等待线程进入 wait
        while (!ready1 || !ready2) std::this_thread::yield();

        WHEN("notify_all is called") {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            cv.notify_all();

            THEN("All threads are unblocked") {
                t1.join();
                t2.join();
                REQUIRE(t1.joinable() == false);
                REQUIRE(t2.joinable() == false);
            }
        }
    }
}