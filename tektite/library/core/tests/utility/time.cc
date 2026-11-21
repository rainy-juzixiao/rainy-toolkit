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
#include <rainy/core/utility/time.hpp>

using namespace rainy::utility::time;

TEST_CASE("time leap year rule", "[time]") {
    STATIC_REQUIRE(is_leap(2000));
    STATIC_REQUIRE(is_leap(2024));
    STATIC_REQUIRE(is_leap(1996));
    STATIC_REQUIRE_FALSE(is_leap(1900));
    STATIC_REQUIRE_FALSE(is_leap(2023));
    STATIC_REQUIRE_FALSE(is_leap(2100));
}

TEST_CASE("time day and month name lookup", "[time]") {
    STATIC_REQUIRE(get_day_index("Sun") == 0);
    STATIC_REQUIRE(get_day_index("Mon") == 1);
    STATIC_REQUIRE(get_day_index("Fri") == 5);
    STATIC_REQUIRE(get_day_index("Sat") == 6);

    STATIC_REQUIRE(get_month_index("Jan") == 0);
    STATIC_REQUIRE(get_month_index("Feb") == 1);
    STATIC_REQUIRE(get_month_index("Mar") == 2);
    STATIC_REQUIRE(get_month_index("Aug") == 7);
    STATIC_REQUIRE(get_month_index("Dec") == 11);
}

TEST_CASE("time days_in honours leap years", "[time]") {
    constexpr int jan = get_month_index("Jan");
    constexpr int feb = get_month_index("Feb");
    constexpr int apr = get_month_index("Apr");

    REQUIRE(days_in(jan, 2024) == 31);
    REQUIRE(days_in(feb, 2024) == 29);
    REQUIRE(days_in(feb, 2023) == 28);
    REQUIRE(days_in(feb, 1900) == 28);
    REQUIRE(days_in(apr, 2024) == 30);
}

TEST_CASE("time make_time converts calendar dates to unix time", "[time]") {
    constexpr int jan = get_month_index("Jan");
    constexpr int feb = get_month_index("Feb");
    constexpr int nov = get_month_index("Nov");

    SECTION("unix epoch") {
        auto result = make_time(1970, jan, 1, 0, 0, 0, -1);
        REQUIRE(result.first);
        REQUIRE(result.second == 0);
    }

    SECTION("year 2000") {
        auto result = make_time(2000, jan, 1, 0, 0, 0, -1);
        REQUIRE(result.first);
        REQUIRE(result.second == 946684800LL);
    }

    SECTION("leap day 2024") {
        auto result = make_time(2024, feb, 29, 0, 0, 0, -1);
        REQUIRE(result.first);
        REQUIRE(result.second == 1709164800LL);
    }

    SECTION("friday in november 2024") {
        auto thursday = get_day_index("Thu");
        auto friday = get_day_index("Fri");
        auto result = make_time(2024, nov, 1, 0, 0, 0, friday);
        REQUIRE(result.first);
        REQUIRE(result.second == 1730419200LL);

        auto mismatched = make_time(2024, nov, 1, 0, 0, 0, thursday);
        REQUIRE_FALSE(mismatched.first);
        REQUIRE(mismatched.second == 0);
    }

    SECTION("time within the day is added") {
        auto midnight = make_time(2024, nov, 1, 0, 0, 0, -1);
        auto with_time = make_time(2024, nov, 1, 13, 45, 30, -1);
        REQUIRE(midnight.first);
        REQUIRE(with_time.first);
        REQUIRE(with_time.second - midnight.second == 13 * 3600 + 45 * 60 + 30);
    }
}
