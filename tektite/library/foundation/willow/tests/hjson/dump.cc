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

#include <sstream>
#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;

TEST_CASE("hjson dump - empty object dumps to empty text", "[willow][hjson][dump]") {
    REQUIRE(hjson::dump(hjson::facade<document>(document_type::object)) == "");
    REQUIRE(hjson::dump(hjson::parse("")) == "");
}

TEST_CASE("hjson dump - quoteless keys and bare values", "[willow][hjson][dump]") {
    hjson::facade<document> doc = hjson::parse("host: example.com\nport: 8080\nflag: true\n");
    const auto text = hjson::dump(doc);
    REQUIRE(text.find("host: example.com") != std::string::npos);
    REQUIRE(text.find("port: 8080") != std::string::npos);
    REQUIRE(text.find("flag: true") != std::string::npos);
    REQUIRE(hjson::parse(text) == doc);
}

TEST_CASE("hjson dump - root braces are omitted", "[willow][hjson][dump]") {
    hjson::facade<document> doc = hjson::parse("a: 1\n");
    REQUIRE(hjson::dump(doc) == "a: 1\n");
}

TEST_CASE("hjson dump - values that look like literals are quoted", "[willow][hjson][dump]") {
    hjson::facade<document> doc = hjson::facade<document>::object(
        {{"num", "42"}, {"kw", "true"}, {"sep", "a:b"}, {"hash", "a#b"}, {"brace", "a{b"}, {"empty", ""}});
    const auto text = hjson::dump(doc);
    REQUIRE(text.find("num: \"42\"") != std::string::npos);
    REQUIRE(text.find("kw: \"true\"") != std::string::npos);
    REQUIRE(text.find("sep: \"a:b\"") != std::string::npos);
    REQUIRE(text.find("hash: \"a#b\"") != std::string::npos);
    REQUIRE(text.find("brace: \"a{b\"") != std::string::npos);
    REQUIRE(text.find("empty: \"\"") != std::string::npos);
    REQUIRE(hjson::parse(text) == doc);
}

TEST_CASE("hjson dump - multiline string uses triple quotes", "[willow][hjson][dump]") {
    hjson::facade<document> doc = hjson::facade<document>::object({{"text", "line one\nline two"}});
    const auto text = hjson::dump(doc);
    REQUIRE(text.find("'''") != std::string::npos);
    REQUIRE(hjson::parse(text) == doc);
}

TEST_CASE("hjson dump - arrays and nested objects", "[willow][hjson][dump]") {
    hjson::facade<document> doc = hjson::parse("list: [1, two, 3.5]\nnested: {inner: x}\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson dump - indent parameter is honored", "[willow][hjson][dump]") {
    hjson::facade<document> doc = hjson::parse("outer: {inner: 1}\n");
    const auto text = hjson::dump(doc, 4);
    REQUIRE(text.find("    inner: 1") != std::string::npos);
    REQUIRE(hjson::parse(text) == doc);
}

TEST_CASE("hjson dump - round trip preserves scalars", "[willow][hjson][dump]") {
    hjson::facade<document> doc =
        hjson::parse("i: -42\nf: 1.25\nb: false\nn: null\ns: plain\nq: \"quoted\"\narr: [1, 2.5, x, true, null]\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson dump - keys with reserved characters are quoted", "[willow][hjson][dump]") {
    hjson::facade<document> doc =
        hjson::facade<document>::object({{"a b", "v"}, {"k:1", "v"}, {"#h", "v"}});
    const auto text = hjson::dump(doc);
    REQUIRE(text.find("\"a b\": v") != std::string::npos);
    REQUIRE(text.find("\"k:1\": v") != std::string::npos);
    REQUIRE(text.find("\"#h\": v") != std::string::npos);
    REQUIRE(hjson::parse(text) == doc);
}

TEST_CASE("hjson dump - wide characters round trip", "[willow][hjson][dump]") {
    const auto doc = hjson::parse<wdocument>(L"a: 1\nb: wide\n");
    REQUIRE(doc[L"a"].as_integer() == 1);
    REQUIRE(doc[L"b"].as_string() == L"wide");
    REQUIRE(hjson::parse<wdocument>(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson dump - nested object layout", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("o: {i: 1, s: two}\n");
    REQUIRE(hjson::dump(doc) == "o: {\n    i: 1\n    s: two  }\n");
}

TEST_CASE("hjson dump - array layout", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("a: [1, two, 3.5]\n");
    REQUIRE(hjson::dump(doc) == "a: [\n    1\n    two\n    3.5\n  ]\n");
}

TEST_CASE("hjson dump - empty containers keep their brackets", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("o: {}\na: []\n");
    REQUIRE(hjson::dump(doc) == "o: {}\na: []\n");
}

TEST_CASE("hjson dump - root array layout honors indent", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("[1, 2]\n");
    REQUIRE(hjson::dump(doc, 4) == "[\n    1\n    2\n]\n");
}

TEST_CASE("hjson dump - multiline string layout", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("t: '''\nl1\nl2\n'''\n");
    REQUIRE(doc["t"].as_string() == "l1\nl2");
    REQUIRE(hjson::dump(doc) == "t: '''\n  l1\n  l2\n  '''\n");
}

TEST_CASE("hjson dump - multiline string nested in an object", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("o: {t: '''\nl1\nl2\n'''}\n");
    REQUIRE(hjson::dump(doc) == "o: {\n    t: '''\n    l1\n    l2\n    '''  }\n");
}

TEST_CASE("hjson dump - indent char is honored", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("o: {i: 1}\n");
    REQUIRE(hjson::dump(doc, 2, '\t') == "o: {\n\t\t\t\ti: 1\t\t}\n");
}

TEST_CASE("hjson dump - an indent of zero falls back to the default width", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("o: {i: 1}\n");
    REQUIRE(hjson::dump(doc, 0) == hjson::dump(doc, 2));
}

TEST_CASE("hjson dump - non finite floats are dumped as null", "[willow][hjson][dump]") {
    const hjson::facade<document> doc =
        hjson::facade<document>::object({{"a", 1.0 / 0.0}, {"b", -1.0 / 0.0}, {"c", 0.1}});
    REQUIRE(hjson::dump(doc) == "a: null\nb: null\nc: 0.1\n");
}

TEST_CASE("hjson dump - integers round trip through dump", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("a: 0\nb: -5\nc: 2147483647\n");
    REQUIRE(hjson::dump(doc) == "a: 0\nb: -5\nc: 2147483647\n");
}

TEST_CASE("hjson dump - dumping to an output adapter writes the same text", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("z: 9\n");
    std::ostringstream out;
    stream_output_adapter<char> adapter(out);
    hjson::dump(doc.as_document(), &adapter);
    REQUIRE(out.str() == "z: 9\n");
}

TEST_CASE("hjson dump - the most negative integer is dumped in full", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("a: -2147483648\nb: 2147483647\nc: -1\n");
    REQUIRE(hjson::dump(doc) == "a: -2147483648\nb: 2147483647\nc: -1\n");
}

TEST_CASE("hjson dump - out of range integers dump without loss", "[willow][hjson][dump]") {
    const hjson::facade<document> doc = hjson::parse("a: 4294967296\n");
    REQUIRE(hjson::dump(doc) == "a: 4294967296\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}
