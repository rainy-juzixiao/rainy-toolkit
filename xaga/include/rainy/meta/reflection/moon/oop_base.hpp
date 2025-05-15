#ifndef RAINY_META_REFLECTION_MOON_OOP_BASE_HPP
#define RAINY_META_REFLECTION_MOON_OOP_BASE_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/moon/element_list.hpp>

namespace rainy::meta::reflection::moon {
    template <typename Ty, bool IsVirtual>
    struct expose_base {
        static constexpr auto info = reflect<Ty>{};
        static constexpr bool is_virtual = IsVirtual;
    };

    template <typename... Bs>
    struct base_list : implements::element_list<Bs...> {
        constexpr base_list(Bs... bs) : implements::element_list<Bs...>{bs...} {
        }
    };
}

#endif