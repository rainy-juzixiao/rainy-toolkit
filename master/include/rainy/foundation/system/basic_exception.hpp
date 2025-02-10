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
#include <rainy/foundation/diagnostics/source_location.hpp>
#include <stdexcept>

namespace rainy::foundation::system::exceptions {
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
            exception_(std::move(except_instance)) {
        }

        explicit wrapexcept(const Except &except_instance) noexcept(std::is_nothrow_copy_constructible_v<Except>) :
            exception_(except_instance) {
        }

        RAINY_NODISCARD const char *what() const noexcept override {
            return exception_.what();
        }

    private:
        Except exception_;
    };

    template <typename Except>
    void throw_exception(const Except &exception) {
        static_assert(std::is_base_of_v<std::exception, Except>, "exception type must be derived from std::exception!");
#if __cpp_exceptions
        throw wrapexcept<Except>{exception}; // 阻止Clang-Tidy的误报
#else
        std::fwrite(exception.what(), sizeof(char), sizeof(char) * information::internals::string_length(exception.what()), stderr);
        std::terminate();
#endif
    }

    RAINY_INLINE exception stdexcept_to_rexcept(
        const std::exception &except, const diagnostics::source_location &location = diagnostics::source_location::current()) {
        if (!dynamic_cast<const exception *>(&except)) {
            // 不应该转换为rainy's toolkit的异常。因为我们要的是标准库异常
            // (dynamic_cast帮助我们识别except，如果认为这个except不是rainy's
            // toolkit的异常，那么就会返回nullptr。此处就是判断是否是nullptr)
            return exception{except.what(), location};
        }
        return exception{""};
    }

    namespace runtime {
        class runtime_error : public exception {
        public:
            using base = exception;

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

    namespace logic {
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

            explicit out_of_range(const char *message, const source &location = source::current()) : base(message, location) {
            }

            explicit out_of_range(const std::string &message, const source &location = source::current()) : base(message, location) {
            }
        };

        RAINY_INLINE void throw_out_of_range(const char *message,
                                             const diagnostics::source_location &location = diagnostics::source_location::current()) {
            throw_exception(out_of_range{message, location});
        }
    }

    namespace cast {
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
}


#if RAINY_HAS_CXX20
template <>
class std::formatter<rainy::foundation::system::exceptions::exception, char> {
public:
    explicit formatter() noexcept = default;

    static auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    static auto format(const rainy::foundation::system::exceptions::exception &value, std::format_context fc) noexcept {
        return std::format_to(fc.out(), "{}", value.what());
    }
};
#endif

namespace rainy::utility {
    using foundation::system::exceptions::stdexcept_to_rexcept;
    using foundation::system::exceptions::throw_exception;
}

namespace rainy::foundation::system::exceptions::runtime {
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

#endif