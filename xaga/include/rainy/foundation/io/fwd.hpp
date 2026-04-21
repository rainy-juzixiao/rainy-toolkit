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
#ifndef RAINY_FOUNDATION_IO_FWD_HPP
#define RAINY_FOUNDATION_IO_FWD_HPP
#include <chrono>

namespace rainy::foundation::io::net {
    class execution_context;

    template <class T, class Executor>
    class executor_binder;

    template <class Executor>
    class executor_work_guard;

    class system_executor;
    class executor;

    template <class Executor>
    class strand;

    class io_context;

    template <typename Clock>
    struct wait_traits;

    template <typename Clock, typename WaitTraits = wait_traits<Clock>>
    class basic_waitable_timer;

    using system_timer = basic_waitable_timer<std::chrono::system_clock>;
    using steady_timer = basic_waitable_timer<std::chrono::steady_clock>;
    using high_resolution_timer = basic_waitable_timer<std::chrono::high_resolution_clock>;
}

#endif
