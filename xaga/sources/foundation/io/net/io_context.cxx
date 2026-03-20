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

namespace rainy::foundation::io::net {
    io_context::executor_type::executor_type(const executor_type &other) noexcept = default;
    io_context::executor_type::executor_type(executor_type &&other) noexcept = default;
    io_context::executor_type &io_context::executor_type::operator=(const executor_type &) noexcept = default;
    io_context::executor_type &io_context::executor_type::operator=(executor_type &&) noexcept = default;

    bool io_context::executor_type::running_in_this_thread() const noexcept {
        return ctx_->impl_->running_in_this_thread();
    }

    io_context &io_context::executor_type::context() const noexcept {
        return *ctx_;
    }

    void io_context::executor_type::on_work_started() const noexcept {
        ctx_->impl_->on_work_started();
    }

    void io_context::executor_type::on_work_finished() const noexcept {
        ctx_->impl_->on_work_finished();
    }

    io_context::io_context() : io_context(0) {
    }

    io_context::io_context(const int concurrency_hint) : impl_(implements::create_io_context_impl(concurrency_hint)) {
    }

    io_context::~io_context() {
        shutdown();
        destroy();
    }

    io_context::executor_type io_context::get_executor() noexcept {
        return executor_type{this};
    }

    io_context::count_type io_context::run() { // NOLINT
        return impl_->run();
    }

    io_context::count_type io_context::run_one() { // NOLINT
        return impl_->run_one();
    }

    io_context::count_type io_context::poll() { // NOLINT
        return impl_->poll();
    }

    io_context::count_type io_context::poll_one() { // NOLINT
        return impl_->poll_one();
    }

    void io_context::stop() { // NOLINT
        impl_->stop();
    }

    bool io_context::stopped() const noexcept {
        return impl_->stopped();
    }

    void io_context::restart() { // NOLINT
        impl_->restart();
    }
}
