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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <rainy/foundation/io/net/io_context.hpp>
#include <windows.h>

namespace rainy::foundation::io::net::implements {
    static constexpr ULONG_PTR COMPLETION_KEY_IMMEDIATE = 1; // post_immediate_completion
    static constexpr ULONG_PTR COMPLETION_KEY_IO = 2; // 真实 I/O overlapped

    struct iocp_op : completion_op {
        OVERLAPPED overlapped{};
        HANDLE associated_handle{INVALID_HANDLE_VALUE};
        DWORD transferred{0};

        explicit iocp_op(fn_type f) noexcept : completion_op(f) {
        }

        static iocp_op *from_overlapped(OVERLAPPED *ov) noexcept {
            return reinterpret_cast<iocp_op *>(ov);
        }
    };

    static void wakeup_op_fn(completion_op *, const op_result &, bool) noexcept {
    }

    static completion_op wakeup_op{&wakeup_op_fn};

    class win_iocp_impl final : public io_context_impl_base {
    public:
        explicit win_iocp_impl(int concurrency_hint) noexcept : concurrency_hint_(concurrency_hint) {
        }

        ~win_iocp_impl() override {
            destroy();
        }

        concurrency::thrd_result init(int concurrency_hint) noexcept override {
            concurrency_hint_ = concurrency_hint;
            iocp_handle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0,
                                                    static_cast<DWORD>(concurrency_hint <= 0 ? 0 : concurrency_hint));
            if (iocp_handle_ == nullptr || iocp_handle_ == INVALID_HANDLE_VALUE) {
                return concurrency::thrd_result::error;
            }
            return concurrency::thrd_result::success;
        }

        void destroy() noexcept override {
            if (iocp_handle_ != INVALID_HANDLE_VALUE) {
                ::CloseHandle(iocp_handle_);
                iocp_handle_ = INVALID_HANDLE_VALUE;
            }
        }

        void on_work_started() noexcept override {
            work_count_.fetch_add(1, concurrency::memory_order_seq_cst);
        }

        void on_work_finished() noexcept override {
            if (work_count_.fetch_sub(1, concurrency::memory_order_seq_cst) == 1) {
                // 工作归零，唤醒所有阻塞中的 run()
                post_wakeup();
            }
        }

        void post_immediate_completion(completion_op *op, bool) noexcept override {
            work_count_.fetch_add(1, concurrency::memory_order_seq_cst);
            ::PostQueuedCompletionStatus(iocp_handle_, 0, COMPLETION_KEY_IMMEDIATE, reinterpret_cast<OVERLAPPED *>(op));
        }

        concurrency::thrd_result associate_handle(completion_op * /*op*/, std::uintptr_t fd, void * /*extra*/) noexcept override {
            HANDLE h = reinterpret_cast<HANDLE>(fd);
            HANDLE result = ::CreateIoCompletionPort(h, iocp_handle_, COMPLETION_KEY_IO, 0);
            return result != nullptr ? concurrency::thrd_result::success : concurrency::thrd_result::error;
        }

        void stop() noexcept override {
            if (!stopped_.exchange(true, concurrency::memory_order_acq_rel)) {
                post_wakeup();
            }
        }

        void restart() noexcept override {
            stopped_.store(false, concurrency::memory_order_seq_cst);
        }

        bool stopped() const noexcept override {
            return stopped_.load(concurrency::memory_order_seq_cst);
        }

        std::size_t run() override {
            std::size_t total = 0;
            in_event_loop_ = true;

            while (!stopped_.load(concurrency::memory_order_seq_cst)) {
                if (work_count_.load(concurrency::memory_order_seq_cst) <= 0) {
                    break;
                }
                std::size_t count = dequeue_and_dispatch(INFINITE);
                total += count;
            }

            in_event_loop_ = false;
            return total;
        }

        std::size_t run_one() override {
            in_event_loop_ = true;
            std::size_t count = 0;
            if (!stopped_.load(concurrency::memory_order_seq_cst)) {
                if (work_count_.load(concurrency::memory_order_seq_cst) > 0) {
                    count = dequeue_and_dispatch(INFINITE);
                }
            }
            in_event_loop_ = false;
            return count;
        }

        std::size_t run_one_for(std::uint64_t timeout_ns) override {
            DWORD timeout_ms = (timeout_ns == 0) ? 0 : static_cast<DWORD>((timeout_ns + 999'999) / 1'000'000);
            in_event_loop_ = true;
            std::size_t count = 0;
            if (!stopped_.load(concurrency::memory_order_seq_cst)) {
                count = dequeue_and_dispatch(timeout_ms);
            }
            in_event_loop_ = false;
            return count;
        }

        std::size_t poll() noexcept override {
            std::size_t total = 0;
            in_event_loop_ = true;
            while (!stopped_.load(concurrency::memory_order_seq_cst)) {
                std::size_t count = dequeue_and_dispatch(0); // 非阻塞
                if (count == 0) {
                    break;
                }
                total += count;
            }
            in_event_loop_ = false;
            return total;
        }

        std::size_t poll_one() noexcept override {
            in_event_loop_ = true;
            std::size_t count = 0;
            if (!stopped_.load(concurrency::memory_order_seq_cst)) {
                count = dequeue_and_dispatch(0); // 非阻塞，只取一个
            }
            in_event_loop_ = false;
            return count;
        }

        bool running_in_this_thread() const noexcept override {
            return in_event_loop_;
        }

        int concurrency_hint() const noexcept override {
            return concurrency_hint_;
        }

    private:
        void post_wakeup() noexcept {
            ::PostQueuedCompletionStatus(iocp_handle_, 0, 0, reinterpret_cast<OVERLAPPED *>(&wakeup_op));
        }

        std::size_t dequeue_and_dispatch(DWORD timeout_ms) {
            DWORD bytes = 0;
            ULONG_PTR key = 0;
            OVERLAPPED *ov = nullptr;
            BOOL ok = ::GetQueuedCompletionStatus(iocp_handle_, &bytes, &key, &ov, timeout_ms);
            // 超时：ov == nullptr 且 GetLastError() == WAIT_TIMEOUT
            if (ov == nullptr) {
                return 0;
            }
            // 唤醒包（stop / on_work_finished 投递，key == 0）：丢弃
            if (ov == reinterpret_cast<OVERLAPPED *>(&wakeup_op)) {
                return 0;
            }
            // 即时完成包（post_immediate_completion 投递）
            if (key == COMPLETION_KEY_IMMEDIATE) {
                auto *op = reinterpret_cast<completion_op *>(ov);
                op_result result{op, 0, 0};
                // 消耗掉之前 post_immediate_completion 增加的 work_count_
                // 必须在 complete() 之前减，避免 complete() 内部递归 post 时
                // work_count_ 短暂归零触发误唤醒
                on_work_finished();
                op->complete(result, false);
                return 1;
            }
            // 真实 I/O 完成包（key == COMPLETION_KEY_IO）
            iocp_op *iop = iocp_op::from_overlapped(ov);
            op_result result{};
            result.user_data = iop;
            result.bytes_transferred = static_cast<std::size_t>(bytes);
            result.error_code = ok ? 0 : static_cast<int>(::GetLastError());
            iop->complete(result, result.error_code == ERROR_OPERATION_ABORTED);
            return 1;
        }

        HANDLE iocp_handle_{INVALID_HANDLE_VALUE};
        int concurrency_hint_{0};

        static thread_local bool in_event_loop_;
    };

    thread_local bool win_iocp_impl::in_event_loop_ = false;
}

namespace rainy::foundation::io::net::implements {
    memory::nebula_ptr<io_context_impl_base> create_io_context_impl(int concurrency_hint) {
        auto impl = memory::make_nebula<win_iocp_impl>(concurrency_hint);
        impl->init(concurrency_hint);
        return impl;
    }
}
