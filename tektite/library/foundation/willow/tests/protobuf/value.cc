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
#include <rainy/foundation/willow/protobuf.hpp>

#if RAINY_HAS_CXX20

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::protobuf;

struct pb_single {
    static constexpr rainy::core::container::tuple<field<"a", proto::int32, 1>> protobuf_fields{};
};

struct pb_pair {
    static constexpr rainy::core::container::tuple<field<"name", proto::string, 1>, field<"age", proto::int32, 2>>
        protobuf_fields{};
};

struct pb_skipped {
    static constexpr rainy::core::container::tuple<field<"a", proto::int32, 1>, field<"b", proto::string, 5>,
                                                   field<"c", proto::int32, 3>>
        protobuf_fields{};
};

TEST_CASE("protobuf value - concept shape uses core tuple and 1-based numbers", "[willow][protobuf]") {
    STATIC_REQUIRE(protobuf_message<pb_single>);
    STATIC_REQUIRE(protobuf_message<pb_pair>);
    STATIC_REQUIRE(field_count_v<pb_single> == 1);
    STATIC_REQUIRE(field_count_v<pb_pair> == 2);
    STATIC_REQUIRE(field_number_v<pb_pair, 0> == 1);
    STATIC_REQUIRE(field_number_v<pb_pair, 1> == 2);
    STATIC_REQUIRE(field_number_v<pb_skipped, 0> == 1);
    STATIC_REQUIRE(field_number_v<pb_skipped, 1> == 5);
    STATIC_REQUIRE(field_number_v<pb_skipped, 2> == 3);
    STATIC_REQUIRE(valid_field_numbers_v<pb_skipped>);
    STATIC_REQUIRE(wire_of_v<proto::int32> == wire_type::varint);
    STATIC_REQUIRE(wire_of_v<proto::string> == wire_type::len);
    STATIC_REQUIRE(wire_of_v<proto::floating> == wire_type::i32);
    STATIC_REQUIRE(wire_of_v<proto::doubling> == wire_type::i64);
}

TEST_CASE("protobuf value - facade wraps object document", "[willow][protobuf]") {
    protobuf::facade<pb_pair, document> msg;
    REQUIRE(msg.is_null());
    msg = protobuf::facade<pb_pair, document>(document(document_type::object));
    REQUIRE(msg.is_object());
    REQUIRE(msg.empty());
    msg["name"] = document(std::string("hi"));
    REQUIRE(msg["name"].as_string() == "hi");
}

TEST_CASE("protobuf value - Test1 vector from encoding doc", "[willow][protobuf]") {
    document doc(document_type::object);
    doc["a"] = document(150);
    auto bytes = protobuf::encode<pb_single>(doc);
    REQUIRE(bytes.size() == 3);
    REQUIRE(bytes[0] == 0x08);
    REQUIRE(bytes[1] == 0x96);
    REQUIRE(bytes[2] == 0x01);
}

TEST_CASE("protobuf value - empty and unknown keys encode to nothing", "[willow][protobuf]") {
    document empty(document_type::object);
    REQUIRE(protobuf::encode<pb_single>(empty).empty());
    document foreign(document_type::object);
    foreign["zzz"] = document(1);
    REQUIRE(protobuf::encode<pb_single>(foreign).empty());
}

TEST_CASE("protobuf value - skipped number encodes with explicit tag", "[willow][protobuf]") {
    document doc(document_type::object);
    doc["b"] = document(std::string("hi"));
    auto bytes = protobuf::encode<pb_skipped>(doc);
    REQUIRE(bytes.size() == 4);
    REQUIRE(bytes[0] == 0x2A);
    REQUIRE(bytes[1] == 0x02);
    document back = protobuf::decode<pb_skipped>(bytes.data(), bytes.size());
    REQUIRE(back["b"].as_string() == "hi");
}

#endif
