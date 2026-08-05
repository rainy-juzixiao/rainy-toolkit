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
#include <rainy/core/type_traits.hpp>

// NOLINTEND

namespace rainy::core::exceptions {
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
    class RAINY_TOOLKIT_API exception : public std::exception {
    public:
        using base = std::exception;
        using source = diagnostics::source_location;

        explicit exception(const char *message, const source &location = source::current());
        RAINY_NODISCARD const char *what() const noexcept override;
        exception(const exception &other);
        exception(exception &&other) noexcept;
        exception &operator=(const exception &other);
        exception &operator=(exception &&other) noexcept;
        ~exception() override;

    protected:
        exception();
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

#define RAINY_DEFINE_EXCEPTION(class_name, base_class, default_message)                                                               \
    class class_name : public base_class {                                                                                            \
    public:                                                                                                                           \
        using base = base_class;                                                                                                      \
        explicit class_name(const char *message = default_message, const source &location = source::current()) :                      \
            base(message, location) {                                                                                                 \
        }                                                                                                                             \
    }


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
    class logic_error : public exception {
    public:
        using base = exception;

        logic_error() = default;

        explicit logic_error(const char *message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_DEFINE_EXCEPTION_WITH_THROW(out_of_range, logic_error, "out_of_range", throw_out_of_range)
}

namespace rainy::core::exceptions::runtime {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(runtime_error, exception, "runtime error", throw_runtime_error)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_alloc, runtime_error, "bad allocation", throw_bad_alloc)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_cast, runtime_error, "bad cast", throw_bad_cast)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(overflow_error, runtime_error, "overflow error", throw_overflow_error)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(underflow_error, runtime_error, "underflow error", throw_underflow_error)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(domain_error, runtime_error, "domain error", throw_domain_error)
}

#endif
