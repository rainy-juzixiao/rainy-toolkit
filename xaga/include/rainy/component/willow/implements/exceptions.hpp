#ifndef RAINY_COMPONENT_WILLOW_IMPLEMENTS_EXCEPTIONS_HPP
#define RAINY_COMPONENT_WILLOW_IMPLEMENTS_EXCEPTIONS_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::exceptions::willow {
    class json_exception : public runtime::runtime_error {
    public:
        using base = runtime_error;

        json_exception(const char *message, const source &loc = source::current()) : base(message, loc) {
        }
    };

    class json_type_error final : public json_exception {
    public:
        using base = json_exception;

        json_type_error(const char *message, const source &loc = source::current()) : base(message, loc) {
        }
    };

    void throw_json_type_error(const char *message,
                               const foundation::diagnostics::source_location &loc = foundation::diagnostics::current_location()) {
        throw_exception(json_type_error(message, loc));
    }

    class json_invalid_key final : public json_exception {
    public:
        json_invalid_key(const char *message, const source &loc = source::current()) : json_exception(message, loc) {
        }
    };

    void throw_json_invalid_key(const char *message,
                                const foundation::diagnostics::source_location &loc = foundation::diagnostics::current_location()) {
        throw_exception(json_invalid_key(message, loc));
    }

    class json_invalid_iterator final : public json_exception {
    public:
        using base = json_exception;

        json_invalid_iterator(const char *message, const source &loc = source::current()) : base(message, loc) {
        }
    };

    void throw_json_invalid_iterator(
        const char *message, const foundation::diagnostics::source_location &loc = foundation::diagnostics::current_location()) {
        throw_exception(json_invalid_iterator(message, loc));
    }

    class json_parse_error final : public json_exception {
    public:
        using base = json_exception;

        json_parse_error(const char *message, const source &loc = source::current()) : base(message, loc) {
        }
    };

    void throw_json_parse_error(const char *message,
                                const foundation::diagnostics::source_location &loc = foundation::diagnostics::current_location()) {
        throw_exception(json_parse_error(message, loc));
    }

    class json_serialize_error final : public json_exception {
    public:
        using base = json_exception;

        json_serialize_error(const char *message, const source &loc = source::current()) : base(message, loc) {
        }
    };

    void throw_json_serialize_error(
        const char *message, const foundation::diagnostics::source_location &loc = foundation::diagnostics::current_location()) {
        throw_exception(json_serialize_error(message, loc));
    }
}


#endif