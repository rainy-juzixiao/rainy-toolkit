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
#include <cstdint>
#include <type_traits>
#include <rainy/core/simd.hpp>

namespace rc = rainy::core;

using simd_vec_f = rc::vec<float>;
using simd_vec_d = rc::vec<double>;
using simd_vec_f8 = rc::vec<float, 8>;
using simd_vec_f1 = rc::vec<float, 1>;
using simd_vec_i = rc::vec<int>;
using simd_vec_u = rc::vec<unsigned>;
using simd_vec_s8 = rc::vec<signed char>;

namespace {
    template <typename V>
    V iota_vec(typename V::value_type first = static_cast<typename V::value_type>(1)) {
        using Ty = typename V::value_type;
        return V{[first](auto index) { return static_cast<Ty>(first + static_cast<Ty>(static_cast<int>(index))); }};
    }
}

TEMPLATE_TEST_CASE("basic_vec scalar and generator construction", "[simd][basic]", simd_vec_f, simd_vec_d,
                   simd_vec_f8, simd_vec_f1, simd_vec_i, simd_vec_u, simd_vec_s8) {
    using V = TestType;
    using Ty = typename V::value_type;

    const V broadcast{static_cast<Ty>(3)};
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("broadcast lane " << i);
        CHECK(broadcast[i] == static_cast<Ty>(3));
    }

    const V generated = iota_vec<V>();
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("generator lane " << i);
        CHECK(generated[i] == static_cast<Ty>(i + 1));
    }

    const V copied{generated};
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("copy lane " << i);
        CHECK(copied[i] == generated[i]);
    }

    const V negated = -generated;
    const V identity = +generated;
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("unary lane " << i);
        CHECK(negated[i] == static_cast<Ty>(-generated[i]));
        CHECK(identity[i] == generated[i]);
    }
}

TEMPLATE_TEST_CASE("basic_vec arithmetic bitwise and shift operators", "[simd][basic]", simd_vec_i, simd_vec_u) {
    using V = TestType;
    using Ty = typename V::value_type;
    const V left = iota_vec<V>(1);
    const V right = iota_vec<V>(2);

    const auto sum = left + right;
    const auto diff = left - right;
    const auto product = left * right;
    const auto quotient = right / left;
    const auto remainder = right % left;
    const auto bit_and = left & right;
    const auto bit_or = left | right;
    const auto bit_xor = left ^ right;
    const auto shift_left = left << V(static_cast<Ty>(1));
    const auto shift_right = left >> V(static_cast<Ty>(1));
    const auto scalar_shift_left = left << 1;
    const auto scalar_shift_right = left >> 1;
    const auto bit_not = ~left;

    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        const Ty a = static_cast<Ty>(i + 1);
        const Ty b = static_cast<Ty>(i + 2);
        CHECK(sum[i] == static_cast<Ty>(a + b));
        CHECK(diff[i] == static_cast<Ty>(a - b));
        CHECK(product[i] == static_cast<Ty>(a * b));
        CHECK(quotient[i] == static_cast<Ty>(b / a));
        CHECK(remainder[i] == static_cast<Ty>(b % a));
        CHECK(bit_and[i] == static_cast<Ty>(a & b));
        CHECK(bit_or[i] == static_cast<Ty>(a | b));
        CHECK(bit_xor[i] == static_cast<Ty>(a ^ b));
        CHECK(bit_not[i] == static_cast<Ty>(~a));
        CHECK(shift_left[i] == static_cast<Ty>(a << 1));
        CHECK(shift_right[i] == static_cast<Ty>(a >> 1));
        CHECK(scalar_shift_left[i] == static_cast<Ty>(a << 1));
        CHECK(scalar_shift_right[i] == static_cast<Ty>(a >> 1));
    }

    V compound = left;
    compound += right;
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("+= lane " << i);
        CHECK(compound[i] == static_cast<Ty>(static_cast<Ty>(i + 1) + static_cast<Ty>(i + 2)));
    }
    compound -= right;
    compound *= V(static_cast<Ty>(2));
    compound &= right;
    compound |= right;
    compound ^= right;
    compound <<= 1;
    compound >>= 1;

    V counter = iota_vec<V>(0);
    ++counter;
    CHECK(counter[0] == static_cast<Ty>(1));
    counter++;
    CHECK(counter[0] == static_cast<Ty>(2));
    --counter;
    CHECK(counter[0] == static_cast<Ty>(1));
    counter--;
    CHECK(counter[0] == static_cast<Ty>(0));
}

TEMPLATE_TEST_CASE("basic_vec comparisons produce masks", "[simd][basic]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    const V left = iota_vec<V>(1);
    const V right = iota_vec<V>(2);

    const auto eq = left == left;
    const auto ne = left != left;
    const auto lt = left < right;
    const auto le = left <= right;
    const auto gt = left > right;
    const auto ge = left >= right;

    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        CHECK(eq[i]);
        CHECK(!ne[i]);
        CHECK(lt[i]);
        CHECK(le[i]);
        CHECK(!gt[i]);
        CHECK(!ge[i]);
    }

    const V zero{static_cast<Ty>(0)};
    const auto logical_not = !zero;
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("not lane " << i);
        CHECK(logical_not[i]);
    }
}

TEMPLATE_TEST_CASE("basic_vec select picks lanes by mask", "[simd][basic]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    const V low = iota_vec<V>(0);
    const V high = iota_vec<V>(100);
    const auto mask = low < V(static_cast<Ty>(2));

    const V selected = rc::select(mask, high, low);
    const V scalar_selected = rc::select(true, high, low);
    const V false_selected = rc::select(false, high, low);

    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        if (i < 2) {
            CHECK(selected[i] == high[i]);
        } else {
            CHECK(selected[i] == low[i]);
        }
        CHECK(scalar_selected[i] == high[i]);
        CHECK(false_selected[i] == low[i]);
    }
}

TEST_CASE("basic_vec cross element type conversions", "[simd][basic]") {
    const simd_vec_f floats = simd_vec_f{[](auto index) {
        return static_cast<float>(static_cast<int>(index) + 1) * 1.5f;
    }};

    const simd_vec_i explicit_from_float(floats);
    for (rc::simd_size_type i = 0; i < simd_vec_f::size(); ++i) {
        INFO("float to int lane " << i);
        CHECK(explicit_from_float[i] == static_cast<int>(floats[i]));
    }

    const simd_vec_f explicit_from_int(explicit_from_float);
    for (rc::simd_size_type i = 0; i < simd_vec_f::size(); ++i) {
        INFO("int to float lane " << i);
        CHECK(explicit_from_int[i] == static_cast<float>(explicit_from_float[i]));
    }

    const simd_vec_u unsigned_from_int(simd_vec_i{[](auto index) { return static_cast<int>(index) - 2; }});
    for (rc::simd_size_type i = 0; i < simd_vec_i::size(); ++i) {
        INFO("int to uint lane " << i);
        const int source = static_cast<int>(i) - 2;
        CHECK(unsigned_from_int[i] == static_cast<unsigned>(source));
    }

    STATIC_REQUIRE(std::is_constructible_v<simd_vec_i, simd_vec_f>);
    STATIC_REQUIRE(!std::is_constructible_v<simd_vec_f, simd_vec_f1>);
    STATIC_REQUIRE(!std::is_constructible_v<simd_vec_f1, simd_vec_f>);
    STATIC_REQUIRE(!std::is_constructible_v<simd_vec_f, int>);
    STATIC_REQUIRE(!std::is_constructible_v<simd_vec_f, double>);
    STATIC_REQUIRE(std::is_constructible_v<simd_vec_f, float>);
    STATIC_REQUIRE(std::is_constructible_v<simd_vec_f, decltype(1.0f)>);
}

TEST_CASE("basic_vec mask and value interop", "[simd][basic]") {
    const simd_vec_f values = simd_vec_f{[](auto index) { return static_cast<float>(static_cast<int>(index) % 2); }};
    const simd_vec_f::mask_type mask = values == simd_vec_f{1.0f};

    const simd_vec_f from_mask = static_cast<simd_vec_f>(mask);
    for (rc::simd_size_type i = 0; i < simd_vec_f::size(); ++i) {
        INFO("lane " << i);
        CHECK(from_mask[i] == (mask[i] ? 1.0f : 0.0f));
    }

    const simd_vec_i::mask_type int_mask = simd_vec_i{[](auto index) {
        return static_cast<int>(static_cast<int>(index) % 2);
    }} == simd_vec_i{1};
    const simd_vec_i mask_as_int = static_cast<simd_vec_i>(int_mask);
    for (rc::simd_size_type i = 0; i < simd_vec_i::size(); ++i) {
        INFO("int lane " << i);
        CHECK(mask_as_int[i] == (int_mask[i] ? 1 : 0));
    }
}
