#ifndef RAINY_META_REFLECTION_MOON_ELEMENT_LIST_HPP
#define RAINY_META_REFLECTION_MOON_ELEMENT_LIST_HPP
#include <tuple>
#include <rainy/core/core.hpp>

namespace rainy::meta::reflection::moon {
    template <typename Ty>
    struct reflect;
}

namespace rainy::meta::reflection::moon::implements {
    template <typename Type>
    struct extract_member_ptr_type {
        using type = Type;
    };

    template <template <typename, typename, bool> typename Template, typename Ty, typename AList, bool Enable>
    struct extract_member_ptr_type<Template<Ty, AList, Enable>> {
        using type = Ty;
    };

    template <typename Type>
    struct is_reflect : type_traits::helper::false_type {};

    template <typename Type>
    struct is_reflect<moon::reflect<Type>>
        : type_traits::helper::bool_constant<type_traits::type_properties::is_default_constructible_v<moon::reflect<Type>>> {};

    template <typename Fs>
    constexpr bool is_valid_property() noexcept {
        return type_traits::primary_types::is_member_object_pointer_v<typename extract_member_ptr_type<Fs>::type>;
    }

    template <typename Fs>
    constexpr bool is_valid_function() noexcept {
        return type_traits::primary_types::function_traits<typename extract_member_ptr_type<Fs>::type>::valid;
    }

    template <std::size_t D, class T, class R, class F>
    constexpr auto DFS_Acc(T t, F &&f, R r) {
        return t.bases.Accumulate(std::move(r), [&](auto r, auto b) {
            if constexpr (b.is_virtual)
                return DFS_Acc<D + 1>(b.info, std::forward<F>(f), std::move(r));
            else
                return DFS_Acc<D + 1>(b.info, std::forward<F>(f), std::forward<F>(f)(std::move(r), b.info, D + 1));
        });
    }

    template <typename TI, typename U, typename F>
    constexpr void NV_Var(TI, U &&u, F &&f) {
        TI::members.ForEach([&](auto &&k) {
            using K = std::decay_t<decltype(k)>;
            if constexpr (!K::is_static && !K::is_func)
                std::forward<F>(f)(k, std::forward<U>(u).*(k.value));
        });
        TI::bases.ForEach([&](auto b) {
            if constexpr (!b.is_virtual)
                NV_Var(b.info, b.info.Forward(std::forward<U>(u)), std::forward<F>(f));
        });
    }

    template <class L, class F, class R>
    constexpr auto acc_impl(const L &, F &&, R r, type_traits::helper::index_sequence<>) {
        return r;
    }

    template <class L, class F, class R, std::size_t N0, std::size_t... Ns>
    constexpr auto acc_impl(const L &l, F &&f, R r, type_traits::helper::index_sequence<N0, Ns...>) {
        return acc_impl(l, utility::forward<F>(f), f(utility::move(r), l.template at<N0>()), type_traits::helper::index_sequence<Ns...>{});
    }

    template <typename Ty>
    struct named_value {
        constexpr named_value(std::string_view name, Ty val) : name{name}, value{val} {
        }

        static constexpr bool has_value = true;

        template <typename Uty>
        constexpr bool equal_with(Uty &&val) {
            if constexpr (std::is_same_v<Ty, Uty>) {
                return value == val;
            } else {
                return false;
            }
        }

        template <typename Uty>
        constexpr bool operator==(Uty &&val) const {
            return equal_with(val);
        }

        Ty value;
        std::string_view name;
    };

    template <>
    struct named_value<void> {
        static constexpr bool has_value = false;
    };

    template <typename... Elem>
    class element_list {
    public:
        constexpr element_list(Elem... member) : elems{member...} {
        }

        template <typename Init, typename Func>
        constexpr auto accumulate(Init init, Func &&func) const {
            return implements::acc_impl(*this, utility::forward<Func>(func), utility::move(init), type_traits::helper::index_sequence_for<Elem...>{});
        }

        template <typename Action>
        constexpr void for_each(Action &&action) const {
            accumulate(0, [&](auto, const auto &field) {
                if constexpr (is_reflect<std::decay_t<decltype(field)>>::value) {
                    utility::forward<Action>(action)(field);
                } else {
                    if constexpr (field.has_value) {
                        utility::forward<Action>(action)(field);
                    }
                }
                return 0;
            });
        }

        template <typename InsertElem>
        constexpr auto push_back(InsertElem elem) const {
            return std::apply([&elem](auto &&...orgign_elem) { return element_list<Elem..., InsertElem>{orgign_elem..., elem}; },
                              elems);
        }

        template <typename InsertElem>
        constexpr auto push_front(InsertElem elem) {
            return std::apply([&elem](auto &&...orgin_elem) { return element_list<InsertElem, Elem...>{elem, orgin_elem...}; }, elems);
        }

        template <typename InsertElem>
        constexpr auto insert(InsertElem elem) const {
            if constexpr ((std::is_same_v<Elem, InsertElem> || ...)) {
                return *this;
            } else {
                return push_back(elem);
            }
        }

        template <std::size_t Idx>
        constexpr auto &at() const {
            return std::get<Idx>(elems);
        }

        template <typename Action>
        constexpr std::size_t find_if(Action &&action) const {
            return find_if_impl(std::forward<Action>(action), std::index_sequence_for<Elem...>{});
        }

        constexpr static std::size_t size() {
            return sizeof...(Elem);
        }

    private:
        template <typename Action, std::size_t... I>
        constexpr std::size_t find_if_impl(Action &&action, type_traits::helper::index_sequence<I...>) const {
            std::size_t result = sizeof...(Elem);
            ((action(at<I>()) ? result = I : -1), ...);
            return result;
        }

        std::tuple<Elem...> elems;
    };
}

#endif