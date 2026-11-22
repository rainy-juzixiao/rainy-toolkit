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
#include <rainy/foundation/text/unicode/unicode.hpp>
#include <string>

using rainy::core::text::string;
using rainy::core::text::string_view;
using rainy::core::text::u16string;
using rainy::core::text::u16string_view;
using rainy::core::text::u32string;
using rainy::core::text::u32string_view;
using rainy::foundation::text::unicode::codepoint_begin;
using rainy::foundation::text::unicode::codepoint_end;
using rainy::foundation::text::unicode::codepoint_iterator;
using rainy::foundation::text::unicode::codepoint_length;
using rainy::foundation::text::unicode::convert_encoding;
using rainy::foundation::text::unicode::convert_encoding_replace;
using rainy::foundation::text::unicode::encoding_error;
using rainy::foundation::text::unicode::replacement_code_point;
using rainy::foundation::text::unicode::to_utf16;
using rainy::foundation::text::unicode::to_utf32;
using rainy::foundation::text::unicode::to_utf8;
using rainy::foundation::text::unicode::validate_utf16;
using rainy::foundation::text::unicode::validate_utf32;
using rainy::foundation::text::unicode::validate_utf8;

namespace {
    template <typename From, typename To>
    bool roundtrip(const rainy::core::text::basic_string<From> &text) {
        rainy::core::text::basic_string<To> forward;
        encoding_error error = encoding_error::none;
        if (!convert_encoding(rainy::core::text::basic_string_view<From>(text), forward, error)) {
            return false;
        }
        if (error != encoding_error::none) {
            return false;
        }
        rainy::core::text::basic_string<From> backward;
        if (!convert_encoding(rainy::core::text::basic_string_view<To>(forward), backward, error)) {
            return false;
        }
        return error == encoding_error::none && backward == text;
    }
}

TEST_CASE("unicode convert_encoding roundtrips", "[foundation][text][unicode]") {
    SECTION("ascii") {
        string text = "hello, world!";
        REQUIRE(roundtrip<char, char16_t>(text));
        REQUIRE(roundtrip<char, char32_t>(text));
    }
    SECTION("two byte code points") {
        string text = "こんにちは";
        REQUIRE(roundtrip<char, char16_t>(text));
        REQUIRE(roundtrip<char, char32_t>(text));
    }
    SECTION("three byte code points") {
        string text = "你好，世界";
        REQUIRE(roundtrip<char, char16_t>(text));
        REQUIRE(roundtrip<char, char32_t>(text));
    }
    SECTION("four byte code points") {
        string text = "\xF0\x9F\x80\x84\xF0\x9F\x88\x9A";
        REQUIRE(roundtrip<char, char16_t>(text));
        REQUIRE(roundtrip<char, char32_t>(text));
    }
    SECTION("utf16 to utf8") {
        u16string input = u"你好";
        input.push_back(static_cast<char16_t>(0xD83C));
        input.push_back(static_cast<char16_t>(0xDC04));
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE(convert_encoding(u16string_view(input), output, error));
        REQUIRE(error == encoding_error::none);
        REQUIRE(output == "\xE4\xBD\xA0\xE5\xA5\xBD\xF0\x9F\x80\x84");
    }
    SECTION("utf32 to utf16") {
        u32string input = U"你好";
        input.push_back(U'\U0001F004');
        u16string output;
        encoding_error error = encoding_error::none;
        REQUIRE(convert_encoding(u32string_view(input), output, error));
        REQUIRE(error == encoding_error::none);
        u16string expected = u"你好";
        expected.push_back(static_cast<char16_t>(0xD83C));
        expected.push_back(static_cast<char16_t>(0xDC04));
        REQUIRE(output == expected);
    }
    SECTION("same type passthrough") {
        string input = "identity";
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE(convert_encoding(string_view(input), output, error));
        REQUIRE(output == input);
    }
    SECTION("to_utf8 to_utf16 to_utf32 entry points") {
        u32string input = U"ab\U0001F004";
        u32string codepoints;
        encoding_error error = encoding_error::none;
        REQUIRE(to_utf32(u32string_view(input), codepoints, error));
        REQUIRE(codepoints == input);
        string narrow;
        REQUIRE(to_utf8(u32string_view(input), narrow, error));
        u16string wide;
        REQUIRE(to_utf16(u32string_view(input), wide, error));
        u32string back;
        REQUIRE(to_utf32(u16string_view(wide), back, error));
        REQUIRE(back == input);
    }
}

TEST_CASE("unicode convert_encoding rejects invalid sequences", "[foundation][text][unicode]") {
    SECTION("stray continuation byte") {
        string text = "a\x80z";
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(string_view(text), output, error));
        REQUIRE(error == encoding_error::invalid_leading_element);
    }
    SECTION("missing continuation byte") {
        string text = "a\xE4\xBD";
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(string_view(text), output, error));
        REQUIRE(error == encoding_error::truncated_sequence);
    }
    SECTION("bad continuation byte") {
        string text = "a\xE4\xFF\xBD";
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(string_view(text), output, error));
        REQUIRE(error == encoding_error::invalid_continuation_element);
    }
    SECTION("overlong encoding") {
        string text = "a\xC0\xAF" "b";
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(string_view(text), output, error));
        REQUIRE(error == encoding_error::overlong_sequence);
    }
    SECTION("out of range") {
        string text = "a\xF5\x80\x80\x80";
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(string_view(text), output, error));
        REQUIRE(error == encoding_error::invalid_code_point);
    }
    SECTION("unpaired high surrogate in utf16") {
        u16string input = u"a";
        input.push_back(static_cast<char16_t>(0xD800));
        u32string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(u16string_view(input), output, error));
        REQUIRE(error == encoding_error::truncated_sequence);
    }
    SECTION("unpaired low surrogate in utf16") {
        u16string input = u"a";
        input.push_back(static_cast<char16_t>(0xDC00));
        u32string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(u16string_view(input), output, error));
        REQUIRE(error == encoding_error::invalid_leading_element);
    }
    SECTION("bad trail surrogate in utf16") {
        u16string input = u"a";
        input.push_back(static_cast<char16_t>(0xD800));
        input.push_back(u'b');
        u32string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(u16string_view(input), output, error));
        REQUIRE(error == encoding_error::invalid_continuation_element);
    }
    SECTION("surrogate in utf32") {
        u32string input = U"a";
        input.push_back(static_cast<char32_t>(0xD800));
        string output;
        encoding_error error = encoding_error::none;
        REQUIRE_FALSE(convert_encoding(u32string_view(input), output, error));
        REQUIRE(error == encoding_error::invalid_code_point);
    }
}

TEST_CASE("unicode convert_encoding_replace substitutes U+FFFD", "[foundation][text][unicode]") {
    SECTION("stray continuation bytes") {
        string text = "a\x80\x80z";
        string output;
        encoding_error error = convert_encoding_replace(string_view(text), output);
        REQUIRE(error == encoding_error::invalid_leading_element);
        REQUIRE(output == "a\xEF\xBF\xBD\xEF\xBF\xBDz");
    }
    SECTION("truncated tail") {
        string text = "\xE4\xBD\xA0\xE4";
        string output;
        encoding_error error = convert_encoding_replace(string_view(text), output);
        REQUIRE(error == encoding_error::truncated_sequence);
        REQUIRE(output == "\xE4\xBD\xA0\xEF\xBF\xBD");
    }
    SECTION("surrogate pair survives utf16 path") {
        u16string input = u"a";
        input.push_back(static_cast<char16_t>(0xD83C));
        input.push_back(static_cast<char16_t>(0xDC04));
        u32string output;
        encoding_error error = convert_encoding_replace(u16string_view(input), output);
        REQUIRE(error == encoding_error::none);
        u32string expected = U"a";
        expected.push_back(U'\U0001F004');
        REQUIRE(output == expected);
    }
}

TEST_CASE("unicode validation", "[foundation][text][unicode]") {
    SECTION("valid utf8") {
        REQUIRE(validate_utf8(string_view(string())));
        REQUIRE(validate_utf8(string_view(string("plain ascii"))));
        REQUIRE(validate_utf8(string_view(string("\xE4\xBD\xA0\xE5\xA5\xBD\xF0\x9F\x80\x84"))));
    }
    SECTION("invalid utf8") {
        REQUIRE_FALSE(validate_utf8(string_view(string("a\x80"))));
        REQUIRE_FALSE(validate_utf8(string_view(string("\xC0\xAF"))));
        REQUIRE_FALSE(validate_utf8(string_view(string("\xED\xA0\x80"))));
        REQUIRE_FALSE(validate_utf8(string_view(string("\xF4\x90\x80\x80"))));
    }
    SECTION("valid utf16") {
        REQUIRE(validate_utf16(u16string_view(u16string())));
        u16string good = u"hello";
        good.push_back(static_cast<char16_t>(0xD83C));
        good.push_back(static_cast<char16_t>(0xDC04));
        REQUIRE(validate_utf16(u16string_view(good)));
    }
    SECTION("invalid utf16") {
        u16string broken = u"a";
        broken.push_back(static_cast<char16_t>(0xD800));
        REQUIRE_FALSE(validate_utf16(u16string_view(broken)));
        u16string lone = u"a";
        lone.push_back(static_cast<char16_t>(0xDC00));
        REQUIRE_FALSE(validate_utf16(u16string_view(lone)));
    }
    SECTION("valid utf32") {
        u32string good = U"hello";
        good.push_back(U'\U0001F004');
        REQUIRE(validate_utf32(u32string_view(good)));
    }
    SECTION("invalid utf32") {
        u32string broken = U"a";
        broken.push_back(static_cast<char32_t>(0x110000));
        REQUIRE_FALSE(validate_utf32(u32string_view(broken)));
    }
}

TEST_CASE("unicode codepoint_length", "[foundation][text][unicode]") {
    REQUIRE(codepoint_length(string_view(string())) == 0);
    REQUIRE(codepoint_length(string_view(string("abc"))) == 3);
    REQUIRE(codepoint_length(string_view(string("\xE4\xBD\xA0\xE5\xA5\xBD"))) == 2);
    REQUIRE(codepoint_length(string_view(string("\xF0\x9F\x80\x84\xF0\x9F\x80\x84"))) == 2);
    REQUIRE(codepoint_length(string_view(string("a\x80z"))) == 3);
}

TEST_CASE("unicode codepoint_iterator", "[foundation][text][unicode]") {
    SECTION("traverses mixed widths") {
        string text = "a\xE4\xBD\xA0\xF0\x9F\x80\x84" "b";
        u32string collected;
        auto first = codepoint_begin(string_view(text));
        auto last = codepoint_end<char>();
        REQUIRE_FALSE(first == last);
        for (; first != last; ++first) {
            collected.push_back(*first);
        }
        u32string expected = U"a";
        expected.push_back(U'你');
        expected.push_back(U'\U0001F004');
        expected.push_back(U'b');
        REQUIRE(collected == expected);
    }
    SECTION("replaces malformed positions") {
        string text = "a\x80z";
        u32string collected;
        auto first = codepoint_begin(string_view(text));
        auto last = codepoint_end<char>();
        for (; first != last; ++first) {
            collected.push_back(*first);
        }
        u32string expected = U"a";
        expected.push_back(replacement_code_point);
        expected.push_back(U'z');
        REQUIRE(collected == expected);
    }
    SECTION("errored flag tracks replacement") {
        string text = "a\x80z";
        auto it = codepoint_begin(string_view(text));
        REQUIRE_FALSE(it.errored());
        ++it;
        REQUIRE(it.errored());
        REQUIRE(it.error() == encoding_error::invalid_leading_element);
        REQUIRE(*it == replacement_code_point);
        ++it;
        REQUIRE_FALSE(it.errored());
        REQUIRE(*it == U'z');
    }
    SECTION("empty range") {
        auto first = codepoint_begin(string_view(string()));
        auto last = codepoint_end<char>();
        REQUIRE(first == last);
    }
}
