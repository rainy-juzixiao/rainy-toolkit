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
#include <rainy/foundation/willow/json.hpp>

using namespace rainy::foundation::willow;

namespace {
    using u32document =
        basic_document<rainy::foundation::collections::unordered_map, rainy::core::collections::vector, rainy::core::text::u32string>;
}

TEST_CASE("json char_type - char parse and round trip", "[willow][json][char_types]") {
    const auto doc = json::parse(R"({"a":1,"b":2.5,"c":"x\ty","d":true,"e":null,"f":[1,2],"g":-2.5})");
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_float() == 2.5);
    REQUIRE(doc["c"].as_string() == "x\ty");
    REQUIRE(doc["d"].as_bool());
    REQUIRE(doc["e"].is_null());
    REQUIRE(doc["f"].size() == 2);
    REQUIRE(doc["g"].as_float() == -2.5);
    REQUIRE(json::parse(R"({"e":-2e3})")["e"].as_float() == -2000.0);
    REQUIRE(json::parse(json::dump(doc)) == doc);
    REQUIRE(json::parse(json::dump(doc, 2)) == doc);
}

TEST_CASE("json char_type - wchar_t parse and round trip", "[willow][json][char_types]") {
    const auto doc = json::parse<wdocument>(LR"({"a":1,"b":2.5,"c":"x\ty","d":true,"e":null,"f":[1,2],"g":-2.5})");
    REQUIRE(doc[L"a"].as_integer() == 1);
    REQUIRE(doc[L"b"].as_float() == 2.5);
    REQUIRE(doc[L"c"].as_string() == L"x\ty");
    REQUIRE(doc[L"d"].as_bool());
    REQUIRE(doc[L"e"].is_null());
    REQUIRE(doc[L"f"].size() == 2);
    REQUIRE(doc[L"g"].as_float() == -2.5);
    REQUIRE(json::parse<wdocument>(LR"({"e":-2e3})")[L"e"].as_float() == -2000.0);
    REQUIRE(json::parse<wdocument>(json::dump(doc)) == doc);
    REQUIRE(json::parse<wdocument>(json::dump(doc, 2)) == doc);
}

TEST_CASE("json char_type - char16_t parse and round trip", "[willow][json][char_types]") {
    const auto doc = json::parse<u16document>(uR"({"a":1,"b":2.5,"c":"x\ty","d":true,"e":null,"f":[1,2],"g":-2.5})");
    REQUIRE(doc[u"a"].as_integer() == 1);
    REQUIRE(doc[u"b"].as_float() == 2.5);
    REQUIRE(doc[u"c"].as_string() == u"x\ty");
    REQUIRE(doc[u"d"].as_bool());
    REQUIRE(doc[u"e"].is_null());
    REQUIRE(doc[u"f"].size() == 2);
    REQUIRE(doc[u"g"].as_float() == -2.5);
    REQUIRE(json::parse<u16document>(uR"({"e":-2e3})")[u"e"].as_float() == -2000.0);
    REQUIRE(json::parse<u16document>(json::dump(doc)) == doc);
    REQUIRE(json::parse<u16document>(json::dump(doc, 2)) == doc);
}

TEST_CASE("json char_type - char32_t parse and round trip", "[willow][json][char_types]") {
    const auto doc = json::parse<u32document>(UR"({"a":1,"b":2.5,"c":"x\ty","d":true,"e":null,"f":[1,2],"g":-2.5})");
    REQUIRE(doc[U"a"].as_integer() == 1);
    REQUIRE(doc[U"b"].as_float() == 2.5);
    REQUIRE(doc[U"c"].as_string() == U"x\ty");
    REQUIRE(doc[U"d"].as_bool());
    REQUIRE(doc[U"e"].is_null());
    REQUIRE(doc[U"f"].size() == 2);
    REQUIRE(doc[U"g"].as_float() == -2.5);
    REQUIRE(json::parse<u32document>(json::dump(doc)) == doc);
    REQUIRE(json::parse<u32document>(json::dump(doc, 2)) == doc);
}

TEST_CASE("json char_type - char8_t parse and round trip", "[willow][json][char_types]") {
    const auto doc = json::parse<u8document>(u8R"({"a":1,"b":2.5,"c":"x\ty","d":true,"e":null,"f":[1,2],"g":-2.5})");
    REQUIRE(doc[u8"a"].as_integer() == 1);
    REQUIRE(doc[u8"b"].as_float() == 2.5);
    REQUIRE(doc[u8"c"].as_string() == u8"x\ty");
    REQUIRE(doc[u8"d"].as_bool());
    REQUIRE(doc[u8"e"].is_null());
    REQUIRE(doc[u8"f"].size() == 2);
    REQUIRE(doc[u8"g"].as_float() == -2.5);
    REQUIRE(json::parse<u8document>(u8R"({"e":-2e3})")[u8"e"].as_float() == -2000.0);
    REQUIRE(json::parse<u8document>(json::dump(doc)) == doc);
    REQUIRE(json::parse<u8document>(json::dump(doc, 2)) == doc);
}

TEST_CASE("json char_type - non ascii round trip", "[willow][json][char_types]") {
    const auto wide = json::parse<wdocument>(LR"({"s":"héllo—x"})");
    REQUIRE(wide[L"s"].as_string() == L"héllo—x");
    REQUIRE(json::parse<wdocument>(json::dump(wide)) == wide);
    const auto narrow = json::parse(R"({"s":"héllo—x"})");
    REQUIRE(narrow["s"].as_string() == "héllo—x");
    REQUIRE(json::parse(json::dump(narrow)) == narrow);
    const auto u8_doc = json::parse<u8document>(u8R"({"s":"héllo—x"})");
    REQUIRE(u8_doc[u8"s"].as_string() == u8"héllo—x");
    REQUIRE(json::parse<u8document>(json::dump(u8_doc)) == u8_doc);
}

TEST_CASE("json char_type - escape unicode round trip", "[willow][json][char_types]") {
    const auto doc = json::parse(R"({"s":"héllo"})");
    auto args = serializer_args<std::decay_t<decltype(doc)>>{};
    args.escape_unicode = true;
    const auto text = json::dump(doc, args);
    REQUIRE(text == R"({"s":"h\u00c3\u00a9llo"})");

    const auto wdoc = json::parse<wdocument>(LR"({"s":"héllo"})");
    auto wargs = serializer_args<std::decay_t<decltype(wdoc)>>{};
    wargs.escape_unicode = true;
    const auto wtext = json::dump(wdoc, wargs);
    REQUIRE(json::parse<wdocument>(wtext) == wdoc);
}
