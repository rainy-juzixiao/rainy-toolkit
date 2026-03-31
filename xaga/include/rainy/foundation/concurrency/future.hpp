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
/**
 * @file future.hpp
 * @brief 异步结果与任务完成状态抽象接口
 *
 * 本文件定义 rainy foundation pal concurrency 中用于表示异步计算结果的
 * future / promise 抽象，以及与之相关的状态同步机制。
 *
 * 该组件用于在不同执行上下文之间安全地传递计算结果，
 * 并为上层 runtime、executor 与调度系统提供统一的
 * 异步完成语义。
 *
 * 设计目标：
 *  - 提供类似 C++ std::future 的基础语义模型
 *  - 解耦任务执行与结果获取过程
 *  - 支持线程间安全的结果传递与等待机制
 *  - 为 coroutine / scheduler / async runtime 提供基础构件
 *
 * 抽象职责：
 *  - future 表示只读的异步结果视图
 *  - shared_future 表示共享所有权的异步结果视图
 *  - promise 负责结果的生产与状态完成
 *  - shared_state 管理生命周期与同步原语
 *  - monad_future 基于执行器机制实现的异步计算流结果的视图
 *  - shared_monad_future 表示共享所有权的基于执行器机制的异步计算流结果的视图
 *  - packaged_task 表示共享所有权的异步计算结果的视图
 *
 * 注意：
 *  - 本命名空间属于 foundation 层基础设施，不直接负责调度执行。
 *  - future 不保证任务一定被执行，仅表示结果获取协议。
 *  - 阻塞行为依赖底层同步实现，可能受到平台调度策略影响。
 *
 * 生命周期原则：
 *  - promise 必须最终设置 value 或 exception 以完成状态
 *  - 未完成的 shared state 可能导致等待方永久阻塞
 *  - future 对象销毁不会取消任务执行
 *
 * 并发语义：
 *  - get() 只能被调用一次（除非为 shared future）
 *  - 多线程访问需遵循接口线程安全约束
 *  - 状态完成对等待线程具有 happens-before 保证
 */
#ifndef RAINY_FOUNDATION_PAL_CONCURRENCY_FUTURE_HPP
#define RAINY_FOUNDATION_PAL_CONCURRENCY_FUTURE_HPP
#include <functional>
#include <future>
#include <memory>
#include <rainy/foundation/concurrency/basic/scheduler.hpp>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/functional/delegate.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>
#include <variant>

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class monad_future;
    template <typename Ty>
    class shared_monad_future;
    template <typename Ty>
    class future;
    template <typename Ty>
    class shared_future;
    template <typename Ty>
    class promise;

    template <typename Ty>
    struct is_future : std::false_type {
        using inner = Ty;
    };

    template <typename Ty>
    struct is_future<monad_future<Ty>> : std::true_type {
        using inner = Ty;
    };

    template <typename Fx, typename Arg>
    struct then_result {
        using raw = std::invoke_result_t<type_traits::other_trans::decay_t<Fx>, Arg>;
        using type = is_future<raw>::inner; // 展平一层
        using is_wrapped = is_future<raw>;
    };

    enum class launch {
        async = 0x1,
        deferred = 0x2,
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(launch);
}

namespace rainy::foundation::concurrency {
    enum class future_status {
        ready,
        timeout,
        deferred
    };
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class shared_state : public std::enable_shared_from_this<shared_state<Ty>> {
    public:
        shared_state() : status_(status::pending) {
        }

        shared_state(const shared_state &) = delete;
        shared_state &operator=(const shared_state &) = delete;

        ~shared_state() {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                if (status_ == status::fulfilling_at_exit) {
                    status_ = status::fulfilled;
                } else if (status_ == status::rejecting_at_exit) {
                    status_ = status::rejected;
                } else {
                    return;
                }
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        Ty &value_ref_unsafe() {
            return std::get<1>(result_);
        }

        std::exception_ptr exception_ref_unsafe() const {
            return std::get<2>(result_);
        }

        void set_value(Ty value) {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                throw_if_already_satisfied();
                result_.template emplace<1>(utility::move(value));
                status_ = status::fulfilled;
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        void set_exception(std::exception_ptr ep) {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                throw_if_already_satisfied();
                result_.template emplace<2>(utility::move(ep));
                status_ = status::rejected;
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        void set_deferred(functional::delegate<void()> fn) {
            lock_guard lk(mutex_);
            if (status_ != status::pending) {
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
            deferred_fn_ = utility::move(fn);
            status_ = status::deferred;
        }

        void wait() const {
            maybe_invoke_deferred();
            unique_lock lk(mutex_);
            cv_.wait(lk, [this] { return is_done(status_); });
        }

        template <typename Rep, typename Period>
        future_status wait_for(const std::chrono::duration<Rep, Period> &rel) const {
            {
                lock_guard lk(mutex_);
                if (status_ == status::deferred) {
                    return future_status::deferred;
                }
            }
            unique_lock lk(mutex_);
            return cv_.wait_for(lk, rel, [this] { return is_done(status_); }) ? future_status::ready : future_status::timeout;
        }

        template <typename Clock, typename Duration>
        future_status wait_until(const std::chrono::time_point<Clock, Duration> &abs) const {
            {
                lock_guard lk(mutex_);
                if (status_ == status::deferred) {
                    return future_status::deferred;
                }
            }
            unique_lock lk(mutex_);
            return cv_.wait_until(lk, abs, [this] { return is_done(status_); }) ? future_status::ready : future_status::timeout;
        }

        Ty get() {
            maybe_invoke_deferred();
            wait();
            lock_guard lk(mutex_);
            if (status_ == status::rejected) {
                std::rethrow_exception(std::get<2>(result_));
            }
            return utility::move(std::get<1>(result_));
        }

        const Ty &get_shared() const {
            maybe_invoke_deferred();
            wait();
            lock_guard lk(mutex_);
            if (status_ == status::rejected) {
                std::rethrow_exception(std::get<2>(result_));
            }
            return std::get<1>(result_);
        }

        bool is_deferred() const {
            lock_guard lk(mutex_);
            return status_ == status::deferred;
        }

        bool is_fulfilled() const {
            lock_guard lk(mutex_);
            return status_ == status::fulfilled;
        }

        bool is_rejected() const {
            lock_guard lk(mutex_);
            return status_ == status::rejected;
        }

        bool is_ready() const {
            lock_guard lk(mutex_);
            return is_done(status_);
        }

        std::exception_ptr get_exception() const {
            lock_guard lk(mutex_);
            if (status_ == status::rejected) {
                return std::get<2>(result_);
            }
            return nullptr;
        }

        void add_continuation(functional::delegate<void()> callback) {
            bool fire_now = false;
            {
                lock_guard lk(mutex_);
                if (is_done(status_)) {
                    fire_now = true;
                } else {
                    continuations_.push_back(utility::move(callback));
                    return;
                }
            }
            if (fire_now) {
                callback();
                callback.reset();
            }
        }

    private:
        enum class status {
            pending,
            deferred,
            fulfilling_at_exit,
            rejecting_at_exit,
            fulfilled,
            rejected
        };

        static bool is_done(status s) noexcept {
            return s == status::fulfilled || s == status::rejected;
        }

        void throw_if_already_satisfied() {
            if (status_ != status::pending) {
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
        }

        void maybe_invoke_deferred() const {
            functional::delegate<void()> fn;
            {
                lock_guard lk(mutex_);
                if (status_ != status::deferred) {
                    return;
                }
                fn = utility::move(deferred_fn_);
                status_ = status::pending;
            }
            fn();
        }

        void flush_at_exit() {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                if (status_ == status::fulfilling_at_exit) {
                    status_ = status::fulfilled;
                } else if (status_ == status::rejecting_at_exit) {
                    status_ = status::rejected;
                }
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        static void drain(std::vector<functional::delegate<void()>> callbacks) {
            for (auto &cb: callbacks) {
                cb();
                cb.reset();
            }
        }

        mutable mutex mutex_;
        mutable condition_variable cv_;
        mutable functional::delegate<void()> deferred_fn_;
        mutable status status_;

        std::vector<functional::delegate<void()>> continuations_;
        std::variant<std::monostate, Ty, std::exception_ptr> result_;
    };
}

namespace rainy::foundation::concurrency {
    template <>
    class shared_state<void> : public std::enable_shared_from_this<shared_state<void>> {
    public:
        shared_state() = default;

        ~shared_state() {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                if (status_ == status::fulfilling_at_exit) {
                    status_ = status::fulfilled;
                } else if (status_ == status::rejecting_at_exit) {
                    status_ = status::rejected;
                } else {
                    return;
                }
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        void set_value() {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                throw_if_already_satisfied();
                status_ = status::fulfilled;
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        void set_exception(std::exception_ptr ep) {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                throw_if_already_satisfied();
                exception_ = utility::move(ep);
                status_ = status::rejected;
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        void wait() const {
            maybe_invoke_deferred();
            unique_lock lk(mutex_);
            cv_.wait(lk, [this] { return is_done(status_); });
        }

        template <typename Rep, typename Period>
        future_status wait_for(const std::chrono::duration<Rep, Period> &rel) const {
            {
                lock_guard lk(mutex_);
                if (status_ == status::deferred)
                    return future_status::deferred;
            }
            unique_lock lk(mutex_);
            return cv_.wait_for(lk, rel, [this] { return is_done(status_); }) ? future_status::ready : future_status::timeout;
        }

        template <typename Clock, typename Duration>
        future_status wait_until(const std::chrono::time_point<Clock, Duration> &abs) const {
            {
                lock_guard lk(mutex_);
                if (status_ == status::deferred)
                    return future_status::deferred;
            }
            unique_lock lk(mutex_);
            return cv_.wait_until(lk, abs, [this] { return is_done(status_); }) ? future_status::ready : future_status::timeout;
        }

        void get() {
            maybe_invoke_deferred();
            wait();
            lock_guard lk(mutex_);
            if (status_ == status::rejected) {
                std::rethrow_exception(exception_);
            }
        }

        void get_shared() const {
            const_cast<shared_state *>(this)->get();
        }

        void set_deferred(functional::delegate<void()> fn) {
            lock_guard lk(mutex_);
            if (status_ != status::pending) {
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
            deferred_fn_ = utility::move(fn);
            status_ = status::deferred;
        }

        bool is_deferred() const {
            lock_guard lk(mutex_);
            return status_ == status::deferred;
        }

        bool is_fulfilled() const {
            lock_guard lk(mutex_);
            return status_ == status::fulfilled;
        }

        bool is_rejected() const {
            lock_guard lk(mutex_);
            return status_ == status::rejected;
        }

        bool is_ready() const {
            lock_guard lk(mutex_);
            return is_done(status_);
        }

        std::exception_ptr get_exception() const {
            lock_guard lk(mutex_);
            return exception_;
        }

        void add_continuation(functional::delegate<void()> callback) {
            bool fire_now = false;
            {
                lock_guard lk(mutex_);
                if (is_done(status_)) {
                    fire_now = true;
                } else {
                    continuations_.push_back(utility::move(callback));
                    return;
                }
            }
            if (fire_now) {
                callback();
                callback.reset();
            }
        }

    private:
        enum class status {
            pending,
            deferred,
            fulfilling_at_exit,
            rejecting_at_exit,
            fulfilled,
            rejected
        };

        static bool is_done(const status s) noexcept {
            return s == status::fulfilled || s == status::rejected;
        }

        void throw_if_already_satisfied() const {
            if (status_ != status::pending) {
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
        }

        void flush_at_exit() {
            std::vector<functional::delegate<void()>> to_drain;
            {
                lock_guard lk(mutex_);
                if (status_ == status::fulfilling_at_exit) {
                    status_ = status::fulfilled;
                } else if (status_ == status::rejecting_at_exit) {
                    status_ = status::rejected;
                }
                to_drain = utility::move(continuations_);
            }
            cv_.notify_all();
            drain(utility::move(to_drain));
        }

        void maybe_invoke_deferred() const {
            functional::delegate<void()> fn;
            {
                lock_guard lk(mutex_);
                if (status_ != status::deferred)
                    return;
                fn = utility::move(deferred_fn_);
                status_ = status::pending;
            }
            fn();
        }

        static void drain(std::vector<functional::delegate<void()>> callbacks) {
            for (auto &cb: callbacks) {
                cb();
                cb.reset();
            }
        }

        mutable mutex mutex_;
        mutable condition_variable cv_;
        mutable functional::delegate<void()> deferred_fn_;
        mutable status status_{status::pending};

        std::exception_ptr exception_;
        std::vector<functional::delegate<void()>> continuations_;
    };
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    std::shared_ptr<shared_state<Ty>> make_shared_state() {
        return std::shared_ptr<shared_state<Ty>>(new shared_state<Ty>());
    }
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class monad_future {
    public:
        using value_type = Ty;

        template <typename UTy>
        friend class monad_future;

        monad_future() noexcept = default;

        explicit monad_future(std::shared_ptr<shared_state<Ty>> state) noexcept : state_(utility::move(state)) {
        }

        explicit monad_future(std::shared_ptr<shared_state<Ty>> state, task_scheduler *sched) noexcept :
            state_(utility::move(state)), executor_(std::make_shared<scheduler_executor_impl>(scheduler_executor_impl{sched})) {
        }

        monad_future(const monad_future &) = delete;
        monad_future &operator=(const monad_future &) = delete;
        monad_future(monad_future &&) noexcept = default;
        monad_future &operator=(monad_future &&) noexcept = default;

        RAINY_NODISCARD rain_fn valid() const noexcept -> bool {
            return state_ != nullptr;
        }

        rain_fn get() -> Ty {
            ensure_valid();
            auto st = utility::move(state_);
            return st->get();
        }

        rain_fn wait() const -> void {
            ensure_valid();
            state_->wait();
        }

        template <typename Rep, typename Period>
        rain_fn wait_for(const std::chrono::duration<Rep, Period> &rel) const -> future_status {
            ensure_valid();
            return state_->wait_for(rel);
        }

        template <typename Clock, typename Duration>
        rain_fn wait_until(const std::chrono::time_point<Clock, Duration> &abs) const -> future_status {
            ensure_valid();
            return state_->wait_until(abs);
        }

        rain_fn share() noexcept -> shared_monad_future<Ty>;

        RAINY_NODISCARD rain_fn is_ready() const -> bool {
            return valid() && state_->is_ready();
        }

        template <typename Fx, typename Raw = std::invoke_result_t<type_traits::other_trans::decay_t<Fx>, Ty>,
                  typename UTy = typename is_future<Raw>::inner>
        rain_fn then(Fx &&fx) -> monad_future<UTy> {
            submit_fn inline_sub = [](functional::delegate<void()> callback) { callback(); };
            return then_impl<UTy, Raw>(utility::move(inline_sub), utility::forward<Fx>(fx));
        }

        template <typename Exec, typename Fx, typename Raw = std::invoke_result_t<type_traits::other_trans::decay_t<Fx>, Ty>,
                  typename UTy = is_future<Raw>::inner>
        rain_fn then(Exec &&exec, Fx &&fx) -> monad_future<UTy> {
            return then_impl<UTy, Raw>(wrap_executor(utility::forward<Exec>(exec)), utility::forward<Fx>(fx));
        }

        template <typename Fx>
        rain_fn catch_error(Fx &&handler) -> monad_future {
            submit_fn inline_sub = [](functional::delegate<void()> callback) { callback(); };
            return catch_error_impl(utility::move(inline_sub), utility::forward<Fx>(handler));
        }

        template <typename Exec, typename Fx>
        monad_future catch_error(Exec &&exec, Fx &&handler) {
            return catch_error_impl(utility::forward<Exec>(exec), utility::forward<Fx>(handler));
        }

        template <typename Fx>
        rain_fn finally(Fx &&fx) -> monad_future<Ty> {
            submit_fn inline_sub = [](functional::delegate<void()> callback) { callback(); };
            return finally_impl(utility::move(inline_sub), utility::forward<Fx>(fx));
        }

        template <typename Exec, typename Fx>
        monad_future finally(Exec &&exec, Fx &&fx) {
            return finally_impl(utility::forward<Exec>(exec), utility::forward<Fx>(fx));
        }

    private:
        void ensure_valid() const {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
        }

        using submit_fn = functional::delegate<void(functional::delegate<void()>)>;

        RAINY_NODISCARD submit_fn get_executor() const {
            if (executor_) {
                auto ex = executor_;
                return [ex](functional::delegate<void()> callback) { ex->submit(utility::move(callback)); };
            }
            return [](functional::delegate<void()> callback) { callback(); };
        }

        static submit_fn wrap_executor(submit_fn fn) {
            return fn;
        }

        template <typename Exec>
        static submit_fn wrap_executor_ref(Exec &exec) {
            return [&exec](functional::delegate<void()> callback) mutable { exec.submit(utility::move(callback)); };
        }

        template <typename Exec>
        static submit_fn wrap_executor_owned(Exec exec) {
            auto shared = std::make_shared<Exec>(utility::move(exec));
            return [shared](functional::delegate<void()> callback) mutable { shared->submit(utility::move(callback)); };
        }

        template <typename Exec>
        static submit_fn wrap_executor(Exec &exec) {
            if constexpr (std::is_same_v<type_traits::other_trans::decay_t<Exec>, submit_fn>) {
                return exec;
            } else {
                return wrap_executor_ref(exec);
            }
        }

        template <typename Exec>
        static submit_fn wrap_executor(Exec &&exec) {
            if constexpr (std::is_same_v<type_traits::other_trans::decay_t<Exec>, submit_fn>) {
                return utility::move(exec);
            } else {
                return wrap_executor_owned(utility::forward<Exec>(exec));
            }
        }

        template <typename UTy, typename Raw, typename Fx>
        monad_future<UTy> then_impl(submit_fn sub, Fx &&fx) {
            ensure_valid();
            auto next = make_shared_state<UTy>();
            auto cur = state_;
            auto fn = utility::forward<Fx>(fx);

            state_->add_continuation([cur, next, sub = utility::move(sub), fn = utility::move(fn)]() mutable {
                if (cur->is_rejected()) {
                    next->set_exception(cur->exception_ref_unsafe());
                    return;
                }
                sub([cur, next, fn = utility::move(fn)]() mutable {
                    try {
                        if constexpr (is_future<Raw>::value) {
                            monad_future<UTy> inner = fn(utility::move(cur->value_ref_unsafe()));
                            auto inner_state = inner.state_;
                            inner_state->add_continuation([inner_state, next]() mutable {
                                if (inner_state->is_rejected()) {
                                    next->set_exception(inner_state->get_exception());
                                } else {
                                    try {
                                        if constexpr (std::is_void_v<UTy>) {
                                            inner_state->get();
                                            next->set_value();
                                        } else {
                                            next->set_value(inner_state->get());
                                        }
                                    } catch (...) {
                                        next->set_exception(std::current_exception());
                                    }
                                }
                            });
                        } else {
                            if constexpr (std::is_void_v<UTy>) {
                                fn(utility::move(cur->value_ref_unsafe()));
                                next->set_value();
                            } else {
                                next->set_value(fn(utility::move(cur->value_ref_unsafe())));
                            }
                        }
                    } catch (...) {
                        next->set_exception(std::current_exception());
                    }
                });
            });
            return monad_future<UTy>(utility::move(next));
        }

        template <typename Exec, typename Fx,
                  typename Raw = std::invoke_result_t<type_traits::other_trans::decay_t<Fx>, std::exception_ptr>,
                  typename Unwrapped = typename is_future<Raw>::inner>
        monad_future catch_error_impl(Exec &&exec_or_fn, Fx &&handler) {
            static_assert(std::is_same_v<Unwrapped, Ty> || std::is_void_v<Ty>,
                          "catch_error handler must return Ty or monad_future<Ty>");
            ensure_valid();
            auto next = make_shared_state<Ty>();
            auto cur = state_;
            auto fn = utility::forward<Fx>(handler);
            auto sub = wrap_executor(utility::forward<Exec>(exec_or_fn));

            state_->add_continuation([cur, next, sub, fn = utility::move(fn)]() mutable {
                sub([cur, next, fn = utility::move(fn)]() mutable {
                    if (cur->is_rejected()) {
                        try {
                            if constexpr (is_future<Raw>::value) {
                                monad_future<Ty> inner = fn(cur->get_exception());
                                auto inner_state = inner.state_;
                                inner_state->add_continuation([inner_state, next]() mutable {
                                    if (inner_state->is_rejected()) {
                                        next->set_exception(inner_state->get_exception());
                                    } else {
                                        try {
                                            if constexpr (std::is_void_v<Ty>) {
                                                inner_state->get();
                                                next->set_value();
                                            } else {
                                                next->set_value(inner_state->get());
                                            }
                                        } catch (...) {
                                            next->set_exception(std::current_exception());
                                        }
                                    }
                                });
                            } else {
                                if constexpr (std::is_void_v<Ty>) {
                                    fn(cur->get_exception());
                                    next->set_value();
                                } else {
                                    next->set_value(fn(cur->get_exception()));
                                }
                            }
                        } catch (...) {
                            next->set_exception(std::current_exception());
                        }
                    } else {
                        try {
                            if constexpr (std::is_void_v<Ty>) {
                                cur->get();
                                next->set_value();
                            } else {
                                next->set_value(cur->get());
                            }
                        } catch (...) {
                            next->set_exception(std::current_exception());
                        }
                    }
                });
            });
            return monad_future<Ty>(utility::move(next));
        }

        template <typename Exec, typename Fx, typename Raw = std::invoke_result_t<type_traits::other_trans::decay_t<Fx>>>
        monad_future<Ty> finally_impl(Exec &&exec_or_fn, Fx &&fx) {
            ensure_valid();
            auto next = make_shared_state<Ty>();
            auto cur = state_;
            auto fn = utility::forward<Fx>(fx);
            auto sub = wrap_executor(utility::forward<Exec>(exec_or_fn));

            state_->add_continuation([cur, next, sub, fn = utility::move(fn)]() mutable {
                sub([cur, next, fn = utility::move(fn)]() mutable {
                    auto propagate = [cur, next]() mutable {
                        if (cur->is_rejected()) {
                            next->set_exception(cur->get_exception());
                        } else {
                            try {
                                if constexpr (std::is_void_v<Ty>) {
                                    cur->get();
                                    next->set_value();
                                } else {
                                    next->set_value(cur->get());
                                }
                            } catch (...) {
                                next->set_exception(std::current_exception());
                            }
                        }
                    };

                    if constexpr (is_future<Raw>::value) {
                        try {
                            monad_future<void> cleanup = fn();
                            auto cleanup_state = cleanup.state_;
                            cleanup_state->add_continuation([cleanup_state, propagate = utility::move(propagate)]() mutable {
                                (void) cleanup_state;
                                propagate();
                            });
                        } catch (...) {
                            propagate();
                        }
                    } else {
                        try {
                            fn();
                        } catch (...) {
                        }
                        propagate();
                    }
                });
            });

            return monad_future<Ty>(utility::move(next));
        }

        struct abstract_executor {
            virtual void submit(functional::delegate<void()>) = 0;
            virtual ~abstract_executor() = default;
        };

        struct scheduler_executor_impl : abstract_executor {
            explicit scheduler_executor_impl(task_scheduler *scheduler) : sched(scheduler) {
            }

            void submit(functional::delegate<void()> fx) override {
                sched->submit(utility::move(fx));
            }

            task_scheduler *sched;
        };

        std::shared_ptr<shared_state<Ty>> state_;
        std::shared_ptr<abstract_executor> executor_;
    };
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class shared_monad_future {
    public:
        shared_monad_future() noexcept = default;
        shared_monad_future(const shared_monad_future &) noexcept = default;
        shared_monad_future &operator=(const shared_monad_future &) noexcept = default;
        shared_monad_future(shared_monad_future &&) noexcept = default;
        shared_monad_future &operator=(shared_monad_future &&) noexcept = default;

        shared_monad_future(monad_future<Ty> &&fx) noexcept : state_(utility::move(fx.state_)) {
        } // NOLINT

        bool valid() const noexcept {
            return state_ != nullptr;
        }

        const Ty &get() const {
            ensure_valid();
            return state_->get_shared();
        }

        void wait() const {
            ensure_valid();
            state_->wait();
        }

        template <typename Rep, typename Period>
        future_status wait_for(const std::chrono::duration<Rep, Period> &rel) const {
            ensure_valid();
            return state_->wait_for(rel);
        }

        template <typename Clock, typename Duration>
        future_status wait_until(const std::chrono::time_point<Clock, Duration> &abs) const {
            ensure_valid();
            return state_->wait_until(abs);
        }

        template <typename Fx, typename Raw = std::invoke_result_t<type_traits::other_trans::decay_t<Fx>, const Ty &>,
                  typename UTy = typename is_future<Raw>::inner>
        monad_future<UTy> then(Fx &&fx) const {
            ensure_valid();
            auto next = make_shared_state<UTy>();
            auto cur = state_;
            auto fn = utility::forward<Fx>(fx);

            state_->add_continuation([cur, next, fn = utility::move(fn)]() mutable {
                if (cur->is_rejected()) {
                    next->set_exception(cur->get_exception());
                    return;
                }
                try {
                    if constexpr (is_future<Raw>::value) {
                        // fx 返回 monad_future<UTy>，需要 unwrap
                        monad_future<UTy> inner = fn(cur->get_shared());
                        auto next_cap = next;
                        inner.state_->add_continuation([inner_state = inner.state_, next_cap]() mutable {
                            if (inner_state->is_rejected()) {
                                next_cap->set_exception(inner_state->get_exception());
                            } else {
                                try {
                                    if constexpr (std::is_void_v<UTy>) {
                                        inner_state->get();
                                        next_cap->set_value();
                                    } else {
                                        next_cap->set_value(inner_state->get());
                                    }
                                } catch (...) {
                                    next_cap->set_exception(std::current_exception());
                                }
                            }
                        });
                    } else {
                        // fx 返回普通值，原有路径
                        if constexpr (std::is_void_v<UTy>) {
                            fn(cur->get_shared());
                            next->set_value();
                        } else {
                            next->set_value(fn(cur->get_shared()));
                        }
                    }
                } catch (...) {
                    next->set_exception(std::current_exception());
                }
            });

            return monad_future<UTy>(utility::move(next));
        }

    private:
        void ensure_valid() const {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
        }

        std::shared_ptr<shared_state<Ty>> state_;
        friend class monad_future<Ty>;
    };

    template <typename Ty>
    shared_monad_future<Ty> monad_future<Ty>::share() noexcept {
        shared_monad_future<Ty> sf;
        sf.state_ = utility::move(state_);
        return sf;
    }
}

namespace rainy::foundation::concurrency {
    template <typename Rx>
    class future {
    public:
        future() noexcept = default;

        future(future &&right) noexcept : state_(utility::move(right.state_)) {
        }

        future(const future &) = delete;

        /**
         * @brief 通过共享状态构造 future
         * @param state 异步操作的共享状态
         */
        explicit future(std::shared_ptr<shared_state<Rx>> state) noexcept : state_(utility::move(state)) {
        }

        ~future() = default;

        future &operator=(const future &) = delete;

        future &operator=(future &&right) noexcept {
            state_ = utility::move(right.state_);
            return *this;
        }

        shared_future<Rx> share() noexcept {
            return shared_future<Rx>(utility::move(*this));
        }

        decltype(auto) get() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            auto state = utility::move(state_); // get() 只能调用一次，调用后 valid() == false
            return state->get();
        }

        bool valid() const noexcept {
            return state_ != nullptr;
        }

        void wait() const {
            check_state();
            state_->wait();
        }

        template <typename Rep, typename Period>
        rain_fn wait_for(const std::chrono::duration<Rep, Period> &rel) const -> future_status {
            check_state();
            return state_->wait_for(rel);
        }

        template <typename Clock, typename Duration>
        rain_fn wait_until(const std::chrono::time_point<Clock, Duration> &abs) const -> future_status {
            check_state();
            return state_->wait_until(abs);
        }

    private:
        friend class promise<Rx>;
        friend class shared_future<Rx>;

        void check_state() const {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
        }

        std::shared_ptr<shared_state<Rx>> state_;
    };

    template <>
    class future<void> {
    public:
        future() noexcept = default;

        future(future &&right) noexcept : state_(utility::move(right.state_)) {
        }

        future(const future &) = delete;
        ~future() = default;

        /**
         * @brief 通过共享状态构造 future
         * @param state 异步操作的共享状态
         */
        explicit future(std::shared_ptr<shared_state<void>> state) noexcept : state_(utility::move(state)) {
        }

        future &operator=(const future &) = delete;

        future &operator=(future &&right) noexcept {
            state_ = utility::move(right.state_);
            return *this;
        }

        template <typename Void = void>
        shared_future<Void> share() noexcept {
            return shared_future<Void>(utility::move(*this));
        }

        void get() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }

            auto state = utility::move(state_);
            state->get(); // 仅等待 + 重新抛异常
        }

        bool valid() const noexcept {
            return state_ != nullptr;
        }

        void wait() const {
            check_state();
            state_->wait();
        }

        template <typename Rep, typename Period>
        future_status wait_for(const std::chrono::duration<Rep, Period> &rel_time) const {
            check_state();
            return state_->wait_for(rel_time);
        }

        template <typename Clock, typename Duration>
        future_status wait_until(const std::chrono::time_point<Clock, Duration> &abs_time) const {
            check_state();
            return state_->wait_until(abs_time);
        }

    private:
        friend class promise<void>;
        friend class shared_future<void>;

        void check_state() const {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
        }

        std::shared_ptr<shared_state<void>> state_;
    };

    template <typename Rx>
    class shared_future {
    public:
        shared_future() noexcept = default;

        shared_future(const shared_future &right) noexcept : state_(right.state_) {
        }

        shared_future(future<Rx> &&right) noexcept : state_(utility::move(right.state_)) { // NOLINT
        }

        shared_future(shared_future &&right) noexcept : state_(utility::move(right.state_)) {
        }

        ~shared_future() = default;

        shared_future &operator=(const shared_future &right) noexcept = default;

        shared_future &operator=(shared_future &&right) noexcept {
            state_ = utility::move(right.state_);
            return *this;
        }

        decltype(auto) get() const {
            check_state();
            return state_->get_shared();
        }

        RAINY_NODISCARD bool valid() const noexcept {
            return state_ != nullptr;
        }

        void wait() const {
            check_state();
            state_->wait();
        }

        template <typename Rep, typename Period>
        future_status wait_for(const std::chrono::duration<Rep, Period> &rel_time) const {
            check_state();
            return state_->wait_for(rel_time);
        }

        template <typename Clock, typename Duration>
        future_status wait_until(const std::chrono::time_point<Clock, Duration> &abs_time) const {
            check_state();
            return state_->wait_until(abs_time);
        }

    private:
        void check_state() const {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
        }

        std::shared_ptr<shared_state<Rx>> state_;
    };
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class promise {
    public:
        promise() : state_(make_shared_state<Ty>()), future_retrieved_(false) {
        }

        template <typename Allocator>
        promise(std::allocator_arg_t, const Allocator &) : promise() {
        }

        promise(promise &&right) noexcept : state_(utility::move(right.state_)), future_retrieved_(right.future_retrieved_) {
            right.future_retrieved_ = false;
        }

        promise(const promise &) = delete;

        ~promise() {
            // NOLINTBEGIN
            if (state_ && !state_->is_ready()) {
                try {
                    state_->set_exception(std::make_exception_ptr(std::future_error(std::future_errc::broken_promise)));
                } catch (...) {
                }
            }
            // NOLINTEND
        }

        promise &operator=(promise &&right) noexcept {
            promise tmp(utility::move(right));
            swap(tmp);
            return *this;
        }
        promise &operator=(const promise &) = delete;

        void swap(promise &other) noexcept {
            std::swap(state_, other.state_);
            std::swap(future_retrieved_, other.future_retrieved_);
        }

        future<Ty> get_future() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (future_retrieved_) {
                throw std::future_error(std::future_errc::future_already_retrieved);
            }
            future_retrieved_ = true;
            return future(state_);
        }

        monad_future<Ty> get_monad_future() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (future_retrieved_) {
                throw std::future_error(std::future_errc::future_already_retrieved);
            }
            future_retrieved_ = true;
            return monad_future<Ty>(state_); // 共享 state，不转移 promise 的所有权
        }

        void set_value(const Ty &v) {
            ensure_state();
            state_->set_value(v);
        }

        void set_value(Ty &&v) {
            ensure_state();
            state_->set_value(utility::move(v));
        }

        void set_exception(std::exception_ptr p) {
            ensure_state();
            state_->set_exception(utility::move(p));
        }

    private:
        void ensure_state() const {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
        }

        std::shared_ptr<shared_state<Ty>> state_;
        bool future_retrieved_;
    };

    template <>
    class promise<void> {
    public:
        promise() : state_(make_shared_state<void>()), future_retrieved_(false) {
        }

        template <typename Allocator>
        promise(std::allocator_arg_t, const Allocator &) : promise() {
        }

        promise(promise &&right) noexcept : state_(utility::move(right.state_)), future_retrieved_(right.future_retrieved_) {
            right.future_retrieved_ = false;
        }

        promise(const promise &) = delete;

        ~promise() {
            // NOLINTBEGIN
            if (state_ && !state_->is_ready()) {
                try {
                    state_->set_exception(std::make_exception_ptr(std::future_error(std::future_errc::broken_promise)));
                } catch (...) {
                }
            }
            // NOLINTEND
        }

        promise &operator=(promise &&right) noexcept {
            promise tmp(utility::move(right));
            swap(tmp);
            return *this;
        }

        promise &operator=(const promise &) = delete;
        
        void swap(promise &other) noexcept {
            std::swap(state_, other.state_);
            std::swap(future_retrieved_, other.future_retrieved_);
        }

        future<void> get_future() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (future_retrieved_) {
                throw std::future_error(std::future_errc::future_already_retrieved);
            }
            future_retrieved_ = true;
            return future(state_);
        }

        monad_future<void> get_monad_future() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (future_retrieved_) {
                throw std::future_error(std::future_errc::future_already_retrieved);
            }
            future_retrieved_ = true;
            return monad_future(state_);
        }

        void set_value() { // NOLINT
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            state_->set_value();
        }

        void set_exception(std::exception_ptr p) { // NOLINT
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            state_->set_exception(utility::move(p));
        }

    private:
        std::shared_ptr<shared_state<void>> state_;
        bool future_retrieved_;
    };

    template <typename Ty>
    void swap(promise<Ty> &left, promise<Ty> &right) noexcept {
        left.swap(right);
    }
}

namespace std { // NOLINT
    template <typename Ty, typename Alloc>
    struct uses_allocator<rainy::foundation::concurrency::promise<Ty>, Alloc> : std::true_type {}; // NOLINT
}

namespace rainy::foundation::concurrency {
    template <typename>
    class packaged_task;

    template <typename Rx, typename... Args>
    class packaged_task<Rx(Args...)> {
    public:
        packaged_task() noexcept = default;

        template <typename Fx, typename = type_traits::other_trans::enable_if_t<
                                   !std::is_same_v<type_traits::other_trans::decay_t<Fx>, packaged_task>>>
        explicit packaged_task(Fx &&fx) : func_(utility::forward<Fx>(fx)), state_(make_shared_state<Rx>()) { // NOLINT
        }

        ~packaged_task() = default;

        packaged_task(const packaged_task &) = delete;
        packaged_task &operator=(const packaged_task &) = delete;

        packaged_task(packaged_task &&right) noexcept :
            func_(utility::move(right.func_)), state_(utility::move(right.state_)), future_retrieved_(right.future_retrieved_) {
            right.future_retrieved_ = false;
        }

        packaged_task &operator=(packaged_task &&right) noexcept {
            packaged_task tmp(utility::move(right));
            swap(tmp);
            return *this;
        }

        void swap(packaged_task &other) noexcept {
            std::swap(func_, other.func_);
            std::swap(state_, other.state_);
            std::swap(future_retrieved_, other.future_retrieved_);
        }

        RAINY_NODISCARD bool valid() const noexcept {
            return static_cast<bool>(func_) && state_ != nullptr;
        }

        future<Rx> get_future() {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (future_retrieved_) {
                throw std::future_error(std::future_errc::future_already_retrieved);
            }
            future_retrieved_ = true;
            return future<Rx>(state_);
        }

        void operator()(Args... args) {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (state_->is_ready()) {
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
            invoke_and_set(state_, false, utility::forward<Args>(args)...);
        }

        void make_ready_at_thread_exit(Args... args) {
            if (!state_) {
                throw std::future_error(std::future_errc::no_state);
            }
            if (state_->is_ready()) {
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
            invoke_and_set(state_, true, utility::forward<Args>(args)...);
        }

        void reset() {
            if (!func_) {
                throw std::future_error(std::future_errc::no_state);
            }
            state_ = make_shared_state<Rx>();
            future_retrieved_ = false;
        }

    private:
        void invoke_and_set(std::shared_ptr<shared_state<Rx>> &st, const bool at_thread_exit, Args... args) {
            try {
                if constexpr (std::is_void_v<Rx>) {
                    func_(utility::forward<Args>(args)...);
                    st->set_value();
                } else {
                    Rx result = func_(utility::forward<Args>(args)...);
                    st->set_value(utility::move(result));
                }
            } catch (...) {
                st->set_exception(std::current_exception());
            }
        }

        functional::delegate<Rx(Args...)> func_;
        std::shared_ptr<shared_state<Rx>> state_;
        bool future_retrieved_{false};
    };

    template <typename Rx, typename... Args>
    packaged_task(Rx (*)(Args...)) -> packaged_task<Rx(Args...)>;

    template <typename Fx>
    packaged_task(Fx) -> packaged_task<type_traits::primary_types::function_return_type<Fx>>;

    template <typename Rx, typename... Args>
    void swap(packaged_task<Rx(Args...)> &left, packaged_task<Rx(Args...)> &right) noexcept {
        left.swap(right);
    }
}

#endif
