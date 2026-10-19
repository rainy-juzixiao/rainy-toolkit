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
#ifndef RAINY_FOUNDATION_CONTAINER_CELL_HPP
#define RAINY_FOUNDATION_CONTAINER_CELL_HPP

#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>

namespace rainy::foundation::container {
    /**
     * \lang english
     * @brief A mutable memory location with interior mutability.
     *
     * @brief Modeled after Rust's std::cell::Cell. It allows mutating the
     * contained value through a shared reference, at the cost of never handing
     * out a reference to the interior through a shared reference: values are
     * read and written by copy or by move. get and update require a copyable
     * type; take requires a default-constructible type; set, replace, swap and
     * into_inner only move values. Cell is not thread-safe.
     *
     * @tparam Ty The contained value type.
     *
     * \lang simp-chinese
     * @brief 具有内部可变性的可变内存位置。
     *
     * @brief 参照 Rust 的 std::cell::Cell 设计。它允许经由共享引用改写所持有的值，
     * 代价是经共享引用时不交出内部引用：值只能按拷贝或移动读写。get 与 update 要求
     * 类型可拷贝；take 要求类型可默认构造；set、replace、swap 与 into_inner 仅移动值。
     * Cell 非线程安全。
     *
     * @tparam Ty 所持有的值类型。
     */
    template <typename Ty>
    class cell {
    public:
        using value_type = Ty;

        cell() = default;

        RAINY_CONSTEXPR20 cell(const Ty &value) noexcept(type_traits::properties::is_nothrow_copy_constructible_v<Ty>) :
            value_(value) {
        }

        RAINY_CONSTEXPR20 cell(Ty &&value) noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty>) :
            value_(utility::move(value)) {
        }

        cell(const cell &) = delete;

        cell(cell &&) = delete;

        cell &operator=(const cell &) = delete;

        cell &operator=(cell &&) = delete;

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty *as_ptr() noexcept {
            return utility::addressof(value_);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const Ty *as_ptr() const noexcept {
            return utility::addressof(value_);
        }

        RAINY_CONSTEXPR20 void set(const Ty &value) noexcept(type_traits::properties::is_nothrow_copy_assignable_v<Ty>) {
            value_ = value;
        }

        RAINY_CONSTEXPR20 void set(Ty &&value) noexcept(type_traits::properties::is_nothrow_move_assignable_v<Ty>) {
            value_ = utility::move(value);
        }

        RAINY_CONSTEXPR20 Ty replace(Ty &&value) noexcept(type_traits::properties::is_nothrow_move_assignable_v<Ty> &&
                                                          type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            Ty old = utility::move(value_);
            value_ = utility::move(value);
            return old;
        }

        RAINY_CONSTEXPR20 void swap(cell &other) noexcept(type_traits::properties::is_nothrow_swappable_v<Ty>) {
            using std::swap;
            swap(value_, other.value_);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty into_inner() noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            return utility::move(value_);
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get_mut() noexcept {
            return value_;
        }

        template <typename Uty = Ty,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_copy_constructible_v<Uty>, int> = 0>
        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty get() const noexcept(type_traits::properties::is_nothrow_copy_constructible_v<Uty>) {
            return value_;
        }

        template <typename Uty = Ty,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Uty>, int> = 0>
        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty take() noexcept(type_traits::properties::is_nothrow_move_constructible_v<Uty> &&
                                                             type_traits::properties::is_nothrow_default_constructible_v<Uty>) {
            Ty old = utility::move(value_);
            if constexpr (type_traits::properties::is_move_assignable_v<Uty>) {
                value_ = Ty{};
            } else {
                utility::destroy_at(utility::addressof(value_));
                ::new (static_cast<void *>(utility::addressof(value_))) Ty{};
            }
            return old;
        }

        template <typename Fx, typename Uty = Ty,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_copy_constructible_v<Uty>, int> = 0>
        RAINY_CONSTEXPR20 void update(Fx &&func) {
            if constexpr (type_traits::properties::is_move_assignable_v<Uty>) {
                value_ = utility::invoke(utility::forward<Fx>(func), value_);
            } else {
                Ty updated = utility::invoke(utility::forward<Fx>(func), value_);
                utility::destroy_at(utility::addressof(value_));
                ::new (static_cast<void *>(utility::addressof(value_))) Ty(utility::move(updated));
            }
        }

    private:
        Ty value_{};
    };

    template <typename Ty>
    RAINY_CONSTEXPR20 void swap(cell<Ty> &left, cell<Ty> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

#endif
