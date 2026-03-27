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
#ifndef RAINY_FOUNDATION_IO_NET_IO_CONTEXT_HPP
#define RAINY_FOUNDATION_IO_NET_IO_CONTEXT_HPP
#include <chrono>
#include <rainy/foundation/io/net/executor.hpp>
#include <rainy/foundation/io/net/implements/io_context.hpp>

namespace rainy::foundation::io::net {
    class RAINY_TOOLKIT_API io_context : public execution_context {
    public:
        template <typename Protocol>
        friend class basic_socket;

        template <typename AcceptableProtocol>
        friend class basic_socket_acceptor;

        template <typename Protocol>
        friend class basic_stream_socket;

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
            void dispatch(Func &&f, const ProtoAllocator &) const {
                if (running_in_this_thread()) {
                    utility::forward<Func>(f)();
                    return;
                }
                auto *op = implements::make_immediate_op(utility::forward<Func>(f));
                ctx_->impl_->post_immediate_completion(op, false);
            }

            template <typename Func, typename ProtoAllocator>
            void post(Func &&f, const ProtoAllocator &) const {
                auto *op = implements::make_immediate_op(utility::forward<Func>(f));
                ctx_->impl_->post_immediate_completion(op, false);
            }

            template <typename Func, typename ProtoAllocator>
            void defer(Func &&f, const ProtoAllocator &) const {
                auto *op = implements::make_immediate_op(utility::forward<Func>(f));
                ctx_->impl_->post_immediate_completion(op, true);
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
