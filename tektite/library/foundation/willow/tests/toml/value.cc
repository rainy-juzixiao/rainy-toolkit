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
#include <rainy/foundation/willow/toml.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::toml;

TEST_CASE("toml value - scalar types", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "i = 42\n"
        "neg = -7\n"
        "f = 3.14\n"
        "exp = 1e10\n"
        "b = true\n"
        "n = false\n"
        "s = \"hello\"\n");

    REQUIRE(doc.is_object());
    REQUIRE(doc["i"].is_integer());
    REQUIRE(doc["i"].as_integer() == 42);
    REQUIRE(doc["neg"].as_integer() == -7);
    REQUIRE(doc["f"].is_float());
    REQUIRE(doc["f"].as_float() == 3.14);
    REQUIRE(doc["exp"].as_float() == 1e10);
    REQUIRE(doc["b"].is_bool());
    REQUIRE(doc["b"].as_bool());
    REQUIRE_FALSE(doc["n"].as_bool());
    REQUIRE(doc["s"].is_string());
    REQUIRE(doc["s"].as_string() == "hello");
}

TEST_CASE("toml value - integer bases and separators", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "hex = 0xff\n"
        "oct = 0o17\n"
        "bin = 0b1010\n"
        "sep = 1_000_000\n");

    REQUIRE(doc["hex"].as_integer() == 255);
    REQUIRE(doc["oct"].as_integer() == 15);
    REQUIRE(doc["bin"].as_integer() == 10);
    REQUIRE(doc["sep"].as_integer() == 1000000);
}

TEST_CASE("toml value - float specials", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "inf = inf\n"
        "ninf = -inf\n"
        "nan = nan\n"
        "sep = 1_0.5_5\n");

    REQUIRE(std::isinf(doc["inf"].as_float()));
    REQUIRE(doc["inf"].as_float() > 0);
    REQUIRE(doc["ninf"].as_float() < 0);
    REQUIRE(std::isnan(doc["nan"].as_float()));
    REQUIRE(doc["sep"].as_float() == 10.55);
}

TEST_CASE("toml value - strings", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "basic = \"a\\tb\\n\"\n"
        "literal = 'c:\\path'\n"
        "ml = \"\"\"\nline1\nline2\n\"\"\"\n"
        "mll = '''\nraw\n'''\n"
        "esc = \"\\u0041\"\n");

    REQUIRE(doc["basic"].as_string() == "a\tb\n");
    REQUIRE(doc["literal"].as_string() == "c:\\path");
    REQUIRE(doc["ml"].as_string() == "line1\nline2\n");
    REQUIRE(doc["mll"].as_string() == "raw\n");
    REQUIRE(doc["esc"].as_string() == "A");
}

TEST_CASE("toml value - tables and nesting", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "title = \"root\"\n"
        "[server]\n"
        "host = \"localhost\"\n"
        "port = 8080\n"
        "[server.limits]\n"
        "max = 10\n");

    REQUIRE(doc["title"].as_string() == "root");
    REQUIRE(doc["server"].is_object());
    REQUIRE(doc["server"]["host"].as_string() == "localhost");
    REQUIRE(doc["server"]["port"].as_integer() == 8080);
    REQUIRE(doc["server"]["limits"]["max"].as_integer() == 10);
}

TEST_CASE("toml value - dotted keys", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "a.b.c = 1\n"
        "a.b.d = 2\n");

    REQUIRE(doc["a"]["b"]["c"].as_integer() == 1);
    REQUIRE(doc["a"]["b"]["d"].as_integer() == 2);
}

TEST_CASE("toml value - arrays", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "ints = [1, 2, 3]\n"
        "strs = [\"a\", \"b\"]\n"
        "nested = [[1, 2], [3, 4]]\n"
        "trailing = [1, 2,]\n"
        "multiline = [\n  1,\n  2,\n]\n");

    REQUIRE(doc["ints"].is_array());
    REQUIRE(doc["ints"].size() == 3);
    REQUIRE(doc["ints"][1].as_integer() == 2);
    REQUIRE(doc["strs"][0].as_string() == "a");
    REQUIRE(doc["nested"][1][0].as_integer() == 3);
    REQUIRE(doc["trailing"].size() == 2);
    REQUIRE(doc["multiline"].size() == 2);
}

TEST_CASE("toml value - inline tables", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "point = { x = 1, y = 2 }\n"
        "empty = {}\n"
        "nested = { a = { b = 3 } }\n");

    REQUIRE(doc["point"].is_object());
    REQUIRE(doc["point"]["x"].as_integer() == 1);
    REQUIRE(doc["point"]["y"].as_integer() == 2);
    REQUIRE(doc["empty"].is_object());
    REQUIRE(doc["empty"].empty());
    REQUIRE(doc["nested"]["a"]["b"].as_integer() == 3);
}

TEST_CASE("toml value - array of tables", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "[[fruit]]\n"
        "name = \"apple\"\n"
        "[[fruit]]\n"
        "name = \"banana\"\n");

    REQUIRE(doc["fruit"].is_array());
    REQUIRE(doc["fruit"].size() == 2);
    REQUIRE(doc["fruit"][0]["name"].as_string() == "apple");
    REQUIRE(doc["fruit"][1]["name"].as_string() == "banana");
}

TEST_CASE("toml value - comments and blank lines", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "# a comment\n"
        "\n"
        "key = 1 # trailing comment\n"
        "\n"
        "# another\n"
        "other = 2\n");

    REQUIRE(doc.size() == 2);
    REQUIRE(doc["key"].as_integer() == 1);
    REQUIRE(doc["other"].as_integer() == 2);
}

TEST_CASE("toml value - empty input", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("");
    REQUIRE(doc.is_object());
    REQUIRE(doc.empty());
}

TEST_CASE("toml value - quoted keys", "[willow][toml]") {
    toml::facade<document> doc = toml::parse(
        "\"a b\" = 1\n"
        "'c.d' = 2\n"
        "\"\" = 3\n");

    REQUIRE(doc["a b"].as_integer() == 1);
    REQUIRE(doc["c.d"].as_integer() == 2);
    REQUIRE(doc[""].as_integer() == 3);
}
