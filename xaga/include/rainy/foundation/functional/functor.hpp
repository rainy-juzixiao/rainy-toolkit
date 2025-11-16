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
#ifndef RAINY_FOUNDATION_FUNCTIONAL_FUNCTOR_HPP
#define RAINY_FOUNDATION_FUNCTIONAL_FUNCTOR_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::functional {
    template <typename Ty = void>
    struct plus {
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left + right;
        }
    };

    template <>
    struct plus<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) + utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) + utility::forward<Ty2>(right)) {
            return left + right;
        }
    };

    template <typename Ty = void>
    struct minus {
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left - right;
        }
    };

    template <>
    struct minus<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) - utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) - utility::forward<Ty2>(right)) {
            return left - right;
        }
    };

    template <typename Ty = void>
    struct multiplies {
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left * right;
        }
    };

    template <>
    struct multiplies<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) * utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) * utility::forward<Ty2>(right)) {
            return left * right;
        }
    };

    template <typename Ty = void>
    struct divides {
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left / right;
        }
    };

    template <>
    struct divides<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) / utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) / utility::forward<Ty2>(right)) {
            return left / right;
        }
    };

    template <typename Ty = void>
    struct modulus {
        constexpr Ty operator()(const Ty &left, const Ty &right) const {
            return left % right;
        }
    };

    template <>
    struct modulus<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) % utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) % utility::forward<Ty2>(right)) {
            return left % right;
        }
    };

    template <typename Ty = void>
    struct negate {
        constexpr Ty operator()(const Ty &object) const {
            return -object;
        }
    };

    template <>
    struct negate<void> {
        template <typename Ty>
        constexpr auto operator()(const Ty &object) const noexcept(noexcept(-utility::forward<Ty>(object)))
            -> decltype(-utility::forward<Ty>(object)) {
            return -object;
        }
    };

    template <typename Ty = void>
    struct equal {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left == right;
        }
    };

    template <>
    struct equal<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) == utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) == utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) == utility::forward<Ty2>(right);
        }
    };

    template <typename Ty = void>
    struct not_equal {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left != right;
        }
    };

    template <>
    struct not_equal<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) != utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) != utility::forward<Ty2>(right)) {
            return left != right;
        }
    };

    template <typename Ty = void>
    struct less {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left < right;
        }
    };

    template <>
    struct less<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) < utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) < utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) < utility::forward<Ty2>(right);
        }
    };

    template <typename Ty = void>
    struct less_equal {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left <= right;
        }
    };

    template <>
    struct less_equal<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) <= utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) <= utility::forward<Ty2>(right)) {
            return left <= right;
        }
    };

    template <typename Ty = void>
    struct greater {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left > right;
        }
    };

    template <>
    struct greater<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) > utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) > utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) > utility::forward<Ty2>(right);
        }
    };

    template <typename Ty = void>
    struct greater_equal {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left >= right;
        }
    };

    template <>
    struct greater_equal<void> {
        template <typename Ty1, typename Ty2>
        constexpr auto operator()(Ty1 &&left, Ty2 &&right) const
            noexcept(noexcept(utility::forward<Ty1>(left) >= utility::forward<Ty2>(right)))
                -> decltype(utility::forward<Ty1>(left) >= utility::forward<Ty2>(right)) {
            return utility::forward<Ty1>(left) >= utility::forward<Ty2>(right);
        }
    };

    template <typename Ty = void>
    struct logical_and {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left && right;
        }
    };

    template <>
    struct logical_and<void> {
        template <typename Ty, typename U>
        constexpr auto operator()(const Ty &left, const U &right) const
            noexcept(noexcept(utility::forward<Ty>(left) && utility::forward<U>(right)))
                -> decltype(utility::forward<Ty>(left) && utility::forward<U>(right)) {
            return left && right;
        }
    };

    template <typename Ty = void>
    struct logical_or {
        constexpr bool operator()(const Ty &left, const Ty &right) const {
            return left || right;
        }
    };

    template <>
    struct logical_or<void> {
        template <typename Ty, typename U>
        constexpr auto operator()(const Ty &left, const U &right) const
            noexcept(noexcept(utility::forward<Ty>(left) || utility::forward<U>(right)))
                -> decltype(utility::forward<Ty>(left) || utility::forward<U>(right)) {
            return left || right;
        }
    };

    template <typename Ty = void>
    struct logical_not {
        constexpr bool operator()(const Ty &object) const {
            return !object;
        }
    };

    template <>
    struct logical_not<void> {
        template <typename Ty>
        constexpr auto operator()(const Ty &object) const noexcept(noexcept(!utility::forward<Ty>(object)))
            -> decltype(!utility::forward<Ty>(object)) {
            return !object;
        }
    };
}

#endif
