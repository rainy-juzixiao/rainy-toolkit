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

#include <limits>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::yaml;

TEST_CASE("yaml dump - scalar documents", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>()) == "null\n");
    REQUIRE(yaml::dump(yaml::facade<document>(true)) == "true\n");
    REQUIRE(yaml::dump(yaml::facade<document>(false)) == "false\n");
    REQUIRE(yaml::dump(yaml::facade<document>(42)) == "42\n");
    REQUIRE(yaml::dump(yaml::facade<document>(-7)) == "-7\n");
    REQUIRE(yaml::dump(yaml::facade<document>(0)) == "0\n");
    REQUIRE(yaml::dump(yaml::facade<document>(2.0)) == "2\n");
    REQUIRE(yaml::dump(yaml::facade<document>(3.25)) == "3.25\n");
}

TEST_CASE("yaml dump - string documents", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>("hi")) == "hi\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a b")) == "a b\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a,b")) == "\"a,b\"\n");
}

TEST_CASE("yaml dump - lexer-number-like strings force quoting", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>("0x1F")) == "\"0x1F\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("0Xf")) == "\"0Xf\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("0o17")) == "\"0o17\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("1_000")) == "\"1_000\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("_1")) == "\"_1\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("+0x10")) == "\"+0x10\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("0x")) == "0x\n");
    REQUIRE(yaml::dump(yaml::facade<document>("0o8")) == "0o8\n");

    REQUIRE(yaml::parse(yaml::dump(yaml::facade<document>("0x1F"))).is_string());
    REQUIRE(yaml::parse(yaml::dump(yaml::facade<document>("1_000"))).as_string() == "1_000");
    REQUIRE(yaml::parse(yaml::dump(yaml::facade<document>("_1"))).as_string() == "_1");
    REQUIRE(yaml::parse("0x1F").as_integer() == 31);
    REQUIRE(yaml::parse("1_000").as_integer() == 1000);
}

TEST_CASE("yaml dump - flow indicators inside strings force quoting", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>("a[0]")) == "\"a[0]\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("x}")) == "\"x}\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("{x")) == "\"{x\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a]b")) == "\"a]b\"\n");

    const yaml::facade<document> flow = yaml::facade<document>::object({{"k", "a,b"}});
    REQUIRE(yaml::dump(flow) == "{k: \"a,b\"}\n");
    REQUIRE(yaml::parse(yaml::dump(flow)) == flow);
}

TEST_CASE("yaml dump - empty containers inside block structures", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::parse("a: {}\n"), 2) == "a:\n  {}\n");
    REQUIRE(yaml::dump(yaml::parse("a: []\n"), 2) == "a:\n  []\n");

    const yaml::facade<document> with_object = yaml::facade<document>::object({{"a", yaml::facade<document>::object({})}});
    REQUIRE(yaml::dump(with_object, 2) == "a:\n  {}\n");

    const yaml::facade<document> with_array = yaml::facade<document>::object({{"a", yaml::facade<document>::array({})}});
    REQUIRE(yaml::dump(with_array, 2) == "a:\n  []\n");

    const yaml::facade<document> nested = yaml::facade<document>::array({yaml::facade<document>::object({})});
    REQUIRE(yaml::dump(nested, 2) == "-\n  {}\n");

    REQUIRE(yaml::parse(yaml::dump(yaml::parse("a: {}\n"), 2)) == yaml::parse("a: {}\n"));
    REQUIRE(yaml::parse(yaml::dump(yaml::parse("a: []\n"), 2)) == yaml::parse("a: []\n"));
}

TEST_CASE("yaml dump - strings needing quotes", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>("")) == "\"\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("42")) == "\"42\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("true")) == "\"true\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("null")) == "\"null\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a:b")) == "\"a:b\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("-x")) == "\"-x\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>(" leading")) == "\" leading\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("trailing ")) == "\"trailing \"\n");
}

TEST_CASE("yaml dump - multiline string becomes a literal block", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>("a\nb")) == "|-\n  a\n  b\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a\nb"), 4) == "|-\n    a\n    b\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a\n")) == "|\n  a\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a\n\n")) == "|+\n  a\n  \n");
    REQUIRE(yaml::dump(yaml::facade<document>("\na")) == "|-\n  \n  a\n");
    REQUIRE(yaml::dump(yaml::facade<document>("a\n\nb")) == "|-\n  a\n  \n  b\n");
}

TEST_CASE("yaml dump - empty containers", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::parse("{}")) == "{}\n");
    REQUIRE(yaml::dump(yaml::parse("[]")) == "[]\n");
}

TEST_CASE("yaml dump - object styles", "[willow][yaml][dump]") {
    const yaml::facade<document> object = yaml::facade<document>::object({{"a", 1}});
    REQUIRE(yaml::dump(object) == "{a: 1}\n");
    REQUIRE(yaml::dump(object, 2) == "a: 1\n");

    const yaml::facade<document> nested = yaml::facade<document>::object({{"a", yaml::facade<document>::object({{"b", 1}})}});
    REQUIRE(yaml::dump(nested) == "{a: {b: 1}}\n");
    REQUIRE(yaml::dump(nested, 2) == "a:\n  b: 1\n");

    const yaml::facade<document> deeper =
        yaml::facade<document>::object({{"a", yaml::facade<document>::object({{"b", yaml::facade<document>::object({{"c", 1}})}})}});
    REQUIRE(yaml::dump(deeper, 2) == "a:\n  b:\n    c: 1\n");
}

TEST_CASE("yaml dump - array styles", "[willow][yaml][dump]") {
    const yaml::facade<document> array = yaml::facade<document>::array({1, 2, 3});
    REQUIRE(yaml::dump(array) == "[1, 2, 3]\n");
    REQUIRE(yaml::dump(array, 2) == "- 1\n- 2\n- 3\n");

    const yaml::facade<document> nested = yaml::facade<document>::array({yaml::facade<document>::array({1, 2})});
    REQUIRE(yaml::dump(nested) == "[[1, 2]]\n");
    REQUIRE(yaml::dump(nested, 2) == "-\n  - 1\n  - 2\n");
}

TEST_CASE("yaml dump - containers as mapping values", "[willow][yaml][dump]") {
    const yaml::facade<document> with_array = yaml::facade<document>::object({{"a", yaml::facade<document>::array({1, 2})}});
    REQUIRE(yaml::dump(with_array) == "{a: [1, 2]}\n");
    REQUIRE(yaml::dump(with_array, 2) == "a:\n  - 1\n  - 2\n");

    const yaml::facade<document> with_object = yaml::facade<document>::object({{"a", yaml::facade<document>::object({})}});
    REQUIRE(yaml::dump(with_object) == "{a: {}}\n");
}

TEST_CASE("yaml dump - mapping inside an array", "[willow][yaml][dump]") {
    const yaml::facade<document> array = yaml::facade<document>::array({yaml::facade<document>::object({{"a", 1}})});
    REQUIRE(yaml::dump(array) == "[{a: 1}]\n");
    REQUIRE(yaml::dump(array, 2) == "-\n  a: 1\n");
}

TEST_CASE("yaml dump - extreme integers", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>(std::numeric_limits<std::int32_t>::min())) == "-2147483648\n");
    REQUIRE(yaml::dump(yaml::facade<document>(std::numeric_limits<std::int32_t>::max())) == "2147483647\n");
    REQUIRE(yaml::dump(yaml::facade<document64>(std::numeric_limits<std::int64_t>::min())) == "-9223372036854775808\n");
    REQUIRE(yaml::dump(yaml::facade<document64>(std::numeric_limits<std::int64_t>::max())) == "9223372036854775807\n");
}

TEST_CASE("yaml dump - float precision is passed to to_chars", "[willow][yaml][dump]") {
    const yaml::facade<document> pi = yaml::facade<document>(3.14159265358979);

    yaml::serializer_args<yaml::facade<document>> args;
    REQUIRE(args.precision == std::numeric_limits<double>::digits10 + 1);
    REQUIRE(yaml::dump(pi, args) == "3.14159265358979\n");

    args.precision = 3;
    REQUIRE(yaml::dump(pi, args) == "3.14\n");

    args.precision = 1;
    REQUIRE(yaml::dump(pi, args) == "3\n");

    yaml::serializer_args<yaml::facade<document>> wide;
    wide.precision = std::numeric_limits<double>::max_digits10;
    REQUIRE(yaml::dump(yaml::facade<document>(0.1 + 0.2), wide) == "0.30000000000000004\n");
    REQUIRE(yaml::dump(yaml::facade<document>(0.1 + 0.2)) == "0.3\n");
}

TEST_CASE("yaml dump - inf and nan stay symbolic", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::facade<document>(std::numeric_limits<double>::infinity())) == ".inf\n");
    REQUIRE(yaml::dump(yaml::facade<document>(-std::numeric_limits<double>::infinity())) == "-.inf\n");
    REQUIRE(yaml::dump(yaml::facade<document>(std::numeric_limits<double>::quiet_NaN())) == ".NaN\n");
}

TEST_CASE("yaml dump - escape_unicode escapes whole code points", "[willow][yaml][dump]") {
    yaml::serializer_args<yaml::facade<document>> args;
    args.escape_unicode = true;

    REQUIRE(yaml::dump(yaml::facade<document>("é"), args) == "\"\\u00e9\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("aéb"), args) == "\"a\\u00e9b\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("😀"), args) == "\"\\ud83d\\ude00\"\n");
    REQUIRE(yaml::dump(yaml::facade<document>("aé"), args) == "\"a\\u00e9\"\n");

    REQUIRE(yaml::parse(yaml::dump(yaml::facade<document>("é"), args)).as_string() == "\xC3\xA9");
    REQUIRE(yaml::parse(yaml::dump(yaml::facade<document>("😀"), args)).as_string() == "\xF0\x9F\x98\x80");

    REQUIRE(yaml::dump(yaml::facade<document>("é")) == "é\n");
    REQUIRE(yaml::dump(yaml::facade<document>("aéb")) == "aéb\n");
}

TEST_CASE("yaml dump - flow containers inside block sequences", "[willow][yaml][dump]") {
    const auto parsed = yaml::parse("- {a: 1}\n- {b: 2}\n");
    REQUIRE(yaml::dump(parsed, 2) == "- {a: 1}\n- {b: 2}\n");
    REQUIRE(yaml::parse(yaml::dump(parsed, 2)) == parsed);

    const auto flow_seq = yaml::parse("- [1, 2]\n");
    REQUIRE(yaml::dump(flow_seq, 2) == "- [1, 2]\n");
    REQUIRE(yaml::parse(yaml::dump(flow_seq, 2)) == flow_seq);
}

TEST_CASE("yaml dump - alias values stay inline", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::parse("outer: &box\n  k: 1\nuser: *box\n")) == "outer: &box\n  k: 1\nuser: *box\n");
    REQUIRE(yaml::dump(yaml::parse("a: &seq\n  - 1\nb: *seq\n")) == "a: &seq\n  - 1\nb: *seq\n");
    REQUIRE(yaml::dump(yaml::parse("s: &s str\nt: *s\n")) == "s: &s str\nt: *s\n");
    REQUIRE(yaml::dump(yaml::parse("- &s 1\n- *s\n")) == "- &s 1\n- *s\n");
}

TEST_CASE("yaml dump - block scalar sequence items", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::parse("- |\n  text\n- more\n")) == "- |\n    text\n- more\n");
    REQUIRE(yaml::dump(yaml::parse("- |-\n  text\n")) == "- text\n");
    REQUIRE(yaml::parse(yaml::dump(yaml::parse("- |\n  text\n"))) == yaml::parse("- |\n  text\n"));
}

TEST_CASE("yaml dump - tags and anchors emit raw names", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::parse("!!str 42")) == "!!str 42\n");
    REQUIRE(yaml::dump(yaml::parse("!custom 42")) == "!custom 42\n");
    REQUIRE(yaml::dump(yaml::parse("&true 1")) == "&true 1\n");
    REQUIRE(yaml::dump(yaml::parse("a: &x 1\nb: *x\n")) == "a: &x 1\nb: *x\n");
    REQUIRE(yaml::dump(yaml::parse("{k: &v 1, j: *v}")) == "{k: &v 1, j: *v}\n");
    REQUIRE(yaml::dump(yaml::parse("key: !custom\n  a: 1\n")) == "key: !custom\n  a: 1\n");
    REQUIRE(yaml::dump(yaml::parse("- &anch\n  k: 1\n")) == "- &anch\n  k: 1\n");
}

TEST_CASE("yaml dump - large documents keep the terminating newline", "[willow][yaml][dump]") {
    yaml::facade<document> doc = yaml::facade<document>::array({});
    for (int i = 0; i < 2000; ++i) {
        doc.push_back(i);
    }
    const rainy::core::text::string dumped = yaml::dump(doc, 2);
    REQUIRE(dumped.size() > 4096);
    REQUIRE(dumped.back() == '\n');
    REQUIRE(yaml::parse(dumped) == doc);
}

TEST_CASE("yaml dump - parsed documents keep their source style", "[willow][yaml][dump]") {
    REQUIRE(yaml::dump(yaml::parse("a: 1\nb: text\n")) == "a: 1\nb: text\n");
    REQUIRE(yaml::dump(yaml::parse("a: 1\nb: text\n"), 4) == "a: 1\nb: text\n");
    REQUIRE(yaml::dump(yaml::parse("{a: 1}")) == "{a: 1}\n");
    REQUIRE(yaml::dump(yaml::parse("- 1\n- 2\n")) == "- 1\n- 2\n");

    const rainy::core::text::string flow = yaml::dump(yaml::parse("{a: 1}"), 2);
    REQUIRE(flow == "{a: 1}\n");
}

TEST_CASE("yaml dump - scalar leaves inside block mappings", "[willow][yaml][dump]") {
    const yaml::facade<document> doc = yaml::parse("a: 1\n");
    REQUIRE(yaml::dump(doc) == "a: 1\n");

    const yaml::facade<document> quoted = yaml::parse("a: 'text'\n");
    REQUIRE(yaml::dump(quoted) == "a: text\n");

    const yaml::facade<document> null_value = yaml::parse("a:\n");
    REQUIRE(yaml::dump(null_value) == "a: null\n");
}
