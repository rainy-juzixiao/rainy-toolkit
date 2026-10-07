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

TEST_CASE("ini value - empty input is an empty object", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("");
    REQUIRE(doc.is_object());
    REQUIRE(doc.empty());
    REQUIRE(doc.size() == 0);
}

TEST_CASE("ini value - comment-only input is an empty object", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("; comment\n# another\n\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.empty());
}

TEST_CASE("ini value - top-level keys decode as strings", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("host = example.com\nport = 8080\nenabled = true\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 3);
    REQUIRE(doc["host"].is_string());
    REQUIRE(doc["host"].as_string() == "example.com");
    REQUIRE(doc["port"].as_string() == "8080");
    REQUIRE(doc["enabled"].as_string() == "true");
}

TEST_CASE("ini value - sections decode as nested objects", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("[server]\nhost = example.com\n[limits]\nmax_conn = 1000\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 2);
    REQUIRE(doc["server"].is_object());
    REQUIRE(doc["server"]["host"].as_string() == "example.com");
    REQUIRE(doc["limits"]["max_conn"].as_string() == "1000");
}

TEST_CASE("ini value - keys and values are trimmed", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("  key  \t = \t value \t\n");
    REQUIRE(doc["key"].as_string() == "value");
}

TEST_CASE("ini value - empty value decodes as empty string", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("key=\n[sec]\ninner=\n");
    REQUIRE(doc["key"].as_string() == "");
    REQUIRE(doc["sec"]["inner"].as_string() == "");
}

TEST_CASE("ini value - values keep inner spacing and comment chars", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("a = x  y\nb = v; not a comment\nc = v # not a comment\n");
    REQUIRE(doc["a"].as_string() == "x  y");
    REQUIRE(doc["b"].as_string() == "v; not a comment");
    REQUIRE(doc["c"].as_string() == "v # not a comment");
}

TEST_CASE("ini value - duplicate sections merge and duplicate keys overwrite", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("[s]\na = 1\n[s]\nb = 2\na = 3\n");
    REQUIRE(doc["s"].size() == 2);
    REQUIRE(doc["s"]["a"].as_string() == "3");
    REQUIRE(doc["s"]["b"].as_string() == "2");
}

TEST_CASE("ini value - windows line endings are accepted", "[willow][ini]") {
    ini::facade<document> doc = ini::parse("[s]\r\nk = v\r\n");
    REQUIRE(doc["s"]["k"].as_string() == "v");
}

TEST_CASE("ini value - string literal suffix operator", "[willow][ini]") {
    const auto doc = "[s]\nk = v\n"_ini;
    REQUIRE(doc["s"]["k"].as_string() == "v");
}
