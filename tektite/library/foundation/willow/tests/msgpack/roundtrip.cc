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

TEST_CASE("msgpack roundtrip - integer formats pick minimal representation", "[willow][msgpack]") {
    REQUIRE(pack(document(0)) == byte_buffer{0x00});
    REQUIRE(pack(document(127)) == byte_buffer{0x7f});
    REQUIRE(pack(document(128)) == byte_buffer{0xcc, 0x80});
    REQUIRE(pack(document(255)) == byte_buffer{0xcc, 0xff});
    REQUIRE(pack(document(256)) == byte_buffer{0xcd, 0x01, 0x00});
    REQUIRE(pack(document(65535)) == byte_buffer{0xcd, 0xff, 0xff});
    REQUIRE(pack(document(65536)) == byte_buffer{0xce, 0x00, 0x01, 0x00, 0x00});
    REQUIRE(pack(document(-1)) == byte_buffer{0xff});
    REQUIRE(pack(document(-32)) == byte_buffer{0xe0});
    REQUIRE(pack(document(-33)) == byte_buffer{0xd0, 0xdf});
    REQUIRE(pack(document(-128)) == byte_buffer{0xd0, 0x80});
    REQUIRE(pack(document(-129)) == byte_buffer{0xd1, 0xff, 0x7f});
}

TEST_CASE("msgpack roundtrip - integers survive a decode", "[willow][msgpack]") {
    const int values[] = {0, 1, 127, 128, 255, 256, 65535, 65536, -1, -32, -33, -128, -129, -32768, -32769,
                          2147483647, -2147483647 - 1};
    for (const int value: values) {
        auto back = unpack(pack(document(value)));
        REQUIRE(back.is_integer());
        REQUIRE(back.as_integer() == value);
    }
}

TEST_CASE("msgpack roundtrip - 64-bit document keeps full range", "[willow][msgpack]") {
    const std::int64_t values[] = {9223372036854775807LL, -9223372036854775807LL - 1, 4294967296LL, -4294967296LL};
    for (const std::int64_t value: values) {
        document64 source(value);
        auto back = unpack<document64>(pack(source));
        REQUIRE(back.as_integer() == value);
    }
}

TEST_CASE("msgpack roundtrip - float picks float32 when lossless", "[willow][msgpack]") {
    REQUIRE(pack(document(1.5)) == byte_buffer{0xca, 0x3f, 0xc0, 0x00, 0x00});
    REQUIRE(pack(document(0.1))[0] == 0xcb);
    REQUIRE(unpack(pack(document(1.5))).as_float() == 1.5);
    REQUIRE(unpack(pack(document(0.1))).as_float() == 0.1);
    REQUIRE(unpack(pack(document(-2.25))).as_float() == -2.25);
}

TEST_CASE("msgpack roundtrip - nil and boolean", "[willow][msgpack]") {
    REQUIRE(pack(document(document_type::null)) == byte_buffer{0xc0});
    REQUIRE(pack(document(true)) == byte_buffer{0xc3});
    REQUIRE(pack(document(false)) == byte_buffer{0xc2});
    REQUIRE(unpack(byte_buffer{0xc0}).is_null());
    REQUIRE(unpack(byte_buffer{0xc3}).as_bool());
    REQUIRE_FALSE(unpack(byte_buffer{0xc2}).as_bool());
}

TEST_CASE("msgpack roundtrip - string formats pick minimal representation", "[willow][msgpack]") {
    REQUIRE(pack(document(std::string(""))) == byte_buffer{0xa0});
    REQUIRE(pack(document(std::string("hi"))) == byte_buffer{0xa2, 0x68, 0x69});
    const std::string text(31, 'a');
    REQUIRE(pack(document(text))[0] == 0xbf);
    const std::string text32(32, 'a');
    REQUIRE(pack(document(text32))[0] == 0xd9);
    REQUIRE(unpack(pack(document(text32))).as_string() == text32);
}

TEST_CASE("msgpack roundtrip - array and map formats pick minimal representation", "[willow][msgpack]") {
    document empty_array(document_type::array);
    REQUIRE(pack(empty_array) == byte_buffer{0x90});

    document small_array(document_type::array);
    for (int i = 0; i < 15; ++i) {
        small_array.push_back(document(i));
    }
    REQUIRE(pack(small_array)[0] == 0x9f);

    document wide_array(document_type::array);
    for (int i = 0; i < 16; ++i) {
        wide_array.push_back(document(i));
    }
    const auto wide_bytes = pack(wide_array);
    REQUIRE(wide_bytes[0] == 0xdc);
    REQUIRE(wide_bytes[1] == 0x00);
    REQUIRE(wide_bytes[2] == 0x10);

    document empty_map(document_type::object);
    REQUIRE(pack(empty_map) == byte_buffer{0x80});

    document small_map(document_type::object);
    for (int i = 0; i < 15; ++i) {
        small_map[std::to_string(i)] = document(i);
    }
    REQUIRE(pack(small_map)[0] == 0x8f);
}

TEST_CASE("msgpack roundtrip - nested documents compare equal", "[willow][msgpack]") {
    document source(document_type::object);
    source["name"] = document(std::string("rainy"));
    source["count"] = document(3);
    source["ratio"] = document(0.5);
    source["flag"] = document(true);
    source["missing"] = document(document_type::null);
    document tags(document_type::array);
    tags.push_back(document(std::string("alpha")));
    tags.push_back(document(7));
    source["tags"] = tags;
    document nested(document_type::object);
    nested["deep"] = document(std::string("value"));
    source["child"] = nested;

    auto back = unpack(pack(source));
    facade<document> expected{from_other_document, source};
    REQUIRE(back.is_object());
    REQUIRE(back == expected);
    REQUIRE(back["child"]["deep"].as_string() == "value");
    REQUIRE(back["tags"][1].as_integer() == 7);
}

TEST_CASE("msgpack roundtrip - non-string map keys decode as text", "[willow][msgpack]") {
    auto back = unpack(byte_buffer{0x81, 0x01, 0x02});
    REQUIRE(back.is_object());
    REQUIRE(back.contains("1"));
    REQUIRE(back["1"].as_integer() == 2);
}
