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
#include <rainy/foundation/willow/toml.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::toml;

namespace {
    template <typename BasicDocument = document>
    void require_roundtrip(const rainy::core::text::basic_string<typename BasicDocument::char_type> &text) {
        toml::facade<BasicDocument> doc = toml::parse<BasicDocument>(text);
        const auto dumped = toml::dump(doc);
        toml::facade<BasicDocument> again = toml::parse<BasicDocument>(dumped);
        REQUIRE(again == doc);
    }
}

TEST_CASE("toml dump - scalars round trip", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "i = 42\n"
        "neg = -7\n"
        "f = 3.5\n"
        "b = true\n"
        "s = \"hello\"\n"});
}

TEST_CASE("toml dump - integer bases round trip as decimal", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("hex = 0xff\noct = 0o17\nbin = 0b1010\n");
    const auto dumped = toml::dump(doc);
    REQUIRE(dumped.find("255") != rainy::core::text::string::npos);
    REQUIRE(dumped.find("15") != rainy::core::text::string::npos);
    REQUIRE(dumped.find("10") != rainy::core::text::string::npos);
    REQUIRE(toml::parse(dumped) == doc);
}

TEST_CASE("toml dump - nested tables round trip", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "title = \"root\"\n"
        "[server]\n"
        "host = \"localhost\"\n"
        "port = 8080\n"
        "[server.limits]\n"
        "max = 10\n"});
}

TEST_CASE("toml dump - dotted keys round trip", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "a.b.c = 1\n"
        "a.b.d = 2\n"});
}

TEST_CASE("toml dump - arrays round trip", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "ints = [1, 2, 3]\n"
        "strs = [\"a\", \"b\"]\n"
        "nested = [[1, 2], [3, 4]]\n"});
}

TEST_CASE("toml dump - inline tables round trip", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "point = { x = 1, y = 2 }\n"
        "nested = { a = { b = 3 } }\n"});
}

TEST_CASE("toml dump - array of tables round trips", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "[[fruit]]\n"
        "name = \"apple\"\n"
        "[[fruit]]\n"
        "name = \"banana\"\n"});
}

TEST_CASE("toml dump - escaped strings round trip", "[willow][toml]") {
    require_roundtrip<>(rainy::core::text::string{
        "tab = \"a\\tb\"\n"
        "nl = \"a\\nb\"\n"
        "quote = \"a\\\"b\"\n"
        "back = \"a\\\\b\"\n"});
}

TEST_CASE("toml dump - the output is valid toml", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "title = \"example\"\n"
        "[owner]\n"
        "name = \"rainy\"\n"
        "[database]\n"
        "enabled = true\n"
        "ports = [8000, 8001]\n");

    const auto dumped = toml::dump(doc);
    toml::facade<document> reparsed = toml::parse(dumped);
    REQUIRE(reparsed["title"].as_string() == "example");
    REQUIRE(reparsed["owner"]["name"].as_string() == "rainy");
    REQUIRE(reparsed["database"]["enabled"].as_bool());
    REQUIRE(reparsed["database"]["ports"][1].as_integer() == 8001);
}

TEST_CASE("toml dump - wide characters round trip", "[willow][toml]") {
    require_roundtrip<wdocument>(rainy::core::text::wstring{
        L"title = \"example\"\n"
        L"[server]\n"
        L"host = \"localhost\"\n"});
}

TEST_CASE("toml dump - non object document is rejected", "[willow][toml]") {
    toml::facade<document> scalar(42);
    REQUIRE_THROWS_AS(toml::dump(scalar), rainy::foundation::exceptions::willow::toml::toml_serialize_error);
}
