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
#include <rainy/core/core.hpp>
#include <rainy/text/char_traits.hpp>

namespace rainy::text {
    template <typename CharType, typename Traits = text::char_traits<CharType>>
    class basic_string_view {
    public:
        using traits_type = Traits;
        using value_type = CharType;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using const_iterator = const value_type *;
        using iterator = const_iterator;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;
        using reverse_iterator = utility::reverse_iterator<const_iterator>;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        static constexpr size_type npos = static_cast<size_type>(-1);

        constexpr basic_string_view() noexcept = default;
        constexpr basic_string_view(const basic_string_view &) noexcept = default;
        constexpr basic_string_view &operator=(const basic_string_view &) noexcept = default;

        constexpr basic_string_view(const value_type *str) : data_{str}, size_{traits_type::length(str)} {
        }

        constexpr basic_string_view(std::nullptr_t) = delete;

        constexpr basic_string_view(const value_type *str, size_type length) : data_{str}, size_{length} {
        }

        template <typename It, typename End,
                  type_traits::other_trans::enable_if_t<
                      type_traits::extras::iterators::is_contiguous_iterator_v<It> &&
                          type_traits::type_relations::is_same_v<type_traits::extras::iterators::iter_value_t<It>, value_type> &&
                          !type_traits::type_relations::is_convertible_v<It, size_type>,
                      int> = 0>
        constexpr basic_string_view(It begin, End end) : data_{begin}, size_{utility::distance(begin, end)} {
        }

        constexpr const_iterator begin() const noexcept {
            return data_;
        }

        constexpr const_iterator end() const noexcept {
            return data_ + size_;
        }

        constexpr const_iterator cbegin() const noexcept {
            return data_;
        }

        constexpr const_iterator cend() const noexcept {
            return data_ + size_;
        }

        constexpr const_reverse_iterator rbegin() const noexcept {
            return reverse_iterator{end()};
        }

        constexpr const_reverse_iterator rend() const noexcept {
            return reverse_iterator{begin()};
        }

        constexpr const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator{cend()};
        }

        constexpr const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator{cbegin()};
        }

        constexpr size_type size() const noexcept {
            return size_;
        }

        constexpr size_type length() const noexcept {
            return size_;
        }

        constexpr size_type max_size() const noexcept {
            return (utility::numeric_limits<value_type>::max)();
        }

        constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        constexpr const_reference operator[](size_type pos) const {
            return data_[pos];
        }

        constexpr const_reference at(size_type pos) const {
            range_check(pos);
            return data_[pos];
        }

        constexpr const_reference front() const {
            return data_[0];
        }

        constexpr const_reference back() const {
            return data_[size() - 1];
        }

        constexpr const_pointer data() const noexcept {
            return data_;
        }

        constexpr void remove_prefix(size_type count) {
            assert(count < size_);
            data_ += count;
            size_ -= count;
        }

        constexpr void remove_suffix(size_type count) {
            assert(count < size_);
            size_ -= count;
        }

        constexpr void swap(basic_string_view &right) noexcept {
            std::swap(this->data_, right.data_);
            std::swap(this->size_, right.size_);
        }

        constexpr size_type copy(pointer dest, size_type count, size_type pos = 0) const {
            range_check(pos);
            size_type rcount = (core::min)(count, size() - pos);
            traits_type::copy(dest, data() + pos, rcount);
            return rcount;
        }

        constexpr basic_string_view substr(size_type pos = 0, size_type count = npos) const {
            range_check(pos);
            size_type rcount = (core::min)(count, size_ - pos);
            return {data_ + pos, rcount};
        }

        constexpr int compare(basic_string_view right) const noexcept {
            const size_type rlen = (core::min)(size_, right.size_);
            int result = traits_type::compare(data_, right.data_, rlen);
            if (result != 0) {
                return result;
            }
            return size_ == right.size_ ? 0 : (size_ < right.size_ ? -1 : 1);
        }

        constexpr int compare(size_type pos1, size_type n1, basic_string_view s) const {
            return substr(pos1, n1).compare(s);
        }

        constexpr int compare(size_type pos1, size_type n1, basic_string_view s, size_type pos2, size_type n2) const {
            return substr(pos1, n1).compare(s.substr(pos2, n2));
        }

        constexpr int compare(const CharType *s) const {
            return compare(basic_string_view(s));
        }

        constexpr int compare(size_type pos1, size_type n1, const CharType *s) const {
            return substr(pos1, n1).compare(basic_string_view(s));
        }

        constexpr int compare(size_type pos1, size_type n1, const CharType *s, size_type n2) const {
            return substr(pos1, n1).compare(basic_string_view(s, n2));
        }

        constexpr bool starts_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = this->begin();
            if (sv_size > this->size()) {
                return false;
            }
            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        constexpr bool starts_with(CharType ch) const noexcept {
            return *this->begin() == ch;
        }

        constexpr bool starts_with(const CharType *cstr) const {
            auto length = traits_type::length(cstr);
            auto begin = this->begin();
            if (length > this->size()) {
                return false;
            }
            return equal_(cstr, cstr + length, begin, begin + length);
        }

        constexpr bool ends_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = this->end();
            if (sv_size > this->size()) {
                return false;
            }
            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        constexpr bool ends_with(CharType ch) const noexcept {
            return *(this->end() - 1) == ch;
        }

        constexpr bool ends_with(CharType const *cstr) const {
            auto length = traits_type::length(cstr);
            if (length > this->size()) {
                return false;
            }
            auto end = this->end();
            return equal_(cstr, cstr + length, end - length, end);
        }

        constexpr bool contains(basic_string_view x) const noexcept {
            return find(x) != npos;
        }

        constexpr bool contains(CharType x) const noexcept {
            for (size_type i = 0; i < size(); ++i) {
                if (data_[i] == x) {
                    return true;
                }
            }
            return false;
        }

        constexpr bool contains(const CharType *x) const {
            return find(x) != npos;
        }

        constexpr size_type find(basic_string_view s, size_type pos = 0) const noexcept {
            if (pos > size_) {
                return npos;
            }
            if (s.size_ == 0) {
                return pos;
            }
            if (s.size_ > size_ - pos) {
                return npos;
            }
            const CharType *result = nullptr;
            for (auto p = data_ + pos; p <= data_ + size_ - s.size_; ++p) {
                if (traits_type::compare(p, s.data_, s.size_) == 0) {
                    result = p;
                    break;
                }
            }
            return result ? result - data_ : npos;
        }

        constexpr size_type find(CharType c, size_type pos = 0) const noexcept {
            if (pos >= size_) {
                return npos;
            }
            const CharType *result = traits_type::find(data_ + pos, size_ - pos, c);
            return result ? result - data_ : npos;
        }

        constexpr size_type find(const CharType *s, size_type pos, size_type n) const {
            return find(basic_string_view(s, n), pos);
        }

        constexpr size_type find(const CharType *s, size_type pos = 0) const {
            return find(basic_string_view(s), pos);
        }

        constexpr size_type rfind(basic_string_view s, size_type pos = npos) const noexcept {
            if (s.size_ == 0) {
                return (core::min)(pos, size_);
            }
            if (s.size_ > size_) {
                return npos;
            }
            pos = (core::min)(pos, size_ - s.size_);
            for (auto p = data_ + pos; p >= data_; --p) {
                if (traits_type::compare(p, s.data_, s.size_) == 0) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type rfind(CharType c, size_type pos = npos) const noexcept {
            if (size_ == 0) {
                return npos;
            }
            pos = (core::min)(pos, size_ - 1);
            for (auto p = data_ + pos; p >= data_; --p) {
                if (traits_type::eq(*p, c)) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type rfind(const CharType *s, size_type pos, size_type n) const {
            return rfind(basic_string_view(s, n), pos);
        }

        constexpr size_type rfind(const CharType *s, size_type pos = npos) const {
            return rfind(basic_string_view(s), pos);
        }

        constexpr size_type find_first_of(basic_string_view s, size_type pos = 0) const noexcept {
            for (auto p = data_ + pos; p < data_ + size_; ++p) {
                if (s.find(*p) != npos) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type find_first_of(CharType c, size_type pos = 0) const noexcept {
            return find(c, pos);
        }

        constexpr size_type find_first_of(const CharType *s, size_type pos, size_type n) const {
            return find_first_of(basic_string_view(s, n), pos);
        }

        constexpr size_type find_first_of(const CharType *s, size_type pos = 0) const {
            return find_first_of(basic_string_view(s), pos);
        }

        constexpr size_type find_last_of(basic_string_view s, size_type pos = npos) const noexcept {
            if (size_ == 0)
                return npos;
            pos = (core::min)(pos, size_ - 1);
            for (auto p = data_ + pos; p >= data_; --p) {
                if (s.find(*p) != npos) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type find_last_of(CharType c, size_type pos = npos) const noexcept {
            return rfind(c, pos);
        }

        constexpr size_type find_last_of(const CharType *s, size_type pos, size_type n) const {
            return find_last_of(basic_string_view(s, n), pos);
        }

        constexpr size_type find_last_of(const CharType *s, size_type pos = npos) const {
            return find_last_of(basic_string_view(s), pos);
        }

        constexpr size_type find_first_not_of(basic_string_view s, size_type pos = 0) const noexcept {
            for (auto p = data_ + pos; p < data_ + size_; ++p) {
                if (s.find(*p) == npos) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type find_first_not_of(CharType c, size_type pos = 0) const noexcept {
            for (auto p = data_ + pos; p < data_ + size_; ++p) {
                if (!traits_type::eq(*p, c)) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type find_first_not_of(const CharType *s, size_type pos, size_type n) const {
            return find_first_not_of(basic_string_view(s, n), pos);
        }

        constexpr size_type find_first_not_of(const CharType *s, size_type pos = 0) const {
            return find_first_not_of(basic_string_view(s), pos);
        }

        constexpr size_type find_last_not_of(basic_string_view s, size_type pos = npos) const noexcept {
            if (size_ == 0)
                return npos;
            pos = (core::min)(pos, size_ - 1);
            for (auto p = data_ + pos; p >= data_; --p) {
                if (s.find(*p) == npos) {
                    return p - data_;
                }
            }
            return npos;
        }

        constexpr size_type find_last_not_of(CharType c, size_type pos = npos) const noexcept {
            if (size_ == 0) {
                return npos;
            }
            if (pos >= size_) {
                pos = size_ - 1;
            }
            for (size_type i = pos + 1; i > 0; --i) {
                if (!traits_type::eq(data_[i - 1], c)) {
                    return i - 1;
                }
            }
            return npos;
        }

        constexpr size_type find_last_not_of(const CharType *s, size_type pos, size_type n) const {
            return find_last_not_of(basic_string_view(s, n), pos);
        }

        constexpr size_type find_last_not_of(const CharType *s, size_type pos = npos) const {
            return find_last_not_of(basic_string_view(s), pos);
        }

        template <typename Elem, typename UTy>
        friend std::basic_ostream<Elem> &operator<<(std::basic_ostream<Elem> & left,basic_string_view<UTy> right) {
            return left.write(right.data(), right.size());
        }

    private:
        static constexpr bool equal_(CharType const *begin, CharType const *end, CharType const *first,
                                     CharType const *last) noexcept {
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

        constexpr void range_check(const std::size_t offset) const {
            if (size_ <= offset) {
                foundation::exceptions::logic::throw_out_of_range("Invalid subscript");
            }
        }

        const_pointer data_{nullptr};
        size_type size_{0};
    };

    template <class It, class End>
    basic_string_view(It, End) -> basic_string_view<type_traits::extras::iterators::iter_value_t<It>>;

    using string_view = basic_string_view<char>;
    using wstring_view = basic_string_view<wchar_t>;
    using u16string_view = basic_string_view<char16_t>;
    using u32string_view = basic_string_view<char32_t>;
#if RAINY_HAS_CXX20
    using u8string_view = basic_string_view<char8_t>;
#endif
}
