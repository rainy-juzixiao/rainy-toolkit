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

TEST_CASE("xml value - a leaf element holds its character data", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<port>8080</port>");

    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 1);
    REQUIRE(doc["port"].is_string());
    REQUIRE(doc["port"].as_string() == "8080");
    REQUIRE(doc["port"].has_name());
    REQUIRE(doc["port"].name() == "port");
}

TEST_CASE("xml value - an element with children is an object", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<server><host>localhost</host><port>8080</port></server>");

    const auto &server = doc["server"];
    REQUIRE(server.is_object());
    REQUIRE(server.has_name());
    REQUIRE(server.name() == "server");
    REQUIRE(server["host"].as_string() == "localhost");
    REQUIRE(server["port"].as_string() == "8080");
}

TEST_CASE("xml value - attributes are kept in document order", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<a first=\"1\" second=\"2\" third=\"3\"/>");

    const auto &node = doc["a"];
    REQUIRE(node.has_attribute("first"));
    REQUIRE(node.has_attribute("second"));
    REQUIRE(node.has_attribute("third"));
    REQUIRE_FALSE(node.has_attribute("fourth"));
    REQUIRE(*node.find_attribute("second") == "2");
    REQUIRE(node.find_attribute("missing") == nullptr);

    const auto &attributes = node.attributes();
    REQUIRE(attributes.size() == 3);
    REQUIRE(attributes[0].name == "first");
    REQUIRE(attributes[1].name == "second");
    REQUIRE(attributes[2].name == "third");
    REQUIRE(attributes[2].value == "3");
}

TEST_CASE("xml value - a self closing element is null", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r><empty/></r>");

    REQUIRE(doc["r"]["empty"].is_null());
    REQUIRE(doc["r"]["empty"].has_name());
    REQUIRE(doc["r"]["empty"].name() == "empty");
}

TEST_CASE("xml value - an empty element pair is null", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r><empty></empty></r>");

    REQUIRE(doc["r"]["empty"].is_null());
}

TEST_CASE("xml value - repeated child elements become an array", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r><item>a</item><item>b</item><item>c</item></r>");

    const auto &items = doc["r"]["item"];
    REQUIRE(items.is_array());
    REQUIRE(items.size() == 3);
    REQUIRE(items[0].as_string() == "a");
    REQUIRE(items[1].as_string() == "b");
    REQUIRE(items[2].as_string() == "c");
}

TEST_CASE("xml value - mixed content keeps its text under the reserved key", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<p>before <b>bold</b> after</p>");

    const auto &paragraph = doc["p"];
    REQUIRE(paragraph.is_object());
    REQUIRE(paragraph["b"].as_string() == "bold");
    REQUIRE(paragraph["#text"].is_string());
    REQUIRE(paragraph["#text"].as_string() == "before  after");
}

TEST_CASE("xml value - whitespace only text is not kept beside children", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r>\n  <a>1</a>\n  <b>2</b>\n</r>");

    REQUIRE(doc["r"]["a"].as_string() == "1");
    REQUIRE(doc["r"]["b"].as_string() == "2");
    REQUIRE_FALSE(doc["r"].contains("#text"));
}

TEST_CASE("xml value - character and entity references are decoded", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r>a &amp; b &lt;x&gt; &quot;q&quot; &apos;a&apos; &#65; &#x42;</r>");

    REQUIRE(doc["r"].as_string() == "a & b <x> \"q\" 'a' A B");
}

TEST_CASE("xml value - a cdata section is kept verbatim", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r><![CDATA[a < b & c]]></r>");

    REQUIRE(doc["r"].is_string());
    REQUIRE(doc["r"].as_string() == "a < b & c");
    REQUIRE(doc["r"].is_cdata());
}

TEST_CASE("xml value - an element without cdata is not marked as cdata", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r>plain</r>");

    REQUIRE_FALSE(doc["r"].is_cdata());
}

TEST_CASE("xml value - the declaration is read from the root", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?><r/>");

    REQUIRE(doc.has_xml_declaration());
    const auto &declaration = doc.declaration();
    REQUIRE(declaration.version == "1.0");
    REQUIRE(declaration.encoding == "utf-8");
    REQUIRE(declaration.standalone == "yes");
}

TEST_CASE("xml value - a document without a declaration reports none", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<r/>");

    REQUIRE_FALSE(doc.has_xml_declaration());
}

TEST_CASE("xml value - comments and processing instructions are stepped over", "[willow][xml]") {
    xml::facade<document> doc = xml::parse(
        "<!-- leading -->\n"
        "<?target data?>\n"
        "<r>\n"
        "  <!-- inner -->\n"
        "  <a>1</a>\n"
        "</r>\n"
        "<!-- trailing -->");

    REQUIRE(doc.size() == 1);
    REQUIRE(doc["r"]["a"].as_string() == "1");
}

TEST_CASE("xml value - a doctype is stepped over", "[willow][xml]") {
    xml::facade<document> doc = xml::parse(
        "<?xml version=\"1.0\"?>\n"
        "<!DOCTYPE note SYSTEM \"note.dtd\">\n"
        "<note><to>rainy</to></note>");

    REQUIRE(doc.has_xml_declaration());
    REQUIRE(doc["note"]["to"].as_string() == "rainy");
}

TEST_CASE("xml value - a doctype with an internal subset is stepped over", "[willow][xml]") {
    xml::facade<document> doc = xml::parse(
        "<!DOCTYPE note [\n"
        "  <!ELEMENT note (to)>\n"
        "  <!ELEMENT to (#PCDATA)>\n"
        "]>\n"
        "<note><to>rainy</to></note>");

    REQUIRE(doc["note"]["to"].as_string() == "rainy");
}

TEST_CASE("xml value - deep nesting is preserved", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<a><b><c><d><e>deep</e></d></c></b></a>");

    REQUIRE(doc["a"]["b"]["c"]["d"]["e"].as_string() == "deep");
    REQUIRE(doc["a"]["b"]["c"]["d"]["e"].name() == "e");
}

TEST_CASE("xml value - the document is an object with one root", "[willow][xml]") {
    xml::facade<document> doc = xml::parse("<root/>");

    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 1);
    REQUIRE(doc.contains("root"));
}
