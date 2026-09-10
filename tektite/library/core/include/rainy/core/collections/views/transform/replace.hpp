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
#ifndef RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_REPLACE_HPP
#define RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_REPLACE_HPP

#include <rainy/core/collections/views/transform/transform.hpp>

namespace rainy::core::collections::views::implements {
    /**
     * @brief Functor that passes elements through, substituting a held replacement
     *        for every element that compares equal to a held old value.
     *        透传元素的函子，对每个与持有旧值相等的元素替换为持有的新值。
     *
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename Ty>
    struct replace_functor {
        Ty old_value;
        Ty new_value;

        template <typename Uty>
        RAINY_NODISCARD constexpr const Ty &operator()(Uty &&value) const {
            if (value == old_value) {
                return new_value;
            }
            return value;
        }
    };

    /**
     * @brief Functor that passes elements through, substituting a held replacement
     *        for every element satisfying a held predicate.
     *        透传元素的函子，对每个满足持有谓词的元素替换为持有的新值。
     *
     * @tparam Pred The predicate type
     *              谓词类型
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename Pred, typename Ty>
    struct replace_if_functor {
        Pred pred;
        Ty new_value;

        template <typename Uty>
        RAINY_NODISCARD constexpr const Ty &operator()(Uty &&value) const {
            if (pred(value)) {
                return new_value;
            }
            return value;
        }
    };

    /**
     * @brief Functor that copies elements, substituting a held replacement for
     *        every element that compares equal to a held old value.
     *        拷贝元素的函子，对每个与持有旧值相等的元素替换为持有的新值。
     *
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename Ty>
    struct replace_copy_functor {
        Ty old_value;
        Ty new_value;

        template <typename Uty>
        RAINY_NODISCARD constexpr Ty operator()(Uty &&value) const {
            if (value == old_value) {
                return new_value;
            }
            return static_cast<Ty>(static_cast<Uty &&>(value));
        }
    };

    /**
     * @brief Functor that copies elements, substituting a held replacement for
     *        every element satisfying a held predicate.
     *        拷贝元素的函子，对每个满足持有谓词的元素替换为持有的新值。
     *
     * @tparam Pred The predicate type
     *              谓词类型
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename Pred, typename Ty>
    struct replace_copy_if_functor {
        Pred pred;
        Ty new_value;

        template <typename Uty>
        RAINY_NODISCARD constexpr Ty operator()(Uty &&value) const {
            if (pred(value)) {
                return new_value;
            }
            return static_cast<Ty>(static_cast<Uty &&>(value));
        }
    };
}

namespace rainy::core::collections::views {
    /**
     * @brief A lazy view that replaces every element equal to an old value with a new one.
     *        将每个等于旧值的元素替换为新元素的惰性视图。
     *
     * @tparam ViewOrContainer The underlying range type
     *                         底层范围类型
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename ViewOrContainer, typename Ty>
    using replace_view = transform_view<ViewOrContainer, implements::replace_functor<Ty>>;

    /**
     * @brief A lazy view that replaces every element satisfying a predicate with a new value.
     *        将每个满足谓词的元素替换为新值的惰性视图。
     *
     * @tparam ViewOrContainer The underlying range type
     *                         底层范围类型
     * @tparam Pred The predicate type
     *              谓词类型
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename ViewOrContainer, typename Pred, typename Ty>
    using replace_if_view = transform_view<ViewOrContainer, implements::replace_if_functor<Pred, Ty>>;

    /**
     * @brief A lazy view whose elements are copies with replaced values.
     *        元素为替换后副本的惰性视图。
     *
     * @tparam ViewOrContainer The underlying range type
     *                         底层范围类型
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename ViewOrContainer, typename Ty>
    using replace_copy_view = transform_view<ViewOrContainer, implements::replace_copy_functor<Ty>>;

    /**
     * @brief A lazy view whose elements are copies with values replaced by a predicate.
     *        元素为按谓词替换后副本的惰性视图。
     *
     * @tparam ViewOrContainer The underlying range type
     *                         底层范围类型
     * @tparam Pred The predicate type
     *              谓词类型
     * @tparam Ty The element value type
     *            元素值类型
     */
    template <typename ViewOrContainer, typename Pred, typename Ty>
    using replace_copy_if_view = transform_view<ViewOrContainer, implements::replace_copy_if_functor<Pred, Ty>>;

    struct replace_fn {
        template <typename Range, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, const Ty &old_value, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_functor<Ty>{old_value, new_value}))) {
            return transform_view(utility::forward<Range>(range), implements::replace_functor<Ty>{old_value, new_value});
        }

        template <typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Ty old_value, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_fn, Ty, Ty>{utility::move(old_value), utility::move(new_value)};
        }
    };

    struct replace_if_fn {
        template <typename Range, typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, Pred pred, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_if_functor<Pred, Ty>{utility::move(pred), new_value}))) {
            return transform_view(utility::forward<Range>(range),
                                  implements::replace_if_functor<Pred, Ty>{utility::move(pred), new_value});
        }

        template <typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Pred pred, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<type_traits::other_trans::decay_t<Pred>> &&
                     type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_if_fn, type_traits::other_trans::decay_t<Pred>, Ty>{
                utility::forward<Pred>(pred), utility::move(new_value)};
        }
    };

    struct replace_copy_fn {
        template <typename Range, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, const Ty &old_value, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_copy_functor<Ty>{old_value, new_value}))) {
            return transform_view(utility::forward<Range>(range), implements::replace_copy_functor<Ty>{old_value, new_value});
        }

        template <typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Ty old_value, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_copy_fn, Ty, Ty>{utility::move(old_value), utility::move(new_value)};
        }
    };

    struct replace_copy_if_fn {
        template <typename Range, typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, Pred pred, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_copy_if_functor<Pred, Ty>{utility::move(pred), new_value}))) {
            return transform_view(utility::forward<Range>(range),
                                  implements::replace_copy_if_functor<Pred, Ty>{utility::move(pred), new_value});
        }

        template <typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Pred pred, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<type_traits::other_trans::decay_t<Pred>> &&
                     type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_copy_if_fn, type_traits::other_trans::decay_t<Pred>, Ty>{
                utility::forward<Pred>(pred), utility::move(new_value)};
        }
    };

    inline constexpr replace_fn replace{};
    inline constexpr replace_if_fn replace_if{};
    inline constexpr replace_copy_fn replace_copy{};
    inline constexpr replace_copy_if_fn replace_copy_if{};
}

namespace rainy::collections::views {
    using core::collections::views::replace;
    using core::collections::views::replace_copy;
    using core::collections::views::replace_copy_fn;
    using core::collections::views::replace_copy_if;
    using core::collections::views::replace_copy_if_fn;
    using core::collections::views::replace_copy_if_view;
    using core::collections::views::replace_copy_view;
    using core::collections::views::replace_copy_fn;
    using core::collections::views::replace_fn;
    using core::collections::views::replace_if;
    using core::collections::views::replace_if_fn;
    using core::collections::views::replace_if_view;
    using core::collections::views::replace_view;
}

#endif
