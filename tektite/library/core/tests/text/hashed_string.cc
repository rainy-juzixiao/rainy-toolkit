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
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/text/hashed_string.hpp>
#include <string>

using namespace rainy::core::text;

TEST_CASE("hashed_string - default construction", "[hashed_string][construction]") {
    hashed_string hs;
    REQUIRE(hs.hash_code() == 0);
    REQUIRE(hs.data() == nullptr);
    REQUIRE(hs.size() == 0);
}

TEST_CASE("hashed_string - construction from string literal", "[hashed_string][construction]") {
    hashed_string hs{"hello"};
    REQUIRE(hs.hash_code() != 0);
    REQUIRE(hs.size() == 5);
    REQUIRE(std::string(hs.data()) == "hello");
}

TEST_CASE("hashed_string - same string produces same hash", "[hashed_string][hash]") {
    hashed_string hs1{"hello"};
    hashed_string hs2{"hello"};
    REQUIRE(hs1.hash_code() == hs2.hash_code());
    REQUIRE(hs1 == hs2);
}

TEST_CASE("hashed_string - different strings produce different hashes", "[hashed_string][hash]") {
    hashed_string hs1{"hello"};
    hashed_string hs2{"world"};
    REQUIRE(hs1.hash_code() != hs2.hash_code());
    REQUIRE(hs1 != hs2);
}

TEST_CASE("hashed_string - copy semantics", "[hashed_string][copy]") {
    hashed_string hs1{"test"};
    hashed_string hs2{hs1};
    REQUIRE(hs1 == hs2);
    REQUIRE(hs1.hash_code() == hs2.hash_code());
    REQUIRE(hs1.size() == hs2.size());
}

TEST_CASE("hashed_string - move semantics", "[hashed_string][move]") {
    hashed_string hs1{"test"};
    std::size_t original_hash = hs1.hash_code();
    std::size_t original_size = hs1.size();

    hashed_string hs2{std::move(hs1)};
    REQUIRE(hs2.hash_code() == original_hash);
    REQUIRE(hs2.size() == original_size);
    REQUIRE(hs1.hash_code() == 0);
    REQUIRE(hs1.size() == 0);
}

TEST_CASE("hashed_string - conversion to const_pointer", "[hashed_string][conversion]") {
    hashed_string hs{"data"};
    const char *ptr = hs;
    REQUIRE(std::string(ptr) == "data");
}

TEST_CASE("hashed_string - make_hashed_string", "[hashed_string][static]") {
    auto hs = hashed_string::make_hashed_string("static");
    REQUIRE(hs.size() == 6);
    REQUIRE(std::string(hs.data()) == "static");
}

TEST_CASE("hashed_string - eval_hash", "[hashed_string][static]") {
    hashed_string hs{"evaluate"};
    auto hash_val = hashed_string::eval_hash(hs);
    REQUIRE(hash_val == hs.hash_code());
}

TEST_CASE("hashed_string - empty string", "[hashed_string][edge]") {
    hashed_string hs{""};
    REQUIRE(hs.size() == 0);
    REQUIRE(hs.hash_code() != 0);
}

TEST_CASE("hashed_string - long string", "[hashed_string][edge]") {
    std::string long_str(1000, 'a');
    hashed_string hs{long_str.c_str()};
    REQUIRE(hs.size() == 1000);
    REQUIRE(hs.hash_code() != 0);
}

TEST_CASE("hashed_string - equality operator", "[hashed_string][comparison]") {
    hashed_string a{"same"};
    hashed_string b{"same"};
    hashed_string c{"different"};

    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a != b);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("hashed_string - max_size computation", "[hashed_string][capacity]") {
    hashed_string hs;
    REQUIRE(hs.max_size() > 0);
}