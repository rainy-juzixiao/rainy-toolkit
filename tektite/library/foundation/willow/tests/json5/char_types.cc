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
#include <rainy/foundation/willow/json5.hpp>

using namespace rainy::foundation::willow;

namespace fc = rainy::foundation::collections;
namespace rc = rainy::core;

namespace {
    using u32document = basic_document<fc::unordered_map, rc::collections::vector, rc::text::u32string>;
}

TEST_CASE("json5 char_type - char parse and round trip", "[willow][json5][char_types]") {
    const auto doc = json5::parse(R"({a:0x1F,b:.5,c:'x',d:-3,e:"y"})");
    REQUIRE(doc["a"].as_integer() == 31);
    REQUIRE(doc["b"].as_float() == 0.5);
    REQUIRE(doc["c"].as_string() == "x");
    REQUIRE(doc["d"].as_integer() == -3);
    REQUIRE(doc["e"].as_string() == "y");
    REQUIRE(json5::parse(json5::dump(doc)) == doc);
}

TEST_CASE("json5 char_type - wchar_t parse and round trip", "[willow][json5][char_types]") {
    const auto doc = json5::parse<wdocument>(LR"({a:0x1F,b:.5,c:'x',d:-3,e:"y"})");
    REQUIRE(doc[L"a"].as_integer() == 31);
    REQUIRE(doc[L"b"].as_float() == 0.5);
    REQUIRE(doc[L"c"].as_string() == L"x");
    REQUIRE(doc[L"d"].as_integer() == -3);
    REQUIRE(doc[L"e"].as_string() == L"y");
    REQUIRE(json5::parse<wdocument>(json5::dump(doc)) == doc);
}

TEST_CASE("json5 char_type - char16_t parse and round trip", "[willow][json5][char_types]") {
    const auto doc = json5::parse<u16document>(uR"({a:0x1F,b:.5,c:'x',d:-3,e:"y"})");
    REQUIRE(doc[u"a"].as_integer() == 31);
    REQUIRE(doc[u"b"].as_float() == 0.5);
    REQUIRE(doc[u"c"].as_string() == u"x");
    REQUIRE(doc[u"d"].as_integer() == -3);
    REQUIRE(doc[u"e"].as_string() == u"y");
    REQUIRE(json5::parse<u16document>(json5::dump(doc)) == doc);
}

TEST_CASE("json5 char_type - char32_t parse and round trip", "[willow][json5][char_types]") {
    const auto doc = json5::parse<u32document>(UR"({a:0x1F,b:.5,c:'x',d:-3,e:"y"})");
    REQUIRE(doc[U"a"].as_integer() == 31);
    REQUIRE(doc[U"b"].as_float() == 0.5);
    REQUIRE(doc[U"c"].as_string() == U"x");
    REQUIRE(doc[U"d"].as_integer() == -3);
    REQUIRE(doc[U"e"].as_string() == U"y");
    REQUIRE(json5::parse<u32document>(json5::dump(doc)) == doc);
}

TEST_CASE("json5 char_type - char8_t parse and round trip", "[willow][json5][char_types]") {
    const auto doc = json5::parse<u8document>(u8R"({a:0x1F,b:.5,c:'x',d:-3,e:"y"})");
    REQUIRE(doc[u8"a"].as_integer() == 31);
    REQUIRE(doc[u8"b"].as_float() == 0.5);
    REQUIRE(doc[u8"c"].as_string() == u8"x");
    REQUIRE(doc[u8"d"].as_integer() == -3);
    REQUIRE(doc[u8"e"].as_string() == u8"y");
    REQUIRE(json5::parse<u8document>(json5::dump(doc)) == doc);
}
