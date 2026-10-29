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
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using allocator_type = Allocator;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = implements::ring_iterator<pointer, value_type>;
        using const_iterator = implements::ring_iterator<const_pointer, value_type>;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        ring_buffer() noexcept : data_(nullptr), capacity_(0), head_(0), size_(0), alloc_() {
        }

        explicit ring_buffer(const size_type capacity, const allocator_type &alloc = allocator_type()) :
            data_(nullptr), capacity_(capacity), head_(0), size_(0), alloc_(alloc) {
            if (capacity_ != 0) {
                data_ = traits::allocate(alloc_, capacity_);
            }
        }

        ring_buffer(const ring_buffer &other) :
            data_(nullptr), capacity_(other.capacity_), head_(0), size_(0),
            alloc_(traits::select_on_container_copy_construction(other.alloc_)) {
            if (capacity_ != 0) {
                data_ = traits::allocate(alloc_, capacity_);
            }
            copy_elements(other);
        }

        ring_buffer(ring_buffer &&other) noexcept :
            data_(other.data_), capacity_(other.capacity_), head_(other.head_), size_(other.size_),
            alloc_(utility::move(other.alloc_)) {
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.head_ = 0;
            other.size_ = 0;
        }

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
                for (; first != last; ++first) {
                    traits::construct(alloc_, data_ + size_, *first);
                    ++size_;
                }
            }
        }

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

        ~ring_buffer() {
            destroy_all();
            if (data_ != nullptr) {
                traits::deallocate(alloc_, data_, capacity_);
            }
        }

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

        RAINY_NODISCARD allocator_type get_allocator() const noexcept {
            return alloc_;
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return size_ == 0;
        }

        RAINY_NODISCARD bool full() const noexcept {
            return size_ == capacity_;
        }

        RAINY_NODISCARD size_type size() const noexcept {
            return size_;
        }

        RAINY_NODISCARD size_type capacity() const noexcept {
            return capacity_;
        }

        RAINY_NODISCARD size_type max_size() const noexcept {
            return capacity_;
        }

        void clear() noexcept {
            destroy_all();
            head_ = 0;
            size_ = 0;
        }

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

        void push_back(const_reference value) {
            emplace_back(value);
        }

        void push_back(value_type &&value) {
            emplace_back(utility::move(value));
        }

        void push_front(const_reference value) {
            emplace_front(value);
        }

        void push_front(value_type &&value) {
            emplace_front(utility::move(value));
        }

        template <typename... Args>
        reference emplace_back(Args &&...args) {
            make_room_for_back();
            const size_type tail = (head_ + size_) % capacity_;
            traits::construct(alloc_, data_ + tail, utility::forward<Args>(args)...);
            ++size_;
            return data_[tail];
        }

        template <typename... Args>
        reference emplace_front(Args &&...args) {
            make_room_for_front();
            head_ = (head_ + capacity_ - 1) % capacity_;
            traits::construct(alloc_, data_ + head_, utility::forward<Args>(args)...);
            ++size_;
            return data_[head_];
        }

        void pop_back() noexcept {
            if (size_ == 0) {
                return;
            }
            const size_type tail = (head_ + size_ - 1) % capacity_;
            traits::destroy(alloc_, data_ + tail);
            --size_;
        }

        void pop_front() noexcept {
            if (size_ == 0) {
                return;
            }
            traits::destroy(alloc_, data_ + head_);
            head_ = (head_ + 1) % capacity_;
            --size_;
        }

        RAINY_NODISCARD reference front() noexcept {
            return data_[head_];
        }

        RAINY_NODISCARD const_reference front() const noexcept {
            return data_[head_];
        }

        RAINY_NODISCARD reference back() noexcept {
            return data_[(head_ + size_ - 1) % capacity_];
        }

        RAINY_NODISCARD const_reference back() const noexcept {
            return data_[(head_ + size_ - 1) % capacity_];
        }

        RAINY_NODISCARD reference operator[](const size_type index) noexcept {
            return data_[(head_ + index) % capacity_];
        }

        RAINY_NODISCARD const_reference operator[](const size_type index) const noexcept {
            return data_[(head_ + index) % capacity_];
        }

        RAINY_NODISCARD reference at(const size_type index) {
            if (index >= size_) {
                core::exceptions::logic::throw_out_of_range("ring_buffer::at: index out of range");
            }
            return data_[(head_ + index) % capacity_];
        }

        RAINY_NODISCARD const_reference at(const size_type index) const {
            if (index >= size_) {
                core::exceptions::logic::throw_out_of_range("ring_buffer::at: index out of range");
            }
            return data_[(head_ + index) % capacity_];
        }

        RAINY_NODISCARD iterator begin() noexcept {
            return iterator(data_, capacity_, head_, 0);
        }

        RAINY_NODISCARD const_iterator begin() const noexcept {
            return const_iterator(data_, capacity_, head_, 0);
        }

        RAINY_NODISCARD const_iterator cbegin() const noexcept {
            return const_iterator(data_, capacity_, head_, 0);
        }

        RAINY_NODISCARD iterator end() noexcept {
            return iterator(data_, capacity_, head_, size_);
        }

        RAINY_NODISCARD const_iterator end() const noexcept {
            return const_iterator(data_, capacity_, head_, size_);
        }

        RAINY_NODISCARD const_iterator cend() const noexcept {
            return const_iterator(data_, capacity_, head_, size_);
        }

        RAINY_NODISCARD reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        RAINY_NODISCARD const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        RAINY_NODISCARD const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(cend());
        }

        RAINY_NODISCARD reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        RAINY_NODISCARD const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

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

    template <typename T, typename Allocator = core::memory::allocator<T>>
    using circular_buffer = ring_buffer<T, Allocator, overflow_policy::overwrite_oldest>;

    template <typename T, typename Allocator, overflow_policy Policy>
    void swap(ring_buffer<T, Allocator, Policy> &left, ring_buffer<T, Allocator, Policy> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

#endif
