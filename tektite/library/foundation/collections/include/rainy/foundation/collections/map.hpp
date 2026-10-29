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
#ifndef RAINY_FOUNDATION_COLLECTIONS_MAP_HPP
#define RAINY_FOUNDATION_COLLECTIONS_MAP_HPP

#include <initializer_list>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/functional/functor.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/collections/implements/rb_tree/tree.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief A sorted associative container with unique keys.
     *
     *  Elements are key/value pairs ordered by their keys. Iteration is in
     *  ascending key order. Lookup, insertion, and removal are logarithmic.
     *
     * @tparam Key The key type.
     * @tparam Mapped The mapped value type.
     * @tparam Compare The strict weak ordering used to order keys.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 具有唯一键的有序关联容器。
     *
     *  元素为按键排序的键值对。迭代按键升序进行。查找、插入与删除均为对数复杂度。
     *
     * @tparam Key 键类型。
     * @tparam Mapped 映射值类型。
     * @tparam Compare 用于排序键的严格弱序。
     * @tparam Allocator 分配器类型。
     */
    template <typename Key, typename Mapped, typename Compare = functional::less<Key>,
              typename Allocator = core::memory::allocator<core::container::pair<const Key, Mapped>>>
    class map : private implements::rb_tree<false, true, Key, Mapped, Compare, Allocator> {
    public:
        // @hide
        using base = implements::rb_tree<false, true, Key, Mapped, Compare, Allocator>;

        /**
         * \lang english
         * @brief The key type.
         *
         * \lang simp-chinese
         * @brief 键类型。
         */
        using key_type = typename base::key_type;

        /**
         * \lang english
         * @brief The type of the value associated with each key.
         *
         * \lang simp-chinese
         * @brief 与每个键关联的值的类型。
         */
        using mapped_type = typename base::mapped_type;

        /**
         * \lang english
         * @brief The key/value pair type stored in the map.
         *
         * \lang simp-chinese
         * @brief 映射中存储的键值对类型。
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
         * @brief The comparator type used to order keys.
         *
         * \lang simp-chinese
         * @brief 用于排序键的比较器类型。
         */
        using key_compare = typename base::key_compare;

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
         * @brief Bidirectional iterator type.
         *
         * \lang simp-chinese
         * @brief 双向迭代器类型。
         */
        using iterator = typename base::iterator;

        /**
         * \lang english
         * @brief Const bidirectional iterator type.
         *
         * \lang simp-chinese
         * @brief 常量双向迭代器类型。
         */
        using const_iterator = typename base::const_iterator;

        /**
         * \lang english
         * @brief The node handle type used to transfer elements between containers.
         *
         * \lang simp-chinese
         * @brief 用于在容器之间转移元素的节点句柄类型。
         */
        using node_type = typename base::node_handle;

        /**
         * \lang english
         * @brief The result type of inserting a node handle.
         *
         * \lang simp-chinese
         * @brief 插入节点句柄的结果类型。
         */
        using insert_return_type = typename base::insert_return_type;

        /**
         * \lang english
         * @brief The comparator type used to order value_type objects.
         *
         * \lang simp-chinese
         * @brief 用于排序 value_type 对象的比较器类型。
         */
        using value_compare = typename base::value_compare;

        using base::base;
        using base::begin;
        using base::cbegin;
        using base::cend;
        using base::clear;
        using base::contains;
        using base::count;
        using base::empty;
        using base::end;
        using base::equal_range;
        using base::erase;
        using base::extract;
        using base::find;
        using base::get_allocator;
        using base::key_comp;
        using base::lower_bound;
        using base::max_size;
        using base::size;
        using base::upper_bound;
        using base::value_comp;

        /**
         * \lang english
         * @brief Exchanges the contents of two maps.
         *
         * @param other The map to exchange contents with
         *
         * \lang simp-chinese
         * @brief 交换两个 map 的内容。
         *
         * @param other 要与之交换内容的 map
         */
        void swap(map &other) noexcept {
            base::swap(other);
        }

        /**
         * \lang english
         * @brief Splices every element of source whose key is not present in this map.
         *
         * @param source The map to splice elements from
         *
         * \lang simp-chinese
         * @brief 将 source 中键不存在于本 map 的元素全部接合进来。
         *
         * @param source 要从中接合元素的 map
         */
        void merge(map &source) {
            base::merge(source);
        }

        /**
         * \lang english
         * @brief Default constructor. Creates an empty map.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。创建一个空的 map。
         */
        map() = default;

        /**
         * \lang english
         * @brief Copy constructor. Copies the elements and the comparator.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。拷贝元素与比较器。
         */
        map(const map &) = default;

        /**
         * \lang english
         * @brief Move constructor. Moves the contents of other into this map.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。将 other 的内容移入本 map。
         */
        map(map &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment. Replaces the contents with a copy of other.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值。用 other 的副本替换内容。
         */
        map &operator=(const map &) = default;

        /**
         * \lang english
         * @brief Move assignment. Replaces the contents by moving from other.
         *
         * \lang simp-chinese
         * @brief 移动赋值。通过移动 other 替换内容。
         */
        map &operator=(map &&) noexcept = default;

        /**
         * \lang english
         * @brief Constructs an empty map with the given comparator and allocator.
         *
         * @param comp The comparator used to order keys
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定的比较器与分配器构造一个空的 map。
         *
         * @param comp 用于排序键的比较器
         * @param alloc 使用的分配器
         */
        explicit map(const Compare &comp, const Allocator &alloc = Allocator()) : base(comp, alloc) {
        }

        /**
         * \lang english
         * @brief Constructs an empty map with the given allocator.
         *
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定的分配器构造一个空的 map。
         *
         * @param alloc 使用的分配器
         */
        explicit map(const Allocator &alloc) : base(alloc) {
        }

        /**
         * \lang english
         * @brief Constructs the map from the range [first, last).
         *
         * @param first The beginning of the range to copy elements from
         * @param last The end of the range to copy elements from
         * @param comp The comparator used to order keys
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从区间 [first, last) 构造 map。
         *
         * @param first 要拷贝元素的区间起点
         * @param last 要拷贝元素的区间终点
         * @param comp 用于排序键的比较器
         * @param alloc 使用的分配器
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        map(InputIt first, InputIt last, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) : base(comp, alloc) {
            this->insert_unique_range(first, last);
        }

        /**
         * \lang english
         * @brief Constructs the map from an initializer list.
         *
         * @param init The initializer list to copy elements from
         * @param comp The comparator used to order keys
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从初始化器列表构造 map。
         *
         * @param init 要拷贝元素的初始化器列表
         * @param comp 用于排序键的比较器
         * @param alloc 使用的分配器
         */
        map(std::initializer_list<value_type> init, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            base(comp, alloc) {
            this->insert_unique_range(init.begin(), init.end());
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
            return this->insert_unique(value);
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
            return this->insert_unique(utility::move(value));
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from value.
         *
         * @tparam P The type forwarded to the constructor of value_type
         * @param value The value used to construct the element
         * @return A pair of the iterator to the inserted or existing element and a bool that is true when the insertion took place
         *
         * \lang simp-chinese
         * @brief 使用 value 就地构造并插入元素。
         *
         * @tparam P 转发给 value_type 构造函数的类型
         * @param value 用于构造元素的值
         * @return 指向插入或已存在元素的迭代器与表示是否发生插入的 bool 组成的 pair
         */
        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        core::container::pair<iterator, bool> insert(P &&value) {
            return this->insert_unique(utility::forward<P>(value));
        }

        /**
         * \lang english
         * @brief Inserts a copy of value, using hint as a position hint.
         *
         * @param hint A position hint for the insertion
         * @param value The element to insert
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 插入 value 的副本，并以 hint 作为位置提示。
         *
         * @param hint 插入的位置提示
         * @param value 要插入的元素
         * @return 指向插入或已存在元素的迭代器
         */
        iterator insert(const_iterator hint, const value_type &value) {
            return this->emplace_hint_unique(hint, value);
        }

        /**
         * \lang english
         * @brief Inserts value by moving it, using hint as a position hint.
         *
         * @param hint A position hint for the insertion
         * @param value The element to move and insert
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 通过移动插入 value，并以 hint 作为位置提示。
         *
         * @param hint 插入的位置提示
         * @param value 要移动并插入的元素
         * @return 指向插入或已存在元素的迭代器
         */
        iterator insert(const_iterator hint, value_type &&value) {
            return this->emplace_hint_unique(hint, utility::move(value));
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from value, using hint as a position hint.
         *
         * @tparam P The type forwarded to the constructor of value_type
         * @param hint A position hint for the insertion
         * @param value The value used to construct the element
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 使用 value 就地构造并插入元素，并以 hint 作为位置提示。
         *
         * @tparam P 转发给 value_type 构造函数的类型
         * @param hint 插入的位置提示
         * @param value 用于构造元素的值
         * @return 指向插入或已存在元素的迭代器
         */
        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert(const_iterator hint, P &&value) {
            return this->emplace_hint_unique(hint, utility::forward<P>(value));
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
            this->insert_unique_range(first, last);
        }

        /**
         * \lang english
         * @brief Inserts the elements of an initializer list.
         *
         * @param init The initializer list to insert
         *
         * \lang simp-chinese
         * @brief 插入初始化器列表的元素。
         *
         * @param init 要插入的初始化器列表
         */
        void insert(std::initializer_list<value_type> init) {
            this->insert_unique_range(init.begin(), init.end());
        }

        /**
         * \lang english
         * @brief Inserts a node handle. If the key is already present, the handle is left unchanged.
         *
         * @param handle The node handle to insert
         * @return The insertion result, holding the position, a bool that is true when the insertion took place,
         *         and the remaining node
         *
         * \lang simp-chinese
         * @brief 插入节点句柄。若键已存在，则句柄保持不变。
         *
         * @param handle 要插入的节点句柄
         * @return 插入结果，包含位置、表示是否发生插入的 bool 以及剩余节点
         */
        insert_return_type insert(node_type &&handle) {
            core::container::pair<iterator, bool> result = this->insert_unique_node(utility::move(handle));
            if (result.second) {
                return {result.first, true, node_type{}};
            }
            return {result.first, false, utility::move(handle)};
        }

        /**
         * \lang english
         * @brief Inserts a node handle, using hint as a position hint.
         *
         * @param hint A position hint for the insertion
         * @param handle The node handle to insert
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 插入节点句柄，并以 hint 作为位置提示。
         *
         * @param hint 插入的位置提示
         * @param handle 要插入的节点句柄
         * @return 指向插入或已存在元素的迭代器
         */
        iterator insert(const_iterator hint, node_type &&handle) {
            (void) hint;
            return insert(utility::move(handle)).position;
        }

        /**
         * \lang english
         * @brief Constructs an element in place from args.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of value_type
         * @param args The arguments forwarded to the constructor of value_type
         * @return A pair of the iterator to the inserted or existing element and a bool that is true when the insertion took place
         *
         * \lang simp-chinese
         * @brief 使用 args 就地构造元素。
         *
         * @tparam Args 转发给 value_type 构造函数的实参类型
         * @param args 转发给 value_type 构造函数的实参
         * @return 指向插入或已存在元素的迭代器与表示是否发生插入的 bool 组成的 pair
         */
        template <typename... Args>
        core::container::pair<iterator, bool> emplace(Args &&...args) {
            return this->emplace_unique(utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Constructs an element in place from args, using hint as a position hint.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of value_type
         * @param hint A position hint for the insertion
         * @param args The arguments forwarded to the constructor of value_type
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 使用 args 就地构造元素，并以 hint 作为位置提示。
         *
         * @tparam Args 转发给 value_type 构造函数的实参类型
         * @param hint 插入的位置提示
         * @param args 转发给 value_type 构造函数的实参
         * @return 指向插入或已存在元素的迭代器
         */
        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
            return this->emplace_hint_unique(hint, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Inserts obj under key, or assigns obj to the mapped value if key already exists.
         *
         * @tparam M The type of the value to assign
         * @param key The key to insert or assign to
         * @param obj The value to insert or assign
         * @return A pair of the iterator to the element and a bool that is true when the element was inserted
         *
         * \lang simp-chinese
         * @brief 将 obj 插入到 key 下；若 key 已存在，则将 obj 赋给对应的映射值。
         *
         * @tparam M 要赋值的值的类型
         * @param key 要插入或赋值的键
         * @param obj 要插入或赋的值
         * @return 指向元素的迭代器与表示是否为新插入的 bool 组成的 pair
         */
        template <typename M>
        core::container::pair<iterator, bool> insert_or_assign(const key_type &key, M &&obj) {
            return insert_or_assign_impl(key, utility::forward<M>(obj));
        }

        /**
         * \lang english
         * @brief Inserts obj under key by moving key, or assigns obj to the mapped value if key already exists.
         *
         * @tparam M The type of the value to assign
         * @param key The key to move, insert or assign to
         * @param obj The value to insert or assign
         * @return A pair of the iterator to the element and a bool that is true when the element was inserted
         *
         * \lang simp-chinese
         * @brief 通过移动 key 将 obj 插入到 key 下；若 key 已存在，则将 obj 赋给对应的映射值。
         *
         * @tparam M 要赋值的值的类型
         * @param key 要移动、插入或赋值的键
         * @param obj 要插入或赋的值
         * @return 指向元素的迭代器与表示是否为新插入的 bool 组成的 pair
         */
        template <typename M>
        core::container::pair<iterator, bool> insert_or_assign(key_type &&key, M &&obj) {
            return insert_or_assign_impl(utility::move(key), utility::forward<M>(obj));
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from args if key is not present.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of mapped_type
         * @param key The key to look up or insert
         * @param args The arguments forwarded to the constructor of mapped_type
         * @return A pair of the iterator to the element and a bool that is true when the element was inserted
         *
         * \lang simp-chinese
         * @brief 若 key 不存在，则使用 args 就地构造元素并插入。
         *
         * @tparam Args 转发给 mapped_type 构造函数的实参类型
         * @param key 要查找或插入的键
         * @param args 转发给 mapped_type 构造函数的实参
         * @return 指向元素的迭代器与表示是否为新插入的 bool 组成的 pair
         */
        template <typename... Args>
        core::container::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args) {
            return try_emplace_impl(key, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from args if key is not present, moving key.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of mapped_type
         * @param key The key to move, look up or insert
         * @param args The arguments forwarded to the constructor of mapped_type
         * @return A pair of the iterator to the element and a bool that is true when the element was inserted
         *
         * \lang simp-chinese
         * @brief 若 key 不存在，则通过移动 key 使用 args 就地构造元素并插入。
         *
         * @tparam Args 转发给 mapped_type 构造函数的实参类型
         * @param key 要移动、查找或插入的键
         * @param args 转发给 mapped_type 构造函数的实参
         * @return 指向元素的迭代器与表示是否为新插入的 bool 组成的 pair
         */
        template <typename... Args>
        core::container::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args) {
            return try_emplace_impl(utility::move(key), utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from args if key is not present, using hint as a position hint.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of mapped_type
         * @param hint A position hint for the insertion
         * @param key The key to look up or insert
         * @param args The arguments forwarded to the constructor of mapped_type
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 若 key 不存在，则使用 args 就地构造元素并插入，并以 hint 作为位置提示。
         *
         * @tparam Args 转发给 mapped_type 构造函数的实参类型
         * @param hint 插入的位置提示
         * @param key 要查找或插入的键
         * @param args 转发给 mapped_type 构造函数的实参
         * @return 指向插入或已存在元素的迭代器
         */
        template <typename... Args>
        iterator try_emplace(const_iterator hint, const key_type &key, Args &&...args) {
            (void) hint;
            return try_emplace_impl(key, utility::forward<Args>(args)...).first;
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from args if key is not present, moving key, with hint as a position hint.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of mapped_type
         * @param hint A position hint for the insertion
         * @param key The key to move, look up or insert
         * @param args The arguments forwarded to the constructor of mapped_type
         * @return An iterator to the inserted or existing element
         *
         * \lang simp-chinese
         * @brief 若 key 不存在，则通过移动 key 使用 args 就地构造元素并插入，并以 hint 作为位置提示。
         *
         * @tparam Args 转发给 mapped_type 构造函数的实参类型
         * @param hint 插入的位置提示
         * @param key 要移动、查找或插入的键
         * @param args 转发给 mapped_type 构造函数的实参
         * @return 指向插入或已存在元素的迭代器
         */
        template <typename... Args>
        iterator try_emplace(const_iterator hint, key_type &&key, Args &&...args) {
            (void) hint;
            return try_emplace_impl(utility::move(key), utility::forward<Args>(args)...).first;
        }

        /**
         * \lang english
         * @brief Returns a reference to the mapped value of key.
         *
         * @param key The key to look up
         * @return A reference to the mapped value
         * @throws core::exceptions::logic::out_of_range If key is not present
         *
         * \lang simp-chinese
         * @brief 返回 key 对应映射值的引用。
         *
         * @param key 要查找的键
         * @return 映射值的引用
         * @throws core::exceptions::logic::out_of_range 若 key 不存在
         */
        mapped_type &at(const key_type &key) {
            iterator it = this->find(key);
            if (it == this->end()) {
                core::exceptions::logic::throw_out_of_range("map::at: key not found");
            }
            return it->second;
        }

        /**
         * \lang english
         * @brief Returns a const reference to the mapped value of key.
         *
         * @param key The key to look up
         * @return A const reference to the mapped value
         * @throws core::exceptions::logic::out_of_range If key is not present
         *
         * \lang simp-chinese
         * @brief 返回 key 对应映射值的常量引用。
         *
         * @param key 要查找的键
         * @return 映射值的常量引用
         * @throws core::exceptions::logic::out_of_range 若 key 不存在
         */
        const mapped_type &at(const key_type &key) const {
            const_iterator it = this->find(key);
            if (it == this->end()) {
                core::exceptions::logic::throw_out_of_range("map::at: key not found");
            }
            return it->second;
        }

        /**
         * \lang english
         * @brief Returns a reference to the mapped value of key, inserting a value-initialized element if key is not present.
         *
         * @param key The key to look up or insert
         * @return A reference to the mapped value
         *
         * \lang simp-chinese
         * @brief 返回 key 对应映射值的引用；若 key 不存在，则插入一个值初始化的元素。
         *
         * @param key 要查找或插入的键
         * @return 映射值的引用
         */
        mapped_type &operator[](const key_type &key) {
            return try_emplace_impl(key).first->second;
        }

        /**
         * \lang english
         * @brief Returns a reference to the mapped value of key, inserting a value-initialized element if key is not present.
         *
         * @param key The key to move, look up or insert
         * @return A reference to the mapped value
         *
         * \lang simp-chinese
         * @brief 返回 key 对应映射值的引用；若 key 不存在，则通过移动 key 插入一个值初始化的元素。
         *
         * @param key 要移动、查找或插入的键
         * @return 映射值的引用
         */
        mapped_type &operator[](key_type &&key) {
            return try_emplace_impl(utility::move(key)).first->second;
        }

    private:
        template <typename K, typename M>
        core::container::pair<iterator, bool> insert_or_assign_impl(K &&key, M &&obj) {
            iterator it = this->find(key);
            if (it != this->end()) {
                it->second = utility::forward<M>(obj);
                return core::container::make_pair(it, false);
            }
            return this->emplace_unique(utility::forward<K>(key), utility::forward<M>(obj));
        }

        template <typename K, typename... Args>
        core::container::pair<iterator, bool> try_emplace_impl(K &&key, Args &&...args) {
            iterator it = this->find(key);
            if (it != this->end()) {
                return core::container::make_pair(it, false);
            }
            return this->emplace_unique(utility::forward<K>(key), mapped_type(utility::forward<Args>(args)...));
        }
    };

    /**
     * \lang english
     * @brief A sorted associative container that allows duplicate keys.
     *
     * @tparam Key The key type.
     * @tparam Mapped The mapped value type.
     * @tparam Compare The strict weak ordering used to order keys.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 允许重复键的有序关联容器。
     *
     * @tparam Key 键类型。
     * @tparam Mapped 映射值类型。
     * @tparam Compare 用于排序键的严格弱序。
     * @tparam Allocator 分配器类型。
     */
    template <typename Key, typename Mapped, typename Compare = functional::less<Key>,
              typename Allocator = core::memory::allocator<core::container::pair<const Key, Mapped>>>
    class multimap : private implements::rb_tree<true, true, Key, Mapped, Compare, Allocator> {
    public:
        // @hide
        using base = implements::rb_tree<true, true, Key, Mapped, Compare, Allocator>;

        /**
         * \lang english
         * @brief The key type.
         *
         * \lang simp-chinese
         * @brief 键类型。
         */
        using key_type = typename base::key_type;

        /**
         * \lang english
         * @brief The type of the value associated with each key.
         *
         * \lang simp-chinese
         * @brief 与每个键关联的值的类型。
         */
        using mapped_type = typename base::mapped_type;

        /**
         * \lang english
         * @brief The key/value pair type stored in the multimap.
         *
         * \lang simp-chinese
         * @brief 多重映射中存储的键值对类型。
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
         * @brief The comparator type used to order keys.
         *
         * \lang simp-chinese
         * @brief 用于排序键的比较器类型。
         */
        using key_compare = typename base::key_compare;

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
         * @brief Bidirectional iterator type.
         *
         * \lang simp-chinese
         * @brief 双向迭代器类型。
         */
        using iterator = typename base::iterator;

        /**
         * \lang english
         * @brief Const bidirectional iterator type.
         *
         * \lang simp-chinese
         * @brief 常量双向迭代器类型。
         */
        using const_iterator = typename base::const_iterator;

        /**
         * \lang english
         * @brief The node handle type used to transfer elements between containers.
         *
         * \lang simp-chinese
         * @brief 用于在容器之间转移元素的节点句柄类型。
         */
        using node_type = typename base::node_handle;

        /**
         * \lang english
         * @brief The comparator type used to order value_type objects.
         *
         * \lang simp-chinese
         * @brief 用于排序 value_type 对象的比较器类型。
         */
        using value_compare = typename base::value_compare;

        using base::base;
        using base::begin;
        using base::cbegin;
        using base::cend;
        using base::clear;
        using base::contains;
        using base::count;
        using base::empty;
        using base::end;
        using base::equal_range;
        using base::erase;
        using base::extract;
        using base::find;
        using base::get_allocator;
        using base::key_comp;
        using base::lower_bound;
        using base::max_size;
        using base::size;
        using base::upper_bound;
        using base::value_comp;

        /**
         * \lang english
         * @brief Exchanges the contents of two multimaps.
         *
         * @param other The multimap to exchange contents with
         *
         * \lang simp-chinese
         * @brief 交换两个 multimap 的内容。
         *
         * @param other 要与之交换内容的 multimap
         */
        void swap(multimap &other) noexcept {
            base::swap(other);
        }

        /**
         * \lang english
         * @brief Splices every element of source into this multimap.
         *
         * @param source The multimap to splice elements from
         *
         * \lang simp-chinese
         * @brief 将 source 的所有元素接合进本 multimap。
         *
         * @param source 要从中接合元素的 multimap
         */
        void merge(multimap &source) {
            base::merge(source);
        }

        /**
         * \lang english
         * @brief Default constructor. Creates an empty multimap.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。创建一个空的 multimap。
         */
        multimap() = default;

        /**
         * \lang english
         * @brief Copy constructor. Copies the elements and the comparator.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。拷贝元素与比较器。
         */
        multimap(const multimap &) = default;

        /**
         * \lang english
         * @brief Move constructor. Moves the contents of other into this multimap.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。将 other 的内容移入本 multimap。
         */
        multimap(multimap &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment. Replaces the contents with a copy of other.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值。用 other 的副本替换内容。
         */
        multimap &operator=(const multimap &) = default;

        /**
         * \lang english
         * @brief Move assignment. Replaces the contents by moving from other.
         *
         * \lang simp-chinese
         * @brief 移动赋值。通过移动 other 替换内容。
         */
        multimap &operator=(multimap &&) noexcept = default;

        /**
         * \lang english
         * @brief Constructs an empty multimap with the given comparator and allocator.
         *
         * @param comp The comparator used to order keys
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定的比较器与分配器构造一个空的 multimap。
         *
         * @param comp 用于排序键的比较器
         * @param alloc 使用的分配器
         */
        explicit multimap(const Compare &comp, const Allocator &alloc = Allocator()) : base(comp, alloc) {
        }

        /**
         * \lang english
         * @brief Constructs an empty multimap with the given allocator.
         *
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定的分配器构造一个空的 multimap。
         *
         * @param alloc 使用的分配器
         */
        explicit multimap(const Allocator &alloc) : base(alloc) {
        }

        /**
         * \lang english
         * @brief Constructs the multimap from the range [first, last).
         *
         * @param first The beginning of the range to copy elements from
         * @param last The end of the range to copy elements from
         * @param comp The comparator used to order keys
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从区间 [first, last) 构造 multimap。
         *
         * @param first 要拷贝元素的区间起点
         * @param last 要拷贝元素的区间终点
         * @param comp 用于排序键的比较器
         * @param alloc 使用的分配器
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        multimap(InputIt first, InputIt last, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            base(comp, alloc) {
            this->insert_multi_range(first, last);
        }

        /**
         * \lang english
         * @brief Constructs the multimap from an initializer list.
         *
         * @param init The initializer list to copy elements from
         * @param comp The comparator used to order keys
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从初始化器列表构造 multimap。
         *
         * @param init 要拷贝元素的初始化器列表
         * @param comp 用于排序键的比较器
         * @param alloc 使用的分配器
         */
        multimap(std::initializer_list<value_type> init, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            base(comp, alloc) {
            this->insert_multi_range(init.begin(), init.end());
        }

        /**
         * \lang english
         * @brief Inserts a copy of value.
         *
         * @param value The element to insert
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 插入 value 的副本。
         *
         * @param value 要插入的元素
         * @return 指向插入元素的迭代器
         */
        iterator insert(const value_type &value) {
            return this->insert_multi(value);
        }

        /**
         * \lang english
         * @brief Inserts value by moving it.
         *
         * @param value The element to move and insert
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 通过移动插入 value。
         *
         * @param value 要移动并插入的元素
         * @return 指向插入元素的迭代器
         */
        iterator insert(value_type &&value) {
            return this->insert_multi(utility::move(value));
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from value.
         *
         * @tparam P The type forwarded to the constructor of value_type
         * @param value The value used to construct the element
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 使用 value 就地构造并插入元素。
         *
         * @tparam P 转发给 value_type 构造函数的类型
         * @param value 用于构造元素的值
         * @return 指向插入元素的迭代器
         */
        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert(P &&value) {
            return this->insert_multi(utility::forward<P>(value));
        }

        /**
         * \lang english
         * @brief Inserts a copy of value, using hint as a position hint.
         *
         * @param hint A position hint for the insertion
         * @param value The element to insert
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 插入 value 的副本，并以 hint 作为位置提示。
         *
         * @param hint 插入的位置提示
         * @param value 要插入的元素
         * @return 指向插入元素的迭代器
         */
        iterator insert(const_iterator hint, const value_type &value) {
            return this->emplace_hint_multi(hint, value);
        }

        /**
         * \lang english
         * @brief Inserts value by moving it, using hint as a position hint.
         *
         * @param hint A position hint for the insertion
         * @param value The element to move and insert
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 通过移动插入 value，并以 hint 作为位置提示。
         *
         * @param hint 插入的位置提示
         * @param value 要移动并插入的元素
         * @return 指向插入元素的迭代器
         */
        iterator insert(const_iterator hint, value_type &&value) {
            return this->emplace_hint_multi(hint, utility::move(value));
        }

        /**
         * \lang english
         * @brief Inserts an element constructed in place from value, using hint as a position hint.
         *
         * @tparam P The type forwarded to the constructor of value_type
         * @param hint A position hint for the insertion
         * @param value The value used to construct the element
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 使用 value 就地构造并插入元素，并以 hint 作为位置提示。
         *
         * @tparam P 转发给 value_type 构造函数的类型
         * @param hint 插入的位置提示
         * @param value 用于构造元素的值
         * @return 指向插入元素的迭代器
         */
        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert(const_iterator hint, P &&value) {
            return this->emplace_hint_multi(hint, utility::forward<P>(value));
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
            this->insert_multi_range(first, last);
        }

        /**
         * \lang english
         * @brief Inserts the elements of an initializer list.
         *
         * @param init The initializer list to insert
         *
         * \lang simp-chinese
         * @brief 插入初始化器列表的元素。
         *
         * @param init 要插入的初始化器列表
         */
        void insert(std::initializer_list<value_type> init) {
            this->insert_multi_range(init.begin(), init.end());
        }

        /**
         * \lang english
         * @brief Inserts a node handle.
         *
         * @param handle The node handle to insert
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 插入节点句柄。
         *
         * @param handle 要插入的节点句柄
         * @return 指向插入元素的迭代器
         */
        iterator insert(node_type &&handle) {
            return this->insert_multi_node(utility::move(handle));
        }

        /**
         * \lang english
         * @brief Inserts a node handle, using hint as a position hint.
         *
         * @param hint A position hint for the insertion
         * @param handle The node handle to insert
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 插入节点句柄，并以 hint 作为位置提示。
         *
         * @param hint 插入的位置提示
         * @param handle 要插入的节点句柄
         * @return 指向插入元素的迭代器
         */
        iterator insert(const_iterator hint, node_type &&handle) {
            (void) hint;
            return this->insert_multi_node(utility::move(handle));
        }

        /**
         * \lang english
         * @brief Constructs an element in place from args.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of value_type
         * @param args The arguments forwarded to the constructor of value_type
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 使用 args 就地构造元素。
         *
         * @tparam Args 转发给 value_type 构造函数的实参类型
         * @param args 转发给 value_type 构造函数的实参
         * @return 指向插入元素的迭代器
         */
        template <typename... Args>
        iterator emplace(Args &&...args) {
            return this->emplace_multi(utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Constructs an element in place from args, using hint as a position hint.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of value_type
         * @param hint A position hint for the insertion
         * @param args The arguments forwarded to the constructor of value_type
         * @return An iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 使用 args 就地构造元素，并以 hint 作为位置提示。
         *
         * @tparam Args 转发给 value_type 构造函数的实参类型
         * @param hint 插入的位置提示
         * @param args 转发给 value_type 构造函数的实参
         * @return 指向插入元素的迭代器
         */
        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
            return this->emplace_hint_multi(hint, utility::forward<Args>(args)...);
        }
    };

    /**
     * \lang english
     * @brief Compares two maps for equality.
     *
     * @param left The left map
     * @param right The right map
     * @return true if both maps have the same size and every corresponding key/value pair is equal
     *
     * \lang simp-chinese
     * @brief 判断两个 map 是否相等。
     *
     * @param left 左侧 map
     * @param right 右侧 map
     * @return 若两个 map 大小相同且每一对对应的键值对都相等则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator==(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        if (left.size() != right.size()) {
            return false;
        }
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end(); ++lit, ++rit) {
            if (lit->first != rit->first || lit->second != rit->second) {
                return false;
            }
        }
        return true;
    }

    /**
     * \lang english
     * @brief Compares two maps for inequality.
     *
     * @param left The left map
     * @param right The right map
     * @return true if the maps are not equal
     *
     * \lang simp-chinese
     * @brief 判断两个 map 是否不相等。
     *
     * @param left 左侧 map
     * @param right 右侧 map
     * @return 若两个 map 不相等则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator!=(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Compares two maps lexicographically, key before mapped value.
     *
     * @param left The left map
     * @param right The right map
     * @return true if left is lexicographically less than right
     *
     * \lang simp-chinese
     * @brief 按键、再按映射值的顺序对两个 map 做字典序比较。
     *
     * @param left 左侧 map
     * @param right 右侧 map
     * @return 若 left 字典序小于 right 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end() && rit != right.end(); ++lit, ++rit) {
            if (lit->first < rit->first) {
                return true;
            }
            if (rit->first < lit->first) {
                return false;
            }
            if (lit->second < rit->second) {
                return true;
            }
            if (rit->second < lit->second) {
                return false;
            }
        }
        return lit == left.end() && rit != right.end();
    }

    /**
     * \lang english
     * @brief Compares two maps: returns true if right is less than left.
     *
     * @param left The left map
     * @param right The right map
     * @return true if right is lexicographically less than left
     *
     * \lang simp-chinese
     * @brief 比较两个 map：若 right 小于 left 则返回 true。
     *
     * @param left 左侧 map
     * @param right 右侧 map
     * @return 若 right 字典序小于 left 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares two maps: returns true if right is not less than left.
     *
     * @param left The left map
     * @param right The right map
     * @return true if right is not lexicographically less than left
     *
     * \lang simp-chinese
     * @brief 比较两个 map：若 right 不小于 left 则返回 true。
     *
     * @param left 左侧 map
     * @param right 右侧 map
     * @return 若 right 字典序不小于 left 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<=(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares two maps: returns true if left is not less than right.
     *
     * @param left The left map
     * @param right The right map
     * @return true if left is not lexicographically less than right
     *
     * \lang simp-chinese
     * @brief 比较两个 map：若 left 不小于 right 则返回 true。
     *
     * @param left 左侧 map
     * @param right 右侧 map
     * @return 若 left 字典序不小于 right 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>=(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Compares two multimaps for equality.
     *
     * @param left The left multimap
     * @param right The right multimap
     * @return true if both multimaps have the same size and every corresponding key/value pair is equal
     *
     * \lang simp-chinese
     * @brief 判断两个 multimap 是否相等。
     *
     * @param left 左侧 multimap
     * @param right 右侧 multimap
     * @return 若两个 multimap 大小相同且每一对对应的键值对都相等则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator==(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        if (left.size() != right.size()) {
            return false;
        }
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end(); ++lit, ++rit) {
            if (lit->first != rit->first || lit->second != rit->second) {
                return false;
            }
        }
        return true;
    }

    /**
     * \lang english
     * @brief Compares two multimaps for inequality.
     *
     * @param left The left multimap
     * @param right The right multimap
     * @return true if the multimaps are not equal
     *
     * \lang simp-chinese
     * @brief 判断两个 multimap 是否不相等。
     *
     * @param left 左侧 multimap
     * @param right 右侧 multimap
     * @return 若两个 multimap 不相等则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator!=(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Compares two multimaps lexicographically, key before mapped value.
     *
     * @param left The left multimap
     * @param right The right multimap
     * @return true if left is lexicographically less than right
     *
     * \lang simp-chinese
     * @brief 按键、再按映射值的顺序对两个 multimap 做字典序比较。
     *
     * @param left 左侧 multimap
     * @param right 右侧 multimap
     * @return 若 left 字典序小于 right 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end() && rit != right.end(); ++lit, ++rit) {
            if (lit->first < rit->first) {
                return true;
            }
            if (rit->first < lit->first) {
                return false;
            }
            if (lit->second < rit->second) {
                return true;
            }
            if (rit->second < lit->second) {
                return false;
            }
        }
        return lit == left.end() && rit != right.end();
    }

    /**
     * \lang english
     * @brief Compares two multimaps: returns true if right is less than left.
     *
     * @param left The left multimap
     * @param right The right multimap
     * @return true if right is lexicographically less than left
     *
     * \lang simp-chinese
     * @brief 比较两个 multimap：若 right 小于 left 则返回 true。
     *
     * @param left 左侧 multimap
     * @param right 右侧 multimap
     * @return 若 right 字典序小于 left 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares two multimaps: returns true if right is not less than left.
     *
     * @param left The left multimap
     * @param right The right multimap
     * @return true if right is not lexicographically less than left
     *
     * \lang simp-chinese
     * @brief 比较两个 multimap：若 right 不小于 left 则返回 true。
     *
     * @param left 左侧 multimap
     * @param right 右侧 multimap
     * @return 若 right 字典序不小于 left 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<=(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares two multimaps: returns true if left is not less than right.
     *
     * @param left The left multimap
     * @param right The right multimap
     * @return true if left is not lexicographically less than right
     *
     * \lang simp-chinese
     * @brief 比较两个 multimap：若 left 不小于 right 则返回 true。
     *
     * @param left 左侧 multimap
     * @param right 右侧 multimap
     * @return 若 left 字典序不小于 right 则返回 true
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>=(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Swaps the contents of two maps.
     *
     * @param left The first map
     * @param right The second map
     *
     * \lang simp-chinese
     * @brief 交换两个 map 的内容。
     *
     * @param left 第一个 map
     * @param right 第二个 map
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    void swap(map<Key, Mapped, Compare, Alloc> &left, map<Key, Mapped, Compare, Alloc> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }

    /**
     * \lang english
     * @brief Swaps the contents of two multimaps.
     *
     * @param left The first multimap
     * @param right The second multimap
     *
     * \lang simp-chinese
     * @brief 交换两个 multimap 的内容。
     *
     * @param left 第一个 multimap
     * @param right 第二个 multimap
     */
    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    void swap(multimap<Key, Mapped, Compare, Alloc> &left,
              multimap<Key, Mapped, Compare, Alloc> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

namespace rainy::collections {
    using foundation::collections::map;
    using foundation::collections::multimap;
}

namespace std { // NOLINT
    template <typename Key, typename Mapped, typename Compare, typename Allocator, typename Alloc>
    struct uses_allocator<rainy::foundation::collections::map<Key, Mapped, Compare, Allocator>, Alloc> :
        rainy::type_traits::helper::true_type {}; // NOLINT

    template <typename Key, typename Mapped, typename Compare, typename Allocator, typename Alloc>
    struct uses_allocator<rainy::foundation::collections::multimap<Key, Mapped, Compare, Allocator>, Alloc> :
        rainy::type_traits::helper::true_type {}; // NOLINT
}

#endif
