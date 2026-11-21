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
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

using namespace rainy::foundation::willow::protobuf;

TEST_CASE("protobuf version - compatibility version matches header", "[willow][protobuf][version]") {
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_VERSION_MAJOR == 6);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_VERSION ==
                   RAINY_WILLOW_PROTOBUF_VERSION_MAJOR * 1000000 + RAINY_WILLOW_PROTOBUF_VERSION_MINOR * 1000 +
                       RAINY_WILLOW_PROTOBUF_VERSION_PATCH);
    STATIC_REQUIRE(protobuf_version::major == RAINY_WILLOW_PROTOBUF_VERSION_MAJOR);
    STATIC_REQUIRE(protobuf_version::value == RAINY_WILLOW_PROTOBUF_VERSION);
    STATIC_REQUIRE(protobuf_version::target == RAINY_WILLOW_PROTOBUF_TARGET_VERSION);
    REQUIRE(protobuf_version::target >= RAINY_WILLOW_PROTOBUF_VERSION_V3_0);
}

TEST_CASE("protobuf version - syntax and feature gates are consistent", "[willow][protobuf][version]") {
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_PROTO2 == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_PACKED == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_GROUPS == 0);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_DIRECT_CODEC == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_DOCUMENT_CODEC == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_DESCRIPTOR_POOL == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_UNKNOWN_FIELDS == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_GET == 1);
#if RAINY_WILLOW_PROTOBUF_TARGET_VERSION >= RAINY_WILLOW_PROTOBUF_VERSION_V3_12
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_MUTATE == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_PROTO3_OPTIONAL == 1);
#endif
#if RAINY_WILLOW_PROTOBUF_TARGET_VERSION >= RAINY_WILLOW_PROTOBUF_VERSION_V5_29
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_EDITIONS == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_MESSAGE_VIEW == 1);
#endif
    REQUIRE(static_cast<int>(proto_syntax::proto2) == 0);
    REQUIRE(static_cast<int>(proto_syntax::editions) == 2);
}

TEST_CASE("protobuf version - standard path selection prefers cxx20", "[willow][protobuf][version]") {
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_CXX17_FIELDS == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_FIELDS_AVAILABLE == 1);
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_AVAILABLE == 1);
#if RAINY_HAS_CXX20
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_CXX20_FIELDS == 1);
#else
    STATIC_REQUIRE(RAINY_WILLOW_PROTOBUF_HAS_CXX20_FIELDS == 0);
#endif
}

#endif
