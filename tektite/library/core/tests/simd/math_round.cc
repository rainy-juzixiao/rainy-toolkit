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
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include "math_test_util.hpp"

namespace rc = rainy::core;
using namespace rainy::simd_tests;

using simd_vec_f = rc::vec<float>;
using simd_vec_d = rc::vec<double>;
using simd_vec_f8 = rc::vec<float, 8>;
using simd_vec_f1 = rc::vec<float, 1>;

namespace {
    template <typename V>
    void check_rounding(const V &input) {
        CHECK_UNARY(sqrt);
        CHECK_UNARY(cbrt);
        CHECK_UNARY(fabs);
        CHECK_UNARY(abs);
        CHECK_UNARY(ceil);
        CHECK_UNARY(floor);
        CHECK_UNARY(trunc);
        CHECK_UNARY(round);
        CHECK_UNARY(nearbyint);
        CHECK_UNARY(rint);
        check_lanes(rc::lrint(input), input, [](auto x) { return std::lrint(x); });
        check_lanes(rc::llrint(input), input, [](auto x) { return std::llrint(x); });
        check_lanes(rc::lround(input), input, [](auto x) { return std::lround(x); });
        check_lanes(rc::llround(input), input, [](auto x) { return std::llround(x); });
    }

    template <typename V>
    void check_abs_and_lane_order() {
        using Ty = typename V::value_type;
        const V signed_values = sample_vec<V>();
        const V absolute = rc::abs(signed_values);
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("lane " << i);
            const Ty expected = signed_values[i] < Ty(0) ? Ty(-signed_values[i]) : signed_values[i];
            CHECK(absolute[i] == expected);
            CHECK(absolute[i] >= Ty(0));
        }

        const V squares{[](auto index) {
            const Ty lane = static_cast<Ty>(static_cast<int>(index) + 1);
            return static_cast<Ty>(lane * lane);
        }};
        const V roots = rc::sqrt(squares);
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("lane " << i);
            CHECK(roots[i] == static_cast<Ty>(i + 1));
        }
    }

    void check_integer_abs() {
        using int_vec = rc::vec<int>;
        const int_vec integers{[](auto index) {
            const int i = static_cast<int>(index);
            return (i & 1) ? -(i + 1) : (i + 1);
        }};
        const int_vec absolute = rc::abs(integers);
        for (rc::simd_size_type i = 0; i < int_vec::size(); ++i) {
            INFO("lane " << i);
            CHECK(absolute[i] == (integers[i] < 0 ? -integers[i] : integers[i]));
            CHECK(absolute[i] >= 0);
        }

        const rc::vec<signed char> bytes{[](auto index) {
            const int i = static_cast<int>(index);
            return static_cast<signed char>((i & 1) ? -(i + 3) : (i + 3));
        }};
        const auto absolute_bytes = rc::abs(bytes);
        for (rc::simd_size_type i = 0; i < rc::vec<signed char>::size(); ++i) {
            INFO("byte lane " << i);
            CHECK(absolute_bytes[i] == (bytes[i] < 0 ? static_cast<signed char>(-bytes[i]) : bytes[i]));
        }
    }
}

TEMPLATE_TEST_CASE("simd root and rounding functions match the scalar library", "[simd][math][round]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_rounding<TestType>(sample_vec<TestType>());
    check_rounding<TestType>(edge_vec<TestType>());
}

TEMPLATE_TEST_CASE("simd abs preserves lanes and sqrt keeps lane order", "[simd][math][round]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_abs_and_lane_order<TestType>();
}

TEST_CASE("simd integral abs handles signed narrow and wide elements", "[simd][math][round]") {
    check_integer_abs();
}
