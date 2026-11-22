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
#include <rainy/foundation/willow/xml.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::xml;
using rainy::foundation::exceptions::willow::xml::xml_parse_error;

TEST_CASE("xml error - a document without a root element is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse(""), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("just text"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<!-- only a comment -->"), xml_parse_error);
}

TEST_CASE("xml error - an unclosed start tag is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a b=\"1\""), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a/"), xml_parse_error);
}

TEST_CASE("xml error - an unclosed element is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a><b></b>"), xml_parse_error);
}

TEST_CASE("xml error - a mismatched closing tag is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a></b>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a><b></a></b>"), xml_parse_error);
}

TEST_CASE("xml error - an empty tag name is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("</>"), xml_parse_error);
}

TEST_CASE("xml error - an unquoted attribute value is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a b=1/>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a b=/>"), xml_parse_error);
}

TEST_CASE("xml error - an attribute without a value is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a b/>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a b c=\"1\"/>"), xml_parse_error);
}

TEST_CASE("xml error - an unterminated attribute value is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a b=\"1/>"), xml_parse_error);
}

TEST_CASE("xml error - a duplicate attribute is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a b=\"1\" b=\"2\"/>"), xml_parse_error);
}

TEST_CASE("xml error - a bare '<' in an attribute value is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a b=\"x < y\"/>"), xml_parse_error);
}

TEST_CASE("xml error - an unterminated comment is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r><!-- never closed</r>"), xml_parse_error);
}

TEST_CASE("xml error - a double dash inside a comment is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r><!-- a -- b --></r>"), xml_parse_error);
}

TEST_CASE("xml error - an unterminated cdata section is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r><![CDATA[never closed</r>"), xml_parse_error);
}

TEST_CASE("xml error - a malformed cdata opener is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r><![CDATA never closed]]></r>"), xml_parse_error);
}

TEST_CASE("xml error - an unsupported markup declaration is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r><!ENTITY x \"y\"></r>"), xml_parse_error);
}

TEST_CASE("xml error - an unterminated doctype is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<!DOCTYPE note"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<!DOCTYPE note [ <!ELEMENT note (to)>"), xml_parse_error);
}

TEST_CASE("xml error - an unterminated processing instruction is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<?target data"), xml_parse_error);
}

TEST_CASE("xml error - an unknown entity is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r>&unknown;</r>"), xml_parse_error);
}

TEST_CASE("xml error - an entity without a terminator is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r>&amp</r>"), xml_parse_error);
}

TEST_CASE("xml error - a character reference out of range is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<r>&#x110000;</r>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<r>&#xD800;</r>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<r>&#;</r>"), xml_parse_error);
}

TEST_CASE("xml error - a declaration without a version is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<?xml encoding=\"utf-8\"?><r/>"), xml_parse_error);
}

TEST_CASE("xml error - an unterminated declaration is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<?xml version=\"1.0\"<r/>"), xml_parse_error);
}

TEST_CASE("xml error - content after the root element is rejected", "[willow][xml]") {
    REQUIRE_THROWS_AS(xml::parse("<a/><b/>"), xml_parse_error);
    REQUIRE_THROWS_AS(xml::parse("<a/>trailing"), xml_parse_error);
}
