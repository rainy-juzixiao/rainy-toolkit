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
#include <rainy/foundation/willow/json5.hpp>
#include <rainy/foundation/willow/protobuf.hpp>
#include <rainy/foundation/willow/yaml.hpp>

using namespace rainy::foundation::willow;

TEST_CASE("willow facade conversion deep-copies generic document values", "[willow][facade]") {
    auto source = json::parse(R"({"name":"rainy","items":[1,{"enabled":true}]})");
    json5::facade<document> same_document_type{from_other_document, source};
    same_document_type["name"] = "copied";
    REQUIRE(source["name"].as_string() == "rainy");

    yaml::facade<document> converted{from_other_document, source};

    REQUIRE(converted.is_object());
    REQUIRE(converted.contains("name"));
    REQUIRE(converted.contains("items"));
    REQUIRE(converted["name"].as_string() == "rainy");
    REQUIRE(converted["items"][0].as_integer() == 1);
    REQUIRE(converted["items"][1]["enabled"].as_bool());

    converted["items"][1]["enabled"] = false;
    REQUIRE(source["items"][1]["enabled"].as_bool());

    json5::facade<document> converted_again{json5::from_other_document, converted};
    REQUIRE(converted_again["name"].as_string() == "rainy");
    REQUIRE(converted_again["items"][1]["enabled"].as_bool() == false);

    yaml::facade<wdocument> wide{from_other_document, source};
    REQUIRE(wide[L"name"].as_string() == L"rainy");
    REQUIRE(wide[L"items"][1][L"enabled"].as_bool());
}

TEST_CASE("willow facade conversion preserves generic values but not YAML metadata", "[willow][facade]") {
    auto source = yaml::parse("name: rainy\nitems: [1, 2]\n");
    source.style() = "block";
    source["name"].style() = "quoted";

    yaml::facade<document> copied{yaml::from_other_document, source};
    REQUIRE(copied.contains("name"));
    REQUIRE(copied.contains("items"));
    REQUIRE(copied == source);
    REQUIRE(!copied.has_style());
    REQUIRE(!copied["name"].has_style());

    json::facade<document> converted{json::from_other_document, source};
    REQUIRE(converted["name"].as_string() == "rainy");
    REQUIRE(converted["items"][1].as_integer() == 2);
}

#if RAINY_HAS_CXX20
TEST_CASE("willow facade conversion transcodes Unicode between character encodings", "[willow][facade]") {
    auto source = json::parse(R"({"文本":"中😀"})");
    REQUIRE(source["文本"].as_string() == "中😀");

    yaml::facade<wdocument> wide{from_other_document, source};
    REQUIRE(wide[L"文本"].as_string().size() == 3);
    REQUIRE(wide[L"文本"].as_string() == L"中😀");
    json::facade<document> from_wide{from_other_document, wide};
    REQUIRE(from_wide["文本"].as_string() == "中😀");

    yaml::facade<u16document> utf16{from_other_document, source};
    REQUIRE(utf16[u"文本"].as_string() == u"中😀");

    using document32 = basic_document<rainy::foundation::collections::unordered_map, rainy::core::collections::vector,
                                      rainy::core::text::u32string>;
    yaml::facade<document32> utf32{from_other_document, source};
    REQUIRE(utf32[U"文本"].as_string() == U"中😀");

    yaml::facade<u8document> utf8{from_other_document, utf32};
    REQUIRE(utf8[u8"文本"].as_string() == u8"中😀");
}

namespace {
    struct facade_conversion_message {
        static constexpr rainy::core::container::tuple<protobuf::field<"value", protobuf::proto::int32, 1>>
            protobuf_fields{};
    };
}

TEST_CASE("willow protobuf facade accepts another facade document", "[willow][facade][protobuf]") {
    auto source = json::parse(R"({"value":42,"extra":"retained"})");
    protobuf::facade<facade_conversion_message, document> converted{protobuf::from_other_document, source};

    REQUIRE(converted.contains("value"));
    REQUIRE(converted.contains("extra"));
    REQUIRE(converted["value"].as_integer() == 42);
    REQUIRE(converted["extra"].as_string() == "retained");
    converted["value"] = 7;
    REQUIRE(source["value"].as_integer() == 42);
}
#endif
