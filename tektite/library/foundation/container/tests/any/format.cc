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
#include <rainy/core/text/format.hpp>
#include <rainy/foundation/container/any.hpp>

#include <string>
#include <iostream>
#include <vector>

using namespace rainy::foundation::container;
using rainy::core::text::format;

TEST_CASE("any - format - integer", "[any]") {
    any value = 42;
    CHECK(format("{}", value) == "42");
}

TEST_CASE("any - format - string", "[any]") {
    any value = std::string("hello");
    CHECK(format("{}", value) == "hello");
}

TEST_CASE("any - format - floating point", "[any]") {
    any value = 1.5;
    CHECK(format("{}", value) == "1.5");
}

TEST_CASE("any - format - boolean", "[any]") {
    any value = true;
    CHECK(format("{}", value) == "1");
}

TEST_CASE("any - format - literal text around the placeholder", "[any]") {
    any value = 7;
    CHECK(format("value = {}", value) == "value = 7");
}

TEST_CASE("any - format - honours the specification", "[any]") {
    any value = 42;
    CHECK(format("{:>6}", value) == "    42");
    CHECK(format("{:06}", value) == "000042");
}

TEST_CASE("any - format - wide string", "[any]") {
    any value = 42;
    CHECK(format(L"{}", value) == L"42");
}

TEST_CASE("any - format - range", "[any]") {
    any value = std::vector<int>{1, 2, 3};
    CHECK(format("{}", value) == "[1, 2, 3]");
}

TEST_CASE("any - format - empty any is not formattable", "[any]") {
    any value;
    rainy::core::text::string out;
    rainy::core::text::basic_format_parse_context<char> parse_context{rainy::core::text::string_view{}};
    CHECK(value.format(parse_context, out) == false);
}
