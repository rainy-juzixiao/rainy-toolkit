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
#include <cstddef>
#include <vector>
#include <rainy/core/simd.hpp>

namespace rc = rainy::core;

using simd_vec_f = rc::vec<float>;
using simd_vec_d = rc::vec<double>;
using simd_vec_f8 = rc::vec<float, 8>;
using simd_vec_f1 = rc::vec<float, 1>;
using simd_vec_i = rc::vec<int>;

namespace {
    template <typename Ty>
    void fill_buffer(Ty *buffer, int count) {
        for (int i = 0; i < count; ++i) {
            buffer[i] = static_cast<Ty>(i + 1);
        }
    }
}

TEMPLATE_TEST_CASE("simd unchecked and partial load from contiguous storage", "[simd][load_store]", simd_vec_f,
                   simd_vec_d, simd_vec_f8, simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    Ty buffer[32] = {};
    fill_buffer(buffer, 32);

    const V full = rc::unchecked_load<V>(buffer, V::size());
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("full lane " << i);
        CHECK(full[i] == static_cast<Ty>(i + 1));
    }

    const V flagged = rc::unchecked_load<V>(buffer, V::size(), rc::flag_default);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("flagged lane " << i);
        CHECK(flagged[i] == static_cast<Ty>(i + 1));
    }

    const V partial = rc::partial_load<V>(buffer, 0);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("empty partial lane " << i);
        CHECK(partial[i] == Ty{});
    }

    if (V::size() > 1) {
        const V half = rc::partial_load<V>(buffer, 1);
        CHECK(half[0] == static_cast<Ty>(1));
        for (rc::simd_size_type i = 1; i < V::size(); ++i) {
            INFO("half lane " << i);
            CHECK(half[i] == Ty{});
        }
    }

    const typename V::mask_type mask{[](auto index) { return static_cast<int>(index) % 2 == 0; }};
    const V blended = rc::unchecked_load<V>(buffer, V::size(), mask);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("blended lane " << i);
        CHECK(blended[i] == (mask[i] ? static_cast<Ty>(i + 1) : Ty{}));
    }

    const V masked_partial = rc::partial_load<V>(buffer, 0, mask);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("masked partial lane " << i);
        CHECK(masked_partial[i] == Ty{});
    }

    std::vector<Ty> dynamic(static_cast<std::size_t>(V::size()) + 4);
    for (std::size_t i = 0; i < dynamic.size(); ++i) {
        dynamic[i] = static_cast<Ty>(i + 100);
    }
    const V from_iterator = rc::unchecked_load<V>(dynamic.begin(), V::size());
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("iterator lane " << i);
        CHECK(from_iterator[i] == static_cast<Ty>(i + 100));
    }
}

TEMPLATE_TEST_CASE("simd store writes back contiguous storage", "[simd][load_store]", simd_vec_f, simd_vec_d,
                   simd_vec_f8, simd_vec_f1, simd_vec_i) {
    using V = TestType;
    using Ty = typename V::value_type;
    using mask_type = typename V::mask_type;

    Ty buffer[32] = {};
    Ty expected[32] = {};
    fill_buffer(buffer, 32);
    for (int i = 0; i < 32; ++i) {
        expected[i] = buffer[i];
    }

    const V value{[](auto index) { return static_cast<typename V::value_type>((static_cast<int>(index) + 1) * 10); }};

    rc::unchecked_store(value, buffer, V::size());
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("store lane " << i);
        CHECK(buffer[i] == value[i]);
    }
    for (rc::simd_size_type i = V::size(); i < 32; ++i) {
        CHECK(buffer[i] == expected[i]);
    }

    fill_buffer(buffer, 32);
    rc::unchecked_store(value, buffer, V::size(), rc::flag_default);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("flag store lane " << i);
        CHECK(buffer[i] == value[i]);
    }

    fill_buffer(buffer, 32);
    rc::partial_store(value, buffer, 0);
    for (int i = 0; i < 32; ++i) {
        CHECK(buffer[i] == expected[i]);
    }

    if (V::size() > 1) {
        fill_buffer(buffer, 32);
        rc::partial_store(value, buffer, 1);
        CHECK(buffer[0] == value[0]);
        for (rc::simd_size_type i = 1; i < 32; ++i) {
            INFO("partial store lane " << i);
            CHECK(buffer[i] == expected[i]);
        }
    }

    const mask_type mask{[](auto index) { return static_cast<int>(index) % 2 == 0; }};
    fill_buffer(buffer, 32);
    rc::unchecked_store(value, buffer, V::size(), mask);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("masked store lane " << i);
        CHECK(buffer[i] == (mask[i] ? value[i] : expected[i]));
    }

    fill_buffer(buffer, 32);
    rc::partial_store(value, buffer, 0, mask);
    for (int i = 0; i < 32; ++i) {
        CHECK(buffer[i] == expected[i]);
    }

    fill_buffer(buffer, 32);
    std::vector<Ty> dynamic(32);
    for (std::size_t i = 0; i < dynamic.size(); ++i) {
        dynamic[i] = static_cast<Ty>(i + 7);
    }
    rc::unchecked_store(value, dynamic.begin(), V::size());
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("iterator store lane " << i);
        CHECK(dynamic[static_cast<std::size_t>(i)] == value[i]);
    }
}
