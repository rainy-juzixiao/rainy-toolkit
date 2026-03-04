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
     * @brief Static array template, similar to std::array but with enhanced encapsulation and functionality.
     *        静态数组模板array，类似于std::array，但是提供更多的封装和功能。
     *
     * @attention This array is non-aggregate type.
     * @attention 该array是非聚合类型。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
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
         * @brief Default constructor.
         *        Constructs all elements with value-initialization.
         *
         *        默认构造函数。
         *        以值初始化的方式构造所有元素。
         *
         * @note If the element type supports noexcept default construction,
         *       this constructor is also noexcept.
         * @note 若元素类型支持 noexcept 默认构造，则该构造函数同样为 noexcept。
         */
        constexpr array() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) : elements{} {
        }

        /**
         * @brief Constructs array from an initializer list.
         *        使用初始化列表对数组进行初始化。
         *
         * @param ilist Initializer list
         *              初始化列表
         * @note If the initializer list size is greater than the array size,
         *       std::terminate() will be called.
         * @note 如果初始化列表的大小大于其数组大小，则std::terminate()将被调用。
         */
        constexpr array(std::initializer_list<Ty> ilist) :
            array(ilist.begin(), ilist.size(), type_traits::helper::make_index_sequence<N>{}) {
            if (ilist.size() > N) {
                std::terminate();
            }
        }

        /**
         * @brief In-place construction using variadic arguments.
         *        就地使用可变参数列表形式初始化数组。
         *
         * @tparam Inits Parameter types convertible to value_type
         *               可转换为 value_type 的参数类型
         * @param inits Initialization parameter pack
         *              初始化参数包
         * @details When the number of passed arguments does not exceed N and the types are convertible,
         *          the array elements can be directly initialized.
         * @details 当传入的参数数量不超过 N 且类型可转换时，可直接初始化数组元素。
         * @note If the number of arguments exceeds N, a compile-time static_assert error will be triggered.
         * @note 若传入参数数量超过 N，会触发 static_assert 编译期错误。
         */
        template <typename... Inits,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::type_relations::is_convertible<Inits, Ty>...> &&
                          !type_traits::type_relations::is_any_of_v<std::in_place_t, Inits...> && sizeof...(Inits) <= N,
                      int> = 0>
        explicit constexpr array(std::in_place_t, Inits &&...inits) :
            elements{static_cast<value_type>(utility::forward<Inits>(inits))...} {
            static_assert(sizeof...(Inits) <= N, "cannot init this array, because the Inits items is too much, cannot to construct");
        }

        /**
         * @brief Copy constructor.
         *        拷贝构造函数。
         *
         * @param right Another array of the same type
         *              另一个同类型数组
         * @details Copies all elements from the parameter array to the current array one by one.
         * @details 将参数数组的所有元素逐个拷贝到当前数组中。
         */
        constexpr array(const array &right) : elements{} {
            for (std::size_t i = 0; i < N; ++i) {
                elements[i] = right[i];
            }
        }

        /**
         * @brief Move constructor.
         *        移动构造函数。
         *
         * @param right Another array of the same type (to be moved)
         *              另一个同类型数组（将被移动）
         * @details Moves elements from the parameter array to the current array one by one.
         * @details 将参数数组中的元素逐个移动到当前数组中。
         * @note If the element type supports noexcept move construction,
         *       this constructor is also noexcept.
         * @note 若元素类型支持 noexcept 移动构造，则该构造函数同样为 noexcept。
         */
        constexpr array(array &&right) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<value_type>) :
            array(utility::move(right), type_traits::helper::make_index_sequence<N>{}) {
        }

        /**
         * @brief Constructs array with a fixed count of identical values.
         *        使用固定数量与值进行构造。
         *
         * @param count Number of elements to fill
         *              要填充的元素数量
         * @param value Value used for filling
         *              用于填充的值
         * @details Assigns the first count elements to the specified value,
         *          leaving the rest default-initialized.
         * @details 将前 count 个元素赋值为指定值，其余保持默认初始化。
         * @note If count >= N, assertion failure will be triggered.
         * @note 若 count >= N，将触发断言失败。
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
         * @brief Constructs array from an iterator range.
         *        使用迭代器区间构造数组。
         *
         * @tparam Iter Type satisfying iterator concept
         *              满足迭代器概念的类型
         * @param begin Iterator to the beginning
         *              起始迭代器
         * @param end Iterator to the end (exclusive)
         *            结束迭代器（不含）
         * @details Copies elements from the iterator range into the array sequentially,
         *          until reaching the end or filling the array.
         * @details 从迭代器区间中依次复制元素到数组中，直到到达 end 或填满数组。
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
         * @brief Constructs a new array by concatenating two arrays.
         *        使用两个数组拼接构造新数组。
         *
         * @tparam L Size of the left array
         *           左数组大小
         * @tparam R Size of the right array
         *           右数组大小
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @details When L + R does not exceed the current array size N,
         *          copies the contents of the left and right arrays sequentially into the current array.
         * @details 当 L + R 不超过当前数组大小 N 时，按顺序将左右数组内容拷贝至当前数组。
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
         * @brief Extracts a sub-range of the array.
         *        截取数组的子区间。
         *
         * @tparam NewSize Size of the new array (defaults to original size N)
         *                 新数组的大小（默认为原数组大小 N）
         * @param begin_slice Start position (inclusive)
         *                    起始位置（包含）
         * @param end_slice End position (exclusive)
         *                  结束位置（不包含）
         * @return Returns a new array copied from the specified range;
         *         returns an empty array if parameters are invalid.
         * @return 返回从指定区间复制的子数组；若参数非法则返回空数组。
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
         * @brief Gets the first n elements from the left side of the array.
         *        获取数组左侧的若干元素。
         *
         * @tparam NewSize Size of the new array (defaults to original size N)
         *                 新数组的大小（默认为原数组大小 N）
         * @param n Number of elements to extract
         *          要截取的元素数量
         * @return Returns a new array containing the first n elements;
         *         returns an empty array if n is out of range.
         * @return 返回包含前 n 个元素的新数组；若 n 超出范围则返回空数组。
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn left(std::size_t n) const -> collections::array<value_type, NewSize> {
            return slice<NewSize>(0, n);
        }

        /**
         * @brief Gets the last n elements from the right side of the array.
         *        获取数组右侧的若干元素。
         *
         * @tparam NewSize Size of the new array (defaults to original size N)
         *                 新数组的大小（默认为原数组大小 N）
         * @param n Number of elements to extract
         *          要截取的元素数量
         * @return Returns a new array containing the last n elements;
         *         returns an empty array if n is out of range.
         * @return 返回包含后 n 个元素的新数组；若 n 超出范围则返回空数组。
         */
        template <std::size_t NewSize = N>
        RAINY_NODISCARD constexpr rain_fn right(std::size_t n) const -> collections::array<value_type, NewSize> {
            return slice<NewSize>(N - n, N);
        }

        /**
         * @brief Finds the index of a specified value in the array.
         *        查找指定值在数组中的索引。
         *
         * @tparam UTy Type of the value to find
         *             待查找的值的类型
         * @param value Value to find
         *              要查找的值
         * @return Returns the index if a matching element is found, otherwise returns npos.
         * @return 若找到匹配元素则返回其索引，否则返回 npos。
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
         * @brief Filters array elements based on a predicate function.
         *        根据谓词函数筛选数组元素。
         *
         * @tparam NewSize Size of the returned array (defaults to original size N)
         *                 返回数组的大小（默认为原数组大小 N）
         * @tparam Pred Predicate function type, receives an element and returns a result convertible to bool
         *              谓词函数类型，接收元素并返回可转换为 bool 的结果
         * @param pred Predicate used for filtering elements
         *             用于筛选元素的谓词
         * @return Returns a new array containing elements that satisfy the predicate;
         *         forces return of an empty array if NewSize is less than the number of filtered elements.
         * @return 返回一个新数组，包含满足谓词的元素；若 NewSize 小于筛选出的元素数，则强制返回空数组。
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
         * @brief Returns a reversed version of the array.
         *        返回数组的逆序版本。
         *
         * @return Returns a new array whose elements are in reverse order of the current array.
         * @return 返回一个新数组，其元素顺序与当前数组相反。
         */
        constexpr rain_fn reverse() const -> collections::array<value_type, N> {
            collections::array<Ty, N> arr{crbegin(), crend()};
            return arr;
        }

        /**
         * @brief Applies a mapping function to each element of the array.
         *        对数组中的每个元素应用映射函数。
         *
         * @tparam Fx Mapping function type
         *            映射函数类型
         * @param func Mapping function
         *             映射函数
         * @return Returns a new array whose elements are the results of applying the mapping function.
         * @return 返回一个新数组，其元素为映射函数作用后的结果。
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
         * @brief Folds array elements using a default initial value.
         *        使用默认初始化值折叠数组元素。
         *
         * @tparam Init Type of the initial value for the fold operation
         *              折叠操作的初始值类型
         * @return Returns the result of the fold computation.
         * @return 返回折叠计算的结果。
         */
        template <typename Init>
        constexpr rain_fn fold() -> decltype(auto) {
            return core::accumulate(begin(), end(), Init{});
        }

        /**
         * @brief Folds array elements using a specified initial value.
         *        使用指定初始值折叠数组元素。
         *
         * @tparam Init Type of the initial value
         *              初始值类型
         * @param init_value Initial value used for fold computation
         *                   用作折叠计算的初始值
         * @return Returns the result of the fold computation.
         * @return 返回折叠计算的结果。
         */
        template <typename Init>
        constexpr rain_fn fold(const Init &init_value) -> decltype(auto) {
            return core::accumulate(begin(), end(), init_value);
        }

        /**
         * @brief Folds array elements using a specified initial type and binary function.
         *        使用指定初始类型和二元函数折叠数组元素。
         *
         * @tparam Init Type of the initial value
         *              初始值类型
         * @tparam Fx Binary fold function type
         *            二元折叠函数类型
         * @param func Binary function used for folding
         *             用于折叠的二元函数
         * @return Returns the result of the fold computation.
         * @return 返回折叠计算的结果。
         */
        template <typename Init, typename Fx>
        constexpr rain_fn fold(Fx &&func) -> decltype(auto) {
            return core::accumulate(begin(), end(), Init{}, utility::forward<Fx>(func));
        }

        /**
         * @brief Folds array elements using a specified initial value and binary function.
         *        使用指定初始值和二元函数折叠数组元素。
         *
         * @tparam Fx Binary fold function type
         *            二元折叠函数类型
         * @tparam Init Type of the initial value
         *              初始值类型
         * @param func Binary function used for folding
         *             用于折叠的二元函数
         * @param init_value Initial value for fold computation
         *                   折叠计算的初始值
         * @return Returns the result of the fold computation.
         * @return 返回折叠计算的结果。
         */
        template <typename Fx, typename Init>
        constexpr rain_fn fold(Fx &&func, const Init &init_value) -> decltype(auto) {
            return core::accumulate(begin(), end(), init_value, utility::forward<Fx>(func));
        }

        /**
         * @brief Concatenates two arrays.
         *        拼接两个数组。
         *
         * @tparam Ni Size of the right array
         *            右侧数组的大小
         * @param right Right array to concatenate
         *              右侧待拼接的数组
         * @return Returns a new array containing elements from both the left and right arrays,
         *         with a size of N + Ni.
         * @return 返回一个包含左侧与右侧元素的新数组，长度为 N + Ni。
         */
        template <std::size_t Ni>
        constexpr rain_fn concat(const array<value_type, Ni> &right) -> array<value_type, N + Ni> {
            constexpr std::size_t size = N + Ni;
            array<value_type, size> arr{*this, right};
            return arr;
        }

        /**
         * @brief Accesses element at the specified index with bounds checking.
         *        获取指定下标的元素（进行范围检查）。
         *
         * @attention This function performs range checking.
         * @attention 此函数进行范围检查。
         * @param off Offset index
         *            偏移量
         * @return Reference to the element at the corresponding index.
         * @return 返回对应下标的元素引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn at(difference_type off) -> reference {
            range_check(off);
            return elements[off];
        }

        /**
         * @brief Accesses element at the specified index with bounds checking (const version).
         *        获取指定下标的元素（进行范围检查，常量版本）。
         *
         * @attention This function performs range checking.
         * @attention 此函数进行范围检查。
         * @param off Offset index
         *            偏移量
         * @return Const reference to the element at the corresponding index.
         * @return 返回对应下标的元素引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn at(difference_type off) const -> const_reference {
            range_check(off);
            return elements[off];
        }

        /**
         * @brief Accesses the first element of the array.
         *        获取当前数组第一个元素。
         *
         * @return Reference to the first element.
         * @return 返回第一个元素的引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn front() -> reference {
            return elements[0];
        }

        /**
         * @brief Accesses the first element of the array (const version).
         *        获取当前数组第一个元素（常量版本）。
         *
         * @return Const reference to the first element.
         * @return 返回第一个元素的引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn front() const -> const_reference {
            return elements[0];
        }

        /**
         * @brief Accesses the last element of the array.
         *        获取当前数组最后一个元素。
         *
         * @return Reference to the last element.
         * @return 返回最后一个元素的引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn back() -> reference {
            return elements[N - 1];
        }

        /**
         * @brief Accesses the last element of the array (const version).
         *        获取当前数组最后一个元素（常量版本）。
         *
         * @return Const reference to the last element.
         * @return 返回最后一个元素的引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn back() const -> const_reference {
            return elements[N - 1];
        }

        /**
         * @brief Accesses element at the specified index without bounds checking.
         *        获取指定下标的元素（不进行范围检查）。
         *
         * @attention This function does NOT perform range checking.
         * @attention 此函数不进行范围检查。
         * @param idx Index
         *            索引下标
         * @return Reference to the element at the corresponding index.
         * @return 返回对应下标的元素引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn operator[](difference_type idx) noexcept -> reference {
            return elements[idx]; // NOLINT
        }

        /**
         * @brief Accesses element at the specified index without bounds checking (const version).
         *        获取指定下标的元素（不进行范围检查，常量版本）。
         *
         * @attention This function does NOT perform range checking.
         * @attention 此函数不进行范围检查。
         * @param idx Index
         *            索引下标
         * @return Const reference to the element at the corresponding index.
         * @return 返回对应下标的元素引用。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn operator[](difference_type idx) const noexcept -> const_reference {
            return elements[idx]; // NOLINT
        }

        /**
         * @brief Swaps the data of this array with another array.
         *        将一个对象的数据与当前对象的数据进行交换。
         *
         * @param right Array object to swap with
         *              待交换的数组对象
         */
        RAINY_ALWAYS_INLINE RAINY_CONSTEXPR20 rain_fn swap(array &right) noexcept(impl_traits::is_nothrow_swappable::value) -> void {
            core::algorithm::swap_ranges(elements, elements + N, right.elements);
        }

        /**
         * @brief Fills the array with a specified value.
         *        将一个值填充到数组中。
         *
         * @param value Value to fill the array with
         *              要填充的值
         */
        RAINY_CONSTEXPR20 rain_fn fill(const Ty &value) -> void {
            core::algorithm::fill_n(begin(), size(), value);
        }

        /**
         * @brief Gets the size of the array.
         *        获取当前数组大小。
         *
         * @return Returns the size of the current array.
         * @return 返回当前数组大小。
         */
        RAINY_ALWAYS_INLINE constexpr static rain_fn size() noexcept -> size_type {
            return N;
        }

        /**
         * @brief Gets the maximum possible size of the array.
         *        获取数组的最大可能大小。
         *
         * @return Returns N (same as size()).
         * @return 返回 N（与 size() 相同）。
         */
        RAINY_INLINE constexpr rain_fn max_size() const noexcept -> size_type {
            return N;
        }

        /**
         * @brief Gets the length of the array.
         *        获取数组的长度。
         *
         * @return Returns N (same as size()).
         * @return 返回 N（与 size() 相同）。
         */
        RAINY_INLINE constexpr rain_fn length() const noexcept -> size_type {
            return N;
        }

        /**
         * @brief Checks if the array container is empty.
         *        检查当前数组容器是否为空。
         *
         * @return true if size() == 0, false otherwise.
         * @return 容器状态。
         */
        RAINY_ALWAYS_INLINE constexpr static rain_fn empty() noexcept -> bool {
            return size() == 0;
        }

        /**
         * @brief Gets a pointer to the underlying array data.
         *        获取当前数组地址。
         *
         * @return Pointer to the array's data.
         * @return 返回数组的地址，以指针形式。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn data() noexcept -> pointer {
            return static_cast<pointer>(elements);
        }

        /**
         * @brief Gets a const pointer to the underlying array data.
         *        获取当前数组地址（常量版本）。
         *
         * @return Const pointer to the array's data.
         * @return 返回数组的地址，以指针形式。
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr rain_fn data() const noexcept -> const_pointer {
            return static_cast<const_pointer>(elements);
        }

        /**
         * @brief Gets an iterator to the beginning of the array.
         *        获取指向当前数组起始位置的迭代器。
         *
         * @return Iterator to the beginning.
         * @return 返回指向起始的迭代器。
         */
        RAINY_ALWAYS_INLINE constexpr rain_fn begin() -> iterator {
            return iterator(elements);
        }

        /**
         * @brief Gets a const iterator to the beginning of the array.
         *        获取指向当前数组起始位置的迭代器（常量版本）。
         *
         * @return Const iterator to the beginning.
         * @return 返回指向起始的迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn begin() const -> const_iterator {
            return const_iterator(elements);
        }

        /**
         * @brief Gets a const iterator to the beginning of the array.
         *        获取指向当前数组起始位置的迭代器常量。
         *
         * @return Const iterator to the beginning.
         * @return 返回指向起始的迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn cbegin() const -> const_iterator {
            return const_iterator(elements);
        }

        /**
         * @brief Gets a reverse iterator to the beginning of the reversed array.
         *        获取指向当前数组末尾位置的反向迭代器。
         *
         * @return Reverse iterator to the beginning.
         * @return 返回指向起始的反向迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rbegin() -> reverse_iterator {
            return reverse_iterator(end());
        }

        /**
         * @brief Gets a const reverse iterator to the beginning of the reversed array.
         *        获取指向当前数组末尾位置的反向迭代器（常量版本）。
         *
         * @return Const reverse iterator to the beginning.
         * @return 返回指向起始的反向迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(end());
        }

        /**
         * @brief Gets a const reverse iterator to the beginning of the reversed array.
         *        获取指向当前数组末尾位置的反向迭代器常量。
         *
         * @return Const reverse iterator to the beginning.
         * @return 返回指向起始的反向迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn crbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(end());
        }

        /**
         * @brief Gets an iterator to the end of the array.
         *        获取指向当前数组末尾位置的迭代器。
         *
         * @return Iterator to the end.
         * @return 返回指向末尾的迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn end() -> iterator {
            return iterator(elements + N);
        }

        /**
         * @brief Gets a const iterator to the end of the array.
         *        获取指向当前数组末尾位置的迭代器（常量版本）。
         *
         * @return Const iterator to the end.
         * @return 返回指向末尾的迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn end() const -> const_iterator {
            return const_iterator(elements + N);
        }

        /**
         * @brief Gets a const iterator to the end of the array.
         *        获取指向当前数组末尾位置的迭代器常量。
         *
         * @return Const iterator to the end.
         * @return 返回指向末尾的迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn cend() const -> const_iterator {
            return const_iterator(elements + N);
        }

        /**
         * @brief Gets a reverse iterator to the end of the reversed array.
         *        获取指向当前数组末尾位置的反向迭代器。
         *
         * @return Reverse iterator to the end.
         * @return 返回指向末尾的反向迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rend() -> reverse_iterator {
            return reverse_iterator(begin());
        }

        /**
         * @brief Gets a const reverse iterator to the end of the reversed array.
         *        获取指向当前数组末尾位置的反向迭代器（常量版本）。
         *
         * @return Const reverse iterator to the end.
         * @return 返回指向末尾的反向迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn rend() const -> const_reverse_iterator {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief Gets a const reverse iterator to the end of the reversed array.
         *        获取指向当前数组末尾位置的反向迭代器常量。
         *
         * @return Const reverse iterator to the end.
         * @return 返回指向末尾的反向迭代器。
         */
        RAINY_ALWASY_INLINE_NODISCARD constexpr rain_fn crend() const -> const_reverse_iterator {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief Accesses the underlying C-style array directly.
         *        用于访问原生数组，而不通过data。
         *
         * @return Reference to the underlying C-style array.
         * @return 返回原生数组的引用。
         */
        constexpr rain_fn access_carrays() noexcept -> carray_type & {
            return elements;
        }

        /**
         * @brief Accesses the underlying C-style array directly (const version).
         *        用于访问原生数组，而不通过data（常量版本）。
         *
         * @return Const reference to the underlying C-style array.
         * @return 返回原生数组的常量引用。
         */
        constexpr rain_fn access_carrays() const noexcept -> const carray_type & {
            return elements;
        }

        /**
         * @brief Equality comparison operator for arrays.
         *        判断两个数组是否相等。
         *
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @return true if all corresponding elements are equal, false otherwise.
         * @return 若两个数组元素一一对应且相等则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator==(const array &left, const array &right) noexcept -> bool {
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief Inequality comparison operator for arrays.
         *        判断两个数组是否不相等。
         *
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @return true if the arrays are not equal, false otherwise.
         * @return 若两个数组不相等则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator!=(const array &left, const array &right) noexcept -> bool {
            return !(left == right);
        }

        /**
         * @brief Less-than comparison operator for arrays (lexicographical).
         *        判断左侧数组是否小于右侧数组（字典序比较）。
         *
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @return true if left is lexicographically less than right, false otherwise.
         * @return 若左侧数组在字典序上小于右侧数组则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator<(const array &left, const array &right) noexcept -> bool {
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief Greater-than comparison operator for arrays (lexicographical).
         *        判断左侧数组是否大于右侧数组（字典序比较）。
         *
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @return true if left is lexicographically greater than right, false otherwise.
         * @return 若左侧数组在字典序上大于右侧数组则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator>(const array &left, const array &right) noexcept -> bool {
            return right < left;
        }

        /**
         * @brief Less-than-or-equal comparison operator for arrays (lexicographical).
         *        判断左侧数组是否小于等于右侧数组（字典序比较）。
         *
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @return true if left is lexicographically less than or equal to right, false otherwise.
         * @return 若左侧数组小于或等于右侧数组则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator<=(const array &left, const array &right) noexcept -> bool {
            return !(left > right);
        }

        /**
         * @brief Greater-than-or-equal comparison operator for arrays (lexicographical).
         *        判断左侧数组是否大于等于右侧数组（字典序比较）。
         *
         * @param left Left array
         *             左侧数组
         * @param right Right array
         *              右侧数组
         * @return true if left is lexicographically greater than or equal to right, false otherwise.
         * @return 若左侧数组大于或等于右侧数组则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator>=(const array &left, const array &right) noexcept -> bool {
            return !(left < right);
        }

    private:
        template <std::size_t... I>
        constexpr array(const_pointer ilist, std::size_t ilist_size, type_traits::helper::index_sequence<I...>) :
            elements{(I < ilist_size ? ilist[I] : value_type{})...} {
        }

        template <std::size_t... I>
        constexpr array(array &&right, type_traits::helper::index_sequence<I...>) : elements{utility::move_if_noexcept(right[I])...} {
        }

        RAINY_ALWAYS_INLINE static constexpr rain_fn range_check(const difference_type offset) noexcept -> void {
            if (offset >= N) {
                std::abort();
            }
        }

        typename impl_traits::type elements;
    };

    /**
     * @brief Deduction guide for array from C-style array.
     *        从C风格数组推导array类型的指引。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param  C-style array reference
     *         C风格数组引用
     */
    template <typename Ty, std::size_t N>
    array(const Ty (&)[N]) -> array<Ty, N>;

    /**
     * @brief Deduction guide for array from std::array.
     *        从std::array推导array类型的指引。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param  std::array reference
     *         std::array引用
     */
    template <typename Ty, std::size_t N>
    array(const std::array<Ty, N> &) -> array<Ty, N>;
}

namespace std {
    /**
     * @brief Tuple size specialization for array.
     *        array的tuple_size特化。
     *
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     */
    template <typename Ty, std::size_t N>
    struct tuple_size<::rainy::collections::array<Ty, N>> : std::integral_constant<std::size_t, N> {};

    /**
     * @brief Tuple element type specialization for array.
     *        array的tuple_element特化。
     *
     * @tparam Idx Element index
     *             元素索引
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     */
    template <std::size_t Idx, typename Ty, std::size_t N>
    struct tuple_element<Idx, ::rainy::collections::array<Ty, N>> {
        static_assert(Idx < N, "Index out of bounds");
        using type = Ty;
    };

    /**
     * @brief Gets reference to array element at compile-time index.
     *        在编译期索引获取数组元素的引用。
     *
     * @tparam Idx Element index (must be within bounds)
     *             元素索引（必须在范围内）
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param val Array to access
     *            要访问的数组
     * @return Reference to the element at index Idx
     *         索引Idx处元素的引用
     */
    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(::rainy::collections::array<Ty, N> &val) noexcept -> Ty & {
        static_assert(Idx < N, "array index is within bounds");
        return val[Idx];
    }

    /**
     * @brief Gets rvalue reference to array element at compile-time index.
     *        在编译期索引获取数组元素的右值引用。
     *
     * @tparam Idx Element index (must be within bounds)
     *             元素索引（必须在范围内）
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param val Array to access (as rvalue)
     *            要访问的数组（作为右值）
     * @return Rvalue reference to the element at index Idx
     *         索引Idx处元素的右值引用
     */
    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(::rainy::collections::array<Ty, N> &&val) noexcept -> Ty && {
        static_assert(Idx < N, "array index is within bounds");
        return std::move(val[Idx]);
    }

    /**
     * @brief Gets const reference to array element at compile-time index.
     *        在编译期索引获取数组元素的常量引用。
     *
     * @tparam Idx Element index (must be within bounds)
     *             元素索引（必须在范围内）
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param val Const array to access
     *            要访问的常量数组
     * @return Const reference to the element at index Idx
     *         索引Idx处元素的常量引用
     */
    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(const ::rainy::collections::array<Ty, N> &val) noexcept -> const Ty & {
        static_assert(Idx < N, "array index is within bounds");
        return val[Idx];
    }

    /**
     * @brief Gets const rvalue reference to array element at compile-time index.
     *        在编译期索引获取数组元素的常量右值引用。
     *
     * @tparam Idx Element index (must be within bounds)
     *             元素索引（必须在范围内）
     * @tparam Ty Element type
     *            元素类型
     * @tparam N Array size
     *           数组大小
     * @param val Const array to access (as rvalue)
     *            要访问的常量数组（作为右值）
     * @return Const rvalue reference to the element at index Idx
     *         索引Idx处元素的常量右值引用
     */
    template <std::size_t Idx, typename Ty, size_t N>
    constexpr rain_fn get(const ::rainy::collections::array<Ty, N> &&val) noexcept -> const Ty && {
        static_assert(Idx < N, "array index is within bounds");
        return std::move(val[Idx]);
    }
}

namespace rainy::collections {
    /**
     * @brief Combines two arrays element-wise using a binary function.
     *        使用二元函数逐元素组合两个数组。
     *
     * @tparam Ty Element type of the left array
     *            左侧数组的元素类型
     * @tparam N Size of both arrays (must be identical)
     *           两个数组的大小（必须相同）
     * @tparam UTy Element type of the right array
     *             右侧数组的元素类型
     * @tparam Fx Binary function type
     *            二元函数类型
     * @param left Left array
     *             左侧数组
     * @param right Right array
     *              右侧数组
     * @param func Binary function to apply to each pair of elements
     *              应用于每对元素的二元函数
     * @return A new array where each element is the result of func(left[i], right[i])
     *         新数组，每个元素是 func(left[i], right[i]) 的结果
     */
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
