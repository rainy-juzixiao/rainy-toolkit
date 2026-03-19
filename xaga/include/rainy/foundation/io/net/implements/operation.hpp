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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTYOR_OPERATION_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTYOR_OPERATION_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/memory/recycling_allocator.hpp>
#include <rainy/foundation/io/net/implements/handler_tracking.hpp>

namespace rainy::foundation::io::net::implements {
    template <typename Function, typename Context>
    RAINY_INLINE rain_fn handler_invoke_helper(Function &function, Context &context) -> void {
        Function tmp(function);
        tmp();
    }

    template <typename Function, typename Context>
    RAINY_INLINE rain_fn handler_invoke_helper(const Function &function, Context &context) -> void {
        Function tmp(function);
        tmp();
    }
}

namespace rainy::foundation::io::net::implements {
    class scheduler_operation {
    public:
        typedef scheduler_operation operation_type;

        void complete(void *owner, const std::error_code &ec, std::size_t bytes_transferred) {
            func_(owner, this, ec, bytes_transferred);
        }

        void destroy() {
            func_(0, this, std::error_code(), 0);
        }

    protected:
        using func_type = void (*)(void *, scheduler_operation *, const std::error_code &, std::size_t);

        scheduler_operation(func_type func) : next_(nullptr), func_(func), task_result_(0) {
        }

        ~scheduler_operation() {
        }

    private:
        friend class op_queue_access;
        scheduler_operation *next_;
        func_type func_;

    protected:
        friend class scheduler;
        unsigned int task_result_;
    };

    using operation = scheduler_operation;

    template <typename Handler, typename Alloc, typename Operation = scheduler_operation>
    class executor_op : public Operation {
    public:
        struct ptr {
            const Alloc *a;
            void *v;
            executor_op *p;
            ~ptr() {
                reset();
            }
            static executor_op *allocate(const Alloc &a) {
                using recycling_allocator_type = typename ::rainy::foundation::memory::get_recycling_allocator<
                    Alloc, ::rainy::foundation::concurrency::implements::thread_info_base::default_tag>::type;
                typename std::allocator_traits<recycling_allocator_type>::template rebind_alloc<executor_op> a1(
                    ::rainy::foundation::memory::get_recycling_allocator<
                        Alloc, ::rainy::foundation::concurrency::implements::thread_info_base::default_tag>::get(a));
                return a1.allocate(1);
            }
            void reset() {
                if (p) {
                    p->~executor_op();
                    p = 0;
                }
                if (v) {
                    typedef typename ::rainy::foundation::memory::get_recycling_allocator<
                        Alloc, ::rainy::foundation::concurrency::implements::thread_info_base::default_tag>::type
                        recycling_allocator_type;
                    typename std::allocator_traits<recycling_allocator_type>::template rebind_alloc<executor_op> a1(
                        ::rainy::foundation::memory::get_recycling_allocator<
                            Alloc, ::rainy::foundation::concurrency::implements::thread_info_base::default_tag>::get(*a));
                    a1.deallocate(static_cast<executor_op *>(v), 1);
                    v = 0;
                }
            }
        };

        template <typename H>
        executor_op(H &&h, const Alloc &allocator) :
            Operation(&executor_op::do_complete), handler_(utility::move(h)), allocator_(allocator) {
        }

        static void do_complete(void *owner, Operation *base, const std::error_code & /*ec*/, std::size_t /*bytes_transferred*/) {
            executor_op *o(static_cast<executor_op *>(base));
            Alloc allocator(o->allocator_);
            ptr p = {utility::addressof(allocator), o, o};
            NET_TS_HANDLER_COMPLETION((*o));
            Handler handler(utility::move(o->handler_));
            p.reset();
            if (owner) {
                concurrency::fenced_block b(concurrency::fenced_block::half);
                NET_TS_HANDLER_INVOCATION_BEGIN(());
                implements::handler_invoke_helper(handler, handler);
                NET_TS_HANDLER_INVOCATION_END;
            }
        }

    private:
        Handler handler_;
        Alloc allocator_;
    };

}

#endif
