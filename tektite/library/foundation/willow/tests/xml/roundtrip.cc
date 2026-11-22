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

namespace {
    template <typename BasicDocument = document>
    void require_roundtrip(const rainy::core::text::basic_string<typename BasicDocument::char_type> &text) {
        xml::facade<BasicDocument> doc = xml::parse<BasicDocument>(text);
        const auto dumped = xml::dump(doc);
        xml::facade<BasicDocument> again = xml::parse<BasicDocument>(dumped);
        REQUIRE(again == doc);
    }
}

TEST_CASE("xml dump - a leaf element round trips", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<port>8080</port>"});
}

TEST_CASE("xml dump - nested elements round trip", "[willow][xml]") {
    require_roundtrip<>(document::string_type{
        "<server>"
        "<host>localhost</host>"
        "<port>8080</port>"
        "</server>"});
}

TEST_CASE("xml dump - attributes round trip", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<a first=\"1\" second=\"two words\"/>"});
}

TEST_CASE("xml dump - a self closing element round trips", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<r><empty/></r>"});
}

TEST_CASE("xml dump - repeated elements round trip", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<r><item>a</item><item>b</item><item>c</item></r>"});
}

TEST_CASE("xml dump - mixed content round trips", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<p>before <b>bold</b> after</p>"});
}

TEST_CASE("xml dump - cdata round trips", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<r><![CDATA[a < b & c]]></r>"});
}

TEST_CASE("xml dump - the declaration round trips", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<?xml version=\"1.0\" encoding=\"utf-8\"?><r/>"});
}

TEST_CASE("xml dump - deep nesting round trips", "[willow][xml]") {
    require_roundtrip<>(document::string_type{"<a><b><c><d>deep</d></c></b></a>"});
}

TEST_CASE("xml dump - reserved characters in text are escaped", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r>a &amp; b &lt;x&gt;</r>");
    const auto dumped = xml::dump(doc);

    REQUIRE(dumped.find("&amp;") != document::string_type::npos);
    REQUIRE(dumped.find("&lt;") != document::string_type::npos);
    REQUIRE(dumped.find("&gt;") != document::string_type::npos);
    REQUIRE(xml::parse(dumped) == doc);
}

TEST_CASE("xml dump - reserved characters in attributes are escaped", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r value=\"a &amp; b &lt; c &quot; d\"/>");
    const auto dumped = xml::dump(doc);

    REQUIRE(dumped.find("&amp;") != document::string_type::npos);
    REQUIRE(dumped.find("&lt;") != document::string_type::npos);
    REQUIRE(dumped.find("&quot;") != document::string_type::npos);
    REQUIRE(xml::parse(dumped) == doc);
}

TEST_CASE("xml dump - a cdata section holding the terminator is split", "[willow][xml]") {
    xml::facade<document> doc(document_type::object);
    xml::facade<document> root(document_type::object);
    xml::facade<document> content(document::string_type{"a]]>b"});
    content.mark_cdata();
    root["r"] = content;
    doc["root"] = root;

    const auto dumped = xml::dump(root);
    REQUIRE(dumped.find("]]&gt;") == document::string_type::npos);
    REQUIRE(dumped.find("<![CDATA[a]]]]><![CDATA[>b]]>") != document::string_type::npos);
}

TEST_CASE("xml dump - the indented form is still parseable", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r><a>1</a><b><c>2</c></b></r>");
    const auto dumped = xml::dump(doc, 2);

    REQUIRE(dumped.find('\n') != document::string_type::npos);
    xml::facade<document> again = xml::parse(dumped);
    REQUIRE(again["r"]["a"].as_string() == "1");
    REQUIRE(again["r"]["b"]["c"].as_string() == "2");
}

TEST_CASE("xml dump - a non object document is rejected", "[willow][xml]") {
    xml::facade<document> scalar(document_type::array);
    REQUIRE_THROWS_AS(xml::dump(scalar), rainy::foundation::exceptions::willow::xml::xml_serialize_error);
}

TEST_CASE("xml dump - wide characters round trip", "[willow][xml]") {
    require_roundtrip<wdocument>(rainy::core::text::wstring{
        L"<server>"
        L"<host>localhost</host>"
        L"</server>"});
}

TEST_CASE("xml dump - the round trip survives a long run", "[willow][xml]") {
    document::string_type body;
    body.reserve(5000);
    for (std::size_t i = 0; i < 5000; ++i) {
        body.push_back('x');
    }
    document::string_type source = document::string_type{"<r>"} + body + document::string_type{"</r>"};

    xml::facade<document> doc = xml::parse(source);
    REQUIRE(doc["r"].as_string() == body);
    const auto dumped = xml::dump(doc);
    REQUIRE(xml::parse(dumped)["r"].as_string() == body);
}
