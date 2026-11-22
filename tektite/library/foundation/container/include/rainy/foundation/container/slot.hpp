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
#ifndef RAINY_FOUNDATION_CONTAINER_SLOT_HPP
#define RAINY_FOUNDATION_CONTAINER_SLOT_HPP

#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>

namespace rainy::foundation::container {
    /**
     * \lang english
     * @brief A single nullable slot holding at most one value.
     *
     * @brief It models an optional-style storage cell with explicit empty and
     * occupied states: a slot can be filled by emplace, inspected through
     * operator*, operator->, value and value_or, and emptied by reset. Unlike
     * optional, a slot is always a distinct storage location whose occupancy
     * can be queried with has_value.
     *
     * @tparam Ty The value type stored in the slot.
     *
     * \lang simp-chinese
     * @brief 最多持有一个值的单值可空槽。
     *
     * @brief 它建模一个 optional 风格的存储单元，具有明确的空/占用两态：槽可由 emplace
     * 填充，经 operator*、operator->、value 与 value_or 访问，并由 reset 清空。与
     * optional 不同，槽始终是一个独立存储位置，其占用状态可由 has_value 查询。
     *
     * @tparam Ty 槽中所存的值类型。
     */
    template <typename Ty>
    class slot {
    public:
        using value_type = Ty;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        RAINY_CONSTEXPR20 slot() noexcept : occupied_(false) {
        }

        RAINY_CONSTEXPR20 slot(const value_type &value) : occupied_(false) {
            construct(value);
        }

        RAINY_CONSTEXPR20 slot(value_type &&value) : occupied_(false) {
            construct(utility::move(value));
        }

        RAINY_CONSTEXPR20 slot(const slot &other) : occupied_(false) {
            if (other.occupied_) {
                construct(other.get());
            }
        }

        RAINY_CONSTEXPR20 slot(slot &&other) noexcept(type_traits::properties::is_nothrow_move_constructible_v<value_type>) :
            occupied_(false) {
            if (other.occupied_) {
                construct(utility::move(other.get()));
                other.reset();
            }
        }

        RAINY_CONSTEXPR20 ~slot() {
            reset();
        }

        RAINY_CONSTEXPR20 slot &operator=(const slot &other) {
            if (this != utility::addressof(other)) {
                if (other.occupied_) {
                    assign(other.get());
                } else {
                    reset();
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 slot &operator=(slot &&other) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<value_type> &&
            type_traits::properties::is_nothrow_move_assignable_v<value_type>) {
            if (this != utility::addressof(other)) {
                if (other.occupied_) {
                    assign(utility::move(other.get()));
                    other.reset();
                } else {
                    reset();
                }
            }
            return *this;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool has_value() const noexcept {
            return occupied_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 explicit operator bool() const noexcept {
            return occupied_;
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 reference emplace(Args &&...args) {
            reset();
            ::new (static_cast<void *>(data())) value_type(utility::forward<Args>(args)...);
            occupied_ = true;
            return get();
        }

        RAINY_CONSTEXPR20 void reset() noexcept {
            if (occupied_) {
                get().~value_type();
                occupied_ = false;
            }
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 reference get() noexcept {
            return *data();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const_reference get() const noexcept {
            return *data();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 reference operator*() noexcept {
            return get();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            return get();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer operator->() noexcept {
            return data();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            return data();
        }

        RAINY_NODISCARD reference value() {
            if (!occupied_) {
                core::exceptions::logic::throw_out_of_range("slot::value: slot is empty");
            }
            return get();
        }

        RAINY_NODISCARD const_reference value() const {
            if (!occupied_) {
                core::exceptions::logic::throw_out_of_range("slot::value: slot is empty");
            }
            return get();
        }

        template <typename Uty>
        RAINY_NODISCARD RAINY_CONSTEXPR20 value_type value_or(Uty &&fallback) const {
            return occupied_ ? get() : static_cast<value_type>(utility::forward<Uty>(fallback));
        }

        RAINY_CONSTEXPR20 void swap(slot &other) noexcept(type_traits::properties::is_nothrow_swappable_v<value_type> &&
                                                           type_traits::properties::is_nothrow_move_constructible_v<value_type>) {
            if (this == utility::addressof(other)) {
                return;
            }
            if (occupied_ && other.occupied_) {
                using std::swap;
                swap(get(), other.get());
            } else if (occupied_) {
                other.construct(utility::move(get()));
                reset();
            } else if (other.occupied_) {
                construct(utility::move(other.get()));
                other.reset();
            }
        }

    private:
        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer data() noexcept {
            return reinterpret_cast<pointer>(utility::addressof(storage_));
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const_pointer data() const noexcept {
            return reinterpret_cast<const_pointer>(utility::addressof(storage_));
        }

        template <typename Uty>
        RAINY_CONSTEXPR20 void construct(Uty &&value) {
            ::new (static_cast<void *>(data())) value_type(utility::forward<Uty>(value));
            occupied_ = true;
        }

        template <typename Uty>
        RAINY_CONSTEXPR20 void assign(Uty &&value) {
            if (occupied_) {
                get() = utility::forward<Uty>(value);
            } else {
                construct(utility::forward<Uty>(value));
            }
        }

        alignas(value_type) unsigned char storage_[sizeof(value_type)];
        bool occupied_;
    };

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(const slot<Ty> &left, const slot<Ty> &right) {
        if (left.has_value() != right.has_value()) {
            return false;
        }
        return !left.has_value() || left.get() == right.get();
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(const slot<Ty> &left, const slot<Ty> &right) {
        return !(left == right);
    }
}

#endif
