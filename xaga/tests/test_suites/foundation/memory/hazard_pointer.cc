/*
* Copyright 2025 rainy-juzixiao
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
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <rainy/foundation/memory/hazard_pointer.hpp>
#include <thread>
#include <vector>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

using namespace rainy::foundation::memory;

TEST_CASE("HazardPointer BasicProtection") {
    auto hp = hazard_pointer_domain<int>::global().acquire();
    rainy_let ptr = new int(42);

    hp.protect(ptr);
    REQUIRE(hp.get_protected() == ptr);
    REQUIRE(hp.is_protected());

    hp.reset_protection();
    REQUIRE(hp.get_protected() == nullptr);
    REQUIRE_FALSE(hp.is_protected());

    delete ptr;
}

TEST_CASE("HazardPointer RetireWithoutProtection") {
    rainy_const ptr = new int(42);
    hazard_pointer_domain<int>::global().retire(ptr);

    const auto reclaimed = hazard_pointer_domain<int>::global().reclaim();
    REQUIRE(reclaimed >= 1);
}

TEST_CASE("HazardPointer ProtectionPreventsReclaim") {
    auto hp = hazard_pointer_domain<int>::global().acquire();
    rainy_let ptr = new int(42);

    hp.protect(ptr);
    hazard_pointer_domain<int>::global().retire(ptr);
    REQUIRE(hp.get_protected() == ptr);

    hp.reset_protection();
    rainy_const reclaimed = hazard_pointer_domain<int>::global().reclaim();
    REQUIRE(reclaimed >= 1);
}

TEST_CASE("HazardPointer MultipleHazardPointers") {
    auto hp1 = hazard_pointer_domain<int>::global().acquire();
    auto hp2 = hazard_pointer_domain<int>::global().acquire();

    rainy_let ptr1 = new int(1);
    rainy_let ptr2 = new int(2);

    hp1.protect(ptr1);
    hp2.protect(ptr2);

    REQUIRE(hp1.get_protected() == ptr1);
    REQUIRE(hp2.get_protected() == ptr2);

    hazard_pointer_domain<int>::global().retire(ptr1);
    hazard_pointer_domain<int>::global().retire(ptr2);
    REQUIRE(hp1.get_protected() == ptr1);
    REQUIRE(hp2.get_protected() == ptr2);

    hp1.reset_protection();
    rainy_let reclaimed = hazard_pointer_domain<int>::global().reclaim();
    REQUIRE(reclaimed >= 1);

    hp2.reset_protection();
    reclaimed = hazard_pointer_domain<int>::global().reclaim();
    REQUIRE(reclaimed >= 1);
}

TEST_CASE("HazardPointer MoveConstructor") {
    hazard_pointer hp1;
    rainy_let ptr = new int(42);
    hp1.protect(ptr);

    hazard_pointer hp2(std::move(hp1));

    REQUIRE_FALSE(hp1.is_protected());
    REQUIRE(hp2.is_protected());
    REQUIRE(hp2.get_protected() == ptr);

    hp2.reset_protection();
    delete ptr;
}

TEST_CASE("HazardPointer MoveAssignment") {
    hazard_pointer hp1;
    rainy_let ptr = new int(42);
    hp1.protect(ptr);

    hazard_pointer hp2 = std::move(hp1);

    REQUIRE_FALSE(hp1.is_protected());
    REQUIRE(hp2.is_protected());
    REQUIRE(hp2.get_protected() == ptr);

    hp2.reset_protection();
    delete ptr;
}

TEST_CASE("HazardPointer Statistics") {
    rainy_const initial_stats = hazard_pointer_domain<int>::global().get_stats();

    rainy_const ptr1 = new int(1);
    rainy_const ptr2 = new int(2);

    hazard_pointer_domain<int>::global().retire(ptr1);
    hazard_pointer_domain<int>::global().retire(ptr2);

    rainy_const after_retire_stats = hazard_pointer_domain<int>::global().get_stats();
    REQUIRE(after_retire_stats.objects_retired >= initial_stats.objects_retired + 2);

    hazard_pointer_domain<int>::global().reclaim();

    rainy_const after_reclaim_stats = hazard_pointer_domain<int>::global().get_stats();
    REQUIRE(after_reclaim_stats.scan_count > initial_stats.scan_count);
    REQUIRE(after_reclaim_stats.objects_reclaimed >= initial_stats.objects_reclaimed + 2);
}

TEST_CASE("HazardPointer ConcurrentRetirement") {
    constexpr int NUM_THREADS = 4;
    constexpr int OBJECTS_PER_THREAD = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([i] {
            for (int j = 0; j < OBJECTS_PER_THREAD; ++j) {
                rainy_const ptr = new int(i * OBJECTS_PER_THREAD + j);
                hazard_pointer_domain<int>::global().retire(ptr);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    hazard_pointer_domain<int>::global().reclaim();

    rainy_const stats = hazard_pointer_domain<int>::global().get_stats();
    REQUIRE(stats.objects_retired >= NUM_THREADS * OBJECTS_PER_THREAD);
}

TEST_CASE("HazardPointer ConcurrentProtectionAndRetirement") {
    constexpr int NUM_ITERATIONS = 100;
    std::atomic_bool stop{false};
    std::thread retire_thread([&stop] {
        int counter = 0;
        while (!stop.load(std::memory_order_acquire)) {
            rainy_const ptr = new int(counter++);
            hazard_pointer_domain<int>::global().retire(ptr);
            if (counter % 10 == 0) {
                hazard_pointer_domain<int>::global().reclaim();
            }
        }
    });
    std::thread protect_thread([] {
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            auto hp = hazard_pointer_domain<int>::global().acquire();
            rainy_const ptr = new int(1000 + i);
            hp.protect(ptr);
            std::this_thread::yield();
            hp.reset_protection();
            delete ptr;
        }
    });
    protect_thread.join();
    stop.store(true, std::memory_order_release);
    retire_thread.join();
    hazard_pointer_domain<int>::global().reclaim();
}

TEST_CASE("HazardPointer CustomType") {
    struct TestNode {
        explicit TestNode(const int v) : value(v), next(nullptr) {}

        int value;
        std::atomic<TestNode*> next; // NOLINT
    };
    auto hp = hazard_pointer_domain<TestNode>::global().acquire();
    rainy_let node = new TestNode(42);
    hp.protect(node);
    REQUIRE(hp.get_protected() == node);
    hazard_pointer_domain<TestNode>::global().retire(node);
    auto reclaimed = hazard_pointer_domain<TestNode>::global().reclaim();
    REQUIRE(reclaimed == 0);
    hp.reset_protection();
    reclaimed = hazard_pointer_domain<TestNode>::global().reclaim();
    REQUIRE(reclaimed >= 1);
}

TEST_CASE("HazardPointer AutomaticReclamation") {
    rainy_const initial_stats = hazard_pointer_domain<int>::global().get_stats();
    constexpr int NUM_OBJECTS = 100;  // Should exceed RECLAIM_THRESHOLD
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        rainy_const ptr = new int(i);
        hazard_pointer_domain<int>::global().retire(ptr);
    }
    rainy_const after_stats = hazard_pointer_domain<int>::global().get_stats();
    REQUIRE(after_stats.scan_count > initial_stats.scan_count);
}

TEST_CASE("HazardPointer RAIIBehavior") {
    {
        rainy_const ptr = new int(42);
        auto hp = hazard_pointer_domain<int>::global().acquire();
        hp.protect(ptr);
        REQUIRE(hp.is_protected());
        hazard_pointer_domain<int>::global().retire(ptr);
        hazard_pointer_domain<int>::global().reclaim();
        rainy_const stats_during = hazard_pointer_domain<int>::global().get_stats();
        rainy_const pending = stats_during.objects_retired - stats_during.objects_reclaimed;
        REQUIRE(pending > 0);
    }
    rainy_const reclaimed = hazard_pointer_domain<int>::global().reclaim();
    REQUIRE(reclaimed >= 1);
}

TEST_CASE("HazardPointer StressTest") {
    constexpr int NUM_THREADS = 8;
    constexpr int ITERATIONS = 1000;
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([i] {
            for (int j = 0; j < ITERATIONS; ++j) {
                auto hp = hazard_pointer_domain<int>::global().acquire();
                rainy_const ptr = new int(i * ITERATIONS + j);
                hp.protect(ptr);
                std::this_thread::yield();
                hazard_pointer_domain<int>::global().retire(ptr);
                hp.reset_protection();
                if (j % 100 == 0) {
                    hazard_pointer_domain<int>::global().reclaim();
                }
            }
        });
    }
    for (auto &t: threads) {
        t.join();
    }
    for (int i = 0; i < 10; ++i) {
        hazard_pointer_domain<int>::global().reclaim();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)
