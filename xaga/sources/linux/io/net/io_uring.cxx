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
#include <liburing.h>
#include <mutex>
#include <queue>
#include <rainy/foundation/io/net/io_context.hpp>
#include <sys/eventfd.h>

namespace rainy::foundation::io::net::implements {
    static void nop_fn(completion_op *, const op_result &, bool) noexcept {
    }

    completion_op non_op{&nop_fn};
}

namespace rainy::foundation::io::net::implements {
    class io_uring_impl final : public io_context_impl_base {
    public:
        explicit io_uring_impl(const int concurrency_hint) noexcept : concurrency_hint_(concurrency_hint) {
        }

        ~io_uring_impl() override {
            destroy();
        }

        concurrency::thrd_result init(const int concurrency_hint) noexcept override {
            concurrency_hint_ = concurrency_hint;
            unsigned queue_depth = (concurrency_hint <= 0) ? 256u : static_cast<unsigned>(concurrency_hint) * 32u;
            if (queue_depth < 64) {
                queue_depth = 64;
            }
            if (queue_depth > 4096) {
                queue_depth = 4096;
            }
            if (const int ret = ::io_uring_queue_init(queue_depth, &ring_, /*flags=*/0); ret < 0) {
                return concurrency::thrd_result::error;
            }
            ring_initialized_ = true;
            event_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            if (event_fd_ < 0) {
                ::io_uring_queue_exit(&ring_);
                ring_initialized_ = false;
                return concurrency::thrd_result::error;
            }
            return concurrency::thrd_result::success;
        }

        void destroy() noexcept override {
            if (event_fd_ >= 0) {
                ::close(event_fd_);
                event_fd_ = -1;
            }
            if (ring_initialized_) {
                ::io_uring_queue_exit(&ring_);
                ring_initialized_ = false;
            }
        }

        std::size_t run() override {
            std::size_t total = 0;
            in_event_loop_ = true;
            while (!stopped_.load(concurrency::memory_order_acquire)) {
                // 先把本地即时队列清空
                total += drain_ready_queue();
                if (stopped_.load(concurrency::memory_order_acquire)) {
                    break;
                }
                if (work_count_.load(concurrency::memory_order_acquire) <= 0) {
                    break;
                }
                total += harvest(1, nullptr);
            }
            in_event_loop_ = false;
            return total;
        }

        std::size_t run_one() override {
            if (stopped_.load(concurrency::memory_order_acquire)) {
                return 0;
            }
            in_event_loop_ = true;
            {
                concurrency::unique_lock lock(ready_mutex_);
                if (!ready_queue_.empty()) {
                    completion_op *op = ready_queue_.front();
                    ready_queue_.pop();
                    lock.unlock();
                    in_event_loop_ = false;
                    op_result r{op, 0, 0}; // NOLINT
                    op->complete(r, false);
                    return 1;
                }
            }
            if (work_count_.load(concurrency::memory_order_acquire) <= 0) {
                in_event_loop_ = false;
                return 0;
            }
            std::size_t n = harvest(1, nullptr);
            in_event_loop_ = false;
            return n;
        }

        std::size_t run_one_for(std::uint64_t timeout_ns) override {
            in_event_loop_ = true;
            std::size_t n = 0;
            if (!stopped_.load(concurrency::memory_order_acquire)) {
                if (timeout_ns == 0) {
                    n = harvest(0, nullptr); // peek
                } else {
                    ::__kernel_timespec ts{};
                    ts.tv_sec = static_cast<long long>(timeout_ns / 1'000'000'000ULL);
                    ts.tv_nsec = static_cast<long long>(timeout_ns % 1'000'000'000ULL);
                    n = harvest(1, &ts);
                }
            }
            in_event_loop_ = false;
            return n;
        }

        std::size_t poll() override {
            std::size_t total = 0;
            in_event_loop_ = true;
            while (!stopped_.load(concurrency::memory_order_acquire)) {
                const std::size_t n = harvest(0, nullptr); // peek，非阻塞
                if (n == 0) {
                    break;
                }
                total += n;
            }
            in_event_loop_ = false;
            return total;
        }

        std::size_t poll_one() override {
            if (stopped_.load(concurrency::memory_order_acquire)) {
                return 0;
            }
            in_event_loop_ = true;
            std::size_t n = drain_ready_queue_one();
            if (n == 0) {
                n = harvest_one_cqe();
            }
            in_event_loop_ = false;
            return n;
        }

        void stop() noexcept override {
            if (!stopped_.exchange(true, concurrency::memory_order_acq_rel)) {
                wakeup();
            }
        }

        void restart() noexcept override {
            stopped_.store(false, concurrency::memory_order_release);
        }

        RAINY_NODISCARD bool stopped() const noexcept override {
            return stopped_.load(concurrency::memory_order_acquire);
        }

        void on_work_started() noexcept override {
            work_count_.fetch_add(1, concurrency::memory_order_relaxed);
        }

        void on_work_finished() noexcept override {
            if (work_count_.fetch_sub(1, concurrency::memory_order_acq_rel) == 1) {
                wakeup();
            }
        }

        void post_immediate_completion(completion_op *op, bool is_continuation) noexcept override {
            {
                concurrency::scoped_lock lock(ready_mutex_);
                ready_queue_.push(op);
            }
            if (!in_event_loop_) {
                wakeup();
            }
        }

        concurrency::thrd_result associate_handle(completion_op *op, const std::uintptr_t fd, void *extra) noexcept override {
            (void) fd;
            (void) extra;
            if (op) {
                op->io_handle = &ring_;
            }
            return concurrency::thrd_result::success;
        }

        RAINY_NODISCARD bool running_in_this_thread() const noexcept override {
            return in_event_loop_;
        }

        RAINY_NODISCARD int concurrency_hint() const noexcept override {
            return concurrency_hint_;
        }

        io_uring_sqe *get_sqe() noexcept {
            return ::io_uring_get_sqe(&ring_);
        }

        concurrency::thrd_result submit_sqe() noexcept {
            const int ret = ::io_uring_submit(&ring_);
            return (ret >= 0) ? concurrency::thrd_result::success : concurrency::thrd_result::error;
        }

    private:
        std::size_t harvest(unsigned wait_nr, ::__kernel_timespec *timeout) {
            std::size_t total = drain_ready_queue();
            io_uring_cqe *cqe = nullptr;
            int ret = 0;
            if (timeout) {
                ret = ::io_uring_wait_cqes(&ring_, &cqe, wait_nr, timeout, /*sigmask=*/nullptr);
            } else if (wait_nr > 0) {
                ret = ::io_uring_wait_cqe(&ring_, &cqe);
            } else {
                ret = ::io_uring_peek_cqe(&ring_, &cqe);
            }

            if (ret < 0 || cqe == nullptr) {
                return total;
            }
            unsigned head = 0;
            unsigned cqe_count = 0;
            io_uring_for_each_cqe(&ring_, head, cqe) {
                if (auto *op = static_cast<completion_op *>(::io_uring_cqe_get_data(cqe)); op && op != &non_op) {
                    op_result result{};
                    result.user_data = op;
                    result.bytes_transferred = (cqe->res >= 0) ? static_cast<std::size_t>(cqe->res) : 0;
                    result.error_code = (cqe->res < 0) ? -cqe->res : 0;
                    op->complete(result,
                                 /*cancelled=*/result.error_code == ECANCELED);
                    ++total;
                }
                ++cqe_count;
            }
            ::io_uring_cq_advance(&ring_, cqe_count);
            return total;
        }

        std::size_t harvest_one_cqe() noexcept {
            io_uring_cqe *cqe = nullptr;
            if (const int ret = ::io_uring_peek_cqe(&ring_, &cqe); ret < 0 || cqe == nullptr) {
                return 0;
            }
            auto *op = static_cast<completion_op *>(::io_uring_cqe_get_data(cqe));
            ::io_uring_cq_advance(&ring_, 1);
            if (!op || op == &non_op) {
                return 0;
            }
            op_result result{};
            result.user_data = op;
            result.bytes_transferred = (cqe->res >= 0) ? static_cast<std::size_t>(cqe->res) : 0;
            result.error_code = (cqe->res < 0) ? -cqe->res : 0;
            op->complete(result, result.error_code == ECANCELED);
            return 1;
        }

        std::size_t drain_ready_queue() {
            std::size_t total = 0;
            concurrency::unique_lock lock(ready_mutex_);
            while (!ready_queue_.empty()) {
                completion_op *op = ready_queue_.front();
                ready_queue_.pop();
                lock.unlock();
                op_result r{op, 0, 0}; // NOLINT
                op->complete(r, false);
                ++total;
                lock.lock();
            }
            return total;
        }

        std::size_t drain_ready_queue_one() noexcept {
            concurrency::unique_lock lock(ready_mutex_);
            if (ready_queue_.empty()) {
                return 0;
            }
            completion_op *op = ready_queue_.front();
            ready_queue_.pop();
            lock.unlock();
            op_result r{op, 0, 0}; // NOLINT
            op->complete(r, false);
            return 1;
        }

        void wakeup() noexcept {
            io_uring_sqe *sqe = ::io_uring_get_sqe(&ring_);
            if (!sqe) {
                return;
            }
            ::io_uring_prep_nop(sqe);
            ::io_uring_sqe_set_data(sqe, &non_op);
            ::io_uring_submit(&ring_);
        }

        concurrency::mutex ready_mutex_;
        std::queue<completion_op *> ready_queue_;
        io_uring ring_{};
        bool ring_initialized_{false};
        int concurrency_hint_{0};
        int event_fd_{-1};
        static thread_local bool in_event_loop_;
    };

    thread_local bool io_uring_impl::in_event_loop_ = false;
}

namespace rainy::foundation::io::net::implements {
    memory::nebula_ptr<io_context_impl_base> create_io_context_impl(int concurrency_hint) {
        auto impl = memory::make_nebula<io_uring_impl>(concurrency_hint);
        impl->init(concurrency_hint);
        return impl;
    }
}
