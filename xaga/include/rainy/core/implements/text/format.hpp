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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_FORMAT_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_FORMAT_HPP
#include <rainy/core/implements/text/format/context.hpp>

namespace rainy::foundation::text::implements {
    template <typename CharT>
    constexpr const CharT *find_next_brace(const CharT *begin, const CharT *end, CharT ch) {
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

    template <typename OutputIt, typename CharT>
    OutputIt write_char(OutputIt out, CharT c) {
        *out = c;
        ++out;
        return out;
    }

    template <typename OutputIt, typename CharT>
    OutputIt write_string(OutputIt out, const CharT *begin, const CharT *end) {
        return std::copy(begin, end, out);
    }

    // 写入字符串视图
    template <typename OutputIt, typename CharT>
    OutputIt write_string(OutputIt out, basic_string_view<CharT> str) {
        return std::copy(str.begin(), str.end(), out);
    }

    // 格式化单个参数
    template <typename OutputIt, typename CharT>
    OutputIt format_arg(OutputIt out, basic_format_parse_context<CharT> &parse_ctx, basic_format_context<OutputIt, CharT> &format_ctx,
                        size_t arg_id) {

        auto arg = format_ctx.arg(arg_id);
        if (!arg) {
            throw format_error("argument index out of range");
        }

        // 使用 visit 来处理不同类型的参数
        return arg.visit([&](auto &&value) -> OutputIt {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                throw format_error("invalid argument");
            } else if constexpr (std::is_same_v<T, typename basic_format_arg<basic_format_context<OutputIt, CharT>>::handle>) {
                // 自定义类型通过 handle 处理
                value.format(parse_ctx, format_ctx);
                return format_ctx.out();
            } else {
                // 使用对应类型的 formatter
                using Context = basic_format_context<OutputIt, CharT>;
                typename Context::template formatter_type<T> f;

                // 解析格式规范
                parse_ctx.advance_to(f.parse(parse_ctx));

                // 格式化值
                format_ctx.advance_to(f.format(value, format_ctx));
                return format_ctx.out();
            }
        });
    }

    template <typename OutputIt, typename CharT>
    OutputIt do_vformat(OutputIt out, basic_string_view<CharT> fmt, basic_format_parse_context<CharT> &parse_ctx,
                        basic_format_context<OutputIt, CharT> &format_ctx,
                        basic_format_args<basic_format_context<OutputIt, CharT>> args) {
        const CharT *p = fmt.data();
        const CharT *end = p + fmt.size();
        size_t next_auto_arg_id = 0;

        while (p != end) {
            // 查找下一个 '{'
            const CharT *brace_begin = find_next_brace(p, end, CharT('{'));

            // 输出 '{' 之前的文字部分
            while (p != brace_begin) {
                if (*p == CharT('{') || *p == CharT('}')) {
                    // 转义的 {{ 或 }}，只输出一个
                    out = write_char(out, *p);
                    ++p;
                    if (p != end && (*p == CharT('{') || *p == CharT('}'))) {
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
                throw format_error("invalid format string: unmatched '{'");
            }

            // 检查是否是转义的 {{
            if (*p == CharT('{')) {
                out = write_char(out, CharT('{'));
                ++p;
                continue;
            }

            // 解析参数 ID
            size_t arg_id;
            const CharT *spec_begin = p; // 先记录位置

            if (*p >= CharT('0') && *p <= CharT('9')) {
                // 显式索引
                arg_id = 0;
                while (p != end && *p >= CharT('0') && *p <= CharT('9')) {
                    arg_id = arg_id * 10 + (*p - CharT('0'));
                    ++p;
                }
                spec_begin = p; // 更新 spec_begin 到数字之后
            } else if (*p == CharT('}') || *p == CharT(':')) {
                // 自动索引：{} 或 {:...}
                arg_id = next_auto_arg_id++;
                // spec_begin 已经正确指向当前位置
            } else {
                throw format_error("invalid format string: expected argument index, ':', or '}'");
            }

            if (arg_id >= args.size()) {
                throw format_error("argument index out of range");
            }

            // 解析格式规范
            if (p != end && *p == CharT(':')) {
                ++p;
                spec_begin = p; // 格式规范从 ':' 之后开始
                int brace_level = 0;

                while (p != end) {
                    if (*p == CharT('{')) {
                        ++brace_level;
                    } else if (*p == CharT('}')) {
                        if (brace_level == 0) {
                            break;
                        }
                        --brace_level;
                    }
                    ++p;
                }
            }

            if (p == end || *p != CharT('}')) {
                throw format_error("invalid format string: unmatched '{'");
            }

            // 创建格式规范的解析上下文
            basic_string_view<CharT> spec(spec_begin, p - spec_begin);
            basic_format_parse_context<CharT> arg_parse_ctx(spec, args.size());

            // 格式化参数
            format_ctx.advance_to(out);
            out = format_arg(out, arg_parse_ctx, format_ctx, arg_id);

            ++p;
        }

        return out;
    }

    // 核心格式化实现
    template <typename OutputIt, typename CharT>
    OutputIt vformat_to_impl(OutputIt out, basic_string_view<CharT> fmt, basic_format_args<basic_format_context<OutputIt, CharT>> args,
                             std::optional<std::locale> loc = std::nullopt) {

        basic_format_parse_context<CharT> parse_ctx(fmt, args.size());

        if (loc.has_value()) {
            basic_format_context<OutputIt, CharT> format_ctx(out, args, loc.value());
            return do_vformat(out, fmt, parse_ctx, format_ctx, args);
        } else {
            basic_format_context<OutputIt, CharT> format_ctx(out, args);
            return do_vformat(out, fmt, parse_ctx, format_ctx, args);
        }
    }
}

namespace rainy::foundation::text {
    template <typename OutputIt, typename CharT>
    OutputIt vformat_to(OutputIt out, basic_string_view<CharT> fmt, basic_format_args<basic_format_context<OutputIt, CharT>> args) {
        try {
            return implements::vformat_to_impl(out, fmt, args, std::nullopt);
        } catch (const std::exception &e) {
            throw format_error(std::string("format error: ") + e.what());
        }
    }

    template <typename OutputIt, typename CharT>
    OutputIt vformat_to(OutputIt out, const std::locale &loc, basic_string_view<CharT> fmt,
                        basic_format_args<basic_format_context<OutputIt, CharT>> args) {

        try {
            return implements::vformat_to_impl(out, fmt, args, loc);
        } catch (const std::exception &e) {
            throw format_error(std::string("format error: ") + e.what());
        }
    }

    RAINY_INLINE string vformat(string_view fmt, format_args args) {
        string result;
        text::vformat_to(utility::back_inserter(result), fmt, args);
        return result;
    }

    RAINY_INLINE wstring vformat(wstring_view fmt, wformat_args args) {
        wstring result;
        vformat_to(utility::back_inserter(result), fmt, args);
        return result;
    }

    RAINY_INLINE string vformat(const std::locale &loc, string_view fmt, format_args args) {
        string result;
        vformat_to(utility::back_inserter(result), loc, fmt, args);
        return result;
    }

    RAINY_INLINE wstring vformat(const std::locale &loc, wstring_view fmt, wformat_args args) {
        wstring result;
        vformat_to(utility::back_inserter(result), loc, fmt, args);
        return result;
    }

    template <typename... Args>
    string format(string_view fmt, const Args &...args) {
        auto arg_store = make_format_args(args...);
        return vformat(fmt, basic_format_args(arg_store));
    }

    template <typename... Args>
    wstring format(wstring_view fmt, const Args &...args) {
        auto arg_store = make_wformat_args(args...);
        return vformat(fmt, basic_format_args(arg_store));
    }

    template <typename... Args>
    string format(const std::locale &loc, string_view fmt, const Args &...args) {
        auto arg_store = make_format_args(args...);
        return vformat(loc, fmt, basic_format_args(arg_store));
    }

    template <typename... Args>
    wstring format(const std::locale &loc, wstring_view fmt, const Args &...args) {
        auto arg_store = make_wformat_args(args...);
        return vformat(loc, fmt, basic_format_args(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, string_view fmt, const Args &...args) {
        auto arg_store = make_format_args<format_context>(args...);
        return vformat_to(out, fmt, basic_format_args<format_context>(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, wstring_view fmt, const Args &...args) {
        auto arg_store = make_format_args<wformat_context>(args...);
        return vformat_to(out, fmt, basic_format_args<wformat_context>(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, const std::locale &loc, std::string_view fmt, const Args &...args) {
        auto arg_store = make_format_args<format_context>(args...);
        return vformat_to(out, loc, fmt, basic_format_args<format_context>(arg_store));
    }

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, const std::locale &loc, std::wstring_view fmt, const Args &...args) {
        auto arg_store = make_format_args<wformat_context>(args...);
        return vformat_to(out, loc, fmt, basic_format_args<wformat_context>(arg_store));
    }
}

#endif