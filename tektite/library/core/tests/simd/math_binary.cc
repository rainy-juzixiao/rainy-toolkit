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
    void check_binary(const V &left, const V &right) {
        CHECK_BINARY(atan2);
        CHECK_BINARY(hypot);
        CHECK_BINARY(pow);
        CHECK_BINARY(fmod);
        CHECK_BINARY(remainder);
        CHECK_BINARY(copysign);
        CHECK_BINARY(nextafter);
        CHECK_BINARY(fdim);
        CHECK_BINARY(fmax);
        CHECK_BINARY(fmin);
    }

    template <typename V>
    void check_ternary(const V &first, const V &second, const V &third) {
        CHECK_TERNARY(fma);
        CHECK_TERNARY(lerp);
        check_lanes3(rc::hypot(first, second, third), first, second, third,
                     [](auto x, auto y, auto z) { return std::hypot(x, y, z); });
    }

    template <typename V>
    void check_binary_mixed(const V &a, const V &b) {
        using D = rc::deduced_vec_t<V>;
        check_same(rc::atan2(D(a), b), rc::atan2(a, b));
        check_same(rc::atan2(a, D(b)), rc::atan2(a, b));
        check_same(rc::hypot(D(a), b), rc::hypot(a, b));
        check_same(rc::hypot(a, D(b)), rc::hypot(a, b));
        check_same(rc::pow(D(a), b), rc::pow(a, b));
        check_same(rc::pow(a, D(b)), rc::pow(a, b));
        check_same(rc::fmod(D(a), b), rc::fmod(a, b));
        check_same(rc::fmod(a, D(b)), rc::fmod(a, b));
        check_same(rc::remainder(D(a), b), rc::remainder(a, b));
        check_same(rc::remainder(a, D(b)), rc::remainder(a, b));
        check_same(rc::copysign(D(a), b), rc::copysign(a, b));
        check_same(rc::copysign(a, D(b)), rc::copysign(a, b));
        check_same(rc::nextafter(D(a), b), rc::nextafter(a, b));
        check_same(rc::nextafter(a, D(b)), rc::nextafter(a, b));
        check_same(rc::fdim(D(a), b), rc::fdim(a, b));
        check_same(rc::fdim(a, D(b)), rc::fdim(a, b));
        check_same(rc::fmax(D(a), b), rc::fmax(a, b));
        check_same(rc::fmax(a, D(b)), rc::fmax(a, b));
        check_same(rc::fmin(D(a), b), rc::fmin(a, b));
        check_same(rc::fmin(a, D(b)), rc::fmin(a, b));

        rc::rebind_t<int, V> quo_first{};
        rc::rebind_t<int, V> quo_plain{};
        check_same(rc::remquo(D(a), b, &quo_first), rc::remquo(a, b, &quo_plain));
        check_same(quo_first, quo_plain);
        check_same(rc::remquo(a, D(b), &quo_first), rc::remquo(a, b, &quo_plain));
        check_same(quo_first, quo_plain);
    }

    template <typename V>
    void check_ternary_mixed(const V &a, const V &b, const V &c) {
        using D = rc::deduced_vec_t<V>;
        check_same(rc::hypot(D(a), b, c), rc::hypot(a, b, c));
        check_same(rc::hypot(a, D(b), c), rc::hypot(a, b, c));
        check_same(rc::hypot(a, b, D(c)), rc::hypot(a, b, c));
        check_same(rc::hypot(D(a), D(b), c), rc::hypot(a, b, c));
        check_same(rc::hypot(D(a), b, D(c)), rc::hypot(a, b, c));
        check_same(rc::hypot(a, D(b), D(c)), rc::hypot(a, b, c));

        check_same(rc::fma(D(a), b, c), rc::fma(a, b, c));
        check_same(rc::fma(a, D(b), c), rc::fma(a, b, c));
        check_same(rc::fma(a, b, D(c)), rc::fma(a, b, c));
        check_same(rc::fma(D(a), D(b), c), rc::fma(a, b, c));
        check_same(rc::fma(D(a), b, D(c)), rc::fma(a, b, c));
        check_same(rc::fma(a, D(b), D(c)), rc::fma(a, b, c));

        check_same(rc::lerp(D(a), b, c), rc::lerp(a, b, c));
        check_same(rc::lerp(a, D(b), c), rc::lerp(a, b, c));
        check_same(rc::lerp(a, b, D(c)), rc::lerp(a, b, c));
        check_same(rc::lerp(D(a), D(b), c), rc::lerp(a, b, c));
        check_same(rc::lerp(D(a), b, D(c)), rc::lerp(a, b, c));
        check_same(rc::lerp(a, D(b), D(c)), rc::lerp(a, b, c));
    }
}

TEMPLATE_TEST_CASE("simd binary functions match the scalar library", "[simd][math][binary]", simd_vec_f, simd_vec_d,
                   simd_vec_f8, simd_vec_f1) {
    check_binary<TestType>(sample_vec<TestType>(), sample_vec<TestType>(3));
    check_binary<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5));
}

TEMPLATE_TEST_CASE("simd ternary functions match the scalar library", "[simd][math][binary]", simd_vec_f, simd_vec_d,
                   simd_vec_f8, simd_vec_f1) {
    check_ternary<TestType>(sample_vec<TestType>(), sample_vec<TestType>(3), sample_vec<TestType>(7));
    check_ternary<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5), edge_vec<TestType>(9));
}

TEMPLATE_TEST_CASE("simd binary mixed deduced-vec overloads agree with plain overloads", "[simd][math][binary]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_binary_mixed<TestType>(sample_vec<TestType>(), sample_vec<TestType>(3));
    check_binary_mixed<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5));
}

TEMPLATE_TEST_CASE("simd ternary mixed deduced-vec overloads agree with plain overloads", "[simd][math][binary]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_ternary_mixed<TestType>(sample_vec<TestType>(), sample_vec<TestType>(3), sample_vec<TestType>(7));
    check_ternary_mixed<TestType>(edge_vec<TestType>(), edge_vec<TestType>(5), edge_vec<TestType>(9));
}
