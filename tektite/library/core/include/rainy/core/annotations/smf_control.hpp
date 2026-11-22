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
/**
 * \lang english
 * @file smf_control.hpp
 * @brief Annotation utilities that automatically control special member function generation based on type properties.
 *
 *  These templates select appropriate construction, copy, move and assignment behaviors for derived classes
 *  according to the copyability, movability and triviality of the types.
 *
 * \lang simp-chinese
 * @brief 提供基于类型性质自动控制特殊成员函数生成方式的注解工具。
 *
 *  这些模板用于根据类型的可复制性、可移动性及其平凡性，为派生类选择合适的
 *  构造、复制、移动及赋值行为。
 */
#ifndef RAINY_CORE_ANNOTATIONS_SMF_CONTROL_HPP
#define RAINY_CORE_ANNOTATIONS_SMF_CONTROL_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/logical.hpp>
#include <rainy/core/type_traits/properties.hpp>

namespace rainy::annotations::smf_control {
    /**
     * \lang english
     * @brief Base class that provides a construct_from forwarding interface for derived classes.
     *
     * @tparam Derived The derived type, which must implement construct_impl_, with the signature:
     *                 template <typename Ty> void construct_impl_(Ty &&other);
     *
     * \lang simp-chinese
     * @brief 为派生类提供 construct_from 转发接口的基类。
     *
     * @tparam Derived 派生类型，需要实现 construct_impl_，签名如下：
     *                 template <typename Ty> void construct_impl_(Ty &&other);
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
     * \lang english
     * @brief Base class that provides an assign_from forwarding interface for derived classes.
     *
     * @tparam Derived The derived type, which must implement assign_impl_, with the signature:
     *                 template <typename Ty> void assign_impl_(Ty &&other);
     *
     * \lang simp-chinese
     * @brief 为派生类提供 assign_from 转发接口的基类。
     *
     * @tparam Derived 派生类型，需要实现 assign_impl_，签名如下：
     *                 template <typename Ty> void assign_impl_(Ty &&other);
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
     * \lang english
     * @brief Wrapper type for handling non-trivial copy construction.
     *
     *  If the wrapped type requires explicit copy behavior, Base::construct_from is invoked in the copy constructor.
     *
     * @tparam Base The base class.
     *
     * \lang simp-chinese
     * @brief 用于处理非平凡复制构造的包装类型。
     *
     *  若被包装类型需要显式复制行为，则会在复制构造中调用 Base::construct_from。
     *
     * @tparam Base 基类。
     */
    template <typename Base>
    struct non_trivial_copy : Base {
        /**
         * \lang english
         * @brief Inherits all constructors from `Base`.
         *
         * \lang simp-chinese
         * @brief 继承 `Base` 的所有构造函数。
         */
        using Base::Base;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        non_trivial_copy() = default;

        /**
         * \lang english
         * @brief User-defined copy constructor.
         *
         * Copies the `Base` subobject, then calls `Base::construct_from` on the
         * copied base to (re)construct the derived state. Declared `noexcept`
         * according to the `noexcept` specification of that `construct_from` call.
         *
         * @param other The object to copy from
         *
         * \lang simp-chinese
         * @brief 用户定义的拷贝构造函数。
         *
         * 先拷贝 `Base` 子对象，再对拷贝后的基类调用 `Base::construct_from`
         * 以（重新）构造派生状态。其 `noexcept` 说明取决于该 `construct_from`
         * 调用是否抛出异常。
         *
         * @param other 要拷贝的对象
         */
        constexpr non_trivial_copy(const non_trivial_copy &other) noexcept(
            noexcept(Base::construct_from(static_cast<const Base &>(other)))) : Base(static_cast<const Base &>(other)) {
            Base::construct_from(static_cast<const Base &>(other));
        }

        /**
         * \lang english
         * @brief Move constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动构造函数（默认）。
         */
        non_trivial_copy(non_trivial_copy &&) = default;

        /**
         * \lang english
         * @brief Copy assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符（默认）。
         */
        non_trivial_copy &operator=(const non_trivial_copy &) = default;

        /**
         * \lang english
         * @brief Move assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符（默认）。
         */
        non_trivial_copy &operator=(non_trivial_copy &&) = default;
    };

    /**
     * \lang english
     * @brief Wrapper type that explicitly deletes copy construction.
     *
     * @tparam Base The base class.
     *
     * \lang simp-chinese
     * @brief 显式删除复制构造的包装类型。
     *
     * @tparam Base 基类。
     */
    template <typename Base>
    struct deleted_copy : Base {
        /**
         * \lang english
         * @brief Inherits all constructors from `Base`.
         *
         * \lang simp-chinese
         * @brief 继承 `Base` 的所有构造函数。
         */
        using Base::Base;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        deleted_copy() = default;

        /**
         * \lang english
         * @brief Deleted copy constructor. Makes the type non-copyable.
         *
         * \lang simp-chinese
         * @brief 被删除的拷贝构造函数。使该类型不可拷贝。
         */
        deleted_copy(const deleted_copy &) = delete;

        /**
         * \lang english
         * @brief Move constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动构造函数（默认）。
         */
        deleted_copy(deleted_copy &&) = default;

        /**
         * \lang english
         * @brief Copy assignment operator (defaulted).
         *
         * Note that copy assignment remains available even though the copy
         * constructor is deleted.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符（默认）。
         *
         * 注意：尽管拷贝构造函数已被删除，拷贝赋值运算符仍然可用。
         */
        deleted_copy &operator=(const deleted_copy &) = default;

        /**
         * \lang english
         * @brief Move assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符（默认）。
         */
        deleted_copy &operator=(deleted_copy &&) = default;
    };

    /**
     * \lang english
     * @brief Selects the copy control strategy according to type properties.
     *
     *  - Uses Base if all types are trivially copyable.
     *  - Uses non_trivial_copy if all types are copyable but non-trivial.
     *  - Otherwise deletes copy behavior.
     *
     * @tparam Base The base class.
     * @tparam Types The types used for the traits checks.
     *
     * \lang simp-chinese
     * @brief 根据类型特性选择复制控制策略。
     *
     *  - 若所有类型可平凡复制，使用 Base。
     *  - 若所有类型可复制但非平凡，使用 non_trivial_copy。
     *  - 否则删除复制行为。
     *
     * @tparam Base 基类。
     * @tparam Types 用于 traits 判断的类型。
     */
    template <typename Base, typename... Types>
    using copy_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_copy_constructible<Types>...>, Base,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_copy_constructible<Types>...>,
            non_trivial_copy<Base>, deleted_copy<Base>>>;

    /**
     * \lang english
     * @brief Non-trivial move construction control.
     *
     *  If the type requires explicit move construction, Base::construct_from is invoked.
     *
     * @tparam Base The base class.
     * @tparam Types The types used for the traits checks.
     *
     * \lang simp-chinese
     * @brief 非平凡移动构造控制。
     *
     *  若类型需要显式移动构造，则调用 Base::construct_from。
     *
     * @tparam Base 基类。
     * @tparam Types traits 检测类型。
     */
    template <typename Base, typename... Types>
    struct non_trivial_move : copy_control<Base, Types...> {
        /**
         * \lang english
         * @brief Alias for the base class `copy_control<Base, Types...>`.
         *
         * \lang simp-chinese
         * @brief 基类 `copy_control<Base, Types...>` 的别名。
         */
        using MyBase = copy_control<Base, Types...>;

        /**
         * \lang english
         * @brief Inherits all constructors from `MyBase`.
         *
         * \lang simp-chinese
         * @brief 继承 `MyBase` 的所有构造函数。
         */
        using MyBase::MyBase;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        non_trivial_move() = default;

        /**
         * \lang english
         * @brief Copy constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数（默认）。
         */
        non_trivial_move(const non_trivial_move &) = default;

        /**
         * \lang english
         * @brief User-defined move constructor.
         *
         * Moves the `Base` subobject, then calls `MyBase::construct_from` on the
         * moved-from base to (re)construct the derived state. Declared `noexcept`
         * according to the `noexcept` specification of that `construct_from` call.
         *
         * @param other The object to move from
         *
         * \lang simp-chinese
         * @brief 用户定义的移动构造函数。
         *
         * 先移动 `Base` 子对象，再对被移走的基类调用 `MyBase::construct_from`
         * 以（重新）构造派生状态。其 `noexcept` 说明取决于该 `construct_from`
         * 调用是否抛出异常。
         *
         * @param other 要移动的对象
         */
        constexpr non_trivial_move(non_trivial_move &&other) noexcept(noexcept(MyBase::construct_from(static_cast<Base &&>(other)))) :
            MyBase(static_cast<Base &&>(other)) {
            MyBase::construct_from(static_cast<Base &&>(other));
        }

        /**
         * \lang english
         * @brief Copy assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符（默认）。
         */
        non_trivial_move &operator=(const non_trivial_move &) = default;

        /**
         * \lang english
         * @brief Move assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符（默认）。
         */
        non_trivial_move &operator=(non_trivial_move &&) = default;
    };

    /**
     * \lang english
     * @brief Wrapper type that deletes move construction.
     *
     * @tparam Base The base class.
     *
     * \lang simp-chinese
     * @brief 删除移动构造的包装类型。
     *
     * @tparam Base 基类。
     */
    template <typename Base, typename... Types>
    struct deleted_move : copy_control<Base, Types...> {
        /**
         * \lang english
         * @brief Alias for the base class `copy_control<Base, Types...>`.
         *
         * \lang simp-chinese
         * @brief 基类 `copy_control<Base, Types...>` 的别名。
         */
        using MyBase = copy_control<Base, Types...>;

        /**
         * \lang english
         * @brief Inherits all constructors from `MyBase`.
         *
         * \lang simp-chinese
         * @brief 继承 `MyBase` 的所有构造函数。
         */
        using MyBase::MyBase;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        deleted_move() = default;

        /**
         * \lang english
         * @brief Copy constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数（默认）。
         */
        deleted_move(const deleted_move &) = default;

        /**
         * \lang english
         * @brief Deleted move constructor. Makes the type non-move-constructible.
         *
         * \lang simp-chinese
         * @brief 被删除的移动构造函数。使该类型不可移动构造。
         */
        deleted_move(deleted_move &&) = delete;

        /**
         * \lang english
         * @brief Copy assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符（默认）。
         */
        deleted_move &operator=(const deleted_move &) = default;

        /**
         * \lang english
         * @brief Move assignment operator (defaulted).
         *
         * Note that move assignment remains available even though the move
         * constructor is deleted.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符（默认）。
         *
         * 注意：尽管移动构造函数已被删除，移动赋值运算符仍然可用。
         */
        deleted_move &operator=(deleted_move &&) = default;
    };

    /**
     * \lang english
     * @brief Selects the move construction strategy according to type properties.
     *
     * @tparam Base The base class.
     * @tparam Types The type parameters.
     *
     * \lang simp-chinese
     * @brief 根据类型特性选择移动构造策略。
     *
     * @tparam Base 基类。
     * @tparam Types 类型参数。
     */
    template <typename Base, typename... Types>
    using move_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_move_constructible<Types>...>,
        copy_control<Base, Types...>, // Use copy control if all types are trivially move constructible
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Types>...>,
            non_trivial_move<Base, Types...>, // Use non-trivial move if all are move constructible
            deleted_move<Base, Types...> // Delete move if any type is not move constructible
            >>;

    /**
     * \lang english
     * @brief Non-trivial copy assignment control.
     *
     *  Calls Base::assign_from during copy assignment.
     *
     * \lang simp-chinese
     * @brief 非平凡复制赋值控制。
     *
     *  在复制赋值时调用 Base::assign_from。
     */
    template <typename Base, typename... Types>
    struct non_trivial_copy_assign : move_control<Base, Types...> {
        /**
         * \lang english
         * @brief Alias for the base class `move_control<Base, Types...>`.
         *
         * \lang simp-chinese
         * @brief 基类 `move_control<Base, Types...>` 的别名。
         */
        using MyBase = move_control<Base, Types...>;

        /**
         * \lang english
         * @brief Inherits all constructors from `MyBase`.
         *
         * \lang simp-chinese
         * @brief 继承 `MyBase` 的所有构造函数。
         */
        using MyBase::MyBase;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        non_trivial_copy_assign() = default;

        /**
         * \lang english
         * @brief Copy constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数（默认）。
         */
        non_trivial_copy_assign(const non_trivial_copy_assign &) = default;

        /**
         * \lang english
         * @brief Move constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动构造函数（默认）。
         */
        non_trivial_copy_assign(non_trivial_copy_assign &&) = default;

        /**
         * \lang english
         * @brief User-defined copy assignment operator.
         *
         * Calls `MyBase::assign_from` on the `Base` subobject of `other` to
         * (re)assign the derived state. Declared `noexcept` according to the
         * `noexcept` specification of that `assign_from` call.
         *
         * @param other The object to copy-assign from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 用户定义的拷贝赋值运算符。
         *
         * 对 `other` 的 `Base` 子对象调用 `MyBase::assign_from`
         * 以（重新）赋值派生状态。其 `noexcept` 说明取决于该 `assign_from`
         * 调用是否抛出异常。
         *
         * @param other 要拷贝赋值的对象
         * @return 此对象的引用
         */
        constexpr non_trivial_copy_assign &operator=(const non_trivial_copy_assign &other) noexcept(
            noexcept(MyBase::assign_from(static_cast<const Base &>(other)))) {
            MyBase::assign_from(static_cast<const Base &>(other));
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符（默认）。
         */
        non_trivial_copy_assign &operator=(non_trivial_copy_assign &&) = default;
    };

    /**
     * \lang english
     * @brief Wrapper type that deletes copy assignment.
     *
     * \lang simp-chinese
     * @brief 删除复制赋值操作的包装类型。
     */
    template <typename Base, typename... Types>
    struct deleted_copy_assign : move_control<Base, Types...> {
        /**
         * \lang english
         * @brief Alias for the base class `move_control<Base, Types...>`.
         *
         * \lang simp-chinese
         * @brief 基类 `move_control<Base, Types...>` 的别名。
         */
        using MyBase = move_control<Base, Types...>;

        /**
         * \lang english
         * @brief Inherits all constructors from `MyBase`.
         *
         * \lang simp-chinese
         * @brief 继承 `MyBase` 的所有构造函数。
         */
        using MyBase::MyBase;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        deleted_copy_assign() = default;

        /**
         * \lang english
         * @brief Copy constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数（默认）。
         */
        deleted_copy_assign(const deleted_copy_assign &) = default;

        /**
         * \lang english
         * @brief Move constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动构造函数（默认）。
         */
        deleted_copy_assign(deleted_copy_assign &&) = default;

        /**
         * \lang english
         * @brief Deleted copy assignment operator. Makes the type non-copy-assignable.
         *
         * Note that the copy constructor remains available even though copy
         * assignment is deleted.
         *
         * \lang simp-chinese
         * @brief 被删除的拷贝赋值运算符。使该类型不可拷贝赋值。
         *
         * 注意：尽管拷贝赋值已被删除，拷贝构造函数仍然可用。
         */
        deleted_copy_assign &operator=(const deleted_copy_assign &) = delete;

        /**
         * \lang english
         * @brief Move assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符（默认）。
         */
        deleted_copy_assign &operator=(deleted_copy_assign &&) = default;
    };

    /**
     * \lang english
     * @brief Selects the copy assignment strategy according to type properties.
     *
     * \lang simp-chinese
     * @brief 根据类型特性选择复制赋值策略。
     */
    template <typename Base, typename... Types>
    using copy_assign_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...,
                                                   type_traits::properties::is_trivially_copy_constructible<Types>...,
                                                   type_traits::properties::is_trivially_copy_assignable<Types>...>,
        move_control<Base, Types...>,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_copy_constructible<Types>...,
                                                       type_traits::properties::is_copy_assignable<Types>...>,
            non_trivial_copy_assign<Base, Types...>, deleted_copy_assign<Base, Types...>>>;

    /**
     * \lang english
     * @brief Non-trivial move assignment control.
     *
     * \lang simp-chinese
     * @brief 非平凡移动赋值操作控制。
     */
    template <typename Base, typename... Types>
    struct non_trivial_move_assign : copy_assign_control<Base, Types...> {
        /**
         * \lang english
         * @brief Alias for the base class `copy_assign_control<Base, Types...>`.
         *
         * \lang simp-chinese
         * @brief 基类 `copy_assign_control<Base, Types...>` 的别名。
         */
        using MyBase = copy_assign_control<Base, Types...>;

        /**
         * \lang english
         * @brief Inherits all constructors from `MyBase`.
         *
         * \lang simp-chinese
         * @brief 继承 `MyBase` 的所有构造函数。
         */
        using MyBase::MyBase;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        non_trivial_move_assign() = default;

        /**
         * \lang english
         * @brief Copy constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数（默认）。
         */
        non_trivial_move_assign(const non_trivial_move_assign &) = default;

        /**
         * \lang english
         * @brief Move constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动构造函数（默认）。
         */
        non_trivial_move_assign(non_trivial_move_assign &&) = default;

        /**
         * \lang english
         * @brief Copy assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符（默认）。
         */
        non_trivial_move_assign &operator=(const non_trivial_move_assign &) = default;

        /**
         * \lang english
         * @brief User-defined move assignment operator.
         *
         * Calls `MyBase::assign_from` on the `Base` subobject of `other` to
         * (re)assign the derived state. Declared `noexcept` according to the
         * `noexcept` specification of that `assign_from` call.
         *
         * @param other The object to move-assign from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 用户定义的移动赋值运算符。
         *
         * 对 `other` 的 `Base` 子对象调用 `MyBase::assign_from`
         * 以（重新）赋值派生状态。其 `noexcept` 说明取决于该 `assign_from`
         * 调用是否抛出异常。
         *
         * @param other 要移动赋值的对象
         * @return 此对象的引用
         */
        constexpr non_trivial_move_assign &operator=(non_trivial_move_assign &&other) noexcept(
            noexcept(MyBase::assign_from(static_cast<Base &&>(other)))) {
            MyBase::assign_from(static_cast<Base &&>(other));
            return *this;
        }
    };

    /**
     * \lang english
     * @brief Wrapper type that deletes move assignment.
     *
     * \lang simp-chinese
     * @brief 删除移动赋值操作的包装类型。
     */
    template <typename Base, typename... Types>
    struct deleted_move_assign : copy_assign_control<Base, Types...> {
        /**
         * \lang english
         * @brief Alias for the base class `copy_assign_control<Base, Types...>`.
         *
         * \lang simp-chinese
         * @brief 基类 `copy_assign_control<Base, Types...>` 的别名。
         */
        using MyBase = copy_assign_control<Base, Types...>;

        /**
         * \lang english
         * @brief Inherits all constructors from `MyBase`.
         *
         * \lang simp-chinese
         * @brief 继承 `MyBase` 的所有构造函数。
         */
        using MyBase::MyBase;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        deleted_move_assign() = default;

        /**
         * \lang english
         * @brief Copy constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数（默认）。
         */
        deleted_move_assign(const deleted_move_assign &) = default;

        /**
         * \lang english
         * @brief Move constructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 移动构造函数（默认）。
         */
        deleted_move_assign(deleted_move_assign &&) = default;

        /**
         * \lang english
         * @brief Copy assignment operator (defaulted).
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符（默认）。
         */
        deleted_move_assign &operator=(const deleted_move_assign &) = default;

        /**
         * \lang english
         * @brief Deleted move assignment operator. Makes the type non-move-assignable.
         *
         * Note that the move constructor remains available even though move
         * assignment is deleted.
         *
         * \lang simp-chinese
         * @brief 被删除的移动赋值运算符。使该类型不可移动赋值。
         *
         * 注意：尽管移动赋值已被删除，移动构造函数仍然可用。
         */
        deleted_move_assign &operator=(deleted_move_assign &&) = delete;
    };

    /**
     * \lang english
     * @brief The final unified special member function control selector.
     *
     *  Selects the concrete copy, move and assignment strategies through layered control.
     *
     * \lang simp-chinese
     * @brief 最终统一的特殊成员函数控制选择器。
     *
     *  通过层层控制选择复制、移动、赋值的具体策略。
     */
    template <typename Base, typename... Types>
    using move_assign_control = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...,
                                                   type_traits::properties::is_trivially_move_constructible<Types>...,
                                                   type_traits::properties::is_trivially_move_assignable<Types>...>,
        copy_assign_control<Base, Types...>,
        type_traits::other_trans::conditional_t<
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Types>...,
                                                       type_traits::properties::is_move_assignable<Types>...>,
            non_trivial_move_assign<Base, Types...>, deleted_move_assign<Base, Types...>>>;

    /**
     * \lang english
     * @brief The final unified control type alias.
     *
     * @tparam Base The base class.
     * @tparam Types The types used for the traits checks.
     *
     * \lang simp-chinese
     * @brief 最终统一的控制类型别名。
     *
     * @tparam Base 基类。
     * @tparam Types 特性检测类型。
     */
    template <typename Base, typename... Types>
    using control = move_assign_control<Base, Types...>;
}

#endif
