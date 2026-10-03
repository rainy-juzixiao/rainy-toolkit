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

#include <cstdint>
#include <string>
#include <vector>

#if RAINY_HAS_CXX20

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::protobuf;

struct d_phone {
    std::string number{};

    static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &d_phone::number>>
        protobuf_fields{};
};

struct d_person {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    d_phone phone{};

    static constexpr rainy::core::container::tuple<member_field<"name", proto::string, 1, &d_person::name>,
                                                   member_field<"age", proto::int32, 2, &d_person::age>,
                                                   member_field<"tags", proto::repeated<proto::string>, 3, &d_person::tags>,
                                                   member_field<"phone", proto::message<d_phone>, 4, &d_person::phone>>
        protobuf_fields{};
};

struct d_scalars {
    std::int32_t i32{};
    std::uint32_t u32{};
    std::int32_t s32{};
    std::int64_t i64{};
    bool flag{};
    std::uint32_t f32{};
    std::int32_t sf32{};
    float f{};
    std::uint64_t f64{};
    std::int64_t sf64{};
    double d{};
    std::string s{};
    std::string b{};

    static constexpr rainy::core::container::tuple<
        member_field<"i32", proto::int32, 1, &d_scalars::i32>, member_field<"u32", proto::uint32, 2, &d_scalars::u32>,
        member_field<"s32", proto::sint32, 3, &d_scalars::s32>, member_field<"i64", proto::int64, 4, &d_scalars::i64>,
        member_field<"flag", proto::boolean, 5, &d_scalars::flag>,
        member_field<"f32", proto::fixed32, 6, &d_scalars::f32>,
        member_field<"sf32", proto::sfixed32, 7, &d_scalars::sf32>,
        member_field<"f", proto::floating, 8, &d_scalars::f>,
        member_field<"f64", proto::fixed64, 9, &d_scalars::f64>,
        member_field<"sf64", proto::sfixed64, 10, &d_scalars::sf64>, member_field<"d", proto::doubling, 11, &d_scalars::d>,
        member_field<"s", proto::string, 12, &d_scalars::s>, member_field<"b", proto::bytes, 13, &d_scalars::b>>
        protobuf_fields{};
};

struct d_numbers {
    std::vector<std::int32_t> e{};

    static constexpr rainy::core::container::tuple<member_field<"e", proto::repeated<proto::int32>, 1, &d_numbers::e>>
        protobuf_fields{};
};

struct d_wide {
    std::int64_t big{};

    static constexpr rainy::core::container::tuple<member_field<"big", proto::int32, 1, &d_wide::big>> protobuf_fields{};
};

namespace {
    d_person make_person() {
        d_person person{};
        person.name = "testing";
        person.age = 32;
        person.tags = {"alpha", "beta"};
        person.phone.number = "1234";
        return person;
    }

    d_scalars make_scalars() {
        d_scalars doc{};
        doc.i32 = -2;
        doc.u32 = 200;
        doc.s32 = -500;
        doc.i64 = -2;
        doc.flag = true;
        doc.f32 = 200;
        doc.sf32 = -200;
        doc.f = 25.5f;
        doc.f64 = 200;
        doc.sf64 = -200;
        doc.d = 25.4;
        doc.s = "hello";
        doc.b = "bytes";
        return doc;
    }

    const protobuf::byte_buffer &official_person_bytes() {
        static const protobuf::byte_buffer bytes{0x0A, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x10, 0x20,
                                                 0x1A, 0x05, 0x61, 0x6C, 0x70, 0x68, 0x61, 0x1A, 0x04, 0x62, 0x65,
                                                 0x74, 0x61, 0x22, 0x06, 0x0A, 0x04, 0x31, 0x32, 0x33, 0x34};
        return bytes;
    }

    const protobuf::byte_buffer &official_scalars_bytes() {
        static const protobuf::byte_buffer bytes{
            0x08, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x10, 0xC8, 0x01, 0x18, 0xE7, 0x07,
            0x20, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x28, 0x01, 0x35, 0xC8, 0x00, 0x00,
            0x00, 0x3D, 0x38, 0xFF, 0xFF, 0xFF, 0x45, 0x00, 0x00, 0xCC, 0x41, 0x49, 0xC8, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x51, 0x38, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x59, 0x66, 0x66, 0x66, 0x66,
            0x66, 0x66, 0x39, 0x40, 0x62, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x6A, 0x05, 0x62, 0x79, 0x74, 0x65,
            0x73};
        return bytes;
    }
}

TEST_CASE("protobuf direct - concept shape requires member fields with explicit numbers", "[willow][protobuf]") {
    STATIC_REQUIRE(direct_message<d_person>);
    STATIC_REQUIRE(direct_message<d_scalars>);
    STATIC_REQUIRE(field_count_v<d_person> == 4);
    STATIC_REQUIRE(field_number_v<d_person, 0> == 1);
    STATIC_REQUIRE(field_number_v<d_person, 3> == 4);
    STATIC_REQUIRE(valid_field_numbers_v<d_person>);
}

TEST_CASE("protobuf direct - empty struct encodes to nothing", "[willow][protobuf]") {
    REQUIRE(protobuf::encode_direct(d_person{}).empty());
    REQUIRE(protobuf::encode_direct(d_scalars{}).empty());
    REQUIRE(protobuf::decode_direct<d_person>(nullptr, 0).name.empty());
}

TEST_CASE("protobuf direct - encoding matches official wire bytes", "[willow][protobuf]") {
    auto person_bytes = protobuf::encode_direct(make_person());
    const auto &expected_person = official_person_bytes();
    REQUIRE(person_bytes.size() == expected_person.size());
    REQUIRE(person_bytes == expected_person);
    auto scalars_bytes = protobuf::encode_direct(make_scalars());
    const auto &expected_scalars = official_scalars_bytes();
    REQUIRE(scalars_bytes.size() == expected_scalars.size());
    REQUIRE(scalars_bytes == expected_scalars);
}

TEST_CASE("protobuf direct - decoding official wire bytes", "[willow][protobuf]") {
    const auto &person_bytes = official_person_bytes();
    d_person person = protobuf::decode_direct<d_person>(person_bytes.data(), person_bytes.size());
    REQUIRE(person.name == "testing");
    REQUIRE(person.age == 32);
    REQUIRE(person.tags.size() == 2);
    REQUIRE(person.tags[1] == "beta");
    REQUIRE(person.phone.number == "1234");
    const auto &scalars_bytes = official_scalars_bytes();
    d_scalars scalars = protobuf::decode_direct<d_scalars>(scalars_bytes.data(), scalars_bytes.size());
    REQUIRE(scalars.i32 == -2);
    REQUIRE(scalars.u32 == 200);
    REQUIRE(scalars.s32 == -500);
    REQUIRE(scalars.i64 == -2);
    REQUIRE(scalars.flag == true);
    REQUIRE(scalars.f32 == 200);
    REQUIRE(scalars.sf32 == -200);
    REQUIRE(scalars.f == 25.5f);
    REQUIRE(scalars.f64 == 200);
    REQUIRE(scalars.sf64 == -200);
    REQUIRE(scalars.d == 25.4);
    REQUIRE(scalars.s == "hello");
    REQUIRE(scalars.b == "bytes");
}

TEST_CASE("protobuf direct - repeated roundtrip and packed input", "[willow][protobuf]") {
    d_numbers numbers{};
    for (int i = 0; i < 2000; ++i) {
        numbers.e.push_back(i);
    }
    auto bytes = protobuf::encode_direct(numbers);
    d_numbers back = protobuf::decode_direct<d_numbers>(bytes);
    REQUIRE(back.e.size() == 2000);
    REQUIRE(back.e[1999] == 1999);
    protobuf::byte_buffer packed{0x0A, 0x03, 0x01, 0x02, 0x03};
    d_numbers packed_back = protobuf::decode_direct<d_numbers>(packed.data(), packed.size());
    REQUIRE(packed_back.e.size() == 3);
    REQUIRE(packed_back.e[2] == 3);
}

TEST_CASE("protobuf direct - last one wins, unknown skipped, into appends", "[willow][protobuf]") {
    protobuf::byte_buffer raw{0x10, 0x01, 0x10, 0x96, 0x01, 0x78, 0x01};
    d_person back = protobuf::decode_direct<d_person>(raw.data(), raw.size());
    REQUIRE(back.age == 150);
    protobuf::byte_buffer packed{0x0A, 0x02, 0x07, 0x08};
    d_numbers numbers{};
    numbers.e.push_back(9);
    protobuf::decode_direct_into<d_numbers>(packed.data(), packed.size(), numbers);
    REQUIRE(numbers.e.size() == 3);
    REQUIRE(numbers.e[0] == 9);
}

TEST_CASE("protobuf direct - repeated run interleave and cut", "[willow][protobuf]") {
    protobuf::byte_buffer interleaved{0x08, 0x01, 0x98, 0x06, 0x07, 0x08, 0x02};
    d_numbers back = protobuf::decode_direct<d_numbers>(interleaved.data(), interleaved.size());
    REQUIRE(back.e.size() == 2);
    REQUIRE(back.e[0] == 1);
    REQUIRE(back.e[1] == 2);
    protobuf::byte_buffer cut_in_run{0x08, 0x01, 0x08};
    REQUIRE_THROWS(protobuf::decode_direct<d_numbers>(cut_in_run.data(), cut_in_run.size()));
}

TEST_CASE("protobuf direct - errors", "[willow][protobuf]") {
    protobuf::byte_buffer cut_tag{0x08};
    REQUIRE_THROWS(protobuf::decode_direct<d_person>(cut_tag.data(), cut_tag.size()));
    protobuf::byte_buffer bad_tag{0x00};
    REQUIRE_THROWS(protobuf::decode_direct<d_person>(bad_tag.data(), bad_tag.size()));
    protobuf::byte_buffer wrong_wire{0x0D, 0x01, 0x00, 0x00, 0x00};
    REQUIRE_THROWS(protobuf::decode_direct<d_person>(wrong_wire.data(), wrong_wire.size()));
    d_wide wide{};
    wide.big = static_cast<std::int64_t>(2147483648ll);
    REQUIRE_THROWS(protobuf::encode_direct(wide));
}

#endif
