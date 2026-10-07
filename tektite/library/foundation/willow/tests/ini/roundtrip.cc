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

TEST_CASE("ini dump - empty object dumps to empty text", "[willow][ini]") {
    REQUIRE(ini::dump(ini::facade<document>(document_type::object)) == "");
    REQUIRE(ini::dump(ini::parse("")) == "");
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>()),
                      rainy::foundation::exceptions::willow::ini::ini_serialize_error);
}

TEST_CASE("ini dump - top-level keys", "[willow][ini]") {
    ini::facade<document> doc = ini::facade<document>::object({{"host", "example.com"}, {"port", "8080"}});
    const auto text = ini::dump(doc);
    REQUIRE(text.find("host = example.com") != rainy::core::text::string::npos);
    REQUIRE(text.find("port = 8080") != rainy::core::text::string::npos);
    REQUIRE(ini::parse(text) == doc);
}

TEST_CASE("ini dump - sections follow top-level keys", "[willow][ini]") {
    ini::facade<document> doc =
        ini::facade<document>::object({{"global", "g"}, {"server", ini::facade<document>::object({{"host", "h"}})}});
    const auto text = ini::dump(doc);
    REQUIRE(text.find("global = g") != rainy::core::text::string::npos);
    REQUIRE(text.find("[server]") != rainy::core::text::string::npos);
    REQUIRE(text.find("global = g") < text.find("[server]"));
    REQUIRE(ini::parse(text) == doc);
}

TEST_CASE("ini dump - round trip preserves sections and values", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("[server]\nhost = example.com\nport = 8080\n[limits]\nmax_conn = 1000\n");
    REQUIRE(ini::parse(ini::dump(doc)) == doc);
}

TEST_CASE("ini dump - wide characters round trip", "[willow][ini]") {
    const auto doc = ini::parse<wdocument>(L"[server]\nhost = example\n");
    REQUIRE(doc[L"server"][L"host"].as_string() == L"example");
    REQUIRE(ini::parse<wdocument>(ini::dump(doc)) == doc);
}

TEST_CASE("ini dump - integers must be strings to survive the format", "[willow][ini]") {
    ini::facade<document> with_int = ini::facade<document>::object({{"port", 8080}});
    REQUIRE_THROWS_AS(ini::dump(with_int), rainy::foundation::exceptions::willow::ini::ini_serialize_error);
    ini::facade<document> with_str = ini::facade<document>::object({{"port", "8080"}});
    REQUIRE(ini::parse(ini::dump(with_str)) == with_str);
}

TEST_CASE("ini dump - non-string values are rejected", "[willow][ini]") {
    using rainy::foundation::exceptions::willow::ini::ini_serialize_error;
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>(42)), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>(true)), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>(1.5)), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::array({1})), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::object({{"a", 1}})), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::object(
                          {{"s", ini::facade<document>::object({{"k", ini::facade<document>::object({})}})}})),
                      ini_serialize_error);
}

TEST_CASE("ini dump - reserved characters are rejected", "[willow][ini]") {
    using rainy::foundation::exceptions::willow::ini::ini_serialize_error;
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::object({{"a=b", "v"}})), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::object({{"[k]", "v"}})), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::object({{"k", "a\nb"}})), ini_serialize_error);
    REQUIRE_THROWS_AS(ini::dump(ini::facade<document>::object({{"[s]", ini::facade<document>::object({{"k", "v"}})}})),
                      ini_serialize_error);
}
