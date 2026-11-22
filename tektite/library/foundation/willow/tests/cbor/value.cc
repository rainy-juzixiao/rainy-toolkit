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
#include <rainy/foundation/willow/cbor.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::cbor;

TEST_CASE("cbor value - integers decode", "[willow][cbor]") {
    REQUIRE(cbor::decode(byte_buffer{0x00}).as_integer() == 0);
    REQUIRE(cbor::decode(byte_buffer{0x17}).as_integer() == 23);
    REQUIRE(cbor::decode(byte_buffer{0x18, 0x18}).as_integer() == 24);
    REQUIRE(cbor::decode(byte_buffer{0x19, 0x03, 0xe8}).as_integer() == 1000);
    REQUIRE(cbor::decode(byte_buffer{0x20}).as_integer() == -1);
    REQUIRE(cbor::decode(byte_buffer{0x39, 0x03, 0xe7}).as_integer() == -1000);
}

TEST_CASE("cbor value - simple values decode", "[willow][cbor]") {
    REQUIRE_FALSE(cbor::decode(byte_buffer{0xf4}).as_bool());
    REQUIRE(cbor::decode(byte_buffer{0xf5}).as_bool());
    REQUIRE(cbor::decode(byte_buffer{0xf6}).is_null());
}

TEST_CASE("cbor value - a text string decodes", "[willow][cbor]") {
    auto doc = cbor::decode(byte_buffer{0x64, 0x49, 0x45, 0x54, 0x46});
    REQUIRE(doc.is_string());
    REQUIRE(doc.as_string() == "IETF");
    REQUIRE_FALSE(doc.is_byte_string());
}

TEST_CASE("cbor value - a byte string decodes and is flagged", "[willow][cbor]") {
    auto doc = cbor::decode(byte_buffer{0x44, 0x01, 0x02, 0x03, 0x04});
    REQUIRE(doc.is_string());
    REQUIRE(doc.as_string().size() == 4);
    REQUIRE(doc.is_byte_string());
}

TEST_CASE("cbor value - arrays and maps decode", "[willow][cbor]") {
    auto array = cbor::decode(byte_buffer{0x83, 0x01, 0x02, 0x03});
    REQUIRE(array.is_array());
    REQUIRE(array.size() == 3);
    REQUIRE(array[2].as_integer() == 3);

    auto map = cbor::decode(byte_buffer{0xa1, 0x61, 0x61, 0x01});
    REQUIRE(map.is_object());
    REQUIRE(map["a"].as_integer() == 1);
}

TEST_CASE("cbor value - floats decode from half, single and double", "[willow][cbor]") {
    REQUIRE(cbor::decode(byte_buffer{0xf9, 0x3c, 0x00}).as_float() == 1.0);
    REQUIRE(cbor::decode(byte_buffer{0xf9, 0x3e, 0x00}).as_float() == 1.5);
    REQUIRE(cbor::decode(byte_buffer{0xf9, 0x7b, 0xff}).as_float() == 65504.0);
    REQUIRE(cbor::decode(byte_buffer{0xfa, 0x47, 0xc3, 0x50, 0x00}).as_float() == 100000.0);
    REQUIRE(cbor::decode(byte_buffer{0xfb, 0x3f, 0xf1, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9a}).as_float() == 1.1);
}

TEST_CASE("cbor value - accessors reject a mismatched type", "[willow][cbor]") {
    auto integer = cbor::decode(byte_buffer{0x01});
    REQUIRE_THROWS_AS(integer.as_string(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(integer.as_bool(), rainy::foundation::exceptions::willow::willow_type_error);
    auto text = cbor::decode(byte_buffer{0x61, 0x61});
    REQUIRE_THROWS_AS(text.as_integer(), rainy::foundation::exceptions::willow::willow_type_error);
}

TEST_CASE("cbor value - a non string map key decodes as text", "[willow][cbor]") {
    auto back = cbor::decode(byte_buffer{0xa1, 0x01, 0x02});
    REQUIRE(back.is_object());
    REQUIRE(back.contains("1"));
    REQUIRE(back["1"].as_integer() == 2);
}

TEST_CASE("cbor value - a node without native state reports none", "[willow][cbor]") {
    auto back = cbor::decode(byte_buffer{0x01});
    REQUIRE_FALSE(back.has_native_value());
    REQUIRE_FALSE(back.is_tagged());
    REQUIRE_FALSE(back.is_byte_string());
}

TEST_CASE("cbor value - the facade converts from another document", "[willow][cbor]") {
    document source(document_type::object);
    source["name"] = document(std::string("rainy"));
    facade<document> converted{from_other_document, source};
    converted["name"] = document(std::string("changed"));
    REQUIRE(source["name"].as_string() == "rainy");
}
