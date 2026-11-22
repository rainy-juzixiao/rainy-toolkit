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
#include <rainy/foundation/willow/json5.hpp>

#include <cmath>
#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json5;

TEST_CASE("json5 dump - scalars", "[willow][json5]") {
    REQUIRE(json5::dump(json5::facade<document>{}) == "null");
    REQUIRE(json5::dump(json5::facade<document>(true)) == "true");
    REQUIRE(json5::dump(json5::facade<document>(false)) == "false");
    REQUIRE(json5::dump(json5::facade<document>(42)) == "42");
    REQUIRE(json5::dump(json5::facade<document>(-7)) == "-7");
    REQUIRE(json5::dump(json5::facade<document>("hi")) == "\"hi\"");
}

TEST_CASE("json5 dump - empty containers", "[willow][json5]") {
    REQUIRE(json5::dump(json5::parse("[]")) == "[]");
    REQUIRE(json5::dump(json5::parse("{}")) == "{}");
}

TEST_CASE("json5 dump - unquoted identifier keys", "[willow][json5]") {
    REQUIRE(json5::dump(json5::parse("{a:1}")) == "{a:1}");
    REQUIRE(json5::dump(json5::parse("{$id:1,_x:2}")) == "{$id:1,_x:2}");
    REQUIRE(json5::dump(json5::parse("{true:1}")) == "{true:1}");
    REQUIRE(json5::dump(json5::parse("{a:{b:[1,2]}}")) == "{a:{b:[1,2]}}");
    REQUIRE(json5::dump(json5::parse("{\"a-b\":1}")) == "{\"a-b\":1}");
    REQUIRE(json5::dump(json5::parse("{\"\":1}")) == "{\"\":1}");
    REQUIRE(json5::dump(json5::parse("{'a b':1}")) == "{\"a b\":1}");
}

TEST_CASE("json5 dump - non-finite floats", "[willow][json5]") {
    REQUIRE(json5::dump(json5::parse("NaN")) == "NaN");
    REQUIRE(json5::dump(json5::parse("Infinity")) == "Infinity");
    REQUIRE(json5::dump(json5::parse("-Infinity")) == "-Infinity");
    REQUIRE(json5::dump(json5::parse("{v:Infinity}")) == "{v:Infinity}");
    REQUIRE(json5::dump(json5::parse("{v:NaN}")) == "{v:NaN}");
}

TEST_CASE("json5 dump - float formatting", "[willow][json5]") {
    REQUIRE(json5::dump(json5::parse("1.5")) == "1.5");
    REQUIRE(json5::dump(json5::parse("-7.25")) == "-7.25");
    REQUIRE(json5::dump(json5::parse("0.1")) == "0.1");
    REQUIRE(json5::dump(json5::facade<document>(2.0)) == "2");
}

TEST_CASE("json5 dump - parse roundtrip preserves value", "[willow][json5]") {
    const rainy::core::text::string source =
        R"({a:1,f:2.5,s:'txt',b:true,nil:null,arr:[1,2],obj:{x:0},"quoted key":.5,})";
    json5::facade<document> parsed = json5::parse(source);
    json5::facade<document> reparsed = json5::parse(json5::dump(parsed));
    REQUIRE(reparsed == parsed);

    json5::facade<document> indented = json5::parse(json5::dump(parsed, 4, ' '));
    REQUIRE(indented == parsed);
}

TEST_CASE("json5 dump - roundtrip with comments and escapes in source", "[willow][json5]") {
    const rainy::core::text::string source = "{// lead\n a: 'a\\tb', /* mid */ b: [1,],}";
    json5::facade<document> parsed = json5::parse(source);
    json5::facade<document> reparsed = json5::parse(json5::dump(parsed));
    REQUIRE(reparsed == parsed);
    REQUIRE(reparsed["a"].as_string() == "a\tb");
}

TEST_CASE("json5 dump - non-finite roundtrip", "[willow][json5]") {
    json5::facade<document> inf = json5::parse("-Infinity");
    json5::facade<document> reinf = json5::parse(json5::dump(inf));
    REQUIRE(reinf.is_float());
    REQUIRE(std::isinf(reinf.as_float()));
    REQUIRE(reinf.as_float() < 0);

    json5::facade<document> nan = json5::parse("NaN");
    json5::facade<document> renan = json5::parse(json5::dump(nan));
    REQUIRE(renan.is_float());
    REQUIRE(std::isnan(renan.as_float()));
}

TEST_CASE("json5 dump - indented output reparses", "[willow][json5]") {
    json5::facade<document> j = json5::parse("{a:[1,{b:'x'}],}");
    const rainy::core::text::string out = json5::dump(j, 2, ' ');
    REQUIRE(out.find('\n') != rainy::core::text::string::npos);
    REQUIRE(out.find("a:") != rainy::core::text::string::npos);
    REQUIRE(out.find("\"a\"") == rainy::core::text::string::npos);
    REQUIRE(json5::parse(out) == j);
}
