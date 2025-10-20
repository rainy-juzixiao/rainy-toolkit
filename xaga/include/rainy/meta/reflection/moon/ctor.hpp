#ifndef RAINY_META_REFLECTION_MOON_CTOR_HPP
#define RAINY_META_REFLECTION_MOON_CTOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/moon/element_list.hpp>
#include <rainy/meta/reflection/moon/meta_list.hpp>

namespace rainy::meta::reflection::moon::implements {
    template <typename Traits, typename TypeList = typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
    struct extract_to_ctor_fn {};

    template <typename Traits, typename... Args>
    struct extract_to_ctor_fn<Traits, type_traits::other_trans::type_list<Args...>> {
        using class_type = typename Traits::return_type;

        static constexpr auto fn = rainy::utility::get_ctor_fn<class_type, Args...>();
    };
}

namespace rainy::meta::reflection::moon {
    template <typename Ty, typename AList, bool Enable>
    struct ctor {
        constexpr ctor(Ty v, AList metas = {}) : metas{metas}, value{v} {
        }

        static constexpr bool has_value = true;

        AList metas;
        Ty value;
    };

    template <typename Ty, typename AList>
    struct ctor<Ty, AList, false> {
        constexpr ctor() = default;

        static constexpr bool has_value = false;
    };

    template <typename Ty>
    ctor(Ty) -> ctor<Ty, meta_list<>, true>;

    template <typename... Fs>
    struct ctor_list : implements::element_list<Fs...> {
        constexpr ctor_list(Fs... fs) : implements::element_list<Fs...>{fs...} {
        }
    };

    template <typename Ty, typename... Args>
    constexpr auto expose_ctor(Ty ctor_fn, meta_list<Args...> list = {}) {
        return ctor<Ty, meta_list<Args...>, true>{ctor_fn, list};
    }

    template <typename Fx, typename... Metas, type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid,int> = 0>
    constexpr auto expose_ctor(meta_list<Metas...> list = {}) {
        constexpr auto fn = implements::extract_to_ctor_fn<type_traits::primary_types::function_traits<Fx>>::fn;
        return ctor<Fx*, meta_list<Metas...>, true>{fn, list};
    }

    template <bool Cond, typename Ty, typename... Args>
    constexpr auto expose_ctor_if(Ty ctor_fn, meta_list<Args...> list = {}) {
        if constexpr (Cond) {
            return ctor<Ty, meta_list<Args...>, Cond>{ctor_fn, list};
        } else {
            return ctor<Ty, meta_list<>, false>{};
        }
    }

    template <bool Cond,typename Fx, typename... Metas,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid, int> = 0>
    constexpr auto expose_ctor_if(meta_list<Metas...> list = {}) {
        if constexpr (Cond) {
            constexpr auto fn = implements::extract_to_ctor_fn<type_traits::primary_types::function_traits<Fx>>::fn;
            return ctor<Fx*, meta_list<Metas...>, true>{fn, list};
        } else {
            return ctor<Fx*, meta_list<Metas...>, false>{};
        }
    }
}

#endif