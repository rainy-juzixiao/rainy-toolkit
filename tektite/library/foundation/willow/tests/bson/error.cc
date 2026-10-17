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

namespace wb = rainy::foundation::exceptions::willow::bson;

TEST_CASE("bson error - a truncated document is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{}), wb::bson_parse_error);
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x05}), wb::bson_parse_error);
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x05, 0x00, 0x00, 0x00}), wb::bson_parse_error);
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x05, 0x00, 0x00, 0x00, 0x00, 0x00}), wb::bson_parse_error);
}

TEST_CASE("bson error - a document without a terminator is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x08, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0x01}), wb::bson_parse_error);
}

TEST_CASE("bson error - a length shorter than the content is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x05, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}),
                      wb::bson_parse_error);
}

TEST_CASE("bson error - a truncated key is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x0c, 0x00, 0x00, 0x00, 0x10, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00, 0x00}),
                      wb::bson_parse_error);
}

TEST_CASE("bson error - a string without its terminator is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x10, 0x00, 0x00, 0x00, 0x02, 0x61, 0x00, 0x02, 0x00, 0x00, 0x00, 0x78, 0x79, 0x00, 0x00}),
                      wb::bson_parse_error);
}

TEST_CASE("bson error - a negative string length is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x0f, 0x00, 0x00, 0x00, 0x02, 0x61, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}),
                      wb::bson_parse_error);
}

TEST_CASE("bson error - a boolean other than 0 or 1 is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x09, 0x00, 0x00, 0x00, 0x08, 0x62, 0x00, 0x02, 0x00}), wb::bson_parse_error);
}

TEST_CASE("bson error - an unknown element type is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x08, 0x00, 0x00, 0x00, 0x7e, 0x62, 0x00, 0x00}), wb::bson_parse_error);
}

TEST_CASE("bson error - trailing bytes after the document are rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(byte_buffer{0x05, 0x00, 0x00, 0x00, 0x00, 0x00}), wb::bson_parse_error);
}

TEST_CASE("bson error - a null pointer with a nonzero size is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode(static_cast<const std::uint8_t *>(nullptr), 4), wb::bson_parse_error);
}

TEST_CASE("bson error - a non object document cannot be encoded", "[willow][bson]") {
    document scalar(document_type::array);
    REQUIRE_THROWS_AS(bson::encode(scalar), wb::bson_serialize_error);
}

TEST_CASE("bson error - a key holding a null byte is rejected", "[willow][bson]") {
    document src(document_type::object);
    src[document::string_type("a\0b", 3)] = document(1);
    REQUIRE_THROWS_AS(bson::encode(src), wb::bson_serialize_error);
}

TEST_CASE("bson error - a string holding a null byte is rejected", "[willow][bson]") {
    document src(document_type::object);
    src["a"] = document(document::string_type("x\0y", 3));
    REQUIRE_THROWS_AS(bson::encode(src), wb::bson_serialize_error);
}
