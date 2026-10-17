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
#ifndef RAINY_CORE_UTILITY_MOVE_ITERATOR_HPP
#define RAINY_CORE_UTILITY_MOVE_ITERATOR_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/modifers.hpp>
#include <rainy/core/type_traits/primary_types.hpp>

namespace rainy::utility {
    /**
     * \lang english
     * @brief An iterator adaptor that dereferences to rvalue references.
     *
     * move_iterator wraps a regular iterator and turns every dereference into an rvalue
     * access, which turns range-based element copies into moves. Incrementing, decrementing
     * and offsetting are forwarded unchanged to the underlying iterator.
     *
     * @tparam Iter The underlying iterator type
     *
     * \lang simp-chinese
     * @brief 解引用结果为右值引用的迭代器适配器。
     *
     * move_iterator 包装一个普通迭代器，并把每次解引用变为右值访问，
     * 从而把基于范围的元素拷贝转化为移动。前后递增与偏移操作原样转发给底层迭代器。
     *
     * @tparam Iter 底层迭代器类型
     */
    template <typename Iter>
    class move_iterator {
    public:
        using iterator_type = Iter;
        using iterator_traits = utility::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using pointer = typename iterator_traits::pointer;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        /**
         * \lang english
         * @brief The reference type produced by dereferencing.
         *
         * An rvalue reference when the underlying iterator yields an lvalue reference,
         * otherwise the underlying reference type unchanged (proxy references).
         *
         * \lang simp-chinese
         * @brief 解引用产生的引用类型。
         *
         * 当底层迭代器产生左值引用时为右值引用，否则保持底层引用类型不变（代理引用）。
         */
        using reference = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_lvalue_reference_v<typename iterator_traits::reference>,
            type_traits::modifers::remove_reference_t<typename iterator_traits::reference> &&, typename iterator_traits::reference>;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        constexpr move_iterator() noexcept = default;

        /**
         * \lang english
         * @brief Constructs a move_iterator from the underlying iterator.
         *
         * @param right The iterator to wrap
         *
         * \lang simp-chinese
         * @brief 从底层迭代器构造 move_iterator。
         *
         * @param right 要包装的迭代器
         */
        constexpr explicit move_iterator(Iter right) noexcept(type_traits::implements::is_nothrow_move_constructible_v<Iter>) :
            current(utility::move(right)) {
        }

        /**
         * \lang english
         * @brief Converting constructor from a move_iterator over a different iterator type.
         *
         * @tparam UTy The underlying iterator type of the source
         * @param right The source move_iterator
         *
         * \lang simp-chinese
         * @brief 从底层迭代器类型不同的 move_iterator 进行转换构造。
         *
         * @tparam UTy 源对象的底层迭代器类型
         * @param right 源 move_iterator
         */
        template <typename UTy>
        constexpr move_iterator(const move_iterator<UTy> &right) noexcept(
            type_traits::implements::is_nothrow_constructible_v<Iter, const UTy &>) : current(right.base()) {
        }

        /**
         * \lang english
         * @brief Assigns from a move_iterator over a different iterator type.
         *
         * @tparam UTy The underlying iterator type of the source
         * @param right The source move_iterator
         * @return Reference to this iterator after assignment
         *
         * \lang simp-chinese
         * @brief 从底层迭代器类型不同的 move_iterator 赋值。
         *
         * @tparam UTy 源对象的底层迭代器类型
         * @param right 源 move_iterator
         * @return 赋值后此迭代器的引用
         */
        template <typename UTy>
        constexpr move_iterator &operator=(const move_iterator<UTy> &right) noexcept(
            type_traits::implements::is_nothrow_assignable_v<Iter &, const UTy &>) {
            current = right.base();
            return *this;
        }

        /**
         * \lang english
         * @brief Returns a const reference to the wrapped iterator.
         *
         * \lang simp-chinese
         * @brief 返回被包装迭代器的常量引用。
         */
        RAINY_NODISCARD constexpr const iterator_type &base() const & noexcept {
            return current;
        }

        /**
         * \lang english
         * @brief Moves the wrapped iterator out of this move_iterator.
         *
         * \lang simp-chinese
         * @brief 将被包装的迭代器从此 move_iterator 中移出。
         */
        RAINY_NODISCARD constexpr iterator_type base() && noexcept(type_traits::implements::is_nothrow_move_constructible_v<Iter>) {
            return utility::move(current);
        }

        /**
         * \lang english
         * @brief Dereferences the wrapped iterator as an rvalue.
         *
         * \lang simp-chinese
         * @brief 以右值方式解引用被包装的迭代器。
         */
        RAINY_NODISCARD constexpr reference operator*() const noexcept(noexcept(static_cast<reference>(*current))) {
            return static_cast<reference>(*current);
        }

        /**
         * \lang english
         * @brief Arrow operator for element access.
         *
         * \lang simp-chinese
         * @brief 用于元素访问的箭头运算符。
         */
        RAINY_NODISCARD constexpr pointer operator->() const noexcept {
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return current;
            } else {
                return current.operator->();
            }
        }

        /**
         * \lang english
         * @brief Prefix increment operator.
         *
         * \lang simp-chinese
         * @brief 前置自增运算符。
         */
        constexpr move_iterator &operator++() noexcept(noexcept(++current)) {
            ++current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix increment operator.
         *
         * \lang simp-chinese
         * @brief 后置自增运算符。
         */
        constexpr move_iterator operator++(int) noexcept(type_traits::implements::is_nothrow_copy_constructible_v<Iter> &&
                                                         noexcept(++current)) {
            move_iterator tmp = *this;
            ++current;
            return tmp;
        }

        /**
         * \lang english
         * @brief Prefix decrement operator.
         *
         * \lang simp-chinese
         * @brief 前置自减运算符。
         */
        constexpr move_iterator &operator--() noexcept(noexcept(--current)) {
            --current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix decrement operator.
         *
         * \lang simp-chinese
         * @brief 后置自减运算符。
         */
        constexpr move_iterator operator--(int) noexcept(type_traits::implements::is_nothrow_copy_constructible_v<Iter> &&
                                                         noexcept(--current)) {
            move_iterator tmp = *this;
            --current;
            return tmp;
        }

        /**
         * \lang english
         * @brief Addition operator.
         *
         * @param off The offset relative to the current position
         * @return An iterator advanced by off positions
         *
         * \lang simp-chinese
         * @brief 加法运算符。
         *
         * @param off 相对于当前位置的偏移
         * @return 前进 off 个位置后的迭代器
         */
        RAINY_NODISCARD constexpr move_iterator operator+(const difference_type off) const
            noexcept(noexcept(move_iterator(current + off))) {
            return move_iterator(current + off);
        }

        /**
         * \lang english
         * @brief Addition assignment operator.
         *
         * @param off The offset relative to the current position
         * @return Reference to this iterator after advancement
         *
         * \lang simp-chinese
         * @brief 加法赋值运算符。
         *
         * @param off 相对于当前位置的偏移
         * @return 前进后此迭代器的引用
         */
        constexpr move_iterator &operator+=(const difference_type off) noexcept(noexcept(current += off)) {
            current += off;
            return *this;
        }

        /**
         * \lang english
         * @brief Subtraction operator.
         *
         * @param off The offset relative to the current position
         * @return An iterator moved back by off positions
         *
         * \lang simp-chinese
         * @brief 减法运算符。
         *
         * @param off 相对于当前位置的偏移
         * @return 后退 off 个位置后的迭代器
         */
        RAINY_NODISCARD constexpr move_iterator operator-(const difference_type off) const
            noexcept(noexcept(move_iterator(current - off))) {
            return move_iterator(current - off);
        }

        /**
         * \lang english
         * @brief Subtraction assignment operator.
         *
         * @param off The offset relative to the current position
         * @return Reference to this iterator after movement
         *
         * \lang simp-chinese
         * @brief 减法赋值运算符。
         *
         * @param off 相对于当前位置的偏移
         * @return 后退后此迭代器的引用
         */
        constexpr move_iterator &operator-=(const difference_type off) noexcept(noexcept(current -= off)) {
            current -= off;
            return *this;
        }

        /**
         * \lang english
         * @brief Subscript operator, yielding an rvalue reference to the element.
         *
         * @param off The offset relative to the current position
         * @return An rvalue reference to the element at offset off
         *
         * \lang simp-chinese
         * @brief 下标运算符，产生元素右值引用。
         *
         * @param off 相对于当前位置的偏移
         * @return 偏移 off 处元素的右值引用
         */
        RAINY_NODISCARD constexpr reference operator[](const difference_type off) const
            noexcept(noexcept(utility::move(current[off]))) {
            return utility::move(current[off]);
        }

    private:
        iterator_type current{};
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_equal_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() ==
                                                                              utility::declval<const Ty &>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_not_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_not_equal_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() !=
                                                                                   utility::declval<const Ty &>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_less_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_less_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() <
                                                                             utility::declval<const Ty &>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_greater_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_greater_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() >
                                                                                utility::declval<const Ty &>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_less_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_less_equal_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() <=
                                                                                     utility::declval<const Ty &>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_greater_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_greater_equal_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() >=
                                                                                        utility::declval<const Ty &>())>> =
            true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL move_iterator_has_difference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        move_iterator_has_difference_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() -
                                                                                     utility::declval<const Ty &>())>> = true;
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Equality comparison operator for move_iterator.
     *
     * Enabled only when the underlying iterators themselves compare equal.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return true if both refer to the same position
     *
     * \lang simp-chinese
     * @brief move_iterator 的相等比较运算符。
     *
     * 仅当底层迭代器本身支持相等比较时参与重载。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 若两者指向相同位置则为 true
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_equal_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr bool operator==(const move_iterator<Iter1> &left,
                                              const move_iterator<Iter2> &right) noexcept(noexcept(left.base() == right.base())) {
        return left.base() == right.base();
    }

    /**
     * \lang english
     * @brief Inequality comparison operator for move_iterator.
     *
     * Enabled only when the underlying iterators themselves compare unequal.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return true if the operands refer to different positions
     *
     * \lang simp-chinese
     * @brief move_iterator 的不等比较运算符。
     *
     * 仅当底层迭代器本身支持不等比较时参与重载。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 若两者指向不同位置则为 true
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_not_equal_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr bool operator!=(const move_iterator<Iter1> &left, const move_iterator<Iter2> &right) noexcept(
        noexcept(!(left == right))) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Less-than comparison operator for move_iterator.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return true if left precedes right
     *
     * \lang simp-chinese
     * @brief move_iterator 的小于比较运算符。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 若 left 在 right 之前则为 true
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_less_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr bool operator<(const move_iterator<Iter1> &left,
                                             const move_iterator<Iter2> &right) noexcept(noexcept(left.base() < right.base())) {
        return left.base() < right.base();
    }

    /**
     * \lang english
     * @brief Greater-than comparison operator for move_iterator.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return true if left follows right
     *
     * \lang simp-chinese
     * @brief move_iterator 的大于比较运算符。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 若 left 在 right 之后则为 true
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_greater_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr bool operator>(const move_iterator<Iter1> &left,
                                             const move_iterator<Iter2> &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Less-than-or-equal comparison operator for move_iterator.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return true if left does not follow right
     *
     * \lang simp-chinese
     * @brief move_iterator 的小于等于比较运算符。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 若 left 不在 right 之后则为 true
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_less_equal_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr bool operator<=(const move_iterator<Iter1> &left,
                                              const move_iterator<Iter2> &right) noexcept(noexcept(!(right < left))) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Greater-than-or-equal comparison operator for move_iterator.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return true if left does not precede right
     *
     * \lang simp-chinese
     * @brief move_iterator 的大于等于比较运算符。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 若 left 不在 right 之前则为 true
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_greater_equal_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr bool operator>=(const move_iterator<Iter1> &left,
                                              const move_iterator<Iter2> &right) noexcept(noexcept(!(left < right))) {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Computes the distance between two move_iterators.
     *
     * @tparam Iter1 The underlying iterator type of the left operand
     * @tparam Iter2 The underlying iterator type of the right operand
     * @param left The left operand
     * @param right The right operand
     * @return The distance between the two iterators
     *
     * \lang simp-chinese
     * @brief 计算两个 move_iterator 之间的距离。
     *
     * @tparam Iter1 左操作数的底层迭代器类型
     * @tparam Iter2 右操作数的底层迭代器类型
     * @param left 左操作数
     * @param right 右操作数
     * @return 两个迭代器之间的距离
     */
    template <typename Iter1, typename Iter2,
              type_traits::other_trans::enable_if_t<implements::move_iterator_has_difference_v<Iter1>, int> = 0>
    RAINY_NODISCARD constexpr rain_fn operator-(const move_iterator<Iter1> &left, const move_iterator<Iter2> &right) noexcept(
        noexcept(left.base() - right.base())) -> decltype(left.base() - right.base()) {
        return left.base() - right.base();
    }

    /**
     * \lang english
     * @brief Creates a move_iterator wrapping the given iterator.
     *
     * @tparam Iter The underlying iterator type
     * @param iter The iterator to wrap
     * @return A move_iterator over the given iterator
     *
     * \lang simp-chinese
     * @brief 创建一个包装给定迭代器的 move_iterator。
     *
     * @tparam Iter 底层迭代器类型
     * @param iter 要包装的迭代器
     * @return 包装给定迭代器的 move_iterator
     */
    template <typename Iter>
    RAINY_NODISCARD constexpr move_iterator<Iter> make_move_iterator(Iter iter) noexcept(
        type_traits::implements::is_nothrow_move_constructible_v<Iter>) {
        return move_iterator<Iter>(utility::move(iter));
    }
}

#endif
