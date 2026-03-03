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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_CHARCONV_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_CHARCONV_HPP
#include <rainy/core/implements/bit.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/decay.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_list.hpp>
#include <system_error>

namespace rainy::foundation::text {
    struct to_chars_result {
        char *ptr;
        std::errc ec;

        friend bool operator==(const to_chars_result &, const to_chars_result &) = default;

        constexpr explicit operator bool() const noexcept {
            return ec == std::errc{};
        }
    };

    struct from_chars_result {
        const char *ptr;
        std::errc ec;

        friend bool operator==(const from_chars_result &, const from_chars_result &) = default;

        constexpr explicit operator bool() const noexcept {
            return ec == std::errc{};
        }
    };
}

namespace rainy::foundation::text::implements {
    template <typename T, typename UTy>
    bool mul_overflow_helper(T a, T b, UTy &result) {
        if (a == 0 || b == 0) {
            result = 0;
            return false;
        }
        if constexpr (type_traits::type_properties::is_signed_v<T>) {
            if (a > 0 && b > 0) {
                if (a > (utility::numeric_limits<T>::max)() / b) {
                    return true;
                }
            } else if (a < 0 && b < 0) {
                if (a < (utility::numeric_limits<T>::max)() / b) {
                    return true;
                }
            } else if (a > 0 && b < 0) {
                if (b < (utility::numeric_limits<T>::min)() / a) {
                    return true;
                }
            } else if (a < 0 && b > 0) {
                if (a < (utility::numeric_limits<T>::min)() / b) {
                    return true;
                }
            }
        } else {
            if (a > (utility::numeric_limits<T>::max)() / b) {
                return true;
            }
        }
        result = a * b;
        return false;
    }

    template <typename Ty>
    struct to_chars_unsigned_type {
        using uints =
            type_traits::other_trans::type_list<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long
#ifdef __SIZEOF_INT128__
                                                ,
                                                unsigned __int128
#endif
#if RAINY_USING_MSVC
                                                ,
                                                unsigned __int64
#endif
                                                >;

        using type = typename type_traits::other_trans::select_type<sizeof(Ty), uints>::type; // NOLINT
    };

    template <typename Ty>
    using unsigned_least_t = typename to_chars_unsigned_type<Ty>::type; // NOLINT

    template <typename Ty>
    constexpr unsigned to_chars_len(Ty value, const int base = 10) noexcept {
        unsigned n = 1;
        const unsigned base_pow2 = base * base;
        const unsigned base_pow3 = base_pow2 * base;
        const unsigned long base_pow4 = base_pow3 * base;
        rain_loop {
            if (value < static_cast<unsigned>(base)) {
                return n;
            }
            if (value < base_pow2) {
                return n + 1;
            }
            if (value < base_pow3) {
                return n + 2;
            }
            if (value < base_pow4) {
                return n + 3;
            }
            value /= static_cast<Ty>(base_pow4);
            n += 4;
        }
    }

    template <typename Ty>
    constexpr unsigned to_chars_len_2(Ty value) noexcept {
        return core::builtin::bit_width(value);
    }

    template <typename Ty>
    RAINY_CONSTEXPR23 void to_chars_10_impl(char *begin, const unsigned int length, Ty value) noexcept {
        constexpr char digits[201] = "0001020304050607080910111213141516171819"
                                     "2021222324252627282930313233343536373839"
                                     "4041424344454647484950515253545556575859"
                                     "6061626364656667686970717273747576777879"
                                     "8081828384858687888990919293949596979899";
        unsigned position = length - 1;
        while (value >= 100) {
            auto const number = (value % 100) * 2;
            value /= 100;
            begin[position] = digits[number + 1];
            begin[position - 1] = digits[number];
            position -= 2;
        }
        if (value >= 10) {
            auto const number = value * 2;
            begin[1] = digits[number + 1];
            begin[0] = digits[number];
        } else {
            begin[0] = static_cast<char>('0' + value);
        }
    }

    template <typename Ty>
    constexpr to_chars_result to_chars(char *begin, char *end, Ty value, int base) noexcept {
        to_chars_result result{};
        const unsigned length = implements::to_chars_len(value, base);
        if (rainy_likely((end - begin) < length)) {
            result.ptr = end;
            result.ec = std::errc::value_too_large;
            return result;
        }
        constexpr char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                   'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
        unsigned position = length - 1;
        while (value >= static_cast<unsigned>(base)) {
            auto const quo = value / base;
            auto const rem = value % base;
            begin[position--] = digits[rem];
            value = static_cast<Ty>(quo);
        }
        *begin = digits[value];
        result.ptr = begin + length;
        result.ec = {};
        return result;
    }

    template <typename Ty>
    constexpr to_chars_result to_chars_16(char *begin, char *end, Ty value) noexcept {
        to_chars_result result{};
        const unsigned length = (to_chars_len_2(value) + 3) / 4;
        if (rainy_likely((end - begin) < length)) {
            result.ptr = end;
            result.ec = std::errc::value_too_large;
            return result;
        }
        constexpr char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        unsigned position = length - 1;
        while (value >= 0x100) {
            auto number = value & 0xF;
            value >>= 4;
            begin[position] = digits[number];
            number = value & 0xF;
            value >>= 4;
            begin[position - 1] = digits[number];
            position -= 2;
        }
        if (value >= 0x10) {
            const auto number = value & 0xF;
            value >>= 4;
            begin[1] = digits[number];
            begin[0] = digits[value];
        } else {
            begin[0] = digits[value];
        }
        result.ptr = begin + length;
        result.ec = {};
        return result;
    }

    template <typename Ty>
    constexpr to_chars_result to_chars_10(char *begin, char *end, Ty value) noexcept {
        to_chars_result result{};
        const unsigned length = to_chars_len(value, 10);
        if (rainy_likely((end - begin) < length)) {
            result.ptr = end;
            result.ec = std::errc::value_too_large;
            return result;
        }
        implements::to_chars_10_impl(begin, length, value);
        result.ptr = begin + length;
        result.ec = {};
        return result;
    }

    template <typename Ty>
    constexpr to_chars_result to_chars_8(char *begin, char *end, Ty value) noexcept {
        to_chars_result result{};
        unsigned length = 0;
        if constexpr (utility::numeric_limits<Ty>::digits <= 16) {
            length = value > 077777u ? 6u : value > 07777u ? 5u : value > 0777u ? 4u : value > 077u ? 3u : value > 07u ? 2u : 1u;
        } else {
            length = (to_chars_len_2(value) + 2) / 3;
        }
        if (rainy_likely((end - begin) < length)) {
            result.ptr = end;
            result.ec = std::errc::value_too_large;
            return result;
        }
        unsigned int position = length - 1;
        while (value >= 0100) {
            auto number = value & 7;
            value >>= 3;
            begin[position] = static_cast<char>('0' + number);
            number = value & 7;
            value >>= 3;
            begin[position - 1] = static_cast<char>('0' + number);
            position -= 2;
        }
        if (value >= 010) {
            auto const number = value & 7;
            value >>= 3;
            begin[1] = static_cast<char>('0' + number);
            begin[0] = static_cast<char>('0' + value);
        } else {
            begin[0] = static_cast<char>('0' + value);
        }
        result.ptr = begin + length;
        result.ec = {};
        return result;
    }

    template <typename Ty>
    constexpr to_chars_result to_chars_2(char *begin, char *end, Ty value) noexcept {
        to_chars_result result{};
        const unsigned length = to_chars_len_2(value);
        if (rainy_likely((end - begin) < length)) {
            result.ptr = end;
            result.ec = std::errc::value_too_large;
            return result;
        }
        unsigned position = length - 1;
        while (position) {
            begin[position--] = '0' + (value & 1);
            value >>= 1;
        }
        begin[0] = '1';
        result.ptr = begin + length;
        result.ec = {};
        return result;
    }

    template <typename Ty>
    constexpr bool raise_and_add(Ty &value, int base, unsigned char ch) {
        const Ty max_val = (utility::numeric_limits<Ty>::max)();

        // 检查乘法是否会溢出
        if (base != 0 && value > max_val / base) {
            return false;
        }

        Ty new_value = value * base;

        // 检查加法是否会溢出
        if (ch != 0 && new_value > max_val - ch) {
            return false;
        }

        value = new_value + ch;
        return true;
    }

    template <bool DecOnly>
    struct from_chars_alnum_to_val_table {
        struct type {
            unsigned char data[1u << CHAR_BIT] = {};
        };

        static constexpr type make_table() {
            // NOLINTBEGIN
            constexpr unsigned char lower_letters[27] = "abcdefghijklmnopqrstuvwxyz";
            constexpr unsigned char upper_letters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            // NOLINTEND
            type table{};
            for (auto &entry: table.data) {
                entry = 127;
            }
            for (int i = 0; i < 10; ++i) {
                table.data['0' + i] = i;
            }
            for (int i = 0; i < 26; ++i) {
                table.data[lower_letters[i]] = 10 + i;
                table.data[upper_letters[i]] = 10 + i;
            }
            return table;
        }

        static constexpr type value = (DecOnly, make_table());
    };

    template <bool DecOnly = false>
    RAINY_CONSTEXPR20 unsigned char from_chars_alnum_to_val(unsigned char ch) {
        if constexpr (DecOnly) {
            return static_cast<unsigned char>(ch - '0');
        } else {
            return from_chars_alnum_to_val_table<DecOnly>::value.data[ch];
        }
    }

   template <bool DecOnly, typename Ty>
    RAINY_CONSTEXPR23 bool from_chars_pow2_base(const char *&begin, const char *end, Ty &value, const int base) {
        static_assert(type_traits::primary_types::is_integral<Ty>::value, "implementation bug");
        static_assert(type_traits::type_properties::is_unsigned<Ty>::value, "implementation bug");
        value = 0;
        const int log2_base = core::builtin::countr_zero(static_cast<unsigned int>(base & 0x3f));
        const ptrdiff_t length = end - begin;
        ptrdiff_t i = 0;
        while (i < length && begin[i] == '0') {
            ++i;
        }
        const ptrdiff_t leading_zeros = i;
        if (rainy_unlikely(i >= length)) {
            begin += i;
            return true;
        }
        unsigned char leading_char = 0;
        if (base != 2) {
            leading_char = from_chars_alnum_to_val<DecOnly>(begin[i]);
            if (rainy_unlikely(leading_char >= base)) {
                begin += i;
                return true;
            }
            value = leading_char;
            ++i;
        }
        for (; i < length; ++i) {
            const unsigned char ch = from_chars_alnum_to_val<DecOnly>(begin[i]);
            if (ch >= base) {
                break;
            }
            value = (value << log2_base) | ch;
        }
        begin += i;
        auto significant_bits = (i - leading_zeros) * log2_base;
        if (base != 2 && leading_char > 0) {
            significant_bits -= log2_base - core::builtin::bit_width(leading_char);
        }
        return significant_bits <= utility::numeric_limits<Ty>::digits;
    }

    template <bool DecOnly, typename Ty>
    constexpr bool from_chars_alnum(const char *&begin, const char *end, Ty &value, int base) {
        assert(DecOnly ? base <= 10 : base <= 36);
        value = 0;
        const int bits_per_digit = core::builtin::bit_width(static_cast<unsigned int>(base & 0x3f));
        int unused_bits_lower_bound = utility::numeric_limits<Ty>::digits;
        for (; begin != end; ++begin) {
            const unsigned char ch = from_chars_alnum_to_val<DecOnly>(*begin);
            if (ch >= base) {
                return true;
            }
            unused_bits_lower_bound -= bits_per_digit;
            if (rainy_likely(unused_bits_lower_bound >= 0)) {
                // We're definitely not going to overflow
                value = value * base + ch;
            } else if (rainy_unlikely(!raise_and_add(value, base, ch))) {
                while (++begin != end && from_chars_alnum_to_val<DecOnly>(*begin) < base) {
                }
                return false;
            }
        }
        return true;
    }
}

namespace rainy::foundation::text::implements {
    template <typename Ty>
    constexpr to_chars_result to_chars_impl(char *begin, char *end, Ty value, int base = 10) {
        assert(2 <= base && base <= 36);
        using unsigned_type = unsigned_least_t<Ty>;
        unsigned_type val = value;
        if (rainy_unlikely(begin == end)) {
            return {end, std::errc::value_too_large};
        }
        // NOLINTBEGIN
        if (value == 0) {
            *begin = '0';
            return {begin + 1, std::errc{}};
        } else if constexpr (std::is_signed_v<Ty>) {
            if (value < 0) {
                *begin++ = '-';
                val = unsigned_type(~value) + unsigned_type(1);
            }
        }
        // NOLINTEND
        switch (base) {
            case 16: {
                return implements::to_chars_16(begin, end, val);
            }
            case 10: {
                return implements::to_chars_10(begin, end, val);
            }
            case 8: {
                return implements::to_chars_8(begin, end, val);
            }
            case 2: {
                return implements::to_chars_2(begin, end, val);
            }
            default: {
                return implements::to_chars(begin, end, val, base);
            }
        }
    }
}

namespace rainy::foundation::text {
#define RAINY_GENERATE_FUN_STUB_TO_CHARS(T)                                                                                           \
    RAINY_CONSTEXPR23 inline to_chars_result to_chars(char *begin, char *end, T value, int base = 10) {                               \
        return rainy::foundation::text::implements::to_chars_impl<T>(begin, end, value, base);                                        \
    }

    RAINY_GENERATE_FUN_STUB_TO_CHARS(char)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(signed char)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(unsigned char)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(signed short)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(unsigned short)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(signed int)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(unsigned int)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(signed long)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(unsigned long)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(signed long long)
    RAINY_GENERATE_FUN_STUB_TO_CHARS(unsigned long long)
#undef RAINY_GENERATE_FUN_STUB_TO_CHARS

    to_chars_result to_chars(char *, char *, bool, int = 10) = delete;

    template <typename Ty>
    RAINY_CONSTEXPR23 from_chars_result from_chars(const char *begin, const char *end, Ty &value, int base = 10) {
        assert(2 <= base && base <= 36);
        from_chars_result result{begin, {}};
        int sign = 1;
        if constexpr (std::is_signed_v<Ty>) {
            if (begin != end && *begin == '-') {
                sign = -1;
                ++begin;
            }
        }
        using unsigned_type = implements::unsigned_least_t<Ty>;
        unsigned_type val = 0;
        const auto start = begin;
        bool valid;
        if ((base & (base - 1)) == 0) {
            if (base <= 8) {
                valid = implements::from_chars_pow2_base<true>(begin, end, val, base);
            } else {
                valid = implements::from_chars_pow2_base<false>(begin, end, val, base);
            }
        } else if (base <= 10) {
            valid = implements::from_chars_alnum<true>(begin, end, val, base);
        } else {
            valid = implements::from_chars_alnum<false>(begin, end, val, base);
        }
        if (rainy_likely(begin == start)) {
            result.ec = std::errc::invalid_argument; // No digits processed
        } else {
            result.ptr = begin;
            if (!valid) {
                result.ec = std::errc::result_out_of_range; // Overflow
            } else {
                if constexpr (std::is_signed_v<Ty>) {
                    // 对于有符号类型，需要特殊处理范围检查
                    // min 的绝对值可能比 max 大 1 (例如 int: min=-2147483648, max=2147483647)
                    constexpr unsigned_type max_positive = static_cast<unsigned_type>((utility::numeric_limits<Ty>::max)());
                    constexpr unsigned_type max_negative =
                        static_cast<unsigned_type>(-static_cast<unsigned_type>((utility::numeric_limits<Ty>::min)()));

                    if (sign == -1) {
                        // 负数
                        if (val > max_negative) {
                            result.ec = std::errc::result_out_of_range;
                        } else if (val == max_negative) {
                            // 特殊情况：val 等于 INT_MIN 的绝对值
                            // 使用这个技巧来安全地表示最小值
                            value = (utility::numeric_limits<Ty>::min)();
                        } else {
                            // 普通负数，可以安全转换
                            value = -static_cast<Ty>(val);
                        }
                    } else {
                        // 正数
                        if (val > max_positive) {
                            result.ec = std::errc::result_out_of_range;
                        } else {
                            value = static_cast<Ty>(val);
                        }
                    }
                } else {
                    // 无符号类型
                    if constexpr ((utility::numeric_limits<unsigned_type>::max)() > (utility::numeric_limits<Ty>::max)()) {
                        if (val > static_cast<unsigned_type>((utility::numeric_limits<Ty>::max)())) {
                            result.ec = std::errc::result_out_of_range;
                        } else {
                            value = static_cast<Ty>(val);
                        }
                    } else {
                        value = static_cast<Ty>(val);
                    }
                }
            }
        }
        return result;
    }

    enum class chars_format {
        scientific = 1,
        fixed = 2,
        hex = 4,
        general = fixed | scientific
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(chars_format);

    RAINY_TOOLKIT_API from_chars_result from_chars(const char *begin, const char *end, float &value,
                                                   chars_format fmt = chars_format::general) noexcept;

    RAINY_TOOLKIT_API from_chars_result from_chars(const char *begin, const char *end, double &value,
                                                   chars_format fmt = chars_format::general) noexcept;

    RAINY_TOOLKIT_API from_chars_result from_chars(const char *begin, const char *end, long double &value,
                                                   chars_format fmt = chars_format::general) noexcept;
}

namespace rainy::foundation::text {
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, float value) noexcept;
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, float value, chars_format fmt) noexcept;
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, float value, chars_format fmt, int precision) noexcept;

    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, double value) noexcept;
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, double value, chars_format fmt) noexcept;
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, double value, chars_format fmt, int precision) noexcept;

    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, long double value) noexcept;
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, long double value, chars_format fmt) noexcept;
    RAINY_TOOLKIT_API to_chars_result to_chars(char *begin, char *end, long double value, chars_format fmt, int precision) noexcept;
}

#endif
