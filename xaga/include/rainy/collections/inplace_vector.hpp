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

namespace rainy::collections {
    template <typename Ty, std::size_t N = 8>
    class inplace_vector {
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

        using impl_traits = implements::array_traits<Ty, N>;

        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

        constexpr inplace_vector() noexcept : inplace_vector{std::in_place, type_traits::helper::make_index_sequence<N>{}} {
        }

        constexpr explicit inplace_vector(size_type count) : size_{count} {
        }

        constexpr inplace_vector(size_type count, const value_type &value) {
            if (count > N) {
                std::terminate();
            }
            for (std::size_t i = 0; i < count; ++i) {
                elements[i] = value;
            }
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        constexpr inplace_vector(Iter begin, Iter end) : elements{} {
            std::size_t index{0};
            for (; begin != end; ++begin, ++index) {
                elements[index] = *begin;
            }
        }

        constexpr inplace_vector(const inplace_vector &right) {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = right.elements[i];
            }
        }

        constexpr inplace_vector(inplace_vector &&right) noexcept : elements{}, size_{right.size_} {
            for (std::size_t i = 0; i < size_; ++i) {
                elements[i] = utility::move_if_noexcept(right.elements[i]);
            }
        }

        constexpr inplace_vector(std::initializer_list<value_type> ilist) :
            inplace_vector{ilist.begin(), ilist.size(), type_traits::helper::make_index_sequence<N>{}} {
            if (ilist.size() > N) {
                std::terminate();
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
        RAINY_NODISCARD constexpr rain_fn slice(std::size_t begin_slice = 0, std::size_t end_slice = N)
            -> collections::inplace_vector<value_type, NewSize> {
            if (std::size_t distance = end_slice - begin_slice; begin_slice < end_slice && distance <= NewSize) {
                collections::inplace_vector<value_type, NewSize> new_slice{};
                for (std::size_t start = begin_slice, i = 0; start < end_slice; ++start, ++i) {
                    new_slice[i] = elements[start];
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
        RAINY_NODISCARD constexpr rain_fn left(std::size_t n) const -> collections::inplace_vector<value_type, NewSize> {
            return slice<NewSize>(0, n);
        }

        /**
         * @brief 获取向量右侧的若干元素
         * @tparam NewSize 新向量的大小（默认为原向量大小 N）
         * @param n 要截取的元素数量
         * @return 返回包含后 n 个元素的新向量；若 n 超出范围则返回空向量
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn right(std::size_t n) const -> collections::inplace_vector<value_type, NewSize> {
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
        constexpr rain_fn index_of(UTy &&value) const noexcept -> std::size_t {
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
        constexpr rain_fn filter(Pred &&pred) const -> collections::inplace_vector<value_type, NewSize> {
            collections::inplace_vector<value_type, NewSize> inplace_vector;
            std::size_t index_mapping[N]{};
            std::size_t raw_view_index = 0;
            for (std::size_t i = 0; i < size_; ++i) {
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
        constexpr rain_fn reverse() const -> collections::inplace_vector<value_type, N> {
            collections::inplace_vector<Ty, N> arr{crbegin(), crend()};
            return arr;
        }

        /**
         * @brief 对向量中的每个元素应用映射函数
         * @tparam Fx 映射函数类型
         * @param func 映射函数
         * @return 返回一个新向量，其元素为映射函数作用后的结果
         */
        template <typename Fx>
        constexpr rain_fn map(Fx &&func) const -> collections::inplace_vector<value_type, N> {
            collections::inplace_vector<value_type, N> arr{};
            for (std::size_t i = 0; i < size_; ++i) {
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
        constexpr rain_fn fold() -> decltype(auto) {
            return core::accumulate(begin(), end(), Init{});
        }

        /**
         * @brief 使用指定初始值折叠向量元素
         * @tparam Init 初始值类型
         * @param init_value 用作折叠计算的初始值
         * @return 返回折叠计算的结果
         */
        template <typename Init>
        constexpr rain_fn fold(const Init &init_value) -> decltype(auto) {
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
        constexpr rain_fn fold(Fx &&func) -> decltype(auto) {
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
        constexpr rain_fn fold(Fx &&func, const Init &init_value) -> decltype(auto) {
            return core::accumulate(begin(), end(), init_value, utility::forward<Fx>(func));
        }

        /**
         * @brief 拼接两个向量
         * @tparam Ni 右侧向量的大小
         * @param right 右侧待拼接的向量
         * @return 返回一个包含左侧与右侧元素的新向量，长度为 N + Ni
         */
        template <std::size_t Ni>
        constexpr rain_fn concat(const inplace_vector<value_type, Ni> &right) -> inplace_vector<value_type, N + Ni> {
            constexpr std::size_t size = N + Ni;
            inplace_vector<value_type, size> arr{*this, right};
            return arr;
        }

        constexpr rain_fn operator=(const inplace_vector &right) noexcept(impl_traits::is_nothrow_assignable::value)
            ->inplace_vector & {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = right[i];
            }
            return *this;
        }

        constexpr rain_fn operator=(inplace_vector &&right) noexcept(impl_traits::is_nothrow_assignable::value)->inplace_vector & {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = utility::move_if_noexcept(right[i]);
            }
            return *this;
        }

        constexpr rain_fn operator=(std::initializer_list<value_type> ilist) -> inplace_vector & {
            assign(ilist);
            return *this;
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        constexpr rain_fn assign(Iter begin, Iter end) -> void {
            std::size_t distance = utility::distance(begin, end);
            if (distance > N) {
                std::terminate();
            }
            std::size_t index{0};
            for (; begin != end; ++begin, ++index) {
                elements[index] = *begin;
            }
        }

        constexpr rain_fn assign(size_type count, const value_type &value) -> void {
            if (count > N) {
                std::terminate();
            }
            for (std::size_t i = 0; i < count; ++i) {
                elements[i] = value;
            }
        }

        constexpr rain_fn assign(std::initializer_list<value_type> ilist) -> void {
            if (ilist.size() > N) {
                std::terminate();
            }
            for (std::size_t i = 0; i < ilist.size(); ++i) {
                elements[i] = ilist[i];
            }
        }

        /**
         * @brief 获取指向当前向量起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn begin() -> iterator {
            return iterator(elements);
        }

        /**
         * @brief 获取指向当前向量起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn begin() const -> const_iterator {
            return const_iterator(elements);
        }

        /**
         * @brief 获取指向当前向量起始位置的迭代器常量
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn cbegin() const -> const_iterator {
            return const_iterator(elements);
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rbegin() -> reverse_iterator {
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器常量
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn crbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前向量末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn end() -> iterator {
            return iterator(elements + size_);
        }

        /**
         * @brief 获取指向当前向量末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn end() const -> const_iterator {
            return const_iterator(elements + size_);
        }

        /**
         * @brief 获取指向当前向量末尾位置的迭代器常量
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn cend() const -> const_iterator {
            return const_iterator(elements + size_);
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rend() -> reverse_iterator {
            return reverse_iterator(begin());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rend() const -> const_reverse_iterator {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief 获取指向当前向量末尾位置的反向迭代器常量
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn crend() const -> const_reverse_iterator {
            return const_reverse_iterator(begin());
        }

        constexpr rain_fn empty() const noexcept -> bool {
            return size_ == 0;
        }

        constexpr rain_fn size() const noexcept -> size_type {
            return size_;
        }

        static constexpr rain_fn capacity() noexcept -> size_type {
            return N;
        }

        static constexpr rain_fn max_size() noexcept -> size_type {
            return N;
        }

        RAINY_CONSTEXPR20 rain_fn resize(size_type new_size) -> void {
            resize(new_size, value_type{});
        }

        RAINY_CONSTEXPR20 rain_fn resize(size_type new_size, const value_type &value) -> void {
            if constexpr (N != 0) {
                if (new_size > N) {
                    std::terminate();
                }
                if (new_size == 0 || new_size == size()) {
                    return;
                }
                if (new_size < size_) { // 则容器将被缩小至其前 count 个元素。
                    size_type old_size = size_;
                    size_ = new_size;
                    for (size_type i = size_; i < old_size; ++i) {
                        (void) utility::addressof(elements[i])->~value_type();
                    }
                } else { // 如果当前大小小于 count，则1) 将追加额外的 默认插入 元素。2) 将追加 value 的额外副本。
                    size_type old_size = size_;
                    size_ = new_size;
                    for (size_type i = old_size; i < size_; ++i) {
                        elements[i] = value;
                    }
                }
            }
        }

        static constexpr rain_fn reserve(size_type new_cap) -> void {
            if (new_cap > N) {
                std::terminate();
            }
        }

        static constexpr rain_fn shrink_to_fit() noexcept -> void {
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn at(difference_type off) -> reference {
            range_check(off);
            return elements[off];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn at(difference_type off) const -> const_reference {
            range_check(off);
            return elements[off];
        }

        /**
         * @brief 获取当前向量第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn front() -> reference {
            return elements[0];
        }

        /**
         * @brief 获取当前向量第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn front() const -> const_reference {
            return elements[0];
        }

        /**
         * @brief 获取当前向量最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn back() -> reference {
            return elements[size_ - 1];
        }

        /**
         * @brief 获取当前向量最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn back() const -> const_reference {
            return elements[size_ - 1];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn operator[](difference_type idx) noexcept -> reference {
            return elements[idx]; // NOLINT
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn operator[](difference_type idx) const noexcept -> const_reference {
            return elements[idx]; // NOLINT
        }

        /**
         * @brief 获取当前向量地址
         * @return 返回向量的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn data() noexcept -> pointer {
            return static_cast<pointer>(elements);
        }

        /**
         * @brief 获取当前向量地址
         * @return 返回向量的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn data() const noexcept -> const_pointer {
            return static_cast<const_pointer>(elements);
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 rain_fn emplace_back(Args &&...args) -> reference {
            if (size_ == N) {
                std::terminate();
            }
#if RAINY_HAS_CXX20
            utility::construct_at(&elements[size_++], utility::forward<Args>(args)...);
#else
            value_type value{utility::forward<Args>(args)...};
            elements[size_++] = utility::move_if_noexcept(value);
#endif
            return back();
        }

        constexpr rain_fn push_back(const value_type &value) -> reference {
            if (size_ == N) {
                std::terminate();
            }
            elements[size_++] = value;
            return back();
        }

        constexpr rain_fn push_back(value_type &&value) -> reference {
            if (size_ == N) {
                std::terminate();
            }
            elements[size_++] = value;
            return back();
        }

        constexpr rain_fn pop_back() -> void {
            if constexpr (N != 0) {
#if RAINY_HAS_CXX20
                utility::addressof(elements[size_--])->~value_type();
#else
                elements[size_--] = {};
#endif
            }
        }

        template <typename... Args>
        constexpr rain_fn try_emplace_back(Args &&...args) -> pointer {
            if (size_ == N) {
                return nullptr;
            }
            return utility::addressof(emplace_back(utility::forward<Args>(args)...));
        }

        constexpr rain_fn try_push_back(const value_type &value) -> pointer {
            if (size_ == N) {
                return nullptr;
            }
            return utility::addressof(push_back(value));
        }

        constexpr rain_fn try_push_back(value_type &&value) -> pointer {
            if (size_ == N) {
                return nullptr;
            }
            return utility::addressof(push_back(utility::move(value)));
        }

        template <typename... Args>
        constexpr rain_fn unchecked_emplace_back(Args &&...args) -> reference {
            return *try_emplace_back(utility::forward<Args>(args)...);
        }

        constexpr rain_fn unchecked_push_back(const value_type &value) -> reference {
            return *try_push_back(value);
        }

        constexpr rain_fn unchecked_push_back(value_type &&value) -> reference {
            return *try_push_back(utility::move(value));
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 rain_fn emplace(const_iterator position, Args &&...args) -> iterator {
            if (size_ == N) {
                std::terminate();
            }
            size_type pos = utility::distance(cbegin(), position);
            if (pos < size_) {
                utility::construct_at(elements + size_, std::move(elements[size_ - 1]));
                if (pos < size_ - 1) {
                    core::algorithm::move_backward(elements + pos, elements + size_ - 1, elements + size_);
                }
                (void) utility::addressof(elements[pos])->~value_type();
            }
            auto iter = utility::construct_at(elements + pos, std::forward<Args>(args)...);
            ++size_;
            return iter;
        }

        RAINY_CONSTEXPR20 rain_fn insert(const_iterator position, const value_type &value) -> iterator {
            return emplace(position, value);
        }

        RAINY_CONSTEXPR20 rain_fn insert(const_iterator position, value_type &&value) -> iterator {
            return emplace(position, utility::move(value));
        }

        RAINY_CONSTEXPR20 rain_fn insert(const_iterator position, size_type count, const value_type &value) -> iterator {
            if (size_ + count > N) {
                std::terminate();
            }
            if (count == 0) {
                return const_cast<iterator>(position);
            }
            size_type pos = utility::distance(cbegin(), position);
            size_type old_size = size_;
            if (pos < old_size) {
                for (size_type i = 0; i < count; ++i) {
                    utility::construct_at(elements + old_size + i, value);
                }
                core::algorithm::move_backward(elements + pos, elements + old_size, elements + old_size + count);
                for (size_type i = pos; i < pos + count; ++i) {
                    elements[i] = value;
                }
            } else {
                for (size_type i = 0; i < count; ++i) {
                    utility::construct_at(elements + pos + i, value);
                }
            }
            size_ += count;
            return iterator{elements + pos};
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        constexpr iterator insert(const_iterator position, Iter first, Iter last) {
            size_type iter_length = utility::distance(first, last);
            if (size_ + iter_length > N) {
                std::terminate();
            }
            if (iter_length == 0) {
                return const_cast<iterator>(position);
            }
            size_type pos = utility::distance(cbegin(), position);
            size_type old_size = size_;
            if (pos < old_size) {
                core::algorithm::move_backward(elements + pos, elements + old_size, elements + old_size + iter_length);
            }
            auto dest = elements + pos;
            for (; first != last; ++first, ++dest) {
                if (dest < elements + old_size) {
                    *dest = *first;
                } else {
                    utility::construct_at(dest, *first);
                }
            }
            size_ += iter_length;
            return iterator{elements + pos};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, std::initializer_list<value_type> ilist) {
            return insert(position, ilist.begin(), ilist.end());
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator position) {
            return erase(position, position + 1);
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
            iterator tmp_first = begin() + (first - begin());
            if (first != last) {
                iterator it = core::algorithm::move(tmp_first + (last - first), end(), tmp_first);
                end()->~value_type();
                size_ = (size() - static_cast<size_type>(last - first));
            }
            return tmp_first;
        }

        RAINY_CONSTEXPR20 rain_fn clear() noexcept(type_traits::type_properties::is_nothrow_destructible_v<value_type>) -> void {
            for (auto &it: *this) {
                (void) utility::addressof(it)->~value_type();
            }
            size_ = 0;
        }

        RAINY_CONSTEXPR20 rain_fn swap(inplace_vector &right) noexcept(impl_traits::is_nothrow_swappable::value) -> void {
            core::algorithm::swap_ranges(elements, right.elements);
        }

        /**
         * @brief 判断两个向量是否相等
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若两个向量元素一一对应且相等则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator==(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief 判断两个向量是否不相等
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若两个向量不相等则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator!=(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            return !(left == right);
        }

        /**
         * @brief 判断左侧向量是否小于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量在字典序上小于右侧向量则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator<(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief 判断左侧向量是否大于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量在字典序上大于右侧向量则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator>(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            return right < left;
        }

        /**
         * @brief 判断左侧向量是否小于等于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量小于或等于右侧向量则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator<=(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            return !(left > right);
        }

        /**
         * @brief 判断左侧向量是否大于等于右侧向量
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量大于或等于右侧向量则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator>=(const inplace_vector &left, const inplace_vector &right) noexcept -> bool {
            return !(left < right);
        }

    private:
        template <std::size_t... I>
        constexpr inplace_vector(const_pointer ilist, std::size_t ilist_size, type_traits::helper::index_sequence<I...>) :
            elements{(I < ilist_size ? ilist[I] : value_type{})...}, size_{ilist_size} {
        }

        template <std::size_t... I>
        constexpr inplace_vector(std::in_place_t, type_traits::helper::index_sequence<I...>) : elements{((void) I, value_type{})...} {
        }

        RAINY_ALWAYS_INLINE static constexpr rain_fn range_check(const difference_type offset) noexcept -> void {
            if (offset >= N) {
                std::abort();
            }
        }

        typename impl_traits::type elements{};
        std::size_t size_{0};
    };

    template <typename Ty, std::size_t N>
    void swap(inplace_vector<Ty, N> &left, inplace_vector<Ty, N> &right) {
        left.swap(right);
    }

    template <typename Ty, std::size_t N>
    inplace_vector(const Ty (&)[N]) -> inplace_vector<Ty, N>;

    template <typename Ty, std::size_t N>
    inplace_vector(const std::array<Ty, N> &) -> inplace_vector<Ty, N>;
}

namespace rainy::collections {
    template <typename Ty, std::size_t N, typename UTy, typename Fx>
    RAINY_NODISCARD constexpr rain_fn zip_with(const inplace_vector<Ty, N> &left, const inplace_vector<UTy, N> &right, Fx &&func)
        -> auto {
        using type = decltype(utility::invoke(utility::forward<Fx>(func), left[0], right[0]));
        collections::inplace_vector<type, N> arr;
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = utility::invoke(utility::forward<Fx>(func), left[i], right[i]);
        }
        return arr;
    }
}

#endif