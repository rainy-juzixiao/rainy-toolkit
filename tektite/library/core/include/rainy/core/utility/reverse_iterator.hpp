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
#ifndef RAINY_CORE_YESOD_REVERSE_ITERATOR_HPP
#define RAINY_CORE_YESOD_REVERSE_ITERATOR_HPP
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/primary_types.hpp>

namespace rainy::utility {
    /**
     * \lang english
     * @brief An iterator adaptor that reverses the direction of a bidirectional iterator.
     *
     * reverse_iterator iterates the underlying iterator in reverse order:
     * incrementing a reverse_iterator decrements the wrapped iterator.
     *
     * @tparam Iter The underlying iterator type
     *
     * \lang simp-chinese
     * @brief 反转双向迭代器方向的迭代器适配器。
     *
     * reverse_iterator 以相反顺序遍历底层迭代器：
     * 递增 reverse_iterator 会递减被包装的迭代器。
     *
     * @tparam Iter 底层迭代器类型
     */
    template <typename Iter>
    class reverse_iterator {
    public:
        using iterator_type = Iter;
        using iterator_traits = utility::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using pointer = typename iterator_traits::pointer;
        using const_pointer = const value_type *;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        constexpr reverse_iterator() noexcept = default;

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param rhs The reverse_iterator to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param rhs 要拷贝的 reverse_iterator
         */
        constexpr reverse_iterator(const reverse_iterator &) noexcept = default;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param rhs The reverse_iterator to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param rhs 要移动的 reverse_iterator
         */
        constexpr reverse_iterator(reverse_iterator &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @return Reference to this reverse_iterator
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @return 此 reverse_iterator 的引用
         */
        constexpr reverse_iterator &operator=(const reverse_iterator &) noexcept = default;

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @return Reference to this reverse_iterator
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @return 此 reverse_iterator 的引用
         */
        constexpr reverse_iterator &operator=(reverse_iterator &&) noexcept = default;

        /**
         * \lang english
         * @brief Destructor.
         *
         * \lang simp-chinese
         * @brief 析构函数。
         */
        RAINY_CONSTEXPR20 ~reverse_iterator() = default;

        /**
         * \lang english
         * @brief Constructs a reverse_iterator from an underlying iterator.
         *
         * @param current The underlying iterator to wrap
         *
         * \lang simp-chinese
         * @brief 从底层迭代器构造 reverse_iterator。
         *
         * @param current 要包装的底层迭代器
         */
        explicit constexpr reverse_iterator(iterator_type current) noexcept : current(current) {
        }

        /**
         * \lang english
         * @brief Returns the underlying iterator.
         *
         * @return The wrapped base iterator
         *
         * \lang simp-chinese
         * @brief 返回底层迭代器。
         *
         * @return 被包装的基础迭代器
         */
        constexpr iterator_type base() const noexcept {
            return current;
        }

        /**
         * \lang english
         * @brief Dereference operator.
         *
         * @return Reference to the element the reverse iterator points to
         *
         * \lang simp-chinese
         * @brief 解引用运算符。
         *
         * @return 反向迭代器所指向元素的引用
         */
        constexpr reference operator*() noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        /**
         * \lang english
         * @brief Dereference operator (const version).
         *
         * @return Const reference to the element the reverse iterator points to
         *
         * \lang simp-chinese
         * @brief 解引用运算符（常量版本）。
         *
         * @return 反向迭代器所指向元素的常量引用
         */
        constexpr const_reference operator*() const noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        /**
         * \lang english
         * @brief Arrow operator.
         *
         * @return Pointer to the element the reverse iterator points to
         *
         * \lang simp-chinese
         * @brief 箭头运算符。
         *
         * @return 指向反向迭代器所指向元素的指针
         */
        constexpr pointer operator->() noexcept {
            iterator_type tmp = current;
            --tmp;
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return tmp;
            } else {
                return tmp.operator->();
            }
        }

        /**
         * \lang english
         * @brief Arrow operator (const version).
         *
         * @return Const pointer to the element the reverse iterator points to
         *
         * \lang simp-chinese
         * @brief 箭头运算符（常量版本）。
         *
         * @return 指向反向迭代器所指向元素的常量指针
         */
        constexpr const_pointer operator->() const noexcept {
            iterator_type tmp = current;
            --tmp;
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return tmp;
            } else {
                return tmp.operator->();
            }
        }

        /**
         * \lang english
         * @brief Prefix increment operator (moves the reverse iterator backward).
         *
         * @return Reference to this iterator after increment
         *
         * \lang simp-chinese
         * @brief 前置自增运算符（反向迭代器向后移动）。
         *
         * @return 自增后此迭代器的引用
         */
        constexpr reverse_iterator &operator++() noexcept {
            --current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix increment operator.
         *
         * @return Copy of this iterator before increment
         *
         * \lang simp-chinese
         * @brief 后置自增运算符。
         *
         * @return 自增前此迭代器的副本
         */
        constexpr reverse_iterator operator++(int) noexcept {
            reverse_iterator tmp = *this;
            --current;
            return tmp;
        }

        /**
         * \lang english
         * @brief Prefix decrement operator (moves the reverse iterator forward).
         *
         * @return Reference to this iterator after decrement
         *
         * \lang simp-chinese
         * @brief 前置自减运算符（反向迭代器向前移动）。
         *
         * @return 自减后此迭代器的引用
         */
        constexpr reverse_iterator &operator--() noexcept {
            ++current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix decrement operator.
         *
         * @return Copy of this iterator before decrement
         *
         * \lang simp-chinese
         * @brief 后置自减运算符。
         *
         * @return 自减前此迭代器的副本
         */
        constexpr reverse_iterator operator--(int) noexcept {
            reverse_iterator tmp = *this;
            ++current;
            return tmp;
        }

        /**
         * \lang english
         * @brief Addition assignment operator.
         *
         * @param n Number of positions to advance
         * @return Reference to this iterator after advancement
         *
         * \lang simp-chinese
         * @brief 加法赋值运算符。
         *
         * @param n 前进的位置数
         * @return 前进后此迭代器的引用
         */
        constexpr reverse_iterator &operator+=(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        /**
         * \lang english
         * @brief Subtraction assignment operator.
         *
         * @param n Number of positions to move back
         * @return Reference to this iterator after moving back
         *
         * \lang simp-chinese
         * @brief 减法赋值运算符。
         *
         * @param n 后退的位置数
         * @return 后退后此迭代器的引用
         */
        constexpr reverse_iterator &operator-=(difference_type n) noexcept {
            current += n;
            return *this;
        }

        /**
         * \lang english
         * @brief Subscript operator.
         *
         * @param n The offset relative to the current position
         * @return Reference to the element at offset n
         *
         * \lang simp-chinese
         * @brief 下标运算符。
         *
         * @param n 相对于当前位置的偏移
         * @return 偏移 n 处元素的引用
         */
        constexpr reference operator[](difference_type n) noexcept {
            return current[-n - 1];
        }

        /**
         * \lang english
         * @brief Subscript operator (const version).
         *
         * @param n The offset relative to the current position
         * @return Const reference to the element at offset n
         *
         * \lang simp-chinese
         * @brief 下标运算符（常量版本）。
         *
         * @param n 相对于当前位置的偏移
         * @return 偏移 n 处元素的常量引用
         */
        constexpr const_reference operator[](difference_type n) const noexcept {
            return current[-n - 1];
        }

        /**
         * \lang english
         * @brief Checks whether the underlying iterator is non-null.
         *
         * @return true if the underlying iterator is non-null, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查底层迭代器是否非空。
         *
         * @return 如果底层迭代器非空则为 true，否则为 false
         */
        constexpr explicit operator bool() const noexcept {
            return current != nullptr;
        }

        /**
         * \lang english
         * @brief Advances the reverse iterator by n positions.
         *
         * @param it The reverse iterator to advance
         * @param n Number of positions to advance
         * @return The advanced reverse iterator
         *
         * \lang simp-chinese
         * @brief 将反向迭代器前进 n 个位置。
         *
         * @param it 要前进的反向迭代器
         * @param n 前进的位置数
         * @return 前进后的反向迭代器
         */
        friend constexpr reverse_iterator operator+(const reverse_iterator &it, difference_type n) {
            return reverse_iterator(it.current - n);
        }

        /**
         * \lang english
         * @brief Moves the reverse iterator back by n positions.
         *
         * @param it The reverse iterator to move
         * @param n Number of positions to move back
         * @return The moved reverse iterator
         *
         * \lang simp-chinese
         * @brief 将反向迭代器后退 n 个位置。
         *
         * @param it 要移动的反向迭代器
         * @param n 后退的位置数
         * @return 移动后的反向迭代器
         */
        friend constexpr reverse_iterator operator-(const reverse_iterator &it, difference_type n) {
            return reverse_iterator(it.current + n);
        }

        /**
         * \lang english
         * @brief Computes the distance between two reverse iterators.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return The distance between lhs and rhs
         *
         * \lang simp-chinese
         * @brief 计算两个反向迭代器之间的距离。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return lhs 与 rhs 之间的距离
         */
        friend constexpr difference_type operator-(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return rhs.current - lhs.current;
        }

        /**
         * \lang english
         * @brief Equality comparison operator.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return true if the iterators are equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 相等比较运算符。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return 如果迭代器相等则为 true，否则为 false
         */
        friend constexpr bool operator==(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current == rhs.current;
        }

        /**
         * \lang english
         * @brief Inequality comparison operator.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return true if the iterators are not equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 不等比较运算符。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return 如果迭代器不相等则为 true，否则为 false
         */
        friend constexpr bool operator!=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current != rhs.current;
        }

        /**
         * \lang english
         * @brief Less-than comparison operator.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return true if lhs precedes rhs, false otherwise
         *
         * \lang simp-chinese
         * @brief 小于比较运算符。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return 如果 lhs 在 rhs 之前则为 true，否则为 false
         */
        friend constexpr bool operator<(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current > rhs.current;
        }

        /**
         * \lang english
         * @brief Less-than-or-equal comparison operator.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return true if lhs does not follow rhs, false otherwise
         *
         * \lang simp-chinese
         * @brief 小于等于比较运算符。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return 如果 lhs 不在 rhs 之后则为 true，否则为 false
         */
        friend constexpr bool operator<=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current >= rhs.current;
        }

        /**
         * \lang english
         * @brief Greater-than comparison operator.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return true if lhs follows rhs, false otherwise
         *
         * \lang simp-chinese
         * @brief 大于比较运算符。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return 如果 lhs 在 rhs 之后则为 true，否则为 false
         */
        friend constexpr bool operator>(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current < rhs.current;
        }

        /**
         * \lang english
         * @brief Greater-than-or-equal comparison operator.
         *
         * @param lhs The left-hand side iterator
         * @param rhs The right-hand side iterator
         * @return true if lhs does not precede rhs, false otherwise
         *
         * \lang simp-chinese
         * @brief 大于等于比较运算符。
         *
         * @param lhs 左侧迭代器
         * @param rhs 右侧迭代器
         * @return 如果 lhs 不在 rhs 之前则为 true，否则为 false
         */
        friend constexpr bool operator>=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current <= rhs.current;
        }

    private:
        iterator_type current;
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Creates a reverse_iterator wrapping the given iterator.
     *
     * @tparam Iter The underlying iterator type
     * @param it The iterator to wrap
     * @return A reverse_iterator over the given iterator
     *
     * \lang simp-chinese
     * @brief 创建一个包装给定迭代器的 reverse_iterator。
     *
     * @tparam Iter 底层迭代器类型
     * @param it 要包装的迭代器
     * @return 包装给定迭代器的 reverse_iterator
     */
    template <typename Iter>
    constexpr reverse_iterator<Iter> make_reverse_iterator(Iter it) noexcept {
        return reverse_iterator<Iter>(it);
    }
}

#endif
