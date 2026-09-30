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
    void check_error_gamma(const V &input) {
        CHECK_UNARY(erf);
        CHECK_UNARY(erfc);
        CHECK_UNARY(lgamma);
        CHECK_UNARY(tgamma);
    }

    template <typename V>
    void check_special_math(const V &input) {
        using unsigned_vec = rc::rebind_t<unsigned, V>;
        const V other = sample_vec<V>(4);
        const V third = sample_vec<V>(8);
        const V positive = positive_vec<V>();
        const V elliptic_k = elliptic_k_vec<V>();
        const V elliptic_nu = elliptic_nu_vec<V>();
        const unsigned_vec order{[](auto index) { return static_cast<unsigned>(static_cast<int>(index) & 3); }};

        check_lanes3(rc::assoc_laguerre(order, order, positive), order, order, positive,
                     [](auto n, auto m, auto x) { return std::assoc_laguerre(n, m, x); });
        check_lanes3(rc::assoc_legendre(order, order, input), order, order, input,
                     [](auto n, auto m, auto x) { return std::assoc_legendre(n, m, x); });
        check_lanes2(rc::beta(input, other), input, other, [](auto x, auto y) { return std::beta(x, y); });
        check_lanes(rc::comp_ellint_1(elliptic_k), elliptic_k, [](auto x) { return std::comp_ellint_1(x); });
        check_lanes(rc::comp_ellint_2(elliptic_k), elliptic_k, [](auto x) { return std::comp_ellint_2(x); });
        check_lanes2(rc::comp_ellint_3(elliptic_k, elliptic_nu), elliptic_k, elliptic_nu,
                     [](auto x, auto y) { return std::comp_ellint_3(x, y); });
        check_lanes2(rc::cyl_bessel_i(positive, positive), positive, positive,
                     [](auto nu, auto x) { return std::cyl_bessel_i(nu, x); });
        check_lanes2(rc::cyl_bessel_j(positive, positive), positive, positive,
                     [](auto nu, auto x) { return std::cyl_bessel_j(nu, x); });
        check_lanes2(rc::cyl_bessel_k(positive, positive), positive, positive,
                     [](auto nu, auto x) { return std::cyl_bessel_k(nu, x); });
        check_lanes2(rc::cyl_neumann(positive, positive), positive, positive,
                     [](auto nu, auto x) { return std::cyl_neumann(nu, x); });
        check_lanes2(rc::ellint_1(elliptic_k, input), elliptic_k, input,
                     [](auto k, auto phi) { return std::ellint_1(k, phi); });
        check_lanes2(rc::ellint_2(elliptic_k, input), elliptic_k, input,
                     [](auto k, auto phi) { return std::ellint_2(k, phi); });
        check_lanes3(rc::ellint_3(elliptic_k, elliptic_nu, third), elliptic_k, elliptic_nu, third,
                     [](auto k, auto nu, auto phi) { return std::ellint_3(k, nu, phi); });
        check_lanes(rc::expint(input), input, [](auto x) { return std::expint(x); });
        check_lanes2(rc::hermite(order, input), order, input, [](auto n, auto x) { return std::hermite(n, x); });
        check_lanes2(rc::laguerre(order, positive), order, positive, [](auto n, auto x) { return std::laguerre(n, x); });
        check_lanes2(rc::legendre(order, input), order, input, [](auto n, auto x) { return std::legendre(n, x); });
        check_lanes(rc::riemann_zeta(input), input, [](auto x) { return std::riemann_zeta(x); });
        check_lanes2(rc::sph_bessel(order, positive), order, positive,
                     [](auto n, auto x) { return std::sph_bessel(n, x); });
        check_lanes3(rc::sph_legendre(order, order, input), order, order, input,
                     [](auto l, auto m, auto theta) { return std::sph_legendre(l, m, theta); });
        check_lanes2(rc::sph_neumann(order, positive), order, positive,
                     [](auto n, auto x) { return std::sph_neumann(n, x); });
    }

    template <typename V>
    void check_special_mixed(const V &a, const V &b) {
        using D = rc::deduced_vec_t<V>;
        const V positive = positive_vec<V>();
        const V elliptic_k = elliptic_k_vec<V>();
        const V elliptic_nu = elliptic_nu_vec<V>();

        check_same(rc::beta(D(a), b), rc::beta(a, b));
        check_same(rc::beta(a, D(b)), rc::beta(a, b));
        check_same(rc::comp_ellint_3(D(elliptic_k), elliptic_nu), rc::comp_ellint_3(elliptic_k, elliptic_nu));
        check_same(rc::comp_ellint_3(elliptic_k, D(elliptic_nu)), rc::comp_ellint_3(elliptic_k, elliptic_nu));
        check_same(rc::cyl_bessel_i(D(positive), positive), rc::cyl_bessel_i(positive, positive));
        check_same(rc::cyl_bessel_i(positive, D(positive)), rc::cyl_bessel_i(positive, positive));
        check_same(rc::cyl_bessel_j(D(positive), positive), rc::cyl_bessel_j(positive, positive));
        check_same(rc::cyl_bessel_j(positive, D(positive)), rc::cyl_bessel_j(positive, positive));
        check_same(rc::cyl_bessel_k(D(positive), positive), rc::cyl_bessel_k(positive, positive));
        check_same(rc::cyl_bessel_k(positive, D(positive)), rc::cyl_bessel_k(positive, positive));
        check_same(rc::cyl_neumann(D(positive), positive), rc::cyl_neumann(positive, positive));
        check_same(rc::cyl_neumann(positive, D(positive)), rc::cyl_neumann(positive, positive));
        check_same(rc::ellint_1(D(elliptic_k), a), rc::ellint_1(elliptic_k, a));
        check_same(rc::ellint_1(elliptic_k, D(a)), rc::ellint_1(elliptic_k, a));
        check_same(rc::ellint_2(D(elliptic_k), a), rc::ellint_2(elliptic_k, a));
        check_same(rc::ellint_2(elliptic_k, D(a)), rc::ellint_2(elliptic_k, a));
        check_same(rc::ellint_3(D(elliptic_k), elliptic_nu, a), rc::ellint_3(elliptic_k, elliptic_nu, a));
        check_same(rc::ellint_3(elliptic_k, D(elliptic_nu), a), rc::ellint_3(elliptic_k, elliptic_nu, a));
        check_same(rc::ellint_3(elliptic_k, elliptic_nu, D(a)), rc::ellint_3(elliptic_k, elliptic_nu, a));
        check_same(rc::ellint_3(D(elliptic_k), D(elliptic_nu), a), rc::ellint_3(elliptic_k, elliptic_nu, a));
        check_same(rc::ellint_3(D(elliptic_k), elliptic_nu, D(a)), rc::ellint_3(elliptic_k, elliptic_nu, a));
        check_same(rc::ellint_3(elliptic_k, D(elliptic_nu), D(a)), rc::ellint_3(elliptic_k, elliptic_nu, a));
    }
}

TEMPLATE_TEST_CASE("simd error and gamma functions match the scalar library", "[simd][math][special]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_error_gamma<TestType>(sample_vec<TestType>());
    check_error_gamma<TestType>(edge_vec<TestType>());
}

TEMPLATE_TEST_CASE("simd special math functions match the scalar library", "[simd][math][special]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_special_math<TestType>(sample_vec<TestType>(1));
    check_special_math<TestType>(positive_vec<TestType>(1));
}

TEMPLATE_TEST_CASE("simd special mixed deduced-vec overloads agree with plain overloads", "[simd][math][special]",
                   simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1) {
    check_special_mixed<TestType>(sample_vec<TestType>(), sample_vec<TestType>(3));
    check_special_mixed<TestType>(positive_vec<TestType>(), positive_vec<TestType>(3));
}
