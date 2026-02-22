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
#ifndef RAINY_FOUNDATION_CONCURRENCY_ACTOR_HPP
#define RAINY_FOUNDATION_CONCURRENCY_ACTOR_HPP
#include <functional>
#include <rainy/foundation/functional/delegate.hpp>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <rainy/foundation/concurrency/thread.hpp>
#include <rainy/foundation/concurrency/work_stealing_deque.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>

namespace rainy::foundation::concurrency {
    class actor_worker {
    public:
        using task_type = functional::delegate<void()>;
        using on_complete = functional::delegate<void()>;

        explicit actor_worker(const std::size_t id) : id_(id), stop_(false) {
        }

        ~actor_worker() {
            signal_stop();
            worker_thread_.reset();
        }

        void start_dedicated(std::vector<actor_worker *> peers) {
            utility::expects(!on_complete_.empty(), "set_on_complete must be called before start_dedicated");
            peers_ = utility::move(peers);
            worker_thread_ = foundation::memory::make_nebula<thread>(thread::policy::auto_join, [this] { run_loop(); });
        }

        void set_peers(std::vector<actor_worker *> peers) {
            peers_ = utility::move(peers);
        }

        // 注入完成回调（pooled 模式由 pool 层设置）
        void set_on_complete(on_complete cb) {
            on_complete_ = utility::move(cb);
        }

        void submit(task_type task) {
            local_queue_.push(utility::move(task));
            cv_.notify_one();
        }

        // 模式A 专用：外部线程驱动，执行一个任务
        bool run_once() {
            if (auto t = local_queue_.pop()) {
                execute(utility::move(*t));
                return true;
            }
            for (auto *peer: peers_) {
                if (peer == this) {
                    continue;
                }
                if (auto t = peer->local_queue_.steal()) {
                    execute(utility::move(*t));
                    return true;
                }
            }
            return false;
        }

        void signal_stop() {
            stop_.store(true, std::memory_order_release);
            cv_.notify_all();
        }

        std::size_t id() const noexcept {
            return id_;
        }
        
        std::size_t queue_size() const {
            return local_queue_.size();
        }
        
        work_stealing_deque &queue() noexcept {
            return local_queue_;
        }

    private:
        void run_loop() {
            while (true) {
                if (auto t = local_queue_.pop()) {
                    execute(utility::move(*t));
                    continue;
                }
                bool stolen = false;
                for (auto *peer: peers_) {
                    if (peer == this) {
                        continue;
                    }
                    if (auto t = peer->local_queue_.steal()) {
                        execute(utility::move(*t));
                        stolen = true;
                        break;
                    }
                }
                if (stolen) {
                    continue;
                }
                {
                    unique_lock lk(cv_mutex_);
                    cv_.wait_for(lk, std::chrono::microseconds(100),
                                 [this] { return !local_queue_.empty() || stop_.load(std::memory_order_acquire); });
                }
                if (stop_.load(std::memory_order_acquire) && local_queue_.empty()) {
                    break;
                }
            }
        }

        void execute(const task_type &task) const {
            task();
            // 通知 pool 层：一个任务执行完毕
            if (on_complete_) {
                on_complete_();
            }
        }

        std::size_t id_;
        std::atomic<bool> stop_;
        work_stealing_deque local_queue_;
        std::vector<actor_worker *> peers_;
        on_complete on_complete_;

        mutex cv_mutex_;
        condition_variable cv_;
        memory::nebula_ptr<thread> worker_thread_;
    };
}

#endif
