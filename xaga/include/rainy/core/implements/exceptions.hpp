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
#include <rainy/core/implements/source_location.hpp>
#include <stdexcept>

namespace rainy::foundation::exceptions {
    using exception_handler_t = void(*)();

    RAINY_TOOLKIT_API exception_handler_t global_exception_handler(exception_handler_t new_handler = nullptr) noexcept;
    RAINY_TOOLKIT_API exception_handler_t current_thread_exception_handler(exception_handler_t new_handler = nullptr) noexcept;
}

namespace rainy::foundation::exceptions::implements {
    RAINY_TOOLKIT_API void invoke_exception_handler() noexcept;

    template <typename Except, bool NoExceptionHandlerInvoke = false>
    void report_error(
        const type_traits::other_trans::conditional_t<type_traits::type_relations::is_void_v<Except>, void *, Except> &exception) {
        if constexpr (type_traits::type_relations::is_void_v<Except>) {
            constexpr const char message[] = "Detected error and the program cause a fatal error!";
            std::fwrite(message, sizeof(char), sizeof(char) * core::builtin::string_length(message), stderr);
        } else {
            std::fwrite(exception.what(), sizeof(char), sizeof(char) * core::builtin::string_length(exception.what()), stderr);
        }
        if constexpr (!NoExceptionHandlerInvoke) {
            invoke_exception_handler();
        }
    }
}

namespace rainy::foundation::exceptions {
    class exception : public std::exception {
    public:
        using base = std::exception;
        using source = diagnostics::source_location;

        explicit exception(const char *message, const source &location = source::current()) :
            message((location.to_string() + " : " + message)) {
        }

        explicit exception(const std::string &message, const source &location = source::current()) :
            message((location.to_string() + " : " + message)) {
        }

        RAINY_NODISCARD const char *what() const noexcept override {
            return message.c_str();
        }

    private:
        std::string message;
    };

    template <typename Except>
    class wrapexcept final : public std::exception {
    public:
        // 使用完美转发避免不必要的拷贝
        explicit wrapexcept(Except &&except_instance) noexcept(std::is_nothrow_move_constructible_v<Except>) :
            exception_(utility::move(except_instance)) {
        }

        explicit wrapexcept(const Except &except_instance) noexcept(std::is_nothrow_copy_constructible_v<Except>) :
            exception_(except_instance) {
        }

        RAINY_NODISCARD const char *what() const noexcept override {
            return exception_.what();
        }

        Except &get_exception() noexcept {
            return exception_;
        }

    private:
        Except exception_;
    };

    inline rain_fn uncaught_exceptions() noexcept -> int {
#if __cpp_exceptions
        return std::uncaught_exceptions();
#else
        return 0;
#endif
    }

    enum class exception_semantic {
        enforce, // 强制检查，如果失败，强制抛出异常
        observe, // 观察，仅输出错误信息报告并继续执行
        quick_enforce, // 强制检查，如果失败，强制退出
        assertion, // 强制断言，如果失败，输出信息后，强制退出程序
        ignored_in_release // 在release阶段忽略异常
    };

    template <typename Except>
    struct with_this_exception_t {
        explicit with_this_exception_t() = default;
    };

    template <typename Except>
    constexpr with_this_exception_t<Except> with_this_exception{};

    template <typename Except>
    void throw_exception(const Except &exception) {
        static_assert(type_traits::type_relations::is_base_of_v<std::exception, Except>,
                      "exception type must be derived from std::exception!");
#if __cpp_exceptions
        throw wrapexcept<Except>{exception}.get_exception(); // 阻止Clang-Tidy的误报
#else
        std::fwrite(exception.what(), sizeof(char), sizeof(char) * core::implements::string_length(exception.what()), stderr);
        std::terminate();
#endif
    }

    template <typename Except, exception_semantic Semantic = exception_semantic::enforce, typename... Args>
    void throw_exception_if(bool cond, Args &&...args) {
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
                                implements::report_error(exception);
                            } else {
                                throw wrapexcept<Except>{utility::move(exception)}.get_exception();
                            }
                        } else {
                            type_traits::other_trans::decay_t<Except> exception(utility::forward<Args>(args)...);
                            implements::report_error(exception);
                        }
                    }
                }
            }
        }
    }

    template <exception_semantic Semantic = exception_semantic::enforce, typename Except, typename... Args>
    RAINY_INLINE void throw_exception_if(with_this_exception_t<Except>, bool cond, Args &&...args) {
        throw_exception_if<Except, Semantic>(cond, utility::forward<Args>(args)...);
    }

    RAINY_INLINE exception stdexcept_to_rexcept(
        const std::exception &except, const diagnostics::source_location &location = diagnostics::source_location::current()) {
        if (!dynamic_cast<const exception *>(&except)) {
            return exception{except.what(), location};
        }
        return exception{""};
    }
}

namespace rainy::foundation::exceptions::runtime {
    class runtime_error : public exception {
    public:
        using base = exception;

        explicit runtime_error(const source &location = source::current()) noexcept : base("I Got a runtime_error!", location) {
        }

        explicit runtime_error(const char *message, const source &location = source::current()) : base(message, location) {
        }

        explicit runtime_error(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_INLINE void throw_runtime_error(const char *message,
                                          const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(runtime_error{message, location});
    }
}

namespace rainy::foundation::exceptions::logic {
    class logic_error : public exception {
    public:
        using base = exception;

        explicit logic_error(const char *message, const source &location = source::current()) : base(message, location) {
        }

        explicit logic_error(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_INLINE void throw_logic_error(const char *message,
                                        const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(logic_error{message, location});
    }

    class out_of_range final : public logic_error {
    public:
        using base = logic_error;

        explicit out_of_range(const char *message = "out_of_range", const source &location = source::current()) : base(message, location) {
        }

        explicit out_of_range(const std::string &message = "out_of_range", const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_INLINE void throw_out_of_range(const char *message = "out_of_range",
                                         const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(out_of_range{message, location});
    }

    class not_implemented final : public logic_error {
    public:
        using base = logic_error;

        explicit not_implemented(const char *message = "not_implemented", const source &location = source::current()) :
            base(message, location) {
        }

        explicit not_implemented(const std::string &message = "not_implemented", const source &location = source::current()) :
            base(message, location) {
        }
    };

    RAINY_INLINE void throw_not_implemented(const char *message = "not_implemented",
                                         const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(not_implemented{message, location});
    }
}

namespace rainy::foundation::exceptions::cast {
    class bad_cast : public exception {
    public:
        using base = exception;

        explicit bad_cast(const source &location = source::current()) : base("bad cast", location) {
        }

        explicit bad_cast(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_INLINE void throw_bad_cast(const char *message,
                                     const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(bad_cast{message, location});
    }
}

#if RAINY_HAS_CXX20
template <>
class std::formatter<rainy::foundation::exceptions::exception, char> {
public:
    explicit formatter() noexcept = default;

    static auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    static auto format(const rainy::foundation::exceptions::exception &value, std::format_context fc) noexcept {
        return std::format_to(fc.out(), "{}", value.what());
    }
};
#endif

namespace rainy::utility {
    using foundation::exceptions::stdexcept_to_rexcept;
    using foundation::exceptions::throw_exception;
    using foundation::exceptions::throw_exception_if;
    using foundation::exceptions::exception_semantic;
    using foundation::exceptions::with_this_exception_t;
    using foundation::exceptions::with_this_exception;
}

namespace rainy::foundation::exceptions::runtime {
    class bad_alloc final : public runtime_error {
    public:
        using base = runtime_error;

        explicit bad_alloc(const source &location = source::current()) : base("bad allocation", location) {
        }
    };

    RAINY_INLINE void throw_bad_alloc() {
        throw_exception(bad_alloc{});
    }
}

namespace rainy::foundation::exceptions {
    class multiple_exceptions : public exception {
    public:
        multiple_exceptions(std::exception_ptr first) noexcept;

        const char *what() const noexcept override {
            return "multiple exceptions";
        }

        std::exception_ptr first_exception() const {
            return first_;
        }

    private:
        std::exception_ptr first_;
    };
}

#endif