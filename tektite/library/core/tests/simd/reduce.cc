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
#include <rainy/core/simd.hpp>

namespace rc = rainy::core;
namespace rf = rainy::functional;

using simd_vec_f = rc::vec<float>;
using simd_vec_d = rc::vec<double>;
using simd_vec_f8 = rc::vec<float, 8>;
using simd_vec_f1 = rc::vec<float, 1>;
using simd_vec_i = rc::vec<int>;

namespace {
    template <typename V>
    V iota_vec(typename V::value_type first) {
        using Ty = typename V::value_type;
        return V{[first](auto index) { return static_cast<Ty>(first + static_cast<Ty>(static_cast<int>(index))); }};
    }
}

TEMPLATE_TEST_CASE("simd reductions fold lanes", "[simd][reduce]", simd_vec_f, simd_vec_d, simd_vec_f8, simd_vec_f1,
                   simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;

    const V values = iota_vec<V>(static_cast<typename V::value_type>(1));
    Ty expected_sum{};
    Ty expected_product{1};
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        expected_sum = static_cast<Ty>(expected_sum + values[i]);
        expected_product = static_cast<Ty>(expected_product * values[i]);
    }

    CHECK(rc::reduce(values) == expected_sum);
    CHECK(rc::reduce(values, rf::plus<>()) == expected_sum);
    CHECK(rc::reduce(values, rf::multiplies<>()) == expected_product);

    const typename V::mask_type all_true{true};
    const typename V::mask_type all_false{false};
    CHECK(rc::reduce(values, all_true) == expected_sum);
    CHECK(rc::reduce(values, all_false) == Ty{});
    CHECK(rc::reduce(values, all_true, rf::multiplies<>()) == expected_product);
    CHECK(rc::reduce(values, all_true, static_cast<Ty>(0), rf::plus<>()) == expected_sum);

    const typename V::mask_type even{[](auto index) { return (static_cast<int>(index) & 1) == 0; }};
    Ty expected_even{};
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        if ((i & 1) == 0) {
            expected_even = static_cast<Ty>(expected_even + values[i]);
        }
    }
    CHECK(rc::reduce(values, even) == expected_even);
}

TEMPLATE_TEST_CASE("simd bitwise reductions on integer vectors", "[simd][reduce]", simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    const V values{[](auto index) { return static_cast<Ty>(0xF0 | static_cast<int>(index)); }};
    const typename V::mask_type all_true{true};

    Ty expected_and{};
    Ty expected_or{};
    Ty expected_xor{};
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        if (i == 0) {
            expected_and = values[0];
            expected_or = values[0];
            expected_xor = values[0];
        } else {
            expected_and = static_cast<Ty>(expected_and & values[i]);
            expected_or = static_cast<Ty>(expected_or | values[i]);
            expected_xor = static_cast<Ty>(expected_xor ^ values[i]);
        }
    }

    CHECK(rc::reduce(values, all_true, rf::bit_and<>()) == expected_and);
    CHECK(rc::reduce(values, all_true, rf::bit_or<>()) == expected_or);
    CHECK(rc::reduce(values, all_true, rf::bit_xor<>()) == expected_xor);
}

TEMPLATE_TEST_CASE("simd reduce_min and reduce_max", "[simd][reduce]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    const V values{[](auto index) {
        const int i = static_cast<int>(index);
        return static_cast<Ty>((i % 2 == 0) ? (10 - i) : (20 + i));
    }};

    Ty expected_min = values[0];
    Ty expected_max = values[0];
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        if (values[i] < expected_min) {
            expected_min = values[i];
        }
        if (values[i] > expected_max) {
            expected_max = values[i];
        }
    }
    CHECK(rc::reduce_min(values) == expected_min);
    CHECK(rc::reduce_max(values) == expected_max);

    const typename V::mask_type low_half{[](auto index) { return static_cast<int>(index) < V::size() / 2; }};
    Ty expected_min_masked{};
    Ty expected_max_masked{};
    bool started = false;
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        if (low_half[i]) {
            if (!started) {
                expected_min_masked = values[i];
                expected_max_masked = values[i];
                started = true;
            } else {
                if (values[i] < expected_min_masked) {
                    expected_min_masked = values[i];
                }
                if (values[i] > expected_max_masked) {
                    expected_max_masked = values[i];
                }
            }
        }
    }
    if (started) {
        CHECK(rc::reduce_min(values, low_half) == expected_min_masked);
        CHECK(rc::reduce_max(values, low_half) == expected_max_masked);
    }
}

TEMPLATE_TEST_CASE("simd min max minmax and clamp", "[simd][reduce]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    const V low = iota_vec<V>(static_cast<typename V::value_type>(1));
    const V high = iota_vec<V>(static_cast<typename V::value_type>(3));

    const V smaller = rc::min(low, high);
    const V larger = rc::max(low, high);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        CHECK(smaller[i] == (low[i] < high[i] ? low[i] : high[i]));
        CHECK(larger[i] == (low[i] > high[i] ? low[i] : high[i]));
    }

    const auto pair = rc::minmax(low, high);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("minmax lane " << i);
        CHECK(pair.first[i] == smaller[i]);
        CHECK(pair.second[i] == larger[i]);
    }

    const V value = iota_vec<V>(static_cast<typename V::value_type>(0));
    const V lo = iota_vec<V>(static_cast<typename V::value_type>(2));
    const V hi = iota_vec<V>(static_cast<typename V::value_type>(4));
    const V clamped = rc::clamp(value, lo, hi);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("clamp lane " << i);
        Ty expected = value[i];
        if (expected < lo[i]) {
            expected = lo[i];
        }
        if (expected > hi[i]) {
            expected = hi[i];
        }
        CHECK(clamped[i] == expected);
    }
}
