/*
 * Copyright 2026 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RING_BUFFER_ITERATOR_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RING_BUFFER_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Random-access iterator over a circular sequence.
     *
     * @tparam BufferPtr Pointer to a (possibly const) element.
     * @tparam Value The element type exposed to users.
     *
     * \lang simp-chinese
     * @brief 环形序列上的随机访问迭代器。
     *
     * @tparam BufferPtr 指向（可能为 const 的）元素的指针。
     * @tparam Value 暴露给用户的元素类型。
     */
    template <typename BufferPtr, typename Value>
    class ring_iterator {
        template <typename, typename>
        friend class ring_iterator;

        using bare = typename std::remove_pointer<BufferPtr>::type;
        static constexpr bool is_const = type_traits::properties::is_const_v<bare>;

    public:
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = type_traits::other_trans::conditional_t<is_const, const value_type *, value_type *>;
        using reference = type_traits::other_trans::conditional_t<is_const, const value_type &, value_type &>;
        using iterator_category = std::random_access_iterator_tag;

        RAINY_CONSTEXPR20 ring_iterator() noexcept : data_(nullptr), capacity_(0), head_(0), pos_(0) {
        }

        RAINY_CONSTEXPR20 ring_iterator(const BufferPtr data, const std::size_t capacity, const std::size_t head,
                                        const std::size_t pos) noexcept : data_(data), capacity_(capacity), head_(head), pos_(pos) {
        }

        template <typename OtherPtr, typename OtherValue,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<BufferPtr, OtherPtr>, int> = 0>
        RAINY_CONSTEXPR20 ring_iterator(const ring_iterator<OtherPtr, OtherValue> &other) noexcept :
            data_(other.data_), capacity_(other.capacity_), head_(other.head_), pos_(other.pos_) {
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 reference operator*() const noexcept {
            return data_[physical()];
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer operator->() const noexcept {
            return data_ + physical();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 reference operator[](const difference_type n) const noexcept {
            return *(*this + n);
        }

        RAINY_CONSTEXPR20 ring_iterator &operator++() noexcept {
            ++pos_;
            return *this;
        }

        RAINY_CONSTEXPR20 ring_iterator operator++(int) noexcept {
            ring_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        RAINY_CONSTEXPR20 ring_iterator &operator--() noexcept {
            --pos_;
            return *this;
        }

        RAINY_CONSTEXPR20 ring_iterator operator--(int) noexcept {
            ring_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        RAINY_CONSTEXPR20 ring_iterator &operator+=(const difference_type n) noexcept {
            pos_ = static_cast<std::size_t>(static_cast<difference_type>(pos_) + n);
            return *this;
        }

        RAINY_CONSTEXPR20 ring_iterator &operator-=(const difference_type n) noexcept {
            return (*this += -n);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 ring_iterator operator+(const difference_type n) const noexcept {
            ring_iterator tmp = *this;
            return tmp += n;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 ring_iterator operator-(const difference_type n) const noexcept {
            ring_iterator tmp = *this;
            return tmp -= n;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 difference_type operator-(const ring_iterator &right) const noexcept {
            return static_cast<difference_type>(pos_) - static_cast<difference_type>(right.pos_);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(const ring_iterator &right) const noexcept {
            return data_ == right.data_ && pos_ == right.pos_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(const ring_iterator &right) const noexcept {
            return !(*this == right);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator<(const ring_iterator &right) const noexcept {
            return pos_ < right.pos_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator>(const ring_iterator &right) const noexcept {
            return pos_ > right.pos_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator<=(const ring_iterator &right) const noexcept {
            return pos_ <= right.pos_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator>=(const ring_iterator &right) const noexcept {
            return pos_ >= right.pos_;
        }

    private:
        RAINY_NODISCARD RAINY_CONSTEXPR20 std::size_t physical() const noexcept {
            return capacity_ == 0 ? 0 : (head_ + pos_) % capacity_;
        }

        BufferPtr data_;
        std::size_t capacity_;
        std::size_t head_;
        std::size_t pos_;
    };
}

#endif
