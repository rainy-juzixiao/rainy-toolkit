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

        constexpr reverse_iterator() noexcept = default;
        constexpr reverse_iterator(const reverse_iterator &) noexcept = default;
        constexpr reverse_iterator(reverse_iterator &&) noexcept = default;
        constexpr reverse_iterator &operator=(const reverse_iterator &) noexcept = default;
        constexpr reverse_iterator &operator=(reverse_iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 ~reverse_iterator() = default;

        explicit constexpr reverse_iterator(iterator_type current) noexcept : current(current) {
        }

        constexpr iterator_type base() const noexcept {
            return current;
        }

        constexpr reference operator*() noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        constexpr const_reference operator*() const noexcept {
            iterator_type tmp = current;
            return *--tmp;
        }

        constexpr pointer operator->() noexcept {
            iterator_type tmp = current;
            --tmp;
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return tmp;
            } else {
                return tmp.operator->();
            }
        }

        constexpr const_pointer operator->() const noexcept {
            iterator_type tmp = current;
            --tmp;
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return tmp;
            } else {
                return tmp.operator->();
            }
        }

        constexpr reverse_iterator &operator++() noexcept {
            --current;
            return *this;
        }

        constexpr reverse_iterator operator++(int) noexcept {
            reverse_iterator tmp = *this;
            --current;
            return tmp;
        }

        constexpr reverse_iterator &operator--() noexcept {
            ++current;
            return *this;
        }

        constexpr reverse_iterator operator--(int) noexcept {
            reverse_iterator tmp = *this;
            ++current;
            return tmp;
        }

        constexpr reverse_iterator &operator+=(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        constexpr reverse_iterator &operator-=(difference_type n) noexcept {
            current += n;
            return *this;
        }

        constexpr reference operator[](difference_type n) noexcept {
            return current[-n - 1];
        }

        constexpr const_reference operator[](difference_type n) const noexcept {
            return current[-n - 1];
        }

        constexpr explicit operator bool() const noexcept {
            return current != nullptr;
        }

        friend constexpr reverse_iterator operator+(const reverse_iterator &it, difference_type n) {
            return reverse_iterator(it.current - n);
        }

        friend constexpr reverse_iterator operator-(const reverse_iterator &it, difference_type n) {
            return reverse_iterator(it.current + n);
        }

        friend constexpr difference_type operator-(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return rhs.current - lhs.current;
        }

        friend constexpr bool operator==(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current == rhs.current;
        }

        friend constexpr bool operator!=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current != rhs.current;
        }

        friend constexpr bool operator<(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current < rhs.current;
        }

        friend constexpr bool operator<=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current <= rhs.current;
        }

        friend constexpr bool operator>(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current > rhs.current;
        }

        friend constexpr bool operator>=(const reverse_iterator &lhs, const reverse_iterator &rhs) {
            return lhs.current >= rhs.current;
        }

    private:
        iterator_type current;
    };
}

#endif
