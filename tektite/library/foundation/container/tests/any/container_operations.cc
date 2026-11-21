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
#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/any.hpp>

#include <string>
#include <vector>

using namespace rainy::foundation::container;

TEST_CASE("any - container - begin and end on vector int", "[any]") {
    any vec = std::vector<int>{10, 20, 30, 40, 50};

    auto it = vec.begin();
    CHECK(it != vec.end());

    std::vector<int> collected;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        collected.push_back((*iter).as<int>());
    }
    CHECK(collected.size() == 5);
    CHECK(collected[0] == 10);
    CHECK(collected[1] == 20);
    CHECK(collected[2] == 30);
    CHECK(collected[3] == 40);
    CHECK(collected[4] == 50);
}

TEST_CASE("any - container - range-for loop on vector", "[any]") {
    any vec = std::vector<int>{1, 2, 3, 4};
    int sum = 0;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        sum += (*iter).as<int>();
    }
    CHECK(sum == 10);
}

TEST_CASE("any - container - begin and end on vector of strings", "[any]") {
    any vec = std::vector<std::string>{"hello", "world", "foo"};

    auto it = vec.begin();
    CHECK(it != vec.end());

    std::vector<std::string> collected;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        collected.push_back((*iter).as<std::string>());
    }
    CHECK(collected.size() == 3);
    CHECK(collected[0] == "hello");
    CHECK(collected[1] == "world");
    CHECK(collected[2] == "foo");
}

TEST_CASE("any - container - const begin and end on vector", "[any]") {
    const any vec = std::vector<int>{1, 2, 3};

    auto it = vec.begin();
    CHECK(it != vec.end());

    int count = 0;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        ++count;
    }
    CHECK(count == 3);
}

TEST_CASE("any - container - begin and end on vector of doubles", "[any]") {
    any vec = std::vector<double>{1.5, 2.5, 3.5};

    auto it = vec.begin();
    CHECK(it != vec.end());

    double sum = 0.0;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        sum += (*iter).as<double>();
    }
    CHECK(sum == 7.5);
}

TEST_CASE("any - container - empty container begin equals end", "[any]") {
    any vec = std::vector<int>{};
    CHECK(vec.begin() == vec.end());
}

TEST_CASE("any - container - size on various vectors", "[any]") {
    any vec = std::vector<int>{1, 2, 3, 4, 5};
    CHECK(vec.size() == 5);

    any empty_vec = std::vector<int>{};
    CHECK(empty_vec.size() == 0);

    any vec2 = std::vector<double>{1.0, 2.0, 3.0};
    CHECK(vec2.size() == 3);

    any vec3 = std::vector<std::string>{"a", "b", "c", "d"};
    CHECK(vec3.size() == 4);
}

TEST_CASE("any - container - resize vector larger", "[any]") {
    any vec = std::vector<int>{1, 2, 3};
    CHECK(vec.size() == 3);

    vec.resize(10);
    CHECK(vec.size() == 10);

    int count = 0;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        ++count;
    }
    CHECK(count == 10);
}

TEST_CASE("any - container - resize vector smaller", "[any]") {
    any vec = std::vector<int>{1, 2, 3, 4, 5};
    CHECK(vec.size() == 5);

    vec.resize(2);
    CHECK(vec.size() == 2);
}

TEST_CASE("any - container - iterate and verify each element", "[any]") {
    any vec = std::vector<int>{100, 200, 300};

    auto it = vec.begin();
    CHECK((*it).as<int>() == 100);
    ++it;
    CHECK((*it).as<int>() == 200);
    ++it;
    CHECK((*it).as<int>() == 300);
    ++it;
    CHECK(it == vec.end());
}

TEST_CASE("any - container - iterate with post-increment", "[any]") {
    any vec = std::vector<std::string>{"a", "b"};

    auto it = vec.begin();
    CHECK((*(it++)).as<std::string>() == "a");
    CHECK((*it).as<std::string>() == "b");
}


