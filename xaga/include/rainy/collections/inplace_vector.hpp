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
#ifndef RAINY_COLLECTIONS_IN_PLACE_VECTOR_HPP
#define RAINY_COLLECTIONS_IN_PLACE_VECTOR_HPP
#include <initializer_list>
#include <rainy/core/core.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6386 6385 6031)
#endif

namespace rainy::collections::implements {
    template <typename Ty, std::size_t N>
    struct iv_aligned_storage {
        RAINY_CONSTEXPR20 Ty *data(std::size_t index) noexcept {
            return reinterpret_cast<Ty *>(data_) + index;
        }

        RAINY_CONSTEXPR20 const Ty *data(std::size_t index) const noexcept {
            return reinterpret_cast<const Ty *>(data_) + index;
        }

        alignas(Ty) core::byte_t data_[sizeof(Ty) * N];
    };

    template <typename Ty>
    class iv_zero_sized_storage {
    public:
        RAINY_CONSTEXPR20 iv_zero_sized_storage() = default;
        RAINY_CONSTEXPR20 iv_zero_sized_storage(const iv_zero_sized_storage &) = default;
        RAINY_CONSTEXPR20 iv_zero_sized_storage(iv_zero_sized_storage &&) = default;
        RAINY_CONSTEXPR20 iv_zero_sized_storage &operator=(const iv_zero_sized_storage &) = default;
        RAINY_CONSTEXPR20 iv_zero_sized_storage &operator=(iv_zero_sized_storage &&) = default;
        RAINY_CONSTEXPR20 ~iv_zero_sized_storage() = default;

    protected:
        static RAINY_CONSTEXPR20 Ty *begin_() noexcept {
            return nullptr;
        }

        static RAINY_CONSTEXPR20 std::size_t size_() noexcept {
            return 0;
        }

        static RAINY_CONSTEXPR20 void unsafe_set_size(std::size_t new_size) noexcept {
            std::terminate();
        }
    };

    template <typename Ty, std::size_t N>
    class iv_trivial_storage {
    public:
        RAINY_CONSTEXPR20 iv_trivial_storage() = default;
        RAINY_CONSTEXPR20 iv_trivial_storage(const iv_trivial_storage &) = default;
        RAINY_CONSTEXPR20 iv_trivial_storage(iv_trivial_storage &&) = default;
        RAINY_CONSTEXPR20 iv_trivial_storage &operator=(const iv_trivial_storage &) = default;
        RAINY_CONSTEXPR20 iv_trivial_storage &operator=(iv_trivial_storage &&) = default;
        RAINY_CONSTEXPR20 ~iv_trivial_storage() = default;

    protected:
        constexpr Ty *begin_() noexcept {
            return data_.begin();
        }

        constexpr const Ty *begin_() const noexcept {
            return data_.begin();
        }

        RAINY_NODISCARD constexpr std::size_t size_() const noexcept {
            return size;
        }

        constexpr void unsafe_set_size(const std::size_t new_size) {
            size = new_size;
        }

    private:
        using data_t = array<Ty, N>;
        alignas(Ty) data_t data_{};
        std::size_t size = 0;
    };

    template <typename Ty, std::size_t N>
    class iv_non_trivial_storage {
    public:
        RAINY_CONSTEXPR20 iv_non_trivial_storage() = default;
        RAINY_CONSTEXPR20 iv_non_trivial_storage(const iv_non_trivial_storage &) = default;
        RAINY_CONSTEXPR20 iv_non_trivial_storage(iv_non_trivial_storage &&) = default;
        RAINY_CONSTEXPR20 iv_non_trivial_storage &operator=(const iv_non_trivial_storage &) = default;
        RAINY_CONSTEXPR20 iv_non_trivial_storage &operator=(iv_non_trivial_storage &&) = default;
        RAINY_CONSTEXPR20 ~iv_non_trivial_storage() = default;

    protected:
        constexpr Ty *begin_() noexcept {
            return data_.data(0);
        }

        constexpr const Ty *begin_() const noexcept {
            return data_.data(0);
        }

        RAINY_NODISCARD constexpr std::size_t size_() const noexcept {
            return size;
        }

        constexpr void unsafe_set_size(const std::size_t new_size) noexcept {
            size = new_size;
        }

    private:
        using data_t = iv_aligned_storage<Ty, N>;
        data_t data_{};
        std::size_t size = 0;
    };

    template <typename Ty, std::size_t N>
    using select_for_iv_storage = type_traits::other_trans::conditional_t<
        N == 0, iv_zero_sized_storage<Ty>,
        type_traits::other_trans::conditional_t<type_traits::type_properties::is_trivial_v<Ty>, iv_trivial_storage<Ty, N>,
                                                iv_non_trivial_storage<Ty, N>>>;
}

namespace rainy::collections {
    template <typename Ty, std::size_t N = 8>
    class inplace_vector : public implements::select_for_iv_storage<Ty, N> {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

        /**
         * @brief 默认构造一个空向量
         */
        RAINY_CONSTEXPR20 inplace_vector() noexcept {
            assert_type_in_constant_eval();
        }

        /**
         * @brief 构造一个指定大小的向量，元素使用默认构造函数初始化
         * @param count 向量大小
         */
        RAINY_CONSTEXPR20 explicit inplace_vector(size_type count) {
            assert_type_in_constant_eval();
            if (count > N) {
                std::terminate();
            }
            auto elements = this->begin_();
            this->unsafe_set_size(count);
            for (size_type i = 0; i < count; ++i) {
                utility::construct_at(elements + i);
            }
        }

        /**
         * @brief 构造一个指定大小的向量，元素使用给定值初始化
         * @param count 向量大小
         * @param value 用于初始化元素的值
         */
        RAINY_CONSTEXPR20 inplace_vector(size_type count, const value_type &value) {
            assert_type_in_constant_eval();
            if (count > N) {
                std::terminate();
            }
            auto elements = this->begin_();
            this->unsafe_set_size(count);
            for (std::size_t i = 0; i < count; ++i) {
                elements[i] = value;
            }
        }

        /**
         * @brief 使用迭代器区间构造向量
         * @tparam Iter 迭代器类型，由参数类型自动推导（要求为iterator）
         * @param begin 迭代器起始位置
         * @param end 迭代器终止位置
         */
        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        RAINY_CONSTEXPR20 inplace_vector(Iter begin, Iter end) {
            assert_type_in_constant_eval();
            this->unsafe_set_size(static_cast<size_type>(utility::distance(begin, end)));
            std::size_t index{0};
            auto elements = this->begin_();
            for (; begin != end; ++begin, ++index) {
                utility::construct_in_place(elements[index], *begin);
            }
        }

        /**
         * @brief 拷贝构造函数
         * @param right 要拷贝的对象
         */
        RAINY_CONSTEXPR20 inplace_vector(const inplace_vector &right) {
            assert_type_in_constant_eval();
            this->unsafe_set_size(right.size());
            auto elements = this->begin_();
            for (std::size_t i = 0; i < right.size(); ++i) {
                utility::construct_in_place(elements[i], right[i]);
            }
        }

        /**
         * @brief 移动构造函数
         * @param right 待移动的对象
         */
        RAINY_CONSTEXPR20 inplace_vector(inplace_vector &&right) noexcept {
            assert_type_in_constant_eval();
            this->unsafe_set_size(right.size());
            auto elements = this->begin_();
            auto ilist_elements = right.begin_();
            for (std::size_t i = 0; i < size(); ++i) {
                utility::construct_in_place(elements[i], utility::move_if_noexcept(right[i]));
            }
        }

        /**
         * @brief 使用初始化列表构造向量
         * @param ilist 初始化列表
         */
        RAINY_CONSTEXPR20 inplace_vector(std::initializer_list<value_type> ilist) {
            assert_type_in_constant_eval();
            this->unsafe_set_size(ilist.size());
            auto elements = this->begin_();
            auto ilist_elements = ilist.begin();
            for (std::size_t i = 0; i < size(); ++i) {
                utility::construct_in_place(elements[i], ilist_elements[i]);
            }
        }

        RAINY_CONSTEXPR20 ~inplace_vector() = default;

        /**
         * @brief 截取向量的子区间
         * @tparam NewSize 新向量的大小（默认为原向量大小 N）
         * @param begin_slice 起始位置（包含）
         * @param end_slice 结束位置（不包含）
         * @return 返回从指定区间复制的子向量；若参数非法则返回空向量
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn slice(const std::size_t begin_slice = 0, std::size_t const end_slice = N) const
            -> inplace_vector<value_type, NewSize> {
            assert_type_in_constant_eval();
            if (const std::size_t distance = end_slice - begin_slice; begin_slice < end_slice && distance <= NewSize) {
                auto elements = this->begin_();
                inplace_vector<value_type, NewSize> new_slice{};
                for (std::size_t start = begin_slice, i = 0; start < end_slice; ++start, ++i) {
                    new_slice.emplace_back(elements[start]);
                }
                return new_slice;
            }
            return {};
        }

        /**
         * @brief 获取向量左侧的若干元素
         * @tparam NewSize 新向量的大小（默认为原向量大小 N）
         * @param n 要截取的元素数量
         * @return 返回包含前 n 个元素的新向量；若 n 超出范围则返回空向量
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn left(const std::size_t n) const -> inplace_vector<value_type, NewSize> {
            return slice<NewSize>(0, n);
        }

        /**
         * @brief 获取向量右侧的若干元素
         * @tparam NewSize 新向量的大小（默认为原向量大小 N）
         * @param n 要截取的元素数量
         * @return 返回包含后 n 个元素的新向量；若 n 超出范围则返回空向量
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn right(const std::size_t n) const -> inplace_vector<value_type, NewSize> {
            return slice<NewSize>(N - n, N);
        }

        /**
         * @brief 查找指定值在向量中的索引
         * @tparam UTy 待查找的值的类型
         * @param value 要查找的值
         * @return 若找到匹配元素则返回其索引，否则返回 npos
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy, value_type> &&
                                                            type_traits::type_properties::is_equal_comparable_v<value_type, UTy>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 rain_fn index_of(UTy &&value) const noexcept -> std::size_t { // NOLINT
            assert_type_in_constant_eval();
            auto iter = core::algorithm::find(begin(), end(), utility::forward<UTy>(value));
            return iter == end() ? npos : (iter - begin());
        }

        /**
         * @brief 根据谓词函数筛选向量元素
         * @tparam NewSize 返回向量的大小（默认为原向量大小 N）
         * @tparam Pred 谓词函数类型，接收元素并返回可转换为 bool 的结果
         * @param pred 用于筛选元素的谓词
         * @return 返回一个新向量，包含满足谓词的元素；若 NewSize 小于筛选出的元素数，则强制返回空向量
         */
        template <std::size_t NewSize = N, typename Pred>
        RAINY_CONSTEXPR20 rain_fn filter(Pred &&pred) const -> inplace_vector<value_type, NewSize> {
            assert_type_in_constant_eval();
            inplace_vector<value_type, NewSize> inplace_vector;
            std::size_t index_mapping[N]{};
            std::size_t raw_view_index = 0;
            auto elements = this->begin_();
            for (std::size_t i = 0; i < size(); ++i) {
                if (static_cast<bool>(pred(elements[i]))) {
                    index_mapping[raw_view_index++] = i;
                }
            }
            if (NewSize >= raw_view_index) {
                for (std::size_t i = 0; i < raw_view_index; ++i) {
                    inplace_vector.emplace_back(elements[index_mapping[i]]);
                }
            }
            return inplace_vector;
        }

        /**
         * @brief 返回向量的逆序版本
         * @return 返回一个新向量，其元素顺序与当前向量相反
         */
        RAINY_CONSTEXPR20 rain_fn reverse() const -> inplace_vector {
            inplace_vector arr{crbegin(), crend()};
            return arr;
        }

        /**
         * @brief 对向量中的每个元素应用映射函数
         * @tparam Fx 映射函数类型
         * @param func 映射函数
         * @return 返回一个新向量，其元素为映射函数作用后的结果
         */
        template <typename Fx>
        RAINY_CONSTEXPR20 rain_fn map(Fx &&func) const -> inplace_vector<value_type, N> {
            inplace_vector arr{};
            auto elements = this->begin_();
            for (std::size_t i = 0; i < size(); ++i) {
                arr.emplace_back(utility::forward<Fx>(func)(elements[i]));
            }
            return arr;
        }

        /**
         * @brief 使用默认初始化值折叠向量元素
         * @tparam Init 折叠操作的初始值类型
         * @return 返回折叠计算的结果
         */
        template <typename Init>
        RAINY_CONSTEXPR20 rain_fn fold() -> decltype(auto) {
            assert_type_in_constant_eval();
            return core::accumulate(begin(), end(), Init{});
        }

        /**
         * @brief 使用指定初始值折叠向量元素
         * @tparam Init 初始值类型
         * @param init_value 用作折叠计算的初始值
         * @return 返回折叠计算的结果
         */
        template <typename Init>
        RAINY_CONSTEXPR20 rain_fn fold(const Init &init_value) -> decltype(auto) {
            assert_type_in_constant_eval();
            return core::accumulate(begin(), end(), init_value);
        }

        /**
         * @brief 使用指定初始类型和二元函数折叠向量元素
         * @tparam Init 初始值类型
         * @tparam Fx 二元折叠函数类型
         * @param func 用于折叠的二元函数
         * @return 返回折叠计算的结果
         */
        template <typename Init, typename Fx>
        RAINY_CONSTEXPR20 rain_fn fold(Fx &&func) -> decltype(auto) {
            assert_type_in_constant_eval();
            return core::accumulate(begin(), end(), Init{}, utility::forward<Fx>(func));
        }

        /**
         * @brief 使用指定初始值和二元函数折叠向量元素
         * @tparam Fx 二元折叠函数类型
         * @tparam Init 初始值类型
         * @param func 用于折叠的二元函数
         * @param init_value 折叠计算的初始值
         * @return 返回折叠计算的结果
         */
        template <typename Fx, typename Init>
        RAINY_CONSTEXPR20 rain_fn fold(Fx &&func, const Init &init_value) -> decltype(auto) {
            assert_type_in_constant_eval();
            return core::accumulate(begin(), end(), init_value, utility::forward<Fx>(func));
        }

        /**
         * @brief 拼接两个向量
         * @tparam Ni 右侧向量的大小
         * @param right 右侧待拼接的向量
         * @return 返回一个包含左侧与右侧元素的新向量，长度为 N + Ni
         */
        template <std::size_t Ni>
        RAINY_CONSTEXPR20 rain_fn concat(const inplace_vector<value_type, Ni> &right) -> inplace_vector<value_type, N + Ni> {
            constexpr std::size_t size = N + Ni;
            inplace_vector<value_type, size> arr{*this, right};
            return arr;
        }

        /**
         * @brief 拷贝赋值运算符
         * @param right 待拷贝的对象
         * @return 当前对象的引用
         */
        RAINY_CONSTEXPR20 rain_fn operator=(const inplace_vector &right)->inplace_vector & {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            for (std::size_t i = 0; i < size(); ++i) {
                elements[i] = right[i];
            }
            for (std::size_t i = size(); i < N; ++i) {
                utility::construct_in_place(elements[i], right[i]);
            }
            return *this;
        }

        // clang-format off

        /**
         * @brief 移动赋值运算符
         * @param right 待移动的对象
         * @return 当前对象的引用
         */
        RAINY_CONSTEXPR20 rain_fn operator=(inplace_vector &&right) noexcept(N == 0 || ( // NOLINT
                                                              type_traits::type_properties::is_nothrow_move_assignable_v<Ty> &&
                                                              type_traits::type_properties::is_nothrow_move_constructible_v<Ty>))
            ->inplace_vector & {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            for (std::size_t i = 0; i < size(); ++i) {
                elements[i] = utility::move_if_noexcept(right[i]);
            }
            for (std::size_t i = size(); i < N; ++i) {
                utility::construct_in_place(elements[i], utility::move_if_noexcept(right[i]));
            }
            return *this;
        }

        // clang-format on

        /**
         * @brief 使用初始化列表赋值
         * @param ilist 初始化列表
         * @return 当前对象的引用
         */
        RAINY_CONSTEXPR20 rain_fn operator=(std::initializer_list<value_type> ilist)->inplace_vector & {
            assign(ilist);
            return *this;
        }

        /**
         * @brief 使用迭代器区间赋值
         * @tparam Iter 迭代器类型，由参数类型自动推导（要求为iterator）
         * @param begin 迭代器起始位置
         * @param end 迭代器终止位置
         */
        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        RAINY_CONSTEXPR20 rain_fn assign(Iter begin, Iter end) -> void {
            assert_type_in_constant_eval();
            std::size_t distance = utility::distance(begin, end);
            if (distance > N) {
                std::terminate();
            }
            clear();
            auto elements = this->begin_();
            std::size_t index{0};
            for (; begin != end; ++begin, ++index) {
                elements[index] = *begin;
            }
            this->unsafe_set_size(distance);
        }

        /**
         * @brief 使用指定数量和初始值赋值
         * @param count 数量
         * @param value 初始化的值
         */
        RAINY_CONSTEXPR20 rain_fn assign(size_type count, const value_type &value) -> void {
            assert_type_in_constant_eval();
            if (count > N) {
                std::terminate();
            }
            clear();
            auto elements = this->begin_();
            static_assert(type_traits::type_properties::is_copy_constructible_v<value_type>,
                          "The value_type must be copy constructible to use this assign method.");
            for (std::size_t i = 0; i < count; ++i) {
                utility::construct_in_place(elements[i], value);
            }
            this->unsafe_set_size(count);
        }

        /**
         * @brief 使用初始化列表赋值
         * @param ilist 初始化列表
         */
        RAINY_CONSTEXPR20 rain_fn assign(std::initializer_list<value_type> ilist) -> void {
            assign(ilist.begin(), ilist.end());
        }

        /**
         * @brief 获取指向当前向量起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn begin() -> iterator {
            assert_type_in_constant_eval();
            return iterator(this->begin_());
        }

        /**
         * @brief 获取指向当前向量起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn begin() const -> const_iterator {
            assert_type_in_constant_eval();
            return const_iterator(this->begin_());
        }

        /**
         * @brief 获取指向当前向量起始位置的迭代器常量
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn cbegin() const -> const_iterator {
            assert_type_in_constant_eval();
            return const_iterator(this->begin_());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn rbegin() -> reverse_iterator {
            assert_type_in_constant_eval();
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn rbegin() const -> const_reverse_iterator {
            assert_type_in_constant_eval();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器常量
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn crbegin() const -> const_reverse_iterator {
            assert_type_in_constant_eval();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前向量末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn end() -> iterator {
            assert_type_in_constant_eval();
            return iterator(this->begin_() + size());
        }

        /**
         * @brief 获取指向当前向量末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn end() const -> const_iterator {
            assert_type_in_constant_eval();
            return const_iterator(this->begin_() + size());
        }

        /**
         * @brief 获取指向当前向量末尾位置的迭代器常量
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn cend() const -> const_iterator {
            assert_type_in_constant_eval();
            return const_iterator(this->begin_() + size());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn rend() -> reverse_iterator {
            assert_type_in_constant_eval();
            return reverse_iterator(begin());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn rend() const -> const_reverse_iterator {
            assert_type_in_constant_eval();
            return const_reverse_iterator(begin());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器常量
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD RAINY_CONSTEXPR20 rain_fn crend() const -> const_reverse_iterator {
            assert_type_in_constant_eval();
            return const_reverse_iterator(begin());
        }

        /**
         * @brief 检查当前向量是否为空
         * @return 如果是空向量则返回 true，否则返回 false
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn empty() const noexcept -> bool {
            assert_type_in_constant_eval();
            return size() == 0;
        }

        /**
         * @brief 获取当前向量的大小
         * @return 返回向量中元素的数量
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn size() const noexcept -> size_type {
            assert_type_in_constant_eval();
            return this->size_();
        }

        /**
         * @brief 获取当前向量的容量
         * @return 返回向量的最大容量（固定为N）
         */
        static constexpr rain_fn capacity() noexcept -> size_type {
            return N;
        }

        /**
         * @brief 获取向量的最大可能大小
         * @return 返回向量的最大可能大小（固定为N）
         */
        static constexpr rain_fn max_size() noexcept -> size_type {
            return N;
        }

        RAINY_CONSTEXPR20 rain_fn resize(const size_type new_size) -> void {
            resize(new_size, value_type{});
        }

        RAINY_CONSTEXPR20 rain_fn resize(size_type new_size, const value_type &value) -> void {
            assert_type_in_constant_eval();
            if (new_size > N) {
                std::terminate();
            }
            if (new_size == size()) {
                return;
            }
            auto elements = this->begin_();
            if (new_size < size()) { // 则容器将被缩小至其前 count 个元素。
                const size_type old_size = size();
                this->unsafe_set_size(new_size);
                for (size_type i = size(); i < old_size; ++i) {
                    (void) utility::addressof(elements[i])->~value_type();
                }
            } else { // 如果当前大小小于 count，则1) 将追加额外的 默认插入 元素。2) 将追加 value 的额外副本。
                const size_type old_size = size();
                this->unsafe_set_size(new_size);
                for (size_type i = old_size; i < size(); ++i) {
                    elements[i] = value;
                }
            }
        }

        static RAINY_CONSTEXPR20 rain_fn reserve(const size_type new_cap) -> void {
            if (new_cap > N) {
                std::terminate();
            }
        }

        static RAINY_CONSTEXPR20 rain_fn shrink_to_fit() noexcept -> void {
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn at(difference_type off) -> reference {
            assert_type_in_constant_eval();
            range_check(off);
            auto elements = this->begin_();
            return elements[off];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn at(difference_type off) const -> const_reference {
            assert_type_in_constant_eval();
            range_check(off);
            auto elements = this->begin_();
            return elements[off];
        }

        /**
         * @brief 获取当前向量第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn front() -> reference {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            return elements[0];
        }

        /**
         * @brief 获取当前向量第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn front() const -> const_reference {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            return elements[0];
        }

        /**
         * @brief 获取当前向量最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn back() -> reference {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            return elements[size() - 1];
        }

        /**
         * @brief 获取当前向量最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn back() const -> const_reference {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            return elements[size() - 1];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn operator[](difference_type idx) noexcept -> reference {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            return elements[idx]; // NOLINT
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn operator[](difference_type idx) const noexcept
            -> const_reference {
            assert_type_in_constant_eval();
            auto elements = this->begin_();
            return elements[idx]; // NOLINT
        }

        /**
         * @brief 获取当前向量地址
         * @return 返回向量的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn data() noexcept -> pointer {
            assert_type_in_constant_eval();
            return static_cast<pointer>(this->begin_());
        }

        /**
         * @brief 获取当前向量地址
         * @return 返回向量的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn data() const noexcept -> const_pointer {
            assert_type_in_constant_eval();
            return static_cast<const_pointer>(this->begin_());
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 rain_fn emplace_back(Args &&...args) -> reference {
            assert_type_in_constant_eval();
            if (size() == N) {
                std::terminate();
            }
            utility::construct_at(end(), utility::forward<Args>(args)...);
            this->unsafe_set_size(size() + 1);
            return back();
        }

        RAINY_CONSTEXPR20 rain_fn push_back(const value_type &value) -> reference {
            return emplace_back(value);
        }

        RAINY_CONSTEXPR20 rain_fn push_back(value_type &&value) -> reference {
            return emplace_back(utility::move(value));
        }

        RAINY_CONSTEXPR20 rain_fn pop_back() -> void {
            unsafe_destroy(end() - 1, end());
            this->unsafe_set_size(size() - 1);
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 rain_fn try_emplace_back(Args &&...args) -> pointer {
            if (size() == N) {
                return nullptr;
            }
            return utility::addressof(emplace_back(utility::forward<Args>(args)...));
        }

        RAINY_CONSTEXPR20 rain_fn try_push_back(const value_type &value) -> pointer {
            if (size() == N) {
                return nullptr;
            }
            return utility::addressof(push_back(value));
        }

        RAINY_CONSTEXPR20 rain_fn try_push_back(value_type &&value) -> pointer {
            if (size() == N) {
                return nullptr;
            }
            return utility::addressof(push_back(utility::move(value)));
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 rain_fn unchecked_emplace_back(Args &&...args) -> reference {
            return *try_emplace_back(utility::forward<Args>(args)...);
        }

        RAINY_CONSTEXPR20 rain_fn unchecked_push_back(const value_type &value) -> reference {
            return *try_push_back(value);
        }

        RAINY_CONSTEXPR20 rain_fn unchecked_push_back(value_type &&value) -> reference {
            return *try_push_back(utility::move(value));
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 rain_fn emplace(const_iterator position, Args &&...args) -> iterator {
            assert_type_in_constant_eval();
            if (size() == N) {
                std::terminate();
            }
            size_type pos = utility::distance(cbegin(), position);
            auto elements = this->begin_();
            if (pos < size()) {
                utility::construct_in_place(elements[size()], utility::move(elements[size() - 1]));
                if (pos < size() - 1) {
                    core::algorithm::move_backward(elements + pos, elements + size() - 1, elements + size());
                }
            }
            this->unsafe_set_size(size() + 1);
            return utility::construct_at(&elements[pos], utility::forward<Args>(args)...);
        }

        RAINY_CONSTEXPR20 rain_fn insert(const_iterator position, const value_type &value) -> iterator {
            return emplace(position, value);
        }

        RAINY_CONSTEXPR20 rain_fn insert(const_iterator position, value_type &&value) -> iterator {
            return emplace(position, utility::move(value));
        }

        RAINY_CONSTEXPR20 rain_fn insert(const_iterator position, size_type count, const value_type &value) -> iterator {
            assert_type_in_constant_eval();
            if (size() + count > N) {
                std::terminate();
            }
            if (count == 0) {
                return const_cast<iterator>(position);
            }
            size_type pos = utility::distance(cbegin(), position);
            auto elements = this->begin_();
            size_type old_size = size();
            if (pos < old_size) {
                for (size_type i = 0; i < count; ++i) {
                    elements[old_size + i] = value;
                }
                core::algorithm::move_backward(elements + pos, elements + old_size, elements + old_size + count);
                for (size_type i = pos; i < pos + count; ++i) {
                    elements[i] = value;
                }
            } else {
                for (size_type i = 0; i < count; ++i) {
                    elements[pos + i] = value;
                }
            }
            this->unsafe_set_size(size() + count);
            return iterator{elements + pos};
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, Iter first, Iter last) {
            assert_type_in_constant_eval();
            size_type iter_length = utility::distance(first, last);
            if (size() + iter_length > N) {
                std::terminate();
            }
            if (iter_length == 0) {
                return const_cast<iterator>(position);
            }
            size_type pos = utility::distance(cbegin(), position);
            auto elements = this->begin_();
            size_type old_size = size();
            if (pos < old_size) {
                core::algorithm::move_backward(elements + pos, elements + old_size, elements + old_size + iter_length);
            }
            auto dest = elements + pos;
            for (; first != last; ++first, ++dest) {
                *dest = *first;
            }
            this->unsafe_set_size(size() + iter_length);
            return iterator{elements + pos};
        }

        RAINY_CONSTEXPR20 iterator insert(utility::in<const_iterator> position, std::initializer_list<value_type> ilist) {
            return insert(position, ilist.begin(), ilist.end());
        }
        
        RAINY_CONSTEXPR20 iterator erase(const_iterator position) {
            return erase(position, position + 1);
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
            assert_type_in_constant_eval();
            iterator tmp_first = begin() + (first - begin());
            if (first != last) {
                core::algorithm::move(tmp_first + (last - first), end(), tmp_first);
                end()->~value_type();
                this->unsafe_set_size(size() - static_cast<size_type>(last - first));
            }
            return tmp_first;
        }

        RAINY_CONSTEXPR20 rain_fn clear() noexcept(type_traits::type_properties::is_nothrow_destructible_v<value_type>) -> void {
            assert_type_in_constant_eval();
            for (auto &it: *this) {
                (void) utility::addressof(it)->~value_type();
            }
            this->unsafe_set_size(0);
        }

        // clang-format off

        RAINY_CONSTEXPR20 rain_fn swap(inplace_vector &right) noexcept(N == 0 || (type_traits::type_properties::is_nothrow_swappable_v<Ty> &&
                                                        type_traits::type_properties::is_nothrow_move_constructible_v<Ty>) ) -> void {
            assert_type_in_constant_eval();
            auto tmp = utility::move(right);
            right = utility::move(*this);
            (*this) = utility::move(tmp);
        }

        // clang-format on

        /**
         * @brief 判断两个向量是否相等
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若两个向量元素一一对应且相等则返回 true，否则返回 false
         */
        friend RAINY_CONSTEXPR20 rain_fn operator==(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            assert_type_in_constant_eval();
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief 判断两个向量是否不相等
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若两个向量不相等则返回 true，否则返回 false
         */
        friend RAINY_CONSTEXPR20 rain_fn operator!=(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            assert_type_in_constant_eval();
            return !(left == right);
        }

        /**
         * @brief 判断左侧向量是否小于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量在字典序上小于右侧向量则返回 true，否则返回 false
         */
        friend RAINY_CONSTEXPR20 rain_fn operator<(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            assert_type_in_constant_eval();
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief 判断左侧向量是否大于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量在字典序上大于右侧向量则返回 true，否则返回 false
         */
        friend RAINY_CONSTEXPR20 rain_fn operator>(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            assert_type_in_constant_eval();
            return right < left;
        }

        /**
         * @brief 判断左侧向量是否小于等于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量小于或等于右侧向量则返回 true，否则返回 false
         */
        friend RAINY_CONSTEXPR20 rain_fn operator<=(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            assert_type_in_constant_eval();
            return !(left > right);
        }

        /**
         * @brief 判断左侧向量是否大于等于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量大于或等于右侧向量则返回 true，否则返回 false
         */
        friend RAINY_CONSTEXPR20 rain_fn operator>=(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            assert_type_in_constant_eval();
            return !(left < right);
        }

    private:
        /*
        @brief The API of inplace_vector<T, Capacity> can be used in constexpr-contexts if is_trivially_copyable_v<T>,
        is_default_constructible_v<T>, and is_trivially_destructible<T> are true.
        @brief This proposal only supports using the constexpr methods in constant expressions if is_trivial_t<T> is true.
        @brief See: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0843r14.html#constexpr-support
        */
        static constexpr void assert_type_in_constant_eval() noexcept { // NOLINT
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                constexpr bool assertion_result = type_traits::type_properties::is_trivially_copyable_v<Ty> &&
                                                  type_traits::type_properties::is_trivially_copyable_v<Ty> &&
                                                  type_traits::type_properties::is_default_constructible_v<Ty> &&
                                                  type_traits::type_properties::is_trivially_destructible_v<Ty>;
                if (!assertion_result) {
                    assert(false && "inplace_vector<T, N> can only be used in constant-evaluated contexts if T is trivially copyable, "
                                  "default constructible, and trivially destructible.");
                }
            }
#endif
        }

        constexpr void unsafe_destroy(value_type *begin, value_type *end) noexcept(std::is_nothrow_destructible_v<value_type>) {
            if constexpr (N > 0 && !std::is_trivial_v<value_type>) {
                for (; begin != end; ++begin) {
                    begin->~value_type();
                }
            }
        }

        RAINY_ALWAYS_INLINE static RAINY_CONSTEXPR20 rain_fn range_check(const difference_type offset) noexcept -> void {
            if (offset >= N) {
                std::abort();
            }
        }
    };

    template <typename Ty, std::size_t N>
    void swap(inplace_vector<Ty, N> &left, inplace_vector<Ty, N> &right) noexcept {
        left.swap(right);
    }

    template <typename Ty, std::size_t N>
    inplace_vector(const Ty (&)[N]) -> inplace_vector<Ty, N>;

    template <typename Ty, std::size_t N>
    inplace_vector(const std::array<Ty, N> &) -> inplace_vector<Ty, N>;
}

namespace rainy::collections {
    template <typename Ty, std::size_t N, typename UTy, typename Fx>
    RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn zip_with(const inplace_vector<Ty, N> &left, const inplace_vector<UTy, N> &right,
                                                       Fx &&func) -> auto {
        using type = decltype(utility::invoke(utility::forward<Fx>(func), left[0], right[0]));
        inplace_vector<type, N> arr;
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = utility::invoke(utility::forward<Fx>(func), left[i], right[i]);
        }
        return arr;
    }
}

#endif
