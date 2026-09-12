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
#ifndef RAINY_CORE_TEXT_WSTRING_CONVERT_HPP
#define RAINY_CORE_TEXT_WSTRING_CONVERT_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/underlying_type.hpp>

namespace rainy::core::text {
    /**
     * \lang english
     * @brief Mode flags controlling byte-order-mark handling and byte order for the codecvt facets.
     *
     * \lang simp-chinese
     * @brief 控制 codecvt facet 的字节序标记处理与字节序的模式标志。
     */
    enum class codecvt_mode {
        /**
         * \lang english
         * @brief Consumes a leading byte-order mark when converting from bytes.
         *
         * \lang simp-chinese
         * @brief 从字节转换时消费开头的字节序标记。
         */
        consume_header = 4,
        /**
         * \lang english
         * @brief Generates a byte-order mark when converting to bytes.
         *
         * \lang simp-chinese
         * @brief 转换为字节时生成字节序标记。
         */
        generate_header = 2,
        /**
         * \lang english
         * @brief Uses little-endian byte order for UTF-16 conversions.
         *
         * \lang simp-chinese
         * @brief UTF-16 转换使用小端字节序。
         */
        little_endian = 1
    };

    using codecvt_mode::consume_header;
    using codecvt_mode::generate_header;
    using codecvt_mode::little_endian;

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(codecvt_mode);

    /**
     * \lang english
     * @brief Converts between UTF-8 byte sequences and wide-character strings.
     *
     * @tparam Elem The wide-character element type
     * @tparam Maxcode The maximum Unicode code point allowed
     * @tparam Mode The conversion mode flags
     *
     * \lang simp-chinese
     * @brief 在 UTF-8 字节序列与宽字符串之间进行转换。
     *
     * @tparam Elem 宽字符元素类型
     * @tparam Maxcode 允许的最大 Unicode 码点
     * @tparam Mode 转换模式标志
     */
    template <typename Elem, unsigned long Maxcode = 0x10ffff, codecvt_mode Mode = codecvt_mode{}>
    class codecvt_utf8 {
    public:
        /**
         * \lang english
         * @brief The byte (octet) element type.
         *
         * \lang simp-chinese
         * @brief 字节（八位组）元素类型。
         */
        using byte_type = char;
        /**
         * \lang english
         * @brief The wide-character element type.
         *
         * \lang simp-chinese
         * @brief 宽字符元素类型。
         */
        using wide_type = Elem;

        /**
         * \lang english
         * @brief The outcome of a conversion step.
         *
         * \lang simp-chinese
         * @brief 一次转换步骤的结果状态。
         */
        enum class result {
            ok,
            partial,
            error,
            noconv
        };

        /**
         * \lang english
         * @brief Converts UTF-8 bytes to wide characters.
         *
         * @param from_begin Pointer to the first byte to convert
         * @param from_end Pointer one past the last byte to convert
         * @param from_next On output, points one past the last byte consumed
         * @param to_begin Pointer to the first wide-character output element
         * @param to_end Pointer one past the last wide-character output element
         * @param to_next On output, points one past the last element written
         * @param seen_header Tracks whether a byte-order mark has been seen
         * @return The conversion result
         *
         * \lang simp-chinese
         * @brief 将 UTF-8 字节转换为宽字符。
         *
         * @param from_begin 指向待转换首字节的指针
         * @param from_end 指向末字节之后位置的指针
         * @param from_next 输出时指向最后一个已消费字节之后
         * @param to_begin 指向首个宽字符输出元素的指针
         * @param to_end 指向末个宽字符输出元素之后位置的指针
         * @param to_next 输出时指向最后一个已写入元素之后
         * @param seen_header 跟踪是否已遇到字节序标记
         * @return 转换结果
         */
        static result to_wide(const byte_type *from_begin, const byte_type *from_end, const byte_type *&from_next, wide_type *to_begin,
                              wide_type *to_end, wide_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            while (from_next != from_end && to_next != to_end) {
                unsigned long by = static_cast<unsigned char>(*from_next);
                unsigned long ch = 0;
                int nextra = 0;

                if (by < 0x80u) {
                    ch = by;
                    nextra = 0;
                } else if (by < 0xc0u) {
                    // 0x80-0xbf 不是首字节
                    ++from_next;
                    return result::error;
                } else if (by < 0xe0u) {
                    ch = by & 0x1f;
                    nextra = 1;
                } else if (by < 0xf0u) {
                    ch = by & 0x0f;
                    nextra = 2;
                } else if (by < 0xf8u) {
                    ch = by & 0x07;
                    nextra = 3;
                } else {
                    ch = by & 0x03;
                    nextra = by < 0xfc ? 4 : 5;
                }

                if (nextra == 0) {
                    ++from_next;
                } else if (from_end - from_next < nextra + 1) {
                    break; // 输入不足
                } else {
                    for (++from_next; 0 < nextra; --nextra, ++from_next) {
                        if ((by = static_cast<unsigned char>(*from_next)) < 0x80u || 0xc0u <= by) {
                            return result::error; // 不是延续字节
                        }
                        ch = ch << 6 | (by & 0x3f);
                    }
                }

                // 检查并消费 BOM
                if (!seen_header) {
                    seen_header = true;

                    if constexpr (constexpr bool consuming = (Mode & consume_header) != static_cast<codecvt_mode>(0)) {
                        if (ch == 0xfeff) {
                            // 跳过 BOM 并重试
                            continue;
                        }
                    }
                }

                if (Maxcode < ch) {
                    return result::error; // 码点太大
                }

#ifdef RAINY_USING_WINDOWS
                if (ch > 0xffff) {
                    if (to_end - to_next < 2) {
                        break;
                    }
                    unsigned long adjusted = ch - 0x10000;
                    *to_next++ = static_cast<wide_type>(0xD800 | (adjusted >> 10));
                    *to_next++ = static_cast<wide_type>(0xDC00 | (adjusted & 0x3FF));
                } else {
                    *to_next++ = static_cast<wide_type>(ch);
                }
#else
                *to_next++ = static_cast<wide_type>(ch);
#endif
            }

            return from_begin == from_next ? result::partial : result::ok;
        }

        /**
         * \lang english
         * @brief Converts wide characters to UTF-8 bytes.
         *
         * @param from_begin Pointer to the first wide character to convert
         * @param from_end Pointer one past the last wide character to convert
         * @param from_next On output, points one past the last wide character consumed
         * @param to_begin Pointer to the first byte output element
         * @param to_end Pointer one past the last byte output element
         * @param to_next On output, points one past the last element written
         * @param seen_header Tracks whether a byte-order mark has been seen
         * @return The conversion result
         *
         * \lang simp-chinese
         * @brief 将宽字符转换为 UTF-8 字节。
         *
         * @param from_begin 指向待转换首宽字符的指针
         * @param from_end 指向末宽字符之后位置的指针
         * @param from_next 输出时指向最后一个已消费宽字符之后
         * @param to_begin 指向首字节输出元素的指针
         * @param to_end 指向末字节输出元素之后位置的指针
         * @param to_next 输出时指向最后一个已写入元素之后
         * @param seen_header 跟踪是否已遇到字节序标记
         * @return 转换结果
         */
       static result to_bytes(const wide_type *from_begin, const wide_type *from_end, const wide_type *&from_next,
                               byte_type *to_begin, byte_type *to_end, byte_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            while (from_next != from_end && to_next != to_end) {
                unsigned long ch = static_cast<unsigned long>(*from_next);
                int consumed = 1;

#ifdef RAINY_USING_WINDOWS
                if (ch >= 0xD800 && ch <= 0xDBFF) {
                    if (from_end - from_next < 2) {
                        break;
                    }
                    unsigned long low = static_cast<unsigned long>(*(from_next + 1));
                    if (low < 0xDC00 || low > 0xDFFF) {
                        return result::error;
                    }
                    ch = 0x10000 + ((ch - 0xD800) << 10) + (low - 0xDC00);
                    consumed = 2;
                } else if (ch >= 0xDC00 && ch <= 0xDFFF) {
                    return result::error;
                }
#endif
                from_next += consumed;

                if (Maxcode < ch) {
                    return result::error;
                }

                byte_type by;
                int nextra;

                if (ch < 0x0080u) {
                    by = static_cast<byte_type>(ch);
                    nextra = 0;
                } else if (ch < 0x0800u) {
                    by = static_cast<byte_type>(0xc0 | ch >> 6);
                    nextra = 1;
                } else if (ch < 0x00010000u) {
                    by = static_cast<byte_type>(0xe0 | ch >> 12);
                    nextra = 2;
                } else if (ch < 0x00200000u) {
                    by = static_cast<byte_type>(0xf0 | ch >> 18);
                    nextra = 3;
                } else if (ch < 0x04000000u) {
                    by = static_cast<byte_type>(0xf8 | ch >> 24);
                    nextra = 4;
                } else {
                    by = static_cast<byte_type>(0xfc | (ch >> 30 & 0x03));
                    nextra = 5;
                }

                // 生成 BOM
                if (!seen_header) {
                    seen_header = true;

                    constexpr bool generating = (Mode & generate_header) != static_cast<codecvt_mode>(0);
                    if constexpr (generating) {
                        if (to_end - to_next < 3 + 1 + nextra) {
                            from_next -= consumed;
                            return result::partial;
                        }

                        *to_next++ = '\xef';
                        *to_next++ = '\xbb';
                        *to_next++ = '\xbf';
                    }
                }

                if (to_end - to_next < 1 + nextra) {
                    from_next -= consumed;
                    break;
                }

                for (*to_next++ = by; 0 < nextra;) {
                    *to_next++ = static_cast<byte_type>((ch >> 6 * --nextra & 0x3f) | 0x80);
                }
            }

            return from_begin == from_next ? result::partial : result::ok;
        }
    };
}

namespace rainy::core::text {
    /**
     * \lang english
     * @brief Converts between UTF-16 byte sequences and wide-character strings, honoring the configured byte order.
     *
     * @tparam Elem The wide-character element type
     * @tparam Maxcode The maximum Unicode code point allowed
     * @tparam Mode The conversion mode flags
     *
     * \lang simp-chinese
     * @brief 在 UTF-16 字节序列与宽字符串之间进行转换，遵循配置的字节序。
     *
     * @tparam Elem 宽字符元素类型
     * @tparam Maxcode 允许的最大 Unicode 码点
     * @tparam Mode 转换模式标志
     */
    template <typename Elem, unsigned long Maxcode = 0x10ffff, codecvt_mode Mode = codecvt_mode{}>
    class codecvt_utf16 {
    public:
        /**
         * \lang english
         * @brief The byte (octet) element type.
         *
         * \lang simp-chinese
         * @brief 字节（八位组）元素类型。
         */
        using byte_type = char;
        /**
         * \lang english
         * @brief The wide-character element type.
         *
         * \lang simp-chinese
         * @brief 宽字符元素类型。
         */
        using wide_type = Elem;

        /**
         * \lang english
         * @brief The outcome of a conversion step.
         *
         * \lang simp-chinese
         * @brief 一次转换步骤的结果状态。
         */
        enum class result {
            ok,
            partial,
            error,
            noconv
        };

        /**
         * \lang english
         * @brief Converts UTF-16 bytes (in the configured byte order) to wide characters.
         *
         * @param from_begin Pointer to the first byte to convert
         * @param from_end Pointer one past the last byte to convert
         * @param from_next On output, points one past the last byte consumed
         * @param to_begin Pointer to the first wide-character output element
         * @param to_end Pointer one past the last wide-character output element
         * @param to_next On output, points one past the last element written
         * @param seen_header Tracks whether a byte-order mark has been seen
         * @return The conversion result
         *
         * \lang simp-chinese
         * @brief 将 UTF-16 字节（按配置的字节序）转换为宽字符。
         *
         * @param from_begin 指向待转换首字节的指针
         * @param from_end 指向末字节之后位置的指针
         * @param from_next 输出时指向最后一个已消费字节之后
         * @param to_begin 指向首个宽字符输出元素的指针
         * @param to_end 指向末个宽字符输出元素之后位置的指针
         * @param to_next 输出时指向最后一个已写入元素之后
         * @param seen_header 跟踪是否已遇到字节序标记
         * @return 转换结果
         */
        static result to_wide(const byte_type *from_begin, const byte_type *from_end, const byte_type *&from_next, wide_type *to_begin,
                              wide_type *to_end, wide_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            // 状态：1=小端序，2=大端序
            char endian_state = 0;

            constexpr bool prefer_le = (Mode & little_endian) != static_cast<codecvt_mode>(0);
            constexpr bool consuming = (Mode & consume_header) != static_cast<codecvt_mode>(0);

            if (consuming && from_end - from_next >= 2) {
                const auto *ptr = reinterpret_cast<const unsigned char *>(from_next);
                unsigned short bom;

                if (prefer_le) {
                    bom = static_cast<unsigned short>(ptr[1] << 8 | ptr[0]);
                } else {
                    bom = static_cast<unsigned short>(ptr[0] << 8 | ptr[1]);
                }

                if (bom == 0xfffe) {
                    // 相反的字节序
                    endian_state = prefer_le ? 2 : 1;
                    from_next += 2;
                } else if (bom == 0xfeff) {
                    // 匹配的字节序
                    endian_state = prefer_le ? 1 : 2;
                    from_next += 2;
                } else {
                    // 没有 BOM，使用默认字节序
                    endian_state = prefer_le ? 1 : 2;
                }
            } else {
                endian_state = prefer_le ? 1 : 2;
            }

            while (from_next != from_end && to_next != to_end) {
                if (from_end - from_next < 2) {
                    break; // 输入不足
                }

                const auto *ptr = reinterpret_cast<const unsigned char *>(from_next);
                unsigned long ch;
                unsigned short ch0, ch1;

                // 根据确定的字节序读取第一个字
                if (endian_state == 1) { // 小端序
                    ch0 = static_cast<unsigned short>(ptr[1] << 8 | ptr[0]);
                } else { // 大端序
                    ch0 = static_cast<unsigned short>(ptr[0] << 8 | ptr[1]);
                }

                if (ch0 < 0xd800u || ch0 >= 0xdc00u) {
                    // 单字字符
                    from_next += 2;
                    ch = ch0;
                } else if (ch0 < 0xdc00u) {
                    // 高位代理，需要读取低位代理
                    if (from_end - from_next < 4) {
                        break; // 输入不足，需要完整的代理对
                    }

                    // 读取第二个字
                    if (endian_state == 1) {
                        ch1 = static_cast<unsigned short>(ptr[3] << 8 | ptr[2]);
                    } else {
                        ch1 = static_cast<unsigned short>(ptr[2] << 8 | ptr[3]);
                    }

                    if (ch1 < 0xdc00u || ch1 >= 0xe000u) {
                        return result::error; // 无效的低位代理
                    }

                    from_next += 4;
                    ch = 0x10000 + ((ch0 - 0xd800) << 10) + (ch1 - 0xdc00);
                } else {
                    // 孤立低位代理
                    return result::error;
                }

                if (Maxcode < ch) {
                    return result::error; // 码点太大
                }

#ifdef RAINY_USING_WINDOWS
                if (ch > 0xffff) {
                    // Windows: wchar_t 是 16 位，需要输出代理对
                    if (to_end - to_next < 2) {
                        break; // 输出空间不足
                    }
                    unsigned long adjusted = ch - 0x10000;
                    *to_next++ = static_cast<wide_type>(0xD800 | (adjusted >> 10));
                    *to_next++ = static_cast<wide_type>(0xDC00 | (adjusted & 0x3FF));
                } else {
                    *to_next++ = static_cast<wide_type>(ch);
                }
#else
                *to_next++ = static_cast<wide_type>(ch);
#endif
            }

            return from_next == from_end ? result::ok : result::partial;
        }

        /**
         * \lang english
         * @brief Converts wide characters to UTF-16 bytes in the configured byte order.
         *
         * @param from_begin Pointer to the first wide character to convert
         * @param from_end Pointer one past the last wide character to convert
         * @param from_next On output, points one past the last wide character consumed
         * @param to_begin Pointer to the first byte output element
         * @param to_end Pointer one past the last byte output element
         * @param to_next On output, points one past the last element written
         * @param seen_header Tracks whether a byte-order mark has been seen
         * @return The conversion result
         *
         * \lang simp-chinese
         * @brief 将宽字符按配置的字节序转换为 UTF-16 字节。
         *
         * @param from_begin 指向待转换首宽字符的指针
         * @param from_end 指向末宽字符之后位置的指针
         * @param from_next 输出时指向最后一个已消费宽字符之后
         * @param to_begin 指向首字节输出元素的指针
         * @param to_end 指向末字节输出元素之后位置的指针
         * @param to_next 输出时指向最后一个已写入元素之后
         * @param seen_header 跟踪是否已遇到字节序标记
         * @return 转换结果
         */
        static result to_bytes(const wide_type *from_begin, const wide_type *from_end, const wide_type *&from_next,
                               byte_type *to_begin, byte_type *to_end, byte_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            // 状态：0=未确定字节序，1=小端序，2=大端序
            char endian_state = 0;

            while (from_next != from_end && to_next != to_end) {
                unsigned long ch = static_cast<unsigned long>(*from_next);

                if (Maxcode < ch) {
                    return result::error;
                }

                // 确定字节序并可能生成 BOM
                if (endian_state == 0) {
                    constexpr bool prefer_le = (Mode & little_endian) != static_cast<codecvt_mode>(0);
                    endian_state = prefer_le ? 1 : 2;

                    constexpr bool generating = (Mode & generate_header) != static_cast<codecvt_mode>(0);
                    if constexpr (generating) {
                        if (to_end - to_next < 2) {
                            return result::partial; // 空间不足
                        }

                        if (endian_state == 1) { // 小端序 BOM: 0xFF 0xFE
                            *to_next++ = '\xff';
                            *to_next++ = '\xfe';
                        } else { // 大端序 BOM: 0xFE 0xFF
                            *to_next++ = '\xfe';
                            *to_next++ = '\xff';
                        }
                    }
                }

                // 检查输出空间
                bool need_surrogate = ch >= 0x10000;
                int needed_bytes = need_surrogate ? 4 : 2;

                if (to_end - to_next < needed_bytes) {
                    break; // 输出空间不足
                }

                ++from_next;

                if (need_surrogate) {
                    // 需要代理对
                    if (ch > 0x10ffff) {
                        return result::error; // 超出 Unicode 范围
                    }

                    unsigned long adjusted = ch - 0x10000;
                    unsigned short high = static_cast<unsigned short>(0xd800 | (adjusted >> 10));
                    unsigned short low = static_cast<unsigned short>(0xdc00 | (adjusted & 0x3ff));

                    if (endian_state == 1) { // 小端序
                        // 高位代理
                        *to_next++ = static_cast<byte_type>(high);
                        *to_next++ = static_cast<byte_type>(high >> 8);
                        // 低位代理
                        *to_next++ = static_cast<byte_type>(low);
                        *to_next++ = static_cast<byte_type>(low >> 8);
                    } else { // 大端序
                        // 高位代理
                        *to_next++ = static_cast<byte_type>(high >> 8);
                        *to_next++ = static_cast<byte_type>(high);
                        // 低位代理
                        *to_next++ = static_cast<byte_type>(low >> 8);
                        *to_next++ = static_cast<byte_type>(low);
                    }
                } else {
                    // 单字字符
                    if (ch >= 0xd800 && ch < 0xe000) {
                        return result::error; // 不允许代理区码点直接编码
                    }

                    unsigned short word = static_cast<unsigned short>(ch);

                    if (endian_state == 1) { // 小端序
                        *to_next++ = static_cast<byte_type>(word);
                        *to_next++ = static_cast<byte_type>(word >> 8);
                    } else { // 大端序
                        *to_next++ = static_cast<byte_type>(word >> 8);
                        *to_next++ = static_cast<byte_type>(word);
                    }
                }
            }

            return from_begin == from_next ? result::partial : result::ok;
        }
    };
}

namespace rainy::core::text {
    /**
     * \lang english
     * @brief Converts between UTF-8 byte sequences and wide-character strings, encoding characters outside the BMP as surrogate pairs.
     *
     * @tparam Elem The wide-character element type
     * @tparam Maxcode The maximum Unicode code point allowed
     * @tparam Mode The conversion mode flags
     *
     * \lang simp-chinese
     * @brief 在 UTF-8 字节序列与宽字符串之间进行转换，对 BMP 之外的字符编码为代理对。
     *
     * @tparam Elem 宽字符元素类型
     * @tparam Maxcode 允许的最大 Unicode 码点
     * @tparam Mode 转换模式标志
     */
    template <typename Elem, unsigned long Maxcode = 0x10ffff, codecvt_mode Mode = codecvt_mode{}>
    class codecvt_utf8_utf16 {
    public:
        /**
         * \lang english
         * @brief The byte (octet) element type.
         *
         * \lang simp-chinese
         * @brief 字节（八位组）元素类型。
         */
        using byte_type = char;
        /**
         * \lang english
         * @brief The wide-character element type.
         *
         * \lang simp-chinese
         * @brief 宽字符元素类型。
         */
        using wide_type = Elem;

        /**
         * \lang english
         * @brief The outcome of a conversion step.
         *
         * \lang simp-chinese
         * @brief 一次转换步骤的结果状态。
         */
        enum class result {
            ok,
            partial,
            error,
            noconv
        };

        /**
         * \lang english
         * @brief Converts UTF-8 bytes to wide characters, decoding surrogate pairs outside the BMP.
         *
         * @param from_begin Pointer to the first byte to convert
         * @param from_end Pointer one past the last byte to convert
         * @param from_next On output, points one past the last byte consumed
         * @param to_begin Pointer to the first wide-character output element
         * @param to_end Pointer one past the last wide-character output element
         * @param to_next On output, points one past the last element written
         * @param seen_header Tracks whether a byte-order mark has been seen
         * @return The conversion result
         *
         * \lang simp-chinese
         * @brief 将 UTF-8 字节转换为宽字符，对 BMP 之外的码点解码为代理对。
         *
         * @param from_begin 指向待转换首字节的指针
         * @param from_end 指向末字节之后位置的指针
         * @param from_next 输出时指向最后一个已消费字节之后
         * @param to_begin 指向首个宽字符输出元素的指针
         * @param to_end 指向末个宽字符输出元素之后位置的指针
         * @param to_next 输出时指向最后一个已写入元素之后
         * @param seen_header 跟踪是否已遇到字节序标记
         * @return 转换结果
         */
        static result to_wide(const byte_type *from_begin, const byte_type *from_end, const byte_type *&from_next, wide_type *to_begin,
                              wide_type *to_end, wide_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            unsigned short state = 0;

            while (from_next != from_end && to_next != to_end) {
                unsigned long by = static_cast<unsigned char>(*from_next);
                unsigned long ch;
                int nextra;

                if (state > 1) {
                    // 处理之前保存的代理对第二部分
                    if (by < 0x80u || 0xc0u <= by) {
                        ++from_next;
                        return result::error; // 不是有效的延续字节
                    }

                    ++from_next;
                    *to_next++ = static_cast<wide_type>(state | (by & 0x3f));
                    state = 1;
                    continue;
                }

                if (by < 0x80u) {
                    ch = by;
                    nextra = 0;
                } else if (by < 0xc0u) {
                    // 0x80-0xbf 不是首字节
                    ++from_next;
                    return result::error;
                } else if (by < 0xe0u) {
                    ch = by & 0x1f;
                    nextra = 1;
                } else if (by < 0xf0u) {
                    ch = by & 0x0f;
                    nextra = 2;
                } else if (by < 0xf8u) {
                    ch = by & 0x07;
                    nextra = 3;
                } else {
                    ch = by & 0x03;
                    nextra = by < 0xfc ? 4 : 5;
                }

                const byte_type *rollback = from_next;
                int skip_bytes = (nextra < 3) ? 0 : 1; // 为可能的代理对保留字节

                if (nextra == 0) {
                    ++from_next;
                } else if (from_end - from_next < nextra + 1 - skip_bytes) {
                    break; // 输入不足
                } else {
                    for (++from_next; skip_bytes < nextra; --nextra, ++from_next) {
                        if ((by = static_cast<unsigned char>(*from_next)) < 0x80u || 0xc0u <= by) {
                            return result::error; // 不是延续字节
                        }
                        ch = ch << 6 | (by & 0x3f);
                    }
                }

                if (skip_bytes > 0) {
                    ch <<= 6; // 为下一次调用保留最后字节
                }

                if (Maxcode < ch) {
                    return result::error; // 码点太大
                }

                // 处理超出 BMP 的字符（需要代理对）
                if (ch > 0xffffu) {
                    if (ch > 0x10ffff) {
                        return result::error; // 超出 Unicode 范围
                    }

                    unsigned short high = static_cast<unsigned short>(0xd800 | ((ch >> 10) - 0x0040));
                    *to_next++ = static_cast<wide_type>(high);
                    state = static_cast<unsigned short>(0xdc00 | (ch & 0x03ff));
                    continue;
                }

                if (skip_bytes != 0) {
                    if (from_next == from_end) {
                        from_next = rollback;
                        break; // 字节不足，非规范值
                    }

                    if ((by = static_cast<unsigned char>(*from_next++)) < 0x80u || 0xc0u <= by) {
                        return result::error; // 不是延续字节
                    }

                    ch |= by & 0x3f; // 完成非规范值
                }

                // 检查并消费 BOM
                if (!seen_header) {
                    seen_header = true;

                    constexpr bool consuming = (Mode & consume_header) != static_cast<codecvt_mode>(0);
                    if constexpr (consuming) {
                        if (ch == 0xfeff) {
                            // 跳过 BOM 并重试
                            continue;
                        }
                    }
                }

                // 检查代理区
                if (ch >= 0xd800 && ch < 0xe000) {
                    return result::error; // 不允许直接编码代理区字符
                }

                *to_next++ = static_cast<wide_type>(ch);
            }

            return from_begin == from_next ? result::partial : result::ok;
        }

        /**
         * \lang english
         * @brief Converts wide characters to UTF-8 bytes, encoding characters outside the BMP as surrogate pairs.
         *
         * @param from_begin Pointer to the first wide character to convert
         * @param from_end Pointer one past the last wide character to convert
         * @param from_next On output, points one past the last wide character consumed
         * @param to_begin Pointer to the first byte output element
         * @param to_end Pointer one past the last byte output element
         * @param to_next On output, points one past the last element written
         * @param seen_header Tracks whether a byte-order mark has been seen
         * @return The conversion result
         *
         * \lang simp-chinese
         * @brief 将宽字符转换为 UTF-8 字节，对 BMP 之外的码点编码为代理对。
         *
         * @param from_begin 指向待转换首宽字符的指针
         * @param from_end 指向末宽字符之后位置的指针
         * @param from_next 输出时指向最后一个已消费宽字符之后
         * @param to_begin 指向首字节输出元素的指针
         * @param to_end 指向末字节输出元素之后位置的指针
         * @param to_next 输出时指向最后一个已写入元素之后
         * @param seen_header 跟踪是否已遇到字节序标记
         * @return 转换结果
         */
        static result to_bytes(const wide_type *from_begin, const wide_type *from_end, const wide_type *&from_next,
                               byte_type *to_begin, byte_type *to_end, byte_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            // 状态：0=正常，>1=保存了代理对的高位
            unsigned short state = 0;
            bool has_pending_surrogate = false; // 标记是否有未完成的代理对

            while (from_next != from_end && to_next != to_end) {
                unsigned long ch;
                unsigned short ch1 = static_cast<unsigned short>(*from_next);
                bool need_surrogate = false;

                if (state > 1) {
                    // 处理之前保存的代理对低位
                    if (ch1 < 0xdc00u || ch1 >= 0xe000u) {
                        return result::error; // 无效的第二个代理字
                    }

                    ch = static_cast<unsigned long>((state << 10) | (ch1 - 0xdc00));
                    has_pending_surrogate = false; // 代理对已完成
                } else if (ch1 >= 0xd800 && ch1 < 0xdc00) {
                    if (from_next + 1 == from_end) {
                        has_pending_surrogate = true;
                        break;
                    }

                    // 有下一个字符，继续处理
                    ch = static_cast<unsigned long>((ch1 - 0xd800 + 0x0040) << 10);
                    need_surrogate = true;
                    has_pending_surrogate = true;
                } else if (ch1 >= 0xdc00 && ch1 < 0xe000) {
                    // 孤立的低位代理（没有高位代理）
                    return result::error;
                } else {
                    ch = ch1; // 普通字符
                    has_pending_surrogate = false;
                }

                byte_type by;
                int nextra;

                if (ch < 0x0080u) {
                    by = static_cast<byte_type>(ch);
                    nextra = 0;
                } else if (ch < 0x0800u) {
                    by = static_cast<byte_type>(0xc0 | (ch >> 6));
                    nextra = 1;
                } else if (ch < 0x00010000u) {
                    by = static_cast<byte_type>(0xe0 | (ch >> 12));
                    nextra = 2;
                } else {
                    by = static_cast<byte_type>(0xf0 | (ch >> 18));
                    nextra = 3;
                }

                int bytes_to_put = (nextra < 3) ? (nextra + 1) : (need_surrogate ? 1 : 3);

                if (to_end - to_next < bytes_to_put) {
                    if (need_surrogate) {
                        --from_next;
                        has_pending_surrogate = true;
                    }
                    break;
                }

                // 生成 BOM
                if (!seen_header) {
                    seen_header = true;

                    constexpr bool generating = (Mode & generate_header) != static_cast<codecvt_mode>(0);
                    if constexpr (generating) {
                        if (to_end - to_next < 3 + bytes_to_put) {
                            return result::partial; // 空间不足（含BOM）
                        }

                        // 添加 UTF-8 BOM
                        *to_next++ = '\xef';
                        *to_next++ = '\xbb';
                        *to_next++ = '\xbf';
                    }
                }

                ++from_next;

                if (need_surrogate || nextra < 3) {
                    *to_next++ = by;
                    --bytes_to_put;
                }

                for (; bytes_to_put > 0; --bytes_to_put) {
                    *to_next++ = static_cast<byte_type>((ch >> (6 * --nextra) & 0x3f) | 0x80);
                }

                state = static_cast<unsigned short>(need_surrogate ? (ch >> 10) : 1);

                if (!need_surrogate) {
                    has_pending_surrogate = false;
                }
            }
            return (from_next == from_end && !has_pending_surrogate) ? result::ok : result::partial;
        }
    };
}

namespace rainy::core::text {
    /**
     * \lang english
     * @brief A stateful conversion wrapper between byte strings and wide strings using a codecvt facet.
     *
     * @tparam Codecvt The codecvt facet type performing the actual conversion
     * @tparam StringTemplate The string class template for both string kinds
     * @tparam Elem The wide-character element type
     * @tparam CharTraits The character traits template
     * @tparam WideAlloc The allocator for wide strings
     * @tparam ByteAlloc The allocator for byte strings
     *
     * \lang simp-chinese
     * @brief 使用 codecvt facet 在字节字符串与宽字符串之间进行有状态转换的包装类型。
     *
     * @tparam Codecvt 执行实际转换的 codecvt facet 类型
     * @tparam StringTemplate 两类字符串共同使用的字符串类模板
     * @tparam Elem 宽字符元素类型
     * @tparam CharTraits 字符 traits 模板
     * @tparam WideAlloc 宽字符串的分配器
     * @tparam ByteAlloc 字节字符串的分配器
     */
    // clang-format off
    template <
        typename Codecvt,
        template <typename CharType, typename Traits, typename Alloc> typename StringTemplate,
        typename Elem = wchar_t,
        template <typename CharType> typename CharTraits = std::char_traits,
        typename WideAlloc = std::allocator<Elem>,
        typename ByteAlloc = std::allocator<char>
    >
    class wstring_convert {
    public:
        // clang-format on
        /**
         * \lang english
         * @brief The byte string type.
         *
         * \lang simp-chinese
         * @brief 字节字符串类型。
         */
        using byte_string = StringTemplate<char, CharTraits<char>, ByteAlloc>;
        /**
         * \lang english
         * @brief The wide string type.
         *
         * \lang simp-chinese
         * @brief 宽字符串类型。
         */
        using wide_string = StringTemplate<Elem, CharTraits<Elem>, WideAlloc>;

        /**
         * \lang english
         * @brief Constructs a converter without error strings; conversions use empty strings on error.
         *
         * \lang simp-chinese
         * @brief 构造不含错误字符串的转换器；出错时转换结果为空字符串。
         */
        explicit wstring_convert() : cvtcount(0), seen_header(false) {
        }

        /**
         * \lang english
         * @brief Constructs a converter with the strings returned when a conversion fails.
         *
         * @param byte_err The byte string returned on failure
         * @param wide_err The wide string returned on failure
         *
         * \lang simp-chinese
         * @brief 构造转换器，并指定转换失败时返回的字符串。
         *
         * @param byte_err 失败时返回的字节字符串
         * @param wide_err 失败时返回的宽字符串
         */
        explicit wstring_convert(const byte_string &byte_err, const wide_string &wide_err = wide_string()) :
            byte_err_string(byte_err), wide_err_string(wide_err), cvtcount(0), seen_header(false) {
        }

        /**
         * \lang english
         * @brief Destroys the converter.
         *
         * \lang simp-chinese
         * @brief 销毁转换器。
         */
        ~wstring_convert() {
        }

        /**
         * \lang english
         * @brief Deleted: a converter is not copyable.
         *
         * \lang simp-chinese
         * @brief 已删除：转换器不可拷贝。
         */
        wstring_convert(const wstring_convert &) = delete;
        /**
         * \lang english
         * @brief Deleted: a converter is not copy-assignable.
         *
         * \lang simp-chinese
         * @brief 已删除：转换器不可拷贝赋值。
         */
        wstring_convert &operator=(const wstring_convert &) = delete;

        /**
         * \lang english
         * @brief Converts a single byte to a wide string.
         *
         * @param byte The byte to convert
         * @return The converted wide string
         *
         * \lang simp-chinese
         * @brief 将单个字节转换为宽字符串。
         *
         * @param byte 待转换的字节
         * @return 转换后的宽字符串
         */
        wide_string from_bytes(char byte) {
            char buf[2] = {byte, '\0'};
            return from_bytes(buf);
        }

        /**
         * \lang english
         * @brief Converts a null-terminated byte string to a wide string.
         *
         * @param ptr The null-terminated byte string to convert
         * @return The converted wide string
         *
         * \lang simp-chinese
         * @brief 将以空字符结尾的字节字符串转换为宽字符串。
         *
         * @param ptr 待转换的以空字符结尾的字节字符串
         * @return 转换后的宽字符串
         */
        wide_string from_bytes(const char *ptr) {
            if (!ptr) {
                return wide_err_string;
            }
            return from_bytes(ptr, ptr + std::strlen(ptr));
        }

        /**
         * \lang english
         * @brief Converts a byte string to a wide string.
         *
         * @param str The byte string to convert
         * @return The converted wide string
         *
         * \lang simp-chinese
         * @brief 将字节字符串转换为宽字符串。
         *
         * @param str 待转换的字节字符串
         * @return 转换后的宽字符串
         */
        wide_string from_bytes(const byte_string &str) {
            return from_bytes(str.data(), str.data() + str.size());
        }

        /**
         * \lang english
         * @brief Converts the byte range [first, last) to a wide string.
         *
         * @param first Pointer to the first byte to convert
         * @param last Pointer one past the last byte to convert
         * @return The converted wide string
         *
         * \lang simp-chinese
         * @brief 将字节区间 [first, last) 转换为宽字符串。
         *
         * @param first 指向待转换首字节的指针
         * @param last 指向末字节之后位置的指针
         * @return 转换后的宽字符串
         */
        wide_string from_bytes(const char *first, const char *last) {
            if (!first || !last || first > last) {
                return wide_err_string;
            }
            seen_header = false;
            wide_string result;
            result.resize((last - first) + 1); // +1 = 可能的BOM

            const char *from_next = first;
            Elem *to_next = result.data();
            bool local_seen_header = false;
            typename Codecvt::result res =
                Codecvt::to_wide(first, last, from_next, result.data(), result.data() + result.size(), to_next, local_seen_header);
            if (res == Codecvt::result::error) {
                return wide_err_string;
            }
            cvtcount = to_next - result.data();
            result.resize(cvtcount);
            return result;
        }

        /**
         * \lang english
         * @brief Converts a single wide character to a byte string.
         *
         * @param wchar The wide character to convert
         * @return The converted byte string
         *
         * \lang simp-chinese
         * @brief 将单个宽字符转换为字节字符串。
         *
         * @param wchar 待转换的宽字符
         * @return 转换后的字节字符串
         */
        byte_string to_bytes(Elem wchar) {
            Elem buf[2] = {wchar, L'\0'};
            return to_bytes(buf);
        }

        /**
         * \lang english
         * @brief Converts a null-terminated wide string to a byte string.
         *
         * @param wptr The null-terminated wide string to convert
         * @return The converted byte string
         *
         * \lang simp-chinese
         * @brief 将以空字符结尾的宽字符串转换为字节字符串。
         *
         * @param wptr 待转换的以空字符结尾的宽字符串
         * @return 转换后的字节字符串
         */
        byte_string to_bytes(const Elem *wptr) {
            if (!wptr) {
                return byte_err_string;
            }
            return to_bytes(wptr, wptr + std::char_traits<Elem>::length(wptr));
        }

        /**
         * \lang english
         * @brief Converts a wide string to a byte string.
         *
         * @param wstr The wide string to convert
         * @return The converted byte string
         *
         * \lang simp-chinese
         * @brief 将宽字符串转换为字节字符串。
         *
         * @param wstr 待转换的宽字符串
         * @return 转换后的字节字符串
         */
        byte_string to_bytes(const wide_string &wstr) {
            return to_bytes(wstr.data(), wstr.data() + wstr.size());
        }

        /**
         * \lang english
         * @brief Converts the wide-character range [first, last) to a byte string.
         *
         * @param first Pointer to the first wide character to convert
         * @param last Pointer one past the last wide character to convert
         * @return The converted byte string
         *
         * \lang simp-chinese
         * @brief 将宽字符区间 [first, last) 转换为字节字符串。
         *
         * @param first 指向待转换首宽字符的指针
         * @param last 指向末宽字符之后位置的指针
         * @return 转换后的字节字符串
         */
        byte_string to_bytes(const Elem *first, const Elem *last) {
            if (!first || !last || first > last) {
                return byte_err_string;
            }
            seen_header = false;
            size_t max_bytes = (last - first) * 6 + 3;
            byte_string result;
            result.resize(max_bytes);
            const Elem *from_next = first;
            char *to_next = result.data();
            bool local_seen_header = false;
            typename Codecvt::result res =
                Codecvt::to_bytes(first, last, from_next, result.data(), result.data() + result.size(), to_next, local_seen_header);
            if (res == Codecvt::result::error) {
                return byte_err_string;
            }
            cvtcount = to_next - result.data();
            result.resize(cvtcount);
            return result;
        }

        /**
         * \lang english
         * @brief Returns the number of characters converted by the most recent call.
         *
         * @return The number of converted characters
         *
         * \lang simp-chinese
         * @brief 返回最近一次调用转换的字符数量。
         *
         * @return 已转换的字符数量
         */
        std::size_t converted() const noexcept {
            return cvtcount;
        }

    private:
        byte_string byte_err_string;
        wide_string wide_err_string;
        size_t cvtcount;
        bool seen_header;
    };
}

namespace rainy::text {
    using core::text::codecvt_mode;
    using core::text::codecvt_utf8;
    using core::text::codecvt_utf16;
    using core::text::codecvt_utf8_utf16;
    using core::text::wstring_convert;
}

#endif
