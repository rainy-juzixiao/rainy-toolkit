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

#include <rainy/core/collections/vector.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

using namespace rainy;
using namespace rainy::collections;

constexpr bool test_default_constructor_constexpr() {
    vector<int> v;
    return v.empty() && v.size() == 0 && v.capacity() == 0;
}

TEST_CASE("vector default constructor", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_default_constructor_constexpr());
    }
}

constexpr bool test_count_constructor_constexpr() {
    vector<int> v(5);
    return v.size() == 5 && v.capacity() >= 5 && !v.empty();
}

TEST_CASE("vector count constructor", "[vector]") {
    SECTION("runtime") {
        vector<int> v(5);
        REQUIRE(v.size() == 5);
        REQUIRE(v.capacity() >= 5);
        REQUIRE_FALSE(v.empty());
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_count_constructor_constexpr());
    }
}

constexpr bool test_count_value_constructor_constexpr() {
    vector<int> v(3, 42);
    return v.size() == 3 && v[0] == 42 && v[1] == 42 && v[2] == 42;
}

TEST_CASE("vector count value constructor", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 42);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 42);
        REQUIRE(v[1] == 42);
        REQUIRE(v[2] == 42);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_count_value_constructor_constexpr());
    }
}

constexpr bool test_copy_constructor_constexpr() {
    vector<int> v(3, 10);
    vector<int> v2(v);
    return v2.size() == 3 && v2[0] == 10 && v2[1] == 10 && v2[2] == 10;
}

TEST_CASE("vector copy constructor", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        vector<int> v2(v);
        REQUIRE(v2.size() == 3);
        REQUIRE(v2[0] == 10);
        REQUIRE(v2[1] == 10);
        REQUIRE(v2[2] == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_copy_constructor_constexpr());
    }
}

constexpr bool test_empty_constexpr() {
    vector<int> v;
    return v.empty();
}

TEST_CASE("vector empty", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        REQUIRE(v.empty());
        v.push_back(1);
        REQUIRE_FALSE(v.empty());
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_empty_constexpr());
    }
}

constexpr bool test_size_constexpr() {
    vector<int> v(3, 5);
    return v.size() == 3;
}

TEST_CASE("vector size", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 5);
        REQUIRE(v.size() == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_size_constexpr());
    }
}

constexpr bool test_capacity_constexpr() {
    vector<int> v(5);
    return v.capacity() >= 5;
}

TEST_CASE("vector capacity", "[vector]") {
    SECTION("runtime") {
        vector<int> v(5);
        REQUIRE(v.capacity() >= 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_capacity_constexpr());
    }
}

constexpr bool test_operator_subscript_constexpr() {
    vector<int> v(3, 10);
    return v[0] == 10 && v[1] == 10 && v[2] == 10;
}

TEST_CASE("vector operator[]", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 10);
        REQUIRE(v[2] == 10);
        v[0] = 20;
        REQUIRE(v[0] == 20);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_subscript_constexpr());
    }
}

constexpr bool test_at_constexpr() {
    vector<int> v(3, 10);
    return v.at(0) == 10 && v.at(1) == 10 && v.at(2) == 10;
}

TEST_CASE("vector at", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        REQUIRE(v.at(0) == 10);
        REQUIRE(v.at(1) == 10);
        REQUIRE(v.at(2) == 10);
        REQUIRE_THROWS_AS(v.at(5), core::exceptions::logic::out_of_range);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_at_constexpr());
    }
}

constexpr bool test_front_constexpr() {
    vector<int> v(3, 10);
    v[0] = 42;
    return v.front() == 42;
}

TEST_CASE("vector front", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        v[0] = 42;
        REQUIRE(v.front() == 42);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_front_constexpr());
    }
}

constexpr bool test_back_constexpr() {
    vector<int> v(3, 10);
    v[2] = 99;
    return v.back() == 99;
}

TEST_CASE("vector back", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        v[2] = 99;
        REQUIRE(v.back() == 99);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_back_constexpr());
    }
}

constexpr bool test_data_constexpr() {
    vector<int> v(3, 10);
    return v.data() != nullptr && *v.data() == 10;
}

TEST_CASE("vector data", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        REQUIRE(v.data() != nullptr);
        REQUIRE(*v.data() == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_data_constexpr());
    }
}

constexpr bool test_begin_end_constexpr() {
    vector<int> v(3, 10);
    return *v.begin() == 10 && *(v.end() - 1) == 10;
}

TEST_CASE("vector begin/end", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        REQUIRE(*v.begin() == 10);
        REQUIRE(*(v.end() - 1) == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_begin_end_constexpr());
    }
}

constexpr bool test_cbegin_cend_constexpr() {
    vector<int> v(3, 10);
    return *v.cbegin() == 10 && *(v.cend() - 1) == 10;
}

TEST_CASE("vector cbegin/cend", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        REQUIRE(*v.cbegin() == 10);
        REQUIRE(*(v.cend() - 1) == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_cbegin_cend_constexpr());
    }
}

constexpr bool test_push_back_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    return v.size() == 3 && v[0] == 1 && v[1] == 2 && v[2] == 3;
}

TEST_CASE("vector push_back", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_push_back_constexpr());
    }
}

constexpr bool test_pop_back_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.pop_back();
    return v.size() == 2 && v.back() == 2;
}

TEST_CASE("vector pop_back", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.pop_back();
        REQUIRE(v.size() == 2);
        REQUIRE(v.back() == 2);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_pop_back_constexpr());
    }
}

constexpr bool test_clear_constexpr() {
    vector<int> v(5, 10);
    v.clear();
    return v.empty() && v.size() == 0;
}

TEST_CASE("vector clear", "[vector]") {
    SECTION("runtime") {
        vector<int> v(5, 10);
        v.clear();
        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_clear_constexpr());
    }
}

constexpr bool test_resize_constexpr() {
    vector<int> v(3, 10);
    v.resize(5);
    return v.size() == 5;
}

TEST_CASE("vector resize", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        v.resize(5);
        REQUIRE(v.size() == 5);
        v.resize(1);
        REQUIRE(v.size() == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_resize_constexpr());
    }
}

constexpr bool test_reserve_constexpr() {
    vector<int> v;
    v.reserve(10);
    return v.capacity() >= 10 && v.size() == 0;
}

TEST_CASE("vector reserve", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.reserve(10);
        REQUIRE(v.capacity() >= 10);
        REQUIRE(v.size() == 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_reserve_constexpr());
    }
}

constexpr bool test_emplace_back_constexpr() {
    vector<int> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    return v.size() == 3 && v[0] == 1 && v[1] == 2 && v[2] == 3;
}

TEST_CASE("vector emplace_back", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_emplace_back_constexpr());
    }
}

constexpr bool test_emplace_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(3);
    v.emplace(v.begin() + 1, 2);
    return v.size() == 3 && v[0] == 1 && v[1] == 2 && v[2] == 3;
}

TEST_CASE("vector emplace", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(3);
        auto it = v.emplace(v.begin() + 1, 2);
        REQUIRE(*it == 2);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_emplace_constexpr());
    }
}

constexpr bool test_insert_single_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(3);
    v.insert(v.begin() + 1, 2);
    return v.size() == 3 && v[0] == 1 && v[1] == 2 && v[2] == 3;
}

TEST_CASE("vector insert single", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(3);
        auto it = v.insert(v.begin() + 1, 2);
        REQUIRE(*it == 2);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_insert_single_constexpr());
    }
}

constexpr bool test_insert_count_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(4);
    v.insert(v.begin() + 1, 2, 99);
    return v.size() == 4 && v[0] == 1 && v[1] == 99 && v[2] == 99 && v[3] == 4;
}

TEST_CASE("vector insert count", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(4);
        auto it = v.insert(v.begin() + 1, 2, 99);
        REQUIRE(*it == 99);
        REQUIRE(v.size() == 4);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 99);
        REQUIRE(v[2] == 99);
        REQUIRE(v[3] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_insert_count_constexpr());
    }
}

constexpr bool test_erase_single_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.erase(v.begin() + 1);
    return v.size() == 2 && v[0] == 1 && v[1] == 3;
}

TEST_CASE("vector erase single", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto it = v.erase(v.begin() + 1);
        REQUIRE(*it == 3);
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_erase_single_constexpr());
    }
}

constexpr bool test_erase_range_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    v.erase(v.begin() + 1, v.begin() + 3);
    return v.size() == 2 && v[0] == 1 && v[1] == 4;
}

TEST_CASE("vector erase range", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        auto it = v.erase(v.begin() + 1, v.begin() + 3);
        REQUIRE(*it == 4);
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_erase_range_constexpr());
    }
}

constexpr bool test_operator_equal_constexpr() {
    vector<int> v1(3, 10);
    vector<int> v2(3, 10);
    return v1 == v2;
}

TEST_CASE("vector operator==", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 10);
        vector<int> v2(3, 10);
        vector<int> v3(3, 20);
        REQUIRE(v1 == v2);
        REQUIRE_FALSE(v1 == v3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_equal_constexpr());
    }
}

constexpr bool test_operator_not_equal_constexpr() {
    vector<int> v1(3, 10);
    vector<int> v2(3, 20);
    return v1 != v2;
}

TEST_CASE("vector operator!=", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 10);
        vector<int> v2(3, 20);
        REQUIRE(v1 != v2);
        REQUIRE_FALSE(v1 != vector<int>(3, 10));
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_not_equal_constexpr());
    }
}

constexpr bool test_operator_less_constexpr() {
    vector<int> v1(3, 1);
    vector<int> v2(3, 2);
    return v1 < v2;
}

TEST_CASE("vector operator<", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 1);
        vector<int> v2(3, 2);
        REQUIRE(v1 < v2);
        REQUIRE_FALSE(v2 < v1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_less_constexpr());
    }
}

constexpr bool test_operator_greater_constexpr() {
    vector<int> v1(3, 2);
    vector<int> v2(3, 1);
    return v1 > v2;
}

TEST_CASE("vector operator>", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 2);
        vector<int> v2(3, 1);
        REQUIRE(v1 > v2);
        REQUIRE_FALSE(v2 > v1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_greater_constexpr());
    }
}

constexpr bool test_operator_less_equal_constexpr() {
    vector<int> v1(3, 1);
    vector<int> v2(3, 2);
    return v1 <= v2 && v1 <= v1;
}

TEST_CASE("vector operator<=", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 1);
        vector<int> v2(3, 2);
        REQUIRE(v1 <= v2);
        REQUIRE(v1 <= v1);
        REQUIRE_FALSE(v2 <= v1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_less_equal_constexpr());
    }
}

constexpr bool test_operator_greater_equal_constexpr() {
    vector<int> v1(3, 2);
    vector<int> v2(3, 1);
    return v1 >= v2 && v1 >= v1;
}

TEST_CASE("vector operator>=", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 2);
        vector<int> v2(3, 1);
        REQUIRE(v1 >= v2);
        REQUIRE(v1 >= v1);
        REQUIRE_FALSE(v2 >= v1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_greater_equal_constexpr());
    }
}

constexpr bool test_swap_constexpr() {
    vector<int> v1(3, 1);
    vector<int> v2(3, 2);
    v1.swap(v2);
    return v1[0] == 2 && v2[0] == 1;
}

TEST_CASE("vector swap", "[vector]") {
    SECTION("runtime") {
        vector<int> v1(3, 1);
        vector<int> v2(3, 2);
        v1.swap(v2);
        REQUIRE(v1[0] == 2);
        REQUIRE(v2[0] == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_swap_constexpr());
    }
}

constexpr bool test_slice_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);
    auto s = v.slice(1, 4);
    return s.size() == 3 && s[0] == 2 && s[1] == 3 && s[2] == 4;
}

TEST_CASE("vector slice", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
        auto s = v.slice(1, 4);
        REQUIRE(s.size() == 3);
        REQUIRE(s[0] == 2);
        REQUIRE(s[1] == 3);
        REQUIRE(s[2] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_slice_constexpr());
    }
}

constexpr bool test_left_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    auto l = v.left(2);
    return l.size() == 2 && l[0] == 1 && l[1] == 2;
}

TEST_CASE("vector left", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto l = v.left(2);
        REQUIRE(l.size() == 2);
        REQUIRE(l[0] == 1);
        REQUIRE(l[1] == 2);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_left_constexpr());
    }
}

constexpr bool test_right_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    auto r = v.right(2);
    return r.size() == 2 && r[0] == 2 && r[1] == 3;
}

TEST_CASE("vector right", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto r = v.right(2);
        REQUIRE(r.size() == 2);
        REQUIRE(r[0] == 2);
        REQUIRE(r[1] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_right_constexpr());
    }
}

constexpr bool test_index_of_constexpr() {
    vector<int> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    return v.index_of(20) == 1 && v.index_of(99) == vector<int>::npos;
}

TEST_CASE("vector index_of", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(10);
        v.push_back(20);
        v.push_back(30);
        REQUIRE(v.index_of(20) == 1);
        REQUIRE(v.index_of(99) == vector<int>::npos);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_index_of_constexpr());
    }
}

constexpr bool test_filter_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    auto f = v.filter([](int x) { return x % 2 == 0; });
    return f.size() == 2 && f[0] == 2 && f[1] == 4;
}

TEST_CASE("vector filter", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        auto f = v.filter([](int x) { return x % 2 == 0; });
        REQUIRE(f.size() == 2);
        REQUIRE(f[0] == 2);
        REQUIRE(f[1] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_filter_constexpr());
    }
}

constexpr bool test_reverse_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    auto r = v.reverse();
    return r.size() == 3 && r[0] == 3 && r[1] == 2 && r[2] == 1;
}

TEST_CASE("vector reverse", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto r = v.reverse();
        REQUIRE(r.size() == 3);
        REQUIRE(r[0] == 3);
        REQUIRE(r[1] == 2);
        REQUIRE(r[2] == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_reverse_constexpr());
    }
}

constexpr bool test_map_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    auto m = v.map([](int x) { return x * 2; });
    return m.size() == 3 && m[0] == 2 && m[1] == 4 && m[2] == 6;
}

TEST_CASE("vector map", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto m = v.map([](int x) { return x * 2; });
        REQUIRE(m.size() == 3);
        REQUIRE(m[0] == 2);
        REQUIRE(m[1] == 4);
        REQUIRE(m[2] == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_map_constexpr());
    }
}

constexpr bool test_fold_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    return v.fold(0) == 6;
}

TEST_CASE("vector fold", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        REQUIRE(v.fold(0) == 6);
        REQUIRE(v.fold(1, [](int a, int b) { return a * b; }) == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_fold_constexpr());
    }
}

constexpr bool test_concat_vector_constexpr() {
    vector<int> v1;
    v1.push_back(1);
    v1.push_back(2);
    vector<int> v2;
    v2.push_back(3);
    v2.push_back(4);
    auto c = v1.concat(v2);
    return c.size() == 4 && c[0] == 1 && c[1] == 2 && c[2] == 3 && c[3] == 4;
}

TEST_CASE("vector concat vector", "[vector]") {
    SECTION("runtime") {
        vector<int> v1;
        v1.push_back(1);
        v1.push_back(2);
        vector<int> v2;
        v2.push_back(3);
        v2.push_back(4);
        auto c = v1.concat(v2);
        REQUIRE(c.size() == 4);
        REQUIRE(c[0] == 1);
        REQUIRE(c[1] == 2);
        REQUIRE(c[2] == 3);
        REQUIRE(c[3] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_concat_vector_constexpr());
    }
}

constexpr bool test_assign_constexpr() {
    vector<int> v;
    v.assign(3, 42);
    return v.size() == 3 && v[0] == 42 && v[1] == 42 && v[2] == 42;
}

TEST_CASE("vector assign", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.assign(3, 42);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 42);
        REQUIRE(v[1] == 42);
        REQUIRE(v[2] == 42);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_assign_constexpr());
    }
}

constexpr bool test_shrink_to_fit_constexpr() {
    vector<int> v;
    v.reserve(10);
    v.push_back(1);
    v.shrink_to_fit();
    return v.capacity() == 1 && v.size() == 1;
}

TEST_CASE("vector shrink_to_fit", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.reserve(10);
        v.push_back(1);
        v.shrink_to_fit();
        REQUIRE(v.capacity() == 1);
        REQUIRE(v.size() == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_shrink_to_fit_constexpr());
    }
}

constexpr bool test_max_size_constexpr() {
    vector<int> v;
    return v.max_size() > 0;
}

TEST_CASE("vector max_size", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        REQUIRE(v.max_size() > 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_max_size_constexpr());
    }
}

constexpr bool test_move_constructor_constexpr() {
    vector<int> v(3, 10);
    vector<int> v2(utility::move(v));
    return v2.size() == 3 && v2[0] == 10;
}

TEST_CASE("vector move constructor", "[vector]") {
    SECTION("runtime") {
        vector<int> v(3, 10);
        vector<int> v2(std::move(v));
        REQUIRE(v2.size() == 3);
        REQUIRE(v2[0] == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_move_constructor_constexpr());
    }
}

constexpr bool test_initializer_list_constructor_constexpr() {
    vector<int> v = {1, 2, 3, 4, 5};
    return v.size() == 5 && v[0] == 1 && v[4] == 5;
}

TEST_CASE("vector initializer_list constructor", "[vector]") {
    SECTION("runtime") {
        vector<int> v = {1, 2, 3, 4, 5};
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 1);
        REQUIRE(v[4] == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_initializer_list_constructor_constexpr());
    }
}

constexpr bool test_rbegin_rend_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    return *v.rbegin() == 3 && *(v.rend() - 1) == 1;
}

TEST_CASE("vector rbegin/rend", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        REQUIRE(*v.rbegin() == 3);
        REQUIRE(*(v.rend() - 1) == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_rbegin_rend_constexpr());
    }
}

constexpr bool test_crbegin_crend_constexpr() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    return *v.crbegin() == 3 && *(v.crend() - 1) == 1;
}

TEST_CASE("vector crbegin/crend", "[vector]") {
    SECTION("runtime") {
        vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        REQUIRE(*v.crbegin() == 3);
        REQUIRE(*(v.crend() - 1) == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_crbegin_crend_constexpr());
    }
}