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
    void append_le32(byte_buffer &out, const std::uint32_t value) {
        out.push_back(static_cast<std::uint8_t>(value & 0xff));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
        out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xff));
        out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xff));
    }

    byte_buffer wrap_document(const byte_buffer &inner) {
        byte_buffer out;
        append_le32(out, static_cast<std::uint32_t>(inner.size() + 8));
        out.push_back(0x03);
        out.push_back('a');
        out.push_back(0x00);
        for (const std::uint8_t byte: inner) {
            out.push_back(byte);
        }
        out.push_back(0x00);
        return out;
    }
}

TEST_CASE("bson decode - deeply nested documents are rejected", "[willow][bson][depth]") {
    byte_buffer payload{0x05, 0x00, 0x00, 0x00, 0x00};
    for (std::size_t i = 0; i < 300; ++i) {
        payload = wrap_document(payload);
    }
    REQUIRE_THROWS_AS(bson::decode(payload), wb::bson_parse_error);
}

TEST_CASE("bson decode - shallow nesting is accepted", "[willow][bson][depth]") {
    REQUIRE_NOTHROW(bson::decode(wrap_document(byte_buffer{0x05, 0x00, 0x00, 0x00, 0x00})));
}
