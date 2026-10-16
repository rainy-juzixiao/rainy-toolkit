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

namespace fc = rainy::foundation::collections;
namespace rc = rainy::core;

namespace {
    using u32document = basic_document<fc::unordered_map, rc::collections::vector, rc::text::u32string>;

    template <typename BasicDocument, typename Char>
    xml::facade<BasicDocument> xml_parse(const Char *text) {
        rc::text::basic_string<Char> input(text);
        string_input_adapter<rc::text::basic_string<Char>> adapter(input);
        xml::facade<BasicDocument> doc;
        xml::implements::xml_parser<BasicDocument, decltype(adapter)>(adapter).parse(doc);
        return doc;
    }

    template <typename BasicDocument, typename Char>
    void xml_round_trip(const Char *text) {
        const auto doc = xml_parse<BasicDocument>(text);
        const auto again = xml_parse<BasicDocument>(xml::dump(doc).data());
        REQUIRE(again == doc);
    }
}

TEST_CASE("xml char_type - char elements", "[willow][xml][char_types]") {
    const auto doc = xml_parse<document>("<port>8080</port>");
    REQUIRE(doc["port"].as_string() == "8080");
    REQUIRE(doc["port"].name() == "port");
    xml_round_trip<document>("<port>8080</port>");
    xml_round_trip<document>("<r a=\"1\"><b>2</b></r>");
}

TEST_CASE("xml char_type - wchar_t elements", "[willow][xml][char_types]") {
    const auto doc = xml_parse<wdocument>(L"<port>8080</port>");
    REQUIRE(doc[L"port"].as_string() == L"8080");
    REQUIRE(doc[L"port"].name() == L"port");
    xml_round_trip<wdocument>(L"<port>8080</port>");
    xml_round_trip<wdocument>(L"<r a=\"1\"><b>2</b></r>");
}

TEST_CASE("xml char_type - char16_t elements", "[willow][xml][char_types]") {
    const auto doc = xml_parse<u16document>(u"<port>8080</port>");
    REQUIRE(doc[u"port"].as_string() == u"8080");
    REQUIRE(doc[u"port"].name() == u"port");
    xml_round_trip<u16document>(u"<port>8080</port>");
    xml_round_trip<u16document>(u"<r a=\"1\"><b>2</b></r>");
}

TEST_CASE("xml char_type - char32_t elements", "[willow][xml][char_types]") {
    const auto doc = xml_parse<u32document>(U"<port>8080</port>");
    REQUIRE(doc[U"port"].as_string() == U"8080");
    REQUIRE(doc[U"port"].name() == U"port");
    xml_round_trip<u32document>(U"<port>8080</port>");
    xml_round_trip<u32document>(U"<r a=\"1\"><b>2</b></r>");
}

TEST_CASE("xml char_type - char8_t elements", "[willow][xml][char_types]") {
    const auto doc = xml_parse<u8document>(u8"<port>8080</port>");
    REQUIRE(doc[u8"port"].as_string() == u8"8080");
    REQUIRE(doc[u8"port"].name() == u8"port");
    xml_round_trip<u8document>(u8"<port>8080</port>");
    xml_round_trip<u8document>(u8"<r a=\"1\"><b>2</b></r>");
}

TEST_CASE("xml char_type - entities decode for every width", "[willow][xml][char_types]") {
    REQUIRE(xml_parse<document>("<r>a &amp; b</r>")["r"].as_string() == "a & b");
    REQUIRE(xml_parse<wdocument>(L"<r>a &amp; b</r>")[L"r"].as_string() == L"a & b");
    REQUIRE(xml_parse<u16document>(u"<r>a &amp; b</r>")[u"r"].as_string() == u"a & b");
    REQUIRE(xml_parse<u32document>(U"<r>a &amp; b</r>")[U"r"].as_string() == U"a & b");
    REQUIRE(xml_parse<u8document>(u8"<r>a &amp; b</r>")[u8"r"].as_string() == u8"a & b");
}

TEST_CASE("xml char_type - the declaration decodes for every width", "[willow][xml][char_types]") {
    REQUIRE(xml_parse<document>("<?xml version=\"1.0\"?><r/>").declaration().version == "1.0");
    REQUIRE(xml_parse<wdocument>(L"<?xml version=\"1.0\"?><r/>").declaration().version == L"1.0");
    REQUIRE(xml_parse<u16document>(u"<?xml version=\"1.0\"?><r/>").declaration().version == u"1.0");
    REQUIRE(xml_parse<u32document>(U"<?xml version=\"1.0\"?><r/>").declaration().version == U"1.0");
    REQUIRE(xml_parse<u8document>(u8"<?xml version=\"1.0\"?><r/>").declaration().version == u8"1.0");
}
