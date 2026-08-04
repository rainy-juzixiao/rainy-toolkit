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
#ifndef RAINY_TEXT_FORMAT_RANGE_FORMATTER_HPP
#define RAINY_TEXT_FORMAT_RANGE_FORMATTER_HPP
#include <rainy/core/text/format/context.hpp>
#include <rainy/core/text/format/formatter.hpp>

namespace rainy::core::text::implements {
    template <typename Ty, typename = void>
    struct is_range : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_range<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().begin()), decltype(utility::declval<Ty &>().end())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_range_v = is_range<Ty>::value;

    template <typename Ty>
    struct range_value {
        using type = type_traits::modifers::remove_cvref_t<decltype(*utility::declval<Ty &>().begin())>;
    };

    template <typename Ty>
    using range_value_t = typename range_value<Ty>::type;

    template <typename Ty, typename CharType>
    inline constexpr bool has_conflicting_formatter_v = false;

    template <typename CharType, typename Traits>
    inline constexpr bool has_conflicting_formatter_v<basic_string_view<CharType, Traits>, CharType> = true;

    template <typename CharType, typename Traits, typename Allocator>
    inline constexpr bool has_conflicting_formatter_v<basic_string<CharType, Traits, Allocator>, CharType> = true;

    template <typename CharType, typename Traits>
    inline constexpr bool has_conflicting_formatter_v<std::basic_string_view<CharType, Traits>, CharType> = true;

    template <typename CharType, typename Traits, typename Allocator>
    inline constexpr bool has_conflicting_formatter_v<std::basic_string<CharType, Traits, Allocator>, CharType> = true;

    template <typename Ty, typename CharType>
    inline constexpr bool is_formattable_range_v =
        is_range_v<Ty> && !has_conflicting_formatter_v<type_traits::modifers::remove_cvref_t<Ty>, CharType>;
}

namespace rainy::core::text {
    template <typename Ty, typename CharType = char>
    class range_formatter {
    public:
        using element_type = Ty;

        range_formatter() = default;

        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) -> typename ParseContext::iterator {
            auto it = ctx.begin();
            auto end = ctx.end();

            if (it == end || *it == '}') {
                return it;
            }

            // 1. Parse fill and alignment
            auto next_it = it + 1;
            if (next_it != end && implements::is_align_char(static_cast<char>(*next_it))) {
                fill_ = *it;
                switch (*next_it) {
                    case '<': {
                        align_ = implements::align_type::left;
                        break;
                    }
                    case '>': {
                        align_ = implements::align_type::right;
                        break;
                    }
                    case '^': {
                        align_ = implements::align_type::center;
                        break;
                    }
                    default:
                        break;
                }
                it += 2;
            } else if (implements::is_align_char(static_cast<char>(*it))) {
                switch (*it) {
                    case '<': {
                        align_ = implements::align_type::left;
                        break;
                    }
                    case '>': {
                        align_ = implements::align_type::right;
                        break;
                    }
                    case '^': {
                        align_ = implements::align_type::center;
                        break;
                    }
                    default:
                        break;
                }
                ++it;
            }

            // 2. Parse width
            if (it != end && *it >= '0' && *it <= '9') {
                width_ = static_cast<int>(implements::parse_nonnegative_int<CharType>(it, end));
            }

            // 3. Parse 'n' flag — no brackets
            if (it != end && *it == 'n') {
                show_brackets_ = false;
                ++it;
            }

            // 4. Parse ':' for element format — everything after is the element format spec
            if (it != end && *it == ':') {
                ++it;
                ctx.advance_to(it);
                it = element_formatter_.parse(ctx);
                return it;
            }

            if (it != end && *it != '}') {
                exceptions::runtime::throw_format_error("invalid format specifier for range");
            }

            return it;
        }

        template <typename Range, typename FormatContext>
        auto format(Range &&range, FormatContext &ctx) const -> typename FormatContext::iterator {
            basic_string<CharType> result;

            if (show_brackets_) {
                result.push_back(open_bracket_);
            }

            bool first = true;
            for (auto &&element: range) {
                if (!first) {
                    result.append(separator_);
                }
                first = false;

                basic_string<CharType> element_str;
                auto element_out = utility::back_inserter(element_str);
                using element_context = basic_format_context<decltype(element_out), CharType>;
                element_context element_ctx(std::move(element_out), basic_format_args<element_context>{});
                element_formatter_.format(element, element_ctx);
                result.append(element_str);
            }

            if (show_brackets_) {
                result.push_back(close_bracket_);
            }

            return write_aligned(result, ctx);
        }

        constexpr void set_separator(basic_string_view<CharType> sep) {
            separator_.assign(sep.data(), sep.size());
        }

        constexpr void set_brackets(CharType open, CharType close) {
            open_bracket_ = open;
            close_bracket_ = close;
            show_brackets_ = true;
        }

        constexpr void clear_brackets() {
            show_brackets_ = false;
        }

    private:
        template <typename FormatContext>
        auto write_aligned(const basic_string<CharType> &str, FormatContext &ctx) const -> typename FormatContext::iterator {
            auto str_size = str.size();
            const auto total_width = width_ > 0 ? static_cast<std::size_t>(width_) : str_size;
            if (str_size >= total_width) {
                return algorithm::copy(str.begin(), str.end(), ctx.out());
            }

            auto padding = total_width - str_size;
            auto out = ctx.out();
            auto align = (align_ == implements::align_type::none) ? implements::align_type::left : align_;

            switch (align) {
                case implements::align_type::left: {
                    out = algorithm::copy(str.begin(), str.end(), std::move(out));
                    return algorithm::fill_n(std::move(out), padding, fill_);
                }
                case implements::align_type::right: {
                    out = algorithm::fill_n(std::move(out), padding, fill_);
                    return algorithm::copy(str.begin(), str.end(), std::move(out));
                }
                case implements::align_type::center: {
                    auto lp = padding / 2;
                    auto rp = padding - lp;
                    out = algorithm::fill_n(std::move(out), lp, fill_);
                    out = algorithm::copy(str.begin(), str.end(), std::move(out));
                    return algorithm::fill_n(std::move(out), rp, fill_);
                }
                default: {
                    return algorithm::copy(str.begin(), str.end(), ctx.out());
                }
            }
        }

        formatter<Ty, CharType> element_formatter_;
        basic_string<CharType> separator_{static_cast<CharType>(','), static_cast<CharType>(' ')};
        CharType fill_ = static_cast<CharType>(' ');
        implements::align_type align_ = implements::align_type::none;
        int width_ = 0;
        CharType open_bracket_ = static_cast<CharType>('[');
        CharType close_bracket_ = static_cast<CharType>(']');
        bool show_brackets_ = true;
    };

    template <typename Range, typename CharType>
    struct formatter<Range, CharType, type_traits::other_trans::enable_if_t<implements::is_formattable_range_v<Range, CharType>>>
        : range_formatter<implements::range_value_t<Range>, CharType> {};
}

#endif
