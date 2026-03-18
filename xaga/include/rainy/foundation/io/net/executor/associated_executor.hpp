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
#ifndef RAINY_FOUNDATION_IO_NET_ASSOCIATED_EXECUTOR_HPP
#define RAINY_FOUNDATION_IO_NET_ASSOCIATED_EXECUTOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/executor/executor_trait.hpp>

namespace rainy::foundation::io::net {
    template <typename Ty, typename Executor = system_executor, typename = void>
    struct associated_executor {
        using type = Executor;

        static type get(const Ty &t, const Executor &executor = Executor()) noexcept {
            (void) t;
            return executor;
        }
    };

    template <typename Ty, typename Executor>
    struct associated_executor<Ty, Executor, type_traits::other_trans::void_t<typename Ty::executor_type>> {
        using type = typename Ty::executor_type; // NOLINT

        static type get(const Ty &t, const Executor &executor = Executor()) noexcept {
            (void) executor;
            return t.get_executor();
        }
    };

    template <typename Ty, typename Executor = system_executor>
    using assoicated_executor_t = typename associated_executor<Ty, Executor>::type; // NOLINT

    template <typename Ty>
    assoicated_executor_t<Ty> get_associated_executor(const Ty &t) noexcept {
        return associated_executor<Ty>::get(t);
    }

    template <typename Ty, typename Executor, type_traits::other_trans::enable_if_t<is_executor_v<Executor>, int> = 0>
    assoicated_executor_t<Ty, Executor> get_associated_executor(const Ty &t, const Executor &executor) noexcept {
        return associated_executor<Ty>::get(t, executor);
    }

    template <typename Ty, typename ExecutionContext,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>, int> = 0>
    assoicated_executor_t<Ty, typename ExecutionContext::executor_type> get_associated_executor(const Ty &t,
                                                                                                const ExecutionContext &ctx) noexcept {
        return associated_executor<Ty>::get(t, ctx.get_executor());
    }
}

#endif
