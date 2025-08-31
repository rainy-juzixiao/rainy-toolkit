#ifndef RAINY_UTILITY_TIME_HPP
#define RAINY_UTILITY_TIME_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/pair.hpp>

namespace rainy::utility::time {
    enum class format {
        http,
        utc,
        utc_without_punctuation
    };

    enum component_of_time_format {
        day_name,
        day,
        month_name,
        month,
        year,
        hour,
        minute,
        second,
        second_decimal_part,
        sp,
        comma,
        colon,
        hyphen,
        dot,
        gmt,
        t,
        z,
        ending
    };

    constexpr collections::array<int, 17> month_table = {11, 4, -1, 7, -1, -1, -1, 0, 6, 3, 5, 2, 10, 8, -1, 9, 1};

    inline constexpr collections::array<int, 17> week_table = {2, 4, 3, 1, -1, -1, -1, 6, -1, -1, -1, -1, 0, -1, -1, 5, -1};

    inline constexpr collections::array<component_of_time_format, 32> http_time_format{
        component_of_time_format::day_name, component_of_time_format::comma,  component_of_time_format::sp,
        component_of_time_format::day,      component_of_time_format::sp,     component_of_time_format::month_name,
        component_of_time_format::sp,       component_of_time_format::year,   component_of_time_format::sp,
        component_of_time_format::hour,     component_of_time_format::colon,  component_of_time_format::minute,
        component_of_time_format::colon,    component_of_time_format::second, component_of_time_format::sp,
        component_of_time_format::gmt,      component_of_time_format::ending};

    inline constexpr collections::array<component_of_time_format, 32> utc_time_format{component_of_time_format::year,
                                                                              component_of_time_format::hyphen,
                                                                              component_of_time_format::month,
                                                                              component_of_time_format::hyphen,
                                                                              component_of_time_format::day,
                                                                              component_of_time_format::t,
                                                                              component_of_time_format::hour,
                                                                              component_of_time_format::colon,
                                                                              component_of_time_format::minute,
                                                                              component_of_time_format::colon,
                                                                              component_of_time_format::second,
                                                                              component_of_time_format::dot,
                                                                              component_of_time_format::second_decimal_part,
                                                                              component_of_time_format::z,
                                                                              component_of_time_format::ending};

    inline constexpr std::array<component_of_time_format, 32> utc_time_without_punctuation_format{
        component_of_time_format::year,   component_of_time_format::month,
        component_of_time_format::day,    component_of_time_format::t,
        component_of_time_format::hour,   component_of_time_format::minute,
        component_of_time_format::second, component_of_time_format::second_decimal_part,
        component_of_time_format::z,      component_of_time_format::ending};
    constexpr inline int len_of_http_time_format = 3 + 1 + 1 + 2 + 1 + 3 + 1 + 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 3;
    constexpr inline int len_of_utc_time_format = 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 0 + 1;
    constexpr inline int len_of_utc_time_without_punctuation_format = 4 + 2 + 2 + 1 + 2 + 2 + 2 + 0 + 1;
    constexpr inline std::int64_t absolute_zero_year = -292277022399;
    constexpr inline std::int64_t days_per_100_years = 365 * 100 + 24;
    constexpr inline std::int64_t days_per_400_years = 365 * 400 + 97;
    constexpr inline std::int64_t days_per_4_years = 365 * 4 + 1;
    constexpr inline std::int64_t seconds_per_minute = 60;
    constexpr inline std::int64_t seconds_per_hour = 60 * seconds_per_minute;
    constexpr inline std::int64_t seconds_per_day = 24 * seconds_per_hour;
    constexpr inline std::int64_t seconds_per_week = 7 * seconds_per_day;
    constexpr inline std::int64_t internal_year = 1;
    constexpr inline std::int64_t absolute_to_internal =
        (absolute_zero_year - internal_year) * std::int64_t(365.2425 * seconds_per_day);
    constexpr inline std::int64_t unix_to_internal = (1969 * 365 + 1969 / 4 - 1969 / 100 + 1969 / 400) * seconds_per_day;
    constexpr inline std::int64_t internal_to_unix = -unix_to_internal;
    constexpr inline collections::array<std::int32_t, 13> days_before = {
        0,
        31,
        31 + 28,
        31 + 28 + 31,
        31 + 28 + 31 + 30,
        31 + 28 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
    };

    constexpr bool is_leap(int year) noexcept {
        return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    }

    constexpr int get_day_index(std::string_view str) {
        return week_table[((str[0] & ~0x20) ^ (str[2] & ~0x20)) % week_table.size()];
    }

    constexpr int get_month_index(std::string_view str) {
        return month_table[((str[1] & ~0x20) + (str[2] & ~0x20)) % month_table.size()];
    }

    constexpr int days_in(int m, int year) {
        constexpr int index = get_month_index("Feb");
        if (m == index && is_leap(year)) {
            return 29;
        }
        return static_cast<int>(days_before[m + 1] - days_before[m]);
    }

    constexpr std::uint64_t days_since_epoch(int year) {
        auto y = static_cast<std::uint64_t>(std::int64_t(year) - absolute_zero_year);
        auto n = y / 400;
        y -= 400 * n;
        auto d = days_per_400_years * n;
        n = y / 100;
        y -= 100 * n;
        d += days_per_100_years * n;
        n = y / 4;
        y -= 4 * n;
        d += days_per_4_years * n;
        n = y;
        d += 365 * n;
        return d;
    }

    utility::pair<bool, std::time_t> make_time(int year, int month, int day, int hour, int min, int sec, int day_of_week) {
        auto d = days_since_epoch(year);
        d += std::uint64_t(days_before[month]);
        constexpr int index = get_month_index("Mar");
        if (is_leap(year) && month >= index) {
            d++; // February 29
        }
        d += std::uint64_t(day - 1);
        auto abs = d * seconds_per_day;
        abs += std::uint64_t(hour * seconds_per_hour + min * seconds_per_minute + sec);
        constexpr int day_index = get_day_index("Mon");
        if (day_of_week != -1) {
            std::int64_t wday = ((abs + std::uint64_t(day_index) * seconds_per_day) % seconds_per_week) / seconds_per_day;
            if (wday != day_of_week) {
                return {false, 0};
            }
        }
        return {true, static_cast<std::int64_t>(abs) + (absolute_to_internal + internal_to_unix)};
    }


}

#endif