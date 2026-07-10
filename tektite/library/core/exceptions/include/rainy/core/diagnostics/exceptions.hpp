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
    class exception : public std::exception {
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
}

#define RAINY_DEFINE_EXCEPTION(class_name, base_class, default_message)                                                               \
    class class_name final : public base_class {                                                                                      \
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
    class class_name final : public base_class {                                                                                      \
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
    class class_name final : public base_class {                                                                                      \
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

        explicit logic_error(const char *message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_DEFINE_EXCEPTION_WITH_THROW(out_of_range, logic_error, "out_of_range", throw_out_of_range)
}

namespace rainy::core::exceptions::runtime {
    class runtime_error : public exception {
    public:
        using base = exception;

        explicit runtime_error(const char *message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_alloc, runtime_error, "bad allocation", throw_bad_alloc)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_cast, runtime_error, "bad cast", throw_bad_cast)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(overflow_error, runtime_error, "overflow error", throw_overflow_error)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(underflow_error, runtime_error, "underflow error", throw_underflow_error)
    RAINY_DEFINE_EXCEPTION_WITH_THROW(domain_error, runtime_error, "domain error", throw_domain_error)
}

#endif
