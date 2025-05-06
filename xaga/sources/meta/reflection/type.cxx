#include <rainy/meta/reflection/type.hpp>

namespace rainy::meta::reflection {
    enumeration *type::as_enumeration() noexcept {
        return is_type(basic_type::enumeration) ? static_cast<enumeration *>(this) : nullptr;
    }

    const enumeration *type::as_enumeration() const noexcept {
        return is_type(basic_type::enumeration) ? static_cast<const enumeration *>(this) : nullptr;
    }

    union_t *type::as_union() noexcept {
        return is_type(basic_type::union_t) ? static_cast<union_t *>(this) : nullptr;
    }

    const union_t *type::as_union() const noexcept {
        return is_type(basic_type::union_t) ? static_cast<const union_t *>(this) : nullptr;
    }
}
