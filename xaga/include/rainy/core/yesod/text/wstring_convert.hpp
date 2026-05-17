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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_WSTRING_CONVERT_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_WSTRING_CONVERT_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::foundation::text {
    enum codecvt_mode {
        consume_header = 4,
        generate_header = 2,
        little_endian = 1
    };

    template <typename Elem, unsigned long Maxcode = 0x10ffff, codecvt_mode Mode = codecvt_mode{}>
    class codecvt_utf8 {
    public:
        using byte_type = char;
        using wide_type = Elem;

        enum class result {
            ok,
            partial,
            error,
            noconv
        };

        // 将 UTF-8 转换为宽字符
        static result to_wide(const byte_type *from_begin, const byte_type *from_end, const byte_type *&from_next, wide_type *to_begin,
                              wide_type *to_end, wide_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            while (from_next != from_end && to_next != to_end) {
                unsigned long by = static_cast<unsigned char>(*from_next);
                unsigned long ch;
                int nextra;

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

                    constexpr bool consuming = (Mode & consume_header) != 0;
                    if constexpr (consuming) {
                        if (ch == 0xfeff) {
                            // 跳过 BOM 并重试
                            continue;
                        }
                    }
                }

                if (Maxcode < ch) {
                    return result::error; // 码点太大
                }

                *to_next++ = static_cast<wide_type>(ch);
            }

            return from_begin == from_next ? result::partial : result::ok;
        }

        // 将宽字符转换为 UTF-8
        static result to_bytes(const wide_type *from_begin, const wide_type *from_end, const wide_type *&from_next,
                               byte_type *to_begin, byte_type *to_end, byte_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            while (from_next != from_end && to_next != to_end) {
                byte_type by;
                int nextra;
                unsigned long ch = static_cast<unsigned long>(*from_next);

                if (Maxcode < ch) {
                    return result::error;
                }

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

                    constexpr bool generating = (Mode & generate_header) != 0;
                    if constexpr (generating) {
                        if (to_end - to_next < 3 + 1 + nextra) {
                            return result::partial; // 空间不足
                        }

                        // 添加 BOM
                        *to_next++ = '\xef';
                        *to_next++ = '\xbb';
                        *to_next++ = '\xbf';
                    }
                }

                if (to_end - to_next < 1 + nextra) {
                    break; // 输出空间不足
                }

                ++from_next;
                for (*to_next++ = by; 0 < nextra;) {
                    *to_next++ = static_cast<byte_type>((ch >> 6 * --nextra & 0x3f) | 0x80);
                }
            }

            return from_begin == from_next ? result::partial : result::ok;
        }
    };
}

namespace rainy::foundation::text {
    template <typename Elem, unsigned long Maxcode = 0x10ffff, codecvt_mode Mode = codecvt_mode{}>
    class codecvt_utf16 {
    public:
        using byte_type = char;
        using wide_type = Elem;

        enum class result {
            ok,
            partial,
            error,
            noconv
        };

        // 将 UTF-16 转换为宽字符
        static result to_wide(const byte_type *from_begin, const byte_type *from_end, const byte_type *&from_next,
                              wide_type *to_begin, wide_type *to_end, wide_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            // 状态：0=未确定字节序，1=小端序，2=大端序
            char endian_state = 0;

            while (from_next != from_end && to_next != to_end) {
                if (from_end - from_next < 2) {
                    break; // 输入不足
                }

                const auto *ptr = reinterpret_cast<const unsigned char*>(from_next);
                unsigned long ch;
                unsigned short ch0, ch1;

                // 确定字节序
                if (endian_state == 0) {
                    constexpr bool prefer_le = (Mode & little_endian) != 0;
                    
                    if (prefer_le) {
                        ch0 = static_cast<unsigned short>(ptr[1] << 8 | ptr[0]);
                    } else {
                        ch0 = static_cast<unsigned short>(ptr[0] << 8 | ptr[1]);
                    }
                    
                    endian_state = prefer_le ? 1 : 2;

                    // 检查并消费 BOM
                    constexpr bool consuming = (Mode & consume_header) != 0;
                    if constexpr (consuming) {
                        if (ch0 == 0xfffe) {
                            // 发现相反的字节序，切换
                            endian_state = prefer_le ? 2 : 1;
                            from_next += 2;
                            continue; // 跳过 BOM 并重试
                        } else if (ch0 == 0xfeff) {
                            // 发现匹配的字节序，跳过 BOM
                            from_next += 2;
                            continue;
                        }
                    }
                }

                // 根据确定的字节序读取第一个字
                if (endian_state == 1) { // 小端序
                    ch0 = static_cast<unsigned short>(ptr[1] << 8 | ptr[0]);
                } else { // 大端序
                    ch0 = static_cast<unsigned short>(ptr[0] << 8 | ptr[1]);
                }

                if (ch0 < 0xd800u || ch0 >= 0xdc00u) {
                    // 单字字符（包括BMP和surrogate高位代理之外的码点）
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

                *to_next++ = static_cast<wide_type>(ch);
            }

            return from_begin == from_next ? result::partial : result::ok;
        }

        // 将宽字符转换为 UTF-16
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
                    constexpr bool prefer_le = (Mode & little_endian) != 0;
                    endian_state = prefer_le ? 1 : 2;

                    constexpr bool generating = (Mode & generate_header) != 0;
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

namespace rainy::foundation::text {
    template <typename Elem, unsigned long Maxcode = 0x10ffff, codecvt_mode Mode = codecvt_mode{}>
    class codecvt_utf8_utf16 {
    public:
        using byte_type = char;
        using wide_type = Elem;

        enum class result {
            ok,
            partial,
            error,
            noconv
        };

        // 将 UTF-8 转换为 UTF-16 宽字符
        static result to_wide(const byte_type *from_begin, const byte_type *from_end, const byte_type *&from_next, wide_type *to_begin,
                              wide_type *to_end, wide_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            // 状态：0=正常，1=等待第二个代理字
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

                    constexpr bool consuming = (Mode & consume_header) != 0;
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

        // 将 UTF-16 宽字符转换为 UTF-8
        static result to_bytes(const wide_type *from_begin, const wide_type *from_end, const wide_type *&from_next,
                               byte_type *to_begin, byte_type *to_end, byte_type *&to_next, bool &seen_header) {
            from_next = from_begin;
            to_next = to_begin;

            // 状态：0=正常，>1=保存了代理对的高位
            unsigned short state = 0;

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
                } else if (ch1 >= 0xd800 && ch1 < 0xdc00) {
                    // 遇到高位代理，需要等待低位代理
                    ch = static_cast<unsigned long>((ch1 - 0xd800 + 0x0040) << 10);
                    need_surrogate = true; // 只输出第一个字节，其余与第二个代理字一起处理
                } else {
                    ch = ch1; // 普通字符
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
                    break; // 空间不足
                }

                // 生成 BOM
                if (!seen_header) {
                    seen_header = true;

                    constexpr bool generating = (Mode & generate_header) != 0;
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
                    // 输出序列的第一个字节（如果需要）
                    *to_next++ = by;
                    --bytes_to_put;
                }

                // 输出剩余的延续字节
                for (; bytes_to_put > 0; --bytes_to_put) {
                    *to_next++ = static_cast<byte_type>((ch >> (6 * --nextra) & 0x3f) | 0x80);
                }

                // 保存状态：如果是代理对，保存高位信息；否则重置为1
                state = static_cast<unsigned short>(need_surrogate ? (ch >> 10) : 1);
            }

            return from_begin == from_next ? result::partial : result::ok;
        }
    };
}

namespace rainy::foundation::text {
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
        using byte_string = StringTemplate<char, CharTraits<char>, ByteAlloc>;
        using wide_string = StringTemplate<Elem, CharTraits<Elem>, WideAlloc>;

        explicit wstring_convert() : cvtcount(0), seen_header(false) {
        }

        explicit wstring_convert(const byte_string &byte_err, const wide_string &wide_err = wide_string()) :
            byte_err_string(byte_err), wide_err_string(wide_err), cvtcount(0), seen_header(false) {
        }

        ~wstring_convert() {
        }

        wstring_convert(const wstring_convert &) = delete;
        wstring_convert &operator=(const wstring_convert &) = delete;

        wide_string from_bytes(char byte) {
            char buf[2] = {byte, '\0'};
            return from_bytes(buf);
        }

        wide_string from_bytes(const char *ptr) {
            if (!ptr) {
                return wide_err_string;
            }
            return from_bytes(ptr, ptr + std::strlen(ptr));
        }

        wide_string from_bytes(const byte_string &str) {
            return from_bytes(str.data(), str.data() + str.size());
        }

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

        byte_string to_bytes(Elem wchar) {
            Elem buf[2] = {wchar, L'\0'};
            return to_bytes(buf);
        }

        byte_string to_bytes(const Elem *wptr) {
            if (!wptr) {
                return byte_err_string;
            }
            return to_bytes(wptr, wptr + std::char_traits<Elem>::length(wptr));
        }

        byte_string to_bytes(const wide_string &wstr) {
            return to_bytes(wstr.data(), wstr.data() + wstr.size());
        }

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
    using foundation::text::codecvt_utf16;
    using foundation::text::codecvt_utf8;
    using foundation::text::codecvt_utf8_utf16;
    using foundation::text::wstring_convert;
}

#endif
