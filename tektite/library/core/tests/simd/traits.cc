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

TEST_CASE("simd_size and deduce_abi produce requested widths", "[simd][traits]") {
    STATIC_REQUIRE(simd_vec_f1::size() == 1);
    STATIC_REQUIRE(simd_vec_f::size() == rc::simd_size_v<float, rc::native_abi_t<float>>);
    STATIC_REQUIRE(simd_vec_f8::size() == 8);
    STATIC_REQUIRE(simd_vec_d::size() == rc::simd_size_v<double, rc::native_abi_t<double>>);
    STATIC_REQUIRE(rc::vec<double, 4>::size() == 4);
    STATIC_REQUIRE(rc::vec<int, 16>::size() == 16);
    STATIC_REQUIRE(simd_vec_i::size() == rc::simd_size_v<int, rc::native_abi_t<int>>);
    STATIC_REQUIRE(rc::simd_size_v<float, rc::implements::scalar_abi> == 1);
    STATIC_REQUIRE(std::is_same_v<rc::vec<float, 1>::abi_type, rc::implements::scalar_abi>);
    STATIC_REQUIRE(std::is_same_v<rc::deduce_abi_t<float, 1>, rc::implements::scalar_abi>);
    STATIC_REQUIRE(!std::is_same_v<rc::deduce_abi_t<float, 1>, rc::deduce_abi_t<float, 4>>);
}

TEST_CASE("alignment traits match register size", "[simd][traits]") {
    STATIC_REQUIRE(rc::alignment_v<simd_vec_f> == sizeof(simd_vec_f::register_type));
    STATIC_REQUIRE(rc::alignment_v<simd_vec_d> == sizeof(simd_vec_d::register_type));
    STATIC_REQUIRE(rc::alignment_v<simd_vec_f8> == sizeof(simd_vec_f8::register_type));
    STATIC_REQUIRE(rc::alignment_v<typename simd_vec_f::mask_type> ==
                   sizeof(typename simd_vec_f::mask_type::register_type));
    STATIC_REQUIRE(rc::alignment_v<simd_vec_f, float> == sizeof(simd_vec_f::register_type));
}

TEST_CASE("rebind keeps element count for vectors and masks", "[simd][traits]") {
    STATIC_REQUIRE(rc::rebind_t<int, simd_vec_f>::size() == simd_vec_f::size());
    STATIC_REQUIRE(rc::rebind_t<long int, simd_vec_f>::size() == simd_vec_f::size());
    STATIC_REQUIRE(rc::rebind_t<unsigned, simd_vec_f>::size() == simd_vec_f::size());
    STATIC_REQUIRE(std::is_same_v<typename rc::rebind_t<int, simd_vec_f>::value_type, int>);
    STATIC_REQUIRE(std::is_same_v<typename rc::rebind_t<int, simd_vec_f>::abi_type, typename simd_vec_f::abi_type>);
    STATIC_REQUIRE(rc::rebind_t<int, simd_vec_f8>::size() == simd_vec_f8::size());
    STATIC_REQUIRE(rc::rebind_t<double, simd_vec_f>::size() == simd_vec_f::size());
    STATIC_REQUIRE(rc::rebind_t<std::int64_t, simd_vec_f>::size() == simd_vec_f::size());

    using mask_type = typename simd_vec_f::mask_type;
    STATIC_REQUIRE(rc::mask_element_size<mask_type> == sizeof(float));
    STATIC_REQUIRE(rc::rebind_t<int, mask_type>::size() == mask_type::size());
    STATIC_REQUIRE(std::is_same_v<rc::rebind_t<int, mask_type>, mask_type>);
    STATIC_REQUIRE(rc::mask_element_size<rc::rebind_t<long long, mask_type>> == sizeof(long long));
    STATIC_REQUIRE(rc::rebind_t<long long, mask_type>::size() == mask_type::size());
}

TEST_CASE("deduced_vec_t and math constraints", "[simd][traits]") {
    STATIC_REQUIRE(std::is_same_v<rc::deduced_vec_t<simd_vec_f>, simd_vec_f>);
    STATIC_REQUIRE(std::is_same_v<rc::deduced_vec_t<simd_vec_f8>, simd_vec_f8>);
    STATIC_REQUIRE(std::is_same_v<rc::deduced_vec_t<simd_vec_f1>, simd_vec_f1>);
    STATIC_REQUIRE(std::is_same_v<rc::deduced_vec_t<int>, void>);
    STATIC_REQUIRE(rc::implements::is_math_floating_point_v<simd_vec_f>);
    STATIC_REQUIRE(rc::implements::is_math_floating_point_v<simd_vec_d>);
    STATIC_REQUIRE(rc::implements::is_math_floating_point_v<simd_vec_f8>);
    STATIC_REQUIRE(!rc::implements::is_math_floating_point_v<simd_vec_i>);
    STATIC_REQUIRE(!rc::implements::is_math_floating_point_v<int>);
    STATIC_REQUIRE(std::is_same_v<rc::implements::enable_if_math_t<simd_vec_f>, int>);
}

TEST_CASE("rebind stays consistent with simd_size", "[simd][traits]") {
    STATIC_REQUIRE(rc::simd_size_v<float, typename rc::rebind_t<int, simd_vec_f>::abi_type> ==
                   rc::simd_size_v<int, typename rc::rebind_t<int, simd_vec_f>::abi_type>);
    STATIC_REQUIRE(std::is_same_v<rc::vec<float>, rc::basic_vec<float, rc::deduce_abi_t<
                                                        float, rc::simd_size_v<float, rc::native_abi_t<float>>>>>);
}
