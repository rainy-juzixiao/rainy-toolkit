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

TEST_CASE("yaml parse - plain scalar typing", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("42\n").is_integer());
    REQUIRE(yaml::parse("42\n").as_integer() == 42);
    REQUIRE(yaml::parse("-7\n").is_integer());
    REQUIRE(yaml::parse("-7\n").as_integer() == -7);
    REQUIRE(yaml::parse("2.5\n").is_float());
    REQUIRE(yaml::parse("2.5\n").as_float() == 2.5);
    REQUIRE(yaml::parse("1e3\n").is_float());
    REQUIRE(yaml::parse("1e3\n").as_float() == 1000.0);
    REQUIRE(yaml::parse("text\n").is_string());
    REQUIRE(yaml::parse("text\n").as_string() == "text");
}

TEST_CASE("yaml parse - integer literal forms", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("0x1F\n").as_integer() == 31);
    REQUIRE(yaml::parse("0o17\n").as_integer() == 15);
    REQUIRE(yaml::parse("1_000\n").as_integer() == 1000);
    REQUIRE(yaml::parse("010\n").as_integer() == 10);
}

TEST_CASE("yaml parse - integer beyond integer_type becomes float", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("2147483648\n").is_float());
}

TEST_CASE("yaml parse - boolean and null keywords", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("true\n").is_bool());
    REQUIRE(yaml::parse("true\n").as_bool());
    REQUIRE(yaml::parse("no\n").is_bool());
    REQUIRE(!yaml::parse("no\n").as_bool());
    REQUIRE(yaml::parse("on\n").as_bool());
    REQUIRE(yaml::parse("~").is_null());
    REQUIRE(yaml::parse("null\n").is_null());
    REQUIRE(yaml::parse("NULL\n").is_null());
}

TEST_CASE("yaml parse - quoted scalars stay strings", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("\"42\"").is_string());
    REQUIRE(yaml::parse("\"42\"").as_string() == "42");
    REQUIRE(yaml::parse("\"true\"").as_string() == "true");
    REQUIRE(yaml::parse("'it''s'").as_string() == "it's");
    REQUIRE(yaml::parse("'a\\b'").as_string() == "a\\b");
}

TEST_CASE("yaml parse - empty input and blank document", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("").is_null());
    REQUIRE(yaml::parse("\n").is_null());
    REQUIRE(yaml::parse("# only a comment\n").is_null());
}

TEST_CASE("yaml parse - block mapping", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("a: 1\nb: text\nc: [1, 2]\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 3);
    REQUIRE(doc.style() == "block");
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_string() == "text");
    REQUIRE(doc["c"].is_array());
    REQUIRE(doc["c"].size() == 2);
    REQUIRE(doc["c"][0].as_integer() == 1);
    REQUIRE(doc["c"][1].as_integer() == 2);
}

TEST_CASE("yaml parse - nested block mapping", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("a:\n  b: 1\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc["a"].is_object());
    REQUIRE(doc["a"]["b"].as_integer() == 1);
}

TEST_CASE("yaml parse - block sequence", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("- 1\n- 2\n");
    REQUIRE(doc.is_array());
    REQUIRE(doc.style() == "block");
    REQUIRE(doc.size() == 2);
    REQUIRE(doc[0].as_integer() == 1);
    REQUIRE(doc[1].as_integer() == 2);
}

TEST_CASE("yaml parse - sequence nested under a mapping key", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("a:\n  - 1\n  - 2\n");
    REQUIRE(doc["a"].is_array());
    REQUIRE(doc["a"].size() == 2);
    REQUIRE(doc["a"][1].as_integer() == 2);
}

TEST_CASE("yaml parse - mapping nested under a sequence entry", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("-\n  a: 1\n");
    REQUIRE(doc.is_array());
    REQUIRE(doc.size() == 1);
    REQUIRE(doc[0].is_object());
    REQUIRE(doc[0]["a"].as_integer() == 1);
}

TEST_CASE("yaml parse - flow collections", "[willow][yaml][parse]") {
    const auto object = yaml::parse("{a: 1, b: [2, 3]}");
    REQUIRE(object.is_object());
    REQUIRE(object.style() == "flow");
    REQUIRE(object["a"].as_integer() == 1);
    REQUIRE(object["b"].is_array());
    REQUIRE(object["b"][1].as_integer() == 3);

    const auto array = yaml::parse("[1, [2, 3]]");
    REQUIRE(array.is_array());
    REQUIRE(array.style() == "flow");
    REQUIRE(array[0].as_integer() == 1);
    REQUIRE(array[1].is_array());
    REQUIRE(array[1][0].as_integer() == 2);
}

TEST_CASE("yaml parse - empty flow collections", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("[]").is_array());
    REQUIRE(yaml::parse("[]").size() == 0);
    REQUIRE(yaml::parse("{}").is_object());
    REQUIRE(yaml::parse("{}").size() == 0);
}

TEST_CASE("yaml parse - mapping key without value is null", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("key:\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc["key"].is_null());
}

TEST_CASE("yaml parse - comments are skipped", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("# leading comment\na: 1 # trailing comment\n");
    REQUIRE(doc.size() == 1);
    REQUIRE(doc["a"].as_integer() == 1);

    REQUIRE(yaml::parse("a#b\n").as_string() == "a#b");
    REQUIRE(yaml::parse("a:b\n").as_string() == "a:b");
}

TEST_CASE("yaml parse - document start marker", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("---\na: 1\n");
    REQUIRE(doc["a"].as_integer() == 1);
}

TEST_CASE("yaml parse - directive is captured as metadata", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("%YAML 1.2\n---\na: 1\n");
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc.has_directive());
    REQUIRE(doc.directive() == "YAML 1.2");
}

TEST_CASE("yaml parse - anchor is captured as metadata", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("&a 1\n");
    REQUIRE(doc.is_integer());
    REQUIRE(doc.has_anchor());
    REQUIRE(doc.anchor() == "a");
}

TEST_CASE("yaml parse - tag is captured as metadata", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("!!str 42\n");
    REQUIRE(doc.has_tag());
    REQUIRE(doc.tag() == "!str");

    const auto named = yaml::parse("!custom text\n");
    REQUIRE(named.has_tag());
    REQUIRE(named.tag() == "custom");
}

TEST_CASE("yaml parse - alias keeps anchor and alias markers", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("a: &x 1\nb: *x\n");
    REQUIRE(doc.is_object());
    REQUIRE(doc.size() == 2);
    REQUIRE(yaml::dump(doc).find("&x") != rainy::core::text::string::npos);
    REQUIRE(yaml::dump(doc).find("*x") != rainy::core::text::string::npos);
}

TEST_CASE("yaml parse - alias resolves the anchored value", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("a: &x 1\nb: *x\nc: *x\n");
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_integer() == 1);
    REQUIRE(doc["c"].as_integer() == 1);

    const auto nested = yaml::parse("outer: &box\n  k: 1\nuser: *box\nmore: *box\nlast: *box\n");
    REQUIRE(nested["user"]["k"].as_integer() == 1);
    REQUIRE(nested["more"]["k"].as_integer() == 1);
    REQUIRE(nested["last"]["k"].as_integer() == 1);

    const auto arr = yaml::parse("a: &seq\n  - 1\n  - 2\nb: *seq\n");
    REQUIRE(arr["b"].is_array());
    REQUIRE(arr["b"].size() == 2);
    REQUIRE(arr["b"][1].as_integer() == 2);
}

TEST_CASE("yaml parse - literal block scalar", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("a: |\n  line1\n  line2\n")["a"].as_string() == "line1\nline2\n");
    REQUIRE(yaml::parse("a: |\n  text")["a"].as_string() == "text");
    REQUIRE(yaml::parse("a: | # header comment\n  text\n")["a"].as_string() == "text\n");
    REQUIRE(yaml::parse("a: |\r\n  text\r\n")["a"].as_string() == "text\n");
    REQUIRE(yaml::parse("| \n  text\n").as_string() == "text\n");
    REQUIRE(yaml::parse("- |\n  text\n")[0].as_string() == "text\n");
}

TEST_CASE("yaml parse - block scalar chomping", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("a: |-\n  text\n")["a"].as_string() == "text");
    REQUIRE(yaml::parse("a: |-\n  text\n\n\n")["a"].as_string() == "text");
    REQUIRE(yaml::parse("a: |+\n  text\n\n")["a"].as_string() == "text\n\n");
    REQUIRE(yaml::parse("a: |+\n  text")["a"].as_string() == "text");
    REQUIRE(yaml::parse("a: |\n  text\n\n")["a"].as_string() == "text\n");
    REQUIRE(yaml::parse("a: |2\n  text\n")["a"].as_string() == "text\n");
    REQUIRE(yaml::parse("a: |2\n    text\n")["a"].as_string() == "  text\n");
}

TEST_CASE("yaml parse - folded block scalar", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("a: >\n  one\n  two\n")["a"].as_string() == "one two\n");
    REQUIRE(yaml::parse("a: >-\n  one\n  two")["a"].as_string() == "one two");
    REQUIRE(yaml::parse("a: >\n  one\n\n  two\n")["a"].as_string() == "one\ntwo\n");
    REQUIRE(yaml::parse("a: >\n  one\n\n\n  two\n")["a"].as_string() == "one\n\ntwo\n");
}

TEST_CASE("yaml parse - block scalar inside nested mapping", "[willow][yaml][parse]") {
    const auto doc = yaml::parse("outer:\n  body: |\n    line1\n    line2\n  next: 2\n");
    REQUIRE(doc["outer"]["body"].as_string() == "line1\nline2\n");
    REQUIRE(doc["outer"]["next"].as_integer() == 2);
    REQUIRE(doc["outer"].size() == 2);
}

TEST_CASE("yaml parse - document end marker", "[willow][yaml][parse]") {
    REQUIRE(yaml::parse("a: 1\n...\n")["a"].as_integer() == 1);
    REQUIRE(yaml::parse("---\na: 1\n...\n")["a"].as_integer() == 1);
    REQUIRE(yaml::parse("42\n...\n").as_integer() == 42);
    REQUIRE(yaml::parse("...\n").is_null());
}

TEST_CASE("yaml parse - literal operator", "[willow][yaml][parse]") {
    const auto doc = "a: 1\n"_yaml;
    REQUIRE(doc["a"].as_integer() == 1);
}
