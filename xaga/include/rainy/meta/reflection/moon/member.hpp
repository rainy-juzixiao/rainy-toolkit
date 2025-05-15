#ifndef RAINY_META_REFLECTION_MOON_MEMBER_HPP
#define RAINY_META_REFLECTION_MOON_MEMBER_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/moon/element_list.hpp>
#include <rainy/meta/reflection/moon/meta_list.hpp>

namespace rainy::meta::reflection::moon {
    template <typename Ty, typename AList, bool Enable>
    struct member {
        constexpr member(std::string_view n, Ty v, AList metas = {}) : name{n}, value{v}, metas{metas} {
        }

        static constexpr bool has_value = true;
        static constexpr bool is_static = !type_traits::composite_types::is_member_pointer_v<Ty>;

        AList metas;
        Ty value;
        std::string_view name;
    };

    template <typename Ty, typename AList>
    struct member<Ty, AList, false> {
        constexpr member() = default;

        static constexpr bool has_value = false;
    };

    template <typename T>
    member(std::string_view, T) -> member<T, meta_list<>, true>;

    template <typename... Fs>
    struct property_list : implements::element_list<Fs...> {
        template <type_traits::other_trans::enable_if_t<(implements::is_valid_property<Fs>() && ...), int> = 0>
        constexpr property_list(Fs... fs) : implements::element_list<Fs...>{fs...} {
        }
    };

    template <typename... Fs>
    struct method_list : implements::element_list<Fs...> {
        template <type_traits::other_trans::enable_if_t<(implements::is_valid_function<Fs>() && ...), int> = 0>
        constexpr method_list(Fs... fs) : implements::element_list<Fs...>{fs...} {
        }
    };

    template <typename Ty, typename... Args>
    constexpr auto expose(std::string_view name, Ty value, meta_list<Args...> list = {}) {
        return member<Ty, meta_list<Args...>, true>{name, value, list};
    }

    template <bool Cond, typename Ty, typename... Args>
    constexpr auto expose_if(std::string_view name, Ty value, meta_list<Args...> list = {}) {
        if constexpr (Cond) {
            return member<Ty, meta_list<Args...>, Cond>{name, value, list};
        } else {
            return member<Ty, meta_list<>, false>{};
        }
    }
}

#endif