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
#ifndef RAINY_CONTRACT_HPP
#define RAINY_CONTRACT_HPP
#include <rainy/core/core.hpp>
#include <rainy/io/stream_print.hpp>
#include <rainy/functional/function_pointer.hpp>

namespace rainy::foundation::system::exceptions::runtime {
    class contract_hit : public exception {
    public:
        using base = exception;

        explicit contract_hit(const source &location) : base("contract is hit!", location) {
        }
    };

    /**
     * @brief 触发契约异常
     * @param location 契约触发位置
     * @attetution 不建议直接调用，因为contract将自动调用此函数。且仅在use_exception选项开启时有效
     */
    RAINY_INLINE void throw_contract_hit(const diagnostics::source_location &location) {
        throw_exception(contract_hit{location});
    }
}

namespace rainy::utility {
#if RAINY_USING_COMMON_SOURCESFILE_BUILD
    class contract_handler_process {
    public:
        using handler_type = foundation::functional::function_pointer<void(const utility::source_location &)>;

        /**
         * @brief 设置自定义的合约处理函数
         * @param fptr 自定义的合约处理函数指针
         * 
         * @attention 如果看到此注释，表明你正在普通源文件构建模式中使用该模块提供的功能，这意味着你可能会遇到一些编译问题
         * @attention 例如，无法寻找符号又或者是链接错误等等
         * @attention 如果你出现了这些问题，推荐在CMake的配置清单中的COMMON_MOUDLE_USE_SOURCE_BUILD_MODE进行禁用再次尝试编译
         * 
         * @return 之前的合约处理函数指针
         */
        static handler_type set_contract_handler(
            const foundation::functional::function_pointer<void(const utility::source_location & /* location */)> fptr);

        /**
         * @brief 调用自定义的合约处理函数
         * @param location 合约触发位置
         * 
         * @attention 如果看到此注释，表明你正在普通源文件构建模式中使用该模块提供的功能，这意味着你可能会遇到一些编译问题
         * @attention 例如，无法寻找符号又或者是链接错误等等
         * @attention 如果你出现了这些问题，推荐在CMake的配置清单中的COMMON_MOUDLE_USE_SOURCE_BUILD_MODE进行禁用再次尝试编译
         */
        static void invoke_contract_handler(const utility::source_location &location);

    private:
        RAINY_DECLARE_SIGNLE_INSTANCE(contract_handler_process)
        handler_type handler;
    };
#else
    class contract_handler_process {
    public:
        using handler_type = foundation::functional::function_pointer<void(const utility::source_location &)>;

        /**
         * @brief 设置自定义的合约处理函数
         * @param fptr 自定义的合约处理函数指针
         * 
         * @attention 如果看到此注释，表明你禁用了普通源文件构建模式，且使用了该模块提供的功能。这意味着你可能不会遇到编译问题，因为源代码完全可用
         * @attention 不过，如果禁用它，一旦你涉及重新编译，这将导致不必要的编译时间，除非你确信你的项目规模很小
         * 
         * @return 之前的合约处理函数指针
         */
        static handler_type set_contract_handler(
            const foundation::functional::function_pointer<void(const utility::source_location & /* location */)> fptr) {
            rainy_ref this_ = instance();
            handler_type ret = this_.handler.reset(fptr.get());
            return ret;
        }

        /**
         * @brief 调用自定义的合约处理函数
         * @param location 合约触发位置
         * 
         * @attention 如果看到此注释，表明你禁用了普通源文件构建模式，且使用了该模块提供的功能。这意味着你可能不会遇到编译问题，因为源代码完全可用
         * @attention 不过，如果禁用它，一旦你涉及重新编译，这将导致不必要的编译时间，除非你确信你的项目规模很小
         */
        static void invoke_contract_handler(const utility::source_location &location) {
            rainy_ref this_ = instance();
            if (this_.handler) {
                this_.handler.invoke(location);
            }
        }

    private:
        RAINY_DECLARE_SIGNLE_INSTANCE(contract_handler_process)
        handler_type handler;
    };
#endif

    struct contract_option {
        static constexpr int default_config = 0; // 使用默认配置，根据编译输出模式决定。DEBUG使用use_debugbreak选项，Release使用no_stderr_msg以及use_exception选项
        static constexpr int no_stderr_msg = 1; // 不输出到stderr
        static constexpr int use_terminate = 2; // 使用std::terminate()
        static constexpr int use_exception = 8; // 使用异常
        static constexpr int use_debugbreak = 16; // 使用debugbreak
        static constexpr int use_compile_time = 32; // 使用编译期异常
        static constexpr int use_custom_handler = 64; // 使用自定义的合约处理函数
        static constexpr int custome_handler_need_continue = 128; // 在执行合约处理函数后继续执行
    };
}

namespace rainy::utility::internals {
#if RAINY_USING_COMMON_SOURCESFILE_BUILD
    // 用于运行期的合约处理函数
    void call_contract(const bool cond, const std::string_view &type, const std::string_view &message, int &options,
                       const utility::source_location &location);
#else
    RAINY_INLINE void call_contract(const bool cond, const std::string_view &type, const std::string_view &message, int &options,
                       const utility::source_location &location) {
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
#endif
}

namespace rainy::utility {
    /**
     * @brief 契约的调用接口
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param type 契约的类型，可以是"Pre-condition"、"Post-condition"，只要能表明契约的性质即可
     * @param options 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * @remarks 在运行期中，按照顺序并在触发的时候，依次读取以下选项并处理：
     * @remarks 1. 如果没有no_stderr_msg：在stderr中输出契约信息
     * @remarks 2. use_custom_handler：在自定义的合约处理函数中处理契约（如果定义，且带有use_custom_handler选项）
     * @remarks 3. use_exception：抛出契约异常（如果带有use_exception选项，如果use_custom_handler被带有，此选项是无效的，除非带有custome_handler_need_continue选项）
     * @remarks 4. use_debugbreak：使用debugbreak断点（如果带有use_debugbreak选项，仅Debug模式上才会有用）
     * @remarks 5. use_terminate：使用std::terminate()终止程序（如果带有use_terminate选项）
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void contract(const bool cond, const std::string_view message,const std::string_view type,
                                         int options = contract_option::default_config,
                                         const utility::source_location &location = utility::source_location::current()) {
        if (options & contract_option::use_compile_time) {
            if (!cond) {
                throw; // 在编译期上下文中并不是为了真正抛出异常，而是强制让编译期报错
            }
            return;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            if (!cond) {
                throw; // 在编译期上下文中并不是为了真正抛出异常，而是强制让编译期报错
            }
            return;
        }
#endif
        internals::call_contract(cond, type, message, options, location);
    }
}

namespace rainy::utility {
    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE void expects(const bool cond,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", contract_option::default_config, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE void expects(const bool cond, const std::string_view message,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Pre-condition", contract_option::default_config, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * 
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE void ensures(const bool cond,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Post-condition", contract_option::default_config, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * 
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE void ensures(const bool cond, const std::string_view message,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", contract_option::default_config, location);
    }
}

namespace rainy::utility {
    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void expects(const bool cond, const int option,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", option, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查  
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void expects(const bool cond, const std::string_view message, const int option,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message,"Pre-condition", option, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param option 契约的选项，可以是多个选项的组合，具体请参考contract_option
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 如果使用C++20标准编译器，在编译期上下文中，即使没有use_compile_time选项，也会进入其检查
     * 
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     * @attention 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     */
    RAINY_INLINE constexpr void ensures(const bool cond, const int option,
                              const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!","Post-condition", option, location);
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
     * @attentions 如果传入选项中，包含了use_complile_time，则在编译期上下文中，会强制让编译期报错。如果是运行期，这是不明确的行为，不推荐使用
     * @attention 如果启用了普通源文件构建模式，如果导致编译问题，请尝试禁用它
     */
    RAINY_INLINE constexpr void ensures(const bool cond, const std::string_view message, const int option,
                                        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", option, location);
    }
}

namespace rainy::utility {
    /**
     * @brief 预期的契约，用于检查函数的输入参数是否符合预期
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 默认被定义为编译期检查，如果在运行期触发，会导致程序终止
     */
    RAINY_INLINE constexpr void ct_expects(const bool cond,
                                           const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "contract is hit!", "Pre-condition", contract_option::use_compile_time, location);
    }

    /**
     * @brief 保证的契约，用于检查函数的输出结果是否符合保证
     * @param cond 条件表达式，如果为false，则触发契约
     * @param message 契约的消息
     * @param location 契约的源代码触发位置，如果使用自定义位置则传入，否则使用当前位置（不需要传入）
     * 
     * @remarks 默认被定义为编译期检查，如果在运行期触发，会导致程序终止
     */
    RAINY_INLINE constexpr void ct_ensures(const bool cond, const std::string_view message,
                                           const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, message, "Post-condition", contract_option::use_compile_time, location);
    }
}

namespace rainy::utility::internals {
    RAINY_INLINE void assert_impl(const bool cond, const std::string_view expr,const std::string_view message,
                                     const utility::source_location &location = utility::source_location::current()) noexcept {
        std::stringstream ss;
        ss << expr << " : " << message.empty() ? "assertion failed" : message;
        contract(cond, ss.view(), "Assertion", contract_option::default_config, location);
    }
}

// 此宏可以实现与assert类似的功能（）
#define rainy_assert(expr, msg) \
    ::rainy::utility::internals::assert_impl((expr), #expr, msg, ::rainy::utility::source_location::current())

#endif
