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
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>

namespace rainy::core::text::implements {
    template <typename Floating>
    inline Floating power_of_10(int exp) noexcept {
        if (exp == 0) {
            return static_cast<Floating>(1.0);
        }
        constexpr int max_exp = std::numeric_limits<Floating>::max_exponent10;
        bool negative = exp < 0;
        int abs_exp = negative ? -exp : exp;
        // 正指数超出范围 → 返回 inf（正指数）或 0（负指数）
        if (abs_exp > max_exp) {
            return negative ? static_cast<Floating>(0.0) : std::numeric_limits<Floating>::infinity();
        }
        Floating result = static_cast<Floating>(1.0);
        Floating base = static_cast<Floating>(10.0);
        for (int i = 0; i < abs_exp; ++i) {
            result *= base;
        }
        return negative ? (static_cast<Floating>(1.0) / result) : result;
    }

    template <typename Floating>
    inline constexpr int exact_pow10_limit() noexcept {
        constexpr int digits = std::numeric_limits<Floating>::digits;
        constexpr std::uint64_t pow2_digits = digits >= 64 ? (std::numeric_limits<std::uint64_t>::max)()
                                                          : (std::uint64_t(1) << digits);
        int limit = 0;
        std::uint64_t power_of_five = 1;
        while (power_of_five <= (std::numeric_limits<std::uint64_t>::max)() / 5) {
            const std::uint64_t next_power_of_five = power_of_five * 5;
            if (next_power_of_five >= pow2_digits) {
                break;
            }
            power_of_five = next_power_of_five;
            ++limit;
        }
        return limit;
    }

    template <typename Floating>
    inline constexpr std::uint64_t exact_significand_limit() noexcept {
        constexpr int digits = std::numeric_limits<Floating>::digits;
        return digits >= 64 ? (std::numeric_limits<std::uint64_t>::max)() : ((std::uint64_t(1) << digits) - 1);
    }

    template <typename Floating>
    Floating refine_by_shortest_roundtrip(const char *begin, const char *end, Floating candidate) noexcept {
        const auto matches = [begin, end](const Floating value) {
            char buffer[64];
            const auto result = text::to_chars(buffer, buffer + sizeof(buffer), value);
            if (result.ec != std::errc{} || result.ptr - buffer != end - begin) {
                return false;
            }
            for (const char *cursor = buffer, *input = begin; cursor != result.ptr; ++cursor, ++input) {
                if (*cursor != *input) {
                    return false;
                }
            }
            return true;
        };
        if (matches(candidate)) {
            return candidate;
        }
        constexpr int max_probes = 4;
        for (int direction = 0; direction < 2; ++direction) {
            const Floating bound = direction == 0 ? -std::numeric_limits<Floating>::infinity()
                                                  : std::numeric_limits<Floating>::infinity();
            Floating probe = candidate;
            for (int i = 0; i < max_probes; ++i) {
                probe = std::nextafter(probe, bound);
                if (matches(probe)) {
                    return probe;
                }
            }
        }
        return candidate;
    }

    template <typename Floating>
    inline Floating power_of_2(int exp) noexcept {
        if (exp == 0) {
            return static_cast<Floating>(1.0);
        }
        constexpr int max_exp = std::numeric_limits<Floating>::max_exponent;
        bool negative = exp < 0;
        int abs_exp = negative ? -exp : exp;
        if (abs_exp > max_exp) {
            return negative ? static_cast<Floating>(0.0) : std::numeric_limits<Floating>::infinity();
        }
        Floating result = static_cast<Floating>(1.0);
        Floating base = static_cast<Floating>(2.0);
        for (int i = 0; i < abs_exp; ++i) {
            result *= base;
        }
        return negative ? (static_cast<Floating>(1.0) / result) : result;
    }

    template <typename Floating>
    inline bool is_finite(Floating x) noexcept {
        return x == x && x != std::numeric_limits<Floating>::infinity() && x != -std::numeric_limits<Floating>::infinity();
    }

    template <typename Floating>
    RAINY_NODISCARD RAINY_INLINE from_chars_result ordinary_floating_from_chars(const char *const begin, const char *const end, Floating &value,
                                                                   const chars_format fmt, const bool minus_sign, const char *next) {
        Floating integer_part = static_cast<Floating>(0.0);
        Floating fractional_part = static_cast<Floating>(0.0);
        int fraction_digits = 0;
        bool has_digits = false;
        bool has_dot = false;
        bool has_exponent = false;
        // 十六进制格式处理
        if (fmt == chars_format::hex) {
            // 检查并跳过 "0x" 或 "0X" 前缀
            if (next + 1 < end && *next == '0' && (next[1] == 'x' || next[1] == 'X')) {
                next += 2;
            }
            if (next == end) {
                return {begin, std::errc::invalid_argument};
            }
            // 解析十六进制整数部分
            while (next < end) {
                char ch = *next;
                int digit = -1;
                if (ch >= '0' && ch <= '9') {
                    digit = ch - '0';
                } else if (ch >= 'a' && ch <= 'f') {
                    digit = ch - 'a' + 10;
                } else if (ch >= 'A' && ch <= 'F') {
                    digit = ch - 'A' + 10;
                } else if (ch == '.') {
                    has_dot = true;
                    ++next;
                    break;
                } else {
                    break;
                }
                if (digit >= 0) {
                    integer_part = integer_part * static_cast<Floating>(16.0) + static_cast<Floating>(digit);
                    has_digits = true;
                    ++next;
                }
            }
            // 解析十六进制小数部分
            if (has_dot) {
                while (next < end) {
                    char ch = *next;
                    int digit = -1;

                    if (ch >= '0' && ch <= '9') {
                        digit = ch - '0';
                    } else if (ch >= 'a' && ch <= 'f') {
                        digit = ch - 'a' + 10;
                    } else if (ch >= 'A' && ch <= 'F') {
                        digit = ch - 'A' + 10;
                    } else {
                        break;
                    }

                    if (digit >= 0) {
                        fractional_part = fractional_part * static_cast<Floating>(16.0) + static_cast<Floating>(digit);
                        fraction_digits++;
                        has_digits = true;
                        ++next;
                    } else {
                        break;
                    }
                }
            }

            if (!has_digits) {
                return {begin, std::errc::invalid_argument};
            }

            // 合并整数部分和小数部分
            Floating result = integer_part;
            if (fraction_digits > 0) {
                Floating divisor = static_cast<Floating>(1.0);
                for (int i = 0; i < fraction_digits; ++i) {
                    divisor *= static_cast<Floating>(16.0);
                }
                result += fractional_part / divisor;
            }

            // 解析二进制指数 (p 或 P)
            if (next < end && (*next == 'p' || *next == 'P')) {
                has_exponent = true;
                ++next;

                bool exp_negative = false;
                if (next < end && *next == '-') {
                    exp_negative = true;
                    ++next;
                } else if (next < end && *next == '+') {
                    ++next;
                }
                if (next >= end || !(*next >= '0' && *next <= '9')) {
                    return {begin, std::errc::invalid_argument};
                }
                int binary_exp = 0;
                while (next < end && *next >= '0' && *next <= '9') {
                    int digit = *next - '0';
                    if (binary_exp > (INT_MAX - digit) / 10) {
                        return {begin, std::errc::result_out_of_range};
                    }
                    binary_exp = binary_exp * 10 + digit;
                    ++next;
                }
                if (exp_negative) {
                    binary_exp = -binary_exp;
                }
                if (binary_exp != 0) {
                    result *= static_cast<Floating>(std::pow(2.0, static_cast<double>(binary_exp)));
                    if (!is_finite(result)) {
                        return {next, std::errc::result_out_of_range};
                    }
                }
            }
            value = minus_sign ? -result : result;
            return {next, std::errc{}};
        }
        // 十进制格式处理
        std::uint64_t significand = 0;
        bool significand_overflow = false;
        const auto append_significand = [&](const int digit) {
            if (significand_overflow) {
                return;
            }
            if (significand > ((std::numeric_limits<std::uint64_t>::max)() - static_cast<std::uint64_t>(digit)) / 10) {
                significand_overflow = true;
                return;
            }
            significand = significand * 10 + static_cast<std::uint64_t>(digit);
        };
        while (next < end && *next >= '0' && *next <= '9') {
            int digit = *next - '0';
            integer_part = integer_part * static_cast<Floating>(10.0) + static_cast<Floating>(digit);
            append_significand(digit);
            has_digits = true;
            ++next;
        }
        // 解析小数点和小数部分
        if (next < end && *next == '.') {
            has_dot = true;
            ++next;

            while (next < end && *next >= '0' && *next <= '9') {
                int digit = *next - '0';
                fractional_part = fractional_part * static_cast<Floating>(10.0) + static_cast<Floating>(digit);
                append_significand(digit);
                fraction_digits++;
                has_digits = true;
                ++next;
            }
        }
        if (!has_digits) {
            return {begin, std::errc::invalid_argument};
        }
        // 解析指数部分
        int exp_value = 0;
        if (next < end && (*next == 'e' || *next == 'E') && fmt != chars_format::fixed) {
            has_exponent = true;
            ++next;
            bool exp_negative = false;
            if (next < end && *next == '-') {
                exp_negative = true;
                ++next;
            } else if (next < end && *next == '+') {
                ++next;
            }

            if (next >= end || !(*next >= '0' && *next <= '9')) {
                return {begin, std::errc::invalid_argument};
            }

            while (next < end && *next >= '0' && *next <= '9') {
                int digit = *next - '0';
                if (exp_value > (INT_MAX - digit) / 10) {
                    return {begin, std::errc::result_out_of_range};
                }
                exp_value = exp_value * 10 + digit;
                ++next;
            }

            if (exp_negative) {
                exp_value = -exp_value;
            }
        }
        // scientific 格式必须有指数
        if (fmt == chars_format::scientific && !has_exponent) {
            return {begin, std::errc::invalid_argument};
        }
        const int total_exponent = exp_value - fraction_digits;
        if (!significand_overflow && significand <= exact_significand_limit<Floating>() &&
            total_exponent >= -exact_pow10_limit<Floating>() && total_exponent <= exact_pow10_limit<Floating>()) {
            const Floating scale = power_of_10<Floating>(total_exponent >= 0 ? total_exponent : -total_exponent);
            const Floating mantissa = static_cast<Floating>(significand);
            const Floating fast_result = total_exponent >= 0 ? mantissa * scale : mantissa / scale;
            value = minus_sign ? -fast_result : fast_result;
            return {next, std::errc{}};
        }
        Floating result = integer_part;
        if (fraction_digits > 0) {
            Floating divisor = power_of_10<Floating>(fraction_digits);
            result += fractional_part / divisor;
        }
        if (exp_value != 0) {
            const Floating power = static_cast<Floating>(std::pow(10.0, static_cast<double>(exp_value)));
            if (exp_value < 0 && power == static_cast<Floating>(0.0) && result != static_cast<Floating>(0.0)) {
                int remaining = exp_value;
                while (remaining <= -20) {
                    result *= power_of_10<Floating>(-20);
                    remaining += 20;
                }
                if (remaining != 0) {
                    result *= static_cast<Floating>(std::pow(10.0, static_cast<double>(remaining)));
                }
            } else {
                result *= power;
            }
            if (!is_finite(result)) {
                return {next, std::errc::result_out_of_range};
            }
        }
        value = refine_by_shortest_roundtrip(begin, next, minus_sign ? -result : result);
        return {next, std::errc{}};
    }

    template <typename Floating>
    RAINY_NODISCARD RAINY_INLINE from_chars_result infinity_from_chars(const char *const begin, const char *const end, Floating &value,
                                                          const bool minus_sign, const char *next) noexcept {

        // 检查 "inf" 或 "infinity"
        const char inf_long[] = {'i', 'n', 'f', 'i', 'n', 'i', 't', 'y'};

        std::size_t matched = 0;
        const char *parse_pos = next;

        // 先尝试匹配 "infinity"
        while (parse_pos < end && matched < 8) {
            unsigned char ch = static_cast<unsigned char>(*parse_pos) | 0x20; // 转小写
            if (ch != inf_long[matched]) {
                break;
            }
            ++matched;
            ++parse_pos;
        }
        // 如果完整匹配了 "infinity"
        if (matched == 8) {
            value = minus_sign ? -utility::numeric_limits<Floating>::infinity() : utility::numeric_limits<Floating>::infinity();
            return {parse_pos, std::errc{}};
        }
        // 如果至少匹配了 "inf"
        if (matched >= 3) {
            value = minus_sign ? -utility::numeric_limits<Floating>::infinity() : utility::numeric_limits<Floating>::infinity();
            return {next + 3, std::errc{}};
        }
        return {begin, std::errc::invalid_argument};
    }

    // 处理 NaN 的转换
    template <typename Floating>
    RAINY_NODISCARD RAINY_INLINE from_chars_result nan_from_chars(const char *const begin, const char *const end, Floating &value,
                                                                  bool minus_sign,
                                                     const char *next) {

        // 检查 "nan" 或 "nan(...)"
        const char nan_str[] = {'n', 'a', 'n'};
        std::size_t matched = 0;
        const char *parse_pos = next;
        // 匹配 "nan"
        while (parse_pos < end && matched < 3) {
            unsigned char ch = static_cast<unsigned char>(*parse_pos) | 0x20; // 转小写
            if (ch != nan_str[matched]) {
                return {begin, std::errc::invalid_argument};
            }
            ++matched;
            ++parse_pos;
        }
        if (matched != 3) {
            return {begin, std::errc::invalid_argument};
        }
        // 检查可选的 "(n-char-sequence)"
        if (parse_pos < end && *parse_pos == '(') {
            ++parse_pos;
            // 跳过括号内的内容（字母数字和下划线）
            while (parse_pos < end) {
                char ch = *parse_pos;
                if (ch == ')') {
                    ++parse_pos;
                    break;
                } else if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
                    ++parse_pos;
                } else {
                    // 无效字符，括号未闭合
                    return {begin, std::errc::invalid_argument};
                }
            }
        }
        // NaN 不受符号影响
        value = utility::numeric_limits<Floating>::quiet_NaN();
        if (minus_sign) {
            value = -value;
        }
        return {parse_pos, std::errc{}};
    }
}

namespace rainy::core::text::implements {
    template <typename Floating>
    RAINY_NODISCARD from_chars_result floating_from_chars(const char *const begin, const char *const last, Floating &value,
                                                          const chars_format fmt) noexcept {
        assert((fmt == chars_format::general || fmt == chars_format::scientific || fmt == chars_format::fixed ||
                fmt == chars_format::hex) &&
               "invalid format in from_chars()");
        bool minus_sign = false;
        const char *next = begin;
        if (next == last) {
            return {begin, std::errc::invalid_argument};
        }
        if (*next == '-') {
            minus_sign = true;
            ++next;
            if (next == last) {
                return {begin, std::errc::invalid_argument};
            }
        }
        const unsigned char folded_start = static_cast<unsigned char>(static_cast<unsigned char>(*next) | 0x20);
        if (folded_start <= 'f') {
            return implements::ordinary_floating_from_chars(begin, last, value, fmt, minus_sign, next);
        } else if (folded_start == 'i') {
            return implements::infinity_from_chars(begin, last, value, minus_sign, next);
        } else if (folded_start == 'n') {
            return implements::nan_from_chars(begin, last, value, minus_sign, next);
        } else {
            return {begin, std::errc::invalid_argument};
        }
    }
}

namespace rainy::core::text {
    from_chars_result from_chars(const char *const begin, const char *const end, float &value, const chars_format fmt) noexcept {
        return implements::floating_from_chars(begin, end, value, fmt);
    }

    from_chars_result from_chars(const char *const begin, const char *const end, double &value, const chars_format fmt) noexcept {
        return implements::floating_from_chars(begin, end, value, fmt);
    }

    from_chars_result from_chars(const char *const begin, const char *const end, long double &value, const chars_format fmt) noexcept {
        double doub_var;
        const from_chars_result result = implements::floating_from_chars(begin, end, doub_var, fmt);
        if (result.ec == std::errc{}) {
            value = doub_var;
        }
        return result;
    }
}
