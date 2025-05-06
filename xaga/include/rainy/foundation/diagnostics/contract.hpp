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

namespace rainy::foundation::diagnostics {
    using unique_error_code = std::uint64_t;

    RAINY_TOOLKIT_API bool does_contract_will_raise();
    RAINY_TOOLKIT_API void make_contract_be_raise();
    RAINY_TOOLKIT_API void make_contract_no_raise();
}

namespace rainy::foundation::diagnostics::implements {
    RAINY_TOOLKIT_API void call_contract(const bool cond, const std::string_view &type, const std::string_view &message,
                                         unique_error_code error_code, int &options, const utility::source_location &location);

    class RAINY_TOOLKIT_API contract_handler_wrapper {
    public:
        using contract_handler_type = void(*)(std::string_view, unique_error_code, const utility::source_location &);

        contract_handler_wrapper() = default;

        contract_handler_wrapper(contract_handler_type handler) : handler_(handler) {
        }

        void invoke(std::string_view str, unique_error_code code, const utility::source_location &loc) {
            handler_(str, code, loc);
        }

        contract_handler_type reset(contract_handler_wrapper handler) {
            auto old_handler = handler_;
            handler_ = handler.handler_;
            return old_handler;
        }

        bool is_valid() const noexcept {
            return handler_;
        }

    private:
        contract_handler_type handler_; // 内部存储无法导出的类型
    };
}

namespace rainy::foundation::exceptions::runtime {
    class contract_hit : public exception {
    public:
        using base = exception;

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
    class RAINY_TOOLKIT_API contract_handler_process {
    public:
        using handler_type = implements::contract_handler_wrapper;

        /**
         * @brief 设置自定义的合约处理函数
         * @param fptr 自定义的合约处理函数指针
         *
         * @return 之前的合约处理函数指针
         */
        static handler_type set_contract_handler(handler_type fptr);

        /**
         * @brief 调用自定义的合约处理函数
         * @param message 契约消息
         * @param error_code 错误代码
         * @param location 合约触发位置
         */
        static void invoke_contract_handler(std::string_view message, unique_error_code error_code,
                                            const utility::source_location &location);

        static bool handler_state() noexcept;

    private:
        RAINY_DECLARE_SIGNLE_INSTANCE(contract_handler_process);
        handler_type handler;
    };

    struct contract_option {
        static constexpr int default_config =
            0; // 使用默认配置，根据编译输出模式决定。DEBUG使用use_debugbreak选项，Release使用use_exception选项
        static constexpr int no_stderr_msg = 1; // 不输出到stderr
        static constexpr int use_terminate = 2; // 使用std::terminate()
        static constexpr int use_exception = 8; // 使用异常
        static constexpr int use_debugbreak = 16; // 使用debugbreak
        static constexpr int use_compile_time = 32; // 使用编译期异常
        static constexpr int use_custom_handler = 64; // 使用自定义的合约处理函数
        static constexpr int custome_handler_need_continue = 128; // 在执行合约处理函数后继续执行
    };

    /**
     * @brief 契约的调用接口
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param type 契约的类型，可以是"Pre-condition"、"Post-condition"，只要能表明契约的性质即可
     * @param error_code 错误代码
     * @param options 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * @remarks 在运行期中，按照顺序并在触发的时候，依次读取以下选项并处理：
     * @remarks 1. 如果没有no_stderr_msg：在stderr中输出契约信息
     * @remarks 2. use_custom_handler：在自定义的合约处理函数中处理契约（如果定义，且带有use_custom_handler选项）
     * @remarks 3.
     * use_exception：抛出契约异常（如果带有use_exception选项，如果use_custom_handler被带有，此选项是无效的，除非带有custome_handler_need_continue选项）
     * @remarks 4. use_debugbreak：使用debugbreak断点（如果带有use_debugbreak选项，仅Debug模式上才会有用）
     * @remarks 5. use_terminate：使用std::terminate()终止程序（如果带有use_terminate选项）
     *
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention
     * 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void contract(const bool cond, const std::string_view message, const std::string_view type,
                                         unique_error_code error_code, int options = contract_option::default_config,
                                         const utility::source_location &location = utility::source_location::current()) {
        if (options & contract_option::use_compile_time) {
            if (!cond) {
                std::terminate(); // 在编译期上下文中并不是为了真正抛出异常，而是强制让编译期报错
            }
            return;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) { // NOLINT
            if (!cond) {
                std::terminate(); // 在编译期上下文中并不是为了真正抛出异常，而是强制让编译期报错
            }
            return;
        }
#endif
        implements::call_contract(cond, type, message, error_code, options, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     */
    RAINY_INLINE void expects(const bool cond,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", 0, contract_option::default_config, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     */
    RAINY_INLINE void expects(const bool cond, const std::string_view message,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Pre-condition", 0, contract_option::default_config, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     */
    RAINY_INLINE void ensures(const bool cond,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Post-condition", 0, contract_option::default_config, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     */
    RAINY_INLINE void ensures(const bool cond, const std::string_view message,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", 0, contract_option::default_config, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     *
     * @attention
     * 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void expects(const bool cond, const int option,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", 0, option, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     *
     * @attention
     * 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void expects(const bool cond, const std::string_view message, const int option,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Pre-condition", 0, option, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     *
     * @attention
     * 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void ensures(const bool cond, const int option,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Post-condition", 0, option, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     *
     * @attention
     * 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void ensures(const bool cond, const std::string_view message, const int option,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", 0, option, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param error_code 错误代码
     * @param options 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     */
    RAINY_INLINE constexpr void expects(const bool cond, unique_error_code error_code, int options = contract_option::default_config,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", error_code, options, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param error_code 错误代码
     * @param options 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     */
    RAINY_INLINE constexpr void expects(const bool cond, const std::string_view message, unique_error_code error_code,
                                        int options = contract_option::default_config,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Pre-condition", error_code, options, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param error_code 错误代码
     * @param options 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     */
    RAINY_INLINE constexpr void ensures(const bool cond, unique_error_code error_code, int options = contract_option::default_config,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Post-condition", error_code, options, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param error_code 错误代码
     * @param options 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     */
    RAINY_INLINE constexpr void ensures(const bool cond, const std::string_view message, unique_error_code error_code,
                                        int options = contract_option::default_config,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", error_code, options, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期，在编译期上下文中进行检查
     * @param cond 条件表达式，如果为false，则触发契约
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 默认被定义为编译期检查，如果在运行期触发，会导致程序终止
     */
    RAINY_INLINE constexpr void ct_expects(const bool cond,
                                           const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", 0, contract_option::use_compile_time, location);
    }

    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期，在编译期上下文中进行检查
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 默认被定义为编译期检查，如果在运行期触发，会导致程序终止
     */
    RAINY_INLINE constexpr void ct_expects(const bool cond, const std::string_view message,
                                           const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Pre-condition", 0, contract_option::use_compile_time, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证，在编译期上下文中进行检查
     * @param cond 条件表达式，如果为false，则触发契约
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 默认被定义为编译期检查，如果在运行期触发，会导致程序终止
     */
    RAINY_INLINE constexpr void ct_ensures(const bool cond,
                                           const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Post-condition", 0, contract_option::use_compile_time, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证，在编译期上下文中进行检查
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     *
     * @remarks 默认被定义为编译期检查，如果在运行期触发，会导致程序终止
     */
    RAINY_INLINE constexpr void ct_ensures(const bool cond, const std::string_view message,
                                           const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", 0, contract_option::use_compile_time, location);
    }
}

namespace rainy::foundation::diagnostics::implements {
    RAINY_INLINE void assert_impl(const bool cond, const std::string_view expr, const std::string_view message,
                                  const utility::source_location &location = utility::source_location::current()) noexcept {
        std::stringstream ss;
        ss << expr << " : " << (message.empty() ? "assertion failed" : message);
        contract(cond, ss.str(), "Assertion", 0, contract_option::default_config, location);
    }
}

// 此宏可以实现与assert类似的功能
#define rainy_assert(expr, msg)                                                                                                       \
    ::rainy::foundation::diagnostics::implements::assert_impl((expr), #expr, msg, ::rainy::utility::source_location::current())

namespace rainy::utility {
    using foundation::diagnostics::contract;
    using foundation::diagnostics::contract_handler_process;
    using foundation::diagnostics::contract_option;
    using foundation::diagnostics::ct_ensures;
    using foundation::diagnostics::ct_expects;
    using foundation::diagnostics::ensures;
    using foundation::diagnostics::expects;
}

#endif // RAINY_FOUNDATION_DIAGNOSTICS_CONTRACT_HPP
