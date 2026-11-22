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
#ifndef RAINY_FOUNDATION_COLLECTIONS_RING_BUFFER_HPP
#define RAINY_FOUNDATION_COLLECTIONS_RING_BUFFER_HPP

#include <cstddef>
#include <initializer_list>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/utility/reverse_iterator.hpp>
#include <rainy/foundation/collections/implements/ring_buffer/fwd.hpp>
#include <rainy/foundation/collections/implements/ring_buffer/iterator.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief A fixed-capacity circular buffer.
     *
     * @tparam T The element type.
     * @tparam Allocator The allocator type.
     * @tparam Policy The overflow policy.
     *
     * \lang simp-chinese
     * @brief 定长环形缓冲。
     *
     * @tparam T 元素类型。
     * @tparam Allocator 分配器类型。
     * @tparam Policy 溢出策略。
     */
    template <typename T, typename Allocator = core::memory::allocator<T>, overflow_policy Policy = overflow_policy::throw_on_full>
    class ring_buffer {
        using traits = core::memory::allocator_traits<Allocator>;

    public:
        /**
         * \lang english
         * @brief The element type.
         *
         * \lang simp-chinese
         * @brief 元素类型。
         */
        using value_type = T;

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
         * @brief The allocator type used to acquire and release memory.
         *
         * \lang simp-chinese
         * @brief 用于获取和释放内存的分配器类型。
         */
        using allocator_type = Allocator;

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
         * @brief Pointer to an element.
         *
         * \lang simp-chinese
         * @brief 指向元素的指针。
         */
        using pointer = value_type *;

        /**
         * \lang english
         * @brief Const pointer to an element.
         *
         * \lang simp-chinese
         * @brief 指向元素的常量指针。
         */
        using const_pointer = const value_type *;

        /**
         * \lang english
         * @brief Random access iterator type.
         *
         * \lang simp-chinese
         * @brief 随机访问迭代器类型。
         */
        using iterator = implements::ring_iterator<pointer, value_type>;

        /**
         * \lang english
         * @brief Const random access iterator type.
         *
         * \lang simp-chinese
         * @brief 常量随机访问迭代器类型。
         */
        using const_iterator = implements::ring_iterator<const_pointer, value_type>;

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

        /**
         * \lang english
         * @brief Default constructor. Creates a buffer with zero capacity.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。创建一个容量为零的缓冲。
         */
        ring_buffer() noexcept : data_(nullptr), capacity_(0), head_(0), size_(0), alloc_() {
        }

        /**
         * \lang english
         * @brief Constructs an empty buffer with the given capacity.
         *
         * @param capacity The maximum number of elements the buffer can hold
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定容量构造一个空的缓冲。
         *
         * @param capacity 缓冲可容纳的最大元素数量
         * @param alloc 使用的分配器
         */
        explicit ring_buffer(const size_type capacity, const allocator_type &alloc = allocator_type()) :
            data_(nullptr), capacity_(capacity), head_(0), size_(0), alloc_(alloc) {
            if (capacity_ != 0) {
                data_ = traits::allocate(alloc_, capacity_);
            }
        }

        /**
         * \lang english
         * @brief Copy constructor. Copies the capacity and the elements of other.
         *
         * @param other The buffer to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。拷贝 other 的容量与元素。
         *
         * @param other 要拷贝的缓冲
         */
        ring_buffer(const ring_buffer &other) :
            data_(nullptr), capacity_(other.capacity_), head_(0), size_(0),
            alloc_(traits::select_on_container_copy_construction(other.alloc_)) {
            if (capacity_ != 0) {
                data_ = traits::allocate(alloc_, capacity_);
            }
            copy_elements(other);
        }

        /**
         * \lang english
         * @brief Move constructor. Steals the storage of other.
         *
         * @param other The buffer to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。接管 other 的存储。
         *
         * @param other 要移动的缓冲
         */
        ring_buffer(ring_buffer &&other) noexcept :
            data_(other.data_), capacity_(other.capacity_), head_(other.head_), size_(other.size_),
            alloc_(utility::move(other.alloc_)) {
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.head_ = 0;
            other.size_ = 0;
        }

        /**
         * \lang english
         * @brief Constructs the buffer from the range [first, last). The capacity equals the range length.
         *
         * @tparam InputIt The forward iterator type
         * @param first The beginning of the range to copy elements from
         * @param last The end of the range to copy elements from
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从区间 [first, last) 构造缓冲，容量等于区间长度。
         *
         * @tparam InputIt 前向迭代器类型
         * @param first 要拷贝元素的区间起点
         * @param last 要拷贝元素的区间终点
         * @param alloc 使用的分配器
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<
                      type_traits::extras::iterators::is_forward_iterator_v<InputIt> &&
                          type_traits::type_relations::is_base_of_v<
                              std::forward_iterator_tag,
                              typename type_traits::extras::iterators::iterator_traits<InputIt>::iterator_category>,
                      int> = 0>
        ring_buffer(InputIt first, InputIt last, const allocator_type &alloc = allocator_type()) :
            data_(nullptr), capacity_(static_cast<size_type>(utility::distance(first, last))), head_(0), size_(0), alloc_(alloc) {
            if (capacity_ != 0) {
                data_ = traits::allocate(alloc_, capacity_);
                try {
                    for (; first != last; ++first) {
                        traits::construct(alloc_, data_ + size_, *first);
                        ++size_;
                    }
                } catch (...) {
                    destroy_all();
                    traits::deallocate(alloc_, data_, capacity_);
                    data_ = nullptr;
                    capacity_ = 0;
                    size_ = 0;
                    throw;
                }
            }
        }

        /**
         * \lang english
         * @brief Constructs the buffer from an initializer list. The capacity equals the list length.
         *
         * @param ilist The initializer list to copy elements from
         * @param alloc The allocator to use
         *
         * \lang simp-chinese
         * @brief 从初始化器列表构造缓冲，容量等于列表长度。
         *
         * @param ilist 要拷贝元素的初始化器列表
         * @param alloc 使用的分配器
         */
        ring_buffer(std::initializer_list<value_type> ilist, const allocator_type &alloc = allocator_type()) :
            data_(nullptr), capacity_(ilist.size()), head_(0), size_(0), alloc_(alloc) {
            if (capacity_ != 0) {
                data_ = traits::allocate(alloc_, capacity_);
                for (const auto &value: ilist) {
                    traits::construct(alloc_, data_ + size_, value);
                    ++size_;
                }
            }
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the elements and releases the storage.
         *
         * \lang simp-chinese
         * @brief 析构函数。销毁元素并释放存储。
         */
        ~ring_buffer() {
            destroy_all();
            if (data_ != nullptr) {
                traits::deallocate(alloc_, data_, capacity_);
            }
        }

        /**
         * \lang english
         * @brief Copy assignment. Replaces the capacity and the elements with a copy of other.
         *
         * @param other The buffer to copy from
         * @return A reference to this buffer
         *
         * \lang simp-chinese
         * @brief 拷贝赋值。用 other 的副本替换容量与元素。
         *
         * @param other 要拷贝的缓冲
         * @return 本缓冲的引用
         */
        ring_buffer &operator=(const ring_buffer &other) {
            if (this != utility::addressof(other)) {
                destroy_all();
                if (data_ != nullptr) {
                    traits::deallocate(alloc_, data_, capacity_);
                    data_ = nullptr;
                }
                if constexpr (traits::propagate_on_container_copy_assignment::value) {
                    alloc_ = other.alloc_;
                }
                capacity_ = other.capacity_;
                head_ = 0;
                size_ = 0;
                if (capacity_ != 0) {
                    data_ = traits::allocate(alloc_, capacity_);
                }
                copy_elements(other);
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment. Replaces the contents by moving from other.
         *
         * @param other The buffer to move from
         * @return A reference to this buffer
         *
         * \lang simp-chinese
         * @brief 移动赋值。通过移动 other 替换内容。
         *
         * @param other 要移动的缓冲
         * @return 本缓冲的引用
         */
        ring_buffer &operator=(ring_buffer &&other) noexcept(traits::propagate_on_container_move_assignment::value ||
                                                             traits::is_always_equal::value) {
            if (this != utility::addressof(other)) {
                if constexpr (traits::propagate_on_container_move_assignment::value) {
                    release_storage();
                    alloc_ = utility::move(other.alloc_);
                    steal_storage(other);
                } else if (alloc_ == other.alloc_) {
                    release_storage();
                    steal_storage(other);
                } else {
                    release_storage();
                    capacity_ = other.size_;
                    head_ = 0;
                    size_ = 0;
                    if (capacity_ != 0) {
                        data_ = traits::allocate(alloc_, capacity_);
                        for (size_type i = 0; i < other.size_; ++i) {
                            traits::construct(alloc_, data_ + i, utility::move(other.data_[(other.head_ + i) % other.capacity_]));
                            ++size_;
                        }
                    }
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Returns the allocator used by this buffer.
         *
         * @return A copy of the allocator
         *
         * \lang simp-chinese
         * @brief 返回本缓冲使用的分配器。
         *
         * @return 分配器的副本
         */
        RAINY_NODISCARD allocator_type get_allocator() const noexcept {
            return alloc_;
        }

        /**
         * \lang english
         * @brief Checks whether the buffer is empty.
         *
         * @return true if the buffer contains no elements
         *
         * \lang simp-chinese
         * @brief 检查缓冲是否为空。
         *
         * @return 若缓冲不包含任何元素则返回 true
         */
        RAINY_NODISCARD bool empty() const noexcept {
            return size_ == 0;
        }

        /**
         * \lang english
         * @brief Checks whether the buffer is full.
         *
         * @return true if the buffer holds as many elements as its capacity
         *
         * \lang simp-chinese
         * @brief 检查缓冲是否已满。
         *
         * @return 若缓冲元素数量达到容量则返回 true
         */
        RAINY_NODISCARD bool full() const noexcept {
            return size_ == capacity_;
        }

        /**
         * \lang english
         * @brief Returns the number of elements in the buffer.
         *
         * @return The number of elements
         *
         * \lang simp-chinese
         * @brief 返回缓冲中的元素数量。
         *
         * @return 元素数量
         */
        RAINY_NODISCARD size_type size() const noexcept {
            return size_;
        }

        /**
         * \lang english
         * @brief Returns the capacity of the buffer.
         *
         * @return The maximum number of elements the buffer can hold
         *
         * \lang simp-chinese
         * @brief 返回缓冲的容量。
         *
         * @return 缓冲可容纳的最大元素数量
         */
        RAINY_NODISCARD size_type capacity() const noexcept {
            return capacity_;
        }

        /**
         * \lang english
         * @brief Returns the maximum number of elements the buffer can hold, which equals its capacity.
         *
         * @return The maximum number of elements
         *
         * \lang simp-chinese
         * @brief 返回缓冲可容纳的最大元素数量，等于其容量。
         *
         * @return 最大元素数量
         */
        RAINY_NODISCARD size_type max_size() const noexcept {
            return capacity_;
        }

        /**
         * \lang english
         * @brief Removes all elements from the buffer, keeping its capacity.
         *
         * \lang simp-chinese
         * @brief 移除缓冲中的所有元素，并保留其容量。
         */
        void clear() noexcept {
            destroy_all();
            head_ = 0;
            size_ = 0;
        }

        /**
         * \lang english
         * @brief Exchanges the contents and capacity of two buffers.
         *
         * @param other The buffer to exchange contents with
         *
         * \lang simp-chinese
         * @brief 交换两个缓冲的内容与容量。
         *
         * @param other 要与之交换内容的缓冲
         */
        void swap(ring_buffer &other) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
            using std::swap;
            if constexpr (traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
                swap(alloc_, other.alloc_);
            }
            swap(data_, other.data_);
            swap(capacity_, other.capacity_);
            swap(head_, other.head_);
            swap(size_, other.size_);
        }

        /**
         * \lang english
         * @brief Appends a copy of value.
         *
         * @param value The element to append
         * @throws core::exceptions::logic::length_error If the buffer is full under the throw_on_full policy, or the capacity is zero
         *
         * \lang simp-chinese
         * @brief 追加 value 的副本。
         *
         * @param value 要追加的元素
         * @throws core::exceptions::logic::length_error 若缓冲已满且策略为 throw_on_full，或容量为零
         */
        void push_back(const_reference value) {
            emplace_back(value);
        }

        /**
         * \lang english
         * @brief Appends value by moving it.
         *
         * @param value The element to move and append
         * @throws core::exceptions::logic::length_error If the buffer is full under the throw_on_full policy, or the capacity is zero
         *
         * \lang simp-chinese
         * @brief 通过移动追加 value。
         *
         * @param value 要移动并追加的元素
         * @throws core::exceptions::logic::length_error 若缓冲已满且策略为 throw_on_full，或容量为零
         */
        void push_back(value_type &&value) {
            emplace_back(utility::move(value));
        }

        /**
         * \lang english
         * @brief Prepends a copy of value.
         *
         * @param value The element to prepend
         * @throws core::exceptions::logic::length_error If the buffer is full under the throw_on_full policy, or the capacity is zero
         *
         * \lang simp-chinese
         * @brief 在头部插入 value 的副本。
         *
         * @param value 要插入的元素
         * @throws core::exceptions::logic::length_error 若缓冲已满且策略为 throw_on_full，或容量为零
         */
        void push_front(const_reference value) {
            emplace_front(value);
        }

        /**
         * \lang english
         * @brief Prepends value by moving it.
         *
         * @param value The element to move and prepend
         * @throws core::exceptions::logic::length_error If the buffer is full under the throw_on_full policy, or the capacity is zero
         *
         * \lang simp-chinese
         * @brief 通过移动在头部插入 value。
         *
         * @param value 要移动并插入的元素
         * @throws core::exceptions::logic::length_error 若缓冲已满且策略为 throw_on_full，或容量为零
         */
        void push_front(value_type &&value) {
            emplace_front(utility::move(value));
        }

        /**
         * \lang english
         * @brief Constructs an element in place at the back of the buffer.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of T
         * @param args The arguments forwarded to the constructor of T
         * @return A reference to the constructed element
         * @throws core::exceptions::logic::length_error If the buffer is full under the throw_on_full policy, or the capacity is zero
         *
         * \lang simp-chinese
         * @brief 在缓冲尾部就地构造元素。
         *
         * @tparam Args 转发给 T 构造函数的实参类型
         * @param args 转发给 T 构造函数的实参
         * @return 被构造元素的引用
         * @throws core::exceptions::logic::length_error 若缓冲已满且策略为 throw_on_full，或容量为零
         */
        template <typename... Args>
        reference emplace_back(Args &&...args) {
            make_room_for_back();
            const size_type tail = (head_ + size_) % capacity_;
            traits::construct(alloc_, data_ + tail, utility::forward<Args>(args)...);
            ++size_;
            return data_[tail];
        }

        /**
         * \lang english
         * @brief Constructs an element in place at the front of the buffer.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of T
         * @param args The arguments forwarded to the constructor of T
         * @return A reference to the constructed element
         * @throws core::exceptions::logic::length_error If the buffer is full under the throw_on_full policy, or the capacity is zero
         *
         * \lang simp-chinese
         * @brief 在缓冲头部就地构造元素。
         *
         * @tparam Args 转发给 T 构造函数的实参类型
         * @param args 转发给 T 构造函数的实参
         * @return 被构造元素的引用
         * @throws core::exceptions::logic::length_error 若缓冲已满且策略为 throw_on_full，或容量为零
         */
        template <typename... Args>
        reference emplace_front(Args &&...args) {
            make_room_for_front();
            head_ = (head_ + capacity_ - 1) % capacity_;
            traits::construct(alloc_, data_ + head_, utility::forward<Args>(args)...);
            ++size_;
            return data_[head_];
        }

        /**
         * \lang english
         * @brief Removes the last element. Does nothing if the buffer is empty.
         *
         * \lang simp-chinese
         * @brief 移除最后一个元素。若缓冲为空则不做任何事。
         */
        void pop_back() noexcept {
            if (size_ == 0) {
                return;
            }
            const size_type tail = (head_ + size_ - 1) % capacity_;
            traits::destroy(alloc_, data_ + tail);
            --size_;
        }

        /**
         * \lang english
         * @brief Removes the first element. Does nothing if the buffer is empty.
         *
         * \lang simp-chinese
         * @brief 移除第一个元素。若缓冲为空则不做任何事。
         */
        void pop_front() noexcept {
            if (size_ == 0) {
                return;
            }
            traits::destroy(alloc_, data_ + head_);
            head_ = (head_ + 1) % capacity_;
            --size_;
        }

        /**
         * \lang english
         * @brief Returns a reference to the first element. The behavior is undefined if the buffer is empty.
         *
         * @return A reference to the first element
         *
         * \lang simp-chinese
         * @brief 返回第一个元素的引用。若缓冲为空则行为未定义。
         *
         * @return 第一个元素的引用
         */
        RAINY_NODISCARD reference front() noexcept {
            return data_[head_];
        }

        /**
         * \lang english
         * @brief Returns a const reference to the first element. The behavior is undefined if the buffer is empty.
         *
         * @return A const reference to the first element
         *
         * \lang simp-chinese
         * @brief 返回第一个元素的常量引用。若缓冲为空则行为未定义。
         *
         * @return 第一个元素的常量引用
         */
        RAINY_NODISCARD const_reference front() const noexcept {
            return data_[head_];
        }

        /**
         * \lang english
         * @brief Returns a reference to the last element. The behavior is undefined if the buffer is empty.
         *
         * @return A reference to the last element
         *
         * \lang simp-chinese
         * @brief 返回最后一个元素的引用。若缓冲为空则行为未定义。
         *
         * @return 最后一个元素的引用
         */
        RAINY_NODISCARD reference back() noexcept {
            return data_[(head_ + size_ - 1) % capacity_];
        }

        /**
         * \lang english
         * @brief Returns a const reference to the last element. The behavior is undefined if the buffer is empty.
         *
         * @return A const reference to the last element
         *
         * \lang simp-chinese
         * @brief 返回最后一个元素的常量引用。若缓冲为空则行为未定义。
         *
         * @return 最后一个元素的常量引用
         */
        RAINY_NODISCARD const_reference back() const noexcept {
            return data_[(head_ + size_ - 1) % capacity_];
        }

        /**
         * \lang english
         * @brief Returns a reference to the element at the given index, counted from the front.
         *
         * @param index The index of the element
         * @return A reference to the element
         *
         * \lang simp-chinese
         * @brief 返回从头部计数的指定下标处元素的引用。
         *
         * @param index 元素下标
         * @return 元素的引用
         */
        RAINY_NODISCARD reference operator[](const size_type index) noexcept {
            return data_[(head_ + index) % capacity_];
        }

        /**
         * \lang english
         * @brief Returns a const reference to the element at the given index, counted from the front.
         *
         * @param index The index of the element
         * @return A const reference to the element
         *
         * \lang simp-chinese
         * @brief 返回从头部计数的指定下标处元素的常量引用。
         *
         * @param index 元素下标
         * @return 元素的常量引用
         */
        RAINY_NODISCARD const_reference operator[](const size_type index) const noexcept {
            return data_[(head_ + index) % capacity_];
        }

        /**
         * \lang english
         * @brief Returns a reference to the element at the given index, with bounds checking.
         *
         * @param index The index of the element
         * @return A reference to the element
         * @throws core::exceptions::logic::out_of_range If index is not less than size()
         *
         * \lang simp-chinese
         * @brief 返回指定下标处元素的引用，并进行边界检查。
         *
         * @param index 元素下标
         * @return 元素的引用
         * @throws core::exceptions::logic::out_of_range 若 index 不小于 size()
         */
        RAINY_NODISCARD reference at(const size_type index) {
            if (index >= size_) {
                core::exceptions::logic::throw_out_of_range("ring_buffer::at: index out of range");
            }
            return data_[(head_ + index) % capacity_];
        }

        /**
         * \lang english
         * @brief Returns a const reference to the element at the given index, with bounds checking.
         *
         * @param index The index of the element
         * @return A const reference to the element
         * @throws core::exceptions::logic::out_of_range If index is not less than size()
         *
         * \lang simp-chinese
         * @brief 返回指定下标处元素的常量引用，并进行边界检查。
         *
         * @param index 元素下标
         * @return 元素的常量引用
         * @throws core::exceptions::logic::out_of_range 若 index 不小于 size()
         */
        RAINY_NODISCARD const_reference at(const size_type index) const {
            if (index >= size_) {
                core::exceptions::logic::throw_out_of_range("ring_buffer::at: index out of range");
            }
            return data_[(head_ + index) % capacity_];
        }

        /**
         * \lang english
         * @brief Returns an iterator to the first element.
         *
         * @return An iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向第一个元素的迭代器。
         *
         * @return 指向第一个元素的迭代器
         */
        RAINY_NODISCARD iterator begin() noexcept {
            return iterator(data_, capacity_, head_, 0);
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the first element.
         *
         * @return A const iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向第一个元素的常量迭代器。
         *
         * @return 指向第一个元素的常量迭代器
         */
        RAINY_NODISCARD const_iterator begin() const noexcept {
            return const_iterator(data_, capacity_, head_, 0);
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the first element.
         *
         * @return A const iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向第一个元素的常量迭代器。
         *
         * @return 指向第一个元素的常量迭代器
         */
        RAINY_NODISCARD const_iterator cbegin() const noexcept {
            return const_iterator(data_, capacity_, head_, 0);
        }

        /**
         * \lang english
         * @brief Returns an iterator past the last element.
         *
         * @return An iterator past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个元素之后的迭代器。
         *
         * @return 指向最后一个元素之后的迭代器
         */
        RAINY_NODISCARD iterator end() noexcept {
            return iterator(data_, capacity_, head_, size_);
        }

        /**
         * \lang english
         * @brief Returns a const iterator past the last element.
         *
         * @return A const iterator past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个元素之后的常量迭代器。
         *
         * @return 指向最后一个元素之后的常量迭代器
         */
        RAINY_NODISCARD const_iterator end() const noexcept {
            return const_iterator(data_, capacity_, head_, size_);
        }

        /**
         * \lang english
         * @brief Returns a const iterator past the last element.
         *
         * @return A const iterator past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个元素之后的常量迭代器。
         *
         * @return 指向最后一个元素之后的常量迭代器
         */
        RAINY_NODISCARD const_iterator cend() const noexcept {
            return const_iterator(data_, capacity_, head_, size_);
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator to the last element.
         *
         * @return A reverse iterator to the last element
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个元素的反向迭代器。
         *
         * @return 指向最后一个元素的反向迭代器
         */
        RAINY_NODISCARD reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the last element.
         *
         * @return A const reverse iterator to the last element
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个元素的常量反向迭代器。
         *
         * @return 指向最后一个元素的常量反向迭代器
         */
        RAINY_NODISCARD const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the last element.
         *
         * @return A const reverse iterator to the last element
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个元素的常量反向迭代器。
         *
         * @return 指向最后一个元素的常量反向迭代器
         */
        RAINY_NODISCARD const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(cend());
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator past the first element.
         *
         * @return A reverse iterator past the first element
         *
         * \lang simp-chinese
         * @brief 返回指向第一个元素之前的反向迭代器。
         *
         * @return 指向第一个元素之前的反向迭代器
         */
        RAINY_NODISCARD reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator past the first element.
         *
         * @return A const reverse iterator past the first element
         *
         * \lang simp-chinese
         * @brief 返回指向第一个元素之前的常量反向迭代器。
         *
         * @return 指向第一个元素之前的常量反向迭代器
         */
        RAINY_NODISCARD const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator past the first element.
         *
         * @return A const reverse iterator past the first element
         *
         * \lang simp-chinese
         * @brief 返回指向第一个元素之前的常量反向迭代器。
         *
         * @return 指向第一个元素之前的常量反向迭代器
         */
        RAINY_NODISCARD const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(cbegin());
        }

    private:
        void destroy_all() noexcept {
            if (data_ == nullptr) {
                return;
            }
            for (size_type i = 0; i < size_; ++i) {
                traits::destroy(alloc_, data_ + (head_ + i) % capacity_);
            }
        }

        void make_room_for_back() {
            if (capacity_ == 0) {
                core::exceptions::logic::throw_length_error("ring_buffer: capacity is zero");
            }
            if (size_ < capacity_) {
                return;
            }
            if constexpr (Policy == overflow_policy::overwrite_oldest) {
                traits::destroy(alloc_, data_ + head_);
                head_ = (head_ + 1) % capacity_;
                --size_;
            } else {
                core::exceptions::logic::throw_length_error("ring_buffer: buffer is full");
            }
        }

        void make_room_for_front() {
            if (capacity_ == 0) {
                core::exceptions::logic::throw_length_error("ring_buffer: capacity is zero");
            }
            if (size_ < capacity_) {
                return;
            }
            if constexpr (Policy == overflow_policy::overwrite_oldest) {
                const size_type tail = (head_ + size_ - 1) % capacity_;
                traits::destroy(alloc_, data_ + tail);
                --size_;
            } else {
                core::exceptions::logic::throw_length_error("ring_buffer: buffer is full");
            }
        }

        void copy_elements(const ring_buffer &other) {
            for (size_type i = 0; i < other.size_; ++i) {
                traits::construct(alloc_, data_ + i, other.data_[(other.head_ + i) % other.capacity_]);
                ++size_;
            }
        }

        void release_storage() noexcept {
            destroy_all();
            if (data_ != nullptr) {
                traits::deallocate(alloc_, data_, capacity_);
                data_ = nullptr;
            }
            capacity_ = 0;
            head_ = 0;
            size_ = 0;
        }

        void steal_storage(ring_buffer &other) noexcept {
            data_ = other.data_;
            capacity_ = other.capacity_;
            head_ = other.head_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.head_ = 0;
            other.size_ = 0;
        }

        pointer data_;
        size_type capacity_;
        size_type head_;
        size_type size_;
        allocator_type alloc_;
    };

    /**
     * \lang english
     * @brief A ring_buffer that overwrites the oldest element when it is full.
     *
     * @tparam T The element type.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 满载时覆盖最旧元素的 ring_buffer。
     *
     * @tparam T 元素类型。
     * @tparam Allocator 分配器类型。
     */
    template <typename T, typename Allocator = core::memory::allocator<T>>
    using circular_buffer = ring_buffer<T, Allocator, overflow_policy::overwrite_oldest>;

    /**
     * \lang english
     * @brief Swaps the contents of two ring buffers.
     *
     * @param left The first buffer
     * @param right The second buffer
     *
     * \lang simp-chinese
     * @brief 交换两个 ring_buffer 的内容。
     *
     * @param left 第一个缓冲
     * @param right 第二个缓冲
     */
    template <typename T, typename Allocator, overflow_policy Policy>
    void swap(ring_buffer<T, Allocator, Policy> &left, ring_buffer<T, Allocator, Policy> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

#endif
