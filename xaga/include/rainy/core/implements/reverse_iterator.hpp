#ifndef RAINY_CORE_IMPLEMENTS_REVERSE_ITERATOR_HPP
#define RAINY_CORE_IMPLEMENTS_REVERSE_ITERATOR_HPP
#include <rainy/core/tmp/iter_traits.hpp>

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

        RAINY_CONSTEXPR20 reverse_iterator() noexcept = default;
        RAINY_CONSTEXPR20 reverse_iterator(const reverse_iterator &) noexcept = default;
        RAINY_CONSTEXPR20 reverse_iterator(reverse_iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 reverse_iterator &operator=(const reverse_iterator &) noexcept = default;
        RAINY_CONSTEXPR20 reverse_iterator &operator=(reverse_iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 ~reverse_iterator() = default;

        explicit RAINY_CONSTEXPR20 reverse_iterator(iterator_type current) noexcept : current(current) {
        }

        RAINY_CONSTEXPR20 iterator_type base() const noexcept {
            return current;
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            iterator_type tmp = current;
            --tmp;
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return tmp;
            } else {
                return tmp.operator->();
            }
        }

        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            iterator_type tmp = current;
            --tmp;
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return tmp;
            } else {
                return tmp.operator->();
            }
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator++() noexcept {
            --current;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator operator++(int) noexcept {
            reverse_iterator tmp = *this;
            --current;
            return tmp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator--() noexcept {
            ++current;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator operator--(int) noexcept {
            reverse_iterator tmp = *this;
            ++current;
            return tmp;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator+=(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        RAINY_CONSTEXPR20 reverse_iterator &operator-=(difference_type n) noexcept {
            current += n;
            return *this;
        }

        RAINY_CONSTEXPR20 reference operator[](difference_type n) noexcept {
            return current[-n - 1];
        }

        RAINY_CONSTEXPR20 const_reference operator[](difference_type n) const noexcept {
            return current[-n - 1];
        }

        RAINY_CONSTEXPR20 explicit operator bool() const noexcept {
            return current != nullptr;
        }

        friend RAINY_CONSTEXPR20 reverse_iterator operator+(const reverse_iterator &it, difference_type n) {
            return reverse_iterator(it.current - n);
        }

        friend RAINY_CONSTEXPR20 reverse_iterator operator-(const reverse_iterator &it, difference_type n) {
            return reverse_iterator(it.current + n);
        }

        friend RAINY_CONSTEXPR20 difference_type operator-(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return rhs.current - lhs.current;
        }

        friend RAINY_CONSTEXPR20 bool operator==(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current == rhs.current;
        }

        friend RAINY_CONSTEXPR20 bool operator!=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current != rhs.current;
        }

        friend RAINY_CONSTEXPR20 bool operator<(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current > rhs.current;
        }

        friend RAINY_CONSTEXPR20 bool operator<=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current >= rhs.current;
        }

        friend RAINY_CONSTEXPR20 bool operator>(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current < rhs.current;
        }

        friend RAINY_CONSTEXPR20 bool operator>=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current <= rhs.current;
        }

    private:
        iterator_type current;
    };
}

#endif