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
     * \lang english
     * @brief Functor that passes elements through, substituting a held replacement
     *         for every element that compares equal to a held old value.
     *
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 透传元素的函子，对每个与持有旧值相等的元素替换为持有的新值。
     *
     * @tparam Ty 元素值类型
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
     * \lang english
     * @brief Functor that passes elements through, substituting a held replacement
     *         for every element satisfying a held predicate.
     *
     * @tparam Pred The predicate type
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 透传元素的函子，对每个满足持有谓词的元素替换为持有的新值。
     *
     * @tparam Pred 谓词类型
     * @tparam Ty 元素值类型
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
     * \lang english
     * @brief Functor that copies elements, substituting a held replacement for
     *         every element that compares equal to a held old value.
     *
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 拷贝元素的函子，对每个与持有旧值相等的元素替换为持有的新值。
     *
     * @tparam Ty 元素值类型
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
     * \lang english
     * @brief Functor that copies elements, substituting a held replacement for
     *         every element satisfying a held predicate.
     *
     * @tparam Pred The predicate type
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 拷贝元素的函子，对每个满足持有谓词的元素替换为持有的新值。
     *
     * @tparam Pred 谓词类型
     * @tparam Ty 元素值类型
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
     * \lang english
     * @brief A lazy view that replaces every element equal to an old value with a new one.
     *
     * @tparam ViewOrContainer The underlying range type
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 将每个等于旧值的元素替换为新元素的惰性视图。
     *
     * @tparam ViewOrContainer 底层范围类型
     * @tparam Ty 元素值类型
     */
    template <typename ViewOrContainer, typename Ty>
    using replace_view = transform_view<ViewOrContainer, implements::replace_functor<Ty>>;

    /**
     * \lang english
     * @brief A lazy view that replaces every element satisfying a predicate with a new value.
     *
     * @tparam ViewOrContainer The underlying range type
     * @tparam Pred The predicate type
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 将每个满足谓词的元素替换为新值的惰性视图。
     *
     * @tparam ViewOrContainer 底层范围类型
     * @tparam Pred 谓词类型
     * @tparam Ty 元素值类型
     */
    template <typename ViewOrContainer, typename Pred, typename Ty>
    using replace_if_view = transform_view<ViewOrContainer, implements::replace_if_functor<Pred, Ty>>;

    /**
     * \lang english
     * @brief A lazy view whose elements are copies with replaced values.
     *
     * @tparam ViewOrContainer The underlying range type
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 元素为替换后副本的惰性视图。
     *
     * @tparam ViewOrContainer 底层范围类型
     * @tparam Ty 元素值类型
     */
    template <typename ViewOrContainer, typename Ty>
    using replace_copy_view = transform_view<ViewOrContainer, implements::replace_copy_functor<Ty>>;

    /**
     * \lang english
     * @brief A lazy view whose elements are copies with values replaced by a predicate.
     *
     * @tparam ViewOrContainer The underlying range type
     * @tparam Pred The predicate type
     * @tparam Ty The element value type
     *
     * \lang simp-chinese
     * @brief 元素为按谓词替换后副本的惰性视图。
     *
     * @tparam ViewOrContainer 底层范围类型
     * @tparam Pred 谓词类型
     * @tparam Ty 元素值类型
     */
    template <typename ViewOrContainer, typename Pred, typename Ty>
    using replace_copy_if_view = transform_view<ViewOrContainer, implements::replace_copy_if_functor<Pred, Ty>>;

    /**
     * \lang english
     * @brief Range adaptor object for `replace`.
     *
     * Provides two call forms:
     * - `replace_fn(range, old_value, new_value)` — returns a `replace_view`
     *   over `range` immediately.
     * - `replace_fn(old_value, new_value)` — returns a range closure that can be
     *   applied to a range later with the pipe operator.
     *
     * \lang simp-chinese
     * @brief `replace` 的范围适配器对象。
     *
     * 提供两种调用形式：
     * - `replace_fn(range, old_value, new_value)` —— 立即返回 `range` 上的
     *   `replace_view`。
     * - `replace_fn(old_value, new_value)` —— 返回范围闭包，之后可通过管道
     *   运算符作用于某个范围。
     */
    struct replace_fn {
        /**
         * \lang english
         * @brief Builds the view directly from a range and the old/new values.
         *
         * @param range The underlying range
         * @param old_value The value to be replaced
         * @param new_value The replacement value
         * @return A `replace_view` over `range`
         *
         * \lang simp-chinese
         * @brief 直接由范围与新旧值构造视图。
         *
         * @param range 底层范围
         * @param old_value 要被替换的值
         * @param new_value 替换后的值
         * @return `range` 上的 `replace_view`
         */
        template <typename Range, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, const Ty &old_value, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range), implements::replace_functor<Ty>{old_value, new_value}))) {
            return transform_view(utility::forward<Range>(range), implements::replace_functor<Ty>{old_value, new_value});
        }

        /**
         * \lang english
         * @brief Builds a range closure that replaces old_value with new_value.
         *
         * @param old_value The value to be replaced
         * @param new_value The replacement value
         * @return A range closure awaiting a range
         *
         * \lang simp-chinese
         * @brief 构造将 old_value 替换为 new_value 的范围闭包。
         *
         * @param old_value 要被替换的值
         * @param new_value 替换后的值
         * @return 等待范围的闭包
         */
        template <typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Ty old_value, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_fn, Ty, Ty>{utility::move(old_value), utility::move(new_value)};
        }
    };

    /**
     * \lang english
     * @brief Range adaptor object for `replace_if`.
     *
     * Provides two call forms:
     * - `replace_if_fn(range, pred, new_value)` — returns a `replace_if_view`
     *   over `range` immediately.
     * - `replace_if_fn(pred, new_value)` — returns a range closure that can be
     *   applied to a range later with the pipe operator.
     *
     * \lang simp-chinese
     * @brief `replace_if` 的范围适配器对象。
     *
     * 提供两种调用形式：
     * - `replace_if_fn(range, pred, new_value)` —— 立即返回 `range` 上的
     *   `replace_if_view`。
     * - `replace_if_fn(pred, new_value)` —— 返回范围闭包，之后可通过管道
     *   运算符作用于某个范围。
     */
    struct replace_if_fn {
        /**
         * \lang english
         * @brief Builds the view directly from a range, a predicate and the new value.
         *
         * @param range The underlying range
         * @param pred The predicate deciding which elements are replaced
         * @param new_value The replacement value
         * @return A `replace_if_view` over `range`
         *
         * \lang simp-chinese
         * @brief 直接由范围、谓词与新值构造视图。
         *
         * @param range 底层范围
         * @param pred 决定哪些元素被替换的谓词
         * @param new_value 替换后的值
         * @return `range` 上的 `replace_if_view`
         */
        template <typename Range, typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, Pred pred, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_if_functor<Pred, Ty>{utility::move(pred), new_value}))) {
            return transform_view(utility::forward<Range>(range),
                                  implements::replace_if_functor<Pred, Ty>{utility::move(pred), new_value});
        }

        /**
         * \lang english
         * @brief Builds a range closure that replaces elements satisfying pred with new_value.
         *
         * @param pred The predicate deciding which elements are replaced
         * @param new_value The replacement value
         * @return A range closure awaiting a range
         *
         * \lang simp-chinese
         * @brief 构造将满足 pred 的元素替换为 new_value 的范围闭包。
         *
         * @param pred 决定哪些元素被替换的谓词
         * @param new_value 替换后的值
         * @return 等待范围的闭包
         */
        template <typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Pred pred, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<type_traits::other_trans::decay_t<Pred>> &&
                     type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_if_fn, type_traits::other_trans::decay_t<Pred>, Ty>{utility::forward<Pred>(pred),
                                                                                                         utility::move(new_value)};
        }
    };

    /**
     * \lang english
     * @brief Range adaptor object for `replace_copy`.
     *
     * Provides two call forms:
     * - `replace_copy_fn(range, old_value, new_value)` — returns a
     *   `replace_copy_view` over `range` immediately.
     * - `replace_copy_fn(old_value, new_value)` — returns a range closure that
     *   can be applied to a range later with the pipe operator.
     *
     * \lang simp-chinese
     * @brief `replace_copy` 的范围适配器对象。
     *
     * 提供两种调用形式：
     * - `replace_copy_fn(range, old_value, new_value)` —— 立即返回 `range` 上的
     *   `replace_copy_view`。
     * - `replace_copy_fn(old_value, new_value)` —— 返回范围闭包，之后可通过
     *   管道运算符作用于某个范围。
     */
    struct replace_copy_fn {
        /**
         * \lang english
         * @brief Builds the view directly from a range and the old/new values.
         *
         * @param range The underlying range
         * @param old_value The value to be replaced
         * @param new_value The replacement value
         * @return A `replace_copy_view` over `range`
         *
         * \lang simp-chinese
         * @brief 直接由范围与新旧值构造视图。
         *
         * @param range 底层范围
         * @param old_value 要被替换的值
         * @param new_value 替换后的值
         * @return `range` 上的 `replace_copy_view`
         */
        template <typename Range, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, const Ty &old_value, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_copy_functor<Ty>{old_value, new_value}))) {
            return transform_view(utility::forward<Range>(range), implements::replace_copy_functor<Ty>{old_value, new_value});
        }

        /**
         * \lang english
         * @brief Builds a range closure that copies elements, replacing old_value with new_value.
         *
         * @param old_value The value to be replaced
         * @param new_value The replacement value
         * @return A range closure awaiting a range
         *
         * \lang simp-chinese
         * @brief 构造拷贝元素、并将 old_value 替换为 new_value 的范围闭包。
         *
         * @param old_value 要被替换的值
         * @param new_value 替换后的值
         * @return 等待范围的闭包
         */
        template <typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Ty old_value, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_copy_fn, Ty, Ty>{utility::move(old_value), utility::move(new_value)};
        }
    };

    /**
     * \lang english
     * @brief Range adaptor object for `replace_copy_if`.
     *
     * Provides two call forms:
     * - `replace_copy_if_fn(range, pred, new_value)` — returns a
     *   `replace_copy_if_view` over `range` immediately.
     * - `replace_copy_if_fn(pred, new_value)` — returns a range closure that can
     *   be applied to a range later with the pipe operator.
     *
     * \lang simp-chinese
     * @brief `replace_copy_if` 的范围适配器对象。
     *
     * 提供两种调用形式：
     * - `replace_copy_if_fn(range, pred, new_value)` —— 立即返回 `range` 上的
     *   `replace_copy_if_view`。
     * - `replace_copy_if_fn(pred, new_value)` —— 返回范围闭包，之后可通过
     *   管道运算符作用于某个范围。
     */
    struct replace_copy_if_fn {
        /**
         * \lang english
         * @brief Builds the view directly from a range, a predicate and the new value.
         *
         * @param range The underlying range
         * @param pred The predicate deciding which elements are replaced
         * @param new_value The replacement value
         * @return A `replace_copy_if_view` over `range`
         *
         * \lang simp-chinese
         * @brief 直接由范围、谓词与新值构造视图。
         *
         * @param range 底层范围
         * @param pred 决定哪些元素被替换的谓词
         * @param new_value 替换后的值
         * @return `range` 上的 `replace_copy_if_view`
         */
        template <typename Range, typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Range &&range, Pred pred, const Ty &new_value) const
            noexcept(noexcept(transform_view(utility::forward<Range>(range),
                                             implements::replace_copy_if_functor<Pred, Ty>{utility::move(pred), new_value}))) {
            return transform_view(utility::forward<Range>(range),
                                  implements::replace_copy_if_functor<Pred, Ty>{utility::move(pred), new_value});
        }

        /**
         * \lang english
         * @brief Builds a range closure that copies elements, replacing those satisfying pred.
         *
         * @param pred The predicate deciding which elements are replaced
         * @param new_value The replacement value
         * @return A range closure awaiting a range
         *
         * \lang simp-chinese
         * @brief 构造拷贝元素、并替换满足 pred 的元素的范围闭包。
         *
         * @param pred 决定哪些元素被替换的谓词
         * @param new_value 替换后的值
         * @return 等待范围的闭包
         */
        template <typename Pred, typename Ty>
        RAINY_NODISCARD constexpr auto operator()(Pred pred, Ty new_value) const
            noexcept(type_traits::properties::is_nothrow_move_constructible_v<type_traits::other_trans::decay_t<Pred>> &&
                     type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return implements::range_closure<replace_copy_if_fn, type_traits::other_trans::decay_t<Pred>, Ty>{
                utility::forward<Pred>(pred), utility::move(new_value)};
        }
    };

    /**
     * \lang english
     * @brief Global range adaptor object for replacing equal elements with a new value.
     *
     * \lang simp-chinese
     * @brief 将相等元素替换为新值的全局范围适配器对象。
     */
    inline constexpr replace_fn replace{};

    /**
     * \lang english
     * @brief Global range adaptor object for replacing elements satisfying a predicate with a new value.
     *
     * \lang simp-chinese
     * @brief 将满足谓词的元素替换为新值的全局范围适配器对象。
     */
    inline constexpr replace_if_fn replace_if{};

    /**
     * \lang english
     * @brief Global range adaptor object for producing copies with equal elements replaced.
     *
     * \lang simp-chinese
     * @brief 生成副本并将相等元素替换的全局范围适配器对象。
     */
    inline constexpr replace_copy_fn replace_copy{};

    /**
     * \lang english
     * @brief Global range adaptor object for producing copies with predicate-satisfying elements replaced.
     *
     * \lang simp-chinese
     * @brief 生成副本并将满足谓词的元素替换的全局范围适配器对象。
     */
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
    using core::collections::views::replace_fn;
    using core::collections::views::replace_if;
    using core::collections::views::replace_if_fn;
    using core::collections::views::replace_if_view;
    using core::collections::views::replace_view;
}

#endif
