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
#ifndef RAINY_COLLECTIONS_CONCURRENCY_ATOMIC_GROW_ARRAY_HPP
#define RAINY_COLLECTIONS_CONCURRENCY_ATOMIC_GROW_ARRAY_HPP
#include <atomic>
#include <cstddef>
#include <new>
#include <thread>
#include <span>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/bind.hpp>

namespace rainy::collections::concurrency {
    template <typename Item, template <typename> class Atom = std::atomic>
    struct atomic_grow_array_policy_default {
        template <typename V>
        using atom = Atom<V>;

        std::size_t grow(std::size_t /* const curr */, std::size_t const index) const noexcept {
            return core::builtin::next_power_of_two(index + 1);
        }

        Item make() const noexcept(noexcept(Item())) {
            return Item();
        }
    };

    template <typename Item, typename Policy = atomic_grow_array_policy_default<Item>>
    class atomic_grow_array : private Policy {
    public:
        using size_type = std::size_t;
        using value_type = Item;

        using pointer_span = std::span<value_type *const>;
        using const_pointer_span = std::span<value_type const *const>;

        struct array;

        struct end_tag {};

        template <bool>
        class basic_view;

        template <bool Const, typename Down>
        class basic_iterator {
        private:
            template <bool, typename>
            friend class basic_iterator;
            template <bool>
            friend class basic_view;

            using self = basic_iterator;
            using down = Down;
            friend down;

            template <typename T>
            using maybe_add_const_t = type_traits::other_trans::maybe_const_t<Const, T>;

            array const *array_{};
            size_type index_{};

            basic_iterator(array const *const a, size_type const i) noexcept : array_{a}, index_{i} {
            }

            basic_iterator(array const *const a, end_tag) noexcept : array_{a}, index_{a ? a->size : 0} {
            }

            template <bool ThatC, typename ThatDown, bool ThisC = Const, typename = std::enable_if_t<!ThatC && ThisC>>
            explicit basic_iterator(basic_iterator<ThatC, ThatDown> that) noexcept : array_{that.array_}, index_{that.index_} {
            }

            down &as_down() noexcept {
                return static_cast<down &>(*this);
            }

            down const &as_down() const noexcept {
                return static_cast<down const &>(*this);
            }

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = atomic_grow_array::value_type;
            using difference_type = std::ptrdiff_t;
            using pointer = maybe_add_const_t<value_type> *;
            using reference = maybe_add_const_t<value_type> &;

            basic_iterator() = default; // produces an invalid iterator

            down &operator++() noexcept {
                return ++index_, as_down();
            }
            
            down operator++(int) noexcept {
                return down{array_, index_++};
            }
            
            down &operator+=(difference_type const count) noexcept {
                return index_ += count, as_down();
            }
            
            down operator+(difference_type const count) noexcept {
                return down{as_down()} += count;
            }
            
            down &operator-=(difference_type const count) noexcept {
                return index_ -= count, as_down();
            }
            
            down operator-(difference_type const count) noexcept {
                return down{as_down()} -= count;
            }
            
            friend difference_type operator-(down const left, down const right) noexcept {
                return left.index_ - right.index_;
            }
            
            friend bool operator==(down const left, down const right) noexcept {
                return left.index_ == right.index_;
            }
            
            friend bool operator!=(down const left, down const right) noexcept {
                return left.index_ != right.index_;
            }
            
            friend bool operator<(down const left, down const right) noexcept {
                return left.index < right.index_;
            }
            
            friend bool operator<=(down const left, down const right) noexcept {
                return left.index <= right.index_;
            }
            
            friend bool operator>(down const left, down const right) noexcept {
                return left.index > right.index_;
            }
            
            friend bool operator>=(down const left, down const right) noexcept {
                return left.index >= right.index_;
            }

            reference operator*() noexcept {
                return *array_->list[index_];
            }
            reference operator[](difference_type const count) {
                return *(*this + count);
            }
        };

        template <bool Const>
        class basic_view {
        public:
            friend atomic_grow_array;

            template <typename T>
            using maybe_add_const_t = type_traits::other_trans::maybe_const_t<Const, T>;

            using up = atomic_grow_array;

            using value_type = typename atomic_grow_array::value_type;
            using size_type = typename atomic_grow_array::size_type;
            using reference = maybe_add_const_t<value_type> &;
            using const_reference = value_type const &;
            using pointer = maybe_add_const_t<value_type> *;
            using const_pointer = value_type const *;
            using iterator = std::conditional_t<Const,typename up::const_iterator,typename up::iterator>;
            using const_iterator = typename up::const_iterator;

            basic_view() = default; // produces an invalid view

            iterator begin() noexcept {
                return iterator{array_, 0};
            }

            const_iterator begin() const noexcept {
                return iterator{array_, 0};
            }

            const_iterator cbegin() const noexcept {
                return iterator{array_, 0};
            }

            iterator end() noexcept {
                return iterator{array_, end_tag{}};
            }

            const_iterator end() const noexcept {
                return iterator{array_, end_tag{}};
            }

            const_iterator cend() const noexcept {
                return iterator{array_, end_tag{}};
            }

            size_type size() const noexcept {
                return array_ ? array_->size : 0;
            }

            bool empty() const noexcept {
                return !size();
            }

            reference operator[](size_type index) noexcept {
                return *array_->list[index];
            }

            const_reference operator[](size_type index) const noexcept {
                return *array_->list[index];
            }

            std::span<pointer const> as_ptr_span() noexcept {
                using type = std::span<pointer const>;
                return array_ ? type{array_->list, array_->size} : type{};
            }

            std::span<const_pointer const> as_ptr_span() const noexcept {
                using type = std::span<const_pointer const>;
                return array_ ? type{array_->list, array_->size} : type{};
            }

            std::span<pointer const> as_ptr_span(size_type const sz) noexcept {
                auto ptrs = as_ptr_span();
                return ptrs.subspan(0, std::min(ptrs.size(), sz));
            }

            std::span<const_pointer const> as_ptr_span(size_type const sz) const noexcept {
                auto ptrs = as_ptr_span();
                return ptrs.subspan(0, std::min(ptrs.size(), sz));
            }

        private:
            explicit basic_view(array const *arr) noexcept : array_{arr} {
            }

            template <bool ThatC, bool ThisC = Const, type_traits::other_trans::enable_if_t<!ThatC && ThisC, int> = 0>
            explicit basic_view(basic_view<ThatC> that) noexcept : array_{that.array_} {
            }

            array const *array_{};
        };

        class iterator;
        class const_iterator;

        atomic_grow_array() = default;
        
        explicit atomic_grow_array(Policy const &policy_)
            noexcept(noexcept(Policy{policy_})) : Policy{policy_} {
        }

        ~atomic_grow_array() {
            reset();
        }

        Policy const &policy() const noexcept {
            return static_cast<Policy const &>(*this);
        }

        size_t size() const noexcept {
            return size_.load(mo_acquire);
        }

        bool empty() const noexcept {
            return size() == 0;
        }

        RAINY_ALWAYS_INLINE value_type &operator[](size_type const index) noexcept(is_nothrow_grow_v) {
            auto const p = array_.load(mo_acquire);
            array *arr = (p && index < p->size) ? p : at_slow(index);
            auto cur = size_.load(std::memory_order_relaxed);
            while (cur <= index) {
                if (size_.compare_exchange_strong(cur, index + 1, std::memory_order_release, std::memory_order_relaxed)) {
                    arr->size = index + 1;
                    break;
                }
            }
            return *arr->list[index];
        }


        class iterator : public basic_iterator<false, iterator> {
        public:
            using base = basic_iterator<false, iterator>;
            friend base;
            friend const_iterator;
            template <bool>
            friend class basic_view;

            using typename base::difference_type;
            using typename base::iterator_category;
            using typename base::pointer;
            using typename base::reference;
            using typename base::value_type;

            using base::base;
            using base::operator++;
            using base::operator+;
            using base::operator+=;
            using base::operator-;
            using base::operator-=;
            using base::operator*;
            using base::operator[];
        };

        class const_iterator : public basic_iterator<true, const_iterator> {
        public:
            using base = basic_iterator<true, const_iterator>;
            friend base;
            template <bool>
            friend class basic_view;

            using typename base::difference_type;
            using typename base::iterator_category;
            using typename base::pointer;
            using typename base::reference;
            using typename base::value_type;

            using base::base;
            using base::operator++;
            using base::operator+;
            using base::operator+=;
            using base::operator-;
            using base::operator-=;
            using base::operator*;
            using base::operator[];

            const_iterator(iterator that) noexcept : base{that} {
            }
        };

        class view : private basic_view<false> {
        public:
            friend atomic_grow_array;
            using base = basic_view<false>;

            using typename base::const_iterator;
            using typename base::const_reference;
            using typename base::iterator;
            using typename base::reference;
            using typename base::size_type;
            using typename base::value_type;

            using base::as_ptr_span;
            using base::base;
            using base::begin;
            using base::cbegin;
            using base::cend;
            using base::empty;
            using base::end;
            using base::size;
            using base::operator[];
        };

        class const_view : private basic_view<true> {
        public:
            friend atomic_grow_array;
            using base = basic_view<true>;

            using typename base::const_iterator;
            using typename base::const_reference;
            using typename base::iterator;
            using typename base::reference;
            using typename base::size_type;
            using typename base::value_type;

            using base::as_ptr_span;
            using base::base;
            using base::begin;
            using base::cbegin;
            using base::cend;
            using base::empty;
            using base::end;
            using base::size;
            using base::operator[];

            const_view(view that) noexcept : base{that} {
            }
        };

        view as_view() noexcept {
            return view{array_.load(mo_acquire)};
        }
        
        const_view as_view() const noexcept {
            return view{array_.load(mo_acquire)};
        }

        pointer_span as_ptr_span() noexcept {
            return as_view().as_ptr_span();
        }
        
        const_pointer_span as_ptr_span() const noexcept {
            return as_view().as_ptr_span();
        }

        pointer_span as_ptr_span(size_type const sz) noexcept {
            return as_view().as_ptr_span(sz);
        }
        
        const_pointer_span as_ptr_span(size_type const sz) const noexcept {
            return as_view().as_ptr_span(sz);
        }

    private:
        static constexpr auto mo_acquire = std::memory_order_acquire;
        static constexpr auto mo_release = std::memory_order_release;
        static constexpr auto mo_acq_rel = std::memory_order_acq_rel;
        static constexpr bool is_nothrow_grow_v = noexcept(utility::declval<Policy const &>().grow(0, 0)) &&
                                                  noexcept(utility::declval<Policy const &>().make()) && noexcept(::operator new(0));

        struct array {
            array *next{};
            size_type size{};
            size_type capacity{};
            value_type **list{};
            value_type *slab{};
        };

        array *at_slow(size_type const index) noexcept(is_nothrow_grow_v) {
            array *p = array_.load(mo_acquire);
            if (p && index < p->capacity)
                return p;
            array *q = nullptr;
            size_type const capacity = policy().grow(p ? p->size : 0, index);
            assert(index < capacity);
            do {
                if (p && index < p->size) {
                    return p;
                }
                q = new_array(capacity, p);
                if (!q) {
                    continue;
                }
                if (array_.compare_exchange_strong(p, q, mo_acq_rel, mo_acquire)) {
                    return q;
                }
                del_array(q);
            } while (1);
        }

        static constexpr size_type array_align() {
            return (core::max)(sizeof(std::max_align_t), alignof(value_type));
        }

        static size_type array_size(size_type const size, size_type const base) {
            constexpr auto a = array_align();
            return core::builtin::ceil(sizeof(array) + size * sizeof(value_type *), a) +
                   core::builtin::ceil((size - base) * sizeof(value_type), a);
        }

        array *new_array(size_type const size, array *&next) {
            auto const base = next ? next->size : 0;
            assert(size > base);
            array *curr = new array;
            auto rollback = utility::make_scope_guard(foundation::functional::bind(&atomic_grow_array::del_array, this, curr));
            curr->next = next;
            curr->list =
                static_cast<value_type **>(::operator new[](size * sizeof(value_type *), std::align_val_t{alignof(value_type *)}));
            curr->slab =
                static_cast<value_type *>(::operator new[]((size - base) * sizeof(value_type), std::align_val_t{alignof(value_type)}));
            curr->capacity = size;
            curr->size = next ? next->size : 0;
            for (size_type i = 0; i < base; ++i) {
                curr->list[i] = next->list[i];
            }
            for (size_type i = base; i < size; ++i) {
                curr->list[i] = nullptr;
            }
            for (size_type i = base; i < size; ++i) {
                if (auto const p = array_.load(std::memory_order_acquire); p != next) {
                    next = p;
                    return nullptr;
                }
                curr->list[i] = ::new (&curr->slab[i - base]) value_type(policy().make());
            }
            rollback.dismiss();
            return curr;
        }

        void del_array(array *const curr) {
            assert(curr);
            auto size = curr->size;
            auto const next = curr->next;
            auto const base = next ? next->size : 0;
            assert(size > base);
            while (size > base && !curr->list[size - 1]) {
                --size;
            }
            for (size_type i = 0; i < size - base; ++i) {
                curr->list[size - 1 - i]->~value_type();
            }
            ::operator delete[](static_cast<void *>(curr->slab), (curr->size - base) * sizeof(value_type),
                                std::align_val_t{alignof(value_type)});
            ::operator delete[](static_cast<void *>(curr->list), curr->size * sizeof(value_type *),
                                std::align_val_t{alignof(value_type *)});
            delete curr;
        }

        void reset() {
            auto curr = array_.load(mo_acquire);
            while (curr) {
                auto const next = curr->next;
                del_array(curr);
                curr = next;
            }
        }

        typename Policy::template atom<size_type> size_{0};
        typename Policy::template atom<array *> array_{nullptr};
    };
}

#endif