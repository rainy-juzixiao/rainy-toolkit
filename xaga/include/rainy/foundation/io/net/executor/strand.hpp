#ifndef RAINY_FOUNDATION_IO_NET_EXECUTYOR_STRAND_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTYOR_STRAND_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/atomic.hpp>
#include <rainy/foundation/io/net/executor/execution_context.hpp>
#include <rainy/foundation/io/net/implements/op_queue.hpp>
#include <rainy/foundation/io/net/implements/operation.hpp>
#include <rainy/foundation/memory/shared_ptr.hpp>

namespace rainy::foundation::io::net::implements {
    class strand_executor_service : public execution_context_service_base<strand_executor_service> {
    public:
        class strand_impl {
        public:
            ~strand_impl();

        private:
            friend class strand_executor_service;

            concurrency::mutex *mutex_;
            bool locked_;
            bool shutdown_;

            op_queue<scheduler_operation> waiting_queue_;
            op_queue<scheduler_operation> ready_queue_;
            strand_impl *next_;
            strand_impl *prev_;
            strand_executor_service *service_;
        };

        using implementation_type = memory::shared_ptr<strand_impl>;

        explicit strand_executor_service(execution_context &context);

        void shutdown() override;

        implementation_type create_implementation();

        template <typename Executor, typename Function, typename Allocator>
        static void strand_executor_service::dispatch(const implementation_type &impl, Executor &ex,
                                                      Function&& function, const Allocator &a) {
            typedef typename type_traits::other_trans::decay<Function>::type function_type;
            if (concurrency::implements::call_stack<strand_impl>::contains(impl.get())) {
                function_type tmp(utility::move(function));
                concurrency::fenced_block b(concurrency::fenced_block::full);
                implements::handler_invoke_helper(tmp, tmp);
                return;
            }
            typedef executor_op<function_type, Allocator> op;
            typename op::ptr p = {utility::addressof(a), op::ptr::allocate(a), 0};
            p.p = new (p.v) op(utility::move(function), a);
            NET_TS_HANDLER_CREATION((impl->service_->context(), *p.p, "strand_executor", impl.get(), 0, "dispatch"));
            // Add the function to the strand and schedule the strand if required.
            const bool first = enqueue(impl, p.p);
            p.v = p.p = 0;
            if (first) {
                ex.dispatch(invoker<Executor>(impl, ex), a);
            }
        }

        template <typename Executor, typename Function, typename Allocator>
        static void post(const implementation_type &impl, Executor &ex, Function&& function, const Allocator &a) {
            typedef typename type_traits::other_trans::decay<Function>::type function_type;
            typedef executor_op<function_type, Allocator> op;
            typename op::ptr p = {utility::addressof(a), op::ptr::allocate(a), 0};
            p.p = new (p.v) op(utility::move(function), a);
            NET_TS_HANDLER_CREATION((impl->service_->context(), *p.p, "strand_executor", impl.get(), 0, "post"));
            const bool first = enqueue(impl, p.p);
            p.v = p.p = 0;
            if (first) {
                ex.post(invoker<Executor>(impl, ex), a);
            }
        }

        template <typename Executor, typename Function, typename Allocator>
        static void defer(const implementation_type &impl, Executor &ex, Function&& function, const Allocator &a) {
            typedef typename type_traits::other_trans::decay<Function>::type function_type;
            typedef executor_op<function_type, Allocator> op;
            typename op::ptr p = {utility::addressof(a), op::ptr::allocate(a), 0};
            p.p = new (p.v) op(utility::move(function), a);
            NET_TS_HANDLER_CREATION((impl->service_->context(), *p.p, "strand_executor", impl.get(), 0, "defer"));
            const bool first = enqueue(impl, p.p);
            p.v = p.p = 0;
            if (first) {
                ex.defer(invoker<Executor>(impl, ex), a);
            }
        }

        static bool running_in_this_thread(const implementation_type &impl);

    private:
        friend class strand_impl;

        template <typename Executor>
        class invoker {
        public:
            invoker(implementation_type impl, Executor &ex) : impl_(utility::move(impl)), work_(ex) {
            }

            invoker(const invoker &other) : impl_(other.impl_), work_(other.work_) {
            }

            invoker(invoker &&other) noexcept : impl_(utility::move(other.impl_)), work_(utility::move(other.work_)) {
            }

            struct on_invoker_exit {
                invoker *this_;

                ~on_invoker_exit() {
                    this_->impl_->mutex_->lock();
                    this_->impl_->ready_queue_.push(this_->impl_->waiting_queue_);
                    const bool more_handlers = this_->impl_->locked_ = !this_->impl_->ready_queue_.empty();
                    this_->impl_->mutex_->unlock();
                    if (more_handlers) {
                        Executor ex(this_->work_.get_executor());
                        memory::recycling_allocator<void> allocator;
                        ex.post(utility::move(*this_), allocator);
                    }
                }
            };

            void operator()() {
                concurrency::implements::call_stack<strand_impl>::context ctx(impl_.get());
                const on_invoker_exit on_exit = {this};
                (void) on_exit;
                while (scheduler_operation *o = impl_->ready_queue_.front()) {
                    std::error_code ec;
                    impl_->ready_queue_.pop();
                    o->complete(impl_.get(), ec, 0);
                }
            }

        private:
            implementation_type impl_;
            executor_work_guard<Executor> work_{};
        };

        static constexpr std::size_t num_mutexes = 193;

        static bool enqueue(const implementation_type &impl, scheduler_operation *op);

        strand_impl *impl_list_;
        concurrency::mutex mutex_;
        memory::nebula_ptr<concurrency::mutex> mutexes_[num_mutexes];
        std::size_t salt_;
    };
}

namespace rainy::foundation::io::net {
    template <typename Executor>
    class strand {
    public:
        using inner_executor_type = Executor;

        strand() : executor_(), impl_(use_service<implements::strand_executor_service>(executor_.context()).create_implementation()) {
        }

        explicit strand(const Executor &e) :
            executor_(e), impl_(use_service<implements::strand_executor_service>(executor_.context()).create_implementation()) {
        }

        strand(const strand &other) noexcept : executor_(other.executor_), impl_(other.impl_) {
        }

        template <typename OtherExecutor>
        strand(const strand<OtherExecutor> &other) noexcept : executor_(other.executor_), impl_(other.impl_) { // NOLINT
        }

        strand &operator=(const strand &other) noexcept = default;

        template <typename OtherExecutor>
        strand &operator=(const strand<OtherExecutor> &other) noexcept {
            executor_ = other.executor_;
            impl_ = other.impl_;
            return *this;
        }

        strand(strand &&other) noexcept : executor_(utility::move(other.executor_)), impl_(utility::move(other.impl_)) {
        }

        template <typename OtherExecutor>
        strand(strand<OtherExecutor> &&other) noexcept : executor_(utility::move(other)), impl_(utility::move(other.impl_)) { // NOLINT
        }

        strand &operator=(strand &&other) noexcept {
            executor_ = utility::move(other);
            impl_ = utility::move(other.impl_);
            return *this;
        }

        template <class OtherExecutor>
        strand &operator=(const strand<OtherExecutor> &&other) noexcept {
            executor_ = utility::move(other);
            impl_ = utility::move(other.impl_);
            return *this;
        }

        ~strand() = default;

        inner_executor_type get_inner_executor() const noexcept {
            return executor_;
        }

        RAINY_NODISCARD execution_context &context() const noexcept {
            return executor_.context();
        }

        void on_work_started() const noexcept {
            executor_.on_work_started();
        }

        void on_work_finished() const noexcept {
            executor_.on_work_finished();
        }

        template <typename Function, typename Allocator>
        void dispatch(Function &&f, const Allocator &a) const {
            implements::strand_executor_service::dispatch(impl_, executor_, utility::move(f), a);
        }

        template <typename Function, typename Allocator>
        void post(Function &&f, const Allocator &a) const {
            implements::strand_executor_service::post(impl_, executor_, utility::move(f), a);
        }

        template <typename Function, typename Allocator>
        void defer(Function &&f, const Allocator &a) const {
            implements::strand_executor_service::defer(impl_, executor_, utility::move(f), a);
        }

        RAINY_NODISCARD bool running_in_this_thread() const noexcept {
            return implements::strand_executor_service::running_in_this_thread(impl_);
        }

        friend bool operator==(const strand &a, const strand &b) noexcept {
            return a.impl_ == b.impl_;
        }

        friend bool operator!=(const strand &a, const strand &b) noexcept {
            return a.impl_ != b.impl_;
        }

    private:
        using implementation_type = implements::strand_executor_service::implementation_type;

        Executor executor_;
        implementation_type impl_;
    };
}

#endif
