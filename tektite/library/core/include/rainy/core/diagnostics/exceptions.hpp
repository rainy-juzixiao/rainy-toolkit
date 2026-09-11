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
#ifndef RAINY_BASIC_EXCEPTIONS_HPP
#define RAINY_BASIC_EXCEPTIONS_HPP
// NOLINTBEGIN

#include <exception>
#include <rainy/core/diagnostics/source_location.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/type_traits.hpp>

// NOLINTEND

namespace rainy::core::exceptions {
    /**
     * \lang english
     * @brief Type definition for exception handler function pointers.
     *
     *  An exception handler is a function that takes no arguments and returns void,
     *  typically called when an unhandled exception occurs.
     *
     * \lang simp-chinese
     * @brief 异常处理函数指针的类型定义。
     *
     *  异常处理程序是一个无参数、无返回值的函数，
     *  通常在未处理的异常发生时被调用。
     */
    using exception_handler_t = void (*)();

    /**
     * \lang english
     * @brief Gets or sets the global exception handler.
     *
     *  This function manages the handler that will be called for unhandled exceptions
     *  at the global level across all threads, unless overridden by a thread-specific handler.
     *
     * @param new_handler Pointer to the new exception handler function.
     * @param new_handler If nullptr, the current handler is returned without changing it.
     * @return The previous global exception handler before this call,
     *          or the current handler if new_handler is nullptr.
     *
     * \lang simp-chinese
     * @brief 获取或设置全局异常处理程序。
     *
     *  此函数管理在全局级别（跨所有线程）处理未处理异常时调用的处理程序，
     *  除非被特定于线程的处理程序覆盖。
     *
     * @param new_handler 指向新的异常处理函数的指针。
     * @param new_handler 如果为nullptr，则返回当前处理程序而不更改它。
     * @return 调用前的全局异常处理程序，如果new_handler为nullptr则返回当前处理程序。
     */
    RAINY_TOOLKIT_API exception_handler_t global_exception_handler(exception_handler_t new_handler = nullptr) noexcept;

    /**
     * \lang english
     * @brief Gets or sets the exception handler for the current thread.
     *
     *  This function manages the thread-specific handler that will be called for
     *  unhandled exceptions in the current thread. If set, this handler overrides
     *  the global exception handler for this thread.
     *
     * @param new_handler Pointer to the new thread-specific exception handler function.
     * @param new_handler If nullptr, the current thread handler is returned without changing it.
     * @return The previous thread-specific exception handler before this call,
     *          or the current thread handler if new_handler is nullptr.
     *
     * \lang simp-chinese
     * @brief 获取或设置当前线程的异常处理程序。
     *
     *  此函数管理当前线程中专用于处理未处理异常的处理程序。
     *  如果设置了此处理程序，它将覆盖当前线程的全局异常处理程序。
     *
     * @param new_handler 指向新的线程特定异常处理函数的指针。
     * @param new_handler 如果为nullptr，则返回当前线程处理程序而不更改它。
     * @return 调用前的线程特定异常处理程序，如果new_handler为nullptr则返回当前线程处理程序。
     */
    RAINY_TOOLKIT_API exception_handler_t current_thread_exception_handler(exception_handler_t new_handler = nullptr) noexcept;
}

namespace rainy::core::exceptions::implements {
    RAINY_TOOLKIT_API void invoke_exception_handler() noexcept;

    template <typename Except, bool NoExceptionHandlerInvoke = false>
    constexpr void report_error( // NOLINT
        const type_traits::other_trans::conditional_t<type_traits::type_relations::is_void_v<Except>, void *, Except> &exception) {
        if constexpr (type_traits::type_relations::is_void_v<Except>) {
            constexpr char message[] = "Detected error and the program cause a fatal error!";
            std::fwrite(message, sizeof(char), sizeof(char) * core::builtin::string_length(message), stderr); // NOLINT
        } else {
            std::fwrite(exception.what(), sizeof(char), sizeof(char) * core::builtin::string_length(exception.what()),
                        stderr); // NOLINT
        }
        if constexpr (!NoExceptionHandlerInvoke) {
            invoke_exception_handler();
        }
    }
}

namespace rainy::core::exceptions {
    /**
     * \lang english
     * @brief Base class for all rainy toolkit exceptions.
     *
     *  Derives from std::exception and augments it with a source location,
     *  so that the position where the exception was raised can be reported.
     *
     * \lang simp-chinese
     * @brief rainy工具包所有异常的基类。
     *
     *  派生自std::exception，并增加了源码位置信息，
     *  从而可以报告异常被抛出时的位置。
     */
    class RAINY_TOOLKIT_API exception : public std::exception {
    public:
        using base = std::exception;
        using source = diagnostics::source_location;

        /**
         * \lang english
         * @brief Constructs an exception with a message and a source location.
         * @param message The exception description.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 使用消息和源码位置构造异常。
         * @param message 异常描述。
         * @param location 抛出异常处的源码位置。
         */
        explicit exception(const char *message, const source &location = source::current());
        /**
         * \lang english
         * @brief Returns the exception description.
         * @return The exception message text.
         *
         * \lang simp-chinese
         * @brief 返回异常描述。
         * @return 异常消息文本。
         */
        RAINY_NODISCARD const char *what() const noexcept override;
        /**
         * \lang english
         * @brief Copy constructor.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         */
        exception(const exception &other);
        /**
         * \lang english
         * @brief Move constructor.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         */
        exception(exception &&other) noexcept;
        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         */
        exception &operator=(const exception &other);
        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         */
        exception &operator=(exception &&other) noexcept;
        /**
         * \lang english
         * @brief Destructor.
         *
         * \lang simp-chinese
         * @brief 析构函数。
         */
        ~exception() override;

    protected:
        /**
         * \lang english
         * @brief Default constructor for derived classes.
         *
         * \lang simp-chinese
         * @brief 供派生类使用的默认构造函数。
         */
        exception();
        /**
         * \lang english
         * @brief Builds the internal message from a description and a source location.
         * @param message The exception description.
         * @param location The source location to append to the message.
         *
         * \lang simp-chinese
         * @brief 从描述和源码位置构建内部消息。
         * @param message 异常描述。
         * @param location 要追加到消息中的源码位置。
         */
        void build_message(const char *message, const source &location);

    private:
        class impl;

        impl *impl_;
    };

    template <typename Except>
    constexpr void throw_exception(const Except &exception) {
        static_assert(type_traits::type_relations::is_base_of_v<std::exception, Except>,
                      "exception type must be derived from std::exception!");
#if __cpp_exceptions
        throw exception; // NOLINT
#else
        std::fwrite(exception.what(), sizeof(char), strlen(exception.what()), stderr);
        std::terminate();
#endif
    }

    /**
     * \lang english
     * @brief Returns the number of uncaught exceptions.
     *
     * @return Number of uncaught exceptions (0 if exceptions are disabled)
     *
     * \lang simp-chinese
     * @brief 返回未捕获异常的数量。
     *
     * @return 未捕获异常的数量（如果异常被禁用则返回0）
     */
    inline rain_fn uncaught_exceptions() noexcept -> int {
#if __cpp_exceptions
        return std::uncaught_exceptions();
#else
        return 0;
#endif
    }

    /**
     * \lang english
     * @brief Exception handling semantics enumeration.
     *
     *  Defines different semantic behaviors for exception handling,
     *  controlling how the program responds when errors occur.
     *
     * \lang simp-chinese
     * @brief 异常处理语义枚举。
     *
     *  定义异常处理的不同语义行为，控制程序在遇到错误时的响应方式。
     */
    enum class exception_semantic {
        /**
         * \lang english
         * @brief Enforce check
         *
         *  Perform strict error checking and throw exception on failure.
         *  Suitable for scenarios where operations must succeed.
         *
         * \lang simp-chinese
         * @brief 强制检查
         *
         *  执行严格的错误检查，在操作失败时抛出异常。
         *  适用于必须确保操作成功的场景。
         */
        enforce,

        /**
         * \lang english
         * @brief Observe only
         *
         *  Only report error information without interrupting execution,
         *  allowing the program to continue running.
         *  Suitable for scenarios where errors do not affect core functionality.
         *
         * \lang simp-chinese
         * @brief 仅观察
         *
         *  仅记录错误信息但不中断执行，允许程序继续运行。
         *  适用于错误不影响核心功能的场景。
         */
        observe,

        /**
         * \lang english
         * @brief Quick enforce
         *
         *  Perform quick error checking and terminate program directly on failure.
         *  Suitable for scenarios with unrecoverable errors.
         *
         * \lang simp-chinese
         * @brief 快速强制
         *
         *  执行快速错误检查，在失败时直接终止程序。
         *  适用于不可恢复的错误场景。
         */
        quick_enforce,

        /**
         * \lang english
         * @brief Assertion check
         *
         *  Use assertion mechanism for checking, output error message and
         *  terminate program on failure.
         *  Suitable for debugging and testing phases.
         *
         * \lang simp-chinese
         * @brief 断言检查
         *
         *  使用断言机制进行检查，失败时输出错误信息并终止程序。
         *  适用于调试和测试阶段。
         */
        assertion,

        /**
         * \lang english
         * @brief Ignore in release builds
         *
         *  Perform checks only in debug builds, completely ignore errors
         *  in release builds.
         *  Suitable for performance-sensitive release environments.
         *
         * \lang simp-chinese
         * @brief 发布版本忽略
         *
         *  仅在调试版本中进行检查，发布版本中完全忽略错误。
         *  适用于性能敏感的发布环境。
         */
        ignored_in_release
    };

    /**
     * \lang english
     * @brief Tag type for specifying which exception to throw.
     *
     * @tparam Except The exception type
     *
     * \lang simp-chinese
     * @brief 用于指定要抛出哪个异常的标签类型。
     *
     * @tparam Except 异常类型
     */
    template <typename Except>
    struct with_this_exception_t {
        explicit with_this_exception_t() = default;
    };

    /**
     * \lang english
     * @brief Global instance of with_this_exception_t.
     *
     * @tparam Except The exception type
     *
     * \lang simp-chinese
     * @brief with_this_exception_t的全局实例。
     *
     * @tparam Except 异常类型
     */
    template <typename Except>
    constexpr with_this_exception_t<Except> with_this_exception{};

    /**
     * \lang english
     * @brief Conditionally throws an exception based on a boolean condition.
     *
     * @tparam Except The exception type (must derive from std::exception)
     * @tparam Semantic The exception handling semantic
     * @tparam Args Constructor argument types for the exception
     * @param cond The condition to check (throws if false)
     * @param args Arguments to forward to the exception constructor
     *
     * \lang simp-chinese
     * @brief 基于布尔条件有条件地抛出异常。
     *
     * @tparam Except 异常类型（必须派生自std::exception）
     * @tparam Semantic 异常处理语义
     * @tparam Args 异常的构造函数参数类型
     * @param cond 要检查的条件（如果为false则抛出）
     * @param args 要转发给异常构造函数的参数
     */
    template <typename Except, exception_semantic Semantic = exception_semantic::enforce, typename... Args>
    constexpr void throw_exception_if(const bool cond, Args &&...args) {
        static_assert(type_traits::type_relations::is_base_of_v<std::exception, Except>,
                      "exception type must be derived from std::exception!");
        if (!cond) {
            if constexpr (Semantic == exception_semantic::ignored_in_release) {
                if constexpr (core::is_rainy_enable_debug) {
                    type_traits::other_trans::decay_t<Except> exception(utility::forward<Args>(args)...);
                    implements::report_error<Except>(exception);
                }
            } else {
                if constexpr (Semantic == exception_semantic::quick_enforce) {
                    implements::report_error<void>(nullptr);
                } else {
                    if constexpr (Semantic == exception_semantic::observe) {
                        Except exception(utility::forward<Args>(args)...);
                        implements::report_error<Except, true>(exception);
                    } else {
                        if constexpr (core::is_rainy_enable_exception) {
                            Except exception(utility::forward<Args>(args)...);
                            if constexpr (Semantic == exception_semantic::assertion) {
                                implements::report_error<Except>(exception);
                            } else {
#if __cpp_exceptions
                                throw exception;
#endif
                            }
                        } else {
                            type_traits::other_trans::decay_t<Except> exception(utility::forward<Args>(args)...);
                            implements::report_error<Except>(exception);
                        }
                    }
                }
            }
        }
    }
}

    /**
     * \lang english
     * @brief Defines an exception class deriving from the given base class.
     *
     *  The generated class provides a constructor taking an optional message
     *  (defaulting to default_message) and a source location.
     *
     * \lang simp-chinese
     * @brief 定义派生自给定基类的异常类。
     *
     *  生成的类提供接受可选消息（默认为default_message）和源码位置的构造函数。
     */
#define RAINY_DEFINE_EXCEPTION(class_name, base_class, default_message)                                                               \
    class class_name : public base_class {                                                                                            \
    public:                                                                                                                           \
        using base = base_class;                                                                                                      \
        explicit class_name(const char *message = default_message, const source &location = source::current()) :                      \
            base(message, location) {                                                                                                 \
        }                                                                                                                             \
    }


    /**
     * \lang english
     * @brief Defines a non-final exception class deriving from the given base class.
     *
     *  Identical to RAINY_DEFINE_EXCEPTION except that the generated class is
     *  not marked final and may be used as a base class.
     *
     * \lang simp-chinese
     * @brief 定义派生自给定基类的非final异常类。
     *
     *  与RAINY_DEFINE_EXCEPTION相同，但生成的类未标记为final，可被用作基类。
     */
#define RAINY_DEFINE_EXCEPTION_NONFINAL(class_name, base_class, default_message)                                                      \
    class class_name : public base_class {                                                                                            \
    public:                                                                                                                           \
        using base = base_class;                                                                                                      \
        explicit class_name(const char *message = default_message, const source &location = source::current()) :                      \
            base(message, location) {                                                                                                 \
        }                                                                                                                             \
    }


#define RAINY_DEFINE_EXCEPTION_WITH_THROW(class_name, base_class, default_message, throw_func)                                        \
    class class_name : public base_class {                                                                                            \
    public:                                                                                                                           \
        using base = base_class;                                                                                                      \
        explicit class_name(const char *message = default_message, const source &location = source::current()) :                      \
            base(message, location) {                                                                                                 \
        }                                                                                                                             \
    };                                                                                                                                \
    RAINY_INLINE rain_fn throw_func(const char *message = default_message,                                                            \
                                    const utility::source_location &location = utility::source_location::current()) -> void {         \
        throw_exception(class_name{message, location});                                                                               \
    }

#define RAINY_DEFINE_EXCEPTION_CUSTOM(class_name, base_class, default_message, params, init_list)                                     \
    class class_name : public base_class {                                                                                            \
    public:                                                                                                                           \
        using base = base_class;                                                                                                      \
        explicit class_name(params, const char *message = default_message, const source &location = source::current()) :              \
            base(message, location), init_list {                                                                                      \
        }                                                                                                                             \
    }

namespace rainy::core::exceptions::logic {
    /**
     * \lang english
     * @brief Exception type for logic errors, such as violations of logical preconditions.
     *
     * \lang simp-chinese
     * @brief 逻辑错误异常类型，例如违反逻辑前置条件的错误。
     */
    class logic_error : public exception {
    public:
        using base = exception;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        logic_error() = default;

        /**
         * \lang english
         * @brief Constructs a logic_error with a message and a source location.
         * @param message The exception description.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 使用消息和源码位置构造logic_error。
         * @param message 异常描述。
         * @param location 抛出异常处的源码位置。
         */
        explicit logic_error(const char *message, const source &location = source::current()) : base(message, location) {
        }
    };

    /**
     * \lang english
     * @brief Exception type for out-of-range access, together with the throw helper throw_out_of_range.
     *
     * \lang simp-chinese
     * @brief 越界访问异常类型，以及抛出辅助函数throw_out_of_range。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(out_of_range, logic_error, "out_of_range", throw_out_of_range)
    /**
     * \lang english
     * @brief Exception type for length errors, together with the throw helper throw_length_error.
     *
     * \lang simp-chinese
     * @brief 长度错误异常类型，以及抛出辅助函数throw_length_error。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(length_error, logic_error, "length error", throw_length_error)
}

namespace rainy::core::exceptions::runtime {
    /**
     * \lang english
     * @brief Exception type for runtime errors.
     *
     * \lang simp-chinese
     * @brief 运行时错误异常类型。
     */
    class runtime_error : public exception {
    public:
        using base = exception;

        /**
         * \lang english
         * @brief Constructs a runtime_error with an optional message and a source location.
         * @param message The exception description, defaulting to "runtime error".
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 使用可选消息和源码位置构造runtime_error。
         * @param message 异常描述，默认为"runtime error"。
         * @param location 抛出异常处的源码位置。
         */
        explicit runtime_error(const char *message = "runtime error", const source &location = source::current()) :
            base(message, location) {
        }
    };

    /**
     * \lang english
     * @brief Constructs and throws a runtime_error.
     * @param message The exception description, defaulting to "runtime error".
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 构造并抛出runtime_error。
     * @param message 异常描述，默认为"runtime error"。
     * @param location 抛出异常处的源码位置。
     */
    inline auto throw_runtime_error(const char *message = "runtime error",
                                    const utility::source_location &location = utility::source_location::current()) -> void {
        throw_exception(runtime_error{message, location});
    }

    /**
     * \lang english
     * @brief Exception type for null pointer dereferences, together with the throw helper throw_nullpointer_exception.
     *
     * \lang simp-chinese
     * @brief 空指针解引用异常类型，以及抛出辅助函数throw_nullpointer_exception。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(nullpointer_exception, exception, "detected nullpointer", throw_nullpointer_exception)
    /**
     * \lang english
     * @brief Exception type for allocation failures, together with the throw helper throw_bad_alloc.
     *
     * \lang simp-chinese
     * @brief 内存分配失败异常类型，以及抛出辅助函数throw_bad_alloc。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_alloc, runtime_error, "bad allocation", throw_bad_alloc)
    /**
     * \lang english
     * @brief Exception type for invalid casts, together with the throw helper throw_bad_cast.
     *
     * \lang simp-chinese
     * @brief 无效类型转换异常类型，以及抛出辅助函数throw_bad_cast。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_cast, runtime_error, "bad cast", throw_bad_cast)
    /**
     * \lang english
     * @brief Exception type for arithmetic overflow, together with the throw helper throw_overflow_error.
     *
     * \lang simp-chinese
     * @brief 算术溢出异常类型，以及抛出辅助函数throw_overflow_error。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(overflow_error, runtime_error, "overflow error", throw_overflow_error)
    /**
     * \lang english
     * @brief Exception type for arithmetic underflow, together with the throw helper throw_underflow_error.
     *
     * \lang simp-chinese
     * @brief 算术下溢异常类型，以及抛出辅助函数throw_underflow_error。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(underflow_error, runtime_error, "underflow error", throw_underflow_error)
    /**
     * \lang english
     * @brief Exception type for domain errors, together with the throw helper throw_domain_error.
     *
     * \lang simp-chinese
     * @brief 定义域错误异常类型，以及抛出辅助函数throw_domain_error。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(domain_error, runtime_error, "domain error", throw_domain_error)
}

namespace rainy::core::exceptions::runtime {
    /**
     * \lang english
     * @brief Exception type that carries an OS or library error code.
     *
     *  Combines a runtime_error message with a std::error_code, and exposes the
     *  error code via code().
     *
     * \lang simp-chinese
     * @brief 携带操作系统或库错误码的异常类型。
     *
     *  将runtime_error消息与std::error_code结合，并通过code()暴露错误码。
     */
    class system_error : public runtime_error {
    public:
        using source = source;

        /**
         * \lang english
         * @brief Constructs a system_error from an error code and a string description.
         * @param ec The error code.
         * @param what_arg The description of the error.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 从错误码和字符串描述构造system_error。
         * @param ec 错误码。
         * @param what_arg 错误的描述。
         * @param location 抛出异常处的源码位置。
         */
        system_error(const std::error_code ec, const core::text::string &what_arg,
                     const source &location = diagnostics::source_location::current()) : estr(make_err_msg(ec, what_arg)), ec{ec} {
            this->build_message(estr.c_str(), location);
        }

        /**
         * \lang english
         * @brief Constructs a system_error from an error code and a C string description.
         * @param ec The error code.
         * @param what_arg The description of the error.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 从错误码和C字符串描述构造system_error。
         * @param ec 错误码。
         * @param what_arg 错误的描述。
         * @param location 抛出异常处的源码位置。
         */
        system_error(const std::error_code ec, const char *what_arg,
                     const source &location = diagnostics::source_location::current()) : estr(make_err_msg(ec, what_arg)), ec{ec} {
            this->build_message(estr.c_str(), location);
        }

        /**
         * \lang english
         * @brief Constructs a system_error from an error code, using its message as the description.
         * @param ec The error code.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 从错误码构造system_error，使用错误码自身的消息作为描述。
         * @param ec 错误码。
         * @param location 抛出异常处的源码位置。
         */
        system_error(const std::error_code ec, const source &location = diagnostics::source_location::current()) :
            estr(make_err_msg(ec, ec.message())), ec{ec} { // NOLINT
            this->build_message(estr.c_str(), location);
        }

        /**
         * \lang english
         * @brief Constructs a system_error from an error value, category, and string description.
         * @param ev The error value.
         * @param ecat The error category.
         * @param what_arg The description of the error.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 从错误值、错误类别和字符串描述构造system_error。
         * @param ev 错误值。
         * @param ecat 错误类别。
         * @param what_arg 错误的描述。
         * @param location 抛出异常处的源码位置。
         */
        system_error(const int ev, const std::error_category &ecat, const core::text::string &what_arg,
                     const source &location = diagnostics::source_location::current()) :
            estr(make_err_msg(std::error_code(ev, ecat), what_arg)), ec{std::error_code(ev, ecat)} {
            this->build_message(estr.c_str(), location);
        }

        /**
         * \lang english
         * @brief Constructs a system_error from an error value, category, and C string description.
         * @param ev The error value.
         * @param ecat The error category.
         * @param what_arg The description of the error.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 从错误值、错误类别和C字符串描述构造system_error。
         * @param ev 错误值。
         * @param ecat 错误类别。
         * @param what_arg 错误的描述。
         * @param location 抛出异常处的源码位置。
         */
        system_error(const int ev, const std::error_category &ecat, const char *what_arg,
                     const source &location = diagnostics::source_location::current()) :
            estr(make_err_msg(std::error_code(ev, ecat), what_arg)), ec{std::error_code(ev, ecat)} {
            this->build_message(estr.c_str(), location);
        }

        /**
         * \lang english
         * @brief Constructs a system_error from an error value and category, using the error message as the description.
         * @param ev The error value.
         * @param ecat The error category.
         * @param location The source location where the exception is raised.
         *
         * \lang simp-chinese
         * @brief 从错误值和错误类别构造system_error，使用错误消息作为描述。
         * @param ev 错误值。
         * @param ecat 错误类别。
         * @param location 抛出异常处的源码位置。
         */
        system_error(const int ev, const std::error_category &ecat, const source &location = diagnostics::source_location::current()) :
            estr(make_err_msg(std::error_code(ev, ecat), std::error_code(ev, ecat).message())), ec{std::error_code(ev, ecat)} {
            this->build_message(estr.c_str(), location);
        }

        /**
         * \lang english
         * @brief Returns the error code associated with this exception.
         * @return The underlying std::error_code.
         *
         * \lang simp-chinese
         * @brief 返回与此异常关联的错误码。
         * @return 底层的std::error_code。
         */
        const std::error_code &code() const noexcept {
            return ec;
        }

    private:
        static core::text::string make_err_msg(std::error_code error_code, core::text::string message) {
            if (!message.empty()) {
                message.append(": ");
            }
            message.append(error_code.message());
            return message;
        }

        core::text::string estr;
        std::error_code ec;
    };

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error code and a string description.
     * @param ec The error code.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误码和字符串描述构造的system_error。
     * @param ec 错误码。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error(const std::error_code ec, const core::text::string &what_arg,
                                      const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw system_error{ec, what_arg, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error code and a C string description.
     * @param ec The error code.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误码和C字符串描述构造的system_error。
     * @param ec 错误码。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error(const std::error_code ec, const char *what_arg,
                                      const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw system_error{ec, what_arg, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error code, using its message as the description.
     * @param ec The error code.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误码构造的system_error，使用错误码自身的消息作为描述。
     * @param ec 错误码。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error(const std::error_code ec,
                                      const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw system_error{ec, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error value, category, and string description.
     * @param ev The error value.
     * @param ecat The error category.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误值、错误类别和字符串描述构造的system_error。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error(const int ev, const std::error_category &ecat, const core::text::string &what_arg,
                                      const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw system_error{ev, ecat, what_arg, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error value, category, and C string description.
     * @param ev The error value.
     * @param ecat The error category.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误值、错误类别和C字符串描述构造的system_error。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error(const int ev, const std::error_category &ecat, const char *what_arg,
                                      const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw system_error{ev, ecat, what_arg, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error value and category, using the error message as the description.
     * @param ev The error value.
     * @param ecat The error category.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误值和错误类别构造的system_error，使用错误消息作为描述。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error(const int ev, const std::error_category &ecat,
                                      const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw system_error{ev, ecat, location};
    }

    /**
     * \lang english
     * @brief Creates a system_error from an error code and a string description.
     * @param ec The error code.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     * @return The constructed system_error object.
     *
     * \lang simp-chinese
     * @brief 从错误码和字符串描述创建system_error。
     * @param ec 错误码。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     * @return 构造出的system_error对象。
     */
    inline rain_fn make_system_error(const std::error_code ec, const core::text::string &what_arg,
                                     const diagnostics::source_location &location = diagnostics::source_location::current())
        -> system_error {
        return system_error{ec, what_arg, location};
    }

    /**
     * \lang english
     * @brief Creates a system_error from an error code and a C string description.
     * @param ec The error code.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     * @return The constructed system_error object.
     *
     * \lang simp-chinese
     * @brief 从错误码和C字符串描述创建system_error。
     * @param ec 错误码。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     * @return 构造出的system_error对象。
     */
    inline rain_fn make_system_error(const std::error_code ec, const char *what_arg,
                                     const diagnostics::source_location &location = diagnostics::source_location::current())
        -> system_error {
        return system_error{ec, what_arg, location};
    }

    /**
     * \lang english
     * @brief Creates a system_error from an error code, using its message as the description.
     * @param ec The error code.
     * @param location The source location where the exception is raised.
     * @return The constructed system_error object.
     *
     * \lang simp-chinese
     * @brief 从错误码创建system_error，使用错误码自身的消息作为描述。
     * @param ec 错误码。
     * @param location 抛出异常处的源码位置。
     * @return 构造出的system_error对象。
     */
    inline rain_fn make_system_error(const std::error_code ec,
                                     const diagnostics::source_location &location = diagnostics::source_location::current())
        -> system_error {
        return system_error{ec, location};
    }

    /**
     * \lang english
     * @brief Creates a system_error from an error value, category, and string description.
     * @param ev The error value.
     * @param ecat The error category.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     * @return The constructed system_error object.
     *
     * \lang simp-chinese
     * @brief 从错误值、错误类别和字符串描述创建system_error。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     * @return 构造出的system_error对象。
     */
    inline rain_fn make_system_error(const int ev, const std::error_category &ecat, const core::text::string &what_arg,
                                     const diagnostics::source_location &location = diagnostics::source_location::current())
        -> system_error {
        return system_error{ev, ecat, what_arg, location};
    }

    /**
     * \lang english
     * @brief Creates a system_error from an error value, category, and C string description.
     * @param ev The error value.
     * @param ecat The error category.
     * @param what_arg The description of the error.
     * @param location The source location where the exception is raised.
     * @return The constructed system_error object.
     *
     * \lang simp-chinese
     * @brief 从错误值、错误类别和C字符串描述创建system_error。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param what_arg 错误的描述。
     * @param location 抛出异常处的源码位置。
     * @return 构造出的system_error对象。
     */
    inline rain_fn make_system_error(const int ev, const std::error_category &ecat, const char *what_arg,
                                     const diagnostics::source_location &location = diagnostics::source_location::current())
        -> system_error {
        return system_error{ev, ecat, what_arg, location};
    }

    /**
     * \lang english
     * @brief Creates a system_error from an error value and category, using the error message as the description.
     * @param ev The error value.
     * @param ecat The error category.
     * @param location The source location where the exception is raised.
     * @return The constructed system_error object.
     *
     * \lang simp-chinese
     * @brief 从错误值和错误类别创建system_error，使用错误消息作为描述。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param location 抛出异常处的源码位置。
     * @return 构造出的system_error对象。
     */
    inline rain_fn make_system_error(const int ev, const std::error_category &ecat,
                                     const diagnostics::source_location &location = diagnostics::source_location::current())
        -> system_error {
        return system_error{ev, ecat, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error code.
     * @param ec The error code.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误码构造的system_error。
     * @param ec 错误码。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error_code(const std::error_code ec,
                                           const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw system_error{ec, location};
    }

    /**
     * \lang english
     * @brief Throws a system_error constructed from an error value and category.
     * @param ev The error value.
     * @param ecat The error category.
     * @param location The source location where the exception is raised.
     *
     * \lang simp-chinese
     * @brief 抛出由错误值和错误类别构造的system_error。
     * @param ev 错误值。
     * @param ecat 错误类别。
     * @param location 抛出异常处的源码位置。
     */
    inline rain_fn throw_system_error_code(int ev, const std::error_category &ecat,
                                           const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw system_error{ev, ecat, location};
    }
}

#endif
