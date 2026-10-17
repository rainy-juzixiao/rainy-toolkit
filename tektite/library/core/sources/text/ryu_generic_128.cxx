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

    inline constexpr std::uint32_t big_decimal_max_limbs = 1400;
    inline constexpr std::uint32_t big_decimal_base = 1000000000u;

    struct big_decimal {
        std::uint32_t limbs[big_decimal_max_limbs];
        std::uint32_t count;
    };

    inline void big_decimal_trim(big_decimal &v) noexcept {
        while (v.count > 0 && v.limbs[v.count - 1] == 0) {
            --v.count;
        }
    }

    inline void big_decimal_from_u128(big_decimal &v, uint128_t value) noexcept {
        v.count = 0;
        while (value != 0) {
            v.limbs[v.count++] = static_cast<std::uint32_t>(value % big_decimal_base);
            value /= big_decimal_base;
        }
    }

    inline void big_decimal_mul_small(big_decimal &v, const std::uint32_t factor) noexcept {
        if (factor == 1) {
            return;
        }
        std::uint64_t carry = 0;
        for (std::uint32_t i = 0; i < v.count; ++i) {
            const std::uint64_t product = static_cast<std::uint64_t>(v.limbs[i]) * factor + carry;
            v.limbs[i] = static_cast<std::uint32_t>(product % big_decimal_base);
            carry = product / big_decimal_base;
        }
        while (carry != 0 && v.count < big_decimal_max_limbs) {
            v.limbs[v.count++] = static_cast<std::uint32_t>(carry % big_decimal_base);
            carry /= big_decimal_base;
        }
    }

    inline void big_decimal_add_small(big_decimal &v, const std::uint32_t addend) noexcept {
        std::uint64_t carry = addend;
        std::uint32_t i = 0;
        while (carry != 0 && i < v.count) {
            const std::uint64_t sum = static_cast<std::uint64_t>(v.limbs[i]) + carry;
            v.limbs[i] = static_cast<std::uint32_t>(sum % big_decimal_base);
            carry = sum / big_decimal_base;
            ++i;
        }
        while (carry != 0 && v.count < big_decimal_max_limbs) {
            v.limbs[v.count++] = static_cast<std::uint32_t>(carry % big_decimal_base);
            carry /= big_decimal_base;
        }
    }

    inline void big_decimal_mul_pow2(big_decimal &v, std::uint32_t exponent) noexcept {
        constexpr std::uint32_t chunk = 536870912u;
        while (exponent >= 29) {
            big_decimal_mul_small(v, chunk);
            exponent -= 29;
        }
        big_decimal_mul_small(v, 1u << exponent);
    }

    inline void big_decimal_mul_pow5(big_decimal &v, std::uint32_t exponent) noexcept {
        constexpr std::uint32_t five13 = 1220703125u;
        while (exponent >= 13) {
            big_decimal_mul_small(v, five13);
            exponent -= 13;
        }
        std::uint32_t factor = 1;
        for (std::uint32_t i = 0; i < exponent; ++i) {
            factor *= 5;
        }
        big_decimal_mul_small(v, factor);
    }

    inline void big_decimal_mul_pow10(big_decimal &v, std::uint32_t exponent) noexcept {
        while (exponent >= 9) {
            big_decimal_mul_small(v, big_decimal_base);
            exponent -= 9;
        }
        static constexpr std::uint32_t small_powers[] = {1u,      10u,      100u,      1000u,     10000u,
                                                        100000u, 1000000u, 10000000u, 100000000u};
        big_decimal_mul_small(v, small_powers[exponent]);
    }

    RAINY_NODISCARD inline std::uint32_t big_decimal_digit_count(const big_decimal &v) noexcept {
        if (v.count == 0) {
            return 1;
        }
        std::uint32_t top = v.limbs[v.count - 1];
        std::uint32_t digits = 0;
        while (top != 0) {
            top /= 10;
            ++digits;
        }
        return (v.count - 1) * 9 + digits;
    }

    inline void big_decimal_to_digits(const big_decimal &v, char *const out) noexcept {
        if (v.count == 0) {
            out[0] = '0';
            return;
        }
        char scratch[10];
        std::uint32_t position = 0;
        std::uint32_t top = v.limbs[v.count - 1];
        std::uint32_t length = 0;
        while (top != 0) {
            scratch[length++] = static_cast<char>('0' + top % 10);
            top /= 10;
        }
        while (length > 0) {
            out[position++] = scratch[--length];
        }
        for (std::uint32_t i = v.count - 1; i > 0; --i) {
            std::uint32_t limb = v.limbs[i - 1];
            for (std::uint32_t k = 0; k < 9; ++k) {
                scratch[8 - k] = static_cast<char>('0' + limb % 10);
                limb /= 10;
            }
            for (std::uint32_t k = 0; k < 9; ++k) {
                out[position++] = scratch[k];
            }
        }
    }

    inline void big_decimal_div_pow10(big_decimal &v, const std::uint32_t exponent) noexcept {
        const std::uint32_t whole_limbs = exponent / 9;
        const std::uint32_t remainder_digits = exponent % 9;
        if (whole_limbs >= v.count) {
            v.count = 0;
            return;
        }
        for (std::uint32_t i = 0; i + whole_limbs < v.count; ++i) {
            v.limbs[i] = v.limbs[i + whole_limbs];
        }
        v.count -= whole_limbs;
        if (remainder_digits != 0) {
            std::uint32_t divisor = 1;
            for (std::uint32_t i = 0; i < remainder_digits; ++i) {
                divisor *= 10;
            }
            std::uint64_t remainder = 0;
            for (std::uint32_t i = v.count; i > 0; --i) {
                const std::uint64_t current = remainder * big_decimal_base + v.limbs[i - 1];
                v.limbs[i - 1] = static_cast<std::uint32_t>(current / divisor);
                remainder = current % divisor;
            }
        }
        big_decimal_trim(v);
    }

    inline void big_decimal_mod_pow10(const big_decimal &v, const std::uint32_t exponent, big_decimal &out) noexcept {
        const std::uint32_t whole_limbs = exponent / 9;
        const std::uint32_t remainder_digits = exponent % 9;
        if (whole_limbs >= v.count) {
            out = v;
            return;
        }
        out.count = whole_limbs;
        for (std::uint32_t i = 0; i < whole_limbs; ++i) {
            out.limbs[i] = v.limbs[i];
        }
        if (remainder_digits != 0) {
            std::uint32_t divisor = 1;
            for (std::uint32_t i = 0; i < remainder_digits; ++i) {
                divisor *= 10;
            }
            out.limbs[out.count++] = v.limbs[whole_limbs] % divisor;
        }
        big_decimal_trim(out);
    }

    inline void big_decimal_div_pow10_round(big_decimal &v, const std::uint32_t exponent) noexcept {
        if (exponent == 0) {
            return;
        }
        big_decimal remainder{};
        big_decimal_mod_pow10(v, exponent, remainder);
        big_decimal_div_pow10(v, exponent);
        if (remainder.count == 0) {
            return;
        }
        const std::uint32_t digits = big_decimal_digit_count(remainder);
        if (digits < exponent) {
            return;
        }
        const std::uint32_t top = remainder.limbs[remainder.count - 1];
        std::uint32_t top_digits = 0;
        for (std::uint32_t t = top; t != 0; t /= 10) {
            ++top_digits;
        }
        std::uint32_t leading_power = 1;
        for (std::uint32_t i = 1; i < top_digits; ++i) {
            leading_power *= 10;
        }
        const std::uint32_t leading = top / leading_power;
        bool sticky = top != leading * leading_power;
        for (std::uint32_t i = 0; i + 1 < remainder.count && !sticky; ++i) {
            sticky = remainder.limbs[i] != 0;
        }
        const bool round_up = leading > 5 || (leading == 5 && (sticky || (v.count > 0 && v.limbs[0] % 2 != 0)));
        if (round_up) {
            big_decimal_add_small(v, 1);
        }
    }

    inline void long_double_decompose(const long double value, uint128_t &mantissa, std::int32_t &binary_exponent) noexcept {
        uint128_t bits = 0;
        std::memcpy(&bits, &value, sizeof(long double));
#if RAINY_LONG_DOUBLE_IS_BINARY128
        constexpr std::uint32_t mantissa_bits = 112;
        constexpr std::int32_t exponent_bias = 16383;
        const std::uint32_t ieee_exponent = static_cast<std::uint32_t>((bits >> mantissa_bits) & 0x7fffu);
        const uint128_t ieee_mantissa = bits & ((static_cast<uint128_t>(1) << mantissa_bits) - 1);
        if (ieee_exponent == 0) {
            mantissa = ieee_mantissa;
            binary_exponent = 1 - exponent_bias - static_cast<std::int32_t>(mantissa_bits);
        } else {
            mantissa = (static_cast<uint128_t>(1) << mantissa_bits) | ieee_mantissa;
            binary_exponent = static_cast<std::int32_t>(ieee_exponent) - exponent_bias - static_cast<std::int32_t>(mantissa_bits);
        }
#else
        constexpr std::uint32_t mantissa_bits = 64;
        constexpr std::int32_t exponent_bias = 16383;
        const std::uint32_t ieee_exponent = static_cast<std::uint32_t>((bits >> 64) & 0x7fffu);
        mantissa = bits & ((static_cast<uint128_t>(1) << 64) - 1);
        if (ieee_exponent == 0) {
            binary_exponent = 1 - exponent_bias - static_cast<std::int32_t>(mantissa_bits) + 1;
        } else {
            binary_exponent =
                static_cast<std::int32_t>(ieee_exponent) - exponent_bias - static_cast<std::int32_t>(mantissa_bits) + 1;
        }
#endif
    }

    RAINY_NODISCARD inline std::int32_t big_decimal_decimal_exponent(const big_decimal &v) noexcept {
        return static_cast<std::int32_t>(big_decimal_digit_count(v)) - 1;
    }

    inline void long_double_exact_decimal(const long double value, big_decimal &numerator, std::uint32_t &scale) noexcept {
        uint128_t mantissa = 0;
        std::int32_t binary_exponent = 0;
        long_double_decompose(value, mantissa, binary_exponent);
        big_decimal_from_u128(numerator, mantissa);
        if (binary_exponent >= 0) {
            big_decimal_mul_pow2(numerator, static_cast<std::uint32_t>(binary_exponent));
            scale = 0;
        } else {
            big_decimal_mul_pow5(numerator, static_cast<std::uint32_t>(-binary_exponent));
            scale = static_cast<std::uint32_t>(-binary_exponent);
        }
    }

    inline void long_double_round_to_scale(big_decimal &numerator, std::uint32_t &scale, const std::uint32_t target) noexcept {
        if (scale > target) {
            big_decimal_div_pow10_round(numerator, scale - target);
            scale = target;
        } else if (scale < target) {
            big_decimal_mul_pow10(numerator, target - scale);
            scale = target;
        }
    }

    template <typename CharType>
    RAINY_NODISCARD CharType *emit_fixed_digits(CharType *first, const big_decimal &numerator, const std::uint32_t precision) noexcept {
        const std::uint32_t length = big_decimal_digit_count(numerator);
        big_decimal_to_digits(numerator, reinterpret_cast<char *>(first));
        if (precision == 0) {
            return first + length;
        }
        if (length > precision) {
            CharType *const point = first + (length - precision);
            std::memmove(point + 1, point, precision * sizeof(CharType));
            *point = static_cast<CharType>('.');
            return first + length + 1;
        }
        const std::uint32_t zeros = precision - length;
        const std::uint32_t shift = zeros + 2;
        std::memmove(first + shift, first, length * sizeof(CharType));
        first[0] = static_cast<CharType>('0');
        first[1] = static_cast<CharType>('.');
        for (std::uint32_t i = 0; i < zeros; ++i) {
            first[2 + i] = static_cast<CharType>('0');
        }
        return first + shift + length;
    }

    template <typename CharType>
    RAINY_NODISCARD CharType *emit_scientific_digits(CharType *first, const big_decimal &numerator, const std::uint32_t precision,
                                                     std::int32_t exponent) noexcept {
        const std::uint32_t length = big_decimal_digit_count(numerator);
        CharType *const digits = first;
        big_decimal_to_digits(numerator, reinterpret_cast<char *>(digits));
        CharType *p = digits + length;
        if (precision > 0) {
            std::memmove(digits + 2, digits + 1, (length - 1) * sizeof(CharType));
            digits[1] = static_cast<CharType>('.');
            p = digits + length + 1;
            const std::uint32_t fraction = length - 1;
            for (std::uint32_t i = fraction; i < precision; ++i) {
                *p++ = static_cast<CharType>('0');
            }
        }
        *p++ = static_cast<CharType>('e');
        if (exponent < 0) {
            *p++ = static_cast<CharType>('-');
            exponent = -exponent;
        } else {
            *p++ = static_cast<CharType>('+');
        }
        CharType buffer[8];
        std::uint32_t written = 0;
        do {
            buffer[written++] = static_cast<CharType>('0' + exponent % 10);
            exponent /= 10;
        } while (exponent != 0);
        while (written < 2) {
            buffer[written++] = static_cast<CharType>('0');
        }
        while (written > 0) {
            *p++ = buffer[--written];
        }
        return p;
    }

    template <typename CharType>
    RAINY_NODISCARD CharType *strip_trailing_zeros(CharType *const first, CharType *const last) noexcept {
        const std::ptrdiff_t length = last - first;
        std::ptrdiff_t exponent = -1;
        for (std::ptrdiff_t i = length - 1; i >= 0; --i) {
            if (first[i] == static_cast<CharType>('e')) {
                exponent = i;
                break;
            }
        }
        const std::ptrdiff_t end = exponent < 0 ? length : exponent;
        std::ptrdiff_t dot = -1;
        for (std::ptrdiff_t i = end - 1; i >= 0; --i) {
            if (first[i] == static_cast<CharType>('.')) {
                dot = i;
                break;
            }
        }
        if (dot < 0) {
            return last;
        }
        std::ptrdiff_t point = end;
        while (point > 0 && first[point - 1] == static_cast<CharType>('0')) {
            --point;
        }
        if (point < dot + 1) {
            return last;
        }
        if (point == dot + 1) {
            point = dot;
        }
        const std::ptrdiff_t tail = length - end;
        std::memmove(first + point, first + end, static_cast<std::size_t>(tail) * sizeof(CharType));
        return first + point + tail;
    }

    template <typename CharType>
    RAINY_NODISCARD container::compressed_pair<CharType *, std::errc> generic_to_chars_precision(
        CharType *const first, CharType *const end, const long double value, const chars_format fmt, int precision) noexcept {
        uint128_t bits = 0;
        std::memcpy(&bits, &value, sizeof(long double));
        const bool negative = ((bits >> (RAINY_LONG_DOUBLE_IS_BINARY128 ? 127 : 79)) & 1) != 0;
        CharType *p = first;
        const auto reserve = [&](const std::uint32_t needed) noexcept {
            return end - p >= static_cast<std::ptrdiff_t>(needed);
        };
        if (negative) {
            if (!reserve(1)) {
                return {end, std::errc::value_too_large};
            }
            *p++ = static_cast<CharType>('-');
        }
        CharType *const digits_begin = p;
        const long double magnitude = negative ? -value : value;
        if (magnitude != magnitude) {
            if (!reserve(3)) {
                return {end, std::errc::value_too_large};
            }
            *p++ = static_cast<CharType>('n');
            *p++ = static_cast<CharType>('a');
            *p++ = static_cast<CharType>('n');
            return {p, std::errc{}};
        }
        if (magnitude > (utility::numeric_limits<long double>::max)()) {
            if (!reserve(3)) {
                return {end, std::errc::value_too_large};
            }
            *p++ = static_cast<CharType>('i');
            *p++ = static_cast<CharType>('n');
            *p++ = static_cast<CharType>('f');
            return {p, std::errc{}};
        }
        if (fmt == chars_format::hex) {
            return {end, std::errc::invalid_argument};
        }
        if (precision < 0) {
            precision = 6;
        } else if (precision >= 1'000'000'000) {
            return {end, std::errc::value_too_large};
        }
        if (fmt == chars_format::general && precision == 0) {
            precision = 1;
        }
        const std::uint32_t requested = static_cast<std::uint32_t>(precision);
        if (requested > static_cast<std::uint32_t>(end - first)) {
            return {end, std::errc::value_too_large};
        }
        if (magnitude == 0) {
            if (fmt == chars_format::scientific) {
                if (!reserve(1 + (requested > 0 ? 1 + requested : 0) + 4)) {
                    return {end, std::errc::value_too_large};
                }
                *p++ = static_cast<CharType>('0');
                if (requested > 0) {
                    *p++ = static_cast<CharType>('.');
                    for (std::uint32_t i = 0; i < requested; ++i) {
                        *p++ = static_cast<CharType>('0');
                    }
                }
                *p++ = static_cast<CharType>('e');
                *p++ = static_cast<CharType>('+');
                *p++ = static_cast<CharType>('0');
                *p++ = static_cast<CharType>('0');
                return {p, std::errc{}};
            }
            if (fmt == chars_format::general) {
                if (!reserve(1)) {
                    return {end, std::errc::value_too_large};
                }
                *p++ = static_cast<CharType>('0');
                return {p, std::errc{}};
            }
            if (!reserve(1 + (requested > 0 ? 1 + requested : 0))) {
                return {end, std::errc::value_too_large};
            }
            *p++ = static_cast<CharType>('0');
            if (requested > 0) {
                *p++ = static_cast<CharType>('.');
                for (std::uint32_t i = 0; i < requested; ++i) {
                    *p++ = static_cast<CharType>('0');
                }
            }
            return {p, std::errc{}};
        }

        big_decimal numerator{};
        std::uint32_t scale = 0;
        long_double_exact_decimal(magnitude, numerator, scale);
        std::int32_t exponent = big_decimal_decimal_exponent(numerator) - static_cast<std::int32_t>(scale);

        if (fmt == chars_format::fixed) {
            long_double_round_to_scale(numerator, scale, requested);
            const std::uint32_t length = big_decimal_digit_count(numerator);
            const std::uint32_t integer_digits = length > requested ? length - requested : 0;
            const std::uint32_t needed = (integer_digits > 0 ? integer_digits : 1) + (requested > 0 ? 1 + requested : 0);
            if (!reserve(needed)) {
                return {end, std::errc::value_too_large};
            }
            p = emit_fixed_digits(p, numerator, requested);
            return {p, std::errc{}};
        }

        if (fmt == chars_format::scientific) {
            const std::int32_t shift = static_cast<std::int32_t>(scale) - static_cast<std::int32_t>(requested) + exponent;
            if (shift > 0) {
                big_decimal_div_pow10_round(numerator, static_cast<std::uint32_t>(shift));
            } else if (shift < 0) {
                big_decimal_mul_pow10(numerator, static_cast<std::uint32_t>(-shift));
            }
            while (big_decimal_digit_count(numerator) > requested + 1) {
                big_decimal_div_pow10(numerator, 1);
                ++exponent;
            }
            const std::uint32_t needed = 1 + (requested > 0 ? 1 + requested : 0) + 2 + 5;
            if (!reserve(needed)) {
                return {end, std::errc::value_too_large};
            }
            p = emit_scientific_digits(p, numerator, requested, exponent);
            return {p, std::errc{}};
        }

        if (static_cast<std::int32_t>(requested) > exponent && exponent >= -4) {
            const std::uint32_t fractional = static_cast<std::uint32_t>(static_cast<std::int32_t>(requested) - 1 - exponent);
            long_double_round_to_scale(numerator, scale, fractional);
            const std::uint32_t length = big_decimal_digit_count(numerator);
            const std::uint32_t integer_digits = length > fractional ? length - fractional : 0;
            const std::uint32_t needed = (integer_digits > 0 ? integer_digits : 1) + (fractional > 0 ? 1 + fractional : 0);
            if (!reserve(needed)) {
                return {end, std::errc::value_too_large};
            }
            p = emit_fixed_digits(p, numerator, fractional);
            CharType *q = fractional > 0 ? strip_trailing_zeros(digits_begin, p) : p;
            return {q, std::errc{}};
        }
        const std::int32_t shift = static_cast<std::int32_t>(scale) - static_cast<std::int32_t>(requested) + 1 + exponent;
        if (shift > 0) {
            big_decimal_div_pow10_round(numerator, static_cast<std::uint32_t>(shift));
        } else if (shift < 0) {
            big_decimal_mul_pow10(numerator, static_cast<std::uint32_t>(-shift));
        }
        while (big_decimal_digit_count(numerator) > requested) {
            big_decimal_div_pow10(numerator, 1);
            ++exponent;
        }
        const std::uint32_t needed = 1 + (requested > 1 ? requested : 0) + 2 + 5;
        if (!reserve(needed)) {
            return {end, std::errc::value_too_large};
        }
        p = emit_scientific_digits(p, numerator, requested - 1, exponent);
        CharType *q = strip_trailing_zeros(digits_begin, p);
        return {q, std::errc{}};
    }

    template container::compressed_pair<char *, std::errc> generic_to_chars_precision<char>(char *, char *, long double,
                                                                                           chars_format, int) noexcept;

    template container::compressed_pair<char *, std::errc> generic_to_chars<char>(char *, char *,
                                                                                 const floating_decimal_128 &,
                                                                                 chars_format, long double) noexcept;
}
#endif
