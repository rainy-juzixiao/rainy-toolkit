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
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <rainy/core/diagnostics/exceptions.hpp>

namespace rainy::core::exceptions {
    static exception_handler_t global_exception_handler_impl = &std::terminate;
    thread_local exception_handler_t current_thread_exception_handler_impl = &std::terminate;

    exception_handler_t global_exception_handler(exception_handler_t new_handler) noexcept {
        exception_handler_t old = global_exception_handler_impl;
        if (new_handler) {
            global_exception_handler_impl = new_handler;
        }
        return old;
    }

    exception_handler_t current_thread_exception_handler(exception_handler_t new_handler) noexcept {
        exception_handler_t old = current_thread_exception_handler_impl;
        if (new_handler) {
            current_thread_exception_handler_impl = new_handler;
        }
        return old;
    }
}

namespace rainy::core::exceptions::implements {
    void invoke_exception_handler() noexcept {
        {
            const auto invoke_address = current_thread_exception_handler();
            if (invoke_address) {
                invoke_address();
            }
        }
        {
            const auto invoke_address = global_exception_handler();
            if (invoke_address) {
                invoke_address();
            }
        }
    }
}