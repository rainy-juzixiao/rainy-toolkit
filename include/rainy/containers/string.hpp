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
#ifndef RAINY_CONTAINERS_STRING_HPP
#define RAINY_CONTAINERS_STRING_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/foundation/system/memory/allocator.hpp>
#include <rainy/text/char_traits.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/utility/pair.hpp>

namespace rainy::containers {
    template <typename CharType, typename Traits, typename Alloc>
    class basic_string;
}

namespace rainy::containers::internals {
    template <typename CharType, typename Traits, typename Alloc>
    class basic_string_storage {
    public:
        template <typename, typename, typename>
        friend class containers::basic_string;

        using value_type = CharType;
        using traits_type = Traits;
        using allocator_type = Alloc;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = utility::iterator<pointer>;
        using const_iterator = utility::const_iterator<const_pointer>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        basic_string_storage() = default;

        basic_string_storage(const allocator_type &alloc) : instance_data(alloc, {}) {};

#pragma region basic_string_properites_query

        RAINY_CONSTEXPR20 allocator_type get_allocator() noexcept {
            return this->instance_data.get_first();
        }

        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return this->instance_data.get_first();
        }

        RAINY_CONSTEXPR20 pointer data() noexcept {
            return ptr();
        }

        RAINY_CONSTEXPR20 const_pointer data() const noexcept {
            return ptr();
        }

        RAINY_CONSTEXPR20 const_pointer c_str() const noexcept {
            return ptr();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool empty() const noexcept {
            return size() == 0;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type size() const noexcept {
            return size_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type length() const {
            return size_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return (std::numeric_limits<size_type>::max)();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            return capacity_;
        }

        RAINY_CONSTEXPR20 reference operator[](size_type pos) noexcept {
            return ptr()[pos];
        }

        RAINY_CONSTEXPR20 const_reference operator[](size_type pos) const noexcept {
            return ptr()[pos];
        }

        RAINY_CONSTEXPR20 reference at(size_type pos) {
            throw_if_out_range(pos);
            return ptr()[pos];
        }

        RAINY_CONSTEXPR20 const_reference at(size_type pos) const {
            throw_if_out_range(pos);
            return ptr()[pos];
        }

        const_reference back() const noexcept {
            return ptr()[size() - 1];
        }

        reference back() noexcept {
            return ptr()[size() - 1];
        }

        const_reference front() const {
            return ptr()[0];
        }

        reference front() {
            return ptr()[0];
        }

        iterator begin() noexcept {
            return iterator{ptr()};
        }

        const_iterator begin() const noexcept {
            return const_iterator{ptr()};
        }

        iterator end() noexcept {
            return iterator{ptr() + size()};   
        }

        const_iterator end() const noexcept {
            return const_iterator{ptr() + size()};
        }

        reverse_iterator rbegin() noexcept {
            return reverse_iterator{end()};
        }

        const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator{end()};
        }

        reverse_iterator rend() noexcept {
            return reverse_iterator{begin()};
        }

        const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator{begin()};
        }

        const_iterator cbegin() const noexcept {
            return const_iterator{ptr()};
        }
        
        const_iterator cend() const noexcept {
            return const_iterator{ptr() + capacity()};
        }

        const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator{end()};
        }

        const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator{begin()};
        }

#pragma endregion
    protected:
        RAINY_NODISCARD RAINY_CONSTEXPR20 bool is_local() const noexcept {
            return capacity_ < SMALL_BUFFER_SIZE;
        }

        RAINY_CONSTEXPR20 void activate_sso_buffer() {
            auto &storage = get_storage();
#if RAINY_HAS_CXX20
            if (!std::is_constant_evaluated()) {
                for (size_type i = 0; i < SMALL_BUFFER_SIZE; ++i) {
                    storage.sso_buffer[i] = value_type{};
                }
                return;
            }
            else // NOLINT
#endif
            {
                for (size_type i = 0; i < SMALL_BUFFER_SIZE; ++i) {
                    storage.sso_buffer[i] = value_type{};
                }
            }
        }

        RAINY_CONSTEXPR20 auto &get_storage() noexcept {
            return instance_data.get_second();
        }

        RAINY_CONSTEXPR20 const auto &get_storage() const noexcept {
            return instance_data.get_second();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const value_type *ptr() const noexcept {
            auto &storage = get_storage();
            const value_type *result = storage.sso_buffer;
            if (!is_local()) {
                result = storage.ptr;
            }
            return result;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 value_type *ptr() noexcept {
            return const_cast<value_type *>(static_cast<const basic_string_storage *>(this)->ptr());
        }

        RAINY_CONSTEXPR20 void throw_if_out_range(const size_type idx) {
            if (rainy_unlikely(idx >= size())) { // 很多时候，不应该发生
                foundation::system::exceptions::logic::throw_out_of_range("Invalid subscript");
            }
        }

    private:
        static constexpr size_type SMALL_BUFFER_SIZE = 16 / sizeof(value_type) < 1 ? 1 : 16 / sizeof(value_type);
        static constexpr size_type SMALL_STRING_CAPACITY = SMALL_BUFFER_SIZE - 1;

        union resources {
            RAINY_CONSTEXPR20 resources() noexcept : ptr() {
            }
            RAINY_CONSTEXPR20 ~resources() noexcept {
            }

            pointer ptr;
            value_type sso_buffer[SMALL_BUFFER_SIZE];
        };

        size_type capacity_{0};
        size_type size_{0};

        utility::compressed_pair<allocator_type, resources> instance_data;
    };
}

namespace rainy::containers {
    template <typename CharType, typename Traits = text::char_traits<CharType>, typename Alloc = foundation::system::memory::allocator<CharType>>
    class basic_string : public internals::basic_string_storage<CharType, Traits, Alloc> {
    public:
        using base_impl = internals::basic_string_storage<CharType, Traits, Alloc>;
        using value_type = typename base_impl::value_type;
        using traits_type = typename base_impl::traits_type;
        using allocator_type = typename base_impl::allocator_type;
        using reference = typename base_impl::reference;
        using const_reference = typename base_impl::const_reference;
        using pointer = typename base_impl::pointer;
        using const_pointer = typename base_impl::const_pointer;
        using size_type = typename base_impl::size_type;
        using difference_type = typename base_impl::difference_type;
        using iterator = typename base_impl::iterator;
        using const_iterator = typename base_impl::const_iterator;
        using reverse_iterator = typename base_impl::reverse_iterator;
        using const_reverse_iterator = typename base_impl::const_reverse_iterator;

        static inline constexpr size_type npos = static_cast<size_type>(-1);

#pragma region basic_string_constrctor
        RAINY_CONSTEXPR20 basic_string() : base_impl{} {
            construct_string<construct_strategy::empty>(nullptr, 0);
        }

        RAINY_CONSTEXPR20 basic_string(const allocator_type &alloc) : base_impl(alloc) {
            construct_string<construct_strategy::empty>(nullptr, 0);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right) {
            construct_string<construct_strategy::from_contaienr>(right.ptr(), right.size());
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right) noexcept {
            this->instance_data = std::move(right.instance_data);
            this->size_ = right.size_;
            this->capacity_ = right.capacity_;
            right.size_ = 0;
            right.capacity_ = 0;
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type right_offset, size_type count = npos) {
            if (right_offset > right.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            size_type real_count = (std::min)(count, right.size() - right_offset);
            construct_string<construct_strategy::from_ptr>(right.data() + right_offset, real_count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type right_offset, size_type count,
                                       const allocator_type &alloc) : base_impl(alloc) {
            if (right_offset > right.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            size_type real_count = (std::min)(count, right.size() - right_offset);
            construct_string<construct_strategy::from_ptr>(right.data() + right_offset, real_count);
        }

        RAINY_CONSTEXPR20 basic_string(const_pointer str, size_type count) {
            construct_string<construct_strategy::from_ptr>(str, count);
        }

        RAINY_CONSTEXPR20 basic_string(const_pointer str, size_type count, const allocator_type &alloc) : base_impl(alloc) {
            construct_string<construct_strategy::from_ptr>(str, count);
        }

        RAINY_CONSTEXPR20 basic_string(const_pointer str) : basic_string(str, traits_type::length(str)) {
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, const allocator_type &alloc) : base_impl(alloc) {
            construct_string<construct_strategy::from_contaienr>(right.ptr(), right.size());
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right, const allocator_type &alloc) : base_impl(alloc) {
            if (alloc == right.get_allocator()) {
                this->instance_data.get_second() = std::move(right.instance_data.get_second());
                this->size_ = right.size_;
                this->capacity_ = right.capacity_;
                right.size_ = 0;
                right.capacity_ = 0;
            } else {
                construct_string<construct_strategy::from_contaienr>(right.ptr(), right.size());
            }
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::templates::is_iterator_v<Iter>,int> = 0>
        RAINY_CONSTEXPR20 basic_string(Iter first, Iter last) {
            std::size_t count = utility::distance(first, last);
            construct_string<construct_strategy::from_ptr>(&(*first), count);
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::templates::is_iterator_v<Iter>, int> = 0>
        RAINY_CONSTEXPR20 basic_string(Iter first, Iter last, const allocator_type &alloc) : base_impl(alloc) {
            std::size_t count = std::distance(first, last);
            construct_string<construct_strategy::from_ptr>(&(*first), count);
        }

        RAINY_CONSTEXPR20 basic_string(size_type count, value_type ch) {
            construct_string<construct_strategy::from_char>(ch, count);
        }

        RAINY_CONSTEXPR20 basic_string(size_type count, value_type ch, const allocator_type &alloc) : base_impl(alloc) {
            construct_string<construct_strategy::from_char>(ch, count);
        }

        RAINY_CONSTEXPR20 basic_string(std::initializer_list<value_type> ilist) {
            construct_string<construct_strategy::from_ptr>(ilist.begin(), ilist.size());
        }

        RAINY_CONSTEXPR20 basic_string(std::initializer_list<value_type> ilist, const allocator_type &alloc) : base_impl(alloc) {
            construct_string<construct_strategy::from_ptr>(ilist.begin(), ilist.size());
        }

        RAINY_CONSTEXPR20 basic_string(const std::basic_string<CharType, Traits, Alloc> &standard_container) {
            construct_string<construct_strategy::from_contaienr>(standard_container.data(), standard_container.size());
        }

        RAINY_CONSTEXPR20 ~basic_string() {
            tidy_helper();
        }
#pragma endregion

#pragma region basic_string_assign

        RAINY_CONSTEXPR20 basic_string &assign(const_pointer str) {
            return assign(str, traits_type::length(str));
        }

        RAINY_CONSTEXPR20 basic_string &assign(const_pointer str, size_type count) {
            if (count <= this->capacity()) {
                traits_type::copy(this->ptr(), str, count);
                this->size_ = count;
                traits_type::assign(this->ptr()[count], value_type{});
                return *this;
            }
            auto pair = allocate_for_new_memory(count);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::copy(new_ptr, str, count);
            traits_type::assign(new_ptr[count], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = count;
            return *this;
        }

        basic_string &assign(const basic_string &right) {
            if (this != &right) {
                return assign(right.data(), right.size());
            }
            return *this;
        }

        basic_string &assign(basic_string &&right) noexcept {
            if (this != &right) {
                tidy_helper();
                this->size_ = right.size_;
                this->capacity_ = right.capacity_;
                if (right.is_local()) {
                    traits_type::copy(this->ptr(), right.data(), right.size());
                    traits_type::assign(this->ptr()[right.size()], value_type{});
                } else {
                    this->get_storage().ptr = right.get_storage().ptr;
                    right.get_storage().ptr = nullptr;
                }
                right.size_ = 0;
                right.capacity_ = 0;
            }
            return *this;
        }

        template <typename InputIterator>
        basic_string &assign(InputIterator first, InputIterator last) {
            size_type count = std::distance(first, last);
            if (count <= this->capacity()) {
                algorithm::container_operater::copy(first, last, this->ptr());
                this->size_ = count;
                traits_type::assign(this->ptr()[count], value_type{});
                return *this;
            }
            auto pair = allocate_for_new_memory(count);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            algorithm::container_operater::copy(first, last, new_ptr);
            traits_type::assign(new_ptr[count], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = count;
            return *this;
        }

        basic_string &assign(size_type count, value_type ch) {
            if (count <= this->capacity()) {
                traits_type::assign(this->ptr(), count, ch);
                this->size_ = count;
                traits_type::assign(this->ptr()[count], value_type{});
                return *this;
            }
            auto pair = allocate_for_new_memory(count);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::assign(new_ptr, count, ch);
            traits_type::assign(new_ptr[count], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = count;
            return *this;
        }

        basic_string &assign(const_pointer str, size_type count, const allocator_type &alloc) {
            if (alloc == this->get_allocator()) {
                return assign(str, count);
            }
            basic_string tmp(str, count, alloc);
            *this = utility::move(tmp);
            return *this;
        }

        basic_string &assign(const allocator_type &alloc) {
            if (alloc == this->get_allocator()) {
                tidy_helper();
                this->size_ = 0;
                this->capacity_ = 0;
                return *this;
            }
            basic_string tmp(alloc);
            *this = utility::move(tmp);
            return *this;
        }

#pragma endregion

#pragma region basic_string_append
        basic_string &append(const_pointer str) {
            return append(str, traits_type::length(str));
        }

        basic_string &append(const_pointer str, size_type count) {
            size_type new_size = this->size_ + count;
            if (new_size <= this->capacity()) {
                traits_type::copy(this->ptr() + this->size_, str, count);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }
            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::copy(new_ptr, this->data(), this->size_);
            traits_type::copy(new_ptr + this->size_, str, count);
            traits_type::assign(new_ptr[new_size], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;
            return *this;
        }

        basic_string &append(const basic_string &right, size_type offset, size_type count) {
            if (offset > right.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            return append(right.data() + offset, (std::min)(count, right.size() - offset));
        }

        basic_string &append(const basic_string &right) {
            return append(right.data(), right.size());
        }

        basic_string &append(size_type count, value_type ch) {
            size_type new_size = this->size_ + count;
            if (new_size <= this->capacity()) {
                traits_type::assign(this->ptr() + this->size_, count, ch);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }
            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::copy(new_ptr, this->data(), this->size_);
            traits_type::assign(new_ptr + this->size_, count, ch);
            traits_type::assign(new_ptr[new_size], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;
            return *this;
        }

        template <typename InputIterator>
        basic_string &append(InputIterator first, InputIterator last) {
            size_type count = utility::distance(first, last);
            size_type new_size = this->size_ + count;
            if (new_size <= this->capacity()) {
                std::copy(first, last, this->ptr() + this->size_);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }

            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;

            traits_type::copy(new_ptr, this->data(), this->size_);
            std::copy(first, last, new_ptr + this->size_);
            traits_type::assign(new_ptr[new_size], value_type{});

            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;

            return *this;
        }

        basic_string &append(const_pointer first, const_pointer last) {
            return append(first, static_cast<size_type>(last - first));
        }

        basic_string &append(const_iterator first, const_iterator last) {
            return append(first, static_cast<size_type>(last - first));
        }
#pragma endregion

#pragma region basic_string_compare
        int compare(const basic_string &right) const {
            const size_type this_size = this->size();
            const size_type str_size = right.size();
            const size_type rlen = (std::min)(this_size, str_size);
            return traits_type::compare(this->data(), right.data(), rlen);
        }

        int compare(size_type pos, size_type count, const basic_string &right) const {
            if (pos > this->size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count = (std::min)(count, this->size() - pos);
            return substr(pos, count).compare(right);
        }

        int compare(size_type pos, size_type count1, const basic_string &right, size_type offset, size_type count2) const {
            if (pos > this->size() || offset > right.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count1 = (std::min)(count1, this->size() - pos);
            count2 = (std::min)(count2, right.size() - offset);
            return substr(pos, count1).compare(right.substr(offset, count2));
        }

        int compare(const value_type *ptr) const {
            return compare(basic_string(ptr));
        }

        int compare(size_type pos, size_type count1, const value_type *ptr) const {
            if (pos > this->size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count1 = (std::min)(count1, this->size() - pos);
            return substr(pos, count1).compare(ptr);
        }

        int compare(size_type pos, size_type count1, const value_type *ptr, size_type count2) const {
            if (pos > this->size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count1 = (std::min)(count1, this->size() - pos);
            return substr(pos, count1).compare(basic_string(ptr, count2));
        }

        template <typename StringViewLike>
        int compare(const StringViewLike &t) const
            noexcept(std::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>>) {
            std::basic_string_view<CharType, Traits> sv = t;
            const size_type this_size = this->size();
            const size_type sv_size = sv.size();
            const size_type rlen = (std::min)(this_size, sv_size);
            return traits_type::compare(this->data(), sv.data(), rlen);
        }

        template <typename StringViewLike>
        int compare(size_type pos1, size_type count1, const StringViewLike &string_view) const {
            if (pos1 > this->size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            std::basic_string_view<CharType, Traits> sv = string_view;
            count1 = (std::min)(count1, this->size() - pos1);
            return substr(pos1, count1).compare(basic_string(sv.data(), sv.size()));
        }

        template <typename StringViewLike>
        int compare(size_type pos1, size_type count1, const StringViewLike &string_view, size_type pos2,
                    size_type count2 = npos) const {
            if (pos1 > this->size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            std::basic_string_view<CharType, Traits> sv = string_view;
            if (pos2 > sv.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count1 = (std::min)(count1, this->size() - pos1);
            count2 = (std::min)(count2, sv.size() - pos2);
            return substr(pos1, count1).compare(basic_string(sv.data() + pos2, count2));
        }
#pragma endregion

#pragma region basic_string_erase
        iterator erase(iterator first, iterator last) {
            if (first == last) {
                return first;
            }
            size_type pos = first - this->ptr();
            size_type len = last - first;
            size_type remaining = this->size_ - (pos + len);
            traits_type::move(this->ptr() + pos, this->ptr() + pos + len, remaining);
            this->size_ -= len;
            traits_type::assign(this->ptr()[this->size_], value_type{});
            return this->ptr() + pos;
        }

        iterator erase(iterator iter) {
            return erase(iter, iter + 1);
        }

        basic_string &erase(size_type offset = 0, size_type count = npos) {
            if (offset > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            count = (std::min)(count, this->size_ - offset);
            size_type remaining = this->size_ - (offset + count);
            traits_type::move(this->ptr() + offset, this->ptr() + offset + count, remaining);
            this->size_ -= count;
            traits_type::assign(this->ptr()[this->size_], value_type{});
            return *this;
        }
#pragma endregion

#pragma region basic_string_copy
        size_type copy(value_type *ptr, size_type count, size_type offset = 0) const {
            if (offset > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            size_type len = (std::min)(count, this->size_ - offset);
            traits_type::copy(ptr, this->data() + offset, len);
            return len;
        }

        size_type copy(value_type ch, size_type count, size_type offset = 0) {
            if (offset > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            size_type len = (std::min)(count, this->size_ - offset);
            traits_type::assign(this->ptr() + offset, len, ch);
            traits_type::assign(this->ptr()[this->size_], value_type{});
            return len;
        }

        size_type copy(const basic_string &right, size_type count = 0, size_type offset = 0) {
            if (offset > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            count = count == 0 ? right.size() : count;
            size_type len = (std::min)(count, this->size_ - offset);
            traits_type::copy(this->ptr() + offset, right.data(), len);
            traits_type::assign(this->ptr()[this->size_], value_type{});
            return len;
        }
#pragma endregion

#pragma region basic_string_replace

        basic_string &replace(size_type pos1, size_type count1, const value_type *ptr) {
            return replace(pos1, count1, ptr, traits_type::length(ptr));
        }

        basic_string &replace(size_type pos1, size_type count1, const basic_string &right) {
            return replace(pos1, count1, right.data(), right.size());
        }

        basic_string &replace(size_type pos1, size_type count1, const value_type *ptr, size_type count2) {
            if (pos1 > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count1 = (std::min)(count1, this->size_ - pos1);
            size_type new_size = this->size_ - count1 + count2;
            if (new_size <= this->capacity_) {
                if (count1 != count2) {
                    traits_type::move(this->ptr() + pos1 + count2, this->ptr() + pos1 + count1, this->size_ - pos1 - count1);
                }
                traits_type::copy(this->ptr() + pos1, ptr, count2);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }
            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::copy(new_ptr, this->data(), pos1);
            traits_type::copy(new_ptr + pos1, ptr, count2);
            traits_type::copy(new_ptr + pos1 + count2, this->data() + pos1 + count1, this->size_ - pos1 - count1);
            traits_type::assign(new_ptr[new_size], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;
            return *this;
        }

        basic_string &replace(size_type pos1, size_type count1, const basic_string &str, size_type pos2, size_type count2) {
            if (pos2 > str.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count2 = (std::min)(count2, str.size() - pos2);
            return replace(pos1, count1, str.data() + pos2, count2);
        }

        basic_string &replace(size_type pos1, size_type count1, size_type count, value_type ch) {
            if (pos1 > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }
            count1 = (std::min)(count1, this->size_ - pos1);

            size_type new_size = this->size_ - count1 + count;
            if (new_size <= this->capacity_) {
                if (count1 != count) {
                    traits_type::move(this->ptr() + pos1 + count, this->ptr() + pos1 + count1, this->size_ - pos1 - count1);
                }
                traits_type::assign(this->ptr() + pos1, count, ch);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }

            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;

            traits_type::copy(new_ptr, this->data(), pos1);
            traits_type::assign(new_ptr + pos1, count, ch);
            traits_type::copy(new_ptr + pos1 + count, this->data() + pos1 + count1, this->size_ - pos1 - count1);
            traits_type::assign(new_ptr[new_size], value_type{});

            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;
            return *this;
        }

        basic_string &replace(iterator begin, iterator end, const value_type *ptr) {
            return replace(begin - this->ptr(), end - begin, ptr);
        }

        basic_string &replace(iterator begin, iterator end, const basic_string &right) {
            return replace(begin - this->ptr(), end - begin, right);
        }

        basic_string &replace(iterator begin, iterator end, const value_type *ptr, size_type count) {
            return replace(begin - this->ptr(), end - begin, ptr, count);
        }

        basic_string &replace(iterator begin, iterator end, size_type count, value_type ch) {
            return replace(begin - this->ptr(), end - begin, count, ch);
        }

        template <typename InputIterator>
        basic_string &replace(iterator begin1, iterator end1, InputIterator begin2, InputIterator end2) {
            return replace(begin1 - this->ptr(), end1 - begin1, &(*begin2), std::distance(begin2, end2));
        }

        basic_string &replace(iterator begin1, iterator end1, const_pointer begin2, const_pointer end2) {
            return replace(begin1 - this->ptr(), end1 - begin1, begin2, end2 - begin2);
        }

        basic_string &replace(iterator begin1, iterator end1, const_iterator begin2, const_iterator end2) {
            return replace(begin1 - this->ptr(), end1 - begin1, begin2, end2 - begin2);
        }

#pragma endregion

#pragma region basic_string_starts_with_and_ends_with
        bool starts_with(const value_type ch) const noexcept {
            if (this->empty()) {
                return false;
            }
            return traits_type::eq(&this->front(), ch);
        }

        bool starts_with(const CharType *const str) const noexcept {
            size_type str_len = traits_type::length(str);
            if (str_len > this->size_) {
                return false;
            }
            return traits_type::compare(&this->front(), str, str_len) == 0;
        }

        bool starts_with(const std::basic_string_view<CharType, Traits> sv) const noexcept {
            if (sv.size() > this->size_) {
                return false;
            }
            return traits_type::compare(&this->front(), sv.data(), sv.size()) == 0;
        }


        bool ends_with(const value_type ch) const noexcept {
            if (this->empty()) {
                return false;
            }
            return traits_type::eq(&this->back(), ch);
        }

        bool ends_with(const CharType *const str) const noexcept {
            size_type str_len = traits_type::length(str);
            if (str_len > this->size_) {
                return false;
            }
            return traits_type::compare(&this->back() - str_len + 1, str, str_len) == 0;
        }

        bool ends_with(const std::basic_string_view<CharType, Traits> sv) const noexcept {
            if (sv.size() > this->size_) {
                return false;
            }
            return traits_type::compare(&this->back() - sv.size() + 1, sv.data(), sv.size()) == 0;
        }

#pragma endregion

#pragma region basic_string_find
        size_type find(value_type ch, size_type offset = 0) const {
            if (offset > this->size_) {
                return npos;
            }
            const value_type *ptr = traits_type::find(this->ptr() + offset, this->size_ - offset, ch);
            return ptr ? ptr - this->ptr() : npos;
        }

        size_type find(const value_type *str, size_type offset = 0) const {
            return find(str, offset, traits_type::length(str));
        }

        size_type find(const value_type *str, size_type offset, size_type count) const {
            if (str == nullptr || count == 0) {
                return offset <= this->size_ ? offset : npos;
            }
            if (offset > this->size_) {
                return npos;
            }
            if (count > this->size_ - offset) {
                return npos;
            }
            const value_type *ptr = this->ptr() + offset;
            const value_type *end = this->ptr() + this->size_ - count + 1;
            for (; ptr < end; ++ptr) {
                if (traits_type::compare(ptr, str, count) == 0) {
                    return ptr - this->ptr();
                }
            }
            return npos;
        }

        size_type find(const basic_string &right, size_type offset = 0) const {
            return find(right.data(), offset, right.size());
        }

        size_type find_first_of(value_type ch, size_type offset = 0) const {
            return find(ch, offset);
        }

        size_type find_first_of(const value_type *str, size_type offset = 0) const {
            return find_first_of(str, offset, traits_type::length(str));
        }

        size_type find_first_of(const value_type *str, size_type offset, size_type count) const {
            if (str == nullptr || count == 0 || offset > this->size_) {
                return npos;
            }
            for (size_type i = offset; i < this->size_; ++i) {
                const value_type *ptr = traits_type::find(str, count, this->ptr()[i]);
                if (ptr) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_first_of(const basic_string &right, size_type offset = 0) const {
            return find_first_of(right.data(), offset, right.size());
        }

        size_type find_first_not_of(value_type ch, size_type offset = 0) const {
            if (offset > this->size_) {
                return npos;
            }
            for (size_type i = offset; i < this->size_; ++i) {
                if (!traits_type::eq(this->ptr()[i], ch)) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_first_not_of(const value_type *str, size_type offset = 0) const {
            return find_first_not_of(str, offset, traits_type::length(str));
        }

        size_type find_first_not_of(const value_type *str, size_type offset, size_type count) const {
            if (offset > this->size_) {
                return npos;
            }
            if (str == nullptr || count == 0) {
                return offset;
            }

            for (size_type i = offset; i < this->size_; ++i) {
                if (!traits_type::find(str, count, this->ptr()[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_first_not_of(const basic_string &right, size_type offset = 0) const {
            return find_first_not_of(right.data(), offset, right.size());
        }

        size_type find_last_of(value_type ch, size_type offset = npos) const {
            if (this->empty()) {
                return npos;
            }

            if (offset >= this->size_) {
                offset = this->size_ - 1;
            }

            for (size_type i = offset + 1; i > 0;) {
                --i;
                if (traits_type::eq(this->ptr()[i], ch)) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_last_of(const value_type *str, size_type offset = npos) const {
            return find_last_of(str, offset, traits_type::length(str));
        }

        size_type find_last_of(const value_type *str, size_type offset, size_type count) const {
            if (this->empty() || str == nullptr || count == 0) {
                return npos;
            }

            if (offset >= this->size_) {
                offset = this->size_ - 1;
            }

            for (size_type i = offset + 1; i > 0;) {
                --i;
                if (traits_type::find(str, count, this->ptr()[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_last_of(const basic_string &right, size_type offset = npos) const {
            return find_last_of(right.data(), offset, right.size());
        }

        size_type find_last_not_of(value_type ch, size_type offset = npos) const {
            if (this->empty()) {
                return npos;
            }

            if (offset >= this->size_) {
                offset = this->size_ - 1;
            }

            for (size_type i = offset + 1; i > 0;) {
                --i;
                if (!traits_type::eq(this->ptr()[i], ch)) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_last_not_of(const value_type *str, size_type offset = npos) const {
            return find_last_not_of(str, offset, traits_type::length(str));
        }

        size_type find_last_not_of(const value_type *str, size_type offset, size_type count) const {
            if (this->empty()) {
                return npos;
            }

            if (offset >= this->size_) {
                offset = this->size_ - 1;
            }

            if (str == nullptr || count == 0) {
                return offset;
            }

            for (size_type i = offset + 1; i > 0;) {
                --i;
                if (!traits_type::find(str, count, this->ptr()[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_last_not_of(const basic_string &right, size_type offset = npos) const {
            return find_last_not_of(right.data(), offset, right.size());
        }

        size_type rfind(value_type ch, size_type offset = npos) const {
            if (this->empty()) {
                return npos;
            }

            if (offset >= this->size_) {
                offset = this->size_ - 1;
            }

            for (size_type i = offset + 1; i > 0;) {
                --i;
                if (traits_type::eq(this->ptr()[i], ch)) {
                    return i;
                }
            }
            return npos;
        }

        size_type rfind(const value_type *str, size_type offset = npos) const {
            return rfind(str, offset, traits_type::length(str));
        }

        size_type rfind(const value_type* str, size_type offset, size_type count) const {
            if (count == 0) {
                return (offset <= this->size_) ? offset : this->size_;
            }
            if (count <= this->size_) {
                offset = (std::min)(offset, this->size_ - count);
                for (size_type i = offset + 1; i > 0;) {
                    --i;
                    if (traits_type::compare(this->data() + i, str, count) == 0) {
                        return i;
                    }
                }
            }
            return npos;
        }


        size_type rfind(const basic_string &right, size_type offset = npos) const {
            return rfind(right.data(), offset, right.size());
        }

#pragma endregion

#pragma region basic_string_insert
        basic_string &insert(size_type position, const value_type *ptr) {
            return insert(position, ptr, traits_type::length(ptr));
        }

        basic_string &insert(size_type position, const value_type *ptr, size_type count) {
            if (position > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }

            size_type new_size = this->size_ + count;
            if (new_size <= this->capacity_) {
                traits_type::move(this->ptr() + position + count, this->ptr() + position, this->size_ - position);
                traits_type::copy(this->ptr() + position, ptr, count);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }

            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;

            traits_type::copy(new_ptr, this->data(), position);
            traits_type::copy(new_ptr + position, ptr, count);
            traits_type::copy(new_ptr + position + count, this->data() + position, this->size_ - position);
            traits_type::assign(new_ptr[new_size], value_type{});

            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;
            return *this;
        }

        basic_string &insert(size_type position, const basic_string &str) {
            return insert(position, str.data(), str.size());
        }

        basic_string &insert(size_type position, const basic_string &str, size_type offset, size_type count) {
            if (offset > str.size()) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid offset");
            }
            count = (std::min)(count, str.size() - offset);
            return insert(position, str.data() + offset, count);
        }

        basic_string &insert(size_type position, size_type count, value_type char_value) {
            if (position > this->size_) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid position");
            }

            size_type new_size = this->size_ + count;
            if (new_size <= this->capacity_) {
                traits_type::move(this->ptr() + position + count, this->ptr() + position, this->size_ - position);
                traits_type::assign(this->ptr() + position, count, char_value);
                this->size_ = new_size;
                traits_type::assign(this->ptr()[new_size], value_type{});
                return *this;
            }

            auto pair = allocate_for_new_memory(new_size);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;

            traits_type::copy(new_ptr, this->data(), position);
            traits_type::assign(new_ptr + position, count, char_value);
            traits_type::copy(new_ptr + position + count, this->data() + position, this->size_ - position);
            traits_type::assign(new_ptr[new_size], value_type{});

            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = new_size;
            return *this;
        }

        iterator insert(iterator iter, value_type char_value) {
            size_type pos = iter - this->ptr();
            insert(pos, 1, char_value);
            return this->ptr() + pos;
        }

        template <typename InputIterator>
        void insert(iterator iter, InputIterator first, InputIterator last) {
            size_type pos = iter - this->ptr();
            size_type count = std::distance(first, last);
            insert(pos, &(*first), count);
        }

        void insert(iterator iter, size_type count, value_type char_value) {
            size_type pos = iter - this->ptr();
            insert(pos, count, char_value);
        }

        void insert(iterator iter, const_pointer first, const_pointer last) {
            size_type pos = iter - this->ptr();
            insert(pos, first, last - first);
        }

        void insert(iterator iter, const_iterator first, const_iterator last) {
            size_type pos = iter - this->ptr();
            insert(pos, first, last - first);
        }
        #pragma endregion

        friend bool operator==(const basic_string &left, const basic_string &right) {
            return left.compare(right) == 0;
        }

        friend bool operator!=(const basic_string &left, const basic_string &right) {
            return left.compare(right) != 0;
        }

        void swap(basic_string &right) noexcept {
            if (this == &right) {
                return;
            }
            if (!this->is_local() && !right.is_local()) {
                auto &this_resources = this->get_storage();
                auto &right_resources = right.get_storage();
                std::swap(this_resources.ptr, right_resources.ptr);
                std::swap(this->size_, right.size_);
                std::swap(this->capacity_, right.capacity_);
                return;
            }
            if (this->is_local() && right.is_local()) {
                value_type temp[internal_storage::SMALL_BUFFER_SIZE]{};
                traits_type::copy(temp, this->data(), this->size_ + 1);
                traits_type::copy(this->ptr(), right.data(), right.size_ + 1);
                traits_type::copy(right.ptr(), temp, this->size_ + 1);
                std::swap(this->size_, right.size_);
                std::swap(this->capacity_, right.capacity_);
                return;
            }
            basic_string *sso_string = this->is_local() ? this : &right;
            basic_string *heap_string = this->is_local() ? &right : this;
            value_type temp[internal_storage::SMALL_BUFFER_SIZE]{};
            traits_type::copy(temp, sso_string->data(), sso_string->size_ + 1);
            size_type sso_size = sso_string->size_;
            size_type sso_capacity = sso_string->capacity_;
            auto &heap_resources = heap_string->get_storage();
            auto &sso_resources = sso_string->get_storage();
            sso_resources.ptr = heap_resources.ptr;
            sso_string->size_ = heap_string->size_;
            sso_string->capacity_ = heap_string->capacity_;
            traits_type::copy(heap_string->ptr(), temp, sso_size + 1);
            heap_string->size_ = sso_size;
            heap_string->capacity_ = sso_capacity;
            heap_resources.ptr = nullptr;
        }

        void clear() {
            assign(this->capacity(), value_type{});
            this->size = 0;
        }

        void resize(size_type count, value_type ch = value_type{}) {
            if (count <= this->size_) {
                this->size_ = count;
                traits_type::assign(this->ptr()[count], value_type{});
                return;
            }
            if (count <= this->capacity_) {
                traits_type::assign(this->ptr() + this->size_, count - this->size_, ch);
                this->size_ = count;
                traits_type::assign(this->ptr()[count], value_type{});
                return;
            }
            auto pair = allocate_for_new_memory(count);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::copy(new_ptr, this->data(), this->size_);
            traits_type::assign(new_ptr + this->size_, count - this->size_, ch);
            traits_type::assign(new_ptr[count], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
            this->size_ = count;
        }

        void reserve(size_type count = 0) {
            if (count == 0) {
                if (this->size_ < internal_storage::SMALL_BUFFER_SIZE) {
                    this->activate_sso_buffer();
                }
                return;
            }
            if (count <= this->capacity_) {
                return;
            }
            auto pair = allocate_for_new_memory(count);
            auto new_ptr = pair.first;
            auto new_capacity = pair.second;
            traits_type::copy(new_ptr, this->data(), this->size_);
            traits_type::assign(new_ptr[this->size_], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = new_capacity;
        }

        void shrink_to_fit() {
            if (this->size_ == this->capacity_ || this->is_local()) {
                return;
            }
            if (this->size_ < internal_storage::SMALL_BUFFER_SIZE) {
                this->activate_sso_buffer();
            }
            auto &allocator = this->instance_data.get_first();
            pointer new_ptr = allocator.allocate(this->size_ + 1);
            traits_type::copy(new_ptr, this->data(), this->size_);
            traits_type::assign(new_ptr[this->size_], value_type{});
            tidy_helper();
            auto &storage = this->get_storage();
            storage.ptr = new_ptr;
            this->capacity_ = this->size_;
        }

        basic_string substr(size_type pos = 0, size_type count = npos) const {
            return basic_string{utility::move(*this), pos, count};
        }

        template <typename Elem, typename Traits_>
        friend std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits_> &ostream, const basic_string &str) {
            ostream.write(str.c_str(), str.size());
            return ostream;
        }

    private:
#pragma region basic_string_impl
        using internal_storage = internals::basic_string_storage<CharType, Traits, Alloc>;

        RAINY_CONSTEXPR20 void tidy_helper() {
            auto &allocator = this->instance_data.get_first();
            auto &storage = this->get_storage();
            if (!this->empty() && !this->is_local()) {
                allocator.deallocate(storage.ptr, this->capacity());
            }
            this->activate_sso_buffer();
        }

        enum class construct_strategy {
            from_contaienr,
            from_char,
            from_ptr,
            empty
        };

        static inline constexpr std::size_t border_less = 3;

        static constexpr inline int border_calc(std::size_t start, std::size_t end, std::size_t value) {
            return core::internals::in_range(start + border_less, end - border_less, value)
                       ? 0
                       : (value < start + border_less ? -1 : 1);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 std::size_t calc_growth(std::size_t request) {
            constexpr containers::array string_allocation_tables({
                utility::make_pair(utility::make_pair(257ul, 512ul), 1.50f),
                utility::make_pair(utility::make_pair(129ul, 256ul), 1.42f),
                utility::make_pair(utility::make_pair(65ul, 128ul), 1.37f),
                utility::make_pair(utility::make_pair(53ul, 64ul), 1.32f),
                utility::make_pair(utility::make_pair(43ul, 52ul), 1.27f),
                utility::make_pair(utility::make_pair(33ul, 42ul), 1.22f),
                utility::make_pair(utility::make_pair(17ul, 32ul), 1.15f),
                utility::make_pair(utility::make_pair(1ul, 16ul), 1.1f),
            });
            constexpr containers::array extra_allocation_tables({
                utility::make_pair(utility::make_pair(257ul, 512ul), 24ul),
                utility::make_pair(utility::make_pair(129ul, 256ul), 18ul),
                utility::make_pair(utility::make_pair(65ul, 128ul), 14ul),
                utility::make_pair(utility::make_pair(53ul, 64ul), 12ul),
                utility::make_pair(utility::make_pair(43ul, 52ul), 10ul),
                utility::make_pair(utility::make_pair(33ul, 42ul), 7ul),
                utility::make_pair(utility::make_pair(17ul, 32ul), 5ul),
                utility::make_pair(utility::make_pair(1ul, 16ul), 2ul),
            });
            std::size_t allocation_size = 0;
            for (const auto &pair: string_allocation_tables) {
                if (!core::internals::in_range<std::size_t>(pair.first.first, pair.first.second, request)) {
                    continue;
                }
                switch (border_calc(pair.first.first, pair.first.second, request)) {
                    case -1:
                        allocation_size = request;
                        break;
                    case 0:
                        allocation_size = static_cast<std::size_t>(request * pair.second); // NOLINT
                        break;
                    case 1:
                        allocation_size = static_cast<std::size_t>(request * pair.second) + (request * 0.07f); // NOLINT
                        break;
                }
            }
            std::size_t extra_allocation = 0;
            for (const auto &pair: extra_allocation_tables) {
                if (!core::internals::in_range<std::size_t>(pair.first.first, pair.first.second, allocation_size)) {
                    continue;
                }
                extra_allocation = pair.second;
            }
            return allocation_size + extra_allocation;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 utility::pair<pointer, std::size_t> allocate_at_first(const std::size_t request) {
            auto &allocator = this->instance_data.get_first();
            rainy_let new_allocated = static_cast<std::size_t>(request * 1.2f + 1); // NOLINT
            return utility::make_pair(allocator.allocate(new_allocated), new_allocated);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 utility::pair<pointer, std::size_t> allocate_for_new_memory(const std::size_t request) {
            auto &allocator = this->instance_data.get_first();
            rainy_let new_allocated = calc_growth(request); // NOLINT
            return utility::make_pair(allocator.allocate(new_allocated), new_allocated);
        }

        template <construct_strategy Strategy, typename CharOrPointer>
        RAINY_CONSTEXPR20 void construct_string(CharOrPointer arg, size_type count) {
#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6385)
#endif
            if (count == 0) {
                this->activate_sso_buffer();
                return;
            }
            auto &storage = this->get_storage();
            auto pair =
                count < internal_storage::SMALL_BUFFER_SIZE ? utility::make_pair(this->ptr(), count) : allocate_at_first(count);
            auto &get = pair.first;
            auto &new_allocated = pair.second;
            this->size_ = count;
            this->capacity_ = new_allocated;
            if (count > internal_storage::SMALL_STRING_CAPACITY) {
                storage.ptr = get;
            }
            auto my_data = this->ptr();
            if constexpr (Strategy == construct_strategy::from_ptr) {
                traits_type::copy(my_data, arg, count);
                traits_type::assign(my_data[count], value_type{});
            } else if constexpr (Strategy == construct_strategy::from_char) {
                traits_type::assign(my_data, count, arg);
                traits_type::assign(my_data[count], value_type{});
            } else if constexpr (Strategy == construct_strategy::from_contaienr) {
                traits_type::copy(my_data, arg,
                                  count < internal_storage::SMALL_BUFFER_SIZE ? internal_storage::SMALL_STRING_CAPACITY : count);
            } else {
                this->activate_sso_buffer();
            }
#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
        }
#pragma endregion
    };

    using string  = basic_string<char, text::char_traits<char>, foundation::system::memory::allocator<char>>;
    using wstring = basic_string<wchar_t, text::char_traits<wchar_t>, foundation::system::memory::allocator<wchar_t>>;
#ifdef __cpp_lib_char8_t
    using u8string = basic_string<char8_t, text::char_traits<char8_t>, foundation::system::memory::allocator<char8_t>>;
#endif
    using u16string = basic_string<char16_t, text::char_traits<char16_t>, foundation::system::memory::allocator<char16_t>>;
    using u32string = basic_string<char32_t, text::char_traits<char32_t>, foundation::system::memory::allocator<char32_t>>;
}

namespace rainy::text {
    using containers::basic_string;
    using containers::string;
    using containers::wstring;
#ifdef __cpp_lib_char8_t
    using containers::u8string;
#endif
    using containers::u16string;
    using containers::u32string;
}

#endif