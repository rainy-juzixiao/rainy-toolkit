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
#include <rainy/foundation/willow/ini.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::ini;
using rainy::foundation::exceptions::willow::ini::ini_parse_error;

TEST_CASE("ini error - line without '=' is rejected", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("just some text\n"), ini_parse_error);
    REQUIRE_THROWS_AS(ini::parse("[s]\nno separator here\n"), ini_parse_error);
}

TEST_CASE("ini error - unclosed section header is rejected", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("[server\n"), ini_parse_error);
    REQUIRE_THROWS_AS(ini::parse("[server\nhost = x\n"), ini_parse_error);
}

TEST_CASE("ini error - empty section name is rejected", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("[]\n"), ini_parse_error);
    REQUIRE_THROWS_AS(ini::parse("[   ]\n"), ini_parse_error);
}

TEST_CASE("ini error - empty key is rejected", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("= value\n"), ini_parse_error);
    REQUIRE_THROWS_AS(ini::parse("   = value\n"), ini_parse_error);
    REQUIRE_THROWS_AS(ini::parse("[s]\n= v\n"), ini_parse_error);
}

TEST_CASE("ini error - trailing text after section header is rejected", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("[s] trailing\n"), ini_parse_error);
    REQUIRE_THROWS_AS(ini::parse("[s] k = v\n"), ini_parse_error);
}

TEST_CASE("ini error - section name conflicting with a top-level key is rejected", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("s = v\n[s]\nk = 1\n"), ini_parse_error);
}

TEST_CASE("ini error - trailing text after top-level key is impossible without '='", "[willow][ini]") {
    REQUIRE_THROWS_AS(ini::parse("key\n"), ini_parse_error);
}
