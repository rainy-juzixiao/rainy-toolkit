/*
* Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_CORE_COLLECTIONS_VIEWS_RANGE_CLOSURE_HPP
#define RAINY_CORE_COLLECTIONS_VIEWS_RANGE_CLOSURE_HPP
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/collections/views/implements/pipeline.hpp>

namespace rainy::core::collections::views::implements {
    template <typename Fx, typename... Types>
    class range_closure : public base<range_closure<Fx, Types...>> {
    public:
        static_assert((type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Types>, Types> && ...), "Types must be decayed");
        static_assert(type_traits::properties::is_empty_v<Fx> && type_traits::properties::is_default_constructible_v<Fx>,
                      "Fx must be empty and default constructible");

        template <typename... UTypes,
                  typename = type_traits::other_trans::enable_if_t<(type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UTypes>, Types> && ...)>>
        constexpr explicit range_closure(UTypes &&...args) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_constructible<Types, UTypes>...>) : captures(utility::forward<UTypes>(args)...) {
        }

        using indices = type_traits::helper::index_sequence_for<Types...>;

        // lvalue overload
        template <typename Ty, typename = decltype(utility::declval<Fx>()(utility::declval<Ty &&>(), utility::declval<Types &>()...))>
        constexpr auto operator()(Ty &&arg) & noexcept(noexcept(invoke_closure(*this, utility::forward<Ty>(arg), indices{}))) {
            return invoke_closure(*this, utility::forward<Ty>(arg), indices{});
        }

        // const lvalue
        template <typename Ty,
                  typename = decltype(utility::declval<Fx>()(utility::declval<Ty &&>(), utility::declval<const Types &>()...))>
        constexpr auto operator()(Ty &&arg) const & noexcept(noexcept(invoke_closure(*this, utility::forward<Ty>(arg), indices{}))) {
            return invoke_closure(*this, utility::forward<Ty>(arg), indices{});
        }

        // rvalue
        template <typename Ty, typename = decltype(utility::declval<Fx>()(utility::declval<Ty &&>(), utility::declval<Types &&>()...))>
        constexpr auto operator()(Ty &&arg) && noexcept(noexcept(invoke_closure(utility::move(*this), utility::forward<Ty>(arg),
                                                                                indices{}))) {
            return invoke_closure(utility::move(*this), utility::forward<Ty>(arg), indices{});
        }

        // const rvalue
        template <typename Ty,
                  typename = decltype(utility::declval<Fx>()(utility::declval<Ty &&>(), utility::declval<const Types &&>()...))>
        constexpr auto operator()(Ty &&arg) const && noexcept(noexcept(invoke_closure(utility::move(*this), utility::forward<Ty>(arg),
                                                                                      indices{}))) {
            return invoke_closure(utility::move(*this), utility::forward<Ty>(arg), indices{});
        }

    private:
        template <typename Self, typename Arg, std::size_t... Idx>
        static constexpr auto invoke_closure(Self &&self, Arg &&arg, type_traits::helper::index_sequence<Idx...>) noexcept(
            noexcept(Fx{}(utility::forward<Arg>(arg), utility::container::get<Idx>(utility::forward<Self>(self).captures)...))) {
            static_assert(type_traits::type_relations::is_same_v<type_traits::helper::index_sequence<Idx...>, indices>, "Index pack mismatch");
            return Fx{}(utility::forward<Arg>(arg), utility::container::get<Idx>(utility::forward<Self>(self).captures)...);
        }

        container::tuple<Types...> captures;
    };
}

#endif