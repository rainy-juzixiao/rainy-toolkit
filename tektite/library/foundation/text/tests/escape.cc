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
#include <rainy/foundation/text/escape/escape.hpp>

using rainy::core::text::string;
using rainy::core::text::string_view;
using rainy::core::text::wstring;
using rainy::core::text::wstring_view;
using rainy::foundation::text::escape::escape_c;
using rainy::foundation::text::escape::escape_error;
using rainy::foundation::text::escape::escape_json;
using rainy::foundation::text::escape::unescape_c;
using rainy::foundation::text::escape::unescape_json;

namespace {
    template <typename CharType>
    bool roundtrip_c(const rainy::core::text::basic_string<CharType> &text) {
        rainy::core::text::basic_string<CharType> escaped;
        escape_c(rainy::core::text::basic_string_view<CharType>(text), escaped);
        rainy::core::text::basic_string<CharType> restored;
        escape_error error = escape_error::none;
        if (!unescape_c(rainy::core::text::basic_string_view<CharType>(escaped), restored, error)) {
            return false;
        }
        return error == escape_error::none && restored == text;
    }

    template <typename CharType>
    bool roundtrip_json(const rainy::core::text::basic_string<CharType> &text) {
        rainy::core::text::basic_string<CharType> escaped;
        escape_json(rainy::core::text::basic_string_view<CharType>(text), escaped);
        rainy::core::text::basic_string<CharType> restored;
        escape_error error = escape_error::none;
        if (!unescape_json(rainy::core::text::basic_string_view<CharType>(escaped), restored, error)) {
            return false;
        }
        return error == escape_error::none && restored == text;
    }
}

TEST_CASE("escape C dialect", "[foundation][text][escape]") {
    SECTION("named escapes") {
        string text = "\a\b\f\n\r\t\v\\\"'";
        string escaped;
        escape_c(string_view(text), escaped);
        REQUIRE(escaped == "\\a\\b\\f\\n\\r\\t\\v\\\\\\\"\\'");
    }
    SECTION("null becomes backslash zero") {
        string text;
        text.push_back('\0');
        string escaped;
        escape_c(string_view(text), escaped);
        REQUIRE(escaped == "\\0");
    }
    SECTION("control units become hex") {
        string text;
        text.push_back(static_cast<char>(0x01));
        text.push_back(static_cast<char>(0x1F));
        text.push_back(static_cast<char>(0x7F));
        string escaped;
        escape_c(string_view(text), escaped);
        REQUIRE(escaped == "\\x01\\x1f\\x7f");
    }
    SECTION("printable text passes through") {
        string text = "hello world";
        string escaped;
        escape_c(string_view(text), escaped);
        REQUIRE(escaped == text);
    }
    SECTION("roundtrip ascii") {
        REQUIRE(roundtrip_c(string("plain text")));
    }
    SECTION("roundtrip with escapes") {
        string text = "line1\nline2\ttab\\slash\"quote";
        REQUIRE(roundtrip_c(text));
    }
    SECTION("roundtrip control units") {
        string text;
        for (int value = 1; value < 0x20; ++value) {
            text.push_back(static_cast<char>(value));
        }
        REQUIRE(roundtrip_c(text));
    }
    SECTION("wide characters survive") {
        wstring text = L"你好\t世界";
        wstring escaped;
        escape_c(wstring_view(text), escaped);
        REQUIRE(escaped == L"你好\\t世界");
        REQUIRE(roundtrip_c(text));
    }
    SECTION("hex escapes and unicode escapes") {
        string escaped = "\\x41\\u4F60\\u597D";
        string restored;
        escape_error error = escape_error::none;
        REQUIRE(unescape_c(string_view(escaped), restored, error));
        REQUIRE(error == escape_error::none);
        REQUIRE(restored == "A\xE4\xBD\xA0\xE5\xA5\xBD");
    }
    SECTION("surrogate pair escape") {
        string escaped = "\\uD83C\\uDC04";
        string restored;
        escape_error error = escape_error::none;
        REQUIRE(unescape_c(string_view(escaped), restored, error));
        REQUIRE(error == escape_error::none);
        REQUIRE(restored == "\xF0\x9F\x80\x84");
    }
    SECTION("output iterator overload matches string overload") {
        string text = "a\tb\\c";
        string via_iterator;
        escape_c(string_view(text), rainy::utility::back_insert_iterator<string>(via_iterator));
        string via_string;
        escape_c(string_view(text), via_string);
        REQUIRE(via_iterator == via_string);
    }
}

TEST_CASE("escape C dialect rejects invalid input", "[foundation][text][escape]") {
    SECTION("trailing backslash") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("abc\\")), restored, error));
        REQUIRE(error == escape_error::unexpected_end);
    }
    SECTION("unknown escape") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("\\q")), restored, error));
        REQUIRE(error == escape_error::unknown_escape);
    }
    SECTION("truncated hex escape") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("\\x4")), restored, error));
        REQUIRE(error == escape_error::unexpected_end);
    }
    SECTION("bad hex digit") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("\\xZZ")), restored, error));
        REQUIRE(error == escape_error::invalid_hex_digit);
    }
    SECTION("truncated unicode escape") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("\\u12")), restored, error));
        REQUIRE(error == escape_error::unexpected_end);
    }
    SECTION("lone high surrogate") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("\\uD83C")), restored, error));
        REQUIRE(error == escape_error::invalid_code_point);
    }
    SECTION("lone low surrogate") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_c(string_view(string("\\uDC04")), restored, error));
        REQUIRE(error == escape_error::invalid_code_point);
    }
}

TEST_CASE("escape JSON dialect", "[foundation][text][escape]") {
    SECTION("quote and backslash") {
        string text = "\"\\";
        string escaped;
        escape_json(string_view(text), escaped);
        REQUIRE(escaped == "\\\"\\\\");
    }
    SECTION("short escapes") {
        string text = "\b\f\n\r\t";
        string escaped;
        escape_json(string_view(text), escaped);
        REQUIRE(escaped == "\\b\\f\\n\\r\\t");
    }
    SECTION("other control units become unicode escapes") {
        string text;
        text.push_back(static_cast<char>(0x01));
        text.push_back(static_cast<char>(0x1F));
        string escaped;
        escape_json(string_view(text), escaped);
        REQUIRE(escaped == "\\u0001\\u001f");
    }
    SECTION("printable text passes through") {
        string text = "hello 你好";
        string escaped;
        escape_json(string_view(text), escaped);
        REQUIRE(escaped == text);
    }
    SECTION("roundtrip ascii") {
        REQUIRE(roundtrip_json(string("plain text")));
    }
    SECTION("roundtrip with escapes") {
        string text = "quote\" backslash\\ newline\n tab\t";
        REQUIRE(roundtrip_json(text));
    }
    SECTION("roundtrip control units") {
        string text;
        for (int value = 1; value < 0x20; ++value) {
            text.push_back(static_cast<char>(value));
        }
        REQUIRE(roundtrip_json(text));
    }
    SECTION("solidus escape is accepted") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE(unescape_json(string_view(string("a\\/b")), restored, error));
        REQUIRE(error == escape_error::none);
        REQUIRE(restored == "a/b");
    }
    SECTION("surrogate pair escape") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE(unescape_json(string_view(string("\\uD83C\\uDC04")), restored, error));
        REQUIRE(error == escape_error::none);
        REQUIRE(restored == "\xF0\x9F\x80\x84");
    }
    SECTION("wide characters survive") {
        wstring text = L"a\tb\"c";
        REQUIRE(roundtrip_json(text));
    }
    SECTION("output iterator overload matches string overload") {
        string text = "a\tb\"c";
        string via_iterator;
        escape_json(string_view(text), rainy::utility::back_insert_iterator<string>(via_iterator));
        string via_string;
        escape_json(string_view(text), via_string);
        REQUIRE(via_iterator == via_string);
    }
}

TEST_CASE("escape JSON dialect rejects invalid input", "[foundation][text][escape]") {
    SECTION("trailing backslash") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("abc\\")), restored, error));
        REQUIRE(error == escape_error::unexpected_end);
    }
    SECTION("C only escape is rejected") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("\\a")), restored, error));
        REQUIRE(error == escape_error::unknown_escape);
    }
    SECTION("truncated unicode escape") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("\\u12")), restored, error));
        REQUIRE(error == escape_error::unexpected_end);
    }
    SECTION("bad hex digit") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("\\u12G4")), restored, error));
        REQUIRE(error == escape_error::invalid_hex_digit);
    }
    SECTION("lone high surrogate") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("\\uD83C")), restored, error));
        REQUIRE(error == escape_error::invalid_code_point);
    }
    SECTION("high surrogate followed by non escape") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("\\uD83Cx")), restored, error));
        REQUIRE(error == escape_error::invalid_code_point);
    }
    SECTION("lone low surrogate") {
        string restored;
        escape_error error = escape_error::none;
        REQUIRE_FALSE(unescape_json(string_view(string("\\uDC04")), restored, error));
        REQUIRE(error == escape_error::invalid_code_point);
    }
}
