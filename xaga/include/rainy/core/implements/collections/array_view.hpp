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
#ifndef RAINY_CORE_IMPLEMENTS_COLLECTIONS_ARRAY_VIEW_HPP
#define RAINY_CORE_IMPLEMENTS_COLLECTIONS_ARRAY_VIEW_HPP
#include <vector>
#include <array>
#include <rainy/core/platform.hpp>
#include <rainy/core/implements/reverse_iterator.hpp>
#include <rainy/core/implements/collections/array.hpp>

namespace rainy::collections::views {
    /**
     * @brief A non-owning view over a contiguous sequence of elements.
     *        对连续元素序列的非拥有视图。
     *
     * @tparam Ty The element type
     *            元素类型
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
         * @brief Default constructor. Creates an empty view.
         *        默认构造函数。创建一个空视图。
         */
        constexpr array_view() noexcept = default;

        /**
         * @brief Constructs a view from a pointer and size.
         *        从指针和大小构造视图。
         *
         * @param data Pointer to the data
         *             指向数据的指针
         * @param size Number of elements
         *             元素数量
         */
        constexpr array_view(pointer data, size_type size) noexcept : data_(data), size_(size) {
        }

        /**
         * @brief Constructs a const view from a const pointer and size.
         *        从常量指针和大小构造常量视图。
         *
         * @param data Const pointer to the data
         *             指向数据的常量指针
         * @param size Number of elements
         *             元素数量
         */
        constexpr array_view(const_pointer data, size_type size) noexcept : data_(const_cast<pointer>(data)), size_(size) {
        }

        /**
         * @brief Constructs a view from a range defined by two pointers.
         *        从两个指针定义的范围构造视图。
         *
         * @param first Pointer to the first element
         *              指向第一个元素的指针
         * @param last Pointer to one past the last element
         *             指向最后一个元素之后位置的指针
         */
        constexpr array_view(const_pointer first, const_pointer last) noexcept : data_(first), size_(last - first) {
        }

        /**
         * @brief Constructs a view from an initializer list.
         *        从初始化列表构造视图。
         *
         * @param value Initializer list
         *              初始化列表
         */
        constexpr array_view(std::initializer_list<value_type> value) noexcept :
            data_(const_cast<pointer>(value.begin())), size_(value.size()) {
        }

        /**
         * @brief Constructs a view from a C-style array.
         *        从C风格数组构造视图。
         *
         * @tparam C Array element type
         *           数组元素类型
         * @tparam N Array size
         *           数组大小
         * @param value C-style array reference
         *              C风格数组引用
         */
        template <typename C, size_type N>
        constexpr array_view(C (&value)[N]) noexcept : array_view(value, N) {
        }

        /**
         * @brief Constructs a view from a std::vector.
         *        从std::vector构造视图。
         *
         * @tparam C Vector element type
         *           vector元素类型
         * @param value Vector reference
         *              vector引用
         */
        template <typename C>
        RAINY_CONSTEXPR20 array_view(std::vector<C> &value) noexcept : array_view(data(value), value.size()) {
        }

        /**
         * @brief Constructs a const view from a const std::vector.
         *        从const std::vector构造常量视图。
         *
         * @tparam C Vector element type
         *           vector元素类型
         * @param value Const vector reference
         *              const vector引用
         */
        template <typename C>
        RAINY_CONSTEXPR20 array_view(std::vector<C> const &value) noexcept : array_view(data(value), value.size()) {
        }

        /**
         * @brief Constructs a view from a std::array.
         *        从std::array构造视图。
         *
         * @tparam C Array element type
         *           数组元素类型
         * @tparam N Array size
         *           数组大小
         * @param value std::array reference
         *              std::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(std::array<C, N> &value) noexcept : array_view(value.data(), value.size()) {
        }

        /**
         * @brief Constructs a const view from a const std::array.
         *        从const std::array构造常量视图。
         *
         * @tparam C Array element type
         *           数组元素类型
         * @tparam N Array size
         *           数组大小
         * @param value Const std::array reference
         *              const std::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(std::array<C, N> const &value) noexcept : array_view(value.data(), value.size()) {
        }

        /**
         * @brief Constructs a view from a collections::array.
         *        从collections::array构造视图。
         *
         * @tparam C Array element type
         *           数组元素类型
         * @tparam N Array size
         *           数组大小
         * @param value collections::array reference
         *              collections::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(collections::array<C, N> &value) noexcept : array_view(value.data(), N) {
        }

        /**
         * @brief Constructs a const view from a const collections::array.
         *        从const collections::array构造常量视图。
         *
         * @tparam C Array element type
         *           数组元素类型
         * @tparam N Array size
         *           数组大小
         * @param value Const collections::array reference
         *              const collections::array引用
         */
        template <typename C, std::size_t N>
        constexpr array_view(collections::array<C, N> const &value) noexcept : array_view(value.data(), N) {
        }

        /**
         * @brief Converting constructor from array_view of different type.
         *        从不同类型的array_view的转换构造函数。
         *
         * @tparam OtherType Source element type
         *                   源元素类型
         * @param other Source array_view
         *              源array_view
         */
        template <typename OtherType, type_traits::other_trans::enable_if_t<
                                          type_traits::type_relations::is_convertible_v<OtherType (*)[], Ty (*)[]>, int> = 0>
        constexpr array_view(array_view<OtherType> const &other) noexcept : array_view(other.data(), other.size()) {
        }

        /**
         * @brief Accesses element at specified position with bounds checking (debug only).
         *        访问指定位置的元素（仅在debug模式下进行边界检查）。
         *
         * @param pos Position index
         *            位置索引
         * @return Reference to the element
         *         元素的引用
         */
        constexpr reference operator[](size_type const pos) noexcept {
            assert(pos < size());
            return data_[pos];
        }

        /**
         * @brief Accesses element at specified position with bounds checking (const version, debug only).
         *        访问指定位置的元素（常量版本，仅在debug模式下进行边界检查）。
         *
         * @param pos Position index
         *            位置索引
         * @return Const reference to the element
         *         元素的常量引用
         */
        constexpr const_reference operator[](size_type const pos) const noexcept {
            assert(pos < size());
            return data_[pos];
        }

        /**
         * @brief Accesses element at specified position with bounds checking (throws on error).
         *        访问指定位置的元素（错误时抛出异常）。
         *
         * @param pos Position index
         *            位置索引
         * @return Reference to the element
         *         元素的引用
         */
        constexpr reference at(size_type const pos) noexcept {
            if (size() <= pos) {
                std::terminate();
            }
            return data_[pos];
        }

        /**
         * @brief Accesses element at specified position with bounds checking (const version, throws on error).
         *        访问指定位置的元素（常量版本，错误时抛出异常）。
         *
         * @param pos Position index
         *            位置索引
         * @return Const reference to the element
         *         元素的常量引用
         */
        constexpr const_reference at(size_type const pos) const {
            if (size() <= pos) {
                std::terminate();
            }
            return data_[pos];
        }

        /**
         * @brief Accesses the first element.
         *        访问第一个元素。
         *
         * @return Reference to the first element
         *         第一个元素的引用
         */
        constexpr reference front() noexcept {
            assert(size_ > 0);
            return *data_;
        }

        /**
         * @brief Accesses the first element (const version).
         *        访问第一个元素（常量版本）。
         *
         * @return Const reference to the first element
         *         第一个元素的常量引用
         */
        constexpr const_reference front() const noexcept {
            assert(size_ > 0);
            return *data_;
        }

        /**
         * @brief Accesses the last element.
         *        访问最后一个元素。
         *
         * @return Reference to the last element
         *         最后一个元素的引用
         */
        constexpr reference back() noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        /**
         * @brief Accesses the last element (const version).
         *        访问最后一个元素（常量版本）。
         *
         * @return Const reference to the last element
         *         最后一个元素的常量引用
         */
        constexpr const_reference back() const noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        /**
         * @brief Returns a pointer to the underlying data.
         *        返回指向底层数据的指针。
         *
         * @return Pointer to the data
         *         指向数据的指针
         */
        constexpr pointer data() const noexcept {
            return data_;
        }

        /**
         * @brief Returns an iterator to the beginning.
         *        返回指向起始的迭代器。
         *
         * @return Iterator to the first element
         *         指向第一个元素的迭代器
         */
        constexpr iterator begin() noexcept {
            return data_;
        }

        /**
         * @brief Returns a const iterator to the beginning.
         *        返回指向起始的常量迭代器。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        constexpr const_iterator begin() const noexcept {
            return data_;
        }

        /**
         * @brief Returns a const iterator to the beginning.
         *        返回指向起始的常量迭代器。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        constexpr const_iterator cbegin() const noexcept {
            return data_;
        }

        /**
         * @brief Returns an iterator to the end.
         *        返回指向末尾的迭代器。
         *
         * @return Iterator to one past the last element
         *         指向最后一个元素之后位置的迭代器
         */
        constexpr iterator end() noexcept {
            return data_ + size_;
        }

        /**
         * @brief Returns a const iterator to the end.
         *        返回指向末尾的常量迭代器。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        constexpr const_iterator end() const noexcept {
            return data_ + size_;
        }

        /**
         * @brief Returns a const iterator to the end.
         *        返回指向末尾的常量迭代器。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        constexpr const_iterator cend() const noexcept {
            return data_ + size_;
        }

        /**
         * @brief Returns a reverse iterator to the beginning.
         *        返回指向起始的反向迭代器。
         *
         * @return Reverse iterator to the first element
         *         指向第一个元素的反向迭代器
         */
        constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        /**
         * @brief Returns a const reverse iterator to the beginning.
         *        返回指向起始的常量反向迭代器。
         *
         * @return Const reverse iterator to the first element
         *         指向第一个元素的常量反向迭代器
         */
        constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        /**
         * @brief Returns a const reverse iterator to the beginning.
         *        返回指向起始的常量反向迭代器。
         *
         * @return Const reverse iterator to the first element
         *         指向第一个元素的常量反向迭代器
         */
        constexpr const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        /**
         * @brief Returns a reverse iterator to the end.
         *        返回指向末尾的反向迭代器。
         *
         * @return Reverse iterator to one past the last element
         *         指向最后一个元素之后位置的反向迭代器
         */
        constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        /**
         * @brief Returns a const reverse iterator to the end.
         *        返回指向末尾的常量反向迭代器。
         *
         * @return Const reverse iterator to one past the last element
         *         指向最后一个元素之后位置的常量反向迭代器
         */
        constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief Returns a const reverse iterator to the end.
         *        返回指向末尾的常量反向迭代器。
         *
         * @return Const reverse iterator to one past the last element
         *         指向最后一个元素之后位置的常量反向迭代器
         */
        constexpr const_reverse_iterator crend() const noexcept {
            return rend();
        }

        /**
         * @brief Checks if the view is empty.
         *        检查视图是否为空。
         *
         * @return true if size() == 0, false otherwise
         *         如果size() == 0则为true，否则为false
         */
        constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        /**
         * @brief Returns the number of elements.
         *        返回元素数量。
         *
         * @return Number of elements
         *         元素数量
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
     * @brief Deduction guide for array_view from C-style array.
     *        从C风格数组推导array_view类型的指引。
     */
    template <typename C, std::size_t N>
    array_view(C (&value)[N]) -> array_view<C>;

    /**
     * @brief Deduction guide for array_view from std::vector.
     *        从std::vector推导array_view类型的指引。
     */
    template <typename C>
    array_view(std::vector<C> &value) -> array_view<C>;

    /**
     * @brief Deduction guide for const array_view from const std::vector.
     *        从const std::vector推导const array_view类型的指引。
     */
    template <typename C>
    array_view(std::vector<C> const &value) -> array_view<C const>;

    /**
     * @brief Deduction guide for array_view from std::array.
     *        从std::array推导array_view类型的指引。
     */
    template <typename C, std::size_t N>
    array_view(std::array<C, N> &value) -> array_view<C>;

    /**
     * @brief Deduction guide for const array_view from const std::array.
     *        从const std::array推导const array_view类型的指引。
     */
    template <typename C, std::size_t N>
    array_view(std::array<C, N> const &value) -> array_view<C const>;

    /**
     * @brief Creates an array_view from a pointer range.
     *        从指针范围创建array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @param first Pointer to the first element
     *              指向第一个元素的指针
     * @param last Pointer to one past the last element
     *             指向最后一个元素之后位置的指针
     * @return array_view over the range [first, last)
     *         覆盖范围[first, last)的array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(Ty *first, Ty *last) -> array_view<Ty> {
        return array_view<Ty>(first, last);
    }

    /**
     * @brief Creates a const array_view from a const pointer range.
     *        从常量指针范围创建常量array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @param first Const pointer to the first element
     *              指向第一个元素的常量指针
     * @param last Const pointer to one past the last element
     *             指向最后一个元素之后位置的常量指针
     * @return const array_view over the range [first, last)
     *         覆盖范围[first, last)的常量array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const Ty *first, const Ty *last) -> array_view<Ty> {
        return array_view<Ty>(first, last);
    }

    /**
     * @brief Creates an array_view from a C-style array.
     *        从C风格数组创建array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param array C-style array reference
     *              C风格数组引用
     * @return array_view over the entire array
     *         覆盖整个数组的array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(Ty (&array)[N]) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * @brief Creates an array_view from a std::vector.
     *        从std::vector创建array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @param vector std::vector reference
     *               std::vector引用
     * @return array_view over the vector's elements
     *         覆盖vector元素的array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(std::vector<Ty> &vector) -> array_view<Ty> {
        return array_view<Ty>(vector);
    }

    /**
     * @brief Creates a const array_view from a const std::vector.
     *        从const std::vector创建常量array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @param vector Const std::vector reference
     *               const std::vector引用
     * @return const array_view over the vector's elements
     *         覆盖vector元素的常量array_view
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const std::vector<Ty> &vector) -> array_view<Ty> {
        return array_view<Ty>(vector);
    }

    /**
     * @brief Creates an array_view from a std::array.
     *        从std::array创建array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param array std::array reference
     *              std::array引用
     * @return array_view over the array's elements
     *         覆盖array元素的array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(std::array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * @brief Creates a const array_view from a const std::array.
     *        从const std::array创建常量array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param array Const std::array reference
     *              const std::array引用
     * @return const array_view over the array's elements
     *         覆盖array元素的常量array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const std::array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * @brief Creates an array_view from a collections::array.
     *        从collections::array创建array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param array collections::array reference
     *              collections::array引用
     * @return array_view over the array's elements
     *         覆盖array元素的array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }

    /**
     * @brief Creates a const array_view from a const collections::array.
     *        从const collections::array创建常量array_view。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param array Const collections::array reference
     *              const collections::array引用
     * @return const array_view over the array's elements
     *         覆盖array元素的常量array_view
     */
    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 rain_fn make_array_view(const array<Ty, N> &array) -> array_view<Ty> {
        return array_view<Ty>(array);
    }
}

#endif
