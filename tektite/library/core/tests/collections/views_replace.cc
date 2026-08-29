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
#include <rainy/core/collections/views/transform/replace.hpp>

#include <vector>

namespace views = rainy::collections::views;

TEST_CASE("replace_view lazily substitutes matching elements", "[collections][views][replace]") {
    std::vector<int> data{1, 2, 1, 3};
    auto view = views::replace(data, 1, 42);

    SECTION("iteration yields replaced values") {
        std::vector<int> collected;
        for (int value : view) {
            collected.push_back(value);
        }
        REQUIRE((collected == std::vector<int>{42, 2, 42, 3}));
    }
    SECTION("the view does not modify the source") {
        std::vector<int> ignore{view.begin(), view.end()};
        (void) ignore;
        REQUIRE((data == std::vector<int>{1, 2, 1, 3}));
    }
    SECTION("size is preserved") {
        REQUIRE(view.size() == 4);
    }
}

TEST_CASE("replace_view supports pipes", "[collections][views][replace]") {
    std::vector<int> data{1, 2, 1, 3};
    std::vector<int> collected;
    for (int value : data | views::replace(1, 42)) {
        collected.push_back(value);
    }
    REQUIRE((collected == std::vector<int>{42, 2, 42, 3}));
}

TEST_CASE("replace_if_view lazily substitutes by predicate", "[collections][views][replace]") {
    std::vector<int> data{1, 8, 3, 10};
    auto view = views::replace_if(data, [](int value) { return value > 5; }, 0);

    std::vector<int> collected;
    for (int value : view) {
        collected.push_back(value);
    }
    REQUIRE((collected == std::vector<int>{1, 0, 3, 0}));

    SECTION("pipe form") {
        std::vector<int> piped;
        for (int value : data | views::replace_if([](int value) { return value > 5; }, 0)) {
            piped.push_back(value);
        }
        REQUIRE((piped == std::vector<int>{1, 0, 3, 0}));
    }
}

TEST_CASE("replace_copy_view yields replaced copies", "[collections][views][replace]") {
    std::vector<int> data{1, 2, 1, 3};
    auto view = views::replace_copy(data, 1, 42);

    std::vector<int> collected{view.begin(), view.end()};
    REQUIRE((collected == std::vector<int>{42, 2, 42, 3}));
    REQUIRE((data == std::vector<int>{1, 2, 1, 3}));

    SECTION("pipe form") {
        auto view = views::replace_copy(data, 1, 42);
        std::vector<int> piped{view.begin(), view.end()};
        REQUIRE((piped == std::vector<int>{42, 2, 42, 3}));
    }
}

TEST_CASE("replace_copy_if_view yields predicate-replaced copies", "[collections][views][replace]") {
    std::vector<int> data{1, 8, 3, 10};
    auto view = views::replace_copy_if(data, [](int value) { return value > 5; }, 0);

    std::vector<int> collected{view.begin(), view.end()};
    REQUIRE((collected == std::vector<int>{1, 0, 3, 0}));

    SECTION("pipe form") {
        std::vector<int> piped;
        for (int value : data | views::replace_copy_if([](int value) { return value > 5; }, 0)) {
            piped.push_back(value);
        }
        REQUIRE((piped == std::vector<int>{1, 0, 3, 0}));
    }
}

TEST_CASE("replace views compose with other closures", "[collections][views][replace]") {
    std::vector<int> data{1, 2, 1, 3};
    std::vector<int> collected;
    for (int value : data | views::replace(1, 9) | views::replace(3, 7)) {
        collected.push_back(value);
    }
    REQUIRE((collected == std::vector<int>{9, 2, 9, 7}));
}
