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

TEST_CASE("swap_ranges between arrays", "[swap_ranges][basic_algorithm]") {
    int a[5] = {1, 2, 3, 4, 5};
    int b[5] = {10, 20, 30, 40, 50};

    SECTION("swap full ranges") {
        auto result = ra::swap_ranges(a, a + 5, b);
        REQUIRE(result == b + 5);
        REQUIRE(a[0] == 10);
        REQUIRE(a[1] == 20);
        REQUIRE(a[2] == 30);
        REQUIRE(a[3] == 40);
        REQUIRE(a[4] == 50);
        REQUIRE(b[0] == 1);
        REQUIRE(b[1] == 2);
        REQUIRE(b[2] == 3);
        REQUIRE(b[3] == 4);
        REQUIRE(b[4] == 5);
    }

    SECTION("swap partial ranges") {
        auto result = ra::swap_ranges(a, a + 3, b);
        REQUIRE(result == b + 3);
        REQUIRE(a[0] == 10);
        REQUIRE(a[1] == 20);
        REQUIRE(a[2] == 30);
        REQUIRE(a[3] == 4);
        REQUIRE(a[4] == 5);
        REQUIRE(b[0] == 1);
        REQUIRE(b[1] == 2);
        REQUIRE(b[2] == 3);
        REQUIRE(b[3] == 40);
        REQUIRE(b[4] == 50);
    }
}

TEST_CASE("swap_ranges with vectors", "[swap_ranges][basic_algorithm]") {
    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {4, 5, 6};

    ra::swap_ranges(v1.begin(), v1.end(), v2.begin());
    REQUIRE(v1[0] == 4);
    REQUIRE(v1[1] == 5);
    REQUIRE(v1[2] == 6);
    REQUIRE(v2[0] == 1);
    REQUIRE(v2[1] == 2);
    REQUIRE(v2[2] == 3);
}

TEST_CASE("swap_ranges with different container types", "[swap_ranges][basic_algorithm]") {
    std::array<int, 3> arr = {1, 2, 3};
    std::vector<int> vec = {4, 5, 6};

    ra::swap_ranges(arr.begin(), arr.end(), vec.begin());
    REQUIRE(arr[0] == 4);
    REQUIRE(arr[1] == 5);
    REQUIRE(arr[2] == 6);
    REQUIRE(vec[0] == 1);
    REQUIRE(vec[1] == 2);
    REQUIRE(vec[2] == 3);
}

TEST_CASE("fill algorithm", "[fill][basic_algorithm]") {
    SECTION("fill array") {
        int arr[5] = {1, 2, 3, 4, 5};
        ra::fill(arr, arr + 5, 42);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(arr[i] == 42);
        }
    }

    SECTION("fill vector") {
        std::vector<int> vec(5, 0);
        ra::fill(vec.begin(), vec.end(), 7);
        for (const auto& v : vec) {
            REQUIRE(v == 7);
        }
    }

    SECTION("fill partial range") {
        int arr[5] = {1, 2, 3, 4, 5};
        ra::fill(arr + 1, arr + 4, 99);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 99);
        REQUIRE(arr[2] == 99);
        REQUIRE(arr[3] == 99);
        REQUIRE(arr[4] == 5);
    }
}

TEST_CASE("all_of", "[all_of][basic_algorithm]") {
    int arr[5] = {2, 4, 6, 8, 10};

    SECTION("all even") {
        bool result = ra::all_of(arr, arr + 5, [](int x) { return x % 2 == 0; });
        REQUIRE(result == true);
    }

    SECTION("not all even") {
        arr[2] = 7;
        bool result = ra::all_of(arr, arr + 5, [](int x) { return x % 2 == 0; });
        REQUIRE(result == false);
    }

    SECTION("empty range") {
        bool result = ra::all_of(arr, arr, [](int x) { return x > 100; });
        REQUIRE(result == true);
    }
}

TEST_CASE("any_of", "[any_of][basic_algorithm]") {
    int arr[5] = {1, 3, 5, 7, 9};

    SECTION("any odd") {
        bool result = ra::any_of(arr, arr + 5, [](int x) { return x % 2 == 1; });
        REQUIRE(result == true);
    }

    SECTION("none odd") {
        int even_arr[3] = {2, 4, 6};
        bool result = ra::any_of(even_arr, even_arr + 3, [](int x) { return x % 2 == 1; });
        REQUIRE(result == false);
    }

    SECTION("empty range") {
        bool result = ra::any_of(arr, arr, [](int x) { return x == 1; });
        REQUIRE(result == false);
    }
}

TEST_CASE("none_of", "[none_of][basic_algorithm]") {
    int arr[5] = {2, 4, 6, 8, 10};

    SECTION("none odd") {
        bool result = ra::none_of(arr, arr + 5, [](int x) { return x % 2 == 1; });
        REQUIRE(result == true);
    }

    SECTION("some odd") {
        arr[2] = 7;
        bool result = ra::none_of(arr, arr + 5, [](int x) { return x % 2 == 1; });
        REQUIRE(result == false);
    }

    SECTION("empty range") {
        bool result = ra::none_of(arr, arr, [](int x) { return x == 0; });
        REQUIRE(result == true);
    }
}

TEST_CASE("find", "[find][basic_algorithm]") {
    int arr[5] = {10, 20, 30, 40, 50};

    SECTION("find existing value") {
        auto it = ra::find(arr, arr + 5, 30);
        REQUIRE(it == arr + 2);
        REQUIRE(*it == 30);
    }

    SECTION("find non-existing value") {
        auto it = ra::find(arr, arr + 5, 99);
        REQUIRE(it == arr + 5);
    }

    SECTION("find first occurrence") {
        int dup_arr[5] = {10, 20, 20, 30, 40};
        auto it = ra::find(dup_arr, dup_arr + 5, 20);
        REQUIRE(it == dup_arr + 1);
        REQUIRE(*it == 20);
    }
}

TEST_CASE("find_if", "[find_if][basic_algorithm]") {
    int arr[5] = {1, 3, 5, 7, 9};

    SECTION("find first even") {
        arr[2] = 6;
        auto it = ra::find_if(arr, arr + 5, [](int x) { return x % 2 == 0; });
        REQUIRE(it == arr + 2);
        REQUIRE(*it == 6);
    }

    SECTION("find first greater than threshold") {
        auto it = ra::find_if(arr, arr + 5, [](int x) { return x > 6; });
        REQUIRE(it == arr + 3);
        REQUIRE(*it == 7);
    }

    SECTION("no match") {
        auto it = ra::find_if(arr, arr + 5, [](int x) { return x > 100; });
        REQUIRE(it == arr + 5);
    }
}

TEST_CASE("find_if_not", "[find_if_not][basic_algorithm]") {
    int arr[5] = {2, 4, 6, 8, 10};

    SECTION("find first not even") {
        arr[3] = 9;
        auto it = ra::find_if_not(arr, arr + 5, [](int x) { return x % 2 == 0; });
        REQUIRE(it == arr + 3);
        REQUIRE(*it == 9);
    }

    SECTION("all match predicate") {
        auto it = ra::find_if_not(arr, arr + 5, [](int x) { return x % 2 == 0; });
        REQUIRE(it == arr + 5);
    }
}

TEST_CASE("equal", "[equal][basic_algorithm]") {
    int a[5] = {1, 2, 3, 4, 5};
    int b[5] = {1, 2, 3, 4, 5};
    int c[5] = {1, 2, 3, 4, 6};

    SECTION("equal ranges") {
        bool result = ra::equal(a, a + 5, b, b + 5);
        REQUIRE(result == true);
    }

    SECTION("different ranges") {
        bool result = ra::equal(a, a + 5, c, c + 5);
        REQUIRE(result == false);
    }

    SECTION("different sizes") {
        bool result = ra::equal(a, a + 3, b, b + 5);
        REQUIRE(result == false);
    }

    SECTION("empty ranges") {
        bool result = ra::equal(a, a, b, b);
        REQUIRE(result == true);
    }
}

TEST_CASE("lexicographical_compare", "[lexicographical_compare][basic_algorithm]") {
    SECTION("first less than second") {
        int a[3] = {1, 2, 3};
        int b[3] = {1, 2, 4};
        bool result = ra::lexicographical_compare(a, a + 3, b, b + 3);
        REQUIRE(result == true);
    }

    SECTION("first greater than second") {
        int a[3] = {1, 2, 4};
        int b[3] = {1, 2, 3};
        bool result = ra::lexicographical_compare(a, a + 3, b, b + 3);
        REQUIRE(result == false);
    }

    SECTION("equal ranges") {
        int a[3] = {1, 2, 3};
        int b[3] = {1, 2, 3};
        bool result = ra::lexicographical_compare(a, a + 3, b, b + 3);
        REQUIRE(result == false);
    }

    SECTION("first is prefix of second") {
        int a[3] = {1, 2};
        int b[3] = {1, 2, 3};
        bool result = ra::lexicographical_compare(a, a + 2, b, b + 3);
        REQUIRE(result == true);
    }

    SECTION("second is prefix of first") {
        int a[3] = {1, 2, 3};
        int b[3] = {1, 2};
        bool result = ra::lexicographical_compare(a, a + 3, b, b + 2);
        REQUIRE(result == false);
    }
}

TEST_CASE("lower_bound", "[lower_bound][basic_algorithm]") {
    int arr[8] = {1, 3, 5, 7, 9, 9, 11, 13};

    SECTION("value exists once") {
        auto it = ra::lower_bound(arr, arr + 8, 5, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 2);
        REQUIRE(*it == 5);
    }

    SECTION("value exists multiple times") {
        auto it = ra::lower_bound(arr, arr + 8, 9, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 4);
        REQUIRE(*it == 9);
    }

    SECTION("value not found") {
        auto it = ra::lower_bound(arr, arr + 8, 6, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 3);
        REQUIRE(*it == 7);
    }

    SECTION("value less than all elements") {
        auto it = ra::lower_bound(arr, arr + 8, 0, [](int a, int b) { return a < b; });
        REQUIRE(it == arr);
        REQUIRE(*it == 1);
    }

    SECTION("value greater than all elements") {
        auto it = ra::lower_bound(arr, arr + 8, 20, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 8);
    }
}

TEST_CASE("upper_bound", "[upper_bound][basic_algorithm]") {
    int arr[8] = {1, 3, 5, 7, 9, 9, 11, 13};

    SECTION("value exists once") {
        auto it = ra::upper_bound(arr, arr + 8, 5, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 3);
        REQUIRE(*it == 7);
    }

    SECTION("value exists multiple times") {
        auto it = ra::upper_bound(arr, arr + 8, 9, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 6);
        REQUIRE(*it == 11);
    }

    SECTION("value not found") {
        auto it = ra::upper_bound(arr, arr + 8, 6, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 3);
        REQUIRE(*it == 7);
    }

    SECTION("value less than all elements") {
        auto it = ra::upper_bound(arr, arr + 8, 0, [](int a, int b) { return a < b; });
        REQUIRE(it == arr);
        REQUIRE(*it == 1);
    }

    SECTION("value greater than all elements") {
        auto it = ra::upper_bound(arr, arr + 8, 20, [](int a, int b) { return a < b; });
        REQUIRE(it == arr + 8);
    }
}

TEST_CASE("binary_search", "[binary_search][basic_algorithm]") {
    int arr[8] = {1, 3, 5, 7, 9, 9, 11, 13};

    SECTION("value exists") {
        bool result = ra::binary_search(arr, arr + 8, 7);
        REQUIRE(result == true);
    }

    SECTION("value exists with duplicates") {
        bool result = ra::binary_search(arr, arr + 8, 9);
        REQUIRE(result == true);
    }

    SECTION("value not found") {
        bool result = ra::binary_search(arr, arr + 8, 6);
        REQUIRE(result == false);
    }

    SECTION("value less than all") {
        bool result = ra::binary_search(arr, arr + 8, 0);
        REQUIRE(result == false);
    }

    SECTION("value greater than all") {
        bool result = ra::binary_search(arr, arr + 8, 20);
        REQUIRE(result == false);
    }
}

TEST_CASE("binary_search with custom comparator", "[binary_search][basic_algorithm]") {
    int arr[5] = {10, 20, 30, 40, 50};

    SECTION("found with comparator") {
        bool result = ra::binary_search(arr, arr + 5, 30, [](int a, int b) { return a < b; });
        REQUIRE(result == true);
    }

    SECTION("not found with comparator") {
        bool result = ra::binary_search(arr, arr + 5, 35, [](int a, int b) { return a < b; });
        REQUIRE(result == false);
    }
}

TEST_CASE("move algorithm", "[move][basic_algorithm]") {
    SECTION("move between arrays") {
        int src[5] = {1, 2, 3, 4, 5};
        int dst[5] = {0};
        auto result = ra::move(src, src + 5, dst);
        REQUIRE(result == dst + 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(dst[i] == src[i]);
        }
    }

    SECTION("move with vectors") {
        std::vector<std::string> src = {"a", "b", "c"};
        std::vector<std::string> dst(3);
        ra::move(src.begin(), src.end(), dst.begin());
        REQUIRE(dst[0] == "a");
        REQUIRE(dst[1] == "b");
        REQUIRE(dst[2] == "c");
    }

    SECTION("move partial") {
        int src[5] = {1, 2, 3, 4, 5};
        int dst[5] = {0};
        ra::move(src + 1, src + 4, dst);
        REQUIRE(dst[0] == 2);
        REQUIRE(dst[1] == 3);
        REQUIRE(dst[2] == 4);
        REQUIRE(dst[3] == 0);
        REQUIRE(dst[4] == 0);
    }
}

TEST_CASE("move_backward", "[move_backward][basic_algorithm]") {
    SECTION("move backward with overlapping ranges") {
        int arr[5] = {1, 2, 3, 4, 5};
        ra::move_backward(arr, arr + 3, arr + 5);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 1);
        REQUIRE(arr[3] == 2);
        REQUIRE(arr[4] == 3);
    }

    SECTION("move backward non-overlapping") {
        int src[5] = {1, 2, 3, 4, 5};
        int dst[5] = {0};
        auto result = ra::move_backward(src, src + 5, dst + 5);
        REQUIRE(result == dst);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(dst[i] == src[i]);
        }
    }
}

TEST_CASE("copy_backward", "[copy_backward][basic_algorithm]") {
    SECTION("copy backward with overlapping ranges") {
        int arr[5] = {1, 2, 3, 4, 5};
        ra::copy_backward(arr, arr + 3, arr + 5);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 1);
        REQUIRE(arr[3] == 2);
        REQUIRE(arr[4] == 3);
    }

    SECTION("copy backward non-overlapping") {
        int src[5] = {1, 2, 3, 4, 5};
        int dst[5] = {0};
        auto result = ra::copy_backward(src, src + 5, dst + 5);
        REQUIRE(result == dst);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(dst[i] == src[i]);
        }
    }
}

TEST_CASE("transform unary", "[transform][basic_algorithm]") {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};

    SECTION("square each element") {
        auto result = ra::transform(src, src + 5, dst, [](int x) { return x * x; });
        REQUIRE(result == dst + 5);
        REQUIRE(dst[0] == 1);
        REQUIRE(dst[1] == 4);
        REQUIRE(dst[2] == 9);
        REQUIRE(dst[3] == 16);
        REQUIRE(dst[4] == 25);
    }

    SECTION("add 10 to each element") {
        ra::transform(src, src + 5, dst, [](int x) { return x + 10; });
        REQUIRE(dst[0] == 11);
        REQUIRE(dst[1] == 12);
        REQUIRE(dst[2] == 13);
        REQUIRE(dst[3] == 14);
        REQUIRE(dst[4] == 15);
    }

    SECTION("transform to string") {
        std::string dst_str[5];
        ra::transform(src, src + 5, dst_str, [](int x) { return std::to_string(x); });
        REQUIRE(dst_str[0] == "1");
        REQUIRE(dst_str[1] == "2");
        REQUIRE(dst_str[2] == "3");
        REQUIRE(dst_str[3] == "4");
        REQUIRE(dst_str[4] == "5");
    }
}

TEST_CASE("transform binary", "[transform][basic_algorithm]") {
    int a[5] = {1, 2, 3, 4, 5};
    int b[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};

    SECTION("add elements") {
        auto result = ra::transform(a, a + 5, b, dst, [](int x, int y) { return x + y; });
        REQUIRE(result == dst + 5);
        REQUIRE(dst[0] == 11);
        REQUIRE(dst[1] == 22);
        REQUIRE(dst[2] == 33);
        REQUIRE(dst[3] == 44);
        REQUIRE(dst[4] == 55);
    }

    SECTION("multiply elements") {
        ra::transform(a, a + 5, b, dst, [](int x, int y) { return x * y; });
        REQUIRE(dst[0] == 10);
        REQUIRE(dst[1] == 40);
        REQUIRE(dst[2] == 90);
        REQUIRE(dst[3] == 160);
        REQUIRE(dst[4] == 250);
    }

    SECTION("different sized ranges") {
        auto result = ra::transform(a, a + 3, b, dst, [](int x, int y) { return x + y; });
        REQUIRE(result == dst + 3);
        REQUIRE(dst[0] == 11);
        REQUIRE(dst[1] == 22);
        REQUIRE(dst[2] == 33);
        REQUIRE(dst[3] == 0);
        REQUIRE(dst[4] == 0);
    }
}

#if __cplusplus >= 201703L
TEST_CASE("transform constexpr", "[transform][basic_algorithm][constexpr]") {
    constexpr auto do_transform = []() -> bool {
        int src[3] = {1, 2, 3};
        int dst[3] = {0};
        auto* end = ra::transform(src, src + 3, dst, [](int x) { return x * 2; });
        return end == dst + 3 && dst[0] == 2 && dst[1] == 4 && dst[2] == 6;
    };
    STATIC_REQUIRE(do_transform());
}
#endif