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
#ifndef RAINY_CORE_EXPECTED_HPP
#define RAINY_CORE_EXPECTED_HPP
#include <rainy/core/type_traits.hpp>
#include <rainy/core/implements/exceptions.hpp>

namespace rainy::utility {
    struct unexpect_t {
        explicit unexpect_t() = default;
    };

    inline constexpr unexpect_t unexpect{};

    template <typename Err>
    class unexpected;

    template <typename Ty, typename Err>
    class expected;
}

namespace rainy::utility::implements {
    template <typename Err>
    struct check_unexpected_argument : type_traits::helper::true_type {
        static_assert(type_traits::composite_types::is_object_v<Err>, "Err must be an object type");
        static_assert(!type_traits::primary_types::is_array_v<Err>, "Err must not be an array type");
        static_assert(!type_traits::type_properties::is_const_v<Err>, "Err must not be const");
        static_assert(!type_traits::type_properties::is_volatile_v<Err>, "Err must not be volatile");
        static_assert(!type_traits::primary_types::is_specialization_v<Err, unexpected>,
                      "Err must not be a specialization of unexpected.");
    };

    template <typename Ty>
    struct check_expected_argument : type_traits::helper::true_type {
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "T must not be a reference type. (N4950 [expected.object.general]/2)");
        static_assert(!type_traits::primary_types::is_function_v<Ty>, "T must not be a function type. (N4950 [expected.object.general]/2)");
        static_assert(!type_traits::primary_types::is_array_v<Ty>, "T must not be an array type. (N4950 [expected.object.general]/2)");
        static_assert(!type_traits::type_relations:: is_same_v<type_traits::cv_modify::remove_cv_t<Ty>, std:: in_place_t>,
                      "T must not be (possibly cv-qualified) in_place_t. (N4950 [expected.object.general]/2)");
        static_assert(!type_traits::type_relations::is_same_v<type_traits::cv_modify:: remove_cv_t<Ty>, unexpect_t>,
                      "T must not be (possibly cv-qualified) unexpect_t. (N4950 [expected.object.general]/2)");
        static_assert(!type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cv_t<Ty>, unexpected>,
                      "T must not be a (possibly cv-qualified) specialization of unexpected. (N4950 [expected.object.general]/2)");
    };

    template <typename Ty, typename Err, typename UTy, typename UErr>
    static constexpr bool expected_allow_unwrapping =
        type_traits::logical_traits::disjunction_v<
            type_traits::type_relations::is_same<type_traits::cv_modify::remove_cv_t<Ty>, bool>,
            type_traits::logical_traits::negation<type_traits::logical_traits::disjunction<
                type_traits::type_properties::is_constructible<Ty, utility::expected<UTy, UErr> &>, //
                type_traits::type_properties::is_constructible<Ty, utility::expected<UTy, UErr>>, //
                type_traits::type_properties::is_constructible<Ty, const utility::expected<UTy, UErr> &>, //
                type_traits::type_properties::is_constructible<Ty, const utility::expected<UTy, UErr>>, //
                type_traits::type_relations::is_convertible<utility::expected<UTy, UErr> &, Ty>, //
                type_traits::type_relations::is_convertible<utility::expected<UTy, UErr> &&, Ty>, //
                type_traits::type_relations::is_convertible<const utility::expected<UTy, UErr> &, Ty>, //
                type_traits::type_relations::is_convertible<const utility::expected<UTy, UErr> &&, Ty>>>> &&
        !type_traits::type_properties::is_constructible_v<utility::unexpected<Err>, utility::expected<UTy, UErr> &> &&
        !type_traits::type_properties::is_constructible_v<utility::unexpected<Err>, utility::expected<UTy, UErr>> &&
        !type_traits::type_properties::is_constructible_v<utility::unexpected<Err>, const utility::expected<UTy, UErr> &> &&
        !type_traits::type_properties::is_constructible_v<utility::unexpected<Err>, const utility::expected<UTy, UErr>>;
}

namespace rainy::utility {
    template <typename Err>
    class unexpected {
    public:
        static_assert(implements::check_unexpected_argument<Err>::value);

        template <typename Ty, typename Err2>
        friend class expected;

        unexpected() = delete;

        template <typename UError,
                  typename type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UError>, unexpected> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UError>, std::in_place_t> &&
                          type_traits::type_properties::is_constructible_v<Err, UError>,
                      int> = 0>
        explicit constexpr unexpected(UError &&unex) noexcept(std::is_nothrow_constructible<Err, UError>::value) :
            unexpected_value(utility::forward<UError>(unex)) {
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Err, Args...>, int> = 0>
        explicit constexpr unexpected(std::in_place_t, Args &&...vals) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Err, Args...>) :
            unexpected_value(utility::forward<Args>(vals)...) {
        }

        template <
            typename Uty, typename... Args,
            std::enable_if_t<type_traits::type_properties::is_constructible_v<Err, std::initializer_list<Uty> &, Args...>, int> = 0>
        explicit constexpr unexpected(std::in_place_t, std::initializer_list<Uty> ilist, Args &&...vals) noexcept(
            std::is_nothrow_constructible<Err, std::initializer_list<Uty> &, Args...>::value) :
            unexpected_value(ilist, utility::forward<Args>(vals)...) {
        }

        constexpr const Err &error() const & noexcept {
            return unexpected_value;
        }
        constexpr Err &error() & noexcept {
            return unexpected_value;
        }
        constexpr const Err &&error() const && noexcept {
            return utility::move(unexpected_value);
        }
        constexpr Err &&error() && noexcept {
            return utility::move(unexpected_value);
        }

        constexpr void swap(unexpected &other) noexcept(type_traits::type_properties::is_nothrow_swappable_v<Err>) {
            static_assert(type_traits::type_properties::is_swappable_v<Err>, "Err must be swappable");
            std::swap(unexpected_value, other.unexpected_value);
        }

        friend constexpr void swap(unexpected &left,
                                   unexpected &right) noexcept(type_traits::type_properties::is_nothrow_swappable_v<Err>) {
            left.swap(right);
        }

        template <typename UErr>
        friend constexpr bool operator==(const unexpected &left,
                                         const unexpected<UErr> &right) noexcept(noexcept(left.error() == right.error())) {
            return left.unexpected_value == right.error();
        }

    private:
        Err unexpected_value;
    };

    template <typename Err>
    unexpected(Err) -> unexpected<Err>;
}

namespace rainy::foundation::exceptions::runtime {
    template <typename Err>
    class bad_expected_access;

    template <>
    class bad_expected_access<void> : runtime_error {
    public:
        bad_expected_access() : runtime_error{} {
        }

        RAINY_NODISCARD const char *what() const noexcept override {
            return "bad expected access";
        }

    protected:
        bad_expected_access(const bad_expected_access &) = default;
        bad_expected_access(bad_expected_access &&) = default;
        bad_expected_access &operator=(const bad_expected_access &) = default;
        bad_expected_access &operator=(bad_expected_access &&) = default;
    };

    template <typename Err>
    class bad_expected_access : bad_expected_access<void> {
    public:
        using base = bad_expected_access<void>;

        explicit bad_expected_access(Err unexpected) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Err>) :
            base(), unexpected(utility::move(unexpected)) {
        }

        RAINY_NODISCARD const Err &error() const & noexcept {
            return unexpected;
        }

        RAINY_NODISCARD Err &error() & noexcept {
            return unexpected;
        }

        RAINY_NODISCARD const Err &&error() const && noexcept {
            return utility::move(unexpected);
        }

        RAINY_NODISCARD Err &&error() && noexcept {
            return utility::move(unexpected);
        }

    private:
        Err unexpected;
    };


}

namespace rainy::utility {
    template <typename Ty, typename Err>
    class expected {
    public:
        static_assert(implements::check_expected_argument<Ty>::value);
        static_assert(implements::check_unexpected_argument<Err>::value);

        template <typename UTy, typename UErr>
        friend class expected;

        using value_type = Ty;
        using error_type = Err;
        using unexpected_type = unexpected<Err>;

        template <typename UTy = Ty,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<UTy>, int> = 0>
        constexpr expected() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) :
            value_(), has_value_{true} {
        }

        constexpr expected(const expected &right) noexcept(type_traits::type_properties::is_nothrow_copy_constructible_v<Ty> &&
                                                 type_traits::type_properties::is_nothrow_copy_constructible_v<Err>) :
            has_value_(right.has_value_) {
            static_assert(type_traits::type_properties::is_copy_constructible<Ty>::value, "Ty must be copy constructible");
            static_assert(type_traits::type_properties::is_copy_constructible<Err>::value, "Err must be copy constructible");
            if (has_value_) {
                utility::construct_at(utility::addressof(value_), right.value_);
            } else {
                utility::construct_at(utility::addressof(unexpected_), right.unexpected_);
            }
        }

        constexpr expected(expected &&right) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Ty> &&
                                                      type_traits::type_properties::is_nothrow_move_constructible_v<Err>) {
            static_assert(type_traits::type_properties::is_move_constructible<Ty>::value, "Ty must be move constructible");
            static_assert(type_traits::type_properties::is_move_constructible<Err>::value, "Err must be move constructible");
            has_value_ = right.has_value_;
            if (has_value_) {
                utility::construct_at(utility::addressof(value_), utility::move(right.value_));
            } else {
                utility::construct_at(utility::addressof(unexpected_), utility::move(right.unexpected_));
            }
        }

        template <typename UTy, typename UErr, type_traits::other_trans::enable_if_t<
            type_traits::type_relations::different_from_v<expected<UTy,UErr>, expected> &&
                                                            type_traits::type_properties::is_constructible_v<Ty, UTy> &&
                                                            implements::expected_allow_unwrapping<Ty, Err, UTy, UErr>
            ,int> = 0>
        constexpr expected(expected<UTy, UErr> &&right) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, UTy> &&
                                                                 type_traits::type_properties::is_nothrow_constructible_v<Err, UErr>) :
            has_value_(right.has_value_) {
            static_assert(type_traits::type_properties::is_constructible_v<Ty, UTy>, "Ty must be constructible from UTy");
            static_assert(type_traits::type_properties::is_constructible_v<Err, UErr>, "Err must be constructible from UErr");
            if (has_value_) {
                utility::construct_at(utility::addressof(value_), utility::move(right.value_));
            } else {
                utility::construct_at(utility::addressof(unexpected_), utility::move(right.unexpected_));
            }
        }

        ~expected() {
        }

    private:
        union {
            value_type value_;
            error_type unexpected_;
            std::max_align_t dummy{};
        };
        bool has_value_;
    };
}

#endif