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
#include <rainy/core/text/char_traits.hpp>
#include <array>
#include <vector>

using rainy::core::text::char_traits;

TEST_CASE("char_traits assign single character", "[char_traits]") {
    char_traits<char>::char_type to = 'a';
    char_traits<char>::char_type from = 'b';
    char_traits<char>::assign(to, from);
    REQUIRE(to == 'b');
}

TEST_CASE("char_traits assign array", "[char_traits]") {
    char_traits<char>::char_type arr[5] = {'a', 'b', 'c', 'd', 'e'};
    char_traits<char>::assign(arr, 5, 'x');
    for (int i = 0; i < 5; ++i) {
        REQUIRE(arr[i] == 'x');
    }
}

TEST_CASE("char_traits eq", "[char_traits]") {
    REQUIRE(char_traits<char>::eq('a', 'a') == true);
    REQUIRE(char_traits<char>::eq('a', 'b') == false);
    REQUIRE(char_traits<wchar_t>::eq(L'a', L'a') == true);
    REQUIRE(char_traits<wchar_t>::eq(L'a', L'b') == false);
}

TEST_CASE("char_traits eq_int_type", "[char_traits]") {
    REQUIRE(char_traits<char>::eq_int_type(1, 1) == true);
    REQUIRE(char_traits<char>::eq_int_type(1, 2) == false);
}

TEST_CASE("char_traits compare", "[char_traits]") {
    SECTION("equal strings") {
        const char* s1 = "hello";
        const char* s2 = "hello";
        REQUIRE(char_traits<char>::compare(s1, s2, 5) == 0);
    }

    SECTION("s1 less than s2") {
        const char* s1 = "apple";
        const char* s2 = "banana";
        REQUIRE(char_traits<char>::compare(s1, s2, 5) < 0);
    }

    SECTION("s1 greater than s2") {
        const char* s1 = "banana";
        const char* s2 = "apple";
        REQUIRE(char_traits<char>::compare(s1, s2, 5) > 0);
    }

    SECTION("count zero") {
        const char* s1 = "hello";
        const char* s2 = "world";
        REQUIRE(char_traits<char>::compare(s1, s2, 0) == 0);
    }

    SECTION("same pointer") {
        const char* s = "hello";
        REQUIRE(char_traits<char>::compare(s, s, 5) == 0);
    }

    SECTION("char16_t") {
        const char16_t* s1 = u"hello";
        const char16_t* s2 = u"hello";
        REQUIRE(char_traits<char16_t>::compare(s1, s2, 5) == 0);
    }

    SECTION("char32_t") {
        const char32_t* s1 = U"hello";
        const char32_t* s2 = U"world";
        REQUIRE(char_traits<char32_t>::compare(s1, s2, 1) < 0);
    }
}

TEST_CASE("char_traits length", "[char_traits]") {
    SECTION("char") {
        const char* s = "hello";
        REQUIRE(char_traits<char>::length(s) == 5);
    }

    SECTION("wchar_t") {
        const wchar_t* s = L"hello";
        REQUIRE(char_traits<wchar_t>::length(s) == 5);
    }

    SECTION("char16_t") {
        const char16_t* s = u"hello";
        REQUIRE(char_traits<char16_t>::length(s) == 5);
    }

    SECTION("char32_t") {
        const char32_t* s = U"hello";
        REQUIRE(char_traits<char32_t>::length(s) == 5);
    }

    SECTION("empty string") {
        const char* s = "";
        REQUIRE(char_traits<char>::length(s) == 0);
    }
}

TEST_CASE("char_traits find", "[char_traits]") {
    SECTION("find existing character") {
        const char* s = "hello world";
        const char* result = char_traits<char>::find(s, 11, 'w');
        REQUIRE(result != nullptr);
        REQUIRE(*result == 'w');
        REQUIRE(result == s + 6);
    }

    SECTION("find character at beginning") {
        const char* s = "hello";
        const char* result = char_traits<char>::find(s, 5, 'h');
        REQUIRE(result != nullptr);
        REQUIRE(*result == 'h');
        REQUIRE(result == s);
    }

    SECTION("find character at end") {
        const char* s = "hello";
        const char* result = char_traits<char>::find(s, 5, 'o');
        REQUIRE(result != nullptr);
        REQUIRE(*result == 'o');
        REQUIRE(result == s + 4);
    }

    SECTION("character not found") {
        const char* s = "hello";
        const char* result = char_traits<char>::find(s, 5, 'z');
        REQUIRE(result == nullptr);
    }

    SECTION("empty range") {
        const char* s = "hello";
        const char* result = char_traits<char>::find(s, 0, 'h');
        REQUIRE(result == nullptr);
    }

    SECTION("char16_t") {
        const char16_t* s = u"hello";
        const char16_t* result = char_traits<char16_t>::find(s, 5, u'e');
        REQUIRE(result != nullptr);
        REQUIRE(*result == u'e');
    }
}

TEST_CASE("char_traits lt", "[char_traits]") {
    REQUIRE(char_traits<char>::lt('a', 'b') == true);
    REQUIRE(char_traits<char>::lt('b', 'a') == false);
    REQUIRE(char_traits<char>::lt('a', 'a') == false);
}

TEST_CASE("char_traits move", "[char_traits]") {
    SECTION("move forward") {
        char src[] = "hello";
        char dest[6];
        char_traits<char>::move(dest, src, 6);
        REQUIRE(std::string(dest) == "hello");
    }

    SECTION("move with overlapping - forward") {
        char buffer[] = "hello world";
        char_traits<char>::move(buffer + 3, buffer, 5);
        REQUIRE(buffer[0] == 'h');
        REQUIRE(buffer[3] == 'h');
        REQUIRE(buffer[4] == 'e');
    }

    SECTION("move with overlapping - backward") {
        char buffer[] = "hello world";
        char_traits<char>::move(buffer, buffer + 3, 5);
        REQUIRE(buffer[0] == 'l');
        REQUIRE(buffer[1] == 'o');
    }

    SECTION("move to std::array") {
        std::array<char, 6> dest{};
        const char* src = "hello";
        char_traits<char>::move(dest, src, 5);
        REQUIRE(std::string(dest.data()) == "hello");
    }

    SECTION("move to rainy::collections::array") {
        rainy::collections::array<char, 6> dest{};
        const char* src = "hello";
        char_traits<char>::move(dest, src, 5);
        REQUIRE(std::string(dest.data()) == "hello");
    }

    SECTION("move to array_view") {
        char buffer[10] = {};
        auto view = rainy::collections::views::array_view<char>(buffer, 10);
        const char* src = "hello";
        char_traits<char>::move(view, src, 5);
        REQUIRE(std::string(buffer) == "hello");
    }

    SECTION("move to vector") {
        std::vector<char> dest(10);
        const char* src = "hello";
        char_traits<char>::move(dest, src, 5);
        REQUIRE(std::string(dest.data()) == "hello");
    }

    SECTION("move_s with array reference") {
        char dest[10];
        const char* src = "hello";
        char_traits<char>::move_s(dest, src, 5);
        REQUIRE(std::string(dest) == "hello");
    }

    SECTION("move_s with size parameter") {
        char dest[10] = {};
        const char* src = "hello";
        char_traits<char>::move_s(dest, 10, src, 5);
        REQUIRE(std::string(dest) == "hello");
    }

    SECTION("move_s fails when dest_size less than count") {
        char dest[5];
        const char* src = "hello";
        char* result = char_traits<char>::move_s(dest, 3, src, 5);
        REQUIRE(result == nullptr);
    }
}

TEST_CASE("char_traits copy", "[char_traits]") {
    SECTION("copy normal") {
        char src[] = "hello";
        char dest[6];
        char_traits<char>::copy(dest, src, 6);
        REQUIRE(std::string(dest) == "hello");
    }

    SECTION("copy empty") {
        char src[] = "";
        char dest[1];
        char_traits<char>::copy(dest, src, 1);
        REQUIRE(dest[0] == '\0');
    }
}

TEST_CASE("char_traits to_int_type to_char_type", "[char_traits]") {
    char ch = 'a';
    auto int_val = char_traits<char>::to_int_type(ch);
    REQUIRE(int_val == 'a');
    auto char_val = char_traits<char>::to_char_type(int_val);
    REQUIRE(char_val == 'a');
}

TEST_CASE("char_traits eof not_eof", "[char_traits]") {
    auto eof_val = char_traits<char>::eof();
    REQUIRE_FALSE(char_traits<char>::not_eof(eof_val));
    REQUIRE(char_traits<char>::not_eof('a'));
}

TEST_CASE("char_traits with wchar_t", "[char_traits]") {
    SECTION("compare") {
        const wchar_t* s1 = L"hello";
        const wchar_t* s2 = L"hello";
        REQUIRE(char_traits<wchar_t>::compare(s1, s2, 5) == 0);
    }

    SECTION("length") {
        const wchar_t* s = L"hello";
        REQUIRE(char_traits<wchar_t>::length(s) == 5);
    }

    SECTION("find") {
        const wchar_t* s = L"hello";
        const wchar_t* result = char_traits<wchar_t>::find(s, 5, L'e');
        REQUIRE(result != nullptr);
        REQUIRE(*result == L'e');
    }
}

TEST_CASE("char_traits compare with unrolled loop for char16_t", "[char_traits]") {
    SECTION("equal strings length 8") {
        const char16_t* s1 = u"12345678";
        const char16_t* s2 = u"12345678";
        REQUIRE(char_traits<char16_t>::compare(s1, s2, 8) == 0);
    }

    SECTION("different strings length 8") {
        const char16_t* s1 = u"12345678";
        const char16_t* s2 = u"12345679";
        REQUIRE(char_traits<char16_t>::compare(s1, s2, 8) < 0);
    }

    SECTION("equal strings length 5") {
        const char16_t* s1 = u"12345";
        const char16_t* s2 = u"12345";
        REQUIRE(char_traits<char16_t>::compare(s1, s2, 5) == 0);
    }

    SECTION("equal strings length 3") {
        const char16_t* s1 = u"123";
        const char16_t* s2 = u"123";
        REQUIRE(char_traits<char16_t>::compare(s1, s2, 3) == 0);
    }
}