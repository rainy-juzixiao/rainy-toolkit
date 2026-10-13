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
#include <rainy/core/platform.hpp>

#if defined(__SIZEOF_INT128__)
#include <rainy/core/container/compressed_pair.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/implements/ryu_generic_128.hpp>
#include <rainy/core/text/implements/ryu_generic_128_table.hpp>

namespace rainy::core::text::implements::ryu {
    using uint128_t = unsigned __int128;

    inline constexpr std::int32_t FD128_EXCEPTIONAL_EXPONENT = 0x7FFFFFFF;

    RAINY_NODISCARD inline std::uint32_t generic_pow5bits(const std::int32_t e) noexcept {
        return static_cast<std::uint32_t>(((static_cast<std::uint64_t>(e) * 163391164108059ull) >> 46) + 1);
    }

    inline void mul_128_256_shift(const std::uint64_t *const a, const std::uint64_t *const b, const std::uint32_t shift,
                                  const std::uint32_t corr, std::uint64_t *const result) noexcept {
        const uint128_t b00 = static_cast<uint128_t>(a[0]) * b[0];
        const uint128_t b01 = static_cast<uint128_t>(a[0]) * b[1];
        const uint128_t b02 = static_cast<uint128_t>(a[0]) * b[2];
        const uint128_t b03 = static_cast<uint128_t>(a[0]) * b[3];
        const uint128_t b10 = static_cast<uint128_t>(a[1]) * b[0];
        const uint128_t b11 = static_cast<uint128_t>(a[1]) * b[1];
        const uint128_t b12 = static_cast<uint128_t>(a[1]) * b[2];
        const uint128_t b13 = static_cast<uint128_t>(a[1]) * b[3];

        const uint128_t s0 = b00;
        const uint128_t s1 = b01 + b10;
        const uint128_t c1 = s1 < b01;
        const uint128_t s2 = b02 + b11;
        const uint128_t c2 = s2 < b02;
        const uint128_t s3 = b03 + b12;
        const uint128_t c3 = s3 < b03;

        const uint128_t p0 = s0 + (s1 << 64);
        const uint128_t d0 = p0 < b00;
        const uint128_t q1 = s2 + (s1 >> 64) + (s3 << 64);
        const uint128_t d1 = q1 < s2;
        const uint128_t p1 = q1 + (c1 << 64) + d0;
        const uint128_t d2 = p1 < q1;
        const uint128_t p2 = b13 + (s3 >> 64) + c2 + (c3 << 64) + d1 + d2;

        if (shift < 128) {
            const uint128_t r0 = corr + ((p0 >> shift) | (p1 << (128 - shift)));
            const uint128_t r1 = ((p1 >> shift) | (p2 << (128 - shift))) + (r0 < corr);
            result[0] = static_cast<std::uint64_t>(r0);
            result[1] = static_cast<std::uint64_t>(r0 >> 64);
            result[2] = static_cast<std::uint64_t>(r1);
            result[3] = static_cast<std::uint64_t>(r1 >> 64);
        } else if (shift == 128) {
            const uint128_t r0 = corr + p1;
            const uint128_t r1 = p2 + (r0 < corr);
            result[0] = static_cast<std::uint64_t>(r0);
            result[1] = static_cast<std::uint64_t>(r0 >> 64);
            result[2] = static_cast<std::uint64_t>(r1);
            result[3] = static_cast<std::uint64_t>(r1 >> 64);
        } else {
            const uint128_t r0 = corr + ((p1 >> (shift - 128)) | (p2 << (256 - shift)));
            const uint128_t r1 = (p2 >> (shift - 128)) + (r0 < corr);
            result[0] = static_cast<std::uint64_t>(r0);
            result[1] = static_cast<std::uint64_t>(r0 >> 64);
            result[2] = static_cast<std::uint64_t>(r1);
            result[3] = static_cast<std::uint64_t>(r1 >> 64);
        }
    }

    inline void generic_compute_pow5(const std::uint32_t i, std::uint64_t *const result) noexcept {
        const std::uint32_t base = i / POW5_TABLE_SIZE;
        const std::uint32_t base2 = base * POW5_TABLE_SIZE;
        const std::uint64_t *const mul = GENERIC_POW5_SPLIT[base];
        if (i == base2) {
            result[0] = mul[0];
            result[1] = mul[1];
            result[2] = mul[2];
            result[3] = mul[3];
        } else {
            const std::uint32_t offset = i - base2;
            const std::uint64_t *const m = GENERIC_POW5_TABLE[offset];
            const std::uint32_t delta = generic_pow5bits(static_cast<std::int32_t>(i)) -
                                        generic_pow5bits(static_cast<std::int32_t>(base2));
            const std::uint32_t corr = static_cast<std::uint32_t>((POW5_ERRORS[i / 32] >> (2 * (i % 32))) & 3);
            mul_128_256_shift(m, mul, delta, corr, result);
        }
    }

    inline void generic_compute_inv_pow5(const std::uint32_t i, std::uint64_t *const result) noexcept {
        const std::uint32_t base = (i + POW5_TABLE_SIZE - 1) / POW5_TABLE_SIZE;
        const std::uint32_t base2 = base * POW5_TABLE_SIZE;
        const std::uint64_t *const mul = GENERIC_POW5_INV_SPLIT[base];
        if (i == base2) {
            result[0] = mul[0] + 1;
            result[1] = mul[1];
            result[2] = mul[2];
            result[3] = mul[3];
        } else {
            const std::uint32_t offset = base2 - i;
            const std::uint64_t *const m = GENERIC_POW5_TABLE[offset];
            const std::uint32_t delta = generic_pow5bits(static_cast<std::int32_t>(base2)) -
                                        generic_pow5bits(static_cast<std::int32_t>(i));
            const std::uint32_t corr =
                static_cast<std::uint32_t>((POW5_INV_ERRORS[i / 32] >> (2 * (i % 32))) & 3) + 1;
            mul_128_256_shift(m, mul, delta, corr, result);
        }
    }

    RAINY_NODISCARD inline std::uint32_t generic_pow5_factor(uint128_t value) noexcept {
        for (std::uint32_t count = 0; value > 0; ++count) {
            if (value % 5 != 0) {
                return count;
            }
            value /= 5;
        }
        return 0;
    }

    RAINY_NODISCARD inline bool generic_multiple_of_power_of_5(const uint128_t value, const std::uint32_t p) noexcept {
        return generic_pow5_factor(value) >= p;
    }

    RAINY_NODISCARD inline bool generic_multiple_of_power_of_2(const uint128_t value, const std::uint32_t p) noexcept {
        return (value & ((static_cast<uint128_t>(1) << p) - 1)) == 0;
    }

    RAINY_NODISCARD inline uint128_t generic_mul_shift(const uint128_t m, const std::uint64_t *const mul,
                                                       const std::int32_t j) noexcept {
        std::uint64_t a[2];
        a[0] = static_cast<std::uint64_t>(m);
        a[1] = static_cast<std::uint64_t>(m >> 64);
        std::uint64_t result[4];
        mul_128_256_shift(a, mul, static_cast<std::uint32_t>(j), 0, result);
        return (static_cast<uint128_t>(result[1]) << 64) | result[0];
    }

    RAINY_NODISCARD inline std::uint32_t generic_decimal_length(const uint128_t v) noexcept {
        static const uint128_t LARGEST_POW10 = (static_cast<uint128_t>(5421010862427522170ull) << 64) | 687399551400673280ull;
        uint128_t p10 = LARGEST_POW10;
        for (std::uint32_t i = 39; i > 0; i--) {
            if (v >= p10) {
                return i;
            }
            p10 /= 10;
        }
        return 1;
    }

    RAINY_NODISCARD inline std::uint32_t generic_log10_pow2(const std::int32_t e) noexcept {
        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(e) * 169464822037455ull) >> 49);
    }

    RAINY_NODISCARD inline std::uint32_t generic_log10_pow5(const std::int32_t e) noexcept {
        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(e) * 196742565691928ull) >> 48);
    }

    floating_decimal_128 generic_binary_to_decimal(const uint128_t bits, const std::uint32_t mantissa_bits,
                                                   const std::uint32_t exponent_bits,
                                                   const bool explicit_leading_bit) noexcept {
        const std::uint32_t bias = (1u << (exponent_bits - 1)) - 1;
        const bool ieee_sign = ((bits >> (mantissa_bits + exponent_bits)) & 1) != 0;
        const uint128_t ieee_mantissa = bits & ((static_cast<uint128_t>(1) << mantissa_bits) - 1);
        const std::uint32_t ieee_exponent =
            static_cast<std::uint32_t>((bits >> mantissa_bits) & ((static_cast<uint128_t>(1) << exponent_bits) - 1u));

        if (ieee_exponent == 0 && ieee_mantissa == 0) {
            return floating_decimal_128{0, 0, ieee_sign};
        }
        if (ieee_exponent == ((1u << exponent_bits) - 1u)) {
            const uint128_t mantissa = explicit_leading_bit
                                           ? ieee_mantissa & ((static_cast<uint128_t>(1) << (mantissa_bits - 1)) - 1)
                                           : ieee_mantissa;
            return floating_decimal_128{mantissa, FD128_EXCEPTIONAL_EXPONENT, ieee_sign};
        }

        std::int32_t e2;
        uint128_t m2;
        if (explicit_leading_bit) {
            if (ieee_exponent == 0) {
                e2 = 1 - static_cast<std::int32_t>(bias) - static_cast<std::int32_t>(mantissa_bits) + 1 - 2;
            } else {
                e2 = static_cast<std::int32_t>(ieee_exponent) - static_cast<std::int32_t>(bias) -
                     static_cast<std::int32_t>(mantissa_bits) + 1 - 2;
            }
            m2 = ieee_mantissa;
        } else {
            if (ieee_exponent == 0) {
                e2 = 1 - static_cast<std::int32_t>(bias) - static_cast<std::int32_t>(mantissa_bits) - 2;
                m2 = ieee_mantissa;
            } else {
                e2 = static_cast<std::int32_t>(ieee_exponent) - static_cast<std::int32_t>(bias) -
                     static_cast<std::int32_t>(mantissa_bits) - 2;
                m2 = (static_cast<uint128_t>(1) << mantissa_bits) | ieee_mantissa;
            }
        }
        const bool even = (m2 & 1) == 0;
        const bool accept_bounds = even;

        const uint128_t mv = 4 * m2;
        const std::uint32_t mm_shift =
            (ieee_mantissa != (explicit_leading_bit ? static_cast<uint128_t>(1) << (mantissa_bits - 1) : 0)) ||
            (ieee_exponent == 0);

        uint128_t vr, vp, vm;
        std::int32_t e10;
        bool vm_is_trailing_zeros = false;
        bool vr_is_trailing_zeros = false;
        if (e2 >= 0) {
            const std::uint32_t q = generic_log10_pow2(e2) - (e2 > 3);
            e10 = static_cast<std::int32_t>(q);
            const std::int32_t k = FLOAT_128_POW5_INV_BITCOUNT + static_cast<std::int32_t>(generic_pow5bits(static_cast<std::int32_t>(q))) - 1;
            const std::int32_t i = -e2 + static_cast<std::int32_t>(q) + k;
            std::uint64_t pow5[4];
            generic_compute_inv_pow5(q, pow5);
            vr = generic_mul_shift(4 * m2, pow5, i);
            vp = generic_mul_shift(4 * m2 + 2, pow5, i);
            vm = generic_mul_shift(4 * m2 - 1 - mm_shift, pow5, i);
            if (q <= 55) {
                if (mv % 5 == 0) {
                    vr_is_trailing_zeros = generic_multiple_of_power_of_5(mv, q - 1);
                } else if (accept_bounds) {
                    vm_is_trailing_zeros = generic_multiple_of_power_of_5(mv - 1 - mm_shift, q);
                } else {
                    vp -= generic_multiple_of_power_of_5(mv + 2, q);
                }
            }
        } else {
            const std::uint32_t q = generic_log10_pow5(-e2) - (-e2 > 1);
            e10 = static_cast<std::int32_t>(q) + e2;
            const std::int32_t i = -e2 - static_cast<std::int32_t>(q);
            const std::int32_t k = static_cast<std::int32_t>(generic_pow5bits(i)) - FLOAT_128_POW5_BITCOUNT;
            const std::int32_t j = static_cast<std::int32_t>(q) - k;
            std::uint64_t pow5[4];
            generic_compute_pow5(static_cast<std::uint32_t>(i), pow5);
            vr = generic_mul_shift(4 * m2, pow5, j);
            vp = generic_mul_shift(4 * m2 + 2, pow5, j);
            vm = generic_mul_shift(4 * m2 - 1 - mm_shift, pow5, j);
            if (q <= 1) {
                vr_is_trailing_zeros = true;
                if (accept_bounds) {
                    vm_is_trailing_zeros = mm_shift == 1;
                } else {
                    --vp;
                }
            } else if (q < 127) {
                vr_is_trailing_zeros = generic_multiple_of_power_of_2(mv, q - 1);
            }
        }

        std::uint32_t removed = 0;
        std::uint8_t last_removed_digit = 0;
        uint128_t output;

        while (vp / 10 > vm / 10) {
            vm_is_trailing_zeros &= vm % 10 == 0;
            vr_is_trailing_zeros &= last_removed_digit == 0;
            last_removed_digit = static_cast<std::uint8_t>(vr % 10);
            vr /= 10;
            vp /= 10;
            vm /= 10;
            ++removed;
        }
        if (vm_is_trailing_zeros) {
            while (vm % 10 == 0) {
                vr_is_trailing_zeros &= last_removed_digit == 0;
                last_removed_digit = static_cast<std::uint8_t>(vr % 10);
                vr /= 10;
                vp /= 10;
                vm /= 10;
                ++removed;
            }
        }
        if (vr_is_trailing_zeros && (last_removed_digit == 5) && (vr % 2 == 0)) {
            last_removed_digit = 4;
        }
        output = vr + ((vr == vm && (!accept_bounds || !vm_is_trailing_zeros)) || (last_removed_digit >= 5));
        const std::int32_t exp = e10 + static_cast<std::int32_t>(removed);

        return floating_decimal_128{output, exp, ieee_sign};
    }

    floating_decimal_128 long_double_to_fd128(const long double value) noexcept {
        uint128_t bits = 0;
        std::memcpy(&bits, &value, sizeof(long double));
#if RAINY_LONG_DOUBLE_IS_BINARY128
        return generic_binary_to_decimal(bits, 112, 15, false);
#else
        return generic_binary_to_decimal(bits, 64, 15, true);
#endif
    }
}
namespace rainy::core::text::implements::ryu {
    RAINY_NODISCARD inline uint128_t pow10_128(std::uint32_t e) noexcept {
        uint128_t r = 1;
        while (e-- > 0) {
            r *= 10;
        }
        return r;
    }

    template <typename CharType>
    RAINY_NODISCARD container::compressed_pair<CharType *, std::errc> generic_to_chars(CharType *const first,
                                                                                       CharType *const end,
                                                                                       const floating_decimal_128 &value,
                                                                                       const chars_format fmt,
                                                                                       const long double original) noexcept {
        if (value.exponent == FD128_EXCEPTIONAL_EXPONENT) {
            const char *str;
            std::size_t len;
            if (value.mantissa != 0) {
                str = "nan";
                len = 3;
            } else if (value.sign) {
                str = "-inf";
                len = 4;
            } else {
                str = "inf";
                len = 3;
            }
            if (end - first < static_cast<std::ptrdiff_t>(len)) {
                return {end, std::errc::value_too_large};
            }
            for (std::size_t i = 0; i < len; ++i) {
                first[i] = static_cast<CharType>(str[i]);
            }
            return {first + len, std::errc{}};
        }

        CharType *p = first;
        if (value.sign) {
            if (p == end) {
                return {end, std::errc::value_too_large};
            }
            *p++ = static_cast<CharType>('-');
        }

        const uint128_t output = value.mantissa;
        if (output == 0) {
            if (end - p < 1) {
                return {end, std::errc::value_too_large};
            }
            *p++ = static_cast<CharType>('0');
            return {p, std::errc{}};
        }
        const std::uint32_t olength = generic_decimal_length(output);
        const std::int32_t scientific_exponent = value.exponent + static_cast<std::int32_t>(olength) - 1;

        chars_format resolved = fmt;
        if (resolved == chars_format{}) {
            std::int32_t lower;
            std::int32_t upper;
            if (olength == 1) {
                lower = -3;
                upper = 4;
            } else {
                lower = -static_cast<std::int32_t>(olength + 3);
                upper = 5;
            }
            resolved = (lower <= value.exponent && value.exponent <= upper) ? chars_format::fixed : chars_format::scientific;
        } else if (resolved == chars_format::general) {
            resolved = (-4 <= scientific_exponent && scientific_exponent < 6) ? chars_format::fixed : chars_format::scientific;
        }

        CharType digits[64];
        std::uint32_t digit_count = 0;
        {
            uint128_t tmp = output;
            while (tmp > 0) {
                digits[digit_count++] = static_cast<CharType>('0' + static_cast<int>(tmp % 10));
                tmp /= 10;
            }
            for (std::uint32_t i = 0; i < digit_count / 2; ++i) {
                const CharType t = digits[i];
                digits[i] = digits[digit_count - 1 - i];
                digits[digit_count - 1 - i] = t;
            }
        }
        std::int32_t fixed_exponent = value.exponent;
        if (resolved == chars_format::fixed && original == original && original != 0) {
            const long double magnitude = original < 0 ? -original : original;
            if (magnitude < 1.0e26L) {
                const uint128_t integer_value = static_cast<uint128_t>(magnitude);
                if (static_cast<long double>(integer_value) == magnitude) {
                    std::uint32_t count = 0;
                    uint128_t tmp = integer_value;
                    while (tmp > 0) {
                        digits[count++] = static_cast<CharType>('0' + static_cast<int>(tmp % 10));
                        tmp /= 10;
                    }
                    for (std::uint32_t i = 0; i < count / 2; ++i) {
                        const CharType t = digits[i];
                        digits[i] = digits[count - 1 - i];
                        digits[count - 1 - i] = t;
                    }
                    digit_count = count;
                    fixed_exponent = 0;
                }
            }
        }

        if (resolved == chars_format::scientific) {
            std::uint32_t exp_length = 1;
            for (std::int32_t t = scientific_exponent < 0 ? -scientific_exponent : scientific_exponent; t >= 10; t /= 10) {
                ++exp_length;
            }
            if (exp_length < 2) {
                exp_length = 2;
            }
            const std::uint32_t total = digit_count + (digit_count > 1 ? 1u : 0u) + 2u + exp_length;
            if (end - p < static_cast<std::ptrdiff_t>(total)) {
                return {end, std::errc::value_too_large};
            }
            *p++ = digits[0];
            if (digit_count > 1) {
                *p++ = static_cast<CharType>('.');
                for (std::uint32_t i = 1; i < digit_count; ++i) {
                    *p++ = digits[i];
                }
            }
            *p++ = static_cast<CharType>('e');
            std::int32_t exp = scientific_exponent;
            if (exp < 0) {
                *p++ = static_cast<CharType>('-');
                exp = -exp;
            } else {
                *p++ = static_cast<CharType>('+');
            }
            for (std::uint32_t i = 0; i < exp_length; ++i) {
                p[exp_length - 1 - i] = static_cast<CharType>('0' + exp % 10);
                exp /= 10;
            }
            p += exp_length;
            return {p, std::errc{}};
        }

        const std::int32_t whole_digits = static_cast<std::int32_t>(digit_count) + fixed_exponent;
        std::uint32_t total;
        if (fixed_exponent >= 0) {
            total = static_cast<std::uint32_t>(whole_digits);
        } else if (whole_digits > 0) {
            total = digit_count + 1;
        } else {
            total = static_cast<std::uint32_t>(2 - fixed_exponent);
        }
        if (end - p < static_cast<std::ptrdiff_t>(total)) {
            return {end, std::errc::value_too_large};
        }
        if (fixed_exponent >= 0) {
            for (std::uint32_t i = 0; i < digit_count; ++i) {
                *p++ = digits[i];
            }
            for (std::int32_t i = 0; i < fixed_exponent; ++i) {
                *p++ = static_cast<CharType>('0');
            }
        } else if (whole_digits > 0) {
            for (std::int32_t i = 0; i < whole_digits; ++i) {
                *p++ = digits[i];
            }
            *p++ = static_cast<CharType>('.');
            for (std::uint32_t i = static_cast<std::uint32_t>(whole_digits); i < digit_count; ++i) {
                *p++ = digits[i];
            }
        } else {
            *p++ = static_cast<CharType>('0');
            *p++ = static_cast<CharType>('.');
            for (std::int32_t i = 0; i < -whole_digits; ++i) {
                *p++ = static_cast<CharType>('0');
            }
            for (std::uint32_t i = 0; i < digit_count; ++i) {
                *p++ = digits[i];
            }
        }
        return {p, std::errc{}};
    }

    template container::compressed_pair<char *, std::errc> generic_to_chars<char>(char *, char *,
                                                                                 const floating_decimal_128 &,
                                                                                 chars_format, long double) noexcept;
}
#endif
