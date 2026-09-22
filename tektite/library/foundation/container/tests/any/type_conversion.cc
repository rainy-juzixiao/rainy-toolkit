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
#include <string_view>

using namespace rainy::foundation::container;

TEST_CASE("any - convert - arithmetic to arithmetic", "[any]") {
    any a = 42;
    CHECK(a.is_convertible<int>() == true);
    CHECK(a.is_convertible<double>() == true);
    CHECK(a.is_convertible<float>() == true);
    CHECK(a.is_convertible<char>() == true);
    CHECK(a.is_convertible<long>() == true);
    CHECK(a.is_convertible<short>() == true);
    CHECK(a.is_convertible<bool>() == true);

    CHECK(a.convert<int>() == 42);
    CHECK(a.convert<double>() == 42.0);
    CHECK(a.convert<float>() == 42.0f);
    CHECK(a.convert<char>() == static_cast<char>(42));
    CHECK(a.convert<bool>() == true);
}

TEST_CASE("any - convert - float to int truncation", "[any]") {
    any a = 3.14;
    CHECK(a.is_convertible<int>() == true);
    CHECK(a.convert<int>() == 3);

    any b = 3.99;
    CHECK(b.convert<int>() == 3);
}

TEST_CASE("any - convert - int to double", "[any]") {
    any a = 100;
    CHECK(a.is_convertible<double>() == true);
    CHECK(a.convert<double>() == 100.0);

    any b = -50;
    CHECK(b.convert<double>() == -50.0);
}

TEST_CASE("any - convert - char to int", "[any]") {
    any a = 'A';
    CHECK(a.is_convertible<int>() == true);
    CHECK(a.convert<int>() == static_cast<int>('A'));

    any b = 'z';
    CHECK(b.is_convertible<int>() == true);
    CHECK(b.convert<int>() == static_cast<int>('z'));
}

TEST_CASE("any - convert - bool to int", "[any]") {
    any a = true;
    CHECK(a.is_convertible<int>() == true);
    CHECK(a.convert<int>() == 1);

    any b = false;
    CHECK(b.convert<int>() == 0);
}

TEST_CASE("any - convert - string to string_view", "[any]") {
    any a = std::string("hello world");
    CHECK(a.is_convertible<std::string_view>() == true);
    std::string_view sv = a.convert<std::string_view>();
    CHECK(sv == "hello world");
}

TEST_CASE("any - convert - string_view to string", "[any]") {
    any a = std::string_view("hello view");
    CHECK(a.is_convertible<std::string>() == true);
    std::string s = a.convert<std::string>();
    CHECK(s == "hello view");
}

TEST_CASE("any - convert - const char ptr to string", "[any]") {
    const char *str = "c_string";
    any a = str;
    CHECK(a.is_convertible<std::string>() == true);
    CHECK(a.convert<std::string>() == "c_string");
}

TEST_CASE("any - convert - const char ptr to string_view", "[any]") {
    const char *str = "c_string_view";
    any a = str;
    CHECK(a.is_convertible<std::string_view>() == true);
    CHECK(a.convert<std::string_view>() == "c_string_view");
}

TEST_CASE("any - convert - to const reference", "[any]") {
    any a = std::string("ref_test");
    CHECK(a.is_convertible<const std::string &>() == true);
    const std::string &ref = a.convert<const std::string &>();
    CHECK(ref == "ref_test");
}

TEST_CASE("any - convert - pointer to nullptr_t", "[any]") {
    int *p = nullptr;
    any a = p;
    CHECK(a.is_convertible<std::nullptr_t>() == true);
}

TEST_CASE("any - convert - empty string edge cases", "[any]") {
    any a = std::string("");
    CHECK(a.is_convertible<std::string_view>() == true);
    CHECK(a.convert<std::string_view>().empty());

    const char *empty = "";
    any b = empty;
    CHECK(b.is_convertible<std::string>() == true);
    CHECK(b.convert<std::string>().empty());
}

TEST_CASE("any - convert - NEGATIVE - incompatible type", "[any]") {
    any a = 42;
    CHECK(a.is_convertible<std::string>() == false);
    CHECK_THROWS_AS(a.convert<std::string>(), std::bad_cast);
}

TEST_CASE("any - is_convertible by typeinfo", "[any]") {
    any a = 42;
    CHECK(a.is_convertible(rainy_typeid(int)) == true);
    CHECK(a.is_convertible(rainy_typeid(double)) == true);
    CHECK(a.is_convertible(rainy_typeid(float)) == true);
    CHECK(a.is_convertible(rainy_typeid(std::string)) == false);
}

TEST_CASE("any - transform by type - different arithmetic types", "[any]") {
    any b = 3.99;
    b.transform<int>();
    CHECK(b.is<int>() == true);
    CHECK(b.as<int>() == 3);

    any c = 42;
    c.transform<double>();
    CHECK(c.is<double>() == true);
    CHECK(c.as<double>() == 42.0);
}

TEST_CASE("any - transform by type - noop when same type", "[any]") {
    any a = 42;
    a.transform<int>();
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 42);
}

TEST_CASE("any - transform by functor - arithmetic", "[any]") {
    any a = 10;
    a.transform([](int val) { return val * 2; });
    CHECK(a.is<int>() == true);
    CHECK(a.as<int>() == 20);

    any b = 100;
    b.transform([](int val) { return val + 50; });
    CHECK(b.as<int>() == 150);
}

TEST_CASE("any - transform by functor - type change", "[any]") {
    any a = 42;
    a.transform([](int val) { return std::to_string(val); });
    CHECK(a.is<std::string>() == true);
    CHECK(a.as<std::string>() == "42");
}

TEST_CASE("any - transform by functor - string manipulation", "[any]") {
    any a = std::string("hello");
    a.transform([](const std::string &s) { return s + " world"; });
    CHECK(a.is<std::string>() == true);
    CHECK(a.as<std::string>() == "hello world");
}

TEST_CASE("any - convert const any", "[any]") {
    const any a = 42;
    CHECK(a.is_convertible<int>() == true);
    CHECK(a.convert<int>() == 42);
    CHECK(a.is_convertible<double>() == true);
    CHECK(a.convert<double>() == 42.0);
}

TEST_CASE("any - convert - cross-type arithmetic chains", "[any]") {
    any a = 10;
    CHECK(a.convert<double>() == 10.0);
    CHECK(a.convert<float>() == 10.0f);
    CHECK(a.convert<long>() == 10L);
    CHECK(a.convert<short>() == static_cast<short>(10));
    CHECK(a.convert<char>() == static_cast<char>(10));
}
