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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_DISPATCHER_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_DISPATCHER_HPP
#include <rainy/foundation/io/net/executor/associated_allocator.hpp>
#include <rainy/foundation/io/net/executor/associated_executor.hpp>
#include <rainy/foundation/io/net/executor/executor_work_guard.hpp>

namespace rainy::foundation::io::net::implements {
    template <typename Handler>
    class work_dispatcher {
    public:
        work_dispatcher(Handler &handler) : work_((get_associated_executor) (handler)), handler_(utility::move(handler)) { // NOLINT
        }

        work_dispatcher(const work_dispatcher &other) : work_(other.work_), handler_(other.handler_) {
        }

        work_dispatcher(work_dispatcher &&other) noexcept :
            work_(utility::move(other.work_)), handler_(utility::move(other.handler_)) {
        }

        void operator()() {
            typename associated_allocator<Handler>::type alloc((get_associated_allocator) (handler_));
            work_.get_executor().dispatch(utility::move(handler_), alloc);
            work_.reset();
        }

    private:
        executor_work_guard<typename associated_executor<Handler>::type> work_;
        Handler handler_;
    };
}

#endif
