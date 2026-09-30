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
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <rainy/core/simd.hpp>

namespace rc = rainy::core;
namespace iter_traits = rainy::type_traits::extras::iterators;

namespace {
    template <typename V>
    V iota_vec() {
        using Ty = typename V::value_type;
        return V{[](auto index) { return static_cast<Ty>(static_cast<int>(index) + 1); }};
    }

    template <typename V>
    void check_iterator_for_width() {
        using value_type = typename V::value_type;
        const V value = iota_vec<V>();

        rc::simd_size_type count = 0;
        for (auto it = value.begin(); it != value.end(); ++it) {
            CHECK(*it == static_cast<value_type>(count + 1));
            ++count;
        }
        CHECK(count == V::size());
        CHECK(std::distance(value.cbegin(), value.cend()) == static_cast<rc::simd_size_type>(V::size()));

        auto it = value.begin();
        auto advanced = it + 2;
        if (V::size() > 2) {
            CHECK(*advanced == static_cast<value_type>(3));
            CHECK(advanced - it == 2);
            CHECK(it < advanced);
            CHECK(advanced >= it);
            CHECK(it[1] == static_cast<value_type>(2));
        }
        advanced = value.end();
        --advanced;
        CHECK(*advanced == static_cast<value_type>(V::size()));
        CHECK(value.end() - value.begin() == V::size());

        typename V::const_iterator converted = value.begin();
        CHECK(converted == value.begin());
        CHECK(value.begin() == converted);

        value_type collected[32] = {};
        std::copy(value.begin(), value.end(), collected);
        for (rc::simd_size_type i = 0; i < V::size(); ++i) {
            INFO("copy lane " << i);
            CHECK(collected[i] == static_cast<value_type>(i + 1));
        }

        STATIC_REQUIRE(iter_traits::is_random_access_iterator_v<typename V::iterator>);
        STATIC_REQUIRE(iter_traits::is_random_access_iterator_v<typename V::const_iterator>);
        STATIC_REQUIRE(!iter_traits::is_contiguous_iterator_v<typename V::iterator>);
    }
}

TEST_CASE("basic_vec iterators visit every lane in order", "[simd][iterator]") {
    using V = rc::vec<float>;
    const V value = iota_vec<V>();

    rc::simd_size_type count = 0;
    for (auto it = value.begin(); it != value.end(); ++it) {
        CHECK(*it == static_cast<float>(count + 1));
        ++count;
    }
    CHECK(count == V::size());

    count = 0;
    for (auto it = value.cbegin(); it != value.cend(); ++it) {
        CHECK(*it == static_cast<float>(count + 1));
        ++count;
    }
    CHECK(count == V::size());

    V mutable_value = iota_vec<V>();
    count = 0;
    for (auto it = mutable_value.begin(); it != mutable_value.end(); ++it) {
        CHECK(*it == static_cast<float>(count + 1));
        ++count;
    }
    CHECK(count == V::size());

    const V &const_ref = mutable_value;
    count = 0;
    for (auto it = const_ref.begin(); it != const_ref.end(); ++it) {
        CHECK(*it == static_cast<float>(count + 1));
        ++count;
    }
    CHECK(count == V::size());
}

TEST_CASE("basic_vec iterator arithmetic", "[simd][iterator]") {
    using V = rc::vec<float>;
    V value = iota_vec<V>();

    auto it = value.begin();
    CHECK(*it == value[0]);
    CHECK(it[1] == value[1]);
    CHECK(it[static_cast<rc::simd_size_type>(V::size()) - 1] == value[V::size() - 1]);

    auto advanced = it + 2;
    CHECK(*advanced == value[2]);
    CHECK(advanced - it == 2);
    CHECK(it - advanced == -2);
    CHECK(2 + it == advanced);
    CHECK(advanced > it);
    CHECK(advanced >= it);
    CHECK(it < advanced);
    CHECK(it <= advanced);
    CHECK(it == it);
    CHECK(it != advanced);

    ++it;
    CHECK(*it == value[1]);
    it++;
    CHECK(*it == value[2]);
    --it;
    CHECK(*it == value[1]);
    it--;
    CHECK(*it == value[0]);

    it += 3;
    CHECK(*it == value[3]);
    it -= 2;
    CHECK(*it == value[1]);

    advanced = value.end();
    --advanced;
    CHECK(*advanced == value[V::size() - 1]);

    CHECK(value.end() - value.begin() == V::size());
    CHECK(value.begin() + V::size() == value.end());
    CHECK(value.end() - 1 == value.begin() + V::size() - 1);

    advanced = value.begin();
    advanced += V::size();
    CHECK(advanced == value.end());
}

TEST_CASE("basic_vec iterator conversions and traits", "[simd][iterator]") {
    using V = rc::vec<float>;
    const V value = iota_vec<V>();

    typename V::const_iterator converted = value.begin();
    CHECK(converted == value.cbegin());
    CHECK(converted == value.begin());
    CHECK(value.begin() == converted);
    CHECK(converted <= value.begin());
    CHECK(converted >= value.begin());

    V mutable_value = iota_vec<V>();
    typename V::iterator mutable_it{};
    typename V::iterator assigned{};
    mutable_it = mutable_value.begin();
    assigned = mutable_it;
    CHECK(assigned == mutable_it);
    CHECK(assigned == mutable_value.begin());

    typename V::const_iterator default_constructed{};
    CHECK(default_constructed == typename V::const_iterator{});

    STATIC_REQUIRE(iter_traits::is_input_iterator_v<typename V::iterator>);
    STATIC_REQUIRE(iter_traits::is_forward_iterator_v<typename V::iterator>);
    STATIC_REQUIRE(iter_traits::is_bidirectional_iterator_v<typename V::iterator>);
    STATIC_REQUIRE(iter_traits::is_random_access_iterator_v<typename V::iterator>);
    STATIC_REQUIRE(iter_traits::is_random_access_iterator_v<typename V::const_iterator>);
    STATIC_REQUIRE(!iter_traits::is_contiguous_iterator_v<typename V::iterator>);

    using traits = rainy::utility::iterator_traits<typename V::iterator>;
    STATIC_REQUIRE(std::is_same_v<traits::value_type, float>);
    STATIC_REQUIRE(std::is_same_v<traits::difference_type, rc::simd_size_type>);
    STATIC_REQUIRE(std::is_same_v<traits::iterator_category, std::random_access_iterator_tag>);

    using const_traits = rainy::utility::iterator_traits<typename V::const_iterator>;
    STATIC_REQUIRE(std::is_same_v<const_traits::value_type, float>);
}

TEST_CASE("basic_vec iterators work with standard algorithms", "[simd][iterator]") {
    using V = rc::vec<double>;
    const V value = iota_vec<V>();

    CHECK(std::distance(value.begin(), value.end()) == V::size());
    CHECK(std::distance(value.cbegin(), value.cend()) == static_cast<rc::simd_size_type>(V::size()));

    double collected[32] = {};
    std::copy(value.begin(), value.end(), collected);
    for (rc::simd_size_type i = 0; i < V::size(); ++i) {
        INFO("lane " << i);
        CHECK(collected[i] == static_cast<double>(i + 1));
    }

    const auto found = std::find_if(value.begin(), value.end(), [](double x) { return x > static_cast<double>(V::size()) - 1.0; });
    REQUIRE(found != value.end());
    CHECK(*found == static_cast<double>(V::size()));

    const auto missing = std::find(value.begin(), value.end(), static_cast<double>(V::size()) + 100.0);
    CHECK(missing == value.end());
}

TEST_CASE("basic_vec iterators work across widths and element types", "[simd][iterator]") {
    check_iterator_for_width<rc::vec<float, 8>>();
    check_iterator_for_width<rc::vec<float, 1>>();
    check_iterator_for_width<rc::vec<int>>();
    check_iterator_for_width<rc::vec<unsigned>>();
    check_iterator_for_width<rc::vec<double>>();
    check_iterator_for_width<rc::vec<signed char>>();
}
