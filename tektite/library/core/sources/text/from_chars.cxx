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
 *
 * Portions of this file are derived from microsoft/STL <charconv>
 * (Apache-2.0 WITH LLVM-exception). Copyright Microsoft Corporation.
 * https://github.com/microsoft/STL
 */
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>

namespace rainy::core::text::implements {
    template <typename Floating>
    struct floating_traits;

    template <>
    struct floating_traits<float> {
        using uint_type = std::uint32_t;
        static constexpr int sign_shift = 31;
        static constexpr int exponent_shift = 23;
        static constexpr int exponent_bias = 127;
        static constexpr int mantissa_bits = 24;
        static constexpr int maximum_binary_exponent = 127;
        static constexpr int minimum_binary_exponent = -126;
        static constexpr uint_type normal_mantissa_mask = 0xffffff;
        static constexpr uint_type shifted_exponent_mask = 0x7f800000u;
        static constexpr uint_type shifted_sign_mask = 0x80000000u;
        static constexpr uint_type special_nan_mantissa_mask = 0x400000;
    };

    template <>
    struct floating_traits<double> {
        using uint_type = std::uint64_t;
        static constexpr int sign_shift = 63;
        static constexpr int exponent_shift = 52;
        static constexpr int exponent_bias = 1023;
        static constexpr int mantissa_bits = 53;
        static constexpr int maximum_binary_exponent = 1023;
        static constexpr int minimum_binary_exponent = -1022;
        static constexpr uint_type normal_mantissa_mask = 0x1fffffffffffffULL;
        static constexpr uint_type shifted_exponent_mask = 0x7ff0000000000000ULL;
        static constexpr uint_type shifted_sign_mask = 0x8000000000000000ULL;
        static constexpr uint_type special_nan_mantissa_mask = 0x8000000000000ULL;
    };

    inline constexpr unsigned char digit_from_char_table[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,
        17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  255, 255, 255, 255, 255,
        255, 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
        33,  34,  35,  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    };

    inline unsigned char digit_from_char(const char ch) noexcept {
        return digit_from_char_table[static_cast<unsigned char>(ch)];
    }

    inline std::uint32_t bit_scan_reverse(const std::uint32_t value) noexcept {
#ifdef _MSC_VER
        unsigned long index = 0;
        _BitScanReverse(&index, value);
        return index + 1;
#else
        return value == 0 ? 0u : 32u - static_cast<std::uint32_t>(__builtin_clz(value));
#endif
    }

    inline std::uint32_t bit_scan_reverse(const std::uint64_t value) noexcept {
#ifdef _MSC_VER
        unsigned long index = 0;
        _BitScanReverse64(&index, value);
        return index + 1;
#else
        return value == 0 ? 0u : 64u - static_cast<std::uint32_t>(__builtin_clzll(value));
#endif
    }

    inline std::uint32_t count_sequential_high_zeroes(const std::uint32_t value) noexcept {
#ifdef _MSC_VER
        unsigned long index = 0;
        return _BitScanReverse(&index, value) ? 31u - index : 32u;
#else
        return value == 0 ? 32u : static_cast<std::uint32_t>(__builtin_clz(value));
#endif
    }

    struct big_integer_flt {
        static constexpr std::uint32_t maximum_bits = 1074 + 2552 + 54;
        static constexpr std::uint32_t element_bits = 32;
        static constexpr std::uint32_t element_count = (maximum_bits + element_bits - 1) / element_bits;

        big_integer_flt() noexcept : used(0) {
        }

        big_integer_flt(const big_integer_flt &other) noexcept : used(other.used) {
            std::memcpy(data, other.data, other.used * sizeof(std::uint32_t));
        }

        big_integer_flt &operator=(const big_integer_flt &other) noexcept {
            used = other.used;
            std::memmove(data, other.data, other.used * sizeof(std::uint32_t));
            return *this;
        }

        bool operator<(const big_integer_flt &rhs) const noexcept {
            if (used != rhs.used) {
                return used < rhs.used;
            }
            for (std::uint32_t i = used - 1; i != static_cast<std::uint32_t>(-1); --i) {
                if (data[i] != rhs.data[i]) {
                    return data[i] < rhs.data[i];
                }
            }
            return false;
        }

        std::uint32_t used;
        std::uint32_t data[element_count];
    };

    inline big_integer_flt make_big_integer_flt_one() noexcept {
        big_integer_flt value{};
        value.data[0] = 1;
        value.used = 1;
        return value;
    }

    inline std::uint32_t bit_scan_reverse(const big_integer_flt &value) noexcept {
        if (value.used == 0) {
            return 0;
        }
        const std::uint32_t top = value.used - 1;
        return bit_scan_reverse(value.data[top]) + top * big_integer_flt::element_bits;
    }

    inline bool shift_left(big_integer_flt &value, const std::uint32_t count) noexcept {
        if (value.used == 0) {
            return true;
        }
        const std::uint32_t unit_shift = count / big_integer_flt::element_bits;
        const std::uint32_t bit_shift = count % big_integer_flt::element_bits;
        if (value.used + unit_shift > big_integer_flt::element_count) {
            value.used = 0;
            return false;
        }
        if (bit_shift == 0) {
            std::memmove(value.data + unit_shift, value.data, value.used * sizeof(std::uint32_t));
            value.used += unit_shift;
        } else {
            const bool bit_shifts_into_next_unit =
                bit_shift > (big_integer_flt::element_bits - bit_scan_reverse(value.data[value.used - 1]));
            const std::uint32_t new_used = value.used + unit_shift + static_cast<std::uint32_t>(bit_shifts_into_next_unit);
            if (new_used > big_integer_flt::element_count) {
                value.used = 0;
                return false;
            }
            const std::uint32_t msb_bits = bit_shift;
            const std::uint32_t lsb_bits = big_integer_flt::element_bits - msb_bits;
            const std::uint32_t lsb_mask = (1u << lsb_bits) - 1u;
            const std::uint32_t msb_mask = ~lsb_mask;
            for (std::uint32_t dest_index = new_used - 1; dest_index != unit_shift - 1; --dest_index) {
                const std::uint32_t upper_source_index = dest_index - unit_shift;
                const std::uint32_t lower_source_index = dest_index - unit_shift - 1;
                const std::uint32_t upper_source = upper_source_index < value.used ? value.data[upper_source_index] : 0;
                const std::uint32_t lower_source = lower_source_index < value.used ? value.data[lower_source_index] : 0;
                const std::uint32_t shifted_upper_source = (upper_source & lsb_mask) << msb_bits;
                const std::uint32_t shifted_lower_source = (lower_source & msb_mask) >> lsb_bits;
                value.data[dest_index] = shifted_upper_source | shifted_lower_source;
            }
            value.used = new_used;
        }
        std::memset(value.data, 0, unit_shift * sizeof(std::uint32_t));
        return true;
    }

    inline bool add(big_integer_flt &value, const std::uint32_t addend) noexcept {
        if (addend == 0) {
            return true;
        }
        std::uint32_t carry = addend;
        for (std::uint32_t i = 0; i != value.used; ++i) {
            const std::uint64_t result = static_cast<std::uint64_t>(value.data[i]) + carry;
            value.data[i] = static_cast<std::uint32_t>(result);
            carry = static_cast<std::uint32_t>(result >> 32);
        }
        if (carry != 0) {
            if (value.used < big_integer_flt::element_count) {
                value.data[value.used] = carry;
                ++value.used;
            } else {
                value.used = 0;
                return false;
            }
        }
        return true;
    }

    inline std::uint32_t add_carry(std::uint32_t &target, const std::uint32_t addend, const std::uint32_t carry_in) noexcept {
        const std::uint64_t result = static_cast<std::uint64_t>(target) + addend + carry_in;
        target = static_cast<std::uint32_t>(result);
        return static_cast<std::uint32_t>(result >> 32);
    }

    inline std::uint32_t add_multiply_carry(std::uint32_t &add_target, const std::uint32_t multiplier_1,
                                            const std::uint32_t multiplier_2, const std::uint32_t carry_in) noexcept {
        const std::uint64_t result = static_cast<std::uint64_t>(multiplier_1) * multiplier_2 + add_target + carry_in;
        add_target = static_cast<std::uint32_t>(result);
        return static_cast<std::uint32_t>(result >> 32);
    }

    inline std::uint32_t multiply_core(std::uint32_t *const multiplicand, const std::uint32_t multiplicand_count,
                                       const std::uint32_t multiplier) noexcept {
        std::uint32_t carry = 0;
        for (std::uint32_t i = 0; i != multiplicand_count; ++i) {
            const std::uint64_t result = static_cast<std::uint64_t>(multiplicand[i]) * multiplier + carry;
            multiplicand[i] = static_cast<std::uint32_t>(result);
            carry = static_cast<std::uint32_t>(result >> 32);
        }
        return carry;
    }

    inline bool multiply(big_integer_flt &multiplicand, const std::uint32_t multiplier) noexcept {
        if (multiplier == 0) {
            multiplicand.used = 0;
            return true;
        }
        if (multiplier == 1) {
            return true;
        }
        if (multiplicand.used == 0) {
            return true;
        }
        const std::uint32_t carry = multiply_core(multiplicand.data, multiplicand.used, multiplier);
        if (carry != 0) {
            if (multiplicand.used < big_integer_flt::element_count) {
                multiplicand.data[multiplicand.used] = carry;
                ++multiplicand.used;
            } else {
                multiplicand.used = 0;
                return false;
            }
        }
        return true;
    }

    inline bool multiply(big_integer_flt &multiplicand, const big_integer_flt &multiplier) noexcept {
        if (multiplicand.used == 0) {
            return true;
        }
        if (multiplier.used == 0) {
            multiplicand.used = 0;
            return true;
        }
        if (multiplier.used == 1) {
            return multiply(multiplicand, multiplier.data[0]);
        }
        if (multiplicand.used == 1) {
            const std::uint32_t small_multiplier = multiplicand.data[0];
            multiplicand = multiplier;
            return multiply(multiplicand, small_multiplier);
        }
        const bool multiplier_is_shorter = multiplier.used < multiplicand.used;
        const std::uint32_t *const terms_1 = multiplier_is_shorter ? multiplier.data : multiplicand.data;
        const std::uint32_t *const terms_2 = multiplier_is_shorter ? multiplicand.data : multiplier.data;
        const std::uint32_t count_1 = multiplier_is_shorter ? multiplier.used : multiplicand.used;
        const std::uint32_t count_2 = multiplier_is_shorter ? multiplicand.used : multiplier.used;
        big_integer_flt result{};
        for (std::uint32_t i = 0; i != count_1; ++i) {
            const std::uint32_t current = terms_1[i];
            if (current == 0) {
                if (i == result.used) {
                    result.data[i] = 0;
                    result.used = i + 1;
                }
                continue;
            }
            std::uint32_t carry = 0;
            std::uint32_t result_index = i;
            for (std::uint32_t j = 0; j != count_2 && result_index != big_integer_flt::element_count; ++j, ++result_index) {
                if (result_index == result.used) {
                    result.data[result_index] = 0;
                    result.used = result_index + 1;
                }
                carry = add_multiply_carry(result.data[result_index], current, terms_2[j], carry);
            }
            while (carry != 0 && result_index != big_integer_flt::element_count) {
                if (result_index == result.used) {
                    result.data[result_index] = 0;
                    result.used = result_index + 1;
                }
                carry = add_carry(result.data[result_index++], 0, carry);
            }
            if (result_index == big_integer_flt::element_count) {
                multiplicand.used = 0;
                return false;
            }
        }
        multiplicand = result;
        return true;
    }

    inline bool multiply_by_power_of_ten(big_integer_flt &value, const std::uint32_t power) noexcept {
        static constexpr std::uint32_t small_powers_of_ten[9] = {10,        100,        1'000,       10'000,       100'000,
                                                                 1'000'000, 10'000'000, 100'000'000, 1'000'000'000};
        std::uint32_t remaining = power;
        while (remaining >= 9) {
            if (!multiply(value, 1'000'000'000u)) {
                return false;
            }
            remaining -= 9;
        }
        if (remaining == 0) {
            return true;
        }
        return multiply(value, small_powers_of_ten[remaining - 1]);
    }

    inline std::uint64_t divide(big_integer_flt &numerator, const big_integer_flt &denominator) noexcept {
        if (numerator.used == 0) {
            return 0;
        }
        std::uint32_t max_numerator_element_index = numerator.used - 1;
        const std::uint32_t max_denominator_element_index = denominator.used - 1;
        if (max_denominator_element_index == 0) {
            const std::uint32_t small_denominator = denominator.data[0];
            if (max_numerator_element_index == 0) {
                const std::uint32_t small_numerator = numerator.data[0];
                if (small_denominator == 1) {
                    numerator.used = 0;
                    return small_numerator;
                }
                numerator.data[0] = small_numerator % small_denominator;
                numerator.used = numerator.data[0] > 0 ? 1u : 0u;
                return small_numerator / small_denominator;
            }
            if (small_denominator == 1) {
                std::uint64_t quotient = numerator.data[1];
                quotient <<= 32;
                quotient |= numerator.data[0];
                numerator.used = 0;
                return quotient;
            }
            std::uint64_t quotient = 0;
            std::uint64_t remainder_wide = 0;
            for (std::uint32_t i = max_numerator_element_index; i != static_cast<std::uint32_t>(-1); --i) {
                remainder_wide = (remainder_wide << 32) | numerator.data[i];
                quotient = (quotient << 32) + static_cast<std::uint32_t>(remainder_wide / small_denominator);
                remainder_wide %= small_denominator;
            }
            numerator.data[1] = static_cast<std::uint32_t>(remainder_wide >> 32);
            numerator.data[0] = static_cast<std::uint32_t>(remainder_wide);
            if (numerator.data[1] > 0) {
                numerator.used = 2u;
            } else if (numerator.data[0] > 0) {
                numerator.used = 1u;
            } else {
                numerator.used = 0u;
            }
            return quotient;
        }
        if (max_denominator_element_index > max_numerator_element_index) {
            return 0;
        }
        const std::uint32_t denominator_count = max_denominator_element_index + 1;
        const int difference = static_cast<int>(max_numerator_element_index - max_denominator_element_index);
        int quotient_digits = difference;
        for (int i = static_cast<int>(max_numerator_element_index);; --i) {
            if (i < difference) {
                ++quotient_digits;
                break;
            }
            if (denominator.data[i - difference] != numerator.data[i]) {
                if (denominator.data[i - difference] < numerator.data[i]) {
                    ++quotient_digits;
                }
                break;
            }
        }
        if (quotient_digits == 0) {
            return 0;
        }
        std::uint32_t denominator_high = denominator.data[denominator_count - 1];
        std::uint32_t denominator_next = denominator.data[denominator_count - 2];
        const std::uint32_t shift_left_amount = count_sequential_high_zeroes(denominator_high);
        const std::uint32_t shift_right_amount = 32 - shift_left_amount;
        if (shift_left_amount > 0) {
            denominator_high = (denominator_high << shift_left_amount) | (denominator_next >> shift_right_amount);
            denominator_next <<= shift_left_amount;
            if (denominator_count > 2) {
                denominator_next |= denominator.data[denominator_count - 3] >> shift_right_amount;
            }
        }
        std::uint64_t quotient = 0;
        for (int i = quotient_digits; --i >= 0;) {
            const std::uint32_t numerator_high =
                (i + static_cast<int>(denominator_count) <= static_cast<int>(max_numerator_element_index))
                    ? numerator.data[i + denominator_count]
                    : 0;
            std::uint64_t numerator_wide =
                (static_cast<std::uint64_t>(numerator_high) << 32) | numerator.data[i + denominator_count - 1];
            std::uint32_t numerator_next = numerator.data[i + denominator_count - 2];
            if (shift_left_amount > 0) {
                numerator_wide = (numerator_wide << shift_left_amount) | (numerator_next >> shift_right_amount);
                numerator_next <<= shift_left_amount;
                if (i + static_cast<int>(denominator_count) >= 3) {
                    numerator_next |= numerator.data[i + denominator_count - 3] >> shift_right_amount;
                }
            }
            std::uint64_t quotient_digit = numerator_wide / denominator_high;
            std::uint64_t remainder = static_cast<std::uint32_t>(numerator_wide % denominator_high);
            if (quotient_digit > UINT32_MAX) {
                remainder += denominator_high * (quotient_digit - UINT32_MAX);
                quotient_digit = UINT32_MAX;
            }
            while (remainder <= UINT32_MAX && quotient_digit * denominator_next > ((remainder << 32) | numerator_next)) {
                --quotient_digit;
                remainder += denominator_high;
            }
            if (quotient_digit > 0) {
                std::uint64_t borrow = 0;
                for (std::uint32_t j = 0; j < denominator_count; ++j) {
                    borrow += quotient_digit * denominator.data[j];
                    const std::uint32_t subtrahend = static_cast<std::uint32_t>(borrow);
                    borrow >>= 32;
                    if (numerator.data[i + j] < subtrahend) {
                        ++borrow;
                    }
                    numerator.data[i + j] -= subtrahend;
                }
                if (numerator_high < borrow) {
                    std::uint32_t carry = 0;
                    for (std::uint32_t j = 0; j < denominator_count; ++j) {
                        const std::uint64_t sum = static_cast<std::uint64_t>(numerator.data[i + j]) + denominator.data[j] + carry;
                        numerator.data[i + j] = static_cast<std::uint32_t>(sum);
                        carry = static_cast<std::uint32_t>(sum >> 32);
                    }
                    --quotient_digit;
                }
                max_numerator_element_index = i + denominator_count - 1;
            }
            quotient = (quotient << 32) + static_cast<std::uint32_t>(quotient_digit);
        }
        std::uint32_t used = max_numerator_element_index + 1;
        while (used != 0 && numerator.data[used - 1] == 0) {
            --used;
        }
        numerator.used = used;
        return quotient;
    }

    struct floating_point_string {
        bool is_negative;
        std::int32_t exponent;
        std::uint32_t mantissa_count;
        std::uint8_t mantissa[768];
    };

    template <typename Floating>
    void assemble_floating_point_zero(const bool is_negative, Floating &result) noexcept {
        using traits = floating_traits<Floating>;
        using uint_type = typename traits::uint_type;
        const uint_type sign_component = static_cast<uint_type>(is_negative) << traits::sign_shift;
        result = std::bit_cast<Floating>(sign_component);
    }

    template <typename Floating>
    void assemble_floating_point_infinity(const bool is_negative, Floating &result) noexcept {
        using traits = floating_traits<Floating>;
        using uint_type = typename traits::uint_type;
        const uint_type sign_component = static_cast<uint_type>(is_negative) << traits::sign_shift;
        result = std::bit_cast<Floating>(static_cast<uint_type>(sign_component | traits::shifted_exponent_mask));
    }

    inline bool should_round_up(const bool lsb_bit, const bool round_bit, const bool has_tail_bits) noexcept {
        return round_bit && (has_tail_bits || lsb_bit);
    }

    inline std::uint64_t right_shift_with_rounding(const std::uint64_t value, const std::uint32_t shift,
                                                   const bool has_zero_tail) noexcept {
        constexpr std::uint32_t total_bits = 64;
        if (shift >= total_bits) {
            if (shift == total_bits) {
                constexpr std::uint64_t extra_bits_mask = (1ULL << (total_bits - 1)) - 1;
                constexpr std::uint64_t round_bit_mask = (1ULL << (total_bits - 1));
                const bool round_bit = (value & round_bit_mask) != 0;
                const bool tail_bits = !has_zero_tail || (value & extra_bits_mask) != 0;
                return static_cast<std::uint64_t>(round_bit && tail_bits);
            }
            return 0;
        }
        const std::uint64_t lsb_bit = value;
        const std::uint64_t round_bit = value << 1;
        const std::uint64_t has_tail_bits = round_bit - static_cast<std::uint64_t>(has_zero_tail);
        const std::uint64_t round_up = ((round_bit & (has_tail_bits | lsb_bit)) >> shift) & std::uint64_t{1};
        return (value >> shift) + round_up;
    }

    template <typename Floating>
    void assemble_floating_point_value_no_shift(const bool is_negative, const int exponent,
                                                const typename floating_traits<Floating>::uint_type mantissa,
                                                Floating &result) noexcept {
        using traits = floating_traits<Floating>;
        using uint_type = typename traits::uint_type;
        const uint_type sign_component = static_cast<uint_type>(is_negative) << traits::sign_shift;
        uint_type exponent_component = static_cast<uint_type>(exponent + (traits::exponent_bias - 1));
        exponent_component <<= traits::exponent_shift;
        result = std::bit_cast<Floating>(static_cast<uint_type>(sign_component | (exponent_component + mantissa)));
    }

    template <typename Floating>
    std::errc assemble_floating_point_value(const std::uint64_t initial_mantissa, const std::int32_t initial_exponent,
                                            const bool is_negative, const bool has_zero_tail, Floating &result) noexcept {
        using traits = floating_traits<Floating>;
        const std::uint32_t initial_mantissa_bits = bit_scan_reverse(initial_mantissa);
        const int normal_mantissa_shift = static_cast<int>(traits::mantissa_bits) - static_cast<int>(initial_mantissa_bits);
        const int normal_exponent = static_cast<int>(initial_exponent) - normal_mantissa_shift;
        if (normal_exponent > traits::maximum_binary_exponent) {
            assemble_floating_point_infinity(is_negative, result);
            return std::errc::result_out_of_range;
        }
        std::uint64_t mantissa = initial_mantissa;
        int exponent = normal_exponent;
        std::errc error_code{};
        if (normal_exponent < traits::minimum_binary_exponent) {
            exponent = traits::minimum_binary_exponent;
            const int denormal_mantissa_shift = static_cast<int>(initial_exponent) - exponent;
            if (denormal_mantissa_shift < 0) {
                mantissa = right_shift_with_rounding(mantissa, static_cast<std::uint32_t>(-denormal_mantissa_shift), has_zero_tail);
                if (mantissa == 0) {
                    error_code = std::errc::result_out_of_range;
                }
            } else {
                mantissa <<= denormal_mantissa_shift;
            }
        } else {
            if (normal_mantissa_shift < 0) {
                mantissa = right_shift_with_rounding(mantissa, static_cast<std::uint32_t>(-normal_mantissa_shift), has_zero_tail);
                if (mantissa > static_cast<std::uint64_t>(traits::normal_mantissa_mask) &&
                    exponent == traits::maximum_binary_exponent) {
                    error_code = std::errc::result_out_of_range;
                }
            } else {
                mantissa <<= normal_mantissa_shift;
            }
        }
        assemble_floating_point_value_no_shift(is_negative, exponent, static_cast<typename traits::uint_type>(mantissa), result);
        return error_code;
    }

    template <typename Floating>
    std::errc assemble_floating_point_value_from_big_integer(const big_integer_flt &integer_value,
                                                             const std::uint32_t integer_bits_of_precision, const bool is_negative,
                                                             const bool has_nonzero_fractional_part, Floating &result) noexcept {
        using traits = floating_traits<Floating>;
        constexpr int base_exponent = traits::mantissa_bits - 1;
        if (integer_bits_of_precision <= 64) {
            const std::uint32_t mantissa_low = integer_value.used > 0 ? integer_value.data[0] : 0;
            const std::uint32_t mantissa_high = integer_value.used > 1 ? integer_value.data[1] : 0;
            const std::uint64_t mantissa = mantissa_low + (static_cast<std::uint64_t>(mantissa_high) << 32);
            return assemble_floating_point_value(mantissa, base_exponent, is_negative, !has_nonzero_fractional_part, result);
        }
        const std::uint32_t top_element_bits = integer_bits_of_precision % 32;
        const std::uint32_t top_element_index = integer_bits_of_precision / 32;
        const std::uint32_t middle_element_index = top_element_index - 1;
        const std::uint32_t bottom_element_index = top_element_index - 2;
        if (top_element_bits == 0) {
            const int exponent = static_cast<int>(base_exponent + bottom_element_index * 32);
            const std::uint64_t mantissa = integer_value.data[bottom_element_index] +
                                           (static_cast<std::uint64_t>(integer_value.data[middle_element_index]) << 32);
            bool has_zero_tail = !has_nonzero_fractional_part;
            for (std::uint32_t i = 0; has_zero_tail && i != bottom_element_index; ++i) {
                has_zero_tail = integer_value.data[i] == 0;
            }
            return assemble_floating_point_value(mantissa, exponent, is_negative, has_zero_tail, result);
        }
        const std::uint32_t top_element_mask = (1u << top_element_bits) - 1;
        const std::uint32_t top_element_shift = 64 - top_element_bits;
        const std::uint32_t middle_element_shift = top_element_shift - 32;
        const std::uint32_t bottom_element_bits = 32 - top_element_bits;
        const std::uint32_t bottom_element_mask = ~top_element_mask;
        const std::uint32_t bottom_element_shift = 32 - bottom_element_bits;
        const int exponent = static_cast<int>(base_exponent + bottom_element_index * 32 + top_element_bits);
        const std::uint64_t mantissa =
            (static_cast<std::uint64_t>(integer_value.data[top_element_index] & top_element_mask) << top_element_shift) +
            (static_cast<std::uint64_t>(integer_value.data[middle_element_index]) << middle_element_shift) +
            ((static_cast<std::uint64_t>(integer_value.data[bottom_element_index] & bottom_element_mask)) >> bottom_element_shift);
        bool has_zero_tail = !has_nonzero_fractional_part && (integer_value.data[bottom_element_index] & top_element_mask) == 0;
        for (std::uint32_t i = 0; has_zero_tail && i != bottom_element_index; ++i) {
            has_zero_tail = integer_value.data[i] == 0;
        }
        return assemble_floating_point_value(mantissa, exponent, is_negative, has_zero_tail, result);
    }

    inline void accumulate_decimal_digits_into_big_integer(const std::uint8_t *first_digit, const std::uint8_t *last_digit,
                                                           big_integer_flt &result) noexcept {
        std::uint32_t accumulator = 0;
        std::uint32_t accumulator_count = 0;
        for (const std::uint8_t *it = first_digit; it != last_digit; ++it) {
            if (accumulator_count == 9) {
                multiply(result, 1'000'000'000u);
                add(result, accumulator);
                accumulator = 0;
                accumulator_count = 0;
            }
            accumulator *= 10;
            accumulator += *it;
            ++accumulator_count;
        }
        if (accumulator_count != 0) {
            multiply_by_power_of_ten(result, accumulator_count);
            add(result, accumulator);
        }
    }

    template <typename Floating>
    std::errc convert_decimal_string_to_floating_type(const floating_point_string &data, Floating &result,
                                                      bool has_zero_tail) noexcept {
        using traits = floating_traits<Floating>;
        constexpr std::uint32_t required_bits_of_precision = static_cast<std::uint32_t>(traits::mantissa_bits + 1);
        {
            constexpr std::uint32_t fast_max_digits = sizeof(Floating) <= 4 ? 7u : 15u;
            constexpr std::int32_t fast_max_exponent = sizeof(Floating) <= 4 ? 10 : 22;
            const std::uint32_t digit_count = data.mantissa_count;
            if (digit_count != 0 && digit_count <= fast_max_digits) {
                std::uint64_t significand = 0;
                for (std::uint32_t i = 0; i != digit_count; ++i) {
                    significand = significand * 10 + data.mantissa[i];
                }
                const std::int32_t exponent = data.exponent - static_cast<std::int32_t>(digit_count);
                if (significand != 0 && exponent >= -fast_max_exponent && exponent <= fast_max_exponent) {
                    static constexpr double positive_power_of_ten[] = {
                        1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
                        1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
                    Floating value = static_cast<Floating>(significand);
                    if (exponent > 0) {
                        value *= static_cast<Floating>(positive_power_of_ten[exponent]);
                    } else if (exponent < 0) {
                        value /= static_cast<Floating>(positive_power_of_ten[-exponent]);
                    }
                    result = data.is_negative ? Floating(-value) : value;
                    return std::errc{};
                }
            }
        }
        const std::uint32_t positive_exponent = static_cast<std::uint32_t>((std::max) (0, data.exponent));
        const std::uint32_t integer_digits_present = (std::min) (positive_exponent, data.mantissa_count);
        const std::uint32_t integer_digits_missing = positive_exponent - integer_digits_present;
        const std::uint8_t *const integer_first = data.mantissa;
        const std::uint8_t *const integer_last = data.mantissa + integer_digits_present;
        const std::uint8_t *const fractional_first = integer_last;
        const std::uint8_t *const fractional_last = data.mantissa + data.mantissa_count;
        const std::uint32_t fractional_digits_present = static_cast<std::uint32_t>(fractional_last - fractional_first);

        big_integer_flt integer_value{};
        accumulate_decimal_digits_into_big_integer(integer_first, integer_last, integer_value);

        if (integer_digits_missing > 0) {
            if (!multiply_by_power_of_ten(integer_value, integer_digits_missing)) {
                assemble_floating_point_infinity(data.is_negative, result);
                return std::errc::result_out_of_range;
            }
        }

        const std::uint32_t integer_bits_of_precision = bit_scan_reverse(integer_value);
        {
            const bool has_zero_fractional_part = fractional_digits_present == 0 && has_zero_tail;
            if (integer_bits_of_precision >= required_bits_of_precision || has_zero_fractional_part) {
                return assemble_floating_point_value_from_big_integer(integer_value, integer_bits_of_precision, data.is_negative,
                                                                      !has_zero_fractional_part, result);
            }
        }

        big_integer_flt fractional_numerator{};
        accumulate_decimal_digits_into_big_integer(fractional_first, fractional_last, fractional_numerator);

        const std::uint32_t fractional_denominator_exponent =
            data.exponent < 0 ? fractional_digits_present + static_cast<std::uint32_t>(-data.exponent) : fractional_digits_present;

        big_integer_flt fractional_denominator = make_big_integer_flt_one();
        if (!multiply_by_power_of_ten(fractional_denominator, fractional_denominator_exponent)) {
            assemble_floating_point_zero(data.is_negative, result);
            return std::errc::result_out_of_range;
        }

        const std::uint32_t fractional_numerator_bits = bit_scan_reverse(fractional_numerator);
        const std::uint32_t fractional_denominator_bits = bit_scan_reverse(fractional_denominator);
        const std::uint32_t fractional_shift =
            fractional_denominator_bits > fractional_numerator_bits ? fractional_denominator_bits - fractional_numerator_bits : 0;
        if (fractional_shift > 0) {
            shift_left(fractional_numerator, fractional_shift);
        }

        const std::uint32_t required_fractional_bits_of_precision = required_bits_of_precision - integer_bits_of_precision;

        std::uint32_t remaining_bits_of_precision_required = required_fractional_bits_of_precision;
        if (integer_bits_of_precision > 0) {
            if (fractional_shift > remaining_bits_of_precision_required) {
                return assemble_floating_point_value_from_big_integer(integer_value, integer_bits_of_precision, data.is_negative,
                                                                      fractional_digits_present != 0 || !has_zero_tail, result);
            }
            remaining_bits_of_precision_required -= fractional_shift;
        }

        const std::uint32_t fractional_exponent =
            fractional_numerator < fractional_denominator ? fractional_shift + 1 : fractional_shift;

        shift_left(fractional_numerator, remaining_bits_of_precision_required);

        std::uint64_t fractional_mantissa = divide(fractional_numerator, fractional_denominator);

        has_zero_tail = has_zero_tail && fractional_numerator.used == 0;

        const std::uint32_t fractional_mantissa_bits = bit_scan_reverse(fractional_mantissa);
        if (fractional_mantissa_bits > required_fractional_bits_of_precision) {
            const std::uint32_t shift = fractional_mantissa_bits - required_fractional_bits_of_precision;
            has_zero_tail = has_zero_tail && (fractional_mantissa & ((1ULL << shift) - 1)) == 0;
            fractional_mantissa >>= shift;
        }

        const std::uint32_t integer_mantissa_low = integer_value.used > 0 ? integer_value.data[0] : 0;
        const std::uint32_t integer_mantissa_high = integer_value.used > 1 ? integer_value.data[1] : 0;
        const std::uint64_t integer_mantissa = integer_mantissa_low + (static_cast<std::uint64_t>(integer_mantissa_high) << 32);

        const std::uint64_t complete_mantissa = (integer_mantissa << required_fractional_bits_of_precision) + fractional_mantissa;

        const std::int32_t final_exponent = integer_bits_of_precision > 0 ? static_cast<std::int32_t>(integer_bits_of_precision - 2)
                                                                          : -static_cast<std::int32_t>(fractional_exponent) - 1;

        return assemble_floating_point_value(complete_mantissa, final_exponent, data.is_negative, has_zero_tail, result);
    }

    template <typename Floating>
    std::errc convert_hexadecimal_string_to_floating_type(const floating_point_string &data, Floating &result,
                                                          bool has_zero_tail) noexcept {
        using traits = floating_traits<Floating>;
        std::uint64_t mantissa = 0;
        std::int32_t exponent = data.exponent + traits::mantissa_bits - 1;
        const std::uint8_t *const mantissa_last = data.mantissa + data.mantissa_count;
        const std::uint8_t *mantissa_it = data.mantissa;
        while (mantissa_it != mantissa_last && mantissa <= static_cast<std::uint64_t>(traits::normal_mantissa_mask)) {
            mantissa *= 16;
            mantissa += *mantissa_it++;
            exponent -= 4;
        }
        while (has_zero_tail && mantissa_it != mantissa_last) {
            has_zero_tail = *mantissa_it++ == 0;
        }
        return assemble_floating_point_value(mantissa, exponent, data.is_negative, has_zero_tail, result);
    }

    template <typename Floating>
    from_chars_result ordinary_floating_from_chars(const char *const first, const char *const last, Floating &value,
                                                   const chars_format fmt, const bool minus_sign, const char *next) {
        const bool is_hexadecimal = fmt == chars_format::hex;
        const int base = is_hexadecimal ? 16 : 10;

        floating_point_string fp_string;
        fp_string.is_negative = minus_sign;

        std::uint8_t *const mantissa_first = fp_string.mantissa;
        std::uint8_t *const mantissa_last = fp_string.mantissa + 768;
        std::uint8_t *mantissa_it = mantissa_first;

        const char *const whole_begin = next;

        for (; next != last && *next == '0'; ++next) {
        }
        const char *const leading_zero_end = next;

        bool has_zero_tail = true;

        for (; next != last; ++next) {
            const unsigned char digit_value = digit_from_char(*next);
            if (digit_value >= base) {
                break;
            }
            if (mantissa_it != mantissa_last) {
                *mantissa_it++ = digit_value;
            } else {
                has_zero_tail = has_zero_tail && digit_value == 0;
            }
        }
        const char *const whole_end = next;

        std::ptrdiff_t exponent_adjustment = whole_end - leading_zero_end;

        if (next != last && *next == '.') {
            ++next;
        }
        const char *const dot_end = next;

        if (exponent_adjustment == 0) {
            for (; next != last && *next == '0'; ++next) {
            }
            exponent_adjustment = dot_end - next;
        }

        for (; next != last; ++next) {
            const unsigned char digit_value = digit_from_char(*next);
            if (digit_value >= base) {
                break;
            }
            if (mantissa_it != mantissa_last) {
                *mantissa_it++ = digit_value;
            } else {
                has_zero_tail = has_zero_tail && digit_value == 0;
            }
        }
        const char *const frac_end = next;

        if (whole_begin == whole_end && dot_end == frac_end) {
            return {first, std::errc::invalid_argument};
        }

        const char exponent_prefix = is_hexadecimal ? 'p' : 'e';

        bool exponent_is_negative = false;
        bool exp_abs_too_large = false;
        std::ptrdiff_t exponent_value = 0;

        constexpr std::ptrdiff_t maximum_temporary_exponent = 5200;
        constexpr std::ptrdiff_t minimum_temporary_exponent = -5200;
        constexpr std::ptrdiff_t exponent_accumulator_limit = (std::numeric_limits<std::ptrdiff_t>::max)() / 10;

        if (fmt != chars_format::fixed && next != last &&
            (static_cast<unsigned char>(*next) | 0x20) == static_cast<unsigned char>(exponent_prefix)) {
            const char *unread = next + 1;
            if (unread != last && (*unread == '+' || *unread == '-')) {
                exponent_is_negative = *unread == '-';
                ++unread;
            }
            while (unread != last) {
                const unsigned char digit_value = digit_from_char(*unread);
                if (digit_value >= 10) {
                    break;
                }
                if (exponent_value < exponent_accumulator_limit ||
                    (exponent_value == exponent_accumulator_limit &&
                     digit_value <= static_cast<unsigned char>((std::numeric_limits<std::ptrdiff_t>::max)() % 10))) {
                    exponent_value = exponent_value * 10 + digit_value;
                } else {
                    exp_abs_too_large = true;
                }
                ++unread;
                next = unread;
            }
            if (exponent_is_negative) {
                exponent_value = -exponent_value;
            }
        }

        const char *const exponent_end = next;

        if (fmt == chars_format::scientific && frac_end == exponent_end) {
            return {first, std::errc::invalid_argument};
        }

        while (mantissa_it != mantissa_first && *(mantissa_it - 1) == 0) {
            --mantissa_it;
        }

        if (mantissa_it == mantissa_first) {
            assemble_floating_point_zero(fp_string.is_negative, value);
            return {next, std::errc{}};
        }

        if (exp_abs_too_large) {
            if (exponent_value > 0) {
                assemble_floating_point_infinity(fp_string.is_negative, value);
            } else {
                assemble_floating_point_zero(fp_string.is_negative, value);
            }
            return {next, std::errc::result_out_of_range};
        }

        if (exponent_value > 0 && exponent_adjustment < 0) {
            if (is_hexadecimal) {
                const std::ptrdiff_t further_adjustment = (std::max) (-((exponent_value - 1) / 4 + 1), exponent_adjustment);
                exponent_value += further_adjustment * 4;
                exponent_adjustment -= further_adjustment;
            } else {
                const std::ptrdiff_t further_adjustment = (std::max) (-exponent_value, exponent_adjustment);
                exponent_value += further_adjustment;
                exponent_adjustment -= further_adjustment;
            }
        } else if (exponent_value < 0 && exponent_adjustment > 0) {
            if (is_hexadecimal) {
                const std::ptrdiff_t further_adjustment = (std::min) ((-exponent_value - 1) / 4 + 1, exponent_adjustment);
                exponent_value += further_adjustment * 4;
                exponent_adjustment -= further_adjustment;
            } else {
                const std::ptrdiff_t further_adjustment = (std::min) (-exponent_value, exponent_adjustment);
                exponent_value += further_adjustment;
                exponent_adjustment -= further_adjustment;
            }
        }

        const int exponent_adjustment_multiplier = is_hexadecimal ? 4 : 1;

        if (exponent_value > maximum_temporary_exponent ||
            exponent_adjustment > maximum_temporary_exponent / exponent_adjustment_multiplier) {
            assemble_floating_point_infinity(fp_string.is_negative, value);
            return {next, std::errc::result_out_of_range};
        }

        if (exponent_value < minimum_temporary_exponent ||
            exponent_adjustment < minimum_temporary_exponent / exponent_adjustment_multiplier) {
            assemble_floating_point_zero(fp_string.is_negative, value);
            return {next, std::errc::result_out_of_range};
        }

        exponent_value += exponent_adjustment * exponent_adjustment_multiplier;

        if (exponent_value > maximum_temporary_exponent) {
            assemble_floating_point_infinity(fp_string.is_negative, value);
            return {next, std::errc::result_out_of_range};
        }

        if (exponent_value < minimum_temporary_exponent) {
            assemble_floating_point_zero(fp_string.is_negative, value);
            return {next, std::errc::result_out_of_range};
        }

        fp_string.exponent = static_cast<std::int32_t>(exponent_value);
        fp_string.mantissa_count = static_cast<std::uint32_t>(mantissa_it - mantissa_first);

        if (is_hexadecimal) {
            const std::errc ec = convert_hexadecimal_string_to_floating_type(fp_string, value, has_zero_tail);
            return {next, ec};
        }
        const std::errc ec = convert_decimal_string_to_floating_type(fp_string, value, has_zero_tail);
        return {next, ec};
    }

    inline bool starts_with_case_insensitive(const char *first, const char *const last, const char *lowercase) noexcept {
        for (; first != last && *lowercase != '\0'; ++first, ++lowercase) {
            if ((static_cast<unsigned char>(*first) | 0x20) != static_cast<unsigned char>(*lowercase)) {
                return false;
            }
        }
        return *lowercase == '\0';
    }

    template <typename Floating>
    from_chars_result infinity_from_chars(const char *const first, const char *const last, Floating &value, const bool minus_sign,
                                          const char *next) noexcept {
        if (!starts_with_case_insensitive(next + 1, last, "nf")) {
            return {first, std::errc::invalid_argument};
        }
        next += 3;
        if (starts_with_case_insensitive(next, last, "inity")) {
            next += 5;
        }
        assemble_floating_point_infinity(minus_sign, value);
        return {next, std::errc{}};
    }

    template <typename Floating>
    from_chars_result nan_from_chars(const char *const first, const char *const last, Floating &value, bool minus_sign,
                                     const char *next) noexcept {
        if (!starts_with_case_insensitive(next + 1, last, "an")) {
            return {first, std::errc::invalid_argument};
        }
        next += 3;
        bool quiet = true;
        if (next != last && *next == '(') {
            const char *const seq_begin = next + 1;
            for (const char *temp = seq_begin; temp != last; ++temp) {
                if (*temp == ')') {
                    next = temp + 1;
                    if (temp - seq_begin == 3 && starts_with_case_insensitive(seq_begin, temp, "ind")) {
                        minus_sign = true;
                    } else if (temp - seq_begin == 4 && starts_with_case_insensitive(seq_begin, temp, "snan")) {
                        quiet = false;
                    }
                    break;
                } else if (*temp == '_' || (*temp >= '0' && *temp <= '9') || (*temp >= 'A' && *temp <= 'Z') ||
                           (*temp >= 'a' && *temp <= 'z')) {
                    continue;
                } else {
                    break;
                }
            }
        }
        using traits = floating_traits<Floating>;
        using uint_type = typename traits::uint_type;
        uint_type uint_value = traits::shifted_exponent_mask;
        if (minus_sign) {
            uint_value |= traits::shifted_sign_mask;
        }
        if (quiet) {
            uint_value |= traits::special_nan_mantissa_mask;
        } else {
            uint_value |= 1;
        }
        value = std::bit_cast<Floating>(uint_value);
        return {next, std::errc{}};
    }
}

namespace rainy::core::text::implements {
    template <typename Floating>
    from_chars_result floating_from_chars(const char *const begin, const char *const last, Floating &value,
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
