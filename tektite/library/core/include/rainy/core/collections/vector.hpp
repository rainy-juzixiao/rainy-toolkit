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
#ifndef RAINY_CORE_COLLECTIONS_VECTOR_HPP
#define RAINY_CORE_COLLECTIONS_VECTOR_HPP
#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <rainy/core/container/compressed_pair.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/utility/reverse_iterator.hpp>

namespace rainy::core::collections {
    /**
     * \lang english
     * @brief Dynamic array container with automatic memory management.
     *
     * Provides contiguous storage of elements, automatic reallocation,
     * and amortized constant-time push_back at the end.
     *
     * @tparam Ty The element type
     * @tparam Alloc The allocator type
     *
     * \lang simp-chinese
     * @brief 具有自动内存管理的动态数组容器。
     *
     * 提供元素的连续存储、自动重新分配以及末尾均摊常数时间的push_back。
     *
     * @tparam Ty 元素类型
     * @tparam Alloc 分配器类型
     */
    template <typename Ty, typename Alloc = core::memory::allocator<Ty>>
    class vector {
    public:
        /**
         * \lang english
         * @brief The element type.
         *
         * \lang simp-chinese
         * @brief 元素类型。
         */
        using value_type = Ty;

        /**
         * \lang english
         * @brief The allocator type used to acquire and release memory.
         *
         * \lang simp-chinese
         * @brief 用于获取和释放内存的分配器类型。
         */
        using allocator_type = Alloc;

        /**
         * \lang english
         * @brief Pointer to an element.
         *
         * \lang simp-chinese
         * @brief 指向元素的指针。
         */
        using pointer = Ty *;

        /**
         * \lang english
         * @brief Const pointer to an element.
         *
         * \lang simp-chinese
         * @brief 指向元素的常量指针。
         */
        using const_pointer = const Ty *;

        /**
         * \lang english
         * @brief Reference to an element.
         *
         * \lang simp-chinese
         * @brief 元素的引用。
         */
        using reference = value_type &;

        /**
         * \lang english
         * @brief Const reference to an element.
         *
         * \lang simp-chinese
         * @brief 元素的常量引用。
         */
        using const_reference = const value_type &;

        /**
         * \lang english
         * @brief Unsigned integer type used to represent sizes.
         *
         * \lang simp-chinese
         * @brief 用于表示大小的无符号整数类型。
         */
        using size_type = std::size_t;

        /**
         * \lang english
         * @brief Signed integer type used to represent distances between iterators.
         *
         * \lang simp-chinese
         * @brief 用于表示迭代器之间距离的有符号整数类型。
         */
        using difference_type = std::ptrdiff_t;

        /**
         * \lang english
         * @brief Random access iterator type.
         *
         * \lang simp-chinese
         * @brief 随机访问迭代器类型。
         */
        using iterator = Ty *;

        /**
         * \lang english
         * @brief Const random access iterator type.
         *
         * \lang simp-chinese
         * @brief 常量随机访问迭代器类型。
         */
        using const_iterator = const Ty *;

        /**
         * \lang english
         * @brief Reverse iterator type.
         *
         * \lang simp-chinese
         * @brief 反向迭代器类型。
         */
        using reverse_iterator = utility::reverse_iterator<iterator>;

        /**
         * \lang english
         * @brief Const reverse iterator type.
         *
         * \lang simp-chinese
         * @brief 常量反向迭代器类型。
         */
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be a object");
        static_assert(type_traits::properties::is_complete_v<Ty>, "Ty require that Ty must a complete type");
        static_assert(type_traits::properties::is_destructible_v<Ty>, "Ty must be destructible");
        static_assert(!type_traits::properties::is_abstract_v<Ty>, "Ty cannot be a abstract type");
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");

        /**
         * \lang english
         * @brief Static constant for npos (not found) value.
         *
         * \lang simp-chinese
         * @brief 静态常量 npos（未找到）值。
         */
        static constexpr size_type npos = static_cast<size_type>(-1);

        /**
         * \lang english
         * @brief Default constructor. Constructs an empty vector.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。构造一个空向量。
         */
        RAINY_CONSTEXPR20 vector() noexcept(noexcept(allocator_type())) : vector(allocator_type()) {
        }

        /**
         * \lang english
         * @brief Constructs an empty vector with the given allocator.
         *
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定的分配器构造一个空向量。
         *
         * @param alloc 要使用的分配器
         */
        RAINY_CONSTEXPR20 explicit vector(const allocator_type &alloc) noexcept : pair(alloc, {}) {
        }

        /**
         * \lang english
         * @brief Constructs a vector with count default-inserted elements.
         *
         * @param count The number of elements
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 构造包含count个默认插入元素的向量。
         *
         * @param count 元素数量
         * @param alloc 要使用的分配器
         */
        RAINY_CONSTEXPR20 explicit vector(size_type count, const allocator_type &alloc = allocator_type()) : pair(alloc, {}) {
            if (count == 0) {
                return;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            object.start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (size_type i = 0; i < count; ++i) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish);
                ++object.finish;
            }
        }

        /**
         * \lang english
         * @brief Constructs a vector with count copies of value.
         *
         * @param count The number of elements
         * @param value The value to copy
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 构造包含value的count个副本的向量。
         *
         * @param count 元素数量
         * @param value 要拷贝的值
         * @param alloc 要使用的分配器
         */
        RAINY_CONSTEXPR20 vector(size_type count, const_reference &value, const allocator_type &alloc = allocator_type()) :
            pair(alloc, {}) {
            if (count == 0) {
                return;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            object.start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (size_type i = 0; i < count; ++i) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, value);
                ++object.finish;
            }
        }

        /**
         * \lang english
         * @brief Constructs a vector from the range [first, last).
         *
         * @tparam InputIter Input iterator type
         * @param first Iterator to the beginning of the range
         * @param last Iterator to the end of the range
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从范围[first, last)构造向量。
         *
         * @tparam InputIter 输入迭代器类型
         * @param first 指向范围起始的迭代器
         * @param last 指向范围末尾的迭代器
         * @param alloc 要使用的分配器
         */
        template <typename InputIter,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIter>, int> = 0>
        RAINY_CONSTEXPR20 vector(InputIter first, InputIter last, const allocator_type &alloc = allocator_type()) : pair(alloc, {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            rainy_const count = static_cast<size_type>(utility::distance(first, last));
            if (count == 0) {
                return;
            }
            object.start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (auto it = first; it != last; ++it) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                ++object.finish;
            }
        }

        /**
         * \lang english
         * @brief Copy constructor. Copies the elements of right.
         *
         * @param right The vector to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。拷贝right中的元素。
         *
         * @param right 要拷贝的源向量
         */
        RAINY_CONSTEXPR20 vector(const vector &right) :
            pair(core::memory::allocator_traits<allocator_type>::select_on_container_copy_construction(right.get_allocator()), {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type count = right.size();
            if (rainy_unlikely(count == 0)) {
                return;
            }
            object.start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (auto it = right.begin(); it != right.end(); ++it) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                ++object.finish;
            }
        }

        /**
         * \lang english
         * @brief Move constructor. Steals the resources of right.
         *
         * @param right The vector to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。窃取right的资源。
         *
         * @param right 要移动的源向量
         */
        RAINY_CONSTEXPR20 vector(vector &&right) noexcept : pair(utility::move(right.get_allocator()), right.vec_object()) {
            right.vec_object() = {};
        }

        /**
         * \lang english
         * @brief Copy constructor with a different allocator.
         *
         * @param right The vector to copy from
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用不同分配器的拷贝构造函数。
         *
         * @param right 要拷贝的源向量
         * @param alloc 要使用的分配器
         */
        RAINY_CONSTEXPR20 vector(const vector &right, const type_traits::helper::identity_t<allocator_type> &alloc) : pair(alloc, {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type count = right.size();
            if (rainy_unlikely(count == 0)) {
                return;
            }
            object.start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (auto it = right.begin(); it != right.end(); ++it) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                ++object.finish;
            }
        }

        /**
         * \lang english
         * @brief Move constructor with a different allocator.
         *
         * If the allocators compare equal, the resources are stolen; otherwise the elements are moved one by one.
         *
         * @param right The vector to move from
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用不同分配器的移动构造函数。
         *
         * 若分配器相等则直接窃取资源；否则逐元素移动构造。
         *
         * @param right 要移动的源向量
         * @param alloc 要使用的分配器
         */
        RAINY_CONSTEXPR20 vector(vector &&right, const type_traits::helper::identity_t<allocator_type> &alloc) : pair(alloc, {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (allocator == right.get_allocator()) { // 相同，那就直接窃取资源
                object = right.vec_object();
                right.vec_object() = {};
            } else {
                const size_type count = right.size(); // 分配器不同，我们必须逐元素 move 构造
                if (count == 0) {
                    return;
                }
                object.start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
                object.finish = object.start;
                object.end_of_storage = object.start + count;
                for (auto it = right.begin(); it != right.end(); ++it) {
                    core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, utility::move(*it));
                    ++object.finish;
                }
            }
        }

        /**
         * \lang english
         * @brief Constructs a vector from an initializer list.
         *
         * @param ilist The initializer list
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从初始化列表构造向量。
         *
         * @param ilist 初始化列表
         * @param alloc 要使用的分配器
         */
        RAINY_CONSTEXPR20 vector(std::initializer_list<value_type> ilist, const allocator_type &alloc = allocator_type()) :
            vector(ilist.begin(), ilist.end(), alloc) {
        }

        /**
         * \lang english
         * @brief Destructor. Destroys all elements and releases the storage.
         *
         * \lang simp-chinese
         * @brief 析构函数。销毁所有元素并释放存储空间。
         */
        RAINY_CONSTEXPR20 ~vector() {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (object.start) {
                if constexpr (!type_traits::properties::is_trivially_destructible_v<value_type>) {
                    for (pointer p = object.start; p != object.finish; ++p) {
                        core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                    }
                }
                core::memory::allocator_traits<allocator_type>::deallocate(
                    allocator, object.start, static_cast<size_type>(object.end_of_storage - object.start));
                object = {};
            }
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @param right The vector to copy from
         * @return Reference to this vector
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @param right 要拷贝的源向量
         * @return 返回本向量的引用
         */
        RAINY_CONSTEXPR20 vector &operator=(const vector &right) {
            if (this == &right) {
                return *this;
            }
            auto &allocator = get_al();
            auto &rim = right.vec_object();
            if constexpr (core::memory::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
                if (allocator != right.get_allocator()) {
                    clear();
                    core::memory::allocator_traits<allocator_type>::deallocate(
                        allocator, vec_object().start, static_cast<size_type>(vec_object().end_of_storage - vec_object().start));
                    vec_object() = {};
                    allocator = right.get_allocator();
                }
            }
            const size_type new_size = right.size();
            if (new_size > capacity()) {
                // 重新分配
                vector tmp(right, allocator);
                swap(tmp);
                return *this;
            }
            auto &object = vec_object();
            // 覆盖已有元素
            const size_type cur_size = size();
            if (new_size <= cur_size) {
                core::algorithm::copy(rim.start, rim.start + new_size, object.start); // copy 覆盖前 new_size 个，然后销毁多余的
                for (pointer p = object.start + new_size; p != object.finish; ++p) {
                    core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                }
            } else {
                core::algorithm::copy(rim.start, rim.start + cur_size, object.start); // copy 覆盖已有的，然后 construct 新增的
                for (pointer p = rim.start + cur_size; p != rim.finish; ++p) {
                    core::memory::allocator_traits<allocator_type>::construct(allocator, object.start + (p - rim.start), *p);
                }
            }
            object.finish = object.start + new_size;
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right The vector to move from
         * @return Reference to this vector
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 要移动的源向量
         * @return 返回本向量的引用
         */
        RAINY_CONSTEXPR20 vector &operator=(vector &&right) noexcept(
            core::memory::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
            core::memory::allocator_traits<allocator_type>::is_always_equal::value) {
            if (rainy_unlikely(this == &right)) {
                return *this;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            for (pointer p = object.start; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            if (object.start) {
                core::memory::allocator_traits<allocator_type>::deallocate(
                    allocator, object.start, static_cast<size_type>(object.end_of_storage - object.start));
            }
            if constexpr (core::memory::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
                allocator = utility::move(right.get_al());
            }
            object = right.vec_object();
            right.vec_object() = {};
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns the elements of an initializer list to the vector.
         *
         * @param ilist The initializer list
         * @return Reference to this vector
         *
         * \lang simp-chinese
         * @brief 将初始化列表的元素赋给向量。
         *
         * @param ilist 初始化列表
         * @return 返回本向量的引用
         */
        RAINY_CONSTEXPR20 vector &operator=(std::initializer_list<value_type> ilist) {
            assign(ilist.begin(), ilist.end());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with the range [first, last).
         *
         * @tparam InputIter Input iterator type
         * @param first Iterator to the beginning of the range
         * @param last Iterator to the end of the range
         *
         * \lang simp-chinese
         * @brief 以范围[first, last)的内容替换当前内容。
         *
         * @tparam InputIter 输入迭代器类型
         * @param first 指向范围起始的迭代器
         * @param last 指向范围末尾的迭代器
         */
        template <typename InputIter,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIter>, int> = 0>
        RAINY_CONSTEXPR20 void assign(InputIter first, InputIter last) {
            rainy_const count = static_cast<size_type>(utility::distance(first, last));
            auto &allocator = get_al();
            auto &object = vec_object();
            if (count > capacity()) {
                vector tmp(first, last, allocator);
                swap(tmp);
                return;
            }
            if (const size_type cur = size(); count <= cur) {
                auto new_finish = core::algorithm::copy(first, last, object.start);
                for (pointer p = new_finish; p != object.finish; ++p) {
                    core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = new_finish;
            } else {
                auto mid = first;
                utility::advance(mid, cur);
                core::algorithm::copy(first, mid, object.start);
                for (auto it = mid; it != last; ++it) {
                    core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                    ++object.finish;
                }
            }
        }

        /**
         * \lang english
         * @brief Replaces the contents with count copies of elem.
         *
         * @param count The number of elements
         * @param elem The value to fill with
         *
         * \lang simp-chinese
         * @brief 以elem的count个副本替换当前内容。
         *
         * @param count 元素数量
         * @param elem 用于填充的值
         */
        RAINY_CONSTEXPR20 void assign(size_type count, const_reference &elem) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (count > capacity()) {
                vector tmp(count, elem, allocator);
                swap(tmp);
                return;
            }
            if (const size_type cur = size(); count <= cur) {
                core::algorithm::fill_n(object.start, count, elem);
                for (pointer p = object.start + count; p != object.finish; ++p) {
                    core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + count;
            } else {
                core::algorithm::fill_n(object.start, cur, elem);
                for (size_type i = cur; i < count; ++i) {
                    core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, elem);
                    ++object.finish;
                }
            }
        }

        /**
         * \lang english
         * @brief Replaces the contents with the elements of an initializer list.
         *
         * @param ilist The initializer list
         *
         * \lang simp-chinese
         * @brief 以初始化列表的元素替换当前内容。
         *
         * @param ilist 初始化列表
         */
        RAINY_CONSTEXPR20 void assign(std::initializer_list<value_type> ilist) {
            assign(ilist.begin(), ilist.end());
        }

        /**
         * \lang english
         * @brief Returns the associated allocator.
         *
         * @return The allocator
         *
         * \lang simp-chinese
         * @brief 返回关联的分配器。
         *
         * @return 分配器
         */
        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return pair.get_first();
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
        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return vec_object().start;
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
        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return vec_object().start;
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
        RAINY_CONSTEXPR20 iterator end() noexcept {
            return vec_object().finish;
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
        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return vec_object().finish;
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
        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
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
        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
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
        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
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
        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
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
        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return begin();
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
        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return end();
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
        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() const noexcept {
            return rbegin();
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
        RAINY_CONSTEXPR20 const_reverse_iterator crend() const noexcept {
            return rend();
        }

        /**
         * \lang english
         * @brief Checks whether the vector is empty.
         *
         * @return true if the vector contains no elements, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查向量是否为空。
         *
         * @return 如果向量不包含任何元素则为true，否则为false
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 bool empty() const noexcept {
            return vec_object().start == vec_object().finish;
        }

        /**
         * \lang english
         * @brief Returns the number of elements.
         *
         * @return The number of elements
         *
         * \lang simp-chinese
         * @brief 返回元素数量。
         *
         * @return 元素数量
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type size() const noexcept {
            return static_cast<size_type>(vec_object().finish - vec_object().start);
        }

        /**
         * \lang english
         * @brief Returns the maximum possible number of elements.
         *
         * @return The maximum number of elements
         *
         * \lang simp-chinese
         * @brief 返回元素的最大可能数量。
         *
         * @return 最大元素数量
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return core::memory::allocator_traits<allocator_type>::max_size(get_allocator());
        }

        /**
         * \lang english
         * @brief Returns the number of elements that can be held without reallocation.
         *
         * @return The current capacity
         *
         * \lang simp-chinese
         * @brief 返回不重新分配即可容纳的元素数量。
         *
         * @return 当前容量
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            return static_cast<size_type>(vec_object().end_of_storage - vec_object().start);
        }

        /**
         * \lang english
         * @brief Resizes the vector to contain new_size elements.
         *
         * @param new_size The new size
         *
         * \lang simp-chinese
         * @brief 将向量大小调整为new_size。
         *
         * @param new_size 新的大小
         */
        RAINY_CONSTEXPR20 void resize(size_type new_size) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (const size_type cur = size(); new_size < cur) {
                for (pointer p = object.start + new_size; p != object.finish; ++p) {
                    core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + new_size;
            } else if (new_size > cur) {
                reserve(new_size);
                for (size_type i = cur; i < new_size; ++i) {
                    core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish);
                    ++object.finish;
                }
            }
        }

        /**
         * \lang english
         * @brief Resizes the vector to contain new_size copies of elem.
         *
         * @param new_size The new size
         * @param elem The value to fill with
         *
         * \lang simp-chinese
         * @brief 将向量大小调整为new_size，并以elem填充新增元素。
         *
         * @param new_size 新的大小
         * @param elem 用于填充的值
         */
        RAINY_CONSTEXPR20 void resize(size_type new_size, const_reference elem) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (const size_type cur = size(); new_size < cur) {
                for (pointer p = object.start + new_size; p != object.finish; ++p) {
                    core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + new_size;
            } else if (new_size > cur) {
                reserve(new_size);
                for (size_type i = cur; i < new_size; ++i) {
                    core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, elem);
                    ++object.finish;
                }
            }
        }

        /**
         * \lang english
         * @brief Reserves storage for at least count elements without reallocation.
         *
         * @param count The number of elements to reserve capacity for
         *
         * \lang simp-chinese
         * @brief 为至少count个元素预留存储空间，避免重新分配。
         *
         * @param count 要预留容量的元素数量
         */
        RAINY_CONSTEXPR20 void reserve(size_type count) {
            if (count <= capacity()) {
                return;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            pointer new_start = core::memory::allocator_traits<allocator_type>::allocate(allocator, count);
            struct guard {
                allocator_type &alloc;
                pointer ptr;
                pointer finish;
                size_type capacity;

                RAINY_CONSTEXPR20 guard(allocator_type &a, pointer p, const size_type cap) :
                    alloc(a), ptr(p), finish(p), capacity(cap) { // NOLINT
                }

                RAINY_CONSTEXPR20 ~guard() {
                    if (ptr) {
                        for (pointer p = ptr; p != finish; ++p) {
                            core::memory::allocator_traits<allocator_type>::destroy(alloc, p);
                        }
                        core::memory::allocator_traits<allocator_type>::deallocate(alloc, ptr, capacity);
                    }
                }

                RAINY_CONSTEXPR20 void release() {
                    ptr = nullptr;
                }
            } guard(allocator, new_start, count);
            pointer new_finish = new_start;
            for (pointer p = object.start; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, new_finish, utility::move_if_noexcept(*p));
                ++new_finish;
                guard.finish = new_finish;
            }
            for (pointer p = object.start; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            if (object.start) {
                core::memory::allocator_traits<allocator_type>::deallocate(
                    allocator, object.start, static_cast<size_type>(object.end_of_storage - object.start));
            }
            object.start = new_start;
            object.finish = new_finish;
            object.end_of_storage = new_start + count;
            guard.release();
        }

        /**
         * \lang english
         * @brief Removes all elements from the vector.
         *
         * \lang simp-chinese
         * @brief 移除向量中的所有元素。
         */
        RAINY_CONSTEXPR20 void clear() noexcept {
            auto &allocator = get_al();
            auto &object = vec_object();
            for (pointer p = object.start; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            object.finish = object.start;
        }

        /**
         * \lang english
         * @brief Reduces capacity to match the current size.
         *
         * \lang simp-chinese
         * @brief 将容量缩减为与当前大小一致。
         */
        RAINY_CONSTEXPR20 void shrink_to_fit() {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type cur_size = size();
            if (cur_size == capacity()) {
                return;
            }
            if (cur_size == 0) {
                if (object.start) {
                    core::memory::allocator_traits<allocator_type>::deallocate(
                        allocator, object.start, static_cast<size_type>(object.end_of_storage - object.start));
                }
                object = {};
                return;
            }
            pointer new_start = core::memory::allocator_traits<allocator_type>::allocate(allocator, cur_size);
            pointer new_finish = new_start;
            for (pointer p = object.start; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::construct(allocator, new_finish, utility::move_if_noexcept(*p));
                ++new_finish;
            }
            for (pointer p = object.start; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            core::memory::allocator_traits<allocator_type>::deallocate(allocator, object.start,
                                                                       static_cast<size_type>(object.end_of_storage - object.start));
            object.start = new_start;
            object.finish = new_finish;
            object.end_of_storage = new_start + cur_size;
        }

        /**
         * \lang english
         * @brief Accesses the element at the specified index without bounds checking.
         *
         * @param index The index of the element
         * @return Reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定索引处的元素，不进行边界检查。
         *
         * @param index 元素索引
         * @return 元素的引用
         */
        RAINY_CONSTEXPR20 reference operator[](size_type index) {
            return vec_object().start[index];
        }

        /**
         * \lang english
         * @brief Accesses the element at the specified index without bounds checking.
         *
         * @param index The index of the element
         * @return Const reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定索引处的元素，不进行边界检查。
         *
         * @param index 元素索引
         * @return 元素的常量引用
         */
        RAINY_CONSTEXPR20 const_reference operator[](size_type index) const {
            return vec_object().start[index];
        }

        /**
         * \lang english
         * @brief Accesses the element at the specified index with bounds checking.
         *
         * Throws an out-of-range exception if index is invalid.
         *
         * @param index The index of the element
         * @return Const reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定索引处的元素，并进行边界检查。
         *
         * 若索引无效则抛出越界异常。
         *
         * @param index 元素索引
         * @return 元素的常量引用
         */
        RAINY_CONSTEXPR20 const_reference at(size_type index) const {
            if (index >= size()) {
                core::implements::throw_exception_out_of_range("vector::at — index out of range");
            }
            return vec_object().start[index];
        }

        /**
         * \lang english
         * @brief Accesses the element at the specified index with bounds checking.
         *
         * Throws an out-of-range exception if index is invalid.
         *
         * @param index The index of the element
         * @return Reference to the element
         *
         * \lang simp-chinese
         * @brief 访问指定索引处的元素，并进行边界检查。
         *
         * 若索引无效则抛出越界异常。
         *
         * @param index 元素索引
         * @return 元素的引用
         */
        RAINY_CONSTEXPR20 reference at(size_type index) {
            if (index >= size()) {
                core::implements::throw_exception_out_of_range("vector::at — index out of range");
            }
            return vec_object().start[index];
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
        RAINY_CONSTEXPR20 reference front() {
            return *vec_object().start;
        }

        /**
         * \lang english
         * @brief Accesses the first element.
         *
         * @return Const reference to the first element
         *
         * \lang simp-chinese
         * @brief 访问第一个元素。
         *
         * @return 第一个元素的常量引用
         */
        RAINY_CONSTEXPR20 const_reference front() const {
            return *vec_object().start;
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
        RAINY_CONSTEXPR20 reference back() {
            return *(vec_object().finish - 1);
        }

        /**
         * \lang english
         * @brief Accesses the last element.
         *
         * @return Const reference to the last element
         *
         * \lang simp-chinese
         * @brief 访问最后一个元素。
         *
         * @return 最后一个元素的常量引用
         */
        RAINY_CONSTEXPR20 const_reference back() const {
            return *(vec_object().finish - 1);
        }

        /**
         * \lang english
         * @brief Returns a pointer to the underlying element storage.
         *
         * @return Pointer to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向底层元素存储的指针。
         *
         * @return 指向第一个元素的指针
         */
        RAINY_CONSTEXPR20 pointer data() noexcept {
            return vec_object().start;
        }

        /**
         * \lang english
         * @brief Returns a const pointer to the underlying element storage.
         *
         * @return Const pointer to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向底层元素存储的常量指针。
         *
         * @return 指向第一个元素的常量指针
         */
        RAINY_CONSTEXPR20 const_pointer data() const noexcept {
            return vec_object().start;
        }

        /**
         * \lang english
         * @brief Appends a new element constructed in place from the given arguments.
         *
         * @tparam Args Argument pack types
         * @param args Arguments to forward to the element constructor
         * @return Reference to the appended element
         *
         * \lang simp-chinese
         * @brief 就地构造并追加一个新元素。
         *
         * @tparam Args 参数包类型
         * @param args 转发给元素构造函数的实参
         * @return 追加元素的引用
         */
        template <typename... Args>
        RAINY_CONSTEXPR20 reference emplace_back(Args &&...args) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (object.finish == object.end_of_storage) {
                const size_type old_cap = capacity();
                const size_type new_cap = old_cap == 0 ? 1 : old_cap * 2;
                reserve(new_cap);
            }
            core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, utility::forward<Args>(args)...);
            return *object.finish++;
        }

        /**
         * \lang english
         * @brief Appends a copy of the given element.
         *
         * @param right The element to copy
         *
         * \lang simp-chinese
         * @brief 追加给定元素的副本。
         *
         * @param right 要拷贝的元素
         */
        RAINY_CONSTEXPR20 void push_back(const_reference right) {
            emplace_back(right);
        }

        /**
         * \lang english
         * @brief Appends the given element by moving it.
         *
         * @param right The element to move
         *
         * \lang simp-chinese
         * @brief 以移动方式追加给定元素。
         *
         * @param right 要移动的元素
         */
        RAINY_CONSTEXPR20 void push_back(value_type &&right) {
            emplace_back(utility::move(right));
        }

        /**
         * \lang english
         * @brief Removes the last element.
         *
         * \lang simp-chinese
         * @brief 移除最后一个元素。
         */
        RAINY_CONSTEXPR20 void pop_back() {
            auto &allocator = get_al();
            auto &object = vec_object();
            --object.finish;
            core::memory::allocator_traits<allocator_type>::destroy(allocator, object.finish);
        }

        /**
         * \lang english
         * @brief Inserts a new element constructed in place at the given position.
         *
         * @tparam Args Argument pack types
         * @param position Iterator to the insertion position
         * @param args Arguments to forward to the element constructor
         * @return Iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 在指定位置就地构造并插入一个新元素。
         *
         * @tparam Args 参数包类型
         * @param position 指向插入位置的迭代器
         * @param args 转发给元素构造函数的实参
         * @return 指向插入元素的迭代器
         */
        template <typename... Args>
        RAINY_CONSTEXPR20 iterator emplace(const_iterator position, Args &&...args) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const auto offset = static_cast<size_type>(position - object.start);
            if (object.finish == object.end_of_storage) {
                const size_type old_cap = capacity();
                const size_type new_cap = old_cap == 0 ? 1 : old_cap * 2;
                reserve(new_cap);
            }
            // reserve 可能使指针失效，重新算插入位置
            pointer pos = object.start + offset;
            if (pos != object.finish) {
                // 在末尾 construct 最后一个元素的移动拷贝，然后向后移位
                core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish,
                                                                          utility::move(*(object.finish - 1)));
                core::algorithm::move_backward(pos, object.finish - 1, object.finish);
                // 销毁旧元素后 placement-construct 新值
                core::memory::allocator_traits<allocator_type>::destroy(allocator, pos);
            }
            core::memory::allocator_traits<allocator_type>::construct(allocator, pos, utility::forward<Args>(args)...);
            ++object.finish;
            return pos;
        }

        /**
         * \lang english
         * @brief Inserts a copy of the given element at the specified position.
         *
         * @param position Iterator to the insertion position
         * @param right The element to insert
         * @return Iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 在指定位置插入给定元素的副本。
         *
         * @param position 指向插入位置的迭代器
         * @param right 要插入的元素
         * @return 指向插入元素的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, const_reference right) {
            return emplace(position, right);
        }

        /**
         * \lang english
         * @brief Inserts the given element at the specified position by moving it.
         *
         * @param position Iterator to the insertion position
         * @param right The element to move
         * @return Iterator to the inserted element
         *
         * \lang simp-chinese
         * @brief 在指定位置以移动方式插入给定元素。
         *
         * @param position 指向插入位置的迭代器
         * @param right 要移动的元素
         * @return 指向插入元素的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, value_type &&right) {
            return emplace(position, utility::move(right));
        }

        /**
         * \lang english
         * @brief Inserts count copies of the given element at the specified position.
         *
         * @param position Iterator to the insertion position
         * @param count The number of elements to insert
         * @param right The value to copy
         * @return Iterator to the first inserted element
         *
         * \lang simp-chinese
         * @brief 在指定位置插入给定元素的count个副本。
         *
         * @param position 指向插入位置的迭代器
         * @param count 要插入的元素数量
         * @param right 要拷贝的值
         * @return 指向第一个插入元素的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, size_type count, const_reference right) {
            if (count == 0) {
                return const_cast<iterator>(position);
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                auto &object = vec_object();
                const auto offset = static_cast<size_type>(position - object.start);
                const size_type cur_size = size();
                const size_type new_size = cur_size + count;
                vector temp;
                temp.reserve(new_size + capacity());
                auto &temp_obj = temp.vec_object();
                auto &temp_alloc = temp.get_al();
                for (size_type i = 0; i < offset; ++i) {
                    core::memory::allocator_traits<allocator_type>::construct(temp_alloc, temp_obj.finish, object.start[i]);
                    ++temp_obj.finish;
                }
                for (size_type i = 0; i < count; ++i) {
                    core::memory::allocator_traits<allocator_type>::construct(temp_alloc, temp_obj.finish, right);
                    ++temp_obj.finish;
                }
                for (size_type i = offset; i < cur_size; ++i) {
                    core::memory::allocator_traits<allocator_type>::construct(temp_alloc, temp_obj.finish, object.start[i]);
                    ++temp_obj.finish;
                }
                this->swap(temp);
                return object.start + offset;
            }
#endif
            {
                auto &allocator = get_al();
                auto &object = vec_object();
                const auto offset = static_cast<size_type>(position - object.start);
                const size_type cur_size = size();
                if (cur_size + count > capacity()) {
                    const size_type new_cap = (core::max) (cur_size + count, capacity() * 2);
                    reserve(new_cap);
                }
                pointer pos = object.start + offset;
                if (const auto elems_after = static_cast<size_type>(object.finish - pos); elems_after > count) {
                    // 尾部有足够多的已有元素，先移动尾部 count 个到未初始化区域
                    pointer old_finish = object.finish;
                    for (pointer p = old_finish - count; p != old_finish; ++p) {
                        core::memory::allocator_traits<allocator_type>::construct(allocator, p + count, utility::move(*p));
                    }
                    object.finish += count;
                    core::algorithm::move_backward(pos, old_finish - count, old_finish);
                    core::algorithm::fill_n(pos, count, right);
                } else {
                    // count >= elems_after
                    pointer old_finish = object.finish;
                    // 先 construct 超出原 finish 的新元素
                    for (size_type i = elems_after; i < count; ++i) {
                        core::memory::allocator_traits<allocator_type>::construct(allocator, object.finish, right);
                        ++object.finish;
                    }
                    // 将原 [pos, old_finish) 移动到新末尾
                    for (pointer p = old_finish - 1; p >= pos; --p) {
                        core::memory::allocator_traits<allocator_type>::construct(allocator, p + count, utility::move(*p));
                        core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
                    }
                    object.finish = object.start + cur_size + count;
                    core::algorithm::fill_n(pos, elems_after, right);
                }
                return pos;
            }
        }

        /**
         * \lang english
         * @brief Inserts elements from the range [first, last) at the specified position.
         *
         * @tparam InputIter Input iterator type
         * @param position Iterator to the insertion position
         * @param first Iterator to the beginning of the range
         * @param last Iterator to the end of the range
         * @return Iterator to the first inserted element
         *
         * \lang simp-chinese
         * @brief 在指定位置插入范围[first, last)中的元素。
         *
         * @tparam InputIter 输入迭代器类型
         * @param position 指向插入位置的迭代器
         * @param first 指向范围起始的迭代器
         * @param last 指向范围末尾的迭代器
         * @return 指向第一个插入元素的迭代器
         */
        template <typename InputIter,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIter>, int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, InputIter first, InputIter last) {
            const auto offset = static_cast<size_type>(position - vec_object().start);
            for (auto it = first; it != last; ++it) {
                // 每次 emplace 后 position 可能因 reserve 失效，用 offset 重新定位
                emplace(vec_object().start + offset + static_cast<size_type>(it - first), *it);
            }
            return vec_object().start + offset;
        }

        /**
         * \lang english
         * @brief Inserts the elements of an initializer list at the specified position.
         *
         * @param position Iterator to the insertion position
         * @param ilist The initializer list
         * @return Iterator to the first inserted element
         *
         * \lang simp-chinese
         * @brief 在指定位置插入初始化列表中的元素。
         *
         * @param position 指向插入位置的迭代器
         * @param ilist 初始化列表
         * @return 指向第一个插入元素的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const const_iterator position, std::initializer_list<value_type> ilist) {
            return insert(position, ilist.begin(), ilist.end());
        }

        /**
         * \lang english
         * @brief Removes the element at the specified position.
         *
         * @param position Iterator to the element to remove
         * @return Iterator to the element following the removed one
         *
         * \lang simp-chinese
         * @brief 移除指定位置的元素。
         *
         * @param position 指向要移除元素的迭代器
         * @return 指向被移除元素之后元素的迭代器
         */
        RAINY_CONSTEXPR20 iterator erase(const_iterator position) {
            auto &allocator = get_al();
            auto &object = vec_object();
            auto pos = const_cast<pointer>(position);
            core::algorithm::move(pos + 1, object.finish, pos);
            --object.finish;
            core::memory::allocator_traits<allocator_type>::destroy(allocator, object.finish);
            return pos;
        }

        /**
         * \lang english
         * @brief Removes the elements in the range [first, last).
         *
         * @param first Iterator to the beginning of the range
         * @param last Iterator to the end of the range
         * @return Iterator to the element following the last removed one
         *
         * \lang simp-chinese
         * @brief 移除范围[first, last)中的元素。
         *
         * @param first 指向范围起始的迭代器
         * @param last 指向范围末尾的迭代器
         * @return 指向最后一个被移除元素之后元素的迭代器
         */
        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
            auto &allocator = get_al();
            auto &object = vec_object();
            auto f = const_cast<pointer>(first);
            auto l = const_cast<pointer>(last);
            pointer new_finish = core::algorithm::move(l, object.finish, f);
            for (pointer p = new_finish; p != object.finish; ++p) {
                core::memory::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            object.finish = new_finish;
            return f;
        }

        /**
         * \lang english
         * @brief Swaps the contents of this vector with right.
         *
         * @param right The vector to swap with
         *
         * \lang simp-chinese
         * @brief 将本向量的内容与right交换。
         *
         * @param right 要交换的向量
         */
        RAINY_CONSTEXPR20 void swap(vector &right) noexcept(
            core::memory::allocator_traits<allocator_type>::propagate_on_container_swap::value ||
            core::memory::allocator_traits<allocator_type>::is_always_equal::value) {
            using std::swap;
            if constexpr (core::memory::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
                swap(get_al(), right.get_al());
            }
            swap(vec_object(), right.vec_object());
        }

        /**
         * \lang english
         * @brief Equality comparison operator for vectors.
         *
         * @param left Left vector
         * @param right Right vector
         * @return true if all corresponding elements are equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 判断两个向量是否相等。
         *
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若两个向量元素一一对应且相等则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator==(const vector &left, const vector &right) noexcept -> bool {
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * \lang english
         * @brief Inequality comparison operator for vectors.
         *
         * @param left Left vector
         * @param right Right vector
         * @return true if the vectors are not equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 判断两个向量是否不相等。
         *
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若两个向量不相等则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator!=(const vector &left, const vector &right) noexcept -> bool {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Less-than comparison operator for vectors (lexicographical).
         *
         * @param left Left vector
         * @param right Right vector
         * @return true if left is lexicographically less than right, false otherwise.
         *
         * \lang simp-chinese
         * @brief 判断左侧向量是否小于右侧向量（字典序比较）。
         *
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量在字典序上小于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator<(const vector &left, const vector &right) noexcept -> bool {
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * \lang english
         * @brief Greater-than comparison operator for vectors (lexicographical).
         *
         * @param left Left vector
         * @param right Right vector
         * @return true if left is lexicographically greater than right, false otherwise.
         *
         * \lang simp-chinese
         * @brief 判断左侧向量是否大于右侧向量（字典序比较）。
         *
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量在字典序上大于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator>(const vector &left, const vector &right) noexcept -> bool {
            return right < left;
        }

        /**
         * \lang english
         * @brief Less-than-or-equal comparison operator for vectors (lexicographical).
         *
         * @param left Left vector
         * @param right Right vector
         * @return true if left is lexicographically less than or equal to right, false otherwise.
         *
         * \lang simp-chinese
         * @brief 判断左侧向量是否小于等于右侧向量（字典序比较）。
         *
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量小于或等于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator<=(const vector &left, const vector &right) noexcept -> bool {
            return !(left > right);
        }

        /**
         * \lang english
         * @brief Greater-than-or-equal comparison operator for vectors (lexicographical).
         *
         * @param left Left vector
         * @param right Right vector
         * @return true if left is lexicographically greater than or equal to right, false otherwise.
         *
         * \lang simp-chinese
         * @brief 判断左侧向量是否大于等于右侧向量（字典序比较）。
         *
         * @param left 左侧向量
         * @param right 右侧向量
         * @return 若左侧向量大于或等于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator>=(const vector &left, const vector &right) noexcept -> bool {
            return !(left < right);
        }

        /**
         * \lang english
         * @brief Swaps the contents of two vectors.
         *
         * @param left The first vector
         * @param right The second vector
         *
         * \lang simp-chinese
         * @brief 交换两个向量的内容。
         *
         * @param left 第一个向量
         * @param right 第二个向量
         */
        friend constexpr rain_fn swap(vector &left, vector &right) noexcept -> void {
            left.swap(right);
        }

        /**
         * \lang english
         * @brief Extracts a sub-range of the vector.
         *
         * @param begin_slice Start position (inclusive)
         * @param end_slice End position (exclusive)
         * @return Returns a new vector copied from the specified range;
         *          returns an empty vector if parameters are invalid.
         *
         * \lang simp-chinese
         * @brief 截取向量的子区间。
         *
         * @param begin_slice 起始位置（包含）
         * @param end_slice 结束位置（不包含）
         * @return 返回从指定区间复制的子向量；若参数非法则返回空向量。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector slice(const size_type begin_slice = 0, const size_type end_slice = npos) const {
            const auto sz = size();
            const auto end_slice1 = (core::min) (sz, end_slice);
            if (begin_slice >= sz || begin_slice == end_slice) {
                return vector(get_allocator());
            }
            const size_type count = end_slice1 - begin_slice;
            vector result(get_allocator());
            result.reserve(count);
            for (size_type i = 0; i < count; ++i) {
                result.emplace_back(this->vec_object().start[i + begin_slice]);
            }
            return result;
        }

        /**
         * \lang english
         * @brief Gets the first n elements from the left side of the vector.
         *
         * @param n Number of elements to extract
         * @return Returns a new vector containing the first n elements;
         *          returns an empty vector if n is out of range.
         *
         * \lang simp-chinese
         * @brief 获取向量左侧的若干元素。
         *
         * @param n 要截取的元素数量
         * @return 返回包含前 n 个元素的新向量；若 n 超出范围则返回空向量。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector left(size_type n) const {
            const size_type sz = size();
            if (n > sz || n == 0) {
                return vector(get_allocator());
            }
            return slice(0, n);
        }

        /**
         * \lang english
         * @brief Gets the last n elements from the right side of the vector.
         *
         * @param n Number of elements to extract
         * @return Returns a new vector containing the last n elements;
         *          returns an empty vector if n is out of range.
         *
         * \lang simp-chinese
         * @brief 获取向量右侧的若干元素。
         *
         * @param n 要截取的元素数量
         * @return 返回包含后 n 个元素的新向量；若 n 超出范围则返回空向量。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector right(size_type n) const {
            const size_type sz = size();
            if (n > sz or n == 0) {
                return vector(get_allocator());
            }
            return slice(sz - n, sz);
        }

        /**
         * \lang english
         * @brief Finds the index of a specified value in the vector.
         *
         * @tparam UTy Type of the value to find
         * @param value Value to find
         * @return Returns the index if a matching element is found, otherwise returns npos.
         *
         * \lang simp-chinese
         * @brief 查找指定值在向量中的索引。
         *
         * @tparam UTy 待查找的值的类型
         * @param value 要查找的值
         * @return 若找到匹配元素则返回其索引，否则返回 npos。
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy, value_type> &&
                                                            type_traits::properties::is_equal_comparable_v<value_type, UTy>,
                                                        int> = 0>
        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type index_of(UTy &&value) const noexcept {
            auto iter = core::algorithm::find(begin(), end(), utility::forward<UTy>(value));
            return iter == end() ? npos : static_cast<size_type>(iter - begin());
        }

        /**
         * \lang english
         * @brief Filters vector elements based on a predicate function.
         *
         * @tparam Pred Predicate function type, receives an element and returns a result convertible to bool
         * @param pred Predicate used for filtering elements
         * @return Returns a new vector containing elements that satisfy the predicate.
         *
         * \lang simp-chinese
         * @brief 根据谓词函数筛选向量元素。
         *
         * @tparam Pred 谓词函数类型，接收元素并返回可转换为 bool 的结果
         * @param pred 用于筛选元素的谓词
         * @return 返回一个新向量，包含满足谓词的元素。
         */
        template <typename Pred>
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector filter(Pred &&pred) const {
            vector result(get_allocator());
            const size_type cur_size = size();
            // 先粗略预分配
            result.reserve(cur_size);
            for (const auto &elem: *this) {
                if (utility::invoke(utility::forward<Pred>(pred), elem)) {
                    result.push_back(elem);
                }
            }
            result.shrink_to_fit();
            return result;
        }

        /**
         * \lang english
         * @brief Returns a reversed version of the vector.
         *
         * @return Returns a new vector whose elements are in reverse order of the current vector.
         *
         * \lang simp-chinese
         * @brief 返回向量的逆序版本。
         *
         * @return 返回一个新向量，其元素顺序与当前向量相反。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector reverse() const {
            vector result(get_allocator());
            const size_type sz = size();
            result.reserve(sz);
            for (auto it = rbegin(); it != rend(); ++it) {
                result.emplace_back(*it);
            }
            return result;
        }

        /**
         * \lang english
         * @brief Applies a mapping function to each element of the vector.
         *
         * @tparam Fx Mapping function type
         * @param func Mapping function
         * @return Returns a new vector whose elements are the results of applying the mapping function.
         *
         * \lang simp-chinese
         * @brief 对向量中的每个元素应用映射函数。
         *
         * @tparam Fx 映射函数类型
         * @param func 映射函数
         * @return 返回一个新向量，其元素为映射函数作用后的结果。
         */
        template <typename Fx, typename NewType = type_traits::properties::invoke_result_t<Fx, value_type>,
                  typename NewAlloc = core::memory::allocator_traits<allocator_type>::template rebind_alloc<NewType>>
        RAINY_NODISCARD RAINY_CONSTEXPR20 rain_fn map(Fx &&func) const -> vector<NewType, NewAlloc> {
            static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be a object");
            static_assert(type_traits::properties::is_complete_v<Ty>, "Ty require that Ty must a complete type");
            static_assert(type_traits::properties::is_destructible_v<Ty>, "Ty must be destructible");
            static_assert(!type_traits::properties::is_abstract_v<Ty>, "Ty cannot be a abstract type");
            static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");
            vector<NewType, NewAlloc> result(get_allocator());
            const size_type sz = size();
            result.reserve(sz);
            for (const auto &elem: *this) {
                result.push_back(utility::invoke(utility::forward<Fx>(func), elem));
            }
            return result;
        }

        /**
         * \lang english
         * @brief Folds vector elements using a specified initial value.
         *
         * @tparam Init Type of the initial value
         * @param init_value Initial value used for fold computation
         * @return Returns the result of the fold computation.
         *
         * \lang simp-chinese
         * @brief 使用指定初始值折叠向量元素。
         *
         * @tparam Init 初始值类型
         * @param init_value 用作折叠计算的初始值
         * @return 返回折叠计算的结果。
         */
        template <typename Init>
        RAINY_NODISCARD RAINY_CONSTEXPR20 Init fold(const Init &init_value) const {
            return core::accumulate(begin(), end(), init_value);
        }

        /**
         * \lang english
         * @brief Folds vector elements using a specified initial value and binary function.
         *
         * @tparam Init Type of the initial value
         * @tparam Fx Binary fold function type
         * @param init_value Initial value for fold computation
         * @param func Binary function used for folding
         * @return Returns the result of the fold computation.
         *
         * \lang simp-chinese
         * @brief 使用指定初始值和二元函数折叠向量元素。
         *
         * @tparam Init 初始值类型
         * @tparam Fx 二元折叠函数类型
         * @param init_value 折叠计算的初始值
         * @param func 用于折叠的二元函数
         * @return 返回折叠计算的结果。
         */
        template <typename Init, typename Fx>
        RAINY_NODISCARD RAINY_CONSTEXPR20 Init fold(const Init &init_value, Fx &&func) const {
            return core::accumulate(begin(), end(), init_value, utility::forward<Fx>(func));
        }

        /**
         * \lang english
         * @brief Concatenates two vectors.
         *
         * @param right Right vector to concatenate
         * @return Returns a new vector containing elements from both the left and right vectors.
         *
         * \lang simp-chinese
         * @brief 拼接两个向量。
         *
         * @param right 右侧待拼接的向量
         * @return 返回一个包含左侧与右侧元素的新向量。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector concat(const vector &right) const {
            vector result(get_allocator());
            const size_type total_size = size() + right.size();
            result.reserve(total_size);
            for (const auto &elem: *this) {
                result.push_back(elem);
            }
            for (const auto &elem: right) {
                result.push_back(elem);
            }
            return result;
        }

        /**
         * \lang english
         * @brief Concatenates a vector with a single element.
         *
         * @param elem Element to append
         * @return Returns a new vector containing elements from the original vector plus the new element.
         *
         * \lang simp-chinese
         * @brief 将向量与单个元素拼接。
         *
         * @param elem 要追加的元素
         * @return 返回一个包含原向量元素和新元素的新向量。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector concat(const value_type &elem) const {
            vector result(get_allocator());
            const size_type total_size = size() + 1;
            result.reserve(total_size);
            for (const auto &e: *this) {
                result.push_back(e);
            }
            result.push_back(elem);
            return result;
        }

        /**
         * \lang english
         * @brief Concatenates a vector with a single element (rvalue).
         *
         * @param elem Element to append (rvalue)
         * @return Returns a new vector containing elements from the original vector plus the new element.
         *
         * \lang simp-chinese
         * @brief 将向量与单个右值元素拼接。
         *
         * @param elem 要追加的右值元素
         * @return 返回一个包含原向量元素和新元素的新向量。
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 vector concat(value_type &&elem) const {
            vector result(get_allocator());
            const size_type total_size = size() + 1;
            result.reserve(total_size);
            for (const auto &e: *this) {
                result.push_back(e);
            }
            result.push_back(utility::move(elem));
            return result;
        }

    private:
        struct impl {
            Ty *start{};
            Ty *finish{};
            Ty *end_of_storage{};
        };

        constexpr impl &vec_object() noexcept {
            return pair.second;
        }

        constexpr const impl &vec_object() const noexcept {
            return pair.second;
        }

        constexpr allocator_type &get_al() noexcept {
            return pair.get_first();
        }

        container::compressed_pair<allocator_type, impl> pair;
    };

    template <typename InputIter,
              typename Allocator = core::memory::allocator<type_traits::extras::iterators::iter_value_t<InputIter>>>
    vector(InputIter, InputIter, Allocator = Allocator())
        -> vector<type_traits::extras::iterators::iter_value_t<InputIter>, Allocator>;
}

namespace rainy::collections {
    using rainy::core::collections::vector;
}


#endif
