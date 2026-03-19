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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_HPP

// NOLINTBEGIN

#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/future.hpp>
#include <rainy/foundation/io/net/executor/associated_allocator.hpp>
#include <rainy/foundation/io/net/executor/associated_executor.hpp>
#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/io/net/executor/execution_context.hpp>
#include <rainy/foundation/io/net/executor/executor_trait.hpp>
#include <rainy/foundation/io/net/executor/bind_executor.hpp>
#include <rainy/foundation/io/net/executor/executor_work_guard.hpp>
#include <rainy/foundation/io/net/fwd.hpp>

// NOLINTEND

namespace rainy::foundation::io::net {
    class bad_executor;
    class executor;

    bool operator==(const executor &allocator, const executor &b) noexcept;
    bool operator==(const executor &e, nullptr_t) noexcept;
    bool operator==(nullptr_t, const executor &e) noexcept;
    bool operator!=(const executor &allocator, const executor &b) noexcept;
    bool operator!=(const executor &e, nullptr_t) noexcept;
    bool operator!=(nullptr_t, const executor &e) noexcept;

    // 13.22, dispatch:
    template <typename CompletionToken>
    DEDUCED dispatch(CompletionToken &&token);

    template <typename Executor, typename CompletionToken>
    DEDUCED dispatch(const Executor &ex, CompletionToken &&token);

    template <class ExecutionContext, typename CompletionToken>
    DEDUCED dispatch(ExecutionContext &ctx, CompletionToken &&token);
    // 13.23, post:
    template <typename CompletionToken>
    DEDUCED post(CompletionToken &&token);

    template <typename Executor, typename CompletionToken>
    DEDUCED post(const Executor &ex, CompletionToken &&token);

    template <class ExecutionContext, typename CompletionToken>
    DEDUCED post(ExecutionContext &ctx, CompletionToken &&token);
    // 13.24, defer:
    template <typename CompletionToken>
    DEDUCED defer(CompletionToken &&token);

    template <typename Executor, typename CompletionToken>
    DEDUCED defer(const Executor &ex, CompletionToken &&token);

    template <class ExecutionContext, typename CompletionToken>
    DEDUCED defer(ExecutionContext &ctx, CompletionToken &&token);

    template <typename Executor>
    class strand;

    template <typename Executor>
    bool operator==(const strand<Executor> &allocator, const strand<Executor> &b);

    template <typename Executor>
    bool operator!=(const strand<Executor> &allocator, const strand<Executor> &b);

    template <typename ProtoAllocator = std::allocator<void>>

    class use_future_t {};

    constexpr use_future_t<> use_future = use_future_t<>();

    template <typename ProtoAllocator, class Result, class... Args>
    class async_result<use_future_t<ProtoAllocator>, Result(Args...)>;

    template <class Result, class... Args, class Signature>
    class async_result<concurrency::packaged_task<Result(Args...)>, Signature>;
}

namespace std { // NOLINT
    template <class Allocator>
    struct uses_allocator<rainy::foundation::io::net::executor, Allocator> : std::true_type {};
}

#endif
