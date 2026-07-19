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

#include <rainy/core/collections/array_view.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

using namespace rainy;
using namespace rainy::collections::views;

constexpr bool test_default_constructor_constexpr() {
    array_view<int> view;
    return view.empty() && view.size() == 0 && view.data() == nullptr;
}

TEST_CASE("array_view default constructor", "[array_view]") {
    SECTION("runtime") {
        array_view<int> view;
        REQUIRE(view.empty());
        REQUIRE(view.size() == 0);
        REQUIRE(view.data() == nullptr);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_default_constructor_constexpr());
    }
}

constexpr bool test_pointer_size_constructor_constexpr() {
    int data[] = {1, 2, 3, 4, 5};
    array_view<int> view(data, 5);
    return view.size() == 5 && view[0] == 1 && view[4] == 5;
}

TEST_CASE("array_view pointer size constructor", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3, 4, 5};
        array_view<int> view(data, 5);
        REQUIRE(view.size() == 5);
        REQUIRE(view[0] == 1);
        REQUIRE(view[4] == 5);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_pointer_size_constructor_constexpr());
    }
}

constexpr bool test_pointer_range_constructor_constexpr() {
    int data[] = {10, 20, 30};
    array_view<int> view(data, data + 3);
    return view.size() == 3 && view[0] == 10 && view[1] == 20 && view[2] == 30;
}

TEST_CASE("array_view pointer range constructor", "[array_view]") {
    SECTION("runtime") {
        int data[] = {10, 20, 30};
        array_view<int> view(data, data + 3);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 10);
        REQUIRE(view[1] == 20);
        REQUIRE(view[2] == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_pointer_range_constructor_constexpr());
    }
}

constexpr bool test_c_array_constructor_constexpr() {
    int data[] = {5, 6, 7, 8};
    array_view<int> view(data);
    return view.size() == 4 && view[0] == 5 && view[3] == 8;
}

TEST_CASE("array_view C-array constructor", "[array_view]") {
    SECTION("runtime") {
        int data[] = {5, 6, 7, 8};
        array_view<int> view(data);
        REQUIRE(view.size() == 4);
        REQUIRE(view[0] == 5);
        REQUIRE(view[3] == 8);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_c_array_constructor_constexpr());
    }
}

constexpr bool test_size_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    return view.size() == 3;
}

TEST_CASE("array_view size", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        REQUIRE(view.size() == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_size_constexpr());
    }
}

constexpr bool test_empty_constexpr() {
    array_view<int> view;
    return view.empty();
}

TEST_CASE("array_view empty", "[array_view]") {
    SECTION("runtime") {
        array_view<int> view;
        REQUIRE(view.empty());
        int data[] = {1};
        array_view<int> view2(data, 1);
        REQUIRE_FALSE(view2.empty());
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_empty_constexpr());
    }
}

constexpr bool test_operator_subscript_constexpr() {
    int data[] = {10, 20, 30};
    array_view<int> view(data, 3);
    return view[0] == 10 && view[1] == 20 && view[2] == 30;
}

TEST_CASE("array_view operator[]", "[array_view]") {
    SECTION("runtime") {
        int data[] = {10, 20, 30};
        array_view<int> view(data, 3);
        REQUIRE(view[0] == 10);
        REQUIRE(view[1] == 20);
        REQUIRE(view[2] == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_operator_subscript_constexpr());
    }
}

constexpr bool test_at_constexpr() {
    int data[] = {10, 20, 30};
    array_view<int> view(data, 3);
    return view.at(0) == 10 && view.at(1) == 20 && view.at(2) == 30;
}

TEST_CASE("array_view at", "[array_view]") {
    SECTION("runtime") {
        int data[] = {10, 20, 30};
        array_view<int> view(data, 3);
        REQUIRE(view.at(0) == 10);
        REQUIRE(view.at(1) == 20);
        REQUIRE(view.at(2) == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_at_constexpr());
    }
}

constexpr bool test_front_constexpr() {
    int data[] = {42, 10, 20};
    array_view<int> view(data, 3);
    return view.front() == 42;
}

TEST_CASE("array_view front", "[array_view]") {
    SECTION("runtime") {
        int data[] = {42, 10, 20};
        array_view<int> view(data, 3);
        REQUIRE(view.front() == 42);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_front_constexpr());
    }
}

constexpr bool test_back_constexpr() {
    int data[] = {10, 20, 99};
    array_view<int> view(data, 3);
    return view.back() == 99;
}

TEST_CASE("array_view back", "[array_view]") {
    SECTION("runtime") {
        int data[] = {10, 20, 99};
        array_view<int> view(data, 3);
        REQUIRE(view.back() == 99);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_back_constexpr());
    }
}

constexpr bool test_data_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    return view.data() != nullptr && *view.data() == 1;
}

TEST_CASE("array_view data", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        REQUIRE(view.data() != nullptr);
        REQUIRE(*view.data() == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_data_constexpr());
    }
}

constexpr bool test_begin_end_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    return *view.begin() == 1 && *(view.end() - 1) == 3;
}

TEST_CASE("array_view begin/end", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        REQUIRE(*view.begin() == 1);
        REQUIRE(*(view.end() - 1) == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_begin_end_constexpr());
    }
}

constexpr bool test_cbegin_cend_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    return *view.cbegin() == 1 && *(view.cend() - 1) == 3;
}

TEST_CASE("array_view cbegin/cend", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        REQUIRE(*view.cbegin() == 1);
        REQUIRE(*(view.cend() - 1) == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_cbegin_cend_constexpr());
    }
}

constexpr bool test_rbegin_rend_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    return *view.rbegin() == 3 && *(view.rend() - 1) == 1;
}

TEST_CASE("array_view rbegin/rend", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        REQUIRE(*view.rbegin() == 3);
        REQUIRE(*(view.rend() - 1) == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_rbegin_rend_constexpr());
    }
}

constexpr bool test_crbegin_crend_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    return *view.crbegin() == 3 && *(view.crend() - 1) == 1;
}

TEST_CASE("array_view crbegin/crend", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        REQUIRE(*view.crbegin() == 3);
        REQUIRE(*(view.crend() - 1) == 1);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_crbegin_crend_constexpr());
    }
}

constexpr bool test_std_array_constructor_constexpr() {
    std::array<int, 4> arr = {1, 2, 3, 4};
    array_view<int> view(arr);
    return view.size() == 4 && view[0] == 1 && view[3] == 4;
}

TEST_CASE("array_view std::array constructor", "[array_view]") {
    SECTION("runtime") {
        std::array<int, 4> arr = {1, 2, 3, 4};
        array_view<int> view(arr);
        REQUIRE(view.size() == 4);
        REQUIRE(view[0] == 1);
        REQUIRE(view[3] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_std_array_constructor_constexpr());
    }
}

constexpr bool test_const_std_array_constructor_constexpr() {
    const std::array<int, 3> arr = {10, 20, 30};
    array_view<int> view(arr);
    return view.size() == 3 && view[0] == 10 && view[2] == 30;
}

TEST_CASE("array_view const std::array constructor", "[array_view]") {
    SECTION("runtime") {
        const std::array<int, 3> arr = {10, 20, 30};
        array_view<int> view(arr);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 10);
        REQUIRE(view[2] == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_const_std_array_constructor_constexpr());
    }
}

constexpr bool test_rainy_array_constructor_constexpr() {
    rainy::collections::array<int, 4> arr = {5, 6, 7, 8};
    array_view<int> view(arr);
    return view.size() == 4 && view[0] == 5 && view[3] == 8;
}

TEST_CASE("array_view rainy::array constructor", "[array_view]") {
    SECTION("runtime") {
        rainy::collections::array<int, 4> arr = {5, 6, 7, 8};
        array_view<int> view(arr);
        REQUIRE(view.size() == 4);
        REQUIRE(view[0] == 5);
        REQUIRE(view[3] == 8);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_rainy_array_constructor_constexpr());
    }
}

constexpr bool test_const_rainy_array_constructor_constexpr() {
    const rainy::collections::array<int, 3> arr = {100, 200, 300};
    array_view<int> view(arr);
    return view.size() == 3 && view[0] == 100 && view[2] == 300;
}

TEST_CASE("array_view const rainy::array constructor", "[array_view]") {
    SECTION("runtime") {
        const rainy::collections::array<int, 3> arr = {100, 200, 300};
        array_view<int> view(arr);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 100);
        REQUIRE(view[2] == 300);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_const_rainy_array_constructor_constexpr());
    }
}

constexpr bool test_make_array_view_pointer_range_constexpr() {
    int data[] = {1, 2, 3, 4};
    auto view = make_array_view(data, data + 4);
    return view.size() == 4 && view[0] == 1 && view[3] == 4;
}

TEST_CASE("array_view make_array_view pointer range", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3, 4};
        auto view = make_array_view(data, data + 4);
        REQUIRE(view.size() == 4);
        REQUIRE(view[0] == 1);
        REQUIRE(view[3] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_make_array_view_pointer_range_constexpr());
    }
}

constexpr bool test_make_array_view_c_array_constexpr() {
    int data[] = {10, 20, 30};
    auto view = make_array_view(data);
    return view.size() == 3 && view[0] == 10 && view[2] == 30;
}

TEST_CASE("array_view make_array_view C-array", "[array_view]") {
    SECTION("runtime") {
        int data[] = {10, 20, 30};
        auto view = make_array_view(data);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 10);
        REQUIRE(view[2] == 30);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_make_array_view_c_array_constexpr());
    }
}

constexpr bool test_make_array_view_std_array_constexpr() {
    std::array<int, 4> arr = {1, 2, 3, 4};
    auto view = make_array_view(arr);
    return view.size() == 4 && view[0] == 1 && view[3] == 4;
}

TEST_CASE("array_view make_array_view std::array", "[array_view]") {
    SECTION("runtime") {
        std::array<int, 4> arr = {1, 2, 3, 4};
        auto view = make_array_view(arr);
        REQUIRE(view.size() == 4);
        REQUIRE(view[0] == 1);
        REQUIRE(view[3] == 4);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_make_array_view_std_array_constexpr());
    }
}

constexpr bool test_make_array_view_const_std_array_constexpr() {
    const std::array<int, 3> arr = {5, 6, 7};
    auto view = make_array_view(arr);
    return view.size() == 3 && view[0] == 5 && view[2] == 7;
}

TEST_CASE("array_view make_array_view const std::array", "[array_view]") {
    SECTION("runtime") {
        const std::array<int, 3> arr = {5, 6, 7};
        auto view = make_array_view(arr);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 5);
        REQUIRE(view[2] == 7);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_make_array_view_const_std_array_constexpr());
    }
}

constexpr bool test_make_array_view_rainy_array_constexpr() {
    rainy::collections::array<int, 3> arr = {7, 8, 9};
    auto view = make_array_view(arr);
    return view.size() == 3 && view[0] == 7 && view[2] == 9;
}

TEST_CASE("array_view make_array_view rainy::array", "[array_view]") {
    SECTION("runtime") {
        rainy::collections::array<int, 3> arr = {7, 8, 9};
        auto view = make_array_view(arr);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 7);
        REQUIRE(view[2] == 9);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_make_array_view_rainy_array_constexpr());
    }
}

constexpr bool test_make_array_view_const_rainy_array_constexpr() {
    const rainy::collections::array<int, 3> arr = {11, 22, 33};
    auto view = make_array_view(arr);
    return view.size() == 3 && view[0] == 11 && view[2] == 33;
}

TEST_CASE("array_view make_array_view const rainy::array", "[array_view]") {
    SECTION("runtime") {
        const rainy::collections::array<int, 3> arr = {11, 22, 33};
        auto view = make_array_view(arr);
        REQUIRE(view.size() == 3);
        REQUIRE(view[0] == 11);
        REQUIRE(view[2] == 33);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_make_array_view_const_rainy_array_constexpr());
    }
}

constexpr bool test_converting_constructor_constexpr() {
    int data[] = {1, 2, 3};
    array_view<int> view(data, 3);
    array_view<int> const_view(view);
    return const_view.size() == 3 && const_view[0] == 1 && const_view[2] == 3;
}

TEST_CASE("array_view converting constructor", "[array_view]") {
    SECTION("runtime") {
        int data[] = {1, 2, 3};
        array_view<int> view(data, 3);
        array_view<int> const_view(view);
        REQUIRE(const_view.size() == 3);
        REQUIRE(const_view[0] == 1);
        REQUIRE(const_view[2] == 3);
    }
    SECTION("compile time") {
        STATIC_REQUIRE(test_converting_constructor_constexpr());
    }
}