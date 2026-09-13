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
 */                                                                                                                                   \
#ifndef RAINY_CORE_CONTAINER_MOVABLE_BOX_HPP
#define RAINY_CORE_CONTAINER_MOVABLE_BOX_HPP

#include <rainy/core/type_traits.hpp>

namespace rainy::core::container::implements {
    /**
     * \lang english
     * @brief Trait indicating whether Ty is a valid movable-box object type.
     *
     *  A type is considered valid for movable_box if it is both copy constructible
     *  and destructible.
     *
     * @tparam Ty The value type to test.
     *
     * \lang simp-chinese
     * @brief 指示 Ty 是否为合法的 movable_box 对象类型的特征。
     *
     *  若某类型既为可拷贝构造、又为可析构，则该类型被视为对 movable_box 合法。
     *
     * @tparam Ty 要测试的值类型。
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_valid_movable_object =
        type_traits::properties::is_copy_constructible_v<Ty> && type_traits::properties::is_destructible_v<Ty>;
}

namespace rainy::core::container {
#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

    /**
     * \lang english
     * @brief An optional-like wrapper that stores a value in a union and supports
     *        both copy and move semantics.
     *
     *  The contained value is stored inside an anonymous union and is only
     *  constructed while the box is engaged. Dereferencing an empty box is
     *  undefined behavior (asserted in debug builds).
     *
     * @tparam Ty The stored value type.
     *
     * \lang simp-chinese
     * @brief 在 union 中存储值、同时支持拷贝与移动语义的类 optional 包装器。
     *
     *  所含值存储于匿名 union 中，仅当盒对象有效时才被构造。对空盒解引用为
     *  未定义行为（在调试构建中断言）。
     *
     * @tparam Ty 所存储的值类型。
     */
    template <typename Ty>
    class movable_box {
    public:
        /**
         * \lang english
         * @brief Default constructor. Value-initializes the contained value and
         *        marks the box as engaged.
         *
         * @note Enabled only if Ty is default constructible.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。对所包含的值进行值初始化并将盒标记为有效。
         *
         * @note 仅在 Ty 可默认构造时启用。
         */
        template <typename UTy = Ty, type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<UTy>, int> = 0>
        constexpr movable_box() noexcept(type_traits::properties::is_nothrow_default_constructible_v<UTy>) :
            value_(), is_valid_{true} {
        }

        /**
         * \lang english
         * @brief In-place constructor. Forwards the given arguments to construct
         *        the contained value and marks the box as engaged.
         *
         * @tparam Args The argument types.
         * @param <unnamed> The std::in_place_t tag.
         * @param args The arguments to forward to the contained value's constructor.
         *
         * \lang simp-chinese
         * @brief 原位构造函数。将给定参数转发以构造所包含的值，并将盒标记为有效。
         *
         * @tparam Args 参数类型。
         * @param <unnamed> std::in_place_t 标签。
         * @param args 要转发给所含值构造函数的参数。
         */
        template <typename... Args>
        constexpr movable_box(std::in_place_t,
                              Args &&...args) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) :
            value_(utility::forward<Args>(args)...), is_valid_{true} {
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the contained value if the box is engaged.
         *
         * \lang simp-chinese
         * @brief 析构函数。若盒有效则销毁所包含的值。
         */
        RAINY_CONSTEXPR20 ~movable_box() {
            if (is_valid_) {
                value_.~Ty();
            }
        }

        /**
         * \lang english
         * @brief Copy constructor. Copies the contained value if the source is engaged.
         *
         * @note Enabled only if Ty is copy constructible.
         *
         * @param right The source movable_box to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。若源对象有效则拷贝所包含的值。
         *
         * @note 仅在 Ty 可拷贝构造时启用。
         *
         * @param right 要拷贝的源 movable_box
         */
        constexpr movable_box(const movable_box &right) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<Ty>)
            requires(type_traits::properties::is_copy_constructible_v<Ty>) : is_valid_{right.is_valid_} {
            if (right.is_valid_) {
                utility::construct_in_place(value_, static_cast<const Ty &>(right.value_));
            }
        }

        /**
         * \lang english
         * @brief Move constructor. Moves the contained value if the source is engaged.
         *
         * @param right The source movable_box to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。若源对象有效则移动所包含的值。
         *
         * @param right 要移动的源 movable_box
         */
        constexpr movable_box(movable_box &&right) : is_valid_{right.is_valid_} {
            if (right.is_valid_) {
                utility::construct_in_place(value_, static_cast<Ty &&>(right.value_));
            }
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @param right The source movable_box to copy from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @param right 要拷贝的源 movable_box
         * @return 此对象的引用
         */
        constexpr movable_box &operator=(const movable_box &right) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_copy_assignable_v<Ty>)
            requires(type_traits::properties::is_copyable_v<Ty>) {
            if (is_valid_) {
                if (right.is_valid_) {
                    static_cast<Ty &>(value_) = static_cast<const Ty &>(right.value_);
                } else {
                    value_.~Ty();
                    is_valid_ = false;
                }
            } else {
                if (right.is_valid_) {
                    utility::construct_in_place(value_, static_cast<const Ty &>(right.value_));
                    is_valid_ = true;
                } else {
                    // nothing to do
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         *  Destroys the currently contained value (if any) and move-constructs the
         *  contained value from right. Self-assignment is a no-op.
         *
         * @param right The source movable_box to move from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         *  销毁当前所含值（若有），并从 right 移动构造所含值。自赋值不执行任何操作。
         *
         * @param right 要移动的源 movable_box
         * @return 此对象的引用
         */
        constexpr movable_box &operator=(movable_box &&right) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<Ty>) {
            if (utility::addressof(right) == this) {
                return *this;
            }
            if (is_valid_) {
                value_.~Ty();
                is_valid_ = false;
            }
            if (right.is_valid_) {
                utility::construct_in_place(value_, static_cast<Ty &&>(right.value_));
                is_valid_ = true;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Checks whether the box is engaged.
         *
         * @return true if the box contains a value.
         *
         * \lang simp-chinese
         * @brief 检查盒是否有效。
         *
         * @return 若盒中包含值则为 true。
         */
        constexpr explicit operator bool() const noexcept {
            return is_valid_;
        }

        /**
         * \lang english
         * @brief Accesses the contained value.
         *
         * @return A reference to the contained value.
         *
         * @note Dereferencing an empty box is undefined behavior.
         *
         * \lang simp-chinese
         * @brief 访问所包含的值。
         *
         * @return 所含值的引用。
         *
         * @note 对空盒解引用为未定义行为。
         */
        RAINY_NODISCARD constexpr Ty &operator*() noexcept {
            assert(is_valid_);
            return value_;
        }

        /**
         * \lang english
         * @brief Accesses the contained value (const overload).
         *
         * @return A const reference to the contained value.
         *
         * @note Dereferencing an empty box is undefined behavior.
         *
         * \lang simp-chinese
         * @brief 访问所包含的值（const 重载）。
         *
         * @return 所含值的常量引用。
         *
         * @note 对空盒解引用为未定义行为。
         */
        RAINY_NODISCARD constexpr const Ty &operator*() const noexcept {
            assert(is_valid_);
            return value_;
        }

    private:
        union {
            type_traits::modifers::remove_cv_t<Ty> value_;
        };
        bool is_valid_;
    };

#else

    /**
     * \lang english
     * @brief An optional-like wrapper that stores a value in a union and supports
     *        both copy and move semantics.
     *
     * @tparam Ty The stored value type.
     *
     * \lang simp-chinese
     * @brief 在 union 中存储值、同时支持拷贝与移动语义的类 optional 包装器。
     *
     * @tparam Ty 所存储的值类型。
     */
    template <typename Ty>
    class movable_box {
    public:
        /**
         * \lang english
         * @brief Default constructor. Value-initializes the contained value and
         *        marks the box as engaged.
         *
         * @note Enabled only if Ty is default constructible.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。对所包含的值进行值初始化并将盒标记为有效。
         *
         * @note 仅在 Ty 可默认构造时启用。
         */
        template <typename UTy = Ty, type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<UTy>, int> = 0>
        constexpr movable_box() noexcept(type_traits::properties::is_nothrow_default_constructible_v<UTy>);

        /**
         * \lang english
         * @brief In-place constructor. Forwards the given arguments to construct
         *        the contained value and marks the box as engaged.
         *
         * @tparam Args The argument types.
         * @param <unnamed> The std::in_place_t tag.
         * @param args The arguments to forward to the contained value's constructor.
         *
         * \lang simp-chinese
         * @brief 原位构造函数。将给定参数转发以构造所包含的值，并将盒标记为有效。
         *
         * @tparam Args 参数类型。
         * @param <unnamed> std::in_place_t 标签。
         * @param args 要转发给所含值构造函数的参数。
         */
        template <typename... Args>
        constexpr movable_box(std::in_place_t, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, Args...>);

        /**
         * \lang english
         * @brief Destructor. Destroys the contained value if the box is engaged.
         *
         * \lang simp-chinese
         * @brief 析构函数。若盒有效则销毁所包含的值。
         */
        RAINY_CONSTEXPR20 ~movable_box();

        /**
         * \lang english
         * @brief Copy constructor. Copies the contained value if the source is engaged.
         *
         * @note Enabled only if Ty is copy constructible.
         *
         * @param right The source movable_box to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。若源对象有效则拷贝所包含的值。
         *
         * @note 仅在 Ty 可拷贝构造时启用。
         *
         * @param right 要拷贝的源 movable_box
         */
        constexpr movable_box(const movable_box &right) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<Ty>)
            requires(type_traits::properties::is_copy_constructible_v<Ty>);

        /**
         * \lang english
         * @brief Move constructor. Moves the contained value if the source is engaged.
         *
         * @param right The source movable_box to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。若源对象有效则移动所包含的值。
         *
         * @param right 要移动的源 movable_box
         */
        constexpr movable_box(movable_box &&right);

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @param right The source movable_box to copy from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @param right 要拷贝的源 movable_box
         * @return 此对象的引用
         */
        constexpr movable_box &operator=(const movable_box &right) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_copy_assignable_v<Ty>)
            requires(type_traits::properties::is_copyable_v<Ty>);

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right The source movable_box to move from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 要移动的源 movable_box
         * @return 此对象的引用
         */
        constexpr movable_box &operator=(movable_box &&right) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<Ty>);

        /**
         * \lang english
         * @brief Checks whether the box is engaged.
         *
         * @return true if the box contains a value.
         *
         * \lang simp-chinese
         * @brief 检查盒是否有效。
         *
         * @return 若盒中包含值则为 true。
         */
        constexpr explicit operator bool() const noexcept;

        /**
         * \lang english
         * @brief Accesses the contained value.
         *
         * @return A reference to the contained value.
         *
         * @note Dereferencing an empty box is undefined behavior.
         *
         * \lang simp-chinese
         * @brief 访问所包含的值。
         *
         * @return 所含值的引用。
         *
         * @note 对空盒解引用为未定义行为。
         */
        RAINY_NODISCARD constexpr Ty &operator*() noexcept;

        /**
         * \lang english
         * @brief Accesses the contained value (const overload).
         *
         * @return A const reference to the contained value.
         *
         * @note Dereferencing an empty box is undefined behavior.
         *
         * \lang simp-chinese
         * @brief 访问所包含的值（const 重载）。
         *
         * @return 所含值的常量引用。
         *
         * @note 对空盒解引用为未定义行为。
         */
        RAINY_NODISCARD constexpr const Ty &operator*() const noexcept;
    };

#endif
}

namespace rainy::container {
    using rainy::core::container::movable_box;
}

#endif