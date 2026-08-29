#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <rainy/core/text/wstring_convert.hpp>
#include <string>
#include <vector>
#include <iostream>

using namespace rainy::core::text;

TEST_CASE("codecvt_utf8 to_wide basic conversions", "[codecvt_utf8]") {
    using codecvt = codecvt_utf8<wchar_t>;

    SECTION("ASCII characters") {
        const char* input = "Hello";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 5, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }

    SECTION("UTF-8 2-byte sequence") {
        const char input[] = "\xc3\xa9";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 2, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 2);
        REQUIRE(to_next - output == 1);
        REQUIRE(output[0] == 0xe9);
    }

    SECTION("UTF-8 3-byte sequence") {
        const char input[] = "\xe2\x82\xac";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 3, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 3);
        REQUIRE(to_next - output == 1);
        REQUIRE(output[0] == 0x20ac);
    }

    SECTION("UTF-8 4-byte sequence") {
        const char input[] = "\xf0\x9f\x98\x81";
        wchar_t output[10];
        const char *from_next = nullptr;
        wchar_t *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 4, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 4);

#ifdef RAINY_USING_WINDOWS
        REQUIRE(to_next - output == 2);
        REQUIRE(output[0] == 0xD83D);
        REQUIRE(output[1] == 0xDE01);
        unsigned long decoded = 0x10000 + ((output[0] - 0xD800) << 10) + (output[1] - 0xDC00);
        REQUIRE(decoded == 0x1F601);
#else
        REQUIRE(to_next - output == 1);
        REQUIRE(output[0] == 0x1f601);
#endif
    }
}

TEST_CASE("codecvt_utf8 to_wide with BOM", "[codecvt_utf8]") {
    using codecvt = codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::consume_header>;

    SECTION("Consume UTF-8 BOM") {
        const char input[] = "\xef\xbb\xbfHello";
        wchar_t output[20];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 8, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 8);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }

    SECTION("No BOM to consume") {
        const char* input = "Hello";
        wchar_t output[20];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 5, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }
}

TEST_CASE("codecvt_utf8 to_wide error cases", "[codecvt_utf8]") {
    using codecvt = codecvt_utf8<wchar_t>;

    SECTION("Invalid continuation byte") {
        const char input[] = "\xc3\xff";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 2, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::error);
    }

    SECTION("Incomplete sequence") {
        const char input[] = "\xc3";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 1, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::partial);
    }

    SECTION("Invalid starting byte") {
        const char input[] = "\xff";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 1, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::partial);
    }
}

TEST_CASE("codecvt_utf8 to_bytes basic conversions", "[codecvt_utf8]") {
    using codecvt = codecvt_utf8<wchar_t>;

    SECTION("ASCII characters") {
        const wchar_t input[] = L"Hello";
        char output[20];
        const wchar_t *from_next = nullptr;
        char *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 5, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::string(output, 5) == "Hello");
    }

    SECTION("2-byte UTF-8") {
        const wchar_t input[] = {0xe9, 0};
        char output[10];
        const wchar_t *from_next = nullptr;
        char *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 1, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 1);
        REQUIRE(to_next - output == 2);
        REQUIRE(std::string(output, 2) == "\xc3\xa9");
    }

    SECTION("3-byte UTF-8") {
        const wchar_t input[] = {0x20ac, 0};
        char output[10];
        const wchar_t *from_next = nullptr;
        char *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 1, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 1);
        REQUIRE(to_next - output == 3);
        REQUIRE(std::string(output, 3) == "\xe2\x82\xac");
    }

    SECTION("4-byte UTF-8") {
#ifdef RAINY_USING_WINDOWS
        const wchar_t input[] = {0xD83D, 0xDE01};
        char output[10];
        const wchar_t *from_next = nullptr;
        char *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 2, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 2);
        REQUIRE(to_next - output == 4);
        REQUIRE(std::string(output, 4) == "\xf0\x9f\x98\x81");
#else
        const wchar_t input[] = {0x1f601, 0};
        char output[10];
        const wchar_t *from_next = nullptr;
        char *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 1, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 1);
        REQUIRE(to_next - output == 4);
        REQUIRE(std::string(output, 4) == "\xf0\x9f\x98\x81");
#endif
    }
}

TEST_CASE("codecvt_utf8 to_bytes with BOM", "[codecvt_utf8]") {
    using codecvt = codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::generate_header>;

    SECTION("Generate UTF-8 BOM") {
        const wchar_t input[] = L"Hello";
        char output[20];
        const wchar_t* from_next = nullptr;
        char* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 5, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 8);
        REQUIRE(std::string(output, 3) == "\xef\xbb\xbf");
        REQUIRE(std::string(output + 3, 5) == "Hello");
    }
}

TEST_CASE("codecvt_utf16 to_wide basic conversions", "[codecvt_utf16]") {
    using codecvt = codecvt_utf16<wchar_t, 0x10ffff, little_endian>;

    SECTION("ASCII characters") {
        const char input[] = {'H', '\0', 'e', '\0', 'l', '\0', 'l', '\0', 'o', '\0'};
        wchar_t output[10];
        const char *from_next = nullptr;
        wchar_t *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 10, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 10);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }

    SECTION("Surrogate pair") {
        unsigned char input_uc[] = {0x3d, 0xd8, 0x01, 0xde};
        const char *input = reinterpret_cast<const char *>(input_uc);
        wchar_t output[10];
        const char *from_next = nullptr;
        wchar_t *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 4, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 4);

#ifdef RAINY_USING_WINDOWS
        REQUIRE(to_next - output == 2);
        REQUIRE(output[0] == 0xD83D);
        REQUIRE(output[1] == 0xDE01); 
        unsigned long decoded = 0x10000 + ((output[0] - 0xD800) << 10) + (output[1] - 0xDC00);
        REQUIRE(decoded == 0x1F601);
#else
        REQUIRE(to_next - output == 1);
        REQUIRE(output[0] == 0x1f601);
#endif
    }
}

TEST_CASE("codecvt_utf16 to_wide with BOM", "[codecvt_utf16]") {
    using codecvt = codecvt_utf16<wchar_t, 0x10ffff, codecvt_mode::consume_header>;

    SECTION("Consume UTF-16 LE BOM") {
        unsigned char input_uc[] = {0xff, 0xfe, 'H', 0, 'e', 0, 'l', 0, 'l', 0, 'o', 0};
        const char* input = reinterpret_cast<const char*>(input_uc);
        wchar_t output[20];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 12, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 12);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }

    SECTION("Consume UTF-16 BE BOM") {
        unsigned char input_uc[] = {0xfe, 0xff, 0, 'H', 0, 'e', 0, 'l', 0, 'l', 0, 'o'};
        const char* input = reinterpret_cast<const char*>(input_uc);
        wchar_t output[20];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 12, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 12);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }
}

TEST_CASE("codecvt_utf16 to_bytes basic conversions", "[codecvt_utf16]") {
    using codecvt = codecvt_utf16<wchar_t, 0x10ffff, little_endian>;

    SECTION("ASCII characters") {
        const wchar_t input[] = L"Hello";
        unsigned char output[20];
        const wchar_t *from_next = nullptr;
        unsigned char *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 5, from_next, reinterpret_cast<char *>(output),
                                        reinterpret_cast<char *>(output + 20), reinterpret_cast<char *&>(to_next), seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 10);
        REQUIRE(output[0] == 'H');
        REQUIRE(output[1] == 0);
        REQUIRE(output[2] == 'e');
    }

    SECTION("Surrogate pair to_wide") {
        unsigned char input_uc[] = {0x3d, 0xd8, 0x01, 0xde};
        const char *input = reinterpret_cast<const char *>(input_uc);
        wchar_t output[10];
        const char *from_next = nullptr;
        wchar_t *to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 4, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 4);

#ifdef RAINY_USING_WINDOWS
        REQUIRE(to_next - output == 2);
        REQUIRE(output[0] == 0xD83D);
        REQUIRE(output[1] == 0xDE01);
#else
        REQUIRE(to_next - output == 1);
        REQUIRE(output[0] == 0x1f601);
#endif
    }
}

TEST_CASE("codecvt_utf16 to_bytes with BOM", "[codecvt_utf16]") {
    using codecvt = codecvt_utf16<wchar_t, 0x10ffff, codecvt_mode::generate_header | codecvt_mode::little_endian>;

    SECTION("Generate UTF-16 LE BOM") {
        const wchar_t input[] = L"He";
        unsigned char output[20];
        const wchar_t* from_next = nullptr;
        unsigned char* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 2, from_next,
                                       reinterpret_cast<char*>(output),
                                       reinterpret_cast<char*>(output + 20),
                                       reinterpret_cast<char*&>(to_next), seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 2);
        REQUIRE(to_next - output == 6);
        REQUIRE(output[0] == 0xff);
        REQUIRE(output[1] == 0xfe);
        REQUIRE(output[2] == 'H');
        REQUIRE(output[4] == 'e');
    }
}

TEST_CASE("codecvt_utf8_utf16 to_wide conversions", "[codecvt_utf8_utf16]") {
    using codecvt = codecvt_utf8_utf16<wchar_t>;

    SECTION("ASCII to UTF-16") {
        const char* input = "Hello";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 5, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::wstring(output, 5) == L"Hello");
    }

    SECTION("UTF-8 3-byte to UTF-16") {
        const char input[] = "\xe2\x82\xac";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 3, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 3);
        REQUIRE(to_next - output == 1);
        REQUIRE(output[0] == 0x20ac);
    }

    SECTION("UTF-8 4-byte to UTF-16 surrogate pair") {
        const char input[] = "\xf0\x9f\x98\x81";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 4, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 4);
        REQUIRE(to_next - output == 2);
        bool is_high_surrogate = (output[0] >= 0xd800) && (output[0] < 0xdc00);
        bool is_low_surrogate = (output[1] >= 0xdc00) && (output[1] < 0xe000);
        REQUIRE(is_high_surrogate);
        REQUIRE(is_low_surrogate);
    }
}

TEST_CASE("codecvt_utf8_utf16 to_bytes conversions", "[codecvt_utf8_utf16]") {
    using codecvt = codecvt_utf8_utf16<wchar_t>;

    SECTION("UTF-16 to UTF-8 ASCII") {
        const wchar_t input[] = L"Hello";
        char output[20];
        const wchar_t* from_next = nullptr;
        char* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 5, from_next, output, output + 20, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 5);
        REQUIRE(to_next - output == 5);
        REQUIRE(std::string(output, 5) == "Hello");
    }

    SECTION("UTF-16 surrogate pair to UTF-8") {
        wchar_t input[3] = {0xd83d, 0xde01, 0};
        char output[10];
        const wchar_t* from_next = nullptr;
        char* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 2, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::ok);
        REQUIRE(from_next == input + 2);
        REQUIRE(to_next - output == 4);
        REQUIRE(std::string(output, 4) == "\xf0\x9f\x98\x81");
    }
}

TEST_CASE("wstring_convert from_bytes", "[wstring_convert]") {
    using convert = wstring_convert<codecvt_utf8<wchar_t>, std::basic_string>;

    SECTION("Convert from ASCII") {
        convert conv;
        auto result = conv.from_bytes("Hello");

        REQUIRE(result == L"Hello");
        REQUIRE(conv.converted() == 5);
    }

    SECTION("Convert from UTF-8") {
        convert conv;
        std::string input = "\xc3\xa9";
        auto result = conv.from_bytes(input);

        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 0xe9);
        REQUIRE(conv.converted() == 1);
    }

    SECTION("Convert single char") {
        convert conv;
        auto result = conv.from_bytes('A');

        REQUIRE(result == L"A");
        REQUIRE(conv.converted() == 1);
    }

    SECTION("Convert with BOM") {
        using convert_bom = wstring_convert<codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::consume_header>, std::basic_string>;
        convert_bom conv;
        std::string input = "\xef\xbb\xbfHello";
        auto result = conv.from_bytes(input);

        REQUIRE(result == L"Hello");
        REQUIRE(conv.converted() == 5);
    }

    SECTION("Null pointer handling") {
        convert conv;
        auto result = conv.from_bytes(nullptr);

        REQUIRE(result == L"");
    }

    SECTION("Empty string") {
        convert conv;
        auto result = conv.from_bytes("");

        REQUIRE(result == L"");
        REQUIRE(conv.converted() == 0);
    }
}

TEST_CASE("wstring_convert to_bytes", "[wstring_convert]") {
    using convert = wstring_convert<codecvt_utf8<wchar_t>, std::basic_string>;

    SECTION("Convert to ASCII") {
        convert conv;
        auto result = conv.to_bytes(L"Hello");

        REQUIRE(result == "Hello");
        REQUIRE(conv.converted() == 5);
    }

    SECTION("Convert to UTF-8") {
        convert conv;
        std::wstring input = {0xe9};
        auto result = conv.to_bytes(input);

        REQUIRE(result == "\xc3\xa9");
        REQUIRE(conv.converted() == 2);
    }

    SECTION("Convert single wchar") {
        convert conv;
        auto result = conv.to_bytes(L'A');

        REQUIRE(result == "A");
        REQUIRE(conv.converted() == 1);
    }

    SECTION("Convert with BOM generation") {
        using convert_bom = wstring_convert<codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::generate_header>, std::basic_string>;
        convert_bom conv;
        auto result = conv.to_bytes(L"Hello");

        REQUIRE(result.substr(0, 3) == "\xef\xbb\xbf");
        REQUIRE(result.substr(3) == "Hello");
        REQUIRE(conv.converted() == 8);
    }

    SECTION("Null pointer handling") {
        convert conv;
        auto result = conv.to_bytes(nullptr);

        REQUIRE(result == "");
    }

    SECTION("Empty string") {
        convert conv;
        auto result = conv.to_bytes(L"");

        REQUIRE(result == "");
        REQUIRE(conv.converted() == 0);
    }
}

TEST_CASE("wstring_convert error handling", "[wstring_convert]") {
    SECTION("Invalid UTF-8 input") {
        using convert = wstring_convert<codecvt_utf8<wchar_t>, std::basic_string>;
        convert conv;
        std::string invalid = "\xc3\xff";
        auto result = conv.from_bytes(invalid);

        REQUIRE(result == L"");
    }

    SECTION("Custom error string") {
        using convert = wstring_convert<codecvt_utf8<wchar_t>, std::basic_string>;
        convert conv("ERROR", L"WERROR");
        std::string invalid = "\xc3\xff";
        auto result = conv.from_bytes(invalid);

        REQUIRE(result == L"WERROR");
    }
}

TEST_CASE("wstring_convert with char16_t", "[wstring_convert]") {
    using convert = wstring_convert<codecvt_utf8<char16_t>, std::basic_string, char16_t>;
    using convertu16 = wstring_convert<codecvt_utf8_utf16<char16_t>, std::basic_string, char16_t>;

    SECTION("from_bytes") {
        convert conv;
        auto result = conv.from_bytes("Hello");
        REQUIRE(result == u"Hello");
    }

    SECTION("to_bytes") {
        convert conv;
        auto result = conv.to_bytes(u"Hello");
        REQUIRE(result == "Hello");
    }

    SECTION("to_bytes with surrogate pair") {
        convertu16 conv;
        std::u16string input;
        input.push_back(static_cast<char16_t>(0xd83d));
        input.push_back(static_cast<char16_t>(0xde01));
        auto result = conv.to_bytes(input);
        REQUIRE(result == "\xf0\x9f\x98\x81");
    }
}

TEST_CASE("wstring_convert with char32_t", "[wstring_convert]") {
    using convert = wstring_convert<codecvt_utf8<char32_t>, std::basic_string, char32_t>;

    SECTION("from_bytes") {
        convert conv;
        auto result = conv.from_bytes("Hello");
        REQUIRE(result == U"Hello");
    }

    SECTION("to_bytes") {
        convert conv;
        auto result = conv.to_bytes(U"Hello");
        REQUIRE(result == "Hello");
    }
}

TEST_CASE("codecvt_utf16 error cases", "[codecvt_utf16]") {
    using codecvt = codecvt_utf16<wchar_t, 0x10ffff, little_endian>;

    SECTION("Invalid surrogate pair") {
        unsigned char input_uc[] = {0x3d, 0xd8, 0x00, 0x00};
        const char* input = reinterpret_cast<const char*>(input_uc);
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 4, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::error);
    }

    SECTION("Incomplete input") {
        unsigned char input_uc[] = {0x3d, 0xd8};
        const char* input = reinterpret_cast<const char*>(input_uc);
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 2, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::partial);
    }

    SECTION("Insufficient output space") {
        const char input[] = {'H', '\0', 'e', '\0'};
        wchar_t output[1];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 4, from_next, output, output + 1, to_next, seen_header);

        REQUIRE(result == codecvt::result::partial);
    }
}

TEST_CASE("codecvt_utf8_utf16 error cases", "[codecvt_utf8_utf16]") {
    using codecvt = codecvt_utf8_utf16<wchar_t>;

    SECTION("Invalid UTF-8") {
        const char input[] = "\xc3\xff";
        wchar_t output[10];
        const char* from_next = nullptr;
        wchar_t* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_wide(input, input + 2, from_next, output, output + 10, to_next, seen_header);

        REQUIRE(result == codecvt::result::error);
    }

    SECTION("Incomplete surrogate pair") {
        wchar_t input[2] = {0xd83d, 0};
        unsigned char output[10];
        const wchar_t* from_next = nullptr;
        unsigned char* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 1, from_next,
                                       reinterpret_cast<char*>(output),
                                       reinterpret_cast<char*>(output + 10),
                                       reinterpret_cast<char*&>(to_next), seen_header);

        REQUIRE(result == codecvt::result::partial);
    }

    SECTION("Invalid surrogate pair for to_bytes") {
        wchar_t input[2] = {0xd83d, 0x1234};
        unsigned char output[10];
        const wchar_t* from_next = nullptr;
        unsigned char* to_next = nullptr;
        bool seen_header = false;

        auto result = codecvt::to_bytes(input, input + 2, from_next,
                                       reinterpret_cast<char*>(output),
                                       reinterpret_cast<char*>(output + 10),
                                       reinterpret_cast<char*&>(to_next), seen_header);

        REQUIRE(result == codecvt::result::error);
    }
}

TEST_CASE("wstring_convert with BOM handling", "[wstring_convert]") {
    SECTION("Consume BOM from UTF-8") {
        using convert = wstring_convert<codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::consume_header>, std::basic_string>;
        convert conv;
        std::string input = "\xef\xbb\xbfHello";
        auto result = conv.from_bytes(input);
        REQUIRE(result == L"Hello");
    }

    SECTION("Generate BOM for UTF-8") {
        using convert = wstring_convert<codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::generate_header>, std::basic_string>;
        convert conv;
        auto result = conv.to_bytes(L"Hello");
        REQUIRE(result.substr(0, 3) == "\xef\xbb\xbf");
    }

    SECTION("Consume BOM from UTF-16 LE") {
        using convert = wstring_convert<codecvt_utf16<wchar_t, 0x10ffff, codecvt_mode::consume_header>, std::basic_string>;
        convert conv;
        std::string input;
        input.push_back(static_cast<char>(0xff));
        input.push_back(static_cast<char>(0xfe));
        input.push_back('H');
        input.push_back(0);
        input.push_back('e');
        input.push_back(0);
        auto result = conv.from_bytes(input);
        REQUIRE(result == L"He");
    }

    SECTION("Generate BOM for UTF-16 LE") {
        using convert = wstring_convert<
            codecvt_utf16<wchar_t, 0x10ffff, codecvt_mode::generate_header | codecvt_mode::little_endian>,
            std::basic_string
        >;
        convert conv;
        auto result = conv.to_bytes(L"He");
        REQUIRE(static_cast<unsigned char>(result[0]) == 0xff);
        REQUIRE(static_cast<unsigned char>(result[1]) == 0xfe);
    }
}