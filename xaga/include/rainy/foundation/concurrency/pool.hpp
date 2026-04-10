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
#ifndef RAINY_FOUNDATION_CONCURRENCY_POOL_HPP
#define RAINY_FOUNDATION_CONCURRENCY_POOL_HPP
#include <queue>
#include <rainy/foundation/concurrency/atomic.hpp>
#include <rainy/foundation/concurrency/basic/actor.hpp>
#include <rainy/foundation/concurrency/basic/scheduler.hpp>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>

namespace rainy::foundation::concurrency {
    class pooled_actor_pool final : public task_scheduler {
    public:
        explicit pooled_actor_pool(const std::size_t thread_count = std::thread::hardware_concurrency(),
                                   const std::size_t actor_count = 0) :
            thread_count_(thread_count), actor_count_(actor_count == 0 ? thread_count : actor_count), round_robin_(0), stop_(false) {
            utility::expects(thread_count_ > 0);
            utility::expects(actor_count_ >= thread_count_);

            actors_.reserve(actor_count_);
            for (std::size_t i = 0; i < actor_count_; ++i) {
                actors_.emplace_back(foundation::memory::make_nebula<actor_worker>(i));
            }
            // 给每个 actor 注入完成回调——减全局计数，而不是 per-actor 计数
            for (auto &a: actors_) {
                a->set_on_complete([this] {
                    const int done = complete_count_.fetch_add(1, memory_order_acq_rel) + 1;
                    if (const int submitted = submit_count_.load(memory_order_acquire); done == submitted) {
                        lock_guard lk(idle_mutex_);
                        idle_cv_.notify_all();
                    }
                });
            }
            std::vector<actor_worker *> all_peers;
            all_peers.reserve(actor_count_);
            for (auto &a: actors_) {
                all_peers.push_back(a.get());
            }
            for (auto &a: actors_) {
                a->set_peers(all_peers);
            }
            threads_.reserve(thread_count_);
            for (std::size_t i = 0; i < thread_count_; ++i) {
                threads_.emplace_back(
                    foundation::memory::make_nebula<thread>(thread::policy::auto_join, [this, i] { thread_loop(i); }));
            }
        }

        ~pooled_actor_pool() override {
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

        void submit(functional::move_only_delegate<void()> task) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            route(utility::move(task));
        }

        void wait_all() override {
            int target = submit_count_.load(memory_order_acquire);
            if (target == 0) {
                return;
            }
            unique_lock lk(idle_mutex_);
            idle_cv_.wait(lk, [this, target] { return complete_count_.load(memory_order_acquire) >= target; });
        }

        RAINY_NODISCARD scheduler_traits traits() const override {
            return {.is_multi_threaded = true,
                    .is_ordered = false,
                    .supports_stealing = true,
                    .supports_affinity = true,
                    .mode = actor_pool_mode::pooled,
                    .concurrency = thread_count_};
        }

        void submit_to(const std::size_t actor_id, functional::move_only_delegate<void()> task) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            if (actor_id < actor_count_) {
                actors_[actor_id]->submit(utility::move(task)); // NOLINT
            } else {
                route(utility::move(task));
            }
        }

        RAINY_NODISCARD bool stopped() const noexcept override {
            return stop_.load(memory_order_acquire);
        }

        void stop() override {
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

        void join() override {
            this->wait_all();
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
            this->threads_.clear();
        }

    private:
        void route(functional::move_only_delegate<void()> task) {
            std::size_t idx = round_robin_.fetch_add(1, memory_order_relaxed) % actor_count_;
            if (const std::size_t next = (idx + 1) % actor_count_;
                actors_[next]->queue_size() < actors_[idx]->queue_size()) { // NOLINT
                idx = next;
            }
            actors_[idx]->submit(utility::move(task)); // NOLINT
        }

        void thread_loop(const std::size_t thread_idx) {
            const std::size_t preferred = thread_idx % actor_count_;
            while (!stop_.load(memory_order_acquire)) {
                bool did_work = false;
                if (actors_[preferred]->run_once()) { // NOLINT
                    did_work = true;
                } else {
                    for (std::size_t i = 0; i < actor_count_; ++i) {
                        if (i == preferred) {
                            continue;
                        }
                        if (actors_[i]->run_once()) { // NOLINT
                            did_work = true;
                            break;
                        }
                    }
                }

                if (!did_work) {
                    thread::sleep_for(0);
                }
            }
        }

        std::size_t thread_count_;
        std::size_t actor_count_;
        atomic<std::size_t> round_robin_;
        atomic<bool> stop_;

        mutex idle_mutex_;
        condition_variable idle_cv_;

        std::vector<memory::nebula_ptr<actor_worker>> actors_;
        std::vector<memory::nebula_ptr<thread>> threads_;
        atomic<int> submit_count_{0};
        atomic<int> complete_count_{0};
    };
}

namespace rainy::foundation::concurrency {
    class dedicated_actor_pool final : public task_scheduler {
    public:
        explicit dedicated_actor_pool(const std::size_t actor_count = std::thread::hardware_concurrency()) :
            actor_count_(actor_count), round_robin_(0), submit_count_(0), complete_count_(0) {
            utility::expects(actor_count_ > 0);

            actors_.reserve(actor_count_);
            for (std::size_t i = 0; i < actor_count_; ++i) {
                actors_.emplace_back(foundation::memory::make_nebula<actor_worker>(i));
            }

            for (auto &a: actors_) {
                a->set_on_complete([this] {
                    const int done = complete_count_.fetch_add(1, memory_order_acq_rel) + 1;
                    if (const int submitted = submit_count_.load(memory_order_acquire); done == submitted) {
                        lock_guard lk(idle_mutex_);
                        idle_cv_.notify_all();
                    }
                });
            }
            std::vector<actor_worker *> all_peers;
            all_peers.reserve(actor_count_);
            for (auto &a: actors_) {
                all_peers.push_back(a.get());
            }
            for (auto &a: actors_) {
                a->start_dedicated({}); // ← 空 peers，禁止 stealing
            }
        }

        ~dedicated_actor_pool() override {
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

        void submit(functional::move_only_delegate<void()> task) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            route(utility::move(task));
        }

        void submit_to(const std::size_t actor_id, functional::move_only_delegate<void()> task) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            if (actor_id < actor_count_) {
                actors_[actor_id]->submit(utility::move(task)); // NOLINT
            } else {
                route(utility::move(task));
            }
        }

        void wait_all() override {
            int target = submit_count_.load(memory_order_acquire);
            if (target == 0) {
                return;
            }
            unique_lock lk(idle_mutex_);
            idle_cv_.wait(lk, [this, target] { return complete_count_.load(memory_order_acquire) >= target; });
        }

        RAINY_NODISCARD scheduler_traits traits() const override {
            return {.is_multi_threaded = true,
                    .is_ordered = true,
                    .supports_stealing = false, // ← dedicated 不偷
                    .supports_affinity = true,
                    .mode = actor_pool_mode::dedicated,
                    .concurrency = actor_count_};
        }

        RAINY_NODISCARD std::size_t actor_count() const noexcept {
            return actor_count_;
        }

        RAINY_NODISCARD bool stopped() const noexcept override {
            return stop_.load(memory_order_acquire);
        }

        void stop() override {
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

        void join() override {
            this->wait_all();
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

    private:
        void route(functional::move_only_delegate<void()> task) {
            std::size_t idx = round_robin_.fetch_add(1, memory_order_relaxed) % actor_count_;
            if (const std::size_t next = (idx + 1) % actor_count_;
                actors_[next]->queue_size() < actors_[idx]->queue_size()) { // NOLINT
                idx = next;
            }
            actors_[idx]->submit(utility::move(task)); // NOLINT
        }

        std::size_t actor_count_;
        atomic<std::size_t> round_robin_;
        atomic<int> submit_count_;
        atomic<int> complete_count_;
        atomic<bool> stop_{false};
        mutex idle_mutex_;
        condition_variable idle_cv_;
        std::vector<memory::nebula_ptr<actor_worker>> actors_;
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief pinned_actor_pool
     *
     * 设计约束：
     *   - actor 与 thread 一一静态绑定（pinned），actor 不迁移
     *   - 每个线程只处理自己 pinned 的 actor，不做 work stealing
     *   - 线程属于共享 pool，可通过 submit() 经 round-robin+轻载 路由
     *   - 支持 submit_to(actor_id, task) 直接投递到指定 actor
     *   - actor_count == thread_count，保证一一对应
     */
    class pinned_actor_pool final : public task_scheduler {
    public:
        explicit pinned_actor_pool(const std::size_t thread_count = std::thread::hardware_concurrency()) :
            thread_count_(thread_count), round_robin_(0), stop_(false) {
            utility::expects(thread_count_ > 0);

            // actor_count == thread_count，严格一一对应
            actors_.reserve(thread_count_);
            for (std::size_t i = 0; i < thread_count_; ++i) {
                actors_.emplace_back(foundation::memory::make_nebula<actor_worker>(i));
            }

            // 完成回调：全局计数，与 pooled/dedicated 保持一致
            for (auto &a: actors_) {
                a->set_on_complete([this] {
                    const int done = complete_count_.fetch_add(1, memory_order_acq_rel) + 1;
                    if (const int submitted = submit_count_.load(memory_order_acquire); done == submitted) {
                        lock_guard lk(idle_mutex_);
                        idle_cv_.notify_all();
                    }
                });
            }

            threads_.reserve(thread_count_);
            for (std::size_t i = 0; i < thread_count_; ++i) {
                // 每个线程 i 只服务 actors_[i]，pinned 绑定
                threads_.emplace_back(
                    foundation::memory::make_nebula<thread>(thread::policy::auto_join, [this, i] { thread_loop(i); }));
            }
        }

        ~pinned_actor_pool() override {
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

        /**
         * @brief 提交任务到 pool，经 round-robin + 轻载选择路由到某个 pinned actor
         */
        void submit(functional::move_only_delegate<void()> task) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            route(utility::move(task));
        }

        /**
         * @brief 直接投递到指定 actor（即固定线程），越过路由
         *        actor_id 越界时退化为 route()
         */
        void submit_to(const std::size_t actor_id, functional::move_only_delegate<void()> task) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            if (actor_id < thread_count_) {
                actors_[actor_id]->submit(utility::move(task)); // NOLINT
            } else {
                route(utility::move(task));
            }
        }

        void wait_all() override {
            const int target = submit_count_.load(memory_order_acquire);
            if (target == 0) {
                return;
            }
            unique_lock lk(idle_mutex_);
            idle_cv_.wait(lk, [this, target] { return complete_count_.load(memory_order_acquire) >= target; });
        }

        RAINY_NODISCARD scheduler_traits traits() const override {
            return {.is_multi_threaded = true,
                    .is_ordered = true,
                    .supports_stealing = false,
                    .supports_affinity = true,
                    .mode = actor_pool_mode::pinned,
                    .concurrency = thread_count_};
        }

        RAINY_NODISCARD std::size_t thread_count() const noexcept {
            return thread_count_;
        }

        bool stopped() const noexcept override {
            return stop_.load(memory_order_acquire);
        }

        void stop() override {
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
        }

        void join() override {
            this->wait_all();
            stop_.store(true, memory_order_release);
            for (auto &a: actors_) {
                a->signal_stop();
            }
            for (auto &t: threads_) {
                if (t) {
                    t->join();
                }
            }
        }

    private:
        void route(functional::move_only_delegate<void()> task) {
            std::size_t idx = round_robin_.fetch_add(1, memory_order_relaxed) % thread_count_;
            if (const std::size_t next = (idx + 1) % thread_count_;
                actors_[next]->queue_size() < actors_[idx]->queue_size()) { // NOLINT
                idx = next;
            }
            actors_[idx]->submit(utility::move(task)); // NOLINT
        }

        void thread_loop(const std::size_t thread_idx) {
            actor_worker *pinned = actors_[thread_idx].get(); // NOLINT
            while (!stop_.load(memory_order_acquire)) {
                if (!pinned->run_once()) { // NOLINT
                    // 本 actor 暂无任务，短暂让出 CPU
                    thread::sleep_for(0);
                }
            }
        }

        std::size_t thread_count_;
        atomic<std::size_t> round_robin_;
        atomic<bool> stop_;

        mutex idle_mutex_;
        condition_variable idle_cv_;

        std::vector<memory::nebula_ptr<actor_worker>> actors_;
        std::vector<memory::nebula_ptr<thread>> threads_;
        atomic<int> submit_count_{0};
        atomic<int> complete_count_{0};
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief priority_actor_pool
     *
     * 设计约束：
     *   - actor 按优先级分为三层（high / normal / low）
     *   - submit(task, priority) 将任务路由到对应层的 actor
     *   - 线程 loop 按加权频率扫描：high > normal > low
     *   - low 层不会饿死：每 4 轮至少被检查一次
     *   - 层内支持 work stealing（同层 actor 间）
     *   - 跨层不 steal，保证优先级语义不被破坏
     */
    class priority_actor_pool final : public task_scheduler {
    public:
        /**
         * @param thread_count   工作线程数
         * @param actors_per_tier 每个优先级层的 actor 数（默认 == thread_count）
         *
         * 总 actor 数 = actors_per_tier * 3
         * actors_per_tier >= 1，建议 >= thread_count / 3
         */
        explicit priority_actor_pool(const std::size_t thread_count = std::thread::hardware_concurrency(),
                                     const std::size_t actors_per_tier = 0) :
            thread_count_(thread_count), actors_per_tier_(actors_per_tier == 0 ? thread_count : actors_per_tier), stop_(false) {
            utility::expects(thread_count_ > 0);
            utility::expects(actors_per_tier_ > 0);

            // 初始化三层 actor
            for (std::size_t tier = 0; tier < actor_priority_levels; ++tier) {
                // NOLINTBEGIN
                auto &tier_actors = actors_[tier];
                auto &rr = round_robin_[tier];
                // NOLINTEND
                rr.store(0, memory_order_relaxed);

                tier_actors.reserve(actors_per_tier_);
                // actor id 全局唯一：tier * actors_per_tier_ + i
                for (std::size_t i = 0; i < actors_per_tier_; ++i) {
                    const std::size_t global_id = tier * actors_per_tier_ + i;
                    tier_actors.emplace_back(foundation::memory::make_nebula<actor_worker>(global_id));
                }

                // 完成回调
                for (auto &a: tier_actors) {
                    a->set_on_complete([this] {
                        const int done = complete_count_.fetch_add(1, memory_order_acq_rel) + 1;
                        if (const int submitted = submit_count_.load(memory_order_acquire); done == submitted) {
                            lock_guard lk(idle_mutex_);
                            idle_cv_.notify_all();
                        }
                    });
                }

                // 同层 peers 注入，支持层内 stealing
                std::vector<actor_worker *> peers;
                peers.reserve(actors_per_tier_);
                for (auto &a: tier_actors) {
                    peers.push_back(a.get());
                }
                for (auto &a: tier_actors) {
                    a->set_peers(peers);
                }
            }

            // 启动线程
            threads_.reserve(thread_count_);
            for (std::size_t i = 0; i < thread_count_; ++i) {
                threads_.emplace_back(
                    foundation::memory::make_nebula<thread>(thread::policy::auto_join, [this, i] { thread_loop(i); }));
            }
        }

        ~priority_actor_pool() override {
            stop_.store(true, memory_order_release);
            for (auto &tier_actors: actors_) {
                for (auto &a: tier_actors) {
                    a->signal_stop();
                }
            }
        }

        /**
         * @brief 提交任务，使用默认优先级 normal
         */
        void submit(functional::move_only_delegate<void()> task) override {
            submit(utility::move(task), actor_priority::normal);
        }

        /**
         * @brief 提交任务，指定优先级层
         * @param task
         * @param priority  调度 hint，决定路由到哪个优先级层
         */
        void submit(functional::move_only_delegate<void()> task, const actor_priority priority) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            route(utility::move(task), priority);
        }

        /**
         * @brief 数值优先级重载，0 == high，1 == normal，>= 2 == low
         */
        void submit(functional::move_only_delegate<void()> task, const std::size_t priority_value) {
            const actor_priority p = priority_value == 0   ? actor_priority::high
                                     : priority_value == 1 ? actor_priority::normal
                                                           : actor_priority::low;
            submit(utility::move(task), p);
        }

        /**
         * @brief 直接投递到指定层的指定 actor
         *        actor_id 是层内索引，越界退化为 route()
         */
        void submit_to(const std::size_t actor_id, functional::move_only_delegate<void()> task,
                       const actor_priority priority) override {
            submit_count_.fetch_add(1, memory_order_relaxed);
            const auto tier = static_cast<std::size_t>(priority);
            if (actor_id < actors_per_tier_) {
                actors_[tier][actor_id]->submit(utility::move(task)); // NOLINT
            } else {
                route(utility::move(task), priority);
            }
        }

        void submit_to(const std::size_t actor_id, functional::move_only_delegate<void()> task) override {
            return submit_to(actor_id, utility::move(task), actor_priority::normal);
        }

        void wait_all() override {
            const int target = submit_count_.load(memory_order_acquire);
            if (target == 0) {
                return;
            }
            unique_lock lk(idle_mutex_);
            idle_cv_.wait(lk, [this, target] { return complete_count_.load(memory_order_acquire) >= target; });
        }

        RAINY_NODISCARD scheduler_traits traits() const override {
            return {.is_multi_threaded = true,
                    .is_ordered = false,
                    .supports_stealing = true, // 层内 stealing
                    .supports_affinity = true,
                    .mode = actor_pool_mode::priority,
                    .concurrency = thread_count_};
        }

        RAINY_NODISCARD std::size_t actors_per_tier() const noexcept {
            return actors_per_tier_;
        }

        bool stopped() const noexcept override {
            return stop_.load(memory_order_acquire);
        }

        void stop() override {
            stop_.store(true, memory_order_release);
            for (auto &tier_actors: actors_) {
                for (auto &a: tier_actors) {
                    a->signal_stop();
                }
            }
        }

        void join() override {
            this->wait_all();
            stop_.store(true, memory_order_release);
            for (auto &tier_actors: actors_) {
                for (auto &a: tier_actors) {
                    a->signal_stop();
                }
            }

            for (auto &t: threads_) {
                if (t) {
                    t->join();
                }
            }
        }

    private:
        /**
         * @brief 层内 round-robin + 轻载修正路由
         */
        void route(functional::move_only_delegate<void()> task, const actor_priority priority) {
            const auto tier = static_cast<std::size_t>(priority);
            // NOLINTBEGIN
            auto &rr = round_robin_[tier];
            auto &pool = actors_[tier];

            std::size_t idx = rr.fetch_add(1, memory_order_relaxed) % actors_per_tier_;
            const std::size_t next = (idx + 1) % actors_per_tier_;
            if (pool[next]->queue_size() < pool[idx]->queue_size()) {
                idx = next;
            }
            pool[idx]->submit(utility::move(task));
            // NOLINTEND
        }

        /**
         * @brief 线程主循环：按优先级加权频率扫描三层
         *
         * 加权策略（基于 tick 计数器）：
         *
         *   tick mod 4 == 0,1,2,3  → 检查 high    （每轮必查，频率 4/4）
         *   tick mod 4 == 0,2      → 检查 normal  （每 2 轮查一次，频率 2/4）
         *   tick mod 4 == 0        → 检查 low     （每 4 轮查一次，频率 1/4）
         *
         * (eg. )（tick 0..7）：
         *
         *   tick: 0  1  2  3  4  5  6  7
         *   high: ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
         *   norm: ✓  .  ✓  .  ✓  .  ✓  .
         *   low:  ✓  .  .  .  ✓  .  .  .
         *
         * 层内扫描从 preferred actor 开始，找到一个可运行即 break（不贪婪）。
         */
        void thread_loop(const std::size_t thread_idx) {
            // 每个线程在各层都有一个"偏好"起点，分散负载
            const std::size_t preferred_base = thread_idx % actors_per_tier_;
            std::uint64_t tick = 0;

            while (!stop_.load(memory_order_acquire)) {
                bool did_work = false;
                for (std::size_t tier = 0; tier < actor_priority_levels; ++tier) {
                    // 按频率掩码决定本 tick 检查哪些层
                    //   high   : 每 tick 都检查          (tick & 0) == 0  → always
                    //   normal : 每 2 tick 检查一次       (tick & 1) == 0
                    //   low    : 每 4 tick 检查一次       (tick & 3) == 0
                    constexpr std::uint64_t check_mask[actor_priority_levels] = {
                        0x0, // high:   tick & 0x0 == 0 → always true
                        0x1, // normal: tick & 0x1 == 0 → every 2 ticks
                        0x3, // low:    tick & 0x3 == 0 → every 4 ticks
                    };
                    if ((tick & check_mask[tier]) != 0) { // NOLINT
                        // 本轮跳过此层
                        continue;
                    }
                    auto &pool = actors_[tier]; // NOLINT
                    // 从偏好位置开始轮转扫描，找到一个可运行的 actor 即执行并 break
                    for (std::size_t off = 0; off < actors_per_tier_; ++off) {
                        const std::size_t idx = (preferred_base + off) % actors_per_tier_; // NOLINT
                        if (pool[idx]->run_once()) { // NOLINT
                            did_work = true;
                            break; // 不贪婪：执行一个后回到外层，重新从 high 开始
                        }
                    }
                    // 高优先级有任务时，本轮不再继续检查低层
                    // （只有高层全空才下探）
                    if (did_work) {
                        break;
                    }
                }
                if (!did_work) {
                    thread::sleep_for(0);
                }
                ++tick;
            }
        }

        std::size_t thread_count_;
        std::size_t actors_per_tier_;
        atomic<bool> stop_;

        mutex idle_mutex_;
        condition_variable idle_cv_;

        std::array<std::vector<memory::nebula_ptr<actor_worker>>, actor_priority_levels> actors_;
        std::array<atomic<std::size_t>, actor_priority_levels> round_robin_;

        std::vector<memory::nebula_ptr<thread>> threads_;
        atomic<int> submit_count_{0};
        atomic<int> complete_count_{0};
    };
}

namespace rainy::foundation::concurrency {
    class blocking_actor_pool final : public task_scheduler {
    public:
        explicit blocking_actor_pool(const std::size_t base_threads = 2,
                                     const std::size_t max_threads = std::thread::hardware_concurrency() * 2) :
            base_threads_(base_threads), max_threads_(max_threads), stop_(false), active_threads_(0), pending_tasks_(0) {
            utility::expects(base_threads_ > 0);
            utility::expects(max_threads_ >= base_threads_);
            for (std::size_t i = 0; i < base_threads_; ++i) {
                spawn_thread();
            }
        }

        ~blocking_actor_pool() override {
            stop_.store(true, memory_order_release);
            {
                lock_guard lk(queue_mutex_);
                queue_cv_.notify_all();
            }
            lock_guard lk(threads_mutex_);
            threads_.clear();
        }

        void submit(functional::move_only_delegate<void()> task) override {
            pending_tasks_.fetch_add(1, memory_order_relaxed);
            {
                lock_guard lk(queue_mutex_);
                task_queue_.push(utility::move(task));
                queue_cv_.notify_one();
            }
            check_expand_pool();
        }

        void submit_to(std::size_t actor_id, functional::move_only_delegate<void()> task) override {
            (void) actor_id;
            submit(utility::move(task));
        }

        void wait_all() override {
            unique_lock lk(idle_mutex_);
            idle_cv_.wait(lk, [this] {
                int p = pending_tasks_.load(memory_order_acquire);
                int c = complete_count_.load(memory_order_acquire);
                return p == 0 || c >= p;
            });
            pending_tasks_.store(0, memory_order_release);
            complete_count_.store(0, memory_order_release);
        }

        RAINY_NODISCARD scheduler_traits traits() const override {
            return {.is_multi_threaded = true,
                    .is_ordered = false,
                    .supports_stealing = false,
                    .supports_affinity = false,
                    .mode = actor_pool_mode::blocking,
                    .concurrency = active_threads_.load(memory_order_relaxed)};
        }

        void submit_blocking(functional::move_only_delegate<void()> task) {
            submit(utility::move(task));
        }

        bool stopped() const noexcept override {
            return stop_.load(memory_order_acquire);
        }

        void stop() override {
            stop_.store(true, memory_order_release);
            {
                lock_guard lk(queue_mutex_);
                queue_cv_.notify_all();
            }
        }

        void join() override {
            this->wait_all();
            stop_.store(true, memory_order_release);
            {
                lock_guard lk(queue_mutex_);
                queue_cv_.notify_all();
            }

            lock_guard lk(threads_mutex_);
            for (auto &t: threads_) {
                if (t) {
                    t->join();
                }
            }
        }

    private:
        void spawn_thread() {
            auto thread_ptr = memory::make_nebula<thread>(thread::policy::auto_join, [this] { thread_loop(); });
            {
                lock_guard lk(threads_mutex_);
                threads_.push_back(std::move(thread_ptr));
            }
            active_threads_.fetch_add(1, memory_order_relaxed);
        }

        void thread_loop() {
            while (!stop_.load(memory_order_acquire)) {
                functional::move_only_delegate<void()> task;
                bool has_task = false;
                {
                    unique_lock lk(queue_mutex_);
                    queue_cv_.wait(lk, [this] {
                        return !task_queue_.empty() || stop_.load(memory_order_acquire);
                    });
                    if (!task_queue_.empty()) {
                        task = std::move(task_queue_.front());
                        task_queue_.pop();
                        has_task = true;
                    }
                }
                if (has_task) {
                    task();
                    const int done = complete_count_.fetch_add(1, memory_order_acq_rel) + 1;
                    const int pending = pending_tasks_.load(memory_order_acquire);
                    if (done == pending) {
                        lock_guard lk(idle_mutex_);
                        idle_cv_.notify_all();
                    }
                }
            }
        }

        void check_expand_pool() {
            std::size_t current_active = active_threads_.load(memory_order_relaxed);
            if (current_active >= max_threads_) {
                return;
            }
            std::size_t queue_size;
            {
                lock_guard lk(queue_mutex_);
                queue_size = task_queue_.size();
            }
            if (queue_size > current_active) {
                std::size_t target = (core::min) (max_threads_, queue_size);
                for (std::size_t i = current_active; i < target; ++i) {
                    spawn_thread();
                }
            }
        }

        const std::size_t base_threads_;
        const std::size_t max_threads_;
        atomic<bool> stop_;

        mutable mutex queue_mutex_;
        condition_variable queue_cv_;
        std::queue<functional::move_only_delegate<void()>> task_queue_;

        mutable mutex threads_mutex_;
        std::vector<memory::nebula_ptr<thread>> threads_;

        mutable mutex idle_mutex_;
        condition_variable idle_cv_;

        atomic<std::size_t> active_threads_;
        atomic<int> pending_tasks_{0};
        atomic<int> complete_count_{0};
    };
}

#endif
