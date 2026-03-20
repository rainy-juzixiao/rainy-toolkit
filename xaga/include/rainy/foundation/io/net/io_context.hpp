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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTORHPP
#define RAINY_FOUNDATION_IO_NET_EXECUTORHPP
#include <chrono>
#include <rainy/foundation/io/net/executor.hpp>
#include <rainy/foundation/concurrency/atomic.hpp>

namespace rainy::foundation::io::net::implements {
    struct op_result {
        void *user_data;
        std::size_t bytes_transferred;
        int error_code;
    };

    struct completion_op {
        using fn_type = void (*)(completion_op *self, const op_result &result, bool is_cancelled);

        explicit completion_op(fn_type f) noexcept : fn(f), next(nullptr) {
        }

        void complete(const op_result &result, bool cancelled) {
            fn(this, result, cancelled);
        }

        fn_type fn;
        completion_op *next;
    };

    template <typename Func>
    class function_completion_op final : public completion_op {
    public:
        template <typename Fx,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Func, Fx &&>, int> = 0>
        explicit function_completion_op(Fx &&func) noexcept(std::is_nothrow_constructible_v<Func, Fx &&>) :
            completion_op(&do_complete), func_(utility::forward<Fx>(func)) {
        }

        function_completion_op(const function_completion_op &) = delete;
        function_completion_op &operator=(const function_completion_op &) = delete;

    private:
        static void do_complete(completion_op *self, const op_result & /*result*/, bool is_cancelled) {
            auto *op = static_cast<function_completion_op *>(self);
            Func func(utility::move(op->func_));
            delete op;
            if (!is_cancelled) {
                func();
            }
        }

        Func func_;
    };

    template <typename Func>
    RAINY_NODISCARD function_completion_op<type_traits::other_trans::decay_t<Func>> *make_function_op(Func &&func) {
        return new function_completion_op<type_traits::other_trans::decay_t<Func>>(std::forward<Func>(func));
    }

    class io_context_impl_base {
    public:
        virtual ~io_context_impl_base() = default;

        /**
         * @brief 初始化平台资源（IOCP handle / io_uring 实例等）
         * @param concurrency_hint 建议并发线程数，0 表示不限制
         * @return success / 平台相关错误码
         */
        virtual concurrency::thrd_result init(int concurrency_hint) noexcept = 0;

        /**
         * @brief 销毁平台资源，释放内核对象
         */
        virtual void destroy() noexcept = 0;

        /**
         * @brief 阻塞运行，直到 stop() 被调用或没有更多工作
         * @return 已处理的完成事件数量
         */
        virtual std::size_t run() = 0;

        /**
         * @brief 运行至多一个完成事件
         * @return 1 表示处理了一个事件，0 表示没有可用事件
         */
        virtual std::size_t run_one() = 0;

        /**
         * @brief 带超时的 run_one
         * @param timeout_ns 超时纳秒数，0 表示立即返回（poll 语义）
         * @return 1 表示处理了一个事件，0 表示超时或无事件
         */
        virtual std::size_t run_one_for(std::uint64_t timeout_ns) = 0;

        /**
         * @brief 非阻塞地处理所有当前就绪的完成事件
         * @return 已处理数量
         */
        virtual std::size_t poll() = 0;

        /**
         * @brief 非阻塞地处理至多一个就绪的完成事件
         * @return 1 或 0
         */
        virtual std::size_t poll_one() = 0;

        /**
         * @brief 通知事件循环停止，所有阻塞中的 run/run_one 应尽快返回
         */
        virtual void stop() noexcept = 0;

        /**
         * @brief 重置 stopped 状态，使 run() 可以再次被调用
         */
        virtual void restart() noexcept = 0;

        /**
         * @brief 查询是否处于 stopped 状态
         */
        RAINY_NODISCARD virtual bool stopped() const noexcept = 0;

        /**
         * @brief 增加未完成工作计数，阻止 run() 因"无工作"而自动退出
         */
        virtual void on_work_started() noexcept = 0;

        /**
         * @brief 减少未完成工作计数
         */
        virtual void on_work_finished() noexcept = 0;

        /**
         * @brief 将一个已就绪（无需等待 I/O）的 op 直接入队到本地就绪队列
         *
         * 用于 post() / dispatch() 的纯函数任务投递。
         * 入队后调用 wakeup() 唤醒阻塞中的 run_one。
         *
         * @param op 待入队的完成操作，生命周期由调用方保证直至 complete() 返回
         */
        virtual void post_immediate_completion(completion_op *op, bool is_continuation) noexcept = 0;

        /**
         * @brief 平台专属：将一个 completion_op 与已发起的 I/O 操作关联
         *
         * 在 IOCP 上：将 overlapped 结构嵌入 op，并绑定到完成端口。
         * 在 io_uring 上：填充 SQE 并提交，SQE 的 user_data 指向 op。
         *
         * @param op      完成回调
         * @param fd      文件描述符 / HANDLE
         * @param extra   平台相关附加参数（可为 nullptr）
         */
        virtual concurrency::thrd_result associate_handle(completion_op *op, std::uintptr_t fd, void *extra) noexcept = 0;

        /**
         * @brief 判断当前线程是否正在此 impl 的事件循环中运行
         */
        RAINY_NODISCARD virtual bool running_in_this_thread() const noexcept = 0;

        /**
         * @brief 返回创建时指定的并发提示
         */
        RAINY_NODISCARD virtual int concurrency_hint() const noexcept = 0;

    protected:
        concurrency::atomic<long> work_count_{0};
        concurrency::atomic<bool> stopped_{false};
    };

    RAINY_TOOLKIT_API memory::nebula_ptr<io_context_impl_base> create_io_context_impl(int concurrency_hint);
}

namespace rainy::foundation::io::net {
    class RAINY_TOOLKIT_API io_context : public execution_context {
    public:
        class RAINY_TOOLKIT_API executor_type {
        public:
            executor_type(const executor_type &other) noexcept;
            executor_type(executor_type &&other) noexcept;
            executor_type &operator=(const executor_type &other) noexcept;
            executor_type &operator=(executor_type &&other) noexcept;

            RAINY_NODISCARD bool running_in_this_thread() const noexcept;
            RAINY_NODISCARD io_context &context() const noexcept;
            void on_work_started() const noexcept;
            void on_work_finished() const noexcept;

            template <typename Func, typename ProtoAllocator>
            void dispatch(Func &&f, const ProtoAllocator &a) const {
                if (running_in_this_thread()) {
                    utility::forward<Func>(f)();
                    return;
                }
                auto *op = implements::make_function_op(std::forward<Func>(f));
                ctx_->impl_->post_immediate_completion(op, /*is_continuation=*/false);
            }

            template <typename Func, typename ProtoAllocator>
            void post(Func &&f, const ProtoAllocator &a) const {
                auto *op = implements::make_function_op(std::forward<Func>(f));
                ctx_->impl_->post_immediate_completion(op, /*is_continuation=*/false);
            }

            template <typename Func, typename ProtoAllocator>
            void defer(Func &&f, const ProtoAllocator &a) const {
                auto *op = implements::make_function_op(std::forward<Func>(f));
                ctx_->impl_->post_immediate_completion(op, /*is_continuation=*/true);
            }

        private:
            friend class io_context;

            explicit executor_type(io_context *ctx) noexcept : ctx_(ctx) {
            }

            io_context *ctx_{nullptr};
        };

        using count_type = std::size_t;

        io_context();

        ~io_context() override;

        explicit io_context(int concurrency_hint);

        io_context(const io_context &) = delete;

        io_context &operator=(const io_context &) = delete;

        executor_type get_executor() noexcept;

        count_type run();

        template <typename Rep, typename Period>
        count_type run_for(const std::chrono::duration<Rep, Period> &rel_time) {
            return run_until(std::chrono::steady_clock::now() + rel_time);
        }

        template <typename Clock, typename Duration>
        count_type run_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
            count_type total = 0;
            while (!stopped()) {
                auto now = Clock::now();
                if (now >= abs_time) {
                    break;
                }
                auto remaining_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(abs_time - now).count();
                std::size_t n = impl_->run_one_for(static_cast<std::uint64_t>(remaining_ns > 0 ? remaining_ns : 0));
                total += n;
            }
            return total;
        }

        count_type run_one();

        template <typename Rep, typename Period>
        count_type run_one_for(const std::chrono::duration<Rep, Period> &rel_time) {
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rel_time).count();
            return impl_->run_one_for(static_cast<std::uint64_t>(ns > 0 ? ns : 0));
        }

        template <typename Clock, typename Duration>
        count_type run_one_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
            auto now = Clock::now();
            if (now >= abs_time) {
                return 0;
            }
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(abs_time - now).count();
            return impl_->run_one_for(static_cast<std::uint64_t>(ns));
        }

        count_type poll();
        count_type poll_one();

        void stop();
        bool stopped() const noexcept;
        void restart();

    private:
        memory::nebula_ptr<implements::io_context_impl_base> impl_;
    };
}

#endif
