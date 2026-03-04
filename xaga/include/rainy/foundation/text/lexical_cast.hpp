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
#ifndef RAINY_FOUNDATION_TEXT_LEXICAL_CAST_HPP
#define RAINY_FOUNDATION_TEXT_LEXICAL_CAST_HPP
#include <memory>
#include <rainy/core/core.hpp>
#include <rainy/foundation/container/pair.hpp>
#include <rainy/text/hashed_string.hpp>

namespace rainy::foundation::text {
    template <typename Ty, typename CharType, typename = void>
    struct lexical_caster {
        static constexpr bool value = false;
    };
}

namespace rainy::foundation::text::implements {
    template <typename Ty, typename CharType, typename = void>
    RAINY_CONSTEXPR_BOOL has_lexical_caster = true;

    template <typename Ty, typename CharType>
    RAINY_CONSTEXPR_BOOL
        has_lexical_caster<Ty, CharType, type_traits::other_trans::void_t<decltype(lexical_caster<Ty, CharType>::value)>> = false;
}

namespace rainy::foundation::exceptions::logic {
    struct bad_lexical_cast : public logic_error {
        bad_lexical_cast(std::string_view type) : logic_error(std::format("cannot proceed function in '{}'", type)) {
        }

        bad_lexical_cast(std::string_view type, std::string_view msg) :
            logic_error(std::format("cannot proceed function in '{}': {}", type, msg)) {
        }
    };

    RAINY_INLINE void throw_bad_lexical_cast(std::string_view type) {
        throw_exception_if(with_this_exception<bad_lexical_cast>, false, type);
    }

    RAINY_INLINE void throw_bad_lexical_cast(std::string_view type, std::string_view msg) {
        throw_exception_if(with_this_exception<bad_lexical_cast>, false, type, msg);
    }
}

namespace rainy::foundation::text::implements {
    inline constexpr int OpenCurly = '{';
    inline constexpr int CloseCurly = '}';
    inline constexpr int OpenBracket = '[';
    inline constexpr int CloseBracket = ']';
    inline constexpr int OpenParen = '(';
    inline constexpr int CloseParen = ')';
    inline constexpr int OpenQuote = '"';
    inline constexpr int CloseQuote = '"';

    template <typename CharType>
    bool is_matching_pair(CharType open, CharType close) {
        return (open == OpenQuote && close == CloseQuote) || (open == OpenCurly && close == CloseCurly) ||
               (open == OpenBracket && close == CloseBracket) || (open == OpenParen && close == CloseParen);
    }

    template <typename CharType, typename Iter>
    bool matching_delims(Iter iter, Iter end) {
        if (iter == end) {
            return false;
        }
        CharType first = *iter;
        CharType last = *(end - 1);
        return is_matching_pair<CharType>(first, last);
    }

    template <typename CharType, typename Iter>
    container::pair<Iter, Iter> unwrap_helper(Iter begin, Iter end) {
        begin = std::find_if_not(begin, end, [](CharType ch) { return ch == ' '; });
        end = std::find_if_not(utility::make_reverse_iterator(end), utility::make_reverse_iterator(begin), [](CharType ch) {
                  return ch == ' ';
              }).base();
        return {begin, end};
    }

    template <typename CharType>
    basic_string_view<CharType> unwrap_helper(std::basic_string_view<CharType> str) {
        auto [b, e] = unwrap_helper<CharType>(str.begin(), str.end());
        return {b, e};
    }

    template <typename CharType, typename Iter>
    container::pair<Iter, Iter> unwrap(Iter begin, Iter end) {
        // 循环处理,直到无法继续去除空格和分隔符
        while (true) {
            auto [b, e] = unwrap_helper<CharType>(begin, end);
            if (b >= e) {
                return {b, e};
            }
            if (matching_delims<CharType>(b, e)) {
                begin = b + 1;
                end = e - 1;
            } else {
                return {b, e};
            }
        }
    }
}

namespace rainy::foundation::text {
    // bool
    template <>
    struct lexical_caster<bool, char> {
        bool convert(string_view input) const {
            text::hashed_string hashed{input};
            rainy_switch_hashed_string(hashed) {
                case rainy_match_hashed_string("0"):
                case rainy_match_hashed_string("f"):
                case rainy_match_hashed_string("F"):
                case rainy_match_hashed_string("false"):
                case rainy_match_hashed_string("False"):
                case rainy_match_hashed_string("FALSE"):
                    return false;
                case rainy_match_hashed_string("1"):
                case rainy_match_hashed_string("t"):
                case rainy_match_hashed_string("T"):
                case rainy_match_hashed_string("true"):
                case rainy_match_hashed_string("True"):
                case rainy_match_hashed_string("TRUE"):
                    return true;
            }; /* fast path ^^^ */
            constexpr std::size_t small_buffer_size = 16;
            if (input.size() >= small_buffer_size) {
                exceptions::logic::throw_bad_lexical_cast("boolean caster convert()", "input is too large");
            }
            char buffer[small_buffer_size]{};
            // 此处，对字符串进行清洗，避免再次匹配失败
            /*
            "t r u e" -> exception error
            " [ true ] " -> ok
            " [ false ]" -> ok
            */
            auto iter = core::algorithm::transform(input.begin(), input.end(), buffer, [](char ch) -> char {
                if (ch == implements::OpenBracket || ch == implements::CloseBracket) {
                    return ' ';
                } else {
                    return static_cast<char>(std::tolower(static_cast<int>(ch)));
                }
            });
            auto iter_pair = implements::unwrap<char>(buffer, iter);
            string_view view{iter_pair.first, iter_pair.second};
            // 再次匹配
            hashed = view;
            rainy_switch_hashed_string(hashed) {
                case rainy_match_hashed_string("0"):
                case rainy_match_hashed_string("f"):
                case rainy_match_hashed_string("F"):
                case rainy_match_hashed_string("false"):
                case rainy_match_hashed_string("False"):
                case rainy_match_hashed_string("FALSE"):
                    return false;
                case rainy_match_hashed_string("1"):
                case rainy_match_hashed_string("t"):
                case rainy_match_hashed_string("T"):
                case rainy_match_hashed_string("true"):
                case rainy_match_hashed_string("True"):
                case rainy_match_hashed_string("TRUE"):
                    return true;
            };
            exceptions::logic::throw_bad_lexical_cast("boolean caster convert()");
            std::terminate();
        }

        string to_string(bool input) const {
            return input ? "true" : "false";
        }
    };

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_char =
        type_traits::type_relations::is_same_v<Ty, char> || type_traits::type_relations::is_same_v<Ty, wchar_t> ||
        type_traits::type_relations::is_same_v<Ty, char16_t> || type_traits::type_relations::is_same_v<Ty, char32_t>
#if RAINY_HAS_CXX20
        || type_traits::type_relations::is_same_v<Ty, char8_t>
#endif
        ;

    // char、wchar_t、char16_t、char32_t、char8_t
    template <typename Ty, typename CharType>
    struct lexical_caster<Ty, CharType, type_traits::other_trans::enable_if_t<is_char<Ty> && is_char<CharType>>> {
        Ty convert(basic_string_view<CharType> input) const {
            if (input.size() != 1) {
                exceptions::logic::throw_bad_lexical_cast("char_type caster convert()", "The string's size must be 1!");
            }
            return static_cast<Ty>(input[0]);
        }

        basic_string<CharType> to_string(bool input) const {
            CharType buf[] = {static_cast<Ty>(input[0]), '\0'};
            return basic_string_view<CharType>{buf, 2};
        }
    };

    template <typename Ty>
    struct lexical_caster<Ty, char,
                          type_traits::other_trans::enable_if_t<type_traits::primary_types::is_integral_v<Ty> && !is_char<Ty>>> {
        Ty convert(string_view input) const {
            Ty val{};
            auto iter_pair = implements::unwrap<char>(input.begin(), input.end());
            string_view view{iter_pair.first, iter_pair.second};
            if (from_chars_result res = from_chars(view.begin(), view.end(), val); !res) {
                string msg = format("from_chars error: {}, input: {}", static_cast<int>(res.ec), input);
                exceptions::logic::throw_bad_lexical_cast("integer caster convert()", msg.data());
            }
            return val;
        }

        string to_string(Ty input) const {
            constexpr std::size_t buffer_size = 128;
            char buffer[buffer_size]{};
            Ty val{};
            to_chars_result res = to_chars(&buffer, &buffer + buffer_size, input);
            if (!res) {
                exceptions::logic::throw_bad_lexical_cast("integer caster to_string()");
            }
            return res.ptr;
        }
    };

    template <typename Ty>
    struct lexical_caster<Ty, char, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_floating_point_v<Ty>>> {
        Ty convert(string_view input) const {
            Ty val{};
            auto iter_pair = implements::unwrap<char>(input.begin(), input.end());
            string_view view{iter_pair.first, iter_pair.second};
            if (from_chars_result res = from_chars(view.begin(), view.end(), val); !res) {
                string msg = format("from_chars error: {}, input: {}", static_cast<int>(res.ec), input);
                exceptions::logic::throw_bad_lexical_cast("floating point caster convert()", msg.data());
            }
            return val;
        }

        string to_string(Ty input) const {
            constexpr std::size_t buffer_size = 128;
            char buffer[buffer_size]{};
            Ty val{};
            to_chars_result res = to_chars(&buffer, &buffer + buffer_size, input);
            if (!res) {
                exceptions::logic::throw_bad_lexical_cast("integer caster to_string()");
            }
            return res.ptr;
        }
    };
}

namespace rainy::foundation::text {
    template <typename T>
    struct string_adapter {
        static constexpr bool is_valid = false;
    };

    // 适配自定义 basic_string
    template <typename Char, typename Traits, typename Alloc>
    struct string_adapter<basic_string<Char, Traits, Alloc>> {
        using string_type = basic_string<Char, Traits, Alloc>;
        using char_type = Char;
        using traits_type = Traits;
        using alloc_type = Alloc;

        template <typename Char, typename Traits, typename Alloc>
        using string_template = basic_string<Char, Traits, Alloc>;

        static constexpr bool is_valid = true;
        static constexpr bool is_std = false;
    };

    // 适配标准库 std::basic_string
    template <typename Char, typename Traits, typename Alloc>
    struct string_adapter<std::basic_string<Char, Traits, Alloc>> {
        using string_type = std::basic_string<Char, Traits, Alloc>;
        using char_type = Char;
        using traits_type = Traits;
        using alloc_type = Alloc;

        template <typename Char, typename Traits, typename Alloc>
        using string_template = std::basic_string<Char, Traits, Alloc>;

        static constexpr bool is_valid = true;
        static constexpr bool is_std = true;
    };

    template <typename TargetString, typename CharType>
    struct lexical_caster<TargetString, CharType,
                          type_traits::other_trans::enable_if_t<string_adapter<TargetString>::is_valid &&
                                                                is_char<typename string_adapter<TargetString>::char_type>>> {
        using adapter = string_adapter<TargetString>;
        using char_type = typename adapter::char_type;
        using traits = typename adapter::traits_type;
        using allocator = typename adapter::alloc_type;
        using string_type = typename adapter::string_type;
        static constexpr bool is_std = adapter::is_std;

        // 提取指定字符类型的分配器
        template <typename Alloc, typename CharT>
        using rebind_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<CharT>;

        using char_allocator = rebind_alloc<allocator, char>;

        template <typename Ty>
        struct extract_traits_template;

        template <typename CharT>
        struct extract_traits_template<std::char_traits<CharT>> {
            template <typename NewCharT>
            using type = std::char_traits<NewCharT>;
        };

        template <template <typename> class TraitsTemplate, typename CharT>
        struct extract_traits_template<TraitsTemplate<CharT>> {
            template <typename NewCharT>
            using type = TraitsTemplate<NewCharT>;
        };

        // 提取 traits 模板
        template <typename NewCharT>
        using traits_template = typename extract_traits_template<traits>::template type<NewCharT>;

        template <typename Char, typename Traits, typename Alloc>
        using string_template = adapter::template string_template<Char, Traits, Alloc>;

        string_type convert(basic_string_view<CharType> input) const {
            if constexpr (type_traits::type_relations::is_same_v<char_type, CharType>) {
                // 字符类型相同，直接构造
                return string_type(input.begin(), input.end());
            } else {
                // 需要字符编码转换
                return convert_encoding<CharType, char_type>(input);
            }
        }

        basic_string<CharType> to_string(const string_type &input) const {
            if constexpr (type_traits::type_relations::is_same_v<char_type, CharType>) {
                return basic_string<CharType>(input.begin(), input.end());
            } else {
                return convert_encoding<char_type, CharType>(basic_string_view<char_type>(input.data(), input.size()));
            }
        }

    private:
        template <typename FromChar, typename ToChar>
        string_type convert_encoding(basic_string_view<FromChar> input) const {
            basic_string<char> utf8 = to_utf8<FromChar>(input);
            return from_utf8<ToChar>(utf8);
        }

        template <typename FromChar>
        string_type to_utf8(basic_string_view<FromChar> input) const {
            if constexpr (type_traits::type_relations::is_same_v<FromChar, char>) {
                // 假设 char 已经是 UTF-8
                return string_type{input.begin(), input.end()};
            }
#if RAINY_HAS_CXX20
            else if constexpr (type_traits::type_relations::is_same_v<FromChar, char8_t>) {
                return string_type{reinterpret_cast<const char *>(input.data()), input.size()};
            }
#endif
            else if constexpr (type_traits::type_relations::is_same_v<FromChar, wchar_t>) {
                // wchar_t -> UTF-8
                using wide_allocator = rebind_alloc<allocator, wchar_t>;
                if constexpr (is_std) {
                    // 目标是 std::basic_string 体系
                    using converter_t = wstring_convert<codecvt_utf8<wchar_t>, std::basic_string, wchar_t, traits_template,
                                                        wide_allocator, char_allocator>;
                    converter_t converter;
                    std::basic_string<char, traits_template<char>, char_allocator> std_utf8 =
                        converter.to_bytes(input.data(), input.data() + input.size());
                    return string_type{std_utf8.begin(), std_utf8.end()};
                } else {
                    // 目标是自定义 basic_string 体系
                    using converter_t =
                        wstring_convert<codecvt_utf8<wchar_t>, basic_string, wchar_t, traits_template, wide_allocator, char_allocator>;
                    converter_t converter;
                    return converter.to_bytes(input.data(), input.data() + input.size());
                }
            } else if constexpr (type_traits::type_relations::is_same_v<FromChar, char16_t>) {
                // char16_t -> UTF-8
                using u16_allocator = rebind_alloc<allocator, char16_t>;

                if constexpr (is_std) {
                    using converter_t = wstring_convert<codecvt_utf8<char16_t>, std::basic_string, char16_t, traits_template,
                                                        u16_allocator, char_allocator>;
                    converter_t converter;
                    std::basic_string<char, traits_template<char>, char_allocator> std_utf8 =
                        converter.to_bytes(input.data(), input.data() + input.size());
                    return string_type{std_utf8.begin(), std_utf8.end()};
                } else {
                    using converter_t = wstring_convert<codecvt_utf8<char16_t>, basic_string, char16_t, traits_template, u16_allocator,
                                                        char_allocator>;
                    converter_t converter;
                    return converter.to_bytes(input.data(), input.data() + input.size());
                }
            } else if constexpr (type_traits::type_relations::is_same_v<FromChar, char32_t>) {
                // char32_t -> UTF-8
                using u32_allocator = rebind_alloc<allocator, char32_t>;
                if constexpr (is_std) {
                    using converter_t = wstring_convert<codecvt_utf8<char32_t>, std::basic_string, char32_t, traits_template,
                                                        u32_allocator, char_allocator>;
                    converter_t converter;
                    std::basic_string<char, traits_template<char>, char_allocator> std_utf8 =
                        converter.to_bytes(input.data(), input.data() + input.size());
                    return string_template<char8_t, traits, allocator>(std_utf8.begin(), std_utf8.end());
                } else {
                    using converter_t = wstring_convert<codecvt_utf8<char32_t>, basic_string, char32_t, traits_template, u32_allocator,
                                                        char_allocator>;
                    converter_t converter;
                    return converter.to_bytes(input.data(), input.data() + input.size());
                }
            } else {
                static_assert(type_traits::implements::always_false<FromChar>, "Unsupported source character type");
            }
        }

        template <typename ToChar>
        string_type from_utf8(const basic_string<char> &utf8) const {
            if constexpr (type_traits::type_relations::is_same_v<ToChar, char>) {
                return string_type{utf8.data(), utf8.size()};
            }
#if RAINY_HAS_CXX20
            else if constexpr (type_traits::type_relations::is_same_v<ToChar, char8_t>) {
                return string_type{reinterpret_cast<const char *>(utf8.data()), utf8.size()};
            }
#endif
            else if constexpr (type_traits::type_relations::is_same_v<ToChar, wchar_t>) {
                // UTF-8 -> wchar_t
                using wide_allocator = rebind_alloc<allocator, wchar_t>;

                if constexpr (is_std) {
                    using converter_t = wstring_convert<codecvt_utf8<wchar_t>, std::basic_string, wchar_t, traits_template,
                                                        wide_allocator, char_allocator>;
                    converter_t converter;
                    std::basic_string<wchar_t, traits_template<wchar_t>, wide_allocator> std_result =
                        converter.from_bytes(utf8.data(), utf8.data() + utf8.size());
                    return string_type{std_result.begin(), std_result.end()};
                } else {
                    using converter_t =
                        wstring_convert<codecvt_utf8<wchar_t>, basic_string, wchar_t, traits_template, wide_allocator, char_allocator>;
                    converter_t converter;
                    return converter.from_bytes(utf8.data(), utf8.data() + utf8.size());
                }
            } else if constexpr (type_traits::type_relations::is_same_v<ToChar, char16_t>) {
                // UTF-8 -> char16_t
                using u16_allocator = rebind_alloc<allocator, char16_t>;

                if constexpr (is_std) {
                    using converter_t = wstring_convert<codecvt_utf8<char16_t>, std::basic_string, char16_t, traits_template,
                                                        u16_allocator, char_allocator>;
                    converter_t converter;
                    std::basic_string<char16_t, traits_template<char16_t>, u16_allocator> std_result =
                        converter.from_bytes(utf8.data(), utf8.data() + utf8.size());
                    return string_type{std_result.begin(), std_result.end()};
                } else {
                    using converter_t = wstring_convert<codecvt_utf8<char16_t>, basic_string, char16_t, traits_template, u16_allocator,
                                                        char_allocator>;
                    converter_t converter;
                    return converter.from_bytes(utf8.data(), utf8.data() + utf8.size());
                }
            } else if constexpr (type_traits::type_relations::is_same_v<ToChar, char32_t>) {
                // UTF-8 -> char32_t
                using u32_allocator = rebind_alloc<allocator, char32_t>;

                if constexpr (is_std) {
                    using converter_t = wstring_convert<codecvt_utf8<char32_t>, std::basic_string, char32_t, traits_template,
                                                        u32_allocator, char_allocator>;
                    converter_t converter;
                    std::basic_string<char32_t, traits_template<char32_t>, u32_allocator> std_result =
                        converter.from_bytes(utf8.data(), utf8.data() + utf8.size());
                    return string_type{std_result.begin(), std_result.end()};
                } else {
                    using converter_t = wstring_convert<codecvt_utf8<char32_t>, basic_string, char32_t, traits_template, u32_allocator,
                                                        char_allocator>;
                    converter_t converter;
                    return converter.from_bytes(utf8.data(), utf8.data() + utf8.size());
                }
            } else {
                static_assert(type_traits::implements::always_false<ToChar>, "Unsupported target character type");
            }
        }
    };

    template <typename TargetTypeChar, typename CharType>
    struct lexical_caster<
        basic_string_view<TargetTypeChar>, CharType,
        type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<TargetTypeChar, CharType> && is_char<CharType>>> {

        basic_string_view<TargetTypeChar> convert(basic_string_view<CharType> input) const {
            return input;
        }

        basic_string<CharType> to_string(basic_string_view<TargetTypeChar> input) const {
            return input;
        }
    };

    template <typename TargetTypeChar, typename CharType>
    struct lexical_caster<
        std::basic_string_view<TargetTypeChar>, CharType,
        type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<TargetTypeChar, CharType> && is_char<CharType>>> {

        std::basic_string_view<TargetTypeChar> convert(basic_string_view<CharType> input) const {
            return std::basic_string_view<TargetTypeChar>{input.data(), input.size()};
        }

        basic_string<CharType> to_string(std::basic_string_view<TargetTypeChar> input) const {
            return {input.data(), input.size()};
        }
    };

    template <typename TargetTypeChar, typename CharType>
    struct lexical_caster<
        const TargetTypeChar *, CharType,
        type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<TargetTypeChar, CharType> && is_char<CharType>>> {

        basic_string_view<TargetTypeChar> convert(basic_string_view<CharType> input) const {
            return input.data();
        }

        basic_string<CharType> to_string(const TargetTypeChar *const input) const {
            return input;
        }
    };
}

namespace rainy::foundation::text {


    // Todo...
    /*template <typename Target, typename Source>
    Target lexical_cast(const Source &arg){}*/

    template <typename Target, typename CharType, typename Traits>
    Target lexical_cast(const basic_string_view<CharType, Traits> str, std::size_t count) {
        return lexical_caster<Target, char>{}.convert({str.data(), count});
    }

    template <typename Target, typename CharType, typename Traits>
    Target lexical_cast(const basic_string_view<CharType, Traits> str) {
        return lexical_caster<Target, char>{}.convert(str);
    }

    template <typename Target, typename CharType>
    Target lexical_cast(const CharType *str) {
        if constexpr (implements::has_lexical_caster<Target, CharType>) {
            return lexical_caster<Target, CharType>{}.convert(str);
        } else {
            static_assert(implements::has_lexical_caster<Target, char>,
                          "Cannot find the lexical caster, ensure you added lexial caster.");
            if constexpr (type_traits::type_relations::is_same_v<Target, CharType>) {
                return lexical_caster<Target, char>{}.convert(str);
            } else {
                wstring_convert<codecvt_utf8<CharType>, basic_string, CharType> converter; // not std::wstring_convert
                return lexical_caster<Target, char>{}.convert(converter.to_bytes(str));
            }
        }
    }

    template <typename Ty, typename Char>
    basic_string<Char> lexical_to_string(const Ty &value) {
    }

    // todo...
    // template <typename Target, typename Source>
    // bool try_lexical_convert(const Source &arg, Target &result){}

    template <typename CharType, typename Traits, typename Target>
    bool try_lexical_convert(const basic_string_view<CharType, Traits> str, std::size_t count, Target &result) {
    }

    // todo...
    /*template <typename Ty, typename Char>
    bool try_lexical_to_string(const Ty &value, std::basic_string<char>& result){}*/
}

#endif
