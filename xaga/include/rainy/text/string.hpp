#ifndef RAINY_COLLECTIONS_STRING_HPP
#define RAINY_COLLECTIONS_STRING_HPP
#include <cstring>
#include <cwchar>
#include <iterator>
#include <memory>
#include <rainy/text/string_view.hpp>
#include <rainy/core/core.hpp>
#include <rainy/text/char_traits.hpp>

#if RAINY_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

namespace rainy::text {
    template <typename CharType, class Traits = std::char_traits<CharType>, class Allocator = std::allocator<CharType>>
    class basic_string {
    public:
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
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static inline constexpr size_type npos = static_cast<size_type>(-1);

        RAINY_CONSTEXPR20 basic_string() noexcept : pair_{{}, {}} {
        }

        RAINY_CONSTEXPR20 basic_string(allocator_type const &allocator) noexcept : pair_{allocator, {}} {
        }

        RAINY_CONSTEXPR20 basic_string(size_type count, CharType ch, allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            allocate_plus_one_(count);
            auto begin = begin_();
            auto end = begin + count;
            std::fill(begin, end, ch);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, size_type count, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            auto other_size = right.size();

            if (pos > other_size)
                throw std::out_of_range{exception_string_};

            count = (core::min)(other_size - pos, count);
            allocate_plus_one_(count);
            auto start = right.begin_() + pos;
            fill_(start, start + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, allocator_type const &allocator = allocator_type()) :
            basic_string(right, pos, right.size() - pos, allocator) {
        }

        RAINY_CONSTEXPR20 basic_string(const value_type *ptr, size_type count, allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            allocate_plus_one_(count);
            fill_(ptr, ptr + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const value_type *ptr, allocator_type const &allocator = allocator_type()) :
            basic_string(ptr, traits_type::length(ptr), allocator) {
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string(InputIt first, InputIt last) {
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto length = std::ranges::distance(first, last);
                allocate_plus_one_(length);
                std::ranges::copy(first, last, begin_());
                resize_(length);
            } else {
                for (; first != last; ++first)
                    push_back(*first);
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

        RAINY_CONSTEXPR20 basic_string(basic_string &&right, size_type pos, size_type count, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            auto other_size = right.size();
            if (pos > other_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min)(other_size - pos, count);

            if (pos != 0) {
                auto other_begin = right.begin_();
                auto start = other_begin + pos;
                auto last = start + count;

                if (std::is_constant_evaluated()) {
                    std::copy(start, last, other_begin);
                } else {
                    std::memmove(other_begin, start, (last - start) * sizeof(CharType));
                }
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
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &sv, allocator_type const &allocator = allocator_type()) :
            basic_string(std::basic_string_view<value_type>{sv}.data(), std::basic_string_view<value_type>{sv}.size(), allocator) {
        }


        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &sv, size_type pos, size_type count, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            auto data = sv.data();
            auto sv_size = sv.size();

            if (pos > sv_size)
                throw std::out_of_range{exception_string_};

            count = (core::min)(sv_size - pos, count);
            allocate_plus_one_(count);
            fill_(data, data + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 ~basic_string() {
            if (is_long_()) {
                dealloc_(get_storage().ls_);            
            }
        }

        basic_string(std::nullptr_t) = delete;
        RAINY_CONSTEXPR20 basic_string &operator=(std::nullptr_t) = delete;

        RAINY_CONSTEXPR20 basic_string &operator=(basic_string &&right) noexcept(
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Allocator>::is_always_equal::value) {
            return assign(utility::move(right));
        }

        RAINY_CONSTEXPR20 basic_string &operator=(const basic_string &right) {
            return assign(right);
        }

        RAINY_CONSTEXPR20 basic_string &operator=(const value_type *ptr) {
            return assign(ptr);
        }

        RAINY_CONSTEXPR20 basic_string &operator=(CharType ch) {
            resize_(1);
            (*begin_()) = ch;

            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator=(std::initializer_list<CharType> ilist) {
            return assign(ilist);
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator=(const StringViewLike &sv) {
            return assign(sv);
        }

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return !size_flag_;
        }

        RAINY_CONSTEXPR20 std::size_t size() const noexcept {
            if (is_long_()) {
                auto &&ls = get_storage().ls_;
                return ls.end_ - ls.begin_;
            }
            return size_flag_;
        }

        RAINY_CONSTEXPR20 std::size_t length() const noexcept {
            return size();
        }

        RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return size_type{-1} / sizeof(CharType) / 2;
        }

        RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            if (is_long_()) {
                auto &&ls = get_storage().ls_;

                return ls.last_ - ls.begin_ - 1;
            } else {
                return short_string_max_;
            }
        }

        RAINY_CONSTEXPR20 void shrink_to_fit() noexcept {
            if (size() <= short_string_max_ && is_long_()) {
                auto ls = get_storage().ls_;
                if (std::is_constant_evaluated()) {
                    get_storage().ss_ = decltype(get_storage().ss_){};
                    std::copy(ls.begin_, ls.end_, get_storage().ss_.data());
                } else {
                    std::memcpy(get_storage().ss_.data(), ls.begin_, (ls.end_ - ls.begin_) * sizeof(CharType));
                }

                dealloc_(ls);
            }
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
            if (pos >= size())
                throw std::out_of_range{exception_string_};

            return *(begin_() + pos);
        }

        RAINY_CONSTEXPR20 reference at(size_type pos) {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).at(pos));
        }

        RAINY_CONSTEXPR20 const_reference operator[](size_type pos) const noexcept {
            assert(("pos >= size, please check the arg", pos < size()));

            return *(begin_() + pos);
        }

        RAINY_CONSTEXPR20 reference operator[](size_type pos) noexcept {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this)[pos]);
        }

        RAINY_CONSTEXPR20 const CharType &front() const noexcept {
            assert(("string is empty", !empty()));

            return *begin_();
        }

        RAINY_CONSTEXPR20 CharType &front() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).front());
        }

        RAINY_CONSTEXPR20 const CharType &back() const noexcept {
            assert(("string is empty", !empty()));

            return *(end_() - 1);
        }

        RAINY_CONSTEXPR20 CharType &back() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).back());
        }

        RAINY_CONSTEXPR20 operator std::basic_string_view<value_type>() const noexcept {
            return std::basic_string_view<value_type>(begin_(), end_());
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
    
        RAINY_CONSTEXPR20 void reserve(size_type new_cap) {
            if (capacity() >= new_cap) {
                return;
            }
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

        RAINY_CONSTEXPR20 void resize(size_type count, CharType ch) {
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

        RAINY_CONSTEXPR20 void push_back(CharType ch) {
            auto size = this->size();
            if (capacity() == size) {
                reserve(size * 2 - size / 2);
            }
            *end_() = ch;
            resize_(size + 1);
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
                auto size = size();
                auto length = std::ranges::distance(first, last);
                auto new_size = size + length;
                reserve(std::max(size * 2, new_size));
                std::ranges::copy(first, last, begin_() + size);
                resize_(new_size);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(size_type count, CharType ch) {
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
            count = (core::min)(npos, (core::min)(str_size - pos, count));
            auto str_begin = begin_();
            assign_(str_begin + pos, str_begin + pos + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(basic_string &&right) noexcept {
            if RAINY_CONSTEXPR20 (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
                right.swap(*this);
            } else {
                if (this->get_al() == right.get_al()) {
                    right.swap_without_ator(*this);
                }
                else {
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
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        basic_string &assign(const StringViewLike &sv) {
            auto data = sv.data();
            assign_(data, data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &assign(const StringViewLike &sv, size_type pos, size_type count = npos) {
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{exception_string_};            
            }
            count = (core::min)(npos, (core::min)(sv_size - pos, count));
            auto data = sv.data();
            assign_(data + pos, data + pos + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(size_type count, CharType ch) {
            auto size = size();
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
            count = (core::min)(npos, (core::min)(str_size - pos, count));
            return append(str.begin_() + pos, count);
        }

        RAINY_CONSTEXPR20 basic_string &append(const value_type *ptr) {
            append_(ptr, ptr + c_style_string_length_(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(std::initializer_list<CharType> ilist) {
            append_(ilist.begin(), ilist.end());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &sv) {
            auto data = sv.data();
            append_(data, data + sv.size());
            return *this;
        }

        template <typename StringViewLike, type_traits::other_trans::enable_if_t<
                                            type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &sv, size_type pos, size_type count = npos) {
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min)(npos, (core::min)(sv_size - count, count));
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
            return push_back(ch);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(const value_type *ptr) {
            return append(ptr);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(std::initializer_list<CharType> ilist) {
            return append(ilist);
        }

        RAINY_CONSTEXPR20 size_type find(std::basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
            if (pos > size()) {
                return npos;
            }
            if (ptr.size() == 0) {
                return pos;
            }
            if (ptr.size() > size() - pos) {
                return npos;
            }
            const value_type *result = nullptr;
            for (auto p = this->begin_() + pos; p <= this->begin_() + size() - ptr.size(); ++p) {
                if (traits_type::compare(p, ptr.begin_(), ptr.size) == 0) {
                    result = p;
                    break;
                }
            }
            return result ? result - this->begin_() : npos;
        }

        RAINY_CONSTEXPR20 size_type find(value_type c, size_type pos = 0) const noexcept {
            if (pos >= size()) {
                return npos;
            }
            const value_type *result = traits_type::find(this->begin_() + pos, size() - pos, c);
            return result ? result - this->begin_() : npos;
        }

        RAINY_CONSTEXPR20 size_type find(const value_type *ptr, size_type pos, size_type count) const {
            return find(std::basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find(const value_type *ptr, size_type pos = 0) const {
            return find(std::basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type rfind(std::basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (ptr.size() == 0) {
                return (core::min)(pos, size());
            }
            if (ptr.size() > size()) {
                return npos;
            }
            pos = (core::min)(pos, size() - ptr.size());
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (traits_type::compare(p, ptr.begin_(), ptr.size()) == 0) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type rfind(value_type c, size_type pos = npos) const noexcept {
            if (size() == 0) {
                return npos;
            }
            pos = (core::min)(pos, size() - 1);
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

        RAINY_CONSTEXPR20 size_type find_first_of(std::basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
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
            return find_first_of(std::basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find_first_of(const value_type *ptr, size_type pos = 0) const {
            return find_first_of(std::basic_string_view<value_type>(ptr), pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_of(std::basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (size() == 0) {
                return npos;
            }
            pos = (core::min)(pos, size() - 1);
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

        RAINY_CONSTEXPR20 size_type find_first_not_of(std::basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
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

        RAINY_CONSTEXPR20 size_type find_last_not_of(std::basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (size() == 0) {
                return npos;
            }
            pos = (core::min)(pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (ptr.find(*p) == npos) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        RAINY_CONSTEXPR20 size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept {
            if (size() == 0) {
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
            return find_last_not_of(std::basic_string_view<value_type>(ptr, count), pos);
        }

        RAINY_CONSTEXPR20 size_type find_last_not_of(const value_type *ptr, size_type pos = npos) const {
            return find_last_not_of(std::basic_string_view<value_type>(ptr), pos);
        }
        
        RAINY_CONSTEXPR20 bool starts_with(std::basic_string_view<value_type> sv) const noexcept {
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
            auto length = c_style_string_length_(ptr);
            auto begin = begin_();
            if (length > size()) {
                return false;
            }
            return equal_(ptr, ptr + length, begin, begin + length);
        }

        RAINY_CONSTEXPR20 bool ends_with(std::basic_string_view<value_type> sv) const noexcept {
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
            auto length = c_style_string_length_(ptr);
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
            if (size() < size) {
                return false;
            }
            // opti for starts_with
            if (equal_(begin, begin + size, data, data + size)) {
                return true;
            }
            return find(sv) != npos;
        }

        RAINY_CONSTEXPR20 bool contains(CharType ch) const noexcept {
            for (auto begin = begin_(), end = end_(); begin != end; ++begin) {
                if (*begin == ch)
                    return true;
            }
            return false;
        }

        RAINY_CONSTEXPR20 bool contains(const value_type *ptr) const noexcept {
            return std::basic_string_view<value_type>{begin_(), end_()}.contains(
                std::basic_string_view<value_type>{ptr, ptr + c_style_string_length_(ptr)});
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, size_type count, CharType ch) {
            auto size = size();
            if (index > size) {
                throw std::out_of_range{exception_string_};
            }
            reserve(size + count);
            auto start = begin_() + index;
            auto end = start + size - index;
            auto last = end + count;
            if (std::is_constant_evaluated()) {
                std::copy_backward(start, end, last);
                std::fill(start, start + count, ch);
            } else {
                std::memmove(end, start, count * sizeof(CharType));
                std::fill(start, start + count, ch);
            }
            resize_(size + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const value_type *ptr, size_type count) {
            insert_(index, ptr, ptr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const value_type *ptr) {
            insert_(index, ptr, ptr + c_style_string_length_(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str) {
            insert_(index, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos) {
            auto s_size = str.size();

            if (s_index > s_size)
                throw std::out_of_range{exception_string_};

            count = (core::min)(npos, (core::min)(s_size - s_index, count));
            auto s_start = str.begin_() + s_index;
            insert_(index, s_start, s_start + count);
            return *this;
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, CharType ch) {
            auto size = size();
            auto start = pos;
            auto end = end_();
            auto index = start - begin_();
            reserve(size + 1);
            auto begin = begin_();
            end = begin + size;
            start = begin + index;
            if (std::is_constant_evaluated()) {
                core::algorithm::copy_backward(start, end, end + 1);
            } else {
                traits_type::move(end, start, 1);
            }
            *start = ch;
            resize_(size + 1);
            return {start};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, size_type count, CharType ch) {
            auto start = pos;
            auto index = start - begin_();
            insert(index, count, ch);
            return {start};
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, InputIt first, InputIt last) {
            assert(("pos isn'sv in this string", pos >= begin_() && pos <= end_()));
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

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, std::initializer_list<CharType> ilist) {
            auto i_data = std::data(ilist);
            auto start = pos;
            insert_(start - begin_(), i_data, i_data + ilist.size());
            return {start};
        }


        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &sv) {
            auto sv_data = sv.data();
            insert_(pos, sv_data, sv_data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &sv, size_type t_index, size_type count = npos) {
            auto sv_size = sv.size();
            auto size = size();
            if (t_index > sv_size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min)(npos, (core::min)(sv_size - t_index, count));
            auto sv_data = sv.data();
            insert_(pos, sv_data + t_index, sv_data + t_index + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &erase(size_type index = 0, size_type count = npos) {
            auto size = this->size();
            if (index > size) {
                throw std::out_of_range{exception_string_};
            }
            count = (core::min)(npos, (core::min)(size - index, count));
            auto start = begin_() + index;
            erase_(start, start + count);

            return *this;
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator position) noexcept {
            auto start = position.base().current_;
            erase_(start, start + 1);
            return {start};
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) noexcept {
            auto start = first;
            erase_(start, last.base().current_);
            return {start};
        }

        RAINY_CONSTEXPR20 void pop_back() noexcept {
            assert(("string is empty", !empty()));
            resize_(size() - 1);
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str) {
            replace_(pos, count, str.begin_(), str.end_());
            return *pos;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const basic_string &str) {
            auto start = first;
            replace_(start - begin_(), last - first, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str, size_type pos2,
                                        size_type count2 = npos) {
            auto str_size = str.size();
            if (pos2 > str_size) {
                throw std::out_of_range{exception_string_};
            }
            count2 = (core::min)(npos, (core::min)(count2, str_size - pos2));
            auto begin = str.begin_();
            replace_(pos, count, begin + count2, begin + count2 + pos2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const value_type *ptr, size_type count2) {
            replace_(pos, count, ptr, ptr + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const value_type *ptr, size_type count2) {
            auto start = first;
            replace_(start - begin_(), last - first, ptr, ptr + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const value_type *ptr) {
            replace_(pos, count, ptr, ptr + c_style_string_length_(ptr));
            return *pos;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const value_type *ptr) {
            auto start = first;
            replace_(start - begin_(), last - first, ptr, ptr + c_style_string_length_(ptr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            auto start = first;
            replace_(start - begin_(), last - first, data, data + ilist.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const StringViewLike &sv) {
            auto data = sv.data();
            replace_(pos, count, data, data + sv.size());
            return *this;
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
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
            count2 = (core::min)(npos, (core::min)(sv_size - pos2, count2));
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

        template <typename InputIt, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
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

        friend RAINY_CONSTEXPR20 bool operator==(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                if (lsize != rsize)
                    return false;
                for (auto l = left.begin_(), r = right.begin_(), end = l + lsize; l != end; ++l, ++r) {
                    if (*l != *r) {
                        return false;
                    }
                }
                return true;
            } else
#endif
            {
                return basic_string::traits_type::compare(left.begin_(), right.begin_(), (core::min)(lsize, rsize)) == 0 &&
                       lsize == rsize;
            }
        }

        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto l = left.begin_(), r = right.begin_(), end = l + (core::min)(lsize, rsize); l != end; ++l, ++r) {
                    if (*l < *r)
                        return true;
                    else if (*l > *r)
                        return false;
                }
                return lsize < rsize;
            } else
#endif
            {
                auto res = basic_string::traits_type::compare(left.begin_(), right.begin_(), (core::min)(rsize, lsize));
                if (res < 0) {
                    return true;
                } else if (res > 0) {
                    return false;
                }
                return lsize < rsize;
            }
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
            auto rsize = basic_string::traits_type::length(start);
            auto lsize = left.size();

            if (lsize != rsize)
                return false;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto l = left.begin_(), end = l + lsize; l != end; ++l, ++start) {
                    if (*l != *start)
                        return false;
                }
                return true;
            } else
#endif
            {
                return basic_string::traits_type::compare(left.begin_(), start, lsize) == 0;
            }
        }

        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = basic_string::traits_type::length(start);
            auto lsize = left.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto l = left.begin_(), end = l + (core::min)(lsize, rsize); l != end; ++l, ++start) {
                    if (*l < *start)
                        return true;
                    else if (*l > *start)
                        return false;
                }
                return lsize < rsize;
            } else
#endif
            {
                auto res = basic_string::traits_type::compare(left.begin_(), start, (core::min)(rsize, lsize));
                if (res < 0)
                    return true;
                else if (res > 0)
                    return false;
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
        static inline RAINY_CONSTEXPR20 std::size_t short_string_max_{sizeof(CharType *) * 4 / sizeof(CharType) - 2};

        struct ls_type_ {
            CharType *begin_{};
            CharType *end_{};
            CharType *last_{};
        };

        union storage_type_ {
            std::array<CharType, short_string_max_ + 1> ss_;
            ls_type_ ls_;
        };

        static inline char exception_string_[] = "parameter is out of range, please check it.";

        RAINY_CONSTEXPR20 bool is_long_() const noexcept {
            return size_flag_ == decltype(size_flag_)(-1);
        }

        RAINY_CONSTEXPR20 bool is_short_() const noexcept {
            return size_flag_ != decltype(size_flag_)(-1);
        }

        RAINY_CONSTEXPR20 void fill_(value_type const *begin, value_type const *end) noexcept {
            assert(("cannot storage string in current allocated storage", static_cast<size_type>(end - begin) <= capacity()));
            traits_type::move(begin_(), begin, end - begin);
        }

        RAINY_CONSTEXPR20 void append_(value_type const *first, value_type const *last) {
            auto length = last - first;
            auto size = this->size();
            reserve(size + length);
            auto end = begin_() + size;
            traits_type::copy(end, first, last - first);
            resize_(size + length);
        }

        RAINY_CONSTEXPR20 void erase_(CharType *first, value_type const *last) noexcept {
            assert(("first or last is not in this string", first >= begin_() && last <= end_()));
            if (std::is_constant_evaluated()) {
                core::algorithm::copy(last, const_cast<basic_string const &>(*this).end_(), first);
            } else {
                traits_type::move(first, last, (const_cast<basic_string const &>(*this).end_() - last));
            }
            resize_(size() - (last - first));
        }

        RAINY_CONSTEXPR20 bool static equal_(value_type const *begin, value_type const *end, value_type const *first, value_type const *last) noexcept {
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
            if (count <= short_string_max_ && !is_long_()) {
                size_flag_ = static_cast<signed char>(count);
                return;
            }
            ++count;
#if defined(__cpp_lib_allocate_at_least) && (__cpp_lib_allocate_at_least >= 202302L)
            auto [ptr, count_] = allocator_traits::allocate_at_least(this->get_al(), count);
            get_storage().ls_ = {ptr, nullptr, ptr + count_};
#else
            auto ptr = allocator_traits::allocate(this->get_al(), count);
            get_storage().ls_ = {ptr, nullptr, ptr + count};
#endif
            size_flag_ = static_cast<unsigned char>(-1);
        }

        RAINY_CONSTEXPR20 void swap_without_ator(basic_string &right) noexcept {
            auto &&self = *this;
            std::ranges::swap(self.get_storage(), right.get_storage());
            std::ranges::swap(self.size_flag_, right.size_flag_);
        }

        RAINY_CONSTEXPR20 void resize_(size_type count) noexcept {
            assert(("count > capacity()", count <= capacity()));

            if (is_long_()) {
                auto &&ls = get_storage().ls_;
                ls.end_ = ls.begin_ + count;
                *ls.end_ = CharType{};
            } else {
                size_flag_ = static_cast<unsigned char>(count);
                get_storage().ss_[count] = CharType{};

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
            auto size = size();

            if (index > size)
                throw std::out_of_range{exception_string_};

            auto length = last - first;
            auto new_size = size + length;
            auto begin = begin_();
            auto end = begin + size;
            auto start = begin + index;

            // if start is not in range and capacity > length + size
            if ((start < first || start > last) && capacity() > new_size) {

                if (std::is_constant_evaluated()) {
                    std::copy_backward(start, end, end + length);
                } else {
                    std::memmove(end, start, length * sizeof(CharType));
                }

                // re-cacl first and last, because range is in *this
                if (first >= start && last <= end) {
                    first += index;
                    last += index;
                }

                if (std::is_constant_evaluated()) {
                    std::copy(first, last, start);
                } else {
                    std::memmove(start, first, length * sizeof(CharType));
                }
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + index;
                auto temp_end = temp_begin + size;

                if (std::is_constant_evaluated()) {
                    std::copy(begin, start, temp_begin);
                    std::copy(first, last, temp_start);
                    std::copy(start, end, temp_start + length);
                } else {
                    std::memcpy(temp_begin, begin, index * sizeof(CharType));
                    std::memcpy(temp_start, first, length * sizeof(CharType));
                    std::memcpy(temp_start + length, start, (end - start) * sizeof(CharType));
                }

                temp.swap(*this);
            }

            resize_(new_size);
        }

        RAINY_CONSTEXPR20 void replace_(size_type pos, size_type count, value_type const *first2, value_type const *last2) {
            auto size = size();

            if (pos > size)
                throw std::out_of_range{exception_string_};

            auto begin = begin_();
            auto first1 = begin + pos;
            auto last1 = begin + (core::min)(pos + count, size);
            auto length1 = last1 - first1;
            auto length2 = last2 - first2;
            auto new_size = size + length1 - length2;
            auto end = begin + size;

            if (!(last1 < first2 || last2 < first1) && new_size <= capacity()) {
                auto diff = length1 - length2;

                if (std::is_constant_evaluated()) {
                    if (diff > 0)
                        std::copy(last1, end, last1 - diff);
                    else if (diff < 0)
                        std::copy_backward(last1, end, end - diff);
                } else {
                    if (diff > 0)
                        std::memmove(last1 + diff, last1, diff * sizeof(CharType));
                    else if (diff < 0)
                        std::memmove(last1 - diff, last1, -diff * sizeof(CharType));
                }

                if (first2 >= last1 && last2 <= end) {
                    first2 += diff;
                    last2 += diff;
                }

                if (std::is_constant_evaluated()) {
                    std::copy(first2, last2, first1);
                } else {
                    std::memmove(first1, first2, length2 * sizeof(CharType));
                }
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp.begin_() + (first1 - begin);

                if (std::is_constant_evaluated()) {
                    std::copy(begin, first1, temp_begin);
                    std::copy(first2, last2, temp_start);
                    std::copy(last1, end, temp_start + length2);
                } else {
                    std::memcpy(begin, temp_begin, first1 - begin * sizeof(CharType));
                    std::memcpy(temp_start, first2, length2 * sizeof(CharType));
                    std::memcpy(temp_start + length2, last1, (end - last1) * sizeof(CharType));
                }

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
            } else {
                return get_storage().ss_.data();
            }
        }

        RAINY_CONSTEXPR20 CharType *begin_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).begin_());
        }

        RAINY_CONSTEXPR20 value_type const *end_() const noexcept {
            if (is_long_())
                return get_storage().ls_.end_;
            else
                return get_storage().ss_.data() + size_flag_;
        }

        RAINY_CONSTEXPR20 CharType *end_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).end_());
        }
        
        Allocator &get_al() noexcept {
            return pair_.get_first();
        }

        const Allocator &get_al() const noexcept {
            return pair_.get_first();
        }

        storage_type_ &get_storage() noexcept {
            return pair_.get_second();
        }

        const storage_type_ &get_storage() const noexcept {
            return pair_.get_second();
        }

        alignas(alignof(CharType)) unsigned char size_flag_{};
        utility::compressed_pair<Allocator, storage_type_> pair_;
    };

    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
    using u8string = basic_string<char8_t>;
    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;

    template <typename CharType, typename Traits, typename Allocator>
    RAINY_CONSTEXPR20 void swap(basic_string<CharType, Traits, Allocator> &left, basic_string<CharType, Traits, Allocator> &right) {
        left.swap(right);
    }

    template <typename StandardTraits,typename value_type, typename Traits, typename Alloc>
    std::basic_ostream<value_type, StandardTraits>& operator<<(std::basic_ostream<value_type, StandardTraits>& ostream, const basic_string<value_type, Traits, Alloc>& str) {
        ostream.write(str.data(), str.size());
        return ostream;
    }
}

namespace std {
    template <typename value_type, typename Traits, typename Alloc>
    struct hash<rainy::text::basic_string<value_type, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::text::basic_string<value_type, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return rainy::utility::implements::hash_array_representation(val.data(), val.size());
        }
    };
}

namespace rainy::utility {
    template <typename value_type, typename Traits, typename Alloc>
    struct hash<rainy::text::basic_string<value_type, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = text::basic_string<value_type, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return implements::hash_array_representation(val.data(), val.size());
        }
    };
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif