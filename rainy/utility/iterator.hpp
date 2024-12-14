#ifndef RAINY_UTILITY_ITERATOR_HPP
#define RAINY_UTILITY_ITERATOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/diagnostics/contract.hpp>
#include <istream>
#include <ostream>

namespace rainy::type_traits::extras::meta_typedef {
    template <typename Ty, typename = void>
    struct has_difference_type : helper::false_type {};

    template <typename Ty>
    struct has_difference_type<Ty, other_trans::void_t<typename Ty::difference_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_difference_type_v = has_difference_type<Ty>::value;

    template <typename Ty, typename = void>
    struct has_value_type : helper::false_type {};

    template <typename Ty>
    struct has_value_type<Ty, other_trans::void_t<typename Ty::value_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_value_type_v = has_value_type<Ty>::value;

    template <typename Ty, typename = void>
    struct has_pointer : helper::false_type {};

    template <typename Ty>
    struct has_pointer<Ty, other_trans::void_t<typename Ty::pointer>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_pointer_v = has_pointer<Ty>::value;

    template <typename Ty, typename = void>
    struct has_reference : helper::false_type {};

    template <typename Ty>
    struct has_reference<Ty, other_trans::void_t<typename Ty::reference>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_reference_v = has_reference<Ty>::value;

    template <typename Ty, typename = void>
    struct has_iterator_category : helper::false_type {};

    template <typename Ty>
    struct has_iterator_category<Ty, other_trans::void_t<typename Ty::reference>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_iterator_category_v = has_iterator_category<Ty>::value;
}

namespace rainy::utility::internals {
    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_value_type_v<Ty>>
    struct try_to_add_value_type {
        using value_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_value_type<Ty, true> {
        using value_type = typename Ty::value_type;
    };

    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_difference_type_v<Ty>>
    struct try_to_add_difference_type {
        using difference_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_difference_type<Ty, true> {
        using difference_type = typename Ty::difference_type;
    };

    template <typename Ty, bool IsPointer = type_traits::internals::_is_pointer_v<Ty>,
              bool Enable = type_traits::extras::meta_typedef::has_iterator_category_v<Ty>>
    struct try_to_add_iterator_category {
        using iterator_category = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, true, false> {
        using iterator_category = std::random_access_iterator_tag; // 为了兼容标准库设计
    };
    
    template <typename Ty>
    struct try_to_add_iterator_category<Ty, false, true> {
        using iterator_category = typename Ty::iterator_category;
    };

    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_reference_v<Ty>>
    struct try_to_add_reference {
        using reference = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_reference<Ty, true> {
        using reference = typename Ty::reference;
    };

    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_pointer_v<Ty>>
    struct try_to_add_pointer {
        using pointer = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_pointer<Ty, true> {
        using pointer = typename Ty::pointer;
    };

    template <typename Ty>
    struct iterator_traits_base : try_to_add_difference_type<Ty>,
                                  try_to_add_iterator_category<Ty>,
                                  try_to_add_pointer<Ty>,
                                  try_to_add_reference<Ty>,
                                  try_to_add_value_type<Ty> {};
}

namespace rainy::utility {
    template <typename Ty>
    struct iterator_traits : internals::iterator_traits_base<Ty> {};

    template <typename Ty>
    struct iterator_traits<Ty *> {
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        using pointer = Ty *;
        using reference = Ty &;
        using value_type = Ty;
    };

    template <typename Iterator>
    class iterator_range {
    public:
        using traits = iterator_traits<Iterator>;
        using iterator_category = typename traits::iterator_category;
        using difference_type = typename traits::difference_type;
        using pointer = typename traits::pointer;
        using reference = typename traits::reference;
        using value_type = typename traits::value_type;
        using size_type = typename std::size_t;

        constexpr iterator_range() : begin_ptr(nullptr), end_ptr(nullptr) {
        }

        constexpr iterator_range(pointer begin, pointer end) : begin_ptr(begin), end_ptr(end) {
        }

        constexpr iterator_range(const iterator_range &right) noexcept {
            this->begin_ptr = right.begin_ptr;
            this->end_ptr = right.end_ptr;
        }

        constexpr iterator_range(iterator_range && right) noexcept {
            this->begin_ptr = right.begin_ptr;
            this->end_ptr = right.end_ptr;
        }

        constexpr bool valid() const noexcept {
            return (begin_ptr && end_ptr) && (begin_ptr < end_ptr);
        }

        constexpr pointer begin() noexcept {
            return begin_ptr;
        }

        constexpr pointer end() noexcept {
            return end_ptr;
        }

        constexpr pointer begin() const noexcept {
            return begin_ptr;
        }

        constexpr pointer end() const noexcept {
            return end_ptr;
        }

        constexpr size_type length() const noexcept {
            return end_ptr - begin_ptr;
        }

        constexpr void run_assert(pointer current, std::string_view error_value_init_iterator_msg,
                                  std::string_view error_out_of_range_msg) const {
#if RAINY_ENABLE_DEBUG
            if (valid()) {
                expects(current, error_value_init_iterator_msg);
                expects(begin_ptr <= current && current < end_ptr, error_out_of_range_msg);
            }
#endif
        }

    private:
        pointer begin_ptr;
        pointer end_ptr;
    };

    template <typename Iter>
    static iterator_range<Iter> placeholder_iter_range;

    template <typename Iterator, typename Ty>
    constexpr auto make_iterator_range(Ty *begin, Ty *end) noexcept {
        return iterator_range<Iterator>(begin, end);
    }
}

namespace rainy::utility {
    template <typename Iter>
    class const_iterator {
    public:
        template <typename>
        friend class iterator;

        using iterator_type = Iter;
        using iterator_traits = utility::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        using range_base = iterator_range<iterator_type>;

        constexpr const_iterator() noexcept {};
        constexpr const_iterator &operator=(const_iterator &&) noexcept = default;
        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;

        explicit constexpr const_iterator(iterator_type current) noexcept :
           range_base_(), current(current) {
        }

        constexpr const_iterator(iterator_type current,range_base safe_range) noexcept :
            range_base_(safe_range), current(current) {
        }

        constexpr const_iterator(const const_iterator &right) : range_base_(right.range_base_), current(right.current) {
        }

        constexpr const_iterator(const_iterator &&right) noexcept : range_base_(right), current(utility::exchange(right.current, {})) {
        }

        RAINY_CONSTEXPR20 ~const_iterator() = default;

        constexpr const_pointer begin() const noexcept {
            return range_base_.begin();
        }

        constexpr const_pointer end() const noexcept {
            return range_base_.end();
        }

        constexpr std::size_t length() const noexcept {
            return range_base_.length();
        }

        constexpr const_pointer get_pointer() const noexcept {
            this->run_assert(current, "can't access value-initialized iterator.", "can't get a pointer from a out of range iterator.");
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return current;
            } else {
                return current.operator->();
            }
        }

        constexpr const_pointer operator->() const noexcept {
            return get_pointer();
        }

        constexpr const_reference dereference() const noexcept {
            range_base_.run_assert(current, "can't access value-initialized iterator.", "can't dereference a out of range iterator.");
            return *current;
        }

        constexpr const_reference operator*() const noexcept {
            return dereference();
        }

        constexpr const_iterator &self_inc_prefix() noexcept {
            ++current;
            return *this;
        }

        constexpr const_iterator self_inc_postfix() noexcept {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        constexpr const_iterator &operator++() noexcept {
            return self_inc_prefix();
        }

        constexpr const_iterator operator++(int) noexcept {
            return self_inc_postfix();
        }

        constexpr const_iterator &self_dec_prefix() noexcept {
            --current;
            return *this;
        }

        constexpr const_iterator self_dec_postfix() noexcept {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr const_iterator &operator--() noexcept {
            return self_dec_prefix();
        }

        constexpr const_iterator operator--(int) noexcept {
            return self_dec_postfix();
        }
        
        constexpr const_iterator &plus_equal(difference_type n) noexcept {
            current += n;
            return *this;
        }

        constexpr const_iterator &minus_equal(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        constexpr const_iterator &operator+=(difference_type n) noexcept {
            return plus_equal(n);
        }

        constexpr const_iterator &operator-=(difference_type n) noexcept {
            return minus_equal(n);
        }

        constexpr bool is_equal(const const_iterator &right) const noexcept {
            return current == right.current;
        }

        constexpr bool not_equal(const const_iterator &right) const noexcept {
            return current != right.current;
        }

        constexpr friend bool operator==(const const_iterator &left, const const_iterator &right) {
            return left.is_equal(right);
        }

        constexpr friend bool operator!=(const const_iterator &left, const const_iterator &right) {
            return left.not_equal(right);
        }

        constexpr const_reference at_subscript(difference_type idx) const noexcept {
            this->run_assert(current, "can't access value-initialized iterator.", "can't dereference a out of range iterator.");
            return current[idx];
        }

        constexpr const_reference operator[](difference_type idx) const noexcept {
            return at_subscript(idx);
        }

        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept {
            return this->current == nullptr;
        }

        constexpr explicit operator bool() const noexcept {
            return !empty();
        }

        constexpr const_iterator add_offset(difference_type n) const noexcept {
            return const_iterator{current + n};
        }
        
        constexpr friend const_iterator operator+(const const_iterator &left, difference_type n) {
            return left.add_offset(n);
        }

        constexpr difference_type subtract(difference_type n) const noexcept {
            return current - n;
        }

        constexpr difference_type subtract(const const_iterator &right) const noexcept {
            return current - right.current;
        }

        constexpr friend difference_type operator-(const const_iterator &left, const const_iterator &right) {
            return left.subtract(right);
        }

        constexpr friend difference_type operator-(const const_iterator &left, difference_type n) {
            return left.subtract(n);
        }

        constexpr bool lt(const const_iterator &right) const noexcept {
            return current < right.current;
        }

        constexpr friend bool operator<(const const_iterator &left, const const_iterator &right) {
            return left.lt(right);
        }

        constexpr bool lt_or_equal(const const_iterator &right) const noexcept {
            return current <= right.current;
        }

        constexpr friend bool operator<=(const const_iterator &left, const const_iterator &right) {
            return left.lt_or_equal(right);
        }

        constexpr bool greater_than(const const_iterator &right) const noexcept {
            return current > right.current;
        }

        constexpr friend bool operator>(const const_iterator &left, const const_iterator &right) {
            return left.greater_than(right);
        }

        constexpr bool greater_than_or_equal(const const_iterator &right) const noexcept {
            return current >= right.current;
        }

        constexpr friend bool operator>=(const const_iterator &left, const const_iterator &right) {
            return left.greater_than_or_equal(right);
        }

    private:
        iterator_type current;
        iterator_range<Iter> range_base_;
    };

    template <typename Iter>
    class iterator : public const_iterator<Iter> {
    public:
        using iterator_type = Iter;
        using iterator_traits = utility::iterator_traits<iterator_type>;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;

        using base = const_iterator<Iter>;

        constexpr iterator() noexcept : base() {};

        constexpr iterator(iterator_type current, iterator_range<Iter> safe_range) noexcept : base(current, safe_range) {
        }

        constexpr iterator(const iterator &right) : base(right) {
        }

        constexpr iterator(iterator &&right) noexcept : base(right) {
        }

        RAINY_CONSTEXPR20 ~iterator() = default;

        constexpr base &as_const_iterator() const noexcept {
            return *this;
        } 

        constexpr pointer begin() noexcept {
            return const_cast<pointer>(this->range_base_.begin());
        }

        constexpr pointer end() noexcept {
            return const_cast<pointer>(this->range_base_.end());
        }

        constexpr pointer get_pointer() noexcept {
            return const_cast<pointer>(static_cast<const base *>(this)->get_pointer());
        }

        constexpr pointer operator->() noexcept {
            return get_pointer();
        }

        constexpr reference dereference() noexcept {
            return const_cast<reference>(static_cast<const base *>(this)->dereference());
        }

        constexpr reference operator*() noexcept {
            return dereference();
        }
    };
}

namespace rainy::utility {
    template <typename Iter>
    struct get_iterator_category {
        using tag = typename Iter::iterator_category;
    };
}

namespace rainy::utility {
    template <typename Iter>
    class reverse_iterator {
    public:
        using iterator_type = Iter;
        using iterator_traits = utility::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using pointer = typename iterator_traits::pointer;
        using const_pointer = const value_type *;

        RAINY_CONSTEXPR20 reverse_iterator() noexcept {};
        RAINY_CONSTEXPR20 reverse_iterator &operator=(reverse_iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 reverse_iterator &operator=(const reverse_iterator &) noexcept = default;

        explicit RAINY_CONSTEXPR20 reverse_iterator(iterator_type current) noexcept : current(current) {
        }

        RAINY_CONSTEXPR20 reverse_iterator(const reverse_iterator &right) : current(right.current) {
        }

        RAINY_CONSTEXPR20 reverse_iterator(reverse_iterator &&right) noexcept : current(utility::exchange(right.current, {})) {
        }

        RAINY_CONSTEXPR20 ~reverse_iterator() = default;

        RAINY_NODISCARD constexpr iterator_type base() const noexcept {
            return current;
        }

        constexpr pointer get_pointer() noexcept {
            return const_cast<pointer>(this->get_pointer());
        }

        constexpr const_pointer get_pointer() const noexcept {
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return current;
            } else {
                return current.operator->();
            }
        }

        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            return get_pointer();
        }

        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            return get_pointer();
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept(noexcept(*--(utility::declval<iterator_type &>()))) {
            iterator_type tmp = current;
            return *--tmp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator++() noexcept {
            --current;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator++(int) noexcept {
            reverse_iterator temp = *this;
            --(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator--() noexcept {
            --current;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator operator--(int) noexcept {
            reverse_iterator temp = *this;
            ++(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator+=(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        reverse_iterator &operator-=(difference_type n) noexcept {
            current += n;
            return *this;
        }

        void swap(reverse_iterator &right) noexcept {
            std::swap(this->current, right.current);
        }

        friend bool operator==(const reverse_iterator &left, const reverse_iterator &right) {
            return left.current == right.current;
        }

        friend bool operator!=(const reverse_iterator &left, const reverse_iterator &right) {
            return left.current != right.current;
        }

        reference operator[](difference_type idx) noexcept {
            return current[-idx - 1];
        }

        const_reference operator[](difference_type idx) const noexcept {
            return current[-idx - 1];
        }

        bool empty() const noexcept {
            return this->current == nullptr;
        }

        explicit operator bool() const noexcept {
            return !empty();
        }

        friend reverse_iterator operator+(const reverse_iterator &_iterator, difference_type n) {
            return reverse_iterator{_iterator.current - n};
        }

        friend reverse_iterator operator+(const reverse_iterator &left, const reverse_iterator &right) {
            return reverse_iterator{left.current - right.current};
        }

        friend difference_type operator-(const reverse_iterator &left, const reverse_iterator &right) {
            return left.current + right.current;
        }

        constexpr bool lt(const reverse_iterator &right) const noexcept {
            return current < right.current;
        }

        constexpr friend bool operator<(const reverse_iterator &left, const reverse_iterator &right) {
            return left.lt(right);
        }

        constexpr bool lt_or_equal(const reverse_iterator &right) const noexcept {
            return current <= right.current;
        }

        constexpr friend bool operator<=(const reverse_iterator &left, const reverse_iterator &right) {
            return left.lt_or_equal(right);
        }

        constexpr bool greater_than(const reverse_iterator &right) const noexcept {
            return current > right.current;
        }

        constexpr friend bool operator>(const reverse_iterator &left, const reverse_iterator &right) {
            return left.greater_than(right);
        }

        constexpr bool greater_than_or_equal(const reverse_iterator &right) const noexcept {
            return current >= right.current;
        }

        constexpr friend bool operator>=(const reverse_iterator &left, const reverse_iterator &right) {
            return left.greater_than_or_equal(right);
        }

    private:
        iterator_type current;
    };
}
namespace rainy::utility {
    template <typename Ty, typename Elem = char, typename Traits = std::char_traits<Elem>,typename Diff = std::ptrdiff_t>
    class istream_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Ty;
        using difference_type = Diff;
        using pointer = const value_type *;
        using reference = const value_type &;
        using char_type = Elem;
        using traits_type = Traits;
        using istream_type = std::basic_istream<Elem, Traits>;

        static_assert(type_traits::logical_traits::conjunction_v<std::is_default_constructible<Ty>, std::is_copy_constructible<Ty>,
                                                                 std::is_copy_assignable<Ty>>,
                      "istream_iterator<T> requires T to be default constructible, copy constructible, and copy assignable. ");

        constexpr istream_iterator() noexcept(std::is_nothrow_default_constructible_v<Ty>) {
        }

        istream_iterator(istream_iterator &ifstream) : istream(addressof(ifstream)){
        }

        RAINY_NODISCARD reference operator*() const noexcept {
            expects(istream, "The stored istream pointer in istream_iterator must be not-null!");
            return value;
        }

        RAINY_NODISCARD pointer operator->() const noexcept {
            expects(istream, "The stored istream pointer in istream_iterator must be not-null!");
            return addressof(value);
        }

        istream_iterator &operator++() {
            read_next();
            return *this;
        }

        istream_iterator self_inc_postfix() {
            
        }

        istream_iterator operator++(int) {
            istream_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        RAINY_NODISCARD bool equal(const istream_iterator& right) const noexcept {
            return istream == right.istream;
        }

        template <typename Ty_, typename Elem_, typename Traits_, typename Diff_>
        RAINY_NODISCARD friend bool operator==(const istream_iterator<Ty_, Elem_, Traits_, Diff_> &left,
                                               const istream_iterator<Ty_, Elem_, Traits_, Diff_> &right) {
            return left.equal(right);
        }

        RAINY_NODISCARD friend bool operator==(const istream_iterator &left, const istream_iterator &right) {
            return left.equal(right);
        }

    private:
        void read_next() {
            expects(istream, "The stored istream pointer in istream_iterator must be not-null!");
            if (!(*istream >> value)) {
                istream = nullptr;
            }
        }

        istream_type *istream{nullptr};
        Ty value{};
    };

    template <typename Ty, typename Elem = char, typename Traits = std::char_traits<Elem>>
    class ostream_iterator {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = void;
        using char_type = Elem;
        using traits_type = Traits;
        using ostream_type = std::basic_ostream<Elem, Traits>;

        ostream_iterator() = delete;

        ostream_iterator(ostream_type &ostream, const char_type *const delim = nullptr) noexcept :
            delim(delim), ostream(rainy::utility::addressof(ostream)) {
        }

        ostream_iterator &operator=(const Ty &val) {
            *ostream << val;
            if (delim) {
                *ostream << delim;
            }
            return *this;
        }

        _NODISCARD ostream_iterator &operator*() noexcept {
            return *this;
        }

        ostream_iterator &operator++() noexcept {
            return *this;
        }

        ostream_iterator &operator++(int) noexcept {
            return *this;
        }

    private:
        const char_type *delim;
        ostream_type *ostream;
    };
}

#endif 
