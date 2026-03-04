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
#ifndef RAINY_FOUNDATION_CONCURRENCY_SCHEDULER_HPP
#define RAINY_FOUNDATION_CONCURRENCY_SCHEDULER_HPP
#include <functional>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/delegate.hpp>

namespace rainy::foundation::concurrency {
    enum class actor_pool_mode {
        pooled, // 模式A：N 线程服务 M 个 Actor 队列（M ≥ N），支持 work-stealing，提高吞吐
        dedicated, // 模式B：每个 Actor 独占一个 rainy::thread，实现强隔离，避免相互干扰
        pinned, // 模式C：Actor 绑定到特定线程，不会迁移，适合需要线程局部状态的任务
        priority, // 模式D：根据 Actor 优先级调度，优先级高的 Actor 更早被执行
        blocking // 模式E：支持阻塞操作的 Actor 模式，线程可能被阻塞等待 I/O 或长任务
    };

    /**
     * @brief 调度优先级
     *
     * 作为 submit(task, priority) 的 hint，决定任务被路由到哪个优先级层。
     * 不提供硬实时保证，仅影响线程扫描频率：
     *   high   被每轮必查
     *   normal 每 2 轮查一次
     *   low    每 4 轮查一次
     *
     * 数值越小优先级越高，支持直接用整数构造：
     *   actor_priority{0} == actor_priority::high
     */
    enum class actor_priority : std::uint8_t {
        high = 0,
        normal = 1,
        low = 2,
        count_ = 3 // 内部使用，标记层数
    };

    constexpr std::size_t actor_priority_levels = static_cast<std::size_t>(actor_priority::count_);

    struct scheduler_traits {
        bool is_multi_threaded = false;
        bool is_ordered = true; // 同一 Actor 内任务有序
        bool supports_stealing = false;
        bool supports_affinity = false;
        actor_pool_mode mode = actor_pool_mode::pooled;
        std::size_t concurrency = 1;
    };
}

namespace rainy::foundation::concurrency {
    class task_scheduler {
    public:
        virtual ~task_scheduler() = default;

        virtual void submit(functional::move_only_delegate<void()> task) = 0;

        virtual void submit(functional::move_only_delegate<void()> task, const actor_priority priority) {
            submit(utility::move(task));
            (void) priority;
        }

        virtual void submit_to(std::size_t actor_id, functional::move_only_delegate<void()> task) = 0;

        virtual void submit_to(std::size_t actor_id, functional::move_only_delegate<void()> task, const actor_priority priority) {
            submit_to(actor_id, utility::move(task));
            (void) priority;
        }

        virtual void wait_all() {
        }

        RAINY_NODISCARD virtual scheduler_traits traits() const {
            return {}; // 默认：单线程、有序
        }
    };
}

#endif
