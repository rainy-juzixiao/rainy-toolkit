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
#ifndef RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_TRANSFORM_HPP
#define RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_TRANSFORM_HPP

#include <rainy/core/collections/views/implements/invoker_view.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif

namespace rainy::core::collections::views::implements {
    template <typename MainContext, typename ViewOrContainer, typename Fx, bool Const>
    class transform_iterator {
    public:
        using parent_t = type_traits::other_trans::maybe_const_t<Const, MainContext>;
        using base_t = type_traits::other_trans::maybe_const_t<Const, ViewOrContainer>;

        using value_type = type_traits::modifers::remove_cvref_t<type_traits::properties::invoke_result_t<
            type_traits::other_trans::maybe_const_t<Const, Fx> &, type_traits::extras::ranges::range_reference_t<base_t>>>;
        using difference_type = type_traits::extras::ranges::range_difference_t<base_t>;
        using iterator_t = type_traits::extras::iterators::iterator_t<base_t>;

        template <
            type_traits::other_trans::enable_if_t<
                type_traits::properties::is_default_constructible_v<type_traits::extras::iterators::iterator_t<base_t>>, int> = 0>
        transform_iterator() {
        }

        constexpr transform_iterator(
            parent_t &parent_,
            iterator_t current_) noexcept(type_traits::properties::is_nothrow_move_constructible_v<iterator_t>) // strengthened
            : current{utility::move(current_)}, parent{utility::addressof(parent_)} {
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

        constexpr decltype(auto) operator++(int) noexcept(noexcept(++current) &&
                                                          (!type_traits::extras::iterators::is_forward_iterator_v<iterator_t> ||
                                                           type_traits::properties::is_nothrow_copy_constructible_v<iterator_t>) ) {
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
                                                              type_traits::properties::is_nothrow_copy_constructible_v<iterator_t>) {
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
        RAINY_NODISCARD friend constexpr bool operator==(const transform_iterator &left, const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current == right.current;
        }

        template <type_traits::other_trans::enable_if_t<type_traits::extras::meta_method::has_operator_eq_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator!=(const transform_iterator &left, const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current != right.current;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator<(const transform_iterator &left, const transform_iterator &right) {
#if RAINY_ENABLE_DEBUG
            left.same_range(right);
#endif
            return left.current < right.current;
        }

        RAINY_NODISCARD friend constexpr bool operator>(const transform_iterator &left, const transform_iterator &right) {
            return right < left;
        }

        RAINY_NODISCARD friend constexpr bool operator<=(const transform_iterator &left, const transform_iterator &right) {
            return !(right < left);
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr bool operator>=(const transform_iterator &left, const transform_iterator &right) {
            return !(left < right);
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr transform_iterator operator+(transform_iterator iter, const difference_type off) {
            iter.current += off;
            return iter;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr transform_iterator operator+(const difference_type off, transform_iterator iter) {
            iter.current += off;
            return iter;
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_random_access_iterator_v<iterator_t>, int> = 0>
        RAINY_NODISCARD friend constexpr transform_iterator operator-(transform_iterator iter, const difference_type off) {
#if RAINY_ENABLE_DEBUG
            core::implements::stl_internal_check(off != utility::min_possible_v<difference_type>, "integer overflow");
#endif
            iter.current -= off;
            return iter;
        }

        RAINY_NODISCARD friend constexpr difference_type operator-(const transform_iterator &left, const transform_iterator &right) {
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

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief A lazy view that applies a function to every element of an underlying range.
     *
     * Elements are not transformed eagerly: each dereference of the view's
     * iterator applies `Fx` to the corresponding element of the underlying range.
     *
     * @tparam ViewOrContainer The underlying range type
     * @tparam Fx The function applied to each element
     *
     * \lang simp-chinese
     * @brief 对底层范围的每个元素应用函数的惰性视图。
     *
     * 元素不会被立即转换：每次解引用视图的迭代器时，才会对底层范围中对应的元素应用 `Fx`。
     *
     * @tparam ViewOrContainer 底层范围类型
     * @tparam Fx 应用于每个元素的函数
     */
    template <typename ViewOrContainer, typename Fx>
    class transform_view : public implements::invoker_view<transform_view<ViewOrContainer, Fx>, ViewOrContainer, Fx> {
    public:
        /**
         * \lang english
         * @brief Mutable iterator type over the transformed elements.
         *
         * \lang simp-chinese
         * @brief 转换后元素的可变迭代器类型。
         */
        using iterator = implements::transform_iterator<transform_view, ViewOrContainer, Fx, false>;

        /**
         * \lang english
         * @brief Immutable (const) iterator type over the transformed elements.
         *
         * \lang simp-chinese
         * @brief 转换后元素的不可变（常量）迭代器类型。
         */
        using const_iterator = implements::transform_iterator<transform_view, ViewOrContainer, Fx, true>;

        /**
         * \lang english
         * @brief The underlying range type.
         *
         * \lang simp-chinese
         * @brief 底层范围类型。
         */
        using view_or_container = ViewOrContainer;

        /**
         * \lang english
         * @brief The function type applied to each element.
         *
         * \lang simp-chinese
         * @brief 应用于每个元素的函数类型。
         */
        using fx_type = Fx;

        /**
         * @hide
         */
        using base = implements::invoker_view<transform_view<ViewOrContainer, Fx>, ViewOrContainer, Fx>;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        transform_view() = default;

        /**
         * \lang english
         * @brief Constructs the view from an lvalue range and a function.
         *
         * @param range_ The underlying range (referenced, not copied)
         * @param func The function applied to each element
         *
         * \lang simp-chinese
         * @brief 由左值范围与函数构造视图。
         *
         * @param range_ 底层范围（引用而非拷贝）
         * @param func 应用于每个元素的函数
         */
        constexpr explicit transform_view(ViewOrContainer &range_, Fx func) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<ViewOrContainer> &&
            type_traits::properties::is_nothrow_move_constructible_v<Fx>) : base(range_, utility::forward<Fx>(func)) {
        }

        /**
         * \lang english
         * @brief Constructs the view from an rvalue range and a function.
         *
         * @param range_ The underlying range (moved into the view)
         * @param func The function applied to each element
         *
         * \lang simp-chinese
         * @brief 由右值范围与函数构造视图。
         *
         * @param range_ 底层范围（被移动进视图）
         * @param func 应用于每个元素的函数
         */
        constexpr explicit transform_view(ViewOrContainer &&range_, Fx func) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<ViewOrContainer> &&
            type_traits::properties::is_nothrow_move_constructible_v<Fx>) : base(utility::move(range_), utility::forward<Fx>(func)) {
        }
    };

    /**
     * \lang english
     * @brief Range adaptor object for `transform`.
     *
     * Provides two call forms:
     * - `transform_fn(range, func)` — returns a `transform_view` over `range`
     *   immediately.
     * - `transform_fn(func)` — returns a range closure that can be applied to a
     *   range later with the pipe operator.
     *
     * \lang simp-chinese
     * @brief `transform` 的范围适配器对象。
     *
     * 提供两种调用形式：
     * - `transform_fn(range, func)` —— 立即返回 `range` 上的 `transform_view`。
     * - `transform_fn(func)` —— 返回范围闭包，之后可通过管道运算符作用于某个范围。
     */
    struct transform_fn {
        /**
         * \lang english
         * @brief Builds the view directly from a range and a function.
         *
         * @param range The underlying range
         * @param func The function applied to each element
         * @return A `transform_view` over `range`
         *
         * \lang simp-chinese
         * @brief 直接由范围与函数构造视图。
         *
         * @param range 底层范围
         * @param func 应用于每个元素的函数
         * @return `range` 上的 `transform_view`
         */
        template <typename Range, typename Fx>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, Fx func) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range), utility::move(func)))) {
            return transform_view(utility::forward<Range>(range), utility::move(func));
        }

        /**
         * \lang english
         * @brief Builds a range closure that applies func to each element.
         *
         * @param func The function applied to each element
         * @return A range closure awaiting a range
         *
         * \lang simp-chinese
         * @brief 构造对每个元素应用 func 的范围闭包。
         *
         * @param func 应用于每个元素的函数
         * @return 等待范围的闭包
         */
        template <typename Fx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<type_traits::other_trans::decay_t<Fx>, Fx>, int> = 0>
        RAINY_NODISCARD constexpr auto operator()(Fx &&func) const
            noexcept(type_traits::properties::is_nothrow_constructible_v<type_traits::other_trans::decay_t<Fx>, Fx>) {
            return implements::range_closure<transform_fn, type_traits::other_trans::decay_t<Fx>>{utility::forward<Fx>(func)};
        }
    };

    /**
     * \lang english
     * @brief Global range adaptor object for lazily transforming each element with a function.
     *
     * \lang simp-chinese
     * @brief 用函数惰性转换每个元素的全局范围适配器对象。
     */
    inline constexpr transform_fn transform;
}

namespace rainy::collections::views {
    using core::collections::views::transform;
    using core::collections::views::transform_fn;
    using core::collections::views::transform_view;
}

#endif
