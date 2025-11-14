#ifndef RAINY_COLLECTIONS_STRING_HPP
#define RAINY_COLLECTIONS_STRING_HPP

#if RAINY_HAS_CXX20
#include <compare>
#include <concepts>
#endif
#include <cstring>
#include <cwchar>
#include <iterator>
#include <memory>
#include <rainy/core/core.hpp>
#include <rainy/text/char_traits.hpp>
#include <rainy/collections/string_view.hpp>
#include <stdexcept>
#include <string_view>

#if RAINY_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

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

        RAINY_CONSTEXPR20 basic_string_storage() noexcept : pair{{}, Allocator{}} {
        }

        RAINY_CONSTEXPR20 basic_string_storage(const Allocator &allocator) : pair{{}, allocator} {
        }

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return !this->size_flag_;
        }

        RAINY_CONSTEXPR20 std::size_t size() const noexcept {
            return this->size_();
        }

        RAINY_CONSTEXPR20 std::size_t length() const noexcept {
            return this->size_();
        }

        /**
         * @brief reserve memory
         * @brief strong exception safety guarantee
         * @brief never shrink
         * @param new_cap new capacity
         */
        RAINY_CONSTEXPR20 void reserve(size_type new_cap) {
            if (this->capacity() >= new_cap) {
                return;
            }
            if (this->is_long_()) {
                auto ls = this->get_stor().large_string;
                allocate_plus_one_(new_cap);
                fill_(ls.begin_, ls.end_);
                auto size = ls.end_ - ls.begin_;
                dealloc_(ls);
                resize_(size);
            } else {
                auto ss = this->get_stor().short_string;
                auto size = this->size_flag_;
                auto data = ss;
                allocate_plus_one_(new_cap);
                fill_(data, data + size);
                resize_(size);
            }
        }

        RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return size_type{-1} / sizeof(CharType) / 2;
        }

        RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            if (this->is_long_()) {
                auto &&ls = this->get_stor().large_string;
                return ls.last_ - ls.begin_ - 1;
            } else {
                return this->short_string_max_;
            }
        }

        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return static_cast<allocator_type>(pair.get_second());
        }

    protected:
        struct long_string_t {
            CharType *begin_{};
            CharType *end_{};
            CharType *last_{};
        };

        static inline constexpr std::size_t short_string_max_ = 16;

        union small_string_t {
            RAINY_CONSTEXPR20 small_string_t() : short_string{} {
            }

            RAINY_CONSTEXPR20 small_string_t(CharType *begin, CharType *end, CharType *last) : large_string{begin, end, last} {
            }

            RAINY_CONSTEXPR20 ~small_string_t() {
            }

            RAINY_CONSTEXPR20 void switch_to_buf() {
                std::destroy_at(&large_string);
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    for (size_type idx = 0; idx < short_string_max_; ++idx) {
                        short_string[idx] = value_type();
                    }
                }
#endif
            }

            RAINY_CONSTEXPR20 void switch_to_ptr(CharType *begin, CharType *end, CharType *last) {
                std::destroy_at(this); // 销毁整个 union
                utility::construct_at(this, begin, end, last); // 使用带指针的构造函数重建
            }

            value_type short_string[short_string_max_]{};
            long_string_t large_string;
        };

        RAINY_CONSTEXPR20 small_string_t &get_stor() noexcept {
            return pair.get_first();
        }

        RAINY_CONSTEXPR20 const small_string_t &get_stor() const noexcept {
            return pair.get_first();
        }

        RAINY_CONSTEXPR20 allocator_type &get_al() noexcept {
            return pair.get_second();
        }

        utility::compressed_pair<small_string_t, Allocator> pair{};

        /**
         * @brief flag > 0: 使用小字符串优化，size_flag_将代表当前小字符串使用的大小
         * @brief flag = 0: 未在使用
         * @brief flag = MAX: 长字符串，约定使用 end - begin 作为长度
         */
        alignas(alignof(CharType)) unsigned char size_flag_{};

        static inline char exception_string_[] = "parameter is out of range, please check it.";

        // RAINY_CONSTEXPR20 bool is_sso_activate() const noexcept {
        //     return storage.is_sso_activate();
        // }

        RAINY_CONSTEXPR20 bool is_long_() const noexcept {
            return this->size_flag_ == decltype(this->size_flag_)(-1);
        }

        RAINY_CONSTEXPR20 bool is_short_() const noexcept {
            return this->size_flag_ != decltype(this->size_flag_)(-1);
        }

        RAINY_CONSTEXPR20 bool is_empty_() const noexcept {
            return !this->size_flag_;
        }

        RAINY_CONSTEXPR20 std::size_t size_() const noexcept {
            if (this->is_long_()) {
                auto &&ls = get_stor().large_string;
                return ls.end_ - ls.begin_;
            } else {
                return this->size_flag_;
            }
        }

        /**
         * @brief internal use
         * @return a pointer to the first element
         */
        RAINY_CONSTEXPR20 CharType const *begin_() const noexcept {
            if (this->is_long_()) {
                return this->get_stor().large_string.begin_;
            } else {
                return this->get_stor().short_string;
            }
        }

        /**
         * @brief internal use
         * @return a pointer to the first element
         */
        RAINY_CONSTEXPR20 CharType *begin_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string_storage const &>(*this).begin_());
        }

        /**
         * @brief internal use
         * @return a pointer to the next position of the last element
         */
        RAINY_CONSTEXPR20 CharType const *end_() const noexcept {
            if (this->is_long_()) {
                return this->get_stor().large_string.end_;
            } else {
                return this->get_stor().short_string + this->size_flag_;
            }
        }

        /**
         * @brief internal use
         * @return a pointer to the next position of the last element
         */
        RAINY_CONSTEXPR20 CharType *end_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string_storage const &>(*this).end_());
        }

        /**
         * @brief allocates memory and automatically adds 1 to store trailing zero
         * @brief strong exception safety guarantee
         * @brief not responsible for reclaiming memory
         * @brief never shrink
         * @param count, expected number of characters
         */
        RAINY_CONSTEXPR20 void allocate_plus_one_(size_type count) {
            // strong exception safe grantee
            if (count <= this->short_string_max_ && !this->is_long_()) {
                this->size_flag_ = static_cast<signed char>(count);
                return;
            }
            ++count;
#if defined(__cpp_lib_allocate_at_least) && (__cpp_lib_allocate_at_least >= 202302L)
            auto [ptr, real_count] = allocator_traits::allocate_at_least(get_al(), count);
            this->get_stor().large_string = {ptr, nullptr, ptr + real_count};
#else
            auto ptr = allocator_traits::allocate(this->get_al(), count);
            this->get_stor().large_string = {ptr, nullptr, ptr + count};
#endif
            this->size_flag_ = static_cast<unsigned char>(-1);
        }

        /**
         * @brief dealloc the memory of long string
         * @brief static member function
         * @param ls, allocated long string
         */
        RAINY_CONSTEXPR20 void dealloc_(long_string_t &ls) noexcept {
            allocator_traits::deallocate(this->get_al(), ls.begin_, ls.last_ - ls.begin_);
        }

        /**
         * @brief conditionally sets size correctly. only legal if count < capacity()
         * @brief write 0 to the tail at the same time
         * @brief never shrink
         * @param count, expected string length
         */
        RAINY_CONSTEXPR20 void resize_(size_type count) noexcept {
            assert(("count > capacity()" && count <= this->capacity()));
            if (this->is_long_()) {
                // if count = 0, keep storage avilable
                auto &&ls = this->get_stor().large_string;
                ls.end_ = ls.begin_ + count;
                // return advance
                *ls.end_ = CharType{};
            } else {
                this->size_flag_ = static_cast<unsigned char>(count);
                this->get_stor().short_string[count] = CharType{};
            }
        }

        /**
         * @brief fill characters to *this
         * @brief not doing anything else
         * @param begin begin of characters
         * @param end end of characters
         */
        RAINY_CONSTEXPR20 void fill_(CharType const *begin, CharType const *end) noexcept {
            assert(("cannot store string in current allocated storage" && static_cast<size_type>(end - begin) <= this->capacity()));
            traits_type::copy(begin_(), begin, static_cast<std::size_t>(end - begin));
        }

        RAINY_CONSTEXPR20 void swap_data(basic_string_storage &right) noexcept {
            auto &my_stor = get_stor();
            auto &other_stor = right.get_stor();
            small_string_t temp = my_stor;
            my_stor = other_stor;
            other_stor = temp;
            std::swap(this->size_flag_, right.size_flag_);
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
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static inline RAINY_CONSTEXPR20 size_type npos = static_cast<size_type>(-1);

        RAINY_CONSTEXPR20 basic_string() noexcept = default;

        RAINY_CONSTEXPR20 basic_string(allocator_type const &a) noexcept : base(a) {
        }

        RAINY_CONSTEXPR20 basic_string(size_type count, CharType ch, allocator_type const &a = allocator_type()) : base(a) {
            allocate_plus_one_(count);
            auto begin = this->begin_();
            auto end = begin + count;
            core::algorithm::fill(begin, end, ch);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, size_type count,
                                       allocator_type const &a = allocator_type()) : base(a) {
            auto other_size = right.size_();
            if (pos > other_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (other_size - pos, count);
            allocate_plus_one_(count);
            auto start = right.begin_() + pos;
            fill_(start, start + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, allocator_type const &a = allocator_type()) :
            basic_string(right, pos, right.size_() - pos, a) {
        }

        RAINY_CONSTEXPR20 basic_string(const CharType *cstr, size_type count, allocator_type const &a = allocator_type()) : base(a) {
            this->allocate_plus_one_(count);
            this->fill_(cstr, cstr + count);
            this->resize_(count);
        }

        RAINY_CONSTEXPR20 basic_string(const CharType *cstr, allocator_type const &a = allocator_type()) :
            basic_string(cstr, c_style_string_length_(cstr), a) {
        }

        template <typename Iter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<Iter>, int> = 0>
        RAINY_CONSTEXPR20 basic_string(Iter first, Iter last) {
            if constexpr (type_traits::extras::iterators::is_random_access_iterator_v<Iter>) {
                auto length = std::distance(first, last);
                allocate_plus_one_(length);
                core::algorithm::copy(first, last, this->begin_());
                resize_(length);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right) :
            base(allocator_traits::select_on_container_copy_construction(right.get_allocator())) {
            auto other_size = right.size_();
            this->allocate_plus_one_(other_size);
            this->fill_(right.begin_(), right.end_());
            this->resize_(other_size);
        }

        RAINY_CONSTEXPR20 basic_string(const basic_string &right, allocator_type const &a) : base(a) {
            auto other_size = right.size_();
            this->allocate_plus_one_(other_size);
            this->fill_(right.begin_(), right.end_());
            this->resize_(other_size);
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right, size_type pos, size_type count,
                                       allocator_type const &a = allocator_type()) : base(a) {
            auto other_size = right.size_();
            if (pos > other_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (other_size - pos, count);
            if (pos != 0) {
                auto other_begin = right.begin_();
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
            right.resize_(count);
            right.swap(*this);
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right) noexcept : base{right.get_allocator()} {
            right.swap(*this);
        }

        RAINY_CONSTEXPR20 basic_string(basic_string &&right, allocator_type const &a) : base(a) {
            if (right.get_al() == a) {
                right.swap(*this);
            } else {
                basic_string temp{right.data(), right.size(), a};
                temp.swap(*this);
                right.swap(temp);
            }
        }

        RAINY_CONSTEXPR20 basic_string(std::initializer_list<CharType> ilist, allocator_type const &a = allocator_type()) :
            basic_string(std::data(ilist), ilist.size(), a) {
        }


        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &t, allocator_type const &a = allocator_type()) :
            basic_string(std::basic_string_view<CharType, Traits>{t}.data(), std::basic_string_view<CharType, Traits>{t}.size(), a) {
        }


        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>>,
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &t, size_type pos, size_type count,
                                       allocator_type const &a = allocator_type()) : base(a) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (sv_size - pos, count);
            allocate_plus_one_(count);
            fill_(data, data + count);
            resize_(count);
        }

        RAINY_CONSTEXPR20 ~basic_string() {
            if (this->is_long_()) {
                this->dealloc_(this->get_stor().large_string);
            }
        }

        RAINY_CONSTEXPR20 basic_string(std::nullptr_t) = delete;
        RAINY_CONSTEXPR20 basic_string &operator=(std::nullptr_t) = delete;

        RAINY_CONSTEXPR20 basic_string &operator=(basic_string &&right) noexcept(
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Allocator>::is_always_equal::value) {
            return assign(utility::move(right));
        }

        RAINY_CONSTEXPR20 basic_string &operator=(const basic_string &str) {
            return assign(str);
        }

        RAINY_CONSTEXPR20 basic_string &operator=(const CharType *cstr) {
            return assign(cstr);
        }

        RAINY_CONSTEXPR20 basic_string &operator=(CharType ch) {
            this->resize_(1);
            (*this->begin_()) = ch;
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &operator=(std::initializer_list<CharType> ilist) {
            return assign(ilist);
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator=(const StringViewLike &t) {
            return assign(t);
        }

        friend RAINY_CONSTEXPR20 bool operator==(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size_();
            auto rsize = right.size_();
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
                return basic_string::traits_type::compare(left.begin_(), right.begin_(), (core::min) (lsize, rsize)) == 0 &&
                       lsize == rsize;
            }
        }

        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size_();
            auto rsize = right.size_();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto l = left.begin_(), r = right.begin_(), end = l + (core::min) (lsize, rsize); l != end; ++l, ++r) {
                    if (*l < *r)
                        return true;
                    else if (*l > *r)
                        return false;
                }
                return lsize < rsize;
            } else
#endif
            {
                auto res = basic_string::traits_type::compare(left.begin_(), right.begin_(), (core::min) (rsize, lsize));
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
            auto rsize = basic_string::c_style_string_length_(start);
            auto lsize = left.size_();

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
            auto rsize = basic_string::c_style_string_length_(start);
            auto lsize = left.size_();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto l = left.begin_(), end = l + (core::min) (lsize, rsize); l != end; ++l, ++start) {
                    if (*l < *start)
                        return true;
                    else if (*l > *start)
                        return false;
                }
                return lsize < rsize;
            } else
#endif
            {
                auto res = basic_string::traits_type::compare(left.begin_(), start, (core::min) (rsize, lsize));
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

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator+=(const StringViewLike &t) {
            return append(t);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(const basic_string &str) {
            return append(str);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(CharType ch) {
            return push_back(ch);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(const CharType *cstr) {
            return append(cstr);
        }

        RAINY_CONSTEXPR20 basic_string &operator+=(std::initializer_list<CharType> ilist) {
            return append(ilist);
        }

        RAINY_CONSTEXPR20 void shrink_to_fit() noexcept {
            if (this->size() <= this->short_string_max_ && this->is_long_()) {
                auto ls = this->sso.large_string;
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    this->sso.short_string = decltype(this->sso.short_string){};
                }
#endif
                traits_type::copy(this->sso.short_string.data(), ls.begin_, static_cast<std::size_t>(ls.end_ - ls.begin_));
                dealloc_(ls);
            }
        }

        RAINY_CONSTEXPR20 CharType const *data() const noexcept {
            return this->begin_();
        }

        RAINY_CONSTEXPR20 CharType *data() noexcept {
            return this->begin_();
        }

        RAINY_CONSTEXPR20 CharType const *c_str() const noexcept {
            return this->begin_();
        }

        RAINY_CONSTEXPR20 const_reference at(size_type pos) const {
            if (pos >= this->size_())
                throw std::out_of_range{this->exception_string_};
            return *(this->begin_() + pos);
        }

        RAINY_CONSTEXPR20 reference at(size_type pos) {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).at(pos));
        }

        RAINY_CONSTEXPR20 const_reference operator[](size_type pos) const noexcept {
            assert(("pos >= size, please check the arg", pos < this->size_()));
            return *(this->begin_() + pos);
        }

        RAINY_CONSTEXPR20 reference operator[](size_type pos) noexcept {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this)[pos]);
        }

        RAINY_CONSTEXPR20 const CharType &front() const noexcept {
            assert(("string is empty", !this->is_empty_()));
            return *this->begin_();
        }

        RAINY_CONSTEXPR20 CharType &front() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).front());
        }

        RAINY_CONSTEXPR20 const CharType &back() const noexcept {
            assert(("string is empty", !this->is_empty_()));
            return *(this->end_() - 1);
        }

        RAINY_CONSTEXPR20 value_type &back() {
            return const_cast<value_type &>(const_cast<basic_string const &>(*this).back());
        }

        RAINY_CONSTEXPR20 operator std::basic_string_view<CharType, Traits>() const noexcept {
            return std::basic_string_view<CharType, Traits>(this->begin_(), this->end_());
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

        /**
         * @brief resize string length
         * @brief strong exception safety guarantee
         * @brief never shrink
         * @param count new size
         * @param ch character to fill
         */
        RAINY_CONSTEXPR20 void resize(size_type count, CharType ch) {
            auto size = this->size_();
            if (count > size) {
                reserve(count);
                auto begin = this->begin_();
                core::algorithm::fill(begin + size, begin + count, ch);
            }
            resize_(count);
        }

        /**
         * @brief resize string length
         * @brief strong exception safety guarantee
         * @brief never shrink
         * @param count new size
         */
        RAINY_CONSTEXPR20 void resize(size_type count) {
            resize(count, CharType{});
        }

        /**
         * @brief equal to resize(0)
         * @brief never shrink
         */
        RAINY_CONSTEXPR20 void clear() noexcept {
            this->resize_(0);
        }

        /**
         * @brief use size * 1.5 for growth
         * @param ch character to fill
         */
        RAINY_CONSTEXPR20 void push_back(CharType ch) {
            auto size = this->size_();
            if (this->capacity() == size) {
                this->reserve(size * 2 - size / 2);
            }
            *this->end_() = ch;
            this->resize_(size + 1);
        }

        RAINY_CONSTEXPR20 void swap(basic_string &right) noexcept {
            if constexpr (allocator_traits::propagate_on_container_swap::value) {
                std::swap(this->get_al(), right.get_al());
            } else {
                assert(right.get_al() == this->get_al());
            }
            this->swap_data(right);
        }

        friend RAINY_CONSTEXPR20 void swap(basic_string &self, basic_string &right) noexcept {
            self.swap(right);
        }

        /**
         * @brief this version provide for Iter version of assign, right version of append and operator+=
         */
        template <typename Iter, typename = typename std::enable_if<std::is_base_of<
                                     std::input_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>::value>::type>
        RAINY_CONSTEXPR20 basic_string &append(Iter first, Iter last) {
            using category = typename std::iterator_traits<Iter>::iterator_category;

            if (std::is_base_of<std::random_access_iterator_tag, category>::value) {
                auto size = this->size_();
                auto length = std::distance(first, last);
                auto new_size = size + length;
                reserve((core::max) (size * 2, new_size));
                traits_type::copy(this->begin_() + size, utility::addressof(*first), static_cast<std::size_t>(length));
                resize_(new_size);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(size_type count, CharType ch) {
            this->reserve(count);
            traits_type::assign(this->begin_(), count, ch);
            this->resize_(count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const basic_string &str) {
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

        RAINY_CONSTEXPR20 basic_string &assign(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size_();
            if (pos > str_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (npos, (core::min) (str_size - pos, count));
            auto str_begin = this->begin_();
            assign_(str_begin + pos, str_begin + pos + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(basic_string &&right) noexcept {
            if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
                right.swap(*this);
            } else {
                if (this->get_al() == right.get_al()) {
                    this->swap_data(right);
                } else {
                    assign_(right.begin_(), right.end_());
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const CharType *cstr, size_type count) {
            assign_(cstr, cstr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &assign(const CharType *cstr) {
            assign_(cstr, cstr + c_style_string_length_(cstr));
            return *this;
        }

        template <class Iter>
        RAINY_CONSTEXPR20 basic_string &assign(Iter first, Iter last) {
            this->resize_(0);
            return append(first, last);
        }

        RAINY_CONSTEXPR20 basic_string &assign(std::initializer_list<CharType> ilist) {
            auto data = ilist.begin();
            assign_(data, data + ilist.size());
            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        basic_string &assign(const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            assign_(data, data + sv.size());
            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &assign(const StringViewLike &t, size_type pos, size_type count = npos) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (npos, (core::min) (sv_size - pos, count));
            auto data = sv.data();
            assign_(data + pos, data + pos + count);

            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(size_type count, CharType ch) {
            auto size = this->size_();
            reserve(size + count);
            auto end = this->begin_() + size;
            std::fill(end, end + count, ch);
            resize_(size + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(const CharType *cstr, size_type count) {
            append_(cstr, cstr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(const basic_string &str) {
            auto begin = str.begin_();
            append_(begin, begin + str.size_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size_();
            if (pos > str_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (npos, (core::min) (str_size - pos, count));
            return append(str.begin_() + pos, count);
        }

        RAINY_CONSTEXPR20 basic_string &append(const CharType *cstr) {
            append_(cstr, cstr + c_style_string_length_(cstr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &append(std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            append_(data, data + ilist.size());

            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            append_(data, data + sv.size());

            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &t, size_type pos, size_type count = npos) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();
            if (pos > sv_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (npos, (core::min) (sv_size - count, count));

            return append(sv.data() + pos, count);
        }

        RAINY_CONSTEXPR20 bool starts_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = this->begin_();
            if (sv_size > this->size_()) {
                return false;
            }
            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        RAINY_CONSTEXPR20 bool starts_with(CharType ch) const noexcept {
            return *this->begin_() == ch;
        }

        RAINY_CONSTEXPR20 bool starts_with(const CharType *cstr) const {
            auto length = c_style_string_length_(cstr);
            auto begin = this->begin_();
            if (length > this->size_()) {
                return false;
            }
            return equal_(cstr, cstr + length, begin, begin + length);
        }

        RAINY_CONSTEXPR20 bool ends_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = this->end_();
            if (sv_size > this->size_()) {
                return false;
            }
            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        RAINY_CONSTEXPR20 bool ends_with(CharType ch) const noexcept {
            return *(this->end_() - 1) == ch;
        }

        RAINY_CONSTEXPR20 bool ends_with(CharType const *cstr) const {
            auto length = c_style_string_length_(cstr);
            if (length > this->size_()) {
                return false;
            }
            auto end = this->end_();
            return equal_(cstr, cstr + length, end - length, end);
        }

        RAINY_CONSTEXPR20 bool contains(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto size = sv.size();
            auto begin = this->begin_();
            auto data = sv.data();
            if (this->size_() < size) {
                return false;
            }
            if (equal_(begin, begin + size, data, data + size)) {
                return true;
            }
            return std::basic_string_view<CharType, Traits>{begin + 1, begin + size}.contains(sv);
        }

        RAINY_CONSTEXPR20 bool contains(CharType ch) const noexcept {
            for (auto begin = this->begin_(), end = this->end_(); begin != end; ++begin) {
                if (*begin == ch) {
                    return true;
                }
            }
            return false;
        }

        RAINY_CONSTEXPR20 bool contains(const CharType *cstr) const noexcept {
            return std::basic_string_view<CharType, Traits>{this->begin_(), this->end_()}.contains(
                std::basic_string_view<CharType, Traits>{cstr, cstr + c_style_string_length_(cstr)});
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, size_type count, CharType ch) {
            auto size = this->size_();
            if (index > size) {
                throw std::out_of_range{this->exception_string_};
            }
            reserve(size + count);
            auto start = this->begin_() + index;
            auto end = start + size - index;
            auto last = end + count;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy_backward(start, end, last);
                std::fill(start, start + count, ch);
            } else
#endif
            {
                std::memmove(end, start, count * sizeof(CharType));
                std::fill(start, start + count, ch);
            }
            resize_(size + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const CharType *cstr, size_type count) {
            insert_(index, cstr, cstr + count);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const CharType *cstr) {
            insert_(index, cstr, cstr + c_style_string_length_(cstr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str) {
            insert_(index, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos) {
            auto s_size = str.size_();
            if (s_index > s_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (npos, (core::min) (s_size - s_index, count));
            auto s_start = str.begin_() + s_index;
            insert_(index, s_start, s_start + count);
            return *this;
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, CharType ch) {
            auto size = this->size_();
            auto start = pos.base().current_;
            auto end = this->end_();
            auto index = start - this->begin_();
            reserve(size + 1);
            auto begin = this->begin_();
            end = begin + size;
            start = begin + index;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy_backward(start, end, end + 1);
            } else
#endif
            {
                traits_type::move(start + 1, start, end - start);
            }
            *start = ch;
            resize_(size + 1);
            return {start};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, size_type count, CharType ch) {
            auto start = pos.base().current_;
            auto index = start - this->begin_();
            insert(index, count, ch);
            return {start};
        }

        /**
         * @brief this function use a temp basic_string, so it can't decl in class decl
         */
        template <typename Iter>
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, Iter first, Iter last) {
            assert(("pos isn't in this string", pos >= this->begin_() && pos <= this->end_()));
            auto size = this->size_();
            auto start = pos.base().current_;
            auto end = this->end_();
            auto index = start - this->begin_();
            if constexpr (type_traits::extras::iterators::is_random_access_iterator_v<Iter>) {
                auto length = std::distance(first, last);
                reserve(size + length);
                auto begin = this->begin_();
                core::algorithm::copy_backward(begin + index, begin + size, begin + size + length);
                core::algorithm::copy(first, last, begin + index);
                resize_(size + length);
            } else {
                basic_string temp{start, end};
                resize_(pos - this->begin_());
                for (; first != last; ++first) {
                    push_back(*first);
                }
                append_(temp.begin_(), temp.end_());
            }
            return {start};
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, std::initializer_list<CharType> ilist) {
            auto i_data = std::data(ilist);
            auto start = pos.base().current_;
            insert_(start - this->begin_(), i_data, i_data + ilist.size());
            return {start};
        }


        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_data = sv.data();
            insert_(pos, sv_data, sv_data + sv.size());
            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &t, size_type t_index, size_type count = npos) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();
            auto size = this->size_();

            if (t_index > sv_size) {
                throw std::out_of_range{this->exception_string_};
            }

            count = (core::min) (npos, (core::min) (sv_size - t_index, count));
            auto sv_data = sv.data();
            insert_(pos, sv_data + t_index, sv_data + t_index + count);

            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &erase(size_type index = 0, size_type count = npos) {
            auto size = this->size_();
            if (index > size) {
                throw std::out_of_range{this->exception_string_};
            }
            count = (core::min) (npos, (core::min) (size - index, count));
            auto start = this->begin_() + index;
            erase_(start, start + count);

            return *this;
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator position) noexcept {
            auto start = position.base().current_;
            erase_(start, start + 1);
            return {start};
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) noexcept {
            auto start = first.base().current_;
            erase_(start, last.base().current_);
            return {start};
        }

        RAINY_CONSTEXPR20 void pop_back() noexcept {
            assert(("string is empty", !this->is_empty_()));
            resize_(this->size_() - 1);
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str) {
            replace_(pos, count, str.begin_(), str.end_());
            return *pos;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const basic_string &str) {
            auto start = first.base().current_;
            replace_(start - this->begin_(), last - first, str.begin_(), str.end_());
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str, size_type pos2,
                                                size_type count2 = npos) {
            auto str_size = str.size_();
            if (pos2 > str_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count2 = (core::min) (npos, (core::min) (count2, str_size - pos2));
            auto begin = str.begin_();
            replace_(pos, count, begin + count2, begin + count2 + pos2);

            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const CharType *cstr, size_type count2) {
            replace_(pos, count, cstr, cstr + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const CharType *cstr, size_type count2) {
            auto start = first.base().current_;
            replace_(start - this->begin_(), last - first, cstr, cstr + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const CharType *cstr) {
            replace_(pos, count, cstr, cstr + c_style_string_length_(cstr));
            return *pos;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const CharType *cstr) {
            auto start = first.base().current_;
            replace_(start - this->begin_(), last - first, cstr, cstr + c_style_string_length_(cstr));
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            auto start = first.base().current_;
            replace_(start - this->begin_(), last - first, data, data + ilist.size());
            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto data = sv.data();
            replace_(pos, count, data, data + sv.size());
            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const StringViewLike &t) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_data = sv.data();
            auto start = first.base().current_;
            replace_(start - this->begin_(), last - first, sv_data, sv_data + sv.size());
            return *this;
        }

        template <
            typename StringViewLike,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<CharType, Traits>> &&
                    (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const CharType *>),
                int> = 0>
        basic_string &replace(size_type pos, size_type count, const StringViewLike &t, size_type pos2, size_type count2 = npos) {
            std::basic_string_view<CharType, Traits> sv = t;
            auto sv_size = sv.size();
            if (pos2 > sv_size) {
                throw std::out_of_range{this->exception_string_};
            }
            count2 = (core::min) (npos, (core::min) (sv_size - pos2, count2));
            auto data = sv.data();
            replace_(pos, count, data + pos2, data + pos2 + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = this->begin_();
            replace_(pos, count, begin, begin + count2);
            return *this;
        }

        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = this->begin_();
            auto start = first.base().current_;
            replace_(start - this->begin_(), last - first, begin, begin + count2);
            return *this;
        }

        template <
            typename Iter,
            std::enable_if_t<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>::value,
                             int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, Iter first2, Iter last2) {
            auto start = first.base().current_;
            using category = typename std::iterator_traits<Iter>::iterator_category;
            if (std::is_base_of<std::random_access_iterator_tag, category>::value) {
                auto data = std::addressof(*first2);
                auto length2 = std::distance(first2, last2);
                replace_(start - this->begin_(), last - first, data, data + length2);
            } else {
                basic_string temp{first2, last2};
                auto temp_begin = temp.begin_();
                auto temp_size = temp.size_();
                replace_(start - this->begin_(), last - first, temp_begin, temp_begin + temp_size);
            }
            return *this;
        }

        friend std::basic_ostream<value_type, traits_type> &operator<<(std::basic_ostream<value_type, traits_type> &os,
                                                                       const basic_string &str) {
            return os.write(str.data(), str.size());
        }

    private:
        RAINY_CONSTEXPR20 void append_(CharType const *first, CharType const *last) {
            auto length = last - first;
            auto size = this->size_();
            this->reserve(size + length);
            auto end = this->begin_() + size;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy(first, last, end);
            } else
#endif
            {
                traits_type::copy(end, first, static_cast<std::size_t>(length));
            }
            this->resize_(size + length);
        }

        RAINY_CONSTEXPR20 bool static equal_(CharType const *begin, CharType const *end, CharType const *first,
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

        /**
         * @brief caculate the length of c style string
         * @param begin begin of characters
         * @return length
         */
        RAINY_CONSTEXPR20 static size_type c_style_string_length_(CharType const *string) noexcept {
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
        RAINY_CONSTEXPR20 void assign_(CharType const *first, CharType const *last) {
            auto size = last - first;
            if (this->capacity() < static_cast<size_type>(size)) {
                if (this->is_long_()) {
                    auto ls = this->get_stor().large_string;
                    this->allocate_plus_one_(size);
                    this->dealloc_(ls);
                } else {
                    this->allocate_plus_one_(size);
                }
            }
            this->fill_(first, last);
            this->resize_(size);
        }

        /**
         * @brief insert characters to *this
         * @brief this function can be called with any legal state
         * @brief strong exception safety guarantee
         */
        RAINY_CONSTEXPR20 void insert_(size_type index, CharType const *first, CharType const *last) {
            auto size = this->size_();
            if (index > size) {
                throw std::out_of_range{this->exception_string_};
            }
            auto length = last - first;
            auto new_size = size + length;
            auto begin = this->begin_();
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

        RAINY_CONSTEXPR20 void replace_(size_type pos, size_type count, CharType const *first2, CharType const *last2) {
            auto size = this->size_();
            if (pos > size) {
                throw std::out_of_range{this->exception_string_};
            }
            auto begin = this->begin_();
            auto first1 = begin + pos;
            auto last1 = begin + (core::min) (pos + count, size);
            auto length1 = last1 - first1;
            auto length2 = last2 - first2;
            auto new_size = size + length2 - length1;
            auto end = begin + size;
            if (!(last1 < first2 || last2 < first1) && new_size <= this->capacity()) {
                auto diff = length2 - length1;
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

        RAINY_CONSTEXPR20 void erase_(CharType *first, CharType const *last) noexcept {
            assert(("first or last is not in this string", first >= this->begin_() && last <= this->end_()));
            traits_type::move(first, last, static_cast<std::size_t>(const_cast<basic_string const &>(*this).end_() - last));
            resize_(this->size() - (last - first));
        }
    };

    template <typename CharType, typename Traits, typename Allocator>
    RAINY_CONSTEXPR20 void swap(basic_string<CharType, Traits, Allocator> &left, basic_string<CharType, Traits, Allocator> &right) {
        left.swap(right);
    }

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
    struct hash<::rainy::text::basic_string<CharType, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::text::basic_string<CharType, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return rainy::utility::implements::hash_array_representation(val.data(), val.size());
        }
    };
}

namespace rainy::utility {
    template <typename CharType, typename Traits, typename Alloc>
    struct hash<::rainy::text::basic_string<CharType, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = text::basic_string<CharType, Traits, Alloc>;

        result_type hash_this_val(const argument_type &val) const {
            return implements::hash_array_representation(val.data(), val.size());
        }

        result_type operator()(const argument_type &val) const {
            return hash_this_val(val);
        }
    };
}

#if RAINY_USING_GCC
#pragma GCC diagnostic pop
#endif

#endif
