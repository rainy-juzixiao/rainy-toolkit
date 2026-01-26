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
    struct array_view {
        using value_type = Ty;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;
        using iterator = value_type *;
        using const_iterator = value_type const *;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        constexpr array_view() noexcept = default;

        constexpr array_view(pointer data, size_type size) noexcept : data_(data), size_(size) {
        }

        constexpr array_view(const_pointer data, size_type size) noexcept : data_(const_cast<pointer>(data)), size_(size) {
        }

        constexpr array_view(const_pointer first, const_pointer last) noexcept : data_(first), size_(last - first) {
        }

        constexpr array_view(std::initializer_list<value_type> value) noexcept : data_(const_cast<pointer>(value.begin())), size_(value.size()) {
        }

        template <typename C, size_type N>
        constexpr array_view(C (&value)[N]) noexcept : array_view(value, N) {
        }

        template <typename C>
        RAINY_CONSTEXPR20 array_view(std::vector<C> &value) noexcept : array_view(data(value), value.size()) {
        }

        template <typename C>
        RAINY_CONSTEXPR20 array_view(std::vector<C> const &value) noexcept :
            array_view(data(value), value.size()) {
        }

        template <typename C, std::size_t N>
        constexpr array_view(std::array<C, N> &value) noexcept : array_view(value.data(), value.size()) {
        }

        template <typename C, std::size_t N>
        constexpr array_view(std::array<C, N> const &value) noexcept : array_view(value.data(), value.size()) {
        }

        template <typename C, std::size_t N>
        constexpr array_view(collections::array<C, N> &value) noexcept : array_view(value.data(), N) {
        }

        template <typename C, std::size_t N>
        constexpr array_view(collections::array<C, N> const &value) noexcept : array_view(value.data(), N) {
        }

        template <typename OtherType, type_traits::other_trans::enable_if_t<
                                          type_traits::type_relations::is_convertible_v<OtherType (*)[], Ty (*)[]>, int> = 0>
        constexpr array_view(array_view<OtherType> const &other) noexcept : array_view(other.data(), other.size()) {
        }

        constexpr reference operator[](size_type const pos) noexcept {
            assert(pos < size());
            return data_[pos];
        }

        constexpr const_reference operator[](size_type const pos) const noexcept {
            assert(pos < size());
            return data_[pos];
        }

        constexpr reference at(size_type const pos) noexcept {
            if (size() <= pos) {
                foundation::exceptions::logic::throw_out_of_range("Invalid Subscript");
            }
            return data_[pos];
        }

        constexpr const_reference at(size_type const pos) const {
            if (size() <= pos) {
                foundation::exceptions::logic::throw_out_of_range("Invalid Subscript");
            }
            return data_[pos];
        }

        constexpr reference front() noexcept {
            assert(size_ > 0);
            return *data_;
        }

        constexpr const_reference front() const noexcept {
            assert(size_ > 0);
            return *data_;
        }

        constexpr reference back() noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        constexpr const_reference back() const noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        constexpr pointer data() const noexcept {
            return data_;
        }

        constexpr iterator begin() noexcept {
            return data_;
        }

        constexpr const_iterator begin() const noexcept {
            return data_;
        }

        constexpr const_iterator cbegin() const noexcept {
            return data_;
        }

        constexpr iterator end() noexcept {
            return data_ + size_;
        }

        constexpr const_iterator end() const noexcept {
            return data_ + size_;
        }

        constexpr const_iterator cend() const noexcept {
            return data_ + size_;
        }

        constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        constexpr const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        constexpr const_reverse_iterator crend() const noexcept {
            return rend();
        }

        constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        constexpr size_type size() const noexcept {
            return size_;
        }

    private:
        template <typename C>
        RAINY_CONSTEXPR20 auto data(std::vector<C> const &value) noexcept {
            static_assert(!type_traits::type_relations::is_same_v<C, bool>,
                          "Cannot use std::vector<bool> as an array_view. Consider std::array or std::unique_ptr<bool[]>.");
            return value.data();
        }

        template <typename C>
        RAINY_CONSTEXPR20 auto data(std::vector<C> &value) noexcept {
            static_assert(!type_traits::type_relations::is_same_v<C, bool>,
                          "Cannot use std::vector<bool> as an array_view. Consider std::array or std::unique_ptr<bool[]>.");
            return value.data();
        }

        pointer data_{nullptr};
        size_type size_{0};
    };

    template <typename C, std::size_t N>
    array_view(C (&value)[N]) -> array_view<C>;
    template <typename C>
    array_view(std::vector<C> &value) -> array_view<C>;
    template <typename C>
    array_view(std::vector<C> const &value) -> array_view<C const>;
    template <typename C, std::size_t N>
    array_view(std::array<C, N> &value) -> array_view<C>;
    template <typename C, std::size_t N>
    array_view(std::array<C, N> const &value) -> array_view<C const>;

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
