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

namespace rainy::core::collections::views::implements {
    template <typename Iter>
    class iterator_range_iterator {
    public:
        using iterator_category = typename utility::iterator_traits<Iter>::iterator_category;
        using value_type = typename utility::iterator_traits<Iter>::value_type;
        using difference_type = typename utility::iterator_traits<Iter>::difference_type;
        using pointer = typename utility::iterator_traits<Iter>::pointer;
        using reference = typename utility::iterator_traits<Iter>::reference;

        /**
         * @brief Constructs an iterator_range_iterator from an underlying iterator.
         *        从底层迭代器构造iterator_range_iterator。
         *
         * @param iter The underlying iterator to wrap
         *             要包装的底层迭代器
         */
        iterator_range_iterator(Iter iter) : iter_{iter} {
        }

        /**
         * @brief Dereference operator.
         *        解引用运算符。
         *
         * @return Reference to the element pointed to by the underlying iterator
         *         底层迭代器所指向元素的引用
         */
        decltype(auto) operator*() const {
            return (*iter_);
        }

        /**
         * @brief Arrow operator.
         *        箭头运算符。
         *
         * @return Pointer to the element pointed to by the underlying iterator
         *         指向底层迭代器所指向元素的指针
         */
        pointer operator->() const {
            return utility::addressof(*iter_);
        }

        /**
         * @brief Prefix increment operator.
         *        前置自增运算符。
         *
         * @return Reference to this iterator after increment
         *         自增后此迭代器的引用
         */
        rain_fn operator++()->iterator_range_iterator & {
            ++iter_;
            return *this;
        }

        /**
         * @brief Postfix increment operator.
         *        后置自增运算符。
         *
         * @return Copy of this iterator before increment
         *         自增前此迭代器的副本
         */
        rain_fn operator++(int) {
            iterator_range_iterator temp = *this;
            ++iter_;
            return temp;
        }

        /**
         * @brief Prefix decrement operator.
         *        前置自减运算符。
         *
         * @return Reference to this iterator after decrement
         *         自减后此迭代器的引用
         */
        rain_fn operator--() {
            --iter_;
            return *this;
        }

        /**
         * @brief Postfix decrement operator.
         *        后置自减运算符。
         *
         * @return Copy of this iterator before decrement
         *         自减前此迭代器的副本
         */
        rain_fn operator--(int) {
            iterator_range_iterator temp = *this;
            --iter_;
            return temp;
        }

        /**
         * @brief Addition operator.
         *        加法运算符。
         *
         * @param n Number of positions to advance
         *          前进的位置数
         * @return New iterator advanced by n positions
         *         前进n个位置后的新迭代器
         */
        rain_fn operator+(difference_type n) const->iterator_range_iterator {
            return iterator_range_iterator(iter_ + n);
        }

        /**
         * @brief Subtraction operator.
         *        减法运算符。
         *
         * @param n Number of positions to move back
         *          后退的位置数
         * @return New iterator moved back by n positions
         *         后退n个位置后的新迭代器
         */
        rain_fn operator-(difference_type n) const->iterator_range_iterator {
            return iterator_range_iterator(iter_ - n);
        }

        /**
         * @brief Addition assignment operator.
         *        加法赋值运算符。
         *
         * @param n Number of positions to advance
         *          前进的位置数
         * @return Reference to this iterator after advancement
         *         前进后此迭代器的引用
         */
        rain_fn &operator+=(difference_type n) {
            iter_ += n;
            return *this;
        }

        /**
         * @brief Subtraction assignment operator.
         *        减法赋值运算符。
         *
         * @param n Number of positions to move back
         *          后退的位置数
         * @return Reference to this iterator after moving back
         *         后退后此迭代器的引用
         */
        rain_fn &operator-=(difference_type n) {
            iter_ -= n;
            return *this;
        }

        /**
         * @brief Difference operator.
         *        差运算符。
         *
         * @param other Another iterator to compare with
         *              要比较的另一个迭代器
         * @return Distance between this iterator and other
         *         此迭代器与other之间的距离
         */
        rain_fn operator-(const iterator_range_iterator &other) const->difference_type {
            return iter_ - other.iter_;
        }

        /**
         * @brief Equality comparison operator.
         *        相等比较运算符。
         *
         * @param left Left-hand side iterator
         *             左侧迭代器
         * @param right Right-hand side iterator
         *              右侧迭代器
         * @return true if iterators are equal, false otherwise
         *         如果迭代器相等则为true，否则为false
         */
        friend bool operator==(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept {
            return left.iter_ == right.iter_;
        }

        /**
         * @brief Inequality comparison operator.
         *        不等比较运算符。
         *
         * @param left Left-hand side iterator
         *             左侧迭代器
         * @param right Right-hand side iterator
         *              右侧迭代器
         * @return true if iterators are not equal, false otherwise
         *         如果迭代器不相等则为true，否则为false
         */
        friend bool operator!=(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept {
            return !(left.iter_ == right.iter_);
        }

    private:
        Iter iter_;
    };

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
         * @brief Default constructor.
         *        默认构造函数。
         */
        adapter_iterator_range() : begin_{}, end_{} {
        }

        /**
         * @brief Constructs an adapter_iterator_range from begin and end iterators.
         *        从开始和结束迭代器构造adapter_iterator_range。
         *
         * @param begin Iterator to the beginning of the range
         *              指向范围起始的迭代器
         * @param end Iterator to the end of the range
         *            指向范围末尾的迭代器
         */
        adapter_iterator_range(Iter begin, Iter end) : begin_{begin}, end_{end} {
        }

        /**
         * @brief Returns a const reference to the base range.
         *        返回基范围的常量引用。
         *
         * @return Const reference to this adapter_iterator_range
         *         此adapter_iterator_range的常量引用
         */
        RAINY_NODISCARD constexpr rain_fn base() const & noexcept -> const adapter_iterator_range & {
            return *this;
        }

        /**
         * @brief Returns an rvalue reference to the base range.
         *        返回基范围的右值引用。
         *
         * @return Rvalue reference to this adapter_iterator_range
         *         此adapter_iterator_range的右值引用
         */
        RAINY_NODISCARD constexpr rain_fn base() && noexcept -> adapter_iterator_range {
            return utility::move(*this);
        }

        /**
         * @brief Returns an iterator to the beginning.
         *        返回指向起始的迭代器。
         *
         * @return Iterator to the first element
         *         指向第一个元素的迭代器
         */
        rain_fn begin() noexcept -> iterator {
            return iterator{begin_};
        }

        /**
         * @brief Returns an iterator to the end.
         *        返回指向末尾的迭代器。
         *
         * @return Iterator to one past the last element
         *         指向最后一个元素之后位置的迭代器
         */
        rain_fn end() noexcept -> iterator {
            return iterator{end_};
        }

        /**
         * @brief Returns a const iterator to the beginning.
         *        返回指向起始的常量迭代器。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        rain_fn begin() const noexcept -> const_iterator {
            return const_iterator{begin_};
        }

        /**
         * @brief Returns a const iterator to the end.
         *        返回指向末尾的常量迭代器。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        rain_fn end() const noexcept -> const_iterator {
            return const_iterator{end_};
        }

        /**
         * @brief Returns a const iterator to the beginning (explicit).
         *        返回指向起始的常量迭代器（显式）。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        rain_fn cbegin() const noexcept -> const_iterator {
            return const_iterator{begin_};
        }

        /**
         * @brief Returns a const iterator to the end (explicit).
         *        返回指向末尾的常量迭代器（显式）。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
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
     * @brief A range view over an iterator pair.
     *        基于迭代器对的区间视图。
     *
     * This class provides a view over a range defined by a pair of iterators,
     * inheriting all functionality from adapter_iterator_range.
     *
     * 此类提供由一对迭代器定义的区间的视图，
     * 继承自 adapter_iterator_range 的所有功能。
     *
     * @tparam Iter The underlying iterator type
     *              底层迭代器类型
     */
    template <typename Iter>
    class iterator_range : public implements::adapter_iterator_range<Iter> {
    public:
        using base = implements::adapter_iterator_range<Iter>;

        /**
         * @brief Constructs an iterator_range from begin and end iterators.
         *        从开始和结束迭代器构造 iterator_range。
         *
         * @param begin Iterator to the beginning of the range
         *              指向范围起始的迭代器
         * @param end Iterator to the end of the range
         *            指向范围末尾的迭代器
         */
        iterator_range(Iter begin, Iter end) : base(begin, end) {
        }
    };
}

namespace rainy::collections::views {
    using core::collections::views::iterator_range;
}

namespace rainy::utility {
    template <typename Ty>
    struct input_iterator_pointer final {
        using value_type = Ty;
        using pointer = Ty *;
        using reference = Ty &;

        constexpr input_iterator_pointer(value_type &&val) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<value_type>) : value{std::move(val)} {
        }

        RAINY_NODISCARD constexpr pointer operator->() noexcept {
            return std::addressof(value);
        }

        RAINY_NODISCARD constexpr reference operator*() noexcept {
            return value;
        }

    private:
        Ty value;
    };

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

        constexpr const_iterator() noexcept : current() {};
        constexpr const_iterator &operator=(const_iterator &&) noexcept = default;
        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;

        /**
         * @brief 构造一个常量迭代器
         * @param current 当前迭代器位置
         */
        explicit constexpr const_iterator(iterator_type current) noexcept : current(current) {
        }

        constexpr const_iterator(const const_iterator &right) : current(right.current) {
        }

        constexpr const_iterator(const_iterator &&right) noexcept : current(rainy::utility::exchange(right.current, nullptr)) {
        }

        RAINY_CONSTEXPR20 ~const_iterator() = default;

        constexpr const_pointer get_pointer() const noexcept {
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return current;
            } else {
                return current.operator->();
            }
        }

        constexpr const_pointer operator->() const noexcept {
            return get_pointer();
        }

        constexpr const_reference dereference() const noexcept {
            return *current;
        }

        constexpr const_reference operator*() const noexcept {
            return dereference();
        }

        constexpr const_iterator &self_inc_prefix() noexcept {
            ++current;
            return *this;
        }

        constexpr const_iterator self_inc_postfix() noexcept {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        constexpr const_iterator &operator++() noexcept {
            return self_inc_prefix();
        }

        constexpr const_iterator operator++(int) noexcept {
            return self_inc_postfix();
        }

        constexpr const_iterator &self_dec_prefix() noexcept {
            --current;
            return *this;
        }

        constexpr const_iterator self_dec_postfix() noexcept {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr const_iterator &operator--() noexcept {
            return self_dec_prefix();
        }

        constexpr const_iterator operator--(int) noexcept {
            return self_dec_postfix();
        }

        constexpr const_iterator &plus_equal(difference_type n) noexcept {
            current += n;
            return *this;
        }

        constexpr const_iterator &minus_equal(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        constexpr const_iterator &operator+=(difference_type n) noexcept {
            return plus_equal(n);
        }

        constexpr const_iterator &operator-=(difference_type n) noexcept {
            return minus_equal(n);
        }

        constexpr bool is_equal(const const_iterator &right) const noexcept {
            return current == right.current;
        }

        constexpr bool not_equal(const const_iterator &right) const noexcept {
            return current != right.current;
        }

        constexpr friend bool operator==(const const_iterator &left, const const_iterator &right) {
            return left.is_equal(right);
        }

        constexpr friend bool operator!=(const const_iterator &left, const const_iterator &right) {
            return left.not_equal(right);
        }

        constexpr const_reference at_subscript(difference_type idx) const noexcept {
            return current[idx];
        }

        constexpr const_reference operator[](difference_type idx) const noexcept {
            return at_subscript(idx);
        }

        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept {
            return this->current == nullptr;
        }

        constexpr explicit operator bool() const noexcept {
            return !empty();
        }

        constexpr const_iterator add_offset(difference_type n) const noexcept {
            return const_iterator{current + n};
        }

        constexpr friend const_iterator operator+(const const_iterator left, difference_type n) {
            return left.add_offset(n);
        }

        constexpr difference_type subtract(difference_type n) const noexcept {
            return current - n;
        }

        constexpr difference_type subtract(const const_iterator &right) const noexcept {
            return current - right.current;
        }

        constexpr friend difference_type operator-(const const_iterator &left, const const_iterator &right) {
            return left.subtract(right);
        }

        constexpr friend difference_type operator-(const const_iterator &left, difference_type n) {
            return left.subtract(n);
        }

        constexpr bool lt(const const_iterator &right) const noexcept {
            return current < right.current;
        }

        constexpr friend bool operator<(const const_iterator &left, const const_iterator &right) {
            return left.lt(right);
        }

        constexpr bool lt_or_equal(const const_iterator &right) const noexcept {
            return current <= right.current;
        }

        constexpr friend bool operator<=(const const_iterator &left, const const_iterator &right) {
            return left.lt_or_equal(right);
        }

        constexpr bool greater_than(const const_iterator &right) const noexcept {
            return current > right.current;
        }

        constexpr friend bool operator>(const const_iterator &left, const const_iterator &right) {
            return left.greater_than(right);
        }

        constexpr bool greater_than_or_equal(const const_iterator &right) const noexcept {
            return current >= right.current;
        }

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

    RAINY_NODISCARD static RAINY_CONSTEXPR20 elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class iterator : public const_iterator<Iter, Traits> {
    public:
        using iterator_type = Iter;
        using iterator_traits = Traits;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;

        using base = const_iterator<Iter, Traits>;

        friend struct core::memory::pointer_traits<const_iterator<Iter, Traits>>;

        constexpr iterator() noexcept : base() {};

        constexpr iterator(iterator_type current) noexcept : base(current) {
        }

        constexpr iterator(const iterator &right) : base(right) {
        }

        constexpr iterator(iterator &&right) noexcept : base(right) {
        }

        constexpr iterator &operator=(iterator &&) noexcept = default;
        constexpr iterator &operator=(const iterator &) noexcept = default;

        RAINY_CONSTEXPR20 ~iterator() = default;

        constexpr base &as_const_iterator() const noexcept {
            return *this;
        }

        constexpr pointer get_pointer() noexcept {
            return const_cast<pointer>(static_cast<const base *>(this)->get_pointer());
        }

        constexpr pointer operator->() noexcept {
            return get_pointer();
        }

        constexpr reference dereference() noexcept {
            return const_cast<reference>(static_cast<const base *>(this)->dereference());
        }

        constexpr reference operator*() noexcept {
            return dereference();
        }

        constexpr iterator add_offset(typename base::difference_type n) const noexcept {
            return iterator{this->current + n};
        }

        constexpr friend iterator operator+(const iterator left, typename base::difference_type n) {
            return left.add_offset(n);
        }

        constexpr typename base::difference_type subtract(typename base::difference_type n) const noexcept {
            return this->current - n;
        }

        constexpr typename base::difference_type subtract(const iterator &right) const noexcept {
            return this->current - right.current;
        }

        constexpr iterator &operator++() noexcept {
            ++this->current;
            return *this;
        }

        constexpr iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        constexpr iterator &operator--() noexcept {
            --this->current;
            return *this;
        }

        constexpr iterator operator--(int) noexcept {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr friend typename base::difference_type operator-(const iterator &left, const iterator &right) {
            return left.subtract(right);
        }

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

    RAINY_NODISCARD static constexpr elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
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

        RAINY_CONSTEXPR20 decltype(auto) operator*() noexcept(noexcept(static_cast<implement_type *>(this)->get_element_impl())) {
            return static_cast<implement_type *>(this)->get_element_impl();
        }

        RAINY_CONSTEXPR20 decltype(auto) operator->() noexcept(noexcept(static_cast<implement_type *>(this)->get_pointer_impl())) {
            return static_cast<implement_type *>(this)->get_pointer_impl();
        }

        RAINY_CONSTEXPR20 decltype(auto) operator*() const
            noexcept(noexcept(static_cast<const implement_type *>(this)->get_element_impl())) {
            return static_cast<const implement_type *>(this)->get_element_impl();
        }

        RAINY_CONSTEXPR20 decltype(auto) operator->() const
            noexcept(noexcept(static_cast<const implement_type *>(this)->get_pointer_impl())) {
            return static_cast<const implement_type *>(this)->get_pointer_impl();
        }

        RAINY_CONSTEXPR20 implement_type &operator++() noexcept(noexcept(static_cast<implement_type *>(this)->next_impl())) {
            static_cast<implement_type *>(this)->next_impl();
            return *static_cast<implement_type *>(this);
        }

        RAINY_CONSTEXPR20 implement_type operator++(int) noexcept(noexcept(static_cast<implement_type *>(this)->next_impl())) {
            implement_type tmp = *static_cast<implement_type *>(this);
            ++(*this);
            return tmp;
        }

        RAINY_CONSTEXPR20 implement_type &operator++() const
            noexcept(noexcept(static_cast<implement_type *>(const_cast<bidirectional_iterator *>(this))->next_impl())) {
            auto self = static_cast<implement_type *>(const_cast<bidirectional_iterator *>(this));
            self->next_impl();
            return *self;
        }

        RAINY_CONSTEXPR20 implement_type operator++(int) const {
            implement_type tmp = *static_cast<const implement_type *>(this);
            ++(*this);
            return tmp;
        }

        RAINY_CONSTEXPR20 implement_type &operator--() noexcept(noexcept(static_cast<implement_type *>(this)->back_impl())) {
            static_cast<implement_type *>(this)->back_impl();
            return *static_cast<implement_type *>(this);
        }

        RAINY_CONSTEXPR20 implement_type operator--(int) noexcept(noexcept(static_cast<implement_type *>(this)->back_impl())) {
            implement_type tmp = *static_cast<implement_type *>(this);
            --(*this);
            return tmp;
        }

        friend RAINY_CONSTEXPR20 bool operator==(const bidirectional_iterator &left, const bidirectional_iterator &right) noexcept(
            implements::is_proxy_equal_with_noexcept<bidirectional_iterator>) {
            return left.proxy_equal_with_helper(right);
        }

        friend RAINY_CONSTEXPR20 bool operator!=(const bidirectional_iterator &left,
                                                 const bidirectional_iterator &right) noexcept(noexcept(left == right)) {
            return !(left == right);
        }

        RAINY_CONSTEXPR20 bool proxy_equal_with_helper(const bidirectional_iterator &right) const noexcept(
            noexcept(static_cast<const implement_type *>(this)->equal_with_impl(static_cast<const implement_type &>(right)))) {
            return static_cast<const implement_type *>(this)->equal_with_impl(static_cast<const implement_type &>(right));
        }
    };

    template <typename MapContainer, typename Iterator>
    class map_mapped_iterator_impl
        : public utility::bidirectional_iterator<
              map_mapped_iterator_impl<MapContainer, Iterator>,
              utility::make_iterator_traits<typename std::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
                                            typename MapContainer::mapped_type *, typename MapContainer::mapped_type &,
                                            typename MapContainer::mapped_type>> {
    public:
        using base = utility::bidirectional_iterator<
            map_mapped_iterator_impl<MapContainer, Iterator>,
            utility::make_iterator_traits<typename std::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
                                          typename MapContainer::mapped_type *, typename MapContainer::mapped_type &,
                                          typename MapContainer::mapped_type>>;

        // 关键：显式构造函数接受底层迭代器
        explicit map_mapped_iterator_impl(Iterator it) : current_(it) {
        }

        // 默认构造函数
        map_mapped_iterator_impl() = default;

        // 非 const 访问（仅当 Iterator 是非 const 时可用）
        template <typename Iter = Iterator, typename = type_traits::other_trans::enable_if_t<!std::is_const_v<
                                                std::remove_reference_t<typename std::iterator_traits<Iter>::reference>>>>
        typename base::reference get_element_impl() noexcept {
            return current_->second;
        }

        template <typename Iter = Iterator, typename = type_traits::other_trans::enable_if_t<!std::is_const_v<
                                                std::remove_reference_t<typename std::iterator_traits<Iter>::reference>>>>
        typename base::pointer get_pointer_impl() noexcept {
            return utility::addressof(current_->second);
        }

        // const 访问（总是可用）
        typename base::const_reference get_element_impl() const noexcept {
            return current_->second;
        }

        typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(current_->second);
        }

        void next_impl() noexcept {
            ++current_;
        }

        void back_impl() noexcept {
            --current_;
        }

        // 支持与不同 cv 限定的迭代器比较
        template <typename OtherIterator>
        bool equal_with_impl(const map_mapped_iterator_impl<MapContainer, OtherIterator> &other) const noexcept {
            return current_ == other.current_;
        }

    private:
        Iterator current_;

        template <typename M, typename I>
        friend class map_mapped_iterator_impl;
    };

    template <typename MapContainer>
    class map_mapped_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator>;

        using base::base;
    };

    template <typename MapContainer>
    class map_mapped_const_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator>;

        using base::base;
    };

    template <typename Map>
    rain_fn mapped_range(Map &map) -> collections::views::iterator_range<map_mapped_iterator<Map>> {
        using iterator = map_mapped_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator(map.begin()), iterator(map.end())};
    }

    template <typename Map>
    rain_fn mapped_range(const Map &map) -> collections::views::iterator_range<map_mapped_const_iterator<Map>> {
        using iterator = map_mapped_const_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator(map.cbegin()), iterator(map.cend())};
    }

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
        explicit map_key_const_iterator(Ty it) : current_(it) {
        }

        typename base::const_reference get_element_impl() const noexcept {
            return current_->first;
        }

        typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(current_->first);
        }

        void next_impl() noexcept {
            ++current_;
        }

        void back_impl() noexcept {
            --current_;
        }

        bool equal_with_impl(const map_key_const_iterator &other) const noexcept {
            return current_ == other.current_;
        }

    private:
        typename MapContainer::const_iterator current_;
    };

    template <typename MapContainer>
    class map_key_iterator : public map_key_const_iterator<MapContainer> {
    public:
        using base = typename map_key_const_iterator<MapContainer>::base;

        using const_iterator = map_key_const_iterator<MapContainer>;
        using const_iterator::const_iterator;

        typename base::pointer get_pointer_impl() noexcept {
            return utility::addressof(this->current_->first);
        }

        typename base::reference &get_element_impl() noexcept {
            return const_cast<typename base::reference &>(this->current_->first);
        }
    };

    template <typename Map>
    rain_fn keyed_range(Map &map) -> collections::views::iterator_range<map_key_iterator<Map>> {
        using iterator = map_key_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator{map.begin()}, iterator{map.end()}};
    }

    template <typename Map>
    rain_fn keyed_range(const Map &map) -> collections::views::iterator_range<map_key_iterator<Map>> {
        using iterator = map_key_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator{map.begin()}, iterator{map.end()}};
    }
}

#endif
