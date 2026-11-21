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
struct r_phone : add_reflection<r_phone>, add_unknown_fields<r_phone> {
    std::string number{};

    static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &r_phone::number>>
        protobuf_fields{};
};

struct r_person : add_reflection<r_person>, add_unknown_fields<r_person> {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    r_phone phone{};

    static constexpr rainy::core::container::tuple<
        member_field<"name", proto::string, 1, &r_person::name>,
        member_field<"age", proto::int32, 2, &r_person::age>,
        member_field<"tags", proto::repeated<proto::string>, 3, &r_person::tags>,
        member_field<"phone", proto::message<r_phone>, 4, &r_person::phone>>
        protobuf_fields{};
};

struct r_scalars : add_reflection<r_scalars>, add_unknown_fields<r_scalars> {
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
        member_field<"i32", proto::int32, 1, &r_scalars::i32>,
        member_field<"u32", proto::uint32, 2, &r_scalars::u32>,
        member_field<"s32", proto::sint32, 3, &r_scalars::s32>,
        member_field<"i64", proto::int64, 4, &r_scalars::i64>,
        member_field<"flag", proto::boolean, 5, &r_scalars::flag>,
        member_field<"f32", proto::fixed32, 6, &r_scalars::f32>,
        member_field<"sf32", proto::sfixed32, 7, &r_scalars::sf32>,
        member_field<"f", proto::floating, 8, &r_scalars::f>,
        member_field<"f64", proto::fixed64, 9, &r_scalars::f64>,
        member_field<"sf64", proto::sfixed64, 10, &r_scalars::sf64>,
        member_field<"d", proto::doubling, 11, &r_scalars::d>,
        member_field<"s", proto::string, 12, &r_scalars::s>,
        member_field<"b", proto::bytes, 13, &r_scalars::b>>
        protobuf_fields{};
};
} // namespace

TEST_CASE("protobuf reflection - get scalar fields by name and number",
          "[willow][protobuf][reflection]") {
    r_scalars sc{};
    sc.i32 = -42;
    sc.u32 = 1000;
    sc.s32 = -7;
    sc.i64 = -100000;
    sc.flag = true;
    sc.f32 = 200;
    sc.sf32 = -200;
    sc.f = 3.14f;
    sc.f64 = 1000000;
    sc.sf64 = -1000000;
    sc.d = 2.718;
    sc.s = "hello";
    sc.b = "bytes";

    REQUIRE(sc.get("i32").as_int64() == -42);
    REQUIRE(sc.get(1).as_int64() == -42);
    REQUIRE(sc.get("u32").as_uint64() == 1000);
    REQUIRE(sc.get(2).as_uint64() == 1000);
    REQUIRE(sc.get("s32").as_int64() == -7);
    REQUIRE(sc.get("i64").as_int64() == -100000);
    REQUIRE(sc.get("flag").as_boolean() == true);
    REQUIRE(sc.get("f32").as_uint64() == 200);
    REQUIRE(sc.get("sf32").as_int64() == -200);
    REQUIRE(sc.get("f").as_double() == Catch::Approx(3.14));
    REQUIRE(sc.get("f64").as_uint64() == 1000000);
    REQUIRE(sc.get("sf64").as_int64() == -1000000);
    REQUIRE(sc.get("d").as_double() == Catch::Approx(2.718));
    REQUIRE(sc.get("s").as_string() == "hello");
    REQUIRE(sc.get("b").as_string() == "bytes");
}

TEST_CASE("protobuf reflection - set scalar fields by name", "[willow][protobuf][reflection]") {
    r_scalars sc{};

    sc.set("i32", std::int64_t{-123});
    REQUIRE(sc.i32 == -123);
    REQUIRE(sc.get("i32").as_int64() == -123);

    sc.set("u32", std::uint64_t{456});
    REQUIRE(sc.u32 == 456);

    sc.set("s32", std::int64_t{-89});
    REQUIRE(sc.s32 == -89);

    sc.set("i64", std::int64_t{-9999999});
    REQUIRE(sc.i64 == -9999999);

    sc.set("flag", true);
    REQUIRE(sc.flag == true);

    sc.set("f32", std::uint64_t{777});
    REQUIRE(sc.f32 == 777);

    sc.set("sf32", std::int64_t{-777});
    REQUIRE(sc.sf32 == -777);

    sc.set("f", 1.5);
    REQUIRE(sc.f == Catch::Approx(1.5));

    sc.set("f64", std::uint64_t{88888888});
    REQUIRE(sc.f64 == 88888888);

    sc.set("sf64", std::int64_t{-88888888});
    REQUIRE(sc.sf64 == -88888888);

    sc.set("d", 9.25);
    REQUIRE(sc.d == Catch::Approx(9.25));

    sc.set("s", "world");
    REQUIRE(sc.s == "world");

    sc.set("b", "raw");
    REQUIRE(sc.b == "raw");

    // set by number
    sc.set(1, std::int64_t{42});
    REQUIRE(sc.i32 == 42);
    sc.set(2, std::uint64_t{99});
    REQUIRE(sc.u32 == 99);
    sc.set(5, true);
    REQUIRE(sc.flag == true);
    sc.set(12, "abc");
    REQUIRE(sc.s == "abc");
}

TEST_CASE("protobuf reflection - set int literal dispatches to signed scalar", "[willow][protobuf][reflection]") {
    r_scalars sc{};
    // 30 is int -> signed integral dispatch -> i32 field accepts it.
    sc.set("i32", 30);
    REQUIRE(sc.i32 == 30);

    // 30u is unsigned -> unsigned integral dispatch -> u32 field accepts it.
    sc.set("u32", 30u);
    REQUIRE(sc.u32 == 30);

    // 30.0 is double -> floating dispatch -> d field accepts it.
    sc.set("d", 30.0);
    REQUIRE(sc.d == Catch::Approx(30.0));
}

TEST_CASE("protobuf reflection - set range check rejects out-of-range int32",
          "[willow][protobuf][reflection]") {
    r_scalars sc{};
    REQUIRE_THROWS(sc.set("i32", std::int64_t{2147483648ll}));
    REQUIRE_THROWS(sc.set("i32", std::int64_t{-2147483649ll}));
    REQUIRE_THROWS(sc.set("u32", std::uint64_t{4294967296ull}));
    REQUIRE_THROWS(sc.set("u32", std::int64_t{-1}));
}

TEST_CASE("protobuf reflection - set type-mismatch throws", "[willow][protobuf][reflection]") {
    r_scalars sc{};
    REQUIRE_THROWS(sc.set("i32", "not-int"));
    REQUIRE_THROWS(sc.set("s", std::int64_t{42}));
    REQUIRE_THROWS(sc.set("flag", "x"));
}

TEST_CASE("protobuf reflection - set rejects message and repeated fields",
          "[willow][protobuf][reflection]") {
    r_person p{};
    REQUIRE_THROWS(p.set("phone", std::int64_t{30}));
    REQUIRE_THROWS(p.set("tags", std::int64_t{30}));
    REQUIRE_THROWS(p.set("phone", "x"));
    REQUIRE_THROWS(p.set("tags", "x"));
}

TEST_CASE("protobuf reflection - has and is_default match encode-skip semantics",
          "[willow][protobuf][reflection]") {
    r_scalars sc{};
    REQUIRE(!sc.has("i32"));
    REQUIRE(sc.is_default("i32"));

    sc.i32 = 5;
    REQUIRE(sc.has("i32"));
    REQUIRE(!sc.is_default("i32"));

    sc.i32 = 0;
    REQUIRE(!sc.has("i32"));
    REQUIRE(sc.is_default("i32"));

    // bool: default is false.
    REQUIRE(!sc.has("flag"));
    sc.flag = true;
    REQUIRE(sc.has("flag"));
    sc.flag = false;
    REQUIRE(!sc.has("flag"));

    // string: default is empty.
    REQUIRE(!sc.has("s"));
    sc.s = "x";
    REQUIRE(sc.has("s"));
    sc.s.clear();
    REQUIRE(!sc.has("s"));

    // has/is_default by number.
    sc.i32 = 7;
    REQUIRE(sc.has(1));
    REQUIRE(!sc.is_default(1));
    sc.i32 = 0;
    REQUIRE(!sc.has(1));
    REQUIRE(sc.is_default(1));
}

TEST_CASE("protobuf reflection - clear resets to default", "[willow][protobuf][reflection]") {
    r_scalars sc{};
    sc.i32 = 100;
    sc.u32 = 200;
    sc.flag = true;
    sc.s = "abc";
    sc.b = "xyz";

    sc.clear("i32");
    REQUIRE(sc.i32 == 0);
    sc.clear("u32");
    REQUIRE(sc.u32 == 0);
    sc.clear("flag");
    REQUIRE(sc.flag == false);
    sc.clear("s");
    REQUIRE(sc.s.empty());
    sc.clear("b");
    REQUIRE(sc.b.empty());

    // Clear by number.
    sc.i32 = 5;
    sc.clear(1);
    REQUIRE(sc.i32 == 0);
}

TEST_CASE("protobuf reflection - clear on repeated and message fields",
          "[willow][protobuf][reflection]") {
    r_person p{};
    p.tags.push_back("a");
    p.tags.push_back("b");
    p.phone.number = "1234";

    p.clear("tags");
    REQUIRE(p.tags.empty());

    p.clear("phone");
    REQUIRE(p.phone.number.empty());
}

TEST_CASE("protobuf reflection - clear then re-encode omits the field",
          "[willow][protobuf][reflection]") {
    r_scalars sc{};
    sc.i32 = 42;
    sc.s = "hello";
    byte_buffer before = encode_direct(sc);
    REQUIRE(!before.empty());

    sc.clear("i32");
    sc.clear("s");
    byte_buffer after = encode_direct(sc);
    REQUIRE(after.empty());
}

TEST_CASE("protobuf reflection - mutate nested message and repeated fields",
          "[willow][protobuf][reflection]") {
    r_person p{};

    r_phone &phone = p.mutate("phone");
    phone.number = "5678";
    REQUIRE(p.phone.number == "5678");

    r_phone &phone_ref = p.mutate("phone");
    phone_ref.set("number", "9999");
    REQUIRE(p.phone.number == "9999");

    std::vector<std::string> &tags = p.mutate("tags");
    tags.push_back("alpha");
    tags.push_back("beta");
    REQUIRE(p.tags.size() == 2);
    REQUIRE(p.tags[0] == "alpha");

    // mutate by number.
    r_phone &phone2 = p.mutate(4);
    phone2.number = "0000";
    REQUIRE(p.phone.number == "0000");

    std::vector<std::string> &tags2 = p.mutate(3);
    tags2.clear();
    REQUIRE(p.tags.empty());
}

TEST_CASE("protobuf reflection - mutate rejects scalar/string fields", "[willow][protobuf][reflection]") {
    r_scalars sc{};
    REQUIRE_THROWS(sc.mutate("i32"));
    REQUIRE_THROWS(sc.mutate("s"));
    REQUIRE_THROWS(sc.mutate(1));
}

TEST_CASE("protobuf reflection - get on unknown field name throws", "[willow][protobuf][reflection]") {
    r_scalars sc{};
    REQUIRE_THROWS(sc.get("nonexistent"));
    REQUIRE_THROWS(sc.get(99));
    REQUIRE_THROWS(sc.set("nonexistent", std::int64_t{1}));
    REQUIRE_THROWS(sc.has("nonexistent"));
    REQUIRE_THROWS(sc.is_default("nonexistent"));
    REQUIRE_THROWS(sc.clear("nonexistent"));
    REQUIRE_THROWS(sc.mutate("nonexistent"));
}

TEST_CASE("protobuf reflection - view form (reflection<T> instance) is equivalent to CRTP",
          "[willow][protobuf][reflection]") {
    r_scalars sc{};
    sc.i32 = 7;
    sc.s = "abc";

    reflection<r_scalars> refl;
    REQUIRE(refl.get(sc, "i32").as_int64() == 7);
    REQUIRE(refl.get(sc, 1).as_int64() == 7);
    REQUIRE(refl.has(sc, "s"));
    refl.set(sc, "i32", std::int64_t{88});
    REQUIRE(sc.i32 == 88);
    refl.clear(sc, "s");
    REQUIRE(sc.s.empty());
}

TEST_CASE("protobuf reflection - get on message/repeated returns pointer dynamic_value",
          "[willow][protobuf][reflection]") {
    r_person p{};
    p.phone.number = "abc";
    p.tags.push_back("x");

    auto phone_v = p.get("phone");
    REQUIRE(phone_v.kind() == dynamic_kind::message);
    const auto *phone_ptr = static_cast<const r_phone *>(phone_v.as_message());
    REQUIRE(phone_ptr != nullptr);
    REQUIRE(phone_ptr->number == "abc");

    auto tags_v = p.get("tags");
    REQUIRE(tags_v.kind() == dynamic_kind::repeated);
    const auto *tags_ptr = static_cast<const std::vector<std::string> *>(tags_v.as_repeated());
    REQUIRE(tags_ptr != nullptr);
    REQUIRE(tags_ptr->size() == 1);
}

#endif
