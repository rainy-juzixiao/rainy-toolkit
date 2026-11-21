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
#include <rainy/foundation/willow/hjson.hpp>

namespace fc = rainy::foundation::collections;
namespace rc = rainy::core;

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;

namespace {
    using u32document = basic_document<fc::unordered_map, rc::collections::vector, rc::text::u32string>;
}

TEST_CASE("hjson char_type - char parse and round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse("a: 1\nb: 2.5\nc: x y\nd: true\ne: null\nf: [1, 2]\ng: -2.5\n");
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_float() == 2.5);
    REQUIRE(doc["c"].as_string() == "x y");
    REQUIRE(doc["d"].as_bool());
    REQUIRE(doc["e"].is_null());
    REQUIRE(doc["f"].size() == 2);
    REQUIRE(doc["g"].as_float() == -2.5);
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
    REQUIRE(hjson::parse(hjson::dump(doc, 2)) == doc);
}

TEST_CASE("hjson char_type - wchar_t parse and round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<wdocument>(L"a: 1\nb: 2.5\nc: x y\nd: true\ne: null\nf: [1, 2]\ng: -2.5\n");
    REQUIRE(doc[L"a"].as_integer() == 1);
    REQUIRE(doc[L"b"].as_float() == 2.5);
    REQUIRE(doc[L"c"].as_string() == L"x y");
    REQUIRE(doc[L"d"].as_bool());
    REQUIRE(doc[L"e"].is_null());
    REQUIRE(doc[L"f"].size() == 2);
    REQUIRE(doc[L"g"].as_float() == -2.5);
    REQUIRE(hjson::parse<wdocument>(hjson::dump(doc)) == doc);
    REQUIRE(hjson::parse<wdocument>(hjson::dump(doc, 2)) == doc);
}

TEST_CASE("hjson char_type - char16_t parse and round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<u16document>(u"a: 1\nc: x y\nd: true\nf: [1, 2]\n");
    REQUIRE(doc[u"a"].as_integer() == 1);
    REQUIRE(doc[u"c"].as_string() == u"x y");
    REQUIRE(doc[u"d"].as_bool());
    REQUIRE(doc[u"f"].size() == 2);
    REQUIRE(hjson::parse<u16document>(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson char_type - char32_t parse and round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<u32document>(U"a: 1\nc: x y\nd: true\nf: [1, 2]\n");
    REQUIRE(doc[U"a"].as_integer() == 1);
    REQUIRE(doc[U"c"].as_string() == U"x y");
    REQUIRE(doc[U"d"].as_bool());
    REQUIRE(doc[U"f"].size() == 2);
    REQUIRE(hjson::parse<u32document>(hjson::dump(doc)) == doc);
}

#if RAINY_HAS_CXX20
TEST_CASE("hjson char_type - char8_t parse and round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<u8document>(u8"a: 1\nc: x y\nd: true\nf: [1, 2]\n");
    REQUIRE(doc[u8"a"].as_integer() == 1);
    REQUIRE(doc[u8"c"].as_string() == u8"x y");
    REQUIRE(doc[u8"d"].as_bool());
    REQUIRE(doc[u8"f"].size() == 2);
    REQUIRE(hjson::parse<u8document>(hjson::dump(doc)) == doc);
}
#endif

TEST_CASE("hjson char_type - non-ascii quoteless values round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse("{\"键\": 值一}");
    REQUIRE(doc["键"].as_string() == "值一");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson char_type - wide character escapes", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<wdocument>(L"k: \"\\u00e9\"\n");
    REQUIRE(doc[L"k"].as_string() == L"é");
}

TEST_CASE("hjson char_type - wide character multiline string round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<wdocument>(L"t:\n  '''\n  l1\n  l2\n  '''\n");
    REQUIRE(doc[L"t"].as_string() == L"l1\nl2");
    REQUIRE(hjson::parse<wdocument>(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson char_type - char32_t non ascii round trip", "[willow][hjson][char_types]") {
    const auto doc = hjson::parse<u32document>(U"k: \"\\u00e9\"\n");
    REQUIRE(doc[U"k"].as_string() == U"é");
    REQUIRE(hjson::parse<u32document>(hjson::dump(doc)) == doc);
}
