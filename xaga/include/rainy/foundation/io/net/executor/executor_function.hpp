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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_FUNCTION_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_FUNCTION_HPP
#include <rainy/foundation/memory/recycling_allocator.hpp>
#include <rainy/foundation/io/net/fwd.hpp>

namespace rainy::foundation::io::net::implements {
    class executor_function_base {
    public:
        using func_type = void (*)(executor_function_base *, bool);

        void complete() {
            func_(this, true);
        }

        void destroy() {
            func_(this, false);
        }

    protected:
        explicit executor_function_base(const func_type func) : func_(func) {
        }

        ~executor_function_base() = default;

    private:
        func_type func_;
    };

    template <typename Function, typename Alloc>
    class executor_function : public executor_function_base {
    public:
        NET_TS_DEFINE_TAGGED_HANDLER_ALLOCATOR_PTR(concurrency::implements::thread_info_base::executor_function_tag,
                                                   executor_function);

        template <typename F>
        executor_function(F &&f, const Alloc &allocator) :
            executor_function_base(&executor_function::do_complete), function_(utility::move(f)), allocator_(allocator) {
        }

        static void do_complete(executor_function_base *base, const bool call) {
            executor_function *o(static_cast<executor_function *>(base));
            Alloc allocator(o->allocator_);
            ptr p = {utility::addressof(allocator), o, o};
            Function function(utility::move(o->function_));
            p.reset();
            if (call) {
                function();
            }
        }

    private:
        Function function_;
        Alloc allocator_;
    };
}

#endif
