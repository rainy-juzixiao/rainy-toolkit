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

TEST_CASE("wrap_any - wrap an any holding an int", "[any][wrap_any]") {
    any a = 42;
    REQUIRE(a.has_value());
    REQUIRE(a.is<int>());

    any wrapper = wrap_any(a);
    REQUIRE(wrapper.has_value());
    CHECK(wrapper.unwraped_is<int>());
    CHECK(wrapper.as<int>() == 42);
}

TEST_CASE("wrap_any - wrap an any holding a string", "[any][wrap_any]") {
    any a = std::string("hello world");
    REQUIRE(a.has_value());

    any wrapper = wrap_any(a);
    REQUIRE(wrapper.has_value());
    CHECK(wrapper.unwraped_is<std::string>());
    CHECK(wrapper.as<std::string>() == "hello world");
}

TEST_CASE("wrap_any - wrap an any holding a vector", "[any][wrap_any]") {
    any a = std::vector<int>{1, 2, 3};
    REQUIRE(a.has_value());

    any wrapper = wrap_any(a);
    REQUIRE(wrapper.has_value());
    CHECK(wrapper.unwraped_is<std::vector<int>>());
    auto vec = wrapper.as<std::vector<int>>();
    CHECK(vec.size() == 3);
    CHECK(vec[0] == 1);
    CHECK(vec[1] == 2);
    CHECK(vec[2] == 3);
}

TEST_CASE("wrap_any - wrapper owns an independent copy", "[any][wrap_any]") {
    any a = 42;
    any wrapper = wrap_any(a);

    // Modifying the original should NOT affect the wrapper (value semantics)
    a = 100;
    CHECK(wrapper.as<int>() == 42);
    CHECK(a.as<int>() == 100);
}

TEST_CASE("wrap_any - forwarded operations: comparison", "[any][wrap_any]") {
    any a = 42;
    any b = 42;
    any wrapper_a = wrap_any(a);
    any wrapper_b = wrap_any(b);

    // Comparison should work through the wrapper
    CHECK(wrapper_a == wrapper_b);
    CHECK(!(wrapper_a != wrapper_b));
    CHECK(wrapper_a <= wrapper_b);
    CHECK(wrapper_a >= wrapper_b);
    CHECK_FALSE(wrapper_a < wrapper_b);
    CHECK_FALSE(wrapper_a > wrapper_b);
}

TEST_CASE("wrap_any - forwarded operations: arithmetic", "[any][wrap_any]") {
    any a = 10;
    any b = 20;
    any w_a = wrap_any(a);
    any w_b = wrap_any(b);

    any sum = w_a + w_b;
    CHECK(sum.is<int>());
    CHECK(sum.as<int>() == 30);

    any diff = w_b - w_a;
    CHECK(diff.is<int>());
    CHECK(diff.as<int>() == 10);
}

TEST_CASE("wrap_any - forwarded operations: increment", "[any][wrap_any]") {
    any a = 5;
    any wrapper = wrap_any(a);

    // prefix ++
    any result = ++wrapper;
    CHECK(result.is<int&>());
    CHECK(result.as<int>() == 6);
    CHECK(wrapper.as<int>() == 6);

    // postfix ++
    result = wrapper++;
    CHECK(result.is<int>());
    CHECK(result.as<int>() == 6);
    CHECK(wrapper.as<int>() == 7);
}

TEST_CASE("wrap_any - forwarded operations: hash_code", "[any][wrap_any]") {
    any a = 42;
    any b = 42;
    any wrapper = wrap_any(a);
    any wrapper_same = wrap_any(b);

    CHECK(wrapper.hash_code() == std::hash<int>{}(42));
    CHECK(wrapper.hash_code() == wrapper_same.hash_code());

    any c = 100;
    any wrapper_diff = wrap_any(c);
    CHECK(wrapper.hash_code() != wrapper_diff.hash_code());
}

TEST_CASE("wrap_any - forwarded operations: type query", "[any][wrap_any]") {
    any a = 3.14;
    any wrapper = wrap_any(a);

    CHECK(wrapper.unwrapped_type() == rainy_typeid(double));
    CHECK(wrapper.has_value());
    CHECK(wrapper.has_ownership()); // wrapper owns the copy
}

TEST_CASE("wrap_any - wrap an empty any", "[any][wrap_any]") {
    any empty;
    REQUIRE_FALSE(empty.has_value());

    any wrapper = wrap_any(empty);
    CHECK_FALSE(wrapper.has_value());
}

TEST_CASE("wrap_any - wrapper constructed directly via wrap_any_tag + value", "[any][wrap_any]") {
    any a = 99;
    any wrapper(wrap_any_tag{}, a);

    REQUIRE(wrapper.has_value());
    CHECK(wrapper.unwraped_is<int>());
    CHECK(wrapper.as<int>() == 99);
}

TEST_CASE("wrap_any - construct from non-any value with tag", "[any][wrap_any]") {
    // Using wrap_any_tag with a value that isn't any also works:
    // it constructs an any from the value and stores it as a wrapped any.
    any w(wrap_any_tag{}, 42);
    REQUIRE(w.has_value());
    CHECK(w.unwraped_is<int>());
    CHECK(w.as<int>() == 42);
}

TEST_CASE("wrap_any - assignment to wrapper replaces the wrapped value", "[any][wrap_any]") {
    any a = 10;
    any wrapper = wrap_any(a);

    // Assign a new value through the wrapper
    wrapper = 200;
    CHECK(wrapper.as<int>() == 200);
    // The original should be unchanged (value semantics)
    CHECK(a.as<int>() == 10);
}

TEST_CASE("wrap_any - wrap and then re-wrap", "[any][wrap_any]") {
    any a = 1;
    any wrapper = wrap_any(a);

    // Wrapping a wrapper produces a second-level copy of the original value.
    any double_wrapped = wrap_any(wrapper);
    CHECK(double_wrapped.unwraped_is<int>());
    CHECK(double_wrapped.as<int>() == 1);

    // Changes to the outermost should not affect the inner
    double_wrapped = 999;
    CHECK(a.as<int>() == 1);
    CHECK(wrapper.as<int>() == 1);
}

TEST_CASE("wrap_any - copy-initialized wrapper via wrap_any_tag", "[any][wrap_any]") {
    any a = 7;
    any w1 = any(wrap_any_tag{}, a);
    CHECK(w1.as<int>() == 7);

    // Move construction
    any w2 = any(wrap_any_tag{}, std::move(a));
    CHECK(w2.as<int>() == 7);
}
