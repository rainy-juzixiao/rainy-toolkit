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
#ifndef RAINY_CORE_IMPLEMENTS_COLLECTIONS_ARRAY_HPP
#define RAINY_CORE_IMPLEMENTS_COLLECTIONS_ARRAY_HPP
#include <array>
#include <rainy/core/implements/basic_algorithm.hpp>
#include <rainy/core/implements/reverse_iterator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <tuple>

namespace rainy::collections {
    template <typename Ty, std::size_t N>
    class array;
}

namespace rainy::collections::implements {
    template <typename Ty, std::size_t N>
    struct array_traits {
        using type = Ty[N];

        using is_swappable = type_traits::type_properties::is_swappable<Ty>;
        using is_nothrow_swappable = type_traits::type_properties::is_nothrow_swappable<Ty>;
        using is_nothrow_assignable =
            type_traits::helper::bool_constant<type_traits::type_properties::is_nothrow_move_assignable_v<Ty> &&
                                               type_traits::type_properties::is_nothrow_move_constructible_v<Ty>>;
    };

    template <typename Ty>
    struct array_traits<Ty, 0> {
        struct type {
            RAINY_INLINE constexpr explicit operator Ty *() const noexcept {
                return nullptr;
            }

            RAINY_INLINE constexpr Ty &operator[](std::size_t) const noexcept {
                std::terminate();
            }
        };

        using is_swappable = type_traits::helper::true_type;
        using is_nothrow_swappable = type_traits::helper::true_type;
        using is_nothrow_assignable = type_traits::helper::true_type;
    };
}

namespace rainy::collections {
    /**
     * @brief 静态数组模板array，类似于std::array
     * @brief 但是提供更多的封装和功能
     * @attention 该array是非聚合类型
     */
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
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        using impl_traits = implements::array_traits<Ty, N>;

        using carray_type = typename impl_traits::type;

        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

        /**
         * @brief 默认构造函数
         * @details 以值初始化的方式构造所有元素
         * @note 若元素类型支持 noexcept 默认构造，则该构造函数同样为 noexcept
         */
        constexpr array() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) : elements{} {
        }

        /**
         * @brief 使用初始化列表对数组进行初始化
         * @param ilist 初始化列表
         * @note 如果初始化列表的大小大于其数组大小，则std::terminate()将被调用
         */
        constexpr array(std::initializer_list<Ty> ilist) :
            array{ilist.begin(), ilist.size(), type_traits::helper::make_index_sequence<N>{}} {
            if (ilist.size() > N) {
                std::terminate();
            }
        }

        /**
         * @brief 拷贝构造函数
         * @param right 另一个同类型数组
         * @details 将参数数组的所有元素逐个拷贝到当前数组中
         */
        constexpr array(const array &right) : elements{} {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = right[i];
            }
        }

        /**
         * @brief 移动构造函数
         * @param right 另一个同类型数组（将被移动）
         * @details 将参数数组中的元素逐个移动到当前数组中
         * @note 若元素类型支持 noexcept 移动构造，则该构造函数同样为 noexcept
         */
        constexpr array(array &&right) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<value_type>) :
            elements{} {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = utility::move_if_noexcept(right[i]);
            }
        }

        /**
         * @brief 使用固定数量与值进行构造
         * @param count 要填充的元素数量
         * @param value 用于填充的值
         * @details 将前 count 个元素赋值为指定值，其余保持默认初始化
         * @note 若 count >= N，将触发断言失败
         */
        constexpr array(std::size_t count, const_reference value) : elements{} {
            if (count > N) {
                std::terminate();
            }
            for (std::size_t i = 0; i < count; ++i) {
                elements[i] = value;
            }
        }

        /**
         * @brief 使用迭代器区间构造数组
         * @tparam Iter 满足迭代器概念的类型
         * @param begin 起始迭代器
         * @param end 结束迭代器（不含）
         * @details 从迭代器区间中依次复制元素到数组中，直到到达 end 或填满数组
         */
        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        constexpr array(Iter begin, Iter end) : elements{} {
            std::size_t distance = utility::distance(begin, end);
            if (distance > N) {
                std::terminate();
            }
            std::size_t index{0};
            for (; begin != end; ++begin, ++index) {
                elements[index] = *begin;
            }
        }

        /**
         * @brief 使用两个数组拼接构造新数组
         * @tparam L 左数组大小
         * @tparam R 右数组大小
         * @param left 左侧数组
         * @param right 右侧数组
         * @details 当 L + R 不超过当前数组大小 N 时，按顺序将左右数组内容拷贝至当前数组
         */
        template <std::size_t L, std::size_t R>
        constexpr array(const array<value_type, L> &left, const array<value_type, R> &right) : elements{} {
            if constexpr ((L + R) <= N) {
                for (std::size_t i = 0; i < L; ++i) {
                    elements[i] = left[i];
                }
                for (std::size_t i = 0; i < R; ++i) {
                    elements[L + i] = right[i];
                }
            }
        }

        RAINY_CONSTEXPR20 ~array() = default;

        /**
         * @brief 截取数组的子区间
         * @tparam NewSize 新数组的大小（默认为原数组大小 N）
         * @param begin_slice 起始位置（包含）
         * @param end_slice 结束位置（不包含）
         * @return 返回从指定区间复制的子数组；若参数非法则返回空数组
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn slice(std::size_t begin_slice = 0, std::size_t end_slice = N)
            -> collections::array<value_type, NewSize> {
            if (std::size_t distance = end_slice - begin_slice; begin_slice < end_slice && distance <= NewSize) {
                collections::array<value_type, NewSize> new_slice{};
                for (std::size_t start = begin_slice, i = 0; start < end_slice; ++start, ++i) {
                    new_slice[i] = elements[start];
                }
                return new_slice;
            }
            return {};
        }

        /**
         * @brief 获取数组左侧的若干元素
         * @tparam NewSize 新数组的大小（默认为原数组大小 N）
         * @param n 要截取的元素数量
         * @return 返回包含前 n 个元素的新数组；若 n 超出范围则返回空数组
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn left(std::size_t n) const -> collections::array<value_type, NewSize> {
            return slice<NewSize>(0, n);
        }

        /**
         * @brief 获取数组右侧的若干元素
         * @tparam NewSize 新数组的大小（默认为原数组大小 N）
         * @param n 要截取的元素数量
         * @return 返回包含后 n 个元素的新数组；若 n 超出范围则返回空数组
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn right(std::size_t n) const -> collections::array<value_type, NewSize> {
            return slice<NewSize>(N - n, N);
        }

        /**
         * @brief 查找指定值在数组中的索引
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
         * @brief 根据谓词函数筛选数组元素
         * @tparam NewSize 返回数组的大小（默认为原数组大小 N）
         * @tparam Pred 谓词函数类型，接收元素并返回可转换为 bool 的结果
         * @param pred 用于筛选元素的谓词
         * @return 返回一个新数组，包含满足谓词的元素；若 NewSize 小于筛选出的元素数，则强制返回空数组
         */
        template <std::size_t NewSize = N, typename Pred>
        constexpr rain_fn filter(Pred &&pred) -> collections::array<value_type, NewSize> {
            collections::array<value_type, NewSize> array;
            std::size_t index_mapping[N]{};
            std::size_t raw_view_index = 0;
            for (std::size_t i = 0; i < N; ++i) {
                if (pred(elements[i])) {
                    index_mapping[raw_view_index++] = i;
                }
            }
            if (NewSize >= raw_view_index) {
                for (std::size_t i = 0; i < raw_view_index; ++i) {
                    array[i] = elements[index_mapping[i]];
                }
            }
            return array;
        }

        /**
         * @brief 返回数组的逆序版本
         * @return 返回一个新数组，其元素顺序与当前数组相反
         */
        constexpr rain_fn reverse() const -> collections::array<value_type, N> {
            collections::array<Ty, N> arr{crbegin(), crend()};
            return arr;
        }

        /**
         * @brief 对数组中的每个元素应用映射函数
         * @tparam Fx 映射函数类型
         * @param func 映射函数
         * @return 返回一个新数组，其元素为映射函数作用后的结果
         */
        template <typename Fx>
        constexpr rain_fn map(Fx &&func) const -> collections::array<value_type, N> {
            collections::array<value_type, N> arr{};
            for (std::size_t i = 0; i < N; ++i) {
                arr[i] = utility::invoke(utility::forward<Fx>(func), elements[i]);
            }
            return arr;
        }

        /**
         * @brief 使用默认初始化值折叠数组元素
         * @tparam Init 折叠操作的初始值类型
         * @return 返回折叠计算的结果
         */
        template <typename Init>
        constexpr rain_fn fold() -> decltype(auto) {
            return core::accumulate(begin(), end(), Init{});
        }

        /**
         * @brief 使用指定初始值折叠数组元素
         * @tparam Init 初始值类型
         * @param init_value 用作折叠计算的初始值
         * @return 返回折叠计算的结果
         */
        template <typename Init>
        constexpr rain_fn fold(const Init &init_value) -> decltype(auto) {
            return core::accumulate(begin(), end(), init_value);
        }

        /**
         * @brief 使用指定初始类型和二元函数折叠数组元素
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
         * @brief 使用指定初始值和二元函数折叠数组元素
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
         * @brief 拼接两个数组
         * @tparam Ni 右侧数组的大小
         * @param right 右侧待拼接的数组
         * @return 返回一个包含左侧与右侧元素的新数组，长度为 N + Ni
         */
        template <std::size_t Ni>
        constexpr rain_fn concat(const array<value_type, Ni> &right) -> array<value_type, N + Ni> {
            constexpr std::size_t size = N + Ni;
            array<value_type, size> arr{*this, right};
            return arr;
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
         * @brief 获取当前数组第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn front() -> reference {
            return elements[0];
        }

        /**
         * @brief 获取当前数组第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn front() const -> const_reference {
            return elements[0];
        }

        /**
         * @brief 获取当前数组最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn back() -> reference {
            return elements[N - 1];
        }

        /**
         * @brief 获取当前数组最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn back() const -> const_reference {
            return elements[N - 1];
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
         * @brief 将一个对象的数据与当前对象的数据进行交换
         * @param right 待交换的数组对象
         */
        RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn swap(array &right) noexcept(impl_traits::is_nothrow_swappable::value) -> void {
            core::algorithm::swap_ranges(elements, elements + N, right.elements);
        }

        /**
         * @brief 将一个值填充到数组中
         * @param value 要填充的值
         */
        RAINY_CONSTEXPR20 rain_fn fill(const Ty &value) -> void {
            core::algorithm::fill_n(begin(), size(), value);
        }

        /**
         * @brief 获取当前数组大小
         * @return 返回当前数组大小
         */
        RAINY_ALWAYS_INLINE constexpr static rain_fn size() noexcept -> size_type {
            return N;
        }

        RAINY_INLINE constexpr rain_fn max_size() const noexcept -> size_type {
            return N;
        }

        RAINY_INLINE constexpr rain_fn length() const noexcept -> size_type {
            return N;
        }

        /**
         * @brief 检查当前数组容器是否为空
         * @return 容器状态
         */
        RAINY_ALWAYS_INLINE constexpr static rain_fn empty() noexcept -> bool {
            return size() == 0;
        }

        /**
         * @brief 获取当前数组地址
         * @return 返回数组的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn data() noexcept -> pointer {
            return static_cast<pointer>(elements);
        }

        /**
         * @brief 获取当前数组地址
         * @return 返回数组的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn data() const noexcept -> const_pointer {
            return static_cast<const_pointer>(elements);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWAYS_INLINE constexpr rain_fn begin() -> iterator {
            return iterator(elements);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn begin() const -> const_iterator {
            return const_iterator(elements);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器常量
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn cbegin() const -> const_iterator {
            return const_iterator(elements);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rbegin() -> reverse_iterator {
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn crbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn end() -> iterator {
            return iterator(elements + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn end() const -> const_iterator {
            return const_iterator(elements + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器常量
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn cend() const -> const_iterator {
            return const_iterator(elements + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rend() -> reverse_iterator {
            return reverse_iterator(begin());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rend() const -> const_reverse_iterator {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn crend() const -> const_reverse_iterator {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief 用于访问原生数组，而不通过data
         * @return 返回原生数组的引用
         */
        constexpr rain_fn access_carrays() noexcept -> carray_type & {
            return elements;
        }

        /**
         * @brief 用于访问原生数组，而不通过data
         * @return 返回原生数组的常量引用
         */
        constexpr rain_fn access_carrays() const noexcept -> const carray_type & {
            return elements;
        }

        /**
         * @brief 判断两个数组是否相等
         * @param left 左侧数组
         * @param right 右侧数组
         * @return 若两个数组元素一一对应且相等则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator==(const array &left, const array &right) noexcept -> bool {
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief 判断两个数组是否不相等
         * @param left 左侧数组
         * @param right 右侧数组
         * @return 若两个数组不相等则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator!=(const array &left, const array &right) noexcept -> bool {
            return !(left == right);
        }

        /**
         * @brief 判断左侧数组是否小于右侧数组
         * @param left 左侧数组
         * @param right 右侧数组
         * @return 若左侧数组在字典序上小于右侧数组则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator<(const array &left, const array &right) noexcept -> bool {
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief 判断左侧数组是否大于右侧数组
         * @param left 左侧数组
         * @param right 右侧数组
         * @return 若左侧数组在字典序上大于右侧数组则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator>(const array &left, const array &right) noexcept -> bool {
            return right < left;
        }

        /**
         * @brief 判断左侧数组是否小于等于右侧数组
         * @param left 左侧数组
         * @param right 右侧数组
         * @return 若左侧数组小于或等于右侧数组则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator<=(const array &left, const array &right) noexcept -> bool {
            return !(left > right);
        }

        /**
         * @brief 判断左侧数组是否大于等于右侧数组
         * @param left 左侧数组
         * @param right 右侧数组
         * @return 若左侧数组大于或等于右侧数组则返回 true，否则返回 false
         */
        friend constexpr rain_fn operator>=(const array &left, const array &right) noexcept -> bool {
            return !(left < right);
        }

    private:
        template <std::size_t... I>
        constexpr array(const_pointer ilist, std::size_t ilist_size,type_traits::helper::index_sequence<I...>) : elements{(I < ilist_size ? ilist[I] : value_type{})...} {
        }

        RAINY_ALWAYS_INLINE static constexpr void range_check(const difference_type offset) noexcept {
            if (offset >= N) {
                std::abort();
            }
        }

        typename impl_traits::type elements;
    };

    template <typename Ty, std::size_t N>
    array(const Ty (&)[N]) -> array<Ty, N>;

    template <typename Ty, std::size_t N>
    array(const std::array<Ty, N> &) -> array<Ty, N>;
}

namespace std {
    template <typename Ty, std::size_t N>
    struct tuple_size<::rainy::collections::array<Ty, N>> : std::integral_constant<std::size_t, N> {};

    template <std::size_t Idx, typename Ty, std::size_t N>
    struct tuple_element<Idx, ::rainy::collections::array<Ty, N>> {
        static_assert(Idx < N, "Index out of bounds");
        using type = Ty;
    };

    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(::rainy::collections::array<Ty, N> &val) noexcept -> Ty & {
        static_assert(Idx < N, "array index is within bounds");
        return val[Idx];
    }

    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(::rainy::collections::array<Ty, N> &&val) noexcept -> Ty && {
        static_assert(Idx < N, "array index is within bounds");
        return std::move(val[Idx]);
    }

    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(const ::rainy::collections::array<Ty, N> &val) noexcept -> const Ty & {
        static_assert(Idx < N, "array index is within bounds");
        return val[Idx];
    }

    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(const ::rainy::collections::array<Ty, N> &&val) noexcept -> const Ty && {
        static_assert(Idx < N, "array index is within bounds");
        return std::move(val[Idx]);
    }
}

namespace rainy::collections {
    template <typename Ty, std::size_t N, typename UTy, typename Fx>
    RAINY_NODISCARD constexpr rain_fn zip_with(const array<Ty, N> &left, const array<UTy, N> &right, Fx &&func) -> auto {
        using type = decltype(utility::invoke(utility::forward<Fx>(func), left[0], right[0]));
        collections::array<type, N> arr;
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = utility::invoke(utility::forward<Fx>(func), left[i], right[i]);
        }
        return arr;
    }
}

#endif
