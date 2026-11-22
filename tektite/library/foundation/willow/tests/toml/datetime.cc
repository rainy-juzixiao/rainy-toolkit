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
#include <rainy/foundation/willow/toml.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::toml;

TEST_CASE("toml datetime - offset date time", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("d = 1979-05-27T07:32:00Z\n");

    const auto &node = doc["d"];
    REQUIRE(node.has_datetime());
    const auto fields = node.as_datetime();
    REQUIRE(fields.kind == datetime_kind::offset_date_time);
    REQUIRE(fields.year == 1979);
    REQUIRE(fields.month == 5);
    REQUIRE(fields.day == 27);
    REQUIRE(fields.hour == 7);
    REQUIRE(fields.minute == 32);
    REQUIRE(fields.second == 0);
    REQUIRE(fields.has_offset);
    REQUIRE(fields.offset_minutes == 0);
    REQUIRE(fields.use_zulu);
}

TEST_CASE("toml datetime - explicit offset", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("d = 1979-05-27T00:32:00-07:00\n");

    const auto fields = doc["d"].as_datetime();
    REQUIRE(fields.kind == datetime_kind::offset_date_time);
    REQUIRE(fields.has_offset);
    REQUIRE_FALSE(fields.use_zulu);
    REQUIRE(fields.offset_minutes == -7 * 60);
}

TEST_CASE("toml datetime - local date time with space separator and fraction", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("d = 1979-05-27 07:32:00.999999\n");

    const auto fields = doc["d"].as_datetime();
    REQUIRE(fields.kind == datetime_kind::local_date_time);
    REQUIRE_FALSE(fields.has_offset);
    REQUIRE(fields.use_space_separator);
    REQUIRE(fields.nanosecond == 999999000);
}

TEST_CASE("toml datetime - local date", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("d = 1979-05-27\n");

    const auto &node = doc["d"];
    REQUIRE(node.has_datetime());
    const auto fields = node.as_datetime();
    REQUIRE(fields.kind == datetime_kind::local_date);
    REQUIRE(fields.year == 1979);
    REQUIRE(fields.month == 5);
    REQUIRE(fields.day == 27);
}

TEST_CASE("toml datetime - local time", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("t = 07:32:00\n");

    const auto &node = doc["t"];
    REQUIRE(node.has_datetime());
    const auto fields = node.as_datetime();
    REQUIRE(fields.kind == datetime_kind::local_time);
    REQUIRE(fields.hour == 7);
    REQUIRE(fields.minute == 32);
    REQUIRE(fields.second == 0);
}

TEST_CASE("toml datetime - a plain string is not a datetime", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("s = \"1979-05-27\"\n");

    const auto &node = doc["s"];
    REQUIRE(node.is_string());
    REQUIRE_FALSE(node.has_datetime());
}

TEST_CASE("toml datetime - the spelling survives a round trip", "[willow][toml]") {
    const rainy::core::text::string source = "a = 1979-05-27T07:32:00Z\nb = 1979-05-27 07:32:00\nc = 07:32:00\nd = 1979-05-27\n";
    toml::facade<document> doc = toml::parse(source);

    const auto dumped = toml::dump(doc);
    REQUIRE(dumped.find("1979-05-27T07:32:00Z") != rainy::core::text::string::npos);
    REQUIRE(dumped.find("1979-05-27 07:32:00") != rainy::core::text::string::npos);
    REQUIRE(dumped.find("07:32:00") != rainy::core::text::string::npos);
    REQUIRE(dumped.find("1979-05-27") != rainy::core::text::string::npos);

    toml::facade<document> again = toml::parse(dumped);
    REQUIRE(again == doc);
    REQUIRE(again["a"].as_datetime().kind == datetime_kind::offset_date_time);
    REQUIRE(again["b"].as_datetime().kind == datetime_kind::local_date_time);
    REQUIRE(again["c"].as_datetime().kind == datetime_kind::local_time);
    REQUIRE(again["d"].as_datetime().kind == datetime_kind::local_date);
}

TEST_CASE("toml datetime - a datetime is not quoted on output", "[willow][toml]") {
    toml::facade<document> doc = toml::parse("d = 1979-05-27\n");
    const auto dumped = toml::dump(doc);
    REQUIRE(dumped.find("\"1979-05-27\"") == rainy::core::text::string::npos);
    REQUIRE(dumped.find("d = 1979-05-27") != rainy::core::text::string::npos);
}

TEST_CASE("toml datetime - two different datetimes are not equal", "[willow][toml]") {
    toml::facade<document> left = toml::parse("d = 1979-05-27\n");
    toml::facade<document> right = toml::parse("d = 1979-05-28\n");
    REQUIRE(left != right);
}
