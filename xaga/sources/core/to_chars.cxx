/*
 * Copyright 2018 Ulf Adams
 * Copyright 2026 rainy-juzixiao
 *
 * This file is derived from an implementation originally written by
 * Ulf Adams. The current version contains modifications and additional
 * development by rainy-juzixiao.
 *
 * ----------------------------------------------------------------------
 * Original Work License (Ulf Adams)
 * ----------------------------------------------------------------------
 *
 * The contents of the original implementation may be used under the
 * terms of either:
 *
 *   1. The Apache License, Version 2.0
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   or
 *
 *   2. The Boost Software License, Version 1.0
 *        https://www.boost.org/LICENSE_1_0.txt
 *
 * ----------------------------------------------------------------------
 * Modifications License (rainy-juzixiao)
 * ----------------------------------------------------------------------
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * ----------------------------------------------------------------------
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the applicable license for the specific language governing
 * permissions and limitations under the License.
 */
#include <rainy/core/core.hpp>
#include <rainy/core/implements/text/charconv.hpp>
#include <rainy/core/implements/text/implements/ryu_table.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4146 6385 6386 6385)
#endif

// 参考 d2s_intrinsics.h
// https://github.com/ulfjack/ryu/blob/master/ryu/d2s_intrinsics.h
namespace rainy::foundation::text::implements::ryu {
    RAINY_NODISCARD inline std::uint32_t decimal_length9(const std::uint32_t value) {
        core::implements::stl_internal_check(value < 1000000000);
        if (value >= 100000000) {
            return 9;
        }
        if (value >= 10000000) {
            return 8;
        }
        if (value >= 1000000) {
            return 7;
        }
        if (value >= 100000) {
            return 6;
        }
        if (value >= 10000) {
            return 5;
        }
        if (value >= 1000) {
            return 4;
        }
        if (value >= 100) {
            return 3;
        }
        if (value >= 10) {
            return 2;
        }
        return 1;
    }

    inline std::int32_t pow5_bits(const std::int32_t e) {
        core::implements::stl_internal_check(e >= 0);
        core::implements::stl_internal_check(e <= 3528);
        return static_cast<std::int32_t>(((static_cast<std::uint32_t>(e) * 1217359) >> 19) + 1);
    }

    RAINY_NODISCARD inline std::uint32_t log10_pow2(const std::int32_t e) {
        // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
        core::implements::stl_internal_check(e >= 0);
        core::implements::stl_internal_check(e <= 1650);
        return (static_cast<std::uint32_t>(e) * 78913) >> 18;
    }

    RAINY_NODISCARD inline std::uint32_t log10_pow5(const std::int32_t e) {
        core::implements::stl_internal_check(e >= 0);
        core::implements::stl_internal_check(e <= 2620);
        return (static_cast<std::uint32_t>(e) * 732923) >> 20;
    }

    RAINY_NODISCARD inline std::uint32_t float_to_bits(const float float_val) {
        std::uint32_t rightits = 0;
        std::memcpy(&rightits, &float_val, sizeof(float));
        return rightits;
    }

    RAINY_NODISCARD inline std::uint64_t double_to_bits(const double double_val) {
        std::uint64_t rightits = 0;
        std::memcpy(&rightits, &double_val, sizeof(double));
        return rightits;
    }

    inline constexpr int DOUBLE_MANTISSA_BITS = 52;
    inline constexpr int DOUBLE_BIAS = 1023;

    inline constexpr int DOUBLE_POW5_INV_BITCOUNT = 122;
    inline constexpr int DOUBLE_POW5_BITCOUNT = 121;

    RAINY_NODISCARD RAINY_ALWAYS_INLINE std::uint64_t umul128(const std::uint64_t left, const std::uint64_t right,
                                                              std::uint64_t *const producthi) {
        const std::uint32_t leftLo = static_cast<std::uint32_t>(left);
        const std::uint32_t leftHi = static_cast<std::uint32_t>(left >> 32);
        const std::uint32_t rightLo = static_cast<std::uint32_t>(right);
        const std::uint32_t rightHi = static_cast<std::uint32_t>(right >> 32);

        const std::uint64_t right_00 = static_cast<std::uint64_t>(leftLo) * rightLo;
        const std::uint64_t right_01 = static_cast<std::uint64_t>(leftLo) * rightHi;
        const std::uint64_t right_10 = static_cast<std::uint64_t>(leftHi) * rightLo;
        const std::uint64_t right_11 = static_cast<std::uint64_t>(leftHi) * rightHi;

        const std::uint32_t right_00lo = static_cast<std::uint32_t>(right_00);
        const std::uint32_t right_00hi = static_cast<std::uint32_t>(right_00 >> 32);

        const std::uint64_t middle_1 = right_10 + right_00hi;
        const std::uint32_t middle_1lo = static_cast<std::uint32_t>(middle_1);
        const std::uint32_t middle_1hi = static_cast<std::uint32_t>(middle_1 >> 32);

        const std::uint64_t middle2 = right_01 + middle_1lo;
        const std::uint32_t middle_2lo = static_cast<std::uint32_t>(middle2);
        const std::uint32_t middle_2hi = static_cast<std::uint32_t>(middle2 >> 32);

        const std::uint64_t product_hi = right_11 + middle_1hi + middle_2hi;
        const std::uint64_t product_lo = (static_cast<std::uint64_t>(middle_2lo) << 32) | right_00lo;

        *producthi = product_hi;
        return product_lo;
    }

    RAINY_NODISCARD inline std::uint64_t shiftright128(const std::uint64_t lo, const std::uint64_t high, const std::uint32_t dist) {
        core::implements::stl_internal_check(dist < 64);
        if (dist == 0) {
            return lo;
        }
        return (high << (64 - dist)) | (lo >> dist);
    }

#if RAINY_USING_32BIT_PLATFORM
    RAINY_NODISCARD inline std::uint64_t umulh_helper(const std::uint64_t left, const std::uint64_t right) {
        std::uint64_t high;
        (void) umul128(left, right, &high);
        return high;
    }

    RAINY_NODISCARD inline std::uint64_t div5(const std::uint64_t val) {
        return umulh_helper(val, 0xCCCCCCCCCCCCCCCDu) >> 2;
    }

    RAINY_NODISCARD inline std::uint64_t div10(const std::uint64_t val) {
        return umulh_helper(val, 0xCCCCCCCCCCCCCCCDu) >> 3;
    }

    RAINY_NODISCARD inline std::uint64_t div100(const std::uint64_t val) {
        return umulh_helper(val >> 2, 0x28F5C28F5C28F5C3u) >> 2;
    }

    RAINY_NODISCARD inline std::uint64_t div1e8(const std::uint64_t val) {
        return umulh_helper(val, 0xABCC77118461CEFDu) >> 26;
    }

    RAINY_NODISCARD inline std::uint64_t div1e9(const std::uint64_t val) {
        return umulh_helper(val >> 9, 0x44B82FA09B5A53u) >> 11;
    }

    RAINY_NODISCARD inline std::uint32_t mod1e9(const std::uint64_t val) {
        /*
         * 尽量避免 64 位运算，如果直接写 return static_cast<std::uint32_t>(val - 1000000000 * div1e9(val))
         * 会触发 32x64 位乘法和 64 位减法，效率低
         * val 和 1000000000 * div1e9(val) 的差值保证小于 10^9
         * 因此它们的高 32 位一定相同
         * 所以在减法前可以直接截断成 std::uint32_t，只处理低 32 位即可
         * 同理，static_cast<std::uint32_t>(1000000000 * div1e9(val)) 也可以简化
         * 可以在乘法之前就截断，因为乘 div1e9(val) 的高 32 位不会影响结果低 32 位
         */
        return static_cast<std::uint32_t>(val) - 1000000000 * static_cast<std::uint32_t>(div1e9(val));
    }
#else

    RAINY_NODISCARD inline std::uint64_t div5(const std::uint64_t val) {
        return val / 5;
    }

    RAINY_NODISCARD inline std::uint64_t div10(const std::uint64_t val) {
        return val / 10;
    }

    RAINY_NODISCARD inline std::uint64_t div100(const std::uint64_t val) {
        return val / 100;
    }

    RAINY_NODISCARD inline std::uint64_t div1e8(const std::uint64_t val) {
        return val / 100000000;
    }

    RAINY_NODISCARD inline std::uint64_t div1e9(const std::uint64_t val) {
        return val / 1000000000;
    }

    RAINY_NODISCARD inline std::uint32_t mod1e9(const std::uint64_t val) {
        return static_cast<std::uint32_t>(val - 1000000000 * div1e9(val));
    }
#endif

    RAINY_NODISCARD inline std::uint32_t pow5_factor(std::uint64_t value) {
        std::uint32_t count = 0;
        rain_loop {
            core::implements::stl_internal_check(value != 0);
            const std::uint64_t q = div5(value);
            const std::uint32_t result = static_cast<std::uint32_t>(value) - 5 * static_cast<std::uint32_t>(q);
            if (result != 0) {
                break;
            }
            value = q;
            ++count;
        }
        return count;
    }

    RAINY_NODISCARD inline bool multiple_of_power_of_5(const std::uint64_t value, const std::uint32_t power) {
        return pow5_factor(value) >= power;
    }

    RAINY_NODISCARD inline bool multiple_of_power_of_2(const std::uint64_t value, const std::uint32_t power) {
        core::implements::stl_internal_check(value != 0);
        core::implements::stl_internal_check(power < 64);
        return (value & ((1ull << power) - 1)) == 0;
    }

    RAINY_NODISCARD static unsigned char bit_scan_forward(unsigned long *index, unsigned long mask) {
        if (mask == 0) {
            return 0; // 没有置位位，行为未定义（通常返回 0）
        }
        unsigned long lowest = mask & -mask;
        static const unsigned long DE_BRUIJN = 0x077CB531UL;
        static const unsigned char bit_pos[32] = {0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
                                                  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};
        unsigned long bit_index = bit_pos[(lowest * DE_BRUIJN) >> 27];
        *index = bit_index;
        return 1;
    }
}

// 参考 d2fixed.c
// https://github.com/ulfjack/ryu/blob/master/ryu/d2fixed.c
namespace rainy::foundation::text::implements::ryu {
    inline constexpr int POW10_ADDITIONAL_BITS = 120;

    RAINY_NODISCARD inline std::uint32_t mul_shift_mod_1e9(const std::uint64_t m, const std::uint64_t *const mul,
                                                           const std::int32_t j) {
        std::uint64_t high0;
        const std::uint64_t low0 = umul128(m, mul[0], &high0);
        std::uint64_t high1;
        const std::uint64_t low1 = umul128(m, mul[1], &high1);
        std::uint64_t high2;
        const std::uint64_t low2 = umul128(m, mul[2], &high2);
        const std::uint64_t s0_low = low0; // 位 0-63
        (void) s0_low; // 未使用
        const std::uint64_t s0_high = low1 + high0; // 位 64-127
        const std::uint32_t carry1 = (s0_high < low1) ? 1 : 0;
        const std::uint64_t s1_low = low2 + high1 + carry1; // 位 128-191
        const std::uint32_t carry2 = (s1_low < low2) ? 1 : 0;
        const std::uint64_t s1_high = high2 + carry2; // 可能的第4个64位字
        core::implements::stl_internal_check(j >= 128);
        core::implements::stl_internal_check(j <= 180);
        if (j < 160) {
            // 右移 [128, 160) 位，需要 96 位输入
            const std::uint64_t r0 = mod1e9(s1_high);
            const std::uint64_t r1 = mod1e9((r0 << 32) | (s1_low >> 32));
            const std::uint64_t r2 = (r1 << 32) | (s1_low & 0xffffffff);
            return mod1e9(r2 >> (j - 128));
        } else {
            // 右移 [160, 192) 位，只需要 64 位输入
            const std::uint64_t r0 = mod1e9(s1_high);
            const std::uint64_t r1 = (r0 << 32) | (s1_low >> 32);
            return mod1e9(r1 >> (j - 160));
        }
    }

    template <typename CharType>
    void append_n_digits(const std::uint32_t output_length, std::uint32_t digits, CharType *const result) {
        std::uint32_t i = 0;
        while (digits >= 10000) {
#if RAINY_USING_CLANG
            const std::uint32_t ch = digits - 10000 * (digits / 10000);
#else
            const std::uint32_t ch = digits % 10000;
#endif
            digits /= 10000;
            const std::uint32_t c0 = (ch % 100) << 1;
            const std::uint32_t c1 = (ch / 100) << 1;
            std::memcpy(result + output_length - i - 2, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
            std::memcpy(result + output_length - i - 4, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
            i += 4;
        }
        if (digits >= 100) {
            const std::uint32_t ch = (digits % 100) << 1;
            digits /= 100;
            std::memcpy(result + output_length - i - 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            i += 2;
        }
        if (digits >= 10) {
            const std::uint32_t ch = digits << 1;
            std::memcpy(result + output_length - i - 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
        } else {
            result[0] = static_cast<CharType>('0' + digits);
        }
    }

    inline void append_d_digits(const std::uint32_t output_length, std::uint32_t digits, char *const result) {
        std::uint32_t i = 0;
        while (digits >= 10000) {
#if RAINY_USING_CLANG
            const std::uint32_t ch = digits - 10000 * (digits / 10000);
#else
            const std::uint32_t ch = digits % 10000;
#endif
            digits /= 10000;
            const std::uint32_t c0 = (ch % 100) << 1;
            const std::uint32_t c1 = (ch / 100) << 1;
            std::memcpy(result + output_length + 1 - i - 2, DIGIT_TABLE<char> + c0, 2);
            std::memcpy(result + output_length + 1 - i - 4, DIGIT_TABLE<char> + c1, 2);
            i += 4;
        }
        if (digits >= 100) {
            const std::uint32_t ch = (digits % 100) << 1;
            digits /= 100;
            std::memcpy(result + output_length + 1 - i - 2, DIGIT_TABLE<char> + ch, 2);
            i += 2;
        }
        if (digits >= 10) {
            const std::uint32_t ch = digits << 1;
            result[2] = DIGIT_TABLE<char>[ch + 1];
            result[1] = '.';
            result[0] = DIGIT_TABLE<char>[ch];
        } else {
            result[1] = '.';
            result[0] = static_cast<char>('0' + digits);
        }
    }

    template <typename CharType>
    void append_c_digits(const std::uint32_t count, std::uint32_t digits, CharType *const result) {
        std::uint32_t i = 0;
        for (; i < count - 1; i += 2) {
            const std::uint32_t ch = (digits % 100) << 1;
            digits /= 100;
            std::memcpy(result + count - i - 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
        }
        if (i < count) {
            const CharType ch = static_cast<CharType>('0') + (digits % 10);
            result[count - i - 1] = ch;
        }
    }

    template <typename CharType>
    void append_nine_digits(std::uint32_t digits, CharType *const result) {
        if (digits == 0) {
            core::algorithm::fill_n(result, 9, '0');
            return;
        }

        for (std::uint32_t i = 0; i < 5; i += 4) {
#if RAINY_USING_CLANG
            const std::uint32_t ch = digits - 10000 * (digits / 10000);
#else
            const std::uint32_t ch = digits % 10000;
#endif
            digits /= 10000;
            const std::uint32_t c0 = (ch % 100) << 1;
            const std::uint32_t c1 = (ch / 100) << 1;
            std::memcpy(result + 7 - i, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
            std::memcpy(result + 5 - i, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
        }
        result[0] = static_cast<CharType>('0' + digits);
    }

    RAINY_NODISCARD inline std::uint32_t index_for_exponent(const std::uint32_t __e) {
        return (__e + 15) / 16;
    }

    RAINY_NODISCARD inline std::uint32_t pow10_bits_for_index(const std::uint32_t idx) {
        return 16 * idx + POW10_ADDITIONAL_BITS;
    }

    RAINY_NODISCARD inline std::uint32_t length_for_index(const std::uint32_t idx) {
        return (log10_pow2(16 * static_cast<std::int32_t>(idx)) + 1 + 16 + 8) / 9;
    }

    template <typename CharType>
    RAINY_NODISCARD utility::compressed_pair<CharType *, std::errc> d2fixed_buffered_n(CharType *first, CharType *const end,
                                                                                       const double double_val,
                                                                                       const std::uint32_t precision) {
        CharType *const original_first = first;
        const std::uint64_t rightits = double_to_bits(double_val);
        // Case distinction; exit early for the easy cases.
        if (rightits == 0) {
            const std::int32_t total_zero_length = 1 // leading zero
                                                   + static_cast<std::int32_t>(precision != 0) // possible decimal point
                                                   + static_cast<std::int32_t>(precision); // zeroes after decimal point
            if (end - first < total_zero_length) {
                return {end, std::errc::value_too_large};
            }
            *first++ = '0';
            if (precision > 0) {
                *first++ = '.';
                core::algorithm::fill_n(first, precision, '0');
                first += precision;
            }
            return {first, std::errc{}};
        }

        // Decode rightits into mantissa and exponent.
        const std::uint64_t ieee_mantissa = rightits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
        const std::uint32_t ieee_exponent = static_cast<std::uint32_t>(rightits >> DOUBLE_MANTISSA_BITS);

        std::int32_t e2;
        std::uint64_t m2;
        if (ieee_exponent == 0) {
            e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
            m2 = ieee_mantissa;
        } else {
            e2 = static_cast<std::int32_t>(ieee_exponent) - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
            m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieee_mantissa;
        }

        bool __nonzero = false;
        if (e2 >= -52) {
            const std::uint32_t idx = e2 < 0 ? 0 : index_for_exponent(static_cast<std::uint32_t>(e2));
            const std::uint32_t power10_bits = pow10_bits_for_index(idx);
            const std::int32_t __len = static_cast<std::int32_t>(length_for_index(idx));
            for (std::int32_t i = __len - 1; i >= 0; --i) {
                const std::uint32_t j = power10_bits - e2;
                // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                // a slightly faster code path in mul_shift_mod_1e9. Instead, we can just increase the multipliers.
                const std::uint32_t digits =
                    mul_shift_mod_1e9(m2 << 8, POW10_SPLIT[POW10_OFFSET[idx] + i], static_cast<std::int32_t>(j + 8));
                if (__nonzero) {
                    if (end - first < 9) {
                        return {end, std::errc::value_too_large};
                    }
                    append_nine_digits(digits, first);
                    first += 9;
                } else if (digits != 0) {
                    const std::uint32_t output_length = decimal_length9(digits);
                    if (end - first < static_cast<ptrdiff_t>(output_length)) {
                        return {end, std::errc::value_too_large};
                    }
                    append_n_digits(output_length, digits, first);
                    first += output_length;
                    __nonzero = true;
                }
            }
        }
        if (!__nonzero) {
            if (first == end) {
                return {end, std::errc::value_too_large};
            }
            *first++ = '0';
        }
        if (precision > 0) {
            if (first == end) {
                return {end, std::errc::value_too_large};
            }
            *first++ = '.';
        }
        if (e2 < 0) {
            const std::int32_t idx = -e2 / 16;
            const std::uint32_t rightlocks = precision / 9 + 1;
            // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
            int round_up = 0;
            std::uint32_t i = 0;
            if (rightlocks <= MIN_BLOCK_2[idx]) {
                i = rightlocks;
                if (end - first < static_cast<ptrdiff_t>(precision)) {
                    return {end, std::errc::value_too_large};
                }
                core::algorithm::fill_n(first, precision, '0');
                first += precision;
            } else if (i < MIN_BLOCK_2[idx]) {
                i = MIN_BLOCK_2[idx];
                if (end - first < static_cast<ptrdiff_t>(9 * i)) {
                    return {end, std::errc::value_too_large};
                }
                core::algorithm::fill_n(first, 9 * i, '0');
                first += 9 * i;
            }
            for (; i < rightlocks; ++i) {
                const std::int32_t j = leftDDITIONAL_BITS_2 + (-e2 - 16 * idx);
                const std::uint32_t power = POW10_OFFSET_2[idx] + i - MIN_BLOCK_2[idx];
                if (power >= POW10_OFFSET_2[idx + 1]) {
                    // If the remaining digits are all 0, then we might as well use memset.
                    // No rounding required in this case.
                    const std::uint32_t __fill = precision - 9 * i;
                    if (end - first < static_cast<ptrdiff_t>(__fill)) {
                        return {end, std::errc::value_too_large};
                    }
                    core::algorithm::fill_n(first, __fill, '0');
                    first += __fill;
                    break;
                }
                // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                // a slightly faster code path in mul_shift_mod_1e9. Instead, we can just increase the multipliers.
                std::uint32_t digits = mul_shift_mod_1e9(m2 << 8, POW10_SPLIT_2[power], j + 8);
                if (i < rightlocks - 1) {
                    if (end - first < 9) {
                        return {end, std::errc::value_too_large};
                    }
                    append_nine_digits(digits, first);
                    first += 9;
                } else {
                    const std::uint32_t maximum = precision - 9 * i;
                    std::uint32_t last_digit = 0;
                    for (std::uint32_t __k = 0; __k < 9 - maximum; ++__k) {
                        last_digit = digits % 10;
                        digits /= 10;
                    }
                    if (last_digit != 5) {
                        round_up = last_digit > 5;
                    } else {
                        // Is m * 10^(additionalDigits + 1) / 2^(-e2) integer?
                        const std::int32_t __requiredTwos = -e2 - static_cast<std::int32_t>(precision) - 1;
                        const bool __trailingZeros =
                            __requiredTwos <= 0 ||
                            (__requiredTwos < 60 && multiple_of_power_of_2(m2, static_cast<std::uint32_t>(__requiredTwos)));
                        round_up = __trailingZeros ? 2 : 1;
                    }
                    if (maximum > 0) {
                        if (end - first < static_cast<ptrdiff_t>(maximum)) {
                            return {end, std::errc::value_too_large};
                        }
                        append_c_digits(maximum, digits, first);
                        first += maximum;
                    }
                    break;
                }
            }
            if (round_up != 0) {
                CharType *round = first;
                CharType *dot = end;
                rain_loop {
                    if (round == original_first) {
                        round[0] = '1';
                        if (dot != end) {
                            dot[0] = '0';
                            dot[1] = '.';
                        }
                        if (first == end) {
                            return {end, std::errc::value_too_large};
                        }
                        *first++ = '0';
                        break;
                    }
                    --round;
                    const CharType ch = round[0];
                    if (ch == '.') {
                        dot = round;
                    } else if (ch == '9') {
                        round[0] = '0';
                        round_up = 1;
                    } else {
                        if (round_up == 1 || ch % 2 != 0) {
                            round[0] = static_cast<CharType>(ch + 1);
                        }
                        break;
                    }
                }
            }
        } else {
            if (end - first < static_cast<ptrdiff_t>(precision)) {
                return {end, std::errc::value_too_large};
            }
            core::algorithm::fill_n(first, precision, '0');
            first += precision;
        }
        return {first, std::errc{}};
    }

    RAINY_NODISCARD inline to_chars_result d2exp_buffered_n(char *first, char *const end, const double double_val,
                                                            std::uint32_t precision) {
        char *const original_first = first;

        const std::uint64_t rightits = double_to_bits(double_val);

        // Case distinction; exit early for the easy cases.
        if (rightits == 0) {
            const std::int32_t total_zero_length = 1 // leading zero
                                                   + static_cast<std::int32_t>(precision != 0) // possible decimal point
                                                   + static_cast<std::int32_t>(precision) // zeroes after decimal point
                                                   + 4; // "e+00"
            if (end - first < total_zero_length) {
                return {end, std::errc::value_too_large};
            }
            *first++ = '0';
            if (precision > 0) {
                *first++ = '.';
                std::memset(first, '0', precision);
                first += precision;
            }
            std::memcpy(first, "e+00", 4);
            first += 4;
            return {first, std::errc{}};
        }

        // Decode rightits into mantissa and exponent.
        const std::uint64_t ieee_mantissa = rightits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
        const std::uint32_t ieee_exponent = static_cast<std::uint32_t>(rightits >> DOUBLE_MANTISSA_BITS);

        std::int32_t e2;
        std::uint64_t m2;
        if (ieee_exponent == 0) {
            e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
            m2 = ieee_mantissa;
        } else {
            e2 = static_cast<std::int32_t>(ieee_exponent) - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
            m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieee_mantissa;
        }

        const bool __printDecimalPoint = precision > 0;
        ++precision;
        std::uint32_t digits = 0;
        std::uint32_t __printedDigits = 0;
        std::uint32_t leftvailableDigits = 0;
        std::int32_t exp = 0;
        if (e2 >= -52) {
            const std::uint32_t idx = e2 < 0 ? 0 : index_for_exponent(static_cast<std::uint32_t>(e2));
            const std::uint32_t power10_bits = pow10_bits_for_index(idx);
            const std::int32_t __len = static_cast<std::int32_t>(length_for_index(idx));
            for (std::int32_t i = __len - 1; i >= 0; --i) {
                const std::uint32_t j = power10_bits - e2;
                // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                // a slightly faster code path in mul_shift_mod_1e9. Instead, we can just increase the multipliers.
                digits = mul_shift_mod_1e9(m2 << 8, POW10_SPLIT[POW10_OFFSET[idx] + i], static_cast<std::int32_t>(j + 8));
                if (__printedDigits != 0) {
                    if (__printedDigits + 9 > precision) {
                        leftvailableDigits = 9;
                        break;
                    }
                    if (end - first < 9) {
                        return {end, std::errc::value_too_large};
                    }
                    append_nine_digits(digits, first);
                    first += 9;
                    __printedDigits += 9;
                } else if (digits != 0) {
                    leftvailableDigits = decimal_length9(digits);
                    exp = i * 9 + static_cast<std::int32_t>(leftvailableDigits) - 1;
                    if (leftvailableDigits > precision) {
                        break;
                    }
                    if (__printDecimalPoint) {
                        if (end - first < static_cast<ptrdiff_t>(leftvailableDigits + 1)) {
                            return {end, std::errc::value_too_large};
                        }
                        append_d_digits(leftvailableDigits, digits, first);
                        first += leftvailableDigits + 1; // +1 for decimal point
                    } else {
                        if (first == end) {
                            return {end, std::errc::value_too_large};
                        }
                        *first++ = static_cast<char>('0' + digits);
                    }
                    __printedDigits = leftvailableDigits;
                    leftvailableDigits = 0;
                }
            }
        }

        if (e2 < 0 && leftvailableDigits == 0) {
            const std::int32_t idx = -e2 / 16;
            for (std::int32_t i = MIN_BLOCK_2[idx]; i < 200; ++i) {
                const std::int32_t j = leftDDITIONAL_BITS_2 + (-e2 - 16 * idx);
                const std::uint32_t power = POW10_OFFSET_2[idx] + static_cast<std::uint32_t>(i) - MIN_BLOCK_2[idx];
                // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                // a slightly faster code path in mul_shift_mod_1e9. Instead, we can just increase the multipliers.
                digits = (power >= POW10_OFFSET_2[idx + 1]) ? 0 : mul_shift_mod_1e9(m2 << 8, POW10_SPLIT_2[power], j + 8);
                if (__printedDigits != 0) {
                    if (__printedDigits + 9 > precision) {
                        leftvailableDigits = 9;
                        break;
                    }
                    if (end - first < 9) {
                        return {end, std::errc::value_too_large};
                    }
                    append_nine_digits(digits, first);
                    first += 9;
                    __printedDigits += 9;
                } else if (digits != 0) {
                    leftvailableDigits = decimal_length9(digits);
                    exp = -(i + 1) * 9 + static_cast<std::int32_t>(leftvailableDigits) - 1;
                    if (leftvailableDigits > precision) {
                        break;
                    }
                    if (__printDecimalPoint) {
                        if (end - first < static_cast<ptrdiff_t>(leftvailableDigits + 1)) {
                            return {end, std::errc::value_too_large};
                        }
                        append_d_digits(leftvailableDigits, digits, first);
                        first += leftvailableDigits + 1; // +1 for decimal point
                    } else {
                        if (first == end) {
                            return {end, std::errc::value_too_large};
                        }
                        *first++ = static_cast<char>('0' + digits);
                    }
                    __printedDigits = leftvailableDigits;
                    leftvailableDigits = 0;
                }
            }
        }

        const std::uint32_t maximum = precision - __printedDigits;
        if (leftvailableDigits == 0) {
            digits = 0;
        }
        std::uint32_t last_digit = 0;
        if (leftvailableDigits > maximum) {
            for (std::uint32_t __k = 0; __k < leftvailableDigits - maximum; ++__k) {
                last_digit = digits % 10;
                digits /= 10;
            }
        }
        // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
        int round_up = 0;
        if (last_digit != 5) {
            round_up = last_digit > 5;
        } else {
            // Is m * 2^e2 * 10^(precision + 1 - exp) integer?
            // precision was already increased by 1, so we don't need to write + 1 here.
            const std::int32_t __rexp = static_cast<std::int32_t>(precision) - exp;
            const std::int32_t __requiredTwos = -e2 - __rexp;
            bool __trailingZeros =
                __requiredTwos <= 0 || (__requiredTwos < 60 && multiple_of_power_of_2(m2, static_cast<std::uint32_t>(__requiredTwos)));
            if (__rexp < 0) {
                const std::int32_t __requiredFives = -__rexp;
                __trailingZeros = __trailingZeros && multiple_of_power_of_5(m2, static_cast<std::uint32_t>(__requiredFives));
            }
            round_up = __trailingZeros ? 2 : 1;
        }
        if (__printedDigits != 0) {
            if (end - first < static_cast<ptrdiff_t>(maximum)) {
                return {end, std::errc::value_too_large};
            }
            if (digits == 0) {
                std::memset(first, '0', maximum);
            } else {
                append_c_digits(maximum, digits, first);
            }
            first += maximum;
        } else {
            if (__printDecimalPoint) {
                if (end - first < static_cast<ptrdiff_t>(maximum + 1)) {
                    return {end, std::errc::value_too_large};
                }
                append_d_digits(maximum, digits, first);
                first += maximum + 1; // +1 for decimal point
            } else {
                if (first == end) {
                    return {end, std::errc::value_too_large};
                }
                *first++ = static_cast<char>('0' + digits);
            }
        }
        if (round_up != 0) {
            char *round = first;
            rain_loop {
                if (round == original_first) {
                    round[0] = '1';
                    ++exp;
                    break;
                }
                --round;
                const char ch = round[0];
                if (ch == '.') {
                    // Keep going.
                } else if (ch == '9') {
                    round[0] = '0';
                    round_up = 1;
                } else {
                    if (round_up == 1 || ch % 2 != 0) {
                        round[0] = static_cast<char>(ch + 1);
                    }
                    break;
                }
            }
        }

        char _Sign_character;

        if (exp < 0) {
            _Sign_character = '-';
            exp = -exp;
        } else {
            _Sign_character = '+';
        }

        const int _Exponent_part_length = exp >= 100 ? 5 // "e+NNN"
                                                     : 4; // "e+NN"

        if (end - first < _Exponent_part_length) {
            return {end, std::errc::value_too_large};
        }

        *first++ = 'e';
        *first++ = _Sign_character;

        if (exp >= 100) {
            const std::int32_t ch = exp % 10;
            std::memcpy(first, DIGIT_TABLE<char> + 2 * (exp / 10), 2);
            first[2] = static_cast<char>('0' + ch);
            first += 3;
        } else {
            std::memcpy(first, DIGIT_TABLE<char> + 2 * exp, 2);
            first += 2;
        }

        return {first, std::errc{}};
    }
}

// 参考 f2s.c
// https://github.com/ulfjack/ryu/blob/master/ryu/f2s.c
namespace rainy::foundation::text::implements::ryu {
    inline constexpr int FLOAT_MANTISSA_BITS = 23;
    inline constexpr int FLOAT_BIAS = 127;

    // This table is generated by PrintFloatLookupTable.
    inline constexpr int FLOAT_POW5_INV_BITCOUNT = 59;
    inline constexpr std::uint64_t FLOAT_POW5_INV_SPLIT[31] = {
        576460752303423489u, 461168601842738791u, 368934881474191033u, 295147905179352826u, 472236648286964522u, 377789318629571618u,
        302231454903657294u, 483570327845851670u, 386856262276681336u, 309485009821345069u, 495176015714152110u, 396140812571321688u,
        316912650057057351u, 507060240091291761u, 405648192073033409u, 324518553658426727u, 519229685853482763u, 415383748682786211u,
        332306998946228969u, 531691198313966350u, 425352958651173080u, 340282366920938464u, 544451787073501542u, 435561429658801234u,
        348449143727040987u, 557518629963265579u, 446014903970612463u, 356811923176489971u, 570899077082383953u, 456719261665907162u,
        365375409332725730u};
    inline constexpr int FLOAT_POW5_BITCOUNT = 61;
    inline constexpr std::uint64_t FLOAT_POW5_SPLIT[47] = {
        1152921504606846976u, 1441151880758558720u, 1801439850948198400u, 2251799813685248000u, 1407374883553280000u,
        1759218604441600000u, 2199023255552000000u, 1374389534720000000u, 1717986918400000000u, 2147483648000000000u,
        1342177280000000000u, 1677721600000000000u, 2097152000000000000u, 1310720000000000000u, 1638400000000000000u,
        2048000000000000000u, 1280000000000000000u, 1600000000000000000u, 2000000000000000000u, 1250000000000000000u,
        1562500000000000000u, 1953125000000000000u, 1220703125000000000u, 1525878906250000000u, 1907348632812500000u,
        1192092895507812500u, 1490116119384765625u, 1862645149230957031u, 1164153218269348144u, 1455191522836685180u,
        1818989403545856475u, 2273736754432320594u, 1421085471520200371u, 1776356839400250464u, 2220446049250313080u,
        1387778780781445675u, 1734723475976807094u, 2168404344971008868u, 1355252715606880542u, 1694065894508600678u,
        2117582368135750847u, 1323488980084844279u, 1654361225106055349u, 2067951531382569187u, 1292469707114105741u,
        1615587133892632177u, 2019483917365790221u};

    RAINY_NODISCARD inline std::uint32_t pow5_factor(std::uint32_t value) {
        std::uint32_t count = 0;
        rain_loop {
            core::implements::stl_internal_check(value != 0);
            const std::uint32_t q = value / 5;
            const std::uint32_t result = value % 5;
            if (result != 0) {
                break;
            }
            value = q;
            ++count;
        }
        return count;
    }

    // Returns true if value is divisible by 5^power.
    RAINY_NODISCARD inline bool multiple_of_power_of_5(const std::uint32_t value, const std::uint32_t power) {
        return pow5_factor(value) >= power;
    }

    // Returns true if value is divisible by 2^power.
    RAINY_NODISCARD inline bool multiple_of_power_of_2(const std::uint32_t value, const std::uint32_t power) {
        core::implements::stl_internal_check(value != 0);
        core::implements::stl_internal_check(power < 32);
        // return rightuiltin_ctz(value) >= power;
        return (value & ((1u << power) - 1)) == 0;
    }

    RAINY_NODISCARD inline std::uint32_t mul_shift(const std::uint32_t m, const std::uint64_t __factor, const std::int32_t __shift) {
        core::implements::stl_internal_check(__shift > 32);

        // The casts here help MSVC to avoid calls to the leftllmul library
        // function.
        const std::uint32_t __factorLo = static_cast<std::uint32_t>(__factor);
        const std::uint32_t __factorHi = static_cast<std::uint32_t>(__factor >> 32);
        const std::uint64_t rightits0 = static_cast<std::uint64_t>(m) * __factorLo;
        const std::uint64_t rightits1 = static_cast<std::uint64_t>(m) * __factorHi;
#if RAINY_USING_64_BIT_PLATFORM
        // On 32-bit platforms we can avoid a 64-bit shift-right since we only
        // need the upper 32 bits of the result and the shift value is > 32.
        const std::uint32_t rightits0Hi = static_cast<std::uint32_t>(rightits0 >> 32);
        std::uint32_t rightits1Lo = static_cast<std::uint32_t>(rightits1);
        std::uint32_t rightits1Hi = static_cast<std::uint32_t>(rightits1 >> 32);
        rightits1Lo += rightits0Hi;
        rightits1Hi += (rightits1Lo < rightits0Hi);
        const std::int32_t __s = __shift - 32;
        return (rightits1Hi << (32 - __s)) | (rightits1Lo >> __s);
#else // ^^^ 32-bit / 64-bit vvv
        const std::uint64_t __sum = (rightits0 >> 32) + rightits1;
        const std::uint64_t __shiftedSum = __sum >> (__shift - 32);
        core::implements::stl_internal_check(__shiftedSum <= UINT32_MAX);
        return static_cast<std::uint32_t>(__shiftedSum);
#endif // ^^^ 64-bit ^^^
    }

    RAINY_NODISCARD inline std::uint32_t __mulPow5InvDivPow2(const std::uint32_t m, const std::uint32_t q, const std::int32_t j) {
        return mul_shift(m, FLOAT_POW5_INV_SPLIT[q], j);
    }

    RAINY_NODISCARD inline std::uint32_t mul_pow5_div_pow2(const std::uint32_t m, const std::uint32_t i, const std::int32_t j) {
        return mul_shift(m, FLOAT_POW5_SPLIT[i], j);
    }

    // A floating decimal representing m * 10^e.
    struct floating_decimal_32 {
        std::uint32_t mantissa;
        std::int32_t exponent;
    };

    RAINY_NODISCARD inline floating_decimal_32 f2d(const std::uint32_t ieee_mantissa, const std::uint32_t ieee_exponent) {
        std::int32_t e2;
        std::uint32_t m2;
        if (ieee_exponent == 0) {
            e2 = 1 - FLOAT_BIAS - FLOAT_MANTISSA_BITS - 2;
            m2 = ieee_mantissa;
        } else {
            e2 = static_cast<std::int32_t>(ieee_exponent) - FLOAT_BIAS - FLOAT_MANTISSA_BITS - 2;
            m2 = (1u << FLOAT_MANTISSA_BITS) | ieee_mantissa;
        }
        const bool is_even = (m2 & 1) == 0;
        const bool accept_bounds = is_even;
        const std::uint32_t mv = 4 * m2;
        const std::uint32_t mp = 4 * m2 + 2;
        const std::uint32_t mmshift = ieee_mantissa != 0 || ieee_exponent <= 1;
        const std::uint32_t mm = 4 * m2 - 1 - mmshift;
        std::uint32_t vr, vp, vm;
        std::int32_t exp10;
        bool vm_is_trailing_zeros = false;
        bool vr_is_trailing_zeros = false;
        uint8_t last_removed_digit = 0;
        if (e2 >= 0) {
            const std::uint32_t q = log10_pow2(e2);
            exp10 = static_cast<std::int32_t>(q);
            const std::int32_t __k = FLOAT_POW5_INV_BITCOUNT + pow5_bits(static_cast<std::int32_t>(q)) - 1;
            const std::int32_t i = -e2 + static_cast<std::int32_t>(q) + __k;
            vr = __mulPow5InvDivPow2(mv, q, i);
            vp = __mulPow5InvDivPow2(mp, q, i);
            vm = __mulPow5InvDivPow2(mm, q, i);
            if (q != 0 && (vp - 1) / 10 <= vm / 10) {
                // We need to know one removed digit even if we are not going to loop below. We could use
                // q = X - 1 above, except that would require 33 bits for the result, and we've found that
                // 32-bit arithmetic is faster even on 64-bit machines.
                const std::int32_t __l = FLOAT_POW5_INV_BITCOUNT + pow5_bits(static_cast<std::int32_t>(q - 1)) - 1;
                last_removed_digit =
                    static_cast<uint8_t>(__mulPow5InvDivPow2(mv, q - 1, -e2 + static_cast<std::int32_t>(q) - 1 + __l) % 10);
            }
            if (q <= 9) {
                // The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems to be safe as well.
                // Only one of mp, mv, and mm can be a multiple of 5, if any.
                if (mv % 5 == 0) {
                    vr_is_trailing_zeros = multiple_of_power_of_5(mv, q);
                } else if (accept_bounds) {
                    vm_is_trailing_zeros = multiple_of_power_of_5(mm, q);
                } else {
                    vp -= multiple_of_power_of_5(mp, q);
                }
            }
        } else {
            const std::uint32_t q = log10_pow5(-e2);
            exp10 = static_cast<std::int32_t>(q) + e2;
            const std::int32_t i = -e2 - static_cast<std::int32_t>(q);
            const std::int32_t __k = pow5_bits(i) - FLOAT_POW5_BITCOUNT;
            std::int32_t j = static_cast<std::int32_t>(q) - __k;
            vr = mul_pow5_div_pow2(mv, static_cast<std::uint32_t>(i), j);
            vp = mul_pow5_div_pow2(mp, static_cast<std::uint32_t>(i), j);
            vm = mul_pow5_div_pow2(mm, static_cast<std::uint32_t>(i), j);
            if (q != 0 && (vp - 1) / 10 <= vm / 10) {
                j = static_cast<std::int32_t>(q) - 1 - (pow5_bits(i + 1) - FLOAT_POW5_BITCOUNT);
                last_removed_digit = static_cast<uint8_t>(mul_pow5_div_pow2(mv, static_cast<std::uint32_t>(i + 1), j) % 10);
            }
            if (q <= 1) {
                vr_is_trailing_zeros = true;
                if (accept_bounds) {
                    vm_is_trailing_zeros = mmshift == 1;
                } else {
                    --vp;
                }
            } else if (q < 31) {
                vr_is_trailing_zeros = multiple_of_power_of_2(mv, q - 1);
            }
        }
        std::int32_t removed = 0;
        std::uint32_t output;
        if (vm_is_trailing_zeros || vr_is_trailing_zeros) {
            while (vp / 10 > vm / 10) {
#if RAINY_USING_CLANG
                vm_is_trailing_zeros &= vm - (vm / 10) * 10 == 0;
#else
                vm_is_trailing_zeros &= vm % 10 == 0;
#endif
                vr_is_trailing_zeros &= last_removed_digit == 0;
                last_removed_digit = static_cast<uint8_t>(vr % 10);
                vr /= 10;
                vp /= 10;
                vm /= 10;
                ++removed;
            }
            if (vm_is_trailing_zeros) {
                while (vm % 10 == 0) {
                    vr_is_trailing_zeros &= last_removed_digit == 0;
                    last_removed_digit = static_cast<uint8_t>(vr % 10);
                    vr /= 10;
                    vp /= 10;
                    vm /= 10;
                    ++removed;
                }
            }
            if (vr_is_trailing_zeros && last_removed_digit == 5 && vr % 2 == 0) {
                // Round even if the exact number is .....50..0.
                last_removed_digit = 4;
            }
            output = vr + ((vr == vm && (!accept_bounds || !vm_is_trailing_zeros)) || last_removed_digit >= 5);
        } else {
            while (vp / 10 > vm / 10) {
                last_removed_digit = static_cast<uint8_t>(vr % 10);
                vr /= 10;
                vp /= 10;
                vm /= 10;
                ++removed;
            }
            // We need to take vr + 1 if vr is outside bounds or we need to round up.
            output = vr + (vr == vm || last_removed_digit >= 5);
        }
        const std::int32_t exp = exp10 + removed;
        floating_decimal_32 floating_decimal{};
        floating_decimal.exponent = exp;
        floating_decimal.mantissa = output;
        return floating_decimal;
    }

    template <typename CharType>
    RAINY_NODISCARD utility::compressed_pair<CharType *, std::errc> large_integer_to_chars(CharType *const first, CharType *const end,
                                                                                           const std::uint32_t mantissa2,
                                                                                           const std::int32_t exponent2) {
        core::implements::stl_internal_check(exponent2 > 0);
        core::implements::stl_internal_check(exponent2 <= 104); // because ieee_exponent <= 254
        constexpr std::uint32_t data_size = 4;
        std::uint32_t data[data_size]{};
        std::uint32_t maxidx = ((24 + static_cast<std::uint32_t>(exponent2) + 31) / 32) - 1;
        core::implements::stl_internal_check(maxidx < data_size);
        const std::uint32_t bit_shift = static_cast<std::uint32_t>(exponent2) % 32;
        if (bit_shift <= 8) {
            data[maxidx] = mantissa2 << bit_shift;
        } else { // mantissa2's 24 bits cross an element boundary
            data[maxidx - 1] = mantissa2 << bit_shift;
            data[maxidx] = mantissa2 >> (32 - bit_shift);
        }
        std::uint32_t blocks[4]{};
        std::int32_t filled_blocks = 0;
        if (maxidx != 0) {
            rain_loop {
                const std::uint32_t most_significant_elem = data[maxidx];
                const std::uint32_t _Initial_remainder = most_significant_elem % 1000000000;
                const std::uint32_t _Initial_quotient = most_significant_elem / 1000000000;
                data[maxidx] = _Initial_quotient;
                std::uint64_t _Remainder = _Initial_remainder;

                // Process less significant elements.
                std::uint32_t _Idx = maxidx;
                do {
                    --_Idx; // Initially, _Remainder is at most 10^9 - 1.

                    // Now, _Remainder is at most (10^9 - 1) * 2^32 + 2^32 - 1, simplified to 10^9 * 2^32 - 1.
                    _Remainder = (_Remainder << 32) | data[_Idx];

                    // floor((10^9 * 2^32 - 1) / 10^9) == 2^32 - 1, so std::uint32_t _Quotient is lossless.
                    const std::uint32_t _Quotient = static_cast<std::uint32_t>(div1e9(_Remainder));
                    _Remainder = static_cast<std::uint32_t>(_Remainder) - 1000000000u * _Quotient;

                    data[_Idx] = _Quotient;
                } while (_Idx != 0);

                // Store a 0-filled 9-digit block.
                blocks[filled_blocks++] = static_cast<std::uint32_t>(_Remainder);
                if (_Initial_quotient == 0) { // Is the large integer shrinking?
                    --maxidx; // log2(10^9) is 29.9, so we can't shrink by more than one element.
                    if (maxidx == 0) {
                        break; // We've finished long division. Now we need to print data[0].
                    }
                }
            }
        }

        core::implements::stl_internal_check(data[0] != 0);
        for (std::uint32_t _Idx = 1; _Idx < data_size; ++_Idx) {
            core::implements::stl_internal_check(data[_Idx] == 0);
        }

        const std::uint32_t _Data_olength = data[0] >= 1000000000 ? 10 : decimal_length9(data[0]);
        const std::uint32_t total_fixed_length = _Data_olength + 9 * filled_blocks;

        if (end - first < static_cast<ptrdiff_t>(total_fixed_length)) {
            return {end, std::errc::value_too_large};
        }

        CharType *_Result = first;

        // Print data[0]. While it's up to 10 digits,
        // which is more than Ryu generates, the code below can handle this.
        append_n_digits(_Data_olength, data[0], _Result);
        _Result += _Data_olength;

        // Print 0-filled 9-digit blocks.
        for (std::int32_t _Idx = filled_blocks - 1; _Idx >= 0; --_Idx) {
            append_nine_digits(blocks[_Idx], _Result);
            _Result += 9;
        }

        return {_Result, std::errc{}};
    }

    template <typename CharType>
    RAINY_NODISCARD utility::compressed_pair<CharType *, std::errc> to_chars(CharType *const first, CharType *const end,
                                                                             const floating_decimal_32 value, chars_format fmt,
                                                                             const std::uint32_t ieee_mantissa,
                                                                             const std::uint32_t ieee_exponent) {
        std::uint32_t output = value.mantissa;
        const std::int32_t ryu_exponent = value.exponent;
        const std::uint32_t output_length = decimal_length9(output);
        std::int32_t scientific_exponent = ryu_exponent + static_cast<std::int32_t>(output_length) - 1;
        if (fmt == chars_format{}) {
            std::int32_t lower;
            std::int32_t upper;
            if (output_length == 1) {
                // Value | Fixed   | Scientific
                // 1e-3  | "0.001" | "1e-03"
                // 1e4   | "10000" | "1e+04"
                lower = -3;
                upper = 4;
            } else {
                // Value   | Fixed       | Scientific
                // 1234e-7 | "0.0001234" | "1.234e-04"
                // 1234e5  | "123400000" | "1.234e+08"
                lower = -static_cast<std::int32_t>(output_length + 3);
                upper = 5;
            }
            if (lower <= ryu_exponent && ryu_exponent <= upper) {
                fmt = chars_format::fixed;
            } else {
                fmt = chars_format::scientific;
            }
        } else if (fmt == chars_format::general) {
            if (-4 <= scientific_exponent && scientific_exponent < 6) {
                fmt = chars_format::fixed;
            } else {
                fmt = chars_format::scientific;
            }
        }
        if (fmt == chars_format::fixed) {
            // ryu_exponent | Printed  | whole_digits | total_fixed_length  | Notes
            // --------------|----------|---------------|----------------------|---------------------------------------
            //             2 | 172900   |  6            | whole_digits        | Ryu can't be used for printing
            //             1 | 17290    |  5            | (sometimes adjusted) | when the trimmed digits are nonzero.
            // --------------|----------|---------------|----------------------|---------------------------------------
            //             0 | 1729     |  4            | whole_digits        | Unified length cases.
            // --------------|----------|---------------|----------------------|---------------------------------------
            //            -1 | 172.9    |  3            | output_length + 1        | This case can't happen for
            //            -2 | 17.29    |  2            |                      | output_length == 1, but no additional
            //            -3 | 1.729    |  1            |                      | code is needed to avoid it.
            // --------------|----------|---------------|----------------------|---------------------------------------
            //            -4 | 0.1729   |  0            | 2 - ryu_exponent    | C11 7.21.6.1 "The fprintf function"/8:
            //            -5 | 0.01729  | -1            |                      | "If a decimal-point character appears,
            //            -6 | 0.001729 | -2            |                      | at least one digit appears before it."
            const std::int32_t whole_digits = static_cast<std::int32_t>(output_length) + ryu_exponent;
            std::uint32_t total_fixed_length;
            if (ryu_exponent >= 0) { // cases "172900" and "1729"
                total_fixed_length = static_cast<std::uint32_t>(whole_digits);
                if (output == 1) {
                    static constexpr uint8_t adjustment[39] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1,
                                                               0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1};
                    total_fixed_length -= adjustment[ryu_exponent];
                    // whole_digits doesn't need to be adjusted because these cases won't refer to it later.
                }
            } else if (whole_digits > 0) { // case "17.29"
                total_fixed_length = output_length + 1;
            } else { // case "0.001729"
                total_fixed_length = static_cast<std::uint32_t>(2 - ryu_exponent);
            }
            if (end - first < static_cast<ptrdiff_t>(total_fixed_length)) {
                return {end, std::errc::value_too_large};
            }
            CharType *middle;
            if (ryu_exponent > 0) { // case "172900"
                bool can_use_ryu;
                if (ryu_exponent > 10) { // 10^10 is the largest power of 10 that's exactly representable as a float.
                    can_use_ryu = false;
                } else {
                    static constexpr std::uint32_t max_shifted_mantissa[11] = {16777215, 3355443, 671088, 134217, 26843, 5368,
                                                                               1073,     214,     42,     8,      1};
                    unsigned long trailing_zero_bits;
                    (void) bit_scan_forward(&trailing_zero_bits, value.mantissa);
                    const std::uint32_t shifted_mantissa = value.mantissa >> trailing_zero_bits;
                    can_use_ryu = shifted_mantissa <= max_shifted_mantissa[ryu_exponent];
                }

                if (!can_use_ryu) {
                    const std::uint32_t mantissa2 = ieee_mantissa | (1u << FLOAT_MANTISSA_BITS);
                    const std::int32_t exponent2 = static_cast<std::int32_t>(ieee_exponent) - FLOAT_BIAS - FLOAT_MANTISSA_BITS;
                    return large_integer_to_chars(first, end, mantissa2, exponent2);
                }
                middle = first + output_length;
            } else { // "1729", "17.29", and "0.001729"
                middle = first + total_fixed_length;
            }
            while (output >= 10000) {
#if RAINY_USING_CLANG
                const std::uint32_t ch = output - 10000 * (output / 10000);
#else
                const std::uint32_t ch = output % 10000;
#endif
                output /= 10000;
                const std::uint32_t c0 = (ch % 100) << 1;
                const std::uint32_t c1 = (ch / 100) << 1;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
            }
            if (output >= 100) {
                const std::uint32_t ch = (output % 100) << 1;
                output /= 100;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            }
            if (output >= 10) {
                const std::uint32_t ch = output << 1;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            } else {
                *--middle = static_cast<CharType>('0' + output);
            }

            if (ryu_exponent > 0) { // case "172900" with can_use_ryu
                // Performance note: it might be more efficient to do this immediately after setting middle.
                core::algorithm::fill_n(first + output_length, ryu_exponent, '0');
            } else if (ryu_exponent == 0) { // case "1729"
                // Done!
            } else if (whole_digits > 0) { // case "17.29"
                // Performance note: moving digits might not be optimal.
                std::memmove(first, first + 1, static_cast<std::size_t>(whole_digits) * sizeof(CharType));
                first[whole_digits] = '.';
            } else { // case "0.001729"
                // Performance note: a larger memset() followed by overwriting '.' might be more efficient.
                first[0] = '0';
                first[1] = '.';
                core::algorithm::fill_n(first + 2, -whole_digits, '0');
            }

            return {first + total_fixed_length, std::errc{}};
        }

        const std::uint32_t total_scientific_length =
            output_length + (output_length > 1) + 4; // digits + possible decimal point + scientific exponent
        if (end - first < static_cast<ptrdiff_t>(total_scientific_length)) {
            return {end, std::errc::value_too_large};
        }
        CharType *const result = first;

        // Print the decimal digits.
        std::uint32_t i = 0;
        while (output >= 10000) {
#if RAINY_USING_CLANG
            const std::uint32_t ch = output - 10000 * (output / 10000);
#else
            const std::uint32_t ch = output % 10000;
#endif
            output /= 10000;
            const std::uint32_t c0 = (ch % 100) << 1;
            const std::uint32_t c1 = (ch / 100) << 1;
            std::memcpy(result + output_length - i - 1, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
            std::memcpy(result + output_length - i - 3, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
            i += 4;
        }
        if (output >= 100) {
            const std::uint32_t ch = (output % 100) << 1;
            output /= 100;
            std::memcpy(result + output_length - i - 1, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            i += 2;
        }
        if (output >= 10) {
            const std::uint32_t ch = output << 1;
            // We can't use memcpy here: the decimal dot goes between these two digits.
            result[2] = DIGIT_TABLE<CharType>[ch + 1];
            result[0] = DIGIT_TABLE<CharType>[ch];
        } else {
            result[0] = static_cast<CharType>('0' + output);
        }

        // Print decimal point if needed.
        std::uint32_t index;
        if (output_length > 1) {
            result[1] = '.';
            index = output_length + 1;
        } else {
            index = 1;
        }

        // Print the exponent.
        result[index++] = 'e';
        if (scientific_exponent < 0) {
            result[index++] = '-';
            scientific_exponent = -scientific_exponent;
        } else {
            result[index++] = '+';
        }

        std::memcpy(result + index, DIGIT_TABLE<CharType> + 2 * scientific_exponent, 2 * sizeof(CharType));
        index += 2;

        return {first + total_scientific_length, std::errc{}};
    }

    RAINY_NODISCARD inline to_chars_result convert_to_chars_result(const utility::compressed_pair<char *, std::errc> &pair) {
        return {pair.first, pair.second};
    }

    template <typename CharType>
    RAINY_NODISCARD utility::compressed_pair<CharType *, std::errc> f2s_buffered_n(CharType *const first, CharType *const end,
                                                                                   const float float_val, const chars_format fmt) {

        const std::uint32_t rightits = float_to_bits(float_val);
        if (rightits == 0) {
            if (fmt == chars_format::scientific) {
                if (end - first < 5) {
                    return {end, std::errc::value_too_large};
                }
                if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                    std::memcpy(first, "0e+00", 5);
                } else {
                    std::memcpy(first, L"0e+00", 5 * sizeof(wchar_t));
                }
                return {first + 5, std::errc{}};
            }
            if (first == end) {
                return {end, std::errc::value_too_large};
            }
            *first = '0';
            return {first + 1, std::errc{}};
        }
        const std::uint32_t ieee_mantissa = rightits & ((1u << FLOAT_MANTISSA_BITS) - 1);
        const std::uint32_t ieee_exponent = rightits >> FLOAT_MANTISSA_BITS;
        if (fmt == chars_format::fixed) {
            const std::uint32_t mantissa2 = ieee_mantissa | (1u << FLOAT_MANTISSA_BITS); // restore implicit bit
            const std::int32_t exponent2 =
                static_cast<std::int32_t>(ieee_exponent) - FLOAT_BIAS - FLOAT_MANTISSA_BITS; // bias and normalization
            if (exponent2 > 0) {
                return large_integer_to_chars(first, end, mantissa2, exponent2);
            }
        }
        const floating_decimal_32 value = f2d(ieee_mantissa, ieee_exponent);
        return to_chars(first, end, value, fmt, ieee_mantissa, ieee_exponent);
    }
}

// 参考 d2s.c
// https://github.com/ulfjack/ryu/blob/master/ryu/d2s.c
namespace rainy::foundation::text::implements::ryu {
    RAINY_ALWASY_INLINE_NODISCARD std::uint64_t mul_shift_all(std::uint64_t m, const std::uint64_t *const mul, const std::int32_t j,
                                                              std::uint64_t *const vp, std::uint64_t *const vm,
                                                              const std::uint32_t mmshift) {
        m <<= 1;
        std::uint64_t tmp;
        const std::uint64_t lo = umul128(m, mul[0], &tmp);
        std::uint64_t high;
        const std::uint64_t middle = tmp + umul128(m, mul[1], &high);
        high += middle < tmp;
        const std::uint64_t lo2 = lo + mul[0];
        const std::uint64_t middle2 = middle + mul[1] + (lo2 < lo);
        const std::uint64_t hi2 = high + (middle2 < middle);
        *vp = shiftright128(middle2, hi2, static_cast<std::uint32_t>(j - 64 - 1));
        if (mmshift == 1) {
            const std::uint64_t lo3 = lo - mul[0];
            const std::uint64_t _middle3 = middle - mul[1] - (lo3 > lo);
            const std::uint64_t hi3 = high - (_middle3 > middle);
            *vm = shiftright128(_middle3, hi3, static_cast<std::uint32_t>(j - 64 - 1));
        } else {
            const std::uint64_t lo3 = lo + lo;
            const std::uint64_t _middle3 = middle + middle + (lo3 < lo);
            const std::uint64_t hi3 = high + high + (_middle3 < middle);
            const std::uint64_t lo4 = lo3 - mul[0];
            const std::uint64_t _middle4 = _middle3 - mul[1] - (lo4 > lo3);
            const std::uint64_t __hi4 = hi3 - (_middle4 > _middle3);
            *vm = shiftright128(_middle4, __hi4, static_cast<std::uint32_t>(j - 64));
        }
        return shiftright128(middle, high, static_cast<std::uint32_t>(j - 64 - 1));
    }
}

namespace rainy::foundation::text::implements::ryu {
    RAINY_NODISCARD inline std::uint32_t decimal_length17(const std::uint64_t value) {
        core::implements::stl_internal_check(value < 100000000000000000u);
        if (value >= 10000000000000000u) {
            return 17;
        }
        if (value >= 1000000000000000u) {
            return 16;
        }
        if (value >= 100000000000000u) {
            return 15;
        }
        if (value >= 10000000000000u) {
            return 14;
        }
        if (value >= 1000000000000u) {
            return 13;
        }
        if (value >= 100000000000u) {
            return 12;
        }
        if (value >= 10000000000u) {
            return 11;
        }
        if (value >= 1000000000u) {
            return 10;
        }
        if (value >= 100000000u) {
            return 9;
        }
        if (value >= 10000000u) {
            return 8;
        }
        if (value >= 1000000u) {
            return 7;
        }
        if (value >= 100000u) {
            return 6;
        }
        if (value >= 10000u) {
            return 5;
        }
        if (value >= 1000u) {
            return 4;
        }
        if (value >= 100u) {
            return 3;
        }
        if (value >= 10u) {
            return 2;
        }
        return 1;
    }

    struct floating_decimal_64 {
        std::uint64_t mantissa;
        std::int32_t exponent;
    };

    RAINY_NODISCARD inline floating_decimal_64 d2d(const std::uint64_t mantissa, const std::uint32_t exponent) {
        std::int32_t exp2;
        std::uint64_t mantissa2;
        // 处理规格化和非规格化数
        if (exponent == 0) {
            // 非规格化数：减去2以便边界计算有额外2位
            exp2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
            mantissa2 = mantissa;
        } else {
            // 规格化数：加上隐含的1
            exp2 = static_cast<std::int32_t>(exponent) - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
            mantissa2 = (1ull << DOUBLE_MANTISSA_BITS) | mantissa;
        }
        const bool is_even = (mantissa2 & 1) == 0;
        const bool accept_bounds = is_even; // 仅当偶数时接受边界值
        // 步骤2：确定有效十进制表示的区间
        const std::uint64_t mv = 4 * mantissa2; // 中间值
        // 是否需要下边界偏移
        const std::uint32_t mm_shift = (mantissa != 0 || exponent <= 1) ? 1 : 0;
        // 步骤3：使用128位算术转换为十进制幂基
        std::uint64_t vr, vp, vm; // 四舍五入值、上边界、下边界
        std::int32_t exp10;
        bool vm_trailing_zeros = false;
        bool vr_trailing_zeros = false;
        if (exp2 >= 0) {
            // 情况1：指数非负，使用乘逆元算法
            const std::uint32_t q = log10_pow2(exp2) - (exp2 > 3 ? 1 : 0);
            exp10 = static_cast<std::int32_t>(q);
            const std::int32_t k = DOUBLE_POW5_INV_BITCOUNT + pow5_bits(static_cast<std::int32_t>(q)) - 1;
            const std::int32_t i = -exp2 + static_cast<std::int32_t>(q) + k;
            vr = mul_shift_all(mantissa2, DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mm_shift);
            if (q <= 21) {
                // 检查5的倍数导致的末尾零
                const std::uint32_t mv_mod5 = static_cast<std::uint32_t>(mv) - 5 * static_cast<std::uint32_t>(div5(mv));
                if (mv_mod5 == 0) {
                    vr_trailing_zeros = multiple_of_power_of_5(mv, q);
                } else if (accept_bounds) {
                    vm_trailing_zeros = multiple_of_power_of_5(mv - 1 - mm_shift, q);
                } else {
                    // 上边界可能需要调整
                    vp -= multiple_of_power_of_5(mv + 2, q);
                }
            }
        } else {
            // 情况2：指数为负，直接乘以5的幂
            const std::uint32_t q = log10_pow5(-exp2) - ((-exp2) > 1 ? 1 : 0);
            exp10 = static_cast<std::int32_t>(q) + exp2;
            const std::int32_t i = -exp2 - static_cast<std::int32_t>(q);
            const std::int32_t k = pow5_bits(i) - DOUBLE_POW5_BITCOUNT;
            const std::int32_t j = static_cast<std::int32_t>(q) - k;
            vr = mul_shift_all(mantissa2, DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mm_shift);
            if (q <= 1) {
                // 小指数情况，一定有末尾零
                vr_trailing_zeros = true;
                if (accept_bounds) {
                    vm_trailing_zeros = (mm_shift == 1);
                } else {
                    --vp; // 调整上边界
                }
            } else if (q < 63) {
                // 检查2的倍数导致的末尾零
                vr_trailing_zeros = multiple_of_power_of_2(mv, q - 1);
            }
        }
        // 步骤4：寻找区间内最短的十进制表示
        std::int32_t removed_digits = 0;
        uint8_t last_removed_digit = 0;
        std::uint64_t result_mantissa;
        // 处理带末尾零的情况（约0.7%的概率）
        if (vm_trailing_zeros || vr_trailing_zeros) {
            // 通用情况：逐步移除末尾数字
            rain_loop {
                const std::uint64_t vp_div10 = div10(vp);
                const std::uint64_t vm_div10 = div10(vm);
                if (vp_div10 <= vm_div10) {
                    break;
                }
                const std::uint32_t vm_mod10 = static_cast<std::uint32_t>(vm) - 10 * static_cast<std::uint32_t>(vm_div10);
                const std::uint64_t vr_div10 = div10(vr);
                const std::uint32_t vr_mod10 = static_cast<std::uint32_t>(vr) - 10 * static_cast<std::uint32_t>(vr_div10);
                vm_trailing_zeros &= (vm_mod10 == 0);
                vr_trailing_zeros &= (last_removed_digit == 0);
                last_removed_digit = static_cast<uint8_t>(vr_mod10);
                vr = vr_div10;
                vp = vp_div10;
                vm = vm_div10;
                ++removed_digits;
            }
            // 继续处理下边界末尾零的情况
            if (vm_trailing_zeros) {
                rain_loop {
                    const std::uint64_t vm_div10 = div10(vm);
                    const std::uint32_t vm_mod10 = static_cast<std::uint32_t>(vm) - 10 * static_cast<std::uint32_t>(vm_div10);
                    if (vm_mod10 != 0) {
                        break;
                    }
                    const std::uint64_t vp_div10 = div10(vp);
                    const std::uint64_t vr_div10 = div10(vr);
                    const std::uint32_t vr_mod10 = static_cast<std::uint32_t>(vr) - 10 * static_cast<std::uint32_t>(vr_div10);
                    vr_trailing_zeros &= (last_removed_digit == 0);
                    last_removed_digit = static_cast<uint8_t>(vr_mod10);
                    vr = vr_div10;
                    vp = vp_div10;
                    vm = vm_div10;
                    ++removed_digits;
                }
            }
            // 处理银行家舍入法的情况
            if (vr_trailing_zeros && last_removed_digit == 5 && (vr % 2) == 0) {
                last_removed_digit = 4; // 向偶数舍入
            }
            // 决定是否需要进位
            bool need_round_up = (vr == vm && (!accept_bounds || !vm_trailing_zeros)) || last_removed_digit >= 5;
            result_mantissa = vr + (need_round_up ? 1 : 0);
        } else {
            // 常见情况优化（约99.3%的概率）
            bool round_up = false;
            const std::uint64_t vp_div100 = div100(vp);
            const std::uint64_t vm_div100 = div100(vm);
            // 优化：一次移除两位数字（约86.2%的情况）
            if (vp_div100 > vm_div100) {
                const std::uint64_t vr_div100 = div100(vr);
                const std::uint32_t vr_mod100 = static_cast<std::uint32_t>(vr) - 100 * static_cast<std::uint32_t>(vr_div100);
                round_up = (vr_mod100 >= 50);
                vr = vr_div100;
                vp = vp_div100;
                vm = vm_div100;
                removed_digits += 2;
            }
            // 逐位移除剩余数字
            rain_loop {
                const std::uint64_t vp_div10 = div10(vp);
                const std::uint64_t vm_div10 = div10(vm);
                if (vp_div10 <= vm_div10) {
                    break;
                }
                const std::uint64_t vr_div10 = div10(vr);
                const std::uint32_t vr_mod10 = static_cast<std::uint32_t>(vr) - 10 * static_cast<std::uint32_t>(vr_div10);
                round_up = (vr_mod10 >= 5);
                vr = vr_div10;
                vp = vp_div10;
                vm = vm_div10;
                ++removed_digits;
            }
            // 决定是否需要进位
            result_mantissa = vr + ((vr == vm || round_up) ? 1 : 0);
        }

        // 构造返回结果
        floating_decimal_64 result;
        result.exponent = exp10 + removed_digits;
        result.mantissa = result_mantissa;
        return result;
    }

    template <typename CharType>
    RAINY_NODISCARD utility::compressed_pair<CharType *, std::errc> to_chars(CharType *const first, CharType *const end,
                                                                             const floating_decimal_64 value, chars_format fmt,
                                                                             const double float_val) {
        // Step 5: Print the decimal representation.
        std::uint64_t output = value.mantissa;
        const std::int32_t ryu_exponent = value.exponent;
        const std::uint32_t output_length = decimal_length17(output);
        std::int32_t scientific_exponent = ryu_exponent + static_cast<std::int32_t>(output_length) - 1;
        if (fmt == chars_format{}) {
            std::int32_t lower;
            std::int32_t upper;
            if (output_length == 1) {
                // Value | Fixed   | Scientific
                // 1e-3  | "0.001" | "1e-03"
                // 1e4   | "10000" | "1e+04"
                lower = -3;
                upper = 4;
            } else {
                // Value   | Fixed       | Scientific
                // 1234e-7 | "0.0001234" | "1.234e-04"
                // 1234e5  | "123400000" | "1.234e+08"
                lower = -static_cast<std::int32_t>(output_length + 3);
                upper = 5;
            }
            if (lower <= ryu_exponent && ryu_exponent <= upper) {
                fmt = chars_format::fixed;
            } else {
                fmt = chars_format::scientific;
            }
        } else if (fmt == chars_format::general) {
            if (-4 <= scientific_exponent && scientific_exponent < 6) {
                fmt = chars_format::fixed;
            } else {
                fmt = chars_format::scientific;
            }
        }
        if (fmt == chars_format::fixed) {
            /*
             * ryu_exponent | Printed  | whole_digits | total_fixed_length  | 备注
             * --------------|----------|---------------|----------------------|---------------------------------------
             *             2 | 172900   |  6            | whole_digits        | 当修剪后的数字不为零时，Ryu 不能用于打印
             *             1 | 17290    |  5            | (有时会调整)        |
             * --------------|----------|---------------|----------------------|---------------------------------------
             *             0 | 1729     |  4            | whole_digits        | 统一长度的情况
             * --------------|----------|---------------|----------------------|---------------------------------------
             *            -1 | 172.9    |  3            | output_length + 1   | 当 output_length == 1 时，
             *            -2 | 17.29    |  2            |                      | 这种情况不会发生，但不需要额外的代码来避免它
             *            -3 | 1.729    |  1            |                      |
             * --------------|----------|---------------|----------------------|---------------------------------------
             *            -4 | 0.1729   |  0            | 2 - ryu_exponent    | C11 7.21.6.1 "The fprintf function"/8：
             *            -5 | 0.01729  | -1            |                      | "如果出现小数点，则在其前面至少出现一位数字"
             *            -6 | 0.001729 | -2            |                      |
             */
            const std::int32_t whole_digits = static_cast<std::int32_t>(output_length) + ryu_exponent;
            std::uint32_t total_fixed_length;
            if (ryu_exponent >= 0) { // "172900" and "1729"
                total_fixed_length = static_cast<std::uint32_t>(whole_digits);
                if (output == 1) {
                    static constexpr uint8_t adjustment[309] = {
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1,
                        1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0,
                        0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
                        1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0,
                        1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0,
                        1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,
                        0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0,
                        0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0};
                    total_fixed_length -= adjustment[ryu_exponent];
                }
            } else if (whole_digits > 0) { // case "17.29"
                total_fixed_length = output_length + 1;
            } else { // case "0.001729"
                total_fixed_length = static_cast<std::uint32_t>(2 - ryu_exponent);
            }
            if (end - first < static_cast<ptrdiff_t>(total_fixed_length)) {
                return {end, std::errc::value_too_large};
            }
            CharType *middle;
            if (ryu_exponent > 0) { // case "172900"
                bool can_use_ryu;
                if (ryu_exponent > 22) {
                    can_use_ryu = false;
                } else {
                    static constexpr std::uint64_t max_shifted_mantissa[23] = {9007199254740991u,
                                                                               1801439850948198u,
                                                                               360287970189639u,
                                                                               72057594037927u,
                                                                               14411518807585u,
                                                                               2882303761517u,
                                                                               576460752303u,
                                                                               115292150460u,
                                                                               23058430092u,
                                                                               4611686018u,
                                                                               922337203u,
                                                                               184467440u,
                                                                               36893488u,
                                                                               7378697u,
                                                                               1475739u,
                                                                               295147u,
                                                                               59029u,
                                                                               11805u,
                                                                               2361u,
                                                                               472u,
                                                                               94u,
                                                                               18u,
                                                                               3u};
                    unsigned long trailing_zero_bits;
                    const std::uint32_t low_mantissa = static_cast<std::uint32_t>(value.mantissa);
                    if (low_mantissa != 0) {
                        (void) bit_scan_forward(&trailing_zero_bits, low_mantissa);
                    } else {
                        const std::uint32_t high_mantissa = static_cast<std::uint32_t>(value.mantissa >> 32); // nonzero here
                        (void) bit_scan_forward(&trailing_zero_bits, high_mantissa);
                        trailing_zero_bits += 32;
                    }
                    const std::uint64_t shifted_mantissa = value.mantissa >> trailing_zero_bits;
                    can_use_ryu = shifted_mantissa <= max_shifted_mantissa[ryu_exponent];
                }

                if (!can_use_ryu) {
                    return d2fixed_buffered_n(first, end, float_val, 0);
                }
                middle = first + output_length;
            } else { // "1729", "17.29", "0.001729"
                middle = first + total_fixed_length;
            }
            if ((output >> 32) != 0) {
                const std::uint64_t q = div1e8(output);
                std::uint32_t output2 = static_cast<std::uint32_t>(output - 100000000 * q);
                output = q;
                const std::uint32_t ch = output2 % 10000;
                output2 /= 10000;
                const std::uint32_t double_val = output2 % 10000;
                const std::uint32_t c0 = (ch % 100) << 1;
                const std::uint32_t c1 = (ch / 100) << 1;
                const std::uint32_t d0 = (double_val % 100) << 1;
                const std::uint32_t __d1 = (double_val / 100) << 1;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + d0, 2 * sizeof(CharType));
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + __d1, 2 * sizeof(CharType));
            }
            std::uint32_t output2 = static_cast<std::uint32_t>(output);
            while (output2 >= 10000) {
#if RAINY_USING_CLANG
                const std::uint32_t ch = output2 - 10000 * (output2 / 10000);
#else
                const std::uint32_t ch = output2 % 10000;
#endif
                output2 /= 10000;
                const std::uint32_t c0 = (ch % 100) << 1;
                const std::uint32_t c1 = (ch / 100) << 1;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
            }
            if (output2 >= 100) {
                const std::uint32_t ch = (output2 % 100) << 1;
                output2 /= 100;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            }
            if (output2 >= 10) {
                const std::uint32_t ch = output2 << 1;
                std::memcpy(middle -= 2, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            } else {
                *--middle = static_cast<CharType>('0' + output2);
            }
            if (ryu_exponent > 0) {
                core::algorithm::fill_n(first + output_length, ryu_exponent, '0');
            } else if (ryu_exponent == 0) { // "1729"
                return {first + total_fixed_length, std::errc{}}; // 不需要处理
            } else if (whole_digits > 0) {
                std::memmove(first, first + 1, static_cast<std::size_t>(whole_digits) * sizeof(CharType));
                first[whole_digits] = '.';
            } else { // "0.001729"
                first[0] = '0';
                first[1] = '.';
                core::algorithm::fill_n(first + 2, -whole_digits, '0');
            }
            return {first + total_fixed_length, std::errc{}};
        }
        const std::uint32_t total_scientific_length =
            output_length + (output_length > 1) // digits + possible decimal point
            + (-100 < scientific_exponent && scientific_exponent < 100 ? 4 : 5); // + scientific exponent
        if (end - first < static_cast<ptrdiff_t>(total_scientific_length)) {
            return {end, std::errc::value_too_large};
        }
        CharType *const result = first;
        std::uint32_t i = 0;
        if ((output >> 32) != 0) {
            const std::uint64_t q = div1e8(output);
            std::uint32_t output2 = static_cast<std::uint32_t>(output) - 100000000 * static_cast<std::uint32_t>(q);
            output = q;

            const std::uint32_t ch = output2 % 10000;
            output2 /= 10000;
            const std::uint32_t double_val = output2 % 10000;
            const std::uint32_t c0 = (ch % 100) << 1;
            const std::uint32_t c1 = (ch / 100) << 1;
            const std::uint32_t d0 = (double_val % 100) << 1;
            const std::uint32_t __d1 = (double_val / 100) << 1;
            std::memcpy(result + output_length - i - 1, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
            std::memcpy(result + output_length - i - 3, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
            std::memcpy(result + output_length - i - 5, DIGIT_TABLE<CharType> + d0, 2 * sizeof(CharType));
            std::memcpy(result + output_length - i - 7, DIGIT_TABLE<CharType> + __d1, 2 * sizeof(CharType));
            i += 8;
        }
        std::uint32_t output2 = static_cast<std::uint32_t>(output);
        while (output2 >= 10000) {
#if RAINY_USING_CLANG
            const std::uint32_t ch = output2 - 10000 * (output2 / 10000);
#else
            const std::uint32_t ch = output2 % 10000;
#endif
            output2 /= 10000;
            const std::uint32_t c0 = (ch % 100) << 1;
            const std::uint32_t c1 = (ch / 100) << 1;
            std::memcpy(result + output_length - i - 1, DIGIT_TABLE<CharType> + c0, 2 * sizeof(CharType));
            std::memcpy(result + output_length - i - 3, DIGIT_TABLE<CharType> + c1, 2 * sizeof(CharType));
            i += 4;
        }
        if (output2 >= 100) {
            const std::uint32_t ch = (output2 % 100) << 1;
            output2 /= 100;
            std::memcpy(result + output_length - i - 1, DIGIT_TABLE<CharType> + ch, 2 * sizeof(CharType));
            i += 2;
        }
        if (output2 >= 10) {
            const std::uint32_t ch = output2 << 1;
            result[2] = DIGIT_TABLE<CharType>[ch + 1];
            result[0] = DIGIT_TABLE<CharType>[ch];
        } else {
            result[0] = static_cast<CharType>('0' + output2);
        }
        std::uint32_t index;
        if (output_length > 1) {
            result[1] = '.';
            index = output_length + 1;
        } else {
            index = 1;
        }
        result[index++] = 'e';
        if (scientific_exponent < 0) {
            result[index++] = '-';
            scientific_exponent = -scientific_exponent;
        } else {
            result[index++] = '+';
        }
        if (scientific_exponent >= 100) {
            const std::int32_t ch = scientific_exponent % 10;
            std::memcpy(result + index, DIGIT_TABLE<CharType> + 2 * (scientific_exponent / 10), 2 * sizeof(CharType));
            result[index + 2] = static_cast<CharType>('0' + ch);
            index += 3;
        } else {
            std::memcpy(result + index, DIGIT_TABLE<CharType> + 2 * scientific_exponent, 2 * sizeof(CharType));
            index += 2;
        }
        return {first + total_scientific_length, std::errc{}};
    }

    RAINY_NODISCARD inline bool d2d_small_int(const std::uint64_t ieee_mantissa, const std::uint32_t ieee_exponent,
                                              floating_decimal_64 *const value) {
        const std::uint64_t m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieee_mantissa;
        const std::int32_t e2 = static_cast<std::int32_t>(ieee_exponent) - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
        if (e2 > 0) {
            return false;
        }
        if (e2 < -52) {
            // f < 1.
            return false;
        }
        const std::uint64_t __mask = (1ull << -e2) - 1;
        const std::uint64_t __fraction = m2 & __mask;
        if (__fraction != 0) {
            return false;
        }
        value->mantissa = m2 >> -e2;
        value->exponent = 0;
        return true;
    }

    template <typename CharType>
    RAINY_NODISCARD utility::compressed_pair<CharType *, std::errc> d2s_buffered_n(CharType *const first, CharType *const end,
                                                                                   const double float_val, const chars_format fmt) {

        // 先解码浮点数，然后进行规范化和次正规情况
        const std::uint64_t rightits = double_to_bits(float_val);
        if (rightits == 0) {
            if (fmt == chars_format::scientific) {
                if (end - first < 5) {
                    return {end, std::errc::value_too_large};
                }

                if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                    std::memcpy(first, "0e+00", 5);
                } else {
                    std::memcpy(first, L"0e+00", 5 * sizeof(wchar_t));
                }

                return {first + 5, std::errc{}};
            }
            if (first == end) {
                return {end, std::errc::value_too_large};
            }
            *first = '0';

            return {first + 1, std::errc{}};
        }
        // 解出尾数和指数
        const std::uint64_t ieee_mantissa = rightits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
        const std::uint32_t ieee_exponent = static_cast<std::uint32_t>(rightits >> DOUBLE_MANTISSA_BITS);
        if (fmt == chars_format::fixed) {
            const std::int32_t exponent2 = static_cast<std::int32_t>(ieee_exponent) - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
            if (exponent2 > 0) {
                return d2fixed_buffered_n(first, end, float_val, 0);
            }
        }
        floating_decimal_64 value;
        const bool is_small_int = d2d_small_int(ieee_mantissa, ieee_exponent, &value);
        if (is_small_int) {
            rain_loop {
                const std::uint64_t q = div10(value.mantissa);
                const std::uint32_t result = static_cast<std::uint32_t>(value.mantissa) - 10 * static_cast<std::uint32_t>(q);
                if (result != 0) {
                    break;
                }
                value.mantissa = q;
                ++value.exponent;
            }
        } else {
            value = d2d(ieee_mantissa, ieee_exponent);
        }

        return to_chars(first, end, value, fmt, float_val);
    }
}

namespace rainy::foundation::text::implements {
    struct floating_point_string {
        bool is_negative;
        std::int32_t exponent;
        std::uint32_t mantissa_count;
        std::uint8_t mantissa[768];
    };

    enum class floating_to_chars_overload {
        plain,
        format_only,
        format_precision
    };

    inline constexpr char charconv_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
                                               'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                               'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    template <typename Floating>
    RAINY_NODISCARD to_chars_result floating_to_chars_ryu(char *const first, char *const end, const Floating value,
                                                          const chars_format fmt) noexcept {
        if constexpr (type_traits::type_relations::is_same_v<Floating, float>) {
            return ryu::convert_to_chars_result(ryu::f2s_buffered_n(first, end, value, fmt));
        } else {
            return ryu::convert_to_chars_result(ryu::d2s_buffered_n(first, end, value, fmt));
        }
    }

    template <typename Floating>
    RAINY_NODISCARD to_chars_result floating_to_chars_scientific_precision(char *const first, char *const end, const Floating value,
                                                                           int precision) noexcept {

        if (precision < 0) {
            precision = 6;
        } else if (precision < 1'000'000'000) {
            // precision is ok.
        } else {
            return {end, std::errc::value_too_large};
        }

        return ryu::d2exp_buffered_n(first, end, value, static_cast<std::uint32_t>(precision));
    }

    template <typename Floating>
    RAINY_NODISCARD to_chars_result floating_to_chars_fixed_precision(char *const first, char *const end, const Floating value,
                                                                      int precision) noexcept {
        if (precision < 0) {
            precision = 6;
        } else if (precision < 1'000'000'000) {
            // ok
        } else {
            return {end, std::errc::value_too_large};
        }
        return ryu::convert_to_chars_result(ryu::d2fixed_buffered_n(first, end, value, static_cast<std::uint32_t>(precision)));
    }

    template <typename Floating>
    RAINY_NODISCARD to_chars_result floating_to_chars_hex_precision(char *first, char *const end, const Floating value,
                                                                    int precision) noexcept {
        constexpr int full_precision = type_traits::type_relations::is_same_v<Floating, float> ? 6 : 13;
        constexpr int adjusted_bits = full_precision * 4;
        if (precision < 0) {
            precision = full_precision;
        }
        using traits = utility::floating_type_traits<Floating>;
        using uint_type = typename traits::uint_type;
        // 提取 IEEE 表示的尾数和指数
        const uint_type uint_value = core::builtin::bit_cast<uint_type>(value);
        const uint_type ieee_mantissa = uint_value & traits::denormal_mantissa_mask;
        const std::int32_t ieee_exponent = static_cast<std::int32_t>(uint_value >> traits::exponent_shift);
        // 调整尾数以对齐十六进制位
        uint_type adjusted_mantissa;
        if constexpr (type_traits::type_relations::is_same_v<Floating, float>) {
            adjusted_mantissa = ieee_mantissa << 1; // float 23位，左移1位对齐4的倍数
        } else {
            adjusted_mantissa = ieee_mantissa; // double 52位，已对齐
        }
        // 计算无偏指数
        std::int32_t unbiased_exponent;
        if (ieee_exponent == 0) {
            if (ieee_mantissa == 0) {
                unbiased_exponent = 0; // 零值
            } else {
                unbiased_exponent = 1 - traits::exponent_bias; // 非规格化数
            }
        } else {
            adjusted_mantissa |= uint_type{1} << adjusted_bits; // 规格化数，添加隐含位
            unbiased_exponent = ieee_exponent - traits::exponent_bias;
        }
        // 分解指数符号和绝对值
        char exp_sign;
        uint32_t exp_abs;
        if (unbiased_exponent < 0) {
            exp_sign = '-';
            exp_abs = static_cast<uint32_t>(-unbiased_exponent);
        } else {
            exp_sign = '+';
            exp_abs = static_cast<uint32_t>(unbiased_exponent);
        }
        // 预计算指数长度并检查缓冲区
        int exp_len;
        if (exp_abs < 10) {
            exp_len = 1;
        } else if (exp_abs < 100) {
            exp_len = 2;
        } else if constexpr (type_traits::type_relations::is_same_v<Floating, float>) {
            exp_len = 3;
        } else if (exp_abs < 1000) {
            exp_len = 3;
        } else {
            exp_len = 4;
        }
        ptrdiff_t remaining = end - first;
        if (remaining < precision) {
            return {end, std::errc::value_too_large};
        }
        remaining -= precision;
        int required_len = 1 // 前导十六进制位
                           + (precision > 0 ? 1 : 0) // 可能的小数点
                           + 2 // "p+" 或 "p-"
                           + exp_len; // 指数部分
        if (remaining < required_len) {
            return {end, std::errc::value_too_large};
        }
        // 精度小于完整精度时进行舍入
        if (precision < full_precision) {
            int dropped_bits = (full_precision - precision) * 4;
            uint_type lsb = adjusted_mantissa;
            uint_type round_bit = adjusted_mantissa << 1;
            uint_type tail_bits = round_bit - 1;
            uint_type should_round = round_bit & (tail_bits | lsb) & (uint_type{1} << dropped_bits);
            adjusted_mantissa += should_round;
        }
        // 输出前导十六进制位
        {
            uint32_t nibble = static_cast<uint32_t>(adjusted_mantissa >> adjusted_bits);
            core::implements::stl_internal_check(nibble < 3);
            *first++ = static_cast<char>('0' + nibble);
            adjusted_mantissa &= (uint_type{1} << adjusted_bits) - 1;
        }
        // 输出小数部分
        if (precision > 0) {
            *first++ = '.';
            int bits_left = adjusted_bits; // float 24, double 52
            while (precision > 0) {
                core::implements::stl_internal_check(bits_left >= 4 && bits_left % 4 == 0);
                bits_left -= 4;
                uint32_t nibble = static_cast<uint32_t>(adjusted_mantissa >> bits_left);
                core::implements::stl_internal_check(nibble < 16);
                *first++ = charconv_digits[nibble];
                if (--precision == 0) {
                    break;
                }
                if (bits_left == 0) {
                    std::memset(first, '0', static_cast<std::size_t>(precision));
                    first += precision;
                    break;
                }
                adjusted_mantissa &= (uint_type{1} << bits_left) - 1;
            }
        }
        // 输出指数部分
        *first++ = 'p';
        *first++ = exp_sign;
        return text::to_chars(first, end, exp_abs);
    }

    template <typename Floating>
    RAINY_NODISCARD to_chars_result floating_to_chars_hex_shortest(char *first, char *const end, const Floating value) noexcept {
        using traits = utility::floating_type_traits<Floating>;
        using uint_type = typename traits::uint_type;
        const uint_type uint_value = core::builtin::bit_cast<uint_type>(value);
        if (uint_value == 0) {
            const char *const str = "0p+0";
            constexpr std::size_t len = 4;
            if (end - first < static_cast<ptrdiff_t>(len)) {
                return {end, std::errc::value_too_large};
            }
            std::memcpy(first, str, len);
            return {first + len, std::errc{}};
        }
        const uint_type ieee_mantissa = uint_value & traits::denormal_mantissa_mask;
        const std::int32_t ieee_exponent = static_cast<std::int32_t>(uint_value >> traits::exponent_shift);
        char leading_hexit;
        std::int32_t unbiased_exponent;
        if (ieee_exponent == 0) { // subnormal
            leading_hexit = '0';
            unbiased_exponent = 1 - traits::exponent_bias;
        } else { // normal
            leading_hexit = '1';
            unbiased_exponent = ieee_exponent - traits::exponent_bias;
        }
        if (first == end) {
            return {end, std::errc::value_too_large};
        }
        *first++ = leading_hexit;
        if (ieee_mantissa == 0) {
        } else {
            if (first == end) {
                return {end, std::errc::value_too_large};
            }
            *first++ = '.';
            uint_type _Adjusted_mantissa;
            std::int32_t _Number_of_bits_remaining;

            if constexpr (type_traits::type_relations::is_same_v<Floating, float>) {
                _Adjusted_mantissa = ieee_mantissa << 1; // align to hexit boundary (23 isn't divisible by 4)
                _Number_of_bits_remaining = 24; // 23 fraction bits + 1 alignment bit
            } else {
                _Adjusted_mantissa = ieee_mantissa; // already aligned (52 is divisible by 4)
                _Number_of_bits_remaining = 52; // 52 fraction bits
            }
            do {
                core::implements::stl_internal_check(_Number_of_bits_remaining >= 4);
                core::implements::stl_internal_check(_Number_of_bits_remaining % 4 == 0);
                _Number_of_bits_remaining -= 4;

                const uint32_t _Nibble = static_cast<uint32_t>(_Adjusted_mantissa >> _Number_of_bits_remaining);
                core::implements::stl_internal_check(_Nibble < 16);
                const char _Hexit = charconv_digits[_Nibble];
                if (first == end) {
                    return {end, std::errc::value_too_large};
                }
                *first++ = _Hexit;

                const uint_type _Mask = (uint_type{1} << _Number_of_bits_remaining) - 1;
                _Adjusted_mantissa &= _Mask;

            } while (_Adjusted_mantissa != 0);
        }
        if (end - first < 2) {
            return {end, std::errc::value_too_large};
        }
        *first++ = 'p';
        if (unbiased_exponent < 0) {
            *first++ = '-';
            unbiased_exponent = -unbiased_exponent;
        } else {
            *first++ = '+';
        }
        return text::to_chars(first, end, static_cast<uint32_t>(unbiased_exponent));
    }

    template <typename Floating>
    RAINY_NODISCARD inline to_chars_result floating_to_chars_general_precision(char *first, char *const end, const Floating value,
                                                                               int precision) noexcept {
        using traits = utility::floating_type_traits<Floating>;
        using uint_type = typename traits::uint_type;
        const uint_type uint_value = core::builtin::bit_cast<uint_type>(value);
        if (uint_value == 0) {
            if (first == end) {
                return {end, std::errc::value_too_large};
            }
            *first++ = '0';
            return {first, std::errc{}};
        }
        // 精度参数处理
        if (precision < 0) {
            precision = 6;
        } else if (precision == 0) {
            precision = 1;
        } else if (precision > 1'000'000) {
            precision = 1'000'000;
        }
        // 根据精度选择预计算表
        using tables = ryu::general_precision_tables_2<Floating>;
        const uint_type *table_begin, *table_end;
        if (precision <= tables::max_special_p) {
            size_t offset = (precision - 1) * (precision + 10) / 2;
            table_begin = tables::speical_x_table + offset;
            table_end = table_begin + precision + 5;
        } else {
            table_begin = tables::ordinary_x_table;
            table_end = table_begin + (core::min)(precision, tables::max_p) + 5;
        }
        // 二分查找合适的表项
        const uint_type *table_lower_bound;
        if constexpr (!type_traits::type_relations::is_same_v<Floating, float>) {
            if (precision > 155) {
                table_lower_bound = core::algorithm::lower_bound(table_begin, table_end, uint_value, std::less{});
            } else {
                table_lower_bound = core::algorithm::find_if(table_begin, table_end, [=](uint_type x) { return uint_value <= x; });
            }
        } else {
            table_lower_bound = core::algorithm::find_if(table_begin, table_end, [=](uint_type x) { return uint_value <= x; });
        }
        ptrdiff_t table_index = table_lower_bound - table_begin;
        int scientific_exponent = static_cast<int>(table_index - 5);
        bool use_fixed = (precision > scientific_exponent && scientific_exponent >= -4);
        // 缓冲区大小预计算
        constexpr int max_buf_size = type_traits::type_relations::is_same_v<Floating, float> ? 117 : 773;
        constexpr int max_fixed_prec = type_traits::type_relations::is_same_v<Floating, float> ? 37 : 66;
        constexpr int max_sci_prec = type_traits::type_relations::is_same_v<Floating, float> ? 111 : 766;
        char buffer[max_buf_size]{};
        const char *sig_start = buffer;
        const char *sig_end = nullptr;
        const char *exp_start = nullptr;
        const char *exp_end = nullptr;
        int effective_precision;
        // 生成数字字符串
        if (use_fixed) {
            effective_precision = (core::min)(precision - (scientific_exponent + 1), max_fixed_prec);
            to_chars_result r = floating_to_chars_fixed_precision(buffer, utility::end(buffer), value, effective_precision);
            core::implements::stl_internal_check(r.ec == std::errc{});
            sig_end = r.ptr;
        } else {
            effective_precision = (core::min)(precision - 1, max_sci_prec);
            to_chars_result r = floating_to_chars_scientific_precision(buffer, utility::end(buffer), value, effective_precision);
            core::implements::stl_internal_check(r.ec == std::errc{});
            sig_end = core::algorithm::find(buffer, r.ptr, 'e');
            exp_start = sig_end;
            exp_end = r.ptr;
        }
        // 移除尾随零
        if (effective_precision > 0) {
            while (sig_end[-1] == '0') {
                --sig_end;
            }
            if (sig_end[-1] == '.') {
                --sig_end;
            }
        }
        // 复制有效数字部分
        ptrdiff_t sig_len = sig_end - sig_start;
        if (end - first < sig_len) {
            return {end, std::errc::value_too_large};
        }
        std::memcpy(first, sig_start, static_cast<std::size_t>(sig_len));
        first += sig_len;
        // 科学计数法时复制指数部分
        if (!use_fixed) {
            ptrdiff_t exp_len = exp_end - exp_start;
            if (end - first < exp_len) {
                return {end, std::errc::value_too_large};
            }
            std::memcpy(first, exp_start, static_cast<std::size_t>(exp_len));
            first += exp_len;
        }
        return {first, std::errc{}};
    }

    template <floating_to_chars_overload Overload, typename Floating>
    RAINY_NODISCARD to_chars_result floating_to_chars(char *first, char *const end, Floating value, const chars_format fmt,
                                                      const int precision) noexcept {
        utility::adl_verify_range(first, end);
        if constexpr (Overload == floating_to_chars_overload::plain) {
            core::implements::stl_internal_check(fmt == chars_format{}); // plain overload must pass chars_format{} internally
        } else {
            assert(fmt == chars_format::general || fmt == chars_format::scientific || fmt == chars_format::fixed ||
                   fmt == chars_format::hex && "invalid format in to_chars()");
        }
        using traits = utility::floating_type_traits<Floating>;
        using uint_type = typename traits::uint_type;
        uint_type uint_value = core::builtin::bit_cast<uint_type>(value);
        const bool was_negative = (uint_value & traits::shifted_sign_mask) != 0;
        if (was_negative) { // sign bit detected; write minus sign and clear sign bit
            if (first == end) {
                return {end, std::errc::value_too_large};
            }

            *first++ = '-';

            uint_value &= ~traits::shifted_sign_mask;
            value = core::builtin::bit_cast<Floating>(uint_value);
        }
        if ((uint_value & traits::shifted_exponent_mask) == traits::shifted_exponent_mask) {
            const char *str;
            std::size_t len;
            const uint_type mantissa = uint_value & traits::denormal_mantissa_mask;
            if (mantissa == 0) {
                str = "inf";
                len = 3;
            } else if (was_negative && mantissa == traits::special_nan_mantissa_mask) {
                str = "nan(ind)";
                len = 8;
            } else if ((mantissa & traits::special_nan_mantissa_mask) != 0) {
                str = "nan";
                len = 3;
            } else {
                str = "nan(snan)";
                len = 9;
            }
            if (end - first < static_cast<ptrdiff_t>(len)) {
                return {end, std::errc::value_too_large};
            }
            std::memcpy(first, str, len);
            return {first + len, std::errc{}};
        }
        if constexpr (Overload == floating_to_chars_overload::plain) {
            return implements::floating_to_chars_ryu(first, end, value, chars_format{});
        } else if constexpr (Overload == floating_to_chars_overload::format_only) {
            if (fmt == chars_format::hex) {
                return implements::floating_to_chars_hex_shortest(first, end, value);
            }
            return implements::floating_to_chars_ryu(first, end, value, fmt);
        } else if constexpr (Overload == floating_to_chars_overload::format_precision) {
            switch (fmt) {
                case chars_format::scientific:
                    return implements::floating_to_chars_scientific_precision(first, end, value, precision);
                case chars_format::fixed:
                    return implements::floating_to_chars_fixed_precision(first, end, value, precision);
                case chars_format::general:
                    return implements::floating_to_chars_general_precision(first, end, value, precision);
                case chars_format::hex:
                default:
                    return implements::floating_to_chars_hex_precision(first, end, value, precision);
            }
        }
    }
}

namespace rainy::foundation::text {
    to_chars_result to_chars(char *begin, char *end, const float value) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::plain>(begin, end, value, chars_format{}, 0);
    }

    to_chars_result to_chars(char *begin, char *end, const float value, chars_format fmt) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::format_only>(begin, end, value, fmt, 0);
    }

    to_chars_result to_chars(char *begin, char *end, const float value, chars_format fmt, int precision) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::format_precision>(begin, end, value, fmt,
                                                                                                       precision);
    }

    to_chars_result to_chars(char *begin, char *end, double value) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::plain>(begin, end, value, chars_format{}, 0);
    }

    to_chars_result to_chars(char *begin, char *end, double value, chars_format fmt) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::format_only>(begin, end, value, fmt, 0);
    }

    to_chars_result to_chars(char *begin, char *end, double value, chars_format fmt, int precision) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::format_precision>(begin, end, value, fmt,
                                                                                                       precision);
    }

    to_chars_result to_chars(char *begin, char *end, long double value) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::plain>(begin, end, static_cast<double>(value),
                                                                                            chars_format{}, 0);
    }

    to_chars_result to_chars(char *begin, char *end, long double value, chars_format fmt) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::format_only>(begin, end,
                                                                                                  static_cast<double>(value), fmt, 0);
    }

    to_chars_result to_chars(char *begin, char *end, long double value, chars_format fmt, int precision) noexcept {
        return implements::floating_to_chars<implements::floating_to_chars_overload::format_precision>(
            begin, end, static_cast<double>(value), fmt, precision);
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
