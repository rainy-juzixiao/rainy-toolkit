/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_RANGE_CLOSURE_HPP
#define RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_RANGE_CLOSURE_HPP
#include <rainy/core/core.hpp>
#include <rainy/core/implements/views/pipeline.hpp>

namespace rainy::collections::views::implements {
    template <typename Fx, typename... Types>
    class range_closure : public base<range_closure<Fx, Types...>> {
    public:
        static_assert((std::is_same<typename std::decay<Types>::type, Types>::value && ...), "Types must be decayed");
        static_assert(std::is_empty<Fx>::value && std::is_default_constructible<Fx>::value,
                      "Fx must be empty and default constructible");

        template <typename... UTypes,
                  typename = std::enable_if_t<(std::is_same<typename std::decay<UTypes>::type, Types>::value && ...)>>
        constexpr explicit range_closure(UTypes &&...args) noexcept(
            std::conjunction<std::is_nothrow_constructible<Types, UTypes>...>::value) : captures(utility::forward<UTypes>(args)...) {
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
            noexcept(Fx{}(utility::forward<Arg>(arg), std::get<Idx>(utility::forward<Self>(self).captures)...))) {
            static_assert(std::is_same<type_traits::helper::index_sequence<Idx...>, indices>::value, "Index pack mismatch");
            return Fx{}(utility::forward<Arg>(arg), std::get<Idx>(utility::forward<Self>(self).captures)...);
        }

        std::tuple<Types...> captures;
    };
}

#endif