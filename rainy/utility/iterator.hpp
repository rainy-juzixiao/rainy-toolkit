#ifndef RAINY_UTILITY_ITERATOR_HPP
#define RAINY_UTILITY_ITERATOR_HPP
#include <rainy/core.hpp>
#include <rainy/meta/type_traits.hpp>

namespace rainy::foundation::type_traits::extras::meta_typedef {
    template <typename IterClass>
    struct has_difference_type {
        template <typename U>
        static auto test(int) -> typename primary_types::is_class<typename U::difference_type>::type {
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    struct has_value_type {
        template <typename U>
        static auto test(int) -> typename primary_types::is_class<typename U::value_type>::type {
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename IterClass>
    struct has_pointer {
        template <typename U>
        static auto test(int) -> typename primary_types::is_class<typename U::pointer>::type {
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename IterClass>
    struct has_reference {
        template <typename U>
        static auto test(int) -> typename primary_types::is_class<typename U::reference>::type {
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename IterClass>
    struct has_iterator_category {
        template <typename U>
        static auto test(int) -> typename primary_types::is_class<typename U::iterator_category>::type {
        }

        template <typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };
}

namespace rainy::utility {
    template <typename Iter, typename Container>
    class iterator {
    public:
        static_assert(foundation::type_traits::is_pointer_v<Iter>, "Iter must me a pointer type.");

        using iterator_type = Iter;
        using iterator_traits = std::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using pointer = typename iterator_traits::pointer;
        using const_pointer = const value_type *;

        RAINY_CONSTEXPR20 iterator() noexcept {
        };
        RAINY_CONSTEXPR20 iterator &operator=(iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 iterator &operator=(const iterator &) noexcept = default;

        explicit RAINY_CONSTEXPR20 iterator(iterator_type current) noexcept : ptr(current) {
        }

        RAINY_CONSTEXPR20 iterator(const iterator &right) : ptr(right.ptr) {
        }

        RAINY_CONSTEXPR20 iterator(iterator &&right) noexcept : ptr(utility::exchange(right.ptr, nullptr)) {
        }

        template <typename Iter_, foundation::type_traits::other_transformations::enable_if_t<
            foundation::type_traits::type_relations::is_convertible_v<Iter_, Iter>, int> = 0>
        RAINY_CONSTEXPR20 explicit iterator(const iterator<Iter_, Container> &right) : ptr(right.ptr) {
        }

        template <typename Iter_, foundation::type_traits::other_transformations::enable_if_t<
            foundation::type_traits::type_relations::is_convertible_v<Iter_, Iter>, int> = 0> 
        RAINY_CONSTEXPR20 explicit iterator(iterator<Iter_, Container> &&right) : ptr(right.ptr) {
        }

        RAINY_CONSTEXPR20 ~iterator() = default;

        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            return ptr;
        }

        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            return ptr;
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            return *ptr;
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            return *ptr;
        }

        RAINY_CONSTEXPR20 iterator operator++() noexcept {
            return iterator{++ptr};
        }

        RAINY_CONSTEXPR20 iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 iterator operator--() noexcept {
            return iterator{--ptr};
        }

        RAINY_CONSTEXPR20 iterator operator--(int) noexcept {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 iterator &operator+=(difference_type n) noexcept {
            ptr += n;
            return *this;
        }

        RAINY_CONSTEXPR20 iterator &operator-=(difference_type n) noexcept {
            ptr -= n;
            return *this;
        }

        RAINY_CONSTEXPR20 void swap(iterator &right) noexcept {
            std::swap(this->ptr, right.ptr);
        }

        RAINY_CONSTEXPR20 friend bool operator==(const iterator &left, const iterator &right) {
            return left.ptr == right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator!=(const iterator &left, const iterator &right) {
            return left.ptr != right.ptr;
        }

        RAINY_CONSTEXPR20 reference operator[](difference_type idx) noexcept {
            return ptr[idx];
        }

        RAINY_CONSTEXPR20 const_reference operator[](difference_type idx) const noexcept {
            return ptr[idx];
        }

        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept {
            return this->ptr == nullptr;
        }

        RAINY_CONSTEXPR20 explicit operator bool() const noexcept {
            return !empty();
        }

        RAINY_CONSTEXPR20 friend iterator operator+(const iterator &_iterator, difference_type n) {
            return iterator{_iterator.ptr + n};
        }

        RAINY_CONSTEXPR20 friend iterator operator+(const iterator &left, const iterator &right) {
            return iterator{left.ptr + right.ptr};
        }

        RAINY_CONSTEXPR20 friend difference_type operator-(const iterator &left, const iterator &right) {
            return left.ptr - right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator<(const iterator &left, const iterator &right) {
            return left.ptr < right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator<=(const iterator &left, const iterator &right) {
            return left.ptr <= right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator>(const iterator &left, const iterator &right) {
            return left.ptr > right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator>=(const iterator &left, const iterator &right) {
            return left.ptr >= right.ptr;
        }

    private:
        pointer ptr;
    };
}

#endif // !RAINY_ITERATOR_HPP
