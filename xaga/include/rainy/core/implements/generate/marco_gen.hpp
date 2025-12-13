/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_CORE_IMPLEMENTS_MARCO_GEN_HPP
#define RAINY_CORE_IMPLEMENTS_MARCO_GEN_HPP

#define RAINY_TO_TUPLE_EXPAND_ARGS(N) RAINY_TO_TUPLE_EXPAND_##N
#define RAINY_TO_TUPLE_EXPAND_1 _1
#define RAINY_TO_TUPLE_EXPAND_2 _1, _2
#define RAINY_TO_TUPLE_EXPAND_3 _1, _2, _3
#define RAINY_TO_TUPLE_EXPAND_4 _1, _2, _3, _4
#define RAINY_TO_TUPLE_EXPAND_5 _1, _2, _3, _4, _5
#define RAINY_TO_TUPLE_EXPAND_6 _1, _2, _3, _4, _5, _6
#define RAINY_TO_TUPLE_EXPAND_7 _1, _2, _3, _4, _5, _6, _7
#define RAINY_TO_TUPLE_EXPAND_8 _1, _2, _3, _4, _5, _6, _7, _8
#define RAINY_TO_TUPLE_EXPAND_9 _1, _2, _3, _4, _5, _6, _7, _8, _9
#define RAINY_TO_TUPLE_EXPAND_10 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10
#define RAINY_TO_TUPLE_EXPAND_11 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11
#define RAINY_TO_TUPLE_EXPAND_12 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12
#define RAINY_TO_TUPLE_EXPAND_13 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13
#define RAINY_TO_TUPLE_EXPAND_14 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14
#define RAINY_TO_TUPLE_EXPAND_15 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15
#define RAINY_TO_TUPLE_EXPAND_16 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
#define RAINY_TO_TUPLE_EXPAND_17 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17
#define RAINY_TO_TUPLE_EXPAND_18 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18
#define RAINY_TO_TUPLE_EXPAND_19 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19
#define RAINY_TO_TUPLE_EXPAND_20 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20
#define RAINY_TO_TUPLE_EXPAND_21 RAINY_TO_TUPLE_EXPAND_20, _21
#define RAINY_TO_TUPLE_EXPAND_22 RAINY_TO_TUPLE_EXPAND_21, _22
#define RAINY_TO_TUPLE_EXPAND_23 RAINY_TO_TUPLE_EXPAND_22, _23
#define RAINY_TO_TUPLE_EXPAND_24 RAINY_TO_TUPLE_EXPAND_23, _24
#define RAINY_TO_TUPLE_EXPAND_25 RAINY_TO_TUPLE_EXPAND_24, _25
#define RAINY_TO_TUPLE_EXPAND_26 RAINY_TO_TUPLE_EXPAND_25, _26
#define RAINY_TO_TUPLE_EXPAND_27 RAINY_TO_TUPLE_EXPAND_26, _27
#define RAINY_TO_TUPLE_EXPAND_28 RAINY_TO_TUPLE_EXPAND_27, _28
#define RAINY_TO_TUPLE_EXPAND_29 RAINY_TO_TUPLE_EXPAND_28, _29
#define RAINY_TO_TUPLE_EXPAND_30 RAINY_TO_TUPLE_EXPAND_29, _30
#define RAINY_TO_TUPLE_EXPAND_31 RAINY_TO_TUPLE_EXPAND_30, _31
#define RAINY_TO_TUPLE_EXPAND_32 RAINY_TO_TUPLE_EXPAND_31, _32
#define RAINY_TO_TUPLE_EXPAND_33 RAINY_TO_TUPLE_EXPAND_32, _33
#define RAINY_TO_TUPLE_EXPAND_34 RAINY_TO_TUPLE_EXPAND_33, _34
#define RAINY_TO_TUPLE_EXPAND_35 RAINY_TO_TUPLE_EXPAND_34, _35
#define RAINY_TO_TUPLE_EXPAND_36 RAINY_TO_TUPLE_EXPAND_35, _36
#define RAINY_TO_TUPLE_EXPAND_37 RAINY_TO_TUPLE_EXPAND_36, _37
#define RAINY_TO_TUPLE_EXPAND_38 RAINY_TO_TUPLE_EXPAND_37, _38
#define RAINY_TO_TUPLE_EXPAND_39 RAINY_TO_TUPLE_EXPAND_38, _39
#define RAINY_TO_TUPLE_EXPAND_40 RAINY_TO_TUPLE_EXPAND_39, _40
#define RAINY_TO_TUPLE_EXPAND_41 RAINY_TO_TUPLE_EXPAND_40, _41
#define RAINY_TO_TUPLE_EXPAND_42 RAINY_TO_TUPLE_EXPAND_41, _42
#define RAINY_TO_TUPLE_EXPAND_43 RAINY_TO_TUPLE_EXPAND_42, _43
#define RAINY_TO_TUPLE_EXPAND_44 RAINY_TO_TUPLE_EXPAND_43, _44
#define RAINY_TO_TUPLE_EXPAND_45 RAINY_TO_TUPLE_EXPAND_44, _45
#define RAINY_TO_TUPLE_EXPAND_46 RAINY_TO_TUPLE_EXPAND_45, _46
#define RAINY_TO_TUPLE_EXPAND_47 RAINY_TO_TUPLE_EXPAND_46, _47
#define RAINY_TO_TUPLE_EXPAND_48 RAINY_TO_TUPLE_EXPAND_47, _48
#define RAINY_TO_TUPLE_EXPAND_49 RAINY_TO_TUPLE_EXPAND_48, _49
#define RAINY_TO_TUPLE_EXPAND_50 RAINY_TO_TUPLE_EXPAND_49, _50
#define RAINY_TO_TUPLE_EXPAND_51 RAINY_TO_TUPLE_EXPAND_50, _51
#define RAINY_TO_TUPLE_EXPAND_52 RAINY_TO_TUPLE_EXPAND_51, _52
#define RAINY_TO_TUPLE_EXPAND_53 RAINY_TO_TUPLE_EXPAND_52, _53
#define RAINY_TO_TUPLE_EXPAND_54 RAINY_TO_TUPLE_EXPAND_53, _54
#define RAINY_TO_TUPLE_EXPAND_55 RAINY_TO_TUPLE_EXPAND_54, _55
#define RAINY_TO_TUPLE_EXPAND_56 RAINY_TO_TUPLE_EXPAND_55, _56
#define RAINY_TO_TUPLE_EXPAND_57 RAINY_TO_TUPLE_EXPAND_56, _57
#define RAINY_TO_TUPLE_EXPAND_58 RAINY_TO_TUPLE_EXPAND_57, _58
#define RAINY_TO_TUPLE_EXPAND_59 RAINY_TO_TUPLE_EXPAND_58, _59
#define RAINY_TO_TUPLE_EXPAND_60 RAINY_TO_TUPLE_EXPAND_59, _60
#define RAINY_TO_TUPLE_EXPAND_61 RAINY_TO_TUPLE_EXPAND_60, _61
#define RAINY_TO_TUPLE_EXPAND_62 RAINY_TO_TUPLE_EXPAND_61, _62
#define RAINY_TO_TUPLE_EXPAND_63 RAINY_TO_TUPLE_EXPAND_62, _63
#define RAINY_TO_TUPLE_EXPAND_64 RAINY_TO_TUPLE_EXPAND_63, _64
#define RAINY_TO_TUPLE_EXPAND_65 RAINY_TO_TUPLE_EXPAND_64, _65
#define RAINY_TO_TUPLE_EXPAND_66 RAINY_TO_TUPLE_EXPAND_65, _66
#define RAINY_TO_TUPLE_EXPAND_67 RAINY_TO_TUPLE_EXPAND_66, _67
#define RAINY_TO_TUPLE_EXPAND_68 RAINY_TO_TUPLE_EXPAND_67, _68
#define RAINY_TO_TUPLE_EXPAND_69 RAINY_TO_TUPLE_EXPAND_68, _69
#define RAINY_TO_TUPLE_EXPAND_70 RAINY_TO_TUPLE_EXPAND_69, _70
#define RAINY_TO_TUPLE_EXPAND_71 RAINY_TO_TUPLE_EXPAND_70, _71
#define RAINY_TO_TUPLE_EXPAND_72 RAINY_TO_TUPLE_EXPAND_71, _72
#define RAINY_TO_TUPLE_EXPAND_73 RAINY_TO_TUPLE_EXPAND_72, _73
#define RAINY_TO_TUPLE_EXPAND_74 RAINY_TO_TUPLE_EXPAND_73, _74
#define RAINY_TO_TUPLE_EXPAND_75 RAINY_TO_TUPLE_EXPAND_74, _75
#define RAINY_TO_TUPLE_EXPAND_76 RAINY_TO_TUPLE_EXPAND_75, _76
#define RAINY_TO_TUPLE_EXPAND_77 RAINY_TO_TUPLE_EXPAND_76, _77
#define RAINY_TO_TUPLE_EXPAND_78 RAINY_TO_TUPLE_EXPAND_77, _78
#define RAINY_TO_TUPLE_EXPAND_79 RAINY_TO_TUPLE_EXPAND_78, _79
#define RAINY_TO_TUPLE_EXPAND_80 RAINY_TO_TUPLE_EXPAND_79, _80

#define RAINY_ARG_COUNT_EXPAND(x) x

#define RAINY_ARG_COUNT(...)                                                                                                          \
    RAINY_ARG_COUNT_EXPAND(RAINY_ARG_COUNT_IMPL(__VA_ARGS__, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63,  \
                                                62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42,   \
                                                41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,   \
                                                20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define RAINY_ARG_COUNT_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22,     \
                             _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, \
                             _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, \
                             _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, N, ...)                  \
    N

// clang-format off

#define RAINY_FE_EXPAND(...) __VA_ARGS__
#define RAINY_FE_EXPAND1(...) RAINY_FE_EXPAND(__VA_ARGS__)
#define RAINY_FE_EXPAND2(...) RAINY_FE_EXPAND1(RAINY_FE_EXPAND1(__VA_ARGS__))
#define RAINY_FE_EXPAND3(...) RAINY_FE_EXPAND2(RAINY_FE_EXPAND2(__VA_ARGS__))
#define RAINY_FE_EXPAND4(...) RAINY_FE_EXPAND3(RAINY_FE_EXPAND3(__VA_ARGS__))
#define RAINY_FE_EXPAND5(...) RAINY_FE_EXPAND4(RAINY_FE_EXPAND4(__VA_ARGS__))

#define RAINY_FE_NARG(...) RAINY_FE_NARG_(__VA_ARGS__, RAINY_FE_RSEQ_N())
#define RAINY_FE_NARG_(...) RAINY_FE_EXPAND(RAINY_FE_ARG_N(__VA_ARGS__))
#define RAINY_FE_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define RAINY_FE_RSEQ_N() 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define RAINY_FE_FOR_EACH_1(macro, obj, a) macro(obj, a)
#define RAINY_FE_FOR_EACH_2(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_1(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_3(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_2(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_4(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_3(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_5(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_4(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_6(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_5(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_7(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_6(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_8(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_7(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_9(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_8(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_10(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_9(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_11(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_10(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_12(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_11(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_13(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_12(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_14(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_13(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_15(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_14(macro, obj, __VA_ARGS__))
#define RAINY_FE_FOR_EACH_16(macro, obj, a, ...) macro(obj, a), RAINY_FE_EXPAND(RAINY_FE_FOR_EACH_15(macro, obj, __VA_ARGS__))

#define RAINY_FE_CAT(a, b) RAINY_FE_CAT_IMPL(a, b)
#define RAINY_FE_CAT_IMPL(a, b) a##b

#define RAINY_FE_FOR_EACH(macro, obj, ...) RAINY_FE_EXPAND5(RAINY_FE_CAT(RAINY_FE_FOR_EACH_, RAINY_FE_NARG(__VA_ARGS__))(macro, obj, __VA_ARGS__))

#define RAINY_GENERATE_MACRO_FOR_256(macro) \
    macro(1) macro(2) macro(3) macro(4) macro(5) macro(6) macro(7) macro(8) macro(9) macro(10) \
    macro(11) macro(12) macro(13) macro(14) macro(15) macro(16) macro(17) macro(18) macro(19) macro(20) \
    macro(21) macro(22) macro(23) macro(24) macro(25) macro(26) macro(27) macro(28) macro(29) macro(30) \
    macro(31) macro(32) macro(33) macro(34) macro(35) macro(36) macro(37) macro(38) macro(39) macro(40) \
    macro(41) macro(42) macro(43) macro(44) macro(45) macro(46) macro(47) macro(48) macro(49) macro(50) \
    macro(51) macro(52) macro(53) macro(54) macro(55) macro(56) macro(57) macro(58) macro(59) macro(60) \
    macro(61) macro(62) macro(63) macro(64) macro(65) macro(66) macro(67) macro(68) macro(69) macro(70) \
    macro(71) macro(72) macro(73) macro(74) macro(75) macro(76) macro(77) macro(78) macro(79) macro(80) \
    macro(81) macro(82) macro(83) macro(84) macro(85) macro(86) macro(87) macro(88) macro(89) macro(90) \
    macro(91) macro(92) macro(93) macro(94) macro(95) macro(96) macro(97) macro(98) macro(99) macro(100) \
    macro(101) macro(102) macro(103) macro(104) macro(105) macro(106) macro(107) macro(108) macro(109) macro(110) \
    macro(111) macro(112) macro(113) macro(114) macro(115) macro(116) macro(117) macro(118) macro(119) macro(120) \
    macro(121) macro(122) macro(123) macro(124) macro(125) macro(126) macro(127) macro(128) macro(129) macro(130) \
    macro(131) macro(132) macro(133) macro(134) macro(135) macro(136) macro(137) macro(138) macro(139) macro(140) \
    macro(141) macro(142) macro(143) macro(144) macro(145) macro(146) macro(147) macro(148) macro(149) macro(150) \
    macro(151) macro(152) macro(153) macro(154) macro(155) macro(156) macro(157) macro(158) macro(159) macro(160) \
    macro(161) macro(162) macro(163) macro(164) macro(165) macro(166) macro(167) macro(168) macro(169) macro(170) \
    macro(171) macro(172) macro(173) macro(174) macro(175) macro(176) macro(177) macro(178) macro(179) macro(180) \
    macro(181) macro(182) macro(183) macro(184) macro(185) macro(186) macro(187) macro(188) macro(189) macro(190) \
    macro(191) macro(192) macro(193) macro(194) macro(195) macro(196) macro(197) macro(198) macro(199) macro(200) \
    macro(201) macro(202) macro(203) macro(204) macro(205) macro(206) macro(207) macro(208) macro(209) macro(210) \
    macro(211) macro(212) macro(213) macro(214) macro(215) macro(216) macro(217) macro(218) macro(219) macro(220) \
    macro(221) macro(222) macro(223) macro(224) macro(225) macro(226) macro(227) macro(228) macro(229) macro(230) \
    macro(231) macro(232) macro(233) macro(234) macro(235) macro(236) macro(237) macro(238) macro(239) macro(240) \
    macro(241) macro(242) macro(243) macro(244) macro(245) macro(246) macro(247) macro(248) macro(249) macro(250) \
    macro(251) macro(252) macro(253) macro(254) macro(255) macro(256)
    
// clang-format on

#endif