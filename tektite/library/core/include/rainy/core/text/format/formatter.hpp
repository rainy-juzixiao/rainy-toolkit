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
#ifndef RAINY_YESOD_TEXT_FORMAT_FORMATTER_HPP
#define RAINY_YESOD_TEXT_FORMAT_FORMATTER_HPP
#include <cmath>
#include <locale>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/format/implements.hpp>
#include <rainy/core/text/wstring_convert.hpp>

namespace rainy::core::text {
    template <typename CharType, typename Traits>
    /**
     * \lang english
     * @brief Formatter specialization for rainy basic_string_view values.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 面向 rainy basic_string_view 值的 formatter 特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     */
    struct formatter<basic_string_view<CharType, Traits>, CharType> {
        /**
         * \lang english
         * @brief Parses the string format specification (fill/align, width, precision, type).
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范（填充/对齐、宽度、精度、类型）。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the pointer format specification.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析指针格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }
            auto next_it = it + 1;
            if (next_it != end && implements::is_align_char(static_cast<char>(*next_it))) {
                specs_.fill = *it;
                specs_.align = parse_align(*next_it);
                it = next_it + 1;
            } else if (implements::is_align_char(static_cast<char>(*it))) {
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

        /**
         * \lang english
         * @brief Formats the string view into the output context.
         *
         * @tparam FormatContext The format context type
         * @param str The string view to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将字符串视图格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param str 待格式化的字符串视图
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
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
            (void) name;
            if (!arg.is_integral()) {
                exceptions::runtime::throw_format_error("argument must be an integer");
            }
            return arg.visit([](auto &&value) -> int { // NOLINT
                using Ty = type_traits::other_trans::decay_t<decltype(value)>;
                if constexpr (type_traits::primary_types::is_integral_v<Ty> && !type_traits::type_relations::is_same_v<Ty, bool>) {
                    return static_cast<int>(value);
                }
                exceptions::runtime::throw_format_error("argument must be an integer");
                return 0;
            });
        }

        implements::format_specs<CharType> specs_;
    };

    template <typename CharType, typename Traits, typename Alloc>
    /**
     * \lang english
     * @brief Formatter specialization for rainy basic_string values.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Alloc The allocator type
     *
     * \lang simp-chinese
     * @brief 面向 rainy basic_string 值的 formatter 特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Alloc 分配器类型
     */
    struct formatter<basic_string<CharType, Traits, Alloc>, CharType> {
        /**
         * \lang english
         * @brief Parses the string format specification, delegating to the string_view formatter.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范，委托给 string_view formatter 处理。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the string format specification, delegating to the string_view formatter.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范，委托给 string_view formatter 处理。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the string format specification, delegating to the string_view formatter.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范，委托给 string_view formatter 处理。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the string format specification, delegating to the string_view formatter.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范，委托给 string_view formatter 处理。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the string format specification, delegating to the string_view formatter.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范，委托给 string_view formatter 处理。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the bool format specification ('s' for text, 'd'/'b'/'o'/'x'/'X' for digits).
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析 bool 格式规范（'s' 输出文本，'d'/'b'/'o'/'x'/'X' 输出数字）。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        /**
         * \lang english
         * @brief Parses the character format specification.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        /**
         * \lang english
         * @brief Formats the string into the output context.
         *
         * @tparam FormatContext The format context type
         * @param str The string to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将字符串格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param str 待格式化的字符串
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
        template <typename FormatContext>
        auto format(const basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str.data(), str.size()), ctx);
        }

    private:
        formatter<basic_string_view<CharType, Traits>, CharType> underlying_;
    };

    template <typename CharType>
    /**
     * \lang english
     * @brief Formatter specialization for null-terminated character pointers.
     *
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 面向以空字符结尾的字符指针的 formatter 特化。
     *
     * @tparam CharType 字符类型
     */
    struct formatter<const CharType *, CharType> {
    public:
        /**
         * \lang english
         * @brief Parses the character string format specification.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符字符串格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        /**
         * \lang english
         * @brief Formats the null-terminated character string into the output context.
         *
         * @tparam FormatContext The format context type
         * @param str The character string to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将以空字符结尾的字符串格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param str 待格式化的字符串
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
        template <typename FormatContext>
        auto format(const CharType *str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str), ctx);
        }

    private:
        formatter<basic_string_view<CharType>, CharType> underlying_;
    };

    template <typename CharType, std::size_t N>
    /**
     * \lang english
     * @brief Formatter specialization for fixed-size character arrays.
     *
     * @tparam CharType The character type
     * @tparam N The array size
     *
     * \lang simp-chinese
     * @brief 面向固定大小字符数组的 formatter 特化。
     *
     * @tparam CharType 字符类型
     * @tparam N 数组大小
     */
    struct formatter<const CharType[N], CharType> {
    public:
        /**
         * \lang english
         * @brief Parses the character array format specification.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符数组格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        /**
         * \lang english
         * @brief Formats the character array into the output context.
         *
         * @tparam FormatContext The format context type
         * @param str The character array to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将字符数组格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param str 待格式化的字符数组
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
        template <typename FormatContext>
        auto format(const CharType (&str)[N], FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str, N), ctx);
        }

    private:
        formatter<basic_string_view<CharType>, CharType> underlying_;
    };

    template <typename CharType, typename Traits, typename Alloc>
    /**
     * \lang english
     * @brief Formatter specialization for std::basic_string values.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Alloc The allocator type
     *
     * \lang simp-chinese
     * @brief 面向 std::basic_string 值的 formatter 特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Alloc 分配器类型
     */
    struct formatter<std::basic_string<CharType, Traits, Alloc>, CharType> {
    public:
        /**
         * \lang english
         * @brief Parses the string format specification.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        /**
         * \lang english
         * @brief Formats the std::basic_string into the output context.
         *
         * @tparam FormatContext The format context type
         * @param str The string to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将 std::basic_string 格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param str 待格式化的字符串
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
        template <typename FormatContext>
        auto format(const std::basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str.data(), str.size()), ctx);
        }

    private:
        formatter<basic_string_view<CharType, Traits>, CharType> underlying_;
    };

    template <typename CharType, typename Traits>
    /**
     * \lang english
     * @brief Formatter specialization for std::basic_string_view values.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 面向 std::basic_string_view 值的 formatter 特化。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     */
    struct formatter<std::basic_string_view<CharType, Traits>, CharType> {
    public:
        /**
         * \lang english
         * @brief Parses the string view format specification.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符串视图格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            return underlying_.parse(ctx);
        }

        /**
         * \lang english
         * @brief Formats the string view into the output context.
         *
         * @tparam FormatContext The format context type
         * @param str The string view to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将字符串视图格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param str 待格式化的字符串视图
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
        template <typename FormatContext>
        auto format(const basic_string_view<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            return underlying_.format(basic_string_view<CharType>(str.data(), str.size()), ctx);
        }

    private:
        formatter<basic_string_view<CharType, Traits>, CharType> underlying_;
    };
}

namespace rainy::core::text {
    template <typename CharType>
    /**
     * \lang english
     * @brief Formatter specialization for bool values, supporting "true"/"false" or "1"/"0" output.
     *
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 面向 bool 值的 formatter 特化，支持输出 "true"/"false" 或 "1"/"0"。
     *
     * @tparam CharType 字符类型
     */
    struct formatter<bool, CharType> {
    public:
        /**
         * \lang english
         * @brief Parses the bool format specification.
         *
         * Supported presentation types: 's' (string), 'd', 'b', 'o', 'x' and 'X'.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析 bool 格式规范。
         *
         * 支持的表示类型：'s'（字符串）、'd'、'b'、'o'、'x' 和 'X'。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }
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

        /**
         * \lang english
         * @brief Formats the bool value into the output context.
         *
         * @tparam FormatContext The format context type
         * @param value The bool value to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将 bool 值格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param value 待格式化的 bool 值
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
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
    /**
     * \lang english
     * @brief Formatter specialization for object pointers, formatting them as hexadecimal addresses.
     *
     * @tparam Ty The pointed-to type
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 面向对象指针的 formatter 特化，将指针格式化为十六进制地址。
     *
     * @tparam Ty 所指向的类型
     * @tparam CharType 字符类型
     */
    struct formatter<Ty *, CharType> {
        /**
         * \lang english
         * @brief Parses the pointer format specification.
         *
         * Only an empty specification is accepted for pointers.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析指针格式规范。
         *
         * 指针仅接受空的格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format specifier for pointer");
            }
            return it;
        }

        /**
         * \lang english
         * @brief Formats the pointer as a 0x-prefixed hexadecimal address.
         *
         * @tparam FormatContext The format context type
         * @param value The pointer to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将指针格式化为以 0x 为前缀的十六进制地址。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param value 待格式化的指针
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
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

namespace rainy::core::text {
    template <typename CharType>
    /**
     * \lang english
     * @brief Formatter specialization for single character values.
     *
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 面向单个字符值的 formatter 特化。
     *
     * @tparam CharType 字符类型
     */
    struct formatter<CharType, CharType> {
    public:
        /**
         * \lang english
         * @brief Parses the character format specification.
         *
         * Only an empty specification is accepted for characters.
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析字符格式规范。
         *
         * 字符仅接受空的格式规范。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {

            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format specifier for char");
            }
            return it;
        }

        /**
         * \lang english
         * @brief Writes the character to the output context.
         *
         * @tparam FormatContext The format context type
         * @param value The character to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将字符写入输出上下文。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param value 待格式化的字符
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
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

namespace rainy::core::text {
    template <typename Ty, typename CharType>
    /**
     * \lang english
     * @brief Formatter specialization for integral types, supporting decimal, hex, octal and binary output.
     *
     * @tparam Ty The integral type
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 面向整数类型的 formatter 特化，支持十进制、十六进制、八进制与二进制输出。
     *
     * @tparam Ty 整数类型
     * @tparam CharType 字符类型
     */
        /**
         * \lang english
         * @brief Parses the integer format specification (fill/align, sign, 'L', zero-fill, width, type).
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析整数格式规范（填充/对齐、符号、'L'、零填充、宽度、类型）。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
    struct formatter<
        Ty, CharType,
        type_traits::other_trans::enable_if_t<type_traits::primary_types::is_integral_v<Ty> && !type_traits::type_relations::is_same_v<Ty, bool> &&
                         !type_traits::type_relations::is_same_v<Ty, char> && !type_traits::type_relations::is_same_v<Ty, wchar_t> &&
                         !type_traits::type_relations::is_same_v<Ty, char16_t> &&
                         !type_traits::type_relations::is_same_v<Ty, char32_t>>> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();
            if (it == end || *it == '}') {
                return it;
            }

            auto next = it;
            ++next;
            if (next != end && implements::is_align_char(static_cast<char>(*next))) {
                specs_.fill = *it++;
                set_align(*it++);
            } else if (implements::is_align_char(static_cast<char>(*it))) {
                set_align(*it++);
            }

            if (it != end && (*it == CharType('+') || *it == CharType('-') || *it == CharType(' '))) {
                sign_ = static_cast<char>(*it++);
            }

            if (it != end && *it == CharType('L')) {
                use_locale_ = true;
                ++it;
            }

            if (it != end && *it == CharType('0') && specs_.align == implements::align_type::none) {
                zero_fill_ = true;
                specs_.fill = static_cast<CharType>('0');
                ++it;
            }

            if (it != end && *it >= '0' && *it <= '9') {
                specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
            }

            if (it != end && *it != '}') {
                presentation_type_ = static_cast<char>(*it++);
                if (presentation_type_ != 'd' && presentation_type_ != 'x' && presentation_type_ != 'X' && presentation_type_ != 'o' &&
                    presentation_type_ != 'b') {
                    exceptions::runtime::throw_format_error("invalid type specifier for integer");
                }
            }

            if (it != end && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format string");
            }

            return it;
        }

        /**
         * \lang english
         * @brief Formats the integer value into the output context.
         *
         * @tparam FormatContext The format context type
         * @param value The integer value to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将整数值格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param value 待格式化的整数值
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
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

            if (use_locale_ && (presentation_type_ == '\0' || presentation_type_ == 'd')) {
                apply_thousands_sep(buf, ctx.locale());
            }

            if (value >= 0 && sign_ != '\0') {
                std::memmove(buf.data + 1, buf.data, static_cast<std::size_t>(buf.len));
                buf.data[0] = static_cast<CharType>(sign_);
                ++buf.len;
            }

            if (zero_fill_ && specs_.width > buf.len) {
                bool has_sign =
                    buf.len > 0 && (buf.data[0] == static_cast<CharType>('-') || buf.data[0] == static_cast<CharType>('+') ||
                                    buf.data[0] == static_cast<CharType>(' '));
                const int sign_len = has_sign ? 1 : 0;
                const int digit_len = buf.len - sign_len;
                const int padding = specs_.width - buf.len;
                std::memmove(buf.data + sign_len + padding, buf.data + sign_len, static_cast<std::size_t>(digit_len));
                for (int i = 0; i < padding; ++i) {
                    buf.data[sign_len + i] = static_cast<CharType>('0');
                }
                buf.len += padding;
                return write_to(buf.data, static_cast<std::size_t>(buf.len), ctx.out());
            }

            return apply_alignment(buf, ctx);
        }

    private:
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
            using UType = type_traits::helper::make_unsigned_t<IntType>;

            bool negative = false;
            UType uval;

            if constexpr (type_traits::properties::is_signed_v<IntType>) {
                if (value < 0) {
                    negative = true;
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

            reverse(buf.data + start, buf.len - start);

            if (negative) {
                std::memmove(buf.data + start + 1, buf.data + start, static_cast<std::size_t>(buf.len - start));
                buf.data[start] = '-';
                ++buf.len;
            }
        }

        template <typename UType>
        static void write_decimal(UType v, implements::stack_buffer<CharType> &buf) noexcept {
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

            while (v >= 100) {
                int r = static_cast<int>(v % 100);
                v /= 100;
                buf.push(DIGITS_LUT[r * 2 + 1]);
                buf.push(DIGITS_LUT[r * 2]);
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
            for (int l = 0, r = len - 1; l < r; ++l, --r) {
                std::swap(p[l], p[r]);
            }
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

                sep_len = static_cast<int>((core::min) (utf8_sep.size(), static_cast<std::size_t>(8)));
                for (int i = 0; i < sep_len; ++i) {
                    sep_data[i] = utf8_sep[i];
                }
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
            int count = 0;

            while (read >= sign_len) {
                if (count == group_size) {
                    for (int s = sep_len - 1; s >= 0; --s) {
                        buf.data[write--] = static_cast<char>(sep_data[s]);
                    }
                    count = 0;
                }
                buf.data[write--] = buf.data[read--];
                ++count;
            }

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

            if (str_len >= total_width) {
                return write_to(buf.data, str_len, out);
            }

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
                for (std::size_t i = 0; i < n; ++i) {
                    *out++ = static_cast<CharType>(src[i]);
                }
                return out;
            }
        }

        implements::format_specs<CharType> specs_;
        char presentation_type_ = '\0';
        bool use_locale_ = false;
        char sign_ = '\0';
        bool zero_fill_ = false;
    };
}

namespace rainy::core::text {
    template <typename Ty, typename CharType>
    /**
     * \lang english
     * @brief Formatter specialization for floating-point types, supporting fixed, scientific, general and hex formats.
     *
     * @tparam Ty The floating-point type
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 面向浮点类型的 formatter 特化，支持定点、科学计数、常规与十六进制格式。
     *
     * @tparam Ty 浮点类型
     * @tparam CharType 字符类型
     */
        /**
         * \lang english
         * @brief Parses the floating-point format specification (fill/align, width, precision, 'L', type).
         *
         * @param ctx The parse context
         * @return An iterator past the parsed specification
         *
         * \lang simp-chinese
         * @brief 解析浮点格式规范（填充/对齐、宽度、精度、'L'、类型）。
         *
         * @param ctx 解析上下文
         * @return 指向已解析规范末尾之后的迭代器
         */
    struct formatter<Ty, CharType, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_floating_point_v<Ty>>> {
    public:
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();

            if (it == end || *it == '}') {
                return it;
            }

            auto next_it = it;
            if (next_it != end) {
                ++next_it;
            }
            if (next_it != end && implements::is_align_char(static_cast<char>(*next_it))) {
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
                    default:
                        break;
                }
            } else if (implements::is_align_char(static_cast<char>(*it))) {
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

            if (it != end && *it >= '0' && *it <= '9') {
                specs_.width = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
            }

            if (it != end && *it == '.') {
                ++it;
                if (it != end && *it >= '0' && *it <= '9') {
                    specs_.precision = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
                } else {
                    specs_.precision = 6;
                }
            }

            if (it != end && *it == CharType('L')) {
                use_locale_ = true;
                ++it;
            }

            if (it != end && *it != '}') {
                presentation_type_ = static_cast<char>(*it++);
                if (presentation_type_ != 'f' && presentation_type_ != 'F' && presentation_type_ != 'e' && presentation_type_ != 'E' &&
                    presentation_type_ != 'g' && presentation_type_ != 'G' && presentation_type_ != 'a' && presentation_type_ != 'A') {
                    exceptions::runtime::throw_format_error("invalid type specifier for floating-point");
                }
            }

            return it;
        }

        /**
         * \lang english
         * @brief Formats the floating-point value into the output context.
         *
         * @tparam FormatContext The format context type
         * @param value The floating-point value to format
         * @param ctx The format context
         * @return The output iterator after formatting
         *
         * \lang simp-chinese
         * @brief 将浮点数值格式化到输出上下文中。
         *
         * @tparam FormatContext 格式化上下文类型
         * @param value 待格式化的浮点数值
         * @param ctx 格式化上下文
         * @return 格式化完成后的输出迭代器
         */
        template <typename FormatContext>
        auto format(Ty value, FormatContext &ctx) const -> typename FormatContext::iterator {
            basic_string<CharType> str;

            if (!std::isfinite(value)) {
                if (std::isnan(value)) {
                    str.assign({CharType('n'), CharType('a'), CharType('n')});
                } else {
                    if (value < 0) {
                        str.assign({CharType('-'), CharType('i'), CharType('n'), CharType('f')});
                    } else {
                        str.assign({CharType('i'), CharType('n'), CharType('f')});
                    }
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
                    dest[0] = static_cast<char>(wc);
                    return 1;
                }
                int n = static_cast<int>((core::min) (utf8.size(), static_cast<std::size_t>(8)));
                for (int i = 0; i < n; ++i) {
                    dest[i] = utf8[i];
                }
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
                basic_string<CharType> dp_str(dp_chars, dp_chars + dp_len);
                str.replace(dot_pos, 1, dp_str);
            }

            if (grouping.empty() || grouping[0] <= 0 || sep_len == 0) {
                return;
            }

            const int group_size = static_cast<unsigned char>(grouping[0]);

            std::size_t sign_end = 0;
            if (!str.empty() && str[0] == CharType('-')) {
                sign_end = 1;
            }

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
            const int first_group = digit_len - sep_count * group_size;
            CharType buf[512]{};
            int buf_len = 0;
            for (std::size_t i = 0; i < sign_end; ++i) {
                buf[buf_len++] = str[i];
            }
            for (int i = 0; i < first_group; ++i) {
                buf[buf_len++] = str[sign_end + static_cast<std::size_t>(i)];
            }
            for (int g = 0; g < sep_count; ++g) {
                for (int k = 0; k < sep_len; ++k) {
                    buf[buf_len++] = sep_chars[k];
                }
                const int base = static_cast<int>(sign_end) + first_group + g * group_size;
                for (int i = 0; i < group_size; ++i) {
                    buf[buf_len++] = str[static_cast<std::size_t>(base + i)];
                }
            }

            for (std::size_t i = int_end; i < str.size(); ++i) {
                buf[buf_len++] = str[i];
            }
            str.assign(buf, static_cast<std::size_t>(buf_len));
        }

        static void format_fixed(Ty value, basic_string<CharType> &str, int precision) {
            char buf[64]{};
            int prec = precision < 0 ? 6 : precision;
            auto [ptr, ec] = to_chars(buf, buf + sizeof(buf), value, chars_format::fixed, prec);
            if (ec != std::errc{}) {
                core::exceptions::runtime::throw_format_error("floating-point to_chars failed");
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

        static void format_scientific(Ty value, basic_string<CharType> &str, int precision, bool uppercase) {
            char buf[64]{};
            int prec = precision < 0 ? 6 : precision;
            auto [ptr, ec] = to_chars(buf, buf + sizeof(buf), value, chars_format::scientific, prec);
            if (ec != std::errc{}) {
                exceptions::runtime::throw_format_error("floating-point to_chars failed");
            }
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

        static void format_general(Ty value, basic_string<CharType> &str, const int precision, const bool uppercase) {
            char buf[64]{};
            int prec = precision < 0 ? 6 : precision;
            auto [ptr, ec] = to_chars(buf, buf + sizeof(buf), value, chars_format::general, prec);
            if (ec != std::errc{}) {
                exceptions::runtime::throw_format_error("floating-point to_chars failed");
            }
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

        static void format_hex(Ty value, basic_string<CharType> &str, int precision, bool uppercase) {
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
            std::uint64_t bits;
            if constexpr (sizeof(Ty) == sizeof(float)) {
                std::uint32_t tmp;
                builtin::copy_memory(&tmp, &value, sizeof(float));
                bits = tmp;
            } else {
                builtin::copy_memory(&bits, &value, sizeof(double));
            }
            constexpr int mantissa_bits = sizeof(Ty) == sizeof(float) ? 23 : 52;
            constexpr int exponent_bits = sizeof(Ty) == sizeof(float) ? 8 : 11;
            constexpr int exponent_bias = sizeof(Ty) == sizeof(float) ? 127 : 1023;
            const std::uint64_t mantissa_mask = (1ULL << mantissa_bits) - 1;
            std::uint64_t mantissa = bits & mantissa_mask;
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
#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
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
#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
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
                return algorithm::copy(str.begin(), str.end(), ctx.out());
            }
            std::size_t padding = total_width - str_size;
            auto out = ctx.out();
            auto align = specs_.align;
            if (align == implements::align_type::none) {
                align = implements::align_type::right;
            }
            switch (align) {
                case implements::align_type::left:
                    out = algorithm::copy(str.begin(), str.end(), out);
                    return algorithm::fill_n(out, padding, specs_.fill);
                case implements::align_type::right:
                    out = algorithm::fill_n(out, padding, specs_.fill);
                    return algorithm::copy(str.begin(), str.end(), out);
                case implements::align_type::center: {
                    std::size_t lp = padding / 2, rp = padding - lp;
                    out = algorithm::fill_n(out, lp, specs_.fill);
                    out = algorithm::copy(str.begin(), str.end(), out);
                    return algorithm::fill_n(out, rp, specs_.fill);
                }
                default:
                    return algorithm::copy(str.begin(), str.end(), out);
            }
        }

        implements::format_specs<CharType> specs_;
        char presentation_type_ = '\0'; // 'f','F','e','E','g','G','a','A'
        bool use_locale_ = false;
    };
}

#if RAINY_HAS_CXX20
/**
 * \lang english
 * @brief std::formatter specialization that formats a rainy source_location via its to_string().
 *
 * \lang simp-chinese
 * @brief 通过 source_location 的 to_string() 对其进行格式化的 std::formatter 特化。
 */
template <>
class std::formatter<rainy::core::diagnostics::source_location, char> {
public:
    explicit formatter() noexcept = default;

    /**
     * \lang english
     * @brief Accepts an empty format specification.
     *
     * @param ctx The parse context
     * @return An iterator past the parsed specification
     *
     * \lang simp-chinese
     * @brief 接受空格式规范。
     *
     * @param ctx 解析上下文
     * @return 指向已解析规范末尾之后的迭代器
     */
    static auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    /**
     * \lang english
     * @brief Formats the source location as its string representation.
     *
     * @param value The source location to format
     * @param fc The format context
     * @return The output iterator after formatting
     *
     * \lang simp-chinese
     * @brief 将源码位置格式化为其字符串表示。
     *
     * @param value 待格式化的源码位置
     * @param fc 格式化上下文
     * @return 格式化完成后的输出迭代器
     */
    RAINY_NODISCARD static auto format(const rainy::core::diagnostics::source_location &value, std::format_context fc) noexcept {
        return std::format_to(fc.out(), "{}", value.to_string());
    }
};
#endif

namespace rainy::text {
    using core::text::formatter;
}

#endif
