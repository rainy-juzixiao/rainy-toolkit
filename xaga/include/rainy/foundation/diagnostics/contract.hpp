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
#ifndef RAINY_FOUNDATION_DIAGNOSTICS_CONTRACT_HPP
#define RAINY_FOUNDATION_DIAGNOSTICS_CONTRACT_HPP

#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/io/stream_print.hpp>

namespace rainy::foundation::exceptions::runtime {
    class contract_hit : public runtime_error {
    public:
        using base = runtime_error;

        explicit contract_hit(std::string_view message, const source &location) : base(message.data(), location) {
        }
    };

    /**
     * @brief 触发契约异常
     * @param location 契约触发位置
     * @attetution 不建议直接调用，因为contract将自动调用此函数。且仅在use_exception选项开启时有效
     */
    RAINY_INLINE void throw_contract_hit(std::string_view message, const diagnostics::source_location &location) {
        throw_exception(contract_hit{message, location});
    }
}

namespace rainy::foundation::diagnostics {
    enum class contract_option {
        no_stderr_msg = 1, // 不输出到stderr
        use_terminate = 2, // 使用std::terminate()
        use_exception = 8, // 使用异常
        use_debugbreak = 16, // 使用debugbreak
#if RAINY_ENABLE_DEBUG
        default_config = use_debugbreak
#else
        default_config = use_exception
#endif
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(contract_option);
}

namespace rainy::foundation::diagnostics::implements {
    RAINY_INLINE void call_contract(const std::string_view type, const std::string_view message, contract_option options,
                                    const utility::source_location &location) {
        if (static_cast<bool>(options & contract_option::use_exception)) {
            if (!static_cast<bool>(options & contract_option::no_stderr_msg)) {
                foundation::io::stderr_println("contract : ", type, " triggered ", location.to_string(), " message: ", message);
            }
            foundation::exceptions::runtime::throw_contract_hit(message, location);
            return;
        }
#if RAINY_ENABLE_DEBUG
        if (static_cast<bool>(options & contract_option::use_debugbreak)) {
            if (!static_cast<bool>(options & contract_option::no_stderr_msg)) {
                foundation::io::stderr_println("contract : ", type, " triggered ", location.to_string(), " message: ", message);
            }
            core::pal::debug_break();
        }
#else
        if (static_cast<bool>(options & contract_option::use_terminate)) {
            if (!static_cast<bool>(options & contract_option::no_stderr_msg)) {
                foundation::io::stderr_println("contract : ", type, " triggered ", location.to_string(), " message: ", message);
            }
            std::terminate();
        }
#endif
    }
}

namespace rainy::foundation::diagnostics {
    RAINY_INLINE void contract(const bool cond, const std::string_view message, const std::string_view type,
                               contract_option options = contract_option::default_config,
                               const utility::source_location &location = utility::source_location::current()) {
        if (cond) {
            return;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) { // NOLINT
            std::terminate(); // 在编译期上下文中并不是为了真正抛出异常，而是强制让编译期报错
            return;
        }
#endif
        implements::call_contract(type, message, options, location);
    }

    RAINY_INLINE void expects(const bool cond, const contract_option option = contract_option::default_config,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", option, location);
    }

    RAINY_INLINE void expects(const bool cond, const std::string_view message, const contract_option option = contract_option::default_config,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Pre-condition", option, location);
    }

    RAINY_INLINE void ensures(const bool cond, contract_option options = contract_option::default_config,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Post-condition",options, location);
    }

    RAINY_INLINE void ensures(const bool cond, const std::string_view message,
                              contract_option options = contract_option::default_config,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", options, location);
    }
}

namespace rainy::foundation::diagnostics::implements {
    RAINY_INLINE void assert_impl(const bool cond, const std::string_view expr, const std::string_view message,
                                  const utility::source_location &location = utility::source_location::current()) noexcept {
        std::stringstream ss;
        ss << expr << " : " << (message.empty() ? "assertion failed" : message);
        contract(cond, ss.str(), "Assertion", contract_option::default_config, location);
    }
}

// 此宏可以实现与assert类似的功能
#define rainy_assert(expr, msg)                                                                                                       \
    ::rainy::foundation::diagnostics::implements::assert_impl((expr), #expr, msg, ::rainy::utility::source_location::current())

namespace rainy::utility {
    using foundation::diagnostics::contract;
    using foundation::diagnostics::contract_option;
    using foundation::diagnostics::ensures;
    using foundation::diagnostics::expects;
}

#endif // RAINY_FOUNDATION_DIAGNOSTICS_CONTRACT_HPP
