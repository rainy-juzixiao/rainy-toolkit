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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_CODEC_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_CODEC_HPP
#include <rainy/foundation/willow/implements/cbor/config.hpp>
#include <rainy/foundation/willow/implements/cbor/exceptions.hpp>
#include <rainy/foundation/willow/implements/cbor/version.hpp>

#if RAINY_WILLOW_CBOR_AVAILABLE

namespace rainy::foundation::willow::cbor::implements {
    using namespace rainy::foundation::exceptions::willow::cbor;

    inline constexpr std::uint8_t additional_indefinite = 31;

    /**
     * \lang english
     * @brief Writes an initial byte followed by the argument in the shortest form that holds it.
     *
     * Rfc 8949 lets a value be written with a wider argument than necessary, but the shortest form is the
     * canonical one, so this always emits it and a decode re-encode round trips.
     *
     * \lang simp-chinese
     * @brief 写入首字节，并以能容纳该参数的最短形式跟随其后。
     *
     * RFC 8949 允许用比必要更宽的参数写出取值，但最短形式是规范形式，因此这里总是写出它，使解码后重新编码可以往返。
     */
    inline void push_head(byte_buffer &out, const major_type major, const std::uint64_t argument) {
        const auto base = static_cast<std::uint8_t>(static_cast<std::uint8_t>(major) << 5);
        if (argument < 24) {
            out.push_back(static_cast<std::uint8_t>(base | argument));
        } else if (argument <= 0xffull) {
            out.push_back(static_cast<std::uint8_t>(base | 24));
            out.push_back(static_cast<std::uint8_t>(argument));
        } else if (argument <= 0xffffull) {
            out.push_back(static_cast<std::uint8_t>(base | 25));
            out.push_back(static_cast<std::uint8_t>((argument >> 8) & 0xffu));
            out.push_back(static_cast<std::uint8_t>(argument & 0xffu));
        } else if (argument <= 0xffffffffull) {
            out.push_back(static_cast<std::uint8_t>(base | 26));
            for (int shift = 24; shift >= 0; shift -= 8) {
                out.push_back(static_cast<std::uint8_t>((argument >> shift) & 0xffu));
            }
        } else {
            out.push_back(static_cast<std::uint8_t>(base | 27));
            for (int shift = 56; shift >= 0; shift -= 8) {
                out.push_back(static_cast<std::uint8_t>((argument >> shift) & 0xffu));
            }
        }
    }

    inline void push_break(byte_buffer &out) {
        out.push_back(0xff);
    }

    inline bool take_u8(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint8_t &value) noexcept {
        if (ptr >= end) {
            return false;
        }
        value = *ptr++;
        return true;
    }

    inline bool take_be16(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint16_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 2) {
            return false;
        }
        value = static_cast<std::uint16_t>((static_cast<std::uint16_t>(ptr[0]) << 8) | static_cast<std::uint16_t>(ptr[1]));
        ptr += 2;
        return true;
    }

    inline bool take_be32(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint32_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 4) {
            return false;
        }
        value = (static_cast<std::uint32_t>(ptr[0]) << 24) | (static_cast<std::uint32_t>(ptr[1]) << 16) |
                (static_cast<std::uint32_t>(ptr[2]) << 8) | static_cast<std::uint32_t>(ptr[3]);
        ptr += 4;
        return true;
    }

    inline bool take_be64(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint64_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 8) {
            return false;
        }
        value = 0;
        for (int i = 0; i < 8; ++i) {
            value = (value << 8) | static_cast<std::uint64_t>(ptr[i]);
        }
        ptr += 8;
        return true;
    }

    /**
     * \lang english
     * @brief Reads the argument that follows an initial byte.
     *
     * @return False when the input ends early or the argument uses the indefinite form, which only a string,
     * array or map may.
     *
     * \lang simp-chinese
     * @brief 读取首字节之后的参数。
     *
     * @return 输入提前结束，或参数使用不定长形式（只有字符串、数组或映射可用）时返回 false。
     */
    inline bool take_argument(const std::uint8_t additional, const std::uint8_t *&ptr, const std::uint8_t *end,
                              std::uint64_t &argument) noexcept {
        if (additional < 24) {
            argument = additional;
            return true;
        }
        switch (additional) {
            case 24: {
                std::uint8_t value = 0;
                if (!take_u8(ptr, end, value)) {
                    return false;
                }
                argument = value;
                return true;
            }
            case 25: {
                std::uint16_t value = 0;
                if (!take_be16(ptr, end, value)) {
                    return false;
                }
                argument = value;
                return true;
            }
            case 26: {
                std::uint32_t value = 0;
                if (!take_be32(ptr, end, value)) {
                    return false;
                }
                argument = value;
                return true;
            }
            case 27: {
                std::uint64_t value = 0;
                if (!take_be64(ptr, end, value)) {
                    return false;
                }
                argument = value;
                return true;
            }
            default:
                return false;
        }
    }

    /**
     * \lang english
     * @brief Reinterprets a big endian half precision bit pattern as a double.
     *
     * \lang simp-chinese
     * @brief 把大端半精度位模式重新解释为 double。
     */
    inline double half_to_double(const std::uint16_t bits) noexcept {
        const std::uint16_t sign = static_cast<std::uint16_t>((bits >> 15) & 0x1u);
        const std::uint16_t exponent = static_cast<std::uint16_t>((bits >> 10) & 0x1fu);
        const std::uint16_t fraction = static_cast<std::uint16_t>(bits & 0x3ffu);
        double value = 0;
        if (exponent == 0) {
            value = static_cast<double>(fraction) * 5.9604644775390625e-8;
        } else if (exponent == 0x1f) {
            value = fraction == 0 ? (utility::numeric_limits<double>::infinity)()
                                  : (utility::numeric_limits<double>::quiet_NaN)();
        } else {
            value = (1.0 + static_cast<double>(fraction) / 1024.0) * std::pow(2.0, static_cast<double>(exponent) - 15.0);
        }
        return sign != 0 ? -value : value;
    }

    inline std::uint16_t float_to_half_rounded(const float value) noexcept {
        std::uint32_t raw = 0;
        core::builtin::copy_memory(&raw, &value, sizeof(float));
        const std::uint32_t sign = (raw >> 16) & 0x8000u;
        const std::int32_t exponent = static_cast<std::int32_t>((raw >> 23) & 0xffu);
        const std::uint32_t mantissa = raw & 0x7fffffu;
        const std::int32_t half_exponent = exponent - 127 + 15;
        if (exponent == 0xff) {
            return static_cast<std::uint16_t>(sign | 0x7c00u | (mantissa != 0 ? 0x200u : 0u));
        }
        if (half_exponent >= 0x1f) {
            return static_cast<std::uint16_t>(sign | 0x7c00u);
        }
        if (half_exponent <= 0) {
            if (half_exponent < -10) {
                return static_cast<std::uint16_t>(sign);
            }
            const std::uint32_t aligned = mantissa | 0x800000u;
            const auto shift = static_cast<std::uint32_t>(14 - half_exponent);
            const std::uint32_t half_mantissa = aligned >> shift;
            const std::uint32_t remainder = aligned & ((1u << shift) - 1u);
            std::uint32_t result = half_mantissa;
            const std::uint32_t halfway = 1u << (shift - 1u);
            if (remainder > halfway || (remainder == halfway && (result & 1u) != 0)) {
                ++result;
            }
            return static_cast<std::uint16_t>(sign | result);
        }
        std::uint32_t result = sign | (static_cast<std::uint32_t>(half_exponent) << 10) | (mantissa >> 13);
        const std::uint32_t remainder = mantissa & 0x1fffu;
        if (remainder > 0x1000u || (remainder == 0x1000u && (result & 1u) != 0)) {
            ++result;
        }
        return static_cast<std::uint16_t>(result);
    }

    /**
     * \lang english
     * @brief Writes a float in the narrowest of half, single and double that reproduces it exactly.
     *
     * \lang simp-chinese
     * @brief 以半精度、单精度与双精度中能精确再现该浮点数的最窄形式写出它。
     */
    inline void push_float_narrowest(byte_buffer &out, const double value) {
        const float narrowed = static_cast<float>(value);
        if (static_cast<double>(narrowed) == value) {
            const std::uint16_t half = float_to_half_rounded(narrowed);
            if (half_to_double(half) == value) {
                out.push_back(0xf9);
                out.push_back(static_cast<std::uint8_t>((half >> 8) & 0xffu));
                out.push_back(static_cast<std::uint8_t>(half & 0xffu));
                return;
            }
            std::uint32_t bits = 0;
            core::builtin::copy_memory(&bits, &narrowed, sizeof(float));
            out.push_back(0xfa);
            for (int shift = 24; shift >= 0; shift -= 8) {
                out.push_back(static_cast<std::uint8_t>((bits >> shift) & 0xffu));
            }
            return;
        }
        std::uint64_t bits = 0;
        core::builtin::copy_memory(&bits, &value, sizeof(double));
        out.push_back(0xfb);
        for (int shift = 56; shift >= 0; shift -= 8) {
            out.push_back(static_cast<std::uint8_t>((bits >> shift) & 0xffu));
        }
    }
}

#endif

#endif
