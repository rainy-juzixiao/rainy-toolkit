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

#include <cstdint>
#include <string>
#include <vector>

#if RAINY_HAS_CXX20

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::cbor;

namespace wc = rainy::foundation::exceptions::willow::cbor;

namespace {
struct d_phone {
    std::string number{};

    static constexpr rainy::core::container::tuple<member_field<"number", types::text_string, &d_phone::number>> cbor_fields{};
};

struct d_person {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    d_phone phone{};

    static constexpr rainy::core::container::tuple<member_field<"name", types::text_string, &d_person::name>,
                                                   member_field<"age", types::int32, &d_person::age>,
                                                   member_field<"tags", types::repeated<types::text_string>, &d_person::tags>,
                                                   member_field<"phone", types::message<d_phone>, &d_person::phone>>
        cbor_fields{};
};

struct d_scalars {
    std::int32_t i32{};
    std::uint32_t u32{};
    std::int64_t i64{};
    std::uint64_t u64{};
    double d{};
    bool flag{};
    std::string s{};
    std::string b{};

    static constexpr rainy::core::container::tuple<member_field<"i32", types::int32, &d_scalars::i32>,
                                                   member_field<"u32", types::uint32, &d_scalars::u32>,
                                                   member_field<"i64", types::int64, &d_scalars::i64>,
                                                   member_field<"u64", types::uint64, &d_scalars::u64>,
                                                   member_field<"d", types::double_, &d_scalars::d>,
                                                   member_field<"flag", types::boolean, &d_scalars::flag>,
                                                   member_field<"s", types::text_string, &d_scalars::s>,
                                                   member_field<"b", types::byte_string, &d_scalars::b>>
        cbor_fields{};
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
        doc.u32 = 200;
        doc.i64 = -4294967296LL;
        doc.u64 = 18446744073709551615ULL;
        doc.d = 1.5;
        doc.flag = true;
        doc.s = "hello";
        doc.b = "bytes";
        return doc;
    }
}

TEST_CASE("cbor direct - the concept shape requires member fields", "[willow][cbor]") {
    STATIC_REQUIRE(direct_message_v<d_person>);
    STATIC_REQUIRE(direct_message_v<d_scalars>);
    STATIC_REQUIRE(field_count_v<d_person> == 4);
    STATIC_REQUIRE(valid_field_names_v<d_person>);
}

TEST_CASE("cbor direct - an empty struct encodes to an empty map", "[willow][cbor]") {
    REQUIRE(cbor::encode_direct(d_person{}) == byte_buffer{0xa0});
    REQUIRE(cbor::encode_direct(d_scalars{}) == byte_buffer{0xa0});
    REQUIRE(cbor::decode_direct<d_person>(nullptr, 0).name.empty());
}

TEST_CASE("cbor direct - encoding is a canonical map", "[willow][cbor]") {
    const auto bytes = cbor::encode_direct(make_person());
    REQUIRE(bytes[0] == 0xa4);
    auto back = cbor::decode_direct<d_person>(bytes);
    REQUIRE(back.name == "testing");
    REQUIRE(back.age == 32);
    REQUIRE(back.tags.size() == 2);
    REQUIRE(back.tags[1] == "beta");
    REQUIRE(back.phone.number == "1234");
}

TEST_CASE("cbor direct - scalars round trip", "[willow][cbor]") {
    const auto scalars = make_scalars();
    auto back = cbor::decode_direct<d_scalars>(cbor::encode_direct(scalars));
    REQUIRE(back.i32 == -2);
    REQUIRE(back.u32 == 200);
    REQUIRE(back.i64 == -4294967296LL);
    REQUIRE(back.u64 == 18446744073709551615ULL);
    REQUIRE(back.d == 1.5);
    REQUIRE(back.flag);
    REQUIRE(back.s == "hello");
    REQUIRE(back.b == "bytes");
}

TEST_CASE("cbor direct - a byte string member is distinguished from a text string", "[willow][cbor]") {
    d_scalars doc{};
    doc.s = "text";
    doc.b = "bytes";
    const auto bytes = cbor::encode_direct(doc);
    auto back = cbor::decode_direct<d_scalars>(bytes);
    REQUIRE(back.s == "text");
    REQUIRE(back.b == "bytes");
}

TEST_CASE("cbor direct - an absent field keeps its previous value", "[willow][cbor]") {
    d_person person{};
    person.name = "keep";
    person.age = 7;
    d_person partial{};
    partial.age = 99;
    const auto bytes = cbor::encode_direct(partial);
    cbor::decode_direct_into<d_person>(bytes, person);
    REQUIRE(person.name == "keep");
    REQUIRE(person.age == 99);
}

TEST_CASE("cbor direct - an unknown key is skipped", "[willow][cbor]") {
    document extra(document_type::object);
    extra["name"] = document(std::string("rainy"));
    extra["unknown"] = document(std::string("ignored"));
    extra["age"] = document(5);
    const auto bytes = cbor::encode(extra);
    d_person person = cbor::decode_direct<d_person>(bytes);
    REQUIRE(person.name == "rainy");
    REQUIRE(person.age == 5);
}

TEST_CASE("cbor direct - a repeated field round trips", "[willow][cbor]") {
    d_person person{};
    for (int i = 0; i < 100; ++i) {
        person.tags.push_back("tag" + std::to_string(i));
    }
    auto back = cbor::decode_direct<d_person>(cbor::encode_direct(person));
    REQUIRE(back.tags.size() == 100);
    REQUIRE(back.tags[99] == "tag99");
}

TEST_CASE("cbor direct - an indefinite map is accepted", "[willow][cbor]") {
    byte_buffer bytes{0xbf};
    const auto inner = cbor::encode_direct(make_person());
    bytes.insert(bytes.end(), inner.begin() + 1, inner.end() - 0);
    bytes.push_back(0xff);
    auto back = cbor::decode_direct<d_person>(bytes);
    REQUIRE(back.name == "testing");
    REQUIRE(back.age == 32);
}

TEST_CASE("cbor direct - a value of the wrong kind is rejected", "[willow][cbor]") {
    document mismatched(document_type::object);
    mismatched["age"] = document(std::string("not a number"));
    const auto bytes = cbor::encode(mismatched);
    REQUIRE_THROWS_AS(cbor::decode_direct<d_person>(bytes), wc::cbor_parse_error);
}

TEST_CASE("cbor direct - an out of range member is rejected", "[willow][cbor]") {
    document64 wide(document_type::object);
    wide["age"] = document64(static_cast<std::int64_t>(0x1'0000'0000));
    const auto bytes = cbor::encode(wide);
    REQUIRE_THROWS_AS(cbor::decode_direct<d_person>(bytes), wc::cbor_parse_error);
}

TEST_CASE("cbor direct - a non map input is rejected", "[willow][cbor]") {
    REQUIRE_THROWS_AS(cbor::decode_direct<d_person>(byte_buffer{0x81, 0x01}), wc::cbor_parse_error);
    REQUIRE_THROWS_AS(cbor::decode_direct<d_person>(byte_buffer{0x01}), wc::cbor_parse_error);
}

#endif
