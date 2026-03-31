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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_SYSTEM_CONTEXT_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_SYSTEM_CONTEXT_HPP
#include <rainy/foundation/io/net/executor/execution_context.hpp>
#include <rainy/foundation/concurrency/executor.hpp>

namespace rainy::foundation::io::net {
    class system_executor;

    class system_context : public execution_context {
    public:
        using executor_type = system_executor;

        system_context();

        ~system_context(); // NOLINT

        executor_type get_executor() noexcept;

        void stop();

        bool stopped() const noexcept;

        void join();

    private:
        friend class system_executor;

        struct thread_function;

        concurrency::executor exec_;
    };
}

#endif
