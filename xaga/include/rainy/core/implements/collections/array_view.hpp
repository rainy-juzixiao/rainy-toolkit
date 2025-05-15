#ifndef RAINY_CORE_IMPLEMENTS_COLLECTIONS_ARRAY_VIEW_HPP
#define RAINY_CORE_IMPLEMENTS_COLLECTIONS_ARRAY_VIEW_HPP
#include <vector>
#include <array>
#include <rainy/core/platform.hpp>
#include <rainy/core/implements/reverse_iterator.hpp>
#include <rainy/core/implements/collections/array.hpp>
#include <rainy/core/implements/exceptions.hpp>

namespace rainy::collections::views {
    template <typename Ty>
    class array_view {
    public:
        using value_type = Ty;
        using size_type = std::size_t;
        using reference = Ty &;
        using const_reference = const Ty &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using difference_type = std::ptrdiff_t;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = const utility::reverse_iterator<iterator>;

        constexpr array_view() = default;

        RAINY_CONSTEXPR20 array_view(array_view &&) = default;
        RAINY_CONSTEXPR20 array_view(const array_view &) = default;
        array_view &operator=(array_view &&) = default;
        array_view &operator=(const array_view &) = default;

        RAINY_CONSTEXPR20 ~array_view() = default;

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(value_type (&reference_array)[N]) : data_(reference_array), size_(N) {
        }

        RAINY_CONSTEXPR20 array_view(std::vector<Ty> &vector) : data_(vector.data()), size_(vector.size()) {
        }

        RAINY_CONSTEXPR20 array_view(const std::vector<Ty> &vector) : data_(vector.data()), size_(vector.size()) {
        }

        RAINY_CONSTEXPR20 array_view(pointer first, pointer last) : data_(first), size_(std::distance(first, last)) {
        }

        RAINY_CONSTEXPR20 array_view(const_pointer first, const_pointer last) : data_(first), size_(std::distance(first, last)) {
        }

#if RAINY_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winit-list-lifetime"
#endif

        RAINY_CONSTEXPR20 array_view(std::initializer_list<Ty> initializer_list) :
            data_(initializer_list.begin()),
            size_(initializer_list.size()) {
        }
        
#if RAINY_USING_GCC
#pragma GCC diagnostic pop
#endif

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(std::array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(const std::array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(const array<Ty, N> &array) : data_(const_cast<Ty *>(array.data())), size_(array.size()) {
        }

        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return iterator(data());
        }

        RAINY_CONSTEXPR20 iterator end() noexcept {
            return iterator{data() + size()};
        }

        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return const_iterator{data()};
        }

        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return const_iterator{data() + size()};
        }

        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return const_iterator{data()};
        }

        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return const_iterator{data() + size()};
        }

        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
            return reverse_iterator{end()};
        }

        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
            return reverse_iterator{begin()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator{end()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator{begin()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator{end()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator{begin()};
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer data() noexcept {
            return const_cast<pointer>(data_);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const_pointer data() const noexcept {
            return data_;
        }

        RAINY_NODISCARD constexpr size_type size() const {
            return size_;
        }

        RAINY_NODISCARD constexpr bool empty() const {
            return size_ == 0;
        }

        RAINY_CONSTEXPR20 reference at(const difference_type idx) {
            rangecheck(size(), idx);
            return const_cast<reference>(data_[idx]);
        }

        RAINY_NODISCARD constexpr const_reference at(const difference_type idx) const {
            rangecheck(size(), idx);
            return data_[idx];
        }

        constexpr reference operator[](const difference_type idx) {
            return const_cast<reference>(data_[idx]);
        }

        constexpr const_reference operator[](const difference_type idx) const {
            return data_[idx];
        }

        friend RAINY_CONSTEXPR20 bool operator==(array_view &left, array_view &right) {
            return core::algorithm::all_of(left.begin(), left.end(), right.begin(), [](const auto &left, const auto &right) { return left == right; });
        }

        friend RAINY_CONSTEXPR20 bool operator!=(array_view &left, array_view &right) {
            return core::algorithm::all_of(left.begin(), left.end(), right.begin(),
                               [](const auto &left, const auto &right) { return left != right; });
        }

    private:
        RAINY_NODISCARD constexpr bool check_index(const size_type idx) const noexcept {
            return idx < size_;
        }

        static void rangecheck(const size_type size, const difference_type idx) {
            if (size <= static_cast<size_type>(idx)) {
                utility::throw_exception(std::out_of_range("Invalid array subscript"));
            }
        }

        const_pointer data_{};
        size_type size_{};
    };

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(Ty *first, Ty *last) {
        return array_view<Ty>(first, last);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const Ty *first, const Ty *last) {
        return array_view<Ty>(first, last);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(Ty (&array)[N]) {
        return array_view<Ty>(array);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(std::vector<Ty> &vector) {
        return array_view<Ty>(vector);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const std::vector<Ty> &vector) {
        return array_view<Ty>(vector);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(std::array<Ty, N> &array) {
        return array_view<Ty>(array);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const std::array<Ty, N> &array) {
        return array_view<Ty>(array);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(array<Ty, N> &array) {
        return array_view<Ty>(array);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const array<Ty, N> &array) {
        return array_view<Ty>(array);
    }
}

#endif
