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
#ifndef RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_INVOKER_VIEW_HPP
#define RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_INVOKER_VIEW_HPP
#include <rainy/collections/views/views_interface.hpp>
#include <rainy/foundation/container/movable_box.hpp>

namespace rainy::collections::views::implements {
    template <typename Context, typename ViewOrContainer, typename Fx>
    class invoker_view : public view_interface<invoker_view<Context, ViewOrContainer, Fx>> {
    public:
        using reference = type_traits::extras::ranges::range_reference_t<ViewOrContainer>;
        using const_reference = type_traits::extras::ranges::range_const_reference_t<ViewOrContainer>;
        using difference_type = type_traits::extras::ranges::range_difference_t<ViewOrContainer>;
        using value_type = type_traits::extras::ranges::range_value_type_t<ViewOrContainer>;

        invoker_view() = default;

        constexpr explicit invoker_view(ViewOrContainer range_, Fx func) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<ViewOrContainer> &&
            type_traits::type_properties::is_nothrow_move_constructible_v<Fx>) :
            range(utility::move(range_)), func{std::in_place, utility::move(func)} {
        }

        RAINY_NODISCARD constexpr ViewOrContainer &base() const & noexcept {
            return range;
        }

        RAINY_NODISCARD constexpr ViewOrContainer base() && noexcept {
            return utility::move(range);
        }

        RAINY_NODISCARD constexpr auto begin() noexcept(noexcept(utility::begin(range)) &&
                                                        type_traits::type_properties::is_nothrow_move_constructible_v<
                                                            type_traits::extras::iterators::iterator_t<ViewOrContainer>>) {
            return typename Context::iterator{static_cast<Context &>(*this), utility::begin(range)};
        }

        template <type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_v<
                          const Fx &, type_traits::extras::ranges::range_reference_t<const ViewOrContainer>>,
                      int> = 0>
        RAINY_NODISCARD constexpr auto begin() const
            noexcept(noexcept(utility::begin(range)) && type_traits::type_properties::is_nothrow_move_constructible_v<
                                                            type_traits::extras::iterators::iterator_t<ViewOrContainer>>) {
            return typename Context::const_iterator{static_cast<const Context &>(*this), utility::begin(range)};
        }

        RAINY_NODISCARD constexpr auto end() noexcept(
            noexcept(utility::end(range)) &&
            type_traits::type_properties::is_nothrow_move_constructible_v<decltype(utility::end(range))>) {
            return typename Context::iterator{static_cast<Context &>(*this), utility::end(range)};
        }

        template <type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_v<
                          const Fx &, type_traits::extras::ranges::range_reference_t<const ViewOrContainer>>,
                      int> = 0>
        RAINY_NODISCARD constexpr auto end() const
            noexcept(noexcept(utility::end(range)) &&
                     type_traits::type_properties::is_nothrow_move_constructible_v<decltype(utility::end(range))>) {
            return typename Context::const_iterator{static_cast<const Context &>(*this), utility::end(range)};
        }

        RAINY_NODISCARD constexpr auto size() noexcept(noexcept(utility::size(range))) {
            return utility::size(range);
        }

        RAINY_NODISCARD constexpr auto size() const noexcept(noexcept(utility::size(range))) {
            return utility::size(range);
        }

        ViewOrContainer range{};
        foundation::container::movable_box<Fx> func{};
    };
}

#endif