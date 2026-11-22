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
#include <rainy/foundation/willow/yaml.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::yaml;
using rainy::foundation::exceptions::willow::yaml::yaml_scalar_error;

TEST_CASE("yaml escape - control character escapes", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("\t")").as_string() == "\t");
    REQUIRE(yaml::parse(R"("\n")").as_string() == "\n");
    REQUIRE(yaml::parse(R"("\r")").as_string() == "\r");
    REQUIRE(yaml::parse(R"("\a")").as_string() == "\a");
    REQUIRE(yaml::parse(R"("\b")").as_string() == "\b");
    REQUIRE(yaml::parse(R"("\v")").as_string() == "\v");
    REQUIRE(yaml::parse(R"("\f")").as_string() == "\f");
    REQUIRE(yaml::parse(R"("\e")").as_string() == "\x1B");
}

TEST_CASE("yaml escape - literal escapes", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("\\")").as_string() == "\\");
    REQUIRE(yaml::parse(R"("\"")").as_string() == "\"");
    REQUIRE(yaml::parse(R"("\/")").as_string() == "/");
    REQUIRE(yaml::parse(R"("\ ")").as_string() == " ");

    const auto null_char = yaml::parse(R"("\0")").as_string();
    REQUIRE(null_char.size() == 1);
    REQUIRE(null_char[0] == '\0');
}

TEST_CASE("yaml escape - hex and unicode escapes", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("\x41")").as_string() == "A");
    REQUIRE(yaml::parse(R"("A")").as_string() == "A");
    REQUIRE(yaml::parse(R"("\U00000041")").as_string() == "A");

    REQUIRE(yaml::parse(R"("é")").as_string() == "\xC3\xA9");
    REQUIRE(yaml::parse(R"("😀")").as_string() == "\xF0\x9F\x98\x80");
    REQUIRE(yaml::parse(R"("𐀀")").as_string() == "\xF0\x90\x80\x80");
}

TEST_CASE("yaml escape - surrogate pairs", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("😀")").as_string() == "\xF0\x9F\x98\x80");
    REQUIRE(yaml::parse(R"("𐀀")").as_string() == "\xF0\x90\x80\x80");
    REQUIRE(yaml::parse(R"("aéb")").as_string() == "a\xC3\xA9" "b");
}

TEST_CASE("yaml escape - unicode line and space escapes", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("\N")").as_string() == "\xC2\x85");
    REQUIRE(yaml::parse(R"("\_")").as_string() == "\xC2\xA0");
    REQUIRE(yaml::parse(R"("\L")").as_string() == "\xE2\x80\xA8");
    REQUIRE(yaml::parse(R"("\P")").as_string() == "\xE2\x80\xA9");
}

TEST_CASE("yaml escape - unknown escape keeps the character", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("\q")").as_string() == "q");
    REQUIRE(yaml::parse(R"("\z")").as_string() == "z");
}

TEST_CASE("yaml escape - escapes inside larger scalars", "[willow][yaml][escape]") {
    REQUIRE(yaml::parse(R"("a\tb")").as_string() == "a\tb");
    REQUIRE(yaml::parse(R"("a\nb")").as_string() == "a\nb");
    REQUIRE(yaml::parse("key: \"a\\tb\"\n")["key"].as_string() == "a\tb");
    REQUIRE(yaml::parse("\"\\u00e9tude\"").as_string() == "\xC3\xA9tude");
}

TEST_CASE("yaml escape - unterminated quoted scalar", "[willow][yaml][escape]") {
    REQUIRE_THROWS_AS(yaml::parse(R"("x\)"), yaml_scalar_error);
}

TEST_CASE("yaml escape - malformed escapes", "[willow][yaml][escape]") {
    REQUIRE_THROWS_AS(yaml::parse(R"("\xZZ")"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse(R"("\x4")"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse(R"("\u12")"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse(R"("\uD800")"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse(R"("\uD800x")"), yaml_scalar_error);
    REQUIRE_THROWS_AS(yaml::parse(R"("\uD800\uD800")"), yaml_scalar_error);
}

TEST_CASE("yaml escape - escaped scalars round trip", "[willow][yaml][escape]") {
    const char *sources[] = {R"("a\tb")", R"("tab\there")", R"("quote\"inside")", R"("back\\slash")", R"("étude")"};
    for (const char *source: sources) {
        const yaml::facade<document> parsed = yaml::parse(source);
        REQUIRE(yaml::parse(yaml::dump(parsed)) == parsed);
    }
}
