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

TEST_CASE("any - reference - as_reference", "[any]") {
    any a = 42;
    auto ref = a.as_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_reference const", "[any]") {
    const any a = 3.14;
    auto ref = a.as_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_const_reference", "[any]") {
    const any a = std::string("hello");
    auto ref = a.as_const_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_lvalue_reference", "[any]") {
    any a = 100;
    auto ref = a.as_lvalue_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_lvalue_reference const", "[any]") {
    const any a = 200;
    auto ref = a.as_lvalue_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_const_lvalue_reference", "[any]") {
    const any a = std::string("const_lvalue");
    auto ref = a.as_const_lvalue_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_rvalue_reference", "[any]") {
    any a = 42;
    auto ref = a.as_rvalue_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_rvalue_reference const", "[any]") {
    const any a = 3.14;
    auto ref = a.as_rvalue_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - as_const_rvalue_reference", "[any]") {
    const any a = std::string("const_rvalue");
    auto ref = a.as_const_rvalue_reference();
    CHECK(ref.has_value() == true);
}

TEST_CASE("any - reference - no ownership on all ref types", "[any]") {
    any a = 42;

    CHECK(a.as_reference().has_ownership() == false);
    CHECK(a.as_lvalue_reference().has_ownership() == false);
    CHECK(a.as_rvalue_reference().has_ownership() == false);
}

TEST_CASE("any - reference - reference to vector", "[any]") {
    any vec = std::vector<int>{10, 20, 30};
    auto ref = vec.as_reference();
    CHECK(ref.has_value() == true);
    CHECK(ref.size() == 3);
}

TEST_CASE("any - reference - reference to string", "[any]") {
    any s = std::string("reference_test");
    auto ref = s.as_reference();
    CHECK(ref.has_value() == true);

    auto lref = s.as_lvalue_reference();
    CHECK(lref.has_value() == true);

    auto rref = s.as_rvalue_reference();
    CHECK(rref.has_value() == true);
}

TEST_CASE("any - reference - NEGATIVE - reference from empty any", "[any]") {
    any a;
    auto ref = a.as_reference();
    CHECK(ref.has_value() == false);
}

TEST_CASE("any - swap_value", "[any]") {
    any a = 10;
    any b = 20;
    a.swap_value(b);
    CHECK(a.as<int>() == 20);
    CHECK(b.as<int>() == 10);
}

TEST_CASE("any - swap_value with vector", "[any]") {
    any a = std::vector<int>{1, 2, 3};
    any b = std::vector<int>{4, 5, 6};
    a.swap_value(b);
    CHECK(a.size() == 3);
    CHECK(b.size() == 3);
}
