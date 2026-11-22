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

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::bson;

TEST_CASE("bson roundtrip - an empty document is five bytes", "[willow][bson]") {
    document empty(document_type::object);
    REQUIRE(bson::encode(empty) == byte_buffer{0x05, 0x00, 0x00, 0x00, 0x00});
}

TEST_CASE("bson roundtrip - a string matches the official encoding", "[willow][bson]") {
    document src(document_type::object);
    src["hello"] = document(std::string("world"));
    const byte_buffer expected{0x16, 0x00, 0x00, 0x00, 0x02, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00,
                               0x06, 0x00, 0x00, 0x00, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x00, 0x00};
    REQUIRE(bson::encode(src) == expected);
}

TEST_CASE("bson roundtrip - a small integer uses int32", "[willow][bson]") {
    document src(document_type::object);
    src["a"] = document(42);
    const byte_buffer expected{0x0c, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00};
    REQUIRE(bson::encode(src) == expected);
}

TEST_CASE("bson roundtrip - a wide integer uses int64", "[willow][bson]") {
    document64 src(document_type::object);
    src["a"] = document64(static_cast<std::int64_t>(0x1'0000'0000));
    const auto bytes = bson::encode(src);
    REQUIRE(bytes[4] == 0x12);
    auto back = bson::decode<document64>(bytes);
    REQUIRE(back["a"].as_integer() == 0x1'0000'0000);
}

TEST_CASE("bson roundtrip - scalars survive a decode", "[willow][bson]") {
    document src(document_type::object);
    src["int"] = document(-7);
    src["float"] = document(3.5);
    src["bool"] = document(true);
    src["text"] = document(std::string("hello"));
    src["nothing"] = document(document_type::null);

    auto back = bson::decode(bson::encode(src));
    REQUIRE(back["int"].as_integer() == -7);
    REQUIRE(back["float"].as_float() == 3.5);
    REQUIRE(back["bool"].as_bool());
    REQUIRE(back["text"].as_string() == "hello");
    REQUIRE(back["nothing"].is_null());
}

TEST_CASE("bson roundtrip - nested documents and arrays survive", "[willow][bson]") {
    document tags(document_type::array);
    tags.push_back(document(std::string("alpha")));
    tags.push_back(document(std::string("beta")));
    document child(document_type::object);
    child["deep"] = document(std::string("value"));
    document src(document_type::object);
    src["tags"] = tags;
    src["child"] = child;

    auto back = bson::decode(bson::encode(src));
    REQUIRE(back["tags"][0].as_string() == "alpha");
    REQUIRE(back["tags"][1].as_string() == "beta");
    REQUIRE(back["child"]["deep"].as_string() == "value");
}

TEST_CASE("bson roundtrip - a decode re-encode is byte identical", "[willow][bson]") {
    document src(document_type::object);
    src["name"] = document(std::string("rainy"));
    src["age"] = document(32);
    src["ratio"] = document(1.5);
    src["flag"] = document(true);
    document array(document_type::array);
    array.push_back(document(1));
    array.push_back(document(2));
    src["nums"] = array;

    const auto bytes = bson::encode(src);
    REQUIRE(bson::encode(bson::decode(bytes)) == bytes);
}

TEST_CASE("bson roundtrip - an empty string round trips", "[willow][bson]") {
    document src(document_type::object);
    src["empty"] = document(std::string(""));
    auto back = bson::decode(bson::encode(src));
    REQUIRE(back["empty"].as_string().empty());
}

TEST_CASE("bson roundtrip - wide characters round trip", "[willow][bson]") {
    document src(document_type::object);
    src["name"] = document(std::string("rainy"));
    const auto bytes = bson::encode(src);
    auto back = bson::decode(bytes);
    REQUIRE(back["name"].as_string() == "rainy");
}
