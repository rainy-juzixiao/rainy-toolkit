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

namespace {
struct u_phone : add_unknown_fields<u_phone> {
    std::string number{};

    static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &u_phone::number>>
        protobuf_fields{};
};

struct u_person : add_unknown_fields<u_person> {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    u_phone phone{};

    static constexpr rainy::core::container::tuple<
        member_field<"name", proto::string, 1, &u_person::name>,
        member_field<"age", proto::int32, 2, &u_person::age>,
        member_field<"tags", proto::repeated<proto::string>, 3, &u_person::tags>,
        member_field<"phone", proto::message<u_phone>, 4, &u_person::phone>>
        protobuf_fields{};
};

struct u_scalars : add_unknown_fields<u_scalars> {
    std::int32_t i32{};
    std::string s{};

    static constexpr rainy::core::container::tuple<member_field<"i32", proto::int32, 1, &u_scalars::i32>,
                                                      member_field<"s", proto::string, 2, &u_scalars::s>>
        protobuf_fields{};
};

// A struct that does NOT inherit unknown_fields_crtp — used to verify the old behavior.
struct u_plain {
    std::int32_t age{};

    static constexpr rainy::core::container::tuple<member_field<"age", proto::int32, 2, &u_plain::age>>
        protobuf_fields{};
};
} // namespace

TEST_CASE("protobuf unknown - empty by default", "[willow][protobuf][unknown]") {
    u_person p{};
    REQUIRE(p.unknown().empty());
    REQUIRE(p.unknown().size() == 0);
}

TEST_CASE("protobuf unknown - decode captures unknown varint field", "[willow][protobuf][unknown]") {
    // 0x0A 0x07 "testing" -> name="testing"
    // 0x10 0x20 -> age=32
    // 0x78 0x01 -> unknown field number 15, varint=1
    byte_buffer raw{0x0A, 0x07, 't', 'e', 's', 't', 'i', 'n', 'g',
                    0x10, 0x20,
                    0x78, 0x01};
    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    REQUIRE(p.name == "testing");
    REQUIRE(p.age == 32);
    REQUIRE(!p.unknown().empty());
    REQUIRE(p.unknown().size() == 1);

    const auto *f = p.unknown().find(15);
    REQUIRE(f != nullptr);
    REQUIRE(f->number() == 15);
    REQUIRE(f->wire() == wire_type::varint);
    REQUIRE(f->as_bytes().size() == 1);
    REQUIRE(f->as_bytes()[0] == 0x01);
    REQUIRE(f->as_varint() == 1);
}

TEST_CASE("protobuf unknown - decode captures multiple unknown fields in order", "[willow][protobuf][unknown]") {
    // 0x10 0x20 -> age=32 (field 2)
    // 0x78 0x01 -> unknown field 15, varint=1
    // 0x80 0x01 0x01 -> unknown field 16 tag, varint=1
    // 0x8A 0x01 0x02 0xAA 0xBB -> unknown field 17, len=2, payload={0xAA, 0xBB}
    byte_buffer raw{0x10, 0x20,
                    0x78, 0x01,
                    0x80, 0x01, 0x01,
                    0x8A, 0x01, 0x02, 0xAA, 0xBB};
    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    REQUIRE(p.age == 32);
    REQUIRE(p.unknown().size() == 3);

    REQUIRE(p.unknown()[0].number() == 15);
    REQUIRE(p.unknown()[0].wire() == wire_type::varint);
    REQUIRE(p.unknown()[1].number() == 16);
    REQUIRE(p.unknown()[1].wire() == wire_type::varint);
    REQUIRE(p.unknown()[2].number() == 17);
    REQUIRE(p.unknown()[2].wire() == wire_type::len);
    REQUIRE(p.unknown()[2].as_bytes().size() == 2);
    REQUIRE(p.unknown()[2].as_bytes()[0] == 0xAA);
    REQUIRE(p.unknown()[2].as_bytes()[1] == 0xBB);
}

TEST_CASE("protobuf unknown - encode roundtrip is byte-exact when unknown fields present",
          "[willow][protobuf][unknown]") {
    byte_buffer raw{0x0A, 0x07, 't', 'e', 's', 't', 'i', 'n', 'g',
                    0x10, 0x20,
                    0x78, 0x01,
                    0x8A, 0x01, 0x02, 0xAA, 0xBB};
    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    REQUIRE(p.unknown().size() == 2);

    byte_buffer out = encode_direct(p);
    // Known fields: name=testing, age=32 (varint encoded).
    // Unknown fields appended in capture order: field 15 varint 1, field 17 len=2 {0xAA, 0xBB}.
    REQUIRE(out.size() == raw.size());
    REQUIRE(out == raw);
}

TEST_CASE("protobuf unknown - find returns nullptr for absent number", "[willow][protobuf][unknown]") {
    byte_buffer raw{0x78, 0x01};
    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    REQUIRE(p.unknown().find(15) != nullptr);
    REQUIRE(p.unknown().find(99) == nullptr);
}

TEST_CASE("protobuf unknown - remove and clear", "[willow][protobuf][unknown]") {
    byte_buffer raw{0x78, 0x01, 0x80, 0x01, 0x01, 0x88, 0x01, 0x01};
    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    REQUIRE(p.unknown().size() == 3);

    p.unknown().remove(16);
    REQUIRE(p.unknown().size() == 2);
    REQUIRE(p.unknown().find(16) == nullptr);

    p.unknown().clear();
    REQUIRE(p.unknown().empty());
    REQUIRE(p.unknown().size() == 0);
}

TEST_CASE("protobuf unknown - as_varint throws on non-varint wire", "[willow][protobuf][unknown]") {
    byte_buffer raw{0x8A, 0x01, 0x02, 0xAA, 0xBB}; // unknown field 17, len=2
    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    const auto *f = p.unknown().find(17);
    REQUIRE(f != nullptr);
    REQUIRE(f->wire() == wire_type::len);
    REQUIRE_THROWS(f->as_varint());
}

TEST_CASE("protobuf unknown - group wire type throws (not captured)", "[willow][protobuf][unknown]") {
    // field number 1, wire type sgroup (3) -> tag = (1<<3) | 3 = 0x0B
    byte_buffer raw{0x0B};
    REQUIRE_THROWS(decode_direct<u_person>(raw.data(), raw.size()));
}

TEST_CASE("protobuf unknown - struct without CRTP drops unknown fields (old behavior)",
          "[willow][protobuf][unknown]") {
    // 0x10 0x20 -> age=32 (field 2)
    // 0x78 0x01 -> unknown field 15, varint=1 (should be dropped)
    byte_buffer raw{0x10, 0x20, 0x78, 0x01};
    u_plain p = decode_direct<u_plain>(raw.data(), raw.size());
    REQUIRE(p.age == 32);
    // No way to access unknown fields since the struct doesn't inherit CRTP — they're silently dropped.
    // Re-encoding should yield only the known field.
    byte_buffer out = encode_direct(p);
    REQUIRE(out.size() == 2);
    REQUIRE(out[0] == 0x10);
    REQUIRE(out[1] == 0x20);
}

TEST_CASE("protobuf unknown - nested message also captures unknown", "[willow][protobuf][unknown]") {
    // phone message with field 1 (number="1234") and an unknown field 99, varint=7
    // phone bytes: 0x0A 0x04 "1234" 0x98 0x06 0x07 -> field 1 LEN 4 "1234", field 99 varint 7
    // person: name="testing" (field 1), phone (field 4) wraps the phone bytes.
    byte_buffer phone_bytes{0x0A, 0x04, '1', '2', '3', '4',
                             0x98, 0x06, 0x07};
    byte_buffer raw;
    raw.push_back(0x0A);
    raw.push_back(0x07);
    raw.insert(raw.end(), "testing", "testing" + 7);
    raw.push_back(0x22);
    raw.push_back(static_cast<std::uint8_t>(phone_bytes.size()));
    raw.insert(raw.end(), phone_bytes.begin(), phone_bytes.end());

    u_person p = decode_direct<u_person>(raw.data(), raw.size());
    REQUIRE(p.name == "testing");
    REQUIRE(p.phone.number == "1234");
    REQUIRE(p.phone.unknown().size() == 1);
    REQUIRE(p.phone.unknown()[0].number() == 99);
    REQUIRE(p.phone.unknown()[0].as_varint() == 7);
    REQUIRE(p.unknown().empty());

    byte_buffer out = encode_direct(p);
    REQUIRE(out == raw);
}

#endif
