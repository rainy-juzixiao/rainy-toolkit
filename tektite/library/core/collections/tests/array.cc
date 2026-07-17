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

#include <rainy/core/collections/array.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

using namespace rainy;
using namespace rainy::collections;

constexpr bool test_default_constructor_constexpr() {
    array<int, 5> arr;
    return arr.size() == 5 && arr[0] == 0 && arr[4] == 0;
}

TEST_CASE("array default constructor", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr;
        REQUIRE(arr.size() == 5);
        REQUIRE(arr[0] == 0);
        REQUIRE(arr[4] == 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_default_constructor_constexpr());
    }
}

constexpr bool test_initializer_list_constructor_constexpr() {
    array<int, 5> arr = {1, 2, 3, 4, 5};
    return arr.size() == 5 && arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5;
}

TEST_CASE("array initializer_list constructor", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr = {1, 2, 3, 4, 5};
        REQUIRE(arr.size() == 5);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        REQUIRE(arr[3] == 4);
        REQUIRE(arr[4] == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_initializer_list_constructor_constexpr());
    }
}

constexpr bool test_copy_constructor_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    array<int, 3> arr2(arr);
    return arr2[0] == 1 && arr2[1] == 2 && arr2[2] == 3;
}

TEST_CASE("array copy constructor", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        array<int, 3> arr2(arr);
        REQUIRE(arr2[0] == 1);
        REQUIRE(arr2[1] == 2);
        REQUIRE(arr2[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_copy_constructor_constexpr());
    }
}

constexpr bool test_move_constructor_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    array<int, 3> arr2(utility::move(arr));
    return arr2[0] == 1 && arr2[1] == 2 && arr2[2] == 3;
}

TEST_CASE("array move constructor", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        array<int, 3> arr2(std::move(arr));
        REQUIRE(arr2[0] == 1);
        REQUIRE(arr2[1] == 2);
        REQUIRE(arr2[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_move_constructor_constexpr());
    }
}

constexpr bool test_count_value_constructor_constexpr() {
    array<int, 5> arr(3, 42);
    return arr[0] == 42 && arr[1] == 42 && arr[2] == 42 && arr[3] == 0 && arr[4] == 0;
}

TEST_CASE("array count value constructor", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr(3, 42);
        REQUIRE(arr[0] == 42);
        REQUIRE(arr[1] == 42);
        REQUIRE(arr[2] == 42);
        REQUIRE(arr[3] == 0);
        REQUIRE(arr[4] == 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_count_value_constructor_constexpr());
    }
}

constexpr bool test_iterator_range_constructor_constexpr() {
    int raw[] = {10, 20, 30};
    array<int, 3> arr(raw, raw + 3);
    return arr[0] == 10 && arr[1] == 20 && arr[2] == 30;
}

TEST_CASE("array iterator range constructor", "[array]") {
    SECTION("runtime") {
        int raw[] = {10, 20, 30};
        array<int, 3> arr(raw, raw + 3);
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 20);
        REQUIRE(arr[2] == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_iterator_range_constructor_constexpr());
    }
}

constexpr bool test_concat_constructor_constexpr() {
    array<int, 2> left = {1, 2};
    array<int, 3> right = {3, 4, 5};
    array<int, 5> arr(left, right);
    return arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5;
}

TEST_CASE("array concat constructor", "[array]") {
    SECTION("runtime") {
        array<int, 2> left = {1, 2};
        array<int, 3> right = {3, 4, 5};
        array<int, 5> arr(left, right);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        REQUIRE(arr[3] == 4);
        REQUIRE(arr[4] == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_concat_constructor_constexpr());
    }
}

constexpr bool test_size_constexpr() {
    array<int, 5> arr;
    return arr.size() == 5;
}

TEST_CASE("array size", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr;
        REQUIRE(arr.size() == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_size_constexpr());
    }
}

constexpr bool test_max_size_constexpr() {
    array<int, 3> arr;
    return arr.max_size() == 3;
}

TEST_CASE("array max_size", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr;
        REQUIRE(arr.max_size() == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_max_size_constexpr());
    }
}

constexpr bool test_length_constexpr() {
    array<int, 4> arr;
    return arr.length() == 4;
}

TEST_CASE("array length", "[array]") {
    SECTION("runtime") {
        array<int, 4> arr;
        REQUIRE(arr.length() == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_length_constexpr());
    }
}

constexpr bool test_empty_constexpr() {
    array<int, 0> arr;
    return arr.empty();
}

TEST_CASE("array empty", "[array]") {
    SECTION("runtime") {
        array<int, 0> arr;
        REQUIRE(arr.empty());
        array<int, 5> arr2;
        REQUIRE_FALSE(arr2.empty());
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_empty_constexpr());
    }
}

constexpr bool test_operator_subscript_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return arr[0] == 1 && arr[1] == 2 && arr[2] == 3;
}

TEST_CASE("array operator[]", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        arr[0] = 10;
        REQUIRE(arr[0] == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_subscript_constexpr());
    }
}

constexpr bool test_at_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return arr.at(0) == 1 && arr.at(1) == 2 && arr.at(2) == 3;
}

TEST_CASE("array at", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(arr.at(0) == 1);
        REQUIRE(arr.at(1) == 2);
        REQUIRE(arr.at(2) == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_at_constexpr());
    }
}

constexpr bool test_front_constexpr() {
    array<int, 3> arr = {10, 20, 30};
    return arr.front() == 10;
}

TEST_CASE("array front", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {10, 20, 30};
        REQUIRE(arr.front() == 10);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_front_constexpr());
    }
}

constexpr bool test_back_constexpr() {
    array<int, 3> arr = {10, 20, 30};
    return arr.back() == 30;
}

TEST_CASE("array back", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {10, 20, 30};
        REQUIRE(arr.back() == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_back_constexpr());
    }
}

constexpr bool test_data_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return arr.data() != nullptr && *arr.data() == 1;
}

TEST_CASE("array data", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(arr.data() != nullptr);
        REQUIRE(*arr.data() == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_data_constexpr());
    }
}

constexpr bool test_begin_end_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return *arr.begin() == 1 && *(arr.end() - 1) == 3;
}

TEST_CASE("array begin/end", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(*arr.begin() == 1);
        REQUIRE(*(arr.end() - 1) == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_begin_end_constexpr());
    }
}

constexpr bool test_cbegin_cend_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return *arr.cbegin() == 1 && *(arr.cend() - 1) == 3;
}

TEST_CASE("array cbegin/cend", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(*arr.cbegin() == 1);
        REQUIRE(*(arr.cend() - 1) == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_cbegin_cend_constexpr());
    }
}

constexpr bool test_rbegin_rend_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return *arr.rbegin() == 3 && *(arr.rend() - 1) == 1;
}

TEST_CASE("array rbegin/rend", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(*arr.rbegin() == 3);
        REQUIRE(*(arr.rend() - 1) == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_rbegin_rend_constexpr());
    }
}

constexpr bool test_crbegin_crend_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return *arr.crbegin() == 3 && *(arr.crend() - 1) == 1;
}

TEST_CASE("array crbegin/crend", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(*arr.crbegin() == 3);
        REQUIRE(*(arr.crend() - 1) == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_crbegin_crend_constexpr());
    }
}

constexpr bool test_fill_constexpr() {
    array<int, 5> arr;
    arr.fill(42);
    return arr[0] == 42 && arr[1] == 42 && arr[2] == 42 && arr[3] == 42 && arr[4] == 42;
}

TEST_CASE("array fill", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr;
        arr.fill(42);
        REQUIRE(arr[0] == 42);
        REQUIRE(arr[1] == 42);
        REQUIRE(arr[2] == 42);
        REQUIRE(arr[3] == 42);
        REQUIRE(arr[4] == 42);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_fill_constexpr());
    }
}

constexpr bool test_slice_constexpr() {
    array<int, 5> arr = {1, 2, 3, 4, 5};
    auto s = arr.slice<3>(1, 4);
    return s[0] == 2 && s[1] == 3 && s[2] == 4;
}

TEST_CASE("array slice", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr = {1, 2, 3, 4, 5};
        auto s = arr.slice<3>(1, 4);
        REQUIRE(s[0] == 2);
        REQUIRE(s[1] == 3);
        REQUIRE(s[2] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_slice_constexpr());
    }
}

constexpr bool test_left_constexpr() {
    array<int, 5> arr = {1, 2, 3, 4, 5};
    auto l = arr.left<3>(3);
    return l[0] == 1 && l[1] == 2 && l[2] == 3;
}

TEST_CASE("array left", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr = {1, 2, 3, 4, 5};
        auto l = arr.left<3>(3);
        REQUIRE(l[0] == 1);
        REQUIRE(l[1] == 2);
        REQUIRE(l[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_left_constexpr());
    }
}

constexpr bool test_right_constexpr() {
    array<int, 5> arr = {1, 2, 3, 4, 5};
    auto r = arr.right<3>(3);
    return r[0] == 3 && r[1] == 4 && r[2] == 5;
}

TEST_CASE("array right", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr = {1, 2, 3, 4, 5};
        auto r = arr.right<3>(3);
        REQUIRE(r[0] == 3);
        REQUIRE(r[1] == 4);
        REQUIRE(r[2] == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_right_constexpr());
    }
}

constexpr bool test_index_of_constexpr() {
    array<int, 5> arr = {10, 20, 30, 40, 50};
    return arr.index_of(30) == 2 && arr.index_of(99) == array<int, 5>::npos;
}

TEST_CASE("array index_of", "[array]") {
    SECTION("runtime") {
        array<int, 5> arr = {10, 20, 30, 40, 50};
        REQUIRE(arr.index_of(30) == 2);
        REQUIRE(arr.index_of(99) == array<int, 5>::npos);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_index_of_constexpr());
    }
}

constexpr bool test_filter_constexpr() {
    array<int, 4> arr = {1, 2, 3, 4};
    auto f = arr.filter<4>([](int x) { return x % 2 == 0; });
    return f[0] == 2 && f[1] == 4 && f[2] == 0 && f[3] == 0;
}

TEST_CASE("array filter", "[array]") {
    SECTION("runtime") {
        array<int, 4> arr = {1, 2, 3, 4};
        auto f = arr.filter<4>([](int x) { return x % 2 == 0; });
        REQUIRE(f[0] == 2);
        REQUIRE(f[1] == 4);
        REQUIRE(f[2] == 0);
        REQUIRE(f[3] == 0);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_filter_constexpr());
    }
}

constexpr bool test_reverse_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    auto r = arr.reverse();
    return r[0] == 3 && r[1] == 2 && r[2] == 1;
}

TEST_CASE("array reverse", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        auto r = arr.reverse();
        REQUIRE(r[0] == 3);
        REQUIRE(r[1] == 2);
        REQUIRE(r[2] == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_reverse_constexpr());
    }
}

constexpr bool test_map_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    auto m = arr.map([](int x) { return x * 2; });
    return m[0] == 2 && m[1] == 4 && m[2] == 6;
}

TEST_CASE("array map", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        auto m = arr.map([](int x) { return x * 2; });
        REQUIRE(m[0] == 2);
        REQUIRE(m[1] == 4);
        REQUIRE(m[2] == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_map_constexpr());
    }
}

constexpr bool test_fold_with_init_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return arr.fold(0) == 6;
}

TEST_CASE("array fold with init", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(arr.fold(0) == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_fold_with_init_constexpr());
    }
}

constexpr bool test_fold_with_func_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return arr.fold([](int a, int b) { return a * b; }, 1) == 6;
}

TEST_CASE("array fold with func", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(arr.fold([](int a, int b) { return a * b; }, 1) == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_fold_with_func_constexpr());
    }
}

constexpr bool test_concat_constexpr() {
    array<int, 2> arr1 = {1, 2};
    array<int, 3> arr2 = {3, 4, 5};
    auto c = arr1.concat(arr2);
    return c[0] == 1 && c[1] == 2 && c[2] == 3 && c[3] == 4 && c[4] == 5;
}

TEST_CASE("array concat", "[array]") {
    SECTION("runtime") {
        array<int, 2> arr1 = {1, 2};
        array<int, 3> arr2 = {3, 4, 5};
        auto c = arr1.concat(arr2);
        REQUIRE(c[0] == 1);
        REQUIRE(c[1] == 2);
        REQUIRE(c[2] == 3);
        REQUIRE(c[3] == 4);
        REQUIRE(c[4] == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_concat_constexpr());
    }
}

constexpr bool test_swap_member_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {4, 5, 6};
    arr1.swap(arr2);
    return arr1[0] == 4 && arr1[1] == 5 && arr1[2] == 6 && arr2[0] == 1 && arr2[1] == 2 && arr2[2] == 3;
}

TEST_CASE("array swap member", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {4, 5, 6};
        arr1.swap(arr2);
        REQUIRE(arr1[0] == 4);
        REQUIRE(arr1[1] == 5);
        REQUIRE(arr1[2] == 6);
        REQUIRE(arr2[0] == 1);
        REQUIRE(arr2[1] == 2);
        REQUIRE(arr2[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_swap_member_constexpr());
    }
}

constexpr bool test_operator_equal_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {1, 2, 3};
    return arr1 == arr2;
}

TEST_CASE("array operator==", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {1, 2, 3};
        array<int, 3> arr3 = {4, 5, 6};
        REQUIRE(arr1 == arr2);
        REQUIRE_FALSE(arr1 == arr3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_equal_constexpr());
    }
}

constexpr bool test_operator_not_equal_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {4, 5, 6};
    return arr1 != arr2;
}

TEST_CASE("array operator!=", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {4, 5, 6};
        REQUIRE(arr1 != arr2);
        REQUIRE_FALSE(arr1 != array<int, 3>{1, 2, 3});
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_not_equal_constexpr());
    }
}

constexpr bool test_operator_less_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {1, 2, 4};
    return arr1 < arr2;
}

TEST_CASE("array operator<", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {1, 2, 4};
        REQUIRE(arr1 < arr2);
        REQUIRE_FALSE(arr2 < arr1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_less_constexpr());
    }
}

constexpr bool test_operator_greater_constexpr() {
    array<int, 3> arr1 = {1, 2, 4};
    array<int, 3> arr2 = {1, 2, 3};
    return arr1 > arr2;
}

TEST_CASE("array operator>", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 4};
        array<int, 3> arr2 = {1, 2, 3};
        REQUIRE(arr1 > arr2);
        REQUIRE_FALSE(arr2 > arr1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_greater_constexpr());
    }
}

constexpr bool test_operator_less_equal_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {1, 2, 3};
    return arr1 <= arr2;
}

TEST_CASE("array operator<=", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {1, 2, 3};
        array<int, 3> arr3 = {1, 2, 4};
        REQUIRE(arr1 <= arr2);
        REQUIRE(arr1 <= arr3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_less_equal_constexpr());
    }
}

constexpr bool test_operator_greater_equal_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {1, 2, 3};
    return arr1 >= arr2;
}

TEST_CASE("array operator>=", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {1, 2, 3};
        array<int, 3> arr3 = {1, 2, 2};
        REQUIRE(arr1 >= arr2);
        REQUIRE(arr1 >= arr3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_greater_equal_constexpr());
    }
}

constexpr bool test_copy_assignment_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {4, 5, 6};
    arr1 = arr2;
    return arr1[0] == 4 && arr1[1] == 5 && arr1[2] == 6;
}

TEST_CASE("array copy assignment", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {4, 5, 6};
        arr1 = arr2;
        REQUIRE(arr1[0] == 4);
        REQUIRE(arr1[1] == 5);
        REQUIRE(arr1[2] == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_copy_assignment_constexpr());
    }
}

constexpr bool test_move_assignment_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {4, 5, 6};
    arr1 = utility::move(arr2);
    return arr1[0] == 4 && arr1[1] == 5 && arr1[2] == 6;
}

TEST_CASE("array move assignment", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {4, 5, 6};
        arr1 = std::move(arr2);
        REQUIRE(arr1[0] == 4);
        REQUIRE(arr1[1] == 5);
        REQUIRE(arr1[2] == 6);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_move_assignment_constexpr());
    }
}

constexpr bool test_access_carrays_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    auto &car = arr.access_carrays();
    return car[0] == 1 && car[1] == 2 && car[2] == 3;
}

TEST_CASE("array access_carrays", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        auto &car = arr.access_carrays();
        REQUIRE(car[0] == 1);
        REQUIRE(car[1] == 2);
        REQUIRE(car[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_access_carrays_constexpr());
    }
}

constexpr bool test_zip_with_constexpr() {
    array<int, 3> arr1 = {1, 2, 3};
    array<int, 3> arr2 = {4, 5, 6};
    auto z = zip_with(arr1, arr2, [](int a, int b) { return a + b; });
    return z[0] == 5 && z[1] == 7 && z[2] == 9;
}

TEST_CASE("array zip_with", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {4, 5, 6};
        auto z = zip_with(arr1, arr2, [](int a, int b) { return a + b; });
        REQUIRE(z[0] == 5);
        REQUIRE(z[1] == 7);
        REQUIRE(z[2] == 9);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_zip_with_constexpr());
    }
}

constexpr bool test_std_get_constexpr() {
    array<int, 3> arr = {1, 2, 3};
    return std::get<0>(arr) == 1 && std::get<1>(arr) == 2 && std::get<2>(arr) == 3;
}

TEST_CASE("array std::get", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr = {1, 2, 3};
        REQUIRE(std::get<0>(arr) == 1);
        REQUIRE(std::get<1>(arr) == 2);
        REQUIRE(std::get<2>(arr) == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_std_get_constexpr());
    }
}

constexpr bool test_in_place_constructor_constexpr() {
    array<int, 3> arr(std::in_place, 10, 20, 30);
    return arr[0] == 10 && arr[1] == 20 && arr[2] == 30;
}

TEST_CASE("array in_place constructor", "[array]") {
    SECTION("runtime") {
        array<int, 3> arr(std::in_place, 10, 20, 30);
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 20);
        REQUIRE(arr[2] == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_in_place_constructor_constexpr());
    }
}