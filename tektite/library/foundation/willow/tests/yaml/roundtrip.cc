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

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::yaml;

namespace {
    void require_round_trip(const char *source) {
        const yaml::facade<document> parsed = yaml::parse(source);
        REQUIRE(yaml::parse(yaml::dump(parsed)) == parsed);
        REQUIRE(yaml::parse(yaml::dump(parsed, 2)) == parsed);
        REQUIRE(yaml::parse(yaml::dump(parsed, 4, ' ')) == parsed);
    }
}

TEST_CASE("yaml roundtrip - scalars", "[willow][yaml][roundtrip]") {
    require_round_trip("42\n");
    require_round_trip("-7\n");
    require_round_trip("2.5\n");
    require_round_trip("text\n");
    require_round_trip("true\n");
    require_round_trip("~\n");
}

TEST_CASE("yaml roundtrip - block structures", "[willow][yaml][roundtrip]") {
    require_round_trip("a: 1\nb: text\n");
    require_round_trip("a:\n  b: 1\n");
    require_round_trip("a:\n  b:\n    c: 1\n");
    require_round_trip("- 1\n- 2\n");
    require_round_trip("a:\n  - 1\n  - 2\n");
    require_round_trip("-\n  a: 1\n");
}

TEST_CASE("yaml roundtrip - flow structures", "[willow][yaml][roundtrip]") {
    require_round_trip("{a: 1, b: [2, 3]}\n");
    require_round_trip("[1, [2, 3]]\n");
    require_round_trip("{}");
    require_round_trip("[]");
}

TEST_CASE("yaml roundtrip - mixed nesting", "[willow][yaml][roundtrip]") {
    require_round_trip("a: [1, 2]\n");
    require_round_trip("a: {b: 1}\n");
    require_round_trip("a: 1\nb: [1, 2]\nc: {d: e}\n");
    require_round_trip("root:\n  list:\n    - 1\n    - 2\n  map:\n    k: v\n");
}

TEST_CASE("yaml roundtrip - block scalars", "[willow][yaml][roundtrip]") {
    require_round_trip("a: |\n  line1\n  line2\n");
    require_round_trip("a: |-\n  text\n");
    require_round_trip("a: |+\n  text\n\n");
    require_round_trip("a: >\n  one\n  two\n");
    require_round_trip("outer:\n  body: |\n    line1\n  next: 2\n");

    for (const char *value: {"a\nb", "a\n", "a\n\n", "\na", "a\n\nb", "no newline"}) {
        const yaml::facade<document> original = yaml::facade<document>(value);
        REQUIRE(yaml::parse(yaml::dump(original)) == original);
        REQUIRE(yaml::parse(yaml::dump(original, 4)) == original);
    }
}

TEST_CASE("yaml roundtrip - anchors, aliases and tags", "[willow][yaml][roundtrip]") {
    require_round_trip("a: &x 1\nb: *x\nc: 2\n");
    require_round_trip("outer: &box\n  k: 1\nuser: *box\n");
    require_round_trip("!!str 42\n");
    require_round_trip("key: !custom\n  a: 1\n");
    require_round_trip("{k: &v 1, j: *v}\n");
    require_round_trip("- &anch\n  k: 1\n");
}

TEST_CASE("yaml roundtrip - quoted and empty values", "[willow][yaml][roundtrip]") {
    require_round_trip("a: 'text'\n");
    require_round_trip("a: 'a b'\n");
    require_round_trip("key:\n");
}

TEST_CASE("yaml roundtrip - documents keep their values", "[willow][yaml][roundtrip]") {
    const yaml::facade<document> parsed = yaml::parse("name: willow\nversion: 1.2\nenabled: true\ntags:\n  - a\n  - b\n");
    const yaml::facade<document> reparsed = yaml::parse(yaml::dump(parsed));
    REQUIRE(reparsed == parsed);
    REQUIRE(reparsed["name"].as_string() == "willow");
    REQUIRE(reparsed["version"].as_float() == 1.2);
    REQUIRE(reparsed["enabled"].as_bool());
    REQUIRE(reparsed["tags"].size() == 2);
    REQUIRE(reparsed["tags"][1].as_string() == "b");
}

TEST_CASE("yaml roundtrip - dump output is reparseable at several widths", "[willow][yaml][roundtrip]") {
    const yaml::facade<document> parsed = yaml::parse("a:\n  b:\n    - 1\n    - 2\n");
    for (const unsigned int width: {2U, 4U, 8U}) {
        const rainy::core::text::string dumped = yaml::dump(parsed, width);
        REQUIRE(dumped.find('\n') != rainy::core::text::string::npos);
        REQUIRE(yaml::parse(dumped) == parsed);
    }
}
