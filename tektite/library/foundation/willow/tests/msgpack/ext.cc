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

namespace wmp = rainy::foundation::exceptions::willow::msgpack;

namespace {
    msgpack_options with(const feature flags) {
        msgpack_options options;
        options.features = flags;
        return options;
    }

    void push_be32(byte_buffer &out, const std::uint32_t value) {
        out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xffu));
        out.push_back(static_cast<std::uint8_t>(value & 0xffu));
    }

    void push_be64(byte_buffer &out, const std::uint64_t value) {
        for (int i = 7; i >= 0; --i) {
            out.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xffu));
        }
    }
}

TEST_CASE("msgpack ext - disabled features reject extension input", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd4, 0x05, 0x42}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xc7, 0x02, 0x05, 0x42, 0x43}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd6, 0xff, 0x00, 0x00, 0x00, 0x01}), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack ext - fixext sizes decode into node state", "[willow][msgpack]") {
    auto one = unpack(byte_buffer{0xd4, 0x05, 0x42}, with(feature::ext));
    REQUIRE(one.has_ext());
    REQUIRE(one.ext_type() == 5);
    REQUIRE(one.ext_data() == byte_buffer{0x42});

    auto two = unpack(byte_buffer{0xd5, 0x07, 0x01, 0x02}, with(feature::ext));
    REQUIRE(two.ext_data() == byte_buffer{0x01, 0x02});

    byte_buffer fixext4{0xd6, 0x03, 0x01, 0x02, 0x03, 0x04};
    auto four = unpack(fixext4, with(feature::ext));
    REQUIRE(four.ext_data().size() == 4);

    byte_buffer fixext8{0xd7, 0x03};
    fixext8.insert(fixext8.end(), 8, 0x09);
    REQUIRE(unpack(fixext8, with(feature::ext)).ext_data().size() == 8);

    byte_buffer fixext16{0xd8, 0x03};
    fixext16.insert(fixext16.end(), 16, 0x09);
    REQUIRE(unpack(fixext16, with(feature::ext)).ext_data().size() == 16);
}

TEST_CASE("msgpack ext - variable length ext decodes and roundtrips", "[willow][msgpack]") {
    byte_buffer ext8{0xc7, 0x03, 0x09, 0xaa, 0xbb, 0xcc};
    auto node = unpack(ext8, with(feature::ext));
    REQUIRE(node.ext_type() == 9);
    REQUIRE(node.ext_data() == byte_buffer{0xaa, 0xbb, 0xcc});
    REQUIRE(pack(node, with(feature::ext)) == ext8);

    byte_buffer ext16{0xc8, 0x00, 0x02, 0x09, 0xaa, 0xbb};
    auto node16 = unpack(ext16, with(feature::ext));
    REQUIRE(node16.ext_data().size() == 2);

    byte_buffer ext32{0xc9, 0x00, 0x00, 0x00, 0x02, 0x09, 0xaa, 0xbb};
    auto node32 = unpack(ext32, with(feature::ext));
    REQUIRE(node32.ext_data().size() == 2);
}

TEST_CASE("msgpack ext - fixext roundtrips to identical bytes", "[willow][msgpack]") {
    byte_buffer fixext2{0xd5, 0x11, 0x01, 0x02};
    auto node = unpack(fixext2, with(feature::ext));
    REQUIRE(pack(node, with(feature::ext)) == fixext2);
}

TEST_CASE("msgpack ext - feature gates are independent", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd4, 0x05, 0x42}, with(feature::timestamp)), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd6, 0xff, 0x00, 0x00, 0x00, 0x01}, with(feature::ext)), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack ext - timestamp32 decodes seconds", "[willow][msgpack]") {
    auto node = unpack(byte_buffer{0xd6, 0xff, 0x00, 0x00, 0x00, 0x01}, with(feature::timestamp));
    REQUIRE(node.is_timestamp());
    REQUIRE(node.timestamp_seconds() == 1);
    REQUIRE(node.timestamp_nanoseconds() == 0);
}

TEST_CASE("msgpack ext - timestamp64 decodes seconds and nanoseconds", "[willow][msgpack]") {
    const std::uint64_t data64 = (static_cast<std::uint64_t>(500000000) << 34) | 1ULL;
    byte_buffer bytes{0xd7, 0xff};
    push_be64(bytes, data64);
    auto node = unpack(bytes, with(feature::timestamp));
    REQUIRE(node.is_timestamp());
    REQUIRE(node.timestamp_seconds() == 1);
    REQUIRE(node.timestamp_nanoseconds() == 500000000);
    REQUIRE(pack(node, with(feature::timestamp)) == bytes);
}

TEST_CASE("msgpack ext - timestamp96 decodes signed seconds", "[willow][msgpack]") {
    byte_buffer bytes{0xc7, 0x0c, 0xff};
    push_be32(bytes, 500000000);
    push_be64(bytes, static_cast<std::uint64_t>(-1LL));
    auto node = unpack(bytes, with(feature::timestamp));
    REQUIRE(node.is_timestamp());
    REQUIRE(node.timestamp_seconds() == -1);
    REQUIRE(node.timestamp_nanoseconds() == 500000000);
    REQUIRE(pack(node, with(feature::timestamp)) == bytes);
}

TEST_CASE("msgpack ext - packing ext state without the feature throws", "[willow][msgpack]") {
    auto node = unpack(byte_buffer{0xd4, 0x05, 0x42}, with(feature::ext));
    REQUIRE_THROWS_AS(pack(node), wmp::msgpack_serialize_error);

    auto stamp = unpack(byte_buffer{0xd6, 0xff, 0x00, 0x00, 0x00, 0x01}, with(feature::timestamp));
    REQUIRE_THROWS_AS(pack(stamp), wmp::msgpack_serialize_error);
}

TEST_CASE("msgpack ext - nodes nested in containers keep their state", "[willow][msgpack]") {
    byte_buffer bytes{0x81, 0xa1, 0x78, 0xd4, 0x05, 0x42};
    auto node = unpack(bytes, with(feature::ext));
    REQUIRE(node.is_object());
    REQUIRE(node["x"].has_ext());
    REQUIRE(node["x"].ext_type() == 5);
    REQUIRE(pack(node, with(feature::ext)) == bytes);
}
