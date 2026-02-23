/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/foundation/concurrency/basic/context.hpp>

namespace rainy::foundation::concurrency::implements {
    thread_info_base::thread_info_base() : has_pending_exception_{0} {
        for (int i = 0; i < max_mem_index; ++i) {
            reusable_memory_[i] = nullptr;
        }
    }

    thread_info_base::~thread_info_base() {
        for (int i = 0; i < max_mem_index; ++i) {
            if (reusable_memory_[i]) {
                core::pal::deallocate(reusable_memory_[i]);
            }
        }
    }

    void thread_info_base::capture_current_exception() {
        switch (has_pending_exception_) {
            case 0:
                has_pending_exception_ = 1;
                pending_exception_ = std::current_exception();
                break;
            case 1:
                has_pending_exception_ = 2;
                pending_exception_ =
                    std::make_exception_ptr<exceptions::multiple_exceptions>(exceptions::multiple_exceptions(pending_exception_));
                break;
            default:
                break;
        }
    }

    void thread_info_base::rethrow_pending_exception() {
        if (has_pending_exception_ > 0) {
            has_pending_exception_ = 0;
            const std::exception_ptr ex(utility::move(pending_exception_));
            std::rethrow_exception(ex);
        }
    }
}

namespace rainy::foundation::concurrency {
    implements::thread_info_base *thread_context::top_of_thread_call_stack() {
        return thread_call_stack::top();
    }
}
