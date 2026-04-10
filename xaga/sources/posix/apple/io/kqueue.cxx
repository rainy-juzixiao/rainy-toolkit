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
#include <rainy/foundation/io/net/io_context.hpp>

#include <array>
#include <queue>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

namespace rainy::foundation::io::net::implements {
    static void nop_fn(completion_op *, const op_result &, bool) noexcept {
    }
    static completion_op non_op{&nop_fn};

    static constexpr uintptr_t WAKEUP_IDENT = static_cast<uintptr_t>(-1);
    static constexpr int HARVEST_BATCH = 64;

    class kqueue_impl final : public io_context_impl_base {
    public:
        explicit kqueue_impl(const int concurrency_hint) noexcept : concurrency_hint_(concurrency_hint) {
        }

        ~kqueue_impl() override {
            destroy();
        }

        concurrency::thrd_result init(const int concurrency_hint) noexcept override {
            concurrency_hint_ = concurrency_hint;
            kq_ = ::kqueue();
            if (kq_ < 0) {
                return concurrency::thrd_result::error;
            }
            struct kevent ev{};
            EV_SET(&ev, WAKEUP_IDENT, EVFILT_USER, EV_ADD | EV_CLEAR, NOTE_FFNOP, 0, &non_op);
            if (::kevent(kq_, &ev, 1, nullptr, 0, nullptr) < 0) {
                ::close(kq_);
                kq_ = -1;
                return concurrency::thrd_result::error;
            }

            kq_initialized_ = true;
            return concurrency::thrd_result::success;
        }

        void destroy() noexcept override {
            if (kq_initialized_) {
                struct kevent ev{};
                EV_SET(&ev, WAKEUP_IDENT, EVFILT_USER, EV_DELETE, 0, 0, nullptr);
                ::kevent(kq_, &ev, 1, nullptr, 0, nullptr);
                ::close(kq_);
                kq_ = -1;
                kq_initialized_ = false;
            }
        }

        std::size_t run() override {
            std::size_t total = 0;
            in_event_loop_ = true;
            while (!stopped_.load(concurrency::memory_order_acquire)) {
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
                    op_result r{op, 0, 0};
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
                    n = harvest(0, nullptr);
                } else {
                    struct timespec ts{};
                    ts.tv_sec = static_cast<time_t>(timeout_ns / 1'000'000'000ULL);
                    ts.tv_nsec = static_cast<long>(timeout_ns % 1'000'000'000ULL);
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
                const std::size_t n = harvest(0, nullptr);
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
                n = harvest_one_event();
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

        void post_immediate_completion(completion_op *op, bool /*is_continuation*/) noexcept override {
            if (destroying_.load(concurrency::memory_order_acquire)) {
                return;
            }
            {
                concurrency::scoped_lock lock(ready_mutex_);
                ready_queue_.push(op);
            }
            if (!in_event_loop_) {
                wakeup();
            }
        }

        concurrency::thrd_result associate_handle(completion_op *op, const std::uintptr_t /*fd*/, void * /*extra*/) noexcept override {
            if (op) {
                op->io_handle = reinterpret_cast<void *>(static_cast<std::uintptr_t>(kq_));
            }
            return concurrency::thrd_result::success;
        }

        RAINY_NODISCARD bool running_in_this_thread() const noexcept override {
            return in_event_loop_;
        }

        RAINY_NODISCARD int concurrency_hint() const noexcept override {
            return concurrency_hint_;
        }

    private:
        // min_events == 0 → 非阻塞（timeout 强制为零值）
        // min_events  > 0, timeout == nullptr → 无限阻塞
        // min_events  > 0, timeout != nullptr → 有限等待
        std::size_t harvest(unsigned min_events, const struct timespec *timeout) {
            std::size_t total = drain_ready_queue();

            std::array<struct kevent, HARVEST_BATCH> events{};
            struct timespec zero_ts{0, 0};
            const struct timespec *ts = (min_events == 0) ? &zero_ts : timeout;

            const int nev = ::kevent(kq_, nullptr, 0, events.data(), HARVEST_BATCH, ts);
            if (nev <= 0) {
                return total;
            }

            for (int i = 0; i < nev; ++i) {
                const struct kevent &ev = events[static_cast<std::size_t>(i)];
                // kevent 在 changelist 注册失败时以 EV_ERROR 返回错误，data 为 errno
                if (ev.flags & EV_ERROR) {
                    auto *op = static_cast<completion_op *>(ev.udata); // NOLINT
                    if (op && op != &non_op) {
                        op_result result{};
                        result.user_data = op;
                        result.bytes_transferred = 0;
                        result.error_code = static_cast<int>(ev.data);
                        op->complete(result, result.error_code == ECANCELED);
                        ++total;
                    }
                    continue;
                }
                auto *op = static_cast<completion_op *>(ev.udata);
                if (!op || op == &non_op) {
                    // wakeup 哨兵，不计入已处理数
                    continue;
                }
                op_result result{};
                result.user_data = op;
                result.bytes_transferred = (ev.data > 0) ? static_cast<std::size_t>(ev.data) : 0;
                result.error_code = 0;
                op->complete(result, false);
                ++total;
            }

            return total;
        }

        std::size_t harvest_one_event() noexcept {
            struct kevent ev{};
            struct timespec zero{0, 0};
            if (::kevent(kq_, nullptr, 0, &ev, 1, &zero) <= 0) {
                return 0;
            }
            if (ev.flags & EV_ERROR) {
                auto *op = static_cast<completion_op *>(ev.udata);
                if (!op || op == &non_op) {
                    return 0;
                }
                op_result result{};
                result.user_data = op;
                result.bytes_transferred = 0;
                result.error_code = static_cast<int>(ev.data);
                op->complete(result, result.error_code == ECANCELED);
                return 1;
            }
            auto *op = static_cast<completion_op *>(ev.udata);
            if (!op || op == &non_op) {
                return 0;
            }
            op_result result{};
            result.user_data = op;
            result.bytes_transferred = (ev.data > 0) ? static_cast<std::size_t>(ev.data) : 0;
            result.error_code = 0;
            op->complete(result, false);
            return 1;
        }

        std::size_t drain_ready_queue() {
            std::size_t total = 0;
            concurrency::unique_lock lock(ready_mutex_);
            while (!ready_queue_.empty()) {
                completion_op *op = ready_queue_.front();
                ready_queue_.pop();
                lock.unlock();
                op_result r{op, 0, 0};
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
            op_result r{op, 0, 0};
            op->complete(r, false);
            return 1;
        }

        void destroy() noexcept override {
            destroying_.store(true, concurrency::memory_order_release);
            if (kq_initialized_) {
                struct kevent ev{};
                EV_SET(&ev, WAKEUP_IDENT, EVFILT_USER, EV_DELETE, 0, 0, nullptr);
                ::kevent(kq_, &ev, 1, nullptr, 0, nullptr);
                ::close(kq_);
                kq_ = -1;
                kq_initialized_ = false;
            }
        }

        void wakeup() noexcept {
            if (destroying_.load(concurrency::memory_order_acquire)) {
                return;
            }
            if (!kq_initialized_) {
                return;
            }
            struct kevent ev{};
            EV_SET(&ev, WAKEUP_IDENT, EVFILT_USER, 0, NOTE_TRIGGER, 0, &non_op);
            // 再次检查，因为在获取锁或准备参数时可能已经开始析构
            if (!destroying_.load(concurrency::memory_order_acquire) && kq_initialized_) {
                ::kevent(kq_, &ev, 1, nullptr, 0, nullptr);
            }
        }

        int kq_{-1};
        bool kq_initialized_{false};
        int concurrency_hint_{0};

        concurrency::mutex ready_mutex_;
        std::queue<completion_op *> ready_queue_;

        static thread_local bool in_event_loop_;
        concurrency::atomic<bool> destroying_{false};
    };

    thread_local bool kqueue_impl::in_event_loop_ = false;
}

namespace rainy::foundation::io::net::implements {
    memory::nebula_ptr<io_context_impl_base> create_io_context_impl(int concurrency_hint) {
        auto impl = memory::make_nebula<kqueue_impl>(concurrency_hint);
        impl->init(concurrency_hint);
        return impl;
    }
}
