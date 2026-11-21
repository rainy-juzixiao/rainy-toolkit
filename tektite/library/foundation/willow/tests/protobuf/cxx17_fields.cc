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

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::protobuf;

struct m17_phone {
    std::string number{};

    RAINY_WILLOW_PROTOBUF_CXX17_MEMBER_FIELD_DECL(number_field, "number", proto::string, 1, &m17_phone::number);

    static constexpr rainy::core::container::tuple<number_field> protobuf_fields{};
};

struct m17_person {
    std::string name{};
    std::int32_t age{};
    std::vector<std::string> tags{};
    m17_phone phone{};

    RAINY_WILLOW_PROTOBUF_CXX17_MEMBER_FIELD_DECL(name_field, "name", proto::string, 1, &m17_person::name);
    RAINY_WILLOW_PROTOBUF_CXX17_MEMBER_FIELD_DECL(age_field, "age", proto::int32, 2, &m17_person::age);
    RAINY_WILLOW_PROTOBUF_CXX17_MEMBER_FIELD_DECL(tags_field, "tags", proto::repeated<proto::string>, 3,
                                                  &m17_person::tags);
    RAINY_WILLOW_PROTOBUF_CXX17_MEMBER_FIELD_DECL(phone_field, "phone", proto::message<m17_phone>, 4,
                                                  &m17_person::phone);

    static constexpr rainy::core::container::tuple<name_field, age_field, tags_field, phone_field> protobuf_fields{};
};

struct m17_doc {
    RAINY_WILLOW_PROTOBUF_CXX17_FIELD_DECL(name_field, "name", proto::string, 1);
    RAINY_WILLOW_PROTOBUF_CXX17_FIELD_DECL(age_field, "age", proto::int32, 2);

    static constexpr rainy::core::container::tuple<name_field, age_field> protobuf_fields{};
};

TEST_CASE("protobuf cxx17 macro fields - traits accept macro-declared fields", "[willow][protobuf][cxx17]") {
    STATIC_REQUIRE(is_field_v<m17_phone::number_field>);
    STATIC_REQUIRE(is_direct_field_v<m17_phone::number_field>);
    STATIC_REQUIRE(is_field_v<m17_doc::name_field>);
    STATIC_REQUIRE(!is_direct_field_v<m17_doc::name_field>);
    STATIC_REQUIRE(is_protobuf_message_v<m17_phone>);
    STATIC_REQUIRE(is_protobuf_message_v<m17_person>);
    STATIC_REQUIRE(is_protobuf_message_v<m17_doc>);
    STATIC_REQUIRE(is_direct_message_v<m17_phone>);
    STATIC_REQUIRE(is_direct_message_v<m17_person>);
    STATIC_REQUIRE(field_count_v<m17_person> == 4);
    STATIC_REQUIRE(field_number_v<m17_person, 0> == 1);
    STATIC_REQUIRE(field_number_v<m17_person, 3> == 4);
    STATIC_REQUIRE(valid_field_numbers_v<m17_person>);
    STATIC_REQUIRE(field_name_of<m17_person::name_field>() == "name");
    STATIC_REQUIRE(field_name_of<m17_person::age_field>() == "age");
}

TEST_CASE("protobuf cxx17 macro fields - direct codec roundtrip", "[willow][protobuf][cxx17]") {
    m17_person person{};
    person.name = "testing";
    person.age = 32;
    person.tags = {"alpha", "beta"};
    person.phone.number = "1234";
    byte_buffer bytes = encode_direct(person);
    REQUIRE(!bytes.empty());
    m17_person back = decode_direct<m17_person>(bytes.data(), bytes.size());
    REQUIRE(back.name == "testing");
    REQUIRE(back.age == 32);
    REQUIRE(back.tags.size() == 2);
    REQUIRE(back.tags[1] == "beta");
    REQUIRE(back.phone.number == "1234");
}

TEST_CASE("protobuf cxx17 macro fields - document codec roundtrip", "[willow][protobuf][cxx17]") {
    document doc(document_type::object);
    doc["name"] = document(std::string("testing"));
    doc["age"] = document(32);
    byte_buffer bytes = encode<m17_doc>(doc);
    REQUIRE(!bytes.empty());
    document back = decode<m17_doc>(bytes.data(), bytes.size());
    REQUIRE(back["name"].as_string() == "testing");
    REQUIRE(back["age"].as_integer() == 32);
}

TEST_CASE("protobuf cxx17 macro fields - descriptor and reflection", "[willow][protobuf][cxx17]") {
    auto &d = descriptor<m17_person>::instance();
    REQUIRE(d.field_count() == 4);
    REQUIRE(d.find_field_by_name("name") != nullptr);
    REQUIRE(d.find_field_by_name("age")->number() == 2);
    REQUIRE(d.find_field_by_number(4)->name() == "phone");
    m17_person person{};
    person.age = 7;
    reflection<m17_person> refl;
    REQUIRE(refl.get(person, "age").as_int64() == 7);
    REQUIRE(refl.has(person, "age"));
    refl.clear(person, "age");
    REQUIRE(!refl.has(person, 2));
}

#endif
