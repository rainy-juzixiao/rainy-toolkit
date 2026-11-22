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
#ifndef RAINY_CORE_MECHANISM_PATTERN_MATCHING_EXPRESSION_HPP
#define RAINY_CORE_MECHANISM_PATTERN_MATCHING_EXPRESSION_HPP
#include <rainy/core/mechanism/pattern_matching/pre.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::core::mechanism::pattern_matching::implements {
    /**
     * \lang english
     * @brief Placeholder pattern that accepts any value.
     *
     * \lang simp-chinese
     * @brief 接受任意值的占位模式。
     */
    class wildcard {};

    /**
     * \lang english
     * @brief The wildcard pattern instance.
     *
     * \lang simp-chinese
     * @brief 通配符模式实例。
     */
    constexpr wildcard _;

    template <typename Ty>
    struct nullary_t : Ty {
        using Ty::operator();
    };

    template <typename Ty>
    constexpr auto nullary(const Ty &t) {
        return nullary_t<Ty>{t};
    }

    template <typename Ty>
    struct identifier;
}

namespace rainy::core::mechanism::pattern_matching {
    /**
     * \lang english
     * @brief Wraps a captured identifier into a deferred expression that reads the bound value on evaluation.
     * @tparam Ty The captured type.
     * @param id The identifier to read from.
     *
     * \lang simp-chinese
     * @brief 将捕获的标识符包装成一个延迟表达式，求值时读取已绑定的值。
     * @tparam Ty 捕获的类型。
     * @param id 要读取的标识符。
     */
    template <typename Ty>
    constexpr rain_fn expr(implements::identifier<Ty> &id) -> auto {
        using implements::nullary;
        return nullary([&] { return *id; });
    }

    /**
     * \lang english
     * @brief Wraps a value into a deferred expression that yields it on evaluation.
     * @tparam Ty The value type.
     * @param v The value to yield.
     *
     * \lang simp-chinese
     * @brief 将值包装成延迟表达式，求值时返回该值。
     * @tparam Ty 值的类型。
     * @param v 要返回的值。
     */
    template <typename Ty>
    constexpr rain_fn expr(const Ty &v) -> auto {
        using implements::nullary;
        return nullary([&](auto const &...) { return v; });
    }

    /**
     * \lang english
     * @brief Normalizes a callable or a value into a nullary expression.
     *
     * Callables pass through unchanged; plain values are wrapped by `expr`.
     *
     * \lang simp-chinese
     * @brief 将可调用对象或普通值统一成零元表达式。
     *
     * 可调用对象原样返回；普通值通过 `expr` 包装。
     */
    template <typename Ty>
    constexpr rain_fn to_nullary(Ty &&v) -> auto {
        if constexpr (type_traits::properties::is_invocable_v<type_traits::other_trans::decay_t<Ty>>) {
            return v;
        } else {
            return expr(v);
        }
    }
}

namespace rainy::core::mechanism::pattern_matching::implements {
    template <typename Ty>
    struct eval_traits {
        template <typename... Args>
        constexpr static rain_fn eval_impl(const Ty &v, const Args &...) -> decltype(auto) {
            return v;
        }
    };

    template <typename Ty>
    struct eval_traits<nullary_t<Ty>> {
        template <typename... Args>
        constexpr static rain_fn eval_impl(const nullary_t<Ty> &e, const Args &...args) -> decltype(auto) {
            return e(args...);
        }
    };

    template <typename Ty>
    struct eval_traits<identifier<Ty>> {
        constexpr static rain_fn eval_impl(const identifier<Ty> &id) -> decltype(auto) {
            return *const_cast<identifier<Ty> &>(id);
        }
    };

    template <>
    struct eval_traits<wildcard> {
        template <typename Arg>
        constexpr static rain_fn eval_impl(const wildcard &, const Arg &arg) -> decltype(auto) {
            return arg;
        }
    };

    template <typename Ty, typename... Args>
    constexpr rain_fn evaluate_(const Ty &t, const Args &...args) -> decltype(auto) {
        return eval_traits<Ty>::eval_impl(t, args...);
    }

    template <typename Ty>
    struct is_nullary_or_identifier : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_nullary_or_identifier<identifier<Ty>> : type_traits::helper::true_type {};

    template <typename Ty>
    struct is_nullary_or_identifier<nullary_t<Ty>> : type_traits::helper::true_type {};

    template <>
    struct is_nullary_or_identifier<wildcard> : type_traits::helper::true_type {};

    template <typename Ty>
    constexpr auto is_nullary_or_identifier_v = is_nullary_or_identifier<type_traits::other_trans::decay_t<Ty>>::value;
}

namespace rainy::core::mechanism::pattern_matching::implements {
    template <typename Ty, type_traits::other_trans::enable_if_t<is_nullary_or_identifier_v<Ty>, int> = 0>
    constexpr rain_fn operator!(const Ty &t)->auto {
        return nullary([&](auto const &...args) { return !evaluate_(t, args...); });
    }

    template <typename Ty, type_traits::other_trans::enable_if_t<is_nullary_or_identifier_v<Ty>, int> = 0>
    constexpr rain_fn operator-(const Ty &t)->auto {
        return nullary([&](auto const &...args) { return -evaluate_(t, args...); });
    }
}

#define RAINY_BIN_OP_FOR_NULLARY(op)                                                                                                  \
    template <typename Ty, typename UTy,                                                                                              \
              type_traits::other_trans::enable_if_t<is_nullary_or_identifier_v<Ty> || is_nullary_or_identifier_v<UTy>, int> = 0>      \
    constexpr rain_fn operator op(const Ty &t, const UTy &u)->auto {                                                                  \
        return nullary([&](auto const &...args) { return evaluate_(t, args...) op evaluate_(u, args...); });                          \
    }

namespace rainy::core::mechanism::pattern_matching::implements {
    RAINY_BIN_OP_FOR_NULLARY(+)
    RAINY_BIN_OP_FOR_NULLARY(-)
    RAINY_BIN_OP_FOR_NULLARY(*)
    RAINY_BIN_OP_FOR_NULLARY(/)
    RAINY_BIN_OP_FOR_NULLARY(%)
    RAINY_BIN_OP_FOR_NULLARY(<)
    RAINY_BIN_OP_FOR_NULLARY(<=)
    RAINY_BIN_OP_FOR_NULLARY(==)
    RAINY_BIN_OP_FOR_NULLARY(!=)
    RAINY_BIN_OP_FOR_NULLARY(>=)
    RAINY_BIN_OP_FOR_NULLARY(>)
    RAINY_BIN_OP_FOR_NULLARY(||)
    RAINY_BIN_OP_FOR_NULLARY(&&)
    RAINY_BIN_OP_FOR_NULLARY(^)
}

#undef RAINY_BIN_OP_FOR_NULLARY

#endif
