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

#include <cstdio>
#include <sstream>

namespace rc = rainy::core;

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::hjson;

TEST_CASE("hjson parse - escape sequences in quoted strings", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse(R"(a: "\b\f\n\r\t\"\'\\\/")");
    REQUIRE(doc["a"].as_string() == "\b\f\n\r\t\"'\\/");
}

TEST_CASE("hjson parse - unicode escapes", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse(R"(a: "\u00e9")");
    REQUIRE(doc["a"].as_string() == "\xc3\xa9");
    hjson::facade<document> upper = hjson::parse(R"(a: "\u00E9")");
    REQUIRE(upper["a"].as_string() == "\xc3\xa9");
    hjson::facade<document> cjk = hjson::parse(R"(a: "\u4e2d\u6587")");
    REQUIRE(cjk["a"].as_string() == "\xe4\xb8\xad\xe6\x96\x87");
}

TEST_CASE("hjson parse - surrogate pair escape", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse(R"(a: "\ud83d\ude00")");
    REQUIRE(doc["a"].as_string() == "\xf0\x9f\x98\x80");
}

TEST_CASE("hjson parse - unknown escape yields the escaped character", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse(R"(a: "\q")")["a"].as_string() == "q");
}

TEST_CASE("hjson parse - escapes work in single quoted strings", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse(R"(a: '\n')")["a"].as_string() == "\n");
    REQUIRE(hjson::parse(R"(a: '\"')")["a"].as_string() == "\"");
}

TEST_CASE("hjson parse - number classification", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse("a: 1e5")["a"].is_float());
    REQUIRE(hjson::parse("a: 1E5")["a"].is_float());
    REQUIRE(hjson::parse("a: .5")["a"].is_float());
    REQUIRE(hjson::parse("a: 1.5e-3")["a"].is_float());
    REQUIRE(hjson::parse("a: -0.0")["a"].is_float());
    REQUIRE(hjson::parse("a: 1.")["a"].is_float());
    REQUIRE(hjson::parse("a: 007")["a"].as_integer() == 7);
    REQUIRE(hjson::parse("a: 0x1f")["a"].as_string() == "0x1f");
    REQUIRE(hjson::parse("a: +1")["a"].as_string() == "+1");
}

TEST_CASE("hjson parse - integer limits", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse("a: 2147483647")["a"].as_integer() == 2147483647);
    REQUIRE(hjson::parse("a: -2147483647")["a"].as_integer() == -2147483647);
    REQUIRE(hjson::parse("a: -2147483648")["a"].as_integer() == -2147483648);
}

TEST_CASE("hjson parse - multiline string keeps quote runs shorter than three", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a:\n  '''\n  it''s here\n  '''\n");
    REQUIRE(doc["a"].as_string() == "it''s here");
}

TEST_CASE("hjson parse - multiline string accepts content on the opener line", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a: '''   \n  x\n  '''\n");
    REQUIRE(doc["a"].as_string() == "x");
}

TEST_CASE("hjson parse - multiline string strips only the opener indentation", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a:\n    '''\n    x\n      y\n    '''\n");
    REQUIRE(doc["a"].as_string() == "x\n  y");
}

TEST_CASE("hjson parse - multiline string opener at column zero", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a: '''\nx\n'''\n");
    REQUIRE(doc["a"].as_string() == "x");
}

TEST_CASE("hjson parse - multiline string indented with tabs", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a:\n\t'''\n\tx\n\ty\n\t'''\n");
    REQUIRE(doc["a"].as_string() == "x\ny");
}

TEST_CASE("hjson parse - multiline string handles blank lines", "[willow][hjson][parse]") {
    hjson::facade<document> first = hjson::parse("a:\n  '''\n\n  x\n  '''\n");
    REQUIRE(first["a"].as_string() == "\nx");
    hjson::facade<document> second = hjson::parse("a:\n  '''\n  x\n\n  y\n  '''\n");
    REQUIRE(second["a"].as_string() == "x\ny");
}

TEST_CASE("hjson parse - multiline string handles windows line endings", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a:\r\n  '''\r\n  x\r\n  y\r\n  '''\r\n");
    REQUIRE(doc["a"].as_string() == "x\ny");
}

TEST_CASE("hjson parse - multiline string followed by another key", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a:\n  '''\n  x\n  '''\nb: 1\n");
    REQUIRE(doc["a"].as_string() == "x");
    REQUIRE(doc["b"].as_integer() == 1);
}

TEST_CASE("hjson parse - single quotes are also used for keys", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("{'a': 1}");
    REQUIRE(doc["a"].as_integer() == 1);
}

TEST_CASE("hjson parse - quoted keys may contain escapes", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse(R"({"a\nb": 1})");
    REQUIRE(doc["a\nb"].as_integer() == 1);
}

TEST_CASE("hjson parse - nested objects and arrays", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("{a:{b:{c:{d:1}}}}");
    REQUIRE(doc["a"]["b"]["c"]["d"].as_integer() == 1);
}

TEST_CASE("hjson parse - arrays may be newline separated", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("[\n1\n2\n]");
    REQUIRE(doc.size() == 2);
    REQUIRE(doc[0].as_integer() == 1);
    REQUIRE(doc[1].as_integer() == 2);
}

TEST_CASE("hjson parse - commas and newlines may be mixed as separators", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("{a: 1, b: 2,\nc: [1,2],}");
    REQUIRE(doc.size() == 3);
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_integer() == 2);
    REQUIRE(doc["c"].size() == 2);
}

TEST_CASE("hjson parse - nested arrays", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("[[1,2],[3]]");
    REQUIRE(doc.size() == 2);
    REQUIRE(doc[0].size() == 2);
    REQUIRE(doc[1][0].as_integer() == 3);
}

TEST_CASE("hjson parse - spaces before the separator are allowed", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a : 1\n");
    REQUIRE(doc["a"].as_integer() == 1);
}

TEST_CASE("hjson parse - comments may appear between key and value", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a: /* c */ 1\n");
    REQUIRE(doc["a"].as_integer() == 1);
}

TEST_CASE("hjson parse - comments may precede the first key", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("/* c */ a: 1\n");
    REQUIRE(doc["a"].as_integer() == 1);
}

TEST_CASE("hjson parse - comments may appear inside braces", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("{/* c */ a: 1}");
    REQUIRE(doc["a"].as_integer() == 1);
}

TEST_CASE("hjson parse - block comments may span lines", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a: 1\n/* x\ny */\nb: 2\n");
    REQUIRE(doc.size() == 2);
    REQUIRE(doc["b"].as_integer() == 2);
}

TEST_CASE("hjson parse - a slash that is not a comment marker stays in the value", "[willow][hjson][parse]") {
    hjson::facade<document> doc = hjson::parse("a: x/y\n");
    REQUIRE(doc["a"].as_string() == "x/y");
}

TEST_CASE("hjson parse - parse from a core string", "[willow][hjson][parse]") {
    const rc::text::string text = "k: v";
    const hjson::facade<document> doc = hjson::parse(text);
    REQUIRE(doc["k"].as_string() == "v");
}

TEST_CASE("hjson parse - parse from a core string view", "[willow][hjson][parse]") {
    const rc::text::string_view text = "k: v";
    const hjson::facade<document> doc = hjson::parse(text);
    REQUIRE(doc["k"].as_string() == "v");
}

TEST_CASE("hjson parse - parse from a stream", "[willow][hjson][parse]") {
    std::istringstream input("a: 1\nb: two\n");
    stream_input_adapter<char> adapter(input);
    const hjson::facade<document> doc = hjson::implements::parse_document<document>(rainy::utility::move(adapter));
    REQUIRE(doc["a"].as_integer() == 1);
    REQUIRE(doc["b"].as_string() == "two");
}

TEST_CASE("hjson parse - parse from a file", "[willow][hjson][parse]") {
    std::FILE *file = std::tmpfile();
    REQUIRE(file != nullptr);
    std::fputs("fk: fv\n", file);
    std::rewind(file);
    const hjson::facade<document> doc = hjson::parse(file);
    std::fclose(file);
    REQUIRE(doc["fk"].as_string() == "fv");
}

TEST_CASE("hjson parse - dump into a stream", "[willow][hjson][parse]") {
    const hjson::facade<document> doc = hjson::parse("z: 9\n");
    std::ostringstream output;
    stream_output_adapter<char> adapter(output);
    hjson::dump(doc.as_document(), &adapter);
    REQUIRE(output.str() == "z: 9\n");
}

TEST_CASE("hjson parse - span and adapter paths agree", "[willow][hjson][parse]") {
    const rc::text::string text = "a:\n  '''\n  x\n  y\n  '''\nb: 1\n";
    const hjson::facade<document> from_span = hjson::parse(text);

    string_input_adapter<rc::text::string> adapter(text);
    const hjson::facade<document> from_adapter =
        hjson::implements::parse_document<document, decltype(adapter)>(adapter, nullptr, 0);

    REQUIRE(from_adapter == from_span);
}

TEST_CASE("hjson parse - stream and span paths agree", "[willow][hjson][parse]") {
    const rc::text::string text = "a:\n  '''\n  x\n  y\n  '''\nb: 1\n";
    const hjson::facade<document> from_span = hjson::parse(text);

    std::istringstream input(text.c_str());
    stream_input_adapter<char> adapter(input);
    const hjson::facade<document> from_stream = hjson::implements::parse_document<document>(rainy::utility::move(adapter));

    REQUIRE(from_stream == from_span);
}

TEST_CASE("hjson parse - integers beyond the integer range fall back to float", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse("a: 2147483648")["a"].is_float());
    REQUIRE(hjson::parse("a: 2147483648")["a"].as_float() == 2147483648.0);
    REQUIRE(hjson::parse("a: 4294967296")["a"].is_float());
    REQUIRE(hjson::parse("a: -2147483649")["a"].is_float());
    REQUIRE(hjson::parse("a: 99999999999999999999")["a"].as_float() == 1e20);
}

TEST_CASE("hjson parse - values inside the integer range stay integers", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse("a: 2147483647")["a"].is_integer());
    REQUIRE(hjson::parse("a: -2147483648")["a"].is_integer());
    REQUIRE(hjson::parse("a: 0")["a"].is_integer());
    REQUIRE(hjson::parse("a: 007")["a"].is_integer());
}

TEST_CASE("hjson parse - integers just past the boundary reparse as floats", "[willow][hjson][parse]") {
    REQUIRE(hjson::parse("a: 2147483647.5")["a"].is_float());
    REQUIRE(hjson::parse("a: 1e5")["a"].is_float());
}
