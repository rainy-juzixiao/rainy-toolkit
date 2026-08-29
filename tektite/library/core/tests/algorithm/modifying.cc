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
#include <rainy/core/algorithm.hpp>
#include <rainy/core/collections/views/algorithm/modifying.hpp>

#include <list>
#include <vector>

namespace ra = rainy::algorithm;
namespace rv = rainy::collections::views;

TEST_CASE("copy variants", "[algorithm][modifying]") {
    const std::vector<int> source{1, 2, 3, 4};
    std::vector<int> target(4, 0);

    REQUIRE(ra::copy(source.begin(), source.end(), target.begin()) == target.end());
    REQUIRE((target == source));

    std::vector<int> partial(2, 0);
    REQUIRE(ra::copy_n(source.begin(), 2, partial.begin()) == partial.end());
    REQUIRE((partial == std::vector<int>{1, 2}));

    std::vector<int> filtered;
    ra::copy_if(source.begin(), source.end(), std::back_inserter(filtered), [](int v) { return v % 2 == 0; });
    REQUIRE((filtered == std::vector<int>{2, 4}));
}

TEST_CASE("generate fills with generated values", "[algorithm][modifying]") {
    std::vector<int> data(4);
    int counter = 0;
    ra::generate(data.begin(), data.end(), [&counter] { return counter++; });
    REQUIRE((data == std::vector<int>{0, 1, 2, 3}));

    std::vector<int> generated(3);
    counter = 10;
    REQUIRE(ra::generate_n(generated.begin(), 2, [&counter] { return counter++; }) == generated.begin() + 2);
    REQUIRE((generated == std::vector<int>{10, 11, 0}));
}

TEST_CASE("swap and iter_swap exchange values", "[algorithm][modifying]") {
    int a = 1;
    int b = 2;
    ra::swap(a, b);
    REQUIRE(a == 2);
    REQUIRE(b == 1);

    std::vector<int> data{1, 2};
    ra::iter_swap(data.begin(), data.begin() + 1);
    REQUIRE((data == std::vector<int>{2, 1}));
}

TEST_CASE("remove variants compact without resizing", "[algorithm][modifying]") {
    std::vector<int> data{1, 2, 1, 3, 1};

    auto end = ra::remove(data.begin(), data.end(), 1);
    data.erase(end, data.end());
    REQUIRE((data == std::vector<int>{2, 3}));

    std::vector<int> numbers{1, 8, 3, 10};
    auto pred_end = ra::remove_if(numbers.begin(), numbers.end(), [](int v) { return v > 5; });
    numbers.erase(pred_end, numbers.end());
    REQUIRE((numbers == std::vector<int>{1, 3}));

    const std::vector<int> source{1, 2, 1, 3};
    std::vector<int> copied;
    ra::remove_copy(source.begin(), source.end(), std::back_inserter(copied), 1);
    REQUIRE((copied == std::vector<int>{2, 3}));

    std::vector<int> filtered;
    ra::remove_copy_if(source.begin(), source.end(), std::back_inserter(filtered), [](int v) { return v > 2; });
    REQUIRE((filtered == std::vector<int>{1, 2, 1}));
}

TEST_CASE("reverse manipulates order", "[algorithm][modifying]") {
    std::vector<int> data{1, 2, 3, 4};
    ra::reverse(data.begin(), data.end());
    REQUIRE((data == std::vector<int>{4, 3, 2, 1}));

    ra::reverse(data.begin() + 1, data.end() - 1);
    REQUIRE((data == std::vector<int>{4, 2, 3, 1}));

    const std::vector<int> source{1, 2, 3};
    std::vector<int> reversed(3, 0);
    ra::reverse_copy(source.begin(), source.end(), reversed.begin());
    REQUIRE((reversed == std::vector<int>{3, 2, 1}));
    REQUIRE((source == std::vector<int>{1, 2, 3}));

    std::list<int> list_data{1, 2, 3};
    ra::reverse(list_data.begin(), list_data.end());
    REQUIRE((list_data == std::list<int>{3, 2, 1}));
}

TEST_CASE("rotate moves elements to a new starting point", "[algorithm][modifying]") {
    std::vector<int> data{1, 2, 3, 4, 5};
    auto result = ra::rotate(data.begin(), data.begin() + 2, data.end());
    REQUIRE((data == std::vector<int>{3, 4, 5, 1, 2}));
    REQUIRE(result == data.begin() + 3);

    std::vector<int> trivial{1, 2};
    REQUIRE(ra::rotate(trivial.begin(), trivial.begin(), trivial.end()) == trivial.begin());
    REQUIRE(ra::rotate(trivial.begin(), trivial.end(), trivial.end()) == trivial.begin());

    const std::vector<int> source{1, 2, 3, 4};
    std::vector<int> rotated(4, 0);
    ra::rotate_copy(source.begin(), source.begin() + 2, source.end(), rotated.begin());
    REQUIRE((rotated == std::vector<int>{3, 4, 1, 2}));
}

TEST_CASE("unique removes consecutive duplicates", "[algorithm][modifying]") {
    std::vector<int> data{1, 1, 2, 2, 2, 3};

    auto end = ra::unique(data.begin(), data.end());
    data.erase(end, data.end());
    REQUIRE((data == std::vector<int>{1, 2, 3}));

    std::vector<int> absolute{1, -1, 2, -2};
    auto pred_end = ra::unique(absolute.begin(), absolute.end(), [](int a, int b) {
        return (a < 0 ? -a : a) == (b < 0 ? -b : b);
    });
    absolute.erase(pred_end, absolute.end());
    REQUIRE((absolute == std::vector<int>{1, 2}));

    const std::vector<int> source{1, 1, 2, 3, 3};
    std::vector<int> copied;
    ra::unique_copy(source.begin(), source.end(), std::back_inserter(copied));
    REQUIRE((copied == std::vector<int>{1, 2, 3}));
}

TEST_CASE("shift_left and shift_right discard shifted-out elements", "[algorithm][modifying]") {
    std::vector<int> data{1, 2, 3, 4, 5};

    auto left_end = ra::shift_left(data.begin(), data.end(), 2);
    REQUIRE(left_end == data.begin() + 3);
    REQUIRE((std::vector<int>{data.begin(), left_end} == std::vector<int>{3, 4, 5}));

    std::vector<int> right_data{1, 2, 3, 4, 5};
    auto right_begin = ra::shift_right(right_data.begin(), right_data.end(), 2);
    REQUIRE(right_begin == right_data.begin() + 2);
    REQUIRE((right_data == std::vector<int>{1, 2, 1, 2, 3}));

    REQUIRE(ra::shift_left(data.begin(), data.end(), 0) == data.end());
    REQUIRE(ra::shift_right(right_data.begin(), right_data.end(), 0) == right_data.begin());
    REQUIRE(ra::shift_left(data.begin(), data.end(), 100) == data.begin());
    REQUIRE(ra::shift_right(right_data.begin(), right_data.end(), 100) == right_data.end());
}

TEST_CASE("ranges modifying algorithms delegate to the range form", "[collections][views][algorithm]") {
    std::vector<int> data{1, 2, 3, 4};

    std::vector<int> copied;
    rv::copy(data, std::back_inserter(copied));
    REQUIRE((copied == data));

    std::vector<int> even;
    rv::copy_if(data, std::back_inserter(even), [](int v) { return v % 2 == 0; });
    REQUIRE((even == std::vector<int>{2, 4}));

    rv::generate(data, [] { return 7; });
    REQUIRE((data == std::vector<int>{7, 7, 7, 7}));

    std::vector<int> with_dups{1, 1, 2, 3};
    auto unique_end = rv::unique(with_dups);
    with_dups.erase(unique_end, with_dups.end());
    REQUIRE((with_dups == std::vector<int>{1, 2, 3}));

    std::vector<int> removable{1, 2, 1};
    auto remove_end = rv::remove(removable, 1);
    removable.erase(remove_end, removable.end());
    REQUIRE((removable == std::vector<int>{2}));

    std::vector<int> reversible{1, 2, 3};
    rv::reverse(reversible);
    REQUIRE((reversible == std::vector<int>{3, 2, 1}));

    std::vector<int> rotatable{1, 2, 3};
    rv::rotate(rotatable, rotatable.begin() + 1);
    REQUIRE((rotatable == std::vector<int>{2, 3, 1}));

    std::vector<int> shifted{1, 2, 3, 4};
    rv::shift_left(shifted, 1);
    REQUIRE((std::vector<int>{shifted.begin(), shifted.end() - 1} == std::vector<int>{2, 3, 4}));
}
