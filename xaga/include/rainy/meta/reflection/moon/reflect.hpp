#ifndef RAINY_META_REFLECTION_MOON_REFLECT_HPP
#define RAINY_META_REFLECTION_MOON_REFLECT_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/moon/element_list.hpp>
#include <rainy/meta/reflection/moon/oop_base.hpp>
#include <rainy/meta/reflection/moon/member.hpp>
#include <rainy/meta/reflection/moon/ctor.hpp>

namespace rainy::meta::reflection::moon::implements {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_properties = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_properties<Ty, type_traits::other_trans::void_t<decltype(utility::declval<moon::reflect<Ty>>().properties)>> = true;

    template <typename Ty>
    constexpr std::size_t eval_valid_properties_count() noexcept {
        if constexpr (has_properties<Ty>) {
            return reflect<Ty>::properties.accumulate(std::size_t{0}, [](auto acc, auto field) {
                if constexpr (field.has_value) {
                    return acc + 1;
                } else {
                    return acc;
                }
            });
        } else {
            return 0;
        }
    }

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_methods = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_methods<Ty, type_traits::other_trans::void_t<decltype(utility::declval<moon::reflect<Ty>>().methods)>> =
        true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_ctors = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_ctors<Ty, type_traits::other_trans::void_t<decltype(utility::declval<moon::reflect<Ty>>().ctors)>> = true;
}

namespace rainy::meta::reflection::moon {
    template <typename Ty, typename... Bases>
    struct reflect_base {
        using reflect_type = Ty;
        
        static constexpr std::string_view name = foundation::ctti::type_name<reflect_type>();
        static constexpr std::size_t size = sizeof(reflect_type);
        static constexpr std::size_t align = alignof(reflect_type);

        static constexpr std::size_t propertie_count = implements::eval_valid_properties_count<reflect_type>();

        static constexpr base_list<Bases...> bases{Bases{}...};

        template <typename Uty>
        static constexpr auto &&forward(Uty &&derived) {
            if constexpr (type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Uty>, Uty>) {
                return static_cast<reflect_type &&>(derived);
            } else if constexpr (type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Uty> &, Uty>) {
                return static_cast<reflect_type &>(derived);
            } else {
                return static_cast<const type_traits::other_trans::decay_t<Uty> &>(derived);
            }
        }

        static constexpr auto virtual_bases() {
            return bases.accumulate(implements::element_list<>{}, [](auto acc, auto base) {
                constexpr auto vbs = base.info.virtual_bases();
                auto concated = vbs.accumulate(acc, [](auto acc, const auto &vb) { return acc.insert(vb); });
                if constexpr (base.is_virtual)
                    return concated.insert(base.info);
                else
                    return concated;
            });
        }

        template <class R, class F>
        static constexpr auto DFS_Acc(R r, F &&f) {
            return implements::DFS_Acc<0>(
                reflect<reflect_type>{}, utility::forward<F>(f),
                virtual_bases().Accumulate(utility::forward<F>(f)(utility::move(r), reflect<reflect_type>{}, 0),
                                           [&](auto acc, auto vb) { return utility::forward<F>(f)(utility::move(acc), vb, 1); }));
        }
    };


}

#endif