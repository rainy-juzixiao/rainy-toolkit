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

#include <codecvt>
#include <iomanip>
#include <locale>
#include <rainy/core/implements/text/wstring_convert.hpp>

namespace rainy::foundation::text::implements {
    // 对齐方式
    enum class align_type {
        none,
        left, // '<'
        right, // '>'
        center // '^'
    };

    // 符号显示方式
    enum class sign_type {
        none, // 默认
        plus, // '+'
        minus, // '-'
        space // ' '
    };

    // 格式规范
    template <typename CharTypeype>
    struct format_specs {
        align_type align = align_type::none;
        CharTypeype fill = static_cast<CharTypeype>(' ');
        int width = 0;
        int precision = -1;
        bool use_locale = false;
        char type = '\0';

        // 动态宽度和精度
        bool dynamic_width = false;
        bool dynamic_precision = false;
        std::size_t width_arg_id = 0;
        std::size_t precision_arg_id = 0;
    };

    // 解析整数（用于宽度、精度、参数ID）
    template <typename CharType>
    constexpr std::size_t parse_nonnegative_int(typename basic_string_view<CharType>::const_iterator &it,
                                                typename basic_string_view<CharType>::const_iterator end, int max_value = INT_MAX) {

        std::size_t value = 0;
        while (it != end && *it >= '0' && *it <= '9') {
            if (value > static_cast<std::size_t>(max_value) / 10) {
                throw format_error("number is too big");
            }
            value = value * 10 + (*it - '0');
            ++it;
        }
        return value;
    }

    // 检查是否为对齐符号
    constexpr bool is_align_char(char c) noexcept {
        return c == '<' || c == '>' || c == '^';
    }
}

namespace rainy::foundation::text {
    template <typename CharType, typename Traits>
    struct formatter<basic_string_view<CharType, Traits>, CharType> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }
            // 解析 fill 和 align
            auto next_it = it;
            if (next_it != end) {
                ++next_it;
            }
            if (next_it != end && implements::is_align_char(*next_it)) {
                specs_.fill = *it;
                ++it;
                switch (*it) {
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
                ++it;
            } else if (implements::is_align_char(*it)) {
                switch (*it) {
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
                ++it;
            }
            // 解析 width
            if (it != end) {
                if (*it >= '0' && *it <= '9') {
                    specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                } else if (*it == '{') {
                    ++it; // 动态宽度
                    if (it == end) {
                        throw format_error("invalid format string");
                    }
                    std::size_t arg_id;
                    if (*it == '}') {
                        // 自动索引
                        arg_id = ctx.next_arg_id();
                    } else {
                        // 手动索引
                        arg_id = implements::parse_nonnegative_int<CharType>(it, end);
                        ctx.check_arg_id(arg_id);
                    }

                    if (it == end || *it != '}') {
                        throw format_error("invalid format string");
                    }
                    ++it;
                    specs_.dynamic_width = true;
                    specs_.width_arg_id = arg_id;
                    ctx.check_dynamic_spec_integral(arg_id);
                }
            }
            if (it != end && *it == '.') {
                ++it;
                if (it == end) {
                    throw format_error("invalid format string");
                }
                if (*it >= '0' && *it <= '9') {
                    specs_.precision = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                } else if (*it == '{') {
                    ++it;
                    if (it == end) {
                        throw format_error("invalid format string");
                    }
                    std::size_t arg_id;
                    if (*it == '}') {
                        arg_id = ctx.next_arg_id(); // 自动索引
                    } else {
                        arg_id = implements::parse_nonnegative_int<CharType>(it, end); // 手动索引
                        ctx.check_arg_id(arg_id);
                    }
                    if (it == end || *it != '}') {
                        throw format_error("invalid format string");
                    }
                    ++it;
                    specs_.dynamic_precision = true;
                    specs_.precision_arg_id = arg_id;
                    // 检查参数必须是整数类型
                    ctx.check_dynamic_spec_integral(arg_id);
                } else {
                    throw format_error("invalid precision format");
                }
            }
            // 解析 type (s 或 ? 等)
            if (it != end && *it != '}') {
                specs_.type = static_cast<char>(*it);
                ++it;
                if (specs_.type != 's' && specs_.type != '?') {
                    throw format_error("invalid type specifier for string");
                }
            }
            if (it != end && *it != '}') {
                throw format_error("invalid format string");
            }
            return it;
        }

        template <typename FormatContext>
        auto format(const basic_string_view<CharType, Traits> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            // 获取实际的宽度和精度（处理动态值）
            int width = specs_.width;
            int precision = specs_.precision;
            if (specs_.dynamic_width) {
                auto width_arg = ctx.arg(specs_.width_arg_id);
                if (!width_arg.is_integral()) {
                    throw format_error("width argument must be an integer");
                }
                // 使用 visit 提取整数值
                width = width_arg.visit([](auto &&value) -> int {
                    using Ty = type_traits::other_trans::decay_t<decltype(value)>;
                    if constexpr (std::is_integral_v<Ty> && !type_traits::type_relations::is_same_v<Ty, bool>) {
                        return static_cast<int>(value);
                    }
                    throw format_error("width argument must be an integer");
                });
            }
            if (specs_.dynamic_precision) {
                auto prec_arg = ctx.arg(specs_.precision_arg_id);
                if (!prec_arg.is_integral()) {
                    throw format_error("precision argument must be an integer");
                }

                precision = prec_arg.visit([](auto &&value) -> int {
                    using Ty = std::decay_t<decltype(value)>;
                    if constexpr (std::is_integral_v<Ty> && !std::is_same_v<Ty, bool>) {
                        return static_cast<int>(value);
                    }
                    throw format_error("precision argument must be an integer");
                });
            }

            // 应用精度（截断字符串）
            basic_string_view<CharType> view = str;
            if (precision >= 0 && static_cast<std::size_t>(precision) < view.size()) {
                view = view.substr(0, precision);
            }

            // 计算填充
            std::size_t str_size = view.size();
            std::size_t total_width = width > 0 ? static_cast<std::size_t>(width) : str_size;

            if (str_size >= total_width) {
                // 不需要填充
                return std::copy(view.begin(), view.end(), ctx.out());
            }

            std::size_t padding = total_width - str_size;
            auto out = ctx.out();

            // 根据对齐方式输出
            auto align = specs_.align;
            if (align == implements::align_type::none) {
                // 字符串默认左对齐
                align = implements::align_type::left;
            }

            switch (align) {
                case implements::align_type::left:
                    // 左对齐：字符串 + 填充
                    out = std::copy(view.begin(), view.end(), out);
                    out = std::fill_n(out, padding, specs_.fill);
                    break;

                case implements::align_type::right:
                    // 右对齐：填充 + 字符串
                    out = std::fill_n(out, padding, specs_.fill);
                    out = std::copy(view.begin(), view.end(), out);
                    break;

                case implements::align_type::center: {
                    // 居中：左填充 + 字符串 + 右填充
                    std::size_t left_padding = padding / 2;
                    std::size_t right_padding = padding - left_padding;

                    out = std::fill_n(out, left_padding, specs_.fill);
                    out = std::copy(view.begin(), view.end(), out);
                    out = std::fill_n(out, right_padding, specs_.fill);
                    break;
                }

                default:
                    out = std::copy(view.begin(), view.end(), out);
                    break;
            }

            return out;
        }

    private:
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
                throw format_error("invalid format specifier for bool");
            }

            return it;
        }

        template <typename FormatContext>
        auto format(bool value, FormatContext &ctx) const -> typename FormatContext::iterator {

            basic_string_view<CharType> str;

            if (use_string_) {
                if constexpr (std::is_same_v<CharType, char>) {
                    str = value ? basic_string_view<CharType>("true", 4) : basic_string_view<CharType>("false", 5);
                } else if constexpr (std::is_same_v<CharType, wchar_t>) {
                    str = value ? basic_string_view<CharType>(L"true", 4) : basic_string_view<CharType>(L"false", 5);
                }
            } else {
                if constexpr (std::is_same_v<CharType, char>) {
                    str = value ? basic_string_view<CharType>("1", 1) : basic_string_view<CharType>("0", 1);
                } else if constexpr (std::is_same_v<CharType, wchar_t>) {
                    str = value ? basic_string_view<CharType>(L"1", 1) : basic_string_view<CharType>(L"0", 1);
                }
            }

            return std::copy(str.begin(), str.end(), ctx.out());
        }

    private:
        implements::format_specs<CharType> specs_;
        bool use_string_ = false; // true: "true"/"false", false: "1"/"0"
    };

    template <typename CharType>
    struct formatter<std::nullptr_t, CharType> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(std::nullptr_t, FormatContext &ctx) const -> typename FormatContext::iterator {
            const CharType *str = "nullptr";
            return core::algorithm::copy(str, str + 7, ctx.out());
        }
    };

    template <typename Ty, typename CharType>
    struct formatter<Ty *, CharType> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                throw format_error("invalid format specifier for pointer");
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
                throw format_error("invalid format specifier for char");
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

    template <typename Ty, typename CharType>
    struct formatter<
        Ty, CharType,
        std::enable_if_t<std::is_integral_v<Ty> && !std::is_same_v<Ty, bool> && !std::is_same_v<Ty, char> &&
                         !std::is_same_v<Ty, wchar_t> && !std::is_same_v<Ty, char16_t> && !std::is_same_v<Ty, char32_t>>> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }
            // 解析对齐和填充
            auto next_it = it;
            if (next_it != end) {
                ++next_it;
            }
            if (next_it != end && implements::is_align_char(*next_it)) {
                specs_.fill = *it;
                ++it;

                switch (*it) {
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
                ++it;
            } else if (implements::is_align_char(*it)) {
                switch (*it) {
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
                ++it;
            }
            // 解析 'L' 标志
            if (it != end && *it == CharType('L')) {
                use_locale_ = true;
                ++it;
            }

            // 解析宽度
            if (it != end && *it >= '0' && *it <= '9') {
                specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
            }

            // 解析类型
            if (it != end && *it != '}') {
                presentation_type_ = static_cast<char>(*it);
                ++it;

                if (presentation_type_ != 'd' && presentation_type_ != 'x' && presentation_type_ != 'X' && presentation_type_ != 'o' &&
                    presentation_type_ != 'b') {
                    throw format_error("invalid type specifier for integer");
                }
            }

            if (it != end && *it != '}') {
                throw format_error("invalid format string");
            }

            return it;
        }

        template <typename FormatContext>
        auto format(Ty value, FormatContext &ctx) const -> typename FormatContext::iterator {
            basic_string<CharType> str;

            // 根据表示类型格式化
            switch (presentation_type_) {
                case 'x':
                    int_to_string(value, str, 16, false);
                    break;
                case 'X':
                    int_to_string(value, str, 16, true);
                    break;
                case 'o':
                    int_to_string(value, str, 8);
                    break;
                case 'b':
                    int_to_string(value, str, 2);
                    break;
                case 'd':
                default:
                    int_to_string(value, str, 10);
                    break;
            }

            // 如果使用 locale 并且是十进制，添加千位分隔符
            if (use_locale_ && (presentation_type_ == '\0' || presentation_type_ == 'd')) {
                auto loc = ctx.locale();
                basic_string<CharType> thousands_sep;
                std::string grouping;

                if constexpr (std::is_same_v<CharType, char>) {
                    // 从 locale 获取千位分隔符
                    auto &numpunct_facet = std::use_facet<std::numpunct<wchar_t>>(loc);
                    wchar_t wsep = numpunct_facet.thousands_sep();
                    grouping = numpunct_facet.grouping();

                    // 使用 wstring_convert 转换 wchar_t 到 UTF-8
                    wstring_convert<codecvt_utf8<wchar_t>, basic_string> converter;
                    try {
                        auto utf8_sep = converter.to_bytes(wsep);
                        thousands_sep.assign(utf8_sep.begin(), utf8_sep.end());
                    } catch (...) {
                        thousands_sep = ","; // 回退到逗号
                    }
                } else {
                    auto &numpunct_facet = std::use_facet<std::numpunct<CharType>>(loc);
                    CharType sep = numpunct_facet.thousands_sep();
                    thousands_sep = basic_string<CharType>(1, sep);
                    grouping = numpunct_facet.grouping();
                }

                if (!grouping.empty() && grouping[0] > 0 && !thousands_sep.empty()) {
                    int group_size = static_cast<unsigned char>(grouping[0]);
                    add_thousands_separator(str, thousands_sep, group_size);
                }
            }

            // 应用宽度和对齐
            std::size_t str_size = str.size();
            std::size_t total_width = specs_.width > 0 ? static_cast<std::size_t>(specs_.width) : str_size;

            if (str_size >= total_width) {
                return std::copy(str.begin(), str.end(), ctx.out());
            }

            std::size_t padding = total_width - str_size;
            auto out = ctx.out();

            auto align = specs_.align;
            if (align == implements::align_type::none) {
                align = implements::align_type::right;
            }

            switch (align) {
                case implements::align_type::left:
                    out = std::copy(str.begin(), str.end(), out);
                    out = std::fill_n(out, padding, specs_.fill);
                    break;

                case implements::align_type::right:
                    out = std::fill_n(out, padding, specs_.fill);
                    out = std::copy(str.begin(), str.end(), out);
                    break;

                case implements::align_type::center: {
                    std::size_t left_padding = padding / 2;
                    std::size_t right_padding = padding - left_padding;

                    out = std::fill_n(out, left_padding, specs_.fill);
                    out = std::copy(str.begin(), str.end(), out);
                    out = std::fill_n(out, right_padding, specs_.fill);
                    break;
                }

                default:
                    out = std::copy(str.begin(), str.end(), out);
                    break;
            }

            return out;
        }

    private:
        static void add_thousands_separator(basic_string<CharType> &str, const basic_string<CharType> &separator, int group_size) {
            if (group_size <= 0 || separator.empty()) {
                return;
            }
            std::size_t start_pos = 0; // 找到数字开始的位置（跳过负号）
            if (!str.empty() && str[0] == CharType('-')) {
                start_pos = 1;
            }
            std::size_t digit_count = str.size() - start_pos;
            if (digit_count <= static_cast<std::size_t>(group_size)) {
                return; // 不需要分隔符
            }
            // 从右向左插入分隔符
            basic_string<CharType> result;
            result.reserve(str.size() + ((digit_count - 1) / group_size) * separator.size());
            if (start_pos > 0) {
                result += str[0];
            }
            std::size_t count = 0;
            for (std::size_t i = start_pos; i < str.size(); ++i) {
                if (count > 0 && (digit_count - count) % group_size == 0) {
                    result += separator;
                }
                result += str[i];
                ++count;
            }
            str = utility::move(result);
        }

        template <typename IntType>
        static void int_to_string(IntType value, basic_string<CharType> &result, int base, bool uppercase = false) {
            if (value == 0) {
                result = basic_string<CharType>(1, CharType('0'));
                return;
            }
            bool negative = false;
            typename std::make_unsigned<IntType>::type abs_value;
            if constexpr (std::is_signed_v<IntType>) {
                if (value < 0) {
                    negative = true;
                    abs_value = static_cast<typename std::make_unsigned<IntType>::type>(-static_cast<std::int64_t>(value));
                } else {
                    abs_value = static_cast<typename std::make_unsigned<IntType>::type>(value);
                }
            } else {
                abs_value = value;
            }
            CharType buffer[65]{}; // 足够存储64位二进制数
            int pos = 0;
            const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
            while (abs_value > 0) {
                buffer[pos++] = CharType(digits[abs_value % base]);
                abs_value /= base;
            }
            result.clear();
            if (negative) {
                result += CharType('-');
            }
            // 反转buffer到result
            for (int i = pos - 1; i >= 0; --i) {
                result += buffer[i];
            }
        }

        implements::format_specs<CharType> specs_;
        char presentation_type_ = '\0';
        bool use_locale_ = false;
    };
}

namespace rainy::foundation::text {
    template <typename Ty, typename CharType>
    struct formatter<Ty, CharType, std::enable_if_t<std::is_floating_point_v<Ty>>> {
    private:
        implements::format_specs<CharType> specs_;
        char presentation_type_ = '\0'; // 'f', 'e', 'E', 'g', 'G', 'a', 'A'

    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {

            auto it = ctx.begin();
            auto end = ctx.end();

            if (it == end || *it == '}') {
                return it;
            }

            // 解析对齐和填充
            auto next_it = it;
            if (next_it != end) {
                ++next_it;
            }

            if (next_it != end && implements::is_align_char(*next_it)) {
                specs_.fill = *it;
                ++it;

                switch (*it) {
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
                ++it;
            } else if (implements::is_align_char(*it)) {
                switch (*it) {
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
                ++it;
            }
            // 解析宽度
            if (it != end && *it >= '0' && *it <= '9') {
                specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
            }

            // 解析精度
            if (it != end && *it == '.') {
                ++it;
                if (it != end && *it >= '0' && *it <= '9') {
                    specs_.precision = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                } else {
                    specs_.precision = 6; // 默认精度
                }
            }
            // 解析类型
            if (it != end && *it != '}') {
                presentation_type_ = static_cast<char>(*it);
                ++it;

                if (presentation_type_ != 'f' && presentation_type_ != 'F' && presentation_type_ != 'e' && presentation_type_ != 'E' &&
                    presentation_type_ != 'g' && presentation_type_ != 'G' && presentation_type_ != 'a' && presentation_type_ != 'A') {
                    throw format_error("invalid type specifier for floating-point");
                }
            }

            return it;
        }

        template <typename FormatContext>
        auto format(Ty value, FormatContext &ctx) const -> typename FormatContext::iterator {
            basic_string<CharType> str;

            // 处理特殊值
            if (!std::isfinite(value)) {
                if (std::isnan(value)) {
                    str = to_basic_string<CharType>("nan");
                } else {
                    str = value < 0 ? to_basic_string<CharType>("-inf") : to_basic_string<CharType>("inf");
                }
            } else {
                // 根据表示类型格式化
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
            }

            // 应用宽度和对齐
            return apply_alignment(str, ctx);
        }

    private:
        // 固定小数点格式
        static void format_fixed(Ty value, basic_string<CharType> &str, int precision) {
            char buffer[128];
            int len = 0;

            if (value < 0) {
                buffer[len++] = '-';
                value = -value;
            }

            // 整数部分
            uint64_t integer_part = static_cast<uint64_t>(value);
            char int_buf[32];
            int int_len = 0;

            if (integer_part == 0) {
                int_buf[int_len++] = '0';
            } else {
                while (integer_part > 0) {
                    int_buf[int_len++] = '0' + (integer_part % 10);
                    integer_part /= 10;
                }
            }

            for (int i = int_len - 1; i >= 0; --i) {
                buffer[len++] = int_buf[i];
            }

            // 小数部分
            if (precision > 0) {
                buffer[len++] = '.';
                Ty frac_part = value - std::floor(value);

                for (int i = 0; i < precision; ++i) {
                    frac_part *= 10;
                    int digit = static_cast<int>(frac_part);
                    buffer[len++] = '0' + digit;
                    frac_part -= digit;
                }
            }

            str = to_basic_string<CharType>(buffer, len);
        }

        // 科学计数法格式
        static void format_scientific(Ty value, basic_string<CharType> &str, int precision, bool uppercase) {
            char buffer[128];
            int len = 0;

            if (value < 0) {
                buffer[len++] = '-';
                value = -value;
            }

            // 计算指数
            int exp10 = 0;
            if (value != 0) {
                exp10 = static_cast<int>(std::floor(std::log10(value)));
                value /= std::pow(static_cast<Ty>(10), exp10);
            }

            // 格式化尾数
            int int_part = static_cast<int>(value);
            buffer[len++] = '0' + int_part;

            if (precision > 0) {
                buffer[len++] = '.';
                Ty frac_part = value - int_part;

                for (int i = 0; i < precision; ++i) {
                    frac_part *= 10;
                    int digit = static_cast<int>(frac_part);
                    buffer[len++] = '0' + digit;
                    frac_part -= digit;
                }
            }

            // 格式化指数
            buffer[len++] = uppercase ? 'E' : 'e';
            buffer[len++] = exp10 >= 0 ? '+' : '-';
            exp10 = exp10 >= 0 ? exp10 : -exp10;

            if (exp10 >= 100) {
                buffer[len++] = '0' + (exp10 / 100);
                exp10 %= 100;
            }
            buffer[len++] = '0' + (exp10 / 10);
            buffer[len++] = '0' + (exp10 % 10);

            str = to_basic_string<CharType>(buffer, len);
        }

        // 通用格式（自动选择fixed或scientific）
        static void format_general(Ty value, basic_string<CharType> &str, int precision, bool uppercase) {
            Ty abs_value = value < 0 ? -value : value;

            // 根据数值大小选择格式
            if (abs_value != 0 && (abs_value >= std::pow(static_cast<Ty>(10), precision) || abs_value < 1e-4)) {
                format_scientific(value, str, precision - 1, uppercase);
            } else {
                // 计算有效数字
                int decimal_places = precision;
                if (abs_value >= 1) {
                    decimal_places = precision - static_cast<int>(std::log10(abs_value)) - 1;
                    if (decimal_places < 0)
                        decimal_places = 0;
                }
                format_fixed(value, str, decimal_places);
            }
        }

        // 十六进制浮点格式
        static void format_hex(Ty value, basic_string<CharType> &str, int precision, bool uppercase) {
            // 简化实现：使用系统函数
            char buffer[128];
            const char *fmt = uppercase ? "%.*A" : "%.*a";
            int len = snprintf(buffer, sizeof(buffer), fmt, precision, static_cast<double>(value));
            str = to_basic_string<CharType>(buffer, len);
        }

        // 辅助函数：转换C字符串到basic_string
        template <typename U = CharType>
        static basic_string<U> to_basic_string(const char *cstr, int len = -1) {
            if (len < 0)
                len = strlen(cstr);
            basic_string<U> result;
            result.reserve(len);
            for (int i = 0; i < len; ++i) {
                result += static_cast<U>(cstr[i]);
            }
            return result;
        }

        // 应用对齐和宽度
        template <typename FormatContext>
        auto apply_alignment(const basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            std::size_t str_size = str.size();
            std::size_t total_width = specs_.width > 0 ? static_cast<std::size_t>(specs_.width) : str_size;

            if (str_size >= total_width) {
                return std::copy(str.begin(), str.end(), ctx.out());
            }

            std::size_t padding = total_width - str_size;
            auto out = ctx.out();

            auto align = specs_.align;
            if (align == implements::align_type::none) {
                align = implements::align_type::right;
            }

            switch (align) {
                case implements::align_type::left:
                    out = std::copy(str.begin(), str.end(), out);
                    out = std::fill_n(out, padding, specs_.fill);
                    break;

                case implements::align_type::right:
                    out = std::fill_n(out, padding, specs_.fill);
                    out = std::copy(str.begin(), str.end(), out);
                    break;

                case implements::align_type::center: {
                    std::size_t left_padding = padding / 2;
                    std::size_t right_padding = padding - left_padding;

                    out = std::fill_n(out, left_padding, specs_.fill);
                    out = std::copy(str.begin(), str.end(), out);
                    out = std::fill_n(out, right_padding, specs_.fill);
                    break;
                }

                default:
                    out = std::copy(str.begin(), str.end(), out);
                    break;
            }

            return out;
        }
    };
}

#endif
