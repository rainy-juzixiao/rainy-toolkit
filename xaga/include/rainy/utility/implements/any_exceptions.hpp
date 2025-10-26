#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_BAD_CAST_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_BAD_CAST_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::exceptions::cast {
    class bad_any_cast final : public bad_cast {
    public:
        using base = bad_cast;

        explicit bad_any_cast(const source &location = source::current()) : base(location.to_string() + " : Bad any_cast : ") {
        }
    };

    RAINY_INLINE rain_fn throw_bad_any_cast(const diagnostics::source_location &location = diagnostics::source_location::current())
        -> void {
        utility::throw_exception(bad_any_cast{location});
    }
}

namespace rainy::foundation::exceptions::logic {
    class any_not_implemented final : public not_implemented {
    public:
        using base = not_implemented;

        explicit any_not_implemented(const char *message, const source &location = source::current()) : base(message, location) {
        }
    };

    RAINY_INLINE rain_fn throw_any_not_implemented(
        const char *message, const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw_exception(any_not_implemented{message, location});
    }
}

#endif