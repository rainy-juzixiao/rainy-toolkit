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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_STRING_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_STRING_HPP

// NOLINTBEGIN
#include <rainy/core/implements/hash.hpp>
#include <rainy/core/implements/text/char_traits.hpp>
#include <rainy/core/implements/text/string_view.hpp>
#include <rainy/core/platform.hpp>
// NOLINTEND

#if RAINY_USING_GCC && !RAINY_USING_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

#if !RAINY_USING_MACOS
#pragma GCC diagnostic ignored "-Wstringop-overflow" // Ok...
#endif

#endif

namespace rainy::collections {
    template <typename Ty, typename Allocator>
    class vector;
}

namespace rainy::foundation::text {
    template <typename CharType, typename Traits = char_traits<CharType>, typename Allocator = std::allocator<CharType>>
    class basic_string {
    public:
        // NOLINTBEGIN

        using traits_type = Traits;
        using value_type = CharType;
        using allocator_type = Allocator;
        using size_type = typename std::allocator_traits<Allocator>::size_type;
        using difference_type = typename std::allocator_traits<Allocator>::difference_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
        using allocator_traits = std::allocator_traits<allocator_type>;
        using iterator = value_type *;
        using const_iterator = const value_type *;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        // NOLINTEND

        static constexpr size_type npos = static_cast<size_type>(-1);

        RAINY_CONSTEXPR20 basic_string() noexcept : pair_{{}, {}} {
        }

        RAINY_CONSTEXPR20 basic_string(allocator_type const &allocator) noexcept : pair_{allocator, {}} { // NOLINT
        }

        RAINY_CONSTEXPR20 basic_string(size_type count, value_type ch, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            if (count > 0) {
                allocate_plus_one_(count);
                auto begin = begin_();
                std::fill(begin, begin + count, ch);
            }
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, size_type count,
                                       allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            auto other_size = right.size();
            if (pos > other_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (other_size - pos, count);
            allocate_plus_one_(count);
            auto start = right.begin_() + pos;
            fill_(start, start + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, allocator_type const &allocator = allocator_type()) :
            basic_string(right, pos, right.size() - pos, allocator) {
        }

        RAINY_CONSTEXPR20 basic_string(const value_type *ptr, size_type count, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            allocate_plus_one_(count);
            fill_(ptr, ptr + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const value_type *ptr, allocator_type const &allocator = allocator_type()) : // NOLINT
            basic_string(ptr, traits_type::length(ptr), allocator) {
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string(InputIt first, InputIt last) {
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto length = utility::distance(first, last);
                allocate_plus_one_(length);
                core::algorithm::copy(first, last, begin_());
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right) :
            pair_{allocator_traits::select_on_container_copy_construction(right.get_al()), {}} {
            auto other_size = right.size();
            allocate_plus_one_(other_size);
            fill_(right.begin_(), right.end_());
            resize_(other_size);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, allocator_type const &allocator) : pair_{allocator, {}} {
            auto other_size = right.size();
            allocate_plus_one_(other_size);
            fill_(right.begin_(), right.end_());
            resize_(other_size);
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right, size_type pos, size_type count,
                                       allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            auto other_size = right.size();
            if (pos > other_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (other_size - pos, count);
            if (pos != 0) {
                auto other_begin = right.begin_();
                auto start = other_begin + pos;
                auto last = start + count;
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) {
                    core::algorithm::copy(start, last, other_begin);
                } else {
                    std::memmove(other_begin, start, (last - start) * sizeof(CharType));
                }
                // NOLINTEND
            }
            right.resize_(count);
            right.swap(*this);
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right) noexcept : pair_{{}, {}} {
            if RAINY_CONSTEXPR20 (allocator_traits::propagate_on_container_swap::value) {
                std::swap(this->get_al(), right.get_al());
            } else {
                assert(right.get_al() == this->get_al());
            }
            right.swap_without_ator(*this);
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right, allocator_type const &allocator) : pair_{allocator, {}} {
            if (right.get_al() == allocator) {
                right.swap_without_ator(*this);
            } else {
                basic_string temp{right.data(), right.size(), allocator};
                temp.swap(*this);
                right.swap(temp);
            }
        }

        RAINY_CONSTEXPR20 basic_string(std::initializer_list<CharType> ilist, allocator_type const &allocator = allocator_type()) :
            basic_string(std::data(ilist), ilist.size(), allocator) {
        }


        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &sv, allocator_type const &allocator = allocator_type()) : // NOLINT
            basic_string(sv.data(), sv.size(), allocator) {
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &sv, size_type pos, size_type count,
                                       allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            auto data = sv.data();
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (sv_size - pos, count);
            allocate_plus_one_(count);
            fill_(data, data + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 ~basic_string() {
            if (is_long_()) {
                dealloc_(get_storage().ls_);
            }
        }

        // NOLINTBEGIN
        RAINY_CONSTEXPR20 basic_string(std::nullptr_t) = delete;
        RAINY_CONSTEXPR20 basic_string &operator=(std::nullptr_t) = delete;
        // NOLINTEND

        RAINY_CONSTEXPR20 basic_string &operator=(basic_string &&right) noexcept(
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Allocator>::is_always_equal::value) {
            assign(utility::move(right));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator=(const basic_string &right) {
            assign(right);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator=(const value_type *ptr) {
            assign(ptr);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator=(CharType ch) {
            resize_(1);
            *begin_() = ch;
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator=(std::initializer_list<CharType> ilist) {
            assign(ilist);
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator=(const StringViewLike &sv) {
            assign(sv);
            return *this;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool empty() const noexcept {
            return !size_flag_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 std::size_t size() const noexcept {
            if (is_long_()) {
                auto &&ls = get_storage().ls_;
                return ls.end_ - ls.begin_;
            }
            return size_flag_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 std::size_t length() const noexcept {
            return size();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 static size_type max_size() noexcept {
            return static_cast<size_type>(-1) / sizeof(CharType) / 2;
        }

        RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            if (is_long_()) {
                auto &&ls = get_storage().ls_;
                return ls.last_ - ls.begin_ - 1;
            }
            return short_string_max_;
        }

        RAINY_CONSTEXPR20 void shrink_to_fit() noexcept {
            auto current_size = size();
            // 情况 1：从长字符串缩减到短字符串
            if (current_size <= short_string_max_ && is_long_()) {
                auto ls = get_storage().ls_;
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) { // NOLINT
                    get_storage().ss_ = decltype(get_storage().ss_){};
                    core::algorithm::copy(ls.begin_, ls.end_, get_storage().ss_.data());
                } else
#endif
                // NOLINTBEGIN
                {
                    std::memcpy(get_storage().ss_.data(), ls.begin_, current_size * sizeof(CharType));
                }
                // NOLINTEND
                dealloc_(ls);
                size_flag_ = static_cast<unsigned char>(current_size);
                get_storage().ss_[current_size] = CharType{}; // null terminator
                return;
            }
            // 情况 2：长字符串缩减容量（但仍保持长字符串）
            if (is_long_()) {
                auto ls = get_storage().ls_;
                auto current_capacity = ls.last_ - ls.begin_ - 1;
                // 如果容量已经接近 size，不需要缩减
                if (current_capacity <= current_size + current_size / 2) {
                    return;
                }
                // 重新分配更小的空间
                auto new_capacity = current_size;
                auto new_ptr = allocator_traits::allocate(get_al(), new_capacity + 1);
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) { // NOLINT
                    core::algorithm::copy(ls.begin_, ls.end_, new_ptr);
                } else
#endif
                // NOLINTBEGIN
                {
                    std::memcpy(new_ptr, ls.begin_, current_size * sizeof(CharType));
                }
                // NOLINTEND
                new_ptr[current_size] = CharType{};
                dealloc_(ls);
                get_storage().ls_ = {new_ptr, new_ptr + current_size, new_ptr + new_capacity + 1};
            }
            // 情况 3：已经是短字符串，无需操作
        }

        RAINY_CONSTEXPR20 value_type const *data() const noexcept {
            return begin_();
        }

        RAINY_CONSTEXPR20 CharType *data() noexcept {
            return begin_();
        }

        RAINY_CONSTEXPR20 value_type const *c_str() const noexcept {
            return begin_();
        }

        RAINY_CONSTEXPR20 const_reference at(size_type pos) const {
            if (pos >= size()) {
                throw std::out_of_range{exception_string_};
            }
            return *(begin_() + pos);
        }

        RAINY_CONSTEXPR20 reference at(size_type pos) {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).at(pos));
        }

        RAINY_CONSTEXPR20 reference operator[](size_type pos) noexcept {
            if (!std::is_constant_evaluated()) {
                assert(("pos >= size, please check the arg" && pos < size()));
            }
            return const_cast<CharType &>(const_cast<basic_string const &>(*this)[pos]);
        }

        RAINY_CONSTEXPR20 const_reference operator[](size_type pos) const noexcept {
            if (!std::is_constant_evaluated()) {
                assert(("pos >= size, please check the arg" && pos < size()));
            }
            return *(begin_() + pos);
        }

        RAINY_CONSTEXPR20 const CharType &front() const noexcept {
            assert(("string is empty" && !empty()));
            return *begin_();
        }

        RAINY_CONSTEXPR20 CharType &front() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).front());
        }

        RAINY_CONSTEXPR20 const CharType &back() const noexcept {
            assert(("string is empty" && !empty()));
            return *(end_() - 1);
        }

        RAINY_CONSTEXPR20 CharType &back() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).back());
        }

        RAINY_CONSTEXPR20 operator basic_string_view<value_type>() const noexcept { // NOLINT
            return basic_string_view<value_type>(begin_(), end_());
        }

        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return {this->begin_()};
        }

        RAINY_CONSTEXPR20 iterator end() noexcept {
            return {this->end_()};
        }

        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return const_iterator{this->begin_()};
        }

        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return const_iterator{this->end_()};
        }

        RAINY_CONSTEXPR20 const_iterator cbegin() noexcept {
            return begin();
        }

        RAINY_CONSTEXPR20 const_iterator cend() noexcept {
            return end();
        }

        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
            return reverse_iterator{this->end_()};
        }

        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
            return reverse_iterator{this->begin_()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator{this->end_()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator{this->begin_()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() noexcept {
            return rbegin();
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crend() noexcept {
            return rend();
        }

        RAINY_CONSTEXPR20 void reserve(size_type new_cap) {
            if (capacity() >= new_cap) {
                return;
            }

            if (std::is_constant_evaluated()) {
                // 编译期：构造副本，分配新空间，交换
                basic_string temp{*this}; // 复制当前字符串
                allocate_plus_one_(new_cap); // 在当前对象上分配新空间
                temp.swap_without_ator(*this); // 交换内容
            } else {
                // 运行期：原逻辑
                if (is_long_()) {
                    auto ls = get_storage().ls_;
                    allocate_plus_one_(new_cap);
                    fill_(ls.begin_, ls.end_);
                    auto size = ls.end_ - ls.begin_;
                    dealloc_(ls);
                    resize_(size);
                } else {
                    auto ss = get_storage().ss_;
                    auto size = size_flag_;
                    auto data = ss.data();
                    allocate_plus_one_(new_cap);
                    fill_(data, data + size);
                    resize_(size);
                }
            }
        }

        RAINY_CONSTEXPR20 void resize(size_type count, value_type ch) {
            auto size = this->size();
            if (count > size) {
                reserve(count);
                auto begin = begin_();
                std::fill(begin + size, begin + count, ch);
            }
            resize_(count);
        }

        RAINY_CONSTEXPR20 void resize(size_type count) {
            resize(count, CharType{});
        }

        RAINY_CONSTEXPR20 void clear() noexcept {
            resize_(0);
        }

        RAINY_CONSTEXPR20 void push_back(value_type ch) {
            auto size = this->size();
            if (capacity() == size) {
                reserve(size * 2 - size / 2);
            }

            if (std::is_constant_evaluated()) {
                auto new_size = size + 1;
                resize_(new_size);
                if (is_long_()) {
                    *(get_storage().ls_.begin_ + size) = ch;
                } else {
                    get_storage().ss_[size] = ch;
                }
            } else {
                *end_() = ch;
                resize_(size + 1);
            }
        }

        RAINY_CONSTEXPR20 void swap(basic_string &right) noexcept {
            if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
                std::swap(right.get_al(), right.get_al());
            } else {
                assert(right.get_al() == this->get_al());
            }
            right.swap_without_ator(*this);
        }

        friend void swap(basic_string &self, basic_string &right) noexcept {
            self.swap(right);
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(InputIt first, InputIt last) {
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto size = this->size();
                auto length = utility::distance(first, last);
                auto new_size = size + length;
                reserve((core::max) (size * 2, new_size));
                core::algorithm::copy(first, last, begin_() + size);
                resize_(new_size);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(size_type count, value_type ch) {
            reserve(count);
            auto begin = begin_();
            auto end = begin + count;
            std::fill(begin, end, ch);
            resize_(count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const basic_string &str) {
            if (std::addressof(str) == this) {
                return *this;
            }
            if RAINY_CONSTEXPR20 (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
                if (this->get_al() != str.get_al()) {
                    basic_string temp{this->get_al()};
                    temp.swap(*this);
                    this->get_al() = str.get_al();
                }
            }
            assign_(str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size();
            if (pos > str_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (str_size - pos, count));
            auto str_begin = begin_();
            assign_(str_begin + pos, str_begin + pos + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(basic_string &&right) noexcept { // NOLINT
            if RAINY_CONSTEXPR20 (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
                right.swap(*this);
            } else {
                if (this->get_al() == right.get_al()) {
                    right.swap_without_ator(*this);
                } else {
                    assign_(right.begin_(), right.end_());
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const value_type *ptr, size_type count) {
            assign_(ptr, ptr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const value_type *ptr) {
            assign_(ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string &assign(InputIt first, InputIt last) {
            resize_(0);
            return append(first, last);
        }

        RAINY_CONSTEXPR20 basic_string &assign(std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            assign_(data, data + ilist.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        basic_string &assign(const StringViewLike &sv) {
            auto data = sv.data();
            assign_(data, data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &assign(const StringViewLike &sv, size_type pos, size_type count = npos) {
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (sv_size - pos, count));
            auto data = sv.data();
            assign_(data + pos, data + pos + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(size_type count, value_type ch) {
            auto size = this->size();
            reserve(size + count);
            auto end = begin_() + size;
            std::fill(end, end + count, ch);
            resize_(size + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(const value_type *ptr, size_type count) {
            append_(ptr, ptr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(const basic_string &str) {
            auto begin = str.begin_();
            append_(begin, begin + str.size());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size();
            if (pos > str_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (str_size - pos, count));
            return append(str.begin_() + pos, count);
        }

        RAINY_CONSTEXPR20 basic_string &append(const value_type *ptr) {
            append_(ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(std::initializer_list<CharType> ilist) {
            append_(ilist.begin(), ilist.end());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &sv) {
            auto data = sv.data();
            append_(data, data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &sv, size_type pos, size_type count = npos) {
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (sv_size - count, count));
            return append(sv.data() + pos, count);
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator+=(const StringViewLike &sv) {
            return append(sv);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(const basic_string &str) {
            return append(str);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(value_type ch) {
            push_back(ch);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(const value_type *ptr) {
            return append(ptr);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(std::initializer_list<CharType> ilist) {
            return append(ilist);
        }

        // NOLINTBEGIN
        RAINY_CONSTEXPR20 size_type find(basic_string_view<value_type> sv, size_type pos = 0) const noexcept {
            const size_type self_size = size();
            const size_type sv_size = sv.size();
            if (pos > self_size || sv_size > self_size - pos) {
                return npos;
            }
            if (sv_size == 0) {
                return pos;
            }
            const value_type *self_begin = this->begin_();
            const size_type limit = self_size - sv_size;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = pos; i <= limit; ++i) {
                    bool match = true;
                    for (size_type j = 0; j < sv_size; ++j) {
                        if (!traits_type::eq(self_begin[i + j], sv[j])) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return i;
                    }
                }
                return npos;
            }
#endif
            for (size_type i = pos; i <= limit; ++i) {
                if (traits_type::compare(self_begin + i, sv.data(), sv_size) == 0) {
                    return i;
                }
            }
            return npos;
        }
        // NOLINTEND

        RAINY_CONSTEXPR20 size_type find(value_type c, size_type pos = 0) const noexcept {
            if (pos >= size()) {
                return npos;
            }
            const value_type *result = traits_type::find(this->begin_() + pos, size() - pos, c);
            return result ? result - this->begin_() : npos;
        }

        RAINY_CONSTEXPR20 size_type find(const value_type *ptr, size_type pos, size_type count) const {
            return find(basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find(const value_type *ptr, size_type pos = 0) const {
            return find(basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type rfind(basic_string_view<value_type> sv, size_type pos = npos) const noexcept {
            const size_type self_size = size();
            const size_type sv_size = sv.size();
            if (sv_size == 0) {
                return (core::min) (pos, self_size);
            }
            if (sv_size > self_size) {
                return npos;
            }
            const value_type *self_begin = this->begin_();
            const size_type limit = (core::min) (pos, self_size - sv_size);
#if RAINY_HAS_CXX20
            // NOLINTBEGIN
            if (std::is_constant_evaluated()) { // NOLINT
                for (size_type i = limit + 1; i-- > 0;) {
                    bool match = true;
                    for (size_type j = 0; j < sv_size; ++j) {
                        if (!traits_type::eq(self_begin[i + j], sv[j])) { // NOLINT
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return i;
                    }
                }
                return npos;
            }
            // NOLINTEND
#endif
            // NOLINTBEGIN
            for (size_type i = limit + 1; i-- > 0;) {
                if (traits_type::compare(self_begin + i, sv.data(), sv_size) == 0) {
                    return i;
                }
            }
            // NOLINTEND
            return npos;
        }

        RAINY_CONSTEXPR20 size_type rfind(value_type c, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            pos = (core::min) (pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (traits_type::eq(*p, c)) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type rfind(const value_type *ptr, size_type pos, size_type count) const {
            return rfind(basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type rfind(const value_type *ptr, size_type pos = npos) const {
            return rfind(basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type find_first_of(basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
            for (auto p = this->begin_() + pos; p < this->begin_() + size(); ++p) {
                if (ptr.find(*p) != npos) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_first_of(value_type c, size_type pos = 0) const noexcept {
            return find(c, pos);
        }

        RAINY_CONSTEXPR20 size_type find_first_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_first_of(basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find_first_of(const value_type *ptr, size_type pos = 0) const {
            return find_first_of(basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_of(basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            pos = (core::min) (pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (ptr.find(*p) != npos) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_last_of(value_type c, size_type pos = npos) const noexcept {
            return rfind(c, pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_last_of(basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_of(const value_type *ptr, size_type pos = npos) const {
            return find_last_of(basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type find_first_not_of(basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
            for (auto p = this->begin_() + pos; p < this->begin_() + size(); ++p) {
                if (ptr.find(*p) == npos) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept {
            for (auto p = this->begin_() + pos; p < this->begin_() + size(); ++p) {
                if (!traits_type::eq(*p, c)) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_first_not_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_first_not_of(basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find_first_not_of(const value_type *ptr, size_type pos = 0) const {
            return find_first_not_of(basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_not_of(basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            pos = (core::min) (pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (ptr.find(*p) == npos) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            if (pos >= size()) {
                pos = size() - 1;
            }
            for (size_type i = pos + 1; i > 0; --i) {
                if (!traits_type::eq(this->begin_()[i - 1], c)) {
                    return i - 1;
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_last_not_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_last_not_of(basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_not_of(const value_type *ptr, size_type pos = npos) const {
            return find_last_not_of(basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 bool starts_with(basic_string_view<value_type> sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = begin_();
            if (sv_size > size()) {
                return false;
            }
            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        RAINY_CONSTEXPR20 bool starts_with(CharType ch) const noexcept {
            return *begin_() == ch;
        }

        RAINY_CONSTEXPR20 bool starts_with(const value_type *ptr) const {
            auto length = traits_type::length(ptr);
            auto begin = begin_();
            if (length > size()) {
                return false;
            }
            return equal_(ptr, ptr + length, begin, begin + length);
        }

        RAINY_CONSTEXPR20 bool ends_with(basic_string_view<value_type> sv) const noexcept {
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = end_();
            if (sv_size > size()) {
                return false;
            }
            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        RAINY_CONSTEXPR20 bool ends_with(CharType ch) const noexcept {
            return *(end_() - 1) == ch;
        }

        RAINY_CONSTEXPR20 bool ends_with(value_type const *ptr) const {
            auto length = traits_type::length(ptr);
            if (length > size()) {
                return false;
            }
            auto end = end_();
            return equal_(ptr, ptr + length, end - length, end);
        }

        RAINY_CONSTEXPR20 bool contains(std::basic_string_view<value_type> sv) const noexcept {
            auto size = sv.size();
            auto begin = begin_();
            auto data = sv.data();
            if (this->size() < size) {
                return false;
            }
            if (equal_(begin, begin + size, data, data + size)) {
                return true;
            }
            return find(sv) != npos;
        }

        RAINY_CONSTEXPR20 bool contains(CharType ch) const noexcept {
            for (auto begin = begin_(), end = end_(); begin != end; ++begin) {
                if (*begin == ch) {
                    return true;
                }
            }
            return false;
        }

        RAINY_CONSTEXPR20 bool contains(const value_type *ptr) const noexcept {
            return basic_string_view<value_type>{begin_(), end_()}.contains(
                basic_string_view<value_type>{ptr, ptr + traits_type::length(ptr)});
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, size_type count, CharType ch) {
            auto size = this->size();
            if (index > size) {
                throw std::out_of_range{exception_string_};
            }
            reserve(size + count);
            auto start = begin_() + index;
            auto end = begin_() + size;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                core::algorithm::copy_backward(start, end, end + count);
                std::fill(start, start + count, ch);
            } else
#endif
            // NOLINTBEGIN
            {
                std::memmove(start + count, start, (end - start) * sizeof(CharType));
                std::fill(start, start + count, ch);
            }
            // NOLINTEND
            resize_(size + count);
            return *this;
        }


        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const value_type *ptr, size_type count) {
            insert_(index, ptr, ptr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const value_type *ptr) {
            insert_(index, ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str) {
            insert_(index, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos) {
            auto s_size = str.size();
            if (s_index > s_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (s_size - s_index, count));
            auto s_start = str.begin_() + s_index;
            insert_(index, s_start, s_start + count);
            return *this;
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, CharType ch) {
            auto size = this->size();
            auto start = const_cast<iterator>(pos);
            auto index = start - begin_();
            reserve(size + 1);
            auto begin = begin_();
            auto end = begin + size;
            start = begin + index;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                core::algorithm::copy_backward(start, end, end + 1);
            } else
#endif
            // NOLINTBEGIN
            {
                traits_type::move(start + 1, start, end - start);
            }
            // NOLINTEND
            *start = ch;
            resize_(size + 1);
            return {start};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, size_type count, CharType ch) { // NOLINT
            auto start = const_cast<iterator>(pos);
            auto index = start - begin_();
            insert(index, count, ch);
            return {start};
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, InputIt first, InputIt last) {
            assert(("pos isn'sv in this string" && pos >= begin_() && pos <= end_()));
            auto size = this->size();
            auto start = pos;
            auto end = end_();
            auto index = start - begin_();
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto length = utility::distance(first, last);
                reserve(size + length);
                auto begin = begin_();
                core::algorithm::copy_backward(begin + index, begin + size, begin + size + length);
                core::algorithm::copy(first, last, begin + index);
                resize_(size + length);
            } else {
                basic_string temp{start, end};
                resize_(pos - begin_());
                for (; first != last; ++first) {
                    push_back(*first);
                }
                append_(temp.begin_(), temp.end_());
            }
            return {start};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, std::initializer_list<CharType> ilist) { // NOLINT
            auto i_data = std::data(ilist);
            auto start = const_cast<iterator>(pos);
            insert_(start - begin_(), i_data, i_data + ilist.size());
            return {start};
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &sv) {
            auto sv_data = sv.data();
            insert_(pos, sv_data, sv_data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &sv, size_type t_index, size_type count = npos) {
            auto sv_size = sv.size();
            auto size = this->size();
            if (t_index > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (sv_size - t_index, count));
            auto sv_data = sv.data();
            insert_(pos, sv_data + t_index, sv_data + t_index + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &erase(size_type index = 0, size_type count = npos) {
            auto size = this->size();
            if (index > size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min) (npos, (core::min) (size - index, count));
            auto start = begin_() + index;
            erase_(start, start + count);
            return *this;
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator position) noexcept {
            auto start = const_cast<iterator>(position);
            erase_(start, start + 1);
            return {start};
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) noexcept {
            auto start = const_cast<iterator>(first);
            erase_(start, last);
            return {start};
        }

        RAINY_CONSTEXPR20 void pop_back() noexcept {
            assert(("string is empty" && !empty()));
            resize_(size() - 1);
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str) {
            replace_(pos, count, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const basic_string &str) {
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str, size_type pos2,
                                                size_type count2 = npos) {
            auto str_size = str.size();
            if (pos2 > str_size) {
                throw std::out_of_range{exception_string_};
            }
            count2 = (core::min) (npos, (core::min) (count2, str_size - pos2));
            auto begin = str.begin_();
            replace_(pos, count, begin + count2, begin + count2 + pos2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const value_type *ptr, size_type count2) {
            replace_(pos, count, ptr, ptr + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const value_type *ptr, size_type count2) {
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, ptr, ptr + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const value_type *ptr) {
            replace_(pos, count, ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const value_type *ptr) {
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, data, data + ilist.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const StringViewLike &sv) {
            auto data = sv.data();
            replace_(pos, count, data, data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const StringViewLike &sv) {
            auto sv_data = sv.data();
            auto start = first;
            replace_(start - begin_(), last - first, sv_data, sv_data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        basic_string &replace(size_type pos, size_type count, const StringViewLike &sv, size_type pos2, size_type count2 = npos) {
            auto sv_size = sv.size();
            if (pos2 > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count2 = (core::min) (npos, (core::min) (sv_size - pos2, count2));
            auto data = sv.data();
            replace_(pos, count, data + pos2, data + pos2 + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = begin_();
            replace_(pos, count, begin, begin + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = begin_();
            auto start = first;
            replace_(start - begin_(), last - first, begin, begin + count2);
            return *this;
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        basic_string &replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) {
            auto start = first;
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto data = utility::addressof(*first2);
                auto length2 = utility::distance(first2, last2);
                replace_(start - begin_(), last - first, data, data + length2);
            } else {
                basic_string temp{first2, last2};
                auto begin = temp.begin_();
                auto size = temp.size();
                replace_(start - begin_(), last - first, begin, begin + size);
            }
            return *this;
        }

        basic_string &repeat(const std::size_t repeat_count) {
            if (repeat_count == 0) {
                clear();
                return *this;
            }
            if (repeat_count == 1) {
                return *this;
            }
            auto original_size = size();
            reserve(original_size * repeat_count);
            basic_string_view<value_type> view(data(), original_size);
            for (std::size_t i = 1; i < repeat_count; ++i) {
                append(view.data(), view.size());
            }
            return *this;
        }

        basic_string repeat_copy(const std::size_t repeat_count) const {
            basic_string result;
            if (repeat_count == 0) {
                return result;
            }
            result.reserve(size() * repeat_count);
            auto original_size = size();
            basic_string_view<value_type> view(data(), original_size);
            for (std::size_t i = 0; i < repeat_count; ++i) {
                result.append(view.data(), view.size());
            }
            return result;
        }

        basic_string &to_upper() {
            core::algorithm::transform(this->begin(), this->end(), this->begin(),
                                       [](value_type ch) { return std::toupper(static_cast<int>(ch)); });
            return *this;
        }

        basic_string to_upper_copy() const {
            basic_string str = *this;
            core::algorithm::transform(this->begin(), this->end(), str.begin(),
                                       [](value_type ch) { return std::toupper(static_cast<int>(ch)); });
            return str;
        }

        basic_string &to_lower() {
            core::algorithm::transform(this->begin(), this->end(), this->begin(),
                                       [](value_type ch) { return std::tolower(static_cast<int>(ch)); });
            return *this;
        }

        basic_string to_lower_copy() const {
            basic_string str = *this;
            core::algorithm::transform(this->begin(), this->end(), str.begin(),
                                       [](value_type ch) { return std::tolower(static_cast<int>(ch)); });
            return str;
        }

        basic_string &trim() {
            static constexpr value_type whitespace[] = {' ', '\t', '\n', '\r', '\f', '\v', '\0'};
            size_type pos = this->find_first_not_of(whitespace);
            if (pos == npos) {
                clear();
                return *this;
            }
            size_type pos2 = this->find_last_not_of(whitespace);
            if (pos2 != npos) {
                *this = substr(pos, pos2 - pos + 1);
            } else {
                *this = substr(pos);
            }
            return *this;
        }

        basic_string trimmed() const {
            static constexpr value_type whitespace[] = {' ', '\t', '\n', '\r', '\f', '\v', '\0'};
            size_type pos = this->find_first_not_of(whitespace);
            if (pos == npos) {
                return basic_string();
            }
            size_type pos2 = this->find_last_not_of(whitespace);
            if (pos2 != npos) {
                return substr(pos, pos2 - pos + 1);
            }
            return substr(pos);
        }

        template <template <typename...> typename SeqContainerTemplate = collections::vector>
        RAINY_CONSTEXPR20 SeqContainerTemplate<basic_string> split(value_type delim) const {
            if (empty()) {
                return {};
            }
            size_type previous = 0;
            size_type current = find_first_of(delim);
            basic_string_view<value_type> str_view = (*this);
            SeqContainerTemplate<basic_string> res{};
            while (current != npos) {
                res.push_back(basic_string(str_view.substr(previous, current - previous)));
                previous = current + 1;
                current = find_first_of(delim, previous);
            }
            res.push_back(basic_string(str_view.substr(previous)));
            return res;
        }

        template <typename... Args>
        basic_string &format(basic_string_view<value_type> fmt, Args const &...args); // implement in format.hpp

        template <typename... Args>
        basic_string format_copy(basic_string_view<value_type> fmt, Args const &...args) const; // implement in format.hpp

        RAINY_CONSTEXPR20 basic_string substr(size_type pos = 0, size_type count = npos) const {
            return basic_string_view<value_type>{data(), size()}.substr(pos, count);
        }

        friend RAINY_CONSTEXPR20 bool operator==(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                if (lsize != rsize) {
                    return false;
                }
                for (auto l = left.begin_(), r = right.begin_(), end = l + lsize; l != end; ++l, ++r) {
                    if (*l != *r) {
                        return false;
                    }
                }
                return true;
            }
#endif
            return traits_type::compare(left.begin_(), right.begin_(), (core::min) (lsize, rsize)) == 0 && lsize == rsize;
        }

        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto l = left.begin_(), r = right.begin_(), end = l + (core::min) (lsize, rsize); l != end; ++l, ++r) {
                    if (*l < *r) {
                        return true;
                    }
                    if (*l > *r) {
                        return false;
                    }
                }
                return lsize < rsize;
            }
#endif
            auto res = traits_type::compare(left.begin_(), right.begin_(), (core::min) (rsize, lsize));
            if (res < 0) {
                return true;
            }
            if (res > 0) {
                return false;
            }
            return lsize < rsize;
        }

        friend RAINY_CONSTEXPR20 bool operator!=(basic_string const &left, basic_string const &right) noexcept {
            return !(left == right);
        }

        friend RAINY_CONSTEXPR20 bool operator>(basic_string const &left, basic_string const &right) noexcept {
            return right < left;
        }

        friend RAINY_CONSTEXPR20 bool operator<=(basic_string const &left, basic_string const &right) noexcept {
            return !(right < left);
        }

        friend RAINY_CONSTEXPR20 bool operator>=(basic_string const &left, basic_string const &right) noexcept {
            return !(left < right);
        }

        friend RAINY_CONSTEXPR20 bool operator==(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = traits_type::length(start);
            auto lsize = left.size();
            if (lsize != rsize) {
                return false;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto l = left.begin_(), end = l + lsize; l != end; ++l, ++start) {
                    if (*l != *start) {
                        return false;
                    }
                }
                return true;
            }
#endif
            {
                return traits_type::compare(left.begin_(), start, lsize) == 0;
            }
        }

        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = traits_type::length(start);
            auto lsize = left.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto l = left.begin_(), end = l + (core::min) (lsize, rsize); l != end; ++l, ++start) {
                    if (*l < *start) {
                        return true;
                    }
                    if (*l > *start) {
                        return false;
                    }
                }
                return lsize < rsize;
            }
#endif
            {
                auto res = traits_type::compare(left.begin_(), start, (core::min) (rsize, lsize));
                if (res < 0) {
                    return true;
                }
                if (res > 0) {
                    return false;
                }
                return lsize < rsize;
            }
        }

        friend RAINY_CONSTEXPR20 bool operator!=(basic_string const &left, CharType const *right) noexcept {
            return !(left == right);
        }

        friend RAINY_CONSTEXPR20 bool operator>(basic_string const &left, CharType const *right) noexcept {
            return right < left;
        }

        friend RAINY_CONSTEXPR20 bool operator<=(basic_string const &left, CharType const *right) noexcept {
            return !(right < left);
        }

        friend RAINY_CONSTEXPR20 bool operator>=(basic_string const &left, CharType const *right) noexcept {
            return !(left < right);
        }

    private:
        static constexpr std::size_t short_string_max_{sizeof(CharType *) * 4 / sizeof(CharType) - 2};

        struct ls_type_ {
            constexpr ls_type_(CharType *b, CharType *e, CharType *l) noexcept : begin_(b), end_(e), last_(l) {
            }

            CharType *begin_{};
            CharType *end_{};
            CharType *last_{};
        };

        union storage_type_ {
            constexpr storage_type_() noexcept : ss_{} {
            }

            std::array<CharType, short_string_max_ + 1> ss_;
            ls_type_ ls_;
        };

        static inline char exception_string_[] = "parameter is out of range, please check it.";

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool is_long_() const noexcept {
            return size_flag_ == static_cast<decltype(size_flag_)>(-1);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool is_short_() const noexcept {
            return size_flag_ != static_cast<decltype(size_flag_)>(-1);
        }

        static RAINY_CONSTEXPR20 void begin_lifetime(pointer begin, size_type n) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = 0; i != n; ++i) {
                    std::construct_at(std::addressof(begin[i]));
                }
            }
#else
            utility::ignore = begin;
            utility::ignore = n;
#endif
        }

        RAINY_CONSTEXPR20 void fill_(value_type const *begin, value_type const *end) noexcept {
            size_type count = static_cast<size_type>(end - begin);
            assert(count <= capacity());
            if (count == 0) {
                return;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = 0; i < count; ++i) {
                    std::construct_at(begin_() + i, begin[i]);
                }
            } else
#endif
            {
                traits_type::copy(begin_(), begin, count);
            }
        }


        RAINY_CONSTEXPR20 void append_(value_type const *first, value_type const *last) {
            auto length = last - first;
            auto size = this->size();
            auto new_size = size + length;
            // clang/gcc对这种可能更敏感，需要进行这种处理
#if (RAINY_USING_CLANG || RAINY_USING_GCC) && RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                if (is_short_()) { // 嗯……需要强制转化成长字符串，先保存，然后我再进行分配，不然constexpr的检查会进行不合理的报错
                    value_type old_data[short_string_max_ + 1];
                    auto old_size = size;
                    for (size_type i = 0; i < old_size; ++i) {
                        old_data[i] = get_storage().ss_[i]; // NOLINT
                    }
                    // 这个workaround并不优雅，不过，能跑就行的标准，不求太多了
                    allocate_plus_one_(new_size); // 应该可以分配了
                    auto new_begin = begin_();
                    for (size_type i = 0; i < old_size; ++i) {
                        utility::construct_at(new_begin + i, old_data[i]); // NOLINT
                    }
                    size = old_size;
                    // 应该不会爆炸了
                }
                if (capacity() < new_size) {
                    reserve(new_size);
                }
                // 追加新数据
                auto begin = begin_();
                for (size_type i = 0; i < length; ++i) {
                    utility::construct_at(begin + size + i, first[i]);
                }
                resize_(new_size);
            } else
#endif
            {
                reserve(new_size);
                auto begin = begin_();
                traits_type::copy(begin + size, first, length);
                resize_(new_size);
            }
        }

        RAINY_CONSTEXPR20 void erase_(CharType *first, value_type const *last) noexcept {
            assert(("first or last is not in this string" && first >= begin_() && last <= end_()));
            // NOLINTBEGIN
            if (std::is_constant_evaluated()) {
                core::algorithm::copy(last, const_cast<basic_string const &>(*this).end_(), first);
            } else {
                traits_type::move(first, last, (const_cast<basic_string const &>(*this).end_() - last));
            }
            // NOLINTEND
            resize_(size() - (last - first));
        }

        RAINY_CONSTEXPR20 bool static equal_(value_type const *begin, value_type const *end, value_type const *first,
                                             value_type const *last) noexcept {
            if (last - first != end - begin) {
                return false;
            }
            for (; begin != end; ++begin, ++first) {
                if (*first != *begin) {
                    return false;
                }
            }
            return true;
        }

        RAINY_CONSTEXPR20 void allocate_plus_one_(size_type count) {
            if (count <= short_string_max_) {
                size_flag_ = static_cast<unsigned char>(count);
                return;
            }
            if (count > max_size()) {
                throw std::length_error("basic_string: requested size exceeds max_size()");
            }
            ++count;
#if defined(__cpp_lib_allocate_at_least) && (__cpp_lib_allocate_at_least >= 202302L)
            auto [ptr, count_] = allocator_traits::allocate_at_least(this->get_al(), count);
            begin_lifetime(ptr, count_);
            utility::construct_at(&get_storage().ls_, ptr, nullptr, ptr + count_);
#else
            auto ptr = allocator_traits::allocate(this->get_al(), count);
            begin_lifetime(ptr, count);
            utility::construct_at(&get_storage().ls_, ptr, nullptr, ptr + count);
#endif
            size_flag_ = static_cast<unsigned char>(-1);
        }

        RAINY_CONSTEXPR20 void swap_without_ator(basic_string &right) noexcept {
            auto &&self = *this;
            std::swap(self.get_storage(), right.get_storage());
            std::swap(self.size_flag_, right.size_flag_);
        }

        RAINY_CONSTEXPR20 void resize_(size_type count) noexcept { // NOLINT
            if (!std::is_constant_evaluated()) {
                assert(("count > capacity()" && count <= capacity()));
            }
            if (empty()) {
                reserve(count);
            }
            if (is_long_()) {
                auto &&ls = get_storage().ls_;
                ls.end_ = ls.begin_ + count;
                if (!std::is_constant_evaluated()) {
                    *ls.end_ = CharType{};
                }
            } else {
                size_flag_ = static_cast<unsigned char>(count);
                if (!std::is_constant_evaluated()) {
                    get_storage().ss_[count] = CharType{};
                }
            }
        }

        RAINY_CONSTEXPR20 void assign_(value_type const *first, value_type const *last) {
            auto size = last - first;
            if (capacity() < static_cast<size_type>(size)) {
                if (is_long_()) {
                    auto ls = get_storage().ls_;
                    allocate_plus_one_(size);
                    dealloc_(ls);
                } else {
                    allocate_plus_one_(size);
                }
            }
            fill_(first, last);
            resize_(size);
        }

        RAINY_CONSTEXPR20 void insert_(size_type index, value_type const *first, value_type const *last) {
            auto size = this->size();
            if (index > size) {
                throw std::out_of_range{exception_string_};
            }
            auto length = last - first;
            auto new_size = size + length;
            auto begin = begin_();
            auto end = begin + size;
            auto start = begin + index;
            if (capacity() >= new_size) {
#if RAINY_HAS_CXX20
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) { // NOLINT
                    core::algorithm::copy_backward(start, end, end + length);
                    core::algorithm::copy(first, last, start);
                } else
#endif
                {
                    std::memmove(start + length, start, (end - start) * sizeof(CharType)); // NOLINT
                    std::memcpy(start, first, length * sizeof(CharType)); // NOLINT
                }
                // NOLINTEND
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + index;
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) { // NOLINT
                    core::algorithm::copy(begin, start, temp_begin);
                    core::algorithm::copy(first, last, temp_start);
                    core::algorithm::copy(start, end, temp_start + length);
                } else
#endif
                {
                    // NOLINTBEGIN
                    std::memcpy(temp_begin, begin, index * sizeof(CharType));
                    std::memcpy(temp_start, first, length * sizeof(CharType));
                    std::memcpy(temp_start + length, start, (end - start) * sizeof(CharType));
                    // NOLINTEND
                }
                temp.swap(*this);
            }
            resize_(new_size);
        }

        RAINY_CONSTEXPR20 void replace_(size_type pos, size_type count, value_type const *first2, value_type const *last2) {
            auto size = this->size();
            if (pos > size) {
                throw std::out_of_range{exception_string_};
            }
            auto begin = begin_();
            auto first1 = begin + pos;
            auto last1 = begin + (core::min) (pos + count, size);
            auto length1 = last1 - first1;
            auto length2 = last2 - first2;
            auto new_size = size - length1 + length2; // Fix 修复：符号对调
            auto end = begin + size;
            if (!(last1 < first2 || last2 < first1) && new_size <= capacity()) {
                auto diff = length1 - length2;
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) {
                    if (diff > 0) {
                        core::algorithm::copy(last1, end, last1 - diff);
                    } else if (diff < 0) {
                        core::algorithm::copy_backward(last1, end, end - diff);
                    }
                } else {
                    if (diff > 0) {
                        std::memmove(last1 + diff, last1, diff * sizeof(CharType));
                    } else if (diff < 0) {
                        std::memmove(last1 - diff, last1, -diff * sizeof(CharType));
                    }
                }
                // NOLINTEND
                if (first2 >= last1 && last2 <= end) {
                    first2 += diff;
                    last2 += diff;
                }
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) {
                    core::algorithm::copy(first2, last2, first1);
                } else {
                    std::memmove(first1, first2, length2 * sizeof(CharType));
                }
                // NOLINTEND
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + (first1 - begin);
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) {
                    core::algorithm::copy(begin, first1, temp_begin);
                    core::algorithm::copy(first2, last2, temp_start);
                    core::algorithm::copy(last1, end, temp_start + length2);
                } else {
                    // Fix 修复：dst/src 对调，括号保证优先级正确
                    std::memcpy(temp_begin, begin, (first1 - begin) * sizeof(CharType));
                    std::memcpy(temp_start, first2, length2 * sizeof(CharType));
                    std::memcpy(temp_start + length2, last1, (end - last1) * sizeof(CharType));
                }
                // NOLINTEND
                temp.swap(*this);
            }
            resize_(new_size);
        }

        RAINY_CONSTEXPR20 void dealloc_(ls_type_ &ls) noexcept {
            allocator_traits::deallocate(this->get_al(), ls.begin_, ls.last_ - ls.begin_);
        }

        RAINY_CONSTEXPR20 value_type const *begin_() const noexcept {
            if (is_long_()) {
                return get_storage().ls_.begin_;
            }
            return get_storage().ss_.data();
        }

        RAINY_CONSTEXPR20 CharType *begin_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).begin_());
        }

        RAINY_CONSTEXPR20 value_type const *end_() const noexcept {
            if (is_long_()) {
                return get_storage().ls_.end_;
            }
            return get_storage().ss_.data() + size_flag_;
        }

        RAINY_CONSTEXPR20 CharType *end_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).end_());
        }

        RAINY_CONSTEXPR20 Allocator &get_al() noexcept {
            return pair_.get_first();
        }

        RAINY_CONSTEXPR20 const Allocator &get_al() const noexcept {
            return pair_.get_first();
        }

        RAINY_CONSTEXPR20 storage_type_ &get_storage() noexcept {
            return pair_.get_second();
        }

        RAINY_CONSTEXPR20 const storage_type_ &get_storage() const noexcept {
            return pair_.get_second();
        }

        alignas(alignof(CharType)) unsigned char size_flag_{};
        utility::compressed_pair<Allocator, storage_type_> pair_;
    };

    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;

#if RAINY_HAS_CXX20
    using u8string = basic_string<char8_t>;
#endif

    template <typename CharType, typename Traits, typename Allocator>
    RAINY_CONSTEXPR20 void swap(basic_string<CharType, Traits, Allocator> &left,
                                basic_string<CharType, Traits, Allocator> &right) noexcept {
        left.swap(right);
    }

    template <typename StandardTraits, typename value_type, typename Traits, typename Alloc>
    std::basic_ostream<value_type, StandardTraits> &operator<<(std::basic_ostream<value_type, StandardTraits> &ostream,
                                                               const basic_string<value_type, Traits, Alloc> &str) {
        ostream.write(str.data(), str.size());
        return ostream;
    }
}

// NOLINTBEGIN
namespace std {
    template <typename value_type, typename Traits, typename Alloc>
    struct hash<rainy::foundation::text::basic_string<value_type, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::foundation::text::basic_string<value_type, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return rainy::utility::implements::hash_array_representation(val.data(), val.size());
        }
    };
}

namespace rainy::utility {
    template <typename value_type, typename Traits, typename Alloc>
    struct hash<rainy::foundation::text::basic_string<value_type, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::foundation::text::basic_string<value_type, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return implements::hash_array_representation(val.data(), val.size());
        }
    };
}
// NOLINTEND

#if RAINY_USING_GCC && !RAINY_USING_CLANG
#pragma GCC diagnostic pop
#endif

#endif
