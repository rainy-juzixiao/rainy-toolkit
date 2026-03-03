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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_IMPLEMENTS_RYU_TABLE_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_IMPLEMENTS_RYU_TABLE_HPP
#include <rainy/core/platform.hpp>

namespace rainy::foundation::text::implements::ryu {
    template <typename Floating>
    struct general_precision_tables_2;

    template <>
    struct general_precision_tables_2<float> {
        static constexpr int max_special_p = 7;
        static const uint32_t speical_x_table[63];
        static constexpr int max_p = 39;
        static const uint32_t ordinary_x_table[44];
    };

    template <>
    struct general_precision_tables_2<double> {
        static constexpr int max_special_p = 15;
        static const uint64_t speical_x_table[195];
        static constexpr int max_p = 309;
        static const uint64_t ordinary_x_table[314];
    };
}

// 参考 digit_table.h
// https://github.com/ulfjack/ryu/blob/master/ryu/digit_table.h
namespace rainy::foundation::text::implements::ryu {
    template <typename CharType>
    constexpr CharType DIGIT_TABLE[] = {CharType{}};

    template <>
    inline constexpr char DIGIT_TABLE<char>[200] = {
        '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', '1', '0', '1', '1', '1',
        '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
        '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
        '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
        '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9', '6', '0', '6', '1', '6',
        '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
        '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8',
        '7', '8', '8', '8', '9', '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'};

    template <>
    inline constexpr wchar_t DIGIT_TABLE<wchar_t>[200] = {
        L'0', L'0', L'0', L'1', L'0', L'2', L'0', L'3', L'0', L'4', L'0', L'5', L'0', L'6', L'0', L'7', L'0', L'8', L'0', L'9',
        L'1', L'0', L'1', L'1', L'1', L'2', L'1', L'3', L'1', L'4', L'1', L'5', L'1', L'6', L'1', L'7', L'1', L'8', L'1', L'9',
        L'2', L'0', L'2', L'1', L'2', L'2', L'2', L'3', L'2', L'4', L'2', L'5', L'2', L'6', L'2', L'7', L'2', L'8', L'2', L'9',
        L'3', L'0', L'3', L'1', L'3', L'2', L'3', L'3', L'3', L'4', L'3', L'5', L'3', L'6', L'3', L'7', L'3', L'8', L'3', L'9',
        L'4', L'0', L'4', L'1', L'4', L'2', L'4', L'3', L'4', L'4', L'4', L'5', L'4', L'6', L'4', L'7', L'4', L'8', L'4', L'9',
        L'5', L'0', L'5', L'1', L'5', L'2', L'5', L'3', L'5', L'4', L'5', L'5', L'5', L'6', L'5', L'7', L'5', L'8', L'5', L'9',
        L'6', L'0', L'6', L'1', L'6', L'2', L'6', L'3', L'6', L'4', L'6', L'5', L'6', L'6', L'6', L'7', L'6', L'8', L'6', L'9',
        L'7', L'0', L'7', L'1', L'7', L'2', L'7', L'3', L'7', L'4', L'7', L'5', L'7', L'6', L'7', L'7', L'7', L'8', L'7', L'9',
        L'8', L'0', L'8', L'1', L'8', L'2', L'8', L'3', L'8', L'4', L'8', L'5', L'8', L'6', L'8', L'7', L'8', L'8', L'8', L'9',
        L'9', L'0', L'9', L'1', L'9', L'2', L'9', L'3', L'9', L'4', L'9', L'5', L'9', L'6', L'9', L'7', L'9', L'8', L'9', L'9'};
}

// 参考 d2fixed_full_table.h
// https://github.com/ulfjack/ryu/blob/master/ryu/d2fixed_full_table.h
namespace rainy::foundation::text::implements::ryu {
    inline constexpr int TABLE_SIZE = 64;

    inline constexpr uint16_t POW10_OFFSET[TABLE_SIZE] = {
        0,   2,   5,   8,   12,  16,  21,  26,  32,  39,  46,  54,  62,  71,  80,   90,   100,  111,  122,  134, 146, 159,
        173, 187, 202, 217, 233, 249, 266, 283, 301, 319, 338, 357, 377, 397, 418,  440,  462,  485,  508,  532, 556, 581,
        606, 632, 658, 685, 712, 740, 769, 798, 828, 858, 889, 920, 952, 984, 1017, 1050, 1084, 1118, 1153, 1188};

    extern const std::uint64_t POW10_SPLIT[1224][3];

    inline constexpr int TABLE_SIZE_2 = 69;
    inline constexpr int ADDITIONAL_BITS_2 = 120;

    inline constexpr uint16_t POW10_OFFSET_2[TABLE_SIZE_2] = {
        0,    2,    6,    12,   20,   29,   40,   52,   66,   80,   95,   112,  130,  150,  170,  192,  215,  240,
        265,  292,  320,  350,  381,  413,  446,  480,  516,  552,  590,  629,  670,  712,  755,  799,  845,  892,
        940,  989,  1040, 1092, 1145, 1199, 1254, 1311, 1369, 1428, 1488, 1550, 1613, 1678, 1743, 1810, 1878, 1947,
        2017, 2088, 2161, 2235, 2311, 2387, 2465, 2544, 2625, 2706, 2789, 2873, 2959, 3046, 3133};

    inline constexpr uint8_t MIN_BLOCK_2[TABLE_SIZE_2] = {0,  0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,
                                                            7,  8,  8,  9,  9,  10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16,
                                                            17, 17, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26,
                                                            27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 34, 0};

    extern const std::uint64_t POW10_SPLIT_2[3133][3];
}

// 参考 d2s_full_table.h
// https://github.com/ulfjack/ryu/blob/master/ryu/d2s_full_table.h
namespace rainy::foundation::text::implements::ryu {
    // Def sources/core/ryu_table.cxx
    extern const std::uint64_t DOUBLE_POW5_INV_SPLIT[292][2];
    extern const std::uint64_t DOUBLE_POW5_SPLIT[326][2];
}

#endif