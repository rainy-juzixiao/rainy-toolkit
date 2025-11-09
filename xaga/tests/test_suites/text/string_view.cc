/*
 * Copyright 2025 rainy-juzixiao
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
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <rainy/collections/string_view.hpp>

using rainy::text::string_view;

SCENARIO("basic_string_view default construction and pointer/size behavior", "[basic_string_view]") {
    GIVEN("A default constructed string_view") {
        string_view sv;

        THEN("size is zero") {
            REQUIRE(sv.size() == 0);
            REQUIRE(sv.length() == 0);
        }

        THEN("empty returns true") {
            REQUIRE(sv.empty());
        }

        THEN("data() may be null or unspecified, but size must be zero") {
            REQUIRE(sv.size() == 0);
        }
    }
}

SCENARIO("basic_string_view construction from char pointer", "[basic_string_view]") {
    GIVEN("A C-string literal") {
        const char *str = "hello";
        string_view sv(str);

        THEN("size matches length") {
            REQUIRE(sv.size() == 5);
        }

        THEN("data points to provided pointer") {
            REQUIRE(sv.data() == str);
        }

        THEN("operator[] works") {
            REQUIRE(sv[0] == 'h');
            REQUIRE(sv[4] == 'o');
        }

        THEN("front() and back() are correct") {
            REQUIRE(sv.front() == 'h');
            REQUIRE(sv.back() == 'o');
        }
    }
}

SCENARIO("basic_string_view construction with pointer and length", "[basic_string_view]") {
    GIVEN("A pointer and a manual length") {
        const char *str = "hello world";
        string_view sv(str, 5);

        THEN("The view consists of 'hello'") {
            REQUIRE(sv.size() == 5);
            REQUIRE(sv.compare("hello") == 0);
        }
    }
}

SCENARIO("basic_string_view iterator behavior", "[basic_string_view]") {
    GIVEN("A string_view 'abc'") {
        string_view sv("abc");

        THEN("begin() and end() traverse all characters") {
            std::string collected;
            for (auto it = sv.begin(); it != sv.end(); ++it) {
                collected.push_back(*it);
            }
            REQUIRE(collected == "abc");
        }

        THEN("cbegin and cend behave the same") {
            std::string collected;
            for (auto it = sv.cbegin(); it != sv.cend(); ++it) {
                collected.push_back(*it);
            }
            REQUIRE(collected == "abc");
        }

        THEN("reverse iterators traverse backwards") {
            std::string collected;
            for (auto it = sv.rbegin(); it != sv.rend(); ++it) {
                collected.push_back(*it);
            }
            REQUIRE(collected == "cba");
        }

        THEN("crbegin and crend work likewise") {
            std::string collected;
            for (auto it = sv.crbegin(); it != sv.crend(); ++it) {
                collected.push_back(*it);
            }
            REQUIRE(collected == "cba");
        }
    }
}

SCENARIO("basic_string_view copy()", "[basic_string_view]") {
    GIVEN("A string_view 'example'") {
        string_view sv("example");

        WHEN("copying into a buffer") {
            char buf[16]{};
            auto written = sv.copy(buf, 7);

            THEN("returns number of written chars") {
                REQUIRE(written == 7);
            }

            THEN("buffer contains copied content") {
                REQUIRE(std::string(buf, 7) == "example");
            }
        }

        WHEN("copying with offset") {
            char buf[16]{};
            auto written = sv.copy(buf, 4, 2);

            THEN("cxtracts substring 'ampl' (from 'example')") {
                REQUIRE(std::string(buf, 4) == "ampl");
                REQUIRE(written == 4);
            }
        }
    }
}

SCENARIO("basic_string_view substr()", "[basic_string_view]") {
    GIVEN("A string_view 'hello world'") {
        string_view sv("hello world");

        THEN("substr(0,5) gives 'hello'") {
            REQUIRE(sv.substr(0, 5).compare("hello") == 0);
        }

        THEN("substr(6) gives 'world'") {
            REQUIRE(sv.substr(6).compare("world") == 0);
        }

        THEN("substr with npos extracts remainder") {
            REQUIRE(sv.substr(6, string_view::npos).compare("world") == 0);
        }
    }
}

SCENARIO("basic_string_view compare overloads", "[basic_string_view]") {
    GIVEN("Three views 'abc', 'abd', 'abc'") {
        string_view a("abc");
        string_view b("abd");
        string_view a2("abc");

        THEN("compare works for equal") {
            REQUIRE(a.compare(a2) == 0);
        }

        THEN("compare detects lexicographical order") {
            REQUIRE(a.compare(b) < 0);
            REQUIRE(b.compare(a) > 0);
        }

        THEN("compare(CharType*) works") {
            REQUIRE(a.compare("abc") == 0);
            REQUIRE(a.compare("abd") < 0);
        }

        THEN("compare(pos,len,sv) works") {
            string_view t("prefix-abc-suffix");
            REQUIRE(t.compare(7, 3, a) == 0);
        }

        THEN("compare(pos,len,cstr) works") {
            string_view t("prefix-abc-suffix");
            REQUIRE(t.compare(7, 3, "abc") == 0);
        }
    }
}

SCENARIO("basic_string_view starts_with and ends_with", "[basic_string_view]") {
    GIVEN("A string_view 'testing'") {
        string_view sv("testing");

        THEN("starts_with various types") {
            REQUIRE(sv.starts_with('t'));
            REQUIRE(sv.starts_with("test"));
            REQUIRE_FALSE(sv.starts_with("sting"));
        }

        THEN("ends_with various types") {
            REQUIRE(sv.ends_with('g'));
            REQUIRE(sv.ends_with("ing"));
            REQUIRE_FALSE(sv.ends_with("test"));
        }
    }
}

SCENARIO("basic_string_view contains()", "[basic_string_view]") {
    GIVEN("A string_view 'foobar'") {
        string_view sv("foobar");

        THEN("contains character") {
            REQUIRE(sv.contains('f'));
            REQUIRE_FALSE(sv.contains('z'));
        }

        THEN("contains substring") {
            REQUIRE(sv.contains("foo"));
            REQUIRE(sv.contains("bar"));
            REQUIRE_FALSE(sv.contains("baz"));
        }

        THEN("contains string_view") {
            REQUIRE(sv.contains(string_view("oba")));
        }
    }
}

SCENARIO("basic_string_view find() and rfind()", "[basic_string_view]") {
    GIVEN("A string_view 'abracadabra'") {
        string_view sv("abracadabra");

        THEN("find character") {
            REQUIRE(sv.find('a') == 0);
            REQUIRE(sv.find('a', 1) == 3);
        }

        THEN("find substring") {
            REQUIRE(sv.find("bra") == 1);
            REQUIRE(sv.find("bra", 2) == 8);
        }

        THEN("rfind character") {
            REQUIRE(sv.rfind('a') == 10);
            REQUIRE(sv.rfind('a', 9) == 7);
        }

        THEN("rfind substring") {
            REQUIRE(sv.rfind("abra") == 7);
        }
    }
}

SCENARIO("basic_string_view find_first_of / find_last_of", "[basic_string_view]") {
    GIVEN("A string_view 'hello world'") {
        string_view sv("hello world");

        THEN("find_first_of works") {
            REQUIRE(sv.find_first_of("aeiou") == 1); // 'e'
        }

        THEN("find_last_of works") {
            REQUIRE(sv.find_last_of("aeiou") == 7); // 'o' in "world"
        }

        THEN("single char overloads") {
            REQUIRE(sv.find_first_of('o') == 4);
            REQUIRE(sv.find_last_of('o') == 7);
        }
    }
}

SCENARIO("basic_string_view find_first_not_of / find_last_not_of", "[basic_string_view]") {
    GIVEN("A string_view '--==test==--'") {
        string_view sv("--==test==--");

        THEN("find_first_not_of works") {
            REQUIRE(sv.find_first_not_of("-=") == 4);
        }

        THEN("find_last_not_of works") {
            REQUIRE(sv.find_last_not_of("-=") == 7);
        }

        THEN("single char overloads work") {
            string_view s2("aaaabcaaa");
            REQUIRE(s2.find_first_not_of('a') == 4);
            REQUIRE(s2.find_last_not_of('a') == 5);
        }
    }
}

SCENARIO("basic_string_view remove_prefix and remove_suffix", "[basic_string_view]") {
    GIVEN("A string_view 'prefix-middle-suffix'") {
        string_view sv("prefix-middle-suffix");

        WHEN("Prefix is removed") {
            sv.remove_prefix(7); // remove "prefix-"
            THEN("View starts at 'middle-suffix'") {
                REQUIRE(sv.compare("middle-suffix") == 0);
            }
        }

        WHEN("Suffix is removed") {
            sv.remove_suffix(7); // remove "-suffix"
            THEN("View becomes 'prefix-middle'") {
                REQUIRE(sv.compare("prefix-middle") == 0);
            }
        }
    }
}

SCENARIO("basic_string_view swap()", "[basic_string_view]") {
    GIVEN("Two string_views") {
        string_view a("hello");
        string_view b("world");

        WHEN("Swapping them") {
            a.swap(b);

            THEN("They exchange their contents") {
                REQUIRE(a.compare("world") == 0);
                REQUIRE(b.compare("hello") == 0);
            }
        }
    }
}

SCENARIO("basic_string_view range_check throws exception", "[basic_string_view]") {
    GIVEN("A string_view 'abc'") {
        string_view sv("abc");

        THEN("at() within range works") {
            REQUIRE(sv.at(1) == 'b');
        }

        THEN("out-of-range access throws") {
            REQUIRE_THROWS(sv.at(3));
        }
    }
}
