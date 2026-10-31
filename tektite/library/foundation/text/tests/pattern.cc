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
#include <rainy/foundation/text/pattern/glob.hpp>

using rainy::core::text::string_view;
using rainy::foundation::text::pattern::glob_match;
using rainy::foundation::text::pattern::glob_match_path;

TEST_CASE("glob literal matching", "[foundation][text][pattern]") {
    REQUIRE(glob_match("", ""));
    REQUIRE(glob_match("abc", "abc"));
    REQUIRE_FALSE(glob_match("abc", "abd"));
    REQUIRE_FALSE(glob_match("abc", "ab"));
    REQUIRE_FALSE(glob_match("abc", "abcd"));
    REQUIRE_FALSE(glob_match("", "a"));
}

TEST_CASE("glob star", "[foundation][text][pattern]") {
    REQUIRE(glob_match("*", ""));
    REQUIRE(glob_match("*", "anything at all"));
    REQUIRE(glob_match("a*", "abc"));
    REQUIRE(glob_match("a*", "a"));
    REQUIRE(glob_match("*c", "abc"));
    REQUIRE(glob_match("a*c", "abc"));
    REQUIRE(glob_match("a*c", "ac"));
    REQUIRE(glob_match("a*c", "aXXc"));
    REQUIRE_FALSE(glob_match("a*c", "ab"));
    REQUIRE(glob_match("*a*", "bab"));
    REQUIRE(glob_match("*.txt", "readme.txt"));
    REQUIRE_FALSE(glob_match("*.txt", "readme.md"));
    REQUIRE(glob_match("**.cc", "deep/nested/file.cc"));
}

TEST_CASE("glob question mark", "[foundation][text][pattern]") {
    REQUIRE(glob_match("a?c", "abc"));
    REQUIRE_FALSE(glob_match("a?c", "ac"));
    REQUIRE_FALSE(glob_match("a?c", "abbc"));
    REQUIRE(glob_match("???", "abc"));
    REQUIRE_FALSE(glob_match("???", "ab"));
    REQUIRE(glob_match("a?c", "a你c"));
}

TEST_CASE("glob character class", "[foundation][text][pattern]") {
    REQUIRE(glob_match("[abc]", "a"));
    REQUIRE(glob_match("[abc]", "c"));
    REQUIRE_FALSE(glob_match("[abc]", "d"));
    REQUIRE(glob_match("[a-z]", "m"));
    REQUIRE_FALSE(glob_match("[a-z]", "M"));
    REQUIRE(glob_match("[a-zA-Z]", "M"));
    REQUIRE(glob_match("[!abc]", "d"));
    REQUIRE_FALSE(glob_match("[!abc]", "a"));
    REQUIRE(glob_match("[^abc]", "d"));
    REQUIRE(glob_match("file[0-9].txt", "file7.txt"));
    REQUIRE_FALSE(glob_match("file[0-9].txt", "filex.txt"));
    REQUIRE(glob_match("[a-cx-z]", "y"));
    REQUIRE_FALSE(glob_match("[a-cx-z]", "m"));
}

TEST_CASE("glob malformed constructs", "[foundation][text][pattern]") {
    SECTION("unclosed bracket is a literal") {
        REQUIRE(glob_match("[abc", "[abc"));
        REQUIRE_FALSE(glob_match("[abc", "a"));
    }
    SECTION("empty class is a literal") {
        REQUIRE(glob_match("[]", "[]"));
    }
    SECTION("consecutive stars collapse") {
        REQUIRE(glob_match("a**b", "ab"));
        REQUIRE(glob_match("a**b", "aXYZb"));
        REQUIRE(glob_match("***", "anything"));
    }
    SECTION("star only pattern matches empty") {
        REQUIRE(glob_match("**", ""));
        REQUIRE(glob_match("***", ""));
    }
}

TEST_CASE("glob path semantics", "[foundation][text][pattern]") {
    SECTION("star does not cross separators") {
        REQUIRE(glob_match_path("*.txt", "readme.txt"));
        REQUIRE_FALSE(glob_match_path("*.txt", "dir/readme.txt"));
        REQUIRE(glob_match_path("src/*.cc", "src/main.cc"));
        REQUIRE_FALSE(glob_match_path("src/*.cc", "src/sub/main.cc"));
    }
    SECTION("globstar crosses separators") {
        REQUIRE(glob_match_path("src/**/*.cc", "src/main.cc"));
        REQUIRE(glob_match_path("src/**/*.cc", "src/sub/deep/main.cc"));
        REQUIRE(glob_match_path("**/*.txt", "a/b/c.txt"));
        REQUIRE(glob_match_path("**", "a/b/c"));
    }
    SECTION("question mark does not cross separators") {
        REQUIRE(glob_match_path("a?c", "abc"));
        REQUIRE_FALSE(glob_match_path("a?c", "a/c"));
    }
    SECTION("character class does not cross separators") {
        REQUIRE_FALSE(glob_match_path("[a-z]", "/"));
    }
    SECTION("backslash is a separator too") {
        REQUIRE_FALSE(glob_match_path("*.txt", "dir\\readme.txt"));
    }
    SECTION("exact path") {
        REQUIRE(glob_match_path("a/b/c", "a/b/c"));
        REQUIRE_FALSE(glob_match_path("a/b/c", "a/b/d"));
    }
    SECTION("trailing star stays in segment") {
        REQUIRE(glob_match_path("src/*", "src/main.cc"));
        REQUIRE_FALSE(glob_match_path("src/*", "src/sub/main.cc"));
    }
    SECTION("leading globstar with segment") {
        REQUIRE(glob_match_path("**/main.cc", "src/sub/main.cc"));
        REQUIRE(glob_match_path("**/main.cc", "main.cc"));
    }
}

TEST_CASE("glob general mode treats separators as ordinary", "[foundation][text][pattern]") {
    REQUIRE(glob_match("*.txt", "dir/readme.txt"));
    REQUIRE(glob_match("a?c", "a/c"));
    REQUIRE(glob_match("*", "a/b/c"));
}
