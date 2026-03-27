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
#ifndef RAINY_FOUNDATION_IO_NET_IMPLEMENTS_IO_CONTEXT_HPP
#define RAINY_FOUNDATION_IO_NET_IMPLEMENTS_IO_CONTEXT_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/atomic.hpp>
#include <rainy/foundation/concurrency/pal.hpp>
#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>

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
        void *io_handle{nullptr};
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
            func();
        }

        Func func_;
    };

    template <typename Func>
    class immediate_op final : public completion_op {
    public:
        template <typename Fx,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Func, Fx &&>, int> = 0>
        explicit immediate_op(Fx &&func) noexcept(std::is_nothrow_constructible_v<Func, Fx &&>) :
            completion_op(&do_complete), func_(utility::forward<Fx>(func)) {
        }

        immediate_op(const immediate_op &) = delete;
        immediate_op &operator=(const immediate_op &) = delete;

    private:
        static void do_complete(completion_op *self, const op_result & /*result*/, bool is_cancelled) {
            auto *op = static_cast<immediate_op *>(self);
            Func func = utility::move(op->func_);
            delete op;
            if (!is_cancelled) {
                func();
            }
        }

        Func func_;
    };

    template <typename Func>
    RAINY_NODISCARD immediate_op<std::decay_t<Func>> *make_immediate_op(Func &&func) {
        return new immediate_op<std::decay_t<Func>>(utility::forward<Func>(func));
    }

    template <typename Func>
    class io_completion_op final : public completion_op {
    public:
        template <typename Fx,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Func, Fx &&>, int> = 0>
        explicit io_completion_op(Fx &&func) noexcept(std::is_nothrow_constructible_v<Func, Fx &&>) :
            completion_op(&do_complete), func_(utility::forward<Fx>(func)) {
        }

        io_completion_op(const io_completion_op &) = delete;
        io_completion_op &operator=(const io_completion_op &) = delete;

    private:
        static void do_complete(completion_op *self, const op_result &result, bool is_cancelled) {
            auto *op = static_cast<io_completion_op *>(self);
            Func func = utility::move(op->func_);
            delete op;
            func(result, is_cancelled);
        }

        Func func_;
    };

    template <typename Func>
    RAINY_NODISCARD io_completion_op<std::decay_t<Func>> *make_io_completion_op(Func &&func) {
        return new io_completion_op<std::decay_t<Func>>(utility::forward<Func>(func));
    }

    template <typename Func>
    RAINY_NODISCARD immediate_op<std::decay_t<Func>> *make_function_op(Func &&func) {
        return make_immediate_op(utility::forward<Func>(func));
    }

    template <typename Func>
    RAINY_NODISCARD function_completion_op<type_traits::other_trans::decay_t<Func>> *make_function_op(Func &&func) {
        return new function_completion_op<type_traits::other_trans::decay_t<Func>>(utility::forward<Func>(func));
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

#endif
