#ifndef RAINY_UTILITY_ITERATOR_HPP
#define RAINY_UTILITY_ITERATOR_HPP
#include <rainy/core.hpp>

namespace rainy::type_traits::extras::meta_typedef {
    template <typename Ty>
    struct has_difference_type {
        template <typename U, typename = other_transformations::void_t<decltype(Ty::difference_type)>>
        static auto test(int) -> helper::true_type {
            return helper::true_type{};
        }

        template <typename, typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    inline constexpr bool has_difference_type_v = has_difference_type<Ty>::value;

    template <typename Ty>
    struct has_value_type {
        template <typename U, typename = other_transformations::void_t<decltype(Ty::value_type)>>
        static auto test(int) -> helper::true_type {
            return helper::true_type{};
        }

        template <typename, typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    inline constexpr bool has_value_type_v = has_value_type<Ty>::value;

    template <typename Ty>
    struct has_pointer {
        template <typename U, typename = other_transformations::void_t<decltype(Ty::pointer)>>
        static auto test(int) -> helper::true_type {
            return helper::true_type{};
        }

        template <typename, typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    inline constexpr bool has_pointer_v = has_pointer<Ty>::value;

    template <typename Ty>
    struct has_reference {
        template <typename U, typename = other_transformations::void_t<decltype(Ty::reference)>>
        static auto test(int) -> helper::true_type {
            return helper::true_type{};
        }

        template <typename, typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    inline constexpr bool has_reference_v = has_reference<Ty>::value;

    template <typename Ty>
    struct has_iterator_category {
        template <typename U, typename = other_transformations::void_t<decltype(Ty::iterator_category)>>
        static auto test(int) -> helper::true_type {
            return helper::true_type{};
        }

        template <typename, typename>
        static helper::false_type test(...) {
            return helper::false_type{};
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test<Ty>(0))::value;
    };

    template <typename Ty>
    inline constexpr bool has_iterator_category_v = has_iterator_category<Ty>::value;
}

namespace rainy::utility::internals {
    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_value_type_v<Ty>>
    struct try_to_add_value_type {};

    template <typename Ty>
    struct try_to_add_value_type<Ty, true> {
        using value_type = typename Ty::value_type;
    };

    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_difference_type_v<Ty>>
    struct try_to_add_difference_type {};

    template <typename Ty>
    struct try_to_add_difference_type<Ty, true> {
        using difference_type = typename Ty::difference_type;
    };

    template <typename Ty, bool IsPointer = type_traits::internals::_is_pointer_v<Ty>,
              bool Enable = type_traits::extras::meta_typedef::has_iterator_category_v<Ty>>
    struct try_to_add_iterator_category {};

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, true, false> {
        using iterator_category = std::random_access_iterator_tag; // 为了兼容std标准库设计
    };
    
    template <typename Ty>
    struct try_to_add_iterator_category<Ty, false, true> {
        using iterator_category = typename Ty::iterator_category;
    };

    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_reference_v<Ty>>
    struct try_to_add_reference {};

    template <typename Ty>
    struct try_to_add_reference<Ty, true> {
        using reference = typename Ty::reference;
    };

    template <typename Ty, bool Enable = type_traits::extras::meta_typedef::has_pointer_v<Ty>>
    struct try_to_add_pointer {};

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
}

namespace rainy::utility {
    template <typename IterRaw, typename Container>
    class iterator {
    public:
        static_assert(type_traits::internals::_is_pointer_v<IterRaw>, "Iter must me a pointer type.");

        using iterator_type = IterRaw;
        using iterator_traits = utility::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using pointer = typename iterator_traits::pointer;
        using const_pointer = const value_type *;

        RAINY_CONSTEXPR20 iterator() noexcept {};
        RAINY_CONSTEXPR20 iterator &operator=(iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 iterator &operator=(const iterator &) noexcept = default;

        explicit RAINY_CONSTEXPR20 iterator(iterator_type current) noexcept : ptr(current) {
        }

        RAINY_CONSTEXPR20 iterator(const iterator &right) : ptr(right.ptr) {
        }

        RAINY_CONSTEXPR20 iterator(iterator &&right) noexcept : ptr(utility::exchange(right.ptr, nullptr)) {
        }

        template <typename UIter,
                  type_traits::other_transformations::enable_if_t<
            type_traits::type_relations::is_convertible_v<UIter, IterRaw>,
            int> = 0>
        RAINY_CONSTEXPR20 explicit iterator(const iterator<UIter, Container> &right) : ptr(right.ptr) {
        }

        template <typename UIter,
                  type_traits::other_transformations::enable_if_t<
            type_traits::type_relations::is_convertible_v<UIter, IterRaw>,
            int> = 0>
        RAINY_CONSTEXPR20 explicit iterator(iterator<UIter, Container> &&right) : ptr(right.ptr) {
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

#endif 
