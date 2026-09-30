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
    void check_trigonometric(const V &input) {
        CHECK_UNARY(sin);
        CHECK_UNARY(cos);
        CHECK_UNARY(tan);
        CHECK_UNARY(asin);
        CHECK_UNARY(acos);
        CHECK_UNARY(atan);
        CHECK_UNARY(sinh);
        CHECK_UNARY(cosh);
        CHECK_UNARY(tanh);
        CHECK_UNARY(asinh);
        CHECK_UNARY(acosh);
        CHECK_UNARY(atanh);
    }

    template <typename V>
    void check_atan2(const V &left, const V &right) {
        CHECK_BINARY(atan2);
        check_lanes2(rc::atan2(left, rc::deduced_vec_t<V>(right)), left, right,
                     [](auto x, auto y) { return std::atan2(x, y); });
        check_lanes2(rc::atan2(rc::deduced_vec_t<V>(left), right), left, right,
                     [](auto x, auto y) { return std::atan2(x, y); });
    }
}

TEMPLATE_TEST_CASE("simd trigonometric functions match the scalar library", "[simd][math][trig]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_trigonometric<TestType>(sample_vec<TestType>());
    check_trigonometric<TestType>(edge_vec<TestType>());
}

TEMPLATE_TEST_CASE("simd hyperbolic and atan2 mixed overloads match the scalar library", "[simd][math][trig]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_atan2<TestType>(sample_vec<TestType>(), sample_vec<TestType>(3));
    check_atan2<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5));
}
