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
#ifndef RAINY_FOUNDATION_ITERATORS_MOVE_ITERATOR_HPP
#define RAINY_FOUNDATION_ITERATORS_MOVE_ITERATOR_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::iterators {
    template <typename Iter>
    class move_iterator {
    public:
        using iterator_type = Iter;
        using value_type = type_traits::extras::iterators::iter_value_t<Iter>;
        using difference_type = type_traits::extras::iterators::iterator_difference_t<Iter>;
        using pointer = Iter;
        using reference = type_traits::other_trans::conditional_t<
            type_traits::composite_types::is_reference_v<type_traits::extras::iterators::iterator_reference_t<Iter>>,
            type_traits::reference_modify::remove_reference_t<type_traits::extras::iterators::iterator_reference_t<Iter>> &&,
            type_traits::extras::iterators::iterator_reference_t<Iter>>;
        using iterator_category = type_traits::extras::iterators::iterator_category_t<Iter>;

        constexpr move_iterator() = default;

        constexpr explicit move_iterator(Iter right) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<Iter>) // strengthened
            : current(utility::move(right)) {
        }

        template <typename UTy>
        constexpr move_iterator(const move_iterator<UTy> &right) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Iter, const UTy &>) // strengthened
            : current(right.base()) {
        }

        template <typename UTy>
        constexpr move_iterator &operator=(const move_iterator<UTy> &right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Iter &, const UTy &>) {
            current = right.base();
            return *this;
        }

        RAINY_NODISCARD constexpr const iterator_type &base() const & noexcept {
            return current;
        }

        RAINY_NODISCARD constexpr iterator_type base() && noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<Iter>) {
            return utility::move(current);
        }

        RAINY_NODISCARD constexpr reference operator*() const noexcept(noexcept(static_cast<reference>(*current))) {
            return static_cast<reference>(*current);
        }

        constexpr move_iterator &operator++() noexcept(noexcept(++current)) {
            ++current;
            return *this;
        }

        constexpr auto operator++(int) noexcept(type_traits::type_properties::is_nothrow_copy_constructible_v<Iter> //
                                                && noexcept(++current)) {
            if constexpr (type_traits::extras::iterators::is_bidirectional_iterator_v<Iter>) {
                ++current;
            } else {
                move_iterator tmp = *this;
                ++current;
                return tmp;
            }
        }

        constexpr move_iterator &operator--() noexcept(noexcept(--current)) {
            --current;
            return *this;
        }

        constexpr move_iterator operator--(int) noexcept(type_traits::type_properties::is_nothrow_copy_constructible_v<Iter> //
                                                         && noexcept(--current)) {
            move_iterator tmp = *this;
            --current;
            return tmp;
        }

        RAINY_NODISCARD constexpr move_iterator operator+(const difference_type off) const
            noexcept(noexcept(move_iterator(current + off))) {
            return move_iterator(current + off);
        }

        constexpr move_iterator &operator+=(const difference_type off) noexcept(noexcept(current += off)) {
            current += off;
            return *this;
        }

        RAINY_NODISCARD constexpr move_iterator operator-(const difference_type off) const
            noexcept(noexcept(move_iterator(current - off))) {
            return move_iterator(current - off);
        }

        constexpr move_iterator &operator-=(const difference_type off) noexcept(noexcept(current -= off)) {
            current -= off;
            return *this;
        }

        RAINY_NODISCARD constexpr reference operator[](const difference_type off) const
            noexcept(noexcept(utility::move(current[off]))) {
            return utility::move(current[off]);
        }

    private:
        Iter current{};
    };

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right == left; }
#endif
    RAINY_NODISCARD constexpr bool operator==(const move_iterator<Iter1> &left, const move_iterator<Iter2> &right) noexcept(
        noexcept(type_traits::implements::fake_copy_init<bool>(left.base() == right.base()))) {
        return left.base() == right.base();
    }

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right != left; }
#endif
    RAINY_NODISCARD constexpr bool operator!=(const move_iterator<Iter1> &left,
                                              const move_iterator<Iter2> &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right < left; }
#endif
    RAINY_NODISCARD constexpr bool operator<(const move_iterator<Iter1> &left, const move_iterator<Iter2> &right) noexcept(
        noexcept(type_traits::implements::fake_copy_init<bool>(left.base() < right.base()))) {
        return left.base() < right.base();
    }

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right < left; }
#endif
    RAINY_NODISCARD constexpr bool operator>(const move_iterator<Iter1> &left,
                                             const move_iterator<Iter2> &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right < left; }
#endif
    RAINY_NODISCARD constexpr bool operator<=(const move_iterator<Iter1> &left,
                                              const move_iterator<Iter2> &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right < left; }
#endif
    RAINY_NODISCARD constexpr bool operator>=(const move_iterator<Iter1> &left,
                                              const move_iterator<Iter2> &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    template <typename Iter1, typename Iter2>
#if RAINY_HAS_CXX20
        requires requires(const Iter1 &left, const Iter2 &right) { right - left; }
#endif
    RAINY_NODISCARD constexpr rain_fn operator-(const move_iterator<Iter1> &left,
                                             const move_iterator<Iter2> &right) noexcept(noexcept(left.base() - right.base()))
        -> decltype(left.base() - right.base()) {
        return left.base() - right.base();
    }

    template <typename Iter>
    RAINY_NODISCARD constexpr move_iterator<Iter> operator+(
        typename move_iterator<Iter>::difference_type _Off,
        const move_iterator<Iter> &right) noexcept(noexcept(move_iterator<Iter>(right.base() + _Off))) {
        return move_iterator<Iter>(right.base() + _Off);
    }

    template <typename Iter>
    RAINY_NODISCARD constexpr move_iterator<Iter> make_move_iterator(Iter iter) noexcept(
        type_traits::type_properties::is_nothrow_move_constructible_v<Iter>) {
        return move_iterator<Iter>(utility::move(iter));
    }
}

#endif