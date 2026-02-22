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
#ifndef RAINY_FOUNDATION_CONCURRENCY_EXECUTOR_HPP
#define RAINY_FOUNDATION_CONCURRENCY_EXECUTOR_HPP
#include <rainy/foundation/concurrency/future.hpp>
#include <rainy/foundation/concurrency/pool.hpp>
#include <rainy/foundation/concurrency/scheduler.hpp>

namespace rainy::foundation::concurrency {
    class RAINY_TOOLKIT_API executor {
    public:
        explicit executor(std::unique_ptr<task_scheduler> scheduler) noexcept : owned_(utility::move(scheduler)), borrowed_(nullptr) {
        }

        explicit executor(task_scheduler &scheduler) noexcept : owned_(nullptr), borrowed_(std::addressof(scheduler)) {
        }

        executor(const executor &) = delete;
        executor &operator=(const executor &) = delete;
        executor(executor &&) = default;
        executor &operator=(executor &&) = default;

        template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
        monad_future<R> submit_to(const std::size_t actor_id, F &&f) {
            auto state = std::make_shared<shared_state<R>>();
            auto *sched = get_scheduler();
            auto fn = utility::forward<F>(f);

            sched->submit_to(actor_id, [state, fn = utility::move(fn)]() mutable {
                try {
                    if constexpr (std::is_void_v<R>) {
                        fn();
                        state->set_value();
                    } else {
                        state->set_value(fn());
                    }
                } catch (...) {
                    state->set_exception(std::current_exception());
                }
            });

            return monad_future<R>(utility::move(state), sched);
        }

        template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
        monad_future<R> submit(F &&f) {
            auto state = make_shared_state<R>();
            auto *sched = get_scheduler();
            auto fn = utility::forward<F>(f);
            sched->submit([state, fn = utility::move(fn)]() mutable {
                try {
                    if constexpr (std::is_void_v<R>) {
                        fn();
                        state->set_value();
                    } else {
                        state->set_value(fn());
                    }
                } catch (...) {
                    state->set_exception(std::current_exception());
                }
            });
            return monad_future<R>(utility::move(state), sched);
        }

        template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
        monad_future<R> submit(const actor_priority priority, F &&f) {
            auto state = make_shared_state<R>();
            auto *sched = get_scheduler();
            auto fn = utility::forward<F>(f);
            sched->submit(
                [state, fn = utility::move(fn)]() mutable {
                    try {
                        if constexpr (std::is_void_v<R>) {
                            fn();
                            state->set_value();
                        } else {
                            state->set_value(fn());
                        }
                    } catch (...) {
                        state->set_exception(std::current_exception());
                    }
                },
                priority);
            return monad_future<R>(utility::move(state), sched);
        }

        template <typename F, typename... Args, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
        monad_future<R> submit(F &&f, Args &&...args) {
            return submit([fn = utility::forward<F>(f), tup = std::make_tuple(utility::forward<Args>(args)...)]() mutable -> R {
                return std::apply(utility::move(fn), utility::move(tup));
            });
        }

        template <typename F, typename... Args, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
        monad_future<R> submit(const actor_priority priority, F &&f, Args &&...args) {
            return submit(priority,
                          [fn = utility::forward<F>(f), tup = std::make_tuple(utility::forward<Args>(args)...)]() mutable -> R {
                              return std::apply(utility::move(fn), utility::move(tup));
                          });
        }

        void wait_all() { // NOLINT
            get_scheduler()->wait_all();
        }

        RAINY_NODISCARD scheduler_traits traits() const {
            return get_scheduler()->traits();
        }

    private:
        RAINY_NODISCARD task_scheduler *get_scheduler() const noexcept {
            return owned_ ? owned_.get() : borrowed_;
        }

        std::unique_ptr<task_scheduler> owned_;
        task_scheduler *borrowed_;
    };

    RAINY_TOOLKIT_API executor &get_global_pooled_executor();
    RAINY_TOOLKIT_API executor &get_global_dedicated_executor();

    RAINY_INLINE executor make_pooled_executor(std::size_t thread_count = std::thread::hardware_concurrency(),
                                               std::size_t actor_count = 0) {
        return executor(std::make_unique<pooled_actor_pool>(thread_count, actor_count));
    }

    RAINY_INLINE executor make_dedicated_executor(std::size_t actor_count = std::thread::hardware_concurrency()) {
        return executor(std::make_unique<dedicated_actor_pool>(actor_count));
    }

    template <
        typename Fx, typename... Args,
        type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_v<type_traits::other_trans::decay_t<Fx>,
                                                                                           type_traits::other_trans::decay_t<Args>...>,
                                              int> = 0>
    rain_fn async(const launch policy, Fx &&fx, Args &&...args)
        -> future<type_traits::type_properties::invoke_result_t<type_traits::other_trans::decay_t<Fx>,
                                                                type_traits::other_trans::decay_t<Args>...>> {
        using Rx = type_traits::type_properties::invoke_result_t<type_traits::other_trans::decay_t<Fx>,
                                                                 type_traits::other_trans::decay_t<Args>...>;
        if ((policy & launch::async) == launch::async) {
            packaged_task<Rx(type_traits::other_trans::decay_t<Args>...)> task(utility::forward<Fx>(fx));
            auto fut = task.get_future();
            get_global_pooled_executor().submit(utility::move(task), utility::forward<Args>(args)...); // 直接通过线程池执行
            return fut;
        }
        // 不用 packaged_task，直接操作 shared_state
        auto state = make_shared_state<Rx>();
        state->set_deferred([state, fx = utility::forward<Fx>(fx), ... args = utility::forward<Args>(args)]() mutable {
            try {
                if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                    fx(utility::forward<Args>(args)...);
                    state->set_value();
                } else {
                    state->set_value(fx(utility::forward<Args>(args)...));
                }
            } catch (...) {
                state->set_exception(std::current_exception());
            }
        });
        return future<Rx>(state);
    }

    template <
        typename Fx, typename... Args,
        type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_v<type_traits::other_trans::decay_t<Fx>,
                                                                                           type_traits::other_trans::decay_t<Args>...>,
                                              int> = 0>
    rain_fn async(Fx &&fx, Args &&...args)
        -> future<type_traits::type_properties::invoke_result_t<type_traits::other_trans::decay_t<Fx>,
                                                                type_traits::other_trans::decay_t<Args>...>> {
        return async(launch::async | launch::deferred, utility::forward<Fx>(fx), utility::forward<Args>(args)...);
    }

    template <typename Fx, typename... Args>
    rain_fn async_isolated(const launch policy, Fx &&fx, Args &&...args)
        -> future<type_traits::type_properties::invoke_result_t<type_traits::other_trans::decay_t<Fx>,
                                                                type_traits::other_trans::decay_t<Args>...>> {
        using Rx = type_traits::type_properties::invoke_result_t<type_traits::other_trans::decay_t<Fx>,
                                                                 type_traits::other_trans::decay_t<Args>...>;
        if ((policy & launch::async) == launch::async) {
            packaged_task<Rx(type_traits::other_trans::decay_t<Args>...)> task(utility::forward<Fx>(fx));
            auto fut = task.get_future();
            get_global_dedicated_executor().submit(utility::move(task), utility::forward<Args>(args)...);
            return fut;
        }
        // 不用 packaged_task，直接操作 shared_state
        auto state = make_shared_state<Rx>();
        state->set_deferred([state, fx = utility::forward<Fx>(fx), ... args = utility::forward<Args>(args)]() mutable {
            try {
                if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                    fx(utility::forward<Args>(args)...);
                    state->set_value();
                } else {
                    state->set_value(fx(utility::forward<Args>(args)...));
                }
            } catch (...) {
                state->set_exception(std::current_exception());
            }
        });
        return future<Rx>(state);
    }

    template <typename Fx, typename... Args>
    rain_fn async_isolated(Fx &&fx, Args &&...args)
        -> future<type_traits::type_properties::invoke_result_t<type_traits::other_trans::decay_t<Fx>,
                                                                type_traits::other_trans::decay_t<Args>...>> {
        return async_isolated(launch::async | launch::deferred, utility::forward<Fx>(fx), utility::forward<Args>(args)...);
    }
}

#endif
