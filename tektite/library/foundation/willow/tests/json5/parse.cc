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
#include <cstdint>
#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::json5;

TEST_CASE("json5 parse - scalar literals", "[willow][json5]") {
    REQUIRE(json5::parse("null").is_null());
    REQUIRE(json5::parse("true").as_bool());
    REQUIRE(!json5::parse("false").as_bool());
    REQUIRE(json5::parse("123").is_integer());
    REQUIRE(json5::parse("123").as_integer() == 123);
    REQUIRE(json5::parse("-45").as_integer() == -45);
    REQUIRE(json5::parse("1.5").is_float());
    REQUIRE(json5::parse("1.5").as_float() == 1.5);
    REQUIRE(json5::parse("\"str\"").as_string() == "str");
    REQUIRE(json5::parse("'str'").as_string() == "str");
    REQUIRE(json5::parse("\"a'b\"").as_string() == "a'b");
    REQUIRE(json5::parse("'a\"b'").as_string() == "a\"b");
    REQUIRE(json5::parse("''").as_string().empty());
}

TEST_CASE("json5 parse - extended number formats", "[willow][json5]") {
    REQUIRE(json5::parse("+5").is_integer());
    REQUIRE(json5::parse("+5").as_integer() == 5);
    REQUIRE(json5::parse(".5").is_float());
    REQUIRE(json5::parse(".5").as_float() == 0.5);
    REQUIRE(json5::parse("-.5").as_float() == -0.5);
    REQUIRE(json5::parse("5.").is_float());
    REQUIRE(json5::parse("5.").as_float() == 5.0);
    REQUIRE(json5::parse("5.e3").is_float());
    REQUIRE(json5::parse("5.e3").as_float() == 5000.0);
    REQUIRE(json5::parse("0.5e-3").as_float() == 0.0005);
    REQUIRE(json5::parse("1E3").is_float());
    REQUIRE(json5::parse("1E3").as_float() == 1000.0);
    REQUIRE(json5::parse("+.5").as_float() == 0.5);
    REQUIRE(json5::parse("0").is_integer());
    REQUIRE(json5::parse("0e1").is_float());
    REQUIRE(json5::parse("99999999999999999999").is_float());
    REQUIRE(json5::parse("100000000000000000000").is_float());
    REQUIRE(json5::parse("100000000000000000000").as_float() == 1e20);
    REQUIRE(json5::parse("1e-400").as_float() == 0.0);
    REQUIRE(std::isinf(json5::parse("1e400").as_float()));
    REQUIRE(json5::parse("1e400").as_float() > 0);
}

TEST_CASE("json5 parse - hexadecimal numbers", "[willow][json5]") {
    REQUIRE(json5::parse("0x0").is_integer());
    REQUIRE(json5::parse("0x0").as_integer() == 0);
    REQUIRE(json5::parse("0x1F").is_integer());
    REQUIRE(json5::parse("0x1F").as_integer() == 31);
    REQUIRE(json5::parse("0X1f").as_integer() == 31);
    REQUIRE(json5::parse("-0x10").as_integer() == -16);
    REQUIRE(json5::parse("+0x10").as_integer() == 16);
    REQUIRE(json5::parse("0xff").as_integer() == 255);
    REQUIRE(json5::parse("0xFFFFFFFFFFFFFFFF").is_float());
    REQUIRE(json5::parse("0xFFFFFFFFFFFFFFFF").as_float() == static_cast<double>((std::numeric_limits<std::uint64_t>::max)()));
}

TEST_CASE("json5 parse - infinity and nan", "[willow][json5]") {
    REQUIRE(json5::parse("Infinity").is_float());
    REQUIRE(std::isinf(json5::parse("Infinity").as_float()));
    REQUIRE(json5::parse("Infinity").as_float() > 0);
    REQUIRE(json5::parse("+Infinity").as_float() > 0);
    REQUIRE(json5::parse("-Infinity").as_float() < 0);
    REQUIRE(json5::parse("NaN").is_float());
    REQUIRE(std::isnan(json5::parse("NaN").as_float()));
    REQUIRE(std::isnan(json5::parse("-NaN").as_float()));
    REQUIRE(std::isnan(json5::parse("+NaN").as_float()));
}

TEST_CASE("json5 parse - string escapes", "[willow][json5]") {
    REQUIRE(json5::parse(R"("a\"b")").as_string() == "a\"b");
    REQUIRE(json5::parse(R"('a\'b')").as_string() == "a'b");
    REQUIRE(json5::parse(R"("a\\b")").as_string() == "a\\b");
    REQUIRE(json5::parse(R"("a\/b")").as_string() == "a/b");
    REQUIRE(json5::parse(R"("a\nb")").as_string() == "a\nb");
    REQUIRE(json5::parse(R"("a\tb")").as_string() == "a\tb");
    REQUIRE(json5::parse(R"("a\vb")").as_string() == "a\vb");
    REQUIRE(json5::parse(R"("\0")").as_string().size() == 1);
    REQUIRE(json5::parse(R"("\0")").as_string()[0] == '\0');
    REQUIRE(json5::parse(R"("\x41")").as_string() == "A");
    REQUIRE(json5::parse(R"("\x61\x62")").as_string() == "ab");
    REQUIRE(json5::parse(R"("\A")").as_string() == "A");
    REQUIRE(json5::parse(R"("\中")").as_string() == "中");
    REQUIRE(json5::parse(R"("é")").as_string() == "é");
    REQUIRE(json5::parse(R"("中")").as_string() == "中");
}

TEST_CASE("json5 parse - surrogate pairs", "[willow][json5]") {
    REQUIRE(json5::parse(R"("😀")").as_string() == "\xF0\x9F\x98\x80");
    REQUIRE(json5::parse(R"('😀')").as_string() == "\xF0\x9F\x98\x80");
    REQUIRE(json5::parse("\"\\uD83D\\uDE00\"").as_string() == "\xF0\x9F\x98\x80");
}

TEST_CASE("json5 parse - line continuation", "[willow][json5]") {
    REQUIRE(json5::parse("\"a\\\nb\"").as_string() == "ab");
    REQUIRE(json5::parse("\"a\\\rb\"").as_string() == "ab");
    REQUIRE(json5::parse("\"a\\\r\nb\"").as_string() == "ab");
    REQUIRE(json5::parse("'a\\\nb'").as_string() == "ab");
    REQUIRE(json5::parse("\"abc\\\n\"").as_string() == "abc");
}

TEST_CASE("json5 parse - comments", "[willow][json5]") {
    REQUIRE(json5::parse("// line\n42").as_integer() == 42);
    REQUIRE(json5::parse("// line\r\n42").as_integer() == 42);
    REQUIRE(json5::parse("/* block */42").as_integer() == 42);
    REQUIRE(json5::parse("42 // trailing").as_integer() == 42);
    REQUIRE(json5::parse("42 /* trailing */").as_integer() == 42);
    REQUIRE(json5::parse("/**/42").as_integer() == 42);
    REQUIRE(json5::parse("/* /* not nested */ 42").as_integer() == 42);
    REQUIRE(json5::parse("/***/42").as_integer() == 42);
    REQUIRE(json5::parse("[1, /*c*/ 2]").size() == 2);
    REQUIRE(json5::parse("{// c\n a: 1}").is_object());
}

TEST_CASE("json5 parse - trailing commas", "[willow][json5]") {
    REQUIRE(json5::parse("[1,2,]").size() == 2);
    REQUIRE(json5::parse("[1,]").size() == 1);
    REQUIRE(json5::parse("[[1,],]").size() == 1);
    REQUIRE(json5::parse("{a:1,}").is_object());
    REQUIRE(json5::parse("{a:1,}").size() == 1);
    REQUIRE(json5::parse("{a:[1,],b:{c:2,},}").size() == 2);
    REQUIRE(json5::parse("[1,2,] // c").size() == 2);
}

TEST_CASE("json5 parse - unquoted object keys", "[willow][json5]") {
    REQUIRE(json5::parse("{a:1}").is_object());
    REQUIRE(json5::parse("{a:1}")["a"].as_integer() == 1);
    REQUIRE(json5::parse("{$:1}")["$"].as_integer() == 1);
    REQUIRE(json5::parse("{_:1}")["_"].as_integer() == 1);
    REQUIRE(json5::parse("{a1:1}")["a1"].as_integer() == 1);
    REQUIRE(json5::parse("{true:1}")["true"].as_integer() == 1);
    REQUIRE(json5::parse("{null:'x'}")["null"].as_string() == "x");
    REQUIRE(json5::parse("{if:1}")["if"].as_integer() == 1);
    REQUIRE(std::isnan(json5::parse("{Infinity:NaN}")["Infinity"].as_float()));
    REQUIRE(json5::parse("{NaN:1}")["NaN"].as_integer() == 1);
    REQUIRE(json5::parse("{a: true}")["a"].as_bool());
    REQUIRE(json5::parse("{true: false}")["true"].is_bool());
    REQUIRE(!json5::parse("{true: false}")["true"].as_bool());
    REQUIRE(json5::parse("{a: {b: 1}}")["a"]["b"].as_integer() == 1);
}

TEST_CASE("json5 parse - extended whitespace", "[willow][json5]") {
    REQUIRE(json5::parse("\v42").as_integer() == 42);
    REQUIRE(json5::parse("\f42").as_integer() == 42);
    REQUIRE(json5::parse("\xA0" "42").as_integer() == 42);
    REQUIRE(json5::parse("\v42\f").as_integer() == 42);
}

TEST_CASE("json5 parse - empty containers", "[willow][json5]") {
    json5::facade<document> arr = json5::parse("[]");
    REQUIRE(arr.is_array());
    REQUIRE(arr.empty());

    json5::facade<document> obj = json5::parse("{}");
    REQUIRE(obj.is_object());
    REQUIRE(obj.empty());
}

TEST_CASE("json5 parse - nested structures", "[willow][json5]") {
    json5::facade<document> j = json5::parse("{a: {b: [1, {c: 'd'}],}, e: .5, // tail\n}");
    REQUIRE(j.is_object());
    REQUIRE(j["a"].is_object());
    REQUIRE(j["a"]["b"].is_array());
    REQUIRE(j["a"]["b"][0].as_integer() == 1);
    REQUIRE(j["a"]["b"][1]["c"].as_string() == "d");
    REQUIRE(j["e"].as_float() == 0.5);
}

TEST_CASE("json5 parse - string_view overload", "[willow][json5]") {
    rainy::core::text::string_view text = "{k: 9,}";
    json5::facade<document> j = json5::parse(text);
    REQUIRE(j["k"].as_integer() == 9);
}

TEST_CASE("json5 parse - complex configuration with all features", "[willow][json5]") {
    rainy::text::string source = R"({
        // top-level comment
        server: {
            host: 'example.com',
            port: 0x1F90, // 8080
            tls: {enabled: true, cert: null, ciphers: ['TLS13', "TLS12",],},
            backups: [
                {name: "a", weight: 1.5,},
                {name: 'b', weight: -.25,}, /* second backup */
            ],
        },
        "limits": {
            max_conn: +1000,
            timeout: 30.,
            retries: [1, 2, 4, 8,],
            factor: .0625,
            floor: -Infinity,
            placeholder: NaN,
        },
        $meta: {version: 0x2, _dirty: false,},
    })";
    json5::facade<document> j = json5::parse(source);
    REQUIRE(j.is_object());
    REQUIRE(j.size() == 3);
    REQUIRE(j["server"]["host"].as_string() == "example.com");
    REQUIRE(j["server"]["port"].is_integer());
    REQUIRE(j["server"]["port"].as_integer() == 8080);
    REQUIRE(j["server"]["tls"]["enabled"].as_bool());
    REQUIRE(j["server"]["tls"]["cert"].is_null());
    REQUIRE(j["server"]["tls"]["ciphers"].size() == 2);
    REQUIRE(j["server"]["tls"]["ciphers"][1].as_string() == "TLS12");
    REQUIRE(j["server"]["backups"].size() == 2);
    REQUIRE(j["server"]["backups"][0]["name"].as_string() == "a");
    REQUIRE(j["server"]["backups"][0]["weight"].as_float() == 1.5);
    REQUIRE(j["server"]["backups"][1]["name"].as_string() == "b");
    REQUIRE(j["server"]["backups"][1]["weight"].as_float() == -0.25);
    REQUIRE(j["limits"]["max_conn"].is_integer());
    REQUIRE(j["limits"]["max_conn"].as_integer() == 1000);
    REQUIRE(j["limits"]["timeout"].is_float());
    REQUIRE(j["limits"]["timeout"].as_float() == 30.0);
    REQUIRE(j["limits"]["retries"].size() == 4);
    REQUIRE(j["limits"]["retries"][3].as_integer() == 8);
    REQUIRE(j["limits"]["factor"].as_float() == 0.0625);
    REQUIRE(std::isinf(j["limits"]["floor"].as_float()));
    REQUIRE(j["limits"]["floor"].as_float() < 0);
    REQUIRE(std::isnan(j["limits"]["placeholder"].as_float()));
    REQUIRE(j["$meta"]["version"].as_integer() == 2);
    REQUIRE(!j["$meta"]["_dirty"].as_bool());
}

TEST_CASE("json5 parse - deep nested mixed structure with comments", "[willow][json5]") {
    constexpr int depth = 64;
    rainy::text::string source;
    for (int i = 0; i < depth; ++i) {
        if (i % 2 == 0) {
            source += "[/* level " + std::to_string(i) + " */";
        } else {
            source += "{\"k" + std::to_string(i) + "\": // key " + std::to_string(i) + "\n";
        }
    }
    source += "42";
    for (int i = depth - 1; i >= 0; --i) {
        source += (i % 2 == 0) ? ",]" : ",}";
    }
    json5::facade<document> j = json5::parse(source);
    json5::facade<document> node = j;
    for (int i = 0; i < depth; ++i) {
        if (i % 2 == 0) {
            REQUIRE(node.is_array());
            REQUIRE(node.size() == 1);
            json5::facade<document> child = node[0];
            node = child;
        } else {
            REQUIRE(node.is_object());
            const rainy::text::string key = "k" + std::to_string(i);
            json5::facade<document> child = node[key.c_str()];
            node = child;
        }
    }
    REQUIRE(node.is_integer());
    REQUIRE(node.as_integer() == 42);
}

TEST_CASE("json5 parse - large mixed array with trailing comma", "[willow][json5]") {
    rainy::text::string source = "[";
    for (int i = 0; i < 250; ++i) {
        if (i) {
            source += ',';
        }
        switch (i % 4) {
            case 0:
                source += std::to_string(i);
                break;
            case 1:
                source += std::to_string(i) + ".5";
                break;
            case 2:
                source += "'s" + std::to_string(i) + "'";
                break;
            default:
                source += (i % 8 == 7) ? "null" : "true";
                break;
        }
    }
    source += ",]";
    json5::facade<document> j = json5::parse(source);
    REQUIRE(j.is_array());
    REQUIRE(j.size() == 250);
    REQUIRE(j[0].is_integer());
    REQUIRE(j[0].as_integer() == 0);
    REQUIRE(j[1].as_float() == 1.5);
    REQUIRE(j[2].as_string() == "s2");
    REQUIRE(j[3].is_bool());
    REQUIRE(j[4].as_integer() == 4);
    REQUIRE(j[7].is_null());
    REQUIRE(j[246].as_string() == "s246");
    REQUIRE(j[11].as_bool());
    REQUIRE(j[248].as_integer() == 248);
    REQUIRE(j[249].as_float() == 249.5);
}

TEST_CASE("json5 parse - nested empty containers with trailing commas", "[willow][json5]") {
    json5::facade<document> j = json5::parse("{a:[[],[{},],], b:{c:{},}, d:[],}");
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
    REQUIRE(j["d"].is_array());
    REQUIRE(j["d"].empty());
}
