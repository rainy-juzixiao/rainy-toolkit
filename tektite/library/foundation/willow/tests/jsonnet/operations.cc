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
#include <rainy/foundation/willow/implements/common/operations.hpp>
#include <rainy/foundation/willow/json.hpp>

using namespace rainy::foundation::willow;
namespace ops = rainy::foundation::willow::implements::operations;

namespace {
    document arr(std::initializer_list<document> items) {
        return document::array(items);
    }

    document obj(std::initializer_list<document> items) {
        return document::object(items);
    }
}

TEST_CASE("operations - type_name", "[willow][operations]") {
    REQUIRE(ops::type_name(document{}) == "null");
    REQUIRE(ops::type_name(document(true)) == "boolean");
    REQUIRE(ops::type_name(document(1)) == "number");
    REQUIRE(ops::type_name(document(1.5)) == "number");
    REQUIRE(ops::type_name(document("x")) == "string");
    REQUIRE(ops::type_name(arr({})) == "array");
    REQUIRE(ops::type_name(obj({})) == "object");
}

TEST_CASE("operations - length over strings, arrays and objects", "[willow][operations]") {
    REQUIRE(ops::length(document("")) == 0);
    REQUIRE(ops::length(document("abc")) == 3);
    REQUIRE(ops::length(document("\xc3\xa9")) == 1);
    REQUIRE(ops::length(document("\xe4\xb8\xad\xe6\x96\x87")) == 2);
    REQUIRE(ops::length(document("\xf0\x9f\x98\x80")) == 1);
    REQUIRE(ops::length(arr({document(1), document(2)})) == 2);
    REQUIRE(ops::length(obj({{"a", 1}, {"b", 2}})) == 2);
    REQUIRE_THROWS(ops::length(document(1)));
}

TEST_CASE("operations - primitive_equals", "[willow][operations]") {
    REQUIRE(ops::primitive_equals(document(1), document(1)));
    REQUIRE(ops::primitive_equals(document(1), document(1.0)));
    REQUIRE(ops::primitive_equals(document("a"), document("a")));
    REQUIRE(ops::primitive_equals(document(true), document(true)));
    REQUIRE(ops::primitive_equals(document{}, document{}));
    REQUIRE_FALSE(ops::primitive_equals(document(1), document(2)));
    REQUIRE_FALSE(ops::primitive_equals(document("a"), document("b")));
    REQUIRE_FALSE(ops::primitive_equals(document(1), document("1")));
    REQUIRE_FALSE(ops::primitive_equals(arr({}), arr({})));
}

TEST_CASE("operations - deep equals", "[willow][operations]") {
    REQUIRE(ops::equals(arr({document(1), document(2)}), arr({document(1), document(2)})));
    REQUIRE_FALSE(ops::equals(arr({document(1), document(2)}), arr({document(2), document(1)})));
    REQUIRE(ops::equals(obj({{"a", 1}, {"b", 2}}), obj({{"b", 2}, {"a", 1}})));
    REQUIRE_FALSE(ops::equals(obj({{"a", 1}}), obj({{"a", 2}})));
    REQUIRE_FALSE(ops::equals(obj({{"a", 1}}), obj({{"a", 1}, {"b", 2}})));
    REQUIRE(ops::equals(document(1), document(1.0)));
}

TEST_CASE("operations - compare orders values", "[willow][operations]") {
    REQUIRE(ops::compare(document(1), document(2)) == -1);
    REQUIRE(ops::compare(document(2), document(1)) == 1);
    REQUIRE(ops::compare(document(2), document(2)) == 0);
    REQUIRE(ops::compare(document("a"), document("b")) == -1);
    REQUIRE(ops::compare(document(false), document(true)) == -1);
    REQUIRE(ops::compare(document(1), document("1")) != 0);
}

TEST_CASE("operations - modulo floors toward the divisor sign", "[willow][operations]") {
    REQUIRE(ops::modulo(7.0, 3.0) == 1.0);
    REQUIRE(ops::modulo(-7.0, 3.0) == 2.0);
    REQUIRE(ops::modulo(7.0, -3.0) == -2.0);
    REQUIRE(ops::modulo(6.0, 3.0) == 0.0);
}

TEST_CASE("operations - codepoint and char round-trip", "[willow][operations]") {
    REQUIRE(ops::codepoint<document>(document("A").as_string()) == 65);
    REQUIRE(ops::codepoint<document>(document("\xe4\xb8\xad").as_string()) == 0x4E2D);
    REQUIRE(ops::codepoint<document>(document("\xf0\x9f\x98\x80").as_string()) == 0x1F600);
    REQUIRE(ops::char_from_code<document>(65).as_string() == "A");
    REQUIRE(ops::char_from_code<document>(0x4E2D).as_string() == "\xe4\xb8\xad");
    REQUIRE(ops::char_from_code<document>(0x1F600).as_string() == "\xf0\x9f\x98\x80");
}

TEST_CASE("operations - string_chars splits by code point", "[willow][operations]") {
    const auto chars = ops::string_chars(document("a\xc3\xa9\xe4\xb8\xad"));
    REQUIRE(chars.is_array());
    REQUIRE(chars.size() == 3);
    REQUIRE(chars[0].as_string() == "a");
    REQUIRE(chars[1].as_string() == "\xc3\xa9");
    REQUIRE(chars[2].as_string() == "\xe4\xb8\xad");
}

TEST_CASE("operations - object_has, object_fields and object_values", "[willow][operations]") {
    const auto object = obj({{"a", 1}, {"b", 2}});
    REQUIRE(ops::object_has(object, "a"));
    REQUIRE_FALSE(ops::object_has(object, "z"));
    REQUIRE_FALSE(ops::object_has(document(1), "a"));
    const auto fields = ops::object_fields(object);
    REQUIRE(fields.size() == 2);
    const auto values = ops::object_values(object);
    REQUIRE(values.size() == 2);
    REQUIRE_THROWS(ops::object_fields(document(1)));
}

TEST_CASE("operations - join strings and arrays", "[willow][operations]") {
    REQUIRE(ops::join(document(","), arr({document("a"), document("b"), document("c")})).as_string() == "a,b,c");
    REQUIRE(ops::join(document(","), arr({})).as_string() == "");
    REQUIRE(ops::join(arr({document(0)}), arr({arr({document(1)}), arr({document(2)})})).size() == 3);
    REQUIRE_THROWS(ops::join(document(","), document(1)));
}

TEST_CASE("operations - reverse, range and flattenArrays", "[willow][operations]") {
    const auto reversed = ops::reverse(arr({document(1), document(2), document(3)}));
    REQUIRE(reversed[0].as_integer() == 3);
    REQUIRE(reversed[2].as_integer() == 1);
    REQUIRE(ops::reverse(document("abc")).as_string() == "cba");
    REQUIRE_THROWS(ops::reverse(document(1)));

    const auto numbers = ops::range<document>(2, 5);
    REQUIRE(numbers.size() == 3);
    REQUIRE(numbers[0].as_integer() == 2);
    REQUIRE(numbers[2].as_integer() == 4);

    const auto flat = ops::flatten_arrays(arr({arr({document(1), document(2)}), arr({document(3)})}));
    REQUIRE(flat.size() == 3);
    REQUIRE(flat[2].as_integer() == 3);
}

TEST_CASE("operations - repeat, count, member and set operations", "[willow][operations]") {
    REQUIRE(ops::repeat(document("ab"), 3).as_string() == "ababab");
    REQUIRE(ops::repeat(document(7), 2).size() == 2);
    REQUIRE(ops::count(arr({document(1), document(2), document(1)}), document(1)) == 2);
    REQUIRE(ops::member(arr({document(1), document(2)}), document(2)));
    REQUIRE_FALSE(ops::member(arr({document(1)}), document(2)));

    const auto uni = ops::set_union(arr({document(1), document(2)}), arr({document(2), document(3)}));
    REQUIRE(uni.size() == 3);
    const auto inter = ops::set_inter(arr({document(1), document(2)}), arr({document(2), document(3)}));
    REQUIRE(inter.size() == 1);
    const auto diff = ops::set_diff(arr({document(1), document(2)}), arr({document(2)}));
    REQUIRE(diff.size() == 1);
    REQUIRE(diff[0].as_integer() == 1);
}

TEST_CASE("operations - ascii case, prefixes and substrings", "[willow][operations]") {
    REQUIRE(ops::ascii_upper(document("aBc")).as_string() == "ABC");
    REQUIRE(ops::ascii_lower(document("aBc")).as_string() == "abc");
    REQUIRE(ops::starts_with(document("hello"), "he"));
    REQUIRE_FALSE(ops::starts_with(document("hello"), "lo"));
    REQUIRE(ops::ends_with(document("hello"), "lo"));
    REQUIRE_FALSE(ops::ends_with(document("hello"), "he"));
    REQUIRE(ops::substr(document("hello"), 1, 3).as_string() == "ell");
    REQUIRE(ops::substr(document("hello"), -2, 2).as_string() == "lo");
    REQUIRE(ops::substr(document("hello"), 0, 100).as_string() == "hello");
}

TEST_CASE("operations - slice over arrays and strings", "[willow][operations]") {
    const auto array = arr({document(0), document(1), document(2), document(3), document(4)});
    REQUIRE(ops::slice(array, 1, 4, 1).size() == 3);
    REQUIRE(ops::slice(array, 0, 5, 2).size() == 3);
    REQUIRE(ops::slice(array, -2, 5, 1).size() == 2);
    REQUIRE(ops::slice(document("abcde"), 1, 4, 1).as_string() == "bcd");
    REQUIRE_THROWS(ops::slice(document(1), 0, 1, 1));
}

TEST_CASE("operations - strip, split and find", "[willow][operations]") {
    REQUIRE(ops::strip_chars(document("  hi  "), " ", true, true).as_string() == "hi");
    REQUIRE(ops::strip_chars(document("xxhixx"), "x", true, false).as_string() == "hixx");
    REQUIRE(ops::strip_chars(document("xxhixx"), "x", false, true).as_string() == "xxhi");

    const auto parts = ops::split_limit(document("a,b,c"), ",", -1);
    REQUIRE(parts.size() == 3);
    const auto limited = ops::split_limit(document("a,b,c"), ",", 1);
    REQUIRE(limited.size() == 2);
    REQUIRE(limited[1].as_string() == "b,c");

    REQUIRE(ops::find_substr(document("hello"), "ll") == 2);
    REQUIRE(ops::find_substr(document("hello"), "z") == -1);
}

TEST_CASE("operations - parse_int and math helpers", "[willow][operations]") {
    REQUIRE(ops::parse_int(document("42")) == 42);
    REQUIRE(ops::parse_int(document("-7")) == -7);
    REQUIRE_THROWS(ops::parse_int(document("4x")));
    REQUIRE(ops::clamp(5.0, 0.0, 3.0) == 3.0);
    REQUIRE(ops::clamp(-1.0, 0.0, 3.0) == 0.0);
    REQUIRE(ops::sign(-2.0) == -1.0);
    REQUIRE(ops::sign(0.0) == 0.0);
    REQUIRE(ops::sign(9.0) == 1.0);
    REQUIRE(ops::is_integer_value(3.0));
    REQUIRE_FALSE(ops::is_integer_value(3.5));
}

TEST_CASE("operations - typed accessors reject mismatches", "[willow][operations]") {
    REQUIRE(ops::as_number(document(3)) == 3.0);
    REQUIRE_THROWS(ops::as_number(document("x")));
    REQUIRE(ops::as_string(document("x")).as_string() == "x");
    REQUIRE_THROWS(ops::as_string(document(1)));
    REQUIRE(ops::as_array(arr({})).is_array());
    REQUIRE_THROWS(ops::as_array(document(1)));
    REQUIRE(ops::as_object(obj({})).is_object());
    REQUIRE_THROWS(ops::as_object(document(1)));
}
