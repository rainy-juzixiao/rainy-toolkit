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
#ifndef RAINY_FOUNDATION_CONTAINER_NOT_NULL_HPP
#define RAINY_FOUNDATION_CONTAINER_NOT_NULL_HPP

#include <cstddef>
#include <rainy/core/diagnostics/contract.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>

namespace rainy::foundation::container::implements {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_comparable_to_nullptr_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        is_comparable_to_nullptr_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() != nullptr)>> = true;
}

namespace rainy::foundation::container {
    /**
     * \lang english
     * @brief A pointer-like wrapper that may only hold non-null values.
     *
     * @brief It restricts a pointer or pointer-like object so that it can never
     * hold a null value, with zero size overhead over the wrapped type. Only
     * single objects are supported: pointer arithmetic, increment, and indexing
     * are deleted.
     *
     * @tparam Ty The wrapped pointer-like type.
     *
     * \lang simp-chinese
     * @brief 只能持有非空值的类指针包装器。
     *
     * @brief 它约束指针或类指针对象，使其永远无法持有空值，且相对被包装类型无额外
     * 大小开销。仅支持单个对象：指针算术、自增与下标均被删除。
     *
     * @tparam Ty 被包装的类指针类型。
     */
    template <typename Ty>
    class not_null {
    public:
        using element_type = Ty;
        using pointer = Ty;

        static_assert(implements::is_comparable_to_nullptr_v<Ty>, "Ty cannot be compared to nullptr.");

        not_null() = delete;

        not_null(std::nullptr_t) = delete;

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, Ty>, int> = 0>
        RAINY_CONSTEXPR20 not_null(Uty &&value) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, Uty>) :
            ptr_(utility::forward<Uty>(value)) {
            core::diagnostics::contracts::ensures(ptr_ != nullptr, "not_null cannot hold a null value");
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, Ty> &&
                                                                          !type_traits::type_relations::is_same_v<Uty, not_null<Ty>>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 not_null(const not_null<Uty> &other) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, Uty>) :
            ptr_(other.get()) {
            core::diagnostics::contracts::ensures(ptr_ != nullptr, "not_null cannot hold a null value");
        }

        not_null(const not_null &) = default;

        not_null(not_null &&) = default;

        not_null &operator=(const not_null &) = default;

        not_null &operator=(not_null &&) = default;

        not_null &operator=(std::nullptr_t) = delete;

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty get() const noexcept {
            return ptr_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 operator Ty() const noexcept {
            return ptr_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 decltype(auto) operator->() const noexcept {
            return get();
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 decltype(auto) operator*() const noexcept {
            return *get();
        }

        not_null &operator++() = delete;

        not_null &operator--() = delete;

        not_null operator++(int) = delete;

        not_null operator--(int) = delete;

        not_null &operator+=(std::ptrdiff_t) = delete;

        not_null &operator-=(std::ptrdiff_t) = delete;

        void operator[](std::ptrdiff_t) const = delete;

    private:
        Ty ptr_;
    };

    template <typename Ty>
    not_null(Ty) -> not_null<Ty>;

    template <typename Left, typename Right>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(const not_null<Left> &left, const not_null<Right> &right) noexcept {
        return left.get() == right.get();
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(const not_null<Left> &left, const not_null<Right> &right) noexcept {
        return !(left == right);
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(const not_null<Ty> &value, std::nullptr_t) noexcept = delete;

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(std::nullptr_t, const not_null<Ty> &value) noexcept = delete;

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(const not_null<Ty> &value, std::nullptr_t) noexcept = delete;

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(std::nullptr_t, const not_null<Ty> &value) noexcept = delete;
}

#endif
