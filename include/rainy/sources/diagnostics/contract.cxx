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
#include <rainy/diagnostics/contract.hpp>

namespace rainy::utility {
    contract_handler_process::handler_type contract_handler_process::set_contract_handler(
        const foundation::functional::function_pointer<void(const utility::source_location & /* location */)> fptr) {
        rainy_ref this_ = instance();
        handler_type ret = this_.handler.reset(fptr.get());
        return ret;
    }

    void contract_handler_process::invoke_contract_handler(const utility::source_location &location) {
        rainy_ref this_ = instance();
        if (this_.handler) {
            this_.handler.invoke(location);
        }
    }
}

void rainy::utility::internals::call_contract(const bool cond, const std::string_view &type, const std::string_view &message,
                                              int &options, const utility::source_location &location) {
    if (options == 0) {
#if RAINY_ENABLE_DEBUG
        // 在编译期使用debugbreak
        options = contract_option::use_debugbreak;
#else
        options = contract_option::no_stderr_msg | contract_option::use_exception;
#endif
    }
    if (!cond) {
        if (!(options & contract_option::no_stderr_msg)) {
            io::print::stderr_println("contract : ", type, " triggered ", location.to_string(), " message: ", message);
        }
        if (options & contract_option::use_custom_handler) {
            contract_handler_process::invoke_contract_handler(location);
            if (!(options & contract_option::custome_handler_need_continue)) {
                return;
            }
        }
        if (options & contract_option::use_exception) {
            foundation::system::exceptions::runtime::throw_contract_hit(location);
            return;
        }
#if RAINY_ENABLE_DEBUG
        if (options & contract_option::use_debugbreak) {
            information::system_call::debug_break();
        }
#else
        if (options & contract_option::use_terminate) {
            std::terminate();
        }
#endif
    }
}
