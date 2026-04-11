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
#ifndef RAINY_FOUNDATION_IO_NET_TIMER_HPP
#define RAINY_FOUNDATION_IO_NET_TIMER_HPP
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/io_context.hpp>
#include <system_error>

namespace rainy::foundation::io::net {
    template <typename Clock>
    struct wait_traits {
        static typename Clock::duration to_wait_duration(const typename Clock::duration &duration) {
            return duration;
        }

        static typename Clock::duration to_wait_duration(const typename Clock::time_point &time_point) {
            auto now = Clock::now();
            auto diff = time_point - now;
            if (diff <= Clock::duration::zero()) {
                return Clock::duration::zero();
            }
            return diff;
        }
    };

    template <typename Clock, typename WaitTraits>
    class basic_waitable_timer {
    public:
        using executor_type = io_context::executor_type;
        using clock_type = Clock;
        using duration = typename clock_type::duration;
        using time_point = typename clock_type::time_point;
        using traits_type = WaitTraits;

        explicit basic_waitable_timer(io_context &ctx) : executor_(ctx.get_executor()), expiry_(time_point{}) {
        }

        basic_waitable_timer(io_context &ctx, const time_point &time_point) : executor_(ctx.get_executor()), expiry_(time_point) {
        }

        basic_waitable_timer(io_context &ctx, const duration &duration) :
            executor_(ctx.get_executor()), expiry_(clock_type::now() + duration) {
        }

        basic_waitable_timer(const basic_waitable_timer &) = delete;

        basic_waitable_timer(basic_waitable_timer &&right) noexcept :
            executor_(utility::move(right.executor_)), expiry_(right.expiry_), impl_(utility::move(right.impl_)) {
            right.expiry_ = time_point{};
        }

        ~basic_waitable_timer() {
            cancel();
        }

        basic_waitable_timer &operator=(const basic_waitable_timer &) = delete;

        basic_waitable_timer &operator=(basic_waitable_timer &&right) noexcept {
            if (this != &right) {
                cancel(); // 取消自身所有挂起等待
                executor_ = utility::move(right.executor_);
                expiry_ = right.expiry_;
                impl_ = utility::move(right.impl_);
                right.expiry_ = time_point{};
            }
            return *this;
        }

        executor_type get_executor() noexcept {
            return executor_;
        }

        size_t cancel() {
            if (!impl_) {
                return 0;
            }
            return impl_->cancel_all();
        }

        size_t cancel_one() {
            if (!impl_) {
                return 0;
            }
            return impl_->cancel_one();
        }

        time_point expiry() const {
            return expiry_;
        }

        size_t expires_at(const time_point &time_point) {
            std::size_t n = cancel();
            expiry_ = time_point;
            return n;
        }

        size_t expires_after(const duration &duration) {
            return expires_at(clock_type::now() + duration);
        }

        void wait() {
            std::error_code ec;
            wait(ec);
            if (ec) {
                throw std::system_error(ec, "basic_waitable_timer::wait");
            }
        }

        void wait(std::error_code &ec) {
            ec.clear();
            auto remaining = traits_type::to_wait_duration(expiry_);
            if (remaining <= duration::zero()) {
                return; // 已过期
            }
            std::this_thread::sleep_for(remaining);
        }

        template <typename CompletionToken>
        rain_fn async_wait(CompletionToken &&token) -> typename async_result<CompletionToken, void(std::error_code)>::return_type {
            using decayed_token = std::decay_t<CompletionToken>;
            using result_type = async_result<decayed_token, void(std::error_code)>;
            using handler_type = typename result_type::completion_handler_type;
            async_completion<decayed_token, void(std::error_code)> init(token);
            ensure_impl();
            impl_->async_wait(expiry_, executor_, utility::move(init.completion_handler));
            return init.result.get();
        }

    private:
        class timer_impl {
        public:
            struct wait_entry {
                time_point expiry;
                executor_type executor;
                std::function<void(std::error_code)> handler;
                bool cancelled{false};
            };

            timer_impl() {
                worker_ = std::thread([this] { run(); });
            }

            ~timer_impl() {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    running_ = false;
                }
                cv_.notify_all();
                if (worker_.joinable()) {
                    worker_.join();
                }
                for (auto& entry : queue_) {
                    if (!entry.cancelled) {
                        auto ec = std::make_error_code(std::errc::operation_canceled);
                        entry.handler(ec);
                        entry.executor.on_work_finished();
                    }
                }
                queue_.clear();
            }

            template <typename Handler>
            void async_wait(const time_point &expiry, const executor_type &ex, Handler &&handler) {
                ex.on_work_started();
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    queue_.push_back(wait_entry{expiry, ex, std::forward<Handler>(handler), false});
                    std::sort(queue_.begin(), queue_.end(),
                              [](const wait_entry &a, const wait_entry &b) { return a.expiry < b.expiry; });
                }
                cv_.notify_one();
            }

            std::size_t cancel_all() {
                std::vector<wait_entry> to_cancel;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (auto &entry: queue_) {
                        if (!entry.cancelled) {
                            entry.cancelled = true;
                            to_cancel.push_back(entry);
                        }
                    }
                    queue_.clear();
                    cv_.notify_all();
                }
                for (auto &entry: to_cancel) {
                    fire(entry, std::make_error_code(std::errc::operation_canceled));
                }
                return to_cancel.size();
            }

            std::size_t cancel_one() {
                std::optional<wait_entry> to_cancel;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (auto it = queue_.begin(); it != queue_.end(); ++it) {
                        if (!it->cancelled) {
                            to_cancel = *it;
                            queue_.erase(it);
                            break;
                        }
                    }
                    if (to_cancel.has_value()) {
                        cv_.notify_all();
                    }
                }
                if (to_cancel.has_value()) {
                    fire(*to_cancel, std::make_error_code(std::errc::operation_canceled));
                    return 1;
                }
                return 0;
            }

        private:
            void fire(const wait_entry &entry, std::error_code ec, bool from_cancel = false) {
                auto handler = entry.handler;
                auto executor = entry.executor;
                if (executor.context().stopped()) {
                    handler(ec);
                    executor.on_work_finished();
                    return;
                }
                executor.post(
                    [handler, executor, ec]() mutable {
                        handler(ec);
                        executor.on_work_finished();
                    },
                    std::allocator<void>{});
            }

            void run() {
                std::unique_lock<std::mutex> lock(mutex_);
                while (running_) {
                    if (queue_.empty()) {
                        cv_.wait(lock, [this] { return !running_ || !queue_.empty(); });
                        continue;
                    }
                    auto wake_at = queue_.front().expiry;
                    auto now = clock_type::now();
                    if (wake_at <= now) {
                        process_expired(lock);
                        continue;
                    }
                    cv_.wait_until(lock, wake_at,
                        [this, &wake_at] {
                            return !running_ || queue_.empty() || queue_.front().expiry != wake_at;
                        });
                    if (!running_) {
                        break;
                    }
                    process_expired(lock);
                }
            }

            void process_expired(std::unique_lock<std::mutex>& lock) {
                auto now = clock_type::now();
                std::vector<wait_entry> to_fire;
                auto it = queue_.begin();
                while (it != queue_.end() && it->expiry <= now) {
                    if (!it->cancelled) {
                        to_fire.push_back(*it);
                    }
                    it = queue_.erase(it);
                }
                lock.unlock();
                for (auto& entry : to_fire) {
                    fire(entry, std::error_code{});
                }
                lock.lock();
            }

            std::mutex mutex_;
            std::condition_variable cv_;
            std::vector<wait_entry> queue_;
            std::thread worker_;
            bool running_{true};
        };

        void ensure_impl() {
            if (!impl_) {
                impl_ = std::make_shared<timer_impl>();
            }
        }

        executor_type executor_;
        time_point expiry_;
        std::shared_ptr<timer_impl> impl_;
    };
}

#endif
