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

TEST_CASE("cbor roundtrip - integers use the shortest form", "[willow][cbor]") {
    REQUIRE(cbor::encode(document(0)) == byte_buffer{0x00});
    REQUIRE(cbor::encode(document(10)) == byte_buffer{0x0a});
    REQUIRE(cbor::encode(document(23)) == byte_buffer{0x17});
    REQUIRE(cbor::encode(document(24)) == byte_buffer{0x18, 0x18});
    REQUIRE(cbor::encode(document(1000)) == byte_buffer{0x19, 0x03, 0xe8});
    REQUIRE(cbor::encode(document(-1)) == byte_buffer{0x20});
    REQUIRE(cbor::encode(document(-1000)) == byte_buffer{0x39, 0x03, 0xe7});
}

TEST_CASE("cbor roundtrip - simple values use their canonical bytes", "[willow][cbor]") {
    REQUIRE(cbor::encode(document(false)) == byte_buffer{0xf4});
    REQUIRE(cbor::encode(document(true)) == byte_buffer{0xf5});
    REQUIRE(cbor::encode(document(document_type::null)) == byte_buffer{0xf6});
}

TEST_CASE("cbor roundtrip - strings use the shortest length form", "[willow][cbor]") {
    REQUIRE(cbor::encode(document(std::string(""))) == byte_buffer{0x60});
    REQUIRE(cbor::encode(document(std::string("a"))) == byte_buffer{0x61, 0x61});
    REQUIRE(cbor::encode(document(std::string("IETF"))) == byte_buffer{0x64, 0x49, 0x45, 0x54, 0x46});
    const std::string text(24, 'a');
    REQUIRE(cbor::encode(document(text))[0] == 0x78);
    const std::string text32(256, 'a');
    REQUIRE(cbor::encode(document(text32))[0] == 0x79);
}

TEST_CASE("cbor roundtrip - arrays and maps use the shortest length form", "[willow][cbor]") {
    document empty_array(document_type::array);
    REQUIRE(cbor::encode(empty_array) == byte_buffer{0x80});

    document array(document_type::array);
    array.push_back(document(1));
    array.push_back(document(2));
    array.push_back(document(3));
    REQUIRE(cbor::encode(array) == byte_buffer{0x83, 0x01, 0x02, 0x03});

    document empty_map(document_type::object);
    REQUIRE(cbor::encode(empty_map) == byte_buffer{0xa0});

    document map(document_type::object);
    map["a"] = document(1);
    REQUIRE(cbor::encode(map) == byte_buffer{0xa1, 0x61, 0x61, 0x01});
}

TEST_CASE("cbor roundtrip - floats use the narrowest exact form", "[willow][cbor]") {
    REQUIRE(cbor::encode(document(0.0)) == byte_buffer{0xf9, 0x00, 0x00});
    REQUIRE(cbor::encode(document(1.0)) == byte_buffer{0xf9, 0x3c, 0x00});
    REQUIRE(cbor::encode(document(1.5)) == byte_buffer{0xf9, 0x3e, 0x00});
    REQUIRE(cbor::encode(document(65504.0)) == byte_buffer{0xf9, 0x7b, 0xff});
    REQUIRE(cbor::encode(document(100000.0)) == byte_buffer{0xfa, 0x47, 0xc3, 0x50, 0x00});
    REQUIRE(cbor::encode(document(1.1)) ==
            byte_buffer{0xfb, 0x3f, 0xf1, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9a});
}

TEST_CASE("cbor roundtrip - scalars survive a decode", "[willow][cbor]") {
    const int values[] = {0, 1, 23, 24, 255, 256, 65535, 65536, -1, -24, -25, -256, -257, 2147483647};
    for (const int value: values) {
        auto back = cbor::decode(cbor::encode(document(value)));
        REQUIRE(back.is_integer());
        REQUIRE(back.as_integer() == value);
    }
}

TEST_CASE("cbor roundtrip - a 64-bit document keeps the full range", "[willow][cbor]") {
    const std::int64_t values[] = {9223372036854775807LL, -9223372036854775807LL - 1, 4294967296LL};
    for (const std::int64_t value: values) {
        auto back = cbor::decode<document64>(cbor::encode(document64(value)));
        REQUIRE(back.as_integer() == value);
    }
}

TEST_CASE("cbor roundtrip - nested documents re-encode identically", "[willow][cbor]") {
    document tags(document_type::array);
    tags.push_back(document(std::string("alpha")));
    tags.push_back(document(7));
    document nested(document_type::object);
    nested["deep"] = document(std::string("value"));
    document src(document_type::object);
    src["name"] = document(std::string("rainy"));
    src["count"] = document(3);
    src["ratio"] = document(0.5);
    src["flag"] = document(true);
    src["missing"] = document(document_type::null);
    src["tags"] = tags;
    src["child"] = nested;

    const auto bytes = cbor::encode(src);
    REQUIRE(cbor::encode(cbor::decode(bytes)) == bytes);
    auto back = cbor::decode(bytes);
    REQUIRE(back["child"]["deep"].as_string() == "value");
    REQUIRE(back["tags"][1].as_integer() == 7);
}

TEST_CASE("cbor roundtrip - an indefinite array decodes", "[willow][cbor]") {
    auto back = cbor::decode(byte_buffer{0x9f, 0x01, 0x02, 0x03, 0xff});
    REQUIRE(back.is_array());
    REQUIRE(back.size() == 3);
}

TEST_CASE("cbor roundtrip - an indefinite map decodes", "[willow][cbor]") {
    auto back = cbor::decode(byte_buffer{0xbf, 0x61, 0x61, 0x01, 0xff});
    REQUIRE(back.is_object());
    REQUIRE(back["a"].as_integer() == 1);
}

TEST_CASE("cbor roundtrip - an indefinite string decodes and re-encodes definite", "[willow][cbor]") {
    auto back = cbor::decode(byte_buffer{0x7f, 0x61, 0x61, 0x61, 0x62, 0xff});
    REQUIRE(back.as_string() == "ab");
    REQUIRE(cbor::encode(back) == byte_buffer{0x62, 0x61, 0x62});
}
