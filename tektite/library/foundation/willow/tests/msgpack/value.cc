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
#include <rainy/foundation/willow/msgpack.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::msgpack;

namespace wex = rainy::foundation::exceptions::willow;
namespace wmp = rainy::foundation::exceptions::willow::msgpack;

TEST_CASE("msgpack value - decoded scalars expose their type", "[willow][msgpack]") {
    REQUIRE(unpack(byte_buffer{0xc0}).is_null());
    REQUIRE(unpack(byte_buffer{0xc3}).is_bool());
    REQUIRE(unpack(byte_buffer{0x2a}).is_integer());
    REQUIRE(unpack(byte_buffer{0xca, 0x3f, 0x80, 0x00, 0x00}).is_float());
    REQUIRE(unpack(byte_buffer{0xa1, 0x78}).is_string());
    REQUIRE(unpack(byte_buffer{0x90}).is_array());
    REQUIRE(unpack(byte_buffer{0x80}).is_object());
}

TEST_CASE("msgpack value - accessors return the decoded payload", "[willow][msgpack]") {
    REQUIRE(unpack(byte_buffer{0x2a}).as_integer() == 42);
    REQUIRE(unpack(byte_buffer{0xca, 0x3f, 0x80, 0x00, 0x00}).as_float() == 1.0);
    REQUIRE(unpack(byte_buffer{0xa3, 0x61, 0x62, 0x63}).as_string() == "abc");
    REQUIRE(unpack(byte_buffer{0x93, 0x01, 0x02, 0x03}).as_array().size() == 3);
}

TEST_CASE("msgpack value - accessors reject mismatched types", "[willow][msgpack]") {
    auto integer = unpack(byte_buffer{0x01});
    REQUIRE_THROWS_AS(integer.as_string(), wex::willow_type_error);
    REQUIRE_THROWS_AS(integer.as_bool(), wex::willow_type_error);
    auto text = unpack(byte_buffer{0xa1, 0x78});
    REQUIRE_THROWS_AS(text.as_integer(), wex::willow_type_error);
}

TEST_CASE("msgpack value - containers support lookup and size", "[willow][msgpack]") {
    document source(document_type::object);
    source["a"] = document(1);
    source["b"] = document(2);
    auto back = unpack(pack(source));
    REQUIRE(back.size() == 2);
    REQUIRE_FALSE(back.empty());
    REQUIRE(back.contains("a"));
    REQUIRE(back.count("a") == 1);
    REQUIRE(back["b"].as_integer() == 2);

    document empty(document_type::object);
    REQUIRE(unpack(pack(empty)).empty());
}

TEST_CASE("msgpack value - nodes without extension state report no ext", "[willow][msgpack]") {
    auto back = unpack(byte_buffer{0x01});
    REQUIRE_FALSE(back.has_ext());
    REQUIRE_FALSE(back.is_timestamp());
    REQUIRE_THROWS_AS(back.ext_type(), wmp::msgpack_type_error);
    REQUIRE_THROWS_AS(back.ext_data(), wmp::msgpack_type_error);
}

TEST_CASE("msgpack value - facade conversion deep copies another document", "[willow][msgpack]") {
    document source(document_type::object);
    source["name"] = document(std::string("rainy"));
    source["items"] = document(document_type::array);
    source["items"].push_back(document(1));
    facade<document> converted{from_other_document, source};
    converted["name"] = document(std::string("changed"));
    REQUIRE(source["name"].as_string() == "rainy");
    REQUIRE(converted["items"][0].as_integer() == 1);
}
