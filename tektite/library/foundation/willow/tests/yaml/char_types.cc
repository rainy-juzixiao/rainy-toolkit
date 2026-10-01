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
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/yaml/parser.hpp>
#include <rainy/foundation/willow/implements/yaml/serializer.hpp>

using namespace rainy::foundation::willow;

namespace fc = rainy::foundation::collections;
namespace rc = rainy::core;

namespace {
    using u32document = basic_document<fc::unordered_map, rc::collections::vector, rc::text::u32string>;

    template <typename BasicDocument, typename Char>
    yaml::facade<BasicDocument> yaml_parse(const Char *text) {
        rc::text::basic_string<Char> input(text);
        string_input_adapter<rc::text::basic_string<Char>> adapter(input);
        yaml::facade<BasicDocument> doc;
        yaml::implements::yaml_parser<BasicDocument, decltype(adapter)>(adapter).parse(doc);
        return doc;
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type yaml_dump(const yaml::facade<BasicDocument> &doc) {
        typename BasicDocument::string_type out;
        string_output_adapter<typename BasicDocument::string_type> adapter(out);
        yaml::implements::yaml_serializer<BasicDocument>(&adapter, serializer_args<BasicDocument>{}).dump(doc);
        return out;
    }

    template <typename BasicDocument, typename Char>
    void yaml_scalar_round_trip(const Char *text) {
        const auto doc = yaml_parse<BasicDocument>(text);
        const auto again = yaml_parse<BasicDocument>(yaml_dump(doc).data());
        REQUIRE(again == doc);
    }
}

TEST_CASE("yaml char_type - char scalars", "[willow][yaml][char_types]") {
    const auto int_doc = yaml_parse<document>("42\n");
    REQUIRE(int_doc.as_integer() == 42);
    const auto float_doc = yaml_parse<document>("2.5\n");
    REQUIRE(float_doc.as_float() == 2.5);
    const auto text_doc = yaml_parse<document>("text\n");
    REQUIRE(text_doc.as_string() == "text");
    const auto bool_doc = yaml_parse<document>("true\n");
    REQUIRE(bool_doc.is_bool());
    REQUIRE(bool_doc.as_bool());
    yaml_scalar_round_trip<document>("42\n");
    yaml_scalar_round_trip<document>("2.5\n");
    yaml_scalar_round_trip<document>("text\n");
    yaml_scalar_round_trip<document>("true\n");
}

TEST_CASE("yaml char_type - wchar_t scalars", "[willow][yaml][char_types]") {
    const auto int_doc = yaml_parse<wdocument>(L"42\n");
    REQUIRE(int_doc.as_integer() == 42);
    const auto text_doc = yaml_parse<wdocument>(L"text\n");
    REQUIRE(text_doc.as_string() == L"text");
    const auto bool_doc = yaml_parse<wdocument>(L"true\n");
    REQUIRE(bool_doc.is_bool());
    yaml_scalar_round_trip<wdocument>(L"42\n");
    yaml_scalar_round_trip<wdocument>(L"2.5\n");
    yaml_scalar_round_trip<wdocument>(L"text\n");
    yaml_scalar_round_trip<wdocument>(L"true\n");
}

TEST_CASE("yaml char_type - char16_t scalars", "[willow][yaml][char_types]") {
    const auto int_doc = yaml_parse<u16document>(u"42\n");
    REQUIRE(int_doc.as_integer() == 42);
    const auto text_doc = yaml_parse<u16document>(u"text\n");
    REQUIRE(text_doc.as_string() == u"text");
    const auto bool_doc = yaml_parse<u16document>(u"true\n");
    REQUIRE(bool_doc.is_bool());
    yaml_scalar_round_trip<u16document>(u"42\n");
    yaml_scalar_round_trip<u16document>(u"2.5\n");
    yaml_scalar_round_trip<u16document>(u"text\n");
    yaml_scalar_round_trip<u16document>(u"true\n");
}

TEST_CASE("yaml char_type - char32_t scalars", "[willow][yaml][char_types]") {
    const auto int_doc = yaml_parse<u32document>(U"42\n");
    REQUIRE(int_doc.as_integer() == 42);
    const auto text_doc = yaml_parse<u32document>(U"text\n");
    REQUIRE(text_doc.as_string() == U"text");
    const auto bool_doc = yaml_parse<u32document>(U"true\n");
    REQUIRE(bool_doc.is_bool());
    yaml_scalar_round_trip<u32document>(U"42\n");
    yaml_scalar_round_trip<u32document>(U"2.5\n");
    yaml_scalar_round_trip<u32document>(U"text\n");
    yaml_scalar_round_trip<u32document>(U"true\n");
}

TEST_CASE("yaml char_type - char8_t scalars", "[willow][yaml][char_types]") {
    const auto int_doc = yaml_parse<u8document>(u8"42\n");
    REQUIRE(int_doc.as_integer() == 42);
    const auto text_doc = yaml_parse<u8document>(u8"text\n");
    REQUIRE(text_doc.as_string() == u8"text");
    const auto bool_doc = yaml_parse<u8document>(u8"true\n");
    REQUIRE(bool_doc.is_bool());
    yaml_scalar_round_trip<u8document>(u8"42\n");
    yaml_scalar_round_trip<u8document>(u8"2.5\n");
    yaml_scalar_round_trip<u8document>(u8"text\n");
    yaml_scalar_round_trip<u8document>(u8"true\n");
}

TEST_CASE("yaml char_type - block mapping parse and round trip", "[willow][yaml][char_types]") {
    {
        const auto doc = yaml_parse<document>("a: 1\nb: text\nc: [1, 2]\n");
        REQUIRE(doc["a"].as_integer() == 1);
        REQUIRE(doc["b"].as_string() == "text");
        REQUIRE(doc["c"].is_array());
        REQUIRE(doc["c"].size() == 2);
        yaml_scalar_round_trip<document>("a: 1\nb: text\nc: [1, 2]\n");
    }
    {
        const auto doc = yaml_parse<wdocument>(L"a: 1\nb: text\nc: [1, 2]\n");
        REQUIRE(doc[L"a"].as_integer() == 1);
        REQUIRE(doc[L"b"].as_string() == L"text");
        REQUIRE(doc[L"c"].is_array());
        REQUIRE(doc[L"c"].size() == 2);
        yaml_scalar_round_trip<wdocument>(L"a: 1\nb: text\nc: [1, 2]\n");
    }
    {
        const auto doc = yaml_parse<u16document>(u"a: 1\nb: text\nc: [1, 2]\n");
        REQUIRE(doc[u"a"].as_integer() == 1);
        REQUIRE(doc[u"b"].as_string() == u"text");
        REQUIRE(doc[u"c"].is_array());
        REQUIRE(doc[u"c"].size() == 2);
        yaml_scalar_round_trip<u16document>(u"a: 1\nb: text\nc: [1, 2]\n");
    }
    {
        const auto doc = yaml_parse<u32document>(U"a: 1\nb: text\nc: [1, 2]\n");
        REQUIRE(doc[U"a"].as_integer() == 1);
        REQUIRE(doc[U"b"].as_string() == U"text");
        REQUIRE(doc[U"c"].is_array());
        REQUIRE(doc[U"c"].size() == 2);
        yaml_scalar_round_trip<u32document>(U"a: 1\nb: text\nc: [1, 2]\n");
    }
    {
        const auto doc = yaml_parse<u8document>(u8"a: 1\nb: text\nc: [1, 2]\n");
        REQUIRE(doc[u8"a"].as_integer() == 1);
        REQUIRE(doc[u8"b"].as_string() == u8"text");
        REQUIRE(doc[u8"c"].is_array());
        REQUIRE(doc[u8"c"].size() == 2);
        yaml_scalar_round_trip<u8document>(u8"a: 1\nb: text\nc: [1, 2]\n");
    }
}
