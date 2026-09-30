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
    void check_classification(const V &input) {
        check_lanes(rc::fpclassify(input), input, [](auto x) { return std::fpclassify(x); });
        check_lanes(rc::isfinite(input), input, [](auto x) { return std::isfinite(x); });
        check_lanes(rc::isinf(input), input, [](auto x) { return std::isinf(x); });
        check_lanes(rc::isnan(input), input, [](auto x) { return std::isnan(x); });
        check_lanes(rc::isnormal(input), input, [](auto x) { return std::isnormal(x); });
        check_lanes(rc::signbit(input), input, [](auto x) { return std::signbit(x); });
    }

    template <typename V>
    void check_comparisons(const V &left, const V &right) {
        check_lanes2(rc::isgreater(left, right), left, right, [](auto x, auto y) { return std::isgreater(x, y); });
        check_lanes2(rc::isgreaterequal(left, right), left, right,
                     [](auto x, auto y) { return std::isgreaterequal(x, y); });
        check_lanes2(rc::isless(left, right), left, right, [](auto x, auto y) { return std::isless(x, y); });
        check_lanes2(rc::islessequal(left, right), left, right,
                     [](auto x, auto y) { return std::islessequal(x, y); });
        check_lanes2(rc::islessgreater(left, right), left, right,
                     [](auto x, auto y) { return std::islessgreater(x, y); });
        check_lanes2(rc::isunordered(left, right), left, right,
                     [](auto x, auto y) { return std::isunordered(x, y); });
    }

    template <typename V>
    void check_comparison_mixed(const V &a, const V &b) {
        using D = rc::deduced_vec_t<V>;
        check_same(rc::isgreater(D(a), b), rc::isgreater(a, b));
        check_same(rc::isgreater(a, D(b)), rc::isgreater(a, b));
        check_same(rc::isgreaterequal(D(a), b), rc::isgreaterequal(a, b));
        check_same(rc::isgreaterequal(a, D(b)), rc::isgreaterequal(a, b));
        check_same(rc::isless(D(a), b), rc::isless(a, b));
        check_same(rc::isless(a, D(b)), rc::isless(a, b));
        check_same(rc::islessequal(D(a), b), rc::islessequal(a, b));
        check_same(rc::islessequal(a, D(b)), rc::islessequal(a, b));
        check_same(rc::islessgreater(D(a), b), rc::islessgreater(a, b));
        check_same(rc::islessgreater(a, D(b)), rc::islessgreater(a, b));
        check_same(rc::isunordered(D(a), b), rc::isunordered(a, b));
        check_same(rc::isunordered(a, D(b)), rc::isunordered(a, b));
    }
}

TEMPLATE_TEST_CASE("simd classification functions match the scalar library", "[simd][math][classify]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_classification<TestType>(special_vec<TestType>());
    check_classification<TestType>(edge_vec<TestType>());
    check_classification<TestType>(sample_vec<TestType>());
}

TEMPLATE_TEST_CASE("simd comparison predicates match the scalar library", "[simd][math][classify]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_comparisons<TestType>(sample_vec<TestType>(), sample_vec<TestType>(5));
    check_comparisons<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5));
    check_comparisons<TestType>(sample_vec<TestType>(), special_vec<TestType>());
    check_comparisons<TestType>(special_vec<TestType>(), sample_vec<TestType>());
}

TEMPLATE_TEST_CASE("simd comparison mixed deduced-vec overloads agree with plain overloads", "[simd][math][classify]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_comparison_mixed<TestType>(sample_vec<TestType>(), sample_vec<TestType>(5));
    check_comparison_mixed<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5));
}
