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
#include <catch2/matchers/catch_matchers_all.hpp>
#include <rainy/core/container/tuple.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace rainy::container;
using Catch::Matchers::Equals;

TEST_CASE("Tuple basic construction and element access", "[tuple]") {
    SECTION("Empty tuple") {
        tuple<> empty;
        REQUIRE(tuple_size_v<decltype(empty)> == 0);
    }

    SECTION("Single element tuple") {
        tuple<int> t(42);
        REQUIRE(get<0>(t) == 42);
        REQUIRE(tuple_size_v<decltype(t)> == 1);
    }

    SECTION("Multiple elements tuple") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        REQUIRE(get<0>(t) == 1);
        REQUIRE(get<1>(t) == 2.5);
        REQUIRE(get<2>(t) == "hello");
        REQUIRE(tuple_size_v<decltype(t)> == 3);
    }

    SECTION("Make tuple") {
        auto t = rainy::container::make_tuple(42, 3.14, std::string("world"));
        REQUIRE(get<0>(t) == 42);
        REQUIRE(get<1>(t) == 3.14);
        REQUIRE(get<2>(t) == "world");
    }

    SECTION("Forward as tuple") {
        int x = 42;
        std::string s = "test";
        double f = 3.14;
        auto t = rainy::container::forward_as_tuple(x, s, f);
        REQUIRE(get<0>(t) == 42);
        REQUIRE(get<1>(t) == "test");
        REQUIRE(get<2>(t) == 3.14);
        get<0>(t) = 100;
        REQUIRE(x == 100);
    }
}

TEST_CASE("Tuple copy and move operations", "[tuple]") {
    SECTION("Copy constructor") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(t1);
        REQUIRE(get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == "hello");
    }

    SECTION("Move constructor") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(std::move(t1));
        REQUIRE(get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == "hello");
    }

    SECTION("Copy assignment") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(100, "world");
        t2 = t1;
        REQUIRE(get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == "hello");
    }

    SECTION("Move assignment") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(100, "world");
        t2 = std::move(t1);
        REQUIRE(get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == "hello");
    }
}

TEST_CASE("Tuple converting constructor and assignment", "[tuple]") {
    SECTION("Converting constructor") {
        tuple<int, double> t1(42, 3.14);
        tuple<double, int> t2(t1);
        REQUIRE(get<0>(t2) == 42.0);
        REQUIRE(get<1>(t2) == 3);
    }

    SECTION("Converting assignment") {
        tuple<int, double> t1(42, 3.14);
        tuple<double, int> t2(1.0, 2);
        t2 = t1;
        REQUIRE(get<0>(t2) == 42.0);
        REQUIRE(get<1>(t2) == 3);
    }
}

TEST_CASE("Tuple swap operations", "[tuple]") {
    SECTION("Swap member function") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(100, "world");
        t1.swap(t2);
        REQUIRE(get<0>(t1) == 100);
        REQUIRE(get<1>(t1) == "world");
        REQUIRE(get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == "hello");
    }

    SECTION("Swap free function") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(100, "world");
        swap(t1, t2);
        REQUIRE(get<0>(t1) == 100);
        REQUIRE(get<1>(t1) == "world");
        REQUIRE(get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == "hello");
    }

    SECTION("Empty tuple swap") {
        tuple<> t1;
        tuple<> t2;
        t1.swap(t2);
        swap(t1, t2);
    }
}

TEST_CASE("Tuple get operations with different value categories", "[tuple]") {
    SECTION("Get lvalue reference") {
        tuple<int, double> t(42, 3.14);
        auto &ref = get<0>(t);
        ref = 100;
        REQUIRE(get<0>(t) == 100);
    }

    SECTION("Get const lvalue reference") {
        const tuple<int, double> t(42, 3.14);
        const auto &ref = get<0>(t);
        REQUIRE(ref == 42);
    }

    SECTION("Get rvalue reference") {
        tuple<int, double> t(42, 3.14);
        auto &&ref = get<0>(std::move(t));
        REQUIRE(ref == 42);
    }

    SECTION("Get const rvalue reference") {
        const tuple<int, double> t(42, 3.14);
        auto &&ref = get<0>(std::move(t));
        REQUIRE(ref == 42);
    }
}

TEST_CASE("Tuple drop operations", "[tuple]") {
    SECTION("Drop N elements - const version") {
        tuple<int, double, std::string, char> t(1, 2.5, "hello", 'a');
        auto dropped = drop<2>(t);
        REQUIRE(tuple_size_v<decltype(dropped)> == 2);
        REQUIRE(get<0>(dropped) == "hello");
        REQUIRE(get<1>(dropped) == 'a');
    }

    SECTION("Drop N elements - rvalue version") {
        tuple<int, double, std::string, char> t(1, 2.5, "hello", 'a');
        auto dropped = drop<2>(std::move(t));
        REQUIRE(tuple_size_v<decltype(dropped)> == 2);
        REQUIRE(get<0>(dropped) == "hello");
        REQUIRE(get<1>(dropped) == 'a');
    }

    SECTION("Drop all elements") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto dropped = drop<3>(t);
        REQUIRE(tuple_size_v<decltype(dropped)> == 0);
    }

    SECTION("Drop zero elements") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto dropped = drop<0>(t);
        REQUIRE(tuple_size_v<decltype(dropped)> == 3);
        REQUIRE(get<0>(dropped) == 1);
        REQUIRE(get<1>(dropped) == 2.5);
        REQUIRE(get<2>(dropped) == "hello");
    }
}

TEST_CASE("Tuple take operations", "[tuple]") {
    SECTION("Take N elements - const version") {
        tuple<int, double, std::string, char> t(1, 2.5, "hello", 'a');
        auto taken = take<2>(t);
        REQUIRE(tuple_size_v<decltype(taken)> == 2);
        REQUIRE(get<0>(taken) == 1);
        REQUIRE(get<1>(taken) == 2.5);
    }

    SECTION("Take N elements - rvalue version") {
        tuple<int, double, std::string, char> t(1, 2.5, "hello", 'a');
        auto taken = take<2>(std::move(t));
        REQUIRE(tuple_size_v<decltype(taken)> == 2);
        REQUIRE(get<0>(taken) == 1);
        REQUIRE(get<1>(taken) == 2.5);
    }

    SECTION("Take all elements") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto taken = take<3>(t);
        REQUIRE(tuple_size_v<decltype(taken)> == 3);
        REQUIRE(get<0>(taken) == 1);
        REQUIRE(get<1>(taken) == 2.5);
        REQUIRE(get<2>(taken) == "hello");
    }

    SECTION("Take zero elements") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto taken = take<0>(t);
        REQUIRE(tuple_size_v<decltype(taken)> == 0);
    }
}

TEST_CASE("Tuple subtuple operations", "[tuple]") {
    SECTION("Subtuple - const version") {
        tuple<int, double, std::string, char, float> t(1, 2.5, "hello", 'a', 3.14f);
        auto sub = subtuple<1, 4>(t);
        REQUIRE(tuple_size_v<decltype(sub)> == 3);
        REQUIRE(get<0>(sub) == 2.5);
        REQUIRE(get<1>(sub) == "hello");
        REQUIRE(get<2>(sub) == 'a');
    }

    SECTION("Subtuple - rvalue version") {
        tuple<int, double, std::string, char, float> t(1, 2.5, "hello", 'a', 3.14f);
        auto sub = subtuple<1, 4>(std::move(t));
        REQUIRE(tuple_size_v<decltype(sub)> == 3);
        REQUIRE(get<0>(sub) == 2.5);
        REQUIRE(get<1>(sub) == "hello");
        REQUIRE(get<2>(sub) == 'a');
    }

    SECTION("Subtuple empty range") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto sub = subtuple<1, 1>(t);
        REQUIRE(tuple_size_v<decltype(sub)> == 0);
    }

    SECTION("Subtuple full range") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto sub = subtuple<0, 3>(t);
        REQUIRE(tuple_size_v<decltype(sub)> == 3);
        REQUIRE(get<0>(sub) == 1);
        REQUIRE(get<1>(sub) == 2.5);
        REQUIRE(get<2>(sub) == "hello");
    }
}

TEST_CASE("Tuple apply operations", "[tuple]") {
    SECTION("Apply with lambda - const version") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto result = rainy::container::apply(
            [](int a, double b, const std::string &c) { return std::to_string(a) + " " + std::to_string(b) + " " + c; }, t);
         REQUIRE((result == "1 2.500000 hello" || result == "1 2.5 hello"));
    }

    SECTION("Apply with lambda - rvalue version") {
        tuple<int, double, std::string> t(1, 2.5, "hello");
        auto result =
            apply([](int a, double b, std::string c) { return std::to_string(a) + " " + std::to_string(b) + " " + c; }, std::move(t));
        REQUIRE((result == "1 2.500000 hello" || result == "1 2.5 hello"));
    }

    SECTION("Apply returning sum") {
        tuple<int, int, int, int> t(1, 2, 3, 4);
        auto sum = apply([](int a, int b, int c, int d) { return a + b + c + d; }, t);
        REQUIRE(sum == 10);
    }
}

TEST_CASE("Tuple equality comparison", "[tuple]") {
    SECTION("Equal tuples") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(42, "hello");
        REQUIRE(t1 == t2);
    }

    SECTION("Different tuples") {
        tuple<int, std::string> t1(42, "hello");
        tuple<int, std::string> t2(100, "hello");
        REQUIRE(!(t1 == t2));
    }

    SECTION("Different types but comparable") {
        tuple<double, int> t1(42.0, 3);
        tuple<int, double> t2(42, 3.14);

        REQUIRE(t1 != t2);
    }

    SECTION("Empty tuples equal") {
        tuple<> t1;
        tuple<> t2;
        REQUIRE(t1 == t2);
    }

    SECTION("Different sizes not equal") {
        tuple<int, double> t1(42, 3.14);
        tuple<int, double, std::string> t2(42, 3.14, "hello");
        REQUIRE(!(t1 == t2));
    }
}

TEST_CASE("Tuple with complex types", "[tuple]") {
    SECTION("Vector in tuple") {
        std::vector<int> vec = {1, 2, 3};
        tuple<std::vector<int>, std::string> t(vec, "test");
        REQUIRE(get<0>(t).size() == 3);
        REQUIRE(get<0>(t)[0] == 1);
        REQUIRE(get<0>(t)[1] == 2);
        REQUIRE(get<0>(t)[2] == 3);
        REQUIRE(get<1>(t) == "test");
    }

    SECTION("Nested tuple") {
        tuple<int, tuple<double, std::string>> t(42, tuple<double, std::string>(3.14, "hello"));
        REQUIRE(get<0>(t) == 42);
        REQUIRE(get<0>(get<1>(t)) == 3.14);
        REQUIRE(get<1>(get<1>(t)) == "hello");
    }

    SECTION("Tuple of references") {
        int x = 42;
        double y = 3.14;
        auto t = forward_as_tuple(x, y);
        get<0>(t) = 100;
        get<1>(t) = 2.71;
        REQUIRE(x == 100);
        REQUIRE(y == 2.71);
    }
}

TEST_CASE("Tuple type traits", "[tuple]") {
    SECTION("tuple_size") {
        REQUIRE(tuple_size_v<tuple<>> == 0);
        REQUIRE(tuple_size_v<tuple<int>> == 1);
        REQUIRE(tuple_size_v<tuple<int, double>> == 2);
        REQUIRE(tuple_size_v<tuple<int, double, std::string>> == 3);
    }

    SECTION("tuple_element") {
        using T = tuple<int, double, std::string>;
        REQUIRE(std::is_same_v<tuple_element_t<0, T>, int>);
        REQUIRE(std::is_same_v<tuple_element_t<1, T>, double>);
        REQUIRE(std::is_same_v<tuple_element_t<2, T>, std::string>);
    }

    SECTION("std::tuple_size compatibility") {
        REQUIRE(std::tuple_size<tuple<int, double, std::string>>::value == 3);
    }

    SECTION("std::tuple_element compatibility") {
        using T = tuple<int, double, std::string>;
        REQUIRE(std::is_same_v<std::tuple_element_t<0, T>, int>);
        REQUIRE(std::is_same_v<std::tuple_element_t<1, T>, double>);
        REQUIRE(std::is_same_v<std::tuple_element_t<2, T>, std::string>);
    }
}

TEST_CASE("Tuple allocator support", "[tuple]") {
    SECTION("Default allocator construction") {
        tuple<int, std::string> t(std::allocator_arg, std::allocator<int>(), 42, "hello");
        REQUIRE(get<0>(t) == 42);
        REQUIRE(get<1>(t) == "hello");
    }

    SECTION("Allocator construction with no arguments") {
        tuple<int, std::string> t(std::allocator_arg, std::allocator<int>());
        REQUIRE(get<0>(t) == 0);
        REQUIRE(get<1>(t) == "");
    }
}

TEST_CASE("Tuple const correctness", "[tuple]") {
    SECTION("Const tuple access") {
        const tuple<int, std::string> t(42, "hello");
        REQUIRE(get<0>(t) == 42);
        REQUIRE(get<1>(t) == "hello");
        // Should not compile:
        // get<0>(t) = 100;
    }

    SECTION("Non-const tuple access") {
        tuple<int, std::string> t(42, "hello");
        get<0>(t) = 100;
        get<1>(t) = "world";
        REQUIRE(get<0>(t) == 100);
        REQUIRE(get<1>(t) == "world");
    }

    SECTION("Const rvalue get") {
        const tuple<int, std::string> t(42, "hello");
        auto &&value = get<0>(std::move(t));
        REQUIRE(value == 42);
    }
}

TEST_CASE("Tuple with move-only types", "[tuple]") {
    SECTION("Move-only type in tuple") {
        using MoveOnly = std::unique_ptr<int>;
        tuple<MoveOnly, int> t(std::make_unique<int>(42), 100);
        REQUIRE(*get<0>(t) == 42);
        REQUIRE(get<1>(t) == 100);
    }

    SECTION("Move construction of tuple with move-only types") {
        using MoveOnly = std::unique_ptr<int>;
        tuple<MoveOnly, int> t1(std::make_unique<int>(42), 100);
        tuple<MoveOnly, int> t2(std::move(t1));
        REQUIRE(*get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == 100);
        // t1 is now in moved-from state
    }

    SECTION("Move assignment of tuple with move-only types") {
        using MoveOnly = std::unique_ptr<int>;
        tuple<MoveOnly, int> t1(std::make_unique<int>(42), 100);
        tuple<MoveOnly, int> t2(std::make_unique<int>(200), 300);
        t2 = std::move(t1);
        REQUIRE(*get<0>(t2) == 42);
        REQUIRE(get<1>(t2) == 100);
    }
}

TEST_CASE("Tuple chain operations", "[tuple]") {
    SECTION("Drop then take") {
        tuple<int, double, std::string, char, float> t(1, 2.5, "hello", 'a', 3.14f);
        auto result = take<2>(drop<1>(t));
        REQUIRE(tuple_size_v<decltype(result)> == 2);
        REQUIRE(get<0>(result) == 2.5);
        REQUIRE(get<1>(result) == "hello");
    }

    SECTION("Take then drop") {
        tuple<int, double, std::string, char, float> t(1, 2.5, "hello", 'a', 3.14f);
        auto result = drop<1>(take<3>(t));
        REQUIRE(tuple_size_v<decltype(result)> == 2);
        REQUIRE(get<0>(result) == 2.5);
        REQUIRE(get<1>(result) == "hello");
    }

    SECTION("Subtuple with apply") {
        tuple<int, double, std::string, char> t(1, 2.5, "hello", 'a');
        auto result = apply([](double d, const std::string &s) { return std::to_string(d) + " " + s; }, subtuple<1, 3>(t));
        REQUIRE((result == "2.500000 hello" || result == "2.5 hello"));
    }
}

TEST_CASE("Tuple with custom types", "[tuple]") {
    struct Custom {
        int value;
        bool operator==(const Custom &other) const {
            return value == other.value;
        }
    };

    SECTION("Custom type in tuple") {
        Custom c{42};
        tuple<Custom, std::string> t(c, "hello");
        REQUIRE(get<0>(t).value == 42);
        REQUIRE(get<1>(t) == "hello");
    }

    SECTION("Custom type comparison") {
        tuple<Custom, int> t1(Custom{42}, 100);
        tuple<Custom, int> t2(Custom{42}, 100);
        tuple<Custom, int> t3(Custom{43}, 100);
        REQUIRE(t1 == t2);
        REQUIRE(!(t1 == t3));
    }

    SECTION("Custom type in make_tuple") {
        Custom c{42};
        auto t = make_tuple(c, "hello");
        REQUIRE(get<0>(t).value == 42);
        REQUIRE(std::string_view(get<1>(t)) == "hello");
    }
}

TEST_CASE("Tuple constexpr support", "[tuple][constexpr]") {
    SECTION("Basic constexpr construction and get") {
        constexpr tuple<int, double, char> t(42, 3.14, 'a');

        STATIC_REQUIRE(tuple_size_v<rainy::type_traits::modifers::remove_cvref_t<decltype(t)>> == 3);
        STATIC_REQUIRE(get<0>(t) == 42);
        STATIC_REQUIRE(get<1>(t) == 3.14);
        STATIC_REQUIRE(get<2>(t) == 'a');

        REQUIRE(get<0>(t) == 42);
        REQUIRE(get<1>(t) == 3.14);
        REQUIRE(get<2>(t) == 'a');
    }

    SECTION("constexpr empty tuple") {
        constexpr tuple<> t;

        STATIC_REQUIRE(tuple_size_v<decltype(t)> == 0);

        REQUIRE(tuple_size_v<decltype(t)> == 0);
    }

    SECTION("constexpr make_tuple") {
        constexpr auto t = make_tuple(1, 2, 3);

        STATIC_REQUIRE(tuple_size_v<rainy::type_traits::modifers::remove_cvref_t<decltype(t)>> == 3);
        STATIC_REQUIRE(get<0>(t) == 1);
        STATIC_REQUIRE(get<1>(t) == 2);
        STATIC_REQUIRE(get<2>(t) == 3);

        REQUIRE(get<0>(t) == 1);
        REQUIRE(get<1>(t) == 2);
        REQUIRE(get<2>(t) == 3);
    }

    SECTION("constexpr tuple comparison") {
        constexpr tuple<int, int> t1(1, 2);
        constexpr tuple<int, int> t2(1, 2);
        constexpr tuple<int, int> t3(2, 3);

        STATIC_REQUIRE(t1 == t2);
        STATIC_REQUIRE(!(t1 == t3));

        REQUIRE(t1 == t2);
        REQUIRE(t1 != t3);
    }

    SECTION("constexpr tuple operations") {
        constexpr tuple<int, int, int, int> t(1, 2, 3, 4);

        constexpr auto first = take<2>(t);
        constexpr auto last = drop<2>(t);

        STATIC_REQUIRE(tuple_size_v<rainy::type_traits::modifers::remove_cvref_t<decltype(first)>> == 2);
        STATIC_REQUIRE(get<0>(first) == 1);
        STATIC_REQUIRE(get<1>(first) == 2);

        STATIC_REQUIRE(tuple_size_v<rainy::type_traits::modifers::remove_cvref_t<decltype(last)>> == 2);
        STATIC_REQUIRE(get<0>(last) == 3);
        STATIC_REQUIRE(get<1>(last) == 4);
    }

    SECTION("constexpr apply") {
        constexpr tuple<int, int, int> t(1, 2, 3);

        constexpr auto result = apply([](int a, int b, int c) { return a + b + c; }, t);

        STATIC_REQUIRE(result == 6);
    }
}
