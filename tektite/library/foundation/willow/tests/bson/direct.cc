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

#include <cstdint>
#include <string>
#include <vector>

#if RAINY_HAS_CXX20

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::bson;

namespace wb = rainy::foundation::exceptions::willow::bson;

namespace {
struct d_phone {
    std::string number{};

    static constexpr rainy::core::container::tuple<member_field<"number", types::string_, &d_phone::number>> bson_fields{};
};

struct d_person {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    d_phone phone{};

    static constexpr rainy::core::container::tuple<member_field<"name", types::string_, &d_person::name>,
                                                   member_field<"age", types::int32, &d_person::age>,
                                                   member_field<"tags", types::repeated<types::string_>, &d_person::tags>,
                                                   member_field<"phone", types::message<d_phone>, &d_person::phone>>
        bson_fields{};
};

struct d_scalars {
    std::int32_t i32{};
    std::int64_t i64{};
    double d{};
    bool flag{};
    std::string s{};
    std::string b{};

    static constexpr rainy::core::container::tuple<member_field<"i32", types::int32, &d_scalars::i32>,
                                                   member_field<"i64", types::int64, &d_scalars::i64>,
                                                   member_field<"d", types::double_, &d_scalars::d>,
                                                   member_field<"flag", types::boolean, &d_scalars::flag>,
                                                   member_field<"s", types::string_, &d_scalars::s>,
                                                   member_field<"b", types::binary, &d_scalars::b>>
        bson_fields{};
};

}

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
        doc.i64 = 4294967296LL;
        doc.d = 25.4;
        doc.flag = true;
        doc.s = "hello";
        doc.b = "bytes";
        return doc;
    }

    const byte_buffer &official_person_bytes() {
        static const byte_buffer bytes{0x61, 0x00, 0x00, 0x00, 0x02, 0x6e, 0x61, 0x6d, 0x65, 0x00, 0x08, 0x00, 0x00,
                                       0x00, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67, 0x00, 0x10, 0x61, 0x67, 0x65,
                                       0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x74, 0x61, 0x67, 0x73, 0x00, 0x1e, 0x00,
                                       0x00, 0x00, 0x02, 0x30, 0x00, 0x06, 0x00, 0x00, 0x00, 0x61, 0x6c, 0x70, 0x68,
                                       0x61, 0x00, 0x02, 0x31, 0x00, 0x05, 0x00, 0x00, 0x00, 0x62, 0x65, 0x74, 0x61,
                                       0x00, 0x00, 0x03, 0x70, 0x68, 0x6f, 0x6e, 0x65, 0x00, 0x16, 0x00, 0x00, 0x00,
                                       0x02, 0x6e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x00, 0x05, 0x00, 0x00, 0x00, 0x31,
                                       0x32, 0x33, 0x34, 0x00, 0x00, 0x00};
        return bytes;
    }
}

TEST_CASE("bson direct - the concept shape requires member fields", "[willow][bson]") {
    STATIC_REQUIRE(direct_message_v<d_person>);
    STATIC_REQUIRE(direct_message_v<d_scalars>);
    STATIC_REQUIRE(field_count_v<d_person> == 4);
    STATIC_REQUIRE(valid_field_names_v<d_person>);
}

TEST_CASE("bson direct - an empty struct encodes to an empty document", "[willow][bson]") {
    REQUIRE(bson::encode_direct(d_person{}) == byte_buffer{0x05, 0x00, 0x00, 0x00, 0x00});
    REQUIRE(bson::encode_direct(d_scalars{}) == byte_buffer{0x05, 0x00, 0x00, 0x00, 0x00});
    REQUIRE(bson::decode_direct<d_person>(nullptr, 0).name.empty());
}

TEST_CASE("bson direct - encoding matches official wire bytes", "[willow][bson]") {
    const auto bytes = bson::encode_direct(make_person());
    const auto &expected = official_person_bytes();
    REQUIRE(bytes.size() == expected.size());
    REQUIRE(bytes == expected);
}

TEST_CASE("bson direct - decoding official wire bytes", "[willow][bson]") {
    const auto &bytes = official_person_bytes();
    d_person person = bson::decode_direct<d_person>(bytes.data(), bytes.size());
    REQUIRE(person.name == "testing");
    REQUIRE(person.age == 32);
    REQUIRE(person.tags.size() == 2);
    REQUIRE(person.tags[1] == "beta");
    REQUIRE(person.phone.number == "1234");
}

TEST_CASE("bson direct - scalars round trip", "[willow][bson]") {
    const auto scalars = make_scalars();
    auto back = bson::decode_direct<d_scalars>(bson::encode_direct(scalars));
    REQUIRE(back.i32 == -2);
    REQUIRE(back.i64 == 4294967296LL);
    REQUIRE(back.d == 25.4);
    REQUIRE(back.flag);
    REQUIRE(back.s == "hello");
    REQUIRE(back.b == "bytes");
}

TEST_CASE("bson direct - an absent field keeps its previous value", "[willow][bson]") {
    d_person person{};
    person.name = "keep";
    person.age = 7;
    d_person partial{};
    partial.age = 99;
    const auto bytes = bson::encode_direct(partial);
    bson::decode_direct_into<d_person>(bytes, person);
    REQUIRE(person.name == "keep");
    REQUIRE(person.age == 99);
}

TEST_CASE("bson direct - an unknown key is skipped", "[willow][bson]") {
    document extra(document_type::object);
    extra["name"] = document(std::string("rainy"));
    extra["unknown"] = document(std::string("ignored"));
    extra["age"] = document(5);
    const auto bytes = bson::encode(extra);
    d_person person = bson::decode_direct<d_person>(bytes);
    REQUIRE(person.name == "rainy");
    REQUIRE(person.age == 5);
}

TEST_CASE("bson direct - a repeated field round trips", "[willow][bson]") {
    d_person person{};
    for (int i = 0; i < 100; ++i) {
        person.tags.push_back("tag" + std::to_string(i));
    }
    auto back = bson::decode_direct<d_person>(bson::encode_direct(person));
    REQUIRE(back.tags.size() == 100);
    REQUIRE(back.tags[99] == "tag99");
}

TEST_CASE("bson direct - an element type that disagrees with the field is rejected", "[willow][bson]") {
    document mismatched(document_type::object);
    mismatched["age"] = document(std::string("not a number"));
    const auto bytes = bson::encode(mismatched);
    REQUIRE_THROWS_AS(bson::decode_direct<d_person>(bytes), wb::bson_parse_error);
}

TEST_CASE("bson direct - an out of range int32 member is rejected", "[willow][bson]") {
    document64 wide(document_type::object);
    wide["age"] = document64(static_cast<std::int64_t>(0x1'0000'0000));
    const auto bytes = bson::encode(wide);
    REQUIRE_THROWS_AS(bson::decode_direct<d_person>(bytes), wb::bson_parse_error);
}

TEST_CASE("bson direct - a truncated map is rejected", "[willow][bson]") {
    REQUIRE_THROWS_AS(bson::decode_direct<d_person>(byte_buffer{0x05, 0x00, 0x00, 0x00}), wb::bson_parse_error);
    REQUIRE_THROWS_AS(bson::decode_direct<d_person>(byte_buffer{0x61, 0x00, 0x00, 0x00}), wb::bson_parse_error);
}

#endif
