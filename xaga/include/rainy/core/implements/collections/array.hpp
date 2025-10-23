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
#include <tuple>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/implements/reverse_iterator.hpp>
#include <rainy/core/implements/basic_algorithm.hpp>

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
        using const_reverse_iterator = utility::reverse_iterator<iterator>;

        using impl_traits = implements::array_traits<Ty, N>;

        using carray_type = typename impl_traits::type;

        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

        constexpr array() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) : elements{} {
        }

        template <typename... Inits,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::type_relations::is_convertible<Inits, Ty>...> &&
                          sizeof...(Inits) <= N,
                      int> = 0>
        constexpr array(Inits &&...inits) : elements{static_cast<value_type>(utility::forward<Inits>(inits))...} {
            static_assert(sizeof...(Inits) <= N, "cannot init this array, because the Inits items is too much, cannot to construct");
        }

        constexpr array(const array &right) : elements{} {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = right[i];
            }
        }

        constexpr array(array &&right) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<value_type>) :
            elements{} {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = utility::move(right[i]);
            }
        }

        constexpr array(std::in_place_t, std::size_t count, const_reference value) : elements{} {
            assert(count < N);
            for (std::size_t i = 0; i < count; ++i) {
                elements[i] = value;
            }
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::templates::is_iterator_v<Iter>, int> = 0>
        constexpr array(Iter begin, Iter end) : elements{} {
            std::size_t index{0};
            for (auto iter = begin; iter != end; ++iter, ++index) {
                elements[index] = *iter;
            }
        }

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

        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn left(std::size_t n) const -> collections::array<value_type, NewSize> {
            return slice<NewSize>(0, n);
        }

        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn right(std::size_t n) const -> collections::array<value_type, NewSize> {
            return slice<NewSize>(N - n, N);
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy, value_type> &&
                                                            type_traits::type_properties::is_equal_comparable_v<value_type, UTy>,
                                                        int> = 0>
        constexpr rain_fn index_of(UTy &&value) noexcept -> std::size_t {
            auto iter = core::algorithm::find(begin(), end(), utility::forward<UTy>(value));
            return iter == end() ? npos : (iter - begin());
        }

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
            core::algorithm::swap_ranges(elements, right.elements);
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
        RAINY_ALWASY_INLINE_NODISCARD constexpr iterator end() {
            return iterator(elements + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_iterator end() const {
            return const_iterator(elements + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器常量
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_iterator cend() const {
            return const_iterator(elements + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr reverse_iterator rend() {
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_reverse_iterator rend() const {
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr const_reverse_iterator crend() const {
            return const_reverse_iterator(end());
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

        friend constexpr rain_fn operator==(const array &left, const array &right) noexcept -> bool {
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        friend constexpr rain_fn operator!=(const array &left, const array &right) noexcept -> bool {
            return !(left == right);
        }

        friend constexpr rain_fn operator<(const array &left, const array &right) noexcept -> bool {
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        friend constexpr rain_fn operator>(const array &left, const array &right) noexcept -> bool {
            return right < left;
        }

        friend constexpr rain_fn operator<=(const array &left, const array &right) noexcept -> bool {
            return !(left > right);
        }

        friend constexpr rain_fn operator>=(const array &left, const array &right) noexcept -> bool {
            return !(left < right);
        }

    private:
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

#endif
