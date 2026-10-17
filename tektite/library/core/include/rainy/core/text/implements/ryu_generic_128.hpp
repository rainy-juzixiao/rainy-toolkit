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
#ifndef RAINY_YESOD_TEXT_IMPLEMENTS_RYU_GENERIC_128_HPP
#define RAINY_YESOD_TEXT_IMPLEMENTS_RYU_GENERIC_128_HPP
#include <rainy/core/container/compressed_pair.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>

#if defined(__SIZEOF_INT128__)
namespace rainy::core::text::implements::ryu {
    /**
     * \lang english
     * @brief A floating decimal representing (-1)^sign * mantissa * 10^exponent.
     *
     * \lang simp-chinese
     * @brief 表示 (-1)^sign * mantissa * 10^exponent 的十进制浮点数。
     */
    struct floating_decimal_128 {
        unsigned __int128 mantissa;
        std::int32_t exponent;
        bool sign;
    };

    /**
     * \lang english
     * @brief Converts a binary floating-point number to its shortest decimal representation.
     *
     * @param bits The raw bit pattern of the value
     * @param mantissa_bits The number of mantissa bits (including the explicit leading bit when present)
     * @param exponent_bits The number of exponent bits
     * @param explicit_leading_bit Whether the leading mantissa bit is stored explicitly
     * @return The shortest decimal representation that round-trips
     *
     * \lang simp-chinese
     * @brief 将二进制浮点数转换为最短十进制表示。
     *
     * @param bits 该值的原始位模式
     * @param mantissa_bits 尾数位数（存在显式整数位时包含它）
     * @param exponent_bits 指数位数
     * @param explicit_leading_bit 尾数首位是否显式存储
     * @return 可往返的最短十进制表示
     */
    floating_decimal_128 generic_binary_to_decimal(unsigned __int128 bits, std::uint32_t mantissa_bits,
                                                   std::uint32_t exponent_bits, bool explicit_leading_bit) noexcept;

    /**
     * \lang english
     * @brief Converts a long double to its shortest decimal representation.
     *
     * \lang simp-chinese
     * @brief 将 long double 转换为最短十进制表示。
     */
    floating_decimal_128 long_double_to_fd128(long double value) noexcept;

    /**
     * \lang english
     * @brief Formats a shortest decimal representation into the given character range.
     *
     * \lang simp-chinese
     * @brief 将最短十进制表示格式化到给定字符范围。
     */
    template <typename CharType>
    container::compressed_pair<CharType *, std::errc> generic_to_chars(CharType *first, CharType *end,
                                                                      const floating_decimal_128 &value,
                                                                      chars_format fmt, long double original) noexcept;

    /**
     * \lang english
     * @brief Formats a long double with a fixed number of significant or fractional digits.
     *
     * @param first Pointer to the beginning of the output buffer
     * @param end Pointer past the end of the output buffer
     * @param value The value to convert
     * @param fmt The formatting style
     * @param precision The number of digits after the decimal point, or significant digits for
     *        chars_format::general
     * @return The end of the written range and an error code
     *
     * \lang simp-chinese
     * @brief 按指定的有效位数或小数位数格式化 long double。
     *
     * @param first 输出缓冲区起始指针
     * @param end 输出缓冲区末尾之后指针
     * @param value 待转换的值
     * @param fmt 格式样式
     * @param precision 小数点后的位数；chars_format::general 时为有效位数
     * @return 写入范围的末尾与错误码
     */
    template <typename CharType>
    container::compressed_pair<CharType *, std::errc> generic_to_chars_precision(CharType *first, CharType *end,
                                                                                long double value, chars_format fmt,
                                                                                int precision) noexcept;
}
#endif

#endif
