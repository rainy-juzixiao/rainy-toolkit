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

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;

TEST_CASE("hjson roundtrip - scalars", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc = hjson::parse("i: -42\nf: 1.25\nb: false\nn: null\ns: plain\nq: \"quoted\"\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - containers", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc = hjson::parse("o: {i: 1}\na: [1, 2]\nn: [[1], [2, [3]]]\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - multiline strings", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc = hjson::parse("t: '''\nl1\nl2\n'''\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - keys needing quotes", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc =
        hjson::facade<document>::object({{"a b", "v"}, {"k:1", "v"}, {"#h", "v"}, {"", "v"}, {"{x}", "v"}});
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - escapes survive dumping", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc =
        hjson::facade<document>::object({{"t", "a\tb\nc"}, {"q", "say \"hi\""}, {"bs", "a\\b"}});
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - control characters are escaped", "[willow][hjson][roundtrip]") {
    rainy::core::text::string value("a");
    value.push_back('\0');
    value.push_back('b');
    const hjson::facade<document> doc = hjson::facade<document>::object({{"c", value}});
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - indented output reparses", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc = hjson::parse("o: {i: 1}\na: [1, 2]\n");
    for (const unsigned int indent: {1u, 2u, 4u, 8u}) {
        REQUIRE(hjson::parse(hjson::dump(doc, indent)) == doc);
    }
}

TEST_CASE("hjson roundtrip - wide characters", "[willow][hjson][roundtrip]") {
    const hjson::facade<wdocument> doc = hjson::parse<wdocument>(L"a: 1\nb: wide\nc: [x, y]\n");
    REQUIRE(hjson::parse<wdocument>(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - integer boundaries", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc =
        hjson::parse("min: -2147483648\nmax: 2147483647\nover: 4294967296\n");
    REQUIRE(hjson::parse(hjson::dump(doc)) == doc);
}

TEST_CASE("hjson roundtrip - non finite floats do not survive", "[willow][hjson][roundtrip]") {
    const hjson::facade<document> doc = hjson::parse("a: 1e400\n");
    REQUIRE(doc["a"].is_float());
    REQUIRE(hjson::dump(doc) == "a: null\n");
}
