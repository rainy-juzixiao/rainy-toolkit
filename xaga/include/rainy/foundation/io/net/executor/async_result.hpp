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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_ASYNC_RESULT_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_ASYNC_RESULT_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::net {
    template <typename CompletionToken, typename Signature>
    class async_result {
    public:
        using completion_handler_type = CompletionToken;
        using return_type = void;

        explicit async_result(completion_handler_type &) {
        }

        async_result(const async_result &) = delete;
        async_result &operator=(const async_result &) = delete;

        return_type get() { // NOLINT
        }
    };

    template <typename CompletionToken, typename Signature>
    struct async_completion {
        using completion_handler_type =
            async_result<type_traits::other_trans::decay_t<CompletionToken>, Signature>::completion_handler_type;
        using handler =
            type_traits::other_trans::conditional_t<type_traits::type_relations::is_same_v<CompletionToken, completion_handler_type>,
                                                    completion_handler_type &, completion_handler_type>;

        explicit async_completion(CompletionToken &t) : completion_handler(t) {
        }

        async_completion(const async_completion &) = delete;
        async_completion &operator=(const async_completion &) = delete;

        handler completion_handler;
        async_result<type_traits::other_trans::decay_t<CompletionToken>, Signature> result;
    };
}

#endif
