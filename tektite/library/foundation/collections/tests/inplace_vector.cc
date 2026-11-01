/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/foundation/collections/inplace_vector.hpp>

#include <string>

using rainy::foundation::collections::inplace_vector;

TEST_CASE("inplace_vector construction and size", "[inplace_vector]") {
    SECTION("default construction is empty") {
        constexpr inplace_vector<int, 8> vec{};
        STATIC_REQUIRE(vec.size() == 0);
        STATIC_REQUIRE(vec.empty());
    }

    SECTION("initializer list construction") {
        inplace_vector<int, 8> vec{1, 2, 3};
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 3);
    }

    SECTION("count and value construction") {
        inplace_vector<int, 8> vec(std::size_t(3), 9);
        REQUIRE(vec.size() == 3);
        for (auto item: vec) {
            REQUIRE(item == 9);
        }
    }

    SECTION("iterator pair construction") {
        rainy::core::collections::array<int, 3> src{4, 5, 6};
        inplace_vector<int, 8> vec(src.begin(), src.end());
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[2] == 6);
    }
}

TEST_CASE("inplace_vector element access", "[inplace_vector]") {
    inplace_vector<int, 8> vec{1, 2, 3};

    SECTION("front and back") {
        REQUIRE(vec.front() == 1);
        REQUIRE(vec.back() == 3);
    }

    SECTION("iteration covers all elements in order") {
        int expected = 1;
        for (auto item: vec) {
            REQUIRE(item == expected++);
        }
    }

    SECTION("at returns a mutable reference") {
        vec.at(1) = 20;
        REQUIRE(vec[1] == 20);
    }

    SECTION("slice produces a sub-vector") {
        auto sub = vec.slice<2>(1, 3);
        STATIC_REQUIRE(decltype(sub)::capacity() == 2);
        REQUIRE(sub.size() == 2);
        REQUIRE(sub[0] == 2);
        REQUIRE(sub[1] == 3);
    }
}

TEST_CASE("inplace_vector grows and shrinks", "[inplace_vector]") {
    SECTION("push_back and pop_back") {
        inplace_vector<std::string, 4> vec;
        vec.push_back("a");
        vec.push_back("b");
        REQUIRE(vec.size() == 2);
        REQUIRE(vec.back() == std::string("b"));
        vec.pop_back();
        REQUIRE(vec.size() == 1);
        REQUIRE(vec.back() == std::string("a"));
    }

    SECTION("emplace_back constructs in place") {
        inplace_vector<std::string, 4> vec;
        vec.emplace_back(3, 'x');
        REQUIRE(vec.size() == 1);
        REQUIRE(vec.back() == std::string("xxx"));
    }

    SECTION("try_push_back refuses past capacity") {
        inplace_vector<int, 2> vec{1, 2};
        REQUIRE(vec.try_push_back(3) == nullptr);
        REQUIRE(vec.size() == 2);
    }

    SECTION("resize fills new slots") {
        inplace_vector<int, 8> vec{1};
        vec.resize(3, 7);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[1] == 7);
        REQUIRE(vec[2] == 7);
        vec.resize(1);
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == 1);
    }
}

TEST_CASE("inplace_vector insert and erase", "[inplace_vector]") {
    inplace_vector<int, 8> vec{1, 4};

    SECTION("insert at position shifts the tail") {
        auto it = vec.insert(vec.begin() + 1, 2);
        REQUIRE(*it == 2);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 4);
    }

    SECTION("insert with count") {
        vec.insert(vec.begin() + 1, std::size_t(2), 9);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[1] == 9);
        REQUIRE(vec[2] == 9);
        REQUIRE(vec[3] == 4);
    }

    SECTION("insert from iterator range") {
        rainy::core::collections::array<int, 2> extra{2, 3};
        vec.insert(vec.begin() + 1, extra.begin(), extra.end());
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 3);
        REQUIRE(vec[3] == 4);
    }

    SECTION("insert from initializer list") {
        vec.insert(vec.cbegin() + 1, {2, 3});
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 3);
        REQUIRE(vec[3] == 4);
    }

    SECTION("erase removes a single element") {
        vec.push_back(5);
        auto it = vec.erase(vec.begin() + 1);
        REQUIRE(*it == 5);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 5);
    }

    SECTION("erase a range") {
        vec.assign({1, 2, 3, 4});
        auto it = vec.erase(vec.begin() + 1, vec.begin() + 3);
        REQUIRE(*it == 4);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 4);
    }
}

TEST_CASE("inplace_vector assign and comparison", "[inplace_vector]") {
    SECTION("assign overwrites the contents") {
        inplace_vector<int, 8> vec{9, 9, 9};
        vec.assign({1, 2});
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
    }

    SECTION("equality compares element-wise") {
        inplace_vector<int, 8> left{1, 2, 3};
        inplace_vector<int, 8> right{1, 2, 3};
        REQUIRE(left == right);
        right.push_back(4);
        REQUIRE(left != right);
    }
}

TEST_CASE("inplace_vector functional helpers", "[inplace_vector]") {
    SECTION("map transforms every element") {
        inplace_vector<int, 4> vec{1, 2, 3};
        auto doubled = vec.map([](int value) { return value * 2; });
        REQUIRE(doubled.size() == 3);
        REQUIRE(doubled[0] == 2);
        REQUIRE(doubled[1] == 4);
        REQUIRE(doubled[2] == 6);
    }

    SECTION("filter keeps matching elements") {
        inplace_vector<int, 4> vec{1, 2, 3, 4};
        auto evens = vec.filter<4>([](int value) { return value % 2 == 0; });
        REQUIRE(evens.size() == 2);
        REQUIRE(evens[0] == 2);
        REQUIRE(evens[1] == 4);
    }

    SECTION("fold reduces to a single value") {
        inplace_vector<int, 4> vec{1, 2, 3};
        auto sum = vec.fold([](int acc, int value) { return acc + value; }, 0);
        REQUIRE(sum == 6);
    }

    SECTION("reverse flips the element order") {
        inplace_vector<int, 4> vec{1, 2, 3};
        auto flipped = vec.reverse();
        REQUIRE(flipped.size() == 3);
        REQUIRE(flipped[0] == 3);
        REQUIRE(flipped[2] == 1);
    }

    SECTION("zip_with pairs two vectors") {
        inplace_vector<int, 2> left{1, 2};
        inplace_vector<int, 2> right{10, 20};
        auto zipped = zip_with(left, right, [](int a, int b) { return a + b; });
        REQUIRE(zipped.size() == 2);
        REQUIRE(zipped[0] == 11);
        REQUIRE(zipped[1] == 22);
    }
}

TEST_CASE("inplace_vector swap and clear", "[inplace_vector]") {
    SECTION("swap exchanges contents") {
        inplace_vector<int, 4> left{1};
        inplace_vector<int, 4> right{2, 3};
        left.swap(right);
        REQUIRE(left.size() == 2);
        REQUIRE(left[1] == 3);
        REQUIRE(right.size() == 1);
        REQUIRE(right[0] == 1);
    }

    SECTION("clear empties the vector") {
        inplace_vector<int, 4> vec{1, 2};
        vec.clear();
        REQUIRE(vec.empty());
        REQUIRE(vec.size() == 0);
    }
}

TEST_CASE("inplace_vector works in constant evaluation", "[inplace_vector]") {
    constexpr auto make_sum = [] {
        inplace_vector<int, 4> vec{1, 2, 3};
        vec.push_back(4);
        int total = 0;
        for (auto item: vec) {
            total += item;
        }
        return total;
    };
    STATIC_REQUIRE(make_sum() == 10);
}
