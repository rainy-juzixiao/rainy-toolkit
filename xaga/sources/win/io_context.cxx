/*
 * Copyright 2025 rainy-juzixiao
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

#include <rainy/core/core.hpp>

#if RAINY_USING_WINDOWS
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <rainy/foundation/pal/asio/io_context.hpp>
#include <rainy/collections/dense_set.hpp>
#include <rainy/foundation/functional/delegate.hpp>
#include <rainy/foundation/pal/atomicinfra.hpp>
#include <rainy/utility/pair.hpp>
#include <unordered_set>
#include <windows.h>
#include <memory_resource>

namespace rainy::foundation::pal::asio::implements {
    using overlapped_adpater = OVERLAPPED;

    // 内部任务标识
    static constexpr ULONG_PTR INTERNAL_TASK_KEY = 0xFFFFFFFFUL;
    static constexpr ULONG_PTR STOP_SIGNAL_KEY = 0xFFFFFFFEUL;

    struct iocp_context_t {
        iocp_context_t() = default;
        iocp_context_t(HANDLE iocp) : iocp{iocp} {
        }

        HANDLE iocp;
        std::mutex task_mutex{};
        std::queue<utility::pair<task_func, void *>> tasks{};
        atomicinfra::atomic_bool stopped{false};
        atomicinfra::atomic_int outstanding_work{0};
        atomicinfra::atomic_bool work_guard{false};
        overlapped_adpater internal_overlapped{};
        overlapped_adpater stop_overlapped{};
    };

    // 前向声明
    struct callback_manager;

    struct internal_callback : overlapped_adpater, implements::callback_t {
        internal_callback(callback_t &&call, callback_manager *manager) :
            OVERLAPPED{}, callback_t{utility::move(call)}, manager_{manager}, active_{true} {
        }

        ~internal_callback() {
            // 确保回调被清理
            if (cb) {
                cb.reset();
            }
        }

        // 移除循环引用的self_ref，改用管理器模式
        callback_manager *manager_;
        atomicinfra::atomic_bool active_;
    };

    struct callback_manager {
        callback_manager(std::pmr::memory_resource *res) : allocator{res}, active_callbacks_{res} {
        }

        ~callback_manager() {
            cleanup_all();
        }

        // 创建并注册回调
        internal_callback *create_callback(callback_t &&cb) {
            return (*active_callbacks_.insert(utility::construct_at(allocator.allocate(1), utility::move(cb), this)).first);
        }

        void deactivate_callback(internal_callback *cb) {
            if (cb) {
                cb->active_.store(false, foundation::pal::atomicinfra::memory_order_release);
            }
        }

        // 清理非活跃的回调
        void cleanup_inactive() {
            auto it = active_callbacks_.begin();
            while (it != active_callbacks_.end()) {
                auto *cb = *it;
                if (!cb->active_.load(foundation::pal::atomicinfra::memory_order_acquire)) {
                    it = active_callbacks_.erase(it);
                    std::destroy_at(cb);
                    allocator.deallocate(cb, 1);
                } else {
                    ++it;
                }
            }
        }

        // 清理所有回调
        void cleanup_all() {
            for (auto *cb: active_callbacks_) {
                if (cb) {
                    cb->active_.store(false, foundation::pal::atomicinfra::memory_order_release);
                    if (cb->cb) {
                        cb->cb.reset();
                    }
                    std::destroy_at(cb);
                    allocator.deallocate(cb, 1);
                }
            }
            active_callbacks_.clear();
        }

        // 获取活跃回调数量（用于监控）
        std::size_t active_count() const {
            return active_callbacks_.size();
        }

    private:
        std::unordered_set<internal_callback *, std::hash<internal_callback *>, std::equal_to<internal_callback *>,
                           std::pmr::polymorphic_allocator<internal_callback *>>
            active_callbacks_;
        std::pmr::polymorphic_allocator<internal_callback> allocator;
    };

    // 单例管理器
    static callback_manager &get_callback_manager() {
        thread_local std::pmr::unsynchronized_pool_resource pools;
        thread_local callback_manager instance(&pools);
        return instance;
    }

    void callback_t::invoke_callback(DWORD last_error, DWORD byte_transfered) {
        auto *internal_cb = static_cast<internal_callback *>(this);
        // 检查回调是否仍然活跃
        if (!internal_cb->active_.load(foundation::pal::atomicinfra::memory_order_acquire)) {
            return;
        }
        // 标记为非活跃
        get_callback_manager().deactivate_callback(internal_cb);
        // 执行回调
        if (cb) {
            cb->invoke(last_error, byte_transfered);
            cb.reset();
        }
    }

    inline void process_callback_overlapped_event(OVERLAPPED *_ov, DWORD last_error, DWORD bytes) {
        internal_callback *ovl_res = reinterpret_cast<internal_callback *>(_ov);
        if (ovl_res && ovl_res->active_.load(foundation::pal::atomicinfra::memory_order_acquire)) {
            ovl_res->invoke_callback(last_error, bytes);
        }
    }

    static iocp_context_t *get_ctx(io_ctx ctx) {
        return reinterpret_cast<iocp_context_t *>(ctx);
    }

    void add_work_guard(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (context) {
            context->work_guard.store(true, foundation::pal::atomicinfra::memory_order_release);
        }
    }

    void remove_work_guard(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (context) {
            context->work_guard.store(false, foundation::pal::atomicinfra::memory_order_release);
            if (context->outstanding_work.load(foundation::pal::atomicinfra::memory_order_acquire) == 0) {
                stop_io_context(ctx);
            }
        }
    }

    static void work_started(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (context) {
            context->outstanding_work.fetch_add(1, foundation::pal::atomicinfra::memory_order_acq_rel);
        }
    }

    static void work_finished(iocp_context_t *ctx) {
        if (ctx) {
            int remaining = ctx->outstanding_work.fetch_sub(1, foundation::pal::atomicinfra::memory_order_acq_rel);
            // 每次工作完成都发送一个通知，让等待的线程检查状态
            // 这样可以避免线程卡死
            PostQueuedCompletionStatus(ctx->iocp, 0, STOP_SIGNAL_KEY, &ctx->stop_overlapped);
            // 当工作计数降为0时，需要唤醒所有等待中的线程
            if (remaining == 1) {
                for (int i = 0; i < core::max_threads; ++i) {
                    PostQueuedCompletionStatus(ctx->iocp, 0, STOP_SIGNAL_KEY, &ctx->stop_overlapped);
                }
            }
        }
    }

    // 定期清理函数
    static void periodic_cleanup() {
        get_callback_manager().cleanup_inactive();
    }

    io_ctx create_io_context() {
        HANDLE cp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        if (cp == nullptr) {
            DWORD error = GetLastError();
            switch (error) {
                case ERROR_INVALID_HANDLE:
                    errno = EINVAL;
                    break;
                case ERROR_NOT_ENOUGH_MEMORY:
                    errno = ENOMEM;
                    break;
                default:
                    errno = EIO;
                    break;
            }
            return core::invalid_handle;
        }
        auto *ctx = new iocp_context_t{cp};
        ZeroMemory(&ctx->internal_overlapped, sizeof(OVERLAPPED));
        ZeroMemory(&ctx->stop_overlapped, sizeof(OVERLAPPED));
        return reinterpret_cast<io_ctx>(ctx);
    }

    void destroy_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return;
        }

        // 停止context
        stop_io_context(ctx);

        // 清理待处理的任务
        {
            std::lock_guard<std::mutex> lock(context->task_mutex);
            while (!context->tasks.empty()) {
                context->tasks.pop();
            }
        }

        // 清理回调
        get_callback_manager().cleanup_all();

        CloseHandle(context->iocp);
        delete context;
    }

    int register_io(io_ctx ctx, core::handle publisher_sock, core::handle key) {
        auto *context = get_ctx(ctx);
        if (!context) {
            errno = EINVAL;
            return -1;
        }
        HANDLE result = nullptr;
        if (key != 0) {
            result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(publisher_sock), context->iocp,
                                            reinterpret_cast<ULONG_PTR>(&process_callback_overlapped_event), 0);
        } else {
            result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(publisher_sock), context->iocp, key, 0);
        }
        if (!result) {
            DWORD error = GetLastError();
            switch (error) {
                case ERROR_INVALID_HANDLE:
                    errno = EBADF;
                    break;
                case ERROR_INVALID_PARAMETER:
                    errno = EINVAL;
                    break;
                default:
                    errno = EIO;
                    break;
            }
            return -1;
        }
        return 0;
    }

    int unregister_io(io_ctx ctx, core::handle sock) {
        if (!get_ctx(ctx)) {
            errno = EINVAL;
            return -1;
        }
        (void) sock;
        return 0; // Windows IOCP 无需明确注销
    }

    void stop_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return;
        }
        // 标记停止
        context->stopped = true;
        // 发送停止信号
        PostQueuedCompletionStatus(context->iocp, 0, STOP_SIGNAL_KEY, &context->stop_overlapped);
    }

    core::handle extract_handle(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return 0;
        }
        return reinterpret_cast<core::handle>(context->iocp);
    }

    core::handle extract_overlapped(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return 0;
        }
        return reinterpret_cast<core::handle>(&context->internal_overlapped);
    }

    void post_task(io_ctx ctx, task_func func, void *arg) {
        auto *context = get_ctx(ctx);
        if (!context || context->stopped.load()) {
            return;
        }
        // 增加待处理工作计数
        work_started(ctx);
        {
            std::lock_guard<std::mutex> lock(context->task_mutex);
            context->tasks.emplace(func, arg);
        }
        PostQueuedCompletionStatus(context->iocp, 0, INTERNAL_TASK_KEY, &context->internal_overlapped);
    }

    template <bool Batch>
    static int execute_pending_tasks(iocp_context_t *context) {
        int count = 0;
        std::unique_lock<std::mutex> lock(context->task_mutex);
        if constexpr (Batch) {
            while (!context->tasks.empty()) {
                auto task = context->tasks.front();
                context->tasks.pop();
                lock.unlock();
                if (task.first) {
                    task.first(task.second);
                    work_finished(context); // 这里减少工作计数
                    ++count;
                } else {
                    work_finished(context); // 即使任务为空也要减少计数
                }
                lock.lock();
            }
        } else {
            if (context->tasks.empty()) {
                return 0;
            }
            auto task = context->tasks.front();
            context->tasks.pop();
            lock.unlock();
            if (task.first) {
                task.first(task.second);
                work_finished(context);
                return 1;
            }
            work_finished(context);
            return 0;
        }
        return count;
    }

    int run_one_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return 0;
        }
        // 定期清理
        thread_local int cleanup_counter = 0;
        if (++cleanup_counter % 450 == 0) {
            periodic_cleanup();
        }
        DWORD bytes = 0;
        ULONG_PTR completion_key = 0;
        LPOVERLAPPED ov = nullptr;
        // 阻塞等待一个 I/O 事件，等待直到有事件或者超时
        BOOL ok = GetQueuedCompletionStatus(context->iocp, &bytes, &completion_key, &ov, INFINITE);
        DWORD error = ok ? ERROR_SUCCESS : GetLastError();
        // 处理停止信号
        if (completion_key == STOP_SIGNAL_KEY && ov == &context->stop_overlapped) {
            if (context->outstanding_work == 0) {
                stop_io_context(ctx);
                return 0; // 如果没有剩余工作，返回并停止
            }
            // 否则继续处理，检查是否还有任务
            return 0;
        }
        // 处理内部任务
        if (completion_key == INTERNAL_TASK_KEY && ov == &context->internal_overlapped) {
            return execute_pending_tasks<false>(context); // 执行一个任务
        }
        // 处理正常回调
        if (ov) {
            process_callback_overlapped_event(ov, error, bytes);
            return 1;
        }
        // 其他未处理的情况（如果当前没有任务，且没有信号，退出）
        return 0;
    }

    int run_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return 0;
        }
        constexpr std::size_t max_events = 64;
        OVERLAPPED_ENTRY events[max_events];
        int total_count = 0;
        thread_local int cleanup_counter = 0;
        while (!context->stopped) {
            if (++cleanup_counter % 1000 == 0) {
                periodic_cleanup();
            }
            ULONG removed = 0;
            BOOL ok = GetQueuedCompletionStatusEx(context->iocp, events, max_events, &removed, INFINITE, FALSE);
            if (!ok) {
                DWORD error = GetLastError();
                if (error == ERROR_ABANDONED_WAIT_0) {
                    break;
                }
                return -1;
            }
            for (ULONG i = 0; i < removed; ++i) {
                OVERLAPPED_ENTRY &entry = events[i];
                DWORD bytes = entry.dwNumberOfBytesTransferred;
                ULONG_PTR completion_key = entry.lpCompletionKey;
                LPOVERLAPPED ov = entry.lpOverlapped;
                // 处理停止信号
                if (completion_key == STOP_SIGNAL_KEY && ov == &context->stop_overlapped) {
                    if (context->outstanding_work.load(foundation::pal::atomicinfra::memory_order_acquire) == 0) {
                        stop_io_context(ctx);
                        return total_count;
                    }
                    continue;
                }
                // 处理内部任务
                if (completion_key == INTERNAL_TASK_KEY && ov == &context->internal_overlapped) {
                    total_count += execute_pending_tasks<true>(context);
                    continue;
                }
                // 处理正常回调
                auto overlapped = entry.lpOverlapped;
                if (overlapped) {
                    process_callback_overlapped_event(overlapped, ERROR_SUCCESS, bytes);
                }
            }
        }
        return total_count;
    }

    int poll_one_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return 0;
        }
        DWORD bytes = 0;
        ULONG_PTR completion_key = 0;
        LPOVERLAPPED ov = nullptr;
        BOOL ok = GetQueuedCompletionStatus(context->iocp, &bytes, &completion_key, &ov, 0);
        DWORD error = ok ? ERROR_SUCCESS : GetLastError();
        // 处理停止信号
        if (completion_key == STOP_SIGNAL_KEY && ov == &context->stop_overlapped) {
            if (context->stopped.load()) {
                return 0; // 任务已停止
            }
        }
        // 处理内部任务
        if (completion_key == INTERNAL_TASK_KEY && ov == &context->internal_overlapped) {
            return execute_pending_tasks<false>(context); // 执行挂起的任务
        }
        using overlapped_proc_func = void (*)(const LPOVERLAPPED, DWORD, DWORD);
        // 处理正常 I/O 完成事件
        if (ov != nullptr && completion_key != INTERNAL_TASK_KEY && completion_key != STOP_SIGNAL_KEY) {
            // completion_key 就是原始注册时传入的回调函数指针
            auto *callback = reinterpret_cast<overlapped_proc_func>(completion_key);
            if (callback) {
                bool success = (ok != FALSE);
                callback(ov, success ? ERROR_SUCCESS : error, bytes); // 调用回调函数
            }
            return 1; // 正常处理完一个事件
        }
        if (!ok && error != ERROR_ABANDONED_WAIT_0) {
            // I/O 操作失败，但仍然需要通知处理程序
            if (ov != nullptr && completion_key != INTERNAL_TASK_KEY && completion_key != STOP_SIGNAL_KEY) {
                auto *callback = reinterpret_cast<overlapped_proc_func>(completion_key);
                if (callback) {
                    callback(ov, error, bytes); // 调用回调函数并传递错误信息
                }
                return 1; // 返回处理失败
            }
            return -1; // 出现异常错误
        }
        return 0; // 未处理的情况
    }

    int poll_io_context(io_ctx ctx) {
        int count = 0;
        int result;
        while ((result = poll_one_io_context(ctx)) > 0) {
            ++count;
        }
        return count;
    }

    void reset_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(context->task_mutex);
            std::queue<utility::pair<task_func, void *>> empty;
            std::swap(context->tasks, empty);
        }
        context->stopped = false;
        context->outstanding_work = 0;
        get_callback_manager().cleanup_all(); // 清理所有回调
    }

    bool io_context_stopped(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context) {
            return true;
        }
        return context->stopped.load();
    }

    core::handle get_callback(callback_t &&callback) {
        auto *internal_cb = get_callback_manager().create_callback(utility::move(callback));
        return reinterpret_cast<core::handle>(internal_cb);
    }
}

#endif
