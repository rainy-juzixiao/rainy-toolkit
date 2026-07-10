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
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <iterator>
#include <vector>
#include <list>
#include <array>
#include <string>
#include <cstring>

namespace ra = rainy::core::algorithm;

TEST_CASE("fill_n fills raw array correctly", "[fill_n][basic_algorithm]") {
    int arr[5] = {1, 2, 3, 4, 5};

    SECTION("fill entire array") {
        auto result = ra::fill_n(arr, 5, 42);
        REQUIRE(result == arr + 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(arr[i] == 42);
        }
    }

    SECTION("fill partial array") {
        auto result = ra::fill_n(arr, 3, 99);
        REQUIRE(result == arr + 3);
        REQUIRE(arr[0] == 99);
        REQUIRE(arr[1] == 99);
        REQUIRE(arr[2] == 99);
        REQUIRE(arr[3] == 4);
        REQUIRE(arr[4] == 5);
    }

    SECTION("fill with zero count") {
        auto result = ra::fill_n(arr, 0, 99);
        REQUIRE(result == arr);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[4] == 5);
    }
}

TEST_CASE("fill_n with vector", "[fill_n][basic_algorithm]") {
    std::vector<int> vec(5, 0);

    SECTION("fill vector") {
        ra::fill_n(vec.begin(), 5, 7);
        for (const auto& v : vec) {
            REQUIRE(v == 7);
        }
    }

    SECTION("fill partial vector") {
        ra::fill_n(vec.begin(), 3, 8);
        REQUIRE(vec[0] == 8);
        REQUIRE(vec[1] == 8);
        REQUIRE(vec[2] == 8);
        REQUIRE(vec[3] == 0);
        REQUIRE(vec[4] == 0);
    }
}

TEST_CASE("fill_n with different value types", "[fill_n][basic_algorithm]") {
    SECTION("double") {
        double arr[4] = {0.0};
        ra::fill_n(arr, 4, 3.14);
        for (const auto& v : arr) {
            REQUIRE(v == Catch::Approx(3.14));
        }
    }

    SECTION("string") {
        std::string arr[3];
        ra::fill_n(arr, 3, std::string("hello"));
        for (const auto& v : arr) {
            REQUIRE(v == "hello");
        }
    }

    SECTION("char") {
        char arr[4] = {};
        ra::fill_n(arr, 3, 'A');
        REQUIRE(arr[0] == 'A');
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'A');
        REQUIRE(arr[3] == '\0');
    }
}

#if __cplusplus >= 201703L
TEST_CASE("fill_n is constexpr-friendly", "[fill_n][basic_algorithm][constexpr]") {
    constexpr auto do_fill = []() -> bool {
        int arr[4] = {0};
        auto* end = ra::fill_n(arr, 4, 10);
        return end == arr + 4 && arr[0] == 10 && arr[1] == 10 && arr[2] == 10 && arr[3] == 10;
    };
    STATIC_REQUIRE(do_fill());
}
#endif

TEST_CASE("copy_n between raw arrays", "[copy_n][basic_algorithm]") {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};

    SECTION("copy all elements") {
        auto result = ra::copy_n(src, 5, dst);
        REQUIRE(result == dst + 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(dst[i] == src[i]);
        }
    }

    SECTION("copy partial") {
        auto result = ra::copy_n(src, 3, dst);
        REQUIRE(result == dst + 3);
        REQUIRE(dst[0] == 10);
        REQUIRE(dst[1] == 20);
        REQUIRE(dst[2] == 30);
        REQUIRE(dst[3] == 0);
        REQUIRE(dst[4] == 0);
    }

    SECTION("copy with zero count") {
        auto result = ra::copy_n(src, 0, dst);
        REQUIRE(result == dst);
        REQUIRE(dst[0] == 0);
    }
}

TEST_CASE("copy_n between pointers (trivially-copyable fast path)", "[copy_n][basic_algorithm]") {
    // The trivially-copyable + pointer + pointer branch uses memcpy internally.
    int src[4] = {1, 2, 3, 4};
    int dst[4] = {0};

    ra::copy_n(static_cast<int*>(src), 4, static_cast<int*>(dst));
    REQUIRE(dst[0] == 1);
    REQUIRE(dst[1] == 2);
    REQUIRE(dst[2] == 3);
    REQUIRE(dst[3] == 4);
}

TEST_CASE("copy_n with vector and array", "[copy_n][basic_algorithm]") {
    std::vector<int> src = {100, 200, 300, 400};
    int dst[4] = {0};

    SECTION("vector iterator to raw pointer") {
        auto result = ra::copy_n(src.begin(), 4, dst);
        REQUIRE(result == dst + 4);
        REQUIRE(dst[0] == 100);
        REQUIRE(dst[1] == 200);
        REQUIRE(dst[2] == 300);
        REQUIRE(dst[3] == 400);
    }

    SECTION("raw pointer to vector iterator") {
        int src_arr[3] = {5, 6, 7};
        std::vector<int> vec(3, 0);
        ra::copy_n(src_arr, 3, vec.begin());
        REQUIRE(vec[0] == 5);
        REQUIRE(vec[1] == 6);
        REQUIRE(vec[2] == 7);
    }
}

TEST_CASE("copy_n with list (non-contiguous iterator)", "[copy_n][basic_algorithm]") {
    std::list<int> src = {1, 2, 3, 4, 5};
    std::vector<int> dst(5, 0);

    // list iterators are not pointers, so this exercises the generic loop path
    ra::copy_n(src.begin(), 5, dst.begin());

    REQUIRE(dst[0] == 1);
    REQUIRE(dst[1] == 2);
    REQUIRE(dst[2] == 3);
    REQUIRE(dst[3] == 4);
    REQUIRE(dst[4] == 5);
}

TEST_CASE("copy_n with non-trivially-copyable types", "[copy_n][basic_algorithm]") {
    SECTION("strings") {
        std::string src[3] = {"alpha", "beta", "gamma"};
        std::string dst[3];
        ra::copy_n(src, 3, dst);
        REQUIRE(dst[0] == "alpha");
        REQUIRE(dst[1] == "beta");
        REQUIRE(dst[2] == "gamma");
    }

    SECTION("strings from vector to array") {
        std::vector<std::string> src_vec = {"x", "y", "z"};
        std::string dst_arr[3];
        ra::copy_n(src_vec.begin(), 3, dst_arr);
        REQUIRE(dst_arr[0] == "x");
        REQUIRE(dst_arr[1] == "y");
        REQUIRE(dst_arr[2] == "z");
    }
}

TEST_CASE("copy_n preserves const-correctness", "[copy_n][basic_algorithm]") {
    const int src[3] = {7, 8, 9};
    int dst[3] = {0};
    ra::copy_n(src, 3, dst);
    REQUIRE(dst[0] == 7);
    REQUIRE(dst[1] == 8);
    REQUIRE(dst[2] == 9);
}

TEST_CASE("copy_n return value", "[copy_n][basic_algorithm]") {
    int src[3] = {1, 2, 3};
    int dst[5] = {0};
    auto it = ra::copy_n(src, 3, dst);
    // returns dest + count — points one-past-the-last-copied element
    REQUIRE(it == dst + 3);
}

#if __cplusplus >= 201703L
TEST_CASE("copy_n constexpr evaluation", "[copy_n][basic_algorithm][constexpr]") {
    constexpr auto do_copy = []() -> bool {
        int src[3] = {10, 20, 30};
        int dst[3] = {0};
        auto* end = ra::copy_n(src, 3, dst);
        return end == dst + 3 && dst[0] == 10 && dst[1] == 20 && dst[2] == 30;
    };
    STATIC_REQUIRE(do_copy());
}
#endif
