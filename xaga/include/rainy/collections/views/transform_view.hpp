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
#ifndef RAINY_COLLECTIONS_VIEWS_TRANSFORM_VIEW_HPP
#define RAINY_COLLECTIONS_VIEWS_TRANSFORM_VIEW_HPP
#include <rainy/core/core.hpp>
#include <rainy/collections/views/implements/invoker_view.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif

namespace rainy::collections::views::implements {
    template <typename MainContext, typename ViewOrContainer, typename Fx, bool Const>
    class transform_iterator {
    public:
        using parent_t = type_traits::other_trans::maybe_const_t<Const, MainContext>;
        using base_t = type_traits::other_trans::maybe_const_t<Const, ViewOrContainer>;

        using value_type = type_traits::cv_modify::remove_cvref_t<type_traits::type_properties::invoke_result_t<
            type_traits::other_trans::maybe_const_t<Const, Fx> &, type_traits::extras::ranges::range_reference_t<base_t>>>;
        using difference_type = type_traits::extras::ranges::range_difference_t<base_t>;
        using iterator_t = type_traits::extras::iterators::iterator_t<base_t>;

        template <
            type_traits::other_trans::enable_if_t<
                type_traits::type_properties::is_default_constructible_v<type_traits::extras::iterators::iterator_t<base_t>>, int> = 0>
        transform_iterator() {
        }

        constexpr transform_iterator(
            parent_t &parent_,
            iterator_t current_) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<iterator_t>) // strengthened
            : current{utility::move(current_)}, parent{utility::addressof(parent_)} {
#if RAINY_ENABLE_DEBUG
            utility::implements::adl_verify_range(current, utility::end(parent_.range));
            if constexpr (type_traits::extras::iterators::is_forward_iterator_v<iterator_t>) {
                utility::implements::adl_verify_range(utility::begin(parent_.range), current);
            }
#endif
        }

        constexpr transform_iterator(const transform_iterator &) = default;

        RAINY_NODISCARD constexpr decltype(auto) base() const & noexcept {
            return (current);
        }

        RAINY_NODISCARD constexpr decltype(auto) operator*() const noexcept(noexcept(utility::invoke(*parent->func, *current))) {
#if RAINY_ENABLE_DEBUG
            check_for_dereference();
            core::implements::stl_internal_check(parent->func &&
                                                 "Cannot dereference iterator into transform_view with no transformation function");
#endif
            return utility::invoke(*parent->func, *current);
        }

        constexpr transform_iterator &operator++() noexcept(noexcept(++current)) {
#if RAINY_ENABLE_DEBUG
            core::implements::stl_internal_check(parent != nullptr && "Cannot increment value-initialized transform_view iterator");
            core::implements::stl_internal_check(current != utility::end(parent->range) &&
                                                 "Cannot increment transform_view iterator past end");
#endif
            ++current;
            return *this;
        }

        constexpr decltype(auto) operator++(int) noexcept(
            noexcept(++current) && (!type_traits::extras::iterators::is_forward_iterator_v<iterator_t> ||
                                    type_traits::type_properties::is_nothrow_copy_constructible_v<iterator_t>) ) {
            if constexpr (type_traits::extras::iterators::is_forward_iterator_v<iterator_t>) {
                auto tmp = *this;
                ++*this;
                return tmp;
            } else {
                ++*this;
            }
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_bidirectional_iterator_v<iterator_t>, int> = 0>
        constexpr transform_iterator &operator--() noexcept(noexcept(--current)) {
#if RAINY_ENABLE_DEBUG
            core::implements::stl_internal_check(parent != nullptr && "Cannot decrement value-initialized transform_view iterator");
            using iterator_t = type_traits::extras::iterators::iterator_t<ViewOrContainer>;
            if constexpr (type_traits::extras::iterators::is_forward_iterator_v<iterator_t>) {
                core::implements::stl_internal_check(current != utility::begin(parent->range) &&
                                                     "Cannot decrement transform_view iterator before begin");
            }
#endif
            --current;
            return *this;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_bidirectional_iterator_v<iterator_t>, int> = 0>
        constexpr transform_iterator operator--(int) noexcept(noexcept(--current) &&
                                                         type_traits::type_properties::is_nothrow_copy_constructible_v<iterator_t>) {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        constexpr transform_iterator &operator+=(const difference_type off) noexcept(noexcept(current += off)) {
            current += off;
            return *this;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        constexpr transform_iterator &operator-=(const difference_type off) noexcept(noexcept(current -= off)) {
            core::implements::stl_internal_check(off != utility::min_possible_v<difference_type>, "integer overflow");
            current -= off;
            return *this;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD constexpr decltype(auto) operator[](const difference_type idx) const
            noexcept(noexcept(utility::invoke(*parent->func, current[idx]))) {
            return utility::invoke(*parent->func, current[idx]);
        }

        template <type_traits::other_trans::enable_if_t<type_traits::extras::meta_method::has_operator_eq_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator==(const transform_iterator &left,
                                                         const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current == right.current;
        }

        template <type_traits::other_trans::enable_if_t<type_traits::extras::meta_method::has_operator_eq_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator!=(const transform_iterator &left,
                                                         const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current != right.current;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator<(const transform_iterator &left,
                                                        const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current < right.current;
        }

        RAINY_NODISCARD friend constexpr bool operator>(const transform_iterator &left,
                                                        const transform_iterator &right) {
            return right < left;
        }

        RAINY_NODISCARD friend constexpr bool operator<=(const transform_iterator &left,
                                                         const transform_iterator &right) {
            return !(right < left);
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator>=(const transform_iterator &left,
                                                         const transform_iterator &right) {
            return !(left < right);
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr transform_iterator operator+(transform_iterator iter,
                                                                 const difference_type off) {
            iter.current += off;
            return iter;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr transform_iterator operator+(const difference_type off,
                                                                 transform_iterator iter) {
            iter.current += off;
            return iter;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr transform_iterator operator-(transform_iterator iter,
                                                                 const difference_type off) {
#if RAINY_ENABLE_DEBUG
            core::implements::stl_internal_check(off != utility::min_possible_v<difference_type>, "integer overflow");
#endif
            iter.current -= off;
            return iter;
        }

        RAINY_NODISCARD friend constexpr difference_type operator-(const transform_iterator &left,
                                                                   const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current - right.current;
        }

    private:
        type_traits::extras::iterators::iterator_t<base_t> current{};
        parent_t *parent{};

#if RAINY_ENABLE_DEBUG
        constexpr void check_for_dereference() const noexcept {
            core::implements::stl_internal_check(parent != nullptr && "cannot dereference value-initialized transform_view iterator");
            core::implements::stl_internal_check(current != utility::end(parent->range) &&
                                                 "cannot dereference end transform_view iterator");
        }
#endif

#if RAINY_ENABLE_DEBUG
        constexpr void same_range(const transform_iterator &right) const noexcept {
            core::implements::stl_internal_check(parent == right.parent && "cannot compare incompatible transform_view iterators");
        }
#endif
    };
}

namespace rainy::collections::views {
    template <typename ViewOrContainer, typename Fx>
    class transform_view : public implements::invoker_view<transform_view<ViewOrContainer, Fx>, ViewOrContainer, Fx> {
    public:
        using iterator = implements::transform_iterator<transform_view, ViewOrContainer, Fx, false>;
        using const_iterator = implements::transform_iterator<transform_view, ViewOrContainer, Fx, true>;
        using view_or_container = ViewOrContainer;
        using fx_type = Fx;

        using base = implements::invoker_view<transform_view<ViewOrContainer, Fx>, ViewOrContainer, Fx>;

        transform_view() = default;

        constexpr explicit transform_view(ViewOrContainer &range_, Fx func) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<ViewOrContainer> &&
            type_traits::type_properties::is_nothrow_move_constructible_v<Fx>) :
            base(utility::move(range_), utility::forward<Fx>(func)) {
        }

        constexpr explicit transform_view(ViewOrContainer &&range_, Fx func) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<ViewOrContainer> &&
            type_traits::type_properties::is_nothrow_move_constructible_v<Fx>) :
            base(utility::move(range_), utility::forward<Fx>(func)) {
        }
    };

    struct transform_fn {
        template <typename Range, typename Fx>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, Fx func) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range), utility::move(func)))) {
            return transform_view(utility::forward<Range>(range), utility::move(func));
        }

        template <typename Fx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<type_traits::other_trans::decay_t<Fx>, Fx>, int> = 0>
        RAINY_NODISCARD constexpr auto operator()(Fx &&func) const
            noexcept(type_traits::type_properties::is_nothrow_constructible_v<type_traits::other_trans::decay_t<Fx>, Fx>) {
            return implements::range_closure<transform_fn, type_traits::other_trans::decay_t<Fx>>{utility::forward<Fx>(func)};
        }
    };

    inline constexpr transform_fn transform;
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif