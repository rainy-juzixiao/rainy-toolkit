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

TEST_CASE("any - default construction", "[any]") {
    any a;
    CHECK(a.has_value() == false);
    CHECK(a.type() == rainy_typeid(void));
}

TEST_CASE("any - construction from value", "[any]") {
    any a = 42;
    CHECK(a.has_value() == true);
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 42);

    any b = std::string("hello");
    CHECK(b.has_value() == true);
    CHECK(b.is<std::string>() == true);
    CHECK(b.as<std::string>() == "hello");

    any c = 3.14;
    CHECK(c.has_value() == true);
    CHECK(c.is<double>() == true);
    CHECK(c.as<double>() == 3.14);
}

TEST_CASE("any - in_place construction", "[any]") {
    any a(std::in_place_type<int>, 100);
    CHECK(a.has_value() == true);
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 100);

    any b(std::in_place_type<std::string>, "in_place_str");
    CHECK(b.has_value() == true);
    CHECK(b.is<std::string>() == true);
    CHECK(b.as<std::string>() == "in_place_str");

    any c(std::in_place_type<std::vector<int>>, std::initializer_list<int>{1, 2, 3});
    CHECK(c.has_value() == true);
    CHECK(c.is<std::vector<int>>() == true);
}

TEST_CASE("any - copy construction", "[any]") {
    any a = 42;
    any b = a;
    CHECK(b.has_value() == true);
    CHECK(b.is<int>() == true);
    CHECK(b.as<int>() == 42);
}

TEST_CASE("any - move construction", "[any]") {
    any a = std::string("move_me");
    any b = std::move(a);
    CHECK(b.has_value() == true);
    CHECK(b.is<std::string>() == true);
    CHECK(b.as<std::string>() == "move_me");
}

TEST_CASE("any - copy assignment", "[any]") {
    any a = 10;
    any b;
    b = a;
    CHECK(b.has_value() == true);
    CHECK(b.is<int>() == true);
    CHECK(b.as<int>() == 10);
}

TEST_CASE("any - move assignment", "[any]") {
    any a = 3.14;
    any b;
    b = std::move(a);
    CHECK(b.has_value() == true);
    CHECK(b.is<double>() == true);
    CHECK(b.as<double>() == 3.14);
}

TEST_CASE("any - value assignment", "[any]") {
    any a;
    a = 42;
    CHECK(a.has_value() == true);
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 42);

    a = std::string("reassigned");
    CHECK(a.has_value() == true);
    CHECK(a.is<std::string>() == true);
    CHECK(a.as<std::string>() == "reassigned");
}

TEST_CASE("any - emplace", "[any]") {
    any a;
    a.emplace<int>(42);
    CHECK(a.has_value() == true);
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 42);

    a.emplace<std::string>("emplaced");
    CHECK(a.has_value() == true);
    CHECK(a.is<std::string>() == true);
    CHECK(a.as<std::string>() == "emplaced");
}

TEST_CASE("any - reset", "[any]") {
    any a = 42;
    CHECK(a.has_value() == true);
    a.reset();
    CHECK(a.has_value() == false);
}

TEST_CASE("any - swap", "[any]") {
    any a = 10;
    any b = 20;
    a.swap(b);
    CHECK(a.as<int>() == 20);
    CHECK(b.as<int>() == 10);

    // free function swap
    swap(a, b);
    CHECK(a.as<int>() == 10);
    CHECK(b.as<int>() == 20);
}

TEST_CASE("any - has_value check", "[any]") {
    any empty;
    CHECK(empty.has_value() == false);

    any filled = 42;
    CHECK(filled.has_value() == true);

    filled.reset();
    CHECK(filled.has_value() == false);
}

TEST_CASE("any - type query", "[any]") {
    any a = 42;
    CHECK(a.type() == rainy_typeid(int));

    any b = 3.14;
    CHECK(b.type() == rainy_typeid(double));

    any c = std::string("hello");
    CHECK(c.type() == rainy_typeid(std::string));
}

TEST_CASE("any - is type check", "[any]") {
    any a = 42;
    CHECK(a.is<int>() == true);
    CHECK(a.is<double>() == false);
    CHECK(a.is<std::string>() == false);

    any b = std::string("hello");
    CHECK(b.is<std::string>() == true);
    CHECK(b.is<int>() == false);
}

TEST_CASE("any - is_one_of check", "[any]") {
    any a = 42;
    CHECK(a.is_one_of<int, double, float>() == true);
    CHECK(a.is_one_of<double, float, std::string>() == false);

    any b = std::string("hello");
    CHECK(b.is_one_of<std::string, int>() == true);
    CHECK(b.is_one_of<int, double>() == false);
}

TEST_CASE("any - is_one_of_convertible check", "[any]") {
    any a = 42;
    CHECK(a.is_one_of_convertible<int, double, float>() == true);

    any b = 3.14;
    CHECK(b.is_one_of_convertible<int, double>() == true);
}

TEST_CASE("any - as type retrieval", "[any]") {
    any a = 42;
    CHECK(a.as<int>() == 42);
    CHECK(a.as<const int &>() == 42);

    any b = std::string("hello");
    CHECK(b.as<std::string>() == "hello");
    CHECK(b.as<const std::string &>() == "hello");
}

TEST_CASE("any - NEGATIVE - as type mismatch", "[any]") {
    any a = 42;
    CHECK_THROWS_AS(a.as<double>(), std::bad_cast);
    CHECK_THROWS_AS(a.as<std::string>(), std::bad_cast);
}

TEST_CASE("any - cast_to_pointer", "[any]") {
    any a = 42;
    const int *ptr = a.cast_to_pointer<int>();
    CHECK(ptr != nullptr);
    CHECK(*ptr == 42);

    const double *dptr = a.cast_to_pointer<double>();
    CHECK(dptr == nullptr);

    const int *cptr = a.cast_to_pointer<const int>();
    CHECK(cptr != nullptr);
    CHECK(*cptr == 42);
}

TEST_CASE("any - cast_to_pointer non-const", "[any]") {
    any a = 42;
    int *ptr = a.cast_to_pointer<int>();
    CHECK(ptr != nullptr);
    CHECK(*ptr == 42);
}

TEST_CASE("any - make_any helper", "[any]") {
    auto a = make_any<int>(42);
    CHECK(a.has_value() == true);
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 42);

    auto b = make_any<std::string>("make_any_str");
    CHECK(b.has_value() == true);
    CHECK(b.is<std::string>() == true);
    CHECK(b.as<std::string>() == "make_any_str");
}

TEST_CASE("any - has_ownership", "[any]") {
    any a = 42;
    CHECK(a.has_ownership() == true);

    any empty;
    CHECK(empty.has_ownership() == false);
}

TEST_CASE("any - hash_code", "[any]") {
    any a = 42;
    std::size_t h = a.hash_code();
    // Hash should be computable for arithmetic types
    CHECK(h == std::hash<int>{}(42));

    any b = 42;
    CHECK(a.hash_code() == b.hash_code());

    any c = 100;
    CHECK(a.hash_code() != c.hash_code());
}

TEST_CASE("any - std hash specialization", "[any]") {
    any a = 42;
    std::hash<any> hasher;
    std::size_t h = hasher(a);
    CHECK(h == a.hash_code());
}

TEST_CASE("any - inner_decleartion_type", "[any]") {
    any a = std::vector<int>{1, 2, 3};
    rainy::core::typeinfo val_t = a.inner_decleartion_type(any_inner_declaertion::value_type);
    CHECK(val_t == rainy_typeid(int));
}

TEST_CASE("any - target_as_void_ptr", "[any]") {
    any a = 42;
    const void *ptr = a.target_as_void_ptr();
    CHECK(ptr != nullptr);
    CHECK(*static_cast<const int *>(ptr) == 42);
}
