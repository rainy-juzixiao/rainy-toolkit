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
}

TEST_CASE("msgpack error - empty and truncated inputs throw", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd9}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xa5, 0x61}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd2, 0x00}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xca, 0x00}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xdc, 0x00, 0x05, 0x01}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0x81, 0xa1, 0x78}), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack error - the reserved format byte is rejected", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xc1}), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack error - trailing bytes are rejected", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0x01, 0x02}), wmp::msgpack_parse_error);
    REQUIRE_NOTHROW(unpack(byte_buffer{0x01}));
}

TEST_CASE("msgpack error - integers outside the document range throw", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xcf, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd3, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xce, 0x80, 0x00, 0x00, 0x00}), wmp::msgpack_parse_error);
    REQUIRE_NOTHROW(unpack<document64>(byte_buffer{0xcf, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00}));
}

TEST_CASE("msgpack error - malformed extension payloads throw", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd4}, with(feature::ext)), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd4, 0x05}, with(feature::ext)), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xc7, 0x05, 0x01, 0x02}, with(feature::ext)), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xd6, 0xff, 0x00, 0x00}, with(feature::timestamp)), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack error - composite map keys are rejected", "[willow][msgpack]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0x81, 0x90, 0x01}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0x81, 0x80, 0x01}), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack error - null pointer with nonzero size throws", "[willow][msgpack]") {
    const std::uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    REQUIRE_THROWS_AS(unpack(payload, 4), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(static_cast<const std::uint8_t *>(nullptr), 4), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(static_cast<const std::uint8_t *>(nullptr), 0), wmp::msgpack_parse_error);
}
