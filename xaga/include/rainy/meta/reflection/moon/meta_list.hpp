#ifndef RAINY_META_REFLECTION_MOON_META_LIST_HPP
#define RAINY_META_REFLECTION_MOON_META_LIST_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/moon/element_list.hpp>

namespace rainy::meta::reflection::moon {
    template <typename Ty, bool Enable>
    struct meta : implements::named_value<Ty> {
        constexpr meta(std::string_view name, Ty value) : implements::named_value<Ty>{name, value} {
        }
    };

    template <typename Ty>
    struct meta<Ty, false> : implements::named_value<void> {
        constexpr meta() = default;
    };

    template <typename... As>
    struct meta_list : implements::element_list<As...> {
        constexpr meta_list(As... as) : implements::element_list<As...>{as...} { // NOLINT
        }
    };

    template <typename T>
    constexpr auto attach(std::string_view name, T value) {
        return meta<T,true>{name, value};
    }

    template <bool Cond,typename T>
    constexpr auto attach_if(std::string_view name, T value) {
        if constexpr (Cond) {
            return meta<T, true>{name, value};
        } else {
            return meta<T, false>{};
        }
    }

    template <typename... Metas>
    constexpr auto bind_meta(Metas &&...metas) {
        return meta_list<std::decay_t<Metas>...>{utility::forward<Metas>(metas)...};
    }

    template <bool Cond,typename... Metas>
    constexpr auto bind_meta_if(Metas &&...metas) {
        if constexpr (Cond) {
            return meta_list<std::decay_t<Metas>...>{utility::forward<Metas>(metas)...};
        } else {
            return meta_list<>{};
        }
    }
}

#endif