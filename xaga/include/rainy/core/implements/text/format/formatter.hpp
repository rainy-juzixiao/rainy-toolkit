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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_FORMATTER_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_FORMATTER_HPP
#include <cmath>
#include <rainy/core/implements/text/format/implements.hpp>
#include <rainy/core/implements/text/wstring_convert.hpp>

namespace rainy::foundation::text {
    template <typename CharType, typename Traits>
    struct formatter<basic_string_view<CharType, Traits>, CharType> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }
            auto next_it = it + 1;
            if (next_it != end && implements::is_align_char(*next_it)) {
                specs_.fill = *it;
                specs_.align = parse_align(*next_it);
                it = next_it + 1;
            } else if (implements::is_align_char(*it)) {
                specs_.align = parse_align(*it);
                ++it;
            }
            // 解析 width
            if (it != end) {
                if (*it >= '0' && *it <= '9') {
                    specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                } else if (*it == '{') {
                    ++it;
                    if (it == end) {
                        exceptions::runtime::throw_format_error("invalid format string");
                    }
                    std::size_t arg_id = (*it == '}') ? ctx.next_arg_id() : implements::parse_nonnegative_int<CharType>(it, end);
                    if (*it == '}') {
                        ctx.check_arg_id(arg_id);
                    }
                    if (it == end || *it != '}') {
                        exceptions::runtime::throw_format_error("invalid format string");
                    }
                    ++it;

                    specs_.dynamic_width = true;
                    specs_.width_arg_id = arg_id;
                    ctx.check_dynamic_spec_integral(arg_id);
                }
            }
            // 解析 precision
            if (it != end && *it == '.') {
                ++it;
                if (it == end) {
                    exceptions::runtime::throw_format_error("invalid format string");
                }

                if (*it >= '0' && *it <= '9') {
                    specs_.precision = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                } else if (*it == '{') {
                    ++it;
                    if (it == end) {
                        exceptions::runtime::throw_format_error("invalid format string");
                    }
                    std::size_t arg_id = (*it == '}') ? ctx.next_arg_id() : implements::parse_nonnegative_int<CharType>(it, end);
                    if (*it == '}') {
                        ctx.check_arg_id(arg_id);
                    }
                    if (it == end || *it != '}') {
                        exceptions::runtime::throw_format_error("invalid format string");
                    }
                    ++it;
                    specs_.dynamic_precision = true;
                    specs_.precision_arg_id = arg_id;
                    ctx.check_dynamic_spec_integral(arg_id);
                } else {
                    exceptions::runtime::throw_format_error("invalid precision format");
                }
            }
            // 解析 type
            if (it != end && *it != '}') {
                char type = static_cast<char>(*it);
                if (type != 's' && type != '?') {
                    exceptions::runtime::throw_format_error("invalid type specifier for string");
                }
                specs_.type = type;
                ++it;
            }
            if (it != end && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format string");
            }
            return it;
        }

        template <typename FormatContext>
        auto format(const basic_string_view<CharType, Traits> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            if (!specs_.dynamic_width && !specs_.dynamic_precision && specs_.width <= 0 && specs_.precision < 0) {
                return core::algorithm::copy(str.begin(), str.end(), ctx.out());
            }
            int width = specs_.width;
            int precision = specs_.precision;
            if (specs_.dynamic_width) {
                width = extract_int_arg(ctx.arg(specs_.width_arg_id), "width");
            }
            if (specs_.dynamic_precision) {
                precision = extract_int_arg(ctx.arg(specs_.precision_arg_id), "precision");
            }
            basic_string_view<CharType> view = str;
            if (precision >= 0 && static_cast<std::size_t>(precision) < view.size()) {
                view = view.substr(0, precision);
            }
            const std::size_t str_size = view.size();
            if (width <= 0 || str_size >= static_cast<std::size_t>(width)) { // NOLINT
                return core::algorithm::copy(view.begin(), view.end(), ctx.out());
            }
            const auto total_width = static_cast<std::size_t>(width);
            std::size_t padding = total_width - str_size;
            auto out = ctx.out();
            switch (auto align = (specs_.align == implements::align_type::none) ? implements::align_type::left : specs_.align) {
                case implements::align_type::left: {
                    out = core::algorithm::copy(view.begin(), view.end(), out);
                    return core::algorithm::fill_n(out, padding, specs_.fill);
                }
                case implements::align_type::right: {
                    out = core::algorithm::fill_n(out, padding, specs_.fill);
                    return core::algorithm::copy(view.begin(), view.end(), out);
                }
                case implements::align_type::center: {
                    std::size_t left_padding = padding >> 1;
                    std::size_t right_padding = padding - left_padding;
                    out = core::algorithm::fill_n(out, left_padding, specs_.fill);
                    out = core::algorithm::copy(view.begin(), view.end(), out);
                    return core::algorithm::fill_n(out, right_padding, specs_.fill);
                }
                default: {
                    return core::algorithm::copy(view.begin(), view.end(), out);
                }
            }
        }

    private:
        static constexpr implements::align_type parse_align(CharType c) {
            switch (c) {
                case '<': {
                    return implements::align_type::left;
                }
                case '>': {
                    return implements::align_type::right;
                }
                case '^': {
                    return implements::align_type::center;
                }
                default: {
                    return implements::align_type::none;
                }
            }
        }

        template <typename Arg>
        static int extract_int_arg(Arg &&arg, const char *name) {
            if (!arg.is_integral()) {
                exceptions::runtime::throw_format_error("argument must be an integer");
            }
            return arg.visit([](auto &&value) -> int { // NOLINT
                using Ty = std::decay_t<decltype(value)>;
                if constexpr (std::is_integral_v<Ty> && !type_traits::type_relations::is_same_v<Ty, bool>) {
                    return static_cast<int>(value);
                }
                exceptions::runtime::throw_format_error("argument must be an integer");
                return 0;
            });
        }

        implements::format_specs<CharType> specs_;
    };

    template <typename CharType, typename Traits, typename Alloc>
    struct formatter<basic_string<CharType, Traits, Alloc>, CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        template <typename FormatContext>
        auto format(const basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str.data(), str.size()), ctx);
        }

    private:
        formatter<basic_string_view<CharType, Traits>, CharType> underlying_;
    };

    template <typename CharType>
    struct formatter<const CharType *, CharType> {
    private:
        formatter<basic_string_view<CharType>, CharType> underlying_;

    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        template <typename FormatContext>
        auto format(const CharType *str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str), ctx);
        }
    };

    template <typename CharType, std::size_t N>
    struct formatter<const CharType[N], CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        template <typename FormatContext>
        auto format(const CharType (&str)[N], FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str, N), ctx);
        }

    private:
        formatter<basic_string_view<CharType>, CharType> underlying_;
    };

    template <typename CharType, typename Traits, typename Alloc>
    struct formatter<std::basic_string<CharType, Traits, Alloc>, CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        template <typename FormatContext>
        auto format(const std::basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str.data(), str.size()), ctx);
        }

    private:
        formatter<basic_string_view<CharType, Traits>, CharType> underlying_;
    };

    template <typename CharType, typename Traits>
    struct formatter<std::basic_string_view<CharType, Traits>, CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        template <typename FormatContext>
        auto format(const basic_string_view<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str.data(), str.size()), ctx);
        }

    private:
        formatter<basic_string_view<CharType, Traits>, CharType> underlying_;
    };
}

namespace rainy::foundation::text {
    template <typename CharType>
    struct formatter<bool, CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }
            // 解析类型
            if (*it == 's') {
                use_string_ = true;
                ++it;
            } else if (*it == 'd' || *it == 'b' || *it == 'o' || *it == 'x' || *it == 'X') {
                use_string_ = false;
                ++it;
            }
            if (it != end && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format specifier for bool");
            }

            return it;
        }

        template <typename FormatContext>
        auto format(bool value, FormatContext &ctx) const -> typename FormatContext::iterator {

            basic_string_view<CharType> str;

            if (use_string_) {
                if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                    str = value ? basic_string_view<CharType>("true", 4) : basic_string_view<CharType>("false", 5);
                } else if constexpr (type_traits::type_relations::is_same_v<CharType, wchar_t>) {
                    str = value ? basic_string_view<CharType>(L"true", 4) : basic_string_view<CharType>(L"false", 5);
                }
            } else {
                if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                    str = value ? basic_string_view<CharType>("1", 1) : basic_string_view<CharType>("0", 1);
                } else if constexpr (type_traits::type_relations::is_same_v<CharType, wchar_t>) {
                    str = value ? basic_string_view<CharType>(L"1", 1) : basic_string_view<CharType>(L"0", 1);
                }
            }

            return core::algorithm::copy(str.begin(), str.end(), ctx.out());
        }

    private:
        implements::format_specs<CharType> specs_;
        bool use_string_ = false; // true: "true"/"false", false: "1"/"0"
    };

    template <typename Ty, typename CharType>
    struct formatter<Ty *, CharType> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format specifier for pointer");
            }
            return it;
        }

        template <typename FormatContext>
        auto format(Ty *value, FormatContext &ctx) const -> typename FormatContext::iterator {
            using UInt = uintptr_t;
            UInt v = reinterpret_cast<UInt>(value);
            CharType buffer[2 + sizeof(UInt) * 2]{};
            CharType *out = buffer;
            *out++ = CharType('0');
            *out++ = CharType('x');
            constexpr CharType digits[] = {CharType('0'), CharType('1'), CharType('2'), CharType('3'), CharType('4'), CharType('5'),
                                           CharType('6'), CharType('7'), CharType('8'), CharType('9'), CharType('a'), CharType('b'),
                                           CharType('c'), CharType('d'), CharType('e'), CharType('f')};
            bool started = false;
            for (int i = (sizeof(UInt) * 2) - 1; i >= 0; --i) {
                unsigned nibble = (v >> (i * 4)) & 0xF;
                if (nibble != 0 || started || i == 0) {
                    started = true;
                    *out++ = digits[nibble];
                }
            }
            auto it = ctx.out();
            for (CharType *p = buffer; p != out; ++p) {
                *it++ = *p;
            }
            return it;
        }
    };
}

namespace rainy::foundation::text {
    template <typename CharType>
    struct formatter<CharType, CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {

            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format specifier for char");
            }
            return it;
        }

        template <typename FormatContext>
        auto format(CharType value, FormatContext &ctx) const -> typename FormatContext::iterator {
            auto out = ctx.out();
            *out = value;
            ++out;
            return out;
        }

    private:
        implements::format_specs<CharType> specs_;
    };
}

namespace rainy::foundation::text {
    template <typename Ty, typename CharType>
    struct formatter<
        Ty, CharType,
        std::enable_if_t<std::is_integral_v<Ty> && !type_traits::type_relations::is_same_v<Ty, bool> &&
                         !type_traits::type_relations::is_same_v<Ty, char> && !type_traits::type_relations::is_same_v<Ty, wchar_t> &&
                         !type_traits::type_relations::is_same_v<Ty, char16_t> &&
                         !type_traits::type_relations::is_same_v<Ty, char32_t>>> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}')
                return it;

            // 解析对齐和填充
            auto next = it;
            ++next;
            if (next != end && implements::is_align_char(*next)) {
                specs_.fill = *it++;
                set_align(*it++);
            } else if (implements::is_align_char(*it)) {
                set_align(*it++);
            }

            // 解析 'L' 标志
            if (it != end && *it == CharType('L')) {
                use_locale_ = true;
                ++it;
            }

            // 解析宽度
            if (it != end && *it >= '0' && *it <= '9')
                specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));

            // 解析类型
            if (it != end && *it != '}') {
                presentation_type_ = static_cast<char>(*it++);
                if (presentation_type_ != 'd' && presentation_type_ != 'x' && presentation_type_ != 'X' && presentation_type_ != 'o' &&
                    presentation_type_ != 'b')
                    exceptions::runtime::throw_format_error("invalid type specifier for integer");
            }

            if (it != end && *it != '}')
                exceptions::runtime::throw_format_error("invalid format string");

            return it;
        }

        template <typename FormatContext>
        auto format(Ty value, FormatContext &ctx) const -> typename FormatContext::iterator {
            implements::stack_buffer<CharType> buf;

            switch (presentation_type_) {
                case 'x':
                    write_int(value, buf, 16, false);
                    break;
                case 'X':
                    write_int(value, buf, 16, true);
                    break;
                case 'o':
                    write_int(value, buf, 8, false);
                    break;
                case 'b':
                    write_int(value, buf, 2, false);
                    break;
                default:
                    write_int(value, buf, 10, false);
                    break;
            }

            // locale 千位分隔符：仅十进制路径
            if (use_locale_ && (presentation_type_ == '\0' || presentation_type_ == 'd')) {
                apply_thousands_sep(buf, ctx.locale());
            }

            return apply_alignment(buf, ctx);
        }

    private:
        // ── parse 辅助 ───────────────────────────────────────────────────
        constexpr void set_align(CharType c) noexcept {
            switch (c) {
                case '<':
                    specs_.align = implements::align_type::left;
                    break;
                case '>':
                    specs_.align = implements::align_type::right;
                    break;
                case '^':
                    specs_.align = implements::align_type::center;
                    break;
            }
        }

        template <typename IntType>
        static void write_int(IntType value, implements::stack_buffer<CharType> &buf, int base, bool uppercase) noexcept {
            using UType = std::make_unsigned_t<IntType>;

            bool negative = false;
            UType uval;

            if constexpr (std::is_signed_v<IntType>) {
                if (value < 0) {
                    negative = true;
                    // 避免 INT_MIN 取反 UB
                    uval = static_cast<UType>(~static_cast<UType>(value) + 1u);
                } else {
                    uval = static_cast<UType>(value);
                }
            } else {
                uval = value;
            }

            if (uval == 0) {
                buf.push('0');
                return;
            }

            // 负号先记下来，最后一起翻转时处理
            const int start = buf.len;

            switch (base) {
                case 10:
                    write_decimal(uval, buf);
                    break;
                case 16:
                    write_base_pow2(uval, buf, 4, uppercase);
                    break;
                case 8:
                    write_base_pow2(uval, buf, 3, false);
                    break;
                case 2:
                    write_base_pow2(uval, buf, 1, false);
                    break;
                default:
                    write_generic(uval, buf, base, uppercase);
                    break;
            }

            // 翻转数字部分
            reverse(buf.data + start, buf.len - start);

            // 负号插入到最前面（翻转后再 prepend）
            if (negative) {
                // 向右挪一位，腾出 buf.data[start] 的位置
                std::memmove(buf.data + start + 1, buf.data + start, static_cast<std::size_t>(buf.len - start));
                buf.data[start] = '-';
                ++buf.len;
            }
        }

        template <typename UType>
        static void write_decimal(UType v, implements::stack_buffer<CharType> &buf) noexcept {
            // 每两位对应字符串 "00".."99"
            static constexpr char DIGITS_LUT[200] = {
                '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', '1', '0', '1',
                '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2', '2',
                '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3',
                '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5',
                '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5',
                '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8',
                '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8',
                '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0', '9', '1',
                '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9',
            };

            // 注意：数字是反向写入的，翻转在 write_int 里统一做
            while (v >= 100) {
                int r = static_cast<int>(v % 100);
                v /= 100;
                buf.push(DIGITS_LUT[r * 2 + 1]); // 个位
                buf.push(DIGITS_LUT[r * 2]); // 十位（反向）
            }
            if (v >= 10) {
                int r = static_cast<int>(v);
                buf.push(DIGITS_LUT[r * 2 + 1]);
                buf.push(DIGITS_LUT[r * 2]);
            } else {
                buf.push('0' + static_cast<char>(v));
            }
        }

        template <typename UType>
        static void write_base_pow2(UType v, implements::stack_buffer<CharType> &buf, int shift, bool uppercase) noexcept {
            const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
            const UType mask = static_cast<UType>((1u << shift) - 1u);
            while (v > 0) {
                buf.push(digits[v & mask]);
                v >>= shift;
            }
        }

        template <typename UType>
        static void write_generic(UType v, implements::stack_buffer<CharType> &buf, int base, bool uppercase) noexcept {
            const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
            while (v > 0) {
                buf.push(digits[v % static_cast<UType>(base)]);
                v /= static_cast<UType>(base);
            }
        }

        static void reverse(CharType *p, int len) noexcept {
            for (int l = 0, r = len - 1; l < r; ++l, --r)
                std::swap(p[l], p[r]);
        }

        template <typename Locale>
        void apply_thousands_sep(implements::stack_buffer<CharType> &buf, const Locale &loc) const {
            CharType sep_data[8]{};
            int sep_len = 0;
            string grouping;

            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                auto &facet = std::use_facet<std::numpunct<wchar_t>>(loc);
                wchar_t wsep = facet.thousands_sep();
                grouping = facet.grouping();

                basic_string<CharType> utf8_sep;
                try {
                    wstring_convert<codecvt_utf8<wchar_t>, basic_string> converter;
                    utf8_sep = converter.to_bytes(wsep);
                } catch (...) {
                    utf8_sep = ",";
                }

                // 把转换结果写进 sep_data（最多 8 字节，截断保护）
                sep_len = static_cast<int>(std::min(utf8_sep.size(), static_cast<std::size_t>(8)));
                for (int i = 0; i < sep_len; ++i)
                    sep_data[i] = utf8_sep[i];
            } else {
                auto &facet = std::use_facet<std::numpunct<CharType>>(loc);
                sep_data[0] = facet.thousands_sep();
                sep_len = 1;
                grouping = facet.grouping();
            }
            if (grouping.empty() || grouping[0] <= 0 || sep_len == 0) {
                return;
            }
            const int group_size = static_cast<unsigned char>(grouping[0]);
            const int sign_len = (buf.len > 0 && buf.data[0] == '-') ? 1 : 0;
            const int digit_len = buf.len - sign_len;
            if (digit_len <= group_size) {
                return;
            }
            const int sep_count = (digit_len - 1) / group_size;
            const int insert_len = sep_count * sep_len;
            const int old_len = buf.len;
            const int new_len = old_len + insert_len;
            buf.len = new_len;
            int read = old_len - 1;
            int write = new_len - 1;
            int count = 0; // 当前组已搬移的数字数

            while (read >= sign_len) {
                if (count == group_size) {
                    // 插入分隔符（倒序写，因为从右往左）
                    for (int s = sep_len - 1; s >= 0; --s)
                        buf.data[write--] = static_cast<char>(sep_data[s]);
                    count = 0;
                }
                buf.data[write--] = buf.data[read--];
                ++count;
            }

            // 搬移符号位
            if (sign_len > 0) {
                buf.data[write] = buf.data[0];
            }
        }

        template <typename FormatContext>
        auto apply_alignment(const implements::stack_buffer<CharType> &buf, FormatContext &ctx) const ->
            typename FormatContext::iterator {
            const std::size_t str_len = static_cast<std::size_t>(buf.len);
            const std::size_t total_width = specs_.width > 0 ? static_cast<std::size_t>(specs_.width) : str_len;
            auto out = ctx.out();

            if (str_len >= total_width)
                return write_to(buf.data, str_len, out);

            const std::size_t padding = total_width - str_len;
            const CharType fill = static_cast<CharType>(specs_.fill);
            const auto align = (specs_.align == implements::align_type::none) ? implements::align_type::right : specs_.align;

            switch (align) {
                case implements::align_type::left:
                    out = write_to(buf.data, str_len, out);
                    return core::algorithm::fill_n(out, padding, fill);

                case implements::align_type::right:
                    out = core::algorithm::fill_n(out, padding, fill);
                    return write_to(buf.data, str_len, out);

                case implements::align_type::center: {
                    const std::size_t lp = padding / 2, rp = padding - lp;
                    out = core::algorithm::fill_n(out, lp, fill);
                    out = write_to(buf.data, str_len, out);
                    return core::algorithm::fill_n(out, rp, fill);
                }
                default:
                    return write_to(buf.data, str_len, out);
            }
        }

        template <typename OutputIt>
        static OutputIt write_to(const CharType *src, std::size_t n, OutputIt out) noexcept {
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                return core::algorithm::copy_n(src, n, out);
            } else {
                for (std::size_t i = 0; i < n; ++i)
                    *out++ = static_cast<CharType>(src[i]);
                return out;
            }
        }

        implements::format_specs<CharType> specs_;
        char presentation_type_ = '\0';
        bool use_locale_ = false;
    };
}

namespace rainy::foundation::text {
    template <typename T, typename CharType>
    struct formatter<T, CharType, std::enable_if_t<std::is_floating_point_v<T>>> {
    private:
        implements::format_specs<CharType> specs_;
        char presentation_type_ = '\0'; // 'f','F','e','E','g','G','a','A'
        bool use_locale_ = false; // ← 新增

    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();

            if (it == end || *it == '}')
                return it;

            // 1. 对齐和填充
            auto next_it = it;
            if (next_it != end)
                ++next_it;
            if (next_it != end && implements::is_align_char(*next_it)) {
                specs_.fill = *it++;
                switch (*it++) {
                    case '<':
                        specs_.align = implements::align_type::left;
                        break;
                    case '>':
                        specs_.align = implements::align_type::right;
                        break;
                    case '^':
                        specs_.align = implements::align_type::center;
                        break;
                }
            } else if (implements::is_align_char(*it)) {
                switch (*it++) {
                    case '<':
                        specs_.align = implements::align_type::left;
                        break;
                    case '>':
                        specs_.align = implements::align_type::right;
                        break;
                    case '^':
                        specs_.align = implements::align_type::center;
                        break;
                    default:
                        break;
                }
            }

            // 2. 宽度
            if (it != end && *it >= '0' && *it <= '9')
                specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));

            // 3. 精度
            if (it != end && *it == '.') {
                ++it;
                if (it != end && *it >= '0' && *it <= '9') {
                    specs_.precision = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                }
                else {
                    specs_.precision = 6;
                }
            }

            // 4. L 标志（在精度之后、类型之前）
            if (it != end && *it == CharType('L')) {
                use_locale_ = true;
                ++it;
            }

            // 5. 类型
            if (it != end && *it != '}') {
                presentation_type_ = static_cast<char>(*it++);
                if (presentation_type_ != 'f' && presentation_type_ != 'F' && presentation_type_ != 'e' && presentation_type_ != 'E' &&
                    presentation_type_ != 'g' && presentation_type_ != 'G' && presentation_type_ != 'a' && presentation_type_ != 'A')
                    foundation::exceptions::runtime::throw_format_error("invalid type specifier for floating-point");
            }

            return it;
        }

        template <typename FormatContext>
        auto format(T value, FormatContext &ctx) const -> typename FormatContext::iterator {
            basic_string<CharType> str;

            if (!std::isfinite(value)) {
                if (std::isnan(value)) {
                    str.assign({CharType('n'), CharType('a'), CharType('n')});
                } else {
                    if (value < 0)
                        str.assign({CharType('-'), CharType('i'), CharType('n'), CharType('f')});
                    else
                        str.assign({CharType('i'), CharType('n'), CharType('f')});
                }
            } else {
                int precision = specs_.precision >= 0 ? specs_.precision : 6;

                switch (presentation_type_) {
                    case 'f':
                    case 'F':
                        format_fixed(value, str, precision);
                        break;
                    case 'e':
                        format_scientific(value, str, precision, false);
                        break;
                    case 'E':
                        format_scientific(value, str, precision, true);
                        break;
                    case 'g':
                    case 'G':
                        format_general(value, str, precision, presentation_type_ == 'G');
                        break;
                    case 'a':
                    case 'A':
                        format_hex(value, str, precision, presentation_type_ == 'A');
                        break;
                    default:
                        format_general(value, str, precision, false);
                        break;
                }

                // ← locale 后处理（'a'/'A' 十六进制格式不做本地化）
                if (use_locale_ && presentation_type_ != 'a' && presentation_type_ != 'A') {
                    apply_locale(str, ctx.locale());
                }
            }

            return apply_alignment(str, ctx);
        }

    private:
        static int wchar_to_chart(wchar_t wc, CharType (&dest)[8]) { // NOLINT
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                basic_string<CharType> utf8;
                try {
                    wstring_convert<codecvt_utf8<wchar_t>, basic_string> conv;
                    wstring ws(1, wc);
                    utf8 = conv.to_bytes(ws.data());
                } catch (...) {
                    // 回退为 ASCII 替代
                    dest[0] = static_cast<char>(wc);
                    return 1;
                }
                int n = static_cast<int>(std::min(utf8.size(), static_cast<std::size_t>(8)));
                for (int i = 0; i < n; ++i)
                    dest[i] = utf8[i];
                return n;
            } else {
                dest[0] = static_cast<CharType>(wc);
                return 1;
            }
        }

        template <typename Locale>
        static void apply_locale(basic_string<CharType> &str, const Locale &loc) {
            CharType dp_chars[8]{};
            int dp_len = 0;
            CharType sep_chars[8]{};
            int sep_len = 0;
            string grouping;
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                auto &facet = std::use_facet<std::numpunct<wchar_t>>(loc);
                dp_len = wchar_to_chart(facet.decimal_point(), dp_chars);
                sep_len = wchar_to_chart(facet.thousands_sep(), sep_chars);
                grouping = facet.grouping();
            } else {
                auto &facet = std::use_facet<std::numpunct<CharType>>(loc);
                dp_chars[0] = facet.decimal_point();
                dp_len = 1;
                sep_chars[0] = facet.thousands_sep();
                sep_len = 1;
                grouping = facet.grouping();
            }
            const CharType ascii_dot = CharType('.');
            const std::size_t dot_pos = str.find(ascii_dot);

            if ((dot_pos != basic_string<CharType>::npos)) {
                // 用 dp_chars[0..dp_len) 替换位置 dot_pos 处的单个 '.'
                basic_string<CharType> dp_str(dp_chars, dp_chars + dp_len);
                str.replace(dot_pos, 1, dp_str);
            }

            if (grouping.empty() || grouping[0] <= 0 || sep_len == 0) {
                return;
            }

            const int group_size = static_cast<unsigned char>(grouping[0]);

            std::size_t sign_end = 0;
            if (!str.empty() && str[0] == CharType('-'))
                sign_end = 1;

            std::size_t int_end = str.size();
            for (std::size_t i = sign_end; i < str.size(); ++i) {
                CharType c = str[i];
                if (c < CharType('0') || c > CharType('9')) {
                    int_end = i;
                    break;
                }
            }
            const int digit_len = static_cast<int>(int_end - sign_end);
            if (digit_len <= group_size) {
                return;
            }
            const int sep_count = (digit_len - 1) / group_size;
            const int first_group = digit_len - sep_count * group_size; // 1..group_size
            CharType buf[512]{};
            int buf_len = 0;
            for (std::size_t i = 0; i < sign_end; ++i) {
                buf[buf_len++] = str[i];
            }
            for (int i = 0; i < first_group; ++i) {
                buf[buf_len++] = str[sign_end + static_cast<std::size_t>(i)];
            }
            // 剩余各组，每组前插分隔符
            for (int g = 0; g < sep_count; ++g) {
                for (int k = 0; k < sep_len; ++k) {
                    buf[buf_len++] = sep_chars[k];
                }
                const int base = static_cast<int>(sign_end) + first_group + g * group_size;
                for (int i = 0; i < group_size; ++i) {
                    buf[buf_len++] = str[static_cast<std::size_t>(base + i)];
                }
            }
            // int_end 之后的内容（小数点、指数等）
            for (std::size_t i = int_end; i < str.size(); ++i) {
                buf[buf_len++] = str[i];
            }
            str.assign(buf, static_cast<std::size_t>(buf_len));
        }

        static void format_fixed(T value, basic_string<CharType> &str, int precision) {
            char buf[64]{};
            int prec = precision < 0 ? 6 : precision;
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::fixed, prec);
            if (ec != std::errc{})
                foundation::exceptions::runtime::throw_format_error("floating-point to_chars failed");
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                str.assign(buf, static_cast<std::size_t>(ptr - buf));
            } else {
                std::size_t n = static_cast<std::size_t>(ptr - buf);
                CharType wbuf[64]{};
                for (std::size_t i = 0; i < n; ++i)
                    wbuf[i] = static_cast<CharType>(buf[i]);
                str.assign(wbuf, n);
            }
        }

        static void format_scientific(T value, basic_string<CharType> &str, int precision, bool uppercase) {
            char buf[64]{};
            int prec = precision < 0 ? 6 : precision;
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::scientific, prec);
            if (ec != std::errc{})
                exceptions::runtime::throw_format_error("floating-point to_chars failed");
            if (uppercase) {
                for (char *p = buf; p != ptr; ++p) {
                    if (*p == 'e') {
                        *p = 'E';
                        break;
                    }
                }
            }
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                str.assign(buf, static_cast<std::size_t>(ptr - buf));
            } else {
                std::size_t n = static_cast<std::size_t>(ptr - buf);
                CharType wbuf[64]{};
                for (std::size_t i = 0; i < n; ++i) {
                    wbuf[i] = static_cast<CharType>(buf[i]);
                }
                str.assign(wbuf, n);
            }
        }

        static void format_general(T value, basic_string<CharType> &str, const int precision, const bool uppercase) {
            char buf[64]{};
            int prec = precision < 0 ? 6 : precision;
            auto fmt = uppercase ? std::chars_format::general // to_chars 无 uppercase general
                                 : std::chars_format::general;
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general, prec);
            if (ec != std::errc{})
                foundation::exceptions::runtime::throw_format_error("floating-point to_chars failed");

            if (uppercase) {
                for (char *p = buf; p != ptr; ++p)
                    if (*p == 'e') {
                        *p = 'E';
                        break;
                    }
            }

            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                str.assign(buf, static_cast<std::size_t>(ptr - buf));
            } else {
                std::size_t n = static_cast<std::size_t>(ptr - buf);
                CharType wbuf[64]{};
                for (std::size_t i = 0; i < n; ++i)
                    wbuf[i] = static_cast<CharType>(buf[i]);
                str.assign(wbuf, n);
            }
        }

        static void format_hex(T value, basic_string<CharType> &str, int precision, bool uppercase) {
            CharType buffer[128];
            int len = 0;
            if (value < 0) {
                buffer[len++] = CharType('-');
                value = -value;
            }
            if (value == 0) {
                buffer[len++] = CharType('0');
                buffer[len++] = uppercase ? CharType('X') : CharType('x');
                buffer[len++] = CharType('0');
                if (precision > 0) {
                    buffer[len++] = CharType('.');
                    for (int i = 0; i < precision; ++i) {
                        buffer[len++] = CharType('0');
                    }
                }
                buffer[len++] = uppercase ? CharType('P') : CharType('p');
                buffer[len++] = CharType('+');
                buffer[len++] = CharType('0');
                str.assign(buffer, buffer + len);
                return;
            }
            uint64_t bits;
            if constexpr (sizeof(T) == sizeof(float)) {
                uint32_t tmp;
                std::memcpy(&tmp, &value, sizeof(float));
                bits = tmp;
            } else {
                std::memcpy(&bits, &value, sizeof(double));
            }
            constexpr int mantissa_bits = sizeof(T) == sizeof(float) ? 23 : 52;
            constexpr int exponent_bits = sizeof(T) == sizeof(float) ? 8 : 11;
            constexpr int exponent_bias = sizeof(T) == sizeof(float) ? 127 : 1023;
            const uint64_t mantissa_mask = (1ULL << mantissa_bits) - 1;
            uint64_t mantissa = bits & mantissa_mask;
            int exponent = static_cast<int>((bits >> mantissa_bits) & ((1 << exponent_bits) - 1));
            if (exponent == 0) {
                exponent = 1 - exponent_bias;
            } else {
                mantissa |= (1ULL << mantissa_bits);
                exponent -= exponent_bias;
            }
            buffer[len++] = CharType('0');
            buffer[len++] = uppercase ? CharType('X') : CharType('x');
            int highest_bit = mantissa_bits;
            if (exponent != 1 - exponent_bias) {
                buffer[len++] = CharType('1');
            } else {
                while (highest_bit >= 0 && !((mantissa >> highest_bit) & 1)) {
                    --highest_bit;
                }
                if (highest_bit >= 0) {
                    buffer[len++] = CharType('1');
                    exponent -= (mantissa_bits - highest_bit);
                } else {
                    buffer[len++] = CharType('0');
                }
            }
            if (precision > 0 || mantissa != 0) {
                buffer[len++] = CharType('.');
                int frac_bits = highest_bit > 0 ? highest_bit : mantissa_bits;
                int digits_to_output = precision >= 0 ? precision : (frac_bits + 3) / 4;

                for (int i = 0; i < digits_to_output; ++i) {
                    int hex_digit = 0;
                    for (int bit = 0; bit < 4; ++bit) {
                        --frac_bits;
                        if (frac_bits >= 0 && (mantissa & (1ULL << frac_bits))) {
                            hex_digit |= (1 << (3 - bit));
                        }
                    }

                    if (hex_digit < 10) {
                        buffer[len++] = CharType('0' + hex_digit);
                    } else {
                        buffer[len++] = (uppercase ? CharType('A') : CharType('a')) + (hex_digit - 10);
                    }
                }
            }
            buffer[len++] = uppercase ? CharType('P') : CharType('p');
            if (exponent >= 0) {
                buffer[len++] = CharType('+');
            } else {
                buffer[len++] = CharType('-');
                exponent = -exponent;
            }
            CharType exp_buf[16];
            int exp_len = 0;
            if (exponent == 0) {
                exp_buf[exp_len++] = CharType('0');
            } else {
                while (exponent > 0) {
                    exp_buf[exp_len++] = CharType('0' + (exponent % 10));
                    exponent /= 10;
                }
            }
            for (int i = exp_len - 1; i >= 0; --i) {
                buffer[len++] = exp_buf[i];
            }
            str.assign(buffer, buffer + len);
        }

        template <typename FormatContext>
        auto apply_alignment(const basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            std::size_t str_size = str.size();
            const std::size_t total_width = specs_.width > 0 ? static_cast<std::size_t>(specs_.width) : str_size;
            if (str_size >= total_width) {
                return core::algorithm::copy(str.begin(), str.end(), ctx.out());
            }
            std::size_t padding = total_width - str_size;
            auto out = ctx.out();
            auto align = specs_.align;
            if (align == implements::align_type::none)
                align = implements::align_type::right;
            switch (align) {
                case implements::align_type::left:
                    out = core::algorithm::copy(str.begin(), str.end(), out);
                    return core::algorithm::fill_n(out, padding, specs_.fill);
                case implements::align_type::right:
                    out = core::algorithm::fill_n(out, padding, specs_.fill);
                    return core::algorithm::copy(str.begin(), str.end(), out);
                case implements::align_type::center: {
                    std::size_t lp = padding / 2, rp = padding - lp;
                    out = core::algorithm::fill_n(out, lp, specs_.fill);
                    out = core::algorithm::copy(str.begin(), str.end(), out);
                    return core::algorithm::fill_n(out, rp, specs_.fill);
                }
                default:
                    return core::algorithm::copy(str.begin(), str.end(), out);
            }
        }
    };
}

#endif
