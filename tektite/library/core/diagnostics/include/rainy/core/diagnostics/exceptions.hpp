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
#include <exception>
#include <new>
#include <string>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/diagnostics/source_location.hpp>

namespace rainy::core::exceptions {
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
    constexpr void throw_exception(const Except &exception) {
        static_assert(type_traits::type_relations::is_base_of_v<std::exception, Except>,
                      "exception type must be derived from std::exception!");
#if __cpp_exceptions
        throw exception;
#else
        std::fwrite(exception.what(), sizeof(char), strlen(exception.what()), stderr);
        std::terminate();
#endif
    }
}

namespace rainy::core::exceptions::logic {
    class logic_error : public exception {
    public:
        using base = exception;

        explicit logic_error(const char *message, const source &location = source::current()) : base(message, location) {
        }

        explicit logic_error(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    class out_of_range final : public logic_error {
    public:
        using base = logic_error;

        explicit out_of_range(const char *message = "out_of_range", const source &location = source::current()) :
            base(message, location) {
        }

        explicit out_of_range(const std::string &message = "out_of_range", const source &location = source::current()) :
            base(message, location) {
        }
    };

    RAINY_INLINE rain_fn throw_out_of_range(const char *message = "out_of_range",
                                            const utility::source_location &location = utility::source_location::current())
        -> void {
        throw_exception(out_of_range{message, location});
    }
}

namespace rainy::core::exceptions::runtime {
    class runtime_error : public exception {
    public:
        using base = exception;

        explicit runtime_error(const char *message, const source &location = source::current()) : base(message, location) {
        }

        explicit runtime_error(const std::string &message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_INLINE rain_fn throw_bad_alloc(const utility::source_location &location = utility::source_location::current())
        -> void {
        throw_exception(runtime_error{"bad allocation", location});
    }
}

#endif
