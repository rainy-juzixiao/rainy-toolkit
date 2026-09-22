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

TEST_CASE("any - match - single typed handler int", "[any]") {
    any a = 42;
    any result = a.match([](int val) { return val * 2; });
    CHECK(result.has_value() == true);
    CHECK(result.as<int>() == 84);
}

TEST_CASE("any - match - single typed handler double", "[any]") {
    any a = 3.0;
    any result = a.match([](double val) { return val * 3.0; });
    CHECK(result.as<double>() == 9.0);
}

TEST_CASE("any - match - single typed handler string", "[any]") {
    any a = std::string("test");
    any result = a.match([](const std::string &s) { return s.size(); });
    CHECK(result.as<std::size_t>() == 4);
}

TEST_CASE("any - match - multiple typed handlers int matches", "[any]") {
    any a = 10;
    any result = a.match(
        [](int val) { return val + 1; },
        [](double val) { return static_cast<int>(val + 2); },
        [](const std::string &s) { return 0; }
    );
    CHECK(result.as<int>() == 11);
}

TEST_CASE("any - match - string among typed handlers", "[any]") {
    any a = std::string("hello");
    any result = a.match(
        [](int val) { return std::to_string(val); },
        [](const std::string &s) { return s + " world"; }
    );
    CHECK(result.as<std::string>() == "hello world");
}

TEST_CASE("any - match - second handler matches double when first cannot convert", "[any]") {
    any a = 3.14;
    any result = a.match(
        [](const std::string &s) { return std::string("str"); },
        [](double val) { return std::string("double"); }
    );
    CHECK(result.as<std::string>() == "double");
}

TEST_CASE("any - match - first handler wins via convert", "[any]") {
    any a = 3.14;
    any result = a.match(
        [](int val) { return std::string("int_via_convert"); },
        [](double val) { return std::string("double_exact"); }
    );
    CHECK(result.as<std::string>() == "int_via_convert");
}

TEST_CASE("any - match - void return handler with side effect", "[any]") {
    any a = 42;
    int captured = 0;
    a.match([&captured](int val) { captured = val; });
    CHECK(captured == 42);
}

TEST_CASE("any - match - convertible handler double to int", "[any]") {
    any a = 3.99;
    any result = a.match([](int val) { return val; });
    CHECK(result.has_value() == true);
    CHECK(result.as<int>() == 3);
}

TEST_CASE("any - match - with explicit return type", "[any]") {
    any a = 42;
    std::string result = a.match<std::string>(
        [](int val) { return std::to_string(val); },
        [](double val) { return std::to_string(val); }
    );
    CHECK(result == "42");
}

TEST_CASE("any - match - default handler without any arg", "[any]") {
    any a = std::vector<int>{1, 2, 3};
    bool default_called = false;
    any result = a.match(
        [](int val) { return std::string("int"); },
        [&default_called](any_default_match) { default_called = true; }
    );
    CHECK(default_called == true);
}

TEST_CASE("any - match - default handler with any arg", "[any]") {
    any a = std::vector<int>{1, 2, 3};
    bool default_called = false;
    any result = a.match(
        [](int val) { return std::string("int"); },
        [&default_called](any_default_match, const any & /*unused*/) { default_called = true; }
    );
    CHECK(default_called == true);
}

TEST_CASE("any - match - default handler not called when typed matches", "[any]") {
    any a = 42;
    any result = a.match(
        [](int val) { return val; },
        [](any_default_match) { return 0; }
    );
    CHECK(result.as<int>() == 42);
}

TEST_CASE("any - match - empty any returns empty", "[any]") {
    any a;
    any result = a.match([](int val) { return val; });
    CHECK(result.has_value() == false);
}

TEST_CASE("any - match - match on bool", "[any]") {
    any a = true;
    any result = a.match(
        [](bool val) { return val ? std::string("true") : std::string("false"); }
    );
    CHECK(result.as<std::string>() == "true");
}
