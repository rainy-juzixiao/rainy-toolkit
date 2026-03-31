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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_SYSTEM_EXECUTOR_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_SYSTEM_EXECUTOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/concurrency.hpp>

namespace rainy::foundation::io::net {
    class system_context;

    class system_executor {
    public:
        using context_type = system_context;

        system_executor() noexcept : ctx_(nullptr) {
        }

        explicit system_executor(system_context &ctx) noexcept : ctx_(std::addressof(ctx)) {
        }

        context_type &context() const noexcept {
            return *ctx_;
        }

        void on_work_started() const noexcept { // NOLINT
        }

        void on_work_finished() const noexcept { // NOLINT
        }

        template <typename Function, typename Allocator>
        void dispatch(Function &&f, const Allocator &) const {
            utility::forward<Function>(f)();
        }

        template <typename Function, typename Allocator>
        void post(Function &&f, const Allocator &) const {
            get_executor().submit(utility::forward<Function>(f));
        }

        template <typename Function, typename Allocator>
        void defer(Function &&f, const Allocator &) const {
            get_executor().submit(utility::forward<Function>(f));
        }

        friend bool operator==(const system_executor &, const system_executor &) noexcept {
            return true;
        }

        friend bool operator!=(const system_executor &, const system_executor &) noexcept {
            return false;
        }

    private:
        system_context *ctx_;

        concurrency::executor &get_executor() const noexcept;
    };
}

#endif
