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
#ifndef RAINY_BASIC_EXCEPTIONS_HPP
#define RAINY_BASIC_EXCEPTIONS_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/implements/source_location.hpp>
#include <rainy/core/implements/text/string.hpp>
#include <utility>

namespace rainy::foundation::exceptions {
    /**
     * @brief Type definition for exception handler function pointers.
     *        异常处理函数指针的类型定义。
     *
     * An exception handler is a function that takes no arguments and returns void,
     * typically called when an unhandled exception occurs.
     *
     * 异常处理程序是一个无参数、无返回值的函数，
     * 通常在未处理的异常发生时被调用。
     */
    using exception_handler_t = void (*)();

    /**
     * @brief Gets or sets the global exception handler.
     *        获取或设置全局异常处理程序。
     *
     * This function manages the handler that will be called for unhandled exceptions
     * at the global level across all threads, unless overridden by a thread-specific handler.
     *
     * 此函数管理在全局级别（跨所有线程）处理未处理异常时调用的处理程序，
     * 除非被特定于线程的处理程序覆盖。
     *
     * @param new_handler Pointer to the new exception handler function.
     *                    If nullptr, the current handler is returned without changing it.
     *                    指向新的异常处理函数的指针。
     *                    如果为nullptr，则返回当前处理程序而不更改它。
     * @return The previous global exception handler before this call,
     *         or the current handler if new_handler is nullptr.
     *         调用前的全局异常处理程序，如果new_handler为nullptr则返回当前处理程序。
     */
    RAINY_TOOLKIT_API exception_handler_t global_exception_handler(exception_handler_t new_handler = nullptr) noexcept;

    /**
     * @brief Gets or sets the exception handler for the current thread.
     *        获取或设置当前线程的异常处理程序。
     *
     * This function manages the thread-specific handler that will be called for
     * unhandled exceptions in the current thread. If set, this handler overrides
     * the global exception handler for this thread.
     *
     * 此函数管理当前线程中专用于处理未处理异常的处理程序。
     * 如果设置了此处理程序，它将覆盖当前线程的全局异常处理程序。
     *
     * @param new_handler Pointer to the new thread-specific exception handler function.
     *                    If nullptr, the current thread handler is returned without changing it.
     *                    指向新的线程特定异常处理函数的指针。
     *                    如果为nullptr，则返回当前线程处理程序而不更改它。
     * @return The previous thread-specific exception handler before this call,
     *         or the current thread handler if new_handler is nullptr.
     *         调用前的线程特定异常处理程序，如果new_handler为nullptr则返回当前线程处理程序。
     */
    RAINY_TOOLKIT_API exception_handler_t current_thread_exception_handler(exception_handler_t new_handler = nullptr) noexcept;
}

namespace rainy::foundation::exceptions::implements {
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

namespace rainy::foundation::exceptions {
    /**
     * @brief Base exception class for the Rainy framework.
     *        Provides source location information along with the error message.
     *
     *        Rainy框架的基础异常类。
     *        提供错误消息和源位置信息。
     */
    class exception : public std::exception {
    public:
        using base = std::exception;
        using source = diagnostics::source_location;

        /**
         * @brief Constructs an exception with a message and source location.
         *        使用消息和源位置构造异常。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location where the exception occurred
         *                 异常发生的源位置
         */
        explicit exception(const char *message, const source &location = source::current()) :
            message((location.to_string() + " : " + message)) {
        }

        /**
         * @brief Constructs an exception with a string message and source location.
         *        使用字符串消息和源位置构造异常。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location where the exception occurred
         *                 异常发生的源位置
         */
        explicit exception(const std::string &message, const source &location = source::current()) :
            message((location.to_string() + " : " + message)) {
        }

        /**
         * @brief Returns the error message.
         *        返回错误消息。
         *
         * @return The error message as a C-string
         *         作为C字符串的错误消息
         */
        RAINY_NODISCARD const char *what() const noexcept override {
            return message.c_str();
        }

    private:
        std::string message;
    };

    /**
     * @brief Wrapper class that adapts any exception type to std::exception.
     *        将任何异常类型适配到std::exception的包装类。
     *
     * @tparam Except The exception type to wrap
     *                要包装的异常类型
     */
    template <typename Except>
    class wrapexcept final : public std::exception {
    public:
        /**
         * @brief Constructs a wrapper from an rvalue reference.
         *        从右值引用构造包装器。
         *
         * @param except_instance The exception instance to move
         *                        要移动的异常实例
         */
        explicit wrapexcept(Except &&except_instance) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Except>) :
            exception_(utility::move(except_instance)) {
        }

        /**
         * @brief Constructs a wrapper from a const lvalue reference.
         *        从常量左值引用构造包装器。
         *
         * @param except_instance The exception instance to copy
         *                        要拷贝的异常实例
         */
        explicit wrapexcept(const Except &except_instance) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<Except>) : exception_(except_instance) {
        }

        /**
         * @brief Returns the error message from the wrapped exception.
         *        返回被包装异常的错误消息。
         *
         * @return The error message as a C-string
         *         作为C字符串的错误消息
         */
        RAINY_NODISCARD const char *what() const noexcept override {
            return exception_.what();
        }

        /**
         * @brief Gets a reference to the wrapped exception.
         *        获取被包装异常的引用。
         *
         * @return Reference to the wrapped exception
         *         被包装异常的引用
         */
        Except &get_exception() noexcept {
            return exception_;
        }

    private:
        Except exception_;
    };

    /**
     * @brief Returns the number of uncaught exceptions.
     *        返回未捕获异常的数量。
     *
     * @return Number of uncaught exceptions (0 if exceptions are disabled)
     *         未捕获异常的数量（如果异常被禁用则返回0）
     */
    inline rain_fn uncaught_exceptions() noexcept -> int {
#if __cpp_exceptions
        return std::uncaught_exceptions();
#else
        return 0;
#endif
    }

    /**
     * @brief Exception handling semantics enumeration.
     *        异常处理语义枚举。
     *
     * Defines different semantic behaviors for exception handling,
     * controlling how the program responds when errors occur.
     * 定义异常处理的不同语义行为，控制程序在遇到错误时的响应方式。
     */
    enum class exception_semantic {
        /**
         * @brief Enforce check
         *        强制检查
         *
         * Perform strict error checking and throw exception on failure.
         * Suitable for scenarios where operations must succeed.
         * 执行严格的错误检查，在操作失败时抛出异常。
         * 适用于必须确保操作成功的场景。
         */
        enforce,

        /**
         * @brief Observe only
         *        仅观察
         *
         * Only report error information without interrupting execution,
         * allowing the program to continue running.
         * Suitable for scenarios where errors do not affect core functionality.
         * 仅记录错误信息但不中断执行，允许程序继续运行。
         * 适用于错误不影响核心功能的场景。
         */
        observe,

        /**
         * @brief Quick enforce
         *        快速强制
         *
         * Perform quick error checking and terminate program directly on failure.
         * Suitable for scenarios with unrecoverable errors.
         * 执行快速错误检查，在失败时直接终止程序。
         * 适用于不可恢复的错误场景。
         */
        quick_enforce,

        /**
         * @brief Assertion check
         *        断言检查
         *
         * Use assertion mechanism for checking, output error message and
         * terminate program on failure.
         * Suitable for debugging and testing phases.
         * 使用断言机制进行检查，失败时输出错误信息并终止程序。
         * 适用于调试和测试阶段。
         */
        assertion,

        /**
         * @brief Ignore in release builds
         *        发布版本忽略
         *
         * Perform checks only in debug builds, completely ignore errors
         * in release builds.
         * Suitable for performance-sensitive release environments.
         * 仅在调试版本中进行检查，发布版本中完全忽略错误。
         * 适用于性能敏感的发布环境。
         */
        ignored_in_release
    };

    /**
     * @brief Tag type for specifying which exception to throw.
     *        用于指定要抛出哪个异常的标签类型。
     *
     * @tparam Except The exception type
     *                异常类型
     */
    template <typename Except>
    struct with_this_exception_t {
        explicit with_this_exception_t() = default;
    };

    /**
     * @brief Global instance of with_this_exception_t.
     *        with_this_exception_t的全局实例。
     *
     * @tparam Except The exception type
     *                异常类型
     */
    template <typename Except>
    constexpr with_this_exception_t<Except> with_this_exception{};

    /**
     * @brief Throws an exception.
     *        抛出异常。
     *
     * @tparam Except The exception type (must derive from std::exception)
     *                异常类型（必须派生自std::exception）
     * @param exception The exception instance to throw
     *                  要抛出的异常实例
     */
    template <typename Except>
    constexpr void throw_exception(const Except &exception) {
        static_assert(type_traits::type_relations::is_base_of_v<std::exception, Except>,
                      "exception type must be derived from std::exception!");
#if __cpp_exceptions
        throw wrapexcept<Except>{exception}.get_exception();
#else
        std::fwrite(exception.what(), sizeof(char), sizeof(char) * core::implements::string_length(exception.what()), stderr);
        std::terminate();
#endif
    }

    /**
     * @brief Conditionally throws an exception based on a boolean condition.
     *        基于布尔条件有条件地抛出异常。
     *
     * @tparam Except The exception type (must derive from std::exception)
     *                异常类型（必须派生自std::exception）
     * @tparam Semantic The exception handling semantic
     *                  异常处理语义
     * @tparam Args Constructor argument types for the exception
     *              异常的构造函数参数类型
     * @param cond The condition to check (throws if false)
     *             要检查的条件（如果为false则抛出）
     * @param args Arguments to forward to the exception constructor
     *             要转发给异常构造函数的参数
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
                        type_traits::other_trans::decay_t<Except> exception(utility::forward<Args>(args)...);
                        implements::report_error<Except, true>(exception);
                    } else {
                        if constexpr (core::is_rainy_enable_exception) {
                            type_traits::other_trans::decay_t<Except> exception(utility::forward<Args>(args)...);
                            if constexpr (Semantic == exception_semantic::assertion) {
                                implements::report_error<Except>(exception);
                            } else {
                                throw wrapexcept<Except>{utility::move(exception)}.get_exception();
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

    /**
     * @brief Conditionally throws an exception using the with_this_exception tag.
     *        使用with_this_exception标签有条件地抛出异常。
     *
     * @tparam Semantic The exception handling semantic
     *                  异常处理语义
     * @tparam Except The exception type
     *                异常类型
     * @tparam Args Constructor argument types
     *              构造函数参数类型
     * @param  with_this_exception_t tag
     * @param cond The condition to check (throws if false)
     *             要检查的条件（如果为false则抛出）
     * @param args Arguments to forward to the exception constructor
     *             要转发给异常构造函数的参数
     */
    template <exception_semantic Semantic = exception_semantic::enforce, typename Except, typename... Args>
    RAINY_INLINE constexpr void throw_exception_if(with_this_exception_t<Except>, bool cond, Args &&...args) {
        throw_exception_if<Except, Semantic>(cond, utility::forward<Args>(args)...);
    }

    /**
     * @brief Converts a std::exception to a rainy::foundation::exceptions::exception.
     *        将std::exception转换为rainy::foundation::exceptions::exception。
     *
     * @param except The std::exception to convert
     *               要转换的std::exception
     * @param location The source location
     *                 源位置
     * @return A rainy::foundation::exceptions::exception instance
     *         rainy::foundation::exceptions::exception实例
     */
    RAINY_INLINE rain_fn stdexcept_to_rexcept(const std::exception &except,
                                              const diagnostics::source_location &location = diagnostics::source_location::current())
        -> exception {
        if (!dynamic_cast<const exception *>(&except)) {
            return exception{except.what(), location};
        }
        return exception{""};
    }
}

namespace rainy::foundation::exceptions::runtime {
    /**
     * @brief Runtime error exception class.
     *        运行时错误异常类。
     */
    class runtime_error : public exception {
    public:
        using base = exception;

        /**
         * @brief Constructs a runtime_error with default message.
         *        使用默认消息构造runtime_error。
         *
         * @param location The source location
         *                 源位置
         */
        explicit runtime_error(const source &location = source::current()) noexcept : base("I Got a runtime_error!", location) {
        }

        /**
         * @brief Constructs a runtime_error with a custom message.
         *        使用自定义消息构造runtime_error。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location
         *                 源位置
         */
        explicit runtime_error(const char *message, const source &location = source::current()) : base(message, location) {
        }

        /**
         * @brief Constructs a runtime_error with a string message.
         *        使用字符串消息构造runtime_error。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location
         *                 源位置
         */
        explicit runtime_error(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    /**
     * @brief Throws a runtime_error.
     *        抛出runtime_error。
     *
     * @param message The error message
     *                错误消息
     * @param location The source location
     *                 源位置
     */
    RAINY_INLINE rain_fn throw_runtime_error(const char *message,
                                             const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(runtime_error{message, location});
    }
}

namespace rainy::foundation::exceptions::logic {
    /**
     * @brief Logic error exception class.
     *        逻辑错误异常类。
     */
    class logic_error : public exception {
    public:
        using base = exception;

        /**
         * @brief Constructs a logic_error with a custom message.
         *        使用自定义消息构造logic_error。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location
         *                 源位置
         */
        explicit logic_error(const char *message, const source &location = source::current()) : base(message, location) {
        }

        /**
         * @brief Constructs a logic_error with a string message.
         *        使用字符串消息构造logic_error。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location
         *                 源位置
         */
        explicit logic_error(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    /**
     * @brief Throws a logic_error.
     *        抛出logic_error。
     *
     * @param message The error message
     *                错误消息
     * @param location The source location
     *                 源位置
     */
    RAINY_INLINE rain_fn throw_logic_error(const char *message,
                                           const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(logic_error{message, location});
    }

    /**
     * @brief Out of range exception class.
     *        范围超出异常类。
     */
    class out_of_range final : public logic_error {
    public:
        using base = logic_error;

        /**
         * @brief Constructs an out_of_range exception.
         *        构造out_of_range异常。
         *
         * @param message The error message (default: "out_of_range")
         *                错误消息（默认："out_of_range"）
         * @param location The source location
         *                 源位置
         */
        explicit out_of_range(const char *message = "out_of_range", const source &location = source::current()) :
            base(message, location) {
        }

        /**
         * @brief Constructs an out_of_range exception with a string message.
         *        使用字符串消息构造out_of_range异常。
         *
         * @param message The error message (default: "out_of_range")
         *                错误消息（默认："out_of_range"）
         * @param location The source location
         *                 源位置
         */
        explicit out_of_range(const std::string &message = "out_of_range", const source &location = source::current()) :
            base(message, location) {
        }
    };

    /**
     * @brief Throws an out_of_range exception.
     *        抛出out_of_range异常。
     *
     * @param message The error message (default: "out_of_range")
     *                错误消息（默认："out_of_range"）
     * @param location The source location
     *                 源位置
     */
    RAINY_INLINE rain_fn throw_out_of_range(const char *message = "out_of_range",
                                            const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(out_of_range{message, location});
    }

    /**
     * @brief Not implemented exception class.
     *        未实现异常类。
     */
    class not_implemented : public logic_error {
    public:
        using base = logic_error;

        /**
         * @brief Constructs a not_implemented exception.
         *        构造not_implemented异常。
         *
         * @param message The error message (default: "not_implemented")
         *                错误消息（默认："not_implemented"）
         * @param location The source location
         *                 源位置
         */
        explicit not_implemented(const char *message = "not_implemented", const source &location = source::current()) :
            base(message, location) {
        }

        /**
         * @brief Constructs a not_implemented exception with a string message.
         *        使用字符串消息构造not_implemented异常。
         *
         * @param message The error message (default: "not_implemented")
         *                错误消息（默认："not_implemented"）
         * @param location The source location
         *                 源位置
         */
        explicit not_implemented(const std::string &message = "not_implemented", const source &location = source::current()) :
            base(message, location) {
        }
    };

    /**
     * @brief Throws a not_implemented exception.
     *        抛出not_implemented异常。
     *
     * @param message The error message (default: "not_implemented")
     *                错误消息（默认："not_implemented"）
     * @param location The source location
     *                 源位置
     */
    RAINY_INLINE rain_fn throw_not_implemented(const char *message = "not_implemented",
                                               const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(not_implemented{message, location});
    }
}

namespace rainy::foundation::exceptions::cast {
    /**
     * @brief Bad cast exception class.
     *        错误的类型转换异常类。
     */
    class bad_cast : public exception {
    public:
        using base = exception;

        /**
         * @brief Constructs a bad_cast exception with default message.
         *        使用默认消息构造bad_cast异常。
         *
         * @param location The source location
         *                 源位置
         */
        explicit bad_cast(const source &location = source::current()) : base("bad cast", location) {
        }

        /**
         * @brief Constructs a bad_cast exception with a custom message.
         *        使用自定义消息构造bad_cast异常。
         *
         * @param message The error message
         *                错误消息
         * @param location The source location
         *                 源位置
         */
        explicit bad_cast(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    /**
     * @brief Throws a bad_cast exception.
     *        抛出bad_cast异常。
     *
     * @param message The error message
     *                错误消息
     * @param location The source location
     *                 源位置
     */
    RAINY_INLINE rain_fn throw_bad_cast(const char *message,
                                        const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        throw_exception(bad_cast{message, location});
    }
}

#if RAINY_HAS_CXX20
template <>
struct std::formatter<rainy::foundation::exceptions::exception, char> {
    explicit formatter() noexcept = default;

    static auto parse(const format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    static auto format(const rainy::foundation::exceptions::exception &value, std::format_context fc) noexcept {
        return std::format_to(fc.out(), "{}", value.what());
    }
};
#endif

namespace rainy::utility {
    using foundation::exceptions::exception_semantic;
    using foundation::exceptions::stdexcept_to_rexcept;
    using foundation::exceptions::throw_exception;
    using foundation::exceptions::throw_exception_if;
    using foundation::exceptions::with_this_exception;
    using foundation::exceptions::with_this_exception_t;
}

namespace rainy::foundation::exceptions::runtime {
    /**
     * @brief Bad allocation exception class.
     *        内存分配失败异常类。
     */
    class bad_alloc final : public runtime_error {
    public:
        using base = runtime_error;

        /**
         * @brief Constructs a bad_alloc exception.
         *        构造bad_alloc异常。
         *
         * @param location The source location
         *                 源位置
         */
        explicit bad_alloc(const source &location = source::current()) : base("bad allocation", location) {
        }
    };

    /**
     * @brief Throws a bad_alloc exception.
     *        抛出bad_alloc异常。
     */
    RAINY_INLINE rain_fn throw_bad_alloc() -> void {
        throw_exception(bad_alloc{});
    }
}

namespace rainy::foundation::exceptions {
    /**
     * @brief Multiple exceptions exception class.
     *        多个异常同时发生的异常类。
     */
    class multiple_exceptions : public exception {
    public:
        /**
         * @brief Constructs a multiple_exceptions exception.
         *        构造multiple_exceptions异常。
         *
         * @param first The first exception_ptr
         *              第一个exception_ptr
         */
        explicit multiple_exceptions(std::exception_ptr first) noexcept :
            exception("multiple exceptions"), first_(utility::move(first)) {
        }

        /**
         * @brief Gets the first exception_ptr.
         *        获取第一个exception_ptr。
         *
         * @return The first exception_ptr
         *         第一个exception_ptr
         */
        RAINY_NODISCARD std::exception_ptr first_exception() const {
            return first_;
        }

    private:
        std::exception_ptr first_;
    };
}

namespace rainy::foundation::exceptions::runtime {
    /**
     * @brief Null pointer exception class.
     *        空指针异常类。
     */
    class nullpointer_exception : public runtime_error {
    public:
        using base = runtime_error;

        /**
         * @brief Constructs a nullpointer_exception.
         *        构造nullpointer_exception。
         *
         * @param loc The source location
         *            源位置
         */
        explicit nullpointer_exception(const source &loc = source::current()) : base("nullpointer detected", loc) {
        }
    };

    /**
     * @brief Throws a nullpointer_exception.
     *        抛出nullpointer_exception。
     *
     * @param loc The source location
     *            源位置
     */
    RAINY_INLINE rain_fn throw_nullpointer_exception(const utility::source_location &loc = utility::source_location::current())
        -> void {
        throw_exception(nullpointer_exception{loc});
    }
}

#endif
