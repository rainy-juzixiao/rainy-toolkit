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
    void check_exp_log(const V &input) {
        CHECK_UNARY(exp);
        CHECK_UNARY(exp2);
        CHECK_UNARY(expm1);
        CHECK_UNARY(log);
        CHECK_UNARY(log10);
        CHECK_UNARY(log1p);
        CHECK_UNARY(log2);
        CHECK_UNARY(logb);
    }

    template <typename V>
    void check_exponent_functions(const V &input) {
        using Ty = typename V::value_type;
        using int_vec = rc::rebind_t<int, V>;

        int_vec exp{};
        const V mantissa = rc::frexp(input, &exp);
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("frexp lane " << i);
            int expected_exponent{};
            const Ty expected_mantissa = std::frexp(input[i], &expected_exponent);
            CHECK(lane_equal(mantissa[i], expected_mantissa));
            CHECK(exp[i] == expected_exponent);
        }

        const int_vec power{[](auto index) { return static_cast<int>(static_cast<int>(index) % 5) - 2; }};
        check_lanes2(rc::ldexp(input, power), input, power, [](auto x, auto n) { return std::ldexp(x, n); });
        check_lanes2(rc::scalbn(input, power), input, power, [](auto x, auto n) { return std::scalbn(x, n); });

        const rc::rebind_t<long int, V> long_power{
            [](auto index) { return static_cast<long int>(static_cast<int>(index) % 5) - 2; }};
        check_lanes2(rc::scalbln(input, long_power), input, long_power,
                     [](auto x, auto n) { return std::scalbln(x, n); });

        check_lanes(rc::ilogb(input), input, [](auto x) { return std::ilogb(x); });
        check_lanes(rc::logb(input), input, [](auto x) { return std::logb(x); });

        V iptr{};
        const V fraction = rc::modf(input, &iptr);
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("modf lane " << i);
            Ty expected_iptr{};
            const Ty expected_fraction = std::modf(input[i], &expected_iptr);
            CHECK(lane_equal(fraction[i], expected_fraction));
            CHECK(lane_equal(iptr[i], expected_iptr));
        }

        const V other = sample_vec<V>(5);
        int_vec quo{};
        const V remainder_value = rc::remquo(input, other, &quo);
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("remquo lane " << i);
            int expected_quo{};
            const Ty expected_remainder = std::remquo(input[i], other[i], &expected_quo);
            CHECK(lane_equal(remainder_value[i], expected_remainder));
            CHECK(quo[i] == expected_quo);
        }
    }
}

TEMPLATE_TEST_CASE("simd exponential and logarithmic functions match the scalar library", "[simd][math][exp]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_exp_log<TestType>(sample_vec<TestType>());
    check_exp_log<TestType>(edge_vec<TestType>());
}

TEMPLATE_TEST_CASE("simd exponent decomposition functions match the scalar library", "[simd][math][exp]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_exponent_functions<TestType>(sample_vec<TestType>(1));
    check_exponent_functions<TestType>(edge_vec<TestType>(1));
}
