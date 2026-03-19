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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_HPP

// NOLINTBEGIN

#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/atomic.hpp>
#include <rainy/foundation/concurrency/future.hpp>
#include <rainy/foundation/io/net/executor/associated_allocator.hpp>
#include <rainy/foundation/io/net/executor/associated_executor.hpp>
#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/io/net/executor/bind_executor.hpp>
#include <rainy/foundation/io/net/executor/execution_context.hpp>
#include <rainy/foundation/io/net/executor/executor_function.hpp>
#include <rainy/foundation/io/net/executor/executor_trait.hpp>
#include <rainy/foundation/io/net/executor/executor_work_guard.hpp>
#include <rainy/foundation/io/net/executor/global.hpp>
#include <rainy/foundation/io/net/executor/system_context.hpp>
#include <rainy/foundation/io/net/executor/system_executor.hpp>
#include <rainy/foundation/io/net/executor/work_dispatcher.hpp>
#include <rainy/foundation/io/net/executor/strand.hpp>
#include <rainy/foundation/io/net/fwd.hpp>

// NOLINTEND

namespace rainy::foundation::exceptions::net {
    class bad_executor : public exception {
    public:
        bad_executor(const source &location = source::current()) : exception("bad executor", location) { // NOLINT
        }
    };
}

namespace rainy::foundation::io::net {
    class executor {
    public:
        executor() noexcept : impl_(nullptr) {
        }

        executor(std::nullptr_t) noexcept : impl_(nullptr) { // NOLINT
        }

        executor(const executor &other) noexcept : impl_(other.clone()) {
        }

        executor(executor &&other) noexcept : impl_(other.impl_) {
            other.impl_ = nullptr;
        }

        template <typename Executor>
        executor(Executor e) : impl_(impl<Executor, std::allocator<void>>::create(e)) { // NOLINT
        }

        template <typename Executor, typename Allocator>
        executor(std::allocator_arg_t, const Allocator &a, Executor e) : impl_(impl<Executor, Allocator>::create(e, a)) {
        }

        ~executor() {
            destroy();
        }

        executor &operator=(const executor &other) noexcept { // NOLINT
            destroy();
            impl_ = other.clone();
            return *this;
        }

        executor &operator=(executor &&other) noexcept {
            destroy();
            impl_ = other.impl_;
            other.impl_ = nullptr;
            return *this;
        }

        executor &operator=(nullptr_t) noexcept {
            destroy();
            impl_ = nullptr;
            return *this;
        }

        template <typename Executor>
        executor &operator=(Executor &&e) noexcept {
            executor tmp(utility::move(e));
            destroy();
            impl_ = tmp.impl_;
            tmp.impl_ = nullptr;
            return *this;
        }

        RAINY_NODISCARD execution_context &context() const noexcept {
            return get_impl()->context();
        }

        void on_work_started() const noexcept {
            get_impl()->on_work_started();
        }

        void on_work_finished() const noexcept {
            get_impl()->on_work_finished();
        }

        template <typename Function, typename Allocator>
        void dispatch(Function &&f, const Allocator &a) const {
            if (impl_base *i = get_impl(); i->fast_dispatch_) {
                system_executor().dispatch(utility::move(f), a);
            } else {
                i->dispatch(function(utility::move(f), a));
            }
        }

        template <typename Function, typename Allocator>
        void post(Function &&f, const Allocator &a) const {
            get_impl()->post(function(utility::move(f), a));
        }

        template <typename Function, typename Allocator>
        void defer(Function &&f, const Allocator &a) const {
            get_impl()->defer(function(utility::move(f), a));
        }

        struct unspecified_bool_type_t {};
        typedef void (*unspecified_bool_type)(unspecified_bool_type_t);

        static void unspecified_bool_true(unspecified_bool_type_t) {
        }

        operator unspecified_bool_type() const noexcept { // NOLINT
            return impl_ ? &executor::unspecified_bool_true : nullptr;
        }

        RAINY_NODISCARD const void *target_type() const noexcept {
            return impl_ ? impl_->target_type() : nullptr;
        }

        template <typename Executor>
        Executor *target() noexcept {
            return impl_ && impl_->target_type() == type_id<Executor>() ? static_cast<Executor *>(impl_->target()) : nullptr;
        }

        template <typename Executor>
        const Executor *target() const noexcept {
            return impl_ && impl_->target_type() == type_id<Executor>() ? static_cast<Executor *>(impl_->target()) : nullptr;
        }

        friend bool operator==(const executor &a, const executor &b) noexcept {
            if (a.impl_ == b.impl_) {
                return true;
            }
            if (!a.impl_ || !b.impl_) {
                return false;
            }
            return a.impl_->equals(b.impl_);
        }

        friend bool operator!=(const executor &a, const executor &b) noexcept {
            return !(a == b);
        }

    private:
        class function {
        public:
            template <typename F, typename Alloc>
            explicit function(F f, const Alloc &a) {
                typedef implements::executor_function<F, Alloc> func_type;
                typename func_type::ptr p = {utility::addressof(a), func_type::ptr::allocate(a), 0};
                func_ = new (p.v) func_type(utility::forward<F>(f), a);
                p.v = 0;
            }

            function(function &&other) noexcept : func_(other.func_) {
                other.func_ = nullptr;
            }

            ~function() {
                if (func_) {
                    func_->destroy();
                }
            }

            void operator()() {
                if (func_) {
                    implements::executor_function_base *func = func_;
                    func_ = nullptr;
                    func->complete();
                }
            }

        private:
            implements::executor_function_base *func_;
        };

        template <typename, typename>
        class impl;

        using type_id_result_type = const void *;

        template <typename T>
        static type_id_result_type type_id() {
            static int unique_id;
            return &unique_id;
        }

        class impl_base { // NOLINT
        public:
            RAINY_NODISCARD virtual impl_base *clone() const noexcept = 0;
            virtual void destroy() noexcept = 0;
            virtual execution_context &context() noexcept = 0;
            virtual void on_work_started() noexcept = 0;
            virtual void on_work_finished() noexcept = 0;
            virtual void dispatch(function &&) = 0;
            virtual void post(function &&) = 0;
            virtual void defer(function &&) = 0;
            RAINY_NODISCARD virtual type_id_result_type target_type() const noexcept = 0;
            virtual void *target() noexcept = 0;
            RAINY_NODISCARD virtual const void *target() const noexcept = 0;
            virtual bool equals(const impl_base *e) const noexcept = 0;

        protected:
            explicit impl_base(const bool fast_dispatch) : fast_dispatch_(fast_dispatch) {
            }

            virtual ~impl_base() = default;

        private:
            friend class executor;
            const bool fast_dispatch_;
        };

        template <typename Executor, typename Allocator>
        class impl final : public impl_base {
        public:
            typedef NET_TS_REBIND_ALLOC(Allocator, impl) allocator_type;

            static impl_base *create(const Executor &e, Allocator a = Allocator()) {
                raw_mem mem(a);
                impl *p = new (mem.ptr_) impl(e, a);
                mem.ptr_ = 0;
                return p;
            }

            impl(const Executor &e, const Allocator &a) noexcept : impl_base(false), ref_count_(1), executor_(e), allocator_(a) {
            }

            impl_base *clone() const noexcept override {
                ++ref_count_;
                return const_cast<impl_base *>(static_cast<const impl_base *>(this));
            }

            void destroy() noexcept override {
                if (--ref_count_ == 0) {
                    allocator_type alloc(allocator_);
                    impl *p = this;
                    p->~impl();
                    alloc.deallocate(p, 1);
                }
            }

            void on_work_started() noexcept override {
                executor_.on_work_started();
            }

            void on_work_finished() noexcept override {
                executor_.on_work_finished();
            }

            execution_context &context() noexcept override {
                return executor_.context();
            }

            void dispatch(function &&f) override {
                executor_.dispatch(utility::move(f), allocator_);
            }

            void post(function &&f) override {
                executor_.post(utility::move(f), allocator_);
            }

            void defer(function &&f) override {
                executor_.defer(utility::move(f), allocator_);
            }

            type_id_result_type target_type() const noexcept override {
                return type_id<Executor>();
            }

            void *target() noexcept override {
                return &executor_;
            }

            const void *target() const noexcept override {
                return &executor_;
            }

            bool equals(const impl_base *e) const noexcept override {
                if (this == e) {
                    return true;
                }
                if (target_type() != e->target_type()) {
                    return false;
                }
                return executor_ == *static_cast<const Executor *>(e->target());
            }

        private:
            mutable concurrency::atomic_int ref_count_;
            Executor executor_;
            Allocator allocator_;

            struct raw_mem {
                allocator_type allocator_;
                impl *ptr_;

                explicit raw_mem(const Allocator &a) : allocator_(a), ptr_(allocator_.allocate(1)) {
                }

                ~raw_mem() {
                    if (ptr_) {
                        allocator_.deallocate(ptr_, 1);
                    }
                }
            };
        };

        template <typename Allocator>
        class executor::impl<system_executor, Allocator> : public impl_base {
        public:
            static impl_base *create(const system_executor &, const Allocator & = Allocator()) {
                return &implements::global<impl<system_executor, std::allocator<void>>>();
            }

            impl() : impl_base(true) {
            }

            RAINY_NODISCARD impl_base *clone() const noexcept override {
                return const_cast<impl_base *>(static_cast<const impl_base *>(this));
            }

            void destroy() noexcept override {
            }

            void on_work_started() noexcept override {
                executor_.on_work_started();
            }

            void on_work_finished() noexcept override {
                executor_.on_work_finished();
            }

            execution_context &context() noexcept override {
                return executor_.context();
            }

            void dispatch(function &&f) override {
                executor_.dispatch(utility::move(f), allocator_);
            }

            void post(function &&f) override {
                executor_.post(utility::move(f), allocator_);
            }

            void defer(function &&f) override {
                executor_.defer(utility::move(f), allocator_);
            }

            RAINY_NODISCARD type_id_result_type target_type() const noexcept override {
                return type_id<system_executor>();
            }

            void *target() noexcept override {
                return &executor_;
            }

            RAINY_NODISCARD const void *target() const noexcept override {
                return &executor_;
            }

            RAINY_NODISCARD bool equals(const impl_base *e) const noexcept override {
                return this == e;
            }

        private:
            system_executor executor_;
            Allocator allocator_;
        };

        RAINY_NODISCARD impl_base *get_impl() const {
            if (!impl_) {
                const exceptions::net::bad_executor ex;
                exceptions::throw_exception(ex);
            }
            return impl_;
        }

        RAINY_NODISCARD impl_base *clone() const noexcept {
            return impl_ ? impl_->clone() : nullptr;
        }

        void destroy() noexcept { // NOLINT
            if (impl_) {
                impl_->destroy();
            }
        }

        impl_base *impl_;
    };
}

// NOLINTBEGIN
namespace rainy::foundation::io::net {
    template <typename CompletionToken>
    rain_fn dispatch(CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        using handler = typename async_result<CompletionToken, void()>::completion_handler_type;
        async_completion<CompletionToken, void()> init(token);
        typename associated_executor<handler>::type ex(get_associated_executor(init.completion_handler));
        typename associated_allocator<handler>::type alloc(get_associated_allocator(init.completion_handler));
        ex.dispatch(utility::move(init.completion_handler), alloc);
        return init.result.get();
    }

    template <typename Executor, typename CompletionToken, type_traits::other_trans::enable_if_t<is_executor_v<Executor>, int> = 0>
    rain_fn dispatch(const Executor &ex, CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        using handler = typename async_result<CompletionToken, void()>::completion_handler_type;
        async_completion<CompletionToken, void()> init(token);
        typename associated_allocator<handler>::type alloc(get_associated_allocator(init.completion_handler));
        ex.dispatch(implements::work_dispatcher<handler>(init.completion_handler), alloc);
        return init.result.get();
    }

    template <typename ExecutionContext, typename CompletionToken,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>, int> = 0>
    rain_fn dispatch(ExecutionContext &ctx, CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        return dispatch(ctx.get_executor(), utility::move(token));
    }

    template <typename CompletionToken>
        rain_fn post(CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        using handler = typename async_result<CompletionToken, void()>::completion_handler_type;
        async_completion<CompletionToken, void()> init(token);
        typename associated_executor<handler>::type ex(get_associated_executor(init.completion_handler));
        typename associated_allocator<handler>::type alloc(get_associated_allocator(init.completion_handler));
        ex.post(utility::move(init.completion_handler), alloc);
        return init.result.get();
    }

    template <typename Executor, typename CompletionToken, type_traits::other_trans::enable_if_t<is_executor_v<Executor>, int> = 0>
    rain_fn post(const Executor &ex, CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        using handler = typename async_result<CompletionToken, void()>::completion_handler_type;
        async_completion<CompletionToken, void()> init(token);
        typename associated_allocator<handler>::type alloc(get_associated_allocator(init.completion_handler));
        ex.post(implements::work_dispatcher<handler>(init.completion_handler), alloc);
        return init.result.get();
    }

    template <typename ExecutionContext, typename CompletionToken,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>, int> = 0>
    rain_fn post(ExecutionContext &ctx, CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        return post(ctx.get_executor(), utility::move(token));
    }

    template <typename CompletionToken>
        rain_fn defer(CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        using handler = typename async_result<CompletionToken, void()>::completion_handler_type;
        async_completion<CompletionToken, void()> init(token);
        typename associated_executor<handler>::type ex(get_associated_executor(init.completion_handler));
        typename associated_allocator<handler>::type alloc(get_associated_allocator(init.completion_handler));
        ex.defer(utility::move(init.completion_handler), alloc);
        return init.result.get();
    }

    template <typename Executor, typename CompletionToken, type_traits::other_trans::enable_if_t<is_executor_v<Executor>, int> = 0>
    rain_fn defer(const Executor &ex, CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        using handler = typename async_result<CompletionToken, void()>::completion_handler_type;
        async_completion<CompletionToken, void()> init(token);
        typename associated_allocator<handler>::type alloc(get_associated_allocator(init.completion_handler));
        ex.defer(implements::work_dispatcher<handler>(init.completion_handler), alloc);
        return init.result.get();
    }

    template <typename ExecutionContext, typename CompletionToken,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>, int> = 0>
    rain_fn defer(ExecutionContext &ctx, CompletionToken &&token) -> typename async_result<CompletionToken, void()>::return_type {
        return defer(ctx.get_executor(), utility::move(token));
    }
}
// NOLINTEND

namespace rainy::foundation::io::net {
    template <typename ProtoAllocator = std::allocator<void>>
    class use_future_t {};

    constexpr use_future_t<> use_future = use_future_t<>();

    template <typename ProtoAllocator, class Result, class... Args>
    class async_result<use_future_t<ProtoAllocator>, Result(Args...)>;

    template <class Result, class... Args, class Signature>
    class async_result<concurrency::packaged_task<Result(Args...)>, Signature>;
}

namespace std { // NOLINT
    template <typename Allocator>
    struct uses_allocator<rainy::foundation::io::net::executor, Allocator> : std::true_type {};
}

#endif
