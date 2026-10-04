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
#include <iostream>
#include <string>
#include <vector>

#if RAINY_HAS_CXX20

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::protobuf;

namespace a {
    namespace {
        struct desc_phone : add_descriptor<desc_phone> {
            std::string number{};

            static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &desc_phone::number>>
                protobuf_fields{};
        };
        struct desc_person : add_descriptor<desc_person> {
            std::string name{};
            std::int32_t age{};
            std::vector<std::string> tags{};
            desc_phone phone{};

            static constexpr rainy::core::container::tuple<member_field<"name", proto::string, 1, &desc_person::name>,
                                                           member_field<"age", proto::int32, 2, &desc_person::age>,
                                                           member_field<"tags", proto::repeated<proto::string>, 3, &desc_person::tags>,
                                                           member_field<"phone", proto::message<desc_phone>, 4, &desc_person::phone>>
                protobuf_fields{};
        };
    }
}


namespace {
    struct desc_phone : add_descriptor<desc_phone> {
        std::string number{};

        static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &desc_phone::number>>
            protobuf_fields{};
    };

    struct desc_person : add_descriptor<desc_person> {
        std::string name{};
        std::int32_t age{};
        std::vector<std::string> tags{};
        desc_phone phone{};

        static constexpr rainy::core::container::tuple<member_field<"name", proto::string, 1, &desc_person::name>,
                                                       member_field<"age", proto::int32, 2, &desc_person::age>,
                                                       member_field<"tags", proto::repeated<proto::string>, 3, &desc_person::tags>,
                                                       member_field<"phone", proto::message<desc_phone>, 4, &desc_person::phone>>
            protobuf_fields{};
    };

    struct desc_scalars : add_descriptor<desc_scalars> {
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
            member_field<"i32", proto::int32, 1, &desc_scalars::i32>, member_field<"u32", proto::uint32, 2, &desc_scalars::u32>,
            member_field<"s32", proto::sint32, 3, &desc_scalars::s32>, member_field<"i64", proto::int64, 4, &desc_scalars::i64>,
            member_field<"flag", proto::boolean, 5, &desc_scalars::flag>, member_field<"f32", proto::fixed32, 6, &desc_scalars::f32>,
            member_field<"sf32", proto::sfixed32, 7, &desc_scalars::sf32>, member_field<"f", proto::floating, 8, &desc_scalars::f>,
            member_field<"f64", proto::fixed64, 9, &desc_scalars::f64>, member_field<"sf64", proto::sfixed64, 10, &desc_scalars::sf64>,
            member_field<"d", proto::doubling, 11, &desc_scalars::d>, member_field<"s", proto::string, 12, &desc_scalars::s>,
            member_field<"b", proto::bytes, 13, &desc_scalars::b>>
            protobuf_fields{};
    };

    struct desc_repeated_msg : add_descriptor<desc_repeated_msg> {
        std::vector<desc_phone> phones{};

        static constexpr rainy::core::container::tuple<
            member_field<"phones", proto::repeated<proto::message<desc_phone>>, 1, &desc_repeated_msg::phones>>
            protobuf_fields{};
    };
}

TEST_CASE("protobuf descriptor - field count and indexed access", "[willow][protobuf][descriptor]") {
    auto &d = descriptor<desc_person>::instance();
    REQUIRE(d.field_count() == 4);

    const auto &f0 = d.field(0);
    REQUIRE(f0.name() == "name");
    REQUIRE(f0.number() == 1);
    REQUIRE(f0.kind() == field_kind::string);
    REQUIRE(f0.wire() == wire_type::len);
    REQUIRE(!f0.is_repeated());
    REQUIRE(!f0.is_message());
    REQUIRE(f0.element_kind() == field_kind::none);
    REQUIRE(f0.index() == 0);

    const auto &f1 = d.field(1);
    REQUIRE(f1.name() == "age");
    REQUIRE(f1.number() == 2);
    REQUIRE(f1.kind() == field_kind::int32);
    REQUIRE(f1.wire() == wire_type::varint);

    const auto &f2 = d.field(2);
    REQUIRE(f2.name() == "tags");
    REQUIRE(f2.number() == 3);
    REQUIRE(f2.kind() == field_kind::string);
    REQUIRE(f2.is_repeated());
    REQUIRE(!f2.is_message());
    REQUIRE(f2.element_kind() == field_kind::string);
    REQUIRE(f2.index() == 2);

    const auto &f3 = d.field(3);
    REQUIRE(f3.name() == "phone");
    REQUIRE(f3.number() == 4);
    REQUIRE(f3.kind() == field_kind::message);
    REQUIRE(!f3.is_repeated());
    REQUIRE(f3.is_message());
    REQUIRE(f3.element_kind() == field_kind::none);
    REQUIRE(f3.index() == 3);
}

TEST_CASE("protobuf descriptor - find by name and number", "[willow][protobuf][descriptor]") {
    auto &d = descriptor<desc_person>::instance();

    REQUIRE(d.find_field_by_name("name") != nullptr);
    REQUIRE(d.find_field_by_name("age") != nullptr);
    REQUIRE(d.find_field_by_name("tags") != nullptr);
    REQUIRE(d.find_field_by_name("phone") != nullptr);
    REQUIRE(d.find_field_by_name("missing") == nullptr);

    REQUIRE(d.find_field_by_name("age")->number() == 2);
    REQUIRE(d.find_field_by_name("tags")->is_repeated());
    REQUIRE(d.find_field_by_name("phone")->is_message());

    REQUIRE(d.find_field_by_number(1) != nullptr);
    REQUIRE(d.find_field_by_number(2) != nullptr);
    REQUIRE(d.find_field_by_number(3) != nullptr);
    REQUIRE(d.find_field_by_number(4) != nullptr);
    REQUIRE(d.find_field_by_number(99) == nullptr);

    REQUIRE(d.find_field_by_number(2)->name() == "age");
    REQUIRE(d.find_field_by_number(4)->name() == "phone");
}

TEST_CASE("protobuf descriptor - scalars field metadata", "[willow][protobuf][descriptor]") {
    auto &d = descriptor<desc_scalars>::instance();
    REQUIRE(d.field_count() == 13);

    REQUIRE(d.field(0).name() == "i32");
    REQUIRE(d.field(0).kind() == field_kind::int32);
    REQUIRE(d.field(0).wire() == wire_type::varint);

    REQUIRE(d.field(1).kind() == field_kind::uint32);
    REQUIRE(d.field(2).kind() == field_kind::sint32);
    REQUIRE(d.field(3).kind() == field_kind::int64);
    REQUIRE(d.field(4).kind() == field_kind::boolean);
    REQUIRE(d.field(5).kind() == field_kind::fixed32);
    REQUIRE(d.field(5).wire() == wire_type::i32);
    REQUIRE(d.field(6).kind() == field_kind::sfixed32);
    REQUIRE(d.field(7).kind() == field_kind::floating);
    REQUIRE(d.field(8).kind() == field_kind::fixed64);
    REQUIRE(d.field(8).wire() == wire_type::i64);
    REQUIRE(d.field(9).kind() == field_kind::sfixed64);
    REQUIRE(d.field(10).kind() == field_kind::doubling);
    REQUIRE(d.field(11).kind() == field_kind::string);
    REQUIRE(d.field(12).kind() == field_kind::bytes);

    for (std::size_t i = 0; i < d.field_count(); ++i) {
        REQUIRE(d.field(i).is_repeated() == false);
        REQUIRE(d.field(i).is_message() == false);
        REQUIRE(d.field(i).element_kind() == field_kind::none);
        REQUIRE(d.field(i).index() == i);
    }
}

TEST_CASE("protobuf descriptor - message_name via type_name", "[willow][protobuf][descriptor]") {
    auto &d = descriptor<desc_person>::instance();
    REQUIRE(d.message_name().find("desc_person") != rainy::core::text::string_view::npos);
}

TEST_CASE("protobuf descriptor - nested message_descriptor()", "[willow][protobuf][descriptor]") {
    auto &d = descriptor<desc_person>::instance();
    const auto &f3 = d.field(3);
    REQUIRE(f3.is_message());
    const auto &nested = f3.message_descriptor();
    REQUIRE(nested.field_count() == 1);
    REQUIRE(nested.field_at(0).name() == "number");
    REQUIRE(nested.field_at(0).kind() == field_kind::string);
    REQUIRE(nested.message_name().find("desc_phone") != rainy::core::text::string_view::npos);
}

TEST_CASE("protobuf descriptor - repeated message element_kind", "[willow][protobuf][descriptor]") {
    auto &d = descriptor<desc_repeated_msg>::instance();
    REQUIRE(d.field_count() == 1);
    const auto &f0 = d.field(0);
    REQUIRE(f0.name() == "phones");
    REQUIRE(f0.is_repeated());
    REQUIRE(!f0.is_message());
    REQUIRE(f0.kind() == field_kind::message);
    REQUIRE(f0.element_kind() == field_kind::message);
    REQUIRE(f0.wire() == wire_type::len);
}

TEST_CASE("protobuf descriptor - descriptor_pool registers by message_name", "[willow][protobuf][descriptor]") {
    auto &d_person = descriptor<desc_person>::instance();
    auto &d_phone = descriptor<desc_phone>::instance();
    auto &d_scalars = descriptor<desc_scalars>::instance();
    (void) d_person;
    (void) d_phone;
    (void) d_scalars;

    auto *any_person = descriptor_pool::instance().find_message(d_person.message_name());
    REQUIRE(any_person != nullptr);
    REQUIRE(any_person->message_name() == d_person.message_name());
    REQUIRE(any_person->field_count() == 4);
    REQUIRE(any_person->field_at(0).name() == "name");

    auto *any_phone = descriptor_pool::instance().find_message(d_phone.message_name());
    REQUIRE(any_phone != nullptr);
    REQUIRE(any_phone->field_count() == 1);

    auto *any_scalars = descriptor_pool::instance().find_message(d_scalars.message_name());
    REQUIRE(any_scalars != nullptr);
    REQUIRE(any_scalars->field_count() == 13);

    auto *any_missing = descriptor_pool::instance().find_message("nonexistent_message");
    REQUIRE(any_missing == nullptr);
}

TEST_CASE("protobuf descriptor - CRTP member returns same instance", "[willow][protobuf][descriptor]") {
    desc_person p{};
    const auto &d1 = p.descriptor();
    const auto &d2 = descriptor<desc_person>::instance();
    REQUIRE(&d1 == &d2);
    REQUIRE(d1.field_count() == 4);
}

#endif
