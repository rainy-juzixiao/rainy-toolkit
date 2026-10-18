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
#ifndef RAINY_CORE_SIMD_MATH_HPP
#define RAINY_CORE_SIMD_MATH_HPP

#include <cmath>
#include <rainy/core/simd/basic_vec.hpp>
#include <rainy/core/simd/math_common.hpp>

namespace rainy::core::implements {
    template <typename R, typename V, typename F>
    RAINY_CONSTEXPR26 R map_unary(const V &val, F f) {
        return R{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            return f(val[i]);
        }};
    }

    template <typename R, typename A, typename B, typename F>
    RAINY_CONSTEXPR26 R map_binary(const A &left, const B &right, F f) {
        return R{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            return f(left[i], right[i]);
        }};
    }

    template <typename R, typename A, typename B, typename C, typename F>
    RAINY_CONSTEXPR26 R map_ternary(const A &first, const B &second, const C &third, F f) {
        return R{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            return f(first[i], second[i], third[i]);
        }};
    }
}

namespace rainy::core {
    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> acos(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::acos(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> asin(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::asin(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> atan(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::atan(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> atan2(const V &y, const V &val) {
        return implements::map_binary<deduced_vec_t<V>>(y, val, [](auto a, auto b) { return std::atan2(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> atan2(const deduced_vec_t<V> &y, const V &val) {
        return implements::map_binary<deduced_vec_t<V>>(y, val, [](auto a, auto b) { return std::atan2(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> atan2(const V &y, const deduced_vec_t<V> &val) {
        return implements::map_binary<deduced_vec_t<V>>(y, val, [](auto a, auto b) { return std::atan2(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> cos(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::cos(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> sin(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::sin(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> tan(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::tan(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> acosh(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::acosh(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> asinh(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::asinh(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> atanh(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::atanh(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> cosh(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::cosh(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> sinh(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::sinh(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> tanh(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::tanh(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> exp(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::exp(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> exp2(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::exp2(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> expm1(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::expm1(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> frexp(const V &value, rebind_t<int, deduced_vec_t<V>> *exp) {
        using exponent_type = rebind_t<int, deduced_vec_t<V>>;
        *exp = exponent_type{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            int extracted{};
            std::frexp(value[i], &extracted);
            return extracted;
        }};
        return implements::map_unary<deduced_vec_t<V>>(value, [](auto x) {
            int extracted{};
            return std::frexp(x, &extracted);
        });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 rebind_t<int, deduced_vec_t<V>> ilogb(const V &val) {
        return implements::map_unary<rebind_t<int, deduced_vec_t<V>>>(val, [](auto x) { return std::ilogb(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> ldexp(const V &val, const rebind_t<int, deduced_vec_t<V>> &exp) {
        return implements::map_binary<deduced_vec_t<V>>(val, exp, [](auto x, auto n) { return std::ldexp(x, n); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> log(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::log(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> log10(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::log10(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> log1p(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::log1p(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> log2(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::log2(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> logb(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::logb(x); });
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 basic_vec<Ty, Abi> modf(const type_traits::primary_types::type_identity_t<basic_vec<Ty, Abi>> &value,
                                       basic_vec<Ty, Abi> *iptr) {
        *iptr = basic_vec<Ty, Abi>{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            Ty integral{};
            std::modf(value[i], &integral);
            return integral;
        }};
        return basic_vec<Ty, Abi>{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            Ty integral{};
            return std::modf(value[i], &integral);
        }};
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> scalbn(const V &val, const rebind_t<int, deduced_vec_t<V>> &n) {
        return implements::map_binary<deduced_vec_t<V>>(val, n, [](auto x, auto k) { return std::scalbn(x, k); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> scalbln(const V &val, const rebind_t<long int, deduced_vec_t<V>> &n) {
        return implements::map_binary<deduced_vec_t<V>>(val, n, [](auto x, auto k) { return std::scalbln(x, k); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> cbrt(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::cbrt(x); });
    }

    template <typename Ty, typename Abi,
              type_traits::other_trans::enable_if_t<
                  type_traits::primary_types::is_integral_v<Ty> && type_traits::properties::is_signed_v<Ty>, int>>
    RAINY_CONSTEXPR26 basic_vec<Ty, Abi> abs(const basic_vec<Ty, Abi> &j) {
        return implements::map_unary<basic_vec<Ty, Abi>>(j, [](auto x) { return x < static_cast<Ty>(0) ? -x : x; });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> abs(const V &j) {
        return implements::map_unary<deduced_vec_t<V>>(j, [](auto x) { return std::abs(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fabs(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::fabs(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::hypot(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::hypot(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::hypot(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const V &val, const V &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const V &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const V &val, const deduced_vec_t<V> &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const V &val, const V &y, const deduced_vec_t<V> &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const deduced_vec_t<V> &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const deduced_vec_t<V> &val, const V &y, const deduced_vec_t<V> &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> hypot(const V &val, const deduced_vec_t<V> &y, const deduced_vec_t<V> &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::hypot(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> pow(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::pow(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> pow(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::pow(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> pow(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::pow(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> sqrt(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::sqrt(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> erf(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::erf(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> erfc(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::erfc(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lgamma(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::lgamma(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> tgamma(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::tgamma(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> ceil(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::ceil(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> floor(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::floor(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> nearbyint(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::nearbyint(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> rint(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::rint(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 rebind_t<long int, deduced_vec_t<V>> lrint(const V &val) {
        return implements::map_unary<rebind_t<long int, deduced_vec_t<V>>>(val, [](auto x) { return std::lrint(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 rebind_t<long long int, deduced_vec_t<V>> llrint(const V &val) {
        return implements::map_unary<rebind_t<long long int, deduced_vec_t<V>>>(val,
                                                                                [](auto x) { return std::llrint(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> round(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::round(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 rebind_t<long int, deduced_vec_t<V>> lround(const V &val) {
        return implements::map_unary<rebind_t<long int, deduced_vec_t<V>>>(val, [](auto x) { return std::lround(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 rebind_t<long long int, deduced_vec_t<V>> llround(const V &val) {
        return implements::map_unary<rebind_t<long long int, deduced_vec_t<V>>>(val,
                                                                                [](auto x) { return std::llround(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> trunc(const V &val) {
        return implements::map_unary<deduced_vec_t<V>>(val, [](auto x) { return std::trunc(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmod(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmod(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmod(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmod(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmod(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmod(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> remainder(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::remainder(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> remainder(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::remainder(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> remainder(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::remainder(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> remquo(const V &val, const V &y, rebind_t<int, deduced_vec_t<V>> *quo) {
        using quotient_type = rebind_t<int, deduced_vec_t<V>>;
        *quo = quotient_type{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            int extracted{};
            std::remquo(val[i], y[i], &extracted);
            return extracted;
        }};
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) {
            int extracted{};
            return std::remquo(a, b, &extracted);
        });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> remquo(const deduced_vec_t<V> &val, const V &y, rebind_t<int, deduced_vec_t<V>> *quo) {
        using quotient_type = rebind_t<int, deduced_vec_t<V>>;
        *quo = quotient_type{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            int extracted{};
            std::remquo(val[i], y[i], &extracted);
            return extracted;
        }};
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) {
            int extracted{};
            return std::remquo(a, b, &extracted);
        });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> remquo(const V &val, const deduced_vec_t<V> &y, rebind_t<int, deduced_vec_t<V>> *quo) {
        using quotient_type = rebind_t<int, deduced_vec_t<V>>;
        *quo = quotient_type{[&](auto index) {
            const simd_size_type i = static_cast<simd_size_type>(index);
            int extracted{};
            std::remquo(val[i], y[i], &extracted);
            return extracted;
        }};
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) {
            int extracted{};
            return std::remquo(a, b, &extracted);
        });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> copysign(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::copysign(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> copysign(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::copysign(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> copysign(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::copysign(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> nextafter(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::nextafter(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> nextafter(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::nextafter(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> nextafter(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::nextafter(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fdim(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fdim(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fdim(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fdim(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fdim(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fdim(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmax(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmax(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmax(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmax(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmax(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmax(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmin(const V &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmin(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmin(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmin(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fmin(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<deduced_vec_t<V>>(val, y, [](auto a, auto b) { return std::fmin(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const V &val, const V &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const deduced_vec_t<V> &val, const V &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const V &val, const deduced_vec_t<V> &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const V &val, const V &y, const deduced_vec_t<V> &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const deduced_vec_t<V> &val, const deduced_vec_t<V> &y, const V &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const deduced_vec_t<V> &val, const V &y, const deduced_vec_t<V> &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> fma(const V &val, const deduced_vec_t<V> &y, const deduced_vec_t<V> &z) {
        return implements::map_ternary<deduced_vec_t<V>>(val, y, z,
                                                         [](auto a, auto b, auto c) { return std::fma(a, b, c); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const V &a, const V &b, const V &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const deduced_vec_t<V> &a, const V &b, const V &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const V &a, const deduced_vec_t<V> &b, const V &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const V &a, const V &b, const deduced_vec_t<V> &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const deduced_vec_t<V> &a, const deduced_vec_t<V> &b, const V &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const deduced_vec_t<V> &a, const V &b, const deduced_vec_t<V> &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 deduced_vec_t<V> lerp(const V &a, const deduced_vec_t<V> &b, const deduced_vec_t<V> &t) noexcept {
        return implements::map_ternary<deduced_vec_t<V>>(a, b, t,
                                                         [](auto x, auto y, auto p) { return std::lerp(x, y, p); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 rebind_t<int, deduced_vec_t<V>> fpclassify(const V &val) {
        return implements::map_unary<rebind_t<int, deduced_vec_t<V>>>(val, [](auto x) { return std::fpclassify(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isfinite(const V &val) {
        return implements::map_unary<typename deduced_vec_t<V>::mask_type>(val,
                                                                           [](auto x) { return std::isfinite(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isinf(const V &val) {
        return implements::map_unary<typename deduced_vec_t<V>::mask_type>(val, [](auto x) { return std::isinf(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isnan(const V &val) {
        return implements::map_unary<typename deduced_vec_t<V>::mask_type>(val, [](auto x) { return std::isnan(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isnormal(const V &val) {
        return implements::map_unary<typename deduced_vec_t<V>::mask_type>(val,
                                                                           [](auto x) { return std::isnormal(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type signbit(const V &val) {
        return implements::map_unary<typename deduced_vec_t<V>::mask_type>(val,
                                                                           [](auto x) { return std::signbit(x); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isgreater(const V &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isgreater(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isgreater(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isgreater(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isgreater(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isgreater(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isgreaterequal(const V &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isgreaterequal(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isgreaterequal(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isgreaterequal(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isgreaterequal(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isgreaterequal(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isless(const V &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isless(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isless(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isless(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isless(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isless(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type islessequal(const V &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::islessequal(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type islessequal(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::islessequal(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type islessequal(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::islessequal(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type islessgreater(const V &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::islessgreater(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type islessgreater(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::islessgreater(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type islessgreater(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::islessgreater(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isunordered(const V &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isunordered(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isunordered(const deduced_vec_t<V> &val, const V &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isunordered(a, b); });
    }

    template <typename V, implements::enable_if_math_t<V>>
    RAINY_CONSTEXPR26 typename deduced_vec_t<V>::mask_type isunordered(const V &val, const deduced_vec_t<V> &y) {
        return implements::map_binary<typename deduced_vec_t<V>::mask_type>(
            val, y, [](auto a, auto b) { return std::isunordered(a, b); });
    }
}

#endif
