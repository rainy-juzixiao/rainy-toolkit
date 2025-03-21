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
#ifndef RAINY_ARRAY_HPP
#define RAINY_ARRAY_HPP
#include <algorithm>
#include <array>
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/utility/iterator.hpp>

namespace rainy::containers {
    template <typename Ty, std::size_t N>
    class array final {
    public:
        using value_type = Ty;
        using size_type = std::size_t;
        using reference = Ty &;
        using const_reference = const Ty &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using difference_type = std::ptrdiff_t;
        /* 普通迭代器 */
        using iterator = utility::iterator<pointer>;
        using const_iterator = utility::const_iterator<const_pointer>;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<iterator>;

        RAINY_CONSTEXPR20 ~array() = default;

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference at(difference_type off) {
            range_check(off);
            return elems_[off];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE const_reference at(difference_type off) const {
            check_zero_length_array();
            range_check(off);
            return elems_[off];
        }

        /**
         * @brief 获取当前数组第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference front() {
            check_zero_length_array();
            return elems_[0];
        }

        /**
         * @brief 获取当前数组第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_reference front() const {
            check_zero_length_array();
            return elems_[0];
        }

        /**
         * @brief 获取当前数组最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference back() {
            check_zero_length_array();
            return elems_[N - 1];
        }

        /**
         * @brief 获取当前数组最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_reference back() const {
            check_zero_length_array();
            return elems_[N - 1];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference operator[](difference_type idx) noexcept {
            return elems_[idx]; // NOLINT
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_reference operator[](difference_type idx) const noexcept {
            return elems_[idx]; // NOLINT
        }

        /**
         * @brief 将一个对象的数据与当前对象的数据进行交换
         * @param right 待交换的数组对象
         */
        RAINY_ALWAYS_INLINE void swap(array &right) noexcept(std::is_nothrow_swappable_v<Ty>) {
            check_zero_length_array();
            std::swap_ranges(elems_, right.elems_);
        }

        /**
         * @brief 将一个值填充到数组中
         * @param value 要填充的值
         */
        RAINY_CONSTEXPR20 void fill(const Ty &value) {
            check_zero_length_array();
            std::fill_n(begin(), size(), value);
        }

        /**
         * @brief 获取当前数组大小
         * @return 返回当前数组大小
         */
        RAINY_ALWAYS_INLINE constexpr static size_type size() noexcept {
            return N;
        }

        /**
         * @brief 检查当前数组容器是否为空
         * @return 容器状态
         */
        RAINY_ALWAYS_INLINE constexpr static bool empty() noexcept {
            return size() == 0;
        }

        /**
         * @brief 获取当前数组地址
         * @return 返回数组的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr pointer data() noexcept {
            return elems_;
        }

        /**
         * @brief 获取当前数组地址
         * @return 返回数组的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_pointer data() const noexcept {
            return elems_;
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWAYS_INLINE constexpr iterator begin() {
            check_zero_length_array();
            return iterator(elems_);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_iterator begin() const {
            check_zero_length_array();
            return const_iterator(elems_);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器常量
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_iterator cbegin() const {
            check_zero_length_array();
            return const_iterator(elems_);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr reverse_iterator rbegin() {
            check_zero_length_array();
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_reverse_iterator rbegin() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_reverse_iterator crbegin() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr iterator end() {
            check_zero_length_array();
            return iterator(elems_ + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_iterator end() const {
            check_zero_length_array();
            return const_iterator(elems_ + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器常量
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_iterator cend() const {
            check_zero_length_array();
            return const_iterator(elems_ + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr reverse_iterator rend() {
            check_zero_length_array();
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_reverse_iterator rend() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_reverse_iterator crend() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        Ty elems_[N == 0 ? 1 : N];

    private:
        RAINY_ALWAYS_INLINE static constexpr int range_check(const difference_type offset) {
            if (offset >= N) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid array subscript");
            }
            return 0; // placeholder
        }

        RAINY_ALWAYS_INLINE static constexpr void check_zero_length_array() {
            if (size() == 0) {
                std::terminate();
            }
        }
    };

    template <typename Ty, std::size_t N>
    array(const Ty (&)[N]) -> array<Ty, N>;

    template <typename Ty, std::size_t N>
    array(const std::array<Ty,N>&) -> array<Ty, N>;

}

#endif