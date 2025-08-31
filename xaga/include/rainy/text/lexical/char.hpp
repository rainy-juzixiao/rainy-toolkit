#pragma once
#ifndef _CLOUDGAMEFIX_foundation_USER_LEXICAL_USER_
#define _CLOUDGAMEFIX_foundation_USER_LEXICAL_USER_
#include <map>
#include <string>
#include <format>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <rainy/text/hashed_string.hpp>
#include <rainy/text/number.hpp>

namespace rainy::text::implements {
    inline constexpr int OpenCurly = '{';
    inline constexpr int CloseCurly = '}';
    inline constexpr int OpenBracket = '[';
    inline constexpr int CloseBracket = ']';
    inline constexpr int OpenParen = '(';
    inline constexpr int CloseParen = ')';
    inline constexpr int OpenQuote = '"';
    inline constexpr int CloseQuote = '"';

    struct lexical_cast_error : public std::runtime_error {
        lexical_cast_error(std::string_view type) :
            std::runtime_error(std::format("cannot parse as '{}'", type)) {
        }

        lexical_cast_error(std::string_view type, std::string_view msg) :
            std::runtime_error(std::format("cannot parse as '{}': {}", type, msg)) {
        }
    };

    template <typename CharType, typename Iter>
    constexpr bool matching_delims(Iter iter, Iter end) {
        if (iter == end) {
            return false;
        }
        CharType first = *iter;
        if (first == OpenQuote || first == OpenCurly || first == OpenBracket || first == OpenParen) {
            CharType last = *(end - 1);
            return last == CloseQuote || last == CloseCurly || last == CloseBracket || last == CloseParen;
        }
        return false;
    }

    template <typename Iter>
    constexpr std::pair<Iter, Iter> unwrap_helper(Iter begin, Iter end) {
        begin = std::find_if_not(begin, end, [](char c) { return std::isspace(c); });
        end = std::find_if_not(std::make_reverse_iterator(end), std::make_reverse_iterator(begin), [](char c) {
                  return std::isspace(c);
              }).base();
        return {begin, end};
    }

    template <typename CharType>
    constexpr std::basic_string_view<CharType> unwrap_helper(std::basic_string_view<CharType> str) {
        auto [b, e] = unwrap_helper(str.begin(), str.end());
        return {b, e};
    }

    template <class Iter>
    constexpr std::pair<Iter, Iter> unwrap(Iter begin, Iter end) {
        auto [b, e] = unwrap_helper(begin, end);
        if (b < e && matching_delims(b, e)) {
            ++b;
            --e;
        }
        return {b, e};
    }

    template <typename CharType, typename T>
    T parse_integral(std::basic_string_view<CharType> input, std::string_view name) {
        auto [ok, number] = text::parse_integer(input);
        if (!ok) {
            throw lexical_cast_error(name);
        }
        return static_cast<T>(number);
    }

    template <typename T>
    constexpr T parse_floating_point(std::string_view input, std::string_view name) {
        auto [ok, number] = text::parse_float(input);
        if (!ok) {
            throw lexical_cast_error(input, name);
        }
        return static_cast<T>(number);
    }
}

namespace rainy::text::lexical {
    template <typename CharType, typename T>
    struct lexical_caster {};

    template <>
    struct lexical_caster<char, bool> {
        bool convert(std::string_view raw_input) const {
            std::string_view input = implements::unwrap_helper(raw_input);
            text::hashed_string hashed{input};
            rainy_switch_hashed_string(hashed) {
                case rainy_match_hashed_string("0"):
                case rainy_match_hashed_string("f"):
                case rainy_match_hashed_string("F"):
                case rainy_match_hashed_string("false"):
                    return false;
                case rainy_match_hashed_string("1"):
                case rainy_match_hashed_string("t"):
                case rainy_match_hashed_string("T"):
                case rainy_match_hashed_string("true"):
                    return true;
            };
            throw implements::lexical_cast_error("bool");
        }

        std::string to_string(bool input) const {
            return input ? "true" : "false";
        }
    };

    template <>
    struct lexical_caster<char, char> {
        constexpr char convert(std::string_view input) const {
            if (input.size() != 1)
                throw implements::lexical_cast_error("char");
            return input[0];
        }

        constexpr std::string to_string(char input) const {
            return {input};
        }
    };

    template <typename CharType,std::integral T>
    struct lexical_caster<CharType, T> {
        constexpr T convert(std::basic_string_view<CharType> input) const {
            return implements::parse_integral<CharType, T>(implements::unwrap_helper(input), "integral");
        }

        constexpr std::string to_string(T value) const {
            using traits_type = text::char_traits<CharType>;
            bool negative = value < 0;
            auto abs_value = negative ? -static_cast<std::uint64_t>(value) : static_cast<std::uint64_t>(value);
            std::basic_string<char> buffer(32, '\0');
            std::size_t pos = sizeof(buffer) / sizeof(buffer[0]);
            buffer[--pos] = '\0';
            while (abs_value > 0) {
                buffer[--pos] = traits_type::to_char_type(static_cast<typename traits_type::char_int_type>('0' + abs_value % 10));
                abs_value /= 10;
            }
            if (negative) {
                buffer[--pos] = traits_type::to_char_type(static_cast<typename traits_type::char_int_type>('-'));
            }
            return buffer;
        }
    };

    template <std::floating_point T>
    struct lexical_caster<char, T> {
        constexpr T convert(std::string_view input) const {
            return implements::parse_floating_point<T>(implements::unwrap_helper(input), "floating point");
        }

        constexpr std::string to_string(T input) const {
            return std::to_string(input);
        }
    };

    template <>
    struct lexical_caster<char, const char *> {
        constexpr const char *convert(std::string_view s) const {
            return s.data();
        }

        constexpr const char *to_string(std::string &value) const {
            return value.c_str();
        }
    };

    template <>
    struct lexical_caster<char, std::string> {
        constexpr std::string convert(std::string_view s) const {
            return std::string(s);
        }

        constexpr std::string to_string(const std::string &value) const {
            return value;
        }
    };

    template <typename Ty, std::size_t N>
    struct lexical_caster<char, std::array<Ty, N>> {
        constexpr std::array<Ty, N> convert(std::string_view s) const {
            const char *iter = s.data();
            const char *end = iter + s.size();
            std::size_t idx{};
            std::array<Ty, N> arr{};
            while (iter < end) {
                if (idx >= N) {
                    throw implements::lexical_cast_error("std::array", "too many elements");
                }
                const char *delim = std::find(iter, end, ',');
                if (iter < delim) {
                    arr[idx] = lexical_caster<char,Ty>{}.convert({iter, static_cast<std::size_t>(delim - iter)});
                    ++idx;
                }
                iter = delim + 1;
            }
            if (idx < N)
                throw implements::lexical_cast_error("std::array", "too few elements");
            return arr;
        }

        constexpr std::string to_string(const std::array<Ty, N> &arr) const {
            std::string r = "[";
            for (std::size_t i = 0; i < N; ++i) {
                if (i > 0)
                    r += ",";
                r += lexical_caster<char,Ty>{}.to_string(arr[i]);
            }
            r += "]";
            return r;
        }
    };

    template <typename Ty>
    struct lexical_caster<char, std::vector<Ty>> {
        constexpr std::vector<Ty> convert(std::string_view s) const {
            const char *iter = s.data();
            const char *end = iter + s.size();
            std::size_t idx{};
            std::vector<Ty> arr{};
            while (iter < end) {
                const char *delim = std::find(iter, end, ',');
                if (iter < delim)
                    arr.emplace_back(lexical_caster<Ty>{}.convert({iter, static_cast<std::size_t>(delim - iter)}));
                iter = delim + 1; // Move past the delimiter
            }
            return arr;
        }

        constexpr std::string to_string(const std::vector<Ty> &arr) const {
            std::string r = '[';
            for (std::size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) {
                    r += ',';
                }
                r += lexical_caster<char, Ty>{}.to_string(arr[i]);
            }
            r += ']';
            return r;
        }
    };

    template <typename Ty>
    struct lexical_caster<char, std::list<Ty>> {
        constexpr std::list<Ty> convert(std::string_view s) const {
            const char *iter = s.data();
            const char *end = iter + s.size();
            std::size_t idx{};
            std::list<Ty> arr{};
            while (iter < end) {
                const char *delim = std::find(iter, end, ',');
                if (iter < delim) {
                    arr.emplace_back(lexical_caster<Ty>{}.convert({iter, static_cast<std::size_t>(delim - iter)}));
                }
                iter = delim + 1; // Move past the delimiter
            }
            return arr;
        }

        constexpr std::string to_string(const std::list<Ty> &arr) const {
            std::string r = "[";
            auto it = arr.begin();
            while (it != arr.end()) {
                if (it != arr.begin())
                    r += ",";
                r += lexical_caster<Ty>{}.to_string(*it);
                ++it;
            }
            r += "]";
            return r;
        }
    };

    template <typename T1, typename T2>
    struct lexical_caster<char, std::pair<T1, T2>> {
        constexpr std::pair<T1, T2> convert(std::string_view s) const {
            const char *iter = s.data();
            const char *end = iter + s.size();
            const char *delim = std::find(iter, end, ':');
            if (delim == end) {
                throw implements::lexical_cast_error(s, "std::pair", "too few elements");
            }
            T1 first = lexical_caster<T1>{}.convert({iter, static_cast<std::size_t>(delim - iter)});
            ++delim; // Move past the delimiter
            T2 second = lexical_caster<T2>{}.convert({delim, static_cast<std::size_t>(end - delim)});
            return {first, second};
        }

        constexpr std::string to_string(const std::pair<T1, T2> &p) const {
            return lexical_caster<T1>{}.to_string(p.first) + "," + lexical_caster<T2>{}.to_string(p.second);
        }
    };

    template <typename Key, typename Value>
    struct lexical_caster<char, std::map<Key, Value>> {
        constexpr std::map<Key, Value> convert(std::string_view s) const {
            const char *iter = s.data();
            const char *end = iter + s.size();
            std::map<Key, Value> map;
            while (iter < end) {
                const char *delim = std::find(iter, end, ',');
                if (delim == iter)
                    throw implements::lexical_cast_error(s, "std::map", "invalid format");
                map.emplace(lexical_caster<std::pair<Key, Value>>{}.convert({iter, static_cast<std::size_t>(delim - iter)}));
                iter = delim + 1;
            }
            return map;
        }

        constexpr std::string to_string(const std::map<Key, Value> &map) const {
            std::string result = "{";
            auto it = map.begin();
            while (it != map.end()) {
                if (it != map.begin())
                    result += ", ";
                result += lexical_caster<std::pair<Key, Value>>{}.to_string(*it);
                ++it;
            }
            result += "}";
            return result;
        }
    };

    template <typename Key, typename Value>
    struct lexical_caster<char, std::unordered_map<Key, Value>> {
        constexpr std::unordered_map<Key, Value> convert(std::string_view s) const {
            const char *iter = s.data();
            const char *end = iter + s.size();
            std::unordered_map<Key, Value> map;
            while (iter < end) {
                const char *delim = std::find(iter, end, ',');
                if (delim == iter)
                    throw implements::lexical_cast_error(s, "std::map", "invalid format");
                map.emplace(lexical_caster<std::pair<Key, Value>>{}.convert({iter, static_cast<std::size_t>(delim - iter)}));
                iter = delim + 1;
            }
            return map;
        }

        constexpr std::string to_string(const std::unordered_map<Key, Value> &map) const {
            std::string result = "{";
            auto it = map.begin();
            while (it != map.end()) {
                if (it != map.begin())
                    result += ", ";
                result += lexical_caster<std::pair<Key, Value>>{}.to_string(*it);
                ++it;
            }
            result += "}";
            return result;
        }
    };
}

#endif