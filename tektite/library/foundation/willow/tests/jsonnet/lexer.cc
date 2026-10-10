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
#include <rainy/foundation/willow/implements/jsonnet/lexer.hpp>

using namespace rainy::foundation::willow::jsonnet::implements;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_parse_error;

namespace {
    token lex_one(const rainy::core::text::string &source) {
        jsonnet_lexer lexer(source.data(), source.size());
        return lexer.scan();
    }

    std::vector<token> lex_all(const rainy::core::text::string &source) {
        jsonnet_lexer lexer(source.data(), source.size());
        std::vector<token> tokens;
        while (true) {
            token t = lexer.scan();
            if (t.type == token_type::end_of_input) {
                break;
            }
            tokens.push_back(rainy::utility::move(t));
        }
        return tokens;
    }
}

TEST_CASE("jsonnet lexer - numbers with separators", "[jsonnet][lexer]") {
    REQUIRE(lex_one("1_000_000").type == token_type::literal_number);
    REQUIRE(lex_one("1_000_000").number == 1000000.0);
    REQUIRE(lex_one("0.000_001").number == 0.000001);
    REQUIRE(lex_one("6.022_140_76e23").number == 6.02214076e23);
    REQUIRE(lex_one("1e5").type == token_type::literal_number);
    REQUIRE_FALSE(lex_one("1e5").is_integer);
    REQUIRE(lex_one("42").is_integer);
}

TEST_CASE("jsonnet lexer - double and single quoted strings", "[jsonnet][lexer]") {
    REQUIRE(lex_one("\"hello\"").text == "hello");
    REQUIRE(lex_one("'world'").text == "world");
    REQUIRE(lex_one("\"a\\nb\"").text == "a\nb");
    REQUIRE(lex_one("\"\\u4e2d\\u6587\"").text == "\xe4\xb8\xad\xe6\x96\x87");
    REQUIRE(lex_one("\"\\ud83d\\ude00\"").text == "\xf0\x9f\x98\x80");
}

TEST_CASE("jsonnet lexer - verbatim strings do not process escapes", "[jsonnet][lexer]") {
    REQUIRE(lex_one("@\"a\\nb\"").text == "a\\nb");
    REQUIRE(lex_one("@'it''s'").text == "it's");
    REQUIRE(lex_one("@\"say \"\"hi\"\"\"").text == "say \"hi\"");
}

TEST_CASE("jsonnet lexer - text blocks strip the common prefix", "[jsonnet][lexer]") {
    const auto token = lex_one("|||\n  hello\n  world\n|||");
    REQUIRE(token.type == token_type::literal_string);
    REQUIRE(token.text == "hello\nworld\n");
}

TEST_CASE("jsonnet lexer - text block chomps the trailing newline", "[jsonnet][lexer]") {
    const auto token = lex_one("|||-\n  a\n  b\n|||");
    REQUIRE(token.text == "a\nb");
}

TEST_CASE("jsonnet lexer - comments are skipped", "[jsonnet][lexer]") {
    REQUIRE(lex_one("# line\n1").type == token_type::literal_number);
    REQUIRE(lex_one("// line\n1").type == token_type::literal_number);
    REQUIRE(lex_one("/* block */ 1").type == token_type::literal_number);
    REQUIRE(lex_one("/* multi\nline */ 1").type == token_type::literal_number);
}

TEST_CASE("jsonnet lexer - keywords are recognized", "[jsonnet][lexer]") {
    REQUIRE(lex_one("local").type == token_type::kw_local);
    REQUIRE(lex_one("function").type == token_type::kw_function);
    REQUIRE(lex_one("self").type == token_type::kw_self);
    REQUIRE(lex_one("super").type == token_type::kw_super);
    REQUIRE(lex_one("import").type == token_type::kw_import);
    REQUIRE(lex_one("importstr").type == token_type::kw_importstr);
    REQUIRE(lex_one("importbin").type == token_type::kw_importbin);
    REQUIRE(lex_one("null").type == token_type::literal_null);
    REQUIRE(lex_one("true").type == token_type::literal_true);
    REQUIRE(lex_one("false").type == token_type::literal_false);
    REQUIRE(lex_one("in").type == token_type::kw_in);
    REQUIRE(lex_one("tailstrict").type == token_type::kw_tailstrict);
    REQUIRE(lex_one("foo").type == token_type::identifier);
    REQUIRE(lex_one("_bar1").type == token_type::identifier);
}

TEST_CASE("jsonnet lexer - operators use longest match", "[jsonnet][lexer]") {
    REQUIRE(lex_one(":::").type == token_type::triple_colon);
    REQUIRE(lex_one("::").type == token_type::double_colon);
    REQUIRE(lex_one(":").type == token_type::colon);
    REQUIRE(lex_one("==").type == token_type::op_equal);
    REQUIRE(lex_one("!=").type == token_type::op_not_equal);
    REQUIRE(lex_one("<=").type == token_type::op_less_equal);
    REQUIRE(lex_one(">=").type == token_type::op_greater_equal);
    REQUIRE(lex_one("<<").type == token_type::op_shift_left);
    REQUIRE(lex_one(">>").type == token_type::op_shift_right);
    REQUIRE(lex_one("&&").type == token_type::op_and);
    REQUIRE(lex_one("||").type == token_type::op_or);
    REQUIRE(lex_one("$").type == token_type::dollar);
}

TEST_CASE("jsonnet lexer - symbols", "[jsonnet][lexer]") {
    REQUIRE(lex_one("{").type == token_type::brace_left);
    REQUIRE(lex_one("}").type == token_type::brace_right);
    REQUIRE(lex_one("[").type == token_type::bracket_left);
    REQUIRE(lex_one("]").type == token_type::bracket_right);
    REQUIRE(lex_one("(").type == token_type::paren_left);
    REQUIRE(lex_one(")").type == token_type::paren_right);
    REQUIRE(lex_one(",").type == token_type::comma);
    REQUIRE(lex_one(".").type == token_type::dot);
    REQUIRE(lex_one(";").type == token_type::semicolon);
}

TEST_CASE("jsonnet lexer - tracks source positions", "[jsonnet][lexer]") {
    const auto tokens = lex_all("local\n  x = 1");
    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens[0].span.begin.line == 1);
    REQUIRE(tokens[0].span.begin.column == 1);
    REQUIRE(tokens[1].span.begin.line == 2);
    REQUIRE(tokens[1].span.begin.column == 3);
}

TEST_CASE("jsonnet lexer - unterminated constructs raise errors", "[jsonnet][lexer]") {
    REQUIRE_THROWS_AS(lex_one("\"unterminated"), jsonnet_parse_error);
    REQUIRE_THROWS_AS(lex_one("/* unterminated"), jsonnet_parse_error);
}
