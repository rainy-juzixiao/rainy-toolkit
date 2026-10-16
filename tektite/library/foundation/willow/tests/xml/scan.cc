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
    using doc_string = document::string_type;

    doc_string repeat(const std::size_t count, const char fill) {
        doc_string result;
        result.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            result.push_back(fill);
        }
        return result;
    }

    doc_string wrap(const doc_string &body) {
        return doc_string{"<r>"} + body + doc_string{"</r>"};
    }
}

TEST_CASE("xml scan - a run shorter than one simd block is still exact", "[willow][xml][scan]") {
    const std::size_t width = static_cast<std::size_t>(rainy::core::vec<char>::size());
    for (std::size_t length = 0; length < width * 2 + 2; ++length) {
        const doc_string body = repeat(length, 'a');
        xml::facade<document> doc = xml::parse(wrap(body));
        if (length == 0) {
            REQUIRE(doc["r"].is_null());
            continue;
        }
        REQUIRE(doc["r"].as_string() == body);
    }
}

TEST_CASE("xml scan - a stop landing on a block boundary is found", "[willow][xml][scan]") {
    const std::size_t width = static_cast<std::size_t>(rainy::core::vec<char>::size());
    for (std::size_t offset = width - 2; offset <= width + 2; ++offset) {
        const doc_string body = repeat(offset, 'a');
        xml::facade<document> doc = xml::parse(wrap(body + doc_string{"&amp;"} + doc_string{"tail"}));
        REQUIRE(doc["r"].as_string() == body + doc_string{"&tail"});
    }
}

TEST_CASE("xml scan - a long character data run spans many blocks", "[willow][xml][scan]") {
    const doc_string body = repeat(5000, 'x');
    xml::facade<document> doc = xml::parse(wrap(body));
    REQUIRE(doc["r"].as_string() == body);
}

TEST_CASE("xml scan - a long comment body spans many blocks", "[willow][xml][scan]") {
    const doc_string body = repeat(5000, 'z');
    xml::facade<document> doc = xml::parse(doc_string{"<r><!--"} + body + doc_string{"--><a>1</a></r>"});
    REQUIRE(doc["r"]["a"].as_string() == "1");
}

TEST_CASE("xml scan - a single dash in a comment body is kept", "[willow][xml][scan]") {
    const doc_string body = repeat(5000, 'z');
    xml::facade<document> doc = xml::parse(doc_string{"<r><!--"} + body + doc_string{"-a--><b>1</b></r>"});
    REQUIRE(doc["r"]["b"].as_string() == "1");
}

TEST_CASE("xml scan - a long cdata body spans many blocks", "[willow][xml][scan]") {
    const doc_string body = repeat(5000, 'y');
    xml::facade<document> doc = xml::parse(doc_string{"<r><![CDATA["} + body + doc_string{"]]></r>"});
    REQUIRE(doc["r"].as_string() == body);
    REQUIRE(doc["r"].is_cdata());
}

TEST_CASE("xml scan - a ']' inside cdata not followed by '>' is kept", "[willow][xml][scan]") {
    xml::facade<document> doc = xml::parse("<r><![CDATA[a]]b]]></r>");
    REQUIRE(doc["r"].as_string() == "a]]b");
}

TEST_CASE("xml scan - the cdata terminator is found after a long run", "[willow][xml][scan]") {
    const doc_string body = repeat(5000, ']');
    xml::facade<document> doc = xml::parse(doc_string{"<r><![CDATA["} + body + doc_string{"]]></r>"});
    REQUIRE(doc["r"].as_string() == body);
}

TEST_CASE("xml scan - a long attribute value spans many blocks", "[willow][xml][scan]") {
    const doc_string body = repeat(5000, 'v');
    xml::facade<document> doc = xml::parse(doc_string{"<r value=\""} + body + doc_string{"\"/>"});
    REQUIRE(*doc["r"].find_attribute("value") == body);
}

TEST_CASE("xml scan - an entity at the very end of the input is decoded", "[willow][xml][scan]") {
    xml::facade<document> doc = xml::parse("<r>tail&amp;</r>");
    REQUIRE(doc["r"].as_string() == "tail&");
}

TEST_CASE("xml scan - an entity as the whole text is decoded", "[willow][xml][scan]") {
    xml::facade<document> doc = xml::parse("<r>&lt;</r>");
    REQUIRE(doc["r"].as_string() == "<");
}
