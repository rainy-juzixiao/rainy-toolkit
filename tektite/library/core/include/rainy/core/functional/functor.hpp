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
#ifndef RAINY_CORE_FUNCTIONAL_FUNCTOR_HPP
#define RAINY_CORE_FUNCTIONAL_FUNCTOR_HPP
#include <rainy/core/platform.hpp>

namespace rainy::functional {
    /**
     * \lang english
     * @brief Function object for the addition operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 加法运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct plus {
        /**
         * \lang english
         * @brief Returns the sum of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left + right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的和。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left + right 的结果。
         */
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left + right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of plus for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief plus针对异构操作数的透明特化。
     */
    template <>
    struct plus<void> {
        /**
         * \lang english
         * @brief Returns the sum of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left + right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的和。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left + right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) + utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) + utility::forward<Ty2>(right)) {
            return left + right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the subtraction operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 减法运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct minus {
        /**
         * \lang english
         * @brief Returns the difference of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left - right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的差。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left - right 的结果。
         */
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left - right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of minus for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief minus针对异构操作数的透明特化。
     */
    template <>
    struct minus<void> {
        /**
         * \lang english
         * @brief Returns the difference of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left - right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的差。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left - right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) - utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) - utility::forward<Ty2>(right)) {
            return left - right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the multiplication operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 乘法运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct multiplies {
        /**
         * \lang english
         * @brief Returns the product of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left * right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的积。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left * right 的结果。
         */
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left * right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of multiplies for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief multiplies针对异构操作数的透明特化。
     */
    template <>
    struct multiplies<void> {
        /**
         * \lang english
         * @brief Returns the product of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left * right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的积。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left * right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) * utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) * utility::forward<Ty2>(right)) {
            return left * right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the division operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 除法运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct divides {
        /**
         * \lang english
         * @brief Returns the quotient of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left / right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的商。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left / right 的结果。
         */
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left / right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of divides for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief divides针对异构操作数的透明特化。
     */
    template <>
    struct divides<void> {
        /**
         * \lang english
         * @brief Returns the quotient of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left / right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的商。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left / right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) / utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) / utility::forward<Ty2>(right)) {
            return left / right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the modulus operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 取模运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct modulus {
        /**
         * \lang english
         * @brief Returns the remainder of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left % right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的余数。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left % right 的结果。
         */
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left % right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of modulus for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief modulus针对异构操作数的透明特化。
     */
    template <>
    struct modulus<void> {
        /**
         * \lang english
         * @brief Returns the remainder of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left % right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的余数。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left % right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) % utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) % utility::forward<Ty2>(right)) {
            return left % right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the unary negation operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 一元取负运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct negate {
        /**
         * \lang english
         * @brief Returns the negation of the operand.
         * @param object The operand.
         * @return The result of -object.
         *
         * \lang simp-chinese
         * @brief 返回操作数的相反数。
         * @param object 操作数。
         * @return -object 的结果。
         */
        constexpr Ty operator()(const Ty &object) const {
            return -object;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of negate for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief negate针对异构操作数的透明特化。
     */
    template <>
    struct negate<void> {
        /**
         * \lang english
         * @brief Returns the negation of the operand.
         * @tparam Ty The type of the operand.
         * @param object The operand.
         * @return The result of -object.
         *
         * \lang simp-chinese
         * @brief 返回操作数的相反数。
         * @tparam Ty 操作数的类型。
         * @param object 操作数。
         * @return -object 的结果。
         */
        template <typename Ty>
        constexpr auto operator()(Ty &&object) const noexcept(noexcept(-utility::forward<Ty>(object)))
            -> decltype(-utility::forward<Ty>(object)) {
            return -object;
        }
    };

    /**
     * \lang english
     * @brief Function object for the equality comparison.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 相等比较的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct equal {
        /**
         * \lang english
         * @brief Checks whether the two operands are equal.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left == right.
         *
         * \lang simp-chinese
         * @brief 检查两个操作数是否相等。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left == right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left == right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of equal for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief equal针对异构操作数的透明特化。
     */
    template <>
    struct equal<void> {
        /**
         * \lang english
         * @brief Checks whether the two operands are equal.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left == right.
         *
         * \lang simp-chinese
         * @brief 检查两个操作数是否相等。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left == right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) == utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) == utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) == utility::forward<Ty2>(right);
        }
    };

    /**
     * \lang english
     * @brief Function object for the inequality comparison.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 不等比较的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct not_equal {
        /**
         * \lang english
         * @brief Checks whether the two operands are not equal.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left != right.
         *
         * \lang simp-chinese
         * @brief 检查两个操作数是否不相等。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left != right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left != right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of not_equal for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief not_equal针对异构操作数的透明特化。
     */
    template <>
    struct not_equal<void> {
        /**
         * \lang english
         * @brief Checks whether the two operands are not equal.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left != right.
         *
         * \lang simp-chinese
         * @brief 检查两个操作数是否不相等。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left != right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) != utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) != utility::forward<Ty2>(right)) {
            return left != right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the less-than comparison.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 小于比较的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct less {
        /**
         * \lang english
         * @brief Checks whether the left operand is less than the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left < right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否小于右操作数。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left < right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left < right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of less for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief less针对异构操作数的透明特化。
     */
    template <>
    struct less<void> {
        /**
         * \lang english
         * @brief Checks whether the left operand is less than the right operand.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left < right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否小于右操作数。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left < right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) < utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) < utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) < utility::forward<Ty2>(right);
        }
    };

    /**
     * \lang english
     * @brief Function object for the less-than-or-equal comparison.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 小于等于比较的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct less_equal {
        /**
         * \lang english
         * @brief Checks whether the left operand is less than or equal to the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left <= right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否小于或等于右操作数。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left <= right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left <= right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of less_equal for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief less_equal针对异构操作数的透明特化。
     */
    template <>
    struct less_equal<void> {
        /**
         * \lang english
         * @brief Checks whether the left operand is less than or equal to the right operand.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left <= right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否小于或等于右操作数。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left <= right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) <= utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) <= utility::forward<Ty2>(right)) {
            return left <= right;
        }
    };

    /**
     * \lang english
     * @brief Function object for the greater-than comparison.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 大于比较的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct greater {
        /**
         * \lang english
         * @brief Checks whether the left operand is greater than the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left > right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否大于右操作数。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left > right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left > right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of greater for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief greater针对异构操作数的透明特化。
     */
    template <>
    struct greater<void> {
        /**
         * \lang english
         * @brief Checks whether the left operand is greater than the right operand.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left > right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否大于右操作数。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left > right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) > utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) > utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) > utility::forward<Ty2>(right);
        }
    };

    /**
     * \lang english
     * @brief Function object for the greater-than-or-equal comparison.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 大于等于比较的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct greater_equal {
        /**
         * \lang english
         * @brief Checks whether the left operand is greater than or equal to the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left >= right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否大于或等于右操作数。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left >= right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left >= right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of greater_equal for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief greater_equal针对异构操作数的透明特化。
     */
    template <>
    struct greater_equal<void> {
        /**
         * \lang english
         * @brief Checks whether the left operand is greater than or equal to the right operand.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left >= right.
         *
         * \lang simp-chinese
         * @brief 检查左操作数是否大于或等于右操作数。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left >= right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) >= utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) >= utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) >= utility::forward<Ty2>(right);
        }
    };

    /**
     * \lang english
     * @brief Function object for the logical AND operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 逻辑与运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct logical_and {
        /**
         * \lang english
         * @brief Returns the logical AND of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left && right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的逻辑与。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left && right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left && right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of logical_and for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief logical_and针对异构操作数的透明特化。
     */
    template <>
    struct logical_and<void> {
        /**
         * \lang english
         * @brief Returns the logical AND of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left && right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的逻辑与。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left && right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) && utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) && utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) && utility::forward<Ty2>(right);
        }
    };

    /**
     * \lang english
     * @brief Function object for the logical OR operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 逻辑或运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct logical_or {
        /**
         * \lang english
         * @brief Returns the logical OR of the two operands.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left || right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的逻辑或。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left || right 的结果。
         */
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left || right;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of logical_or for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief logical_or针对异构操作数的透明特化。
     */
    template <>
    struct logical_or<void> {
        /**
         * \lang english
         * @brief Returns the logical OR of the two operands.
         * @tparam Ty1 The type of the left operand.
         * @tparam Ty2 The type of the right operand.
         * @param left The left operand.
         * @param right The right operand.
         * @return The result of left || right.
         *
         * \lang simp-chinese
         * @brief 返回两个操作数的逻辑或。
         * @tparam Ty1 左操作数的类型。
         * @tparam Ty2 右操作数的类型。
         * @param left 左操作数。
         * @param right 右操作数。
         * @return left || right 的结果。
         */
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) || utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) || utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) || utility::forward<Ty2>(right);
        }
    };

    /**
     * \lang english
     * @brief Function object for the unary logical NOT operation.
     * @tparam Ty The operand type, defaulting to void.
     *
     * \lang simp-chinese
     * @brief 一元逻辑非运算的函数对象。
     * @tparam Ty 操作数类型，默认为void。
     */
    template <typename Ty = void>
    struct logical_not {
        /**
         * \lang english
         * @brief Returns the logical negation of the operand.
         * @param object The operand.
         * @return The result of !object.
         *
         * \lang simp-chinese
         * @brief 返回操作数的逻辑非。
         * @param object 操作数。
         * @return !object 的结果。
         */
        constexpr bool operator()(const Ty &object) const {
            return !object;
        }
    };

    /**
     * \lang english
     * @brief Transparent specialization of logical_not for heterogeneous operands.
     *
     * \lang simp-chinese
     * @brief logical_not针对异构操作数的透明特化。
     */
    template <>
    struct logical_not<void> {
        /**
         * \lang english
         * @brief Returns the logical negation of the operand.
         * @tparam Ty The type of the operand.
         * @param object The operand.
         * @return The result of !object.
         *
         * \lang simp-chinese
         * @brief 返回操作数的逻辑非。
         * @tparam Ty 操作数的类型。
         * @param object 操作数。
         * @return !object 的结果。
         */
        template <typename Ty>
        constexpr auto operator()(Ty &&object) const noexcept(noexcept(utility::forward<Ty>(!object)))
            -> decltype(-utility::forward<Ty>(!object)) {
            return !object;
        }
    };
}

#endif
