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

struct rt_phone {
    static constexpr rainy::core::container::tuple<field<"number", proto::string, 1>> protobuf_fields{};
};

struct rt_person {
    static constexpr rainy::core::container::tuple<field<"name", proto::string, 1>, field<"age", proto::int32, 2>,
                                                   field<"tags", proto::repeated<proto::string>, 3>,
                                                   field<"phone", proto::message<rt_phone>, 4>>
        protobuf_fields{};
};

struct rt_scalars {
    static constexpr rainy::core::container::tuple<
        field<"i32", proto::int32, 1>, field<"u32", proto::uint32, 2>, field<"s32", proto::sint32, 3>, field<"i64", proto::int64, 4>,
        field<"flag", proto::boolean, 5>, field<"f32", proto::fixed32, 6>, field<"sf32", proto::sfixed32, 7>,
        field<"f", proto::floating, 8>, field<"f64", proto::fixed64, 9>, field<"sf64", proto::sfixed64, 10>,
        field<"d", proto::doubling, 11>, field<"s", proto::string, 12>, field<"b", proto::bytes, 13>>
        protobuf_fields{};
};

struct rt_numbers {
    static constexpr rainy::core::container::tuple<field<"e", proto::repeated<proto::int32>, 1>> protobuf_fields{};
};

TEST_CASE("protobuf roundtrip - string record Test2 vector", "[willow][protobuf]") {
    document doc(document_type::object);
    doc["name"] = document(std::string("testing"));
    auto bytes = protobuf::encode<rt_person>(doc);
    REQUIRE(bytes.size() == 9);
    REQUIRE(bytes[0] == 0x0A);
    REQUIRE(bytes[1] == 0x07);
    document back = protobuf::decode<rt_person>(bytes.data(), bytes.size());
    REQUIRE(back["name"].as_string() == "testing");
}

TEST_CASE("protobuf roundtrip - scalars and nesting", "[willow][protobuf]") {
    document doc(document_type::object);
    doc["i32"] = document(-2);
    doc["u32"] = document(200);
    doc["s32"] = document(-500);
    doc["i64"] = document(-2);
    doc["flag"] = document(true);
    doc["f32"] = document(200);
    doc["sf32"] = document(-200);
    doc["f"] = document(25.5f);
    doc["f64"] = document(200);
    doc["sf64"] = document(-200);
    doc["d"] = document(25.4);
    doc["s"] = document(std::string("hello"));
    doc["b"] = document(std::string("bytes"));
    auto bytes = protobuf::encode<rt_scalars>(doc);
    REQUIRE(!bytes.empty());
    document back = protobuf::decode<rt_scalars>(bytes.data(), bytes.size());
    REQUIRE(back["i32"].as_integer() == -2);
    REQUIRE(back["u32"].as_integer() == 200);
    REQUIRE(back["s32"].as_integer() == -500);
    REQUIRE(back["i64"].as_integer() == -2);
    REQUIRE(back["flag"].as_bool() == true);
    REQUIRE(back["s"].as_string() == "hello");

    document person(document_type::object);
    person["name"] = document(std::string("n"));
    document phone(document_type::object);
    phone["number"] = document(std::string("1234"));
    person["phone"] = phone;
    document tags(document_type::array);
    tags.push_back(document(std::string("a")));
    tags.push_back(document(std::string("b")));
    person["tags"] = tags;
    auto pbytes = protobuf::encode<rt_person>(person);
    document pback = protobuf::decode<rt_person>(pbytes.data(), pbytes.size());
    REQUIRE(pback["phone"]["number"].as_string() == "1234");
    REQUIRE(pback["tags"].as_array().size() == 2);
}

TEST_CASE("protobuf roundtrip - packed input decodes as repeated", "[willow][protobuf]") {
    protobuf::byte_buffer packed{0x0A, 0x03, 0x01, 0x02, 0x03};
    document back = protobuf::decode<rt_numbers>(packed.data(), packed.size());
    REQUIRE(back["e"].as_array().size() == 3);
    REQUIRE(back["e"].as_array()[0].as_integer() == 1);
    REQUIRE(back["e"].as_array()[2].as_integer() == 3);
}

TEST_CASE("protobuf roundtrip - last one wins and unknown skipped", "[willow][protobuf]") {
    protobuf::byte_buffer raw{0x10, 0x01, 0x10, 0x96, 0x01, 0x78, 0x01};
    document back = protobuf::decode<rt_person>(raw.data(), raw.size());
    REQUIRE(back["age"].as_integer() == 150);
}

#endif
