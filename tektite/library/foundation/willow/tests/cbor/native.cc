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

TEST_CASE("cbor native - a byte string re-encodes with its major type", "[willow][cbor]") {
    const byte_buffer bytes{0x44, 0x01, 0x02, 0x03, 0x04};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc.is_byte_string());
    REQUIRE(cbor::encode(doc) == bytes);
}

TEST_CASE("cbor native - a text string is not flagged as a byte string", "[willow][cbor]") {
    auto doc = cbor::decode(byte_buffer{0x64, 0x49, 0x45, 0x54, 0x46});
    REQUIRE_FALSE(doc.is_byte_string());
    REQUIRE_FALSE(doc.has_native_value());
}

TEST_CASE("cbor native - tag 0 wrapping a text string round trips", "[willow][cbor]") {
    const byte_buffer bytes{0xc0, 0x64, 0x49, 0x45, 0x54, 0x46};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc.has_native_value());
    REQUIRE(doc.is_tagged());
    REQUIRE(doc.tag() == 0);
    REQUIRE(doc.as_string() == "IETF");
    REQUIRE(cbor::encode(doc) == bytes);
}

TEST_CASE("cbor native - a non zero tag round trips", "[willow][cbor]") {
    const byte_buffer bytes{0xd8, 0x2a, 0x01};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc.is_tagged());
    REQUIRE(doc.tag() == 42);
    REQUIRE(cbor::encode(doc) == bytes);
}

TEST_CASE("cbor native - tag 1 wrapping an integer round trips", "[willow][cbor]") {
    const byte_buffer bytes{0xc1, 0x1a, 0x51, 0x4b, 0x67, 0xb0};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc.tag() == 1);
    REQUIRE(cbor::encode(doc) == bytes);
}

TEST_CASE("cbor native - undefined keeps its byte", "[willow][cbor]") {
    const byte_buffer bytes{0xf7};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc.has_native_value());
    REQUIRE_FALSE(doc.is_tagged());
    REQUIRE(doc.simple() == static_cast<std::uint8_t>(simple_value::undefined));
    REQUIRE(cbor::encode(doc) == bytes);
}

TEST_CASE("cbor native - an unassigned simple value keeps its number", "[willow][cbor]") {
    const byte_buffer small{0xf0};
    auto low = cbor::decode(small);
    REQUIRE(low.simple() == 16);
    REQUIRE(cbor::encode(low) == small);

    const byte_buffer extended{0xf8, 0xff};
    auto high = cbor::decode(extended);
    REQUIRE(high.simple() == 255);
    REQUIRE(cbor::encode(high) == extended);
}

TEST_CASE("cbor native - a plain node reports no native value", "[willow][cbor]") {
    auto back = cbor::decode(byte_buffer{0x01});
    REQUIRE_FALSE(back.has_native_value());
    REQUIRE_THROWS_AS(back.tag(), wc::cbor_type_error);
    REQUIRE_THROWS_AS(back.simple(), wc::cbor_type_error);
}

TEST_CASE("cbor native - a tag nested in an array survives", "[willow][cbor]") {
    const byte_buffer bytes{0x82, 0xc1, 0x01, 0x02};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc[0].is_tagged());
    REQUIRE(doc[0].tag() == 1);
    REQUIRE(cbor::encode(doc) == bytes);
}

TEST_CASE("cbor native - a tagged value inside a map value survives", "[willow][cbor]") {
    const byte_buffer bytes{0xa1, 0x61, 0x61, 0xc1, 0x01};
    auto doc = cbor::decode(bytes);
    REQUIRE(doc["a"].is_tagged());
    REQUIRE(doc["a"].tag() == 1);
    REQUIRE(cbor::encode(doc) == bytes);
}
