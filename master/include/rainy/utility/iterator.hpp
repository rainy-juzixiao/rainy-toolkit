#ifndef RAINY_UTILITY_ITERATOR_HPP
#define RAINY_UTILITY_ITERATOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>

namespace rainy::utility {
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class const_iterator {
    public:
        template <typename, typename>
        friend class iterator;

        friend class foundation::system::memory::pointer_traits<const_iterator<Iter, Traits>>;

        using iterator_type = Iter;
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;
        constexpr const_iterator() noexcept : current() {};
        constexpr const_iterator &operator=(const_iterator &&) noexcept = default;
        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;

        explicit constexpr const_iterator(iterator_type current) noexcept : current(current) {
        }

        constexpr const_iterator(const const_iterator &right) : current(right.current) {
        }

        constexpr const_iterator(const_iterator &&right) noexcept : current(rainy::utility::exchange(right.current, nullptr)) {
        }

        RAINY_CONSTEXPR20 ~const_iterator() = default;

        constexpr const_pointer get_pointer() const noexcept {
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
    };
}

//template <typename Iter,typename Traits>
//struct rainy::foundation::system::memory::pointer_traits<rainy::utility::const_iterator<Iter, Traits>> {
//    using pointer = typename rainy::utility::const_iterator<Iter, Traits>;
//    using elemen_type = typename pointer::value_type;
//    using difference_type = typename pointer::difference_type;
//
//    RAINY_NODISCARD static RAINY_CONSTEXPR20 elemen_type* to_address(const pointer val) noexcept {
//        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
//        return utility::to_address(val.current);
//    }
//};

namespace rainy::utility {
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class iterator : public const_iterator<Iter,Traits> {
    public:
        using iterator_type = Iter;
        using iterator_traits = Traits;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;

        using base = const_iterator<Iter, Traits>;

        friend class foundation::system::memory::pointer_traits<const_iterator<Iter, Traits>>;

        constexpr iterator() noexcept : base() {};

        constexpr iterator(iterator_type current) noexcept : base(current) {
        }

        constexpr iterator(const iterator &right) : base(right) {
        }

        constexpr iterator(iterator &&right) noexcept : base(right) {
        }

        RAINY_CONSTEXPR20 ~iterator() = default;

        constexpr base &as_const_iterator() const noexcept {
            return *this;
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

template <typename Iter, typename Traits>
struct rainy::foundation::system::memory::pointer_traits<rainy::utility::iterator<Iter, Traits>> {
    using pointer = typename rainy::utility::iterator<Iter, Traits>;
    using elemen_type = typename pointer::value_type;
    using difference_type = typename pointer::difference_type;

    RAINY_NODISCARD static constexpr elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
    template <typename Iter>
    struct iterator_category {
        using type = typename Iter::iterator_category;
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

        RAINY_CONSTEXPR20 reference dereference() noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        RAINY_CONSTEXPR20 const_reference dereference() const noexcept(noexcept(*--(utility::declval<iterator_type &>()))) {
            iterator_type tmp = current;
            return *--tmp;
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            return dereference();
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            return dereference();
        }

        RAINY_CONSTEXPR20 reverse_iterator &self_inc_prefix() noexcept {
            --current;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator self_inc_postfix(int) noexcept {
            reverse_iterator temp = *this;
            --(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &self_dec_prefix() noexcept {
            ++current;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator self_dec_postfix() noexcept {
            reverse_iterator temp = *this;
            ++(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator++() noexcept {
            return self_inc_prefix();
        }

        RAINY_CONSTEXPR20 reverse_iterator operator++(int) noexcept {
            return self_inc_postfix();
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator--() noexcept {
            return self_dec_prefix();
        }

        RAINY_CONSTEXPR20 reverse_iterator operator--(int) noexcept {
            return self_dec_postfix();
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
    template <typename Container, typename PredicateType, typename ProtoTypeGetData, typename ProtoTypeGetSize>
    class const_bidirectional_iterator {
    public:
        using container_type = Container;
        using iterator_traits =
            rainy::utility::make_iterator_traits<std::ptrdiff_t, std::bidirectional_iterator_tag, typename container_type::pointer,
                                                 typename container_type::reference, typename container_type::value_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using pointer = typename iterator_traits::pointer;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        const_bidirectional_iterator(container_type *container, std::size_t idx, PredicateType pred, ProtoTypeGetData get_data,
                                     ProtoTypeGetSize get_size) :
            container(container), current_index(idx),
            functions(move(pred), (move(get_data), move(get_size))) {
            find_next_occupied_forward();
        }

        const_reference operator*() const {
            return get_data(current_index);
        }

        const_bidirectional_iterator &operator++() {
            if (current_index < get_size()) {
                ++current_index;
                find_next_occupied_forward();
            }
            return *this;
        }

        const_bidirectional_iterator operator++(int) {
            const_bidirectional_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_bidirectional_iterator &operator--() {
            if (current_index > 0) {
                --current_index;
                find_next_occupied_backward();
            }
            return *this;
        }

        const_bidirectional_iterator operator--(int) {
            const_bidirectional_iterator temp = *this;
            --(*this);
            return temp;
        }

        friend bool operator==(const const_bidirectional_iterator &left, const const_bidirectional_iterator &right) {
            return left.current_index == right.current_index && left.container == right.container;
        }

        friend bool operator!=(const const_bidirectional_iterator &left, const const_bidirectional_iterator &right) {
            return left.current_index != right.current_index && left.container != right.container;
        }

    private:
        void find_next_occupied_forward() {
            while (current_index < get_size() && !predicate(current_index)) {
                ++current_index;
            }
        }

        void find_next_occupied_backward() {
            while (current_index > 0 && !predicate(current_index - 1)) {
                --current_index;
            }
        }

        bool predicate(std::size_t index) const noexcept {
            return functions.get_first()(container, index);
        }

        const_reference get_data(std::size_t index) const {
            return functions.get_second().get_first()(container, index);
        }

        std::size_t get_size() const {
            return functions.get_second().get_second()(container);
        }

        container_type *container;
        std::size_t current_index;
        compressed_pair<PredicateType, compressed_pair<ProtoTypeGetData, ProtoTypeGetSize>> functions;
    };
}

#endif 
