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

TEST_CASE("cbor decode - deeply nested arrays and maps are rejected", "[willow][cbor][depth]") {
    byte_buffer arrays(300, 0x81);
    arrays.push_back(0xf6);
    REQUIRE_THROWS_AS(cbor::decode(arrays), wc::cbor_parse_error);

    byte_buffer maps(300, 0xa1);
    maps.push_back(0x61);
    maps.push_back('a');
    maps.push_back(0xf6);
    REQUIRE_THROWS_AS(cbor::decode(maps), wc::cbor_parse_error);
}

TEST_CASE("cbor decode - shallow nesting is accepted", "[willow][cbor][depth]") {
    REQUIRE_NOTHROW(cbor::decode(byte_buffer{0x81, 0x81, 0x01}));
}
