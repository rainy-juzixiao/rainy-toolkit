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
#ifndef RAINY_FOUNDATION_COLLECTIONS_DENSE_SET_HPP
#define RAINY_FOUNDATION_COLLECTIONS_DENSE_SET_HPP

#include <initializer_list>
#include <rainy/core/functional/functor.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/utility/hash.hpp>
#include <rainy/foundation/collections/implements/dense_hash/table.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief Flat-hash set with stable iteration order (backshift erase).
     *
     * @tparam Key The element type.
     * @tparam Hash The hash function object type.
     * @tparam KeyEqual The key equality function object type.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 迭代顺序稳定的扁平哈希集合（backshift 删除）。
     *
     * @tparam Key 元素类型。
     * @tparam Hash 哈希函数对象类型。
     * @tparam KeyEqual 键相等函数对象类型。
     * @tparam Allocator 分配器类型。
     */
    template <typename Key, typename Hash = utility::hash<Key>, typename KeyEqual = functional::equal<Key>,
              typename Allocator = core::memory::allocator<Key>>
    class dense_set : public implements::dense_table<erase_policy::backshift, false, Key, Key, Hash, KeyEqual, Allocator> {
        using base = implements::dense_table<erase_policy::backshift, false, Key, Key, Hash, KeyEqual, Allocator>;

    public:
        /**
         * \lang english
         * @brief The element type.
         *
         * \lang simp-chinese
         * @brief 元素类型。
         */
        using key_type = typename base::key_type;

        /**
         * \lang english
         * @brief The element type.
         *
         * \lang simp-chinese
         * @brief 元素类型。
         */
        using value_type = typename base::value_type;

        /**
         * \lang english
         * @brief Unsigned integer type used to represent sizes.
         *
         * \lang simp-chinese
         * @brief 用于表示大小的无符号整数类型。
         */
        using size_type = typename base::size_type;

        /**
         * \lang english
         * @brief Signed integer type used to represent distances between iterators.
         *
         * \lang simp-chinese
         * @brief 用于表示迭代器之间距离的有符号整数类型。
         */
        using difference_type = typename base::difference_type;

        /**
         * \lang english
         * @brief The hash function object type.
         *
         * \lang simp-chinese
         * @brief 哈希函数对象类型。
         */
        using hasher = typename base::hasher;

        /**
         * \lang english
         * @brief The key equality function object type.
         *
         * \lang simp-chinese
         * @brief 键相等函数对象类型。
         */
        using key_equal = typename base::key_equal;

        /**
         * \lang english
         * @brief The allocator type used to acquire and release memory.
         *
         * \lang simp-chinese
         * @brief 用于获取和释放内存的分配器类型。
         */
        using allocator_type = typename base::allocator_type;

        /**
         * \lang english
         * @brief Reference to an element.
         *
         * \lang simp-chinese
         * @brief 元素的引用。
         */
        using reference = typename base::reference;

        /**
         * \lang english
         * @brief Const reference to an element.
         *
         * \lang simp-chinese
         * @brief 元素的常量引用。
         */
        using const_reference = typename base::const_reference;

        /**
         * \lang english
         * @brief Pointer to an element.
         *
         * \lang simp-chinese
         * @brief 指向元素的指针。
         */
        using pointer = typename base::pointer;

        /**
         * \lang english
         * @brief Const pointer to an element.
         *
         * \lang simp-chinese
         * @brief 指向元素的常量指针。
         */
        using const_pointer = typename base::const_pointer;

        /**
         * \lang english
         * @brief Forward iterator type.
         *
         * \lang simp-chinese
         * @brief 前向迭代器类型。
         */
        using iterator = typename base::iterator;

        /**
         * \lang english
         * @brief Const forward iterator type.
         *
         * \lang simp-chinese
         * @brief 常量前向迭代器类型。
         */
        using const_iterator = typename base::const_iterator;

        /**
         * \lang english
         * @brief Iterator type over the elements of a single bucket.
         *
         * \lang simp-chinese
         * @brief 遍历单个桶内元素的迭代器类型。
         */
        using local_iterator = typename base::local_iterator;

        /**
         * \lang english
         * @brief Const iterator type over the elements of a single bucket.
         *
         * \lang simp-chinese
         * @brief 遍历单个桶内元素的常量迭代器类型。
         */
        using const_local_iterator = typename base::const_local_iterator;

        using base::base;

        /**
         * \lang english
         * @brief Default constructor. Creates an empty set.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。创建一个空的 set。
         */
        dense_set() = default;

        /**
         * \lang english
         * @brief Copy constructor. Copies the elements, the hash function and the equality predicate.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。拷贝元素、哈希函数与相等谓词。
         */
        dense_set(const dense_set &) = default;

        /**
         * \lang english
         * @brief Move constructor. Moves the contents of other into this set.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。将 other 的内容移入本 set。
         */
        dense_set(dense_set &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment. Replaces the contents with a copy of other.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值。用 other 的副本替换内容。
         */
        dense_set &operator=(const dense_set &) = default;

        /**
         * \lang english
         * @brief Move assignment. Replaces the contents by moving from other.
         *
         * \lang simp-chinese
         * @brief 移动赋值。通过移动 other 替换内容。
         */
        dense_set &operator=(dense_set &&) noexcept = default;

        /**
         * \lang english
         * @brief Constructs the set from an initializer list.
         *
         * @param ilist The initializer list to copy elements from
         * @param hash The hash function to use
         * @param equal The key equality predicate to use
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从初始化器列表构造 set。
         *
         * @param ilist 要拷贝元素的初始化器列表
         * @param hash 使用的哈希函数
         * @param equal 使用的键相等谓词
         * @param alloc 使用的分配器
         */
        dense_set(std::initializer_list<value_type> ilist, const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                  const allocator_type &alloc = allocator_type()) : base(ilist.size(), hash, equal, alloc) {
            insert(ilist);
        }

        /**
         * \lang english
         * @brief Constructs the set from the range [first, last).
         *
         * @param first The beginning of the range to copy elements from
         * @param last The end of the range to copy elements from
         * @param hash The hash function to use
         * @param equal The key equality predicate to use
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从区间 [first, last) 构造 set。
         *
         * @param first 要拷贝元素的区间起点
         * @param last 要拷贝元素的区间终点
         * @param hash 使用的哈希函数
         * @param equal 使用的键相等谓词
         * @param alloc 使用的分配器
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        dense_set(InputIt first, InputIt last, const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                  const allocator_type &alloc = allocator_type()) : base(hash, equal, alloc) {
            insert(first, last);
        }

        /**
         * \lang english
         * @brief Inserts a copy of value.
         *
         * @param value The element to insert
         * @return A pair of the iterator to the inserted or existing element and a bool that is true when the insertion took place
         *
         * \lang simp-chinese
         * @brief 插入 value 的副本。
         *
         * @param value 要插入的元素
         * @return 指向插入或已存在元素的迭代器与表示是否发生插入的 bool 组成的 pair
         */
        core::container::pair<iterator, bool> insert(const value_type &value) {
            return this->insert_value(value);
        }

        /**
         * \lang english
         * @brief Inserts value by moving it.
         *
         * @param value The element to move and insert
         * @return A pair of the iterator to the inserted or existing element and a bool that is true when the insertion took place
         *
         * \lang simp-chinese
         * @brief 通过移动插入 value。
         *
         * @param value 要移动并插入的元素
         * @return 指向插入或已存在元素的迭代器与表示是否发生插入的 bool 组成的 pair
         */
        core::container::pair<iterator, bool> insert(value_type &&value) {
            return this->insert_value(utility::move(value));
        }

        /**
         * \lang english
         * @brief Inserts the elements of the range [first, last).
         *
         * @tparam InputIt The input iterator type
         * @param first The beginning of the range to insert
         * @param last The end of the range to insert
         *
         * \lang simp-chinese
         * @brief 插入区间 [first, last) 的元素。
         *
         * @tparam InputIt 输入迭代器类型
         * @param first 要插入的区间起点
         * @param last 要插入的区间终点
         */
        template <typename InputIt>
        void insert(InputIt first, InputIt last) {
            for (; first != last; ++first) {
                this->insert_value(*first);
            }
        }

        /**
         * \lang english
         * @brief Inserts the elements of an initializer list.
         *
         * @param ilist The initializer list to insert
         *
         * \lang simp-chinese
         * @brief 插入初始化器列表的元素。
         *
         * @param ilist 要插入的初始化器列表
         */
        void insert(std::initializer_list<value_type> ilist) {
            insert(ilist.begin(), ilist.end());
        }
    };

    /**
     * \lang english
     * @brief Swaps the contents of two dense sets.
     *
     * @param left The first set
     * @param right The second set
     *
     * \lang simp-chinese
     * @brief 交换两个 dense_set 的内容。
     *
     * @param left 第一个 set
     * @param right 第二个 set
     */
    template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
    void swap(dense_set<Key, Hash, KeyEqual, Alloc> &left, dense_set<Key, Hash, KeyEqual, Alloc> &right) noexcept {
        left.swap(right);
    }
}

#endif
