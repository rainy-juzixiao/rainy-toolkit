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

namespace wc = rainy::foundation::exceptions::willow::cbor;

TEST_CASE("cbor error - an empty or truncated input is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x18}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x19, 0x01}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x1a, 0x01, 0x02, 0x03}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x1b, 0x01}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - trailing bytes are rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x01, 0x02}), wc::cbor_parse_error);
    REQUIRE_NOTHROW(cbor::decode(byte_buffer{0x01}));
}

TEST_CASE("cbor error - a truncated string is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x64, 0x49, 0x45}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x44, 0x01, 0x02}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - a truncated array or map is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x83, 0x01, 0x02}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0xa1, 0x61, 0x61}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - an unterminated indefinite item is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x9f, 0x01}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0xbf, 0x61, 0x61, 0x01}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x7f, 0x61, 0x61}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - a break byte outside an indefinite item is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0xff}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - an integer with the indefinite form is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x1f}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x3f}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - a string chunk of the wrong type is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x7f, 0x41, 0x61, 0xff}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x5f, 0x61, 0x61, 0xff}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - a nested indefinite chunk is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x7f, 0x7f, 0x61, 0x61, 0xff, 0xff}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - an integer outside the document range is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}), wc::cbor_parse_error);
    REQUIRE_NOTHROW(cbor::decode<document64>(byte_buffer{0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}));
}

TEST_CASE("cbor error - a composite map key is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0xa1, 0x80, 0x01}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode(byte_buffer{0xa1, 0xa0, 0x01}), wc::cbor_parse_error);
}

TEST_CASE("cbor error - a null pointer with a nonzero size is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode(static_cast<const std::uint8_t *>(nullptr), 4), wc::cbor_parse_error);
}
