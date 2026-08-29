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
#include <array>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <mutex>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/memory/hazard_pointer.hpp>
#include <thread>
#include <vector>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

using namespace rainy::core::memory;

namespace {
    void drain_reclamation() {
        while (hazard_pointer_domain<int>::global().reclaim() > 0) {
        }
    }

    struct stack_node {
        explicit stack_node(const int v) : value(v) {
            live_count.fetch_add(1, rainy::core::concurrency::memory_order_relaxed);
        }

        ~stack_node() {
            live_count.fetch_sub(1, rainy::core::concurrency::memory_order_relaxed);
        }

        int value;
        rainy::core::concurrency::atomic<stack_node*> next{nullptr};
        static rainy::core::concurrency::atomic<int> live_count;
    };
    rainy::core::concurrency::atomic<int> stack_node::live_count{0};
}

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
        threads.emplace_back([&, i] {
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
    std::thread protect_thread([&] {
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
        threads.emplace_back([&] {
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

TEST_CASE("HazardPointer TryProtect") {
    using rainy::core::concurrency::atomic;
    using rainy::core::concurrency::memory_order_relaxed;
    atomic<int*> src{nullptr};
    int a = 1;
    int b = 2;
    auto hp = hazard_pointer_domain<int>::global().acquire();

    // 指针未变化:保护成功
    src.store(&a, memory_order_relaxed);
    int* p = &a;
    REQUIRE(hp.try_protect(p, src));
    REQUIRE(hp.get_protected() == &a);
    REQUIRE(p == &a);

    // 指针已变化:保护失败,不保护新值,且 ptr 被更新为当前值
    src.store(&b, memory_order_relaxed);
    p = &a;
    REQUIRE_FALSE(hp.try_protect(p, src));
    REQUIRE_FALSE(hp.is_protected());
    REQUIRE(p == &b);
}

TEST_CASE("HazardPointer MakeHazardPointer") {
    auto hp = make_hazard_pointer();
    rainy_let ptr = new int(7);
    REQUIRE(hp.protect(ptr) == ptr); // protect 返回实际加载的指针
    REQUIRE(hp.is_protected());
    REQUIRE(hp.get_protected() == ptr);
    hp.reset_protection();
    REQUIRE_FALSE(hp.is_protected());
    delete ptr;
}

TEST_CASE("HazardPointer ProtectReturnsPointer") {
    auto hp = hazard_pointer_domain<int>::global().acquire();
    rainy_let ptr = new int(9);
    REQUIRE(hp.protect(ptr) == ptr);
    REQUIRE(hp.get_protected() == ptr);
    hp.reset_protection();
    delete ptr;
}

TEST_CASE("HazardPointer SlotReuseAfterDestruction") {
    // 析构归还槽位,之后 acquire 应能复用
    {
        auto hp = hazard_pointer_domain<int>::global().acquire();
        REQUIRE_FALSE(hp.is_protected());
    }
    auto hp2 = hazard_pointer_domain<int>::global().acquire();
    REQUIRE_FALSE(hp2.is_protected());
    rainy_let ptr = new int(3);
    hp2.protect(ptr);
    REQUIRE(hp2.is_protected());
    hp2.reset_protection();
    delete ptr;
}

TEST_CASE("HazardPointer SlotExhaustionThrows") {
    // 每线程最多 MAX_HAZARDS_PER_THREAD 个槽位,第 9 个 acquire 应抛出运行时错误
    std::array<hazard_pointer, implements::thread_hazard_list::MAX_HAZARDS_PER_THREAD> hps;
    REQUIRE_THROWS_AS(hazard_pointer_domain<int>::global().acquire(),
                      rainy::core::exceptions::runtime::runtime_error);
}

TEST_CASE("HazardPointer EightSlotsAllProtected") {
    drain_reclamation();
    std::array<hazard_pointer, 8> hps;
    std::array<int*, 8> ptrs{};
    for (std::size_t i = 0; i < hps.size(); ++i) {
        ptrs[i] = new int(static_cast<int>(i));
        hps[i].protect(ptrs[i]);
        REQUIRE(hps[i].is_protected());
    }
    for (auto* p : ptrs) {
        hazard_pointer_domain<int>::global().retire(p);
    }
    // 全部受保护,任何节点都不应被回收
    REQUIRE(hazard_pointer_domain<int>::global().reclaim() == 0);
    // 逐个释放保护并回收
    std::size_t total = 0;
    for (std::size_t i = 0; i < hps.size(); ++i) {
        hps[i].reset_protection();
        total += hazard_pointer_domain<int>::global().reclaim();
    }
    REQUIRE(total == hps.size());
}

TEST_CASE("HazardPointer MoveAssignmentReleasesOldProtection") {
    auto hp1 = hazard_pointer_domain<int>::global().acquire();
    auto hp2 = hazard_pointer_domain<int>::global().acquire();
    rainy_let p1 = new int(1);
    rainy_let p2 = new int(2);
    hp1.protect(p1);
    hp2.protect(p2);

    hp2 = std::move(hp1); // hp2 原有保护(p2)应被释放,并接管 p1
    REQUIRE_FALSE(hp1.is_protected());
    REQUIRE(hp2.is_protected());
    REQUIRE(hp2.get_protected() == p1);

    hp2.reset_protection();
    delete p1;
    delete p2;
}

TEST_CASE("HazardPointer CrossThreadReclamation") {
    drain_reclamation();
    std::thread worker([] {
        // thread_local 使保护持续到线程退出,节点被置入全局孤儿表
        thread_local hazard_pointer tls_hp;
        auto& hp = tls_hp;
        rainy_const ptr = new int(5);
        hp.protect(ptr);
        hazard_pointer_domain<int>::global().retire(ptr);
    });
    worker.join();
    // worker 已退出,其槽位已清空;孤儿节点此时不受保护,应能由其他线程回收
    REQUIRE(implements::global_reclamation_manager::instance().get_orphaned_count() > 0);
    REQUIRE(hazard_pointer_domain<int>::global().reclaim() >= 1);
    REQUIRE(implements::global_reclamation_manager::instance().get_orphaned_count() == 0);
}

TEST_CASE("HazardPointer ActiveThreadCount") {
    using rainy::core::concurrency::atomic;
    using rainy::core::concurrency::memory_order_relaxed;
    atomic<std::size_t> count_in_thread{0};
    std::thread worker([&count_in_thread] {
        auto hp = hazard_pointer_domain<int>::global().acquire(); // 注册线程
        count_in_thread.store(implements::hazard_pointer_registry::instance().get_active_thread_count(),
                              memory_order_relaxed);
    });
    worker.join();
    REQUIRE(count_in_thread.load() >= 1); // 至少包含 worker 自身
}

TEST_CASE("HazardPointer RetireNullNoop") {
    rainy_const before = hazard_pointer_domain<int>::global().get_stats();
    hazard_pointer_domain<int>::global().retire(nullptr); // NOLINT
    rainy_const after = hazard_pointer_domain<int>::global().get_stats();
    REQUIRE(after.objects_retired == before.objects_retired);
}

TEST_CASE("HazardPointer DomainIsolation") {
    // 不同 T 的 domain 相互独立
    rainy_const int_before = hazard_pointer_domain<int>::global().get_stats();
    rainy_const double_before = hazard_pointer_domain<double>::global().get_stats();

    rainy_const ptr = new double(3.14);
    hazard_pointer_domain<double>::global().retire(ptr);

    REQUIRE(hazard_pointer_domain<double>::global().get_stats().objects_retired >= double_before.objects_retired + 1);
    REQUIRE(hazard_pointer_domain<int>::global().get_stats().objects_retired == int_before.objects_retired);

    hazard_pointer_domain<double>::global().reclaim();
}

TEST_CASE("HazardPointer LockFreeStack") {
    // 端到端集成:8 线程基于库 atomic 的无锁栈,pop 使用 try_protect 保护,
    // 验证弹出的值集合完整且最终无泄漏
    using rainy::core::concurrency::atomic;
    using rainy::core::concurrency::memory_order;
    auto& domain = hazard_pointer_domain<stack_node>::global();
    atomic<stack_node*> head{nullptr};
    atomic<int> seq{0};
    atomic<int> popped{0};
    constexpr int threads = 8;
    constexpr int per_threads = 1000;
    constexpr int total = threads * per_threads;

    std::vector<int> all_values;
    std::mutex values_mutex;
    std::vector<std::thread> mythreads;
    mythreads.reserve(threads);
    for (int i = 0; i < threads; ++i) {
        mythreads.emplace_back([&] {
            // push 阶段
            for (int j = 0; j < per_threads; ++j) {
                const int v = seq.fetch_add(1, memory_order::relaxed);
                auto* n = new stack_node(v);
                stack_node* old = head.load(memory_order::relaxed);
                do {
                    n->next.store(old, memory_order::relaxed);
                } while (!head.compare_exchange_weak(old, n, memory_order::acq_rel, memory_order::relaxed));
            }
            // pop 阶段:try_protect 模式弹出并 retire
            std::vector<int> mine;
            mine.reserve(per_threads);
            while (popped.load(memory_order::relaxed) < total) {
                stack_node* n = head.load(memory_order::acquire);
                bool ok = false;
                while (n != nullptr && !ok) {
                    auto hp = domain.acquire();
                    if (hp.try_protect(n, head)) { // 验证 n 仍为栈顶
                        stack_node* next = n->next.load(memory_order::relaxed);
                        if (head.compare_exchange_strong(n, next, memory_order::acq_rel, memory_order::relaxed)) {
                            domain.retire(n);
                            mine.push_back(n->value); // n 仍受 hp 保护,可安全读取
                            ok = true;
                        }
                    }
                }
                if (ok) {
                    popped.fetch_add(1, memory_order::relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
            std::lock_guard<std::mutex> lock(values_mutex);
            all_values.insert(all_values.end(), mine.begin(), mine.end());
        });
    }
    for (auto& t : mythreads) {
        t.join();
    }

    REQUIRE(all_values.size() == total);
    std::vector<char> seen(static_cast<std::size_t>(total), 0);
    for (const int v : all_values) {
        REQUIRE(v >= 0);
        REQUIRE(v < total);
        REQUIRE_FALSE(seen[static_cast<std::size_t>(v)]);
        seen[static_cast<std::size_t>(v)] = 1;
    }

    // 所有节点都应被销毁,无泄漏
    while (domain.reclaim() > 0) {
    }
    REQUIRE(stack_node::live_count.load() == 0);
    rainy_const stats = domain.get_stats();
    REQUIRE(stats.objects_retired == total);
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)
