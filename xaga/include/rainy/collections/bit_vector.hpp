/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain left copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_COLLECTIONS_BIT_VECTOR_HPP
#define RAINY_COLLECTIONS_BIT_VECTOR_HPP
#include <rainy/core/core.hpp>

namespace rainy::collections {
    template <typename Alloc = std::allocator<bool>>
    class bit_vector;
}

namespace rainy::collections::implements {
    using block_type = std::uint64_t;

    inline constexpr std::size_t bits_per_block = sizeof(block_type) * CHAR_BIT;

    inline constexpr std::size_t blocks_for(std::size_t count) noexcept {
        return (count + bits_per_block - 1) / bits_per_block;
    }

    inline constexpr std::size_t bit_index(std::size_t pos) noexcept {
        return pos % bits_per_block;
    }

    inline constexpr std::size_t block_index(std::size_t pos) noexcept {
        return pos / bits_per_block;
    }

    inline constexpr block_type low_mask(std::size_t count) noexcept {
        return count == 0 ? block_type{0} : count == bits_per_block ? ~block_type{0} : ((block_type{1} << count) - block_type{1});
    }

    inline int popcount64(std::uint64_t v) noexcept {
#if RAINY_USING_CLANG || RAINY_USING_GCC
        return __builtin_popcountll(static_cast<unsigned long long>(v));
#elif defined(_MSC_VER)
        return static_cast<int>(__popcnt64(v));
#endif
    }

    template <bool IsConst, typename BitVec>
    class bit_iterator_impl {
    public:
        using block_ptr = std::conditional_t<IsConst, const block_type *, block_type *>;

        template <typename Alloc>
        friend class collections::bit_vector;

        template <bool, typename>
        friend class bit_iterator_impl;

        RAINY_CONSTEXPR20 bit_iterator_impl(block_ptr d, std::size_t p) noexcept : data_(d), pos_(p) {
        }

        using iterator_category = std::random_access_iterator_tag;
        using value_type = bool;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = std::conditional_t<IsConst, bool, typename BitVec::reference>;

        RAINY_CONSTEXPR20 bit_iterator_impl() noexcept : data_(nullptr), pos_(0) {
        }

        template <bool OtherConst, type_traits::other_trans::enable_if_t<IsConst && !OtherConst, int> = 0>
        RAINY_CONSTEXPR20 bit_iterator_impl(const bit_iterator_impl<OtherConst, BitVec> &right) noexcept :
            data_(right.data_), pos_(right.pos_) {
        }

        RAINY_CONSTEXPR20 reference operator*() const noexcept {
            if constexpr (IsConst) {
                return (data_[implements::block_index(pos_)] >> implements::bit_index(pos_)) & block_type{1};
            } else {
                return reference(data_ + implements::block_index(pos_), implements::bit_index(pos_));
            }
        }

        RAINY_CONSTEXPR20 bit_iterator_impl &operator++() noexcept {
            ++pos_;
            return *this;
        }

        RAINY_CONSTEXPR20 bit_iterator_impl operator++(int) noexcept {
            auto t = *this;
            ++pos_;
            return t;
        }

        RAINY_CONSTEXPR20 bit_iterator_impl &operator--() noexcept {
            --pos_;
            return *this;
        }

        RAINY_CONSTEXPR20 bit_iterator_impl operator--(int) noexcept {
            auto t = *this;
            --pos_;
            return t;
        }

        RAINY_CONSTEXPR20 bit_iterator_impl &operator+=(difference_type count) noexcept {
            pos_ = static_cast<std::size_t>(static_cast<difference_type>(pos_) + count);
            return *this;
        }
        RAINY_CONSTEXPR20 bit_iterator_impl &operator-=(difference_type count) noexcept {
            pos_ = static_cast<std::size_t>(static_cast<difference_type>(pos_) - count);
            return *this;
        }

        RAINY_CONSTEXPR20 friend bit_iterator_impl operator+(bit_iterator_impl it, difference_type count) noexcept {
            it += count;
            return it;
        }

        RAINY_CONSTEXPR20 friend bit_iterator_impl operator+(difference_type count, bit_iterator_impl it) noexcept {
            it += count;
            return it;
        }

        RAINY_CONSTEXPR20 friend bit_iterator_impl operator-(bit_iterator_impl it, difference_type count) noexcept {
            it -= count;
            return it;
        }

        RAINY_CONSTEXPR20 friend difference_type operator-(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return static_cast<difference_type>(left.pos_) - static_cast<difference_type>(right.pos_);
        }

        RAINY_CONSTEXPR20 reference operator[](difference_type count) const noexcept {
            return *(*this + count);
        }

        RAINY_CONSTEXPR20 friend bool operator==(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return left.pos_ == right.pos_;
        }

        RAINY_CONSTEXPR20 friend bool operator!=(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return left.pos_ != right.pos_;
        }

        RAINY_CONSTEXPR20 friend bool operator<(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return left.pos_ < right.pos_;
        }

        RAINY_CONSTEXPR20 friend bool operator<=(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return left.pos_ <= right.pos_;
        }

        RAINY_CONSTEXPR20 friend bool operator>(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return left.pos_ > right.pos_;
        }

        RAINY_CONSTEXPR20 friend bool operator>=(const bit_iterator_impl &left, const bit_iterator_impl &right) noexcept {
            return left.pos_ >= right.pos_;
        }

    private:
        block_ptr data_;
        std::size_t pos_;
    };
}

namespace rainy::collections {
    template <typename Alloc>
    class bit_vector {
    public:
        using block_type = implements::block_type;
        using block_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<block_type>;
        using block_alloc_traits = std::allocator_traits<block_alloc_type>;

        using value_type = bool;
        using allocator_type = Alloc;
        using const_reference = bool;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        class reference {
        public:
            template <typename Alloc>
            friend class collections::bit_vector;

            RAINY_CONSTEXPR20 reference(const reference &) noexcept = default;

            RAINY_CONSTEXPR20 reference(block_type *right, std::size_t bit_pos) noexcept :
                block_(right), mask_(block_type{1} << bit_pos) {
            }

            RAINY_CONSTEXPR20 ~reference() = default;

            RAINY_CONSTEXPR20 operator bool() const noexcept {
                return (*block_ & mask_) != 0;
            }

            RAINY_CONSTEXPR20 reference &operator=(bool val) noexcept {
                if (val) {
                    *block_ |= mask_;
                } else {
                    *block_ &= ~mask_;
                }
                return *this;
            }

            RAINY_CONSTEXPR20 reference &operator=(const reference &right) noexcept {
                return operator=(static_cast<bool>(right));
            }

            RAINY_CONSTEXPR20 const reference &operator=(bool val) const noexcept {
                if (val) {
                    *block_ |= mask_;
                } else {
                    *block_ &= ~mask_;
                }
                return *this;
            }

            RAINY_CONSTEXPR20 void flip() noexcept {
                *block_ ^= mask_;
            }

            RAINY_CONSTEXPR20 bool operator~() const noexcept {
                return (*block_ & mask_) == 0;
            }

        private:
            block_type *block_;
            block_type mask_;
        };

        using iterator = implements::bit_iterator_impl<false, bit_vector>;
        using const_iterator = implements::bit_iterator_impl<true, bit_vector>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using pointer = iterator;
        using const_pointer = const_iterator;

        RAINY_CONSTEXPR20 bit_vector() noexcept(noexcept(allocator_type())) : bit_vector(allocator_type()) {
        }

        RAINY_CONSTEXPR20 explicit bit_vector(const allocator_type &right) noexcept : pair{right, impl{}} {
        }

        RAINY_CONSTEXPR20 explicit bit_vector(size_type count, const allocator_type &right = allocator_type()) : bit_vector(right) {
            resize(count, false);
        }

        RAINY_CONSTEXPR20 bit_vector(size_type count, const bool &value, const allocator_type &right = allocator_type()) :
            bit_vector(right) {
            resize(count, value);
        }

        template <
            typename InputIter,
            type_traits::other_trans::enable_if_t<
                std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>, int> = 0>
        RAINY_CONSTEXPR20 bit_vector(InputIter first, InputIter last, const allocator_type &right = allocator_type()) :
            bit_vector(right) {
            for (; first != last; ++first) {
                push_back(static_cast<bool>(*first));
            }
        }

        RAINY_CONSTEXPR20 bit_vector(std::initializer_list<bool> ilist, const allocator_type &right = allocator_type()) :
            bit_vector(ilist.begin(), ilist.end(), right) {
        }

        RAINY_CONSTEXPR20 bit_vector(const bit_vector &right) :
            bit_vector(block_alloc_traits::select_on_container_copy_construction(right.pair.get_first())) {
            copy_from(right);
        }

        RAINY_CONSTEXPR20 bit_vector(bit_vector &&right) noexcept :
            pair{utility::move(right.pair.get_first()),
                 impl{right.pair.second.data, right.pair.second.size, right.pair.second.cap_bits}} {
            right.pair = {};
        }

        RAINY_CONSTEXPR20 bit_vector(const bit_vector &right, const type_traits::helper::identity_t<allocator_type> &left) :
            bit_vector(left) {
            copy_from(right);
        }

        RAINY_CONSTEXPR20 bit_vector(bit_vector &&right, const type_traits::helper::identity_t<allocator_type> &left) :
            pair{left.get_first(), impl{}} {
            auto &allocator = pair.get_first();
            if (allocator == right.get_first()) {
                pair.second.data = right.second.data;
                pair.second.size = right.size();
                pair.second.cap_bits = right.cap_bits;
                right.pair.second = {};
            } else {
                copy_from(right);
            }
        }

        RAINY_CONSTEXPR20 ~bit_vector() {
            deallocate();
        }

        RAINY_CONSTEXPR20 bit_vector &operator=(const bit_vector &right) {
            if (this == &right) {
                return *this;
            }
            auto &allocator = pair.get_first();
            if constexpr (block_alloc_traits::propagate_on_container_copy_assignment::value) {
                if (allocator != right.get_first()) {
                    deallocate();
                    allocator = right.get_first();
                }
            }
            copy_from(right);
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &operator=(bit_vector &&right) noexcept(
            block_alloc_traits::propagate_on_container_move_assignment::value || block_alloc_traits::is_always_equal::value) {
            if (this == &right) {
                return *this;
            }
            auto &allocator = pair.get_first();
            if constexpr (block_alloc_traits::propagate_on_container_move_assignment::value) {
                deallocate();
                allocator = utility::move(right.pair.get_first());
                steal(right);
            } else if (allocator == right.pair.get_first()) {
                deallocate();
                steal(right);
            } else {
                copy_from(right);
            }
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &operator=(std::initializer_list<bool> ilist) {
            assign(ilist);
            return *this;
        }

        template <
            typename InputIter,
            type_traits::other_trans::enable_if_t<
                std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>, int> = 0>
        RAINY_CONSTEXPR20 void assign(InputIter first, InputIter last) {
            clear();
            for (; first != last; ++first) {
                push_back(static_cast<bool>(*first));
            }
        }

        RAINY_CONSTEXPR20 void assign(size_type count, const bool &val) {
            clear();
            resize(count, val);
        }

        RAINY_CONSTEXPR20 void assign(std::initializer_list<bool> ilist) {
            assign(ilist.begin(), ilist.end());
        }

        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return allocator_type(pair.get_first());
        }

        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return {pair.second.data, 0};
        }

        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return {pair.second.data, 0};
        }

        RAINY_CONSTEXPR20 iterator end() noexcept {
            return {pair.second.data, size()};
        }

        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return {pair.second.data, size()};
        }

        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return begin();
        }

        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return end();
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crend() const noexcept {
            return rend();
        }

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return pair.second.size == 0;
        }

        RAINY_CONSTEXPR20 size_type size() const noexcept {
            return pair.second.size;
        }

        RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return block_alloc_traits::max_size(pair.get_first()) * implements::bits_per_block;
        }

        RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            return pair.second.cap_bits;
        }

        RAINY_CONSTEXPR20 void reserve(size_type new_cap) {
            if (new_cap <= pair.second.cap_bits) {
                return;
            }
            reallocate(implements::blocks_for(new_cap));
        }

        RAINY_CONSTEXPR20 void resize(size_type new_size, bool val = false) {
            if (new_size > size()) {
                reserve(new_size);
                if (val) {
                    if (size() % implements::bits_per_block != 0) {
                        std::size_t used = size() % implements::bits_per_block;
                        block_type mask = implements::low_mask(implements::bits_per_block - used) << used;
                        pair.second.data[implements::block_index(size())] |= mask;
                    }
                    std::size_t old_full = implements::blocks_for(size());
                    std::size_t new_full = implements::blocks_for(new_size);
                    for (std::size_t i = old_full; i < new_full; ++i) {
                        pair.second.data[i] = ~block_type{0};
                    }
                }
            }
            pair.second.size = new_size;
            zero_unused_bits();
        }

        RAINY_CONSTEXPR20 void shrink_to_fit() {
            std::size_t needed = implements::blocks_for(size());
            std::size_t current = implements::blocks_for(pair.second.cap_bits);
            if (needed >= current) {
                return;
            }
            reallocate(needed);
        }

        RAINY_CONSTEXPR20 reference operator[](size_type pos) noexcept {
            return {pair.second.data + implements::block_index(pos), implements::bit_index(pos)};
        }

        RAINY_CONSTEXPR20 const_reference operator[](size_type pos) const noexcept {
            return (pair.second.data[implements::block_index(pos)] >> implements::bit_index(pos)) & block_type{1};
        }

        RAINY_CONSTEXPR20 reference at(size_type pos) {
            range_check(pos);
            return (*this)[pos];
        }

        RAINY_CONSTEXPR20 const_reference at(size_type pos) const {
            range_check(pos);
            return (*this)[pos];
        }

        RAINY_CONSTEXPR20 reference front() noexcept {
            return (*this)[0];
        }

        RAINY_CONSTEXPR20 const_reference front() const noexcept {
            return (*this)[0];
        }

        RAINY_CONSTEXPR20 reference back() noexcept {
            return (*this)[size() - 1];
        }

        RAINY_CONSTEXPR20 const_reference back() const noexcept {
            return (*this)[size() - 1];
        }

        RAINY_CONSTEXPR20 void push_back(const bool &val) {
            if (rainy_unlikely(pair.second.size == pair.second.cap_bits)) {
                grow();
            }
            (*this)[size()] = val;
            incr_size();
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 reference emplace_back(Args &&...args) {
            push_back(static_cast<bool>(utility::forward<Args>(args))...);
            return back();
        }

        RAINY_CONSTEXPR20 void pop_back() noexcept {
            decr_size();
            zero_unused_bits();
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 iterator emplace(const_iterator position, Args &&...args) {
            return insert(position, static_cast<bool>(std::forward<Args>(args))...);
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, const bool &val) {
            return insert(position, size_type{1}, val);
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, size_type count, const bool &val) {
            std::size_t idx = position.pos_;
            if (count == 0) {
                return iterator{pair.second.data, idx};
            }
            std::size_t new_size = size() + count;
            reserve(new_size);
            shift_right(idx, count);
            pair.second.size = new_size;
            for (std::size_t i = idx; i < idx + count; ++i) {
                (*this)[i] = val;
            }
            return iterator{pair.second.data, idx};
        }

        template <
            typename InputIter,
            std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>,
                             int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, InputIter first, InputIter last) {
            std::size_t idx = position.pos_;
            bit_vector tmp(first, last, get_allocator());
            if (tmp.empty()) {
                return iterator{pair.second.data, idx};
            }
            std::size_t count = tmp.size();
            std::size_t new_size = size() + count;
            reserve(new_size);
            shift_right(idx, count);
            pair.second.size = new_size;
            for (std::size_t i = 0; i < count; ++i) {
                (*this)[idx + i] = tmp[i];
            }
            return iterator{pair.second.data, idx};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, std::initializer_list<bool> ilist) {
            return insert(position, ilist.begin(), ilist.end());
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator position) {
            return erase(position, position + 1);
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
            std::size_t f = first.pos_;
            std::size_t l = last.pos_;
            if (f == l) {
                return iterator{pair.second.data, f};
            }
            shift_left(f, l - f);
            pair.second.size -= (l - f);
            zero_unused_bits();
            return iterator{pair.second.data, f};
        }

        RAINY_CONSTEXPR20 void swap(bit_vector &right) noexcept(block_alloc_traits::propagate_on_container_swap::value ||
                                                                block_alloc_traits::is_always_equal::value) {
            auto &allocator = pair.get_first();
            if constexpr (block_alloc_traits::propagate_on_container_swap::value) {
                std::swap(allocator, right.get_first());
            }
            std::swap(pair.second.data, right.pair.second.data);
            std::swap(pair.second.size, right.pair.second.size);
            std::swap(pair.second.cap_bits, right.pair.second.cap_bits);
        }

        static RAINY_CONSTEXPR20 void swap(reference left, reference right) noexcept {
            bool tmp = static_cast<bool>(left);
            left = static_cast<bool>(right);
            right = tmp;
        }

        RAINY_CONSTEXPR20 void flip() noexcept {
            std::size_t nb = implements::blocks_for(size());
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] = ~pair.second.data[i];
            }
            zero_unused_bits();
        }

        RAINY_CONSTEXPR20 void clear() noexcept {
            pair.second.size = 0;
            std::size_t nb = implements::blocks_for(pair.second.cap_bits);
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] = block_type{0};
            }
        }

        RAINY_CONSTEXPR20 bool test(size_type pos) const {
            range_check(pos);
            return (*this)[pos];
        }

        RAINY_CONSTEXPR20 bit_vector &set(size_type pos, bool val = true) {
            range_check(pos);
            (*this)[pos] = val;
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &set() noexcept {
            std::size_t nb = implements::blocks_for(size());
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] = ~block_type{0};
            }
            zero_unused_bits();
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &reset(size_type pos) {
            range_check(pos);
            (*this)[pos] = false;
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &reset() noexcept {
            std::size_t nb = implements::blocks_for(size());
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] = block_type{0};
            }
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &flip(size_type pos) {
            range_check(pos);
            pair.second.data[implements::block_index(pos)] ^= block_type{1} << implements::bit_index(pos);
            return *this;
        }

        RAINY_CONSTEXPR20 size_type count() const noexcept {
            std::size_t nb = implements::blocks_for(size());
            size_type cnt = 0;
            for (std::size_t i = 0; i < nb; ++i) {
                cnt += static_cast<size_type>(implements::popcount64(pair.second.data[i]));
            }
            return cnt;
        }

        RAINY_CONSTEXPR20 bool all() const noexcept {
            if (pair.second.size == 0) {
                return true;
            }
            std::size_t nb = implements::blocks_for(size());
            std::size_t tail = size() % implements::bits_per_block;
            for (std::size_t i = 0; i + 1 < nb; ++i) {
                if (pair.second.data[i] != ~block_type{0}) {
                    return false;
                }
            }
            block_type mask = tail ? implements::low_mask(tail) : ~block_type{0};
            return (pair.second.data[nb - 1] & mask) == mask;
        }

        RAINY_CONSTEXPR20 bool any() const noexcept {
            std::size_t nb = implements::blocks_for(size());
            for (std::size_t i = 0; i < nb; ++i) {
                if (pair.second.data[i]) {
                    return true;
                }
            }
            return false;
        }

        RAINY_CONSTEXPR20 bool none() const noexcept {
            return !any();
        }

        RAINY_CONSTEXPR20 bit_vector operator~() const {
            bit_vector result(*this);
            result.flip();
            return result;
        }

        RAINY_CONSTEXPR20 bit_vector &operator&=(const bit_vector &right) noexcept {
            std::size_t nb = (core::min)(implements::blocks_for(size()), implements::blocks_for(right.size()));
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] &= right.pair.second.data[i];
            }
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &operator|=(const bit_vector &right) noexcept {
            std::size_t nb = (core::min)(implements::blocks_for(size()), implements::blocks_for(right.size()));
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] |= right.pair.second.data[i];
            }
            return *this;
        }

        RAINY_CONSTEXPR20 bit_vector &operator^=(const bit_vector &right) noexcept {
            std::size_t nb = (core::min)(implements::blocks_for(size()), implements::blocks_for(right.size()));
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] ^= right.pair.second.data[i];
            }
            return *this;
        }

        RAINY_CONSTEXPR20 friend bit_vector operator&(bit_vector left, const bit_vector &right) {
            left &= right;
            return left;
        }

        RAINY_CONSTEXPR20 friend bit_vector operator|(bit_vector left, const bit_vector &right) {
            left |= right;
            return left;
        }

        RAINY_CONSTEXPR20 friend bit_vector operator^(bit_vector left, const bit_vector &right) {
            left ^= right;
            return left;
        }

        RAINY_CONSTEXPR20 bit_vector operator<<(size_type count) const {
            bit_vector result(size(), false, get_allocator());
            if (count < size()) {
                for (size_type i = count; i < size(); ++i) {
                    result[i] = (*this)[i - count];
                }
            }
            return result;
        }

        RAINY_CONSTEXPR20 bit_vector operator>>(size_type count) const {
            bit_vector result(size(), false, get_allocator());
            if (count < size()) {
                for (size_type i = 0; i + count < size(); ++i) {
                    result[i] = (*this)[i + count];
                }
            }
            return result;
        }

        RAINY_CONSTEXPR20 bit_vector &operator<<=(size_type count) {
            return *this = *this << count;
        }
        RAINY_CONSTEXPR20 bit_vector &operator>>=(size_type count) {
            return *this = *this >> count;
        }

        RAINY_CONSTEXPR20 std::string to_string(char zero = '0', char one = '1') const {
            std::string s(size(), zero);
            for (size_type i = 0; i < size(); ++i) {
                if ((*this)[i]) {
                    s[size() - 1 - i] = one;
                }
            }
            return s;
        }

        RAINY_CONSTEXPR20 unsigned long to_ulong() const {
            constexpr std::size_t ul_bits = sizeof(unsigned long) * CHAR_BIT;
            overflow_check(ul_bits);
            unsigned long result = 0;
            size_type limit = (core::min)(size(), ul_bits);
            for (size_type i = 0; i < limit; ++i) {
                if ((*this)[i]) {
                    result |= static_cast<unsigned long>(1) << i;
                }
            }
            return result;
        }

        RAINY_CONSTEXPR20 unsigned long long to_ullong() const {
            constexpr std::size_t ull_bits = sizeof(unsigned long long) * CHAR_BIT;
            overflow_check(ull_bits);
            unsigned long long result = 0;
            size_type limit = (core::min)(size(), ull_bits);
            for (size_type i = 0; i < limit; ++i) {
                if ((*this)[i]) {
                    result |= static_cast<unsigned long long>(1) << i;
                }
            }
            return result;
        }

        RAINY_CONSTEXPR20 friend bool operator==(const bit_vector &left, const bit_vector &right) noexcept {
            if (left.size() != right.size()) {
                return false;
            }
            std::size_t nb = implements::blocks_for(left.size());
            for (std::size_t i = 0; i < nb; ++i) {
                if (left.pair.second.data[i] != right.pair.second.data[i]) {
                    return false;
                }
            }
            return true;
        }

        RAINY_CONSTEXPR20 friend bool operator!=(const bit_vector &left, const bit_vector &right) noexcept {
            return !(left == right);
        }

        RAINY_CONSTEXPR20 void swap(bit_vector &left, bit_vector &right) noexcept(noexcept(left.swap(right))) {
            left.swap(right);
        }

    private:
        RAINY_CONSTEXPR20 void incr_size() noexcept {
            ++pair.second.size;
        }

        RAINY_CONSTEXPR20 void decr_size() noexcept {
            --pair.second.size;
        }

        RAINY_CONSTEXPR20 void deallocate() noexcept {
            auto &allocator = pair.get_first();
            if (pair.second.data) {
                block_alloc_traits::deallocate(allocator, pair.second.data, implements::blocks_for(pair.second.cap_bits));
                pair.second.data = nullptr;
                pair.second.cap_bits = 0;
            }
        }

        RAINY_CONSTEXPR20 void reallocate(std::size_t new_block_count) {
            auto &allocator = pair.get_first();
            block_type *new_data = block_alloc_traits::allocate(allocator, new_block_count);
            std::size_t old_nb = implements::blocks_for(pair.second.cap_bits);
            std::size_t copy_nb = (core::min)(old_nb, new_block_count);
            for (std::size_t i = 0; i < copy_nb; ++i) {
                new_data[i] = pair.second.data[i];
            }
            for (std::size_t i = copy_nb; i < new_block_count; ++i) {
                new_data[i] = block_type{0};
            }
            if (pair.second.data) {
                block_alloc_traits::deallocate(allocator, pair.second.data, old_nb);
            }
            pair.second.data = new_data;
            pair.second.cap_bits = new_block_count * implements::bits_per_block;
        }

        RAINY_CONSTEXPR20 void grow() {
            std::size_t new_cap = pair.second.cap_bits == 0 ? implements::bits_per_block : pair.second.cap_bits * 2;
            reallocate(implements::blocks_for(new_cap));
        }

        RAINY_CONSTEXPR20 void copy_from(const bit_vector &right) {
            std::size_t nb = implements::blocks_for(right.size());
            if (nb > implements::blocks_for(pair.second.cap_bits)) {
                reallocate(nb);
            }
            for (std::size_t i = 0; i < nb; ++i) {
                pair.second.data[i] = right.pair.second.data[i];
            }
            std::size_t cur_nb = implements::blocks_for(pair.second.cap_bits);
            for (std::size_t i = nb; i < cur_nb; ++i) {
                pair.second.data[i] = block_type{0};
            }
            pair.second.size = right.size();
        }

        RAINY_CONSTEXPR20 void steal(bit_vector &right) noexcept {
            pair.second.data = right.pair.second.data;
            pair.second.size = right.size();
            pair.second.cap_bits = right.pair.second.cap_bits;
            right.pair.second.data = nullptr;
            right.pair.second.size = 0;
            right.pair.second.cap_bits = 0;
        }

        RAINY_CONSTEXPR20 void zero_unused_bits() noexcept {
            std::size_t tail = size() % implements::bits_per_block;
            if (tail == 0 || pair.second.size == 0) {
                return;
            }
            std::size_t last_blk = implements::block_index(size() - 1);
            pair.second.data[last_blk] &= implements::low_mask(tail);
        }

        RAINY_CONSTEXPR20 void shift_right(std::size_t from, std::size_t shift) {
            for (std::size_t i = size(); i > from; --i) {
                (*this)[i - 1 + shift] = (*this)[i - 1];
            }
        }

        RAINY_CONSTEXPR20 void shift_left(std::size_t from, std::size_t shift) {
            for (std::size_t i = from + shift; i < size(); ++i) {
                (*this)[i - shift] = (*this)[i];
            }
        }

        RAINY_CONSTEXPR20 void range_check(size_type pos) const {
            if (pos >= size()) {
                throw std::out_of_range("bit_vector: index out of range");
            }
        }

        RAINY_CONSTEXPR20 void overflow_check(std::size_t target_bits) const {
            for (size_type i = target_bits; i < size(); ++i) {
                if ((*this)[i]) {
                    throw std::overflow_error("bit_vector: value does not fit in target type");
                }
            }
        }

        struct impl {
            block_type *data = nullptr;
            size_type size = 0;
            size_type cap_bits = 0;
        };

        foundation::container::compressed_pair<block_alloc_type, impl> pair;
    };
}

#endif
