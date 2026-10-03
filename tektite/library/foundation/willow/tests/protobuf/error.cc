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

struct err_msg {
    static constexpr rainy::core::container::tuple<field<"a", proto::int32, 1>, field<"s", proto::string, 2>>
        protobuf_fields{};
};

TEST_CASE("protobuf error - truncated inputs throw parse error", "[willow][protobuf]") {
    protobuf::byte_buffer empty{};
    REQUIRE_NOTHROW(protobuf::decode<err_msg>(empty));
    protobuf::byte_buffer cut_tag{0x08};
    REQUIRE_THROWS(protobuf::decode<err_msg>(cut_tag.data(), cut_tag.size()));
    protobuf::byte_buffer cut_varint{0x08, 0x80};
    REQUIRE_THROWS(protobuf::decode<err_msg>(cut_varint.data(), cut_varint.size()));
    protobuf::byte_buffer cut_len{0x12, 0x07, 0x61, 0x62};
    REQUIRE_THROWS(protobuf::decode<err_msg>(cut_len.data(), cut_len.size()));
    protobuf::byte_buffer bad_tag{0x00};
    REQUIRE_THROWS(protobuf::decode<err_msg>(bad_tag.data(), bad_tag.size()));
    protobuf::byte_buffer bad_wire{0x0C, 0x01};
    REQUIRE_THROWS(protobuf::decode<err_msg>(bad_wire.data(), bad_wire.size()));
}

TEST_CASE("protobuf error - type mismatches throw", "[willow][protobuf]") {
    document wrong(document_type::object);
    wrong["a"] = document(std::string("not-int"));
    REQUIRE_THROWS(protobuf::encode<err_msg>(wrong));
    document not_object(document_type::array);
    REQUIRE_THROWS(protobuf::encode<err_msg>(not_object));
    protobuf::byte_buffer wrong_wire{0x0D, 0x01, 0x00, 0x00, 0x00};
    REQUIRE_THROWS(protobuf::decode<err_msg>(wrong_wire.data(), wrong_wire.size()));
}

#endif
