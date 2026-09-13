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
#ifndef RAINY_UTILITY_ITERATOR_HPP
#define RAINY_UTILITY_ITERATOR_HPP
#include <rainy/core/collections/views/views_interface.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/utility/reverse_iterator.hpp>

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::collections::views::implements {
    /**
     * \lang english
     * @brief An iterator adaptor that wraps an underlying iterator to expose a
     *        unified interface for iterator ranges.
     *
     *  Forwards all iterator operations to the wrapped iterator, providing the
     *  standard iterator typedefs derived from the underlying iterator's traits.
     *
     * @tparam Iter The underlying iterator type.
     *
     * \lang simp-chinese
     * @brief 包装底层迭代器以暴露统一接口的迭代器适配器，用于迭代器区间。
     *
     *  将所有迭代器操作转发给被包装的迭代器，并提供源自底层迭代器 traits 的
     *  标准迭代器 typedef。
     *
     * @tparam Iter 底层迭代器类型。
     */
    template <typename Iter>
    class iterator_range_iterator {
    public:
        using iterator_category = typename utility::iterator_traits<Iter>::iterator_category;
        using value_type = typename utility::iterator_traits<Iter>::value_type;
        using difference_type = typename utility::iterator_traits<Iter>::difference_type;
        using pointer = typename utility::iterator_traits<Iter>::pointer;
        using reference = typename utility::iterator_traits<Iter>::reference;

        /**
         * \lang english
         * @brief Constructs an iterator_range_iterator from an underlying iterator.
         *
         * @param iter The underlying iterator to wrap
         *
         * \lang simp-chinese
         * @brief 从底层迭代器构造iterator_range_iterator。
         *
         * @param iter 要包装的底层迭代器
         */
        iterator_range_iterator(Iter iter) : iter_{iter} {
        }

        /**
         * \lang english
         * @brief Dereference operator.
         *
         * @return Reference to the element pointed to by the underlying iterator
         *
         * \lang simp-chinese
         * @brief 解引用运算符。
         *
         * @return 底层迭代器所指向元素的引用
         */
        decltype(auto) operator*() const {
            return (*iter_);
        }

        /**
         * \lang english
         * @brief Arrow operator.
         *
         * @return Pointer to the element pointed to by the underlying iterator
         *
         * \lang simp-chinese
         * @brief 箭头运算符。
         *
         * @return 指向底层迭代器所指向元素的指针
         */
        pointer operator->() const {
            return utility::addressof(*iter_);
        }

        /**
         * \lang english
         * @brief Prefix increment operator.
         *
         * @return Reference to this iterator after increment
         *
         * \lang simp-chinese
         * @brief 前置自增运算符。
         *
         * @return 自增后此迭代器的引用
         */
        rain_fn operator++()->iterator_range_iterator & {
            ++iter_;
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
        rain_fn operator++(int) {
            iterator_range_iterator temp = *this;
            ++iter_;
            return temp;
        }

        /**
         * \lang english
         * @brief Prefix decrement operator.
         *
         * @return Reference to this iterator after decrement
         *
         * \lang simp-chinese
         * @brief 前置自减运算符。
         *
         * @return 自减后此迭代器的引用
         */
        rain_fn operator--() {
            --iter_;
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
        rain_fn operator--(int) {
            iterator_range_iterator temp = *this;
            --iter_;
            return temp;
        }

        /**
         * \lang english
         * @brief Addition operator.
         *
         * @param n Number of positions to advance
         * @return New iterator advanced by n positions
         *
         * \lang simp-chinese
         * @brief 加法运算符。
         *
         * @param n 前进的位置数
         * @return 前进n个位置后的新迭代器
         */
        rain_fn operator+(difference_type n) const->iterator_range_iterator {
            return iterator_range_iterator(iter_ + n);
        }

        /**
         * \lang english
         * @brief Subtraction operator.
         *
         * @param n Number of positions to move back
         * @return New iterator moved back by n positions
         *
         * \lang simp-chinese
         * @brief 减法运算符。
         *
         * @param n 后退的位置数
         * @return 后退n个位置后的新迭代器
         */
        rain_fn operator-(difference_type n) const->iterator_range_iterator {
            return iterator_range_iterator(iter_ - n);
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
        rain_fn &operator+=(difference_type n) {
            iter_ += n;
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
        rain_fn &operator-=(difference_type n) {
            iter_ -= n;
            return *this;
        }

        /**
         * \lang english
         * @brief Difference operator.
         *
         * @param other Another iterator to compare with
         * @return Distance between this iterator and other
         *
         * \lang simp-chinese
         * @brief 差运算符。
         *
         * @param other 要比较的另一个迭代器
         * @return 此迭代器与other之间的距离
         */
        rain_fn operator-(const iterator_range_iterator &other) const->difference_type {
            return iter_ - other.iter_;
        }

        /**
         * \lang english
         * @brief Equality comparison operator.
         *
         * @param left Left-hand side iterator
         * @param right Right-hand side iterator
         * @return true if iterators are equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 相等比较运算符。
         *
         * @param left 左侧迭代器
         * @param right 右侧迭代器
         * @return 如果迭代器相等则为true，否则为false
         */
        friend bool operator==(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept {
            return left.iter_ == right.iter_;
        }

        /**
         * \lang english
         * @brief Inequality comparison operator.
         *
         * @param left Left-hand side iterator
         * @param right Right-hand side iterator
         * @return true if iterators are not equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 不等比较运算符。
         *
         * @param left 左侧迭代器
         * @param right 右侧迭代器
         * @return 如果迭代器不相等则为true，否则为false
         */
        friend bool operator!=(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept {
            return !(left.iter_ == right.iter_);
        }

    private:
        Iter iter_;
    };

    /**
     * \lang english
     * @brief Adapts an iterator pair into a view with begin/end accessors.
     *
     * @tparam Iter The underlying iterator type.
     *
     * \lang simp-chinese
     * @brief 将一对迭代器适配为具有 begin/end 访问器的视图。
     *
     * @tparam Iter 底层迭代器类型。
     */
    template <typename Iter>
    class adapter_iterator_range : public views::view_interface<adapter_iterator_range<Iter>> {
    public:
        using iterator = iterator_range_iterator<Iter>;
        using const_iterator = const iterator_range_iterator<const Iter>;
        using reference = type_traits::extras::iterators::iterator_reference_t<iterator>;
        using const_reference = type_traits::modifers::add_const_t<reference>;
        using difference_type = type_traits::extras::iterators::iterator_difference_t<const_iterator>;
        using value_type = type_traits::extras::iterators::iter_value_t<const_iterator>;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        adapter_iterator_range() : begin_{}, end_{} {
        }

        /**
         * \lang english
         * @brief Constructs an adapter_iterator_range from begin and end iterators.
         *
         * @param begin Iterator to the beginning of the range
         * @param end Iterator to the end of the range
         *
         * \lang simp-chinese
         * @brief 从开始和结束迭代器构造adapter_iterator_range。
         *
         * @param begin 指向范围起始的迭代器
         * @param end 指向范围末尾的迭代器
         */
        adapter_iterator_range(Iter begin, Iter end) : begin_{begin}, end_{end} {
        }

        /**
         * \lang english
         * @brief Returns a const reference to the base range.
         *
         * @return Const reference to this adapter_iterator_range
         *
         * \lang simp-chinese
         * @brief 返回基范围的常量引用。
         *
         * @return 此adapter_iterator_range的常量引用
         */
        RAINY_NODISCARD constexpr rain_fn base() const & noexcept -> const adapter_iterator_range & {
            return *this;
        }

        /**
         * \lang english
         * @brief Returns an rvalue reference to the base range.
         *
         * @return Rvalue reference to this adapter_iterator_range
         *
         * \lang simp-chinese
         * @brief 返回基范围的右值引用。
         *
         * @return 此adapter_iterator_range的右值引用
         */
        RAINY_NODISCARD constexpr rain_fn base() && noexcept -> adapter_iterator_range {
            return utility::move(*this);
        }

        /**
         * \lang english
         * @brief Returns an iterator to the beginning.
         *
         * @return Iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的迭代器。
         *
         * @return 指向第一个元素的迭代器
         */
        rain_fn begin() noexcept -> iterator {
            return iterator{begin_};
        }

        /**
         * \lang english
         * @brief Returns an iterator to the end.
         *
         * @return Iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的迭代器。
         *
         * @return 指向最后一个元素之后位置的迭代器
         */
        rain_fn end() noexcept -> iterator {
            return iterator{end_};
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning.
         *
         * @return Const iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量迭代器。
         *
         * @return 指向第一个元素的常量迭代器
         */
        rain_fn begin() const noexcept -> const_iterator {
            return const_iterator{begin_};
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the end.
         *
         * @return Const iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量迭代器。
         *
         * @return 指向最后一个元素之后位置的常量迭代器
         */
        rain_fn end() const noexcept -> const_iterator {
            return const_iterator{end_};
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning (explicit).
         *
         * @return Const iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量迭代器（显式）。
         *
         * @return 指向第一个元素的常量迭代器
         */
        rain_fn cbegin() const noexcept -> const_iterator {
            return const_iterator{begin_};
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the end (explicit).
         *
         * @return Const iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量迭代器（显式）。
         *
         * @return 指向最后一个元素之后位置的常量迭代器
         */
        rain_fn cend() const noexcept -> const_iterator {
            return const_iterator{end_};
        }

    private:
        Iter begin_;
        Iter end_;
    };
}

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief A range view over an iterator pair.
     *
     *  This class provides a view over a range defined by a pair of iterators,
     *  inheriting all functionality from adapter_iterator_range.
     *
     * @tparam Iter The underlying iterator type
     *
     * \lang simp-chinese
     * @brief 基于迭代器对的区间视图。
     *
     *  此类提供由一对迭代器定义的区间的视图，
     *  继承自 adapter_iterator_range 的所有功能。
     *
     * @tparam Iter 底层迭代器类型
     */
    template <typename Iter>
    class iterator_range : public implements::adapter_iterator_range<Iter> {
    public:
        using base = implements::adapter_iterator_range<Iter>;

        /**
         * \lang english
         * @brief Constructs an iterator_range from begin and end iterators.
         *
         * @param begin Iterator to the beginning of the range
         * @param end Iterator to the end of the range
         *
         * \lang simp-chinese
         * @brief 从开始和结束迭代器构造 iterator_range。
         *
         * @param begin 指向范围起始的迭代器
         * @param end 指向范围末尾的迭代器
         */
        iterator_range(Iter begin, Iter end) : base(begin, end) {
        }
    };
}

namespace rainy::collections::views {
    using core::collections::views::iterator_range;
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief A minimal pointer-like wrapper that owns a value and exposes it via
     *        operator-> and operator*.
     *
     * @tparam Ty The owned value type.
     *
     * \lang simp-chinese
     * @brief 一种最小化的类指针包装器，持有值并通过 operator-> 与 operator* 暴露它。
     *
     * @tparam Ty 所持有的值类型。
     */
    template <typename Ty>
    struct input_iterator_pointer final {
        using value_type = Ty;
        using pointer = Ty *;
        using reference = Ty &;

        /**
         * \lang english
         * @brief Constructs the wrapper by moving the given value into it.
         *
         * @param val The value to own.
         *
         * \lang simp-chinese
         * @brief 通过将给定值移动到其中来构造该包装器。
         *
         * @param val 要持有的值。
         */
        constexpr input_iterator_pointer(value_type &&val) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<value_type>) : value{utility::move(val)} {
        }

        /**
         * \lang english
         * @brief Returns a pointer to the owned value.
         *
         * @return A pointer to the owned value.
         *
         * \lang simp-chinese
         * @brief 返回指向所持有值的指针。
         *
         * @return 指向所持有值的指针。
         */
        RAINY_NODISCARD constexpr pointer operator->() noexcept {
            return utility::addressof(value);
        }

        /**
         * \lang english
         * @brief Returns a reference to the owned value.
         *
         * @return A reference to the owned value.
         *
         * \lang simp-chinese
         * @brief 返回所持有值的引用。
         *
         * @return 所持有值的引用。
         */
        RAINY_NODISCARD constexpr reference operator*() noexcept {
            return value;
        }

    private:
        Ty value;
    };

    /**
     * \lang english
     * @brief A const iterator adaptor wrapping an underlying iterator.
     *
     *  Presents the wrapped iterator as a const iterator, providing the standard
     *  iterator operations (dereference, increment, decrement, arithmetic,
     *  comparison) and the const value_type / pointer / reference aliases.
     *
     * @tparam Iter The underlying iterator type.
     * @tparam Traits The iterator traits of Iter.
     *
     * \lang simp-chinese
     * @brief 包装底层迭代器的常量迭代器适配器。
     *
     *  将所包装的迭代器呈现为常量迭代器，提供标准迭代器操作（解引用、自增、
     *  自减、算术、比较）以及常量 value_type / pointer / reference 别名。
     *
     * @tparam Iter 底层迭代器类型。
     * @tparam Traits Iter 的迭代器 traits。
     */
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class const_iterator {
    public:
        template <typename, typename>
        friend class iterator;

        friend struct core::memory::pointer_traits<const_iterator<Iter, Traits>>;

        using iterator_type = Iter;
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        /**
         * \lang english
         * @brief Default constructor. Leaves the underlying iterator value-initialized.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。将底层迭代器值初始化。
         */
        constexpr const_iterator() noexcept : current() {};

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         */
        constexpr const_iterator &operator=(const_iterator &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         */
        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;

        /**
         * \lang english
         * @brief Constructs a const iterator.
         *
         * @param current The current iterator position
         *
         * \lang simp-chinese
         * @brief 构造一个常量迭代器
         * @param current 当前迭代器位置
         */
        explicit constexpr const_iterator(iterator_type current) noexcept : current(current) {
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param right The iterator to copy.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param right 要拷贝的迭代器。
         */
        constexpr const_iterator(const const_iterator &right) : current(right.current) {
        }

        /**
         * \lang english
         * @brief Move constructor. Leaves the source iterator in a null state.
         *
         * @param right The iterator to move from.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。将源迭代器置为空状态。
         *
         * @param right 要移动的迭代器。
         */
        constexpr const_iterator(const_iterator &&right) noexcept : current(rainy::utility::exchange(right.current, nullptr)) {
        }

        RAINY_CONSTEXPR20 ~const_iterator() = default;

        /**
         * \lang english
         * @brief Retrieves the underlying pointer.
         *
         * @return The underlying pointer.
         *
         * \lang simp-chinese
         * @brief 获取底层指针。
         *
         * @return 底层指针。
         */
        constexpr const_pointer get_pointer() const noexcept {
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return current;
            } else {
                return current.operator->();
            }
        }

        /**
         * \lang english
         * @brief Arrow operator.
         *
         * @return The underlying pointer.
         *
         * \lang simp-chinese
         * @brief 箭头运算符。
         *
         * @return 底层指针。
         */
        constexpr const_pointer operator->() const noexcept {
            return get_pointer();
        }

        /**
         * \lang english
         * @brief Dereferences the iterator.
         *
         * @return A const reference to the pointed-to element.
         *
         * \lang simp-chinese
         * @brief 解引用该迭代器。
         *
         * @return 指向元素的常量引用。
         */
        constexpr const_reference dereference() const noexcept {
            return *current;
        }

        /**
         * \lang english
         * @brief Dereference operator.
         *
         * @return A const reference to the pointed-to element.
         *
         * \lang simp-chinese
         * @brief 解引用运算符。
         *
         * @return 指向元素的常量引用。
         */
        constexpr const_reference operator*() const noexcept {
            return dereference();
        }

        /**
         * \lang english
         * @brief Prefix increment helper; advances the iterator and returns *this.
         *
         * @return Reference to this iterator.
         *
         * \lang simp-chinese
         * @brief 前置自增辅助函数；前进迭代器并返回 *this。
         *
         * @return 此迭代器的引用。
         */
        constexpr const_iterator &self_inc_prefix() noexcept {
            ++current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix increment helper; returns a copy and advances.
         *
         * @return Copy of this iterator before the increment.
         *
         * \lang simp-chinese
         * @brief 后置自增辅助函数；返回副本并前进。
         *
         * @return 自增前此迭代器的副本。
         */
        constexpr const_iterator self_inc_postfix() noexcept {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * \lang english
         * @brief Prefix increment operator.
         *
         * @return Reference to this iterator after increment.
         *
         * \lang simp-chinese
         * @brief 前置自增运算符。
         *
         * @return 自增后此迭代器的引用。
         */
        constexpr const_iterator &operator++() noexcept {
            return self_inc_prefix();
        }

        /**
         * \lang english
         * @brief Postfix increment operator.
         *
         * @return Copy of this iterator before increment.
         *
         * \lang simp-chinese
         * @brief 后置自增运算符。
         *
         * @return 自增前此迭代器的副本。
         */
        constexpr const_iterator operator++(int) noexcept {
            return self_inc_postfix();
        }

        /**
         * \lang english
         * @brief Prefix decrement helper; moves the iterator back and returns *this.
         *
         * @return Reference to this iterator.
         *
         * \lang simp-chinese
         * @brief 前置自减辅助函数；后退迭代器并返回 *this。
         *
         * @return 此迭代器的引用。
         */
        constexpr const_iterator &self_dec_prefix() noexcept {
            --current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix decrement helper; returns a copy and moves back.
         *
         * @return Copy of this iterator before the decrement.
         *
         * \lang simp-chinese
         * @brief 后置自减辅助函数；返回副本并后退。
         *
         * @return 自减前此迭代器的副本。
         */
        constexpr const_iterator self_dec_postfix() noexcept {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        /**
         * \lang english
         * @brief Prefix decrement operator.
         *
         * @return Reference to this iterator after decrement.
         *
         * \lang simp-chinese
         * @brief 前置自减运算符。
         *
         * @return 自减后此迭代器的引用。
         */
        constexpr const_iterator &operator--() noexcept {
            return self_dec_prefix();
        }

        /**
         * \lang english
         * @brief Postfix decrement operator.
         *
         * @return Copy of this iterator before decrement.
         *
         * \lang simp-chinese
         * @brief 后置自减运算符。
         *
         * @return 自减前此迭代器的副本。
         */
        constexpr const_iterator operator--(int) noexcept {
            return self_dec_postfix();
        }

        /**
         * \lang english
         * @brief Advances the iterator by n positions and returns *this.
         *
         * @param n Number of positions to advance.
         * @return Reference to this iterator.
         *
         * \lang simp-chinese
         * @brief 将迭代器前进 n 个位置并返回 *this。
         *
         * @param n 前进的位置数。
         * @return 此迭代器的引用。
         */
        constexpr const_iterator &plus_equal(difference_type n) noexcept {
            current += n;
            return *this;
        }

        /**
         * \lang english
         * @brief Moves the iterator back by n positions and returns *this.
         *
         * @param n Number of positions to move back.
         * @return Reference to this iterator.
         *
         * \lang simp-chinese
         * @brief 将迭代器后退 n 个位置并返回 *this。
         *
         * @param n 后退的位置数。
         * @return 此迭代器的引用。
         */
        constexpr const_iterator &minus_equal(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        /**
         * \lang english
         * @brief Addition assignment operator.
         *
         * @param n Number of positions to advance.
         * @return Reference to this iterator.
         *
         * \lang simp-chinese
         * @brief 加法赋值运算符。
         *
         * @param n 前进的位置数。
         * @return 此迭代器的引用。
         */
        constexpr const_iterator &operator+=(difference_type n) noexcept {
            return plus_equal(n);
        }

        /**
         * \lang english
         * @brief Subtraction assignment operator.
         *
         * @param n Number of positions to move back.
         * @return Reference to this iterator.
         *
         * \lang simp-chinese
         * @brief 减法赋值运算符。
         *
         * @param n 后退的位置数。
         * @return 此迭代器的引用。
         */
        constexpr const_iterator &operator-=(difference_type n) noexcept {
            return minus_equal(n);
        }

        /**
         * \lang english
         * @brief Checks whether the underlying iterators are equal.
         *
         * @param right The iterator to compare with.
         * @return true if equal.
         *
         * \lang simp-chinese
         * @brief 检查底层迭代器是否相等。
         *
         * @param right 要比较的迭代器。
         * @return 若相等则为 true。
         */
        constexpr bool is_equal(const const_iterator &right) const noexcept {
            return current == right.current;
        }

        /**
         * \lang english
         * @brief Checks whether the underlying iterators are not equal.
         *
         * @param right The iterator to compare with.
         * @return true if not equal.
         *
         * \lang simp-chinese
         * @brief 检查底层迭代器是否不相等。
         *
         * @param right 要比较的迭代器。
         * @return 若不相等则为 true。
         */
        constexpr bool not_equal(const const_iterator &right) const noexcept {
            return current != right.current;
        }

        /**
         * \lang english
         * @brief Equality comparison operator.
         *
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if iterators are equal.
         *
         * \lang simp-chinese
         * @brief 相等比较运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若迭代器相等则为 true。
         */
        constexpr friend bool operator==(const const_iterator &left, const const_iterator &right) {
            return left.is_equal(right);
        }

        /**
         * \lang english
         * @brief Inequality comparison operator.
         *
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if iterators are not equal.
         *
         * \lang simp-chinese
         * @brief 不等比较运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若迭代器不相等则为 true。
         */
        constexpr friend bool operator!=(const const_iterator &left, const const_iterator &right) {
            return left.not_equal(right);
        }

        /**
         * \lang english
         * @brief Subscript access helper.
         *
         * @param idx The index.
         * @return A const reference to the element at idx.
         *
         * \lang simp-chinese
         * @brief 下标访问辅助函数。
         *
         * @param idx 索引。
         * @return 位于 idx 处元素的常量引用。
         */
        constexpr const_reference at_subscript(difference_type idx) const noexcept {
            return current[idx];
        }

        /**
         * \lang english
         * @brief Subscript operator.
         *
         * @param idx The index.
         * @return A const reference to the element at idx.
         *
         * \lang simp-chinese
         * @brief 下标运算符。
         *
         * @param idx 索引。
         * @return 位于 idx 处元素的常量引用。
         */
        constexpr const_reference operator[](difference_type idx) const noexcept {
            return at_subscript(idx);
        }

        /**
         * \lang english
         * @brief Checks whether the iterator is empty (i.e. the underlying iterator is null).
         *
         * @return true if empty.
         *
         * \lang simp-chinese
         * @brief 检查迭代器是否为空（即底层迭代器为 null）。
         *
         * @return 若为空则为 true。
         */
        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept {
            return this->current == nullptr;
        }

        /**
         * \lang english
         * @brief Checks whether the iterator is non-empty.
         *
         * @return true if non-empty.
         *
         * \lang simp-chinese
         * @brief 检查迭代器是否非空。
         *
         * @return 若非空则为 true。
         */
        constexpr explicit operator bool() const noexcept {
            return !empty();
        }

        /**
         * \lang english
         * @brief Returns a copy of the iterator advanced by n positions.
         *
         * @param n Number of positions to advance.
         * @return A new iterator.
         *
         * \lang simp-chinese
         * @brief 返回前进 n 个位置后的迭代器副本。
         *
         * @param n 前进的位置数。
         * @return 新的迭代器。
         */
        constexpr const_iterator add_offset(difference_type n) const noexcept {
            return const_iterator{current + n};
        }

        /**
         * \lang english
         * @brief Addition operator.
         *
         * @param left The iterator.
         * @param n Number of positions to advance.
         * @return A new iterator advanced by n positions.
         *
         * \lang simp-chinese
         * @brief 加法运算符。
         *
         * @param left 迭代器。
         * @param n 前进的位置数。
         * @return 前进 n 个位置后的新迭代器。
         */
        constexpr friend const_iterator operator+(const const_iterator left, difference_type n) {
            return left.add_offset(n);
        }

        /**
         * \lang english
         * @brief Subtracts n from the iterator position.
         *
         * @param n Number of positions to subtract.
         * @return The resulting position.
         *
         * \lang simp-chinese
         * @brief 从迭代器位置减去 n。
         *
         * @param n 要减去的位数。
         * @return 结果位置。
         */
        constexpr difference_type subtract(difference_type n) const noexcept {
            return current - n;
        }

        /**
         * \lang english
         * @brief Computes the distance between two iterators.
         *
         * @param right The other iterator.
         * @return The distance.
         *
         * \lang simp-chinese
         * @brief 计算两个迭代器之间的距离。
         *
         * @param right 另一个迭代器。
         * @return 距离。
         */
        constexpr difference_type subtract(const const_iterator &right) const noexcept {
            return current - right.current;
        }

        /**
         * \lang english
         * @brief Subtraction operator between two iterators.
         *
         * @param left The left iterator.
         * @param right The right iterator.
         * @return The distance.
         *
         * \lang simp-chinese
         * @brief 两个迭代器之间的减法运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 距离。
         */
        constexpr friend difference_type operator-(const const_iterator &left, const const_iterator &right) {
            return left.subtract(right);
        }

        /**
         * \lang english
         * @brief Subtraction operator between an iterator and an offset.
         *
         * @param left The iterator.
         * @param n The offset.
         * @return The resulting position.
         *
         * \lang simp-chinese
         * @brief 迭代器与偏移之间的减法运算符。
         *
         * @param left 迭代器。
         * @param n 偏移量。
         * @return 结果位置。
         */
        constexpr friend difference_type operator-(const const_iterator &left, difference_type n) {
            return left.subtract(n);
        }

        /**
         * \lang english
         * @brief Less-than comparison helper.
         *
         * @param right The other iterator.
         * @return true if this iterator is less than right.
         *
         * \lang simp-chinese
         * @brief 小于比较辅助函数。
         *
         * @param right 另一个迭代器。
         * @return 若此迭代器小于 right 则为 true。
         */
        constexpr bool lt(const const_iterator &right) const noexcept {
            return current < right.current;
        }

        /**
         * \lang english
         * @brief Less-than comparison operator.
         *
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if left is less than right.
         *
         * \lang simp-chinese
         * @brief 小于比较运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若 left 小于 right 则为 true。
         */
        constexpr friend bool operator<(const const_iterator &left, const const_iterator &right) {
            return left.lt(right);
        }

        /**
         * \lang english
         * @brief Less-or-equal comparison helper.
         *
         * @param right The other iterator.
         * @return true if this iterator is less than or equal to right.
         *
         * \lang simp-chinese
         * @brief 小于等于比较辅助函数。
         *
         * @param right 另一个迭代器。
         * @return 若此迭代器小于等于 right 则为 true。
         */
        constexpr bool lt_or_equal(const const_iterator &right) const noexcept {
            return current <= right.current;
        }

        /**
         * \lang english
         * @brief Less-or-equal comparison operator.
         *
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if left is less than or equal to right.
         *
         * \lang simp-chinese
         * @brief 小于等于比较运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若 left 小于等于 right 则为 true。
         */
        constexpr friend bool operator<=(const const_iterator &left, const const_iterator &right) {
            return left.lt_or_equal(right);
        }

        /**
         * \lang english
         * @brief Greater-than comparison helper.
         *
         * @param right The other iterator.
         * @return true if this iterator is greater than right.
         *
         * \lang simp-chinese
         * @brief 大于比较辅助函数。
         *
         * @param right 另一个迭代器。
         * @return 若此迭代器大于 right 则为 true。
         */
        constexpr bool greater_than(const const_iterator &right) const noexcept {
            return current > right.current;
        }

        /**
         * \lang english
         * @brief Greater-than comparison operator.
         *
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if left is greater than right.
         *
         * \lang simp-chinese
         * @brief 大于比较运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若 left 大于 right 则为 true。
         */
        constexpr friend bool operator>(const const_iterator &left, const const_iterator &right) {
            return left.greater_than(right);
        }

        /**
         * \lang english
         * @brief Greater-or-equal comparison helper.
         *
         * @param right The other iterator.
         * @return true if this iterator is greater than or equal to right.
         *
         * \lang simp-chinese
         * @brief 大于等于比较辅助函数。
         *
         * @param right 另一个迭代器。
         * @return 若此迭代器大于等于 right 则为 true。
         */
        constexpr bool greater_than_or_equal(const const_iterator &right) const noexcept {
            return current >= right.current;
        }

        /**
         * \lang english
         * @brief Greater-or-equal comparison operator.
         *
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if left is greater than or equal to right.
         *
         * \lang simp-chinese
         * @brief 大于等于比较运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若 left 大于等于 right 则为 true。
         */
        constexpr friend bool operator>=(const const_iterator &left, const const_iterator &right) {
            return left.greater_than_or_equal(right);
        }

    private:
        iterator_type current;
    };
}

template <typename Iter, typename Traits>
struct rainy::core::memory::pointer_traits<rainy::utility::const_iterator<Iter, Traits>> {
    using pointer = typename rainy::utility::const_iterator<Iter, Traits>;
    using elemen_type = typename pointer::value_type;
    using difference_type = typename pointer::difference_type;

    /**
     * \lang english
     * @brief Converts an iterator to a raw pointer via the wrapped iterator.
     *
     * @param val The iterator to convert.
     * @return A raw pointer to the referenced element.
     *
     * \lang simp-chinese
     * @brief 通过被包装的迭代器将迭代器转换为原始指针。
     *
     * @param val 要转换的迭代器。
     * @return 指向被引用元素的原始指针。
     */
    RAINY_NODISCARD static RAINY_CONSTEXPR20 elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
    /**
     * \lang english
     * @brief A mutable iterator adaptor wrapping an underlying iterator.
     *
     *  Derives from const_iterator and adds the mutable dereference, arrow, and
     *  arithmetic operations, allowing the wrapped iterator to be used as a mutable
     *  iterator.
     *
     * @tparam Iter The underlying iterator type.
     * @tparam Traits The iterator traits of Iter.
     *
     * \lang simp-chinese
     * @brief 包装底层迭代器的可变迭代器适配器。
     *
     *  派生自 const_iterator 并新增可变解引用、箭头和算术操作，使所包装的迭代器
     *  可作为可变迭代器使用。
     *
     * @tparam Iter 底层迭代器类型。
     * @tparam Traits Iter 的迭代器 traits。
     */
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class iterator : public const_iterator<Iter, Traits> {
    public:
        using iterator_type = Iter;
        using iterator_traits = Traits;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;

        using base = const_iterator<Iter, Traits>;

        friend struct core::memory::pointer_traits<const_iterator<Iter, Traits>>;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        constexpr iterator() noexcept : base() {};

        /**
         * \lang english
         * @brief Constructs the mutable iterator from an underlying iterator.
         *
         * @param current The underlying iterator.
         *
         * \lang simp-chinese
         * @brief 从底层迭代器构造可变迭代器。
         *
         * @param current 底层迭代器。
         */
        constexpr iterator(iterator_type current) noexcept : base(current) {
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param right The iterator to copy.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param right 要拷贝的迭代器。
         */
        constexpr iterator(const iterator &right) : base(right) {
        }

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param right The iterator to move from.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param right 要移动的迭代器。
         */
        constexpr iterator(iterator &&right) noexcept : base(right) {
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         */
        constexpr iterator &operator=(iterator &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         */
        constexpr iterator &operator=(const iterator &) noexcept = default;

        RAINY_CONSTEXPR20 ~iterator() = default;

        /**
         * \lang english
         * @brief Returns a reference to the base const_iterator.
         *
         * @return A reference to the base const_iterator.
         *
         * \lang simp-chinese
         * @brief 返回基类 const_iterator 的引用。
         *
         * @return 基类 const_iterator 的引用。
         */
        constexpr base &as_const_iterator() const noexcept {
            return *this;
        }

        /**
         * \lang english
         * @brief Retrieves the underlying mutable pointer.
         *
         * @return The underlying mutable pointer.
         *
         * \lang simp-chinese
         * @brief 获取底层可变指针。
         *
         * @return 底层可变指针。
         */
        constexpr pointer get_pointer() noexcept {
            return const_cast<pointer>(static_cast<const base *>(this)->get_pointer());
        }

        /**
         * \lang english
         * @brief Arrow operator.
         *
         * @return The underlying mutable pointer.
         *
         * \lang simp-chinese
         * @brief 箭头运算符。
         *
         * @return 底层可变指针。
         */
        constexpr pointer operator->() noexcept {
            return get_pointer();
        }

        /**
         * \lang english
         * @brief Dereferences the iterator.
         *
         * @return A mutable reference to the pointed-to element.
         *
         * \lang simp-chinese
         * @brief 解引用该迭代器。
         *
         * @return 指向元素的可变引用。
         */
        constexpr reference dereference() noexcept {
            return const_cast<reference>(static_cast<const base *>(this)->dereference());
        }

        /**
         * \lang english
         * @brief Dereference operator.
         *
         * @return A mutable reference to the pointed-to element.
         *
         * \lang simp-chinese
         * @brief 解引用运算符。
         *
         * @return 指向元素的可变引用。
         */
        constexpr reference operator*() noexcept {
            return dereference();
        }

        /**
         * \lang english
         * @brief Returns a copy of the iterator advanced by n positions.
         *
         * @param n Number of positions to advance.
         * @return A new iterator.
         *
         * \lang simp-chinese
         * @brief 返回前进 n 个位置后的迭代器副本。
         *
         * @param n 前进的位置数。
         * @return 新的迭代器。
         */
        constexpr iterator add_offset(typename base::difference_type n) const noexcept {
            return iterator{this->current + n};
        }

        /**
         * \lang english
         * @brief Addition operator.
         *
         * @param left The iterator.
         * @param n Number of positions to advance.
         * @return A new iterator advanced by n positions.
         *
         * \lang simp-chinese
         * @brief 加法运算符。
         *
         * @param left 迭代器。
         * @param n 前进的位置数。
         * @return 前进 n 个位置后的新迭代器。
         */
        constexpr friend iterator operator+(const iterator left, typename base::difference_type n) {
            return left.add_offset(n);
        }

        /**
         * \lang english
         * @brief Subtracts n from the iterator position.
         *
         * @param n Number of positions to subtract.
         * @return The resulting position.
         *
         * \lang simp-chinese
         * @brief 从迭代器位置减去 n。
         *
         * @param n 要减去的位数。
         * @return 结果位置。
         */
        constexpr typename base::difference_type subtract(typename base::difference_type n) const noexcept {
            return this->current - n;
        }

        /**
         * \lang english
         * @brief Computes the distance between two iterators.
         *
         * @param right The other iterator.
         * @return The distance.
         *
         * \lang simp-chinese
         * @brief 计算两个迭代器之间的距离。
         *
         * @param right 另一个迭代器。
         * @return 距离。
         */
        constexpr typename base::difference_type subtract(const iterator &right) const noexcept {
            return this->current - right.current;
        }

        /**
         * \lang english
         * @brief Prefix increment operator.
         *
         * @return Reference to this iterator after increment.
         *
         * \lang simp-chinese
         * @brief 前置自增运算符。
         *
         * @return 自增后此迭代器的引用。
         */
        constexpr iterator &operator++() noexcept {
            ++this->current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix increment operator.
         *
         * @return Copy of this iterator before increment.
         *
         * \lang simp-chinese
         * @brief 后置自增运算符。
         *
         * @return 自增前此迭代器的副本。
         */
        constexpr iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * \lang english
         * @brief Prefix decrement operator.
         *
         * @return Reference to this iterator after decrement.
         *
         * \lang simp-chinese
         * @brief 前置自减运算符。
         *
         * @return 自减后此迭代器的引用。
         */
        constexpr iterator &operator--() noexcept {
            --this->current;
            return *this;
        }

        /**
         * \lang english
         * @brief Postfix decrement operator.
         *
         * @return Copy of this iterator before decrement.
         *
         * \lang simp-chinese
         * @brief 后置自减运算符。
         *
         * @return 自减前此迭代器的副本。
         */
        constexpr iterator operator--(int) noexcept {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        /**
         * \lang english
         * @brief Subtraction operator between two iterators.
         *
         * @param left The left iterator.
         * @param right The right iterator.
         * @return The distance.
         *
         * \lang simp-chinese
         * @brief 两个迭代器之间的减法运算符。
         *
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 距离。
         */
        constexpr friend typename base::difference_type operator-(const iterator &left, const iterator &right) {
            return left.subtract(right);
        }

        /**
         * \lang english
         * @brief Subtraction operator between an iterator and an offset.
         *
         * @param left The iterator.
         * @param n The offset.
         * @return The resulting position.
         *
         * \lang simp-chinese
         * @brief 迭代器与偏移之间的减法运算符。
         *
         * @param left 迭代器。
         * @param n 偏移量。
         * @return 结果位置。
         */
        constexpr friend typename base::difference_type operator-(const iterator &left, typename base::difference_type n) {
            return left.subtract(n);
        }
    };
}

template <typename Iter, typename Traits>
struct rainy::core::memory::pointer_traits<rainy::utility::iterator<Iter, Traits>> {
    using pointer = typename rainy::utility::iterator<Iter, Traits>;
    using elemen_type = typename pointer::value_type;
    using difference_type = typename pointer::difference_type;

    /**
     * \lang english
     * @brief Converts an iterator to a raw pointer via the wrapped iterator.
     *
     * @param val The iterator to convert.
     * @return A raw pointer to the referenced element.
     *
     * \lang simp-chinese
     * @brief 通过被包装的迭代器将迭代器转换为原始指针。
     *
     * @param val 要转换的迭代器。
     * @return 指向被引用元素的原始指针。
     */
    RAINY_NODISCARD static constexpr elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
    /**
     * \lang english
     * @brief Extracts the iterator_category type from an iterator type.
     *
     * @tparam Iter The iterator type.
     *
     * \lang simp-chinese
     * @brief 从迭代器类型中提取 iterator_category 类型。
     *
     * @tparam Iter 迭代器类型。
     */
    template <typename Iter>
    struct iterator_category {
        using type = typename Iter::iterator_category;
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename = void>
    static RAINY_CONSTEXPR_BOOL is_proxy_equal_with_noexcept = false;

    template <typename Ty>
    static RAINY_CONSTEXPR_BOOL is_proxy_equal_with_noexcept<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().proxy_equal_with_helper(
                utility::declval<const Ty &>()))>> =
        noexcept(utility::declval<const Ty &>().proxy_equal_with_helper(utility::declval<const Ty &>()));
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief A CRTP base class for bidirectional iterators.
     *
     *  The derived class (Implement) provides get_element_impl, get_pointer_impl,
     *  next_impl, back_impl and equal_with_impl; this base forwards the standard
     *  iterator operators (dereference, arrow, increment, decrement, equality) to
     *  those implementations.
     *
     * @tparam Implement The derived iterator implementation type.
     * @tparam Traits The iterator traits describing the iterator.
     *
     * \lang simp-chinese
     * @brief 双向迭代器的 CRTP 基类。
     *
     *  派生类（Implement）提供 get_element_impl、get_pointer_impl、next_impl、
     *  back_impl 与 equal_with_impl；本基类将标准迭代器运算符（解引用、箭头、
     *  自增、自减、相等）转发给这些实现。
     *
     * @tparam Implement 派生迭代器实现类型。
     * @tparam Traits 描述该迭代器的迭代器 traits。
     */
    template <typename Implement, typename Traits>
    class bidirectional_iterator {
    public:
        using implement_type = Implement;
        /* 由于编译器的特性，可能无法找到Implement定义的类型，因此，我们此处需要用户提供驱动此迭代器工作用的iterator_traits */
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using pointer = value_type &;
        using reference = value_type &;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;
#if RAINY_HAS_CXX20
        using iterator_concept = typename iterator_traits::iterator_category;
#endif

        template <typename UImplement, typename UTraits>
        friend class bidirectional_iterator;

        /**
         * \lang english
         * @brief Dereference operator (mutable).
         * @return The result of the derived get_element_impl().
         *
         * \lang simp-chinese
         * @brief 解引用运算符（可变）。
         * @return 派生类 get_element_impl() 的结果。
         */
        RAINY_CONSTEXPR20 decltype(auto) operator*() noexcept(noexcept(static_cast<implement_type *>(this)->get_element_impl())) {
            return static_cast<implement_type *>(this)->get_element_impl();
        }

        /**
         * \lang english
         * @brief Arrow operator (mutable).
         * @return The result of the derived get_pointer_impl().
         *
         * \lang simp-chinese
         * @brief 箭头运算符（可变）。
         * @return 派生类 get_pointer_impl() 的结果。
         */
        RAINY_CONSTEXPR20 decltype(auto) operator->() noexcept(noexcept(static_cast<implement_type *>(this)->get_pointer_impl())) {
            return static_cast<implement_type *>(this)->get_pointer_impl();
        }

        /**
         * \lang english
         * @brief Dereference operator (const).
         * @return The result of the derived get_element_impl().
         *
         * \lang simp-chinese
         * @brief 解引用运算符（const）。
         * @return 派生类 get_element_impl() 的结果。
         */
        RAINY_CONSTEXPR20 decltype(auto) operator*() const
            noexcept(noexcept(static_cast<const implement_type *>(this)->get_element_impl())) {
            return static_cast<const implement_type *>(this)->get_element_impl();
        }

        /**
         * \lang english
         * @brief Arrow operator (const).
         * @return The result of the derived get_pointer_impl().
         *
         * \lang simp-chinese
         * @brief 箭头运算符（const）。
         * @return 派生类 get_pointer_impl() 的结果。
         */
        RAINY_CONSTEXPR20 decltype(auto) operator->() const
            noexcept(noexcept(static_cast<const implement_type *>(this)->get_pointer_impl())) {
            return static_cast<const implement_type *>(this)->get_pointer_impl();
        }

        /**
         * \lang english
         * @brief Prefix increment operator.
         * @return Reference to the derived iterator.
         *
         * \lang simp-chinese
         * @brief 前置自增运算符。
         * @return 派生迭代器的引用。
         */
        RAINY_CONSTEXPR20 implement_type &operator++() noexcept(noexcept(static_cast<implement_type *>(this)->next_impl())) {
            static_cast<implement_type *>(this)->next_impl();
            return *static_cast<implement_type *>(this);
        }

        /**
         * \lang english
         * @brief Postfix increment operator.
         * @return Copy of the derived iterator before increment.
         *
         * \lang simp-chinese
         * @brief 后置自增运算符。
         * @return 自增前派生迭代器的副本。
         */
        RAINY_CONSTEXPR20 implement_type operator++(int) noexcept(noexcept(static_cast<implement_type *>(this)->next_impl())) {
            implement_type tmp = *static_cast<implement_type *>(this);
            ++(*this);
            return tmp;
        }

        /**
         * \lang english
         * @brief Prefix increment operator (const overload).
         * @return Reference to the derived iterator.
         *
         * \lang simp-chinese
         * @brief 前置自增运算符（const 重载）。
         * @return 派生迭代器的引用。
         */
        RAINY_CONSTEXPR20 implement_type &operator++() const
            noexcept(noexcept(static_cast<implement_type *>(const_cast<bidirectional_iterator *>(this))->next_impl())) {
            auto self = static_cast<implement_type *>(const_cast<bidirectional_iterator *>(this));
            self->next_impl();
            return *self;
        }

        /**
         * \lang english
         * @brief Postfix increment operator (const overload).
         * @return Copy of the derived iterator before increment.
         *
         * \lang simp-chinese
         * @brief 后置自增运算符（const 重载）。
         * @return 自增前派生迭代器的副本。
         */
        RAINY_CONSTEXPR20 implement_type operator++(int) const {
            implement_type tmp = *static_cast<const implement_type *>(this);
            ++(*this);
            return tmp;
        }

        /**
         * \lang english
         * @brief Prefix decrement operator.
         * @return Reference to the derived iterator.
         *
         * \lang simp-chinese
         * @brief 前置自减运算符。
         * @return 派生迭代器的引用。
         */
        RAINY_CONSTEXPR20 implement_type &operator--() noexcept(noexcept(static_cast<implement_type *>(this)->back_impl())) {
            static_cast<implement_type *>(this)->back_impl();
            return *static_cast<implement_type *>(this);
        }

        /**
         * \lang english
         * @brief Postfix decrement operator.
         * @return Copy of the derived iterator before decrement.
         *
         * \lang simp-chinese
         * @brief 后置自减运算符。
         * @return 自减前派生迭代器的副本。
         */
        RAINY_CONSTEXPR20 implement_type operator--(int) noexcept(noexcept(static_cast<implement_type *>(this)->back_impl())) {
            implement_type tmp = *static_cast<implement_type *>(this);
            --(*this);
            return tmp;
        }

        /**
         * \lang english
         * @brief Equality comparison operator.
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if both iterators are equal.
         *
         * \lang simp-chinese
         * @brief 相等比较运算符。
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若两个迭代器相等则为 true。
         */
        friend RAINY_CONSTEXPR20 bool operator==(const bidirectional_iterator &left, const bidirectional_iterator &right) noexcept(
            implements::is_proxy_equal_with_noexcept<bidirectional_iterator>) {
            return left.proxy_equal_with_helper(right);
        }

        /**
         * \lang english
         * @brief Inequality comparison operator.
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if the iterators are not equal.
         *
         * \lang simp-chinese
         * @brief 不等比较运算符。
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 若迭代器不相等则为 true。
         */
        friend RAINY_CONSTEXPR20 bool operator!=(const bidirectional_iterator &left,
                                                 const bidirectional_iterator &right) noexcept(noexcept(left == right)) {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Proxy equality helper that forwards to the derived
         *        equal_with_impl().
         *
         * @param right The iterator to compare with.
         * @return true if the derived iterator reports equality.
         *
         * \lang simp-chinese
         * @brief 代理相等辅助函数，转发到派生类的 equal_with_impl()。
         *
         * @param right 要比较的迭代器。
         * @return 若派生迭代器报告相等则为 true。
         */
        RAINY_CONSTEXPR20 bool proxy_equal_with_helper(const bidirectional_iterator &right) const noexcept(
            noexcept(static_cast<const implement_type *>(this)->equal_with_impl(static_cast<const implement_type &>(right)))) {
            return static_cast<const implement_type *>(this)->equal_with_impl(static_cast<const implement_type &>(right));
        }
    };

    /**
     * \lang english
     * @brief Implementation of an iterator over the mapped values of a map.
     *
     *  Wraps a map iterator and dereferences to the mapped_type (the second member
     *  of the underlying key/value pair).
     *
     * @tparam MapContainer The map container type.
     * @tparam Iterator The underlying map iterator type.
     *
     * \lang simp-chinese
     * @brief 遍历 map 映射值的迭代器实现。
     *
     *  包装 map 迭代器，并解引用为 mapped_type（底层键/值对的第二个成员）。
     *
     * @tparam MapContainer map 容器类型。
     * @tparam Iterator 底层 map 迭代器类型。
     */
    template <typename MapContainer, typename Iterator>
    class map_mapped_iterator_impl
        : public utility::bidirectional_iterator<
              map_mapped_iterator_impl<MapContainer, Iterator>,
              utility::make_iterator_traits<typename type_traits::extras::iterators::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
                                            typename MapContainer::mapped_type *, typename MapContainer::mapped_type &,
                                            typename MapContainer::mapped_type>> {
    public:
        using base = utility::bidirectional_iterator<
            map_mapped_iterator_impl<MapContainer, Iterator>,
            utility::make_iterator_traits<typename type_traits::extras::iterators::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
                                          typename MapContainer::mapped_type *, typename MapContainer::mapped_type &,
                                          typename MapContainer::mapped_type>>;

        /**
         * \lang english
         * @brief Constructs the iterator from the underlying map iterator.
         *
         * @param it The underlying map iterator.
         *
         * \lang simp-chinese
         * @brief 从底层 map 迭代器构造该迭代器。
         *
         * @param it 底层 map 迭代器。
         */
        explicit map_mapped_iterator_impl(Iterator it) : current_(it) {
        }

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        map_mapped_iterator_impl() = default;

        /**
         * \lang english
         * @brief Mutable access to the mapped value.
         *
         * @note Applies to: non-const underlying iterators only.
         * @return A mutable reference to the mapped value.
         *
         * \lang simp-chinese
         * @brief 对映射值的可变访问。
         *
         * @note 适用类型：仅非 const 的底层迭代器。
         * @return 映射值的可变引用。
         */
        template <typename Iter = Iterator, typename = type_traits::other_trans::enable_if_t<!std::is_const_v<
                                                type_traits::modifers::remove_reference_t<typename type_traits::extras::iterators::iterator_traits<Iter>::reference>>>>
        typename base::reference get_element_impl() noexcept {
            return current_->second;
        }

        /**
         * \lang english
         * @brief Mutable access to the pointer of the mapped value.
         *
         * @note Applies to: non-const underlying iterators only.
         * @return A mutable pointer to the mapped value.
         *
         * \lang simp-chinese
         * @brief 对映射值指针的可变访问。
         *
         * @note 适用类型：仅非 const 的底层迭代器。
         * @return 映射值的可变指针。
         */
        template <typename Iter = Iterator, typename = type_traits::other_trans::enable_if_t<!std::is_const_v<
                                                type_traits::modifers::remove_reference_t<typename type_traits::extras::iterators::iterator_traits<Iter>::reference>>>>
        typename base::pointer get_pointer_impl() noexcept {
            return utility::addressof(current_->second);
        }

        /**
         * \lang english
         * @brief Const access to the mapped value.
         *
         * @return A const reference to the mapped value.
         *
         * \lang simp-chinese
         * @brief 对映射值的常量访问。
         *
         * @return 映射值的常量引用。
         */
        typename base::const_reference get_element_impl() const noexcept {
            return current_->second;
        }

        /**
         * \lang english
         * @brief Const access to the pointer of the mapped value.
         *
         * @return A const pointer to the mapped value.
         *
         * \lang simp-chinese
         * @brief 对映射值指针的常量访问。
         *
         * @return 映射值的常量指针。
         */
        typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(current_->second);
        }

        /**
         * \lang english
         * @brief Advances the underlying iterator.
         *
         * \lang simp-chinese
         * @brief 前进底层迭代器。
         */
        void next_impl() noexcept {
            ++current_;
        }

        /**
         * \lang english
         * @brief Moves the underlying iterator back.
         *
         * \lang simp-chinese
         * @brief 后退底层迭代器。
         */
        void back_impl() noexcept {
            --current_;
        }

        /**
         * \lang english
         * @brief Equality comparison with another mapped iterator, possibly with a
         *        different cv-qualification.
         *
         * @tparam OtherIterator The underlying iterator type of the other object.
         * @param other The other mapped iterator.
         * @return true if the underlying iterators are equal.
         *
         * \lang simp-chinese
         * @brief 与另一个映射迭代器比较（允许不同的 cv 限定）。
         *
         * @tparam OtherIterator 对方的底层迭代器类型。
         * @param other 另一个映射迭代器。
         * @return 若底层迭代器相等则为 true。
         */
        template <typename OtherIterator>
        bool equal_with_impl(const map_mapped_iterator_impl<MapContainer, OtherIterator> &other) const noexcept {
            return current_ == other.current_;
        }

    private:
        Iterator current_;

        template <typename M, typename I>
        friend class map_mapped_iterator_impl;
    };

    /**
     * \lang english
     * @brief A mutable iterator over the mapped values of a map.
     *
     * @tparam MapContainer The map container type.
     *
     * \lang simp-chinese
     * @brief 遍历 map 映射值的可变迭代器。
     *
     * @tparam MapContainer map 容器类型。
     */
    template <typename MapContainer>
    class map_mapped_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator>;

        using base::base;
    };

    /**
     * \lang english
     * @brief A const iterator over the mapped values of a map.
     *
     * @tparam MapContainer The map container type.
     *
     * \lang simp-chinese
     * @brief 遍历 map 映射值的常量迭代器。
     *
     * @tparam MapContainer map 容器类型。
     */
    template <typename MapContainer>
    class map_mapped_const_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator>;

        using base::base;
    };

    /**
     * \lang english
     * @brief Creates a view over the mapped values of a map.
     *
     * @tparam Map The map type.
     * @param map The map to view.
     * @return An iterator_range over the mapped values.
     *
     * \lang simp-chinese
     * @brief 创建遍历 map 映射值的视图。
     *
     * @tparam Map map 类型。
     * @param map 要查看的 map。
     * @return 遍历映射值的 iterator_range。
     */
    template <typename Map>
    rain_fn mapped_range(Map &map) -> collections::views::iterator_range<map_mapped_iterator<Map>> {
        using iterator = map_mapped_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator(map.begin()), iterator(map.end())};
    }

    /**
     * \lang english
     * @brief Creates a view over the mapped values of a const map.
     *
     * @tparam Map The map type.
     * @param map The map to view.
     * @return An iterator_range over the mapped values.
     *
     * \lang simp-chinese
     * @brief 创建遍历 const map 映射值的视图。
     *
     * @tparam Map map 类型。
     * @param map 要查看的 map。
     * @return 遍历映射值的 iterator_range。
     */
    template <typename Map>
    rain_fn mapped_range(const Map &map) -> collections::views::iterator_range<map_mapped_const_iterator<Map>> {
        using iterator = map_mapped_const_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator(map.cbegin()), iterator(map.cend())};
    }

    /**
     * \lang english
     * @brief A const iterator over the keys of a map.
     *
     * @tparam MapContainer The map container type.
     *
     * \lang simp-chinese
     * @brief 遍历 map 键的常量迭代器。
     *
     * @tparam MapContainer map 容器类型。
     */
    template <typename MapContainer>
    class map_key_const_iterator
        : public utility::bidirectional_iterator<
              map_key_const_iterator<MapContainer>,
              utility::make_iterator_traits<typename MapContainer::iterator::difference_type, std::bidirectional_iterator_tag,
                                            typename MapContainer::key_type *, typename MapContainer::key_type &,
                                            typename MapContainer::key_type>> {
    public:
        template <typename MapContainer_>
        friend class map_mapped_iterator;

        using base = utility::bidirectional_iterator<
            map_key_const_iterator<MapContainer>,
            utility::make_iterator_traits<typename MapContainer::iterator::difference_type, std::bidirectional_iterator_tag,
                                          typename MapContainer::key_type *, typename MapContainer::key_type &,
                                          typename MapContainer::key_type>>;

        /**
         * \lang english
         * @brief Constructs the iterator from an underlying map iterator.
         *
         * @tparam Ty The underlying iterator type.
         * @param it The underlying iterator.
         *
         * \lang simp-chinese
         * @brief 从底层 map 迭代器构造该迭代器。
         *
         * @tparam Ty 底层迭代器类型。
         * @param it 底层迭代器。
         */
        template <typename Ty>
        explicit map_key_const_iterator(Ty it) : current_(it) {
        }

        /**
         * \lang english
         * @brief Const access to the key.
         *
         * @return A const reference to the key.
         *
         * \lang simp-chinese
         * @brief 对键的常量访问。
         *
         * @return 键的常量引用。
         */
        typename base::const_reference get_element_impl() const noexcept {
            return current_->first;
        }

        /**
         * \lang english
         * @brief Const access to the pointer of the key.
         *
         * @return A const pointer to the key.
         *
         * \lang simp-chinese
         * @brief 对键指针的常量访问。
         *
         * @return 键的常量指针。
         */
        typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(current_->first);
        }

        /**
         * \lang english
         * @brief Advances the underlying iterator.
         *
         * \lang simp-chinese
         * @brief 前进底层迭代器。
         */
        void next_impl() noexcept {
            ++current_;
        }

        /**
         * \lang english
         * @brief Moves the underlying iterator back.
         *
         * \lang simp-chinese
         * @brief 后退底层迭代器。
         */
        void back_impl() noexcept {
            --current_;
        }

        /**
         * \lang english
         * @brief Equality comparison with another key iterator.
         *
         * @param other The other key iterator.
         * @return true if the underlying iterators are equal.
         *
         * \lang simp-chinese
         * @brief 与另一个键迭代器比较。
         *
         * @param other 另一个键迭代器。
         * @return 若底层迭代器相等则为 true。
         */
        bool equal_with_impl(const map_key_const_iterator &other) const noexcept {
            return current_ == other.current_;
        }

    private:
        typename MapContainer::const_iterator current_;
    };

    /**
     * \lang english
     * @brief A mutable iterator over the keys of a map.
     *
     * @tparam MapContainer The map container type.
     *
     * \lang simp-chinese
     * @brief 遍历 map 键的可变迭代器。
     *
     * @tparam MapContainer map 容器类型。
     */
    template <typename MapContainer>
    class map_key_iterator : public map_key_const_iterator<MapContainer> {
    public:
        using base = typename map_key_const_iterator<MapContainer>::base;

        using const_iterator = map_key_const_iterator<MapContainer>;
        using const_iterator::const_iterator;

        /**
         * \lang english
         * @brief Mutable access to the pointer of the key.
         *
         * @return A mutable pointer to the key.
         *
         * \lang simp-chinese
         * @brief 对键指针的可变访问。
         *
         * @return 键的可变指针。
         */
        typename base::pointer get_pointer_impl() noexcept {
            return utility::addressof(this->current_->first);
        }

        /**
         * \lang english
         * @brief Mutable access to the key.
         *
         * @return A mutable reference to the key.
         *
         * \lang simp-chinese
         * @brief 对键的可变访问。
         *
         * @return 键的可变引用。
         */
        typename base::reference &get_element_impl() noexcept {
            return const_cast<typename base::reference &>(this->current_->first);
        }
    };

    /**
     * \lang english
     * @brief Creates a view over the keys of a map.
     *
     * @tparam Map The map type.
     * @param map The map to view.
     * @return An iterator_range over the keys.
     *
     * \lang simp-chinese
     * @brief 创建遍历 map 键的视图。
     *
     * @tparam Map map 类型。
     * @param map 要查看的 map。
     * @return 遍历键的 iterator_range。
     */
    template <typename Map>
    rain_fn keyed_range(Map &map) -> collections::views::iterator_range<map_key_iterator<Map>> {
        using iterator = map_key_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator{map.begin()}, iterator{map.end()}};
    }

    /**
     * \lang english
     * @brief Creates a view over the keys of a const map.
     *
     * @tparam Map The map type.
     * @param map The map to view.
     * @return An iterator_range over the keys.
     *
     * \lang simp-chinese
     * @brief 创建遍历 const map 键的视图。
     *
     * @tparam Map map 类型。
     * @param map 要查看的 map。
     * @return 遍历键的 iterator_range。
     */
    template <typename Map>
    rain_fn keyed_range(const Map &map) -> collections::views::iterator_range<map_key_iterator<Map>> {
        using iterator = map_key_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator{map.begin()}, iterator{map.end()}};
    }
}

#else

namespace rainy::core::collections::views::implements {
    /**
     * \lang english
     * @brief An iterator adaptor that wraps an underlying iterator to expose a
     *        unified interface for iterator ranges.
     *
     * @tparam Iter The underlying iterator type.
     *
     * \lang simp-chinese
     * @brief 包装底层迭代器以暴露统一接口的迭代器适配器。
     *
     * @tparam Iter 底层迭代器类型。
     */
    template <typename Iter>
    class iterator_range_iterator {
    public:
        using iterator_category = typename utility::iterator_traits<Iter>::iterator_category;
        using value_type = typename utility::iterator_traits<Iter>::value_type;
        using difference_type = typename utility::iterator_traits<Iter>::difference_type;
        using pointer = typename utility::iterator_traits<Iter>::pointer;
        using reference = typename utility::iterator_traits<Iter>::reference;

        /**
         * \lang english
         * @brief Constructs an iterator_range_iterator from an underlying iterator.
         *
         * @param iter The underlying iterator to wrap
         *
         * \lang simp-chinese
         * @brief 从底层迭代器构造iterator_range_iterator。
         *
         * @param iter 要包装的底层迭代器
         */
        iterator_range_iterator(Iter iter);

        /**
         * \lang english
         * @brief Dereference operator.
         * @return Reference to the element pointed to by the underlying iterator.
         *
         * \lang simp-chinese
         * @brief 解引用运算符。
         * @return 底层迭代器所指向元素的引用。
         */
        decltype(auto) operator*() const;

        /**
         * \lang english
         * @brief Arrow operator.
         * @return Pointer to the element pointed to by the underlying iterator.
         *
         * \lang simp-chinese
         * @brief 箭头运算符。
         * @return 指向底层迭代器所指向元素的指针。
         */
        pointer operator->() const;

        /**
         * \lang english
         * @brief Prefix increment operator.
         * @return Reference to this iterator after increment.
         *
         * \lang simp-chinese
         * @brief 前置自增运算符。
         * @return 自增后此迭代器的引用。
         */
        rain_fn operator++()->iterator_range_iterator &;

        /**
         * \lang english
         * @brief Postfix increment operator.
         * @return Copy of this iterator before increment.
         *
         * \lang simp-chinese
         * @brief 后置自增运算符。
         * @return 自增前此迭代器的副本。
         */
        rain_fn operator++(int);

        /**
         * \lang english
         * @brief Prefix decrement operator.
         * @return Reference to this iterator after decrement.
         *
         * \lang simp-chinese
         * @brief 前置自减运算符。
         * @return 自减后此迭代器的引用。
         */
        rain_fn operator--();

        /**
         * \lang english
         * @brief Postfix decrement operator.
         * @return Copy of this iterator before decrement.
         *
         * \lang simp-chinese
         * @brief 后置自减运算符。
         * @return 自减前此迭代器的副本。
         */
        rain_fn operator--(int);

        /**
         * \lang english
         * @brief Addition operator.
         * @param n Number of positions to advance.
         * @return New iterator advanced by n positions.
         *
         * \lang simp-chinese
         * @brief 加法运算符。
         * @param n 前进的位置数。
         * @return 前进n个位置后的新迭代器。
         */
        rain_fn operator+(difference_type n) const->iterator_range_iterator;

        /**
         * \lang english
         * @brief Subtraction operator.
         * @param n Number of positions to move back.
         * @return New iterator moved back by n positions.
         *
         * \lang simp-chinese
         * @brief 减法运算符。
         * @param n 后退的位置数。
         * @return 后退n个位置后的新迭代器。
         */
        rain_fn operator-(difference_type n) const->iterator_range_iterator;

        /**
         * \lang english
         * @brief Addition assignment operator.
         * @param n Number of positions to advance.
         * @return Reference to this iterator after advancement.
         *
         * \lang simp-chinese
         * @brief 加法赋值运算符。
         * @param n 前进的位置数。
         * @return 前进后此迭代器的引用。
         */
        rain_fn &operator+=(difference_type n);

        /**
         * \lang english
         * @brief Subtraction assignment operator.
         * @param n Number of positions to move back.
         * @return Reference to this iterator after moving back.
         *
         * \lang simp-chinese
         * @brief 减法赋值运算符。
         * @param n 后退的位置数。
         * @return 后退后此迭代器的引用。
         */
        rain_fn &operator-=(difference_type n);

        /**
         * \lang english
         * @brief Difference operator.
         * @param other Another iterator to compare with.
         * @return Distance between this iterator and other.
         *
         * \lang simp-chinese
         * @brief 差运算符。
         * @param other 要比较的另一个迭代器。
         * @return 此迭代器与other之间的距离。
         */
        rain_fn operator-(const iterator_range_iterator &other) const->difference_type;

        /**
         * \lang english
         * @brief Equality comparison operator.
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if iterators are equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 相等比较运算符。
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 如果迭代器相等则为true，否则为false。
         */
        friend bool operator==(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept;

        /**
         * \lang english
         * @brief Inequality comparison operator.
         * @param left Left-hand side iterator.
         * @param right Right-hand side iterator.
         * @return true if iterators are not equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 不等比较运算符。
         * @param left 左侧迭代器。
         * @param right 右侧迭代器。
         * @return 如果迭代器不相等则为true，否则为false。
         */
        friend bool operator!=(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept;
    };

    /**
     * \lang english
     * @brief Adapts an iterator pair into a view with begin/end accessors.
     *
     * @tparam Iter The underlying iterator type.
     *
     * \lang simp-chinese
     * @brief 将一对迭代器适配为具有 begin/end 访问器的视图。
     *
     * @tparam Iter 底层迭代器类型。
     */
    template <typename Iter>
    class adapter_iterator_range : public views::view_interface<adapter_iterator_range<Iter>> {
    public:
        using iterator = iterator_range_iterator<Iter>;
        using const_iterator = const iterator_range_iterator<const Iter>;
        using reference = type_traits::extras::iterators::iterator_reference_t<iterator>;
        using const_reference = type_traits::modifers::add_const_t<reference>;
        using difference_type = type_traits::extras::iterators::iterator_difference_t<const_iterator>;
        using value_type = type_traits::extras::iterators::iter_value_t<const_iterator>;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        adapter_iterator_range();

        /**
         * \lang english
         * @brief Constructs an adapter_iterator_range from begin and end iterators.
         *
         * @param begin Iterator to the beginning of the range
         * @param end Iterator to the end of the range
         *
         * \lang simp-chinese
         * @brief 从开始和结束迭代器构造adapter_iterator_range。
         *
         * @param begin 指向范围起始的迭代器
         * @param end 指向范围末尾的迭代器
         */
        adapter_iterator_range(Iter begin, Iter end);

        /**
         * \lang english
         * @brief Returns a const reference to the base range.
         *
         * \lang simp-chinese
         * @brief 返回基范围的常量引用。
         */
        RAINY_NODISCARD constexpr rain_fn base() const & noexcept -> const adapter_iterator_range &;

        /**
         * \lang english
         * @brief Returns an rvalue reference to the base range.
         *
         * \lang simp-chinese
         * @brief 返回基范围的右值引用。
         */
        RAINY_NODISCARD constexpr rain_fn base() && noexcept -> adapter_iterator_range;

        /**
         * \lang english
         * @brief Returns an iterator to the beginning.
         *
         * \lang simp-chinese
         * @brief 返回指向起始的迭代器。
         */
        rain_fn begin() noexcept -> iterator;

        /**
         * \lang english
         * @brief Returns an iterator to the end.
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的迭代器。
         */
        rain_fn end() noexcept -> iterator;

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning.
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量迭代器。
         */
        rain_fn begin() const noexcept -> const_iterator;

        /**
         * \lang english
         * @brief Returns a const iterator to the end.
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量迭代器。
         */
        rain_fn end() const noexcept -> const_iterator;

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning (explicit).
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量迭代器（显式）。
         */
        rain_fn cbegin() const noexcept -> const_iterator;

        /**
         * \lang english
         * @brief Returns a const iterator to the end (explicit).
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量迭代器（显式）。
         */
        rain_fn cend() const noexcept -> const_iterator;
    };
}

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief A range view over an iterator pair.
     *
     * @tparam Iter The underlying iterator type
     *
     * \lang simp-chinese
     * @brief 基于迭代器对的区间视图。
     *
     * @tparam Iter 底层迭代器类型
     */
    template <typename Iter>
    class iterator_range : public implements::adapter_iterator_range<Iter> {
    public:
        using base = implements::adapter_iterator_range<Iter>;

        /**
         * \lang english
         * @brief Constructs an iterator_range from begin and end iterators.
         *
         * \lang simp-chinese
         * @brief 从开始和结束迭代器构造 iterator_range。
         */
        iterator_range(Iter begin, Iter end);
    };
}

namespace rainy::collections::views {
    using core::collections::views::iterator_range;
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief A minimal pointer-like wrapper that owns a value.
     *
     * @tparam Ty The owned value type.
     *
     * \lang simp-chinese
     * @brief 一种最小化的类指针包装器，持有值。
     *
     * @tparam Ty 所持有的值类型。
     */
    template <typename Ty>
    struct input_iterator_pointer final {
        using value_type = Ty;
        using pointer = Ty *;
        using reference = Ty &;

        /**
         * \lang english
         * @brief Constructs the wrapper by moving the given value into it.
         *
         * \lang simp-chinese
         * @brief 通过将给定值移动到其中来构造该包装器。
         */
        constexpr input_iterator_pointer(value_type &&val) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<value_type>);

        /**
         * \lang english
         * @brief Returns a pointer to the owned value.
         *
         * \lang simp-chinese
         * @brief 返回指向所持有值的指针。
         */
        RAINY_NODISCARD constexpr pointer operator->() noexcept;

        /**
         * \lang english
         * @brief Returns a reference to the owned value.
         *
         * \lang simp-chinese
         * @brief 返回所持有值的引用。
         */
        RAINY_NODISCARD constexpr reference operator*() noexcept;
    };

    /**
     * \lang english
     * @brief A const iterator adaptor wrapping an underlying iterator.
     *
     * @tparam Iter The underlying iterator type.
     * @tparam Traits The iterator traits of Iter.
     *
     * \lang simp-chinese
     * @brief 包装底层迭代器的常量迭代器适配器。
     *
     * @tparam Iter 底层迭代器类型。
     * @tparam Traits Iter 的迭代器 traits。
     */
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class const_iterator {
    public:
        template <typename, typename>
        friend class iterator;

        friend struct core::memory::pointer_traits<const_iterator<Iter, Traits>>;

        using iterator_type = Iter;
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        constexpr const_iterator() noexcept;
        constexpr const_iterator &operator=(const_iterator &&) noexcept = default;
        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;
        explicit constexpr const_iterator(iterator_type current) noexcept;
        constexpr const_iterator(const const_iterator &right);
        constexpr const_iterator(const_iterator &&right) noexcept;
        RAINY_CONSTEXPR20 ~const_iterator() = default;
        constexpr const_pointer get_pointer() const noexcept;
        constexpr const_pointer operator->() const noexcept;
        constexpr const_reference dereference() const noexcept;
        constexpr const_reference operator*() const noexcept;
        constexpr const_iterator &self_inc_prefix() noexcept;
        constexpr const_iterator self_inc_postfix() noexcept;
        constexpr const_iterator &operator++() noexcept;
        constexpr const_iterator operator++(int) noexcept;
        constexpr const_iterator &self_dec_prefix() noexcept;
        constexpr const_iterator self_dec_postfix() noexcept;
        constexpr const_iterator &operator--() noexcept;
        constexpr const_iterator operator--(int) noexcept;
        constexpr const_iterator &plus_equal(difference_type n) noexcept;
        constexpr const_iterator &minus_equal(difference_type n) noexcept;
        constexpr const_iterator &operator+=(difference_type n) noexcept;
        constexpr const_iterator &operator-=(difference_type n) noexcept;
        constexpr bool is_equal(const const_iterator &right) const noexcept;
        constexpr bool not_equal(const const_iterator &right) const noexcept;
        constexpr friend bool operator==(const const_iterator &left, const const_iterator &right);
        constexpr friend bool operator!=(const const_iterator &left, const const_iterator &right);
        constexpr const_reference at_subscript(difference_type idx) const noexcept;
        constexpr const_reference operator[](difference_type idx) const noexcept;
        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept;
        constexpr explicit operator bool() const noexcept;
        constexpr const_iterator add_offset(difference_type n) const noexcept;
        constexpr friend const_iterator operator+(const const_iterator left, difference_type n);
        constexpr difference_type subtract(difference_type n) const noexcept;
        constexpr difference_type subtract(const const_iterator &right) const noexcept;
        constexpr friend difference_type operator-(const const_iterator &left, const const_iterator &right);
        constexpr friend difference_type operator-(const const_iterator &left, difference_type n);
        constexpr bool lt(const const_iterator &right) const noexcept;
        constexpr friend bool operator<(const const_iterator &left, const const_iterator &right);
        constexpr bool lt_or_equal(const const_iterator &right) const noexcept;
        constexpr friend bool operator<=(const const_iterator &left, const const_iterator &right);
        constexpr bool greater_than(const const_iterator &right) const noexcept;
        constexpr friend bool operator>(const const_iterator &left, const const_iterator &right);
        constexpr bool greater_than_or_equal(const const_iterator &right) const noexcept;
        constexpr friend bool operator>=(const const_iterator &left, const const_iterator &right);
    };

    /**
     * \lang english
     * @brief A mutable iterator adaptor wrapping an underlying iterator.
     *
     * @tparam Iter The underlying iterator type.
     * @tparam Traits The iterator traits of Iter.
     *
     * \lang simp-chinese
     * @brief 包装底层迭代器的可变迭代器适配器。
     *
     * @tparam Iter 底层迭代器类型。
     * @tparam Traits Iter 的迭代器 traits。
     */
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class iterator : public const_iterator<Iter, Traits> {
    public:
        using iterator_type = Iter;
        using iterator_traits = Traits;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;
        using base = const_iterator<Iter, Traits>;

        friend struct core::memory::pointer_traits<const_iterator<Iter, Traits>>;

        constexpr iterator() noexcept;
        constexpr iterator(iterator_type current) noexcept;
        constexpr iterator(const iterator &right);
        constexpr iterator(iterator &&right) noexcept;
        constexpr iterator &operator=(iterator &&) noexcept = default;
        constexpr iterator &operator=(const iterator &) noexcept = default;
        RAINY_CONSTEXPR20 ~iterator() = default;
        constexpr base &as_const_iterator() const noexcept;
        constexpr pointer get_pointer() noexcept;
        constexpr pointer operator->() noexcept;
        constexpr reference dereference() noexcept;
        constexpr reference operator*() noexcept;
        constexpr iterator add_offset(typename base::difference_type n) const noexcept;
        constexpr friend iterator operator+(const iterator left, typename base::difference_type n);
        constexpr typename base::difference_type subtract(typename base::difference_type n) const noexcept;
        constexpr typename base::difference_type subtract(const iterator &right) const noexcept;
        constexpr iterator &operator++() noexcept;
        constexpr iterator operator++(int) noexcept;
        constexpr iterator &operator--() noexcept;
        constexpr iterator operator--(int) noexcept;
        constexpr friend typename base::difference_type operator-(const iterator &left, const iterator &right);
        constexpr friend typename base::difference_type operator-(const iterator &left, typename base::difference_type n);
    };

    /**
     * \lang english
     * @brief Extracts the iterator_category type from an iterator type.
     *
     * @tparam Iter The iterator type.
     *
     * \lang simp-chinese
     * @brief 从迭代器类型中提取 iterator_category 类型。
     *
     * @tparam Iter 迭代器类型。
     */
    template <typename Iter>
    struct iterator_category {
        using type = typename Iter::iterator_category;
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename = void>
    static RAINY_CONSTEXPR_BOOL is_proxy_equal_with_noexcept = false;

    template <typename Ty>
    static RAINY_CONSTEXPR_BOOL is_proxy_equal_with_noexcept<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().proxy_equal_with_helper(
                utility::declval<const Ty &>()))>> =
        noexcept(utility::declval<const Ty &>().proxy_equal_with_helper(utility::declval<const Ty &>()));
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief A CRTP base class for bidirectional iterators.
     *
     * @tparam Implement The derived iterator implementation type.
     * @tparam Traits The iterator traits describing the iterator.
     *
     * \lang simp-chinese
     * @brief 双向迭代器的 CRTP 基类。
     *
     * @tparam Implement 派生迭代器实现类型。
     * @tparam Traits 描述该迭代器的迭代器 traits。
     */
    template <typename Implement, typename Traits>
    class bidirectional_iterator {
    public:
        using implement_type = Implement;
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using pointer = value_type &;
        using reference = value_type &;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;
#if RAINY_HAS_CXX20
        using iterator_concept = typename iterator_traits::iterator_category;
#endif

        template <typename UImplement, typename UTraits>
        friend class bidirectional_iterator;

        RAINY_CONSTEXPR20 decltype(auto) operator*() noexcept;
        RAINY_CONSTEXPR20 decltype(auto) operator->() noexcept;
        RAINY_CONSTEXPR20 decltype(auto) operator*() const noexcept;
        RAINY_CONSTEXPR20 decltype(auto) operator->() const noexcept;
        RAINY_CONSTEXPR20 implement_type &operator++() noexcept;
        RAINY_CONSTEXPR20 implement_type operator++(int) noexcept;
        RAINY_CONSTEXPR20 implement_type &operator++() const noexcept;
        RAINY_CONSTEXPR20 implement_type operator++(int) const;
        RAINY_CONSTEXPR20 implement_type &operator--() noexcept;
        RAINY_CONSTEXPR20 implement_type operator--(int) noexcept;
        friend RAINY_CONSTEXPR20 bool operator==(const bidirectional_iterator &left,
                                                 const bidirectional_iterator &right) noexcept;
        friend RAINY_CONSTEXPR20 bool operator!=(const bidirectional_iterator &left,
                                                 const bidirectional_iterator &right) noexcept;
        RAINY_CONSTEXPR20 bool proxy_equal_with_helper(const bidirectional_iterator &right) const noexcept;
    };

    /**
     * \lang english
     * @brief Implementation of an iterator over the mapped values of a map.
     *
     * @tparam MapContainer The map container type.
     * @tparam Iterator The underlying map iterator type.
     *
     * \lang simp-chinese
     * @brief 遍历 map 映射值的迭代器实现。
     *
     * @tparam MapContainer map 容器类型。
     * @tparam Iterator 底层 map 迭代器类型。
     */
    template <typename MapContainer, typename Iterator>
    class map_mapped_iterator_impl
        : public utility::bidirectional_iterator<
              map_mapped_iterator_impl<MapContainer, Iterator>,
              utility::make_iterator_traits<typename type_traits::extras::iterators::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
                                            typename MapContainer::mapped_type *, typename MapContainer::mapped_type &,
                                            typename MapContainer::mapped_type>> {
    public:
        using base = utility::bidirectional_iterator<
            map_mapped_iterator_impl<MapContainer, Iterator>,
            utility::make_iterator_traits<typename type_traits::extras::iterators::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
                                          typename MapContainer::mapped_type *, typename MapContainer::mapped_type &,
                                          typename MapContainer::mapped_type>>;

        explicit map_mapped_iterator_impl(Iterator it);
        map_mapped_iterator_impl() = default;

        template <typename Iter = Iterator, typename = type_traits::other_trans::enable_if_t<!std::is_const_v<
                                                type_traits::modifers::remove_reference_t<typename type_traits::extras::iterators::iterator_traits<Iter>::reference>>>>
        typename base::reference get_element_impl() noexcept;

        template <typename Iter = Iterator, typename = type_traits::other_trans::enable_if_t<!std::is_const_v<
                                                type_traits::modifers::remove_reference_t<typename type_traits::extras::iterators::iterator_traits<Iter>::reference>>>>
        typename base::pointer get_pointer_impl() noexcept;

        typename base::const_reference get_element_impl() const noexcept;
        typename base::const_pointer get_pointer_impl() const noexcept;
        void next_impl() noexcept;
        void back_impl() noexcept;

        template <typename OtherIterator>
        bool equal_with_impl(const map_mapped_iterator_impl<MapContainer, OtherIterator> &other) const noexcept;
    };

    /**
     * \lang english
     * @brief A mutable iterator over the mapped values of a map.
     *
     * \lang simp-chinese
     * @brief 遍历 map 映射值的可变迭代器。
     */
    template <typename MapContainer>
    class map_mapped_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator>;
        using base::base;
    };

    /**
     * \lang english
     * @brief A const iterator over the mapped values of a map.
     *
     * \lang simp-chinese
     * @brief 遍历 map 映射值的常量迭代器。
     */
    template <typename MapContainer>
    class map_mapped_const_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator>;
        using base::base;
    };

    /**
     * \lang english
     * @brief Creates a view over the mapped values of a map.
     *
     * \lang simp-chinese
     * @brief 创建遍历 map 映射值的视图。
     */
    template <typename Map>
    rain_fn mapped_range(Map &map) -> collections::views::iterator_range<map_mapped_iterator<Map>>;

    /**
     * \lang english
     * @brief Creates a view over the mapped values of a const map.
     *
     * \lang simp-chinese
     * @brief 创建遍历 const map 映射值的视图。
     */
    template <typename Map>
    rain_fn mapped_range(const Map &map) -> collections::views::iterator_range<map_mapped_const_iterator<Map>>;

    /**
     * \lang english
     * @brief A const iterator over the keys of a map.
     *
     * \lang simp-chinese
     * @brief 遍历 map 键的常量迭代器。
     */
    template <typename MapContainer>
    class map_key_const_iterator
        : public utility::bidirectional_iterator<
              map_key_const_iterator<MapContainer>,
              utility::make_iterator_traits<typename MapContainer::iterator::difference_type, std::bidirectional_iterator_tag,
                                            typename MapContainer::key_type *, typename MapContainer::key_type &,
                                            typename MapContainer::key_type>> {
    public:
        template <typename MapContainer_>
        friend class map_mapped_iterator;

        using base = utility::bidirectional_iterator<
            map_key_const_iterator<MapContainer>,
            utility::make_iterator_traits<typename MapContainer::iterator::difference_type, std::bidirectional_iterator_tag,
                                          typename MapContainer::key_type *, typename MapContainer::key_type &,
                                          typename MapContainer::key_type>>;

        template <typename Ty>
        explicit map_key_const_iterator(Ty it);

        typename base::const_reference get_element_impl() const noexcept;
        typename base::const_pointer get_pointer_impl() const noexcept;
        void next_impl() noexcept;
        void back_impl() noexcept;
        bool equal_with_impl(const map_key_const_iterator &other) const noexcept;
    };

    /**
     * \lang english
     * @brief A mutable iterator over the keys of a map.
     *
     * \lang simp-chinese
     * @brief 遍历 map 键的可变迭代器。
     */
    template <typename MapContainer>
    class map_key_iterator : public map_key_const_iterator<MapContainer> {
    public:
        using base = typename map_key_const_iterator<MapContainer>::base;
        using const_iterator = map_key_const_iterator<MapContainer>;
        using const_iterator::const_iterator;

        typename base::pointer get_pointer_impl() noexcept;
        typename base::reference &get_element_impl() noexcept;
    };

    /**
     * \lang english
     * @brief Creates a view over the keys of a map.
     *
     * \lang simp-chinese
     * @brief 创建遍历 map 键的视图。
     */
    template <typename Map>
    rain_fn keyed_range(Map &map) -> collections::views::iterator_range<map_key_iterator<Map>>;

    /**
     * \lang english
     * @brief Creates a view over the keys of a const map.
     *
     * \lang simp-chinese
     * @brief 创建遍历 const map 键的视图。
     */
    template <typename Map>
    rain_fn keyed_range(const Map &map) -> collections::views::iterator_range<map_key_iterator<Map>>;
}

#endif

#endif