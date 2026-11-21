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
struct c_phone : add_clearme<c_phone>, add_unknown_fields<c_phone> {
    std::string number{};

    static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &c_phone::number>>
        protobuf_fields{};
};

struct c_person : add_clearme<c_person>, add_unknown_fields<c_person> {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    c_phone phone{};

    static constexpr rainy::core::container::tuple<
        member_field<"name", proto::string, 1, &c_person::name>,
        member_field<"age", proto::int32, 2, &c_person::age>,
        member_field<"tags", proto::repeated<proto::string>, 3, &c_person::tags>,
        member_field<"phone", proto::message<c_phone>, 4, &c_person::phone>>
        protobuf_fields{};
};

struct c_scalars : add_clearme<c_scalars>, add_unknown_fields<c_scalars> {
    std::int32_t i32{};
    std::uint32_t u32{};
    bool flag{};
    float f{};
    double d{};
    std::string s{};
    std::string b{};

    static constexpr rainy::core::container::tuple<
        member_field<"i32", proto::int32, 1, &c_scalars::i32>,
        member_field<"u32", proto::uint32, 2, &c_scalars::u32>,
        member_field<"flag", proto::boolean, 3, &c_scalars::flag>,
        member_field<"f", proto::floating, 4, &c_scalars::f>,
        member_field<"d", proto::doubling, 5, &c_scalars::d>,
        member_field<"s", proto::string, 6, &c_scalars::s>,
        member_field<"b", proto::bytes, 7, &c_scalars::b>>
        protobuf_fields{};
};
} // namespace

TEST_CASE("protobuf clearme - resets scalar fields to initialized state", "[willow][protobuf][clearme]") {
    c_scalars sc{};
    sc.i32 = -42;
    sc.u32 = 1000;
    sc.flag = true;
    sc.f = 3.14f;
    sc.d = 2.718;
    sc.s = "hello";
    sc.b = "bytes";

    sc.clearme();

    REQUIRE(sc.i32 == 0);
    REQUIRE(sc.u32 == 0);
    REQUIRE(sc.flag == false);
    REQUIRE(sc.f == 0.0f);
    REQUIRE(sc.d == 0.0);
    REQUIRE(sc.s.empty());
    REQUIRE(sc.b.empty());
}

TEST_CASE("protobuf clearme - resets repeated and nested message fields", "[willow][protobuf][clearme]") {
    c_person p{};
    p.name = "alice";
    p.age = 30;
    p.tags.push_back("a");
    p.tags.push_back("b");
    p.phone.number = "1234";

    p.clearme();

    REQUIRE(p.name.empty());
    REQUIRE(p.age == 0);
    REQUIRE(p.tags.empty());
    REQUIRE(p.phone.number.empty());
}

TEST_CASE("protobuf clearme - clears unknown fields", "[willow][protobuf][clearme]") {
    c_scalars sc{};
    const std::uint8_t payload[] = {0x01, 0x02};
    sc.unknown().append(99, wire_type::len, payload, 2);
    REQUIRE(sc.unknown().size() == 1);

    sc.clearme();

    REQUIRE(sc.unknown().empty());
}

TEST_CASE("protobuf clearme - cleared object encodes empty", "[willow][protobuf][clearme]") {
    c_person p{};
    p.name = "alice";
    p.age = 30;
    p.tags.push_back("a");
    p.phone.number = "1234";
    REQUIRE(!encode_direct(p).empty());

    p.clearme();

    REQUIRE(encode_direct(p).empty());
}

TEST_CASE("protobuf clearme - already-cleared object stays default", "[willow][protobuf][clearme]") {
    c_scalars sc{};
    sc.clearme();
    REQUIRE(protobuf::implements::direct_message_is_default(sc));

    sc.clearme();
    REQUIRE(protobuf::implements::direct_message_is_default(sc));
}

#endif
