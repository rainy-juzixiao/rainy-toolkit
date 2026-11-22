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

namespace {
    /**
     * Builds a one element document whose element type is not one the floor holds, so the element body is
     * carried on the node verbatim.
     */
    byte_buffer wrap(const std::uint8_t type_byte, const byte_buffer &body) {
        byte_buffer out{};
        const std::uint32_t length = static_cast<std::uint32_t>(4 + 1 + 2 + body.size() + 1);
        out.push_back(static_cast<std::uint8_t>(length & 0xffu));
        out.push_back(static_cast<std::uint8_t>((length >> 8) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((length >> 16) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((length >> 24) & 0xffu));
        out.push_back(type_byte);
        out.push_back(0x61);
        out.push_back(0x00);
        out.insert(out.end(), body.begin(), body.end());
        out.push_back(0x00);
        return out;
    }
}

TEST_CASE("bson native - an element the floor cannot hold keeps its type byte", "[willow][bson]") {
    const byte_buffer object_id{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    const auto bytes = wrap(0x07, object_id);
    auto doc = bson::decode(bytes);

    REQUIRE(doc["a"].has_native_element());
    REQUIRE(doc["a"].native_element() == element_type::object_id);
    REQUIRE(doc["a"].native_payload() == object_id);
}

TEST_CASE("bson native - a native element re-encodes byte for byte", "[willow][bson]") {
    const byte_buffer object_id{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    const auto bytes = wrap(0x07, object_id);
    REQUIRE(bson::encode(bson::decode(bytes)) == bytes);
}

TEST_CASE("bson native - a datetime keeps its eight bytes", "[willow][bson]") {
    const byte_buffer millis{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    const auto bytes = wrap(0x09, millis);
    auto doc = bson::decode(bytes);

    REQUIRE(doc["a"].native_element() == element_type::datetime);
    REQUIRE(doc["a"].native_payload() == millis);
    REQUIRE(bson::encode(doc) == bytes);
}

TEST_CASE("bson native - a binary keeps its subtype and payload", "[willow][bson]") {
    const byte_buffer body{0x03, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xbb, 0xcc};
    const auto bytes = wrap(0x05, body);
    auto doc = bson::decode(bytes);

    REQUIRE(doc["a"].native_element() == element_type::binary);
    REQUIRE(doc["a"].native_payload() == body);
    REQUIRE(bson::encode(doc) == bytes);
}

TEST_CASE("bson native - a decimal128 keeps its sixteen bytes", "[willow][bson]") {
    byte_buffer body{};
    for (std::uint8_t i = 0; i < 16; ++i) {
        body.push_back(i);
    }
    const auto bytes = wrap(0x13, body);
    auto doc = bson::decode(bytes);

    REQUIRE(doc["a"].native_element() == element_type::decimal128);
    REQUIRE(doc["a"].native_payload() == body);
    REQUIRE(bson::encode(doc) == bytes);
}

TEST_CASE("bson native - a min key and a max key carry no body", "[willow][bson]") {
    const auto min_key = wrap(0xff, {});
    const auto max_key = wrap(0x7f, {});

    REQUIRE(bson::decode(min_key)["a"].native_element() == element_type::min_key);
    REQUIRE(bson::decode(max_key)["a"].native_element() == element_type::max_key);
    REQUIRE(bson::encode(bson::decode(min_key)) == min_key);
    REQUIRE(bson::encode(bson::decode(max_key)) == max_key);
}

TEST_CASE("bson native - an undefined value carries no body", "[willow][bson]") {
    const auto bytes = wrap(0x06, {});
    REQUIRE(bson::decode(bytes)["a"].native_element() == element_type::undefined);
    REQUIRE(bson::encode(bson::decode(bytes)) == bytes);
}

TEST_CASE("bson native - a timestamp keeps its eight bytes", "[willow][bson]") {
    const byte_buffer body{0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
    const auto bytes = wrap(0x11, body);
    REQUIRE(bson::decode(bytes)["a"].native_element() == element_type::timestamp);
    REQUIRE(bson::encode(bson::decode(bytes)) == bytes);
}

TEST_CASE("bson native - a plain node reports no native element", "[willow][bson]") {
    document src(document_type::object);
    src["a"] = document(1);
    auto back = bson::decode(bson::encode(src));
    REQUIRE_FALSE(back["a"].has_native_element());
    REQUIRE_THROWS_AS(back["a"].native_element(), wb::bson_type_error);
    REQUIRE_THROWS_AS(back["a"].native_payload(), wb::bson_type_error);
}

TEST_CASE("bson native - a native element nested in an array survives", "[willow][bson]") {
    const byte_buffer body{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
    byte_buffer array_body{0x10, 0x00, 0x00, 0x00, 0x09, 0x30, 0x00};
    array_body.insert(array_body.end(), body.begin(), body.end());
    array_body.push_back(0x00);
    const auto bytes = wrap(0x04, array_body);
    auto doc = bson::decode(bytes);

    REQUIRE(doc["a"].is_array());
    REQUIRE(doc["a"][0].native_element() == element_type::datetime);
    REQUIRE(bson::encode(doc) == bytes);
}
