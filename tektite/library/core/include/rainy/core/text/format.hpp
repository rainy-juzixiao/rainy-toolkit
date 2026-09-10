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
#ifndef RAINY_TEXT_FORMAT_FORMAT_HPP
#define RAINY_TEXT_FORMAT_FORMAT_HPP
#include <optional>
#include <rainy/core/text/format/context.hpp>
#include <rainy/core/text/format/formatter.hpp>
#include <rainy/core/text/format/range_formatter.hpp>

namespace rainy::core::text::implements {
    template <typename CharType>
    constexpr const CharType *find_next_brace(const CharType *begin, const CharType *end, CharType ch) {
        for (; begin != end; ++begin) {
            if (*begin == ch) {
                // 检查是否是转义的 {{ 或 }}
                if (begin + 1 != end && *(begin + 1) == ch) {
                    ++begin; // 跳过转义序列
                    continue;
                }
                return begin;
            }
        }
        return end;
    }

    template <typename OutputIt, typename CharType>
    OutputIt write_char(OutputIt out, CharType c) {
        *out = c;
        ++out;
        return out;
    }

    template <typename OutputIt, typename CharType>
    OutputIt write_string(OutputIt out, const CharType *begin, const CharType *end) {
        return std::copy(begin, end, out);
    }

    // 写入字符串视图
    template <typename OutputIt, typename CharType>
    OutputIt write_string(OutputIt out, basic_string_view<CharType> str) {
        return std::copy(str.begin(), str.end(), out);
    }

    // 格式化单个参数
    template <typename OutputIt, typename CharType>
    OutputIt format_arg(OutputIt out, basic_format_parse_context<CharType> &parse_ctx, basic_format_context<OutputIt, CharType> &format_ctx,
                        size_t arg_id) {

        auto arg = format_ctx.arg(arg_id);
        if (!arg) {
            exceptions::runtime::throw_format_error("argument index out of range");
        }
        return arg.visit([&](auto &&value) -> OutputIt { // NOLINT
            using type = type_traits::other_trans::decay_t<decltype(value)>;

            if constexpr (type_traits::type_relations::is_same_v<type, std::monostate>) {
                exceptions::runtime::throw_format_error("invalid argument");
                return out;
            } else if constexpr (type_traits::type_relations::is_same_v<type, typename basic_format_arg<basic_format_context<OutputIt, CharType>>::handle>) {
                // 自定义类型通过 handle 处理
                value.format(parse_ctx, format_ctx);
                return format_ctx.out();
            } else {
                // 使用对应类型的 formatter
                using Context = basic_format_context<OutputIt, CharType>;
                typename Context::template formatter_type<type> f;
                // 解析格式规范
                parse_ctx.advance_to(f.parse(parse_ctx));
                // 格式化值
                format_ctx.advance_to(f.format(value, format_ctx));
                return format_ctx.out();
            }
        });
    }

    template <typename OutputIt, typename CharType>
    OutputIt do_vformat(OutputIt out, basic_string_view<CharType> fmt, basic_format_parse_context<CharType> &parse_ctx,
                        basic_format_context<OutputIt, CharType> &format_ctx,
                        basic_format_args<basic_format_context<OutputIt, CharType>> args) {
        (void) parse_ctx;
        const CharType *p = fmt.data();
        const CharType *end = p + fmt.size();
        size_t next_auto_arg_id = 0;

        while (p != end) {
            // 查找下一个 '{'
            const CharType *brace_begin = find_next_brace(p, end, CharType('{'));

            // 输出 '{' 之前的文字部分
            while (p != brace_begin) {
                if (*p == CharType('{') || *p == CharType('}')) {
                    // 转义的 {{ 或 }}，只输出一个
                    out = write_char(out, *p);
                    ++p;
                    if (p != end && (*p == CharType('{') || *p == CharType('}'))) {
                        ++p; // 跳过第二个
                    }
                } else {
                    out = write_char(out, *p);
                    ++p;
                }
            }

            if (p == end) {
                break;
            }

            // 现在 p 指向 '{'
            ++p;
            if (p == end) {
                exceptions::runtime::throw_format_error("invalid format string: unmatched '{'");
            }

            // 检查是否是转义的 {{
            if (*p == CharType('{')) {
                out = write_char(out, CharType('{'));
                ++p;
                continue;
            }

            // 解析参数 ID
            size_t arg_id = 0;
            const CharType *spec_begin = p; // 先记录位置

            if (*p >= CharType('0') && *p <= CharType('9')) {
                // 显式索引
                arg_id = 0;
                while (p != end && *p >= CharType('0') && *p <= CharType('9')) {
                    arg_id = arg_id * 10 + (*p - CharType('0'));
                    ++p;
                }
                spec_begin = p; // 更新 spec_begin 到数字之后
            } else if (*p == CharType('}') || *p == CharType(':')) {
                // 自动索引：{} 或 {:...}
                arg_id = next_auto_arg_id++;
                // spec_begin 已经正确指向当前位置
            } else {
                exceptions::runtime::throw_format_error("invalid format string: expected argument index, ':', or '}'");
            }

            if (arg_id >= args.size()) {
                exceptions::runtime::throw_format_error("argument index out of range");
            }

            // 解析格式规范
            if (p != end && *p == CharType(':')) {
                ++p;
                spec_begin = p; // 格式规范从 ':' 之后开始
                int brace_level = 0;

                while (p != end) {
                    if (*p == CharType('{')) {
                        ++brace_level;
                    } else if (*p == CharType('}')) {
                        if (brace_level == 0) {
                            break;
                        }
                        --brace_level;
                    }
                    ++p;
                }
            }
            if (p == end || *p != CharType('}')) {
                exceptions::runtime::throw_format_error("invalid format string: unmatched '{'");
            }
            // 创建格式规范的解析上下文
            basic_string_view<CharType> spec(spec_begin, p - spec_begin);
            basic_format_parse_context<CharType> arg_parse_ctx(spec, args.size());
            // 格式化参数
            format_ctx.advance_to(out);
            out = format_arg(out, arg_parse_ctx, format_ctx, arg_id);
            ++p;
        }

        return out;
    }

    // 核心格式化实现
    template <typename OutputIt, typename CharType>
    OutputIt vformat_to_impl(OutputIt out, basic_string_view<CharType> fmt, basic_format_args<basic_format_context<OutputIt, CharType>> args,
                             std::optional<std::locale> loc = std::nullopt) {

        basic_format_parse_context<CharType> parse_ctx(fmt, args.size());

        if (loc.has_value()) {
            basic_format_context<OutputIt, CharType> format_ctx(out, args, loc.value());
            return do_vformat(out, fmt, parse_ctx, format_ctx, args);
        }
        basic_format_context<OutputIt, CharType> format_ctx(out, args);
        return do_vformat(out, fmt, parse_ctx, format_ctx, args);
    }
}

namespace rainy::core::text {
    template <typename OutputIt, typename CharType>
    OutputIt vformat_to(OutputIt out, basic_string_view<CharType> fmt, // NOLINT
                        basic_format_args<basic_format_context<OutputIt, CharType>> args) {
        try {
            return implements::vformat_to_impl(out, fmt, args, std::nullopt);
        } catch (const std::exception &e) {
            exceptions::runtime::throw_format_error(e.what());
        }
        return out; // never reach
    }

    template <typename OutputIt, typename CharType>
    OutputIt vformat_to(OutputIt out, const std::locale &loc, basic_string_view<CharType> fmt, // NOLINT
                        basic_format_args<basic_format_context<OutputIt, CharType>> args) {

        try {
            return implements::vformat_to_impl(out, fmt, args, loc);
        } catch (const std::exception &e) {
            exceptions::runtime::throw_format_error(e.what());
        }
        return out; // never reach
    }

    RAINY_INLINE string vformat(const string_view fmt, const format_args args) {
        string result;
        text::vformat_to(utility::back_inserter(result), fmt, args);
        return result;
    }

    RAINY_INLINE wstring vformat(const wstring_view fmt, const wformat_args args) {
        wstring result;
        vformat_to(utility::back_inserter(result), fmt, args);
        return result;
    }

    RAINY_INLINE string vformat(const std::locale &loc, const string_view fmt, const format_args args) {
        string result;
        vformat_to(utility::back_inserter(result), loc, fmt, args);
        return result;
    }

    RAINY_INLINE wstring vformat(const std::locale &loc, const wstring_view fmt, const wformat_args args) {
        wstring result;
        vformat_to(utility::back_inserter(result), loc, fmt, args);
        return result;
    }

    template <typename... Args>
    string format(string_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args(args...);
        return vformat(fmt, basic_format_args(arg_store));
    }

    template <typename... Args>
    wstring format(wstring_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args(args...);
        return vformat(fmt, basic_format_args(arg_store));
    }

    template <typename... Args>
    string format(const std::locale &loc, string_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args(args...);
        return vformat(loc, fmt, basic_format_args(arg_store));
    }

    template <typename... Args>
    wstring format(const std::locale &loc, wstring_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args(args...);
        return vformat(loc, fmt, basic_format_args(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, string_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args<format_context>(args...);
        return vformat_to(out, fmt, basic_format_args<format_context>(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, wstring_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args<wformat_context>(args...);
        return vformat_to(out, fmt, basic_format_args<wformat_context>(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, const std::locale &loc, std::string_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args<format_context>(args...);
        return vformat_to(out, loc, fmt, basic_format_args<format_context>(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, const std::locale &loc, std::wstring_view fmt, const Args &...args) {
        auto arg_store = text::make_format_args<wformat_context>(args...);
        return vformat_to(out, loc, fmt, basic_format_args<wformat_context>(arg_store));
    }
}

namespace rainy::core::text {
    template <typename Char, typename Traits, typename Alloc>
    template <typename... Args>
    basic_string<Char, Traits, Alloc> &basic_string<Char, Traits, Alloc>::format(basic_string_view<Char> fmt,
                                                                                 Args const &...args) {
        if constexpr (type_traits::type_relations::is_same_v<char, Char>) {
            auto fmt_args = text::make_format_args<format_context>(args...);
            text::vformat_to(utility::back_inserter(*this), fmt, basic_format_args<format_context>(fmt_args));
        } else if constexpr (type_traits::type_relations::is_same_v<wchar_t, Char>) {
            auto fmt_args = text::make_format_args<wformat_context>(args...);
            text::vformat_to(utility::back_inserter(*this), fmt, basic_format_args<wformat_context>(fmt_args));
        } else {
        }
        return *this;
    }

    template <typename Char, typename Traits, typename Alloc>
    template <typename... Args>
    basic_string<Char, Traits, Alloc> basic_string<Char, Traits, Alloc>::format_copy(basic_string_view<Char> fmt,
                                                                                     Args const &...args) const {
        basic_string<Char, Traits, Alloc> result;
        if constexpr (type_traits::type_relations::is_same_v<char, Char>) {
            auto fmt_args = make_format_args<format_context>(args...);
            text::vformat_to(utility::back_inserter(result), fmt, basic_format_args<format_context>(fmt_args));
        } else if constexpr (type_traits::type_relations::is_same_v<wchar_t, Char>) {
            auto fmt_args = make_format_args<wformat_context>(args...);
            text::vformat_to(utility::back_inserter(result), fmt, basic_format_args<wformat_context>(fmt_args));
        } else {
        }
        return result;
    }
}

namespace rainy::text {
    using core::text::format;
    using core::text::format_to;
    using core::text::vformat;
    using core::text::vformat_to;
}

#endif
