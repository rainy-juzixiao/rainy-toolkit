#pragma once

#include <array>
#include <cassert>
#if RAINY_HAS_CXX20
#include <compare>
#include <concepts>
#endif
#include <cstring>
#include <cwchar>
#include <iterator>
#include <memory>
#include <rainy/core/core.hpp>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <version>


namespace rainy::text::implements {
    template <typename CharType, typename Traits, typename Allocator>
    class basic_string_storage {
    public:
        using traits_type = Traits;
        using value_type = CharType;
        using allocator_type = Allocator;
        using allocator_traits = std::allocator_traits<allocator_type>;
        using size_type = typename allocator_traits::size_type;
        using difference_type = typename allocator_traits::difference_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = typename allocator_traits::pointer;
        using const_pointer = typename allocator_traits::const_pointer;

        basic_string_storage() = default;

        basic_string_storage(const Allocator &allocator) : allocator_{allocator} {
        }

        constexpr bool empty() const noexcept {
            return this->is_empty_();
        }

        constexpr std::size_t size() const noexcept {
            return this->size_();
        }

        constexpr std::size_t length() const noexcept {
            return this->size_();
        }

        /**
         * @return size_type{ -1 } / sizeof(CharType) / 2
         */
        constexpr size_type max_size() const noexcept {
            return size_type{-1} / sizeof(CharType) / 2;
        }

        constexpr size_type capacity() const noexcept {
            if (this->is_long_()) {
                auto &&ls = this->stor_.ls_;
                return ls.last_ - ls.begin_ - 1;
            } else {
                return this->short_string_max_;
            }
        }

    protected:
        /**
         * @brief type of long string
         */
        struct ls_type_ {
            CharType *begin_{};
            CharType *end_{};
            CharType *last_{};
        };

        static inline constexpr std::size_t short_string_max_{sizeof(CharType *) * 4 / sizeof(CharType) - 2};

        /**
         * @brief union storage long string and short string
         */

        union storage_type_ {
            std::array<CharType, short_string_max_ + 1> ss_;
            ls_type_ ls_;
        };

        /**
         * https://github.com/microsoft/STL/issues/1364
         */
#ifdef _MSC_VER
        [[msvc::no_unique_address]] Allocator allocator_{};
#else
        [[no_unique_address]] Allocator allocator_{};
#endif

        /**
         * @brief storage of string
         */
        storage_type_ stor_{};

        /**
         * @brief flag > 0: 使用小字符串优化，size_flag_将代表当前小字符串使用的大小
         * @brief flag = 0: 未在使用
         * @brief flag = MAX: 长字符串，约定使用 end - begin 作为长度
         */
        alignas(alignof(CharType)) unsigned char size_flag_{};

        static inline char exception_string_[] = "parameter is out of range, please check it.";

        constexpr bool is_long_() const noexcept {
            return this->size_flag_ == decltype(this->size_flag_)(-1);
        }

        constexpr bool is_short_() const noexcept {
            return this->size_flag_ != decltype(this->size_flag_)(-1);
        }

        constexpr bool is_empty_() const noexcept {
            return !this->size_flag_;
        }

        constexpr std::size_t size_() const noexcept {
            if (this->is_long_()) {
                auto &&ls = stor_.ls_;

                return ls.end_ - ls.begin_;
            } else {
                return this->size_flag_;
            }
        }
    };
}
namespace rainy::text {
    template <typename CharType, class Traits = std::char_traits<CharType>, typename Allocator = std::allocator<CharType>>
    class basic_string : public implements::basic_string_storage<CharType, Traits, Allocator> {
    public:
        using base = implements::basic_string_storage<CharType, Traits, Allocator>;

        using traits_type = typename base::traits_type;
        using value_type = typename base::value_type;
        using allocator_type = typename base::allocator_type;
        using allocator_traits = typename base::allocator_traits;
        using size_type = typename base::size_type;
        using difference_type = typename base::difference_type;
        using reference = typename base::reference;
        using const_reference = typename base::const_reference;
        using pointer = typename base::pointer;
        using const_pointer = typename base::const_pointer;

        static inline constexpr size_type npos = static_cast<size_type>(-1);

        // ********************************* begin volume ******************************


        /**
         * @brief shrink only occur on short string is enough to storage
         */
        constexpr void shrink_to_fit() noexcept {
            if (this->size() <= this->short_string_max_ && this->is_long_()) {
                auto ls = this->stor_.ls_;
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    this->stor_.ss_ = decltype(this->stor_.ss_){};
                }
#endif
                traits_type::copy(this->stor_.ss_.data(), ls.begin_, static_cast<std::size_t>(ls.end_ - ls.begin_));
                dealloc_(ls);
            }
        }

        // ********************************* begin element access ******************************

    private:
    public:
        constexpr CharType const *data() const noexcept {
            return begin_();
        }

        constexpr CharType *data() noexcept {
            return begin_();
        }

        constexpr CharType const *c_str() const noexcept {
            return begin_();
        }

        constexpr const_reference at(size_type pos) const {
            if (pos >= this->size_())
                throw std::out_of_range{this->exception_string_};

            return *(begin_() + pos);
        }

        constexpr reference at(size_type pos) {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).at(pos));
        }

        constexpr const_reference operator[](size_type pos) const noexcept {
            assert(("pos >= size, please check the arg", pos < this->size_()));
            return *(begin_() + pos);
        }

        constexpr reference operator[](size_type pos) noexcept {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this)[pos]);
        }

        constexpr const CharType &front() const noexcept {
            assert(("string is empty", !this->is_empty_()));

            return *begin_();
        }

        constexpr CharType &front() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).front());
        }

        constexpr const CharType &back() const noexcept {
            assert(("string is empty", !this->is_empty_()));

            return *(end_() - 1);
        }

        constexpr CharType &back() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).back());
        }

        constexpr operator std::basic_string_view<CharType, Traits>() const noexcept {
            return std::basic_string_view<CharType, Traits>(begin_(), end_());
        }

        // ********************************* begin iterator type ******************************

    private:
        struct iterator_type_ {
            using difference_type = std::ptrdiff_t;
            using value_type = CharType;
            using pointer = std::add_pointer_t<value_type>;
            using reference = std::add_lvalue_reference_t<CharType>;
            using iterator_category = std::random_access_iterator_tag;
#if RAINY_HAS_CXX20
            using iterator_concept = std::contiguous_iterator_tag;
#endif

        private:
            CharType *current_{};

#ifndef NDEBUG
            basic_string *target_{};
#endif

            constexpr void check() const noexcept {
#ifndef NDEBUG
                if (current_ && current_ <= target_->end_() && current_ >= target_->begin_())
                    ;
                else
                    assert(("iterator is invalidated", false));
#endif
            }

            friend class basic_string;

        public:
            iterator_type_() noexcept = default;
            iterator_type_(iterator_type_ const &) noexcept = default;
            iterator_type_(iterator_type_ &&) noexcept = default;
            iterator_type_ &operator=(iterator_type_ const &) & noexcept = default;
            iterator_type_ &operator=(iterator_type_ &&) & noexcept = default;

        private:
#ifndef NDEBUG
            constexpr iterator_type_(CharType *current, basic_string *target) : current_(current), target_(target) {
            }
#else
            constexpr iterator_type_(CharType *current) : current_(current) {
            }
#endif

        public:
            constexpr iterator_type_ operator+(difference_type n) const & noexcept {
                auto temp = *this;
                temp.current_ += n;
                temp.check();

                return temp;
            }

            constexpr iterator_type_ operator-(difference_type n) const & noexcept {
                auto temp = *this;
                temp.current_ -= n;
                temp.check();

                return temp;
            }

            constexpr friend iterator_type_ operator+(difference_type n, iterator_type_ const &right) noexcept {
                auto temp = right;
                temp.current_ += n;
                temp.check();

                return temp;
            }

            constexpr friend iterator_type_ operator-(difference_type n, iterator_type_ const &right) noexcept {
                auto temp = right;
                temp.current_ -= n;
                temp.check();

                return temp;
            }

            constexpr friend difference_type operator-(iterator_type_ const &left, iterator_type_ const &right) noexcept {
                assert(("iter belongs to different strings", left.target_ == right.target_));

                return left.current_ - right.current_;
            }

            constexpr iterator_type_ &operator+=(difference_type n) & noexcept {
                current_ += n;
                check();
                return *this;
            }

            constexpr iterator_type_ &operator-=(difference_type n) & noexcept {
                current_ -= n;
                check();
                return *this;
            }

            constexpr iterator_type_ &operator++() & noexcept {
                ++current_;
                check();
                return *this;
            }

            constexpr iterator_type_ &operator--() & noexcept {
                --current_;
                check();
                return *this;
            }

            constexpr iterator_type_ operator++(int) & noexcept {
                iterator_type_ temp{*this};
                ++current_;
                check();
                return temp;
            }

            constexpr iterator_type_ operator--(int) & noexcept {
                iterator_type_ temp{*this};
                --current_;
                check();
                return temp;
            }

            constexpr CharType &operator[](difference_type n) const noexcept {
#ifndef NDEBUG
                iterator_type_ end = (*this) + n;
                end.check();
#endif

                return *(current_ + n);
            }

            constexpr CharType &operator*() const noexcept {
                return *current_;
            }

            constexpr CharType *operator->() const noexcept {
                return current_;
            }
#if RAINY_HAS_CXX20
            friend constexpr std::strong_ordering operator<=>(iterator_type_ const &, iterator_type_ const &) noexcept = default;
#endif
        };

        // ********************************* begin iterator function ******************************

    public:
        using iterator = iterator_type_;
        using const_iterator = iterator_type_;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr iterator begin() noexcept {
#ifndef NDEBUG
            return {begin_(), this};
#else
            return {begin_()};
#endif
        }

        constexpr iterator end() noexcept {
#ifndef NDEBUG
            return iterator_type_{end_(), this};
#else
            return iterator_type_{end_()};
#endif
        }

        constexpr const_iterator begin() const noexcept {
#ifndef NDEBUG
            return iterator_type_{const_cast<CharType *>(begin_()), const_cast<basic_string *>(this)};
#else
            return iterator_type_{const_cast<CharType *>(begin_())};
#endif
        }

        constexpr const_iterator end() const noexcept {
#ifndef NDEBUG
            return iterator_type_{const_cast<CharType *>(end_()), const_cast<basic_string *>(this)};
#else
            return iterator_type_{const_cast<CharType *>(end_())};
#endif
        }

        constexpr const_iterator cbegin() noexcept {
            return begin();
        }

        constexpr const_iterator cend() noexcept {
            return end();
        }

        // ********************************* begin memory management ******************************

    private:
        /**
         * @brief allocates memory and automatically adds 1 to store trailing zero
         * @brief strong exception safety guarantee
         * @brief not responsible for reclaiming memory
         * @brief never shrink
         * @param n, expected number of characters
         */
        constexpr void allocate_plus_one_(size_type n) {
            // strong exception safe grantee
            if (n <= this->short_string_max_ && !this->is_long_()) {
                this->size_flag_ = static_cast<signed char>(n);
                return;
            }
            ++n;
#if defined(__cpp_lib_allocate_at_least) && (__cpp_lib_allocate_at_least >= 202302L)
            auto [ptr, count] = allocator_traits::allocate_at_least(this->allocator_, n);
            this->stor_.ls_ = {ptr, nullptr, ptr + count};
#else
            auto ptr = allocator_traits::allocate(this->allocator_, n);
            this->stor_.ls_ = {ptr, nullptr, ptr + n};
#endif
            this->size_flag_ = static_cast<unsigned char>(-1);
        }

        /**
         * @brief dealloc the memory of long string
         * @brief static member function
         * @param ls, allocated long string
         */
        constexpr void dealloc_(typename base::ls_type_ &ls) noexcept {
            allocator_traits::deallocate(this->allocator_, ls.begin_, ls.last_ - ls.begin_);
        }

        /**
         * @brief conditionally sets size correctly. only legal if n < capacity()
         * @brief write 0 to the tail at the same time
         * @brief never shrink
         * @param n, expected string length
         */
        constexpr void resize_(size_type n) noexcept {
            assert(("n > capacity()", n <= this->capacity()));
            if (this->is_long_()) {
                // if n = 0, keep storage avilable
                auto &&ls = this->stor_.ls_;
                ls.end_ = ls.begin_ + n;
                // return advance
                *ls.end_ = CharType{};
            } else {
                this->size_flag_ = static_cast<unsigned char>(n);
                // gcc thinks it's out of range,
                // so make gcc happy
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
                this->stor_.ss_[n] = CharType{};
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
            }
        }

        /**
         * @brief fill characters to *this
         * @brief not doing anything else
         * @param begin begin of characters
         * @param end end of characters
         */
        constexpr void fill_(CharType const *begin, CharType const *end) noexcept {
            assert(("cannot store string in current allocated storage", static_cast<size_type>(end - begin) <= this->capacity()));
            traits_type::copy(begin_(), begin, static_cast<std::size_t>(end - begin));
        }

        /**
         * @brief caculate the length of c style string
         * @param begin begin of characters
         * @return length
         */
        constexpr static size_type c_style_string_length_(CharType const *string) noexcept {
            return traits_type::length(string);
        }

        /**
         * @brief assign characters to *this
         * @brief this function can be called with any legal state
         * @brief strong exception safety guarantee
         * @brief this function provide for assign and operator=
         * @param begin begin of characters
         * @param end end of characters
         */
        constexpr void assign_(CharType const *first, CharType const *last) {
            auto size = last - first;
            if (this->capacity() < static_cast<size_type>(size)) {
                if (this->is_long_()) {
                    auto ls = this->stor_.ls_;
                    allocate_plus_one_(size);
                    dealloc_(ls);
                } else {
                    allocate_plus_one_(size);
                }
            }
            fill_(first, last);
            resize_(size);
        }

        /**
         * @brief insert characters to *this
         * @brief this function can be called with any legal state
         * @brief strong exception safety guarantee
         */
        constexpr void insert_(size_type index, CharType const *first, CharType const *last) {
            auto size = this->size_();
            if (index > size) {
                throw std::out_of_range{this->exception_string_};
            }
            auto length = last - first;
            auto new_size = size + length;
            auto begin = begin_();
            auto end = begin + size;
            auto start = begin + index;
            // if start is not in range and capacity > length + size
            if ((start < first || start > last) && this->capacity() > new_size) {
                traits_type::move(end, start, static_cast<std::size_t>(length));
                // re-calc first and last, because range is in *this
                if (first >= start && last <= end) {
                    first += index;
                    last += index;
                }
                traits_type::copy(start, first, static_cast<std::size_t>(length));
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + index;
                auto temp_end = temp_begin + size;
                traits_type::copy(temp_begin, begin, static_cast<std::size_t>(index));
                traits_type::copy(temp_start, first, static_cast<std::size_t>(length));
                traits_type::copy(temp_start + length, start, static_cast<std::size_t>(end - start));
                temp.swap(*this);
            }
            resize_(new_size);
        }


        constexpr void replace_(size_type pos, size_type count, CharType const *first2, CharType const *last2) {
            auto size = this->size_();
            if (pos > size) {
                throw std::out_of_range{this->exception_string_};
            }
            auto begin = begin_();
            auto first1 = begin + pos;
            auto last1 = begin + (core::min)(pos + count, size);
            auto length1 = last1 - first1;
            auto length2 = last2 - first2;
            auto new_size = size + length2 - length1; // 注意！是 length2 - length1
            auto end = begin + size;
            if (!(last1 < first2 || last2 < first1) && new_size <= this->capacity()) {
                auto diff = length2 - length1; // length 变化量
                if (diff > 0) {
                    traits_type::move(end + diff, end, static_cast<std::size_t>(-diff));
                } else if (diff < 0) {
                    traits_type::move(last1 + diff, last1, static_cast<std::size_t>(-diff));
                }
                if (first2 >= last1 && last2 <= end) {
                    first2 += diff;
                    last2 += diff;
                }
                traits_type::move(first1, first2, static_cast<std::size_t>(length2));
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + (first1 - begin);
                traits_type::copy(temp_begin, begin, static_cast<std::size_t>(first1 - begin));
                traits_type::copy(temp_start, first2, static_cast<std::size_t>(length2));
                traits_type::copy(temp_start + length2, last1, static_cast<std::size_t>(end - last1));
                temp.swap(*this);
            }
            resize_(new_size);
        }

    public:
        RAINY_CONSTEXPR20 ~basic_string() {
            if (this->is_long_())
                dealloc_(this->stor_.ls_);
        }

        /**
         * @brief reserve memory
         * @brief strong exception safety guarantee
         * @brief never shrink
         * @param new_cap new capacity
         */
        constexpr void reserve(size_type new_cap) {
            if (this->capacity() >= new_cap) {
                return;
            }
            if (this->is_long_()) {
                auto ls = this->stor_.ls_;
                allocate_plus_one_(new_cap);
                fill_(ls.begin_, ls.end_);
                // even though I'm only subtracting two pointers without r/w
                // gcc still thinks it's use after free, so make gcc happy
                auto size = ls.end_ - ls.begin_;
                dealloc_(ls);
                // resize_(ls.end_ - ls.begin_);
                resize_(size);
            } else {
                auto ss = this->stor_.ss_;
                auto size = this->size_flag_;
                auto data = ss.data();
                allocate_plus_one_(new_cap);
                fill_(data, data + size);
                resize_(size);
            }
        }

        /**
         * @brief resize string length
         * @brief strong exception safety guarantee
         * @brief never shrink
         * @param count new size
         * @param ch character to fill
         */
        constexpr void resize(size_type count, CharType ch) {
            auto size = this->size_();

            if (count > size) {
                reserve(count);
                auto begin = begin_();
                std::fill(begin + size, begin + count, ch);
            }

            resize_(count);
        }

        /**
         * @brief resize string length
         * @brief strong exception safety guarantee
         * @brief never shrink
         * @param count new size
         */
        constexpr void resize(size_type count) {
            resize(count, CharType{});
        }

        /**
         * @brief equal to resize(0)
         * @brief never shrink
         */
        constexpr void clear() noexcept {
            resize_(0);
        }

        /**
         * @brief use size * 1.5 for growth
         * @param ch character to fill
         */
        constexpr void push_back(CharType ch) {
            auto size = this->size_();
            if (this->capacity() == size) {
                reserve(size * 2 - size / 2);
            }
            *end_() = ch;
            resize_(size + 1);
        }

        // ********************************* begin swap ******************************

    private:
        constexpr void swap_without_ator(basic_string &other) noexcept {
            auto &&self = *this;
            std::swap(self.stor_, other.stor_);
            std::swap(self.size_flag_, other.size_flag_);
        }

    public:
        constexpr void swap(basic_string &other) noexcept {
            if (allocator_traits::propagate_on_container_swap::value) {
                std::swap(this->allocator_, other.allocator_);
            } else {
                assert(other.allocator_ == this->allocator_);
            }
            other.swap_without_ator(*this);
        }

        friend void swap(basic_string &self, basic_string &other) noexcept {
            self.swap(other);
        }

        // ********************************* begin constructor ******************************
    public:
        constexpr basic_string() noexcept = default;

        constexpr basic_string(allocator_type const &a) noexcept : base(a) {
        }

        constexpr basic_string(size_type n, CharType ch, allocator_type const &a = allocator_type()) : base(a) {
            allocate_plus_one_(n);
            auto begin = begin_();
            auto end = begin + n;
            std::fill(begin, end, ch);
            resize_(n);
        }

        constexpr basic_string(const basic_string &other, size_type pos, size_type count, allocator_type const &a = allocator_type()) :
            base(a) {
            auto other_size = other.size_();

            if (pos > other_size)
                throw std::out_of_range{this->exception_string_};

            count = (core::min)(other_size - pos, count);
            allocate_plus_one_(count);
            auto start = other.begin_() + pos;
            fill_(start, start + count);
            resize_(count);
        }

        constexpr basic_string(const basic_string &other, size_type pos, allocator_type const &a = allocator_type()) :
            basic_string(other, pos, other.size_() - pos, a) {
        }

        constexpr basic_string(const CharType *s, size_type count, allocator_type const &a = allocator_type()) : base(a) {
            allocate_plus_one_(count);
            fill_(s, s + count);
            resize_(count);
        }

        constexpr basic_string(const CharType *s, allocator_type const &a = allocator_type()) :
            basic_string(s, c_style_string_length_(s), a) {
        }

        template <class InputIt,
                  std::enable_if_t<
                      std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, int> = 0>
        constexpr basic_string(InputIt first, InputIt last) {
            using category = typename std::iterator_traits<InputIt>::iterator_category;
            if (std::is_base_of<std::random_access_iterator_tag, category>::value) {
                auto length = std::distance(first, last);
                allocate_plus_one_(length);
                std::copy(first, last, begin_());
                resize_(length);
            } else {
                for (; first != last; ++first)
                    push_back(*first);
            }
        }

        constexpr basic_string(const basic_string &other) :
            base(allocator_traits::select_on_container_copy_construction(other.allocator_)) {
            auto other_size = other.size_();
            allocate_plus_one_(other_size);
            fill_(other.begin_(), other.end_());
            resize_(other_size);
        }

        constexpr basic_string(const basic_string &other, allocator_type const &a) : base(a) {
            auto other_size = other.size_();
            allocate_plus_one_(other_size);
            fill_(other.begin_(), other.end_());
            resize_(other_size);
        }

        constexpr basic_string(basic_string &&other, size_type pos, size_type count, allocator_type const &a = allocator_type()) :
            base(a) {
            auto other_size = other.size_();
            if (pos > other_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min)(other_size - pos, count);
            if (pos != 0) {
                auto other_begin = other.begin_();
                auto start = other_begin + pos;
                auto last = start + count;
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    core::algorithm::copy(start, last, other_begin);
                } else
#endif
                {
                    std::memmove(other_begin, start, (last - start) * sizeof(CharType));
                }
            }
            other.resize_(count);
            other.swap(*this);
        }

        constexpr basic_string(basic_string &&other) noexcept : base{other.allocator_} {
            other.swap_without_ator(*this);
        }

        constexpr basic_string(basic_string &&other, allocator_type const &a) : base(a) {
            if (other.allocator_ == a) {
                other.swap_without_ator(*this);
            } else {
                basic_string temp{other.data(), other.size(), a};
                temp.swap(*this);
                other.swap(temp);
            }
        }

        constexpr basic_string(std::initializer_list<CharType> ilist, allocator_type const &a = allocator_type()) :
            basic_string(std::data(ilist), ilist.size(), a) {
        }

        // clang-format off
        template <typename StringViewLike, type_traits::other_trans::enable_if_t<
                         std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int
            > = 0>
        constexpr basic_string(const StringViewLike& t, allocator_type const& a = allocator_type())
		     : basic_string(std::basic_string_view<CharType, Traits>{ t }.data(), std::basic_string_view<CharType, Traits>{ t }.size(),a)
        {
        }

        // clang-format on

        template <class StringViewLike, type_traits::other_trans::enable_if_t<
                      std::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>>, int> = 0>
        constexpr basic_string(const StringViewLike &t, size_type pos, size_type n, allocator_type const &a = allocator_type()) :
            base(a) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{this->exception_string_};
            }
            n = (core::min)(sv_size - pos, n);
            allocate_plus_one_(n);
            fill_(data, data + n);
            resize_(n);
        }

#if defined(__cpp_lib_containers_ranges) && (__cpp_lib_containers_ranges >= 202202L)
        // tagged constructors to construct from container compatible range
        template <class R>
            requires std::ranges::range<R> && requires {
                typename R::value_type;
                requires std::same_as<typename R::value_type, CharType>;
            }
        constexpr basic_string(std::from_range_t, R &&rg, allocator_type const &a = allocator_type()) : base(a) {
            if constexpr (std::ranges::sized_range<R>) {
                auto size = std::ranges::size(rg);
                allocate_plus_one_(size);
                std::ranges::copy(std::ranges::begin(rg), std::ranges::end(rg), begin_());
                resize_(size);
            } else {
                for (auto first = std::ranges::begin(rg), last = std::ranges::end(rg); first != last; ++first)
                    push_back(*first);
            }
        }
#endif

        /**
         * @brief this version provide for InputIt version of assign, other version of append and operator+=
         */
        template <class InputIt, typename = typename std::enable_if<std::is_base_of<
                                     std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value>::type>
        constexpr basic_string &append(InputIt first, InputIt last) {
            using category = typename std::iterator_traits<InputIt>::iterator_category;

            if (std::is_base_of<std::random_access_iterator_tag, category>::value) {
                auto size = this->size_();
                auto length = std::distance(first, last);
                auto new_size = size + length;
                reserve((core::max)(size * 2, new_size));
                traits_type::copy(begin_() + size, utility::addressof(*first), static_cast<std::size_t>(length));
                resize_(new_size);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
            return *this;
        }

        // ********************************* begin assign ******************************

        constexpr basic_string &assign(size_type count, CharType ch) {
            reserve(count);
            traits_type::assign(begin_(), count, ch);
            resize_(count);
            return *this;
        }

        constexpr basic_string &assign(const basic_string &str) {
            if (std::addressof(str) == this) {
                return *this;
            }
            if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
                if (this->allocator_ != str.allocator_) {
                    basic_string temp{this->allocator_};
                    temp.swap(*this);
                    this->allocator_ = str.allocator_;
                }
            }
            assign_(str.begin_(), str.end_());
            return *this;
        }

        constexpr basic_string &assign(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size_();
            if (pos > str_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min)(npos, (core::min)(str_size - pos, count));
            auto str_begin = begin_();
            assign_(str_begin + pos, str_begin + pos + count);
            return *this;
        }

        constexpr basic_string &assign(basic_string &&other) noexcept {
            if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
                other.swap(*this);
            } else {
                if (this->allocator_ == other.allocator_) {
                    other.swap_without_ator(*this);
                } else {
                    assign_(other.begin_(), other.end_());
                }
            }

            return *this;
        }

        constexpr basic_string &assign(const CharType *s, size_type count) {
            assign_(s, s + count);

            return *this;
        }

        constexpr basic_string &assign(const CharType *s) {
            assign_(s, s + c_style_string_length_(s));

            return *this;
        }

        template <class InputIt>
        constexpr basic_string &assign(InputIt first, InputIt last) {
            resize_(0);

            return append(first, last);
        }

        constexpr basic_string &assign(std::initializer_list<CharType> ilist) {
            auto data = ilist.begin();
            assign_(data, data + ilist.size());
            return *this;
        }

        // clang-format off
        template <class StringViewLike, type_traits::other_trans::enable_if_t<
            std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        basic_string& assign(const StringViewLike& t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            assign_(data, data + sv.size());
            return *this;
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& assign(const StringViewLike& t, size_type pos, size_type count = npos)
        {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();

            if (pos > sv_size)
                throw std::out_of_range{ this->exception_string_ };

            count = (core::min)(npos, (core::min)(sv_size - pos, count));
            auto data = sv.data();
            assign_(data + pos, data + pos + count);

            return *this;
        }

        basic_string(std::nullptr_t) = delete;
        constexpr basic_string &operator=(std::nullptr_t) = delete;

        constexpr basic_string &operator=(basic_string &&other) noexcept(
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Allocator>::is_always_equal::value) {
            return assign(utility::move(other));
        }

        constexpr basic_string &operator=(const basic_string &str) {
            return assign(str);
        }

        constexpr basic_string &operator=(const CharType *s) {
            return assign(s);
        }

        constexpr basic_string &operator=(CharType ch) {
            resize_(1);
            (*begin_()) = ch;

            return *this;
        }

        constexpr basic_string &operator=(std::initializer_list<CharType> ilist) {
            return assign(ilist);
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& operator=(const StringViewLike& t)
        {
            return assign(t);
        }

        #if RAINY_HAS_CXX20
        friend constexpr std::strong_ordering operator<=>(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size_();
            auto rsize = right.size_();
            if (std::is_constant_evaluated()) {
                for (auto begin = left.begin_(), end = begin + (core::min)(lsize, rsize), start = right.begin_(); begin != end;
                     ++begin, ++start) {
                    if (*begin > *start)
                        return std::strong_ordering::greater;
                    else if (*begin < *start)
                        return std::strong_ordering::less;
                }
                if (lsize > rsize)
                    return std::strong_ordering::greater;
                else if (lsize < rsize)
                    return std::strong_ordering::less;

                return std::strong_ordering::equal;
            } else {
                auto res = basic_string::traits_type::compare(left.begin_(), right.begin_(), (core::min)(rsize, lsize));
                if (res > 0)
                    return std::strong_ordering::greater;
                else if (res < 0)
                    return std::strong_ordering::less;

                if (lsize > rsize)
                    return std::strong_ordering::greater;
                else if (lsize < rsize)
                    return std::strong_ordering::less;
                else
                    return std::strong_ordering::equal;
            }
        }

        friend constexpr std::strong_ordering operator<=>(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = basic_string::c_style_string_length_(start);
            auto lsize = left.size_();
            if (std::is_constant_evaluated()) {
                for (auto begin = left.begin_(), end = begin + (core::min)(lsize, rsize); begin != end; ++begin, ++start) {
                    if (*begin > *start)
                        return std::strong_ordering::greater;
                    else if (*begin < *start)
                        return std::strong_ordering::less;
                }

                if (lsize > rsize)
                    return std::strong_ordering::greater;
                else if (lsize < rsize)
                    return std::strong_ordering::less;
                else
                    return std::strong_ordering::equal;
            } else {
                auto res = basic_string::traits_type::compare(left.begin_(), start, (core::min)(rsize, lsize));
                if (res > 0)
                    return std::strong_ordering::greater;
                else if (res < 0)
                    return std::strong_ordering::less;

                if (lsize > rsize)
                    return std::strong_ordering::greater;
                else if (lsize < rsize)
                    return std::strong_ordering::less;
                else
                    return std::strong_ordering::equal;
            }
        }
        #endif

        friend constexpr bool operator==(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size_();
            auto rsize = right.size_();
            if (lsize != rsize) {
                return false;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto begin = left.begin_(), end = begin + (core::min)(lsize, rsize), start = right.begin_(); begin != end;
                     ++begin, ++start) {
                    if (*begin != *start)
                        return false;
                }

                return true;
            } else
#endif
            {
                return !basic_string::traits_type::compare(left.begin_(), right.begin_(), (core::min)(rsize, lsize));
            }
        }

        friend constexpr bool operator==(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = basic_string::c_style_string_length_(start);
            auto lsize = left.size_();
            if (lsize != rsize) {
                return false;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto begin = left.begin_(), end = begin + (core::min)(lsize, rsize); begin != end; ++begin, ++start) {
                    if (*begin != *start)
                        return false;
                }

                return true;
            } else
#endif
            {
                return !basic_string::traits_type::compare(left.begin_(), start, (core::min)(rsize, lsize));
            }
        }

        // ********************************* begin append ******************************

    private:
        constexpr void append_(CharType const *first, CharType const *last) {
            auto length = last - first;
            auto size = this->size_();
            reserve(size + length);
            auto end = begin_() + size;

#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy(first, last, end);
            } else
#endif
            {
                traits_type::copy(end, first, static_cast<std::size_t>(length));
            }

            resize_(size + length);
        }

    public:
        constexpr basic_string &append(size_type count, CharType ch) {
            auto size = this->size_();
            reserve(size + count);
            auto end = begin_() + size;
            std::fill(end, end + count, ch);
            resize_(size + count);

            return *this;
        }

        constexpr basic_string &append(const CharType *s, size_type count) {
            append_(s, s + count);
            return *this;
        }

        constexpr basic_string &append(const basic_string &str) {
            auto begin = str.begin_();
            append_(begin, begin + str.size_());
            return *this;
        }

        constexpr basic_string &append(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size_();
            if (pos > str_size)
                throw std::out_of_range{this->exception_string_};

            count = (core::min)(npos, (core::min)(str_size - pos, count));

            return append(str.begin_() + pos, count);
        }

        constexpr basic_string &append(const CharType *s) {
            append_(s, s + c_style_string_length_(s));

            return *this;
        }

        constexpr basic_string &append(std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            append_(data, data + ilist.size());

            return *this;
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& append(const StringViewLike& t)
        {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            append_(data, data + sv.size());

            return *this;
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& append(const StringViewLike& t, size_type pos, size_type count = npos)
        {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();

            if (pos > sv_size)
                throw std::out_of_range{ this->exception_string_ };

            count = (core::min)(npos, (core::min)(sv_size - count, count));

            return append(sv.data() + pos, count);
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& operator+=(const StringViewLike& t)
        {
            return append(t);
        }

        constexpr basic_string &operator+=(const basic_string &str) {
            return append(str);
        }

        constexpr basic_string &operator+=(CharType ch) {
            return push_back(ch);
        }

        constexpr basic_string &operator+=(const CharType *s) {
            return append(s);
        }

        constexpr basic_string &operator+=(std::initializer_list<CharType> ilist) {
            return append(ilist);
        }

    private:
        constexpr bool static equal_(CharType const *begin, CharType const *end, CharType const *first, CharType const *last) noexcept {
            if (last - first != end - begin)
                return false;

            for (; begin != end; ++begin, ++first) {
                if (*first != *begin) {
                    return false;
                }
            }

            return true;
        }

    public:
        constexpr bool starts_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = begin_();

            if (sv_size > this->size_())
                return false;

            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        constexpr bool starts_with(CharType ch) const noexcept {
            return *begin_() == ch;
        }

        constexpr bool starts_with(const CharType *s) const {
            auto length = c_style_string_length_(s);
            auto begin = begin_();

            if (length > this->size_())
                return false;

            return equal_(s, s + length, begin, begin + length);
        }

        constexpr bool ends_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = end_();

            if (sv_size > this->size_())
                return false;

            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        constexpr bool ends_with(CharType ch) const noexcept {
            return *(end_() - 1) == ch;
        }

        constexpr bool ends_with(CharType const *s) const {
            auto length = c_style_string_length_(s);

            if (length > this->size_())
                return false;

            auto end = end_();

            return equal_(s, s + length, end - length, end);
        }

#if defined(__cpp_lib_string_contains) && (__cpp_lib_string_contains >= 202011L)
        constexpr bool contains(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto size = sv.size();
            auto begin = begin_();
            auto data = sv.data();

            if (this->size_() < size)
                return false;

            // opti for starts_with
            if (equal_(begin, begin + size, data, data + size))
                return true;

            return std::basic_string_view<CharType, Traits>{begin + 1, begin + size}.contains(sv);
        }

        constexpr bool contains(CharType ch) const noexcept {
            for (auto begin = begin_(), end = end_(); begin != end; ++begin) {
                if (*begin == ch)
                    return true;
            }

            return false;
        }

        constexpr bool contains(const CharType *s) const noexcept {
            return std::basic_string_view<CharType, Traits>{begin_(), end_()}.contains(
                std::basic_string_view<CharType, Traits>{s, s + c_style_string_length_(s)});
        }
#endif

        // ********************************* begin insert ******************************

        constexpr basic_string &insert(size_type index, size_type count, CharType ch) {
            auto size = this->size_();

            if (index > size)
                throw std::out_of_range{this->exception_string_};

            reserve(size + count);
            auto start = begin_() + index;
            auto end = start + size - index;
            auto last = end + count;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy_backward(start, end, last);
                std::fill(start, start + count, ch);
            }else
#endif
             {
                std::memmove(end, start, count * sizeof(CharType));
                std::fill(start, start + count, ch);
            }
            resize_(size + count);
            return *this;
        }

        constexpr basic_string &insert(size_type index, const CharType *s, size_type count) {
            insert_(index, s, s + count);

            return *this;
        }

        constexpr basic_string &insert(size_type index, const CharType *s) {
            insert_(index, s, s + c_style_string_length_(s));

            return *this;
        }

        constexpr basic_string &insert(size_type index, const basic_string &str) {
            insert_(index, str.begin_(), str.end_());

            return *this;
        }

        constexpr basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos) {
            auto s_size = str.size_();

            if (s_index > s_size){
                throw std::out_of_range{this->exception_string_};
            }

            count = (core::min)(npos, (core::min)(s_size - s_index, count));
            auto s_start = str.begin_() + s_index;
            insert_(index, s_start, s_start + count);
            return *this;
        }

        constexpr iterator insert(const_iterator pos, CharType ch) {
            auto size = this->size_();
            auto start = pos.base().current_;
            auto end = end_();
            auto index = start - begin_();
            reserve(size + 1);
            auto begin = begin_();
            end = begin + size;
            start = begin + index;
            #if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy_backward(start, end, end + 1);
            }else
            #endif
            
             {
                traits_type::move(start + 1, start, end - start);
            }
            *start = ch;
            resize_(size + 1);
#ifndef NDEBUG
            return {start, this};
#else
            return {start};
#endif
        }

        constexpr iterator insert(const_iterator pos, size_type count, CharType ch) {
            auto start = pos.base().current_;
            auto index = start - begin_();
            insert(index, count, ch);
#ifndef NDEBUG
            return {start, this};
#else
            return {start};
#endif
        }

        /**
         * @brief this function use a temp basic_string, so it can't decl in class decl
         */
        template <class InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
            assert(("pos isn't in this string", pos.base().current_ >= begin_() && pos.base().current_ <= end_()));

            auto size = this->size_();
            auto start = pos.base().current_;
            auto end = end_();
            auto index = start - begin_();
            #if RAINY_HAS_CXX20
            if constexpr (std::random_access_iterator<InputIt>) {
                auto length = std::distance(first, last);
                reserve(size + length);
                auto begin = begin_();
                std::copy_backward(begin + index, begin + size, begin + size + length);
                std::ranges::copy(first, last, begin + index);
                resize_(size + length);
            } else 
            #endif
            {
                basic_string temp{start, end};
                resize_(pos - begin_());
                for (; first != last; ++first)
                    push_back(*first);
                append_(temp.begin_(), temp.end_());
            }

#ifndef NDEBUG
            return {start, this};
#else
            return {start};
#endif
        }

        constexpr iterator insert(const_iterator pos, std::initializer_list<CharType> ilist) {
            auto i_data = std::data(ilist);
            auto start = pos.base().current_;

            insert_(start - begin_(), i_data, i_data + ilist.size());

#ifndef NDEBUG
            return {start, this};
#else
            return {start};
#endif
        }

        // clang-format off
        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& insert(size_type pos, const StringViewLike& t)
        {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_data = sv.data();
            insert_(pos, sv_data, sv_data + sv.size());

            return *this;
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<std::is_convertible_v<const StringViewLike&, std::basic_string_view<CharType, Traits>> && (!std::is_convertible_v<const StringViewLike&, const CharType*>),int> = 0>
        constexpr basic_string& insert(size_type pos, const StringViewLike& t, size_type t_index, size_type count = npos)
        {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();
            auto size = this->size_();

            if (t_index > sv_size)
                throw std::out_of_range{ this->exception_string_ };

            count = (core::min)(npos, (core::min)(sv_size - t_index, count));
            auto sv_data = sv.data();
            insert_(pos, sv_data + t_index, sv_data + t_index + count);

            return *this;
        }

        // clang-format on

        // ********************************* begin erase ******************************

    private:
    public:
        constexpr basic_string &erase(size_type index = 0, size_type count = npos) {
            auto size = this->size_();

            if (index > size)
                throw std::out_of_range{this->exception_string_};

            count = (core::min)(npos, (core::min)(size - index, count));
            auto start = begin_() + index;
            erase_(start, start + count);

            return *this;
        }

        constexpr iterator erase(const_iterator position) noexcept {
            auto start = position.base().current_;
            erase_(start, start + 1);

#ifndef NDEBUG
            return {start, this};
#else
            return {start};
#endif
        }

        constexpr iterator erase(const_iterator first, const_iterator last) noexcept {
            auto start = first.base().current_;
            erase_(start, last.base().current_);

#ifndef NDEBUG
            return {start, this};
#else
            return {start};
#endif
        }

        // ********************************* begin pop_back ******************************

        constexpr void pop_back() noexcept {
            assert(("string is empty", !this->is_empty_()));
            resize_(this->size_() - 1);
        }

        // ********************************* begin replace ******************************

        constexpr basic_string &replace(size_type pos, size_type count, const basic_string &str) {
            replace_(pos, count, str.begin_(), str.end_());

            return *pos;
        }

        constexpr basic_string &replace(const_iterator first, const_iterator last, const basic_string &str) {
            auto start = first.base().current_;
            replace_(start - begin_(), last - first, str.begin_(), str.end_());

            return *this;
        }

        constexpr basic_string &replace(size_type pos, size_type count, const basic_string &str, size_type pos2,
                                        size_type count2 = npos) {
            auto str_size = str.size_();

            if (pos2 > str_size)
                throw std::out_of_range{this->exception_string_};

            count2 = (core::min)(npos, (core::min)(count2, str_size - pos2));
            auto begin = str.begin_();
            replace_(pos, count, begin + count2, begin + count2 + pos2);

            return *this;
        }

        constexpr basic_string &replace(size_type pos, size_type count, const CharType *cstr, size_type count2) {
            replace_(pos, count, cstr, cstr + count2);

            return *this;
        }

        constexpr basic_string &replace(const_iterator first, const_iterator last, const CharType *cstr, size_type count2) {
            auto start = first.base().current_;
            replace_(start - begin_(), last - first, cstr, cstr + count2);

            return *this;
        }

        constexpr basic_string &replace(size_type pos, size_type count, const CharType *cstr) {
            replace_(pos, count, cstr, cstr + c_style_string_length_(cstr));

            return *pos;
        }

        constexpr basic_string &replace(const_iterator first, const_iterator last, const CharType *cstr) {
            auto start = first.base().current_;
            replace_(start - begin_(), last - first, cstr, cstr + c_style_string_length_(cstr));

            return *this;
        }

        constexpr basic_string &replace(const_iterator first, const_iterator last, std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            auto start = first.base().current_;
            replace_(start - begin_(), last - first, data, data + ilist.size());

            return *this;
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<
                                            std::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                                                (!std::is_convertible_v<const StringViewLike &, const CharType *>),
                                            int> = 0>
        constexpr basic_string &replace(size_type pos, size_type count, const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            replace_(pos, count, data, data + sv.size());

            return *this;
        }

                template <class StringViewLike, type_traits::other_trans::enable_if_t<
                                            std::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                                                (!std::is_convertible_v<const StringViewLike &, const CharType *>),
                                            int> = 0>

        constexpr basic_string &replace(const_iterator first, const_iterator last, const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_data = sv.data();
            auto start = first.base().current_;
            replace_(start - begin_(), last - first, sv_data, sv_data + sv.size());

            return *this;
        }

        template <class StringViewLike, type_traits::other_trans::enable_if_t<
                                            std::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                                                (!std::is_convertible_v<const StringViewLike &, const CharType *>),
                                            int> = 0>
        basic_string &replace(size_type pos, size_type count, const StringViewLike &t, size_type pos2, size_type count2 = npos) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();

            if (pos2 > sv_size)
                throw std::out_of_range{this->exception_string_};

            count2 = (core::min)(npos, (core::min)(sv_size - pos2, count2));

            auto data = sv.data();
            replace_(pos, count, data + pos2, data + pos2 + count2);

            return *this;
        }

        constexpr basic_string &replace(size_type pos, size_type count, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = begin_();
            replace_(pos, count, begin, begin + count2);

            return *this;
        }

        constexpr basic_string &replace(const_iterator first, const_iterator last, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = begin_();
            auto start = first.base().current_;
            replace_(start - begin_(), last - first, begin, begin + count2);

            return *this;
        }

        template <
            typename InputIt,
            std::enable_if_t<
                std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value, int> = 0>
        constexpr basic_string &replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) {
            auto start = first.base().current_;
            using category = typename std::iterator_traits<InputIt>::iterator_category;
            if (std::is_base_of<std::random_access_iterator_tag, category>::value) {
                auto data = std::addressof(*first2);
                auto length2 = std::distance(first2, last2);
                replace_(start - begin_(), last - first, data, data + length2);
            } else {
                basic_string temp{first2, last2};
                auto temp_begin = temp.begin_();
                auto temp_size = temp.size_();
                replace_(start - begin_(), last - first, temp_begin, temp_begin + temp_size);
            }
            return *this;
        }

    private:
        /**
         * @brief internal use
         * @return a pointer to the first element
         */
        constexpr CharType const *begin_() const noexcept {
            if (this->is_long_()) {
                return this->stor_.ls_.begin_;
            } else {
                return this->stor_.ss_.data();
            }
        }

        /**
         * @brief internal use
         * @return a pointer to the first element
         */
        constexpr CharType *begin_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).begin_());
        }

        /**
         * @brief internal use
         * @return a pointer to the next position of the last element
         */
        constexpr CharType const *end_() const noexcept {
            if (this->is_long_())
                return this->stor_.ls_.end_;
            else
                return this->stor_.ss_.data() + this->size_flag_;
        }

        /**
         * @brief internal use
         * @return a pointer to the next position of the last element
         */
        constexpr CharType *end_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).end_());
        }

        constexpr void erase_(CharType *first, CharType const *last) noexcept {
            assert(("first or last is not in this string", first >= begin_() && last <= end_()));
            traits_type::move(first, last, static_cast<std::size_t>(const_cast<basic_string const &>(*this).end_() - last));
            resize_(this->size() - (last - first));
        }
    };

    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;

#if RAINY_HAS_CXX20
    using u8string = basic_string<char8_t>;
#endif
}

namespace std {
    template <typename CharType, typename Traits, typename Alloc>
    struct hash<rainy::text::basic_string<CharType, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::text::basic_string<CharType, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return rainy::utility::implements::hash_array_representation(val.data(), val.size());
        }
    };
}

namespace rainy::utility {
    template <typename CharType, typename Traits, typename Alloc>
    struct hash<rainy::text::basic_string<CharType, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = text::basic_string<CharType, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return implements::hash_array_representation(val.data(), val.size());
        }
    };
}
