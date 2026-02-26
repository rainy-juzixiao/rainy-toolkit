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
#ifndef RAINY_FOUNDATION_CONTAINER_OPTIONAL_HPP
#define RAINY_FOUNDATION_CONTAINER_OPTIONAL_HPP
#include <optional>
#include <rainy/annotations/smf_control.hpp>
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

namespace rainy::foundation::container {
    using std::nullopt;
    using std::nullopt_t;

    template <typename Ty>
    class optional;
}

namespace rainy::foundation::exceptions::runtime {
    class bad_optional_access final : public runtime_error {
    public:
        using base = runtime_error;

        explicit bad_optional_access(const source &location = source::current()) :
            base(location.to_string() + " : bad variant access : ") {
        }
    };

    // clang-format off

    RAINY_INLINE rain_fn throw_bad_optional_access(const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw_exception(bad_optional_access{location});
    }

    // clang-format on
}

namespace rainy::foundation::container::implements {
    template <typename Ty, bool = type_traits::type_properties::is_trivially_destructible_v<Ty>>
    struct optional_destruct_base {
        constexpr optional_destruct_base() : dummy{}, has_value_{false} { // NOLINT
        }

        template <typename... Args>
        constexpr optional_destruct_base(std::in_place_t, Args &&...args) : value_(utility::forward<Args>(args)...), has_value_{true} {
        }

        optional_destruct_base(const optional_destruct_base &) = default;
        optional_destruct_base(optional_destruct_base &&) = default;
        optional_destruct_base &operator=(const optional_destruct_base &) = default;
        optional_destruct_base &operator=(optional_destruct_base &&) = default;

        constexpr void reset() noexcept {
            this->has_value_ = false;
        }

        union {
            std::in_place_t dummy;
            type_traits::cv_modify::remove_cv_t<Ty> value_;
        };
        bool has_value_;
    };

    template <typename Ty>
    struct optional_destruct_base<Ty, false> {
        optional_destruct_base() : has_value_{false} {
        }

        template <typename... Args>
        constexpr optional_destruct_base(std::in_place_t, Args &&...args) : value_(utility::forward<Args>(args)...), has_value_{true} {
        }

        RAINY_CONSTEXPR20 ~optional_destruct_base() {
            this->reset();
        }

        RAINY_CONSTEXPR20 optional_destruct_base(const optional_destruct_base &other) : has_value_{false} {
            if (other.has_value_) {
                utility::construct_in_place(value_, other.value_);
                has_value_ = true;
            }
        }

        RAINY_CONSTEXPR20 optional_destruct_base(optional_destruct_base &&other) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<Ty>) : has_value_{false} {
            if (other.has_value_) {
                utility::construct_in_place(value_, utility::move(other.value_));
                has_value_ = true;
            }
        }

        RAINY_CONSTEXPR20 optional_destruct_base &operator=(const optional_destruct_base &other) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<Ty> &&
            type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>) {
            if (this != &other) {
                if (other.has_value_) {
                    if (has_value_) {
                        value_ = other.value_;
                    } else {
                        utility::construct_in_place(value_, other.value_);
                        has_value_ = true;
                    }
                } else {
                    reset();
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 optional_destruct_base &operator=(optional_destruct_base &&other) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<Ty> &&
            type_traits::type_properties::is_nothrow_move_assignable_v<Ty>) {
            if (this != &other) {
                if (other.has_value_) {
                    if (has_value_) {
                        value_ = utility::move(other.value_);
                    } else {
                        utility::construct_in_place(value_, utility::move(other.value_));
                        has_value_ = true;
                    }
                } else {
                    reset();
                }
            }
            return *this;
        }

        constexpr void reset() noexcept {
            if (this->has_value_) {
                value_.~Ty();
            }
            this->has_value_ = false;
        }

        union {
            std::in_place_t dummy;
            type_traits::cv_modify::remove_cv_t<Ty> value_;
        };
        bool has_value_;
    };

    template <typename Ty>
    struct optional_base : optional_destruct_base<Ty>,
                           annotations::smf_control::constructible_base<optional_base<Ty>>,
                           annotations::smf_control::assignable_base<optional_base<Ty>> {
        using optional_destruct_base<Ty>::optional_destruct_base;

        template <typename UTy>
        using allow_direct_conversion = type_traits::helper::bool_constant<
            !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<UTy>, optional<Ty>> &&
            !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<UTy>, std::in_place_t> &&
            !(type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, bool> &&
              type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cvref_t<UTy>, optional>) &&
            type_traits::type_properties::is_constructible_v<Ty, UTy>>;

        template <typename UTy>
        using allow_unwrapping_assignment = type_traits::helper::bool_constant<!type_traits::logical_traits::disjunction_v<
            type_traits::type_relations::is_same<Ty, UTy>, type_traits::type_properties::is_assignable<Ty &, optional<UTy> &>,
            type_traits::type_properties::is_assignable<Ty &, const optional<UTy> &>,
            type_traits::type_properties::is_assignable<Ty &, const optional<UTy>>,
            type_traits::type_properties::is_assignable<Ty &, optional<UTy>>>>;

        template <typename UTy>
        using allow_unwrapping = type_traits::helper::bool_constant<type_traits::logical_traits::disjunction_v<
            type_traits::type_relations::is_same<type_traits::cv_modify::remove_cv_t<Ty>, bool>,
            type_traits::logical_traits::negation<type_traits::logical_traits::disjunction<
                type_traits::type_relations::is_same<Ty, UTy>, type_traits::type_properties::is_constructible<Ty, optional<UTy> &>,
                type_traits::type_properties::is_constructible<Ty, const optional<UTy> &>,
                type_traits::type_properties::is_constructible<Ty, const optional<UTy>>,
                type_traits::type_properties::is_constructible<Ty, optional<UTy>>,
                type_traits::type_relations::is_convertible<optional<UTy> &, Ty>,
                type_traits::type_relations::is_convertible<const optional<UTy> &, Ty>,
                type_traits::type_relations::is_convertible<const optional<UTy>, Ty>,
                type_traits::type_relations::is_convertible<optional<UTy>, Ty>>>>>;

        template <typename UTy>
        using allow_assignment = type_traits::helper::bool_constant<
            !type_traits::type_relations::is_same_v<optional<Ty>, type_traits::cv_modify::remove_cvref_t<UTy>> &&
            !type_traits::logical_traits::conjunction_v<
                type_traits::composite_types::is_scalar<Ty>,
                type_traits::type_relations::is_same<Ty, type_traits::other_trans::decay_t<UTy>>> &&
            type_traits::type_properties::is_constructible_v<Ty, UTy> && type_traits::type_properties::is_assignable_v<Ty &, UTy>>;

        template <typename Self>
        RAINY_CONSTEXPR20 void construct_impl_(Self &&right) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, decltype(*utility::forward<Self>(right))>) {
            if (right.has_value_) {
                construct_value_(*utility::forward<Self>(right));
            }
        }

        template <typename Self>
        RAINY_CONSTEXPR20 void assign_impl_(Self &&right) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, decltype(*utility::forward<Self>(right))> &&
            type_traits::type_properties::is_nothrow_assignable_v<Ty &, decltype(*utility::forward<Self>(right))>) {
            if (right.has_value_) {
                assign_value_(*utility::forward<Self>(right));
            } else {
                this->reset();
            }
        }

        template <typename UTy>
        RAINY_CONSTEXPR20 void assign_value_(UTy &&right) noexcept(type_traits::type_properties::is_nothrow_assignable_v<Ty &, UTy> &&
                                                                   type_traits::type_properties::is_nothrow_constructible_v<Ty, UTy>) {
            if (this->has_value_) {
                static_cast<Ty &>(this->value_) = utility::forward<UTy>(right);
            } else {
                construct_value_(utility::forward<UTy>(right));
            }
        }

        template <typename UTy>
        RAINY_CONSTEXPR20 void construct_value_(UTy &&val) {
            utility::construct_in_place(this->value_, utility::forward<UTy>(val));
            this->has_value_ = true;
        }

        RAINY_NODISCARD constexpr Ty &operator*() & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr const Ty &operator*() const & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr Ty &&operator*() && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        RAINY_NODISCARD constexpr const Ty &&operator*() const && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 Ty &construct_(Args &&...val) {
            this->reset();
            utility::construct_in_place(this->value_, utility::forward<Args>(val)...);
            this->has_value_ = true;
            return this->value_;
        }
    };
}

namespace rainy::foundation::container {
    template <typename Ty>
    class optional final : private annotations::smf_control::control<implements::optional_base<Ty>> {
    public:
        using base = annotations::smf_control::control<implements::optional_base<Ty>>;

        using value_type = Ty;
        using iterator = Ty *;
        using const_iterator = const Ty *;

        template <typename UTy>
        using allow_direct_conversion = typename base::template allow_direct_conversion<UTy>;

        template <typename UTy>
        using allow_unwrapping = typename base::template allow_unwrapping<UTy>;

        template <typename UTy>
        using allow_unwrapping_assignment = typename base::template allow_unwrapping_assignment<UTy>;

        template <typename UTy>
        using allow_assignment = typename base::template allow_assignment<UTy>;

        constexpr optional() noexcept = default;

        constexpr explicit optional(nullopt_t) noexcept {
        }

        template <typename... Args>
        constexpr explicit optional(std::in_place_t, Args &&...args) : base(std::in_place, utility::forward<Args>(args)...) {
        }

        template <typename Elem, typename... Args>
        constexpr explicit optional(std::in_place_t, std::initializer_list<Elem> ilist, Args &&...args) :
            base(std::in_place, ilist, utility::forward<Args>(args)...) {
        }

        template <typename UTy = type_traits::cv_modify::remove_cv_t<Ty>,
                  type_traits::other_trans::enable_if_t<allow_direct_conversion<UTy>::value, int> = 0>
        constexpr optional(UTy &&val) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, UTy>) :
            base(std::in_place, utility::forward<UTy>(val)) {
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<allow_unwrapping<UTy>>, int> = 0>
        explicit RAINY_CONSTEXPR20 optional(const optional<UTy> &right) {
            static_assert(type_traits::type_properties::is_constructible_v<Ty, const UTy &>,
                          "Cannot passing right [type = const optional<UTy>&] to make a copy because is_constructible<Ty, const UTy "
                          "&> results false");
            if (right.has_value()) {
                this->construct_(*right);
            }
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<allow_unwrapping<UTy>>, int> = 0>
        explicit RAINY_CONSTEXPR20 optional(optional<UTy> &&right) {
            static_assert(
                type_traits::type_properties::is_constructible_v<Ty, UTy>,
                "Cannot passing right [type = optional<UTy>&&] to make a move because is_constructible<Ty, UTy> results false");
            if (right.has_value()) {
                this->construct_(utility::move(*right));
            }
        }

        ~optional() = default;

        constexpr bool has_value() const noexcept {
            return this->has_value_;
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<allow_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 void assign(UTy &&right) noexcept(type_traits::type_properties::is_nothrow_assignable_v<Ty &, UTy> &&
                                                            type_traits::type_properties::is_nothrow_constructible_v<Ty, UTy>) {
            if (this->has_value()) {
                static_cast<Ty &>(this->value_) = utility::forward<UTy>(right);
            } else {
                this->construct_(utility::forward<UTy>(right));
            }
        }

        RAINY_CONSTEXPR20 optional &operator=(nullopt_t) noexcept {
            this->reset();
            return *this;
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<allow_unwrapping_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 optional &operator=(const optional<UTy> &right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Ty &, const UTy &> &&
            type_traits::type_properties::is_nothrow_constructible_v<Ty, const UTy &>) /* strengthened */ {
            static_assert(type_traits::type_properties::is_constructible_v<Ty, const UTy &> &&
                              type_traits::type_properties::is_assignable_v<Ty &, const UTy &>,
                          "Cannot passing right [type = const optional<UTy>&] to make a copy because "
                          "type_traits::type_properties::is_constructible_v<Ty, const UTy &> &&"
                          "type_traits::type_properties::is_assignable_v<Ty &, const UTy &> results false");
            if (right) {
                this->assign(*right);
            } else {
                reset();
            }
            return *this;
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<allow_unwrapping_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 optional &operator=(optional<UTy> &&right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Ty &, const UTy &> &&
            type_traits::type_properties::is_nothrow_constructible_v<Ty, const UTy &>) /* strengthened */ {
            static_assert(type_traits::type_properties::is_constructible_v<Ty, UTy> &&
                              type_traits::type_properties::is_assignable_v<Ty &, UTy>,
                          "Cannot passing right [type = optional<UTy>&&] to make a copy because "
                          "type_traits::type_properties::is_constructible_v<Ty, UTy> && "
                          "type_traits::type_properties::is_assignable_v<Ty &, UTy> results false");
            if (right) {
                this->assign(*right);
            } else {
                reset();
            }
            return *this;
        }

        using base::operator*;

        RAINY_NODISCARD constexpr value_type *operator->() {
            if (!has_value()) {
                foundation::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::addressof(this->value_);
        }

        RAINY_NODISCARD constexpr const value_type *operator->() const {
            if (!has_value()) {
                foundation::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::addressof(this->value_);
        }

        RAINY_NODISCARD constexpr Ty &value() & {
            if (!has_value()) {
                foundation::exceptions::runtime::throw_bad_optional_access();
            }
            return this->value_;
        }

        RAINY_NODISCARD constexpr const Ty &value() const & {
            if (!has_value()) {
                foundation::exceptions::runtime::throw_bad_optional_access();
            }
            return this->value_;
        }

        RAINY_NODISCARD constexpr Ty &&value() && {
            if (!has_value()) {
                foundation::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::move(this->value_);
        }

        RAINY_NODISCARD constexpr const Ty &&value() const && {
            if (!has_value()) {
                foundation::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::move(this->value_);
        }

        template <typename UTy = type_traits::cv_modify::remove_cv_t<Ty>>
        RAINY_NODISCARD constexpr type_traits::cv_modify::remove_cv_t<Ty> value_or(UTy &&right) const & {
            static_assert(type_traits::type_relations::is_convertible_v<const Ty &, type_traits::cv_modify::remove_cv_t<Ty>>,
                          "The const overload of optional<T>::value_or requires const T& to be convertible to remove_cv_t<T> ");
            static_assert(type_traits::type_relations::is_convertible_v<UTy, type_traits::cv_modify::remove_cv_t<Ty>>,
                          "optional<T>::value_or(U) requires U to be convertible to remove_cv_t<T> ");
            if (this->has_value()) {
                return static_cast<const Ty &>(this->value_);
            }
            return static_cast<type_traits::cv_modify::remove_cv_t<Ty>>(utility::forward<UTy>(right));
        }

        template <typename UTy = type_traits::cv_modify::remove_cv_t<Ty>>
        RAINY_NODISCARD constexpr type_traits::cv_modify::remove_cv_t<Ty> value_or(UTy &&_Right) && {
            static_assert(type_traits::type_relations::is_convertible_v<Ty, type_traits::cv_modify::remove_cv_t<Ty>>,
                          "The rvalue overload of optional<T>::value_or requires T to be convertible to remove_cv_t<T> ");
            static_assert(type_traits::type_relations::is_convertible_v<UTy, type_traits::cv_modify::remove_cv_t<Ty>>,
                          "optional<T>::value_or(U) requires U to be convertible to remove_cv_t<T> ");
            if (this->has_value()) {
                return static_cast<Ty &&>(this->value_);
            }
            return static_cast<type_traits::cv_modify::remove_cv_t<Ty>>(utility::forward<UTy>(_Right));
        }

        RAINY_CONSTEXPR20 void swap(optional &right) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Ty> &&
                                                              type_traits::type_properties::is_nothrow_swappable_v<Ty>) {

            if constexpr (type_traits::type_properties::is_move_constructible_v<Ty>) {
                static_assert(type_traits::type_properties::is_swappable_v<Ty>, "optional<T>::swap requires T to be swappable");
            } else {
                static_assert(false, "optional<T>::swap requires T to be move constructible");
            }
            const bool this_engaged = this->has_value();
            const bool right_engaged = right.has_value();
            if (!this_engaged && !right_engaged) {
                return;
            }
            if (this_engaged && right_engaged) {
                using std::swap;
                swap(*(*this), *right);
            } else {
                optional &source = this_engaged ? *this : right;
                optional &target = this_engaged ? right : *this;
                target.construct_(utility::move(*source));
                source.reset();
            }
        }

        using base::reset;

        template <typename UTy, type_traits::other_trans::enable_if_t<allow_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 optional &operator=(UTy &&_Right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Ty &, UTy> &&
            type_traits::type_properties::is_nothrow_constructible_v<Ty, UTy>) {
            this->assign(utility::forward<UTy>(_Right));
            return *this;
        }

        explicit operator bool() const noexcept {
            return this->has_value();
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 value_type &emplace(Args &&...args) {
            this->reset();
            return this->construct_(utility::forward<Args>(args)...);
        }

        template <typename Elem, typename... Args>
        RAINY_CONSTEXPR20 value_type &emplace(std::initializer_list<Elem> ilist, Args &&...args) {
            this->reset();
            return this->construct_(ilist, utility::forward<Args>(args)...);
        }

        template <typename Fx>
        constexpr auto and_then(Fx fx) & {
            using result_type = type_traits::type_properties::invoke_result_t<Fx, Ty &>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<U>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), this->value_);
            }
            return result_type{};
        }

        template <typename Fx>
        constexpr auto and_then(Fx &&fx) && {
            using result_type = type_traits::type_properties::invoke_result_t<Fx, Ty &&>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<U>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_));
            }
            return result_type{};
        }

        template <typename Fx>
        constexpr auto and_then(Fx fx) const & {
            using result_type = type_traits::type_properties::invoke_result_t<Fx, const Ty &>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<U>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), this->value_);
            }
            return result_type{};
        }

        template <typename Fx>
        constexpr auto and_then(Fx &&fx) const && {
            using result_type = type_traits::type_properties::invoke_result_t<Fx, const Ty &&>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<U>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_));
            }
            return result_type{};
        }

        template <typename Fx>
        constexpr auto transform(Fx &&fx) & {
            using U = type_traits::type_properties::invoke_result_t<Fx, Ty &>;
            if (has_value()) {
                return optional<U>(utility::invoke(utility::forward<Fx>(fx), this->value_));
            }
            return optional<U>{};
        }

        template <typename Fx>
        constexpr auto transform(Fx &&fx) && {
            using U = type_traits::type_properties::invoke_result_t<Fx, Ty &&>;
            if (has_value()) {
                return optional<U>(utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_)));
            }
            return optional<U>{};
        }

        template <typename Fx>
        constexpr auto transform(Fx &&fx) const & {
            using U = type_traits::type_properties::invoke_result_t<Fx, const Ty &>;
            if (has_value()) {
                return optional<U>(utility::invoke(utility::forward<Fx>(fx), this->value_));
            }
            return optional<U>{};
        }

        template <typename Fx>
        constexpr auto transform(Fx &&fx) const && {
            using U = type_traits::type_properties::invoke_result_t<Fx, const Ty &&>;
            if (has_value()) {
                return optional<U>(utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_)));
            }
            return optional<U>{};
        }

        template <typename Fx>
        constexpr optional or_else(Fx &&fx) && {
            static_assert(type_traits::type_relations::is_same_v<type_traits::type_properties::invoke_result_t<Fx>, optional>,
                          "Fx must return optional<Ty>");
            if (has_value()) {
                return utility::move(*this);
            }
            return utility::invoke(utility::forward<Fx>(fx));
        }

        template <typename Fx>
        constexpr optional or_else(Fx &&fx) const & {
            static_assert(type_traits::type_relations::is_same_v<type_traits::type_properties::invoke_result_t<Fx>, optional>,
                          "Fx must return optional<Ty>");
            if (has_value()) {
                return *this;
            }
            return utility::invoke(utility::forward<Fx>(fx));
        }

        constexpr iterator begin() noexcept {
            return has_value() ? utility::addressof(this->value_) : nullptr;
        }

        constexpr const_iterator begin() const noexcept {
            return has_value() ? utility::addressof(this->value_) : nullptr;
        }

        constexpr iterator end() noexcept {
            return has_value() ? utility::addressof(this->value_) + 1 : nullptr;
        }

        constexpr const_iterator end() const noexcept {
            return has_value() ? utility::addressof(this->value_) + 1 : nullptr;
        }

        constexpr const_iterator cbegin() const noexcept {
            return begin();
        }

        constexpr const_iterator cend() const noexcept {
            return end();
        }
    };

    template <typename T>
    constexpr optional<type_traits::other_trans::decay_t<T>> make_optional(T &&value) {
        return optional<type_traits::other_trans::decay_t<T>>(utility::forward<T>(value));
    }

    template <typename T, typename... Args>
    constexpr optional<T> make_optional(Args &&...args) {
        return optional<T>(std::in_place, utility::forward<Args>(args)...);
    }

    template <typename T, typename U, typename... Args>
    constexpr optional<T> make_optional(std::initializer_list<U> il, Args &&...args) {
        return optional<T>(std::in_place, il, utility::forward<Args>(args)...);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 void swap(optional<Ty> &left, optional<Ty> &right) {
        left.swap(right);
    }
}

namespace rainy::utility {
    using foundation::container::make_optional;
    using foundation::container::nullopt;
    using foundation::container::nullopt_t;
    using foundation::container::optional;
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
