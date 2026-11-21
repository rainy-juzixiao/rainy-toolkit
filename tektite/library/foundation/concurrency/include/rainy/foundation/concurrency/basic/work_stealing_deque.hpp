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
#ifndef RAINY_FOUNDATION_CONCURRENCY_WORK_STEALING_QUEUE_HPP
#define RAINY_FOUNDATION_CONCURRENCY_WORK_STEALING_QUEUE_HPP
#include <deque>
#include <rainy/foundation/concurrency/mutex.hpp>

namespace rainy::foundation::concurrency {
    class work_stealing_deque {
    public:
        using task_type = functional::delegate<void()>;

        void push(task_type task) {
            lock_guard lk(mutex_);
            deque_.push_back(std::move(task));
        }

        std::optional<task_type> pop() {
            lock_guard lk(mutex_);
            if (deque_.empty()) {
                return std::nullopt;
            }
            auto task = std::move(deque_.front());
            deque_.pop_front();
            return task;
        }

        std::optional<task_type> steal() {
            if (!mutex_.try_lock()) {
                return std::nullopt; // 拿不到锁直接放弃，下次再试
            }
            lock_guard lk(mutex_, adopt_lock);
            if (deque_.empty()) {
                return std::nullopt;
            }
            auto task = std::move(deque_.back()); // 从 back 偷，与 pop 的 front 错开，减少竞争
            deque_.pop_back();
            return task;
        }

        bool empty() const {
            lock_guard lk(mutex_);
            return deque_.empty();
        }

        std::size_t size() const {
            lock_guard lk(mutex_);
            return deque_.size();
        }

    private:
        mutable mutex mutex_;
        std::deque<task_type> deque_;
    };
}

#endif