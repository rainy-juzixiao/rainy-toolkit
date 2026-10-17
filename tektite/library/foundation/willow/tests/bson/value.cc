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
#include <rainy/foundation/willow/bson.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::bson;

TEST_CASE("bson value - scalar element types decode", "[willow][bson]") {
    using bytes = byte_buffer;
    REQUIRE(bson::decode(bytes{0x09, 0x00, 0x00, 0x00, 0x08, 0x62, 0x00, 0x01, 0x00})["b"].as_bool());
    REQUIRE(bson::decode(bytes{0x08, 0x00, 0x00, 0x00, 0x0a, 0x6e, 0x00, 0x00})["n"].is_null());
    REQUIRE(bson::decode(bytes{0x0c, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00})["a"].as_integer() == 42);
    REQUIRE(bson::decode(bytes{0x10, 0x00, 0x00, 0x00, 0x01, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00})["d"]
                .as_float() == 1.0);
}

TEST_CASE("bson value - a string decodes", "[willow][bson]") {
    const byte_buffer bytes{0x16, 0x00, 0x00, 0x00, 0x02, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00,
                            0x06, 0x00, 0x00, 0x00, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x00, 0x00};
    auto doc = bson::decode(bytes);
    REQUIRE(doc.is_object());
    REQUIRE(doc["hello"].as_string() == "world");
}

TEST_CASE("bson value - an int64 element decodes", "[willow][bson]") {
    const byte_buffer bytes{0x10, 0x00, 0x00, 0x00, 0x12, 0x61, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    REQUIRE(bson::decode(bytes)["a"].as_integer() == 42);
}

TEST_CASE("bson value - a nested document decodes to an object", "[willow][bson]") {
    document inner(document_type::object);
    inner["x"] = document(1);
    document outer(document_type::object);
    outer["child"] = inner;
    auto back = bson::decode(bson::encode(outer));
    REQUIRE(back["child"].is_object());
    REQUIRE(back["child"]["x"].as_integer() == 1);
}

TEST_CASE("bson value - an array decodes to an array", "[willow][bson]") {
    document array(document_type::array);
    array.push_back(document(1));
    array.push_back(document(2));
    array.push_back(document(3));
    document root(document_type::object);
    root["nums"] = array;
    auto back = bson::decode(bson::encode(root));
    REQUIRE(back["nums"].is_array());
    REQUIRE(back["nums"].size() == 3);
    REQUIRE(back["nums"][2].as_integer() == 3);
}

TEST_CASE("bson value - accessors reject a mismatched type", "[willow][bson]") {
    auto doc = bson::decode(byte_buffer{0x0c, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00});
    REQUIRE_THROWS_AS(doc["a"].as_string(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(doc["a"].as_bool(), rainy::foundation::exceptions::willow::willow_type_error);
}

TEST_CASE("bson value - the document is an object with the decoded keys", "[willow][bson]") {
    document src(document_type::object);
    src["a"] = document(1);
    src["b"] = document(std::string("two"));
    auto back = bson::decode(bson::encode(src));
    REQUIRE(back.size() == 2);
    REQUIRE(back.contains("a"));
    REQUIRE(back.contains("b"));
    REQUIRE(back.count("a") == 1);
}

TEST_CASE("bson value - the facade converts from another document", "[willow][bson]") {
    document source(document_type::object);
    source["name"] = document(std::string("rainy"));
    facade<document> converted{from_other_document, source};
    converted["name"] = document(std::string("changed"));
    REQUIRE(source["name"].as_string() == "rainy");
}
