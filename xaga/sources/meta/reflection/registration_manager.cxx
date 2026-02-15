#include <rainy/meta/reflection/refl_impl/type_register.hpp>

namespace rainy::meta::reflection {
    registration_manager &get_registration_manager() noexcept { // NOLINT
        static registration_manager obj;
        return obj;
    }
}
