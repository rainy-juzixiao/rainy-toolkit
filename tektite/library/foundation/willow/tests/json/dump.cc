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
#include <rainy/foundation/willow/json.hpp>

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json;

TEST_CASE("json dump - scalars", "[willow][json]") {
    REQUIRE(json::dump(json::facade<document>{}) == "null");
    REQUIRE(json::dump(json::facade<document>(true)) == "true");
    REQUIRE(json::dump(json::facade<document>(false)) == "false");
    REQUIRE(json::dump(json::facade<document>(42)) == "42");
    REQUIRE(json::dump(json::facade<document>(-7)) == "-7");
    REQUIRE(json::dump(json::facade<document>("hi")) == "\"hi\"");
}

TEST_CASE("json dump - empty containers", "[willow][json]") {
    REQUIRE(json::dump(json::parse("[]")) == "[]");
    REQUIRE(json::dump(json::parse("{}")) == "{}");
}

TEST_CASE("json dump - compact array", "[willow][json]") {
    json::facade<document> j = json::facade<document>::array({1, 2, 3});
    REQUIRE(json::dump(j) == "[1,2,3]");
}

TEST_CASE("json dump - compact object", "[willow][json]") {
    json::facade<document> j = {{"a", 1}, {"b", "x"}};
    const rainy::core::text::string out = json::dump(j);
    REQUIRE(out.find("\"a\":1") != rainy::core::text::string::npos);
    REQUIRE(out.find("\"b\":\"x\"") != rainy::core::text::string::npos);
    REQUIRE(out.front() == '{');
    REQUIRE(out.back() == '}');
}

TEST_CASE("json dump - nested structure compact", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"list":[1,{"k":true}]})");
    REQUIRE(json::dump(j) == R"({"list":[1,{"k":true}]})");
}

TEST_CASE("json dump - indented with spaces", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"a":1})");
    const rainy::core::text::string out = json::dump(j, 2, ' ');
    REQUIRE(out.find('\n') != rainy::core::text::string::npos);
    REQUIRE(out.find("\"a\"") != rainy::core::text::string::npos);
    REQUIRE(out.find("  \"a\"") != rainy::core::text::string::npos);
}

TEST_CASE("json dump - indented with tabs", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"a":1})");
    const rainy::core::text::string out = json::dump(j, 1, '\t');
    REQUIRE(out.find('\n') != rainy::core::text::string::npos);
    REQUIRE(out.find("\t\"a\"") != rainy::core::text::string::npos);
}

TEST_CASE("json dump - string escaping", "[willow][json]") {
    json::facade<document> j = "a\"b\\c\nd";
    const rainy::core::text::string out = json::dump(j);
    REQUIRE(out.find("\\\"") != rainy::core::text::string::npos);
    REQUIRE(out.find("\\\\") != rainy::core::text::string::npos);
    REQUIRE(out.find("\\n") != rainy::core::text::string::npos);
}

TEST_CASE("json dump - float integral value stays compact", "[willow][json]") {
    json::facade<document> j = 2.0;
    REQUIRE(j.is_float());
    const rainy::core::text::string out = json::dump(j);
    REQUIRE(out == "2");
}

TEST_CASE("json dump - parse roundtrip preserves value", "[willow][json]") {
    const rainy::core::text::string source = R"({"n":1,"f":2.5,"s":"txt","b":true,"nil":null,"arr":[1,2],"obj":{"x":0}})";
    json::facade<document> parsed = json::parse(source);
    json::facade<document> reparsed = json::parse(json::dump(parsed));
    REQUIRE(reparsed == parsed);

    json::facade<document> indented = json::parse(json::dump(parsed, 4, ' '));
    REQUIRE(indented == parsed);
}
