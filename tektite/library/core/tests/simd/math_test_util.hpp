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
#ifndef RAINY_TOOLKIT_TESTS_SIMD_MATH_TEST_UTIL_HPP
#define RAINY_TOOLKIT_TESTS_SIMD_MATH_TEST_UTIL_HPP

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <limits>
#include <rainy/core/simd.hpp>

namespace rainy::simd_tests {
    namespace rc = rainy::core;

    constexpr double sample_table[16] = {0.0,   0.5,   -0.5,  1.0,   -1.0,  2.5,    -2.5,  0.25,
                                         10.0,  -10.0, 0.75,  -0.75, 3.14159265358979, 0.1, -0.1, 100.0};
    constexpr double edge_table[16] = {0.0,
                                       -0.0,
                                       1.0,
                                       -1.0,
                                       std::numeric_limits<double>::infinity(),
                                       -std::numeric_limits<double>::infinity(),
                                       std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::denorm_min(),
                                       -std::numeric_limits<double>::denorm_min(),
                                       std::numeric_limits<double>::max(),
                                       std::numeric_limits<double>::lowest(),
                                       0.5,
                                       -0.5,
                                       2.0,
                                       -2.0,
                                       100.0};
    constexpr double positive_table[16] = {0.0, 0.5,  1.0,  2.5,   10.0,  0.25,  0.75, 3.14159265358979,
                                           0.1, 100.0, 1.5, 5.0,   7.5,   0.05,  25.0, 4.0};
    constexpr double elliptic_k_table[16] = {0.0,  0.25, -0.5,  0.75,  -0.25, 0.5,  -0.75, 0.1,
                                             -0.1, 0.3,  -0.3,  0.6,   -0.6,  0.8,  -0.8,  0.9};
    constexpr double elliptic_nu_table[16] = {0.0, 0.5,  -0.5, 0.25, -1.0, -2.0, 0.1,  -0.1,
                                              0.4, -0.4, 0.3,  -0.3, 0.2,  -0.2, -0.75, 0.5};

    template <typename V>
    V table_vec(const double (&table)[16], int shift = 0) {
        using Ty = typename V::value_type;
        return V{[&table, shift](auto index) {
            return static_cast<Ty>(table[static_cast<std::size_t>((static_cast<int>(index) + shift) & 15)]);
        }};
    }

    template <typename V>
    V sample_vec(int shift = 0) {
        return table_vec<V>(sample_table, shift);
    }

    template <typename V>
    V edge_vec(int shift = 0) {
        return table_vec<V>(edge_table, shift);
    }

    template <typename V>
    V positive_vec(int shift = 0) {
        return table_vec<V>(positive_table, shift);
    }

    template <typename V>
    V elliptic_k_vec() {
        return table_vec<V>(elliptic_k_table);
    }

    template <typename V>
    V elliptic_nu_vec() {
        return table_vec<V>(elliptic_nu_table);
    }

    template <typename V>
    V special_vec() {
        using Ty = typename V::value_type;
        return V{[](auto index) {
            switch (static_cast<int>(index) % 6) {
                case 0:
                    return Ty(0);
                case 1:
                    return Ty(-1);
                case 2:
                    return std::numeric_limits<Ty>::infinity();
                case 3:
                    return std::numeric_limits<Ty>::quiet_NaN();
                case 4:
                    return -std::numeric_limits<Ty>::infinity();
                default:
                    return std::numeric_limits<Ty>::denorm_min();
            }
        }};
    }

    template <typename A, typename B>
    bool lane_equal(A a, B b) {
        if (std::isnan(a) && std::isnan(b)) {
            return true;
        }
        return a == b;
    }

    template <typename R, typename V, typename F>
    void check_lanes(const R &result, const V &input, F expected) {
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("lane " << i);
            CHECK(lane_equal(result[i], expected(input[i])));
        }
    }

    template <typename R, typename A, typename B, typename F>
    void check_lanes2(const R &result, const A &first, const B &second, F expected) {
        for (rc::simd_size_type i = 0; i < A::size(); ++i) {
            INFO("lane " << i);
            CHECK(lane_equal(result[i], expected(first[i], second[i])));
        }
    }

    template <typename R, typename A, typename B, typename C, typename F>
    void check_lanes3(const R &result, const A &first, const B &second, const C &third, F expected) {
        for (rc::simd_size_type i = 0; i < A::size(); ++i) {
            INFO("lane " << i);
            CHECK(lane_equal(result[i], expected(first[i], second[i], third[i])));
        }
    }

    template <typename R, typename S>
    void check_same(const R &left, const S &right) {
        for (rc::simd_size_type i = 0; i < R::size(); ++i) {
            INFO("lane " << i);
            CHECK(lane_equal(left[i], right[i]));
        }
    }
}

#define SIMD_TEST_SAMPLES(V) rainy::simd_tests::sample_vec<V>(), rainy::simd_tests::edge_vec<V>()
#define CHECK_UNARY(fn) check_lanes(rc::fn(input), input, [](auto x) { return std::fn(x); })
#define CHECK_BINARY(fn) check_lanes2(rc::fn(left, right), left, right, [](auto x, auto y) { return std::fn(x, y); })
#define CHECK_TERNARY(fn)                                                                                    \
    check_lanes3(rc::fn(first, second, third), first, second, third,                                         \
                 [](auto x, auto y, auto z) { return std::fn(x, y, z); })

#endif
