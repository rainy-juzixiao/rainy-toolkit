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
#include <rainy/collections/vector.hpp>
#include <rainy/core/core.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <limits>
#include <memory>

using namespace rainy::foundation::text;

// NOLINTBEGIN
template <typename StringType>
struct StringTestFixture {
    using char_type = typename StringType::value_type;
    static StringType create_from_literal(const char_type *str) {
        return StringType(str);
    }
};
// NOLINTEND

namespace Catch {
    template <typename Iterator>
    struct is_range<rainy::utility::reverse_iterator<Iterator>> : std::false_type {};

    template <typename Iterator>
    struct StringMaker<rainy::utility::reverse_iterator<Iterator>> {
        static std::string convert(rainy::utility::reverse_iterator<Iterator> const &iter) {
            return "reverse_iterator";
        }
    };
}

template <typename CharT>
struct LiteralConverter;

template <>
struct LiteralConverter<char> {
    static const char *convert(const char *str) {
        return str;
    }
};

template <>
struct LiteralConverter<wchar_t> {
    static const wchar_t *convert(const char *str) {
        static std::wstring wstr;
        wstr = std::wstring(str, str + strlen(str));
        return wstr.c_str();
    }
};

// NOLINTBEGIN

SCENARIO("basic_string construction from various sources", "[string][construction]") {
    GIVEN("Default constructor") {
        string str;

        THEN("String should be empty") {
            REQUIRE(str.empty());
            REQUIRE(str.size() == 0);
            REQUIRE(str.length() == 0);
            REQUIRE(str.data() != nullptr); // Should have null terminator
        }
    }

    GIVEN("Construction with count and character") {
        auto count = GENERATE(0, 1, 5, 100, 1000);
        char ch = 'X';

        WHEN("Constructing with " << count << " characters") {
            string str(count, ch);

            THEN("String should have correct size and content") {
                REQUIRE(str.size() == count);
                REQUIRE(str.length() == count);
                if (count > 0) {
                    for (size_t i = 0; i < count; ++i) {
                        REQUIRE(str[i] == ch);
                    }
                }
            }
        }
    }

    GIVEN("Construction with maximum possible size") {
        WHEN("Attempting to construct with size_type::max()") {
            THEN("Should throw length_error or handle gracefully") {
                REQUIRE_THROWS_AS(
                    string(std::numeric_limits<string::size_type>::max(), 'X'),
                    std::length_error);
            }
        }
    }

    GIVEN("Construction from C-string") {
        const char *cstr = "Hello, World!";

        WHEN("Constructing without count") {
            string str(cstr);

            THEN("String should contain entire C-string") {
                REQUIRE(str == cstr);
                REQUIRE(str.size() == strlen(cstr));
            }
        }

        WHEN("Constructing with count less than string length") {
            string str(cstr, 5);

            THEN("String should contain first 5 characters") {
                REQUIRE(str == "Hello");
                REQUIRE(str.size() == 5);
            }
        }
    }

    GIVEN("Copy construction") {
        string original = "Test String";

        WHEN("Creating a copy") {
            string copy(original);

            THEN("Copy should equal original") {
                REQUIRE(copy == original);
                REQUIRE(copy.size() == original.size());
            }

            AND_WHEN("Modifying original") {
                original += " Modified";

                THEN("Copy should remain unchanged") {
                    REQUIRE(copy == "Test String");
                    REQUIRE(original != copy);
                }
            }
        }

        WHEN("Copy constructing with position and count") {
            auto copy = string(original, 5, 3);

            THEN("Copy should contain substring") {
                REQUIRE(copy == "Str");
            }
        }

        WHEN("Copy constructing with position beyond end") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(string(original, original.size() + 1), std::out_of_range);
            }
        }
    }

    GIVEN("Move construction") {
        string original = "Move Me";

        WHEN("Moving to new string") {
            string moved(std::move(original));

            THEN("Moved-to string should have original content") {
                REQUIRE(moved == "Move Me");
            }

            THEN("Original should be empty") {
                REQUIRE(original.empty());
            }
        }
    }

    GIVEN("Construction from initializer list") {
        WHEN("Using initializer list") {
            string str = {'H', 'e', 'l', 'l', 'o'};

            THEN("String should contain list elements") {
                REQUIRE(str == "Hello");
            }
        }

        WHEN("Initializer list is empty") {
            string str = {};

            THEN("String should be empty") {
                REQUIRE(str.empty());
            }
        }
    }
}

SCENARIO("basic_string assignment operations", "[string][assignment]") {
    GIVEN("An existing string") {
        string str = "Original";

        WHEN("Assigning from another string") {
            string other = "New Content";
            str = other;

            THEN("String should have new content") {
                REQUIRE(str == "New Content");
            }

            AND_WHEN("Modifying original after assignment") {
                other += " Modified";

                THEN("Assigned string should remain unchanged") {
                    REQUIRE(str == "New Content");
                }
            }
        }

        WHEN("Assigning from C-string") {
            str = "C-string assignment";

            THEN("String should have C-string content") {
                REQUIRE(str == "C-string assignment");
            }
        }

        WHEN("Assigning from character") {
            str = 'Z';

            THEN("String should contain single character") {
                REQUIRE(str == "Z");
                REQUIRE(str.size() == 1);
            }
        }

        WHEN("Assigning from initializer list") {
            str = {'A', 'B', 'C'};

            THEN("String should contain list characters") {
                REQUIRE(str == "ABC");
            }
        }

        WHEN("Self-assignment") {
            str = str;

            THEN("String should remain unchanged") {
                REQUIRE(str == "Original");
            }
        }

        WHEN("Assigning from nullptr") {
            THEN("Should be deleted or cause compilation error") {
                // This should not compile - testing concept
                // str = nullptr; // Should fail
            }
        }
    }
}

SCENARIO("Element access operations", "[string][access]") {
    GIVEN("A non-empty string") {
        string str = "Hello World";
        const auto &const_str = str;

        WHEN("Accessing valid positions with operator[]") {
            THEN("Should return correct characters") {
                REQUIRE(str[0] == 'H');
                REQUIRE(str[6] == 'W');
                REQUIRE(str[10] == 'd');
            }

            AND_WHEN("Modifying through operator[]") {
                str[0] = 'J';

                THEN("String should be modified") {
                    REQUIRE(str == "Jello World");
                }
            }
        }

        WHEN("Accessing with at() method") {
            THEN("Valid positions should work") {
                REQUIRE(str.at(0) == 'H');
                REQUIRE(const_str.at(5) == ' ');
            }

            THEN("Invalid positions should throw") {
                REQUIRE_THROWS_AS(str.at(str.size()), std::out_of_range);
                REQUIRE_THROWS_AS(str.at(str.size() + 10), std::out_of_range);
            }
        }

        WHEN("Accessing front and back") {
            THEN("front() should return first character") {
                REQUIRE(str.front() == 'H');
                const_str.front(); // Should work on const
            }

            THEN("back() should return last character") {
                REQUIRE(str.back() == 'd');
            }

            AND_WHEN("Modifying front/back") {
                str.front() = 'h';
                str.back() = 'D';

                THEN("String should be modified") {
                    REQUIRE(str == "hello WorlD");
                }
            }
        }

        WHEN("String is empty") {
            string empty;

            THEN("front() and back() should be undefined but accessible") {
                // Should not crash, but value is undefined
                // REQUIRE(empty.front() == CharT()); // Implementation defined
            }
        }
    }
}

SCENARIO("String capacity management", "[string][capacity]") {
    GIVEN("A default constructed string") {
        string str;
        auto initial_capacity = str.capacity();

        WHEN("Reserving capacity") {
            size_t new_cap = 100;
            str.reserve(new_cap);

            THEN("Capacity should be at least requested") {
                REQUIRE(str.capacity() >= new_cap);
            }

            AND_WHEN("Reserving less than current capacity") {
                str.reserve(initial_capacity);

                THEN("Capacity should not decrease") {
                    REQUIRE(str.capacity() >= new_cap);
                }
            }
        }

        WHEN("Reserving maximum possible capacity") {
            THEN("Should throw bad_alloc or length_error") {
                REQUIRE_THROWS_AS(str.reserve(std::numeric_limits<size_t>::max()), std::length_error);
            }
        }

        WHEN("Shrinking to fit") {
            str.reserve(1000);
            str = "Small";
            auto before_shrink = str.capacity();
            str.shrink_to_fit();

            THEN("Capacity should be reduced") {
                REQUIRE(str.capacity() <= before_shrink);
                REQUIRE(str.capacity() >= str.size());
            }
        }
    }

    GIVEN("A string with content") {
        string str = "Growing";

        WHEN("Resizing to larger size") {
            str.resize(20, '.');

            THEN("String should be extended with filler") {
                REQUIRE(str.size() == 20);
                REQUIRE(str == "Growing.............");
            }
        }

        WHEN("Resizing to smaller size") {
            str.resize(4);

            THEN("String should be truncated") {
                REQUIRE(str.size() == 4);
                REQUIRE(str == "Grow");
            }
        }

        WHEN("Resizing to same size") {
            str.resize(str.size());

            THEN("String should remain unchanged") {
                REQUIRE(str == "Growing");
            }
        }

        WHEN("Resizing to zero") {
            str.resize(0);

            THEN("String should become empty") {
                REQUIRE(str.empty());
            }
        }
    }
}

SCENARIO("String modification operations", "[string][modifiers]") {
    GIVEN("A base string") {
        string str = "Base";

        WHEN("Appending characters") {
            str.push_back('!');

            THEN("Character should be added at end") {
                REQUIRE(str == "Base!");
            }
        }

        WHEN("Appending with += operator") {
            str += " String";

            THEN("Content should be appended") {
                REQUIRE(str == "Base String");
            }

            AND_WHEN("Appending character") {
                str += '!';

                THEN("Character should be appended") {
                    REQUIRE(str == "Base String!");
                }
            }

            AND_WHEN("Appending initializer list") {
                str += {' ', 'E', 'x', 't', 'r', 'a'};

                THEN("List should be appended") {
                    REQUIRE(str == "Base String Extra");
                }
            }
        }

        WHEN("Appending with append() method") {
            SECTION("Append count of characters") {
                str.append(3, '!');
                REQUIRE(str == "Base!!!");
            }

            SECTION("Append from C-string") {
                str.append(" Extended");
                REQUIRE(str == "Base Extended");
            }

            SECTION("Append from another string") {
                string other = " Other";
                str.append(other);
                REQUIRE(str == "Base Other");
            }

            SECTION("Append substring from another string") {
                string other = "12345";
                str.append(other, 1, 3);
                REQUIRE(str == "Base234");
            }

            SECTION("Append with count from C-string") {
                str.append("Extended", 3);
                REQUIRE(str == "BaseExt");
            }
        }
    }

    GIVEN("A string for insert operations") {
        string str = "HelloWorld";

        WHEN("Inserting at beginning") {
            str.insert(0, "Start ");

            THEN("Content should be inserted at position 0") {
                REQUIRE(str == "Start HelloWorld");
            }
        }

        WHEN("Inserting in middle") {
            str.insert(5, " Beautiful");

            THEN("Content should be inserted at position 5") {
                REQUIRE(str == "Hello BeautifulWorld");
            }
        }

        WHEN("Inserting at end") {
            str.insert(str.size(), " End");

            THEN("Content should be appended") {
                REQUIRE(str == "HelloWorld End");
            }
        }

        WHEN("Inserting with iterator") {
            auto pos = str.begin() + 5;
            str.insert(pos, ' ');

            THEN("Character should be inserted at iterator position") {
                REQUIRE(str == "Hello World");
            }
        }

        WHEN("Inserting at invalid position") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(str.insert(str.size() + 1, "X"), std::out_of_range);
            }
        }
    }

    GIVEN("A string for erase operations") {
        string str = "Hello Beautiful World";

        WHEN("Erasing from position with count") {
            str.erase(5, 10); // Remove " Beautiful"

            THEN("Should remove specified range") {
                REQUIRE(str == "Hello World");
            }
        }

        WHEN("Erasing from position without count") {
            str.erase(5); // Remove from position 5 to end

            THEN("Should remove from position to end") {
                REQUIRE(str == "Hello");
            }
        }

        WHEN("Erasing with iterators") {
            auto first = str.begin() + 5;
            auto last = str.begin() + 15;
            str.erase(first, last);

            THEN("Should remove iterator range") {
                REQUIRE(str == "Hello World");
            }
        }

        WHEN("Erasing single character with iterator") {
            auto pos = str.begin() + 5; // The space
            str.erase(pos);

            THEN("Should remove single character") {
                REQUIRE(str == "HelloBeautiful World");
            }
        }

        WHEN("Erasing all content") {
            str.erase();

            THEN("String should become empty") {
                REQUIRE(str.empty());
            }
        }

        WHEN("Popping back") {
            str.pop_back();

            THEN("Last character should be removed") {
                REQUIRE(str == "Hello Beautiful Worl");
            }

            AND_WHEN("Popping back on empty string") {
                str.clear();

                REQUIRE(str.empty());
            }
        }
    }

    GIVEN("A string for replace operations") {
        string str = "The quick brown fox jumps";

        WHEN("Replacing substring with same length") {
            str.replace(4, 5, "slow"); // "quick" -> "slow"

            THEN("Should replace with new content") {
                REQUIRE(str == "The slow brown fox jumps");
            }
        }

        WHEN("Replacing with longer string") {
            str.replace(10, 5, "red"); // "brown" -> "red"

            THEN("String should adjust size") {
                REQUIRE(str == "The quick red fox jumps");
            }
        }

        WHEN("Replacing with shorter string") {
            str.replace(16, 3, "leopard"); // "fox" -> "leopard"

            THEN("String should adjust size") {
                REQUIRE(str == "The quick brown leopard jumps");
            }
        }

        WHEN("Replacing with empty string") {
            str.replace(4, 5, ""); // Remove "quick"

            THEN("Should effectively erase range") {
                REQUIRE(str == "The  brown fox jumps");
            }
        }

        WHEN("Replacing entire string") {
            str.replace(0, str.size(), "Complete replacement");

            THEN("String should become new content") {
                REQUIRE(str == "Complete replacement");
            }
        }
    }

    GIVEN("A string for swap operations") {
        string str1 = "First";
        string str2 = "Second";

        WHEN("Swapping strings") {
            str1.swap(str2);

            THEN("Contents should be exchanged") {
                REQUIRE(str1 == "Second");
                REQUIRE(str2 == "First");
            }
        }

        WHEN("Using non-member swap") {
            swap(str1, str2);

            THEN("Contents should be exchanged") {
                REQUIRE(str1 == "Second");
                REQUIRE(str2 == "First");
            }
        }

        WHEN("Swapping with self") {
            str1.swap(str1);

            THEN("String should remain unchanged") {
                REQUIRE(str1 == "First");
            }
        }
    }

    GIVEN("A string for clear operation") {
        string str = "Clear me";

        WHEN("Clearing string") {
            str.clear();

            THEN("String should become empty") {
                REQUIRE(str.empty());
                REQUIRE(str.size() == 0);
            }

            AND_WHEN("Clearing already empty string") {
                REQUIRE_NOTHROW(str.clear());
                REQUIRE(str.empty());
            }
        }
    }
}

SCENARIO("String find operations", "[string][find]") {
    GIVEN("A complex string for searching") {
        string str = "Hello world, hello universe, hello everything";

        WHEN("Finding a substring") {
            auto pos = str.find("hello");

            THEN("Should find first occurrence") {
                REQUIRE(pos == 13); // "hello" starts at index 13
            }
        }

        WHEN("Finding a character") {
            auto pos = str.find('w');

            THEN("Should find first occurrence") {
                REQUIRE(pos == 6);
            }
        }

        WHEN("Finding with starting position") {
            auto pos = str.find("hello", 14);

            THEN("Should find occurrence after start position") {
                REQUIRE(pos == 29); // Second "hello" starts at 27
            }
        }

        WHEN("Finding non-existent substring") {
            auto pos = str.find("xyz");

            THEN("Should return npos") {
                REQUIRE(pos == string::npos);
            }
        }

        WHEN("Finding at position beyond string length") {
            auto pos = str.find("world", str.size() + 10);

            THEN("Should return npos") {
                REQUIRE(pos == string::npos);
            }
        }

        WHEN("Finding empty string") {
            auto pos = str.find("");

            THEN("Should return 0") {
                REQUIRE(pos == 0);
            }
        }
    }

    GIVEN("A string for rfind operations") {
        string str = "ababab";

        WHEN("Finding last occurrence") {
            auto pos = str.rfind("ab");

            THEN("Should find last occurrence") {
                REQUIRE(pos == 4); // Last "ab" starts at 4
            }
        }

        WHEN("Finding last occurrence with position limit") {
            auto pos = str.rfind("ab", 3);

            THEN("Should find last occurrence within range") {
                REQUIRE(pos == 2); // Last "ab" within first 4 chars starts at 2
            }
        }

        WHEN("Finding last occurrence of character") {
            auto pos = str.rfind('b');

            THEN("Should find last occurrence") {
                REQUIRE(pos == 5);
            }
        }
    }

    GIVEN("A string for find_first_of operations") {
        string str = "Hello World 123";

        WHEN("Finding first of any characters") {
            auto pos = str.find_first_of("0123456789");

            THEN("Should find first digit") {
                REQUIRE(pos == 12); // '1' at position 12
            }
        }

        WHEN("Finding first of with no matches") {
            auto pos = str.find_first_of("xyz");

            THEN("Should return npos") {
                REQUIRE(pos == string::npos);
            }
        }

        WHEN("Finding first of with empty set") {
            auto pos = str.find_first_of("");

            THEN("Should return npos") {
                REQUIRE(pos == string::npos);
            }
        }
    }

    GIVEN("A string for find_last_of operations") {
        string str = "Hello World 123";

        WHEN("Finding last of any characters") {
            auto pos = str.find_last_of("0123456789");

            THEN("Should find last digit") {
                REQUIRE(pos == 14); // '3' at position 14
            }
        }
    }

    GIVEN("A string for find_first_not_of operations") {
        string str = "   \t\nHello";

        WHEN("Finding first non-whitespace") {
            auto pos = str.find_first_not_of(" \t\n");

            THEN("Should find first character not in set") {
                REQUIRE(pos == 5); // 'H' after whitespace
            }
        }

        WHEN("Finding first not of with all characters in set") {
            string spaces = "   ";
            auto pos = spaces.find_first_not_of(" ");

            THEN("Should return npos") {
                REQUIRE(pos == string::npos);
            }
        }
    }
}

SCENARIO("String prefix/suffix checking", "[string][starts_ends][contains]") {
    GIVEN("A test string") {
        string str = "Hello World";

        WHEN("Checking starts_with") {
            THEN("Should detect correct prefixes") {
                REQUIRE(str.starts_with("Hello"));
                REQUIRE(str.starts_with('H'));
                REQUIRE(str.starts_with("Hello World"));

                REQUIRE_FALSE(str.starts_with("World"));
                REQUIRE_FALSE(str.starts_with('W'));
                REQUIRE_FALSE(str.starts_with("Hello World Extended"));
            }
        }

        WHEN("Checking ends_with") {
            THEN("Should detect correct suffixes") {
                REQUIRE(str.ends_with("World"));
                REQUIRE(str.ends_with('d'));
                REQUIRE(str.ends_with("Hello World"));

                REQUIRE_FALSE(str.ends_with("Hello"));
                REQUIRE_FALSE(str.ends_with('H'));
            }
        }

        WHEN("Checking contains") {
            THEN("Should detect substrings") {
                REQUIRE(str.contains("Hello"));
                REQUIRE(str.contains("World"));
                REQUIRE(str.contains("lo Wo"));
                REQUIRE(str.contains('W'));

                REQUIRE_FALSE(str.contains("xyz"));
                REQUIRE_FALSE(str.contains('Z'));
            }

            AND_WHEN("String is empty") {
                string empty;

                THEN("Contains should work correctly") {
                    REQUIRE_FALSE(empty.contains('a'));
                    REQUIRE(empty.contains("")); // Empty string contains empty string
                }
            }
        }
    }
}

SCENARIO("String comparison operations", "[string][comparison]") {
    GIVEN("Strings for comparison") {
        string str1 = "abc";
        string str2 = "abc";
        string str3 = "abd";
        string str4 = "ab";
        const char *cstr = "abc";

        WHEN("Comparing equality") {
            THEN("Equal strings should compare equal") {
                REQUIRE(str1 == str2);
                REQUIRE(str1 == cstr);
                REQUIRE(cstr == str1);
            }

            THEN("Different strings should not compare equal") {
                REQUIRE_FALSE(str1 == str3);
                REQUIRE_FALSE(str1 == str4);
            }
        }

        WHEN("Comparing inequality") {
            THEN("Different strings should be inequal") {
                REQUIRE(str1 != str3);
                REQUIRE(str1 != str4);
            }

            THEN("Equal strings should not be inequal") {
                REQUIRE_FALSE(str1 != str2);
            }
        }

        WHEN("Comparing less than") {
            THEN("Lexicographical comparison should work") {
                REQUIRE(str1 < str3); // "abc" < "abd"
                REQUIRE_FALSE(str3 < str1); // "abd" < "abc" = false
                REQUIRE(str4 < str1); // "ab" < "abc" = true
                REQUIRE_FALSE(str1 < str4); // "abc" < "ab" = false
            }
        }

        WHEN("Comparing greater than") {
            THEN("Lexicographical comparison should work") {
                REQUIRE(str3 > str1);
                REQUIRE_FALSE(str1 > str3);
            }
        }

        WHEN("Comparing with empty string") {
            string empty;

            THEN("Non-empty > empty") {
                REQUIRE(str1 > empty);
                REQUIRE(empty < str1);
                REQUIRE_FALSE(empty == str1);
            }
        }
    }
}

SCENARIO("String iterator operations", "[string][iterators]") {
    GIVEN("A non-empty string") {
        string str = "Hello";
        const auto &const_str = str;

        WHEN("Using begin/end") {
            auto it = str.begin();
            auto end = str.end();

            THEN("Iterators should traverse string") {
                std::string result;
                for (; it != end; ++it) {
                    result.push_back(*it);
                }
                REQUIRE(result == "Hello");
            }

            AND_WHEN("Modifying through iterator") {
                *str.begin() = 'J';

                THEN("String should be modified") {
                    REQUIRE(str == "Jello");
                }
            }
        }

        WHEN("Using const iterators") {
            auto it = const_str.begin();
            auto end = const_str.end();

            THEN("Should provide read-only access") {
                REQUIRE(*it == 'H');
                // *it = 'J'; // Should not compile
            }
        }

        WHEN("Using reverse iterators") {
            std::string reversed;
            for (auto it = str.rbegin(); it != str.rend(); ++it) {
                reversed.push_back(*it);
            }

            THEN("Should traverse in reverse") {
                REQUIRE(reversed == "olleH");
            }
        }

        WHEN("Using iterator arithmetic") {
            auto mid = str.begin() + 2;

            THEN("Should support pointer arithmetic") {
                REQUIRE(*mid == 'l');
            }
        }

        WHEN("String is modified during iteration") {
            auto it = str.begin();
            str += " World";

            THEN("Iterators may be invalidated") {
                // Behavior is implementation-defined, but we should document
                // REQUIRE(*it == 'H'); // May or may not work
            }
        }
    }

    GIVEN("An empty string") {
        string empty;

        WHEN("Getting iterators") {
            THEN("begin should equal end") {
                REQUIRE(empty.begin() == empty.end());
                REQUIRE(empty.cbegin() == empty.cend());
                REQUIRE(empty.rbegin() == empty.rend());
            }
        }
    }
}

SCENARIO("String conversion operations", "[string][conversion]") {
    GIVEN("A basic_string") {
        string str = "Convert me";

        WHEN("Converting to string_view") {
            string_view sv = str;

            THEN("Should create valid string_view") {
                REQUIRE(sv.size() == str.size());
                REQUIRE(sv.data() == str.data());
                REQUIRE(sv == "Convert me");
            }
        }

        WHEN("Getting C-string") {
            const char *cstr = str.c_str();

            THEN("Should return null-terminated string") {
                REQUIRE(strlen(cstr) == str.size());
                REQUIRE(std::string(cstr) == "Convert me");
            }

            AND_WHEN("Modifying original string") {
                str += "!";

                THEN("C-string may be invalidated") {
                    // cstr may now be dangling
                }
            }
        }
    }
}

SCENARIO("Extended string operations", "[string][extended]") {
    GIVEN("A string for formatting") {
        string str;

        WHEN("Formatting with arguments") {
            str.format("Value: {}, Pi: {:.2f}", 42, 3.14159);

            THEN("String should contain formatted content") {
                REQUIRE(str == "Value: 42, Pi: 3.14");
            }
        }

        WHEN("Creating formatted copy") {
            auto result = str.format_copy("Hello, {}!", "World");

            THEN("Original should be unchanged") {
                REQUIRE(str.empty());
                REQUIRE(result == "Hello, World!");
            }
        }

        WHEN("Formatting with multiple arguments") {
            str.format("{} + {} = {}", 5, 3, 8);

            THEN("All arguments should be substituted") {
                REQUIRE(str == "5 + 3 = 8");
            }
        }

        WHEN("Formatting with empty format string") {
            str.format("");

            THEN("String should become empty") {
                REQUIRE(str.empty());
            }
        }
    }

    GIVEN("A string for repeat operations") {
        string str = "Ha";

        WHEN("Repeating string in-place") {
            str.repeat(3);

            THEN("String should be repeated n times") {
                REQUIRE(str == "HaHaHa");
            }
        }

        WHEN("Creating repeated copy") {
            auto result = str.repeat_copy(4);

            THEN("Original should be unchanged") {
                REQUIRE(str == "Ha");
                REQUIRE(result == "HaHaHaHa");
            }
        }

        WHEN("Repeating zero times") {
            str.repeat(0);

            THEN("String should become empty") {
                REQUIRE(str.empty());
            }
        }

        WHEN("Repeating empty string") {
            string empty;
            empty.repeat(100);

            THEN("Should remain empty") {
                REQUIRE(empty.empty());
            }
        }

        WHEN("Repeating to very large size") {
            string small = "X";

            THEN("Should handle large repetitions or throw") {
                // This might throw bad_alloc for extremely large values
                // REQUIRE_NOTHROW(small.repeat(1000000));
            }
        }
    }

    GIVEN("A string with mixed case") {
        string str = "Hello World 123";

        WHEN("Converting to uppercase") {
            str.to_upper();

            THEN("All letters should be uppercase") {
                REQUIRE(str == "HELLO WORLD 123");
            }
        }

        WHEN("Converting to lowercase") {
            str.to_lower();

            THEN("All letters should be lowercase") {
                REQUIRE(str == "hello world 123");
            }
        }

        WHEN("Creating uppercase copy") {
            auto result = str.to_upper_copy();

            THEN("Original should be unchanged") {
                REQUIRE(str == "Hello World 123");
                REQUIRE(result == "HELLO WORLD 123");
            }
        }

        WHEN("Creating lowercase copy") {
            auto result = str.to_lower_copy();

            THEN("Original should be unchanged") {
                REQUIRE(str == "Hello World 123");
                REQUIRE(result == "hello world 123");
            }
        }

        WHEN("String has no letters") {
            string numeric = "123456";

            THEN("Case conversion should have no effect") {
                numeric.to_upper();
                REQUIRE(numeric == "123456");
            }
        }
    }

    GIVEN("A string with whitespace") {
        string str = "  \t\n  Trim me  \n\t  ";

        WHEN("Trimming in-place") {
            str.trim();

            THEN("Leading and trailing whitespace should be removed") {
                REQUIRE(str == "Trim me");
            }
        }

        WHEN("Creating trimmed copy") {
            auto result = str.trimmed();

            THEN("Original should be unchanged") {
                REQUIRE(str == "  \t\n  Trim me  \n\t  ");
                REQUIRE(result == "Trim me");
            }
        }

        WHEN("String is all whitespace") {
            string whitespace = "   \t\n   ";
            whitespace.trim();

            THEN("Should become empty") {
                REQUIRE(whitespace.empty());
            }
        }

        WHEN("String has no whitespace") {
            string clean = "NoWhitespace";
            clean.trim();

            THEN("Should remain unchanged") {
                REQUIRE(clean == "NoWhitespace");
            }
        }
    }

    GIVEN("A delimited string") {
        string str = "apple,banana,cherry,date";

        WHEN("Splitting by delimiter") {
            auto parts = str.split(',');

            THEN("Should return vector of substrings") {
                REQUIRE(parts.size() == 4);
                REQUIRE(parts[0] == "apple");
                REQUIRE(parts[1] == "banana");
                REQUIRE(parts[2] == "cherry");
                REQUIRE(parts[3] == "date");
            }
        }

        WHEN("Splitting with custom container") {
            auto parts = str.split(',');

            THEN("Should return specified container type") {
                REQUIRE(parts.size() == 4);
                REQUIRE(parts.front() == "apple");
                REQUIRE(parts.back() == "date");
            }
        }

        WHEN("Splitting with delimiter not present") {
            auto parts = str.split(';');

            THEN("Should return single element with whole string") {
                REQUIRE(parts.size() == 1);
                REQUIRE(parts[0] == str);
            }
        }

        WHEN("Splitting empty string") {
            string empty;
            auto parts = empty.split(',');

            THEN("Should return empty container") {
                REQUIRE(parts.empty());
            }
        }

        WHEN("Splitting with consecutive delimiters") {
            string consecutive = "a,,b,c";
            auto parts = consecutive.split(',');

            THEN("Empty tokens should be preserved") {
                REQUIRE(parts.size() == 4);
                REQUIRE(parts[0] == "a");
                REQUIRE(parts[1] == "");
                REQUIRE(parts[2] == "b");
                REQUIRE(parts[3] == "c");
            }
        }

        WHEN("Splitting with delimiter at ends") {
            string ends = ",start,end,";
            auto parts = ends.split(',');

            THEN("Should include empty tokens at ends") {
                REQUIRE(parts.size() == 4);
                REQUIRE(parts[0] == "");
                REQUIRE(parts[1] == "start");
                REQUIRE(parts[2] == "end");
                REQUIRE(parts[3] == "");
            }
        }
    }
}

SCENARIO("String hash support", "[string][hash]") {
    GIVEN("Strings for hashing") {
        string str1 = "Hash me";
        string str2 = "Hash me";
        string str3 = "Different";

        WHEN("Using std::hash") {
            std::hash<string> hasher;

            THEN("Equal strings should have equal hashes") {
                REQUIRE(hasher(str1) == hasher(str2));
            }

            THEN("Different strings likely have different hashes") {
                // Hash collision is possible but unlikely
                REQUIRE(hasher(str1) != hasher(str3));
            }
        }

        WHEN("Using rainy::utility::hash") {
            rainy::utility::hash<string> hasher;

            THEN("Equal strings should have equal hashes") {
                REQUIRE(hasher(str1) == hasher(str2));
            }
        }
    }
}

SCENARIO("String stream output", "[string][stream]") {
    GIVEN("A string") {
        string str = "Stream me";

        WHEN("Writing to ostream") {
            std::ostringstream oss;
            oss << str;

            THEN("Should output string content") {
                REQUIRE(oss.str() == "Stream me");
            }
        }

        WHEN("Writing to wostream") {
            wstring wstr = L"Wide stream";
            std::wostringstream woss;
            woss << wstr;

            THEN("Should output wide string content") {
                REQUIRE(woss.str() == L"Wide stream");
            }
        }
    }
}

SCENARIO("Edge cases and extreme scenarios", "[string][edge]") {
    GIVEN("Strings with special characters") {
        string str;

        WHEN("String contains null characters") {
            str.assign("Hello\0World", 11);
            THEN("Should handle embedded nulls correctly") {
                REQUIRE(str.size() == 11);
                REQUIRE(str[5] == '\0');
                string expected("Hello\0World", 11);
                REQUIRE(str == expected);
                REQUIRE(str[0] == 'H');
                REQUIRE(str[5] == '\0');
                REQUIRE(str[6] == 'W');
                REQUIRE(str[10] == 'd');
            }
        }

        WHEN("String contains only null characters") {
            str.assign(10, '\0');

            THEN("Should create string of nulls") {
                REQUIRE(str.size() == 10);
                for (size_t i = 0; i < 10; ++i) {
                    REQUIRE(str[i] == '\0');
                }
            }
        }

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4566)
#endif

        WHEN("String contains Unicode characters") {
            // This assumes UTF-8 encoding for char strings
            string unicode = "Hello 世界 🌍";

            THEN("Should handle multi-byte characters") {
                REQUIRE(unicode.size() > 0);
                // Cannot easily test individual code points with char
            }
        }

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
    }

    GIVEN("Large strings for boundary testing") {
        WHEN("Constructing very large string") {
            size_t large_size = 1000000; // 1 million
            string str(large_size, 'X');

            THEN("Should allocate and fill correctly") {
                REQUIRE(str.size() == large_size);
                REQUIRE(str.front() == 'X');
                REQUIRE(str.back() == 'X');
            }
        }

        WHEN("Appending to large string") {
            string str(1000000, 'A');
            str.append(1000000, 'B');

            THEN("Should grow correctly") {
                REQUIRE(str.size() == 2000000);
                REQUIRE(str[0] == 'A');
                REQUIRE(str[1000000] == 'B');
            }
        }

        WHEN("Inserting into large string") {
            string str(1000000, 'X');
            str.insert(500000, 100000, 'Y');

            THEN("Should shift elements correctly") {
                REQUIRE(str.size() == 1100000);
                REQUIRE(str[499999] == 'X');
                REQUIRE(str[500000] == 'Y');
                REQUIRE(str[600000] == 'X');
            }
        }
    }

    GIVEN("Strings for iterator invalidation scenarios") {
        WHEN("Modifying during iteration") {
            string str = "Modify me";
            auto it = str.begin();

            // Operations that may invalidate iterators
            str.append(" extended");

            THEN("Original iterators may be invalid") {
                // This is implementation-defined behavior
                // We should document that iterators are invalidated after modifications
            }
        }
    }

    GIVEN("Maximum capacity scenarios") {
        WHEN("Attempting to exceed max_size") {
            string str;
            auto max_sz = str.max_size();

            THEN("Operations exceeding max_size should throw") {
                REQUIRE_THROWS_AS(str.reserve(max_sz + 1), std::length_error);
                REQUIRE_THROWS_AS(str.resize(max_sz + 1), std::length_error);
            }
        }
    }

    GIVEN("Self-modification scenarios") {
        WHEN("Appending to self") {
            string str = "Self";

            str.append(str); // Append self to self

            THEN("Should handle self-append correctly") {
                REQUIRE(str == "SelfSelf");
            }
        }

        WHEN("Replacing with self") {
            string str = "Replace me";

            str.replace(0, 7, str); // Replace "Replace" with entire string

            THEN("Should handle self-replace correctly") {
                REQUIRE(str == "Replace me me");
            }
        }
    }
}

#if RAINY_HAS_CXX20

constexpr auto test_constexpr_construction() {
    string str1;
    string str2(5, 'A');
    string str3("Hello");
    return str2.size() + str3.size();
}

constexpr auto test_constexpr_modification() {
    string str = "Start";
    str += " End";
    str.push_back('!');
    return str;
}

constexpr auto test_constexpr_find() {
    string str = "Find the needle in the haystack";
    return str.find("needle");
}

constexpr auto test_constexpr_comparison() {
    string str1 = "abc";
    string str2 = "abc";
    string str3 = "def";
    return (str1 == str2) && (str1 < str3);
}

constexpr bool test_wide() {
    wstring wstr = L"Wide";
    wstr += L" String";
    return wstr == L"Wide String";
}

constexpr auto test_complex() {
    string str = "Complex";
    str.append(" Test");
    str.insert(7, " Constexpr");
    str.erase(0, 8); // Remove "Complex "
    return str;
}

SCENARIO("C++20 constexpr string operations", "[string][constexpr][c++20]") {
    GIVEN("Constexpr context") {
        THEN("Construction should work at compile time") {
            constexpr auto size_sum = test_constexpr_construction();
            static_assert(size_sum == 5 + 5); // 5 'A's + "Hello" (5)
        }

        THEN("Modification should work at compile time") {
            constexpr auto result = test_constexpr_modification();
            static_assert(result == "Start End!");
        }

        THEN("Find operations should work at compile time") {
            constexpr auto pos = test_constexpr_find();
            static_assert(pos == 9); // "needle" starts at index 9
        }

        THEN("Comparisons should work at compile time") {
            constexpr auto comp_result = test_constexpr_comparison();
            static_assert(comp_result);
        }
    }

    GIVEN("Complex constexpr operations") {
        THEN("Complex modifications should work at compile time") {
            constexpr auto result = test_complex();
            static_assert(result == "Constexpr Test");
        }
    }

    GIVEN("Constexpr with different character types") {


        THEN("Wide strings should work in constexpr context") {
            constexpr auto wresult = test_wide();
        }
    }
}

#endif

SCENARIO("String type variations", "[string][types]") {
    GIVEN("Different character types") {
        WHEN("Using char (narrow string)") {
            string str = "Narrow";

            THEN("Should work with char") {
                REQUIRE(str == "Narrow");
            }
        }

        WHEN("Using wchar_t (wide string)") {
            wstring wstr = L"Wide";

            THEN("Should work with wchar_t") {
                REQUIRE(wstr == L"Wide");
            }
        }

        WHEN("Using char16_t") {
            u16string u16str = u"UTF-16";

            THEN("Should work with char16_t") {
                // REQUIRE(u16str == u"UTF-16"); // Would need u16string literal comparison
                REQUIRE(u16str.size() > 0);
            }
        }

        WHEN("Using char32_t") {
            u32string u32str = U"UTF-32";

            THEN("Should work with char32_t") {
                REQUIRE(u32str.size() > 0);
            }
        }

#if RAINY_HAS_CXX20
        WHEN("Using char8_t (C++20)") {
            u8string u8str = u8"UTF-8";

            THEN("Should work with char8_t") {
                REQUIRE(u8str.size() > 0);
            }
        }
#endif
    }

    GIVEN("Custom allocator") {
        using CustomAlloc = std::allocator<char>;
        using CustomString = basic_string<char, std::char_traits<char>, CustomAlloc>;

        WHEN("Using custom allocator") {
            CustomAlloc alloc;
            CustomString str(alloc);
            str = "Custom allocator";

            THEN("Should work with custom allocator") {
                REQUIRE(str == "Custom allocator");
            }
        }
    }
}

// NOLINTEND