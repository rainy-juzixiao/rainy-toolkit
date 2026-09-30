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

using simd_vec_f = rc::vec<float>;
using simd_vec_d = rc::vec<double>;
using simd_vec_f8 = rc::vec<float, 8>;
using simd_vec_f1 = rc::vec<float, 1>;
using simd_vec_i = rc::vec<int>;

namespace {
    template <typename V>
    V pattern_vec() {
        using Ty = typename V::value_type;
        return V{[](auto index) { return static_cast<Ty>(static_cast<int>(index) % 3); }};
    }
}

TEMPLATE_TEST_CASE("basic_mask construction and indexing", "[simd][mask]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using mask_type = typename V::mask_type;

    const mask_type all{true};
    const mask_type none{false};
    for (rc::simd_size_type i = 0; i < mask_type::size(); ++i) {
        INFO("lane " << i);
        CHECK(all[i]);
        CHECK(!none[i]);
    }

    const mask_type even{[](auto index) { return (static_cast<int>(index) & 1) == 0; }};
    for (rc::simd_size_type i = 0; i < mask_type::size(); ++i) {
        INFO("even lane " << i);
        CHECK(even[i] == ((i & 1) == 0));
    }

    CHECK(rc::all_of(even) == (mask_type::size() == 1 ? true : false));
    CHECK(rc::any_of(even));
    CHECK(!rc::none_of(even));
    CHECK(rc::none_of(none));
    CHECK(!rc::any_of(none));
    CHECK(rc::all_of(all));
    CHECK(rc::reduce_count(even) == (mask_type::size() + 1) / 2);
    CHECK(rc::reduce_count(none) == 0);
    CHECK(rc::reduce_min_index(even) == 0);
    CHECK(rc::reduce_max_index(even) == (mask_type::size() % 2 == 0 ? mask_type::size() - 2 : mask_type::size() - 1));
}

TEMPLATE_TEST_CASE("basic_mask logic and comparison operators", "[simd][mask]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using mask_type = typename TestType::mask_type;

    const mask_type even{[](auto index) { return (static_cast<int>(index) & 1) == 0; }};
    const mask_type low{[](auto index) { return static_cast<int>(index) < 2; }};

    const auto both = even && low;
    const auto either = even || low;
    const auto band = even & low;
    const auto bor = even | low;
    const auto bxor = even ^ low;
    const auto inverted = !even;

    for (rc::simd_size_type i = 0; i < mask_type::size(); ++i) {
        INFO("lane " << i);
        const bool e = (i & 1) == 0;
        const bool l = i < 2;
        CHECK(both[i] == (e && l));
        CHECK(either[i] == (e || l));
        CHECK(band[i] == (e && l));
        CHECK(bor[i] == (e || l));
        CHECK(bxor[i] == (e != l));
        CHECK(inverted[i] == !e);
    }

    const auto equal = even == even;
    const auto not_equal = even != even;
    const auto greater = even >= low;
    const auto less = even < low;

    for (rc::simd_size_type i = 0; i < mask_type::size(); ++i) {
        INFO("compare lane " << i);
        CHECK(equal[i]);
        CHECK(!not_equal[i]);
        CHECK(greater[i] == (even[i] >= low[i]));
        CHECK(less[i] == (even[i] < low[i]));
    }

    mask_type assigned{false};
    assigned = even;
    assigned |= low;
    CHECK(assigned[0]);
    assigned &= even;
    assigned ^= even;
    for (rc::simd_size_type i = 0; i < mask_type::size(); ++i) {
        INFO("compound lane " << i);
        CHECK(!assigned[i]);
    }
}

TEMPLATE_TEST_CASE("basic_mask conversions and select", "[simd][mask]", simd_vec_f, simd_vec_d, simd_vec_f8,
                   simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using mask_type = typename V::mask_type;
    using Ty = typename V::value_type;

    const V values = pattern_vec<V>();
    const mask_type mask = values < V(static_cast<Ty>(1));

    const V from_mask = static_cast<V>(mask);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        CHECK(from_mask[i] == (mask[i] ? static_cast<Ty>(1) : static_cast<Ty>(0)));
    }

    const V high{static_cast<Ty>(42)};
    const V low{static_cast<Ty>(-42)};
    const V picked = rc::select(mask, high, low);
    const mask_type all_false{false};
    const mask_type picked_left = rc::select(mask, mask, all_false);
    const mask_type picked_right = rc::select(mask, all_false, !mask);
    const bool bool_picked = rc::select(true, true, false);

    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        CHECK(picked[i] == (mask[i] ? static_cast<Ty>(42) : static_cast<Ty>(-42)));
        CHECK(picked_left[i] == mask[i]);
        CHECK(picked_right[i] == !mask[i]);
    }
    CHECK(bool_picked);

    const mask_type from_small{[](auto index) { return static_cast<int>(index) == 0; }};
    const mask_type converted{from_small};
    for (rc::simd_size_type i = 0; i < mask_type::size(); ++i) {
        INFO("convert lane " << i);
        CHECK(converted[i] == from_small[i]);
    }

    STATIC_REQUIRE(!std::is_constructible_v<mask_type, bool, bool>);
    STATIC_REQUIRE(rc::all_of(static_cast<bool>(true)));
    CHECK(rc::reduce_count(true) == 1);
    CHECK(rc::reduce_count(false) == 0);
    CHECK(!rc::any_of(false));
    CHECK(rc::none_of(false));
}
