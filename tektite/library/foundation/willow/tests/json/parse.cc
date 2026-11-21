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

TEST_CASE("json parse - scalar literals", "[willow][json]") {
    REQUIRE(json::parse("null").is_null());
    REQUIRE(json::parse("true").as_bool());
    REQUIRE(!json::parse("false").as_bool());
    REQUIRE(json::parse("123").is_integer());
    REQUIRE(json::parse("123").as_integer() == 123);
    REQUIRE(json::parse("-45").as_integer() == -45);
    REQUIRE(json::parse("1.5").is_float());
    REQUIRE(json::parse("1.5").as_float() == 1.5);
    REQUIRE(json::parse("\"str\"").as_string() == "str");
}

TEST_CASE("json parse - number formats", "[willow][json]") {
    REQUIRE(json::parse("0").as_integer() == 0);
    REQUIRE(json::parse("-0").as_integer() == 0);
    REQUIRE(json::parse("1e3").is_float());
    REQUIRE(json::parse("1E3").is_float());
    REQUIRE(json::parse("1.25e2").as_float() == 125.0);
    REQUIRE(json::parse("1.0").is_float());
    REQUIRE(json::parse("1e-400").as_float() == 0.0);
    REQUIRE(std::isinf(json::parse("1e400").as_float()));
}

TEST_CASE("json parse - empty array and object", "[willow][json]") {
    json::facade<document> arr = json::parse("[]");
    REQUIRE(arr.is_array());
    REQUIRE(arr.empty());
    REQUIRE(arr.size() == 0);

    json::facade<document> obj = json::parse("{}");
    REQUIRE(obj.is_object());
    REQUIRE(obj.empty());
    REQUIRE(obj.size() == 0);
}

TEST_CASE("json parse - array with mixed scalars", "[willow][json]") {
    json::facade<document> arr = json::parse("[1, 2.5, \"x\", true, null]");
    REQUIRE(arr.is_array());
    REQUIRE(arr.size() == 5);
    REQUIRE(arr[0].is_integer());
    REQUIRE(arr[1].is_float());
    REQUIRE(arr[2].is_string());
    REQUIRE(arr[3].is_bool());
    REQUIRE(arr[4].is_null());
}

TEST_CASE("json parse - nested structures", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"a": {"b": [1, {"c": "d"}]}, "e": 2})");
    REQUIRE(j.is_object());
    REQUIRE(j["a"].is_object());
    REQUIRE(j["a"]["b"].is_array());
    REQUIRE(j["a"]["b"][0].as_integer() == 1);
    REQUIRE(j["a"]["b"][1].is_object());
    REQUIRE(j["a"]["b"][1]["c"].as_string() == "d");
    REQUIRE(j["e"].as_integer() == 2);
}

TEST_CASE("json parse - string escapes", "[willow][json]") {
    REQUIRE(json::parse(R"("a\"b")").as_string() == "a\"b");
    REQUIRE(json::parse(R"("a\\b")").as_string() == "a\\b");
    REQUIRE(json::parse(R"("a\nb")").as_string() == "a\nb");
    REQUIRE(json::parse(R"("a\tb")").as_string() == "a\tb");
    REQUIRE(json::parse(R"("a\/b")").as_string() == "a/b");
    REQUIRE(json::parse(R"("A")").as_string() == "A");
}

TEST_CASE("json parse - unicode escape", "[willow][json]") {
    REQUIRE(json::parse(R"("中")").as_string() == "中");
    REQUIRE(json::parse(R"("A")").as_string() == "A");
}

TEST_CASE("json parse - trailing whitespace tolerated", "[willow][json]") {
    REQUIRE(json::parse("  42  ").as_integer() == 42);
    REQUIRE(json::parse("[1] ").is_array());
}

TEST_CASE("json parse - string_view overload", "[willow][json]") {
    rainy::core::text::string_view text = R"({"k": 9})";
    json::facade<document> j = json::parse(text);
    REQUIRE(j["k"].as_integer() == 9);
}

TEST_CASE("json parse - invalid input throws json_parse_error", "[willow][json]") {
    using rainy::foundation::exceptions::willow::json::json_parse_error;

    REQUIRE_THROWS_AS(json::parse(""), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("{"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("["), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("{\"a\"}"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("nul"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("01x"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("\"unterminated"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("truex"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("1.2.3"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("[1, 2"), json_parse_error);
    REQUIRE_THROWS_AS(json::parse("42 42"), json_parse_error);
}

TEST_CASE("json parse - complex nested configuration document", "[willow][json]") {
    const char *source = R"({
        "server": {
            "host": "example.com",
            "port": 8080,
            "tls": {"enabled": true, "cert": null, "ciphers": ["TLS13", "TLS12"]},
            "backups": [
                {"name": "a", "weight": 1.5},
                {"name": "b", "weight": 2.5}
            ]
        },
        "limits": {
            "max_conn": 1000,
            "timeout": 30.0,
            "retries": [1, 2, 4, 8],
            "flags": [false, true, null]
        },
        "tags": [["x"], [{"y": []}], {}]
    })";
    json::facade<document> j = json::parse(source);
    REQUIRE(j.is_object());
    REQUIRE(j.size() == 3);
    REQUIRE(j["server"].size() == 4);
    REQUIRE(j["server"]["host"].as_string() == "example.com");
    REQUIRE(j["server"]["port"].as_integer() == 8080);
    REQUIRE(j["server"]["tls"]["enabled"].as_bool());
    REQUIRE(j["server"]["tls"]["cert"].is_null());
    REQUIRE(j["server"]["tls"]["ciphers"].size() == 2);
    REQUIRE(j["server"]["tls"]["ciphers"][0].as_string() == "TLS13");
    REQUIRE(j["server"]["backups"].size() == 2);
    REQUIRE(j["server"]["backups"][0]["weight"].as_float() == 1.5);
    REQUIRE(j["server"]["backups"][1]["name"].as_string() == "b");
    REQUIRE(j["limits"]["max_conn"].as_integer() == 1000);
    REQUIRE(j["limits"]["timeout"].is_float());
    REQUIRE(j["limits"]["retries"][3].as_integer() == 8);
    REQUIRE(j["limits"]["flags"].size() == 3);
    REQUIRE(!j["limits"]["flags"][0].as_bool());
    REQUIRE(j["limits"]["flags"][2].is_null());
    REQUIRE(j["tags"].size() == 3);
    REQUIRE(j["tags"][0][0].as_string() == "x");
    REQUIRE(j["tags"][1][0]["y"].is_array());
    REQUIRE(j["tags"][1][0]["y"].empty());
    REQUIRE(j["tags"][2].is_object());
    REQUIRE(j["tags"][2].empty());
}

TEST_CASE("json parse - deep nested mixed structure", "[willow][json]") {
    constexpr int depth = 64;
    rainy::text::string source;
    for (int i = 0; i < depth; ++i) {
        if (i % 2 == 0) {
            source += '[';
        } else {
            source += "{\"k" + std::to_string(i) + "\":";
        }
    }
    source += "42";
    for (int i = depth - 1; i >= 0; --i) {
        source += (i % 2 == 0) ? ']' : '}';
    }
    json::facade<document> j = json::parse(source);
    json::facade<document> node = j;
    for (int i = 0; i < depth; ++i) {
        if (i % 2 == 0) {
            REQUIRE(node.is_array());
            REQUIRE(node.size() == 1);
            json::facade<document> child = node[0];
            node = child;
        } else {
            REQUIRE(node.is_object());
            const rainy::text::string key = "k" + std::to_string(i);
            json::facade<document> child = node[key.c_str()];
            node = child;
        }
    }
    REQUIRE(node.is_integer());
    REQUIRE(node.as_integer() == 42);
}

TEST_CASE("json parse - large mixed array", "[willow][json]") {
    rainy::text::string source = "[";
    for (int i = 0; i < 250; ++i) {
        if (i) {
            source += ',';
        }
        switch (i % 5) {
            case 0:
                source += std::to_string(i);
                break;
            case 1:
                source += std::to_string(i) + ".5";
                break;
            case 2:
                source += "\"s" + std::to_string(i) + "\"";
                break;
            case 3:
                source += (i % 2) ? "true" : "false";
                break;
            default:
                source += "null";
                break;
        }
    }
    source += ']';
    json::facade<document> j = json::parse(source);
    REQUIRE(j.is_array());
    REQUIRE(j.size() == 250);
    REQUIRE(j[0].is_integer());
    REQUIRE(j[0].as_integer() == 0);
    REQUIRE(j[1].as_float() == 1.5);
    REQUIRE(j[2].as_string() == "s2");
    REQUIRE(j[3].as_bool());
    REQUIRE(j[4].is_null());
    REQUIRE(j[13].as_bool());
    REQUIRE(j[245].as_integer() == 245);
    REQUIRE(j[246].as_float() == 246.5);
    REQUIRE(j[247].as_string() == "s247");
    REQUIRE(j[248].is_bool());
    REQUIRE(j[249].is_null());
}

TEST_CASE("json parse - nested empty containers", "[willow][json]") {
    json::facade<document> j = json::parse(R"({"a":[[],[{}]],"b":{"c":{}},"d":[{}]})");
    REQUIRE(j.is_object());
    REQUIRE(j.size() == 3);
    REQUIRE(j["a"].is_array());
    REQUIRE(j["a"].size() == 2);
    REQUIRE(j["a"][0].is_array());
    REQUIRE(j["a"][0].empty());
    REQUIRE(j["a"][1][0].is_object());
    REQUIRE(j["a"][1][0].empty());
    REQUIRE(j["b"]["c"].is_object());
    REQUIRE(j["b"]["c"].empty());
    REQUIRE(j["d"][0].is_object());
    REQUIRE(j["d"][0].empty());
}
