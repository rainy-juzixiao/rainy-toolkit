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
#ifndef RAINY_CORE_COLLECTIONS_ARRAY_VIEW_HPP
#define RAINY_CORE_COLLECTIONS_ARRAY_VIEW_HPP
#include <array>
#include <exception>
#include <rainy/core/platform.hpp>
#include <rainy/core/utility/reverse_iterator.hpp>
#include <vector>

namespace rainy::core::collections {
    template <typename Ty, typename Alloc>
    class vector;

    template <typename Ty, std::size_t Alloc>
    class array;
}

namespace rainy::core::collections::views {
    /**
     * \lang english
     * @brief A non-owning view over a contiguous sequence of elements.
     *
     * @tparam Ty The element type
     *
     * \lang simp-chinese
     * @brief 对连续元素序列的非拥有视图。
     *
     * @tparam Ty 元素类型
     */
    template <typename Ty>
    struct array_view {
        using value_type = Ty;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;
        using iterator = value_type *;
        using const_iterator = value_type const *;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        /**
         * \lang english
         * @brief Default constructor. Creates an empty view.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。创建一个空视图。
         */
        constexpr array_view() noexcept = default;

        /**
         * \lang english
         * @brief Constructs a view from a pointer and size.
         *
         * @param data Pointer to the data
         * @param size Number of elements
         *
         * \lang simp-chinese
         * @brief 从指针和大小构造视图。
         *
         * @param data 指向数据的指针
         * @param size 元素数量
         */
        constexpr array_view(pointer data, size_type size) noexcept : data_(data), size_(size) {
        }

        /**
         * \lang english
         * @brief Constructs a const view from a const pointer and size.
         *
         * @param data Const pointer to the data
         * @param size Number of elements
         *
         * \lang simp-chinese
         * @brief 从常量指针和大小构造常量视图。
         *
         * @param data 指向数据的常量指针
         * @param size 元素数量
         */
        constexpr array_view(const_pointer data, size_type size) noexcept : data_(const_cast<pointer>(data)), size_(size) {
        }

        /**
         * \lang english
         * @brief Constructs a view from a range defined by two pointers.
         *
         * @param first Pointer to the first element
         * @param last Pointer to one past the last element
         *
         * \lang simp-chinese
         * @brief 从两个指针定义的范围构造视图。
         *
         * @param first 指向第一个元素的指针
         * @param last 指向最后一个元素之后位置的指针
         */
        constexpr array_view(const_pointer first, const_pointer last) noexcept :
            data_(const_cast<pointer>(first)), size_(last - first) {
        }

        /**
         * \lang english
         * @brief Constructs a view from an initializer list.
         *
         * @param value Initializer list
         *
         * \lang simp-chinese
         * @brief 从初始化列表构造视图。
         *
         * @param value 初始化列表
         */
        constexpr array_view(std::initializer_list<value_type> value) noexcept :
            data_(const_cast<pointer>(value.begin())), size_(value.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a view from a C-style array.
         *
         * @tparam C Array element type
         * @tparam N Array size
         * @param value C-style array reference
         *
         * \lang simp-chinese
         * @brief 从C风格数组构造视图。
         *
         * @tparam C 数组元素类型
         * @tparam N 数组大小
         * @param value C风格数组引用
         */
        template <typename C, size_type N>
        constexpr array_view(C (&value)[N]) noexcept : array_view(value, N) {
        }

        /**
         * \lang english
         * @brief Constructs a view from a std::vector.
         *
         * @tparam C Vector element type
         * @param value Vector reference
         *
         * \lang simp-chinese
         * @brief 从std::vector构造视图。
         *
         * @tparam C vector元素类型
         * @param value vector引用
         */
        template <typename C>
        RAINY_CONSTEXPR20 array_view(std::vector<C> &value) noexcept : array_view(data(value), value.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a const view from a const std::vector.
         *
         * @tparam C Vector element type
         * @param value Const vector reference
         *
         * \lang simp-chinese
         * @brief 从const std::vector构造常量视图。
         *
         * @tparam C vector元素类型
         * @param value const vector引用
         */
        template <typename C>
        RAINY_CONSTEXPR20 array_view(std::vector<C> const &value) noexcept : array_view(data(value), value.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a view from a std::array.
         *
         * @tparam C Array element type
         * @tparam N Array size
         * @param value std::array reference
         *
         * \lang simp-chinese
         * @brief 从std::array构造视图。
         *
         * @tparam C 数组元素类型
         * @tparam N 数组大小
         * @param value std::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(std::array<C, N> &value) noexcept : array_view(value.data(), value.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a const view from a const std::array.
         *
         * @tparam C Array element type
         * @tparam N Array size
         * @param value Const std::array reference
         *
         * \lang simp-chinese
         * @brief 从const std::array构造常量视图。
         *
         * @tparam C 数组元素类型
         * @tparam N 数组大小
         * @param value const std::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(std::array<C, N> const &value) noexcept : array_view(value.data(), value.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a view from a collections::array.
         *
         * @tparam C Array element type
         * @tparam N Array size
         * @param value collections::array reference
         *
         * \lang simp-chinese
         * @brief 从collections::array构造视图。
         *
         * @tparam C 数组元素类型
         * @tparam N 数组大小
         * @param value collections::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(collections::array<C, N> &value) noexcept : array_view(value.data(), N) {
        }

        /**
         * \lang english
         * @brief Constructs a const view from a const collections::array.
         *
         * @tparam C Array element type
         * @tparam N Array size
         * @param value Const collections::array reference
         *
         * \lang simp-chinese
         * @brief 从const collections::array构造常量视图。
         *
         * @tparam C 数组元素类型
         * @tparam N 数组大小
         * @param value const collections::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(collections::array<C, N> const &value) noexcept : array_view(value.data(), N) {
        }

        /**
         * \lang english
         * @brief Converting constructor from array_view of different type.
         *
         * @tparam OtherType Source element type
         * @param other Source array_view
         *
         * \lang simp-chinese
         * @brief 从不同类型的array_view的转换构造函数。
         *
         * @tparam OtherType 源元素类型
         * @param other 源array_view
         */
        template <typename OtherType, type_traits::other_trans::enable_if_t<
                                          type_traits::type_relations::is_convertible_v<OtherType (*)[], Ty (*)[]>, int> = 0>
        constexpr array_view(array_view<OtherType> const &other) noexcept : array_view(other.data(), other.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a view from a collections::vector.
         *
         * @tparam C Vector element type
         * @tparam Alloc Vector Allocator type
         * @param value Vector reference
         *
         * \lang simp-chinese
         * @brief 从collections::vector构造视图。
         *
         * @tparam C vector元素类型
         * @tparam Alloc vector分配器类型
         * @param value vector引用
         */
        template <typename C, typename Alloc>
        RAINY_CONSTEXPR20 array_view(collections::vector<C, Alloc> &value) noexcept : array_view(data(value), value.size()) {
        }

        /**
         * \lang english
         * @brief Constructs a const view from a const collections::vector.
         *
         * @tparam C Vector element type
         * @tparam Alloc Vector Allocator type
         * @param value Const vector reference
         *
         * \lang simp-chinese
         * @brief 从const collections::vector构造常量视图。
         *
         * @tparam C vector元素类型
         * @tparam Alloc vector分配器类型
         * @param value const vector引用
         */
        template <typename C, typename Alloc>
        RAINY_CONSTEXPR20 array_view(collections::vector<C, Alloc> const &value) noexcept : array_view(data(value), value.size()) {
        }

        /**
         * \lang english
         * @brief Accesses element at specified position with bounds checking (debug only).
         *
         * @param pos Position index
         * @return Reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定位置的元素（仅在debug模式下进行边界检查）。
         *
         * @param pos 位置索引
         * @return 元素的引用
         */
        constexpr reference operator[](size_type const pos) noexcept {
            assert(pos < size());
            return data_[pos];
        }

        /**
         * \lang english
         * @brief Accesses element at specified position with bounds checking (const version, debug only).
         *
         * @param pos Position index
         * @return Const reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定位置的元素（常量版本，仅在debug模式下进行边界检查）。
         *
         * @param pos 位置索引
         * @return 元素的常量引用
         */
        constexpr const_reference operator[](size_type const pos) const noexcept {
            assert(pos < size());
            return data_[pos];
        }

        /**
         * \lang english
         * @brief Accesses element at specified position with bounds checking (throws on error).
         *
         * @param pos Position index
         * @return Reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定位置的元素（错误时抛出异常）。
         *
         * @param pos 位置索引
         * @return 元素的引用
         */
        constexpr reference at(size_type const pos) noexcept {
            if (size() <= pos) {
                std::terminate();
            }
            return data_[pos];
        }

        /**
         * \lang english
         * @brief Accesses element at specified position with bounds checking (const version, throws on error).
         *
         * @param pos Position index
         * @return Const reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定位置的元素（常量版本，错误时抛出异常）。
         *
         * @param pos 位置索引
         * @return 元素的常量引用
         */
        constexpr const_reference at(size_type const pos) const {
            if (size() <= pos) {
                std::terminate();
            }
            return data_[pos];
        }

        /**
         * \lang english
         * @brief Accesses the first element.
         *
         * @return Reference to the first element
         *
         * \lang simp-chinese
         * @brief 访问第一个元素。
         *
         * @return 第一个元素的引用
         */
        constexpr reference front() noexcept {
            assert(size_ > 0);
            return *data_;
        }

        /**
         * \lang english
         * @brief Accesses the first element (const version).
         *
         * @return Const reference to the first element
         *
         * \lang simp-chinese
         * @brief 访问第一个元素（常量版本）。
         *
         * @return 第一个元素的常量引用
         */
        constexpr const_reference front() const noexcept {
            assert(size_ > 0);
            return *data_;
        }

        /**
         * \lang english
         * @brief Accesses the last element.
         *
         * @return Reference to the last element
         *
         * \lang simp-chinese
         * @brief 访问最后一个元素。
         *
         * @return 最后一个元素的引用
         */
        constexpr reference back() noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        /**
         * \lang english
         * @brief Accesses the last element (const version).
         *
         * @return Const reference to the last element
         *
         * \lang simp-chinese
         * @brief 访问最后一个元素（常量版本）。
         *
         * @return 最后一个元素的常量引用
         */
        constexpr const_reference back() const noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        /**
         * \lang english
         * @brief Returns a pointer to the underlying data.
         *
         * @return Pointer to the data
         *
         * \lang simp-chinese
         * @brief 返回指向底层数据的指针。
         *
         * @return 指向数据的指针
         */
        constexpr pointer data() const noexcept {
            return data_;
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
        constexpr iterator begin() noexcept {
            return data_;
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
        constexpr const_iterator begin() const noexcept {
            return data_;
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
        constexpr const_iterator cbegin() const noexcept {
            return data_;
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
        constexpr iterator end() noexcept {
            return data_ + size_;
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
        constexpr const_iterator end() const noexcept {
            return data_ + size_;
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
        constexpr const_iterator cend() const noexcept {
            return data_ + size_;
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator to the beginning.
         *
         * @return Reverse iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的反向迭代器。
         *
         * @return 指向第一个元素的反向迭代器
         */
        constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the beginning.
         *
         * @return Const reverse iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量反向迭代器。
         *
         * @return 指向第一个元素的常量反向迭代器
         */
        constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the beginning.
         *
         * @return Const reverse iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量反向迭代器。
         *
         * @return 指向第一个元素的常量反向迭代器
         */
        constexpr const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator to the end.
         *
         * @return Reverse iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的反向迭代器。
         *
         * @return 指向最后一个元素之后位置的反向迭代器
         */
        constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the end.
         *
         * @return Const reverse iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量反向迭代器。
         *
         * @return 指向最后一个元素之后位置的常量反向迭代器
         */
        constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the end.
         *
         * @return Const reverse iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量反向迭代器。
         *
         * @return 指向最后一个元素之后位置的常量反向迭代器
         */
        constexpr const_reverse_iterator crend() const noexcept {
            return rend();
        }

        /**
         * \lang english
         * @brief Checks if the view is empty.
         *
         * @return true if size() == 0, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否为空。
         *
         * @return 如果size() == 0则为true，否则为false
         */
        constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        /**
         * \lang english
         * @brief Returns the number of elements.
         *
         * @return Number of elements
         *
         * \lang simp-chinese
         * @brief 返回元素数量。
         *
         * @return 元素数量
         */
        constexpr size_type size() const noexcept {
            return size_;
        }

    private:
        template <typename C>
        RAINY_CONSTEXPR20 auto data(std::vector<C> const &value) noexcept {
            static_assert(!type_traits::type_relations::is_same_v<C, bool>,
                          "Cannot use std::vector<bool> as an array_view. Consider std::array or std::unique_ptr<bool[]>.");
            return value.data();
        }

        template <typename C>
        RAINY_CONSTEXPR20 auto data(std::vector<C> &value) noexcept {
            static_assert(!type_traits::type_relations::is_same_v<C, bool>,
                          "Cannot use std::vector<bool> as an array_view. Consider std::array or std::unique_ptr<bool[]>.");
            return value.data();
        }

        pointer data_{nullptr};
        size_type size_{0};
    };

    /**
     * \lang english
     * @brief Deduction guide for array_view from C-style array.
     *
     * \lang simp-chinese
     * @brief 从C风格数组推导array_view类型的指引。
     */
    template <typename C, std::size_t N>
    array_view(C (&value)[N]) -> array_view<C>;

    /**
     * \lang english
     * @brief Deduction guide for array_view from std::vector.
     *
     * \lang simp-chinese
     * @brief 从std::vector推导array_view类型的指引。
     */
    template <typename C>
    array_view(std::vector<C> &value) -> array_view<C>;

    /**
     * \lang english
     * @brief Deduction guide for const array_view from const std::vector.
     *
     * \lang simp-chinese
     * @brief 从const std::vector推导const array_view类型的指引。
     */
    template <typename C>
    array_view(std::vector<C> const &value) -> array_view<C const>;

    /**
     * \lang english
     * @brief Deduction guide for array_view from std::array.
     *
     * \lang simp-chinese
     * @brief 从std::array推导array_view类型的指引。
     */
    template <typename C, std::size_t N>
    array_view(std::array<C, N> &value) -> array_view<C>;

    /**
     * \lang english
     * @brief Deduction guide for const array_view from const std::array.
     *
     * \lang simp-chinese
     * @brief 从const std::array推导const array_view类型的指引。
     */
    template <typename C, std::size_t N>
    array_view(std::array<C, N> const &value) -> array_view<C const>;

    /**
     * \lang english
     * @brief Creates an array_view from a pointer range.
     *
     * @tparam Ty Element type
     * @param first Pointer to the first element
     * @param last Pointer to one past the last element
     * @return array_view over the range [first, last)
     *
     * \lang simp-chinese
     * @brief 从指针范围创建array_view。
     *
     * @tparam Ty 元素类型
     * @param first 指向第一个元素的指针
     * @param last 指向最后一个元素之后位置的指针
     * @return 覆盖范围[first, last)的array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(Ty *first, Ty *last) -> array_view<Ty> {
        return array_view<Ty>(first, last);
    }

    /**
     * \lang english
     * @brief Creates a const array_view from a const pointer range.
     *
     * @tparam Ty Element type
     * @param first Const pointer to the first element
     * @param last Const pointer to one past the last element
     * @return const array_view over the range [first, last)
     *
     * \lang simp-chinese
     * @brief 从常量指针范围创建常量array_view。
     *
     * @tparam Ty 元素类型
     * @param first 指向第一个元素的常量指针
     * @param last 指向最后一个元素之后位置的常量指针
     * @return 覆盖范围[first, last)的常量array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const Ty *first, const Ty *last) -> array_view<Ty> {
        return array_view<Ty>(first, last);
    }

    /**
     * \lang english
     * @brief Creates an array_view from a C-style array.
     *
     * @tparam Ty Element type
     * @tparam N Array size
     * @param array C-style array reference
     * @return array_view over the entire array
     *
     * \lang simp-chinese
     * @brief 从C风格数组创建array_view。
     *
     * @tparam Ty 元素类型
     * @tparam N 数组大小
     * @param array C风格数组引用
     * @return 覆盖整个数组的array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(Ty (&array)[N]) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * \lang english
     * @brief Creates an array_view from a std::vector.
     *
     * @tparam Ty Element type
     * @param vector std::vector reference
     * @return array_view over the vector's elements
     *
     * \lang simp-chinese
     * @brief 从std::vector创建array_view。
     *
     * @tparam Ty 元素类型
     * @param vector std::vector引用
     * @return 覆盖vector元素的array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(std::vector<Ty> &vector) -> array_view<Ty> {
        return array_view<Ty>(vector);
    }

    /**
     * \lang english
     * @brief Creates a const array_view from a const std::vector.
     *
     * @tparam Ty Element type
     * @param vector Const std::vector reference
     * @return const array_view over the vector's elements
     *
     * \lang simp-chinese
     * @brief 从const std::vector创建常量array_view。
     *
     * @tparam Ty 元素类型
     * @param vector const std::vector引用
     * @return 覆盖vector元素的常量array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const std::vector<Ty> &vector) -> array_view<Ty> {
        return array_view<Ty>(vector);
    }

    /**
     * \lang english
     * @brief Creates an array_view from a std::array.
     *
     * @tparam Ty Element type
     * @tparam N Array size
     * @param array std::array reference
     * @return array_view over the array's elements
     *
     * \lang simp-chinese
     * @brief 从std::array创建array_view。
     *
     * @tparam Ty 元素类型
     * @tparam N 数组大小
     * @param array std::array引用
     * @return 覆盖array元素的array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(std::array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * \lang english
     * @brief Creates a const array_view from a const std::array.
     *
     * @tparam Ty Element type
     * @tparam N Array size
     * @param array Const std::array reference
     * @return const array_view over the array's elements
     *
     * \lang simp-chinese
     * @brief 从const std::array创建常量array_view。
     *
     * @tparam Ty 元素类型
     * @tparam N 数组大小
     * @param array const std::array引用
     * @return 覆盖array元素的常量array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const std::array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * \lang english
     * @brief Creates an array_view from a collections::array.
     *
     * @tparam Ty Element type
     * @tparam N Array size
     * @param array collections::array reference
     * @return array_view over the array's elements
     *
     * \lang simp-chinese
     * @brief 从collections::array创建array_view。
     *
     * @tparam Ty 元素类型
     * @tparam N 数组大小
     * @param array collections::array引用
     * @return 覆盖array元素的array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * \lang english
     * @brief Creates a const array_view from a const collections::array.
     *
     * @tparam Ty Element type
     * @tparam N Array size
     * @param array Const collections::array reference
     * @return const array_view over the array's elements
     *
     * \lang simp-chinese
     * @brief 从const collections::array创建常量array_view。
     *
     * @tparam Ty 元素类型
     * @tparam N 数组大小
     * @param array const collections::array引用
     * @return 覆盖array元素的常量array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }
}

namespace rainy::collections::views {
    using rainy::core::collections::views::array_view;
    using rainy::core::collections::views::make_array_view;
}


#endif
