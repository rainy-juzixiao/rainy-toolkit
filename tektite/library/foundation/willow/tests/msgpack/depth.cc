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

TEST_CASE("msgpack decode - deeply nested arrays and maps are rejected", "[willow][msgpack][depth]") {
    byte_buffer arrays(300, 0x91);
    arrays.push_back(0xc0);
    REQUIRE_THROWS_AS(unpack(arrays), wmp::msgpack_parse_error);

    byte_buffer maps(300, 0x81);
    maps.push_back(0xa1);
    maps.push_back('a');
    maps.push_back(0xc0);
    REQUIRE_THROWS_AS(unpack(maps), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack decode - declared lengths beyond the input are rejected", "[willow][msgpack][depth]") {
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xdd, 0xff, 0xff, 0xff, 0xff}), wmp::msgpack_parse_error);
    REQUIRE_THROWS_AS(unpack(byte_buffer{0xdf, 0xff, 0xff, 0xff, 0xff}), wmp::msgpack_parse_error);
}

TEST_CASE("msgpack decode - shallow nesting is accepted", "[willow][msgpack][depth]") {
    REQUIRE_NOTHROW(unpack(byte_buffer{0x91, 0x91, 0x01}));
}
