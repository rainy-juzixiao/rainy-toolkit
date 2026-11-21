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
#include <rainy/foundation/willow/hjson.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;

TEST_CASE("hjson value - empty input is an empty object", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("");
    REQUIRE(doc.is_object());
    REQUIRE(doc.empty());
    REQUIRE(doc.size() == 0);
}

TEST_CASE("hjson value - comment-only input is an empty object", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("# only comment\n// another\n/* block */\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.empty());
}

TEST_CASE("hjson value - quoted scalars", "[willow][hjson][value]") {
    REQUIRE(hjson::parse("null").is_null());
    REQUIRE(hjson::parse("true").as_bool());
    REQUIRE(!hjson::parse("false").as_bool());
    REQUIRE(hjson::parse("-123").as_integer() == -123);
    REQUIRE(hjson::parse("42.5").as_float() == 42.5);
    REQUIRE(hjson::parse("0.1e-10").is_float());
    REQUIRE(hjson::parse("7e+5").as_float() == 700000.0);
    REQUIRE(hjson::parse("\"str\"").as_string() == "str");
    REQUIRE(hjson::parse("'str'").as_string() == "str");
    REQUIRE(hjson::parse(R"("a\"b")").as_string() == "a\"b");
    REQUIRE(hjson::parse(R"("a\nb")").as_string() == "a\nb");
}

TEST_CASE("hjson value - unbraced root object", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("host: example.com\nport: 8080\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 2);
    REQUIRE(doc["host"].as_string() == "example.com");
    REQUIRE(doc["port"].as_integer() == 8080);
}

TEST_CASE("hjson value - braced root object", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse(R"({host: example.com, port: 8080})");
    REQUIRE(doc["host"].as_string() == "example.com");
    REQUIRE(doc["port"].as_integer() == 8080);
}

TEST_CASE("hjson value - comma is optional and trailing commas are ignored", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("{\n  one: 1\n  two: 2,\n  more: [3,4,5,]\n  trailing: 6,\n}");
    REQUIRE(doc["one"].as_integer() == 1);
    REQUIRE(doc["two"].as_integer() == 2);
    REQUIRE(doc["more"].is_array());
    REQUIRE(doc["more"].size() == 3);
    REQUIRE(doc["more"][2].as_integer() == 5);
    REQUIRE(doc["trailing"].as_integer() == 6);
}

TEST_CASE("hjson value - quoteless string values", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: 5 times\nb: hello world\nc: plain/text\n");
    REQUIRE(doc["a"].as_string() == "5 times");
    REQUIRE(doc["b"].as_string() == "hello world");
    REQUIRE(doc["c"].as_string() == "plain/text");
}

TEST_CASE("hjson value - comment markers terminate quoteless values", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: v1 # tail\nb: v2 // tail\n");
    REQUIRE(doc["a"].as_string() == "v1");
    REQUIRE(doc["b"].as_string() == "v2");
}

TEST_CASE("hjson value - quoteless value is classified", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: true\nb: false\nc: null\nd: 42\ne: -1.5\nf: v1.2\n");
    REQUIRE(doc["a"].is_bool());
    REQUIRE(doc["a"].as_bool());
    REQUIRE(doc["b"].is_bool());
    REQUIRE(!doc["b"].as_bool());
    REQUIRE(doc["c"].is_null());
    REQUIRE(doc["d"].is_integer());
    REQUIRE(doc["d"].as_integer() == 42);
    REQUIRE(doc["e"].is_float());
    REQUIRE(doc["e"].as_float() == -1.5);
    REQUIRE(doc["f"].as_string() == "v1.2");
}

TEST_CASE("hjson value - multiline string", "[willow][hjson][value]") {
    hjson::facade<document> doc =
        hjson::parse("haiku:\n  '''\n  My half empty glass,\n  I will fill your empty half.\n  Now you are half full.\n  '''\n");
    REQUIRE(doc["haiku"].as_string() == "My half empty glass,\nI will fill your empty half.\nNow you are half full.");
}

TEST_CASE("hjson value - multiline string keeps extra indentation", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("text:\n  '''\n  first\n    indented\n  '''\n");
    REQUIRE(doc["text"].as_string() == "first\n  indented");
}

TEST_CASE("hjson value - comments in all styles", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("# leading\na: 1 # trailing\nb: 2 // trailing\n/* block */ c: 3\nd: 4 /* mid */\n");
    REQUIRE(doc.size() == 4);
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_integer() == 2);
    REQUIRE(doc["c"].as_integer() == 3);
    REQUIRE(doc["d"].as_integer() == 4);
}

TEST_CASE("hjson value - quoted keys", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse(R"({"test case": 1, "{option}": 2, "": 3})");
    REQUIRE(doc["test case"].as_integer() == 1);
    REQUIRE(doc["{option}"].as_integer() == 2);
    REQUIRE(doc[""].as_integer() == 3);
}

TEST_CASE("hjson value - nested containers", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("{a: {b: [1, {c: d}]}, e: 2}");
    REQUIRE(doc["a"]["b"][0].as_integer() == 1);
    REQUIRE(doc["a"]["b"][1]["c"].as_string() == "d");
    REQUIRE(doc["e"].as_integer() == 2);
}

TEST_CASE("hjson value - empty containers", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: {}\nb: []");
    REQUIRE(doc["a"].is_object());
    REQUIRE(doc["a"].empty());
    REQUIRE(doc["b"].is_array());
    REQUIRE(doc["b"].empty());
}

TEST_CASE("hjson value - root array", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("[1, 2, three]");
    REQUIRE(doc.is_array());
    REQUIRE(doc.size() == 3);
    REQUIRE(doc[0].as_integer() == 1);
    REQUIRE(doc[2].as_string() == "three");
}

TEST_CASE("hjson value - duplicate keys overwrite", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("k: 1\nk: 2\n");
    REQUIRE(doc["k"].as_integer() == 2);
}

TEST_CASE("hjson value - windows line endings", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: 1\r\nb: two\r\n");
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_string() == "two");
}

TEST_CASE("hjson value - string literal suffix operator", "[willow][hjson][value]") {
    const auto doc = "a: 1\nb: x\n"_hjson;
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_string() == "x");
}

TEST_CASE("hjson value - type predicates reflect the parsed value", "[willow][hjson][value]") {
    const hjson::facade<document> doc = hjson::parse("i: 1\nf: 1.5\ns: x\nb: true\nn: null\no: {}\na: []\n");
    REQUIRE(doc["i"].is_integer());
    REQUIRE(doc["i"].is_number());
    REQUIRE(doc["f"].is_float());
    REQUIRE(doc["f"].is_number());
    REQUIRE(doc["s"].is_string());
    REQUIRE(doc["b"].is_bool());
    REQUIRE(doc["n"].is_null());
    REQUIRE(doc["o"].is_object());
    REQUIRE(doc["a"].is_array());
    REQUIRE(doc["s"].is_primitive());
    REQUIRE(!doc["o"].is_primitive());
    REQUIRE(doc["i"].type_name() == "integer");
    REQUIRE(doc["f"].type_name() == "float");
}

TEST_CASE("hjson value - contains and count", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: 1\nb: 2\n");
    REQUIRE(doc.contains("a"));
    REQUIRE(doc.contains("b"));
    REQUIRE(!doc.contains("c"));
    REQUIRE(doc.count("a") == 1);
    REQUIRE(doc.count("c") == 0);
}

TEST_CASE("hjson value - erase by key", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("a: 1\nb: 2\nc: 3\n");
    REQUIRE(doc.erase("b") == 1);
    REQUIRE(doc.size() == 2);
    REQUIRE(!doc.contains("b"));
    REQUIRE(doc.erase("b") == 0);
}

TEST_CASE("hjson value - erase by array index", "[willow][hjson][value]") {
    hjson::facade<document> doc = hjson::parse("[1, 2, 3]\n");
    doc.erase(1);
    REQUIRE(doc.size() == 2);
    REQUIRE(doc[0].as_integer() == 1);
    REQUIRE(doc[1].as_integer() == 3);
}

TEST_CASE("hjson value - iteration visits every member", "[willow][hjson][value]") {
    const hjson::facade<document> doc = hjson::parse("a: 1\nb: 2\nc: 3\n");
    std::size_t count = 0;
    for (auto it = doc.begin(); it != doc.end(); ++it) {
        ++count;
    }
    REQUIRE(count == 3);
}

TEST_CASE("hjson value - conversion from another document", "[willow][hjson][value]") {
    const hjson::facade<document> source = hjson::parse("a: 1\nb: [x, y]\n");
    const hjson::facade<document> converted{from_other_document, source};
    REQUIRE(converted == source);
}

TEST_CASE("hjson value - document64 uses wider integers", "[willow][hjson][value]") {
    const auto doc = hjson::parse<document64>("a: 5000000000\nb: -5000000000\n");
    REQUIRE(doc["a"].as_integer() == 5000000000);
    REQUIRE(doc["b"].as_integer() == -5000000000);
}

TEST_CASE("hjson value - wdocument64 uses wider integers", "[willow][hjson][value]") {
    const auto doc = hjson::parse<wdocument64>(L"a: 5000000000\n");
    REQUIRE(doc[L"a"].as_integer() == 5000000000);
}
