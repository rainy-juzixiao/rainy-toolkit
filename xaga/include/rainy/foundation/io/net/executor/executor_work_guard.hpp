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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_EXECUTOR_WORK_GUARD_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_EXECUTOR_WORK_GUARD_HPP
#include <rainy/foundation/io/net/executor/associated_executor.hpp>

namespace rainy::foundation::io::net {
    template <typename Executor>
    class executor_work_guard {
    public:
        using executor_type = Executor;

        executor_work_guard() = delete;

        explicit executor_work_guard(const executor_type &executor) : executor_{executor}, owns_{true} {
        }

        explicit executor_work_guard(const executor_work_guard &&right) noexcept : executor_{right.executor_}, owns_{right.owns_} {
        }

        explicit executor_work_guard(executor_work_guard &&other) noexcept :
            executor_(utility::move(other.executor_)), owns_(other.owns_) {
            other.owns_ = false;
        }

        executor_type get_executor() const noexcept {
            return executor_;
        }

        RAINY_NODISCARD bool owns_work() const noexcept {
            return owns_;
        }

        void reset() noexcept {
            if (owns_) {
                executor_.on_work_finished();
                owns_ = false;
            }
        }

    private:
        executor_type executor_;
        bool owns_;
    };

    template <typename Executor, type_traits::other_trans::enable_if_t<is_executor_v<Executor>, int> = 0>
    rain_fn make_work_guard(const Executor &ex) -> executor_work_guard<Executor> {
        return executor_work_guard<Executor>(ex);
    }

    template <typename ExecutionContext,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>, int> = 0>
    rain_fn make_work_guard(ExecutionContext &ctx) -> executor_work_guard<typename ExecutionContext::executor_type> {
        return executor_work_guard<typename ExecutionContext::executor_type>(ctx.get_executor());
    }

    template <typename Ty,
              type_traits::other_trans::enable_if_t<
                  !is_executor_v<Ty> && !type_traits::type_relations::is_convertible_v<Ty &, execution_context &>, int> = 0>
    rain_fn make_work_guard(const Ty &t) -> executor_work_guard<assoicated_executor_t<Ty>> {
        return executor_work_guard<typename associated_executor<Ty>::type>(associated_executor<Ty>::get(t));
    }

    template <typename Ty, typename Executor>
    rain_fn make_work_guard(const Ty &t, Executor &&ex)
        -> decltype(make_work_guard(get_associated_executor(t, utility::forward<Executor>(ex)))) {
        return executor_work_guard<typename associated_executor<Ty, Executor>::type>(associated_executor<Ty, Executor>::get(t, ex));
    }

    template <typename T, typename ExecutionContext,
              type_traits::other_trans::enable_if_t<
                  !is_executor<T>::value && !type_traits::type_relations::is_convertible_v<T &, execution_context &> &&
                      type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>,
                  int> = 0>
    rain_fn make_work_guard(const T &t, ExecutionContext &ctx)
        -> executor_work_guard<typename associated_executor<T, typename ExecutionContext::executor_type>::type> {
        return executor_work_guard<typename associated_executor<T, typename ExecutionContext::executor_type>::type>(
            associated_executor<T, typename ExecutionContext::executor_type>::get(t, ctx.get_executor()));
    }
}

#endif
