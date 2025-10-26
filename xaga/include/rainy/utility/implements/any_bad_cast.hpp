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

    RAINY_INLINE void throw_bad_any_cast() {
        utility::throw_exception(bad_any_cast{});
    }
}

#endif