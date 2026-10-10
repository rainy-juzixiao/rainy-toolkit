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
#include <rainy/foundation/willow/json.hpp>
#include <rainy/foundation/willow/jsonnet.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::jsonnet;

namespace {
    facade<document> parse_text(const char *source) {
        return jsonnet::parse(source);
    }
}

TEST_CASE("jsonnet dump - evaluate and render as JSON text", "[jsonnet][roundtrip]") {
    REQUIRE(jsonnet::dump(parse_text("1 + 2")) == "3");
    REQUIRE(jsonnet::dump(parse_text("{ a: 1, b: [1, 2] }")) == R"({"a":1,"b":[1,2]})");
    REQUIRE(jsonnet::dump(parse_text("[1, \"two\", true, null]")) == R"([1,"two",true,null])");
}

TEST_CASE("jsonnet dump - hidden fields are not manifested", "[jsonnet][roundtrip]") {
    REQUIRE(jsonnet::dump(parse_text("{ a: 1, b:: 2 }")) == R"({"a":1})");
}

TEST_CASE("jsonnet dump - expressions are evaluated before rendering", "[jsonnet][roundtrip]") {
    REQUIRE(jsonnet::dump(parse_text("local x = 10; { value: x * 2 }")) == R"({"value":20})");
    REQUIRE(jsonnet::dump(parse_text("[i * i for i in [1, 2, 3]]")) == "[1,4,9]");
    REQUIRE(jsonnet::dump(parse_text("({ a: 1 } + { a: super.a + 1 }).a")) == "2");
}

TEST_CASE("jsonnet dump - AST export is independent of evaluation", "[jsonnet][roundtrip]") {
    facade<document> doc = parse_text("{ a: 1 + 2 }");
    const auto text = dump_ast(doc);
    REQUIRE(text.find("object") != rainy::core::text::string::npos);
    REQUIRE(text.find("binary") != rainy::core::text::string::npos);
    REQUIRE(text.find("number") != rainy::core::text::string::npos);
}

TEST_CASE("jsonnet roundtrip - evaluate then reparse the rendered JSON", "[jsonnet][roundtrip]") {
    const char *source = R"({
        name: "widget",
        count: 3,
        enabled: true,
        tags: ["a", "b"],
        nested: { deep: [1, 2, 3] }
    })";
    const rainy::core::text::string rendered = jsonnet::dump(parse_text(source));
    const json::facade<document> reparsed = json::parse(rendered);
    REQUIRE(reparsed["name"].as_string() == "widget");
    REQUIRE(reparsed["count"].as_integer() == 3);
    REQUIRE(reparsed["enabled"].as_bool());
    REQUIRE(reparsed["tags"].size() == 2);
    REQUIRE(reparsed["nested"]["deep"][2].as_integer() == 3);
}

TEST_CASE("jsonnet roundtrip - a configuration-like document", "[jsonnet][roundtrip]") {
    const char *source = R"(
        local base = { replicas: 2, image: "app:1.0" };
        local prod = base + { replicas: 5, env: "production" };
        {
            deployment: prod,
            total: prod.replicas * 3,
            names: [std.char(65 + i) for i in [0, 1, 2]],
        }
    )";
    const document result = evaluate(parse_text(source));
    REQUIRE(result["deployment"]["replicas"].as_integer() == 5);
    REQUIRE(result["deployment"]["image"].as_string() == "app:1.0");
    REQUIRE(result["total"].as_integer() == 15);
    REQUIRE(result["names"][0].as_string() == "A");
    REQUIRE(result["names"][2].as_string() == "C");
}
