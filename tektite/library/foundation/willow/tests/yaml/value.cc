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
#include <rainy/foundation/willow/yaml.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::yaml;

TEST_CASE("yaml value - type predicates", "[willow][yaml][value]") {
    REQUIRE(yaml::parse("{}").is_object());
    REQUIRE(yaml::parse("[]").is_array());
    REQUIRE(yaml::parse("text\n").is_string());
    REQUIRE(yaml::parse("true\n").is_bool());
    REQUIRE(yaml::parse("42\n").is_integer());
    REQUIRE(yaml::parse("2.5\n").is_float());
    REQUIRE(yaml::parse("~\n").is_null());

    REQUIRE(yaml::parse("42\n").is_number());
    REQUIRE(yaml::parse("2.5\n").is_number());
    REQUIRE(!yaml::parse("text\n").is_number());

    REQUIRE(yaml::parse("42\n").is_primitive());
    REQUIRE(yaml::parse("2.5\n").is_primitive());
    REQUIRE(yaml::parse("text\n").is_primitive());
    REQUIRE(yaml::parse("true\n").is_primitive());
    REQUIRE(yaml::parse("~\n").is_primitive());
    REQUIRE(!yaml::parse("[]").is_primitive());
    REQUIRE(!yaml::parse("{}").is_primitive());
}

TEST_CASE("yaml value - type_name", "[willow][yaml][value]") {
    REQUIRE(yaml::parse("{}").type_name() == "object");
    REQUIRE(yaml::parse("[]").type_name() == "array");
    REQUIRE(yaml::parse("text\n").type_name() == "string");
    REQUIRE(yaml::parse("true\n").type_name() == "boolean");
    REQUIRE(yaml::parse("42\n").type_name() == "integer");
    REQUIRE(yaml::parse("2.5\n").type_name() == "float");
    REQUIRE(yaml::parse("~\n").type_name() == "null");
}

TEST_CASE("yaml value - scalar accessors", "[willow][yaml][value]") {
    REQUIRE(yaml::parse("42\n").as_integer() == 42);
    REQUIRE(yaml::parse("2.5\n").as_float() == 2.5);
    REQUIRE(yaml::parse("text\n").as_string() == "text");
    REQUIRE(yaml::parse("text\n").as_string_view() == "text");
    REQUIRE(yaml::parse("true\n").as_bool());
}

TEST_CASE("yaml value - container accessors", "[willow][yaml][value]") {
    const auto object = yaml::parse("a: 1\n");
    REQUIRE(object.as_object().size() == 1);

    const auto array = yaml::parse("- 1\n- 2\n");
    REQUIRE(array.as_array().size() == 2);

    const auto string = yaml::parse("text\n");
    REQUIRE_THROWS_AS(string.as_integer(), rainy::foundation::exceptions::willow::willow_type_error);
    REQUIRE_THROWS_AS(string.as_array(), rainy::foundation::exceptions::willow::willow_type_error);
}

TEST_CASE("yaml value - size and empty", "[willow][yaml][value]") {
    REQUIRE(yaml::parse("{}").size() == 0);
    REQUIRE(yaml::parse("{}").empty());
    REQUIRE(yaml::parse("[]").empty());
    REQUIRE(!yaml::parse("a: 1\n").empty());
    REQUIRE(yaml::parse("a: 1\n").size() == 1);
    REQUIRE(yaml::parse("42\n").size() == 1);
}

TEST_CASE("yaml value - lookup helpers", "[willow][yaml][value]") {
    const auto doc = yaml::parse("alpha: 1\nbeta: 2\n");
    REQUIRE(doc.contains("alpha"));
    REQUIRE(!doc.contains("gamma"));
    REQUIRE(doc.count("alpha") == 1);
    REQUIRE(doc.count("gamma") == 0);
    REQUIRE(doc.find("beta") != doc.cend());
    REQUIRE(doc.find("beta")->value().as_integer() == 2);
    REQUIRE(yaml::parse("- 1\n").count("alpha") == 0);
}

TEST_CASE("yaml value - array subscript", "[willow][yaml][value]") {
    const auto doc = yaml::parse("[10, 20, 30]");
    REQUIRE(doc[0].as_integer() == 10);
    REQUIRE(doc[2].as_integer() == 30);
}

TEST_CASE("yaml value - const out of range access throws", "[willow][yaml][value]") {
    const yaml::facade<document> array = yaml::parse("[1]");
    REQUIRE_THROWS_AS(array[5], std::out_of_range);

    const yaml::facade<document> object = yaml::parse("k: 1\n");
    REQUIRE_THROWS_AS(object["missing"], std::out_of_range);
}

TEST_CASE("yaml value - subscript on wrong type throws", "[willow][yaml][value]") {
    using rainy::foundation::exceptions::willow::willow_invalid_key;
    yaml::facade<document> scalar = yaml::parse("42\n");
    REQUIRE_THROWS_AS(scalar["key"], willow_invalid_key);
}

TEST_CASE("yaml value - get and explicit conversion", "[willow][yaml][value]") {
    REQUIRE(yaml::parse("42\n").get<int>() == 42);
    REQUIRE(yaml::parse("2.5\n").get<double>() == 2.5);
    REQUIRE(yaml::parse("true\n").get<bool>());
    REQUIRE(yaml::parse("text\n").get<document::string_type>() == "text");
    REQUIRE(static_cast<int>(yaml::parse("42\n")) == 42);
    REQUIRE(static_cast<document::string_type>(yaml::parse("text\n")) == "text");
}

TEST_CASE("yaml value - equality and ordering", "[willow][yaml][value]") {
    const auto lhs = yaml::parse("a: 1\nb: [1, 2]\n");
    const auto rhs = yaml::parse("a: 1\nb: [1, 2]\n");
    REQUIRE(lhs == rhs);
    REQUIRE(!(lhs != rhs));

    REQUIRE(yaml::parse("1\n") == yaml::parse("1\n"));
    REQUIRE(yaml::parse("1\n") != yaml::parse("2\n"));
    REQUIRE(yaml::parse("1\n") < yaml::parse("2\n"));
    REQUIRE(yaml::parse("2\n") > yaml::parse("1\n"));
}

TEST_CASE("yaml value - iteration over parsed containers", "[willow][yaml][value]") {
    const auto array = yaml::parse("[1, 2, 3]");
    int sum = 0;
    for (const auto &element: array) {
        sum += element.value().as_integer();
    }
    REQUIRE(sum == 6);

    const auto object = yaml::parse("a: 1\nb: 2\n");
    int total = 0;
    for (const auto &element: object) {
        total += element.value().as_integer();
    }
    REQUIRE(total == 3);
}

TEST_CASE("yaml value - mapping keys via iteration", "[willow][yaml][value]") {
    const auto doc = yaml::parse("alpha: 1\nbeta: 2\n");
    std::vector<std::string> keys;
    for (const auto &element: doc) {
        keys.emplace_back(element.key().c_str());
    }
    std::sort(keys.begin(), keys.end());
    REQUIRE(keys.size() == 2);
    REQUIRE(keys[0] == "alpha");
    REQUIRE(keys[1] == "beta");
}

TEST_CASE("yaml value - key() on a non-mapping throws", "[willow][yaml][value]") {
    using rainy::foundation::exceptions::willow::willow_invalid_iterator;
    const auto array = yaml::parse("- 1\n");
    REQUIRE_THROWS_AS(array.cbegin()->key(), willow_invalid_iterator);

    const auto scalar = yaml::parse("42\n");
    REQUIRE_THROWS_AS(scalar.cbegin()->key(), willow_invalid_iterator);
}

TEST_CASE("yaml value - reverse iteration", "[willow][yaml][value]") {
    const auto array = yaml::parse("[1, 2]");
    auto iter = array.rbegin();
    REQUIRE(iter->value().as_integer() == 2);
    ++iter;
    REQUIRE(iter->value().as_integer() == 1);
    ++iter;
    REQUIRE(iter == array.rend());
}

TEST_CASE("yaml value - erase and clear", "[willow][yaml][value]") {
    auto object = yaml::parse("a: 1\nb: 2\n");
    REQUIRE(object.erase("a") == 1);
    REQUIRE(object.size() == 1);
    REQUIRE(!object.contains("a"));

    auto array = yaml::parse("- 1\n- 2\n- 3\n");
    array.erase(static_cast<yaml::facade<document>::size_type>(0));
    REQUIRE(array.size() == 2);
    REQUIRE(array[0].as_integer() == 2);

    array.clear();
    REQUIRE(array.is_null());
}

TEST_CASE("yaml value - push_back and emplace_back", "[willow][yaml][value]") {
    yaml::facade<document> array = yaml::parse("[]");
    array.push_back(1);
    array.push_back("two");
    array.emplace_back(true);
    REQUIRE(array.size() == 3);
    REQUIRE(array[0].as_integer() == 1);
    REQUIRE(array[1].as_string() == "two");
    REQUIRE(array[2].as_bool());
}

TEST_CASE("yaml value - metadata accessors require state", "[willow][yaml][value]") {
    const auto plain = yaml::parse("a: 1\n");
    REQUIRE(!plain.has_anchor());
    REQUIRE(!plain.has_alias());
    REQUIRE(!plain.has_tag());
    REQUIRE(!plain.has_directive());
    REQUIRE(plain.has_style());
    REQUIRE_THROWS_AS(plain.anchor(), rainy::core::exceptions::logic::out_of_range);
    REQUIRE_THROWS_AS(plain.tag(), rainy::core::exceptions::logic::out_of_range);
}
