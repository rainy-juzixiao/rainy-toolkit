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
#include <variant>

using namespace rainy::foundation::container;

TEST_CASE("any - match_for - with explicit variant types int", "[any]") {
    any a = 42;
    std::variant<int, double, std::string> result = a.match_for<int, double, std::string>(
        [](int val) { return val * 2; },
        [](double val) { return static_cast<int>(val * 2.0); },
        [](const std::string &s) { return s; }
    );
    CHECK(std::holds_alternative<int>(result));
    CHECK(std::get<int>(result) == 84);
}

TEST_CASE("any - match_for - with explicit variant types string", "[any]") {
    any a = std::string("hello");
    std::variant<int, double, std::string> result = a.match_for<int, double, std::string>(
        [](int val) { return val; },
        [](double val) { return static_cast<int>(val); },
        [](const std::string &s) { return s; }
    );
    CHECK(std::holds_alternative<std::string>(result));
    CHECK(std::get<std::string>(result) == "hello");
}

TEST_CASE("any - match_for - with explicit variant types double via convert", "[any]") {
    any a = 3.14;
    std::variant<int, double, std::string> result = a.match_for<int, double, std::string>(
        [](int val) { return val; },
        [](double val) { return static_cast<int>(val); },
        [](const std::string &s) { return s; }
    );
    CHECK(std::holds_alternative<int>(result));
}

TEST_CASE("any - match_for - with auto_deduce int", "[any]") {
    any a = 100;
    auto result = a.match_for(auto_deduce,
        [](int val) { return val; },
        [](double val) { return static_cast<std::size_t>(val); },
        [](const std::string &s) { return static_cast<float>(s.size()); }
    );
    CHECK(rainy::core::container::holds_alternative<int>(result));
    CHECK(std::get<int>(result) == 100);
}

TEST_CASE("any - match_for - auto_deduce with double exact match", "[any]") {
    any a = 2.718;
    auto result = a.match_for(auto_deduce,
        [](double val) { return static_cast<int>(val * 100); },
        [](const std::string &s) { return s; }
    );
    CHECK(rainy::core::container::holds_alternative<int>(result));
    CHECK(std::get<int>(result) == 271);
}

TEST_CASE("any - match_for - auto_deduce with string", "[any]") {
    any a = std::string("world");
    auto result = a.match_for(auto_deduce,
        [](int val) { return static_cast<double>(val); },
        [](const std::string &s) { return s; }
    );
    CHECK(rainy::core::container::holds_alternative<std::string>(result));
    CHECK(std::get<std::string>(result) == "world");
}

TEST_CASE("any - match_for - auto_deduce bool", "[any]") {
    any a = true;
    auto result = a.match_for(auto_deduce,
        [](bool val) { return val ? 1 : 0; },
        [](int val) { return std::to_string(val); }
    );
    CHECK(rainy::core::container::holds_alternative<int>(result));
    CHECK(std::get<int>(result) == 1);
}

TEST_CASE("any - match_for - empty any returns default variant", "[any]") {
    any a;
    std::variant<int, std::string> result = a.match_for<int, std::string>(
        [](int val) { return val; },
        [](const std::string &s) { return s; }
    );
    CHECK(result.index() == 0);
}
