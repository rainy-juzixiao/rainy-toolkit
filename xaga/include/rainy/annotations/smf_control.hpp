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
#ifndef RAINY_ANNOTATIONS_SMF_CONTROL_HPP
#define RAINY_ANNOTATIONS_SMF_CONTROL_HPP
#include <rainy/core/core.hpp>

namespace rainy::annotations::smf_control {
    template <typename Derived>
    struct constructible_base {
    protected:
        template <typename Ty>
        static constexpr void construct_from(Ty &&other) {
            static_cast<Derived *>(this)->construct_impl_(utility::forward<Ty>(other));
        }
    };

    template <typename Derived>
    struct assignable_base {
    protected:
        template <typename Ty>
        static constexpr void assign_from(Ty &&other) {
            static_cast<Derived *>(this)->assign_impl_(utility::forward<Ty>(other));
        }
    };

    template <typename Base>
    struct non_trivial_copy : Base {
        using Base::Base;

        non_trivial_copy() = default;

        constexpr non_trivial_copy(const non_trivial_copy &other) noexcept(
            noexcept(Base::construct_from(static_cast<const Base &>(other)))) : Base(static_cast<const Base &>(other)) {
            Base::construct_from(static_cast<const Base &>(other));
        }

        non_trivial_copy(non_trivial_copy &&) = default;
        non_trivial_copy &operator=(const non_trivial_copy &) = default;
        non_trivial_copy &operator=(non_trivial_copy &&) = default;
    };

    template <typename Base>
    struct deleted_copy : Base {
        using Base::Base;

        deleted_copy() = default;
        deleted_copy(const deleted_copy &) = delete;
        deleted_copy(deleted_copy &&) = default;
        deleted_copy &operator=(const deleted_copy &) = default;
        deleted_copy &operator=(deleted_copy &&) = default;
    };

    template <typename Base, typename... Types>
    using copy_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_copy_constructible<Types>...>, Base,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_copy_constructible<Types>...>,
                                                non_trivial_copy<Base>, deleted_copy<Base>>>;

    template <typename Base, typename... Types>
    struct non_trivial_move : copy_control<Base, Types...> {
        using MyBase = copy_control<Base, Types...>;
        using MyBase::MyBase;

        non_trivial_move() = default;
        non_trivial_move(const non_trivial_move &) = default;

        constexpr non_trivial_move(non_trivial_move &&other) noexcept(noexcept(MyBase::construct_from(static_cast<Base &&>(other)))) :
            MyBase(static_cast<Base &&>(other)) {
            MyBase::construct_from(static_cast<Base &&>(other));
        }

        non_trivial_move &operator=(const non_trivial_move &) = default;
        non_trivial_move &operator=(non_trivial_move &&) = default;
    };

    template <typename Base, typename... Types>
    struct deleted_move : copy_control<Base, Types...> {
        using MyBase = copy_control<Base, Types...>;
        using MyBase::MyBase;

        deleted_move() = default;
        deleted_move(const deleted_move &) = default;
        deleted_move(deleted_move &&) = delete;
        deleted_move &operator=(const deleted_move &) = default;
        deleted_move &operator=(deleted_move &&) = default;
    };

    template <typename Base, typename... Types>
    using move_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_move_constructible<Types>...>,
        copy_control<Base, Types...>, // Use copy control if all types are trivially move constructible
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_move_constructible<Types>...>,
            non_trivial_move<Base, Types...>, // Use non-trivial move if all are move constructible
            deleted_move<Base, Types...> // Delete move if any type is not move constructible
            >>;

    template <typename Base, typename... Types>
    struct non_trivial_copy_assign : move_control<Base, Types...> {
        using MyBase = move_control<Base, Types...>;
        using MyBase::MyBase;

        non_trivial_copy_assign() = default;
        non_trivial_copy_assign(const non_trivial_copy_assign &) = default;
        non_trivial_copy_assign(non_trivial_copy_assign &&) = default;

        constexpr non_trivial_copy_assign &operator=(const non_trivial_copy_assign &other) noexcept(
            noexcept(MyBase::assign_from(static_cast<const Base &>(other)))) {
            MyBase::assign_from(static_cast<const Base &>(other));
            return *this;
        }

        non_trivial_copy_assign &operator=(non_trivial_copy_assign &&) = default;
    };

    template <typename Base, typename... Types>
    struct deleted_copy_assign : move_control<Base, Types...> {
        using MyBase = move_control<Base, Types...>;
        using MyBase::MyBase;

        deleted_copy_assign() = default;
        deleted_copy_assign(const deleted_copy_assign &) = default;
        deleted_copy_assign(deleted_copy_assign &&) = default;
        deleted_copy_assign &operator=(const deleted_copy_assign &) = delete;
        deleted_copy_assign &operator=(deleted_copy_assign &&) = default;
    };

    template <typename Base, typename... Types>
    using copy_assign_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_destructible<Types>...,
                                                   type_traits::type_properties::is_trivially_copy_constructible<Types>...,
                                                   type_traits::type_properties::is_trivially_copy_assignable<Types>...>,
        move_control<Base, Types...>,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_copy_constructible<Types>...,
                                                       type_traits::type_properties::is_copy_assignable<Types>...>,
            non_trivial_copy_assign<Base, Types...>,
            deleted_copy_assign<Base, Types...>
            >>;

    template <typename Base, typename... Types>
    struct non_trivial_move_assign : copy_assign_control<Base, Types...> {
        using MyBase = copy_assign_control<Base, Types...>;
        using MyBase::MyBase;

        non_trivial_move_assign() = default;
        non_trivial_move_assign(const non_trivial_move_assign &) = default;
        non_trivial_move_assign(non_trivial_move_assign &&) = default;
        non_trivial_move_assign &operator=(const non_trivial_move_assign &) = default;

        constexpr non_trivial_move_assign &operator=(non_trivial_move_assign &&other) noexcept(
            noexcept(MyBase::assign_from(static_cast<Base &&>(other)))) {
            MyBase::assign_from(static_cast<Base &&>(other));
            return *this;
        }
    };

    template <typename Base, typename... Types>
    struct deleted_move_assign : copy_assign_control<Base, Types...> {
        using MyBase = copy_assign_control<Base, Types...>;
        using MyBase::MyBase;

        deleted_move_assign() = default;
        deleted_move_assign(const deleted_move_assign &) = default;
        deleted_move_assign(deleted_move_assign &&) = default;
        deleted_move_assign &operator=(const deleted_move_assign &) = default;
        deleted_move_assign &operator=(deleted_move_assign &&) = delete;
    };

    template <typename Base, typename... Types>
    using move_assign_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_destructible<Types>...,
                                                   type_traits::type_properties::is_trivially_move_constructible<Types>...,
                                                   type_traits::type_properties::is_trivially_move_assignable<Types>...>,
        copy_assign_control<Base, Types...>,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_move_constructible<Types>...,
                                                       type_traits::type_properties::is_move_assignable<Types>...>,
            non_trivial_move_assign<Base, Types...>, deleted_move_assign<Base, Types...>>>;

    template <typename Base, typename... Types>
    using control = move_assign_control<Base, Types...>;
}

#endif
