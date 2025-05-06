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
#include <atomic>
#include <rainy/foundation/diagnostics/contract.hpp>

namespace rainy::foundation::diagnostics {
    contract_handler_process::handler_type contract_handler_process::set_contract_handler(
        contract_handler_process::handler_type fptr) {
        rainy_ref this_ = instance();
        handler_type ret = this_.handler.reset(fptr);
        return ret;
    }

    void contract_handler_process::invoke_contract_handler(std::string_view message, unique_error_code error_code,
                                                           const utility::source_location &location) {
        rainy_ref this_ = instance();
        if (this_.handler.is_valid()) {
            this_.handler.invoke(message, error_code, location);
        }
    }

    bool contract_handler_process::handler_state() noexcept {
        rainy_ref this_ = instance();
        return this_.handler.is_valid();
    }
}

namespace rainy::foundation::diagnostics {
    static std::atomic_bool contract_raise_state = true;
    
    bool does_contract_will_raise() {
        return contract_raise_state.load();
    }
    
    void make_contract_be_raise() {
        contract_raise_state.store(true);
    }
    
    void make_contract_no_raise() {
        contract_raise_state.store(false);
    }
}

namespace rainy::foundation::diagnostics::implements {
    void call_contract(const bool cond, const std::string_view &type, const std::string_view &message, unique_error_code error_code,
                       int &options, const utility::source_location &location) {
        if (options == 0) {
#if RAINY_ENABLE_DEBUG
            // 在编译期使用debugbreak
            options = contract_option::use_debugbreak;
#else
            options = contract_option::use_exception | contract_option::use_custom_handler;
#endif
        }
        if (!cond) {
            if (!(options & contract_option::no_stderr_msg)) {
                foundation::io::stderr_println("contract : ", type, " triggered ", location.to_string(), " message: ", message);
            }
            if (!contract_raise_state.load(std::memory_order_acquire)) {
                return;
            }
            if (options & contract_option::use_custom_handler) {
                if (contract_handler_process::handler_state()) {
                    contract_handler_process::invoke_contract_handler(message, error_code ,location);
                    if (!(options & contract_option::custome_handler_need_continue)) {
                        return;
                    }
                }
            }
            if (options & contract_option::use_exception) {
                foundation::exceptions::runtime::throw_contract_hit(message, location);
                return;
            }
#if RAINY_ENABLE_DEBUG
            if (options & contract_option::use_debugbreak) {
                core::pal::debug_break();
            }
#else
            if (options & contract_option::use_terminate) {
                std::terminate();
            }
#endif
        }
    }
}
