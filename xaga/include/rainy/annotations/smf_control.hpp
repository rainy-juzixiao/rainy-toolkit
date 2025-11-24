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
/**
 * @file smf_control.hpp
 * @brief 提供基于类型性质自动控制特殊成员函数生成方式的注解工具。
 * @brief
 * 
 * 这些模板用于根据类型的可复制性、可移动性及其平凡性，为派生类选择合适的
 * 构造、复制、移动及赋值行为。
 */
#ifndef RAINY_ANNOTATIONS_SMF_CONTROL_HPP
#define RAINY_ANNOTATIONS_SMF_CONTROL_HPP
#include <rainy/core/core.hpp>

namespace rainy::annotations::smf_control {
    /**
     * @brief 为派生类提供 construct_from 转发接口的基类。
     *
     * @tparam Derived 派生类型，需要实现 construct_impl_。签名如下：
     * template <typename Ty> 
     * void construct_impl_(Ty&& other);
     */
    template <typename Derived>
    struct constructible_base {
    protected:
        template <typename Ty>
        constexpr void construct_from(Ty &&other) {
            static_cast<Derived *>(this)->construct_impl_(utility::forward<Ty>(other));
        }
    };

    /**
     * @brief 为派生类提供 assign_from 转发接口的基类。
     *
     * @tparam Derived 派生类型，需要实现 assign_impl_。
     * template <typename Ty> 
     * void assign_impl_(Ty&& other);
     */
    template <typename Derived>
    struct assignable_base {
    protected:
        template <typename Ty>
        constexpr void assign_from(Ty &&other) {
            static_cast<Derived *>(this)->assign_impl_(utility::forward<Ty>(other));
        }
    };

    /**
     * @brief 用于处理非平凡复制构造的包装类型。
     *
     * 若被包装类型需要显式复制行为，则会在复制构造中调用 Base::construct_from。
     *
     * @tparam Base 基类。
     */
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

    /**
     * @brief 显式删除复制构造的包装类型。
     *
     * @tparam Base 基类。
     */
    template <typename Base>
    struct deleted_copy : Base {
        using Base::Base;

        deleted_copy() = default;
        deleted_copy(const deleted_copy &) = delete;
        deleted_copy(deleted_copy &&) = default;
        deleted_copy &operator=(const deleted_copy &) = default;
        deleted_copy &operator=(deleted_copy &&) = default;
    };

    /**
     * @brief 根据类型特性选择复制控制策略。
     *
     * - 若所有类型可平凡复制，使用 Base。
     * - 若所有类型可复制但非平凡，使用 non_trivial_copy。
     * - 否则删除复制行为。
     *
     * @tparam Base 基类。
     * @tparam Types 用于 traits 判断的类型。
     */
    template <typename Base, typename... Types>
    using copy_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_copy_constructible<Types>...>, Base,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_copy_constructible<Types>...>,
                                                non_trivial_copy<Base>, deleted_copy<Base>>>;
    
    /**
     * @brief 非平凡移动构造控制。
     *
     * 若类型需要显式移动构造，则调用 Base::construct_from。
     *
     * @tparam Base 基类。
     * @tparam Types traits 检测类型。
     */
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

    /**
     * @brief 删除移动构造的包装类型。
     *
     * @tparam Base 基类。
     */
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

    /**
     * @brief 根据类型特性选择移动构造策略。
     *
     * @tparam Base 基类。
     * @tparam Types 类型参数。
     */
    template <typename Base, typename... Types>
    using move_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_move_constructible<Types>...>,
        copy_control<Base, Types...>, // Use copy control if all types are trivially move constructible
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_move_constructible<Types>...>,
            non_trivial_move<Base, Types...>, // Use non-trivial move if all are move constructible
            deleted_move<Base, Types...> // Delete move if any type is not move constructible
            >>;
    
    /**
     * @brief 非平凡复制赋值控制。
     *
     * 在复制赋值时调用 Base::assign_from。
     */
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

    /**
     * @brief 删除复制赋值操作的包装类型。
     */
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

    /**
     * @brief 根据类型特性选择复制赋值策略。
     */
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
    
    /**
     * @brief 非平凡移动赋值操作控制。
     */
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

    /**
     * @brief 删除移动赋值操作的包装类型。
     */
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

    /**
     * @brief 最终统一的特殊成员函数控制选择器。
     *
     * 通过层层控制选择复制、移动、赋值的具体策略。
     */
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
    
    /**
     * @brief 最终统一的控制类型别名。
     *
     * @tparam Base 基类。
     * @tparam Types 特性检测类型。
     */
    template <typename Base, typename... Types>
    using control = move_assign_control<Base, Types...>;
}

#endif
